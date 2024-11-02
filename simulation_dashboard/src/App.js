import React from "react";
import SimulationDashboard from "./components/SimulationDashboard";

function App() {
  return (
    <div className="min-h-screen bg-background p-4">
      <header className="mb-8">
        <h1 className="text-3xl font-bold text-center">
          NS-FLS Simulation Dashboard
        </h1>
      </header>
      <main>
        <SimulationDashboard />
      </main>
    </div>
  );
}

export default App;
