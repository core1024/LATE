#include "game_bgun.h"

// Game related
#define BULLETS_COUNT 5

static U8G2 *gr;

struct data_t {
  uint8_t gameOn;
  uint32_t score;
  uint32_t hiScore;
  uint8_t lives;
  uint8_t gunPos;
  struct {
    uint8_t x;
    uint8_t y;
    uint8_t col[3];
  } target;
};

static struct data_t *data;

static uint8_t btns;

static uint8_t bullets[BULLETS_COUNT][2];

static unsigned long current_time, target_time, lift_time, bullet_time;


// Utility functions
////////////////////

static void constrainedMove(uint8_t *initial_value, int8_t move_dir, uint8_t min_pos, uint8_t max_pos) {
  if ((*initial_value > min_pos && move_dir < 0) || (*initial_value < max_pos && move_dir > 0)) {
    *initial_value += move_dir;
  }
}


static uint8_t targetCleared(void) {
  return data->target.col[0] == data->target.col[1] && data->target.col[1] == data->target.col[2];
}

static int8_t targetHit(uint8_t x, uint8_t y) {
  for (int8_t t = 0; t < 3; t++) {
    if (x == data->target.x + t && data->target.col[t] + data->target.y >= y) {
      return t;
    }
  }
  return -1;
}

static uint8_t targetLost(void) {
  return data->target.y + max(
           max(data->target.col[0], data->target.col[1]),
           max(data->target.col[1], data->target.col[2])
         ) > 18;
}

static void resetTarget(void) {
  data->target.x = 4;
  data->target.y = 0;
  do {
    data->target.col[0] = random(3) + 1;
    data->target.col[1] = random(3) + 1;
    data->target.col[2] = random(3) + 1;
  } while (targetCleared());
  while (min(
           min(data->target.col[0], data->target.col[1]),
           min(data->target.col[1], data->target.col[2])
         ) > 1) {
    data->target.col[0]--;
    data->target.col[1]--;
    data->target.col[2]--;
  }
}

static void resetBullets(void) {
  for (int8_t b = 0; b < BULLETS_COUNT; b++) {
    bullets[b][1] = 0;
  }
}

static void fireBullet(void) {
  for (int8_t b = 0; b < BULLETS_COUNT; b++) {
    if (bullets[b][1] <= 0) {
      bullets[b][0] = data->gunPos;
      bullets[b][1] = 19;
      break;
    }
  }
}

static void moveBullets(void) {
  for (int8_t b = 0; b < BULLETS_COUNT; b++) {
    if (bullets[b][1] > 0) {
      bullets[b][1]--;
      int8_t t = targetHit(bullets[b][0], bullets[b][1]);
      if (t >= 0) {
        data->target.col[t]++;
        bullets[b][1] = 0;
        if (targetCleared()) {
          resetTarget();
          resetBullets();
        }
      }
    }
  }
}

static void drawGun(void) {
  if (data->gunPos > 0) {
    blockDraw(data->gunPos - 1, 19);
  }
  blockDraw(data->gunPos, 19);
  blockDraw(data->gunPos, 18);
  if (data->gunPos < 9) {
    blockDraw(data->gunPos + 1, 19);
  }
}

static void drawTarget(void) {
  for (int8_t i = 0; i < 3; i++) {
    for (int j = 0; j < data->target.col[i]; j++) {
      blockDraw(data->target.x + i, data->target.y + j);
    }
  }
}

static void drawBullets(void) {
  for (int8_t b = 0; b < BULLETS_COUNT; b++) {
    if (bullets[b][1] > 0) {
      blockDraw(bullets[b][0], bullets[b][1] - 1);
    }
  }
}

// Transition functions
//////////////////

static void game_reset(void) {
  data->gameOn = 1;
  data->gunPos = 5;
  resetTarget();
}

static void game_new(void) {
  data->gameOn = 1;
  data->gunPos = 5;
  data->score = 0;
  data->lives = 5;
  resetTarget();
}

// State functions
//////////////////

static void game_on(void) {
  for (;;) {
    current_time = millis();
    btns = buttonsUpdate();

    switch (btns) {
      case BTN_GO_B: return;
    }
    if (btns & BTN_GO_LEFT) {
      constrainedMove(&data->gunPos, -1, 0, 9);
    }
    if (btns & BTN_GO_RIGHT) {
      constrainedMove(&data->gunPos, 1, 0, 9);
    }
    if (btns & (BTN_GO_UP | BTN_GO_A )) {
      fireBullet();
    }

    if (current_time > bullet_time + 15) {
      bullet_time = current_time;
      moveBullets();
    }

    if (current_time > target_time + 1000) {
      target_time = current_time;
      constrainedMove(&data->target.x, random(3) - 1, 0, 7);
    }

    if (current_time > lift_time + 10000) {
      lift_time = current_time;
      data->target.y++;
    }

    if (targetLost()) {
      // Game Over
      data->gameOn = 0;
      return;
    }

    gr->firstPage();
    do {
      blockDrawFrame();
      drawGun();
      drawTarget();
      drawBullets();
    } while (gr->nextPage());
  } // for(;;)
}

void gameBGun(U8G2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore) {
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

