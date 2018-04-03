#pragma once

#include <U8g2lib.h>
#include "buttons.h"
#include "block.h"

void gameBGun(U8G2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore);
