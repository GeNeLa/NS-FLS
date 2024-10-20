#include "fls-application.h"
#include "packet-trace.h"
#include "trace-based-mobility-model.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/mac48-address.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/packet.h"
#include "ns3/wifi-module.h"

#include <map>
#include <vector>
// #include <unistd.h>
// #include <limits.h>

#include "ns3/arp-cache.h"
#include "ns3/ipv4-l3-protocol.h"

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
PrintStatistics()
{
    for (const auto& nodePair : nodeSentPackets)
    {
        uint32_t nodeId = nodePair.first;
        const auto& packets = nodePair.second;

        uint32_t totalSent = packets.size();
        uint32_t totalReceived = 0;
        double totalDelay = 0.0;
        uint32_t totalBytes = 0;

        NS_LOG_INFO("Node " << nodeId << " sent " << totalSent << " packets");

        for (const auto& packet : packets)
        {
            if (packet.received)
            {
                totalReceived++;
                totalDelay += (packet.receiveTime - packet.sendTime);
                NS_LOG_INFO("  Packet " << packet.packetId << " was received by Node "
                                        << packet.destinationNode);
            }
            else
            {
                NS_LOG_INFO("  Packet " << packet.packetId << " was not received by Node "
                                        << packet.destinationNode);
            }
            totalBytes += packet.size;
        }

        double packetLossRate =
            totalSent > 0 ? static_cast<double>(totalSent - totalReceived) / totalSent * 100.0
                          : 0.0;
        double averageDelay = totalReceived > 0 ? totalDelay / totalReceived * 1000 : 0; // in ms
        double throughput = totalBytes * 8.0 / Simulator::Now().GetSeconds() / 1000000;  // in Mbps

        NS_LOG_INFO("Node " << nodeId << " statistics:");
        NS_LOG_INFO("  Sent packets: " << totalSent);
        NS_LOG_INFO("  Received packets: " << totalReceived);
        NS_LOG_INFO("  Packet loss rate: " << packetLossRate << "%");
        NS_LOG_INFO("  Average delay: " << averageDelay << " ms");
        NS_LOG_INFO("  Throughput: " << throughput << " Mbps");
        NS_LOG_INFO("");
    }
}

void
PacketSendCallback(std::string context, Ptr<const Packet> packet)
{
    uint32_t nodeId = std::stoul(context.substr(10));
    Ptr<Packet> copy = packet->Copy();

    Ipv4Header ipHeader;
    copy->RemoveHeader(ipHeader);
    uint32_t destinationNode = (ipHeader.GetDestination().Get() & 0xFF) % 50;
    PacketInfo info;
    info.packetId = packet->GetUid();
    info.sendTime = Simulator::Now().GetSeconds();
    info.size = packet->GetSize();
    info.destinationNode = destinationNode;
    info.received = false;

    nodeSentPackets[nodeId].push_back(info);

    // NS_LOG_INFO("Node " << nodeId << " sent packet " << info.packetId << " to Node "
    //                     << info.destinationNode << " at " << info.sendTime << "s");
}

void
PacketReceiveCallback(std::string context, Ptr<const Packet> packet)
{
    uint32_t nodeId = std::stoul(context.substr(10));
    Ptr<Packet> copy = packet->Copy();

    Ipv4Header ipHeader;
    copy->RemoveHeader(ipHeader);

    uint32_t senderNodeId = ipHeader.GetSource().Get() & 0xFF;

    // BUG senderNodeId
    //  NS_LOG_INFO("Node " << nodeId << " received packet " << packet->GetUid() << " from Node "
    //                      << senderNodeId << " at " << Simulator::Now().GetSeconds() << "s");

    // 在发送节点的已发送包列表中查找并标记为已接收
    if (nodeSentPackets.find(senderNodeId) != nodeSentPackets.end())
    {
        auto& senderPackets = nodeSentPackets[senderNodeId];
        for (auto& sentPacket : senderPackets)
        {
            if (sentPacket.packetId == packet->GetUid())
            {
                sentPacket.received = true;
                sentPacket.receiveTime = Simulator::Now().GetSeconds();
                NS_LOG_INFO("Marked packet " << sentPacket.packetId << " as received");
                break;
            }
        }
    }
    else
    {
        // NS_LOG_WARN("No sent packets found for Node " << senderNodeId);
    }
}

