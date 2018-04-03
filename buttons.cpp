#include "buttons.h"

#define GET_BTN(pin, code) ((digitalRead(pin) == LOW) ? code : 0)
#define BTN_GO_DIR(dir) btnDir[(dir - btnRot) % 4]

#define BTN_GO_DIR_UP BTN_GO_DIR(4)
#define BTN_GO_DIR_RIGHT BTN_GO_DIR(5)
#define BTN_GO_DIR_DOWN BTN_GO_DIR(6)
#define BTN_GO_DIR_LEFT BTN_GO_DIR(7)


static uint8_t btnRot = 0, btnPrev = 0, btnPinA, btnPinRight, btnPinLeft, btnPinUp, btnPinDown, btnPinB, btnPinC;
static const uint8_t btnDir[] = {
  BTN_GO_UP,
  BTN_GO_RIGHT,
  BTN_GO_DOWN,
  BTN_GO_LEFT
};

void buttonsSetup(const uint8_t setRot, const uint8_t pinA, const uint8_t pinRight, const uint8_t pinLeft, const uint8_t pinUp, const uint8_t pinDown, const uint8_t pinB, const uint8_t pinC) {
  btnRot = setRot;
  btnPinA = pinA;
  btnPinRight = pinRight;
  btnPinLeft = pinLeft;
  btnPinUp = pinUp;
  btnPinDown = pinDown;
  btnPinB = pinB;
  btnPinC = pinC;
  pinMode(btnPinA, INPUT_PULLUP);
  pinMode(btnPinRight, INPUT_PULLUP);
  pinMode(btnPinLeft, INPUT_PULLUP);
  pinMode(btnPinUp, INPUT_PULLUP);
  pinMode(btnPinDown, INPUT_PULLUP);
  pinMode(btnPinB, INPUT_PULLUP);
  pinMode(btnPinC, INPUT_PULLUP);
}

void buttonsRotate(uint8_t setRot) {
  btnRot = setRot;
}

uint8_t buttonsRead(void) {
  const uint8_t btnNow = GET_BTN(btnPinA, BTN_GO_A) |
         GET_BTN(btnPinUp, BTN_GO_DIR_UP) |
         GET_BTN(btnPinRight, BTN_GO_DIR_RIGHT) |
         GET_BTN(btnPinDown, BTN_GO_DIR_DOWN) |
         GET_BTN(btnPinLeft, BTN_GO_DIR_LEFT) |
         GET_BTN(btnPinB, BTN_GO_B) |
         GET_BTN(btnPinC, BTN_GO_C);
    return btnPrev == btnNow ? btnNow : btnNow & btnPrev ^ btnNow;
}

uint8_t buttonsUpdate(void) {
  static uint8_t btns_last = 0;
  static unsigned long btns_last_time = 0;
  unsigned long current_time = millis();
  uint8_t btns = buttonsRead();
  if (btns && (current_time > btns_last_time + 50 || btns_last != btns)) {
    if (btns_last != btns) {
      btns_last_time = current_time + 150;
    } else {
      btns_last_time = current_time;
    }
    btns_last = btns;
    return btns;
  }
  btns_last = btns;
  return 0;
}

