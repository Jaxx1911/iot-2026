const BASE = "/api";

async function fetchJSON<T>(path: string): Promise<T> {
  const res = await fetch(`${BASE}${path}`);
  if (!res.ok) throw new Error(`API error ${res.status}`);
  return res.json();
}

async function postJSON<T>(path: string, body: unknown): Promise<T> {
  const res = await fetch(`${BASE}${path}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(body),
  });
  if (!res.ok) throw new Error(`API error ${res.status}`);
  return res.json();
}

export const api = {
  getLatestTelemetry: () => fetchJSON<Record<string, unknown>>("/telemetry/latest"),

  getTelemetryHistory: (packId = "pack01", rangeMinutes = 60) =>
    fetchJSON<{ data: Record<string, unknown>[] }>(
      `/telemetry/history?pack_id=${packId}&range_minutes=${rangeMinutes}`
    ),

  getLatestAlerts: (limit = 50) =>
    fetchJSON<Record<string, unknown>[]>(`/alerts/latest?limit=${limit}`),

  getAlertHistory: (packId = "pack01", rangeMinutes = 1440) =>
    fetchJSON<{ data: Record<string, unknown>[] }>(
      `/alerts/history?pack_id=${packId}&range_minutes=${rangeMinutes}`
    ),

  sendCommand: (cmd: string, reason = "", packId = "pack01") =>
    postJSON("/control/command", { pack_id: packId, cmd, reason }),

  getThresholds: () => fetchJSON<Record<string, number>>("/config/thresholds"),
};
