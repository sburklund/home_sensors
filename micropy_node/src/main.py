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
    await uasyncio.sleep(10)

async def test_publish(msg_queue):
    while True:
        await uasyncio.sleep(1)
        await msg_queue.put((b'/hello', b'Hello from async'))

loop = uasyncio.get_event_loop()
loop.create_task(mqtt_task(mqtt_queue))
loop.create_task(test_publish(mqtt_queue))
loop.run_until_complete(killer())
