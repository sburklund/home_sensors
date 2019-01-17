#define TRIG_PIN 6
#define ECHO_PIN 5

unsigned long get_tof();

void setup() {
  // put your setup code here, to run once:
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long tof = get_tof();
  Serial.print("Time of flight: ");
  Serial.println(tof);
  delay(100);
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

