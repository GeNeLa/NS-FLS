#ifndef FLS_APPLICATION_H
#define FLS_APPLICATION_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/vector.h"
#include "packet-trace.h"
#include <vector>


namespace ns3{

class FLSApplication : public Application
{
public:
    static TypeId GetTypeId(void);
    FLSApplication();
    virtual ~FLSApplication();

    uint32_t GetPacketsSent(void) const;
    uint32_t GetPacketsReceived(void) const;
    void SetupTraceFile(const std::string & filename);

private:
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

}

#endif