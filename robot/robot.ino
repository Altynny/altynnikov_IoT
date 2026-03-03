#define DIR_RIGHT 4
#define SPEED_RIGHT 5

#define DIR_LEFT 7
#define SPEED_LEFT 6

#define FORWARD_RIGHT LOW
#define FORWARD_LEFT HIGH
#define BACKWARD_RIGHT HIGH
#define BACKWARD_LEFT LOW

void move(
  bool left_dir, int left_speed,
  bool right_dir, int right_speed
) {
  digitalWrite(DIR_LEFT, left_dir);
  digitalWrite(DIR_RIGHT, right_dir);
  analogWrite(SPEED_LEFT, left_speed);
  analogWrite(SPEED_RIGHT, right_speed*2);
}

void forward(int speed) {
  move(FORWARD_LEFT, speed, FORWARD_RIGHT, speed);
}

void backward(int speed) {
  move(BACKWARD_LEFT, speed, BACKWARD_RIGHT, speed);
}

void turn_left(int steepness) {
  // TODO: calculate steep
}

void turn_right(int steepness) {
  // TODO: calculate steep
}

void rotate_left(int speed) {
  move(FORWARD_LEFT, speed, BACKWARD_RIGHT, speed);
}

void rotate_right(int speed) {
  move(BACKWARD_LEFT, speed, FORWARD_RIGHT, speed);
}

void setup() {
  pinMode(DIR_RIGHT, OUTPUT);
  pinMode(SPEED_RIGHT, OUTPUT);
  pinMode(DIR_LEFT, OUTPUT);
  pinMode(SPEED_LEFT, OUTPUT);

  rotate_left(120);
  delay(2000);
  rotate_right(120);
  delay(2000);
  forward(100);
  delay(1000);
  backward(100);
  delay(1000);
  forward(0);
}

void loop() {
}
