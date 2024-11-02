#include "mobility-controller.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

#include <fstream>
#include <sstream>

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(TraceBasedMobilityModel);
NS_LOG_COMPONENT_DEFINE("TraceBasedMobilityModel");

TypeId
TraceBasedMobilityModel::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::TraceBasedMobilityModel")
            .SetParent<MobilityModel>()
            .SetGroupName("Mobility")
            .AddConstructor<TraceBasedMobilityModel>()
            .AddAttribute("InterpolationInterval",
                          "Time interval for interpolation in seconds",
                          DoubleValue(0.01), // 10ms默认值
                          MakeDoubleAccessor(&TraceBasedMobilityModel::m_interpolationInterval),
                          MakeDoubleChecker<double>(0.001, 1.0)); // 1ms到1s的范围

    return tid;
}

TraceBasedMobilityModel::TraceBasedMobilityModel()
    : m_interpolationInterval(0.01)
{
}

void
TraceBasedMobilityModel::LoadTrace(std::string filename)
{
    m_trace.clear();
    m_interpolatedTrace.clear();

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

    Interpolate();

    // m_position = m_interpolatedTrace.begin()->second;
    m_position = m_trace.begin()->second;
    m_event = Simulator::Schedule(Seconds(0.0), &TraceBasedMobilityModel::UpdatePosition, this);
}

void
TraceBasedMobilityModel::Interpolate()
{
    if (m_trace.size() < 2)
        return;

    auto it = m_trace.begin();
    auto next = std::next(it);

    while (next != m_trace.end())
    {
        double t1 = it->first;
        double t2 = next->first;
        Vector pos1 = it->second;
        Vector pos2 = next->second;

        // 在两个原始点之间插值
        for (double t = t1; t < t2; t += m_interpolationInterval)
        {
            double alpha = (t - t1) / (t2 - t1); // 插值因子
            Vector interpolatedPos;

            // 线性插值
            interpolatedPos.x = pos1.x + alpha * (pos2.x - pos1.x);
            interpolatedPos.y = pos1.y + alpha * (pos2.y - pos1.y);
            interpolatedPos.z = pos1.z + alpha * (pos2.z - pos1.z);

            m_interpolatedTrace[t] = interpolatedPos;
        }

        ++it;
        ++next;
    }

    // 添加最后一个点
    m_interpolatedTrace[m_trace.rbegin()->first] = m_trace.rbegin()->second;

    NS_LOG_INFO("Interpolation completed: Original points: "
                << m_trace.size() << ", Interpolated points: " << m_interpolatedTrace.size());
}

Vector
TraceBasedMobilityModel::DoGetPosition(void) const
{
    return m_position;
}

void
TraceBasedMobilityModel::DoSetPosition(const Vector& position)
{
    m_position = position;
}

Vector
TraceBasedMobilityModel::DoGetVelocity(void) const
{
    // For simplicity, we're not calculating velocity here
    return Vector(0, 0, 0);
}

void
TraceBasedMobilityModel::UpdatePosition(void)
{
    double now = Simulator::Now().GetSeconds();
    auto it = m_interpolatedTrace.lower_bound(now);

    if (it != m_interpolatedTrace.end())
    {
        m_position = it->second;
        NotifyCourseChange();
    }

    if (std::next(it) != m_interpolatedTrace.end())
    {
        m_event = Simulator::Schedule(Seconds(std::next(it)->first - now),
                                      &TraceBasedMobilityModel::UpdatePosition,
                                      this);
    }
}

} // namespace ns3