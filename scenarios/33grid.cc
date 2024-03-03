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
        AnnotatedTopologyReader topologyReader("", 25);
        topologyReader.SetFileName("/home/ndnSIM/t-blockNDN-v2/topology/topology.txt");
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

        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[0], 80);
        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[1], 10);
        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[2], 20);

        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[3], 40);
        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[4], 10);
        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[5], 60);

        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[6], 70);
        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[7], 50);
        Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[8], 10);

        Simulator::Schedule(Seconds(5.0), &ndnBlockchainApp::pullUpdateBCStatus, apps[4]);

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