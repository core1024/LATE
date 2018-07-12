#pragma once
#include <Arduboy2.h>

#define BLOCK_LINE 0

void blocksSetup(Arduboy2 *gr);
uint8_t blockScale(int8_t i);
void blockDraw(int8_t x, int8_t y);
void blockDrawFrame(void);
