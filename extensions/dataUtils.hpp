#include "BBlock.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>
#include <fstream>

namespace ndnbcDataUtils
{
    // 解析兴趣
    std::vector<std::string>
    decode_interest(std::string interestName)
    {
        std::string word;
        std::vector<std::string> vec;
        for (char &c : interestName)
            if (c == '/')
                c = ' ';
        std::istringstream is(interestName);

        while (is >> word)
        {
            vec.push_back(word);
        }

        return vec;
    }

    // 解析数据
    std::string
    decode_data(std::shared_ptr<const ns3::ndn::Data> &data)
    {
        auto contentBlock = data->getContent();
        std::stringstream ss;
        std::string content, temp;

        ss << contentBlock;
        ss >> temp;

        int status = 0;
        for (int i = 0, sz = temp.size(); i < sz; i++)
        {
            if (temp[i] == '[')
            {
                status = SIZE_BEGIN;
            }
            else if (temp[i] == ']')
            {
                status = SIZE_END;
            }
            else if (temp[i] == '=')
            {
                status = CONTENT_BEGIN;
            }
            else
            {
                if (status != CONTENT_BEGIN)
                    continue;
                std::string hexStr;
                hexStr += temp[i];
                i++;
                hexStr += temp[i];

                char c = stoi(hexStr, 0, 16);
                content += c;
            }
        }

        return content;
    }

    // 从字符串中解析出区块
    ns3::BBlock
    parseBlockFromString(std::string &decodedData)
    {
        ns3::BBlock newBlock;
        std::string word;
        std::istringstream is(decodedData);

        is >> word;
        std::string blockSource = word;
        newBlock.creator = blockSource;

        is >> word; // sz
        newBlock.sz = std::stoi(word);

        is >> word;
        newBlock.hashLast = std::stoi(word);

        is >> word;
        newBlock.hashThis = std::stoi(word);

        is >> word;
        newBlock.pow = std::stoi(word);

        for (int i = 0; i < newBlock.sz; i++)
        {
            is >> word;
            newBlock.trSource.push_back(word);

            is >> word;
            newBlock.trDest.push_back(word);
        }

        return newBlock;
    }

    // 生成区块字符串
    std::string
    generateBlockContentString(ns3::BBlock &block)
    {
        std::string blockContent;
        blockContent += block.creator;
        blockContent += " " + std::to_string(block.sz);
        blockContent += " " + std::to_string(block.hashLast);
        blockContent += " " + std::to_string(block.hashThis);
        blockContent += " " + std::to_string(block.pow);

        for (int i = 0; i < block.sz; i++)
        {
            blockContent += " " + block.trSource[i];
            blockContent += " " + block.trDest[i];
        }
        return blockContent;
    }

    // 从文件中读取交易数据
    std::vector<std::pair<std::string, std::string>> readFromFile(const std::string &filename)
    {
        std::vector<std::pair<std::string, std::string>> trList;
        std::ifstream inputFile(filename);

        if (!inputFile.is_open())
        {
            std::cerr << "Error opening file for reading: " << filename << std::endl;
            return trList;
        }
        // 读取文件内容并解析成交易数据
        std::string line;
        while (std::getline(inputFile, line))
        {
            std::istringstream iss(line);
            std::string source, destination;
            if (iss >> source >> destination)
            {
                trList.push_back(std::make_pair(source, destination));
            }
        }

        inputFile.close();

        return trList;
    }

    // 创建临时区块
    ns3::BBlock
    createTempBlock(std::string &nameOfNode, std::list<ns3::BBlock> &blockChain, std::string filename)
    {
        ns3::BBlock temporaryBlock;
        temporaryBlock.creator = nameOfNode;
        temporaryBlock.hashLast = blockChain.back().hashThis;
        temporaryBlock.hashThis = blockChain.back().hashThis + 1;
        temporaryBlock.pow = 5 * blockChain.size();
        temporaryBlock.blockNumber = blockChain.back().blockNumber + 1;

        // 模拟生成交易
        std::vector<std::pair<std::string, std::string>> trList = readFromFile(filename);

        temporaryBlock.sz = trList.size();

        // 导入所有交易
        for (auto it = trList.begin(); it != trList.end(); ++it)
        {
            temporaryBlock.trSource.push_back(it->first);
            temporaryBlock.trDest.push_back(it->second);
        }
        return temporaryBlock;
    }

