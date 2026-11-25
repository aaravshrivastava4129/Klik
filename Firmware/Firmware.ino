#include <Arduino.h>
#include <BleGamepad.h>

BleGamepad bleGamepad("KLIK Gamepad", "DIY", 100);
BleGamepadConfiguration config;

// D-PAD PINS
const int PIN_DPAD_UP    = 12;
const int PIN_DPAD_DOWN  = 13;
const int PIN_DPAD_LEFT  = 14;
const int PIN_DPAD_RIGHT = 27;

// FACE BUTTON PINS
const int PIN_BTN_A      = 26;
const int PIN_BTN_B      = 25;
const int PIN_BTN_X      = 21;
const int PIN_BTN_Y      = 16;

// SHOULDER PINS
const int PIN_BTN_L1     = 23;
const int PIN_BTN_L2     = 22;
const int PIN_BTN_R1     = 4;
const int PIN_BTN_R2     = 5;

// JOYSTICK PINS
const int PIN_LEFT_VRX   = 34;
const int PIN_LEFT_VRY   = 35;
const int PIN_LEFT_SW    = 18;

const int PIN_RIGHT_VRX  = 32;
const int PIN_RIGHT_VRY  = 33;
const int PIN_RIGHT_SW   = 19;

// JOYSTICK CALIBRATION
#define LEFT_VRX_CENTER   1790
#define LEFT_VRY_CENTER   1842
#define RIGHT_VRX_CENTER  1786
#define RIGHT_VRY_CENTER  1803
#define DEADZONE          800

static uint32_t lastReportMs = 0;
static const uint32_t REPORT_INTERVAL_MS = 10;


int16_t processAxis(int raw, int center, int deadzone) {
  int diff = raw - center;
  if (abs(diff) <= deadzone) {
    return 0;
  }
  if (diff > 0) {
    long val = map(raw, center + deadzone, 4095, 0, 32767);
    return (int16_t)constrain(val, 0, 32767);
  } else {
    long val = map(raw, 0, center - deadzone, -32767, 0);
    return (int16_t)constrain(val, -32767, 0);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_DPAD_UP, INPUT_PULLUP);
  pinMode(PIN_DPAD_DOWN, INPUT_PULLUP);
  pinMode(PIN_DPAD_LEFT, INPUT_PULLUP);
  pinMode(PIN_DPAD_RIGHT, INPUT_PULLUP);

  pinMode(PIN_BTN_A, INPUT_PULLUP);
  pinMode(PIN_BTN_B, INPUT_PULLUP);
  pinMode(PIN_BTN_X, INPUT_PULLUP);
  pinMode(PIN_BTN_Y, INPUT_PULLUP);

  pinMode(PIN_BTN_L1, INPUT_PULLUP);
  pinMode(PIN_BTN_L2, INPUT_PULLUP);
  pinMode(PIN_BTN_R1, INPUT_PULLUP);
  pinMode(PIN_BTN_R2, INPUT_PULLUP);

  pinMode(PIN_LEFT_SW, INPUT_PULLUP);
  pinMode(PIN_RIGHT_SW, INPUT_PULLUP);

  config.setGamepadMode(GamepadMode::XInputOneS);
  config.setAutoReport(false);
  
  bleGamepad.begin(&config);
  bleGamepad.setBatteryLevel(100);
}

void updateDpad() {
  bool up    = (digitalRead(PIN_DPAD_UP) == LOW);
  bool down  = (digitalRead(PIN_DPAD_DOWN) == LOW);
  bool left  = (digitalRead(PIN_DPAD_LEFT) == LOW);
  bool right = (digitalRead(PIN_DPAD_RIGHT) == LOW);

  if (up && right)        bleGamepad.setHat(HAT_UP_RIGHT);
  else if (up && left)    bleGamepad.setHat(HAT_UP_LEFT);
  else if (down && right) bleGamepad.setHat(HAT_DOWN_RIGHT);
  else if (down && left)  bleGamepad.setHat(HAT_DOWN_LEFT);
  else if (up)            bleGamepad.setHat(HAT_UP);
  else if (down)          bleGamepad.setHat(HAT_DOWN);
  else if (left)          bleGamepad.setHat(HAT_LEFT);
  else if (right)         bleGamepad.setHat(HAT_RIGHT);
  else                    bleGamepad.setHat(HAT_CENTERED);
}

void updateButtons() {
  
  digitalRead(PIN_BTN_A)    == LOW ? bleGamepad.press(1) : bleGamepad.release(1);
  digitalRead(PIN_BTN_B)    == LOW ? bleGamepad.press(2) : bleGamepad.release(2);
  digitalRead(PIN_BTN_X)    == LOW ? bleGamepad.press(3) : bleGamepad.release(3);
  digitalRead(PIN_BTN_Y)    == LOW ? bleGamepad.press(4) : bleGamepad.release(4);
  digitalRead(PIN_BTN_L1)   == LOW ? bleGamepad.press(5) : bleGamepad.release(5);
  digitalRead(PIN_BTN_R1)   == LOW ? bleGamepad.press(6) : bleGamepad.release(6);
  digitalRead(PIN_LEFT_SW)  == LOW ? bleGamepad.press(7) : bleGamepad.release(7);
  digitalRead(PIN_RIGHT_SW) == LOW ? bleGamepad.press(8) : bleGamepad.release(8);
}

void updateTriggers() {

  int16_t ltValue = (digitalRead(PIN_BTN_L2) == LOW) ? 32767 : 0;
  int16_t rtValue = (digitalRead(PIN_BTN_R2) == LOW) ? 32767 : 0;

  bleGamepad.setTriggers(ltValue, rtValue);
}

void updateJoysticks() {
  int16_t lx = processAxis(analogRead(PIN_LEFT_VRX),  LEFT_VRX_CENTER,  DEADZONE);
  int16_t ly = processAxis(analogRead(PIN_LEFT_VRY),  LEFT_VRY_CENTER,  DEADZONE);
  int16_t rx = -processAxis(analogRead(PIN_RIGHT_VRX), RIGHT_VRX_CENTER, DEADZONE);
  int16_t ry = -processAxis(analogRead(PIN_RIGHT_VRY), RIGHT_VRY_CENTER, DEADZONE);

  bleGamepad.setLeftThumb(lx, ly);
  bleGamepad.setRightThumb(rx, ry);
}

void loop() {
  if (!bleGamepad.isConnected()) {
    delay(100);
    return;
  }

  uint32_t now = millis();
  if (now - lastReportMs >= REPORT_INTERVAL_MS) {
    lastReportMs = now;

    updateDpad();
    updateButtons();
    updateTriggers();
    updateJoysticks();

    bleGamepad.sendReport();
  }
}
