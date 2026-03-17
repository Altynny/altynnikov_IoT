#include <SoftwareSerial.h>
#define DIR_RIGHT 4
#define SPEED_RIGHT 5

#define DIR_LEFT 7
#define SPEED_LEFT 6

#define FORWARD_RIGHT LOW
#define FORWARD_LEFT HIGH
#define BACKWARD_RIGHT HIGH
#define BACKWARD_LEFT LOW

SoftwareSerial mySerial(2, 3);

void move(
  bool left_dir, int left_speed,
  bool right_dir, int right_speed
) {
  digitalWrite(DIR_LEFT, left_dir);
  digitalWrite(DIR_RIGHT, right_dir);
  analogWrite(SPEED_LEFT, left_speed);
  analogWrite(SPEED_RIGHT, right_speed);
}

void stop() {
  move(FORWARD_LEFT, 0, FORWARD_RIGHT, 0);
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
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  mySerial.begin(9600);
  stop();
}

void loop() {
  if (mySerial.available()) {
    char command = mySerial.read();
    Serial.write(command);
    switch (command) {
    case 'F': 
      forward(200);
      break;
    case 'B': 
      backward(200);
      break;
    case 'R': 
      rotate_right(200);
      break;
    case 'L': 
      rotate_left(200);
      break;
    case '0': 
      stop();
      break;
    }
  }
}
