volatile unsigned long timer2_millis = 0;
volatile uint8_t f = 0;
void setup() {
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  // 256 * 64 / 16 000 000 * 1000 = 1,024 ms|
  TCCR2B = TCCR2B | (1 << CS22); // <-------|
  TIMSK2 = TIMSK2 | (1 << TOIE2); 
  sei();
  Serial.begin(9600);
}

ISR(TIMER2_OVF_vect) {
  timer2_millis++;
  f++;
  if (f >= 41) {
    timer2_millis++;
    f -= 42;
  }
}

unsigned long t2_millis() {
  unsigned long m;
  cli();
  m = timer2_millis;
  sei();
  return m;
}
void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    if (command=='m') {
      Serial.print("T2 custom millis(): ");
      Serial.println(t2_millis());
      Serial.print("T0 arduino millis(): ");
      Serial.println(millis());
    }
  }
}
