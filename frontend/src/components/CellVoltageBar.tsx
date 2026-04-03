interface CellVoltageBarProps {
  cells: number[];
}

const MIN_V = 2.5;
const MAX_V = 3.65;

function getBarColor(v: number): string {
  if (v > 3.60 || v < 2.6) return "danger";
  if (v > 3.55 || v < 2.7) return "warn";
  return "";
}

export default function CellVoltageBar({ cells }: CellVoltageBarProps) {
  return (
    <div className="card">
      <h2>Cell Voltages</h2>
      <div className="cell-bar-group">
        {cells.map((v, i) => {
          const pct = Math.max(0, Math.min(100, ((v - MIN_V) / (MAX_V - MIN_V)) * 100));
          return (
            <div key={i} className="cell-bar-wrapper">
              <span className="cell-voltage-label">{v.toFixed(3)}V</span>
              <div
                className={`cell-bar ${getBarColor(v)}`}
                style={{ height: `${pct}%` }}
              />
              <span className="cell-name">Cell {i + 1}</span>
            </div>
          );
        })}
      </div>
    </div>
  );
}
