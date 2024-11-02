#include "mobility-controller.h"
#include "packet-trace.h"
#include "statistics-manager.h"
#include "traffic-controller.h"

#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/arp-cache.h"
#include "ns3/core-module.h"
#include "ns3/energy-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/mac48-address.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/packet.h"
#include "ns3/wifi-module.h"

#include <json/json.h>
#include <map>
#include <vector>

struct PacketInfo
{
    uint32_t packetId;
    double sendTime;
    double receiveTime;
    uint32_t size;
    uint32_t destinationNode;
    bool received;
};

// 为每个节点存储发送的数据包信息
std::map<uint32_t, std::vector<PacketInfo>> nodeSentPackets;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FLSSimulation");

void
PrintTrafficStatistics(NodeContainer& nodes)
{
    std::cout << "\n=== Traffic Statistics ===\n";

    uint32_t totalSentPackets = 0;
    uint32_t totalReceivedPackets = 0;
    uint64_t totalSentBytes = 0;
    uint64_t totalReceivedBytes = 0;

    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<Node> node = nodes.Get(i);
        Ptr<FLSApplication> app = DynamicCast<FLSApplication>(node->GetApplication(0));

        if (app)
        {
            const FLSApplication::TrafficStats& stats = app->GetStats();

            double sendDuration = (stats.sentPackets > 0)
                                      ? (stats.lastSentTime - stats.firstSentTime).GetSeconds()
                                      : 0;
            double receiveDuration =
                (stats.receivedPackets > 0)
                    ? (stats.lastReceivedTime - stats.firstReceivedTime).GetSeconds()
                    : 0;

            std::cout << "\nNode " << i << " statistics:\n"
                      << "  Sent:\n"
                      << "    Packets: " << stats.sentPackets << "\n"
                      << "    Bytes: " << stats.sentBytes << "\n";

            if (sendDuration > 0)
            {
                std::cout << "    Send Rate: " << (stats.sentBytes * 8.0) / sendDuration / 1000000
                          << " Mbps\n";
            }

            std::cout << "  Received:\n"
                      << "    Packets: " << stats.receivedPackets << "\n"
                      << "    Bytes: " << stats.receivedBytes << "\n";

            if (receiveDuration > 0)
            {
                std::cout << "    Receive Rate: "
                          << (stats.receivedBytes * 8.0) / receiveDuration / 1000000 << " Mbps\n";
            }

            totalSentPackets += stats.sentPackets;
            totalReceivedPackets += stats.receivedPackets;
            totalSentBytes += stats.sentBytes;
            totalReceivedBytes += stats.receivedBytes;
        }
    }

    std::cout << "\nOverall Statistics:\n"
              << "  Total Sent: " << totalSentPackets << " packets (" << totalSentBytes
              << " bytes)\n"
              << "  Total Received: " << totalReceivedPackets << " packets (" << totalReceivedBytes
              << " bytes)\n"
              << "  Average Reception per Node: " << (double)totalReceivedPackets / nodes.GetN()
              << " packets\n";
}

void
IpTrace(const Ipv4Header& ipHeader, Ptr<const Packet> packet, uint32_t interface)
{
    std::ostringstream oss;
    packet->Print(oss);

    // NS_LOG_INFO("IP Trace: " << Simulator::Now().GetSeconds() << "s\t"
    //                          << "Interface: " << interface << "\tPacketID: " << packet->GetUid()
    //                          << "\tSize: " << packet->GetSize() << "\tFrom: "
    //                          << ipHeader.GetSource() << "\tTo: " << ipHeader.GetDestination()
    //                          << "\tProtocol: " << (uint32_t)ipHeader.GetProtocol() << "\n"
    //                          << oss.str());
}

void
printNodePositions(NodeContainer nodes)
{
    double now = Simulator::Now().GetSeconds();

    NS_LOG_INFO("=== Node positions at " << now << " seconds ===");

    for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
        Vector pos = mobility->GetPosition();
        NS_LOG_INFO("Node " << node->GetId() << " is at (" << pos.x << ", " << pos.y << ", "
                            << pos.z << ")");
    }

    Simulator::Schedule(Seconds(0.1), &printNodePositions, nodes);
}

