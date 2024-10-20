#include "trace-based-mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/log.h"       
#include "ns3/string.h"    
#include <fstream>
#include <sstream>

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (TraceBasedMobilityModel);
NS_LOG_COMPONENT_DEFINE ("TraceBasedMobilityModel"); 

TypeId
TraceBasedMobilityModel::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::TraceBasedMobilityModel")
        .SetParent<MobilityModel>()
        .SetGroupName("Mobility")
        .AddConstructor<TraceBasedMobilityModel>();
    
    return tid;
}

TraceBasedMobilityModel::TraceBasedMobilityModel()
{
}

void
TraceBasedMobilityModel::LoadTrace(std::string filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        NS_LOG_ERROR("Unable to open trace file" << filename);
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        double time;
        Vector position;
        if (!(iss >> time >> position.x >> position.y >> position.z))
        {
            NS_LOG_ERROR("Invalid line in trace file" << line);
            continue;
        }
        NS_LOG_INFO("Loading trace from file: " << filename);
        m_trace[time] = position;
    }

    if (m_trace.empty())
    {
        NS_LOG_ERROR("No valid entries found in trace file" << filename);
        return;
    }

    m_event = Simulator::Schedule(Seconds(0.0), &TraceBasedMobilityModel::UpdatePosition, this);

}


Vector
TraceBasedMobilityModel::DoGetPosition(void) const
{
    return m_position;
}

void
TraceBasedMobilityModel::DoSetPosition(const Vector &position)
{
    m_position = position;
}

Vector
TraceBasedMobilityModel::DoGetVelocity (void) const
{
  // For simplicity, we're not calculating velocity here
  return Vector(0, 0, 0);
}

void
TraceBasedMobilityModel::UpdatePosition(void)
{
    double now = Simulator::Now().GetSeconds();
    auto it = m_trace.lower_bound(now);
    if (it != m_trace.end())
    {
        m_position = it->second;
        NotifyCourseChange();
    }

    if (std::next(it) != m_trace.end())
    {
        m_event = Simulator::Schedule(Seconds(std::next(it)->first - now),
                                    &TraceBasedMobilityModel::UpdatePosition, this);
    }
}

}