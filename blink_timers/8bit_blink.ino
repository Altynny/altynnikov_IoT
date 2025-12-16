volatile bool state = false;

void setup() {
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2B = TCCR2B | ((1 << CS22) | (1 << CS21) | (1 << CS20));
  TIMSK2 = TIMSK2 | (1 << TOIE2);
  pinMode(LED_BUILTIN, OUTPUT);
  sei();
}

ISR(TIMER2_OVF_vect) {
  state = !state; 
}

void loop() {
    digitalWrite(LED_BUILTIN, state); 
}