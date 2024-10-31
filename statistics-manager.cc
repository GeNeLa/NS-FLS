#include "statistics-manager.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("StatisticsManager");

StatisticsManager::StatisticsManager()
    : m_flowMonitor(0),
      m_classifier(0)
{
}

StatisticsManager::~StatisticsManager()
{
}

void
StatisticsManager::Setup(Ptr<FlowMonitor> monitor, Ptr<Ipv4FlowClassifier> classifier)
{
    m_flowMonitor = monitor;
    m_classifier = classifier;
}

void
StatisticsManager::CollectStatistics()
{
    if (!m_flowMonitor || !m_classifier)
    {
        std::cout << "FlowMonitor or classifier not initialized!" << std::endl;
        return;
    }

    m_flowMonitor->CheckForLostPackets();
    AggregateFlowStats();
    CalculateNodeStats();
}

void
StatisticsManager::AggregateFlowStats()
{
    std::map<FlowId, FlowMonitor::FlowStats> stats = m_flowMonitor->GetFlowStats();

    // 清理旧的统计数据
    m_nodeStats.clear();

    // 遍历所有流
    for (const auto& flow : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = m_classifier->FindFlow(flow.first);

        // 获取源节点和目标节点的ID
        uint32_t sourceNode = (t.sourceAddress.Get() & 0xFF) - 1;
        uint32_t destNode = (t.destinationAddress.Get() & 0xFF) - 1;

        // 更新源节点统计
        m_nodeStats[sourceNode].txBytes += flow.second.txBytes;
        m_nodeStats[sourceNode].txPackets += flow.second.txPackets;
        m_nodeStats[sourceNode].lostPackets += flow.second.lostPackets;

        // 更新目标节点统计
        m_nodeStats[destNode].rxBytes += flow.second.rxBytes;
        m_nodeStats[destNode].rxPackets += flow.second.rxPackets;

        // 更新延迟统计
        if (flow.second.rxPackets > 0)
        {
            double delay = flow.second.delaySum.GetSeconds() / flow.second.rxPackets;
            m_nodeStats[destNode].totalDelay += delay;
            m_nodeStats[destNode].meanDelay = delay;
            m_nodeStats[destNode].meanJitter =
                flow.second.jitterSum.GetSeconds() / (flow.second.rxPackets - 1);
        }
    }
}

void
StatisticsManager::CalculateNodeStats()
{
    double simulationTime = Simulator::Now().GetSeconds();

    for (auto& nodeStat : m_nodeStats)
    {
        NodeStats& stats = nodeStat.second;

        // 计算吞吐量 (Mbps)
        stats.throughput = (stats.rxBytes * 8.0) / simulationTime / 1000000;

        // 计算丢包率
        if (stats.txPackets > 0)
        {
            stats.packetLossRate = static_cast<double>(stats.lostPackets) / stats.txPackets * 100.0;
        }
    }
}

void
StatisticsManager::PrintStats()
{
    std::cout << "\n=== Node Statistics ===\n";

    for (const auto& nodeStat : m_nodeStats)
    {
        std::cout << "\nNode " << nodeStat.first << ":\n"
                  << "  Transmitted Packets: " << nodeStat.second.txPackets << "\n"
                  << "  Received Packets: " << nodeStat.second.rxPackets << "\n"
                  << "  Lost Packets: " << nodeStat.second.lostPackets << "\n"
                  << "  Packet Loss Rate: " << nodeStat.second.packetLossRate << "%\n"
                  << "  Mean Delay: " << nodeStat.second.meanDelay * 1000 << " ms\n"
                  << "  Mean Jitter: " << nodeStat.second.meanJitter * 1000 << " ms\n"
                  << "  Throughput: " << nodeStat.second.throughput << " Mbps\n";
    }
}

Json::Value
StatisticsManager::GenerateJsonReport()
{
    Json::Value report;
    Json::Value nodeStats(Json::arrayValue);

    for (const auto& nodeStat : m_nodeStats)
    {
        Json::Value node;
        node["nodeId"] = nodeStat.first;
        node["txPackets"] = nodeStat.second.txPackets;
        node["rxPackets"] = nodeStat.second.rxPackets;
        node["lostPackets"] = nodeStat.second.lostPackets;
        node["packetLossRate"] = nodeStat.second.packetLossRate;
        node["meanDelay"] = nodeStat.second.meanDelay * 1000;   // 转换为毫秒
        node["meanJitter"] = nodeStat.second.meanJitter * 1000; // 转换为毫秒
        node["throughput"] = nodeStat.second.throughput;

        nodeStats.append(node);
    }

    report["nodeStats"] = nodeStats;
    return report;
}

} // namespace ns3