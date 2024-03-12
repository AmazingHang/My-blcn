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
        topologyReader.SetFileName("/home/ndnSIM/t-blockNDN-v2/topology/topology20.txt");
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

        for (int i = 0; i < 20; i++)
        {
            if (i == 1)
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[1], 50);
            }
            else if (i == 2)
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[2], 80);
            }
            else if (i == 3)
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[3], 70);
            }
            else if (i == 4)
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[4], 90);
            }
            else if (i == 6)
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[6], 80);
            }
            else if (i == 11)
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[11], 80);
            }
            else if (i == 12)
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[12], 70);
            }
            else if (i == 14)
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[14], 90);
            }
            else if (i == 15)
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[15], 80);
            }
            else
            {
                Simulator::Schedule(Seconds(1.000), &ndnBlockchainApp::makeBlockchain, apps[i], 100);
            }
        }

        Simulator::Schedule(Seconds(5.0), &ndnBlockchainApp::pullUpdateBCStatus, apps[1]);

        Simulator::Schedule(Seconds(10.0), &ndnBlockchainApp::NewBlockandPush, apps[14]);
        Simulator::Schedule(Seconds(15.0), &ndnBlockchainApp::NewBlockandPush, apps[14]);
        Simulator::Schedule(Seconds(20.0), &ndnBlockchainApp::NewBlockandPush, apps[14]);
        Simulator::Schedule(Seconds(25.0), &ndnBlockchainApp::NewBlockandPush, apps[14]);
        Simulator::Schedule(Seconds(30.0), &ndnBlockchainApp::NewBlockandPush, apps[14]);
        Simulator::Schedule(Seconds(35.0), &ndnBlockchainApp::NewBlockandPush, apps[14]);
        Simulator::Schedule(Seconds(40.0), &ndnBlockchainApp::NewBlockandPush, apps[14]);

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