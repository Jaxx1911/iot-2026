import { useEffect, useState } from "react";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  ResponsiveContainer,
} from "recharts";
import { api } from "../api";

interface Row {
  time: string;
  [key: string]: unknown;
}

export default function HistoryChart() {
  const [data, setData] = useState<Row[]>([]);
  const [range, setRange] = useState(30);

  useEffect(() => {
    let cancelled = false;
    const load = async () => {
      try {
        const res = await api.getTelemetryHistory("pack01", range);
        if (!cancelled) setData(res.data as Row[]);
      } catch {
        /* ignore */
      }
    };
    load();
    const id = setInterval(load, 15000);
    return () => {
      cancelled = true;
      clearInterval(id);
    };
  }, [range]);

  const formatted = data.map((r) => ({
    ...r,
    label: new Date(r.time).toLocaleTimeString(),
  }));

  return (
    <div className="chart-container">
      <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: 12 }}>
        <h2>History</h2>
        <div style={{ display: "flex", gap: 6 }}>
          {[10, 30, 60, 360].map((m) => (
            <button
              key={m}
              className={`btn ${range === m ? "success" : ""}`}
              style={{ padding: "4px 12px", fontSize: "0.75rem" }}
              onClick={() => setRange(m)}
            >
              {m}m
            </button>
          ))}
        </div>
      </div>

      {formatted.length === 0 ? (
        <div className="no-data">No history data yet</div>
      ) : (
        <ResponsiveContainer width="100%" height={300}>
          <LineChart data={formatted}>
            <CartesianGrid strokeDasharray="3 3" stroke="#2e3240" />
            <XAxis dataKey="label" stroke="#8b8fa3" fontSize={11} />
            <YAxis stroke="#8b8fa3" fontSize={11} />
            <Tooltip
              contentStyle={{
                background: "#1a1d27",
                border: "1px solid #2e3240",
                borderRadius: 8,
                fontSize: 12,
              }}
            />
            <Legend />
            <Line type="monotone" dataKey="cell_0" stroke="#3b82f6" dot={false} name="Cell 1" />
            <Line type="monotone" dataKey="cell_1" stroke="#22c55e" dot={false} name="Cell 2" />
            <Line type="monotone" dataKey="cell_2" stroke="#eab308" dot={false} name="Cell 3" />
            <Line type="monotone" dataKey="cell_3" stroke="#f97316" dot={false} name="Cell 4" />
          </LineChart>
        </ResponsiveContainer>
      )}
    </div>
  );
}
