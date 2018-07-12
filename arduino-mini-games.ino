#include <Arduino.h>
#include <Arduboy2.h>
#include <EEPROM.h>

#include "buttons.h"
#include "block.h"

#include "game_tetris.h"
#include "game_bgun.h"
#include "game_snake.h"

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

const uint8_t games_count = 3;
struct game_t games[games_count];

uint8_t choice = 0;

uint8_t fh;

void setup() {
  games[0].name = F("TETRIS");
  games[0].address = EEPROM_STORAGE_SPACE_START + sizeof(game_data);
  games[0].play = &gameTetris;
  games[1].name = F("BGUN");
  games[1].address = EEPROM_STORAGE_SPACE_START;
  games[1].play = &gameBGun;
  games[2].name = F("SNAKE");
  games[2].address = EEPROM_STORAGE_SPACE_START + 2 * sizeof(game_data);
  games[2].play = &gameSnake;

  arduboy.begin();
  arduboy.setFrameRate(30);
  blocksSetup(&arduboy);
  buttonsSetup(&arduboy);

  fh = 8;
}

void loop() {
  uint32_t score, hiScore;
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
    if(arduboy.justPressed(RIGHT_BUTTON) || arduboy.justPressed(A_BUTTON)) {
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

  do {
    // Show the menu
    for (;;) {
      if (!arduboy.nextFrame()) {
        continue;
      }
      arduboy.pollButtons();
      arduboy.clear();

      arduboy.drawRect(0, 0, WIDTH, HEIGHT);
      arduboy.setCursor(2, 2);
      arduboy.print(games[choice].name);
      arduboy.drawFastHLine(0, 10, WIDTH);
      arduboy.setCursor(2, 12);
      arduboy.print(F(">-START"));
      arduboy.setCursor(2, 22);
      arduboy.print(F("^-EXIT"));
      if (game_on) {
        arduboy.setCursor(2, 32);
        arduboy.print(F("v-CONTINUE"));
      }
      arduboy.setCursor(2, 41);
      arduboy.print(score);

      arduboy.setCursor(2, 51);
      arduboy.print(hiScore);
      arduboy.display();

      // Handle buttons
      if(arduboy.justPressed(A_BUTTON)) {
          menu = game_on ? MENU_RESUME : MENU_NEW; goto end_menu;
      }
      if(arduboy.justPressed(RIGHT_BUTTON)) {
          menu = MENU_NEW; goto end_menu;
      }
      if(arduboy.justPressed(UP_BUTTON) || arduboy.justPressed(B_BUTTON)) {
          menu = MENU_EXIT; goto end_menu;
      }
      if(arduboy.justPressed(DOWN_BUTTON)) {
        if (game_on) {
          menu = MENU_RESUME; goto end_menu;
        }
      }

    }
end_menu:

    // Start the game
    (*games[choice].play)(&arduboy, game_data, menu, &game_on, &score, &hiScore);
    
    // SAVE GAME DATA
    EEPROM.put(games[choice].address, game_data);

  } while (menu != MENU_EXIT);
}
