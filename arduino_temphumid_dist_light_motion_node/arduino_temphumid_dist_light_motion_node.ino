// RFM69HCW Example Sketc
// Send serial input characters from one RFM69 node to another
// Based on RFM69 library sample code by Felix Rusu
// http://LowPowerLab.com/contact
// Modified for RFM69HCW by Mike Grusin, 4/16

// This sketch will show you the basics of using an
// RFM69HCW radio module. SparkFun's part numbers are:
// 915MHz: https://www.sparkfun.com/products/12775
// 434MHz: https://www.sparkfun.com/products/12823

// See the hook-up guide for wiring instructions:
// https://learn.sparkfun.com/tutorials/rfm69hcw-hookup-guide

// Uses the RFM69 library by Felix Rusu, LowPowerLab.com
// Original library: https://www.github.com/lowpowerlab/rfm69
// SparkFun repository: https://github.com/sparkfun/RFM69HCW_Breakout

// Include the RFM69 and SPI libraries:

#include <RFM69.h>
#include <SPI.h>
#include <DHT.h>
#include <avr/wdt.h>
#include "keys.h"

// Addresses for this node. CHANGE THESE FOR EACH NODE!

#define NETWORKID     0   // Must be the same for all nodes
#define MYNODEID      2   // My node ID
#define TONODEID      1   // Destination node ID

// RFM69 frequency, uncomment the frequency of your module:

//#define FREQUENCY   RF69_433MHZ
#define FREQUENCY     RF69_915MHZ

// AES encryption (or not):

#define ENCRYPT       true // Set to "true" to use encryption

// Use ACKnowledge when sending messages (or not):

#define USEACK        true // Request ACKs or not

// Create a library object for our RFM69HCW module:
RFM69 radio;

// Set the pin the DHT sensor is on
#define DHTPIN 4
// Set the type of DHT sensor
#define DHTTYPE DHT11

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Time holder for next update time
unsigned long next_updatetime_temp;
unsigned long next_updatetime_humid;
unsigned long next_updatetime_dist;
unsigned long next_updatetime_light;
unsigned long next_updatetime_motion;

// Update interval for the sensor information (in milliseconds)
#define UPDATE_INTERVAL 10000

// Set pin definitions for the ultrasonic sensor
#define TRIG_PIN 6
#define ECHO_PIN 5
// Timeout for sonar sensor (microseconds)
#define SONAR_SETUP_TIMEOUT 1000
#define SONAR_TIMEOUT 1000000
// Function prototype for getting time of flight
unsigned long long get_tof();

// Definitions for the light sensor
#define LIGHT_PIN A0
int get_light();

// Definitions for the motion sensor
#define MOTION_PIN 7
bool detect_motion();

void setup()
{
  wdt_enable(WDTO_8S);
  // Open a serial port so we can send keystrokes to the module:

  Serial.begin(115200);
  Serial.print("Node ");
  Serial.print(MYNODEID,DEC);
  Serial.println(" ready");  

  // Set up the distance sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Set up the motion sensor
  pinMode(MOTION_PIN, INPUT);

  // Initialize the RFM69HCW:

  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  //radio.setHighPower(); // Always use this for RFM69HCW
  radio.setPowerLevel(31);

  // Turn on encryption if desired:

  if (ENCRYPT)
    radio.encrypt(ENCRYPTKEY);

  // Initialize the DHT sensor
  dht.begin();

  //Initialize the last update time
  unsigned long now = millis();
  next_updatetime_temp = now;
  next_updatetime_humid = now+(UPDATE_INTERVAL/5);
  next_updatetime_dist = now +((UPDATE_INTERVAL*2)/5);
  next_updatetime_light = now+((UPDATE_INTERVAL*3)/5);
  next_updatetime_motion = now+((UPDATE_INTERVAL/5)*4);
}

