#ifndef PACKET_TRACE_H
#define PACKET_TRACE_H

#include <cstdint>
#include <string>

struct PacketTrace
{
    /* data */
    double timestamp;
    uint32_t size;
    std::string destination;
};

#endif
