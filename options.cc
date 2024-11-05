#include "options.h"

#include "ns3/log.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("SimulationOptions");

SimulationOptions::SimulationOptions()
    : nNodes(454),
      simulationTime(30.0),
      txPower(23.0),
      rxSensitivity(-82.0),
      rxNoiseFigure(7.0),
      wifiStandard("80211b")
{
}

bool
SimulationOptions::Parse(int argc, char* argv[])
{
    CommandLine cmd;

    cmd.AddValue("nNodes", "Number of nodes", nNodes);
    cmd.AddValue("simTime", "Simulation time in seconds", simulationTime);
    cmd.AddValue("txPower", "Transmission power in dBm", txPower);
    cmd.AddValue("rxSensitivity", "Receiver sensitivity in dBm", rxSensitivity);
    cmd.AddValue("noiseFigure", "Receiver noise figure", rxNoiseFigure);
    cmd.AddValue("wifi", "WiFi standard (80211b/80211ax/80211n/80211ac/80211g)", wifiStandard);

    cmd.Parse(argc, argv);

    NS_LOG_INFO("Simulation Configuration:");
    NS_LOG_INFO("  Number of nodes: " << nNodes);
    NS_LOG_INFO("  Simulation time: " << simulationTime << " seconds");
    NS_LOG_INFO("  Tx Power: " << txPower << " dBm");
    NS_LOG_INFO("  Rx Sensitivity: " << rxSensitivity << " dBm");
    NS_LOG_INFO("  Noise Figure: " << rxNoiseFigure);
    NS_LOG_INFO("  WiFi Standard: " << wifiStandard);

    return true;
}
} // namespace ns3