void
IpTrace(const Ipv4Header& ipHeader, Ptr<const Packet> packet, uint32_t interface)
{
    std::ostringstream oss;
    packet->Print(oss);

    NS_LOG_INFO("IP Trace: " << Simulator::Now().GetSeconds() << "s\t"
                             << "Interface: " << interface << "\tPacketID: " << packet->GetUid()
                             << "\tSize: " << packet->GetSize() << "\tFrom: "
                             << ipHeader.GetSource() << "\tTo: " << ipHeader.GetDestination()
                             << "\tProtocol: " << (uint32_t)ipHeader.GetProtocol() << "\n"
                             << oss.str());
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

    Simulator::Schedule(Seconds(10.0), &printNodePositions, nodes);
}

int
main(int argc, char* argv[])
{
    LogComponentEnable("FLSSimulation", LOG_LEVEL_INFO);
    LogComponentEnable("FLSApplication", LOG_LEVEL_INFO);
    LogComponentEnable("TraceBasedMobilityModel", LOG_LEVEL_INFO);
    // LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_ALL);
    // LogComponentEnable ("ArpL3Protocol", LOG_LEVEL_DEBUG);
    // LogComponentEnable ("ArpCache", LOG_LEVEL_DEBUG);
    // LogComponentEnable("UdpSocketImpl", LOG_LEVEL_DEBUG);

    // ns3::LogComponentEnable("FLSSimulation", ns3::LOG_PREFIX_TIME);
    // ns3::LogComponentEnable("FLSApplication", ns3::LOG_PREFIX_TIME);
    // ns3::LogComponentEnable("TraceBasedMobilityModel", ns3::LOG_PREFIX_TIME);

    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold",
                       StringValue("2200"));
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
    Config::SetDefault("ns3::WifiMacQueue::MaxSize", QueueSizeValue(QueueSize("100p")));

    std::string traceDir = "scratch/FLS/traces/";

    uint32_t nNodes = 50;
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
        std::string filename = traceDir + "trace_node_" + std::to_string(i) + ".txt";
        NS_LOG_INFO("Loading trace for node " << i << " from file: " << filename);
        model->LoadTrace(filename);
    }

    // NS_LOG_INFO("Initial positions:");
    Simulator::Schedule(Seconds(0.0), &printNodePositions, nodes);
    // printNodePositions(nodes);

    WifiHelper wifi;
    // Using WIFI 6
    wifi.SetStandard(WIFI_STANDARD_80211ax);

    // Using Adhoc network
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                   "MaxRange",
                                   DoubleValue(100.0));
    wifiPhy.SetChannel(wifiChannel.Create());

    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    // Install protocol stack
    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // ADD: Print routing tables and ARP caches
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(&std::cout);
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<Ipv4> ipv4 = nodes.Get(i)->GetObject<Ipv4>();
        Ptr<Ipv4RoutingProtocol> routingProtocol = ipv4->GetRoutingProtocol();
        Ptr<Ipv4ListRouting> listRouting = DynamicCast<Ipv4ListRouting>(routingProtocol);

        if (listRouting)
        {
            for (uint32_t j = 0; j < listRouting->GetNRoutingProtocols(); j++)
            {
                int16_t priority;
                Ptr<Ipv4RoutingProtocol> protocol = listRouting->GetRoutingProtocol(j, priority);
                Ptr<Ipv4StaticRouting> staticRouting = DynamicCast<Ipv4StaticRouting>(protocol);
                if (staticRouting)
                {
                    NS_LOG_INFO("Routing table for node " << i << ":");
                    staticRouting->PrintRoutingTable(routingStream);
                    break;
                }
            }
        }

        // MODIFY: Use GetObject<Ipv4L3Protocol>() to get the L3 protocol
        Ptr<Ipv4L3Protocol> ipv4L3 = ipv4->GetObject<Ipv4L3Protocol>();
        if (ipv4L3)
        {
            for (uint32_t j = 0; j < ipv4L3->GetNInterfaces(); j++)
            {
                Ptr<ArpCache> arpCache = ipv4L3->GetInterface(j)->GetArpCache();
                if (arpCache)
                {
                    NS_LOG_INFO("ARP cache for node " << i << ", interface " << j << ":");
                    arpCache->PrintArpCache(routingStream);
                }
            }
        }
    }

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

    int port = 9;
    ApplicationContainer flsApps;
    for (uint32_t i = 0; i < nNodes; ++i)
    {
        Ptr<FLSApplication> app = CreateObject<FLSApplication>();
        nodes.Get(i)->AddApplication(app);
        app->SetStartTime(Seconds(1.0));
        app->SetStopTime(Seconds(90.0));
        // loading packet trace file
        std::string traceFilename = traceDir + "packet_trace_node_" + std::to_string(i) + ".txt";
        app->SetupTraceFile(traceFilename);
        flsApps.Add(app);
    }

    NS_LOG_INFO("FLS application installed");

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();
    NS_LOG_INFO("Simulation started");
    Simulator::Stop(Seconds(90.0));

    Config::Connect("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
                    MakeCallback(&PacketSendCallback));
    Config::Connect("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",
                    MakeCallback(&PacketReceiveCallback));
    AnimationInterface anim("fls-animation.xml");
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

    NS_LOG_INFO("Simulation statistics:");
    // PrintStatistics();

    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    // 打印统计信息
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin();
         i != stats.end();
         ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

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
                  << i->second.jitterSum.GetSeconds() / (i->second.rxPackets - 1) * 1000 << " ms\n";
        std::cout << "\n";
    }

    Simulator::Destroy();
    NS_LOG_INFO("Simulation completed successfully");

    return 0;
}

