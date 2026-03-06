#define DIGIT_ON_US     2000   // 2 ms per digit => ~125 Hz full refresh (4 digits)
#define DEBOUNCE_MS     25     // button debounce time

const byte pDigit[4]   = {12, 11, 10,  9};   // Digits 1 - 4
const byte pSegment[7] = { 2,  3,  4,  5,  6,  7,  8}; // Segments A - G

const byte numbers[10] = {
  B0111111, B0000110, B1011011, B1001111, B1100110,
  B1101101, B1111101, B0000111, B1111111, B1101111
};

const byte buttonPin = A0;
const byte resetPin = A1;

int n = 0;
byte dig[4] = {0, 0, 0, 0};

static inline void updateDigitsFromN() {
  dig[0] = (n / 1000) % 10;
  dig[1] = (n / 100)  % 10;
  dig[2] = (n / 10)   % 10;
  dig[3] =  n % 10;
}

void setup() {
  for (byte i = 2; i <= 13; i++) pinMode(i, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // button to GND (pressed = LOW)
  pinMode(resetPin, INPUT_PULLUP);
  disableAllDigits();
  setSegments(0);

  updateDigitsFromN();
}

void loop() {
// ---------- Increment Button ----------
  static bool lastReading = HIGH;
  static bool stableState = HIGH;
  static unsigned long lastChangeMs = 0;

  bool reading = digitalRead(buttonPin);
  unsigned long nowMs = millis();

  if (reading != lastReading) {
    lastChangeMs = nowMs;      // reset debounce timer
    lastReading = reading;
  }

  // if reading has been stable for DEBOUNCE_MS, accept it
  if ((nowMs - lastChangeMs) >= DEBOUNCE_MS && reading != stableState) {
    stableState = reading;

    // detect press (HIGH -> LOW with INPUT_PULLUP)
    if (stableState == LOW) {
      n++;
      if (n > 9999) n = 0;
      updateDigitsFromN();
    }
  }
  // ---------- Reset Button ----------
static bool lastReadingReset = HIGH;
static bool stableReset = HIGH;
static unsigned long lastChangeReset = 0;

bool readingReset = digitalRead(resetPin);

if (readingReset != lastReadingReset) {
  lastChangeReset = nowMs;
  lastReadingReset = readingReset;
}

if ((nowMs - lastChangeReset) >= DEBOUNCE_MS && readingReset != stableReset) {
  stableReset = readingReset;

  if (stableReset == LOW) {   // reset pressed
    n = 0;
    updateDigitsFromN();
  }
}

  // ---------- 2) Multiplex refresh (smooth, no delay) ----------
  static unsigned long lastMuxUs = 0;
  static byte activeDigit = 0;

  unsigned long nowUs = micros();
  if (nowUs - lastMuxUs >= DIGIT_ON_US) {
    lastMuxUs = nowUs;

    activeDigit = (activeDigit + 1) & 0x03;

    disableAllDigits();
    setSegments(dig[activeDigit]);
    enableDigit(activeDigit);
  }
}

inline void disableAllDigits() {
  for (byte i = 0; i < 4; i++) digitalWrite(pDigit[i], LOW);
}

inline void enableDigit(byte index0) {        // index0: 0..3
  digitalWrite(pDigit[index0], HIGH);
}

inline void setSegments(byte num) {
  byte mask = numbers[num];
  for (byte s = 0; s < 7; s++) {
    digitalWrite(pSegment[s], bitRead(mask, s));
  }
}
