"""
Fault scenario tester – injects abnormal telemetry to verify rule engine + alerts.

Usage:
    python test_fault_scenarios.py [--broker localhost] [--port 1883]
"""

import argparse
import json
import time
import paho.mqtt.client as mqtt

PACK_ID = "pack01"
TOPIC = f"bms/{PACK_ID}/telemetry"


def make_payload(**overrides):
    base = {
        "pack_id": PACK_ID,
        "ts": int(time.time()),
        "cells": [3.30, 3.31, 3.29, 3.32],
        "v_pack": 13.22,
        "current": 1.5,
        "temp": [28.0, 29.0],
        "soc": 70.0,
    }
    base.update(overrides)
    base["v_pack"] = sum(base["cells"])
    return base


SCENARIOS = [
    {
        "name": "Normal operation",
        "payload": make_payload(),
        "expected": "No alerts",
    },
    {
        "name": "Over-voltage on Cell 3",
        "payload": make_payload(cells=[3.30, 3.31, 3.70, 3.32]),
        "expected": "Alert: over_voltage, Action: charge_off",
    },
    {
        "name": "Under-voltage on Cell 1",
        "payload": make_payload(cells=[2.40, 3.31, 3.29, 3.32]),
        "expected": "Alert: under_voltage, Action: discharge_off",
    },
    {
        "name": "Over-current",
        "payload": make_payload(current=12.5),
        "expected": "Alert: over_current, Action: relay_off",
    },
    {
        "name": "Over-temperature sensor 2",
        "payload": make_payload(temp=[28.0, 48.0]),
        "expected": "Alert: over_temperature, Action: relay_off",
    },
    {
        "name": "Cell imbalance",
        "payload": make_payload(cells=[3.30, 3.30, 3.30, 3.15]),
        "expected": "Alert: cell_imbalance (warning)",
    },
    {
        "name": "Recovery – normal again",
        "payload": make_payload(),
        "expected": "No alerts",
    },
]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--broker", default="localhost")
    parser.add_argument("--port", type=int, default=1883)
    args = parser.parse_args()

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.connect(args.broker, args.port, 60)
    client.loop_start()

    print(f"Running {len(SCENARIOS)} fault scenarios against {args.broker}:{args.port}\n")

    for i, sc in enumerate(SCENARIOS, 1):
        print(f"--- Scenario {i}: {sc['name']} ---")
        print(f"  Expected: {sc['expected']}")
        payload = json.dumps(sc["payload"])
        client.publish(TOPIC, payload)
        print(f"  Published. Waiting 3s...\n")
        time.sleep(3)

    print("All scenarios sent. Check the dashboard and backend logs for results.")
    client.loop_stop()
    client.disconnect()


if __name__ == "__main__":
    main()
