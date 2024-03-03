#include "ndnBlockchainApp.hpp"
#include "dataUtils.hpp"
#include "statusUtils.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/ndnSIM/ndn-cxx/lp/tags.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"
#include "ns3/random-variable-stream.h"

#include <random>

#include "ns3/string.h"
#include "ns3/integer.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("ndnBlockchainApp");
  NS_OBJECT_ENSURE_REGISTERED(ndnBlockchainApp);

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type>
      dist(0, std::numeric_limits<uint32_t>::max());

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // A. 处理到来的状态更新兴趣

  void ndnBlockchainApp::handleInterest_PullBlockNumber(std::shared_ptr<const ndn::Interest> &interest, int receivedBlockNumber)
  {
    if (receivedBlockNumber > localHighestBlockNumber)
    {
      handlePullNumGreaterThanLocalHighest(receivedBlockNumber);
    }
    else if (receivedBlockNumber == localHighestBlockNumber)
    {
      return; // 不反应
    }
    else if (receivedBlockNumber < localHighestBlockNumber)
    {
      if (my_status == IDLE)
      {
        pullUpdateBCStatus();
        my_status = TEMP_UPDATED;
      }
      else
      {
        return;
      }
      // handlePullNuLessThanLocalHighest(interest);
    }
  }

  // A.a 处理收到的区块号大于本地最高区块号的情况
  void ndnBlockchainApp::handlePullNumGreaterThanLocalHighest(int receivedBlockNumber)
  {
    switch (my_status)
    {
    case UPDATING_TARGET:
      if (targetNum < receivedBlockNumber) // 如果正在更新
      {
        requestSpecificStatus(receivedBlockNumber, targetNum);
      }
      break;
    case IDLE:
      my_status = UPDATING_TARGET;
      requestSpecificStatus(receivedBlockNumber, localHighestBlockNumber);
      break;
    case TEMP_UPDATED:
      my_status = UPDATING_TARGET;
      requestSpecificStatus(receivedBlockNumber, localHighestBlockNumber);
      break;
    default:
      break;
    }
    return;
  }

  // A.b 处理收到的区块号小于本地最高区块号的情况
  void ndnBlockchainApp::handlePullNuLessThanLocalHighest(std::shared_ptr<const ndn::Interest> &interest)
  {
    if (localHighestBlockNumber < targetNum) // 本地处于更新状态
    {
      NS_LOG_DEBUG("发送本地目标 : " << targetNum);
      std::string numberContent = std::to_string(targetNum);
      SendData(interest, numberContent);
    }
    else // 如果未更新
    {
      NS_LOG_DEBUG("本地区块号是最高区块号 send blockNum : " << localHighestBlockNumber);
      std::string numberContent = std::to_string(localHighestBlockNumber);
      SendData(interest, numberContent);
    }
  }

  // B 处理到来的区块兴趣
  void ndnBlockchainApp::handleInterest_UpdateSpecific(std::shared_ptr<const ndn::Interest> &interest, int receivedBlockNumber, int receivedTargetNumber)
  {
    switch (my_status)
    {
    case IDLE:
      if (receivedTargetNumber > localHighestBlockNumber)
      {
        my_status = UPDATING_TARGET;
        NS_LOG_DEBUG("未更新，收到update兴趣，更新目标值");
        requestSpecificStatus(receivedTargetNumber, targetNum);
      }
      break;
    case TEMP_UPDATED:
      if (receivedTargetNumber > localHighestBlockNumber)
      {
        my_status = UPDATING_TARGET;
        NS_LOG_DEBUG("未更新，收到update兴趣，更新目标值");
        requestSpecificStatus(receivedTargetNumber, targetNum);
      }
      break;
    case UPDATING_TARGET:
      if (receivedTargetNumber > targetNum)
      {
        NS_LOG_DEBUG("正在更新，收到update兴趣，更新目标值");
        requestSpecificStatus(receivedTargetNumber, targetNum);
      }
      break;
    default:
      break;
    }
    // int originNum = std::stoi(vec[3]); // 用于计算差值的区块
    //  NS_LOG_DEBUG("receivedBlockNumber:" << receivedBlockNumber << " localHighestBlockNumber: " << localHighestBlockNumber);
    BBlock requiredBlock = ndnbcStatusUtils::findBlockByNum(blockChain, receivedBlockNumber); // 打包区块内容
    if (requiredBlock.hashThis == 0)                                                          // 如果未找到对应区块，则转发此兴趣区块
    {
      BBlock findBlockStore = ndnbcStatusUtils::findBlockByNum(blockStore, receivedBlockNumber);
      if (findBlockStore.hashThis != 0)
        requiredBlock = findBlockStore;
      else
      {
        NS_LOG_DEBUG("未找到区块");
        return;
      }
    }

    std::string blockContent = ndnbcDataUtils::generateBlockContentString(requiredBlock);
    if (targetNum > localHighestBlockNumber)
    {
      blockContent += "/" + std::to_string(targetNum); // 把本地的最高区块号发送出去
      SendData(interest, blockContent);
    }
    else
    {
      blockContent += "/" + std::to_string(localHighestBlockNumber); // 把本地的最高区块号发送出去
      SendData(interest, blockContent);
    };
    return;
  }

  // C. 处理状态拉取数据
  void ndnBlockchainApp::handleData_PullNum(int receivedBlockNumber)
  {
    if (receivedBlockNumber < localHighestBlockNumber || receivedBlockNumber == localHighestBlockNumber)
    {
      // 如果本地区块号是最高区块号，则直接返回
      return;
    }
    if (receivedBlockNumber > localHighestBlockNumber)
    {
      handlePullNumForUpdate(receivedBlockNumber);
    }
  }
  // C.a 如果本地区块号不是最高区块号，则进入更新状态
  void ndnBlockchainApp::handlePullNumForUpdate(int receivedBlockNumber)
  {
    if (targetNum < receivedBlockNumber) // 如果有更新的目标
    {
      maxNetNum = receivedBlockNumber;

      if (targetNum > localHighestBlockNumber) // 如果正在更新
      {
        NS_LOG_DEBUG("正在更新，收到数据，更新目标值");
        requestSpecificStatus(maxNetNum, targetNum);
      }
      else // 如果未更新
      {
        NS_LOG_DEBUG("未更新，收到数据，更新目标值");
        requestSpecificStatus(maxNetNum, localHighestBlockNumber);
      }
    }
    return;
  }

  // D. 处理新区块数据
  void ndnBlockchainApp::handleData_ReceivedBlock(BBlock &newBlock, int receivedNum)
  {
    if (targetNum < receivedNum) // 如果有更新的目标
    {
      NS_LOG_DEBUG("收到区块后，更新目标值发送区块请求兴趣");
      requestSpecificStatus(receivedNum, targetNum);
    }

    // NS_LOG_DEBUG("目标未完成,目标: " << targetNum << "当前： " << localHighestBlockNumber);
    if (addedToBC[newBlock.hashThis] || addedToBCStore[newBlock.hashThis]) // 已存入区块链或暂存区则return
    {
      NS_LOG_DEBUG("阻止重复存入 ");
      return;
    }

    if (newBlock.hashLast == blockChain.back().hashThis) // 当收到的区块满足要求时，判断是否需要更新暂存区
    {
      ndnbcDataUtils::processNewBlock(newBlock, addedToBC,
                                      blockChain, blockStore,
                                      localHighestBlockNumber,
                                      nameOfNode);
      localHighestBlockNumber = ndnbcStatusUtils::getLocalHighestBlockNumber(blockChain); // 更新最新最高区块数量
                                                                                          // NS_LOG_DEBUG("目标: " << targetNum << "当前： " << localHighestBlockNumber);
      if (localHighestBlockNumber == targetNum)                                           // 当达到目标时，判断是否需要进一步更新
      {
        NS_LOG_DEBUG("完成目标: " << targetNum);
        my_status = IDLE;
      }
      return;
    }

    ndnbcDataUtils::addBlock2Store(newBlock, addedToBCStore, blockStore); // 存入暂存区
    return;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace ns3
