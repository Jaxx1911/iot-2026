"""MQTT subscriber/publisher running in a background thread."""

import json
import threading
import time
import paho.mqtt.client as paho_mqtt
from . import config
from . import influx
from .rule_engine import evaluate

_client: paho_mqtt.Client | None = None
_latest_telemetry: dict = {}
_latest_alerts: list[dict] = []
_lock = threading.Lock()


def _on_connect(client, userdata, flags, rc, properties=None):
    print(f"[MQTT] Connected rc={rc}")
    client.subscribe("bms/+/telemetry")
    client.subscribe("bms/+/alert")


def _on_message(client, userdata, msg):
    global _latest_telemetry
    try:
        payload = json.loads(msg.payload.decode())
    except json.JSONDecodeError:
        return

    topic = msg.topic
    if topic.endswith("/telemetry"):
        with _lock:
            _latest_telemetry = payload

        try:
            influx.write_telemetry(payload)
        except Exception as e:
            print(f"[INFLUX] Write error: {e}")

        alerts, cmd = evaluate(payload)
        for a in alerts:
            alert_dict = a.model_dump()
            try:
                influx.write_alert(alert_dict)
            except Exception as e:
                print(f"[INFLUX] Alert write error: {e}")
            with _lock:
                _latest_alerts.append(alert_dict)
                if len(_latest_alerts) > 200:
                    _latest_alerts.pop(0)

        if cmd:
            _publish_cmd(cmd.pack_id, cmd.cmd, cmd.reason)

    elif topic.endswith("/alert"):
        with _lock:
            _latest_alerts.append(payload)
            if len(_latest_alerts) > 200:
                _latest_alerts.pop(0)
        try:
            influx.write_alert(payload)
        except Exception:
            pass


def _publish_cmd(pack_id: str, cmd: str, reason: str = ""):
    if _client and _client.is_connected():
        payload = json.dumps({
            "pack_id": pack_id,
            "cmd": cmd,
            "reason": reason,
            "ts": int(time.time()),
        })
        _client.publish(f"bms/{pack_id}/cmd", payload)
        print(f"[MQTT] Published cmd: {cmd} reason={reason}")


def send_command(pack_id: str, cmd: str, reason: str = ""):
    _publish_cmd(pack_id, cmd, reason)


def get_latest_telemetry() -> dict:
    with _lock:
        return dict(_latest_telemetry)


def get_latest_alerts(limit: int = 50) -> list[dict]:
    with _lock:
        return list(_latest_alerts[-limit:])


def start():
    global _client
    _client = paho_mqtt.Client(paho_mqtt.CallbackAPIVersion.VERSION2)
    _client.on_connect = _on_connect
    _client.on_message = _on_message

    def _loop():
        while True:
            try:
                _client.connect(config.MQTT_BROKER, config.MQTT_PORT, 60)
                _client.loop_forever()
            except Exception as e:
                print(f"[MQTT] Connection error: {e}, retrying in 5s...")
                time.sleep(5)

    t = threading.Thread(target=_loop, daemon=True)
    t.start()
    print("[MQTT] Background thread started")
