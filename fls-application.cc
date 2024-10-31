#include "fls-application.h"

#include "ns3/log.h"
#include "ns3/mobility-model.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("FLSApplication");

NS_OBJECT_ENSURE_REGISTERED(FLSApplication);

TypeId
FLSApplication::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::FLSApplication")
                            .SetParent<Application>()
                            .SetGroupName("FLS")
                            .AddConstructor<FLSApplication>()
                            .AddAttribute("Interval",
                                          "Packet send interval",
                                          TimeValue(Seconds(1.0)),
                                          MakeTimeAccessor(&FLSApplication::m_interval),
                                          MakeTimeChecker())
                            .AddAttribute("PacketSize",
                                          "Size of packets sent",
                                          UintegerValue(1024),
                                          MakeUintegerAccessor(&FLSApplication::m_packetSize),
                                          MakeUintegerChecker<uint32_t>(1));
    return tid;
}

FLSApplication::FLSApplication()
    : m_socket(0),
      m_packetsSent(0),
      m_packetsReceived(0)
{
}

FLSApplication::~FLSApplication()
{
    m_socket = 0;
}

void
FLSApplication::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    if (m_socket == nullptr)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 9);
        m_socket->Bind(local);
        m_socket->SetRecvCallback(MakeCallback(&FLSApplication::ReceivePacket, this));
    }
    if (!m_packetTraces.empty())
    {
        // ScheduleNextPacket();
        Simulator::Schedule(Seconds(1.0), &FLSApplication::ScheduleNextPacket, this);
    }
    else
    {
        m_sendEvent = Simulator::Schedule(Seconds(0.0), &FLSApplication::SendPacket, this);
    }
}

void
FLSApplication::StopApplication(void)
{
    if (m_socket != nullptr)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
FLSApplication::SendPacket(void)
{
    Ptr<MobilityModel> mobility = GetNode()->GetObject<MobilityModel>();
    // Vector myPos = mobility->GetPosition();

    if (!m_packetTraces.empty())
    {
        const PacketTrace& trace = m_packetTraces[m_currentTraceIndex];
        Ptr<Packet> packet = Create<Packet>(trace.size);

        Ipv4Address destAddr(trace.destination.c_str());
        bool isBroadCast = (trace.destination == "255.255.255.255");

        if (!m_socket)
        {
            NS_LOG_ERROR("Socket is null");
            return;
        }

        if (isBroadCast)
        {
            m_socket->SetAllowBroadcast(true);
            InetSocketAddress broadcast = InetSocketAddress(Ipv4Address::GetBroadcast(), 9);
            int ret = m_socket->SendTo(packet, 0, broadcast);
            if (ret == -1)
            {
                NS_LOG_ERROR("Error broadcasting packet: " << m_socket->GetErrno());
            }
            else
            {
                NS_LOG_INFO("APP LAYER: " << Simulator::Now().GetSeconds() << "s\t"
                                          << "Node " << GetNode()->GetId() << "\tBroadcast\t"
                                          << trace.size << " bytes"
                                          << "\tPacketID: " << packet->GetUid());
                m_packetsSent++;
            }
        }
        else
        {
            InetSocketAddress remote = InetSocketAddress(destAddr, 9);
            int ret = m_socket->SendTo(packet, 0, remote);
            if (ret == -1)
            {
                NS_LOG_ERROR("Error broadcasting packet: " << m_socket->GetErrno());
            }
            else
            {
                NS_LOG_INFO("APP LAYER: " << Simulator::Now().GetSeconds() << "s\t"
                                          << "Node " << GetNode()->GetId() << "\tUniCast\t"
                                          << trace.size << " bytes"
                                          << "\tPacketID: " << packet->GetUid());
                m_packetsSent++;
            }
        }

        NS_LOG_INFO("Socket state after send: " << m_socket->GetErrno());

        switch (m_socket->GetErrno())
        {
        case Socket::ERROR_NOTERROR:
            NS_LOG_ERROR("No error");
            break;
        case Socket::ERROR_ISCONN:
            NS_LOG_ERROR("Socket is connected");
            break;
        case Socket::ERROR_NOTCONN:
            NS_LOG_ERROR("Socket is not connected");
            break;
        case Socket::ERROR_MSGSIZE:
            NS_LOG_ERROR("Message too long");
            break;
        case Socket::ERROR_INVAL:
            NS_LOG_ERROR("Invalid argument");
            break;
        default:
            NS_LOG_ERROR("Unknown error");
            break;
        }

        m_currentTraceIndex++;
        ScheduleNextPacket();
    }

    else // if we don't have packet trace file, send packets as usual
    {
        NS_LOG_INFO("All packets from trace file have been sent or trace file is empty.");

        //     NodeContainer allNodes = NodeContainer::GetGlobal ();

        //   for (NodeContainer::Iterator i = allNodes.Begin (); i != allNodes.End (); ++i)
        //   {
        //     Ptr<Node> otherNode = *i;
        //     if (otherNode->GetId() == GetNode()->GetId())
        //       continue;

        //     Ptr<MobilityModel> otherMobility = otherNode->GetObject<MobilityModel>();
        //     Vector otherPos = otherMobility->GetPosition();

        //     if (CalculateDistance(myPos, otherPos) <= 10.0)
        //     {
        //       Ptr<Packet> packet = Create<Packet> (m_packetSize);
        //       m_socket->SendTo (packet, 0, InetSocketAddress
        //       (otherNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 9)); m_packetsSent++;
        //     }
        //   }

        //   Simulator::Schedule (m_interval, &FLSApplication::SendPacket, this);
    }
}

void
FLSApplication::ReceivePacket(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        m_packetsReceived++;

        NS_LOG_INFO("Node " << GetNode()->GetId() << " received packet from "
                            << InetSocketAddress::ConvertFrom(from).GetIpv4() << " at time "
                            << Simulator::Now().GetSeconds() << "s");
    }
}

double
FLSApplication::CalculateDistance(Vector a, Vector b)
{
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double dz = b.z - a.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

uint32_t
FLSApplication::GetPacketsSent(void) const
{
    return m_packetsSent;
}

uint32_t
FLSApplication::GetPacketsReceived(void) const
{
    return m_packetsReceived;
}

void
FLSApplication::SetupTraceFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        NS_LOG_ERROR("Unable to open packet trace file" << filename);
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        PacketTrace trace;
        if (!(iss >> trace.timestamp >> trace.size >> trace.destination))
        {
            NS_LOG_ERROR("Error parsing trace file line:" << line);
            continue;
        }
        m_packetTraces.push_back(trace);
        NS_LOG_INFO("Loaded trace: " << trace.timestamp << "s, " << trace.size
                                     << " bytes, To: " << trace.destination);
    }
    NS_LOG_INFO("Loaded " << m_packetTraces.size() << " packet traces from " << filename);
    m_currentTraceIndex = 0;
}

void
FLSApplication::ScheduleNextPacket()
{
    if (m_currentTraceIndex >= m_packetTraces.size())
    {
        NS_LOG_ERROR("All packets from this trace file have been sent.");
        return;
    }
    Time tNext;
    if (m_currentTraceIndex == 0)
    {
        tNext = Seconds(m_packetTraces[m_currentTraceIndex].timestamp);
    }
    else
    {
        double timeDiff = m_packetTraces[m_currentTraceIndex].timestamp -
                          m_packetTraces[m_currentTraceIndex - 1].timestamp;
        tNext = Seconds(timeDiff);
    }
    m_sendEvent = Simulator::Schedule(tNext, &FLSApplication::SendPacket, this);
}
} // namespace ns3