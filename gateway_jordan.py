import serial
import paho.mqtt.client as mqtt
from time import sleep
import signal
import sys
import re
import datetime

# Use the hostname of Dad's computer instead of an IP address
broker_address = "mint-xfce"

# Serial device for the arduino
serial_port = "/dev/ttyACM0"
# Node number for this gateway device
my_node_id = 1

# Flag to halt the processing loop
should_run = True

# Callback function for when a message is received from the broker
def on_message(test_client, userdata, message):
  print("message received " ,str(message.payload.decode("utf-8")))
  print("message topic=",message.topic)
  print("message qos=",message.qos)
  print("message retain flag=",message.retain)
  print(client)
  print(userdata)

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


# Connect to the MQTT broker
client = mqtt.Client()
client.connect(broker_address)
client.on_message = on_message

# Subscribe to our test topic
client.subscribe("lights")

# Kickoff the MQTT networking thread
client.loop_start()

while should_run:
  # Read in a line from the Arduino
  read_line = arduino.readline().decode("utf-8")

  # Check if we read anything
  if len(read_line) > 0:
    # Check to see if it is a line that contains publishing information
    if "received from node" in read_line:
      # Setup Regexs for parsing data
      nodeid_re = re.compile("(?<=node )[0-9]{1,3}")
      topic_re = re.compile("\[(.*)\|")
      message_re = re.compile("\|(.*)\]")
      rssi_re = re.compile("(?<=RSSI )-?[0-9]*")
      # Parse the data from the input
      nodeid = nodeid_re.findall(read_line)[0]
      topic = topic_re.findall(read_line)[0]
      message = message_re.findall(read_line)[0]
      rssi = rssi_re.findall(read_line)[0]
      # Print it out for debugging
      print(nodeid, topic, message, rssi)
      # Publish the message
      client.publish("/node"+nodeid+topic, message)
      client.publish("/node"+nodeid+"/RSSI", rssi)
      client.publish("/node"+nodeid+"/time", str(datetime.datetime.now()))


# We are now shutting down.  Shutdown stuff safely
print("Shutting down.  Goodnight moon")
client.loop_stop()
arduino.close()
