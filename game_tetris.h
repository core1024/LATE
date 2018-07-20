#pragma once

#include <Arduboy2.h>
#include "menu.h"

void gameTetris(Arduboy2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore);
