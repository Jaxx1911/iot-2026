"""
BMS Simulator – generates realistic LiFePO4 4S telemetry over MQTT.
Run this when you don't have physical hardware connected.

Usage:
    python bms_simulator.py [--broker localhost] [--port 1883] [--interval 2]
"""

import argparse
import json
import math
import random
import time

import paho.mqtt.client as mqtt

PACK_ID = "pack01"


class BatterySimulator:
    def __init__(self):
        self.soc = 80.0
        self.charging = True
        self.base_temp = 25.0
        self.cycle_start = time.time()

    def step(self, dt: float) -> dict:
        # SOC drifts up when charging, down when discharging
        if self.charging:
            self.soc += random.uniform(0.02, 0.08) * dt
            if self.soc >= 95:
                self.charging = False
        else:
            self.soc -= random.uniform(0.02, 0.08) * dt
            if self.soc <= 20:
                self.charging = True

        self.soc = max(0, min(100, self.soc))

        # Map SOC to cell voltage (LiFePO4 flat curve ~3.2-3.3V in 20-80%)
        v_base = 2.5 + (self.soc / 100.0) * 1.15
        cells = [
            round(v_base + random.gauss(0, 0.005), 3)
            for _ in range(4)
        ]

        current = round(
            (random.uniform(1.5, 3.0) if self.charging
             else -random.uniform(1.0, 2.5)),
            3,
        )

        # Temperature slightly oscillates
        elapsed = time.time() - self.cycle_start
        t_offset = 2.0 * math.sin(elapsed / 120)
        temps = [
            round(self.base_temp + t_offset + random.gauss(0, 0.3), 1),
            round(self.base_temp + t_offset + 1.0 + random.gauss(0, 0.3), 1),
        ]

        return {
            "pack_id": PACK_ID,
            "ts": int(time.time()),
            "cells": cells,
            "v_pack": round(sum(cells), 2),
            "current": current,
            "temp": temps,
            "soc": round(self.soc, 1),
        }


def main():
    parser = argparse.ArgumentParser(description="BMS MQTT Simulator")
    parser.add_argument("--broker", default="localhost")
    parser.add_argument("--port", type=int, default=1883)
    parser.add_argument("--interval", type=float, default=2.0,
                        help="Publish interval in seconds")
    args = parser.parse_args()

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.connect(args.broker, args.port, 60)
    client.loop_start()

    sim = BatterySimulator()
    topic = f"bms/{PACK_ID}/telemetry"

    print(f"Simulator publishing to {args.broker}:{args.port} topic={topic}")
    print(f"Interval: {args.interval}s  |  Ctrl+C to stop\n")

    try:
        while True:
            data = sim.step(args.interval)
            payload = json.dumps(data)
            client.publish(topic, payload)
            print(f"[{data['ts']}] SOC={data['soc']}% "
                  f"Vpack={data['v_pack']}V I={data['current']}A "
                  f"T={data['temp']}")
            time.sleep(args.interval)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        client.loop_stop()
        client.disconnect()


if __name__ == "__main__":
    main()