/*
Wifi Standard
802.11a
ns-3 设置：wifi.SetStandard(WIFI_STANDARD_80211a);
802.11b
ns-3 设置：wifi.SetStandard(WIFI_STANDARD_80211b);
802.11g
ns-3 设置：wifi.SetStandard(WIFI_STANDARD_80211g);
802.11n
wifi.SetStandard(WIFI_STANDARD_80211n);
802.11ac
wifi.SetStandard(WIFI_STANDARD_80211ac);

PropagationLossModel

wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                               "Exponent", DoubleValue(3.0));
wifiChannel.AddPropagationLoss("ns3::ThreeLogDistancePropagationLossModel");
wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");
wifiChannel.AddPropagationLoss("ns3::HybridBuildingsPropagationLossModel");
wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel",
                               "Rss", DoubleValue(-80.0));
wifiChannel.AddPropagationLoss("ns3::TwoRayGroundPropagationLossModel");

TxPower
wifiPhy.Set("TxPowerStart", DoubleValue(x));  // 最小发射功率
wifiPhy.Set("TxPowerEnd", DoubleValue(x));    // 最大发射功率

ChannelSetting Channel Number - bandwith - Frequency band
wifiPhy.Set("ChannelSettings", StringValue("{36, 80, BAND_5GHZ, 0}"));


Mac Type
wifiMac.SetType("ns3::AdhocWifiMac");
wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));
wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
mesh.SetMacType("ns3::MeshWifiInterfaceMac");


Ptr<BuildingsPropagationLossModel> propagationLossModel =
CreateObject<BuildingsPropagationLossModel>();
propagationLossModel->SetAttribute("ShadowSigmaOutdoor", DoubleValue(7.0));
propagationLossModel->SetAttribute("ShadowSigmaIndoor", DoubleValue(8.0));
channel.AddPropagationLoss(propagationLossModel);


wifi.SetStandard(WIFI_STANDARD_80211ax)


Ptr<SpectrumInterferenceHelper> interferenceHelper = CreateObject<SpectrumInterferenceHelper>();
Ptr<SpectrumValue> interferencePsd = Create<SpectrumValue>(specificSpectrumModel);
interferenceHelper->AddInterference(interferencePsd, Seconds(1.0));


Ptr<FixedRssLossModel> rssLoss = CreateObject<FixedRssLossModel>();
rssLoss->SetRss(-90);
channel.AddPropagationLoss(rssLoss);

Ptr<GridBuildingAllocator> gridBuildingAllocator = CreateObject<GridBuildingAllocator>();
gridBuildingAllocator->SetAttribute("GridWidth", UintegerValue(X));
gridBuildingAllocator->SetAttribute("BufildingHeight", DoubleValue(X));
gridBuildingAllocator->Create(X);


*/
