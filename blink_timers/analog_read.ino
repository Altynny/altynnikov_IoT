const int sensorPin = A0;

void setup() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15625;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  Serial.begin(9600);
  sei();
}

ISR(TIMER1_COMPA_vect) {
  Serial.print("Value: ");
  Serial.println(analogRead(sensorPin));
  Serial.print("Time:  ");
  Serial.println(millis());
  Serial.println();
}

void loop() {
}
