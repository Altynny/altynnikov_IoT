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
  digitalWrite(SPEED_LEFT, left_speed);
  digitalWrite(SPEED_RIGHT, right_speed);
}

void forward(int speed) {
  move(FORWARD_LEFT, speed, FORWARD_RIGHT, speed);
}

void backward(int speed) {
  move(BACKWARD_LEFT, speed, BACKWARD_RIGHT, speed);
}

void turn_left(int steepness) {

}

void turn_right(int steepness) {
  
}

void rotate_left(int steepness) {

}

void rotate_right(int steepness) {
  
}

void setup() {
  pinMode(DIR_RIGHT, OUTPUT);
  pinMode(SPEED_RIGHT, OUTPUT);
  pinMode(DIR_LEFT, OUTPUT);
  pinMode(SPEED_LEFT, OUTPUT);

  move(FORWARD_LEFT, 1, FORWARD_RIGHT, 0);
  delay(2000);
  move(FORWARD_LEFT, 0, FORWARD_RIGHT, 1);
  delay(2000);
  move(BACKWARD_LEFT, 1, BACKWARD_RIGHT, 1);
}

void loop() {
}
