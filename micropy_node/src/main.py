# main.py
import os
import config
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

loop = uasyncio.get_event_loop()
loop.create_task(mqtt_task(mqtt_queue))
loop.create_task(heartbeat_task(mqtt_queue))
loop.create_task(light_task(mqtt_queue))
loop.run_until_complete(killer())
