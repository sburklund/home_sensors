# home_sensors
Home sensors with OpenHAB

# OpenHAB Configuration
Default environment variables can be modified in `/etc/default/openhab2`
Change the configuration file location `OPENHAB_CONFIG` to the directory
of the repository: e.g. `OPENHAB_CONF=~/home_automation/openhab2`

# Set-up of microcontroller
Run esptool to flash micropython to esp8266 controller

`esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash --verify --flash_size=detect 0 ~/home_sensor_ws/esp8266-20191220-v1.12.bin`

# Validate code is loaded and correct
From the Pycom Console REPL run the code below
```
import esp
esp.check_fw()
```

# Update the configuration settings and push the main and config python scripts to the node

1. Change the node number in the config.py to the node you are updating
    `MQTT_PREFIX = b'/node2' `
2. Verify that the sensors on on the pins set in config.py
    ```
    # pin number on board
    TEMP_SENSOR_PIN   = 14
    MOTION_SENSOR_PIN = 12
    ```
3. Change the temp/humidity sensor type and has sonar sensor settings if needed
    ```
    # sensors available
    TEMP_SENSOR_TYPE  = 'DHT22'
    HAS_SONAR_SENSOR  = True
    ```
4. Use the 'Upload' button in the VSCode status bar to push the main.py and config.py to the node
