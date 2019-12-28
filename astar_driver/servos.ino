
// Time for which to drive servos
#define COMMAND_TIME 250

// Angle limits
#define MIN_CAMERA_TILT 0
#define MAX_CAMERA_TILT 180
#define MIN_CAMERA_PAN 40
#define MAX_CAMERA_PAN 180

#define CAMERA_PAN 5
#define CAMERA_TILT 7
Servo cameraPanServo, cameraTiltServo;

void attachServos() {
  cameraPanServo.attach(CAMERA_PAN);
  cameraTiltServo.attach(CAMERA_TILT);
}

void detachServos() {
  cameraPanServo.detach();
  cameraTiltServo.detach();
}

void cameraPan(uint8_t &angle) {
  angle = min(max(angle, MIN_CAMERA_PAN), MAX_CAMERA_PAN);
  cameraPanServo.write(angle);
}

void cameraTilt(uint8_t &angle) {
  angle = min(max(angle, MIN_CAMERA_TILT), MAX_CAMERA_TILT);
  cameraTiltServo.write(angle);
}

void handleServos(bool &servoCommand) {
  static unsigned long servoCommandTime = millis();
  if (servoCommand) {
    servoCommand = false;
    servoCommandTime = millis();
    attachServos();
  } else if (millis() > servoCommandTime + COMMAND_TIME) {
    detachServos();
  }
}
