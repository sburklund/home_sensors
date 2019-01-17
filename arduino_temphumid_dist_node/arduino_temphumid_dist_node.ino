// RFM69HCW Example Sketch
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

// Addresses for this node. CHANGE THESE FOR EACH NODE!

#define NETWORKID     0   // Must be the same for all nodes
#define MYNODEID      2   // My node ID
#define TONODEID      1   // Destination node ID

// RFM69 frequency, uncomment the frequency of your module:

//#define FREQUENCY   RF69_433MHZ
#define FREQUENCY     RF69_915MHZ

// AES encryption (or not):

#define ENCRYPT       true // Set to "true" to use encryption
#define ENCRYPTKEY    "TOPSECRETPASSWRD" // Use the same 16-byte key on all nodes

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

// Update interval for the sensor information (in milliseconds)
#define UPDATE_INTERVAL 10000

// Set pin definitions for the ultrasonic sensor
#define TRIG_PIN 6
#define ECHO_PIN 5
// Function prototype for getting time of flight
unsigned long get_tof();

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
  next_updatetime_temp = millis();
  next_updatetime_humid = millis()+(UPDATE_INTERVAL/3);
  next_updatetime_dist = millis() + ((UPDATE_INTERVAL*2)/3);
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
    unsigned int tof = get_tof();
    tof += get_tof();
    tof += get_tof();
    tof = tof/3;
    
    //Send the sensor data if it is valid
    sendlength = sprintf(sendbuffer, "/tof|%d", tof);

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
    next_updatetime_dist += UPDATE_INTERVAL;
    wdt_reset();
  }
}

// Returns the time of flight of the distance in microseconds
unsigned long get_tof() {
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
  while(!digitalRead(ECHO_PIN)) {}

  start_time = micros();
  //Time how long the pulse takes
  while(digitalRead(ECHO_PIN)) {}

  //Compute the time of flight (in microseconds)
  stop_time = micros();
  return stop_time - start_time;
}

