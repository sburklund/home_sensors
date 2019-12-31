# main.py
import os
import config
import dht
import machine
import uasyncio
import uasyncio.queues
import ubinascii
from umqtt.simple import MQTTClient

print(os.listdir())

mqtt_queue = uasyncio.queues.Queue()

async def mqtt_task(msg_queue):
    client_id = ubinascii.hexlify(machine.unique_id())
    client = MQTTClient(client_id, config.MQTT_HOST)
    client.connect()
    while True:
        result = await msg_queue.get()
        client.publish(result[0], result[1])

async def killer():
    await uasyncio.sleep(30)

async def heartbeat_task(msg_queue):
    while True:
        await uasyncio.sleep(config.HEARTBEAT_PERIOD)
        await msg_queue.put((config.MQTT_TOPIC_HEARTBEAT, b'heartbeat ok'))

async def light_task(msg_queue):
    light_adc = machine.ADC(0)
    while True:
        await uasyncio.sleep(config.LIGHT_POLLING_PERIOD)
        light_value = 1024 - light_adc.read()
        await msg_queue.put((config.MQTT_TOPIC_LIGHT, str(light_value).encode('ascii')))

async def temp_humid_task(msg_queue):
    temp_humid_sensor = dht.DHT11(machine.Pin(config.TEMP_SENSOR_PIN))
    while True:
        await uasyncio.sleep(config.TEMP_POLLING_PERIOD)
        temp_humid_sensor.measure()
        temp_value = (temp_humid_sensor.temperature() * (9/5)) + 32
        humid_value = temp_humid_sensor.humidity()
        await msg_queue.put((config.MQTT_TOPIC_TEMP, str(temp_value).encode('ascii')))
        await msg_queue.put((config.MQTT_TOPIC_HUMID, str(humid_value).encode('ascii')))

loop = uasyncio.get_event_loop()
loop.create_task(mqtt_task(mqtt_queue))
loop.create_task(heartbeat_task(mqtt_queue))
loop.create_task(light_task(mqtt_queue))
loop.create_task(temp_humid_task(mqtt_queue))
loop.run_until_complete(killer())
