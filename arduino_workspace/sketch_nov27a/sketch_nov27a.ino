void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

const int analogInPin = A0;
int sensorValue = 0;

void loop() {
  // put your main code here, to run repeatedly:
  sensorValue = analogRead(analogInPin);

  Serial.print("sensor = ");
  Serial.print(sensorValue);
  Serial.println(".");

}
