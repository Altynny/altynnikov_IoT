const int lightSensorPin = A0;
const char GET_VALUE = 'p';
const char STREAM_MODE = 's';
bool stream_state = false;

void setup() {
  Serial.begin(9600);
}

void UpdateState(int lightValue) {
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case GET_VALUE:
        stream_state = false;
        Serial.print("SENSOR_VALUE:");
        Serial.println(lightValue);
        break;
      case STREAM_MODE:
        stream_state = true;
        Serial.println("STREAM_STARTED");
        break;
    }
  }
}

void loop() {
  int lightValue = analogRead(lightSensorPin);
  UpdateState(lightValue);
  if (stream_state) {
    Serial.print("SENSOR_VALUE:");
    Serial.println(lightValue);
  }
}