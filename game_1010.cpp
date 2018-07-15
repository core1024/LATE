#include "game_1010.h"

// Game related
#define BOARD_LINES 10
#define MIN_TAIL_LENGTH 4

static Arduboy2 *gr;

struct point_t {
  int8_t x;
  int8_t y;
};

struct data_t {
  uint8_t gameOn;
  uint32_t score;
  uint32_t hiScore;
  uint16_t screen[BOARD_LINES];
  uint8_t x;
  uint8_t y;
  int8_t current;
  int8_t next[3];
};

static struct data_t *data;

#define NUM_TILES 20
static const uint8_t tiles[NUM_TILES][5] = {
  // Test case all set
  {
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
  },
  // SQ1x1
  {
    0b00000,
    0b00000,
    0b00100,
    0b00000,
    0b00000
  },
  // SQ2x2
  {
    0b00000,
    0b00000,
    0b01100,
    0b01100,
    0b00000
  },
  // SQ3x3
  {
    0b00000,
    0b01110,
    0b01110,
    0b01110,
    0b00000
  },
  // D2H
  {
    0b00000,
    0b00000,
    0b01100,
    0b00000,
    0b00000
  },
  // D2V
  {
    0b00000,
    0b00000,
    0b01000,
    0b01000,
    0b00000
  },
  // D3H
  {
    0b00000,
    0b00000,
    0b01110,
    0b00000,
    0b00000
  },
  // D3V
  {
    0b00000,
    0b01000,
    0b01000,
    0b01000,
    0b00000
  },
  // D4H
  {
    0b00000,
    0b00000,
    0b01111,
    0b00000,
    0b00000
  },
  // D4V
  {
    0b00000,
    0b01000,
    0b01000,
    0b01000,
    0b01000
  },
  // D5H
  {
    0b00000,
    0b00000,
    0b11111,
    0b00000,
    0b00000
  },
  // D5V
  {
    0b01000,
    0b01000,
    0b01000,
    0b01000,
    0b01000
  },
  // L2R1
  {
    0b00000,
    0b00000,
    0b01100,
    0b01000,
    0b00000
  },
  // L2R2
  {
    0b00000,
    0b00000,
    0b00110,
    0b00010,
    0b00000
  },
  // L2R3
  {
    0b00000,
    0b00000,
    0b00010,
    0b00110,
    0b00000
  },
  // L2R4
  {
    0b00000,
    0b00000,
    0b01000,
    0b01100,
    0b00000
  },
  // L3R1
  {
    0b00000,
    0b01110,
    0b01000,
    0b01000,
    0b00000
  },
  // L3R2
  {
    0b00000,
    0b01110,
    0b00010,
    0b00010,
    0b00000
  },
  // L3R3
  {
    0b00000,
    0b00010,
    0b00010,
    0b01110,
    0b00000
  },
  // L3R3
  {
    0b00000,
    0b01000,
    0b01000,
    0b01110,
    0b00000
  }
};

static uint8_t get_pixel(int8_t ox, int8_t oy, int8_t x, int8_t y, uint8_t color) {
  return bitRead(data->screen[oy + y], ox + x) == color;
} // end of get_pixel

static uint8_t set_pixel(int8_t ox, int8_t oy, int8_t x, int8_t y, uint8_t color) {
  bitWrite(data->screen[oy + y], ox + x, color);
  return 0;
} // end of set_pixel


static uint8_t set_tile(int8_t x, int8_t y, uint8_t item[5], uint8_t color, uint8_t (*draw)(int8_t ox, int8_t oy, int8_t x, int8_t y, uint8_t color)) {
  for (uint8_t i = 0; i < 25; i++) {
    int8_t py = i / 5;
    int8_t px = i % 5;
    if (bitRead(item[py], px)) {
      if (draw(x, y, px, py, color)) {
        return 0;
      }
    }
  }
  return 1;
}

static uint8_t fit_tile(int8_t x, int8_t y, uint16_t item) {
  return set_tile(x, y, item, 1, get_pixel);
}

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

static void display_square(uint8_t x, uint8_t y, uint8_t w, uint8_t mask, uint8_t color /*, uint8_t fill*/) {
  // Top
  gr->drawFastHLine(x, y, w, color);
  // Left
  gr->drawFastVLine(x, y, w, color);
  // Bottom
  gr->drawFastHLine(x, y + w - 1, w, color);
  // Right
  gr->drawFastVLine(x + w - 1, y, w, color);

  //if(!fill || w < 3) return;
  for (int i = x; i < x + w; i+=2) {
  }
}

static void display_background(void) {
  gr->drawRect(0, 0, 63, 63, WHITE);
  gr->drawBitmap(64+29, 1, cupImg, 8, 8, WHITE);
}

static void display_board(void) {
  for (int x = 0; x < 10; x++) {
    int sx = x * 6 + 2;
    for (int y = 0; y < 10; y++) {
      if (get_pixel(0, 0, x, y, 1)) {
        int sy = y * 6 + 2;
        display_square(sx, sy, 5, 0, WHITE);
      }
    }
  }
}

static uint8_t display_next_square(int8_t ox, int8_t oy, int8_t x, int8_t y, uint8_t color) {
  display_square(ox + 4 * x, oy + 4 * y, 3, 0, 1);
  return 0;
}

static void display_next(void) {
  gr->fillRect(64, 43, 66, 22, BLACK);
  for (int i = 0; i < 3; i++) {
     set_tile(22 * i + 64, 43, tiles[data->next[i]], 1, display_next_square);
  }
}

static void game_new(void) {
  data->gameOn = 1;
  data->score = 0;
  data->current = -1;
  data->next[0] = 4;
  data->next[1] = 5;
  data->next[2] = 6;
}

static void game_on(void) {
  gr->clear();
  set_tile(-1, -1, tiles[2], 1, set_pixel);
  for (;;) {
    if (!gr->nextFrame()) {
      continue;
    }
    gr->pollButtons();

    if (gr->justPressed(B_BUTTON)) {
      return;
    }
    display_background();
    display_board();
    display_next();
    if (gr->everyXFrames(60)) {
      for (int i = 0; i < 3; i++) {
        data->next[i] = (data->next[i] + 1) % NUM_TILES;
      }
      gr->display();
      gr->idle();
    }
  }
}

void game1010(Arduboy2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore) {
  gr = sgr;
  data = (struct data_t *)gdat;

  if (!data->gameOn) {
    data->score = 0;
  }

  if (menu == MENU_NEW) {
    game_new();
  }

  if (menu != MENU_EXIT) {
    game_on();
  }

  *gameOn = data->gameOn;
  *score = data->score;
  *hiScore = data->hiScore;
}

