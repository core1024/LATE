#include "buttons.h"

#define GET_BTN(pin, code) ((digitalRead(pin) == LOW) ? code : 0)
#define BTN_GO_DIR(dir) btnDir[(dir - btnRot) % 4]

#define BTN_GO_DIR_UP BTN_GO_DIR(4)
#define BTN_GO_DIR_RIGHT BTN_GO_DIR(5)
#define BTN_GO_DIR_DOWN BTN_GO_DIR(6)
#define BTN_GO_DIR_LEFT BTN_GO_DIR(7)


static uint8_t btnRot = 0;
static const uint8_t btnDir[] = {
  BTN_GO_UP,
  BTN_GO_RIGHT,
  BTN_GO_DOWN,
  BTN_GO_LEFT
};

static Arduboy2 *arduboy;

void buttonsSetup(Arduboy2 *ab) {
  arduboy = ab;
}

uint8_t buttonsUpdate(void) {
  static uint8_t btns_last = 0;
  static unsigned long btns_last_time = 0;
  unsigned long current_time = millis();
  uint8_t ret_btns = 0, btns = arduboy->buttonsState();

  // Simple debounce
  if (current_time > btns_last_time) {
    // Press or release
    if (btns_last != btns) {
      // First press
      if (!btns_last && btns) {
        btns_last_time = current_time + 300;
      }
      btns_last = btns;
    } else {
      btns_last_time = current_time + 60;
    }
    ret_btns = btns_last;
  } else {
    ret_btns = 0;
  }
  return ret_btns;
}
