import paho.mqtt.client as mqtt 

# use the hostname of computer instead of IP address
broker_address = "mint-xfce" 

def on_message(client, userdata, message):
  print("message received " ,str(message.payload.decode("utf-8"))) 
  print("message topic=",message.topic) 
  print("message qos=",message.qos)
  print("message retain flag=",message.retain)
  print(client) 
  print(userdata) 

# Connect to the MQTT broker 
client = mqtt.Client() 
client.connect(broker_address) 
client.on_message = on_message 

# Subscribe to our test topi 
client.subscribe("lights") 

# Just continuously look while we receive messages
client.loop_forever() 
