from pydantic import BaseModel
from typing import Optional


class TelemetryPayload(BaseModel):
    pack_id: str
    ts: int
    cells: list[float]
    v_pack: float
    current: float
    temp: list[float]
    soc: float


class Alert(BaseModel):
    pack_id: str
    type: str
    severity: str
    cell_index: int
    value: float
    threshold: float
    ts: int
    action_taken: str


class ControlCommand(BaseModel):
    pack_id: str = "pack01"
    cmd: str
    reason: str = ""


class ThresholdConfig(BaseModel):
    cell_ov: Optional[float] = None
    cell_uv: Optional[float] = None
    pack_oc: Optional[float] = None
    temp_ot: Optional[float] = None
    cell_delta_max: Optional[float] = None
