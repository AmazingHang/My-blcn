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
    enum STATUS
    {
      IDLE,
      UPDATING_TARGET,
      TEMP_UPDATED,
    };
    int localHighestBlockNumber = 0;

    std::list<BBlock> blockChain;
    std::list<BBlock> blockStore;

    std::vector<std::pair<std::string, std::string>> trList;
    BBlock temporaryBlock;
    STATUS my_status = IDLE;
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
    void handleInterest_PullBlockNumber(std::shared_ptr<const ndn::Interest> &interest, int receivedBlockNumber);
    void handlePullNumGreaterThanLocalHighest(int receivedBlockNumber);
    void handlePullNuLessThanLocalHighest(std::shared_ptr<const ndn::Interest> &interest);

    void handleInterest_UpdateSpecific(std::shared_ptr<const ndn::Interest> &interest, int receivedBlockNumber, int receivedTargetNumber);

    void handleData_PullNum(int receivedBlockNumber);
    void handlePullNumForUpdate(int receivedBlockNumber);

    void handleData_ReceivedBlock(BBlock &newBlock, int receivedNum);
  };
} // namespace ns3

#endif