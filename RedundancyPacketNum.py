import numpy as np


# 日志文件预处理，保存在list中
def preHandle(logPath):
    consumerList = []
    forwarderList = []
    producerList = []
    f = open(logPath)
    line = f.readline()
    while line:
        if "Consumer" in line:
            consumerList.append(line.strip())
        # 排除包含localhost的forwarder行，在后面forwarder冗余包计算会用到
        elif ("Forwarder" in line) and ("localhost" not in line):
            forwarderList.append(line.strip())
        elif "Producer" in line:
            producerList.append(line.strip())
        line = f.readline()
    f.close()
    return consumerList, forwarderList, producerList


# 获取真正需要的数据包总数，兴趣包总数
def getRealNumPacket(consumerList):
    realNumPacket = 0
    # consumer生成的兴趣包总数
    for string in consumerList:
        if "SendPacket():" in string:
            realNumPacket += 1
    print("真实需要的数据包总数为：" + str(realNumPacket))
    return realNumPacket


# 计算冗余包总数
def getRedundancyNumPacket(consumerList, forwarderList):
    redundancyNumPacket = 0

    # 计算consumer节点的冗余包数量
    redundancyNumPacketConsumer = getRedundancyNumPacketFromConsumer(consumerList)
    print("所有consumer节点产生的数据包冗余数量为：" + str(redundancyNumPacketConsumer))

    # 计算forwarder节点的冗余包数量
    redundancyNumPacketForwarder = getRedundancyNumPacketFromForwarder(forwarderList)
    print("所有forwarder节点产生的数据包冗余数量为：" + str(redundancyNumPacketForwarder))

    redundancyNumPacket = redundancyNumPacketConsumer + redundancyNumPacketForwarder
    print("冗余包总数为：" + str(redundancyNumPacket))
    return redundancyNumPacket


# 计算consumer节点的冗余包数量
def getRedundancyNumPacketFromConsumer(consumerList):
    redundancyNumPacketConsumer = 0
    consumerDict = {}
    for str in consumerList:
        if "DATA for" in str:
            consumerIndex = str.split(" ")[1]
            consumerReceiveData = str.split("DATA for ")[1]
            # 字典有该consumer节点的key
            if consumerDict.get(consumerIndex):
                # 该consumer节点已经接收过该数据包
                if consumerDict[consumerIndex].get(consumerReceiveData):
                    consumerDict[consumerIndex][consumerReceiveData] += 1
                else:
                    consumerDict[consumerIndex][consumerReceiveData] = 1
            else:
                consumerDict.setdefault(consumerIndex, {})[consumerReceiveData] = 1

    # 获取所有consumer节点的values，values也是字典，key为收到的数据包路径，value为收到该数据包的次数
    consumerDictValue = list(consumerDict.values())
    for i in range(len(consumerDictValue)):
        arr = np.array(list(consumerDictValue[i].values()))
        total = arr.sum()
        # 每个数据包都要-1，整体-长度
        redundancyNumPacketConsumer += total - len(consumerDictValue[i])
    return redundancyNumPacketConsumer


# 计算forwarder节点的冗余包数量
def getRedundancyNumPacketFromForwarder(forwarderList):
    redundancyNumPacketForwarder = 0
    forwarderDict = {}

    for str in forwarderList:
        if "onIncomingData in" in str:
            forwarderIndex = str.split(" ")[1]
            dataInfo = str.split("data=")[1]
            index = dataInfo.find("dataSize")
            # 判断是否包含dataSize
            if index == -1:
                forwarderReceiveData = str.split("data=")[1]
            else:
                forwarderReceiveData = dataInfo[:index]

            # 字典有该forwarder节点的key
            if forwarderDict.get(forwarderIndex):
                # 该forwarder节点已经接收过该数据包
                if forwarderDict[forwarderIndex].get(forwarderReceiveData):
                    forwarderDict[forwarderIndex][forwarderReceiveData] += 1
                else:
                    forwarderDict[forwarderIndex][forwarderReceiveData] = 1
            else:
                forwarderDict.setdefault(forwarderIndex, {})[forwarderReceiveData] = 1

    # 获取所有forwarder节点的values，values也是字典，key为收到的数据包路径，value为收到该数据包的次数
    forwarderDictValue = list(forwarderDict.values())
    for i in range(len(forwarderDictValue)):
        arr = np.array(list(forwarderDictValue[i].values()))
        total = arr.sum()
        # 每个数据包都要-1，整体-长度
        redundancyNumPacketForwarder += total - len(forwarderDictValue[i])
    return redundancyNumPacketForwarder


# 计算冗余度
def getRedundancy(consumerList, forwarderList):
    realNumPacket = getRealNumPacket(consumerList)
    redundancyNumPacket = getRedundancyNumPacket(consumerList, forwarderList)
    redundancy = redundancyNumPacket / (redundancyNumPacket + realNumPacket)
    return redundancy


if __name__ == "__main__":
    logPath = "ws25out.log"
    consumerList, forwarderList, producerList = preHandle(logPath)
    redundancy = getRedundancy(consumerList, forwarderList)
    print("冗余度为：" + str(redundancy))
