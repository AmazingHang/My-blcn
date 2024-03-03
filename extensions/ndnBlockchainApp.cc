#include "ndnbcUtils.hpp"

namespace ns3
{

  TypeId
  ndnBlockchainApp::GetTypeId()
  {
    auto checker = MakeIntegerChecker<int64_t>();
    static TypeId tid =
        TypeId("ndnBlockchainApp")
            .SetParent<ndn::App>()
            .AddAttribute("NodeName", "Name of the Node", StringValue("/"),
                          MakeStringAccessor(&ndnBlockchainApp::nameOfNode), MakeStringChecker())
            .AddAttribute("cssz", "CS Size", IntegerValue((int64_t)50),
                          MakeIntegerAccessor(&ndnBlockchainApp::cssz), checker)
            .AddConstructor<ndnBlockchainApp>();

    return tid;
  }

  void
  ndnBlockchainApp::StartApplication()
  {

    originBlock.sz = 0;
    blockChain.push_back(originBlock);
    temporaryBlock.hashLast = originBlock.hashThis;
    // NS_LOG_INFO("ndnBlockchainApp start: " << nameOfNode);
    ndn::App::StartApplication();
    std::string route = "/ndn.blockchain";
    ndn::FibHelper::AddRoute(GetNode(), route, m_face, 0);
  }

  void
  ndnBlockchainApp::StopApplication()
  {
    ndn::App::StopApplication();
  }

  void
  ndnBlockchainApp::SendPacket(std::string prefix)
  {
    auto newinterest = std::make_shared<ndn::Interest>(prefix);
    newinterest->setNonce(dist(rng));
    newinterest->setInterestLifetime(ndn::time::seconds(2000));
    NS_LOG_INFO("Consumer > Interest for " + prefix);
    m_transmittedInterests(newinterest, this, m_face);
    m_appLink->onReceiveInterest(*newinterest);
  }

  void
  ndnBlockchainApp::SendData(std::shared_ptr<const ndn::Interest> interest, std::string &content)
  {
    auto data = std::make_shared<ndn::Data>(interest->getName());
    data->setFreshnessPeriod(ndn::time::milliseconds(2000));
    data->setContent(std::make_shared<::ndn::Buffer>(content.begin(), content.end()));
    ndn::StackHelper::getKeyChain().sign(*data);
    NS_LOG_INFO("Producer < Send DATA");
    m_transmittedDatas(data, this, m_face);
    m_appLink->onReceiveData(*data);
  }

  void
  ndnBlockchainApp::NewBlock()
  {
    std::string filename = "blockdata/initiation_data.txt";
    temporaryBlock = ndnbcDataUtils::createTempBlock(nameOfNode, blockChain, filename);
    ndnbcDataUtils::addBlock2BC(temporaryBlock, addedToBC, blockChain, localHighestBlockNumber, nameOfNode);
  }

  void
  ndnBlockchainApp::NewBlockandPush()
  {
    if (localHighestBlockNumber < targetNum)
    {
      NS_LOG_DEBUG("更新不可创新块！");
      return;
    }
    if (targetNum < localHighestBlockNumber)
    {
      targetNum = localHighestBlockNumber;
      NS_LOG_DEBUG("newpush 校对");
    }

    std::string filename = "blockdata/initiation_data.txt";
    temporaryBlock = ndnbcDataUtils::createTempBlock(nameOfNode, blockChain, filename);
    ndnbcDataUtils::addBlock2BC(temporaryBlock, addedToBC, blockChain, localHighestBlockNumber, nameOfNode);
    pullUpdateBCStatus();
  }

  void
  ndnBlockchainApp::makeBlockchain(int i)
  {
    while (i > 0)
    {
      NewBlock();
      i--;
    }
    if (targetNum < localHighestBlockNumber)
    {
      targetNum = localHighestBlockNumber;
      NS_LOG_DEBUG("make chain 校对");
    }
  }

