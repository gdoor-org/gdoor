import argparse
import paho.mqtt.client as mqtt
import json
from rich.live import Live
from rich.table import Table, Row
import gdoor

parser = argparse.ArgumentParser(description='Listen to MQTT Server for GDoor message')
parser.add_argument('-s','--server', help='MQTT Server', default="127.0.0.1")
parser.add_argument('-p','--port', help='MQTT Port', default=1883)
parser.add_argument('-t','--topic', help='MQTT Topic', default="/gdoor/bus_rx")
parser.add_argument('-u','--unique', help='Show only unique', default=True)
args = parser.parse_args()

table = Table(show_header=True, header_style="bold magenta")
table.add_column("#")
table.add_column("Action")
table.add_column("Source")
table.add_column("Destination")
table.add_column("Parameters")
table.add_column("Len")
table.add_column("State")
table.add_column("Type")
table.add_column("Hex")

def prependRow(row):
    for index, cell in enumerate(row):
        column = table.columns[index]
        column._cells.insert(0, cell)
    table.rows.append(Row(style=None, end_section=False))

datas = []
def check_uinque(data):
    if data in datas:
        return False
    
    datas.append(data)
    return True


with Live(table, refresh_per_second=4):  # update 4 times a second to feel fluid
    def on_connect(client, userdata, flags, reason_code, properties=None):
        client.subscribe(topic=args.topic)

    def on_message(client, userdata, message, properties=None):
        data = json.loads(message.payload)
        elem = gdoor.GDOOR(data["busdata"])

        if args.unique and check_uinque(data["busdata"]):
            prependRow([data["event_id"], data["action"], data["source"], data["destination"], data["parameters"], data["busdata"][0:2], data["busdata"][2:4], data["type"], data["busdata"]])

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="mqttlisten.py", clean_session=True)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(host=args.server, port=args.port, keepalive=60)
    client.loop_forever()