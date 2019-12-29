# main.py
import os
import config
import machine
import ubinascii
from umqtt.simple import MQTTClient

print(os.listdir())

client_id = ubinascii.hexlify(machine.unique_id())
client = MQTTClient(client_id, config.MQTT_HOST)
client.connect()
client.publish(b'/hello', b'Hello from espp ' + client_id)
