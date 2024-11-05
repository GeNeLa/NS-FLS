#ifndef SIMULATION_OPTIONS_H
#define SIMULATION_OPTIONS_H

#include "ns3/command-line.h"
#include "ns3/core-module.h"

#include <string>

namespace ns3
{

class SimulationOptions
{
  public:
    SimulationOptions();

    bool Parse(int argc, char* argv[]);

    uint32_t GetNumberOfNodes() const
    {
        return nNodes;
    }

    double GetSimulationTime() const
    {
        return simulationTime;
    }

    double GetTxPower() const
    {
        return txPower;
    }

    double GetRxSensitivity() const
    {
        return rxSensitivity;
    }

    double GetNoiseFigure() const
    {
        return rxNoiseFigure;
    }

    std::string GetWifiStandard() const
    {
        return wifiStandard;
    }

  private:
    uint32_t nNodes;          // Number of nodes
    double simulationTime;    // Simulation duration
    double txPower;           // Transmission power (dBm)
    double rxSensitivity;     // Receiver sensitivity (dBm)
    double rxNoiseFigure;     // Receiver noise figure
    std::string wifiStandard; // WiFi standard
};

} // namespace ns3

#endif