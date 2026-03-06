#define DIGIT_ON_US  2000    // 2 ms per digit
#define DEBOUNCE_MS  25

const byte pDigit[4]   = {12, 11, 10,  9};
const byte pSegment[7] = { 2,  3,  4,  5,  6,  7,  8};

const byte numbers[10] = {
  B0111111, B0000110, B1011011, B1001111, B1100110,
  B1101101, B1111101, B0000111, B1111111, B1101111
};

const byte incPin   = A0;
const byte resetPin = A1;

int n = 0;
byte dig[4] = {0, 0, 0, 0};

// ---- Button (debounced edge detect) ----
struct DebouncedButton {
  byte pin;
  bool lastReading = HIGH;
  bool stableState = HIGH;
  unsigned long lastChangeMs = 0;

  void begin() const { pinMode(pin, INPUT_PULLUP); }

  // returns true exactly once when the button is pressed (HIGH->LOW)
  bool pressed(unsigned long nowMs) {
    bool reading = digitalRead(pin);

    if (reading != lastReading) {
      lastChangeMs = nowMs;
      lastReading = reading;
    }

    if ((nowMs - lastChangeMs) >= DEBOUNCE_MS && reading != stableState) {
      stableState = reading;
      if (stableState == LOW) return true;
    }
    return false;
  }
};

DebouncedButton btnInc  ;
DebouncedButton btnReset ;

// ---- Display helpers ----
inline void updateDigitsFromN() {
  dig[0] = (n / 1000) % 10;
  dig[1] = (n / 100)  % 10;
  dig[2] = (n / 10)   % 10;
  dig[3] =  n % 10;
}

inline void disableAllDigits() {
  for (byte i = 0; i < 4; i++) digitalWrite(pDigit[i], LOW);
}

inline void enableDigit(byte index0) {
  digitalWrite(pDigit[index0], HIGH);
}

inline void setSegmentsFromMask(byte mask) {
  for (byte s = 0; s < 7; s++) {
    digitalWrite(pSegment[s], (mask >> s) & 0x01);
  }
}

inline void setNumber(byte num) {
  setSegmentsFromMask(numbers[num]);
}

void setup() {
  btnInc.pin = incPin;
  btnReset.pin = resetPin;
  for (byte i = 2; i <= 13; i++) pinMode(i, OUTPUT);

  btnInc.begin();
  btnReset.begin();

  disableAllDigits();
  setNumber(0);
  updateDigitsFromN();
}

void loop() {
  unsigned long nowMs = millis();

  // Button actions (fast, every loop)
  if (btnInc.pressed(nowMs)) {
    n++;
    if (n > 9999) n = 0;
    updateDigitsFromN();
  }

  if (btnReset.pressed(nowMs)) {
    n = 0;
    updateDigitsFromN();
  }

  // Multiplex refresh (timed with micros)
  static unsigned long lastMuxUs = 0;
  static byte activeDigit = 0;

  unsigned long nowUs = micros();
  if (nowUs - lastMuxUs >= DIGIT_ON_US) {
    lastMuxUs = nowUs;

    activeDigit = (activeDigit + 1) & 0x03; // 0..3
    disableAllDigits();
    setNumber(dig[activeDigit]);
    enableDigit(activeDigit);
  }
}
