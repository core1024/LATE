#include <Arduino.h>
#include <Arduboy2.h>
#include <EEPROM.h>

#include "buttons.h"
#include "block.h"

#include "game_tetris.h"
#include "game_1010.h"
#include "game_bgun.h"

#define BTN_PIN_UP 9
#define BTN_PIN_DOWN 6
#define BTN_PIN_LEFT 8
#define BTN_PIN_RIGHT 7
#define BTN_PIN_A 4
#define BTN_PIN_B 2
#define BTN_PIN_C A3

Arduboy2 arduboy;

struct game_t {
  const __FlashStringHelper *name;
  int address;
  void (*play)(Arduboy2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore);
};

const uint8_t game_data_sz = 240;
uint8_t game_data[game_data_sz];

const uint8_t games_count = 2;
struct game_t games[games_count];

uint8_t choice = 0;

uint8_t fh;

static unsigned long button_wait_time;

void setup() {
  games[0].name = F("TETRIS");
  games[0].address = EEPROM_STORAGE_SPACE_START + sizeof(game_data);
  games[0].play = &gameTetris;
  games[1].name = F("1010");
  games[1].address = EEPROM_STORAGE_SPACE_START + 2 * sizeof(game_data);
  games[1].play = &game1010;
  // games[2].name = F("BGUN");
  // games[2].address = EEPROM_STORAGE_SPACE_START;
  // games[2].play = &gameBGun;

  arduboy.begin();
  arduboy.setFrameRate(30);
  blocksSetup(&arduboy);
  buttonsSetup(&arduboy);

  fh = 8;
}

void loop() {
  uint32_t score, hiScore, last_score;
  uint8_t menu;
  uint8_t game_on = 0;
  uint8_t i;

  while (!game_on) {
    if (!arduboy.nextFrame()) {
      continue;
    }
    arduboy.pollButtons();
    arduboy.clear();

    arduboy.drawRect(0, 0, WIDTH, HEIGHT);
    arduboy.setCursor(2, 2);
    arduboy.print(F("MENU"));
    arduboy.drawFastHLine(0, fh + 2, WIDTH);

    arduboy.setCursor(2, fh * choice + fh + fh - 2);
    arduboy.print(">");

    for (i = 0; i < games_count; i++) {
      arduboy.setCursor(8, fh * i + fh + fh - 2);
      arduboy.print(games[i].name);
    }
    arduboy.display();

    // Handle buttons
    if(arduboy.justPressed(UP_BUTTON)) {
        choice = (games_count + choice - 1) % games_count;
    }
    if(arduboy.justPressed(DOWN_BUTTON)) {
        choice = (games_count + choice + 1) % games_count;
    }
    if(arduboy.justPressed(A_BUTTON)) {
        game_on = 1;
    }
  }

  arduboy.initRandomSeed();

  // READ GAME DATA
  EEPROM.get(games[choice].address, game_data);
  if(arduboy.pressed(LEFT_BUTTON)) {
    memset(game_data, 0, sizeof(game_data));
  }

  // RUN THE GAME

  // Call game with MENU_EXIT to obtain scores
  (*games[choice].play)(&arduboy, game_data, MENU_EXIT, &game_on, &score, &hiScore);
  last_score = ~0;
  do {
    arduboy.clear();

    arduboy.drawRect(0, 0, WIDTH, HEIGHT);
    arduboy.setCursor(2, 2);
    arduboy.print(games[choice].name);
    arduboy.drawFastHLine(0, 10, WIDTH);

    // Game over
    if(!game_on && last_score != ~0) {
      arduboy.setCursor(2, 12);
      arduboy.print(F("GAME OVER"));

      // Congrats for hight score?
      arduboy.display();
      for (;;) {
        arduboy.pollButtons();
        if(arduboy.justPressed(A_BUTTON)) {
          break;
        }
      }
    }

    // Show the menu
    for (;;) {
      if (!arduboy.nextFrame()) {
        continue;
      }
      arduboy.pollButtons();

      arduboy.setCursor(2, 12);
      arduboy.print(F("[A] - "));
      if (game_on) {
        arduboy.print(F("CONTINUE"));
      } else {
        arduboy.print(F("START"));
      }
      arduboy.setCursor(2, 22);
      arduboy.print(F("[B] - EXIT"));
      arduboy.setCursor(2, 41);
      arduboy.print(score);

      arduboy.setCursor(2, 51);
      arduboy.print(hiScore);
      arduboy.display();

      // Handle buttons
      if(arduboy.justPressed(A_BUTTON)) {
        menu = game_on && ! arduboy.pressed(LEFT_BUTTON) ? MENU_RESUME : MENU_NEW; break;
      }

      if(arduboy.justPressed(B_BUTTON)) {
          menu = MENU_EXIT; break;
      }
    }

    // Start the game
    last_score = hiScore;
    (*games[choice].play)(&arduboy, game_data, menu, &game_on, &score, &hiScore);

    // SAVE GAME DATA
    EEPROM.put(games[choice].address, game_data);

  } while (menu != MENU_EXIT);
}
