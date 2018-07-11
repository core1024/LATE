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

static int8_t dx, dy, dr;

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
  // const uint8_t next[4] = {
  //   (const uint8_t)(tetrominoes[data->tet_next_sel][data->tet_next_rot] & 0b1111),
  //   (const uint8_t)((tetrominoes[data->tet_next_sel][data->tet_next_rot] >> 4) & 0b1111),
  //   (const uint8_t)((tetrominoes[data->tet_next_sel][data->tet_next_rot] >> 8) & 0b1111),
  //   (const uint8_t)((tetrominoes[data->tet_next_sel][data->tet_next_rot] >> 12) & 0b1111)
  // };
  // gr->drawSlowXYBitmap(50, 0, next, 4, 4, WHITE);
  gr->setCursor(0, 0);
  gr->print(F("TETRIS"));

  blockDrawFrame();
  ltoa(data->score, strnum, 10);
  gr->setCursor(0, 16);
  gr->print(F("SCORE"));
  gr->setCursor(50, 16);
  gr->print(strnum);
  ltoa(data->lines, strnum, 10);
  gr->setCursor(0, 32);
  gr->print(F("LINES"));
  gr->setCursor(50, 32);
  gr->print(strnum);
  ltoa(data->level, strnum, 10);
  gr->setCursor(0, 48);
  gr->print(F("LEVEL"));
  gr->setCursor(50, 48);
  gr->print(strnum);

  for (int x = 1; x <= 10; x++) {
    for (int y = 0; y < 20; y++) {
      if (get_pixel(x, y + BOARD_LINES - 21)) {
        blockDraw(x - 1, y);
      }
    }
  }
  for (int i = 0; i < 16; i++) {
    //if(bitRead(tetrominoes[data->tet_next_sel][data->tet_next_rot], i)) {
      blockDraw((i % 4), (i / 4));
    //}
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

    if (gr->justPressed(A_BUTTON)) {
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

    dy = data->ty + gr->pressed(DOWN_BUTTON);
    dr = (data->tet_now_rot + (gr->justPressed(UP_BUTTON) || gr->justPressed(B_BUTTON))) % 4;

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

