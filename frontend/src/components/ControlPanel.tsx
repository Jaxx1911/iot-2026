import { api } from "../api";

export default function ControlPanel() {
  const send = (cmd: string) => {
    api.sendCommand(cmd, "manual").catch((err) => {
      console.error("Command failed:", err);
    });
  };

  return (
    <div className="card">
      <h2>Control</h2>
      <div className="control-panel">
        <button className="btn success" onClick={() => send("relay_on")}>
          All ON
        </button>
        <button className="btn success" onClick={() => send("charge_on")}>
          Charge ON
        </button>
        <button className="btn success" onClick={() => send("discharge_on")}>
          Discharge ON
        </button>
        <button className="btn danger" onClick={() => send("relay_off")}>
          All OFF
        </button>
        <button className="btn danger" onClick={() => send("charge_off")}>
          Charge OFF
        </button>
        <button className="btn danger" onClick={() => send("discharge_off")}>
          Discharge OFF
        </button>
      </div>
    </div>
  );
}
