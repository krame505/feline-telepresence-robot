#include <AStar32U4.h>
#include <PololuRPiSlave.h>
// Modified servo library
// Uses timer3 instead of timer1 to avoid conflict with Pololu motor controller
#include <ServoT3.h>

struct __attribute__((packed)) Data {
  bool led;
  uint16_t batteryMillivolts;
  
  bool playNotes;
  char notes[14];
  
  int16_t leftMotor, rightMotor;
  int32_t leftEncoder, rightEncoder;

  bool cameraServoCommand;
  uint8_t cameraPan, cameraTilt;
  bool laserServoCommand;
  uint8_t laserPan, laserTilt;
  bool laserPower;
  uint8_t laserPattern;

  bool dispenseTreats;
};

enum LaserPattern {
  STEADY,
  BLINK,
  RANDOM_WALK
};

PololuRPiSlave<struct Data, 5> slave;
PololuBuzzer buzzer;
AStar32U4Motors motors;

#define LASER 11
#define LASER_POWER 0.8
#define LASER_BLINK_PERIOD 200
#define LASER_BLINK_DUTY 0.7
#define LASER_WALK_MAX_STEP 1
#define LASER_WALK_PERIOD 10

void setup() {
  Serial.begin(115200);
  setup_encoders();

  motors.flipM1(true);
  
  pinMode(LASER, OUTPUT);
  
  // Set up the slave at I2C address 20.
  slave.init(20);

  // Play startup sound.
  buzzer.play("v10>>g16>>>c16");
}

void loop() {
  // Call updateBuffer() before using the buffer, to get the latest
  // data including recent master writes.
  slave.updateBuffer();

  // Write various values into the data structure.
  slave.buffer.batteryMillivolts = readBatteryMillivoltsLV();

  ledYellow(slave.buffer.led);

  // Playing music involves both reading and writing, since we only
  // want to do it once.
  static bool startedPlaying = false;
  
  if (slave.buffer.playNotes && !startedPlaying) {
    buzzer.play(slave.buffer.notes);
    startedPlaying = true;
  }
  else if (startedPlaying && !buzzer.isPlaying()) {
    slave.buffer.playNotes = false;
    startedPlaying = false;
  }
  
  motors.setSpeeds(slave.buffer.rightMotor, slave.buffer.leftMotor);

  slave.buffer.rightEncoder = getM1Counts();
  slave.buffer.leftEncoder = getM2Counts();
  
  handleCameraServos(slave.buffer.cameraServoCommand);
  cameraPan(slave.buffer.cameraPan);
  cameraTilt(slave.buffer.cameraTilt);

  static unsigned long lastWalkUpdate = millis();
  if (slave.buffer.laserPattern == RANDOM_WALK && millis() > lastWalkUpdate + LASER_WALK_PERIOD) {
    lastWalkUpdate = millis();
    slave.buffer.laserServoCommand = true;
    slave.buffer.laserPan += random(-LASER_WALK_MAX_STEP, LASER_WALK_MAX_STEP + 1);
    slave.buffer.laserTilt += random(-LASER_WALK_MAX_STEP, LASER_WALK_MAX_STEP + 1);
  }
  
  handleLaserServos(slave.buffer.laserServoCommand);
  laserPan(slave.buffer.laserPan);
  laserTilt(slave.buffer.laserTilt);
  
  bool blink_on = true;
  if (slave.buffer.laserPattern == BLINK) {
    blink_on = millis() % LASER_BLINK_PERIOD > LASER_BLINK_PERIOD * LASER_BLINK_DUTY;
  }
  analogWrite(LASER, slave.buffer.laserPower * 255 * LASER_POWER * blink_on);

  if (slave.buffer.dispenseTreats) {
    slave.buffer.dispenseTreats = false;
    dispenseTreats();
  }

  // When done WRITING, call finalizeWrites() to make modified
  // data available to I2C master.
  // READING the buffer is allowed before or after finalizeWrites().
  slave.finalizeWrites();
}
