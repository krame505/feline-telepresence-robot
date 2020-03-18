
#define M1_INT_1 15
#define M1_INT_2 14
#define M2_INT_1 16
#define M2_INT_2 17

static volatile long M1_counts = 0;
static volatile long M2_counts = 0;

inline void handle_encoder(
    int pin1, int pin2,
    bool &prev1, bool &prev2,
    volatile long &counts) {
  bool curr1 = digitalRead(pin1);
  bool curr2 = digitalRead(pin2);

  bool plus  = curr1 ^ prev2;
  bool minus = curr2 ^ prev1;

  prev1 = curr1;
  prev2 = curr2;

  counts += plus - minus;
}

// Handle pin change interrupt for D8 to D13
ISR(PCINT0_vect) {
  static bool M1_prev1 = false;
  static bool M1_prev2 = false;
  static bool M2_prev1 = false;
  static bool M2_prev2 = false;
  handle_encoder(M1_INT_1, M1_INT_2, M1_prev1, M1_prev2, M1_counts);
  handle_encoder(M2_INT_1, M2_INT_2, M2_prev1, M2_prev2, M2_counts);
}

void setup_encoders() {
  pinMode(M1_INT_1, INPUT);
  pinMode(M1_INT_2, INPUT);
  pinMode(M2_INT_1, INPUT);
  pinMode(M2_INT_2, INPUT);
  PCICR = (1 << PCIE0);
  PCMSK0 |= (1 << PCINT3) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT0);
}

inline long getM1Counts() {
  return M1_counts;
}

inline long getM2Counts() {
  return M2_counts;
}
