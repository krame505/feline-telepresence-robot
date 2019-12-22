#include <AStar32U4.h>
#include <PololuRPiSlave.h>

struct Data {
  bool yellow, green, red;
  bool buttonA, buttonB, buttonC;

  int16_t leftMotor, rightMotor;
  uint16_t batteryMillivolts;
  uint16_t analog[6];

  bool playNotes;
  char notes[14];
  
  int32_t leftEncoder, rightEncoder;
};

PololuRPiSlave<struct Data, 5> slave;
PololuBuzzer buzzer;
AStar32U4Motors motors;
AStar32U4ButtonA buttonA;
AStar32U4ButtonB buttonB;
AStar32U4ButtonC buttonC;

void setup() {
  Serial.begin(115200);
  setup_encoders();

  motors.flipM2(true);

  // Encoder power
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
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
  slave.buffer.buttonA = buttonA.isPressed();
  slave.buffer.buttonB = buttonB.isPressed();
  slave.buffer.buttonC = buttonC.isPressed();

  slave.buffer.batteryMillivolts = readBatteryMillivoltsLV();

  for (uint8_t i=0; i<6; i++) {
    slave.buffer.analog[i] = analogRead(i);
  }

  ledYellow(slave.buffer.yellow);
  ledGreen(slave.buffer.green);
  ledRed(slave.buffer.red);
  motors.setSpeeds(slave.buffer.leftMotor, slave.buffer.rightMotor);

  slave.buffer.leftEncoder = getM1Counts();
  slave.buffer.rightEncoder = getM2Counts();

  // Playing music involves both reading and writing, since we only
  // want to do it once.
  static bool startedPlaying = false;
  
  if(slave.buffer.playNotes && !startedPlaying) {
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
