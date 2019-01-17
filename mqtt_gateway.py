import serial
import paho.mqtt.client as mqtt
from time import sleep
import signal
import sys
import re
import datetime
import logging
from logging.handlers import RotatingFileHandler

# Use the hostname of Dad's computer instead of an IP address
broker_address = "mint-xfce"

# Serial device for the arduino
serial_port = "/dev/ttyACM0"
# Node number for this gateway device
my_node_id = 1

# Flag to halt the processing loop
should_run = True

# Setup logging
formatter = "%(asctime)s %(levelname)s: %(message)s"
#logging.basicConfig(filename="mqtt_gateway.log", format=formatter, level=logging.DEBUG)
logging.basicConfig(format=formatter, level=logging.DEBUG)
logger = logging.getLogger(__name__)
# About 1 MByte / 2 hours.  Log file every ~12 hours, and retain the past 2 days
rfh = RotatingFileHandler("/home/sbmint/home_automation/mqtt_gateway.log", mode='a', maxBytes=6*1024*1024, backupCount=4)
rfh.setLevel(logging.DEBUG)
rfh.setFormatter(logging.Formatter(formatter))
logger.addHandler(rfh)

# Callback function for when a message is received from the broker
def on_message(test_client, userdata, message):
  print("message received " ,str(message.payload.decode("utf-8")))
  print("message topic=",message.topic)
  print("message qos=",message.qos)
  print("message retain flag=",message.retain)
  print(client)
  print(userdata)
  logger.info("Received MQTT message from topic: ",str(message.topic),": "+str(message.payload.decode("utf-8")))

# Callback on SIGINT to properly shutdown everything
def signal_handler(signal, frame):
  # Just tell the processing loop that it should stop
  global should_run
  should_run = False

# Setup the callback for sigint to shutdown stuff safely
signal.signal(signal.SIGINT, signal_handler)

# Setup the connection to the Arduino
arduino = serial.Serial(serial_port, timeout=1, baudrate=115200)

# Wait for a bit for the Arduino to reset
sleep(3)

# Check that the Arduino is ready to go
'''read_line = arduino.readline().encode("utf-8")
while read_line != ("Node "+str(my_node_id)+" ready\n\r").encode("utf-8"):
  print(bytearray(read_line))
  read_line = arduino.readline().decode("utf-8")

print("Connected to the Arduino!")'''
# Just assume the Arduino is ready to go
print("Connected to Arduino")

# Connect to the MQTT broker
client = mqtt.Client()
client.connect(broker_address)
client.on_message = on_message

# Subscribe to our test topic
client.subscribe("lights")

# Kickoff the MQTT networking thread
client.loop_start()
print("Connected to MQTT broker")

# Setup Regexs for parsing data
nodeid_re = re.compile("(?<=node )[0-9]{1,3}")
topic_re = re.compile("\[(.*)\|")
message_re = re.compile("\|(.*)\]")
rssi_re = re.compile("(?<=RSSI )-?[0-9]*")


while should_run:
  # Read in a line from the Arduino
  try:
    read_line = arduino.readline()
    if len(read_line) < 1:
      continue
    read_line = read_line.decode("utf-8")
  except:
    read_line = arduino.readline()
    logger.error("Received packet that cannot decode to UTF-8 len("+str(len(read_line))+"): "+"".join(':{:02x}'.format(x) for x in read_line))
    continue

  # Log the message for debugging
  logger.debug("Received message from RFM69: "+read_line.rstrip())

  # Check if we read anything
  if len(read_line) > 0:
    # Check to see if it is a line that contains publishing information
    if "received from node" in read_line:
      try:
        # Parse the data from the input
        nodeid = nodeid_re.findall(read_line)[0]
        topic = topic_re.findall(read_line)[0]
        message = message_re.findall(read_line)[0]
        rssi = rssi_re.findall(read_line)[0]
        # Print it out for debugging
        logger.info("Publishing message from node {0} {1}: {2}, RSSI: {3}".format(nodeid, topic, message, rssi))
        # Publish the message
        client.publish("/node"+nodeid+topic, message)
        client.publish("/node"+nodeid+"/RSSI", rssi)
        client.publish("/node"+nodeid+"/time", str(datetime.datetime.now().isoformat()))
      except:
        print("Invalid string: "+read_line)


# We are now shutting down.  Shutdown stuff safely
print("Shutting down")
client.loop_stop()
arduino.close()
