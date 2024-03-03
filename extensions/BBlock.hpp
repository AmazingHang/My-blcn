#ifndef BBLOCK_H
#define BBLOCK_H

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/ndnSIM/ndn-cxx/lp/tags.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

using namespace std;

enum AppMode
{
  MINER,
  USER
};

namespace ns3
{

#define SIZE_BEGIN 1
#define SIZE_END 2
#define CONTENT_BEGIN 3

  struct BBlock
  {
    string creator;
    int sz; // max block size
    int hashLast, hashThis;
    int pow;
    int blockNumber;

    vector<string>
        trSource, trDest;

    BBlock() : hashLast(0), hashThis(0), pow(0), blockNumber(0){};
  };

} // namespace ns3

#endif
