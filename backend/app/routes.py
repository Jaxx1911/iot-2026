from fastapi import APIRouter, Query
from .models import ControlCommand, ThresholdConfig
from . import mqtt_client, influx, config

router = APIRouter()


@router.get("/telemetry/latest")
def get_latest_telemetry():
    data = mqtt_client.get_latest_telemetry()
    if not data:
        return {"status": "no_data"}
    return data


@router.get("/telemetry/history")
def get_telemetry_history(
    pack_id: str = "pack01",
    range_minutes: int = Query(default=60, ge=1, le=10080),
):
    rows = influx.query_telemetry(pack_id, range_minutes)
    return {"pack_id": pack_id, "range_minutes": range_minutes, "data": rows}


@router.get("/alerts/latest")
def get_latest_alerts(limit: int = Query(default=50, ge=1, le=200)):
    return mqtt_client.get_latest_alerts(limit)


@router.get("/alerts/history")
def get_alert_history(
    pack_id: str = "pack01",
    range_minutes: int = Query(default=1440, ge=1, le=43200),
):
    rows = influx.query_alerts(pack_id, range_minutes)
    return {"pack_id": pack_id, "range_minutes": range_minutes, "data": rows}


@router.post("/control/command")
def send_command(body: ControlCommand):
    mqtt_client.send_command(body.pack_id, body.cmd, body.reason)
    return {"status": "sent", "cmd": body.cmd}


@router.get("/config/thresholds")
def get_thresholds():
    return {
        "cell_ov": config.CELL_OV,
        "cell_uv": config.CELL_UV,
        "pack_oc": config.PACK_OC,
        "temp_ot": config.TEMP_OT,
        "cell_delta_max": config.CELL_DELTA_MAX,
    }


@router.put("/config/thresholds")
def update_thresholds(body: ThresholdConfig):
    if body.cell_ov is not None:
        config.CELL_OV = body.cell_ov
    if body.cell_uv is not None:
        config.CELL_UV = body.cell_uv
    if body.pack_oc is not None:
        config.PACK_OC = body.pack_oc
    if body.temp_ot is not None:
        config.TEMP_OT = body.temp_ot
    if body.cell_delta_max is not None:
        config.CELL_DELTA_MAX = body.cell_delta_max
    return get_thresholds()


@router.get("/health")
def health():
    return {"status": "ok"}