  // pull_BlockchainStatus -> update_SpecificBcS -> block_ack
  // 发送请求更新区块链状态的兴趣
  void ndnBlockchainApp::pullUpdateBCStatus()
  {
    std::string prefix = "/ndn.blockchain/";
    prefix += nameOfNode + "/"; // 改：在前缀上添加兴趣名，防止兴趣合并
    prefix += std::to_string(localHighestBlockNumber) + "/";
    prefix += "pull_BlockchainStatus";
    // NS_LOG_DEBUG(nameOfNode << " 进行状态拉取，当前是 :  " << localHighestBlockNumber);
    this->SendPacket(prefix);
  };

  // 进入更新状态，更新到目标区块号
  void ndnBlockchainApp::requestSpecificStatus(int targetNum, int originNum)
  {
    this->targetNum = targetNum;
    std::string basePrefix = "/ndn.blockchain/";
    std::string targetPrefix = std::to_string(targetNum);
    NS_LOG_DEBUG("目标为:" + targetPrefix);
    for (int i = originNum + 1; i <= targetNum; i++)
    {
      std::string prefix = basePrefix + std::to_string(i) + "/" + "update_SpecificBcS";
      this->SendPacket(prefix);
    }
  }

  void
  ndnBlockchainApp::OnInterest(std::shared_ptr<const ndn::Interest> interest)
  {
    ndn::App::OnInterest(interest);
    std::string interestName = "";
    std::stringstream ss;
    ss << interest->getName();
    ss >> interestName;
    NS_LOG_INFO("Recieved Interest " << interestName);

    std::vector<std::string> vec = ndnbcDataUtils::decode_interest(interestName);

    // ToDo  20 -> 80 ,80 -> 20
    if (vec.back() == "pull_BlockchainStatus") // 通过与提取的区块号对比，判断最新状态
    {
      int receivedBlockNumber = std::stoi(vec[2]);
      handleInterest_PullBlockNumber(interest, receivedBlockNumber);
      return;
    }

    if (vec.back() == "update_SpecificBcS") // 如果收到更新兴趣时，比较区块号，进行下一步操作
    {
      int receivedBlockNumber = std::stoi(vec[1]);
      // int receivedTargetNumber = std::stoi(vec[2]);
      handleInterest_UpdateSpecific(interest, receivedBlockNumber, 0);
    }
  }

  void
  ndnBlockchainApp::OnData(std::shared_ptr<const ndn::Data> data)
  {
    App::OnData(data);
    // NS_LOG_FUNCTION(this << data);
    std::string name = data->getName().toUri();
    std::string lastWord = ndnbcStatusUtils::getLastWord(name);
    NS_LOG_INFO("Consumer < DATA for " + name);

    if (lastWord == "pull_BlockchainStatus") // 打包收到的最新区块号，轮询最高区块号
    {
      std::string decodedData = ndnbcDataUtils::decode_data(data);
      int receivedBlockNumber = std::stoi(decodedData);
      handleData_PullNum(receivedBlockNumber);
      return;
    }

    if (lastWord == "update_SpecificBcS") // 如果收到hashList中，则根据此列表发送区块请求
    {
      // NS_LOG_DEBUG("收到update_SpecificBcS数据时的 maxNetNum: " << maxNetNum);
      std::string decodedData = ndnbcDataUtils::decode_data(data);
      std::vector<std::string> ContentVec = ndnbcStatusUtils::spreadString(decodedData);
      BBlock newBlock = ndnbcDataUtils::parseBlockFromString(ContentVec[0]);
      int receivedNum = std::stoi(ContentVec[1]);
      // NS_LOG_DEBUG("receivedBlock : " << ContentVec[0]);
      handleData_ReceivedBlock(newBlock, receivedNum);
      return;
    }

    return;
  }

} // namespace ns3

// TODO:初步判断是兴趣过多的问题