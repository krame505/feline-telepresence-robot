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
  int16_t leftSpeed, rightSpeed;

  bool cameraServoCommand;
  uint8_t cameraPan, cameraTilt;
  bool laserServoCommand;
  uint8_t laserPan, laserTilt;
  bool laserPower;
  uint8_t laserPattern;

  uint8_t dispenseTreatsCode;
};

enum LaserPattern {
  STEADY,
  BLINK,
  RANDOM_WALK
};

PololuRPiSlave<struct Data, 5> slave;
PololuBuzzer buzzer;
AStar32U4Motors motors;

#define MOTOR_UPDATE_PERIOD 150
#define MOTOR_KP 0.6
#define MOTOR_KI 0.10
#define MOTOR_MAX_I 7500

#define LASER 11
#define LASER_POWER 0.8
#define LASER_BLINK_PERIOD 200
#define LASER_BLINK_DUTY 0.7
#define LASER_WALK_MAX_STEP 1
#define LASER_WALK_PERIOD 10

void setup() {
  Serial.begin(115200);
  setup_encoders();

  motors.flipM2(true);
  
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

  slave.buffer.leftEncoder = getM1Counts();
  slave.buffer.rightEncoder = getM2Counts();
  
  static unsigned long nextMotorUpdate = 0;
  if (millis() > nextMotorUpdate) {
    nextMotorUpdate += MOTOR_UPDATE_PERIOD;

    static int32_t lastM1 = getM1Counts();
    static int32_t lastM2 = getM2Counts();
    int16_t m1Speed = getM1Counts() - lastM1;
    int16_t m2Speed = getM2Counts() - lastM2;
    lastM1 = getM1Counts();
    lastM2 = getM2Counts();
    slave.buffer.leftSpeed = m1Speed;
    slave.buffer.rightSpeed = m2Speed;

    int16_t m1TargetSpeed = slave.buffer.leftMotor;
    int16_t m2TargetSpeed = slave.buffer.rightMotor;
    static int32_t m1I = 0, m2I = 0;
    m1I += m1TargetSpeed - m1Speed;
    m2I += m2TargetSpeed - m2Speed;
    m1I = max(min(m1I, MOTOR_MAX_I), -MOTOR_MAX_I);
    m2I = max(min(m2I, MOTOR_MAX_I), -MOTOR_MAX_I);
    int16_t m1Power = MOTOR_KP * m1TargetSpeed + MOTOR_KI * m1I;
    int16_t m2Power = MOTOR_KP * m2TargetSpeed + MOTOR_KI * m2I;
    
    motors.setSpeeds(m2Power, m1Power);
  }
  
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

  if (slave.buffer.dispenseTreatsCode == 0xAA) {
    slave.buffer.dispenseTreatsCode = 0;
    dispenseTreats();
  }

  // When done WRITING, call finalizeWrites() to make modified
  // data available to I2C master.
  // READING the buffer is allowed before or after finalizeWrites().
  slave.finalizeWrites();
}
