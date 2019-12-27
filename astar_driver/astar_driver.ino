#include <AStar32U4.h>
#include <PololuRPiSlave.h>
// Modified servo library
// Uses timer3 instead of timer1 to avoid conflict with Pololu motor controller
#include <ServoT3.h>

struct __attribute__((packed)) Data {
  bool led;
  uint16_t batteryMillivolts;
  uint16_t analog[6];
  
  bool playNotes;
  char notes[14];
  
  int16_t leftMotor, rightMotor;
  int32_t leftEncoder, rightEncoder;

  uint8_t cameraPan, cameraTilt;
};

PololuRPiSlave<struct Data, 5> slave;
PololuBuzzer buzzer;
AStar32U4Motors motors;

#define MIN_PAN 40
#define MAX_PAN 180

#define CAMERA_PAN 5
#define CAMERA_TILT 7
Servo cameraPanServo, cameraTiltServo;

void setup() {
  Serial.begin(115200);
  setup_encoders();

  motors.flipM2(true);

  cameraPanServo.attach(CAMERA_PAN);
  cameraTiltServo.attach(CAMERA_TILT);
  
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

  for (uint8_t i=0; i<6; i++) {
    slave.buffer.analog[i] = analogRead(i);
  }

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
  
  motors.setSpeeds(slave.buffer.leftMotor, slave.buffer.rightMotor);

  slave.buffer.leftEncoder = getM1Counts();
  slave.buffer.rightEncoder = getM2Counts();

  slave.buffer.cameraPan = min(max(slave.buffer.cameraPan, MIN_PAN), MAX_PAN);
  slave.buffer.cameraTilt = min(slave.buffer.cameraTilt, 180);
  cameraPanServo.write(slave.buffer.cameraPan);
  cameraTiltServo.write(slave.buffer.cameraTilt);

  // When done WRITING, call finalizeWrites() to make modified
  // data available to I2C master.
  // READING the buffer is allowed before or after finalizeWrites().
  slave.finalizeWrites();
}
