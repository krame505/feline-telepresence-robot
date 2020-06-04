
// Time for which to drive servos
#define COMMAND_TIME 250

// Angle limits
#define MIN_CAMERA_TILT 0
#define MAX_CAMERA_TILT 180
#define MIN_CAMERA_PAN 25
#define MAX_CAMERA_PAN 180

#define MIN_LASER_TILT 100
#define MAX_LASER_TILT 180
#define MIN_LASER_PAN 0
#define MAX_LASER_PAN 160

#define DISPENSE_IN 5
#define DISPENSE_OUT 100

// Servo defs
#define CAMERA_PAN 5
#define CAMERA_TILT 7
#define LASER_PAN 8
#define LASER_TILT 20
#define DISPENSE 22
Servo cameraPanServo, cameraTiltServo;
Servo laserPanServo, laserTiltServo;
Servo dispenseServo;

void attachCameraServos() {
  cameraPanServo.attach(CAMERA_PAN);
  cameraTiltServo.attach(CAMERA_TILT);
}

void detachCameraServos() {
  cameraPanServo.detach();
  cameraTiltServo.detach();
  digitalWrite(CAMERA_PAN, LOW);
  digitalWrite(CAMERA_TILT, LOW);
}

void attachLaserServos() {
  laserPanServo.attach(LASER_PAN);
  laserTiltServo.attach(LASER_TILT);
}

void detachLaserServos() {
  laserPanServo.detach();
  laserTiltServo.detach();
  digitalWrite(LASER_PAN, LOW);
  digitalWrite(LASER_TILT, LOW);
}

void cameraPan(uint8_t &angle) {
  angle = min(max(angle, MIN_CAMERA_PAN), MAX_CAMERA_PAN);
  cameraPanServo.write(angle);
}

void cameraTilt(uint8_t &angle) {
  angle = min(max(angle, MIN_CAMERA_TILT), MAX_CAMERA_TILT);
  cameraTiltServo.write(angle);
}

void laserPan(uint8_t &angle) {
  angle = min(max(angle, MIN_LASER_PAN), MAX_LASER_PAN);
  laserPanServo.write(angle);
}

void laserTilt(uint8_t &angle) {
  angle = min(max(angle, MIN_LASER_TILT), MAX_LASER_TILT);
  laserTiltServo.write(angle);
}

void handleCameraServos(bool &servoCommand) {
  static unsigned long servoCommandTime = millis();
  if (servoCommand) {
    servoCommand = false;
    servoCommandTime = millis();
    attachCameraServos();
  } else if (millis() > servoCommandTime + COMMAND_TIME) {
    detachCameraServos();
  }
}

void handleLaserServos(bool &servoCommand) {
  static unsigned long servoCommandTime = millis();
  if (servoCommand) {
    servoCommand = false;
    servoCommandTime = millis();
    attachLaserServos();
  } else if (millis() > servoCommandTime + COMMAND_TIME) {
    detachLaserServos();
  }
}

void dispenseTreats() {
  dispenseServo.attach(DISPENSE);
  dispenseServo.write(DISPENSE_IN);
  delay(500);
  dispenseServo.write(DISPENSE_OUT);
  delay(500);
  dispenseServo.detach();
}
