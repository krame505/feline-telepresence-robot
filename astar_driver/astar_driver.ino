#include <AStar32U4.h>

void setup() {
  Serial.begin(115200);
  setup_encoders();

  // Encoder power
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  
}

void loop() {
  Serial.print(getM1Counts());
  Serial.print(" ");
  Serial.print(getM2Counts());
  Serial.println();
}
