#define PINS 5
volatile bool states[PINS] = {false};
volatile unsigned int counters[PINS] = {0};
const unsigned int period = 250;
void setup()
{
  cli();
  for (int pin = 1; pin <= PINS; pin++) {
  	DDRB |= 1 << pin;
  }
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2B |= 1 << CS22;
  TIMSK2 |= 1 << TOIE2;
  sei();
}

ISR(TIMER2_OVF_vect) {
  uint8_t new_state = 0;
  for (int i = 0; i < PINS; i++){
  	counters[i]++;
    if (counters[i] > (i+1) * period) {
      counters[i] = 0;
      states[i] = !states[i];
    }
    new_state |= int(states[i]) << i;
  }
  PORTB = new_state;
}

void loop()
{
}