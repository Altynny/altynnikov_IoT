volatile bool state = false;
const char TIMER_MODE = 'T';
const char MILLIS_MODE = 'M';
const char DELAY_MODE = 'D';
char mode = TIMER_MODE;
unsigned long start = 0;

void setup() {
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2B = TCCR2B | ((1 << CS22) | (1 << CS21) | (1 << CS20));
  TIMSK2 = TIMSK2 | (1 << TOIE2);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  sei();
}

ISR(TIMER2_OVF_vect) {
  if (mode == TIMER_MODE)
  	state = !state; 
}

void updateMode() {
   if (Serial.available() > 0) {
    	char new_mode = Serial.read();
     	switch (new_mode) {
     		case TIMER_MODE:
          		mode = new_mode;
          		Serial.println("LED goes TIMER_MODE");
          		break;
          	case MILLIS_MODE:
		        mode = new_mode;
          		Serial.println("LED goes MILLIS_MODE");
          		start = millis();
          		break;
          	case DELAY_MODE:
          		mode = new_mode;
          		Serial.println("LED goes DELAY_MODE");
          		break;
        } 
  }
}

void handleLED() {
  if (mode == MILLIS_MODE) {
    unsigned long end = millis();
    if (end - start >= 250) {
      state = !state;
      start = end;
    }
  } 
  else if (mode == DELAY_MODE) {
     delay(50);
     state = !state;
  }
}

void loop() {
  	updateMode();
  	handleLED();
  	digitalWrite(LED_BUILTIN, state);
}