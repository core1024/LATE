#include "game_snake.h"

// Game related
#define MAX_TAIL_LENGTH 20
#define MIN_TAIL_LENGTH 4

static U8G2 *gr;

struct point_t {
  int8_t x;
  int8_t y;
};

struct data_t {
  uint8_t gameOn;
  uint32_t score;
  uint32_t hiScore;
  uint8_t level;
  uint8_t lives;
  uint8_t tail_l;
  struct point_t tail[MAX_TAIL_LENGTH];
  struct point_t target;
  struct point_t go;
};

static struct data_t *data;

static uint8_t btns;

static unsigned long current_time, target_time;


// Utility functions
////////////////////
static uint8_t drawTail(point_t *head) {
  for (uint8_t i = 0; i < data->tail_l; i++) {
    if (head && i && head->x == data->tail[i].x && head->y == data->tail[i].y) {
      return 1;
    } else {
      blockDraw(data->tail[i].x, data->tail[i].y);
    }
  }
  return 0;
}


static void targetNew(void) {
  do {
  data->target.x = random(10);
  data->target.y = random(20);
  } while(drawTail(&data->target));
}

static void drawTarget(void) {
  if ((current_time / 200) % 2) {
    blockDraw(data->target.x, data->target.y);
  }
}

// Transition functions
//////////////////

static void game_reset(void) {
  targetNew();
  data->tail_l = MIN_TAIL_LENGTH;
  memset(data->tail, 0, sizeof(data->tail));
  data->tail[0].x = 5;
  data->tail[1].x = 5;
  data->tail[2].x = 5;
  data->tail[0].y = 18;
  data->tail[1].y = 18;
  data->tail[2].y = 19;
  data->go.x = 0;
  data->go.y = 19;
}

void game_new(void) {
  data->gameOn = 1;
  data->score = 0;
  data->level = 0;
  data->lives = 5;
  game_reset();
}

// State functions
//////////////////
static void game_frame(void) {
  data->tail[0].x = (data->tail[0].x + data->go.x) % 10;
  data->tail[0].y = (data->tail[0].y + data->go.y) % 20;
  if (drawTail(&data->tail[0])) {
    // Game Over
    data->gameOn = 0;
  }
  if (data->tail[0].x == data->target.x && data->tail[0].y == data->target.y) {
    targetNew();
    data->tail_l = (data->tail_l + 1) % MAX_TAIL_LENGTH;
    if (data->tail_l == 0) {
      game_reset();
    }
    data->score++;
    if (data->score > data->hiScore) {
      data->hiScore = data->score;
    }
  }
  memmove((void *)&data->tail[1], (void *)&data->tail[0], sizeof(data->tail) - sizeof(data->tail[0]));
  target_time = current_time;
}

static void game_draw(void) {
  char strnum[12];
  gr->firstPage();
  do {
    gr->setCursor(0, 0);
    gr->print(F("zMiq"));
    gr->setFontDirection(1);
    ltoa(data->score, strnum, 10);
    gr->drawStr(blockScale(10) + 6, BLOCK_LINE + 1, strnum);
    gr->setFontDirection(0);
    blockDrawFrame();
    drawTarget();
    drawTail(NULL);
  } while (gr->nextPage());
}

static void game_on(void) {
  while (data->gameOn) {
    current_time = millis();
    btns = buttonsUpdate();

    if (btns) {
      if (btns & BTN_GO_B) {
        return;
      }
      if (btns & BTN_GO_LEFT && data->go.x != 1) {
        data->go.y = 0;
        data->go.x = 9;
      }
      if (btns & BTN_GO_RIGHT && data->go.x != 9) {
        data->go.y = 0;
        data->go.x = 1;
      }

      if (btns & BTN_GO_UP && data->go.y != 1) {
        data->go.x = 0;
        data->go.y = 19;
      }
      if (btns & BTN_GO_DOWN && data->go.y != 19) {
        data->go.x = 0;
        data->go.y = 1;
      }
      game_frame();
    } // if btns
    if (current_time > target_time + 400) {
      game_frame();
    }
    game_draw();
  } // hwile game on
}

void gameSnake(U8G2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore) {
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

