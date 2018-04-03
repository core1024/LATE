#include "block.h"
static uint8_t blockSide, blockGap, blockWithGap;
static U8G2 *gr;

// Calculate block size and set a gap between blocks if there's enough space
void blocksSetup(U8G2 *sgr) {
  gr = sgr;
  blockGap = 0;
  blockSide = floor((gr->getHeight() - BLOCK_LINE) / 21);
  if (blockSide > 2) {
    blockSide--;
    blockGap++;
  }
  blockWithGap = blockSide + blockGap;
}

// Scale block coordinate (0..19) to actual screen position
uint8_t blockScale(int8_t i) {
  // Here we add one for the frame
  return blockWithGap * i + 1 + blockGap;
}

void blockDraw(int8_t x, int8_t y) {
  gr->drawBox(blockScale(x), blockScale(y) + BLOCK_LINE, blockSide, blockSide);
}

// Board frame 10x20
void blockDrawFrame(void) {
  gr->drawFrame(0, BLOCK_LINE, blockScale(10) + 1, blockScale(20) + 1);
}

