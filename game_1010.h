#pragma once

#include <Arduboy2.h>
#include "buttons.h"

extern Arduboy2 arduboy;

void game1010(Arduboy2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore);

const uint8_t numImg[][3] PROGMEM = {
  {0b11111, 0b10001, 0b11111},
  {0b10010, 0b11111, 0b10000},
  {0b11101, 0b10101, 0b10111},
  {0b10001, 0b10101, 0b11111},
  {0b00111, 0b00100, 0b11111},
  {0b10111, 0b10101, 0b11101},
  {0b11111, 0b10101, 0b11101},
  {0b00001, 0b11101, 0b00111},
  {0b11111, 0b10101, 0b11111},
  {0b10111, 0b10101, 0b11111}
};

const uint8_t cupImg[] PROGMEM = {
	0b00000110,
	0b01001001,
	0b01011111,
	0b01111111,
	0b01111111,
	0b01011111,
	0b01001001,
	0b00000110
};
