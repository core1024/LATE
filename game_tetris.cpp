#include "game_tetris.h"

#define NEXT_TET_X 4
#define NEXT_TET_Y 0
#define BOARD_LINES 24

static Arduboy2 *gr;

// Game state data
struct data_t {
  uint8_t gameOn;
  uint32_t score;
  uint32_t hiScore;
  uint16_t screen[BOARD_LINES];
  uint8_t level;
  uint8_t lines;
  uint8_t bag[7];
  uint8_t bag_pos;
  uint8_t tet_now_sel;
  uint8_t tet_now_rot;
  uint8_t tet_next_sel;
  uint8_t tet_next_rot;
  uint8_t tx;
  uint8_t ty;
};

static struct data_t *data;

static int8_t dx, dy, dr, stopDrop;

static unsigned long button_wait_time;

static const uint16_t tetrominoes[7][4] = {
  // L
  {17504, 3712, 50240, 11776},
  // J
  {17600, 36352, 25664, 3616},
  // T
  {3648, 19520, 19968, 17984},
  // Z
  {3168, 9792, 3168, 9792},
  // S
  {1728, 17952, 1728, 17952},
  // I
  {17476, 3840, 17476, 3840},
  // O
  {26112, 26112, 26112, 26112}
};

static uint8_t get_pixel(uint8_t x, uint8_t y) {
  return bitRead(data->screen[y], x);
} // end of get_pixel

static void set_pixel(uint8_t x, uint8_t y, uint8_t color) {
  bitWrite(data->screen[y], x, color);
} // end of set_pixel

////////////////////////////////////////////////////
static void shuffle(uint8_t a[7]) {
  uint8_t j, x, i;
  for (i = 7; i; i--) {
    j = floor(random(i));
    x = a[i - 1];
    a[i - 1] = a[j];
    a[j] = x;
  }
}

static uint8_t draw_tetromino(int8_t x, int8_t y, uint16_t item, uint8_t color) {
  for (uint8_t i = 0; i < 16; i++) {
    int8_t py = 1 + (i / 4) + y;
    int8_t px = 1 + (i % 4) + x;
    if (py < 0) continue;
    if (bitRead(item, i)) {
      if (color == 2) {
        if (get_pixel(px, py)) {
          return 0;
        }
      } else {
        set_pixel(px, py, color);
      }
    }
  }
  return 1;
}

static uint8_t fit_tetromino(int8_t x, int8_t y, uint16_t item) {
  return draw_tetromino(x, y, item, 2);
}

static void remove_lines() {
  uint8_t lines_removed = 0;
  const uint16_t lines_points[5] = {0, 40, 100, 300, 1200};
  for (uint8_t i = 2; i < 23; i++) {
    if (data->screen[i] != 0b111111111111) continue;
    lines_removed++;
    for (uint8_t j = i; j > 0; j--) {
      data->screen[j] = data->screen[j - 1];
    }
  }
  data->score += lines_points[lines_removed] * (data->level + 1);
  if (data->score > data->hiScore) {
    data->hiScore = data->score;
  }
  data->lines += lines_removed;
  if (data->lines > (data->level + 1) * 10) {
    data->lines = 0;
    data->level++;
  }
}

static void reset_board() {
  for (uint8_t i = 0; i < BOARD_LINES; i++) {
    data->screen[i] = 0b100000000001;
  }
  data->screen[BOARD_LINES - 1] = ~0;
}

static void next_tetromino() {
  data->bag_pos = (data->bag_pos + 1) % 7;
  if (!data->bag_pos) {
    shuffle(data->bag);
  }

  data->tet_now_sel = data->tet_next_sel;
  data->tet_now_rot = dr = data->tet_next_rot;
  data->tet_next_sel = data->bag[data->bag_pos];
  data->tet_next_rot = random(4);
}