void loop()
{
  // Set up a "buffer" for characters that we'll send:

  static char sendbuffer[62];
  static int sendlength = 0;

  // SENDING

  // In this section, we'll gather serial characters and
  // send them to the other node if we (1) get a carriage return,
  // or (2) the buffer is full (61 characters).

  // If there is any serial input, add it to the buffer:

  if (Serial.available() > 0)
  {
    char input = Serial.read();

    if (input != '\r') // not a carriage return
    {
      sendbuffer[sendlength] = input;
      sendlength++;
    }

    // If the input is a carriage return, or the buffer is full:

    if ((input == '\r') || (sendlength == 61)) // CR or buffer full
    {
      // Send the packet!


      Serial.print("sending to node ");
      Serial.print(TONODEID, DEC);
      Serial.print(", message [");
      for (byte i = 0; i < sendlength; i++)
        Serial.print(sendbuffer[i]);
      Serial.println("]");

      // There are two ways to send packets. If you want
      // acknowledgements, use sendWithRetry():

      if (USEACK)
      {
        if (radio.sendWithRetry(TONODEID, sendbuffer, sendlength))
          Serial.println("ACK received!");
        else
          Serial.println("no ACK received");
      }

      // If you don't need acknowledgements, just use send():

      else // don't use ACK
      {
        radio.send(TONODEID, sendbuffer, sendlength);
      }

      sendlength = 0; // reset the packet
    }
  }

  // RECEIVING

  // In this section, we'll check with the RFM69HCW to see
  // if it has received any packets:

  if (radio.receiveDone()) // Got one!
  {
    // Print out the information:

    Serial.print("received from node ");
    Serial.print(radio.SENDERID, DEC);
    Serial.print(", message [");

    // The actual message is contained in the DATA array,
    // and is DATALEN bytes in size:

    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);

    // RSSI is the "Receive Signal Strength Indicator",
    // smaller numbers mean higher power.

    Serial.print("], RSSI ");
    Serial.println(radio.RSSI);

    // Send an ACK if requested.
    // (You don't need this code if you're not using ACKs.)

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.println("ACK sent");
    }
  }

  unsigned long curr_time = millis();
  //Check if an update should be sent for the temperature
  if (curr_time > next_updatetime_temp) {
    Serial.println("Updating Temp");
    // Get the DHT sensor temperature data
    float f = dht.readTemperature(true);
    
    //Send the sensor data if it is valid
    if (!isnan(f)) {
      Serial.println("Sending packet");
      sendlength = sprintf(sendbuffer, "/temp|%d.%02d", (int)f, (int)(f*100)%100);
      if (USEACK) {
        if (radio.sendWithRetry(TONODEID, sendbuffer, sendlength))
          Serial.println("ACK received!");
        else
          Serial.println("no ACK received");
      }
      // If you don't need acknowledgements, just use send():
      else { // don't use ACK
         radio.send(TONODEID, sendbuffer, sendlength);
      }
    }
    next_updatetime_temp += UPDATE_INTERVAL;
    wdt_reset();
  }

  if (curr_time > next_updatetime_humid) {
    Serial.println("Updating Humidity");
    float h = dht.readHumidity();
    if (!isnan(h)) {
      Serial.println("Sending packet");
      sendlength = sprintf(sendbuffer, "/humid|%d.%02d", (int)h, (int)(h*100)%100);
      if (USEACK) {
        if (radio.sendWithRetry(TONODEID, sendbuffer, sendlength))
          Serial.println("ACK received!");
        else
          Serial.println("no ACK received");
      }
      // If you don't need acknowledgements, just use send():
      else { // don't use ACK
         radio.send(TONODEID, sendbuffer, sendlength);
      }
    }
    next_updatetime_humid += UPDATE_INTERVAL;
    wdt_reset();
  }

  //Check if an update should be sent for the distance
  if (curr_time > next_updatetime_dist) {
    Serial.println("Updating Dist");

    //Get a 3 sample average of the time of flight
    unsigned long long tof = get_tof();
    tof += get_tof();Serial.begin(115200);
    tof += get_tof();
    tof = tof/3;

    unsigned long dist = (13504*tof)/2000000;  //tof to inches
    
    //Send the sensor data if it is valid
    sendlength = sprintf(sendbuffer, "/tof|%lu", dist);

    Serial.println("Sending packet");
    if (USEACK) {
      if (radio.sendWithRetry(TONODEID, sendbuffer, sendlength))
        Serial.println("ACK received!");
      else
        Serial.println("no ACK received");
    }
    // If you don't need acknowledgements, just use send():
    else { // don't use ACK
       radio.send(TONODEID, sendbuffer, sendlength);
    }
    Serial.print("Distance: "); Serial.println((unsigned long) tof);
    next_updatetime_dist += UPDATE_INTERVAL;
    wdt_reset();
  }

  //Check if an update should be sent for the light
  if (curr_time > next_updatetime_light) {
    Serial.println("Updating Light");

    //Get a 3 sample average of the light value
    int light = get_light();
    light += get_light();
    light += get_light();
    light = light/3;
    
    //Send the sensor data if it is valid
    sendlength = sprintf(sendbuffer, "/light|%d", light);

    Serial.println("Sending packet");
    if (USEACK) {
      if (radio.sendWithRetry(TONODEID, sendbuffer, sendlength))
        Serial.println("ACK received!");
      else
        Serial.println("no ACK received");
    }
    // If you don't need acknowledgements, just use send():
    else { // don't use ACK
       radio.send(TONODEID, sendbuffer, sendlength);
    }
    next_updatetime_light += UPDATE_INTERVAL;
    wdt_reset();
  }

  //Check if an update should be sent for the motion sensor
  if (curr_time > next_updatetime_motion) {
    Serial.println("Updating Motion");

    // Send the respective state for motion
    if (detect_motion()) {
      sendlength = sprintf(sendbuffer, "/motion|Activity");
    } else {
      sendlength = sprintf(sendbuffer, "/motion|No_Activity");
    }

    Serial.println("Sending packet");
    if (USEACK) {
      if (radio.sendWithRetry(TONODEID, sendbuffer, sendlength))
        Serial.println("ACK received!");
      else
        Serial.println("no ACK received");
    }
    // If you don't need acknowledgements, just use send():
    else { // don't use ACK
       radio.send(TONODEID, sendbuffer, sendlength);
    }
    next_updatetime_motion += UPDATE_INTERVAL;
    wdt_reset();
  }
}

// Returns the time of flight of the distance in microseconds
unsigned long long get_tof() {
  unsigned long start_time;
  unsigned long stop_time;
  // Set the pins to a known state first
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);  //Allow time for propagation

  // Set the trigger pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  //Wait for the echo pin to go to high (should start low)
  unsigned long setup_start = micros();
  while(!digitalRead(ECHO_PIN)) {
    if ((micros()-setup_start) > SONAR_SETUP_TIMEOUT) {
      return SONAR_TIMEOUT;
    }
  }

  start_time = micros();
  //Time how long the pulse takes
  while(digitalRead(ECHO_PIN) && ((micros()-start_time) < SONAR_TIMEOUT)) {}

  //Compute the time of flight (in microseconds)
  stop_time = micros();
  return stop_time - start_time;
}

// Get the amount of light as a value between 0 and 1024
int get_light() {
  return analogRead(LIGHT_PIN);
}

// Check if motion sensor sees anything
bool detect_motion() {
  return digitalRead(MOTION_PIN);
}

