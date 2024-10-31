import React, { useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "./ui/card";
import { Button } from "./ui/button";
import { Slider } from "./ui/slider";
import { Label } from "./ui/label";
import { Alert, AlertDescription } from "./ui/alert";
import { RadioGroup, RadioGroupItem } from "./ui/radio-group";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "./ui/tabs";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
} from "recharts";
import { Play, Pause } from "lucide-react";

const SimulationDashboard = () => {
  // 状态管理
  const [phyType, setPhyType] = useState("wifi");
  const [nodeCount, setNodeCount] = useState(10);
  const [simulationTime, setSimulationTime] = useState(10);
  const [txPower, setTxPower] = useState(20);
  const [frequency, setFrequency] = useState(5);
  const [channelWidth, setChannelWidth] = useState(80);
  const [propagationModel, setPropagationModel] = useState("LogDistance");
  const [isRunning, setIsRunning] = useState(false);
  const [results, setResults] = useState(null);

  const phyTypes = [
    { id: "wifi", name: "Wi-Fi (802.11ax)" },
    { id: "ble", name: "Bluetooth LE", disabled: true },
    { id: "zigbee", name: "ZigBee", disabled: true },
  ];

  const propagationModels = [
    "LogDistance",
    "Friis",
    "ThreeLogDistance",
    "RangePropagation",
    "Nakagami",
  ];

  // 处理仿真运行
  const handleRunSimulation = async () => {
    setIsRunning(true);
    try {
      const response = await fetch("http://localhost:5000/run-simulation", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          phyType,
          nodeCount,
          simulationTime,
          txPower,
          frequency,
          channelWidth,
          propagationModel,
        }),
      });

      const data = await response.json();
      if (data.status === "success") {
        setResults(data.results);
      } else {
        console.error("Simulation failed:", data.message);
      }
    } catch (error) {
      console.error("Error running simulation:", error);
    } finally {
      setIsRunning(false);
    }
  };

  return (
    <div className="max-w-6xl mx-auto">
      <Tabs defaultValue="config" className="w-full">
        <TabsList className="grid w-full grid-cols-3">
          <TabsTrigger value="config">Configuration</TabsTrigger>
          <TabsTrigger value="control">Simulation Control</TabsTrigger>
          <TabsTrigger value="results">Results</TabsTrigger>
        </TabsList>

        <TabsContent value="config">
          <Card>
            <CardHeader>
              <CardTitle>Wireless Network Configuration</CardTitle>
            </CardHeader>
            <CardContent className="space-y-6">
              {/* PHY Type Selection */}
              <div className="space-y-2">
                <Label>PHY Type</Label>
                <RadioGroup
                  value={phyType}
                  onValueChange={setPhyType}
                  className="flex gap-4"
                >
                  {phyTypes.map((type) => (
                    <div key={type.id} className="flex items-center space-x-2">
                      <RadioGroupItem
                        value={type.id}
                        id={type.id}
                        disabled={type.disabled}
                      />
                      <Label
                        htmlFor={type.id}
                        className={type.disabled ? "text-gray-400" : ""}
                      >
                        {type.name}
                      </Label>
                    </div>
                  ))}
                </RadioGroup>
              </div>

              {phyType === "wifi" && (
                <>
                  <div className="space-y-2">
                    <Label>Transmission Power (dBm)</Label>
                    <div className="flex items-center gap-4">
                      <Slider
                        value={[txPower]}
                        onValueChange={([value]) => setTxPower(value)}
                        min={0}
                        max={30}
                        step={1}
                        className="flex-1"
                      />
                      <span className="w-12 text-right">{txPower}</span>
                    </div>
                  </div>

                  <div className="space-y-2">
                    <Label>Frequency Band (GHz)</Label>
                    <div className="flex items-center gap-4">
                      <Slider
                        value={[frequency]}
                        onValueChange={([value]) => setFrequency(value)}
                        min={2.4}
                        max={6}
                        step={0.1}
                        className="flex-1"
                      />
                      <span className="w-12 text-right">{frequency}</span>
                    </div>
                  </div>

                  <div className="space-y-2">
                    <Label>Channel Width (MHz)</Label>
                    <div className="flex items-center gap-4">
                      <Slider
                        value={[channelWidth]}
                        onValueChange={([value]) => setChannelWidth(value)}
                        min={20}
                        max={160}
                        step={20}
                        className="flex-1"
                      />
                      <span className="w-12 text-right">{channelWidth}</span>
                    </div>
                  </div>

                  <div className="space-y-2">
                    <Label>Propagation Loss Model</Label>
                    <select
                      className="w-full p-2 border rounded-md"
                      value={propagationModel}
                      onChange={(e) => setPropagationModel(e.target.value)}
                    >
                      {propagationModels.map((model) => (
                        <option key={model} value={model}>
                          {model}PropagationLossModel
                        </option>
                      ))}
                    </select>
                  </div>
                </>
              )}
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="control">
          <Card>
            <CardHeader>
              <CardTitle>Simulation Control</CardTitle>
            </CardHeader>
            <CardContent className="space-y-6">
              <div className="space-y-2">
                <Label>Number of Nodes</Label>
                <div className="flex items-center gap-4">
                  <Slider
                    value={[nodeCount]}
                    onValueChange={([value]) => setNodeCount(value)}
                    min={2}
                    max={50}
                    step={1}
                    className="flex-1"
                  />
                  <span className="w-12 text-right">{nodeCount}</span>
                </div>
              </div>

              <div className="space-y-2">
                <Label>Simulation Time (seconds)</Label>
                <div className="flex items-center gap-4">
                  <Slider
                    value={[simulationTime]}
                    onValueChange={([value]) => setSimulationTime(value)}
                    min={1}
                    max={30}
                    step={1}
                    className="flex-1"
                  />
                  <span className="w-12 text-right">{simulationTime}</span>
                </div>
              </div>

              <Button
                onClick={handleRunSimulation}
                disabled={isRunning}
                className="w-full"
              >
                {isRunning ? (
                  <>
                    <Pause className="w-4 h-4 mr-2" />
                    Running Simulation...
                  </>
                ) : (
                  <>
                    <Play className="w-4 h-4 mr-2" />
                    Run Simulation
                  </>
                )}
              </Button>
            </CardContent>
          </Card>
        </TabsContent>

        <TabsContent value="results">
          <Card>
            <CardHeader>
              <CardTitle>Simulation Results</CardTitle>
            </CardHeader>
            <CardContent>
              {results ? (
                <div className="space-y-6">
                  <div className="space-y-4">
                    <h3 className="text-lg font-semibold">
                      Throughput Over Time
                    </h3>
                    <LineChart
                      width={700}
                      height={300}
                      data={results.timeSeriesData}
                      margin={{ top: 5, right: 30, left: 20, bottom: 5 }}
                    >
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis
                        dataKey="time"
                        label={{ value: "Time (s)", position: "bottom" }}
                      />
                      <YAxis
                        label={{
                          value: "Throughput (Mbps)",
                          angle: -90,
                          position: "left",
                        }}
                      />
                      <Tooltip />
                      <Legend />
                      <Line
                        type="monotone"
                        dataKey="throughput"
                        stroke="#8884d8"
                        name="Throughput"
                      />
                    </LineChart>
                  </div>
                </div>
              ) : (
                <Alert>
                  <AlertDescription>
                    Run a simulation to see results
                  </AlertDescription>
                </Alert>
              )}
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>
    </div>
  );
};

export default SimulationDashboard;
