import type { Alert } from "../types";

interface AlertPanelProps {
  alerts: Alert[];
}

const typeLabels: Record<string, string> = {
  over_voltage: "Qua ap",
  under_voltage: "Thieu ap",
  over_current: "Qua dong",
  over_temperature: "Qua nhiet",
  cell_imbalance: "Mat can bang",
};

export default function AlertPanel({ alerts }: AlertPanelProps) {
  if (!alerts.length) {
    return (
      <div className="card">
        <h2>Alerts</h2>
        <div className="no-data">No alerts</div>
      </div>
    );
  }

  return (
    <div className="card">
      <h2>Alerts ({alerts.length})</h2>
      <div className="alert-list">
        {[...alerts].reverse().map((a, i) => {
          const time = a.time
            ? new Date(a.time).toLocaleTimeString()
            : new Date(a.ts * 1000).toLocaleTimeString();
          return (
            <div key={i} className={`alert-item ${a.severity}`}>
              <span className="alert-time">{time}</span>
              <span style={{ fontWeight: 600 }}>
                {typeLabels[a.type] ?? a.type}
              </span>
              <span>
                {a.cell_index >= 0 ? `Cell ${a.cell_index + 1} ` : ""}
                {typeof a.value === "number" ? a.value.toFixed(2) : a.value}
                {" / "}
                {typeof a.threshold === "number" ? a.threshold.toFixed(2) : a.threshold}
              </span>
              {a.action_taken && a.action_taken !== "none" && (
                <span style={{ color: "var(--red)", marginLeft: "auto" }}>
                  {a.action_taken}
                </span>
              )}
            </div>
          );
        })}
      </div>
    </div>
  );
}