static void display_board() {
  char strnum[12];
  gr->clear();
  // gr->paint8Pixels((const uint8_t)(tetrominoes[data->tet_next_sel][data->tet_next_rot] & 0b1111));
  // gr->paint8Pixels((const uint8_t)((tetrominoes[data->tet_next_sel][data->tet_next_rot] >> 4) & 0b1111));
  // gr->paint8Pixels((const uint8_t)((tetrominoes[data->tet_next_sel][data->tet_next_rot] >> 8) & 0b1111));
  // gr->paint8Pixels((const uint8_t)((tetrominoes[data->tet_next_sel][data->tet_next_rot] >> 12) & 0b1111));

  //Score rounded rect
  gr->drawRoundRect(63, 0, 46, 13, 3);
  gr->fillRoundRect(65, 2, 42, 9, 1);
  gr->drawFastHLine(44, 7, 19);
  gr->drawFastHLine(109, 7, 19);

  // The score square
  gr->drawFastHLine(44, 15, 84);
  gr->fillRect(44, 17, 84, 9);
  gr->drawFastHLine(44, 27, 84);

  gr->setTextBackground(WHITE);
  gr->setTextColor(BLACK);
  
  //snprintf(strnum, 11, "%10ull", (unsigned long long)data->score);
  gr->setCursor(72, 3);
  gr->print(F("SCORE"));

  gr->setCursor(66, 18);
  gr->print(data->score);

  //Level rounded rect
  gr->drawRoundRect(66, 34, 62, 13, 3);
  gr->fillRoundRect(68, 36, 58, 9, 1);

  snprintf(strnum, 11, "%4u", data->level);
  gr->setCursor(70, 37);
  gr->print(F("LEVEL"));
  gr->setCursor(102, 37);
  gr->print(strnum);

  // Lines rounded rect
  gr->drawRoundRect(66, 51, 62, 13, 3);
  gr->fillRoundRect(68, 53, 58, 9, 1);

  snprintf(strnum, 11, "%4u", data->lines);
  gr->setCursor(70, 54);
  gr->print(F("LINES"));
  gr->setCursor(102, 54);
  gr->print(strnum);

  // End text
  gr->setTextBackground(BLACK);
  gr->setTextColor(WHITE);

  // Next rounded rect
  gr->fillRect(48, 42, 13, 13);
  gr->drawRoundRect(46, 40, 17, 17, 2);

  // Vertical lines arround the board
  gr->drawFastVLine(0, 0, 64);
  gr->drawFastVLine(1, 0, 64);
  gr->drawFastVLine(42, 0, 64);
  gr->drawFastVLine(43, 0, 64);

  // Bottom wall
  gr->drawPixel(36, 61);
  gr->drawPixel(37, 63);

  // Board
  for (int x = 1; x <= 10; x++) {
    int x3 = x * 3 + 2;
    // Bottom wall
    gr->drawPixel(x3 + 0, 61);
    gr->drawPixel(x3 + 1, 61);
    gr->drawPixel(x3 + 3, 61);
    gr->drawPixel(x3 + 0, 63);
    gr->drawPixel(x3 + 2, 63);
    gr->drawPixel(x3 + 3, 63);

    for (int y = 0; y < 20; y++) {
      if (get_pixel(x, y + BOARD_LINES - 21)) {
        gr->drawRect(x3 + 2, 3 * y, 3, 3);
      } else {
        gr->drawPixel(x3 + 3, 3 * y + 1);
      }
    }
  }

  for (int i = 0; i < 16; i++) {
    int v = i * 4 + 1;

    // Left wall
    gr->drawPixel(2, v);
    gr->drawPixel(3, v);
    gr->drawPixel(5, v);

    // Rigth wall
    gr->drawPixel(38, v);
    gr->drawPixel(39, v);
    gr->drawPixel(41, v);

    v += 2;
    // Left wall
    gr->drawPixel(2, v);
    gr->drawPixel(4, v);
    gr->drawPixel(5, v);

    // Rigth wall
    gr->drawPixel(38, v);
    gr->drawPixel(40, v);
    gr->drawPixel(41, v);

    // Next
    if(bitRead(tetrominoes[data->tet_next_sel][data->tet_next_rot], i)) {
      gr->drawRect(3 * ((i % 4)) + 49, 3 * (i / 4) + 43, 2, 2, BLACK);
    }
  }
  gr->display();
}

static void game_on() {
  for (;;) {
    if (!gr->nextFrame()) {
      continue;
    }
    gr->pollButtons();

    uint8_t current_key = buttonsUpdate();

    if (gr->justPressed(B_BUTTON)) {
      return;
    }

    draw_tetromino(data->tx, data->ty, tetrominoes[data->tet_now_sel][data->tet_now_rot], 0);

    if(gr->pressed(LEFT_BUTTON) && (millis() - button_wait_time > 250)) {
      if(gr->justPressed(LEFT_BUTTON)) {
        button_wait_time = millis();
      }
      dx = data->tx - 1;
    }

    if(gr->pressed(RIGHT_BUTTON) && (millis() - button_wait_time > 250)) {
      if(gr->justPressed(RIGHT_BUTTON)) {
        button_wait_time = millis();
      }
      dx = data->tx + 1;
    }

    dy = data->ty;
    if((gr->pressed(DOWN_BUTTON) && !stopDrop) || gr->justPressed(DOWN_BUTTON)) {
      stopDrop = 0;
      dy += gr->pressed(DOWN_BUTTON);
    }
    dr = (data->tet_now_rot + (gr->justPressed(UP_BUTTON) || gr->justPressed(A_BUTTON))) % 4;

    if (fit_tetromino(dx, dy, tetrominoes[data->tet_now_sel][dr])) {
      data->tx = dx;
      data->ty = dy;
      data->tet_now_rot = dr;
    } else {
      dx = data->tx;
      dy = data->ty;
      dr = data->tet_now_rot;
    }
    draw_tetromino(data->tx, data->ty, tetrominoes[data->tet_now_sel][data->tet_now_rot], 1);

    if (gr->everyXFrames((2.75 * (9 - min(9, data->level))))) {
      draw_tetromino(data->tx, data->ty, tetrominoes[data->tet_now_sel][data->tet_now_rot], 0);
      if (fit_tetromino(data->tx, data->ty + 1, tetrominoes[data->tet_now_sel][data->tet_now_rot])) {
        data->ty++;
      } else {
        draw_tetromino(data->tx, data->ty, tetrominoes[data->tet_now_sel][data->tet_now_rot], 1);
        remove_lines();
        // Game over?
        if (data->ty <= 1) {
          data->gameOn = 0;
          return;
        }
        data->tx = 3;
        data->ty = 0;
        dx = data->tx;
        stopDrop = 1;
        next_tetromino();
      }
      draw_tetromino(data->tx, data->ty, tetrominoes[data->tet_now_sel][data->tet_now_rot], 1);
    }
    display_board();
  } // for(;;)
}

static void game_new(void) {
  data->score = 0;
  data->lines = 0;
  data->level = 0;
  data->tx = dx = 3;
  data->ty = dy = 0;

  data->tet_next_sel = random(7);
  data->tet_next_rot = random(4);

  data->bag_pos = 6;
  for (int i = 0; i < 7; i++) {
    data->bag[i] = i;
  }
  reset_board();
  next_tetromino();
  data->gameOn = 1;
}

void gameTetris(Arduboy2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore) {
  // gr->setFrameRate(60);
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
  // gr->setFrameRate(30);
}

