const int led_pin = 13;
const char LED_ON = 'u';
const char LED_OFF = 'd';

char LED_state = 'd';

void setup() {
  Serial.begin(9600);
  pinMode(led_pin, OUTPUT);
}

void updateState() {
   if(Serial.available() > 0) {
    LED_state = Serial.read();
    if(LED_state==LED_ON) {
      Serial.println("LED_GOES_ON");
    } else if(LED_state==LED_OFF) {
      Serial.println("LED_GOES_OFF");
    }
  }
}

void handleLED() {
   if(LED_state==LED_ON) {
      digitalWrite(led_pin, HIGH);
    } else if(LED_state==LED_OFF) {
      digitalWrite(led_pin, LOW);
    }
}

void loop() {
   updateState();
   handleLED();
}
