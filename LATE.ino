#include <Arduino.h>
#include <Arduboy2.h>
#include <EEPROM.h>

#include "common.h"

#include "game_tetris.h"
#include "game_1010.h"

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
  const uint8_t *logo;
};

const uint8_t game_data_sz = 240;
uint8_t game_data[game_data_sz];

const uint8_t games_count = 2;
struct game_t games[games_count];

uint8_t choice = 0;

uint8_t fh;

static unsigned long button_wait_time;

void setup() {
  games[0].name = F("Blocks Arcade");
  games[0].address = EEPROM_STORAGE_SPACE_START + sizeof(game_data);
  games[0].play = &gameTetris;
  games[0].logo = gameTetrisLogo;
  games[1].name = F("Blocks Puzzle");
  games[1].address = EEPROM_STORAGE_SPACE_START + 2 * sizeof(game_data);
  games[1].play = &game1010;
  games[1].logo = game1010Logo;

  arduboy.begin();
  arduboy.setFrameRate(30);
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
    arduboy.print(F("Choose Game Mode"));
    arduboy.drawFastHLine(0, fh + 2, WIDTH);

    arduboy.setCursor(2, fh * choice + fh + fh - 2);
    arduboy.print(">");

    arduboy.drawBitmap(2, 55, dPadBmp, 7, 7, WHITE);
    arduboy.setCursor(12, 55);
    arduboy.print(F("Move"));
    arduboy.drawBitmap(66, 55, aBmp, 7, 7, WHITE);
    arduboy.setCursor(76, 55);
    arduboy.print(F("Select"));


    for (i = 0; i < games_count; i++) {
      arduboy.drawBitmap(9, fh * i + fh + fh - 2, games[i].logo, 7, 7, WHITE);
      arduboy.setCursor(23, fh * i + fh + fh - 2);
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
    arduboy.idle();
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

  if(hiScore == ~0 && game_on) {
    memset(game_data, 0, sizeof(game_data));
    game_on = score = hiScore = 0;
  }
  last_score = ~0;
  do {
    arduboy.clear();

    arduboy.drawRect(0, 0, WIDTH, HEIGHT);
    arduboy.setCursor(2, 2);
    arduboy.print(games[choice].name);
    arduboy.drawFastHLine(0, 10, WIDTH);

    arduboy.drawBitmap(60, 12, cupBmp, 8, 8, WHITE);
    if (game_on) {
      drawNumber(&arduboy, 32, 13, score, WHITE, 7);
    } else {
      arduboy.drawFastHLine(56, 15, 3);
    }
    drawNumber(&arduboy, 69, 13, hiScore, WHITE, 0);

    // Game over
    if(!game_on && last_score != ~0) {

      arduboy.setCursor(37, 20);
      arduboy.print(F("Game Over"));

      if(last_score < score) {
        arduboy.setCursor(2, 37);
        arduboy.print(F("Congratulations!"));
        arduboy.setCursor(2, 46);
        arduboy.print(F("That's a new record!"));
      } else if(score / last_score * 100 > 75) {
        arduboy.setCursor(20, 28);
        arduboy.print(F("You almost beat"));
        arduboy.setCursor(20, 37);
        arduboy.print("the record.");
        arduboy.setCursor(20, 46);
        arduboy.print("Better luck");
        arduboy.setCursor(20, 55);
        arduboy.print("next time.");
      } else {
        arduboy.setCursor(2, 28);
        arduboy.print(F("That was"));
        arduboy.setCursor(2, 37);
        arduboy.print("unfortunate.");
        arduboy.setCursor(2, 46);
        arduboy.print("Next time you will");
        arduboy.setCursor(2, 55);
        arduboy.print("do better.");
      }
      arduboy.display();
      for (;;) {
        arduboy.pollButtons();
        if(arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
          break;
        }
        arduboy.idle();
      }
      arduboy.fillRect(61, 20, 124, 40, BLACK);
    } else {
      arduboy.drawBitmap(61, 32, games[choice].logo, 7, 7, WHITE);
    }

    arduboy.drawBitmap(2, 55, aBmp, 7, 7, WHITE);
    arduboy.setCursor(12, 55);
    if (game_on) {
      arduboy.print(F("Resume"));
    } else {
      arduboy.print(F("Start"));
    }
    arduboy.drawBitmap(66, 55, bBmp, 7, 7, WHITE);
    arduboy.setCursor(76, 55);
    arduboy.print(F("Exit"));

    arduboy.display();

    // Handle buttons
    for (;;) {
      if (!arduboy.nextFrame()) {
        continue;
      }
      arduboy.pollButtons();

      if(arduboy.justPressed(A_BUTTON)) {
        menu = game_on && ! arduboy.pressed(LEFT_BUTTON) ? MENU_RESUME : MENU_NEW; break;
      }

      if(arduboy.justPressed(B_BUTTON)) {
          menu = MENU_EXIT; break;
      }
      arduboy.idle();
    }

    // Start the game
    last_score = hiScore;
    (*games[choice].play)(&arduboy, game_data, menu, &game_on, &score, &hiScore);

    // SAVE GAME DATA
    EEPROM.put(games[choice].address, game_data);

  } while (menu != MENU_EXIT);
}
