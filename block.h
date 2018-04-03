#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

#define BLOCK_LINE 6

void blocksSetup(U8G2 *gr);
uint8_t blockScale(int8_t i);
void blockDraw(int8_t x, int8_t y);
void blockDrawFrame(void);
