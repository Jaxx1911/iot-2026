import { useEffect, useState } from "react";
import { Battery, Thermometer, Zap, Activity } from "lucide-react";
import StatusCard from "./components/StatusCard";
import CellVoltageBar from "./components/CellVoltageBar";
import AlertPanel from "./components/AlertPanel";
import ControlPanel from "./components/ControlPanel";
import HistoryChart from "./components/HistoryChart";
import type { Telemetry, Alert } from "./types";
import { api } from "./api";

const POLL_MS = 2000;

function getSocColor(soc: number): "green" | "yellow" | "red" {
  if (soc > 50) return "green";
  if (soc > 20) return "yellow";
  return "red";
}

export default function App() {
  const [telem, setTelem] = useState<Telemetry | null>(null);
  const [alerts, setAlerts] = useState<Alert[]>([]);
  const [online, setOnline] = useState(false);

  useEffect(() => {
    let cancelled = false;

    const poll = async () => {
      try {
        const t = await api.getLatestTelemetry();
        if (!cancelled && t && !("status" in t && t.status === "no_data")) {
          setTelem(t as unknown as Telemetry);
          setOnline(true);
        } else {
          setOnline(false);
        }
      } catch {
        setOnline(false);
      }

      try {
        const a = await api.getLatestAlerts(50);
        if (!cancelled) setAlerts(a as unknown as Alert[]);
      } catch {
        /* ignore */
      }
    };

    poll();
    const id = setInterval(poll, POLL_MS);
    return () => {
      cancelled = true;
      clearInterval(id);
    };
  }, []);

  return (
    <>
      <div className="header">
        <h1>BMS Dashboard — LiFePO4 4S</h1>
        <div className="header-status">
          <div className={`status-dot ${online ? "" : "offline"}`} />
          {online ? "Online" : "Offline"}
        </div>
      </div>

      {!telem ? (
        <div className="no-data" style={{ height: 400 }}>
          Waiting for telemetry data...
        </div>
      ) : (
        <>
          <div className="grid">
            <StatusCard
              icon={<Battery size={18} />}
              label="Pack Voltage"
              value={telem.v_pack.toFixed(2)}
              unit="V"
              color="blue"
            />
            <StatusCard
              icon={<Activity size={18} />}
              label="SOC"
              value={telem.soc.toFixed(1)}
              unit="%"
              color={getSocColor(telem.soc)}
            />
            <StatusCard
              icon={<Zap size={18} />}
              label="Current"
              value={telem.current.toFixed(2)}
              unit="A"
              color={telem.current >= 0 ? "green" : "yellow"}
            />
            <StatusCard
              icon={<Thermometer size={18} />}
              label="Temperature"
              value={Math.max(...telem.temp).toFixed(1)}
              unit="°C"
              color={Math.max(...telem.temp) > 40 ? "red" : "green"}
            />
          </div>

          <div className="two-col">
            <CellVoltageBar cells={telem.cells} />
            <ControlPanel />
          </div>

          <HistoryChart />

          <AlertPanel alerts={alerts} />
        </>
      )}
    </>
  );
}
