from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime, timezone
from . import config


_client = InfluxDBClient(
    url=config.INFLUXDB_URL,
    token=config.INFLUXDB_TOKEN,
    org=config.INFLUXDB_ORG,
)
_write_api = _client.write_api(write_options=SYNCHRONOUS)
_query_api = _client.query_api()


def write_telemetry(data: dict):
    p = Point("telemetry").tag("pack_id", data["pack_id"])

    for i, v in enumerate(data["cells"]):
        p = p.field(f"cell_{i}", float(v))

    p = (
        p.field("v_pack", float(data["v_pack"]))
        .field("current", float(data["current"]))
        .field("soc", float(data["soc"]))
    )

    for i, t in enumerate(data["temp"]):
        p = p.field(f"temp_{i}", float(t))

    p = p.time(datetime.fromtimestamp(data["ts"], tz=timezone.utc))
    _write_api.write(bucket=config.INFLUXDB_BUCKET, record=p)


def write_alert(alert: dict):
    p = (
        Point("alert")
        .tag("pack_id", alert["pack_id"])
        .tag("type", alert["type"])
        .tag("severity", alert["severity"])
        .field("cell_index", alert.get("cell_index", -1))
        .field("value", float(alert["value"]))
        .field("threshold", float(alert["threshold"]))
        .field("action_taken", alert.get("action_taken", ""))
        .time(datetime.fromtimestamp(alert["ts"], tz=timezone.utc))
    )
    _write_api.write(bucket=config.INFLUXDB_BUCKET, record=p)


def query_telemetry(pack_id: str, range_minutes: int = 60):
    query = f'''
    from(bucket: "{config.INFLUXDB_BUCKET}")
      |> range(start: -{range_minutes}m)
      |> filter(fn: (r) => r._measurement == "telemetry")
      |> filter(fn: (r) => r.pack_id == "{pack_id}")
      |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
      |> sort(columns: ["_time"], desc: false)
    '''
    tables = _query_api.query(query)
    results = []
    for table in tables:
        for record in table.records:
            row = {"time": record.get_time().isoformat()}
            row.update({k: v for k, v in record.values.items()
                        if k not in ("result", "table", "_start", "_stop",
                                     "_time", "_measurement", "pack_id")})
            results.append(row)
    return results


def query_alerts(pack_id: str, range_minutes: int = 1440):
    query = f'''
    from(bucket: "{config.INFLUXDB_BUCKET}")
      |> range(start: -{range_minutes}m)
      |> filter(fn: (r) => r._measurement == "alert")
      |> filter(fn: (r) => r.pack_id == "{pack_id}")
      |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
      |> sort(columns: ["_time"], desc: true)
    '''
    tables = _query_api.query(query)
    results = []
    for table in tables:
        for record in table.records:
            row = {
                "time": record.get_time().isoformat(),
                "type": record.values.get("type", ""),
                "severity": record.values.get("severity", ""),
            }
            row.update({k: v for k, v in record.values.items()
                        if k not in ("result", "table", "_start", "_stop",
                                     "_time", "_measurement", "pack_id",
                                     "type", "severity")})
            results.append(row)
    return results
