#include <AStar32U4.h>
#include <PololuRPiSlave.h>

struct __attribute__((packed)) Data {
  bool led;
  uint16_t batteryMillivolts;
  uint16_t analog[6];
  
  bool playNotes;
  char notes[14];
  
  int16_t leftMotor, rightMotor;
  int32_t leftEncoder, rightEncoder;

};

PololuRPiSlave<struct Data, 5> slave;
PololuBuzzer buzzer;
AStar32U4Motors motors;

void setup() {
  Serial.begin(115200);
  setup_encoders();

  motors.flipM2(true);
  
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
  motors.setSpeeds(slave.buffer.leftMotor, slave.buffer.rightMotor);

  slave.buffer.leftEncoder = getM1Counts();
  slave.buffer.rightEncoder = getM2Counts();
//  Serial.print(slave.buffer.leftEncoder);
//  Serial.print(' ');
//  Serial.print(slave.buffer.rightEncoder);
//  Serial.println();

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

  // When done WRITING, call finalizeWrites() to make modified
  // data available to I2C master.
  // READING the buffer is allowed before or after finalizeWrites().
  slave.finalizeWrites();
}
