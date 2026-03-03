#define DIR_RIGHT 4
#define SPEED_RIGHT 5

#define DIR_LEFT 6
#define SPEED_LEFT 7

void setup() {
  pinMode(DIR_RIGHT, OUTPUT);
  pinMode(SPEED_RIGHT, OUTPUT);
  pinMode(DIR_LEFT, OUTPUT);
  pinMode(SPEED_LEFT, OUTPUT);
}

void loop() {
  digitalWrite(DIR_RIGHT, LOW);
  digitalWrite(SPEED_RIGHT, LOW);
  digitalWrite(DIR_LEFT, HIGH);
  digitalWrite(SPEED_LEFT, HIGH);
}
