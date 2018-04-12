#include "game_sudoku.h"
#define GAMES_FILE "SUDEAS.TXT"
#define BOARD_SIZE 81
#define UNDO_STEPS 64

static U8G2 *gr;
static UINT nr;

// Game state data
struct data_t {
  uint8_t gameOn;
  uint8_t level;
  char screen[BOARD_SIZE];
  char undoValue[UNDO_STEPS];
  char undoPosition[UNDO_STEPS];
  uint8_t undoLast;
  uint8_t undoCurrent;
  uint8_t cursor;
};

static struct data_t *data;

static const char kb_data[] = "123456789URC";
static uint8_t kb_cursor, kb_show, cursor, cursor_mx, cursor_my, check;

static char solution[BOARD_SIZE];
static unsigned long redraw_time;

static void show_kb(void) {
  cursor = kb_cursor;
  cursor_mx = 3;
  cursor_my = 4;
  kb_show = 1;
}

static void hide_kb(void) {
  cursor = data->cursor;
  cursor_mx = 9;
  cursor_my = 9;
  kb_show = 0;
}

static void display_board(void) {
  gr->firstPage();
  do {
    uint8_t i, x, y;
    gr->setCursor(0, 0);
    gr->print(F("SUDOKU"));
    gr->drawHLine(0, 6, 63);

    // Game board
    for (i = 0; i < 81; i++) {
      x = i % 9;
      y = i / 9;
      if(data->screen[i] >= '1' && data->screen[i] <= '9') {
        if(check < 2 || (check == 2 && data->screen[i] == solution[i])) {
          gr->setCursor(x * 7 + 1, y * 7 + 11);
          gr->print(data->screen[i]);
        }
      }
    }

    if(kb_show) {
      // Keypad
      for (i = 0; i < 12; i++) {
        x = i % 3;
        y = i / 3;
        gr->setCursor(x * 9 + 20, y * 9 + 81);
        gr->print(kb_data[i]);
      }

      gr->setDrawColor(2);
      gr->drawBox((kb_cursor % 3) * 9 + 19, (kb_cursor / 3) * 9 + 80, 7 ,7);
      gr->setDrawColor(1);
      gr->drawFrame((data->cursor % 9) * 7, (data->cursor / 9) * 7 + 10, 7 ,7);
    } else {
      gr->setDrawColor(2);
      gr->drawBox((data->cursor % 9) * 7, (data->cursor / 9) * 7 + 10, 7 ,7);
      gr->setDrawColor(1);
    }

    // The # grid
    gr->drawHLine(0, 31, 63);
    gr->drawHLine(0, 51, 63);
    gr->drawVLine(21, 10, 63);
    gr->drawVLine(41, 10, 63);
  } while (gr->nextPage());
}

static void game_on() {
  kb_cursor = 4;
  hide_kb();

  if (!pf_open(GAMES_FILE)) {
    pf_lseek(data->level * (BOARD_SIZE + BOARD_SIZE + 2) + BOARD_SIZE + 1);
    pf_read(solution, sizeof(solution), &nr);
    if(nr != sizeof(solution)) return;
  }

  display_board();
  for (;;) {
    uint8_t current_key = buttonsUpdate();
    if(current_key) {
      check = 0;
      switch(current_key) {
        case BTN_GO_B: 
          if(kb_show) {
            hide_kb();
          } else {
            return;
          }
          break;
        case BTN_GO_LEFT:
          cursor = floor(cursor / cursor_mx) * cursor_mx + ((cursor + cursor_mx - 1) % cursor_mx);
          break;
        case BTN_GO_RIGHT:
          cursor = floor(cursor / cursor_mx) * cursor_mx + ((cursor + cursor_mx + 1) % cursor_mx);
          break;
        case BTN_GO_UP:
          cursor = (cursor + cursor_mx * cursor_my - cursor_mx) % (cursor_mx * cursor_my);
          break;
        case BTN_GO_DOWN:
          cursor = (cursor + cursor_mx) % (cursor_mx * cursor_my);
          break;
        case BTN_GO_A:
          if(kb_show) {
            // Set digit
            if(data->screen[data->cursor] == '0' && kb_cursor < 9) {
              data->screen[data->cursor] = kb_data[kb_cursor];
              data->undoValue[data->undoCurrent] = kb_data[kb_cursor];
              data->undoPosition[data->undoCurrent] = data->cursor;
              data->undoCurrent++;
              data->undoLast = data->undoCurrent;

              // Check if solved
              if(!memcmp(data->screen, solution, BOARD_SIZE)) {
                data->gameOn = 0;
                return;
              }
            }

            if(kb_data[kb_cursor] == 'C') {
              check = 1;
            }

            // Undo
            if(kb_data[kb_cursor] == 'U' && data->undoCurrent > 0) {
              data->undoCurrent--;
              data->cursor = data->undoPosition[data->undoCurrent];
              data->screen[data->cursor] = '0';
            }

            // Redo
            if(kb_data[kb_cursor] == 'R' && data->undoCurrent < data->undoLast) {
              data->cursor = data->undoPosition[data->undoCurrent];
              data->screen[data->cursor] = data->undoValue[data->undoCurrent];
              data->undoCurrent++;
            }

            hide_kb();
          } else {
            show_kb();
          }
          break;
      } // switch(current_key)

      if(kb_show) {
        kb_cursor = cursor;
      } else {
        data->cursor = cursor;
      }
      display_board();
    } // if(current_key)

    if(check && (millis() > redraw_time)) {
      redraw_time = millis() + 500;
      check = (check % 2) + 1;
      display_board();
    }
  } // for(;;)
}

static void game_new(void) {
  data->gameOn = 1;
  data->cursor = 40;
  data->undoLast = 0;
  data->undoCurrent = 0;
  data->level++;

  if (!pf_open(GAMES_FILE)) {
    do {
      pf_lseek(data->level * (BOARD_SIZE + BOARD_SIZE + 2));
      pf_read(data->screen, sizeof(data->screen), &nr);
      if(nr == sizeof(data->screen)) {
        break;
      }
      data->level = 0;
    } while(data->level > 0);
  }
}

void gameSudoku(U8G2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore) {
  gr = sgr;
  data = (struct data_t *)gdat;

  if (menu == MENU_NEW) {
    game_new();
  }

  if (menu != MENU_EXIT) {
    game_on();
  }

  *gameOn = data->gameOn;
  *score = data->level;
  *hiScore = sizeof(data_t);
}

