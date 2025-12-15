const int dataPin = 7;
const int latchPin = 6;      
const int clockPin = 5;
const bool common_anode = false;
volatile uint8_t sec = 0;

// 2 3 4 5 6 7 8 9 
// A B . C D E G F
bool digits[10][8] = {
  {1,1,0,1,1,1,0,1},  // 0
  {0,1,0,1,0,0,0,0},  // 1
  {1,1,0,0,1,1,1,0},  // 2
  {1,1,0,1,1,0,1,0},  // 3
  {0,1,0,1,0,0,1,1},  // 4
  {1,0,0,1,1,0,1,1},  // 5
  {1,0,1,1,1,1,1,1},  // 6
  {1,1,0,1,0,0,0,0},  // 7
  {1,1,0,1,1,1,1,1},  // 8
  {1,1,1,1,1,0,1,1}   // 9
};

void setup() 
{
  cli();
  if (common_anode == true)
    for (int digit = 0; digit < 10; digit++)
      for (int i = 0; i < 8; i++)
    	digits[digit][i] = !digits[digit][i];

  DDRD |= (1 << dataPin) | (1 << latchPin) | (1 << clockPin);
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);  
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  Serial.begin(9600);
  sei();
}

ISR(TIMER1_COMPA_vect) {
  sec %= 60;
  show_number();
  ++sec;
}

void show_number() {
  PORTD &= ~(1 << latchPin);
  for (int digit : (int[]){sec / 10, sec % 10})
  for (int i = 7; i >= 0; i--)
    shift_and_set(digits[digit][i]);
  PORTD |= 1 << latchPin;
}

void shift_and_set(bool val) {
  PORTD &= ~(1 << dataPin);
  PORTD |= (int)val << dataPin;
  PORTD |= 1 << clockPin;
  PORTD &= ~(1 << clockPin);
}

void update_starting_time() {
	if (Serial.available()) {
    	uint8_t new_time = Serial.parseInt();
      	if (new_time >= 0 && new_time < 60) {
      		cli();
      		sec = new_time;
      		sei();
        }
    }
}

void loop() 
{
  update_starting_time();
}
