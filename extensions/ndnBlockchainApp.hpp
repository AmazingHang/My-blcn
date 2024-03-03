#ifndef NDNBCAPP_HPP
#define NDNBCAPP_HPP

#include "BBlock.hpp"

#include "ns3/ndnSIM/apps/ndn-app.hpp"

using namespace std;

namespace ns3
{
  class ndnBlockchainApp : public ndn::App
  {
  public:
    BBlock originBlock;
    AppMode appMode;

    std::string nameOfNode;
    int64_t cssz = 50;

    int localHighestBlockNumber = 0;

    std::list<BBlock> blockChain;
    std::list<BBlock> blockStore;

    std::vector<std::pair<std::string, std::string>> trList;
    BBlock temporaryBlock;

    int targetNum = 0; // 目标值
    int maxNetNum = 0; // 已知最大值

    std::unordered_map<int, bool> addedToBC;      // 验证是否已经保存到区块链
    std::unordered_map<int, bool> addedToBCStore; // 验证是否已经保存到暂存区

    static TypeId GetTypeId();
    virtual void StartApplication();
    virtual void StopApplication();
    virtual void OnInterest(std::shared_ptr<const ndn::Interest> interest);
    virtual void OnData(std::shared_ptr<const ndn::Data> contentObject);

    void SendPacket(std::string prefix);
    void SendData(std::shared_ptr<const ndn::Interest> interest, std::string &content);
    void NewBlock();
    void NewBlockandPush();
    void makeBlockchain(int i);
    void pullUpdateBCStatus();
    void requestSpecificStatus(int targetNum, int originNum);

  private:
    void sendBlockAckRequests(const std::vector<int> &hashList);
    void sendGroupedBlockAckRequests(const std::vector<int> &hashList);
    void sendIndividualBlockAckRequests(const std::vector<int> &hashList);
    void sendBlockAckRequestWithDelay(int blockHash, Time &delayTime);
    void sendBlockAckRequest(int blockNumber);

    void handleData_ReceivedBlock(BBlock &newBlock, int receivedNum);
    void handleUpdateCompletion();

    void handleInterest_PullBlockNumber(std::shared_ptr<const ndn::Interest> &interest, int receivedBlockNumber);
    void handleInterest_UpdateSpecific(std::shared_ptr<const ndn::Interest> &interest, int receivedBlockNumber, int receivedTargetNumber);

    void handlePullNumGreaterThanLocalHighest(int receivedBlockNumber);
    void handlePullNuLessThanLocalHighest(std::shared_ptr<const ndn::Interest> &interest);

    // void handleInteres_SpecificNumForUpdate(std::shared_ptr<const ndn::Interest> &interest, int originNum, int receivedBlockNumber);
    void handleSpecificNumGreaterThanLocalHighest(int receivedBlockNumber);
    void handleSpecificNumLessThanOrEqualLocalHighest(std::shared_ptr<const ndn::Interest> &interest, int originNum);

    void handleData_PullNum(int receivedBlockNumber);
    void handlePullNumForUpdate(int receivedBlockNumber);
  };
} // namespace ns3

#endif