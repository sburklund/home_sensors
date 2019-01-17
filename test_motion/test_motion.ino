#define MOTION_PIN 7

bool detect_motion();

void setup() {
  // put your setup code here, to run once:
  pinMode(MOTION_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, detect_motion());
  Serial.println(digitalRead(MOTION_PIN));
}

bool detect_motion() {
  return digitalRead(MOTION_PIN);
}

