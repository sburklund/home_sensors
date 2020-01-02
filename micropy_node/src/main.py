# main.py
import os
import config
import dht
import machine
import network
import uasyncio
import uasyncio.queues
import ubinascii
from umqtt.simple import MQTTClient
import utime

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
    await uasyncio.sleep(60)

async def heartbeat_task(msg_queue):
    while True:
        await uasyncio.sleep(config.HEARTBEAT_PERIOD)
        await msg_queue.put((config.MQTT_TOPIC_HEARTBEAT, b'heartbeat ok'))

async def blink_light_task():
    led = machine.Pin(2, machine.Pin.OUT)
    while True:
        led.off()
        await uasyncio.sleep_ms(int(config.BLINK_ON_TIME * 1000))
        led.on()
        await uasyncio.sleep_ms(int(config.BLINK_OFF_TIME * 1000))

async def light_task(msg_queue):
    light_adc = machine.ADC(0)
    while True:
        await uasyncio.sleep(config.LIGHT_POLLING_PERIOD)
        light_value = 1024 - light_adc.read()
        await msg_queue.put((config.MQTT_TOPIC_LIGHT, str(light_value).encode('ascii')))

async def temp_humid_task(msg_queue):
    if config.TEMP_SENSOR_TYPE == 'DHT22':
        temp_humid_sensor = dht.DHT22(machine.Pin(config.TEMP_SENSOR_PIN))
    else:
        temp_humid_sensor = dht.DHT11(machine.Pin(config.TEMP_SENSOR_PIN))
    while True:
        await uasyncio.sleep(config.TEMP_POLLING_PERIOD)
        temp_humid_sensor.measure()
        temp_value = (temp_humid_sensor.temperature() * (9/5)) + 32
        humid_value = temp_humid_sensor.humidity()
        await msg_queue.put((config.MQTT_TOPIC_TEMP, str(temp_value).encode('ascii')))
        await msg_queue.put((config.MQTT_TOPIC_HUMID, str(humid_value).encode('ascii')))

async def motion_task(msg_queue):
    motion_sensor = machine.Pin(config.MOTION_SENSOR_PIN, machine.Pin.IN)
    last_motion_time = utime.ticks_ms()
    while True:
        await uasyncio.sleep(config.MOTION_POLLING_PERIOD)
        motion_value = motion_sensor.value()
        if motion_value:
            last_motion_time = utime.ticks_ms()
            await msg_queue.put((config.MQTT_TOPIC_MOTION, b'Activity'))
        elif not motion_value and utime.ticks_diff(utime.ticks_ms(), utime.ticks_add(last_motion_time, config.MOTION_MIN_ONTIME * 1000)) < 0:
            await msg_queue.put((config.MQTT_TOPIC_MOTION, b'Activity'))
        else:
            await msg_queue.put((config.MQTT_TOPIC_MOTION, b'No Activity'))

async def sonar_task(msg_queue):
    trigger_pin = machine.Pin(config.SONAR_TRIGGER_PIN, machine.Pin.OUT)
    echo_pin = machine.Pin(config.SONAR_ECHO_PIN, machine.Pin.IN)
    while True:
        await uasyncio.sleep(config.SONAR_POLLING_PERIOD)
        trigger_pin.on()
        utime.sleep_us(10)
        trigger_pin.off()
        time_of_flight = machine.time_pulse_us(echo_pin, 1, int(config.SONAR_TIMEOUT * 1e6))
        if time_of_flight > 0:
            distance = time_of_flight / 148
        else:
            distance = time_of_flight  # report time of flight errors
        await msg_queue.put((config.MQTT_TOPIC_SONAR, str(distance).encode('ascii')))

async def rssi_task(msg_queue):
    sta_if = network.WLAN(network.STA_IF)
    while True:
        await uasyncio.sleep(config.RSSI_POLLING_PERIOD)
        await msg_queue.put((config.MQTT_TOPIC_RSSI, str(sta_if.status('rssi')).encode('ascii')))

async def ip_addr_task(msg_queue):
    sta_if = network.WLAN(network.STA_IF)
    while True:
        # Publish before waiting so that this shows soon after boot
        await msg_queue.put((config.MQTT_TOPIC_IP_ADDR, sta_if.ifconfig()[0].encode('ascii')))
        await uasyncio.sleep(config.IP_ADDR_POLLING_PERIOD)

loop = uasyncio.get_event_loop()
loop.create_task(mqtt_task(mqtt_queue))
loop.create_task(heartbeat_task(mqtt_queue))
loop.create_task(blink_light_task())
loop.create_task(light_task(mqtt_queue))
loop.create_task(temp_humid_task(mqtt_queue))
loop.create_task(motion_task(mqtt_queue))
if config.HAS_SONAR_SENSOR:
    loop.create_task(sonar_task(mqtt_queue))
loop.create_task(rssi_task(mqtt_queue))
loop.create_task(ip_addr_task(mqtt_queue))
loop.run_forever()
#loop.run_until_complete(killer())
