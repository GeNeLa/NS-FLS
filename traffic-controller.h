#ifndef FLS_APPLICATION_H
#define FLS_APPLICATION_H

#include "packet-trace.h"

#include "ns3/application.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/socket.h"
#include "ns3/vector.h"

#include <vector>

namespace ns3
{

class FLSApplication : public Application
{
  public:
    static TypeId GetTypeId(void);
    FLSApplication();
    virtual ~FLSApplication();

    uint32_t GetPacketsSent(void) const;
    uint32_t GetPacketsReceived(void) const;
    void SetupTraceFile(const std::string& filename);

    struct TrafficStats
    {
        // 发送统计
        uint32_t sentPackets{0};
        uint64_t sentBytes{0};
        Time firstSentTime{Seconds(0)};
        Time lastSentTime{Seconds(0)};

        // 接收统计
        uint32_t receivedPackets{0};
        uint64_t receivedBytes{0};
        Time firstReceivedTime{Seconds(0)};
        Time lastReceivedTime{Seconds(0)};

        // 清除统计
        void Clear()
        {
            sentPackets = 0;
            sentBytes = 0;
            receivedPackets = 0;
            receivedBytes = 0;
            firstSentTime = Seconds(0);
            lastSentTime = Seconds(0);
            firstReceivedTime = Seconds(0);
            lastReceivedTime = Seconds(0);
        }
    };

    const TrafficStats& GetStats() const
    {
        return m_stats;
    }

  private:
    TrafficStats m_stats;
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void SendPacket();
    void ScheduleNextPacket();
    void ReceivePacket(Ptr<Socket> socket);
    double CalculateDistance(Vector a, Vector b);

    Ptr<Socket> m_socket;
    Time m_interval;
    uint32_t m_packetSize;
    uint32_t m_packetsSent;
    uint32_t m_packetsReceived;
    std::vector<PacketTrace> m_packetTraces;
    uint32_t m_currentTraceIndex;
    EventId m_sendEvent;
};

} // namespace ns3

#endif