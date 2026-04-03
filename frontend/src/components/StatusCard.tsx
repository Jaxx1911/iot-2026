interface StatusCardProps {
  label: string;
  value: string;
  unit: string;
  color?: "green" | "yellow" | "red" | "blue";
  icon?: React.ReactNode;
}

export default function StatusCard({ label, value, unit, color, icon }: StatusCardProps) {
  return (
    <div className="card">
      <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
        {icon}
        <h2>{label}</h2>
      </div>
      <div className={`card-value ${color ?? ""}`}>
        {value}
        <span style={{ fontSize: "1rem", fontWeight: 400, marginLeft: 4 }}>{unit}</span>
      </div>
    </div>
  );
}
