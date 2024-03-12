#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/integer.h"

#include "ndnBlockchainApp.hpp"

namespace ns3
{
  int main(int argc, char *argv[])
  {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    int startTime = 0, endTime = 500;

    // 导入拓扑
    AnnotatedTopologyReader topologyReader("", 150);
    topologyReader.SetFileName("/home/ndnSIM/t-blockNDN-v2/topology/ws25.txt");
    topologyReader.Read();

    NodeContainer allNodes = topologyReader.GetNodes(); // 取得节点

    LogComponentEnable("ndnBlockchainApp", LOG_LEVEL_DEBUG);

    // 安装ndnhelper
    ndn::StackHelper ndnHelper;
    ndnHelper.SetDefaultRoutes(true);
    ndnHelper.setPolicy("nfd::cs::lru");
    ndnHelper.setCsSize(1000);
    ndnHelper.InstallAll();

    ndn::StrategyChoiceHelper::InstallAll("/ndn.blockchain",
                                          "/localhost/nfd/strategy/multicast");

    ndn::GlobalRoutingHelper ndnRoutingHelper;
    ndnRoutingHelper.InstallAll();

    ndn::AppHelper appHelper("ndnBlockchainApp");
    vector<Ptr<ndnBlockchainApp>> apps;

    // 在每个节点上安装应用
    for (auto &node : allNodes)
    {
      std::string nodeName = Names::FindName(node);

      Ptr<ndnBlockchainApp> app = CreateObject<ndnBlockchainApp>();
      app->SetStartTime(Seconds(startTime));
      app->SetStopTime(Seconds(endTime));
      app->SetAttribute("NodeName", StringValue(nodeName));
      app->SetAttribute("cssz", IntegerValue(100));

      apps.push_back(app);

      node->AddApplication(app);
      std::string prefix = "/ndn.blockchain/";
      ndnRoutingHelper.AddOrigins(prefix, node);
    }

    ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes();

    // 模拟结果
    // AnimationInterface anim("/home/ndnSIM/t-blockNDN/results/animation.xml");
    for (int i = 0; i < 25; i++)
    {
      if (i % 5 == 0)
      {
        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[i], 1);
        continue;
      }
      Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[i], 100);
    }

    Simulator::Schedule(Seconds(2.0), &ndnBlockchainApp::pullUpdateBCStatus, apps[0]);

    Simulator::Schedule(Seconds(5.0), &ndnBlockchainApp::NewBlockandPush, apps[10]);
    // Simulator::Schedule(Seconds(10.0), &ndnBlockchainApp::NewBlockandPush, apps[7]);
    // Simulator::Schedule(Seconds(15.0), &ndnBlockchainApp::NewBlockandPush, apps[20]);
    // Simulator::Schedule(Seconds(20.0), &ndnBlockchainApp::NewBlockandPush, apps[5]);
    // Simulator::Schedule(Seconds(25.0), &ndnBlockchainApp::NewBlockandPush, apps[1]);
    // Simulator::Schedule(Seconds(30.0), &ndnBlockchainApp::NewBlockandPush, apps[13]);
    // Simulator::Schedule(Seconds(35.0), &ndnBlockchainApp::NewBlockandPush, apps[6]);
    // Simulator::Schedule(Seconds(40.0), &ndnBlockchainApp::NewBlockandPush, apps[23]);
    // Simulator::Schedule(Seconds(45.0), &ndnBlockchainApp::NewBlockandPush, apps[9]);

    Simulator::Stop(Seconds(endTime * 2));
    // SCHEDULER END
    Simulator::Run();
    Simulator::Destroy();

    return 0;
  }
} // namespace ns3

int main(int argc, char *argv[])
{
  return ns3::main(argc, argv);
}