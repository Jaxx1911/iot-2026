export interface Telemetry {
  pack_id: string;
  ts: number;
  cells: number[];
  v_pack: number;
  current: number;
  temp: number[];
  soc: number;
}

export interface Alert {
  pack_id: string;
  type: string;
  severity: "critical" | "warning" | "info";
  cell_index: number;
  value: number;
  threshold: number;
  ts: number;
  action_taken: string;
  time?: string;
}

export interface Thresholds {
  cell_ov: number;
  cell_uv: number;
  pack_oc: number;
  temp_ot: number;
  cell_delta_max: number;
}

export interface HistoryRow {
  time: string;
  cell_0?: number;
  cell_1?: number;
  cell_2?: number;
  cell_3?: number;
  v_pack?: number;
  current?: number;
  soc?: number;
  temp_0?: number;
  temp_1?: number;
}
