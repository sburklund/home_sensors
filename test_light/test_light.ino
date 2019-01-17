#define LIGHT_PIN A0

int get_light();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Light: ");
  Serial.println(get_light());
  delay(200);
}

int get_light() {
  return analogRead(LIGHT_PIN);
}