    // 把本地区块链保存到文件中
    void
    saveBlockchain2File(std::string nameOfNode, std::list<ns3::BBlock> blockChain)
    {
        const std::string filename = "blockdata/ndnBlockChain-" + nameOfNode + ".json";

        // 读取现有文件中的 JSON 数据
        nlohmann::json jsonBlockchain;

        std::ofstream outputFile(filename, std::ios_base::out | std::ios_base::trunc);

        for (const auto &block : blockChain)
        {
            nlohmann::json jsonBlock; // JSON object to represent each block
            jsonBlock["Creator"] = block.creator;
            jsonBlock["Size"] = block.sz;
            jsonBlock["HashLast"] = block.hashLast;
            jsonBlock["HashThis"] = block.hashThis;
            jsonBlock["Pow"] = block.pow;
            jsonBlock["BlockNumber"] = block.blockNumber;
            // Save transaction details
            nlohmann::json jsonTransactions;
            for (size_t i = 0; i < block.sz; ++i)
            {
                nlohmann::json jsonTransaction;
                jsonTransaction["Source"] = block.trSource[i];
                jsonTransaction["Destination"] = block.trDest[i];
                jsonTransactions.push_back(jsonTransaction);
            }

            jsonBlock["Transactions"] = jsonTransactions;
            // 将整个 JSON 数组写回文件
            jsonBlockchain.push_back(jsonBlock);
        }

        if (outputFile.is_open())
        {
            outputFile << std::setw(4) << jsonBlockchain; // 使用 setw(4) 进行缩进
            outputFile.close();
            return;
        }
        else
        {
            std::cerr << "saving falied" << std::endl;
            return;
        }
    }

    // 存入本地区块链
    void
    addBlock2BC(ns3::BBlock &newBlock,
                std::unordered_map<int, bool> &addedToBC,
                std::list<ns3::BBlock> &blockChain,
                int &localHighestBlockNumber,
                std::string nameOfNode)
    {
        // 如果已存入区块链或者区块不符合条件，则return
        if (addedToBC[newBlock.hashThis] || newBlock.hashLast != blockChain.back().hashThis)
            return;
        newBlock.blockNumber = blockChain.back().blockNumber + 1;
        blockChain.push_back(newBlock); // 把区块存入区块链
        addedToBC[newBlock.hashThis] = true;
        localHighestBlockNumber = blockChain.back().blockNumber;
        // NS_LOG_DEBUG("Block " << blockChain.size() << " added to " << nameOfNode << "\'s blockchain");
        //   ndnbcDataUtils::saveBlockchain2File(nameOfNode, blockChain);
    }

    // 比较函数，用于指定排序规则
    bool
    compare2SortBlocks(const ns3::BBlock &block1, const ns3::BBlock &block2)
    {
        return block1.blockNumber < block2.blockNumber;
    }

    // 存入本地暂存区,并排序
    void
    addBlock2Store(const ns3::BBlock &newBlock,
                   std::unordered_map<int, bool> &addedToBCStore,
                   std::list<ns3::BBlock> &blockStore)
    {
        if (addedToBCStore[newBlock.hashThis])
            return;
        blockStore.push_back(newBlock); // 暂存到 blockStore
        addedToBCStore[newBlock.hashThis] = true;
        blockStore.sort(compare2SortBlocks); // 对传入blockStore的区块进行排序
    }

    // 处理新来的区块，首先把新区块存入区块链，然后判断暂存区中的其他区块是否能加入到区块链
    void
    processNewBlock(ns3::BBlock &newBlock,
                    std::unordered_map<int, bool> &addedToBC,
                    std::list<ns3::BBlock> &blockChain,
                    std::list<ns3::BBlock> &blockStore,
                    int &localHighestBlockNumber,
                    std::string &nameOfNode)
    {
        // 1.存入区块链，清除暂存区中的对应区块
        addBlock2BC(newBlock, addedToBC, blockChain, localHighestBlockNumber, nameOfNode);
        int targetHash = newBlock.hashThis;
        // blockStore.erase(std::remove_if(blockStore.begin(), blockStore.end(), [&targetHash](const ns3::BBlock &block)
        //                                 { return block.hashThis == targetHash; }),
        //                  blockStore.end());
        // 2.判断暂存区中的区块是否能加入到区块链
        for (auto it = blockStore.begin(); it != blockStore.end();)
        {
            if (it->hashLast == blockChain.back().hashThis)
            {
                if (!addedToBC[it->hashThis])
                {
                    addBlock2BC(*it, addedToBC, blockChain, localHighestBlockNumber, nameOfNode);
                    targetHash = it->hashThis;
                    blockStore.erase(std::remove_if(it, blockStore.end(), [&targetHash](const ns3::BBlock &block)
                                                    { return block.hashThis == targetHash; }),
                                     blockStore.end()); // erase 操作会返回指向被删除元素之后的元素的迭代器，因此迭代器已经自动移动到了下一个位置。
                }
                else
                {
                    ++it;
                }
            }
            else
            {
                ++it;
            }
        }
    }
}