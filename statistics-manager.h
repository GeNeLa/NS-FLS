#ifndef STATISTICS_MANAGER_H
#define STATISTICS_MANAGER_H

#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include <json/json.h>
#include <map>

namespace ns3
{

// 节点统计信息结构体
struct NodeStats
{
    // 基本传输统计
    uint64_t txBytes = 0;
    uint64_t rxBytes = 0;
    uint32_t txPackets = 0;
    uint32_t rxPackets = 0;
    uint32_t lostPackets = 0;

    // 延迟统计
    double totalDelay = 0.0;
    double meanDelay = 0.0;
    double meanJitter = 0.0;

    // 吞吐量统计
    double throughput = 0.0;
    double packetLossRate = 0.0;
};

class StatisticsManager
{
  public:
    StatisticsManager();
    ~StatisticsManager();

    // 初始化方法
    void Setup(Ptr<FlowMonitor> monitor, Ptr<Ipv4FlowClassifier> classifier);

    // 收集统计信息
    void CollectStatistics();

    // 打印统计信息
    void PrintStats();

    // 生成JSON报告
    Json::Value GenerateJsonReport();

  private:
    // 将流统计聚合到节点统计
    void AggregateFlowStats();

    // 计算每个节点的统计信息
    void CalculateNodeStats();

    Ptr<FlowMonitor> m_flowMonitor;
    Ptr<Ipv4FlowClassifier> m_classifier;
    std::map<uint32_t, NodeStats> m_nodeStats; // 节点ID -> 统计信息
};

} // namespace ns3

#endif // STATISTICS_MANAGER_H