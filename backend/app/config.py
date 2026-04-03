import os

MQTT_BROKER = os.getenv("MQTT_BROKER", "localhost")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))

INFLUXDB_URL = os.getenv("INFLUXDB_URL", "http://localhost:8086")
INFLUXDB_TOKEN = os.getenv("INFLUXDB_TOKEN", "bms-secret-token")
INFLUXDB_ORG = os.getenv("INFLUXDB_ORG", "bms-org")
INFLUXDB_BUCKET = os.getenv("INFLUXDB_BUCKET", "bms")

# LiFePO4 thresholds (mirrored from firmware for server-side rules)
CELL_OV = float(os.getenv("CELL_OV", "3.65"))
CELL_UV = float(os.getenv("CELL_UV", "2.50"))
PACK_OC = float(os.getenv("PACK_OC", "10.0"))
TEMP_OT = float(os.getenv("TEMP_OT", "45.0"))
CELL_DELTA_MAX = float(os.getenv("CELL_DELTA_MAX", "0.10"))
