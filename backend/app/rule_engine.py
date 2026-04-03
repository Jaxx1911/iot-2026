"""Server-side rule engine – evaluates telemetry and returns alerts + commands."""

from . import config
from .models import Alert, ControlCommand
import time


def evaluate(data: dict) -> tuple[list[Alert], ControlCommand | None]:
    alerts: list[Alert] = []
    cmd: ControlCommand | None = None
    ts = int(time.time())
    pack_id = data.get("pack_id", "pack01")

    cells = data.get("cells", [])
    current = float(data.get("current", 0))
    temps = data.get("temp", [])

    # Over-voltage
    for i, v in enumerate(cells):
        v = float(v)
        if v > config.CELL_OV:
            alerts.append(Alert(
                pack_id=pack_id, type="over_voltage", severity="critical",
                cell_index=i, value=v, threshold=config.CELL_OV,
                ts=ts, action_taken="charge_off",
            ))
            cmd = ControlCommand(pack_id=pack_id, cmd="charge_off",
                                 reason="over_voltage")

    # Under-voltage
    for i, v in enumerate(cells):
        v = float(v)
        if v < config.CELL_UV:
            alerts.append(Alert(
                pack_id=pack_id, type="under_voltage", severity="critical",
                cell_index=i, value=v, threshold=config.CELL_UV,
                ts=ts, action_taken="discharge_off",
            ))
            cmd = ControlCommand(pack_id=pack_id, cmd="discharge_off",
                                 reason="under_voltage")

    # Over-current
    if abs(current) > config.PACK_OC:
        alerts.append(Alert(
            pack_id=pack_id, type="over_current", severity="critical",
            cell_index=-1, value=current, threshold=config.PACK_OC,
            ts=ts, action_taken="relay_off",
        ))
        cmd = ControlCommand(pack_id=pack_id, cmd="relay_off",
                             reason="over_current")

    # Over-temperature
    for i, t in enumerate(temps):
        t = float(t)
        if t > config.TEMP_OT and t != -127.0:
            alerts.append(Alert(
                pack_id=pack_id, type="over_temperature", severity="critical",
                cell_index=i, value=t, threshold=config.TEMP_OT,
                ts=ts, action_taken="relay_off",
            ))
            cmd = ControlCommand(pack_id=pack_id, cmd="relay_off",
                                 reason="over_temperature")

    # Cell imbalance
    if len(cells) >= 2:
        fv = [float(v) for v in cells]
        delta = max(fv) - min(fv)
        if delta > config.CELL_DELTA_MAX:
            alerts.append(Alert(
                pack_id=pack_id, type="cell_imbalance", severity="warning",
                cell_index=-1, value=delta, threshold=config.CELL_DELTA_MAX,
                ts=ts, action_taken="none",
            ))

    return alerts, cmd
