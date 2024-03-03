#include "BBlock.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

namespace ndnbcStatusUtils
{
    // 获取本地最高区块号
    int
    getLocalHighestBlockNumber(const std::list<ns3::BBlock> &blockChain)
    {
        if (blockChain.empty())
        {
            return 0; // 如果本地没有区块，返回0
        }
        else
        {
            return blockChain.back().blockNumber;
        }
    }

    // 比较本地最高区块号和收到的区块号，并返回差值区块的哈希值列表
    std::vector<int>
    compare2FindHighestBlockNumbers(int receivedBlockNumber, int localHighestBlockNumber, const std::list<ns3::BBlock> &blockChain)
    {
        std::vector<int> hashList;

        if (localHighestBlockNumber > receivedBlockNumber)
        {
            // 本地区块号较新
            for (const auto &block : blockChain)
            {
                if (block.blockNumber > receivedBlockNumber)
                {
                    hashList.push_back(block.hashThis);
                }
            }
        }

        return hashList;
    }
    std::vector<std::string>
    spreadString(std::string inputString)
    {
        // 使用 istringstream 分割字符串
        std::istringstream iss(inputString);
        std::vector<std::string> result;

        // 分割字符串并存储到 vector 中
        std::string part;
        while (std::getline(iss, part, '/'))
        {
            result.push_back(part);
        }

        return result;
    }

    // 生成hashList内容的字付串
    std::string
    generateHashListContentString(std::vector<int> &hashList)
    {
        std::string hashListContent;
        for (auto &it : hashList)
        {
            hashListContent += " " + std::to_string(it);
        }
        return hashListContent;
    }

    // 从string中解析出hashList
    std::vector<int>
    parseHashListContent(const std::string &hashListContent)
    {
        std::vector<int> resultList;

        // 使用 std::istringstream 进行字符串流的解析
        std::istringstream iss(hashListContent);

        int intValue;
        while (iss >> intValue)
        {
            resultList.push_back(intValue);
            // 检查是否遇到非数字字符
            if (iss.peek() == ' ')
            {
                iss.ignore();
            }
        }
        return resultList;
    }

    // 根据 hashThis 查找特定区块
    ns3::BBlock
    findBlockByNum(const std::list<ns3::BBlock> &blockChain, int targetNum)
    {
        ns3::BBlock targetBlock;
        for (auto &it : blockChain)
        {
            if (it.blockNumber == targetNum)
            {
                targetBlock = it;
            }
        }
        return targetBlock; // 如果未找到，返回末尾迭代器
    }

    std::string
    getLastWord(const std::string &inputString)
    {
        std::istringstream iss(inputString);
        std::string lastWord;

        // 使用 '/' 作为分隔符
        while (std::getline(iss, lastWord, '/'))
        {
            // 如果不是空字符串，更新最后一个单词
            if (!lastWord.empty())
            {
                lastWord = lastWord;
                // 最后一个单词可能包含其他非字母数字字符，根据需要进行处理
            }
        }
        return lastWord;
    }
}