int
main(int argc, char* argv[])
{
    // BACK END
    //  CommandLine cmd;
    //  uint32_t nNodes = 10;                         // 默认值：10个节点
    //  double simulationTime = 10.0;                 // 默认值：10秒
    //  double txPower = 20.0;                        // 默认值：20 dBm
    //  double frequency = 5.0;                       // 默认值：5 GHz
    //  uint32_t channelWidth = 80;                   // 默认值：80 MHz
    //  std::string propagationModel = "LogDistance"; // 默认传播模型

    // // 注册命令行参数
    // cmd.AddValue("nNodes", "Number of nodes", nNodes);
    // cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
    // cmd.AddValue("txPower", "Transmission power in dBm", txPower);
    // cmd.AddValue("frequency", "Frequency band in GHz", frequency);
    // cmd.AddValue("channelWidth", "Channel width in MHz", channelWidth);
    // cmd.AddValue("propagationModel", "Propagation loss model", propagationModel);
    // cmd.Parse(argc, argv);

    LogComponentEnable("FLSSimulation", LOG_LEVEL_INFO);
    LogComponentEnable("FLSApplication", LOG_LEVEL_INFO);
    LogComponentEnable("TraceBasedMobilityModel", LOG_LEVEL_INFO);

    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                       StringValue("2200"));
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
    Config::SetDefault("ns3::WifiMacQueue::MaxSize", QueueSizeValue(QueueSize("100p")));

    std::string traceDir = "scratch/FLS/traces/";

    uint32_t nNodes = 454;
    NodeContainer nodes;
    nodes.Create(nNodes);

    // Testing if nodes creation successful
    NS_LOG_INFO("Created " << nNodes << " nodes.");

    // Set mobilityModel
    MobilityHelper mobility;

    // Set initile position
    mobility.SetMobilityModel("ns3::TraceBasedMobilityModel");
    NS_LOG_INFO("Mobility model set.");

    mobility.Install(nodes);
    NS_LOG_INFO("Mobility installed on nodes.");

    for (uint32_t i = 0; i < nNodes; ++i)
    {
        Ptr<TraceBasedMobilityModel> model = nodes.Get(i)->GetObject<TraceBasedMobilityModel>();
        // std::string filename = traceDir + "trace_node_" + std::to_string(i) + ".txt";
        std::string filename = traceDir + "trace_node_" + std::to_string(i);
        NS_LOG_INFO("Loading trace for node " << i << " from file: " << filename);
        model->LoadTrace(filename);
    }

    // NS_LOG_INFO("Initial positions:");
    Simulator::Schedule(Seconds(0.0), &printNodePositions, nodes);
    // printNodePositions(nodes);

    WifiHelper wifi;
    // Using WIFI 6
    // wifi.SetStandard(WIFI_STANDARD_80211ax);
    wifi.SetStandard(WIFI_STANDARD_80211b);

    // Using Adhoc network
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel;

    // wifiPhy.Set("TxPowerStart", DoubleValue(23.0)); // 较高发射功率 (dBm)
    wifiPhy.Set("TxPowerStart", DoubleValue(23.0)); // 较高发射功率 (dBm)
    wifiPhy.Set("TxPowerEnd", DoubleValue(23.0));
    wifiPhy.Set("RxSensitivity", DoubleValue(-82)); // 较高的接收灵敏度

    // wifiPhy.Set("TxPowerStart", DoubleValue(10.0)); // 降低发射功率
    // wifiPhy.Set("TxPowerEnd", DoubleValue(10.0));
    // wifiPhy.Set("RxSensitivity", DoubleValue(-70));
    // wifiPhy.Set("RxNoiseFigure", DoubleValue(10.0));

    // 替换原有的传播模型配置
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    // wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
    //                                "Exponent",
    //                                DoubleValue(4.0),
    //                                "ReferenceDistance",
    //                                DoubleValue(1.0),
    //                                "ReferenceLoss",
    //                                DoubleValue(50.0));
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                   "MaxRange",
                                   DoubleValue(1000.0));
    wifiPhy.SetChannel(wifiChannel.Create());
    // wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");

    // PHY Setting
    wifiPhy.Set("RxNoiseFigure", DoubleValue(7.0));

    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    // Install protocol stack
    InternetStackHelper internet;
    // AodvHelper aodv;
    // internet.SetRoutingHelper(aodv);

    internet.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.254.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Ptr<OutputStreamWrapper> routingStream =
        Create<OutputStreamWrapper>("routes.txt", std::ios::out);
    Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(Seconds(0.0), routingStream); // 初始状态
    Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(Seconds(1.5), routingStream); // 应用启动后
    Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(Seconds(30.0), routingStream); // 运行一段时间后

    NS_LOG_INFO("Wi-Fi and Internet stack installed.");

    // print each node's IP
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<Ipv4> ipv4 = nodes.Get(i)->GetObject<Ipv4>();
        if (ipv4)
        {
            ipv4->TraceConnectWithoutContext("SendOutgoing", MakeCallback(&IpTrace));
            ipv4->TraceConnectWithoutContext("LocalDeliver", MakeCallback(&IpTrace));
        }
    }

    // int port = 9;
    ApplicationContainer flsApps;
    for (uint32_t i = 0; i < nNodes; ++i)
    {
        Ptr<FLSApplication> app = CreateObject<FLSApplication>();
        nodes.Get(i)->AddApplication(app);
        app->SetStartTime(Seconds(1.0));
        app->SetStopTime(Seconds(30.0));
        // loading packet trace file
        // std::string traceFilename = traceDir + "packet_trace_node_" + std::to_string(i) + ".txt";
        std::string traceFilename = traceDir + "packet_trace_node_" + std::to_string(i);
        app->SetupTraceFile(traceFilename);
        flsApps.Add(app);
    }

    NS_LOG_INFO("FLS application installed");

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    StatisticsManager statistics;
    statistics.Setup(monitor, classifier);

    NS_LOG_INFO("Simulation started");
    Simulator::Stop(Seconds(30.0));

    AnimationInterface anim("fls-animation.xml");
    anim.EnablePacketMetadata(true);

    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        anim.UpdateNodeColor(nodes.Get(i), 255, 0, 0);
        anim.UpdateNodeSize(nodes.Get(i)->GetId(), 5, 5);
    }

    anim.SetMobilityPollInterval(Seconds(0.1));

    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<Ipv4> ipv4 = nodes.Get(i)->GetObject<Ipv4>();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
        NS_LOG_INFO("Node " << i << " has IP address: " << iaddr.GetLocal());
    }
    Simulator::Run();

    PrintTrafficStatistics(nodes);

    Json::Value results;
    Json::Value flowStats(Json::arrayValue);

    monitor->CheckForLostPackets();
    statistics.CollectStatistics();
    statistics.PrintStats();

    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    // 打印统计信息
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin();
         i != stats.end();
         ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

        // Json::Value flow;
        // flow["flowId"] = i->first;
        // flow["sourceAddress"] = t.sourceAddress.Get();
        // flow["destinationAddress"] = t.destinationAddress.Get();
        // flow["txPackets"] = i->second.txPackets;     // 发送的数据包数
        // flow["rxPackets"] = i->second.rxPackets;     // 接收的数据包数
        // flow["lostPackets"] = i->second.lostPackets; // 丢失的数据包数

        // // 计算吞吐量（Mbps）
        // flow["throughput"] =
        //     i->second.rxBytes * 8.0 /
        //     (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())
        //     / 1000000;

        // // 计算平均延迟（毫秒）
        // flow["delay"] = i->second.delaySum.GetSeconds() / i->second.rxPackets * 1000;

        // // 将流信息添加到数组
        // flowStats.append(flow);

        std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> "
                  << t.destinationAddress << ")\n";
        std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
        std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
        std::cout << "  Lost Packets: " << i->second.lostPackets << "\n";
        std::cout << "  Packet Loss Ratio: "
                  << ((double)i->second.lostPackets / (double)i->second.txPackets) * 100 << "%\n";
        std::cout << "  Throughput: "
                  << i->second.rxBytes * 8.0 /
                         (i->second.timeLastRxPacket.GetSeconds() -
                          i->second.timeFirstTxPacket.GetSeconds()) /
                         1000000
                  << " Mbps\n";
        std::cout << "  Mean Delay: "
                  << i->second.delaySum.GetSeconds() / i->second.rxPackets * 1000 << " ms\n";
        std::cout << "  Mean Jitter: "
                  << i->second.jitterSum.GetSeconds() / (i->second.rxPackets - 1) * 1000 << "ms\n";
        std::cout << "\n";
    }

    results["flowStats"] = flowStats;

    std::ofstream resultFile("simulation-results.json");
    resultFile << results;
    resultFile.close();

    Simulator::Destroy();
    NS_LOG_INFO("Simulation completed successfully");

    return 0;
}
