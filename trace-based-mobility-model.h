#ifndef TRACE_BASED_MOBILITY_MODEL_H
#define TRACE_BASED_MOBILITY_MODEL_H

#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include <vector>
#include <map>

namespace ns3 {
class TraceBasedMobilityModel: public MobilityModel
{
    public:
        static TypeId GetTypeId(void);
        TraceBasedMobilityModel();

        void LoadTrace(std::string filename);
        virtual Vector DoGetPosition(void) const;
        virtual void DoSetPosition(const Vector &position);
        virtual Vector DoGetVelocity(void) const;
    
    private:
        void UpdatePosition(void);
        std::map<double, Vector>m_trace;
        Vector m_position;
        EventId m_event;
};
}

#endif