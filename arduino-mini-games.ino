#include <Arduino.h>
#include <Arduboy2.h>

#include "buttons.h"
#include "block.h"

#include "game_bgun.h"
#include "game_tetris.h"
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
  char *file;
  void (*play)(Arduboy2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore);
};

const uint8_t game_data_sz = 240;
uint8_t game_data[game_data_sz];

const uint8_t games_count = 4;
struct game_t games[games_count];

uint8_t rot = 1;

uint8_t choice = 0;

uint8_t fh;

void setup() {
  games[0].name = F("CTPElbA");
  games[0].file = (char *)"BGUN";
  games[0].play = &gameBGun;
  games[1].name = F("TETPiC");
  games[1].file = (char *)"TETRIS";
  games[1].play = &gameTetris;
  games[2].name = F("zMiq");
  games[2].file = (char *)"SNAKE";
  games[2].play = &gameSnake;

  buttonsSetup(&arduboy);
  arduboy.begin();
  arduboy.setFrameRate(30);

  fh = BLOCK_LINE;
}

void loop() {
  uint32_t score, hiScore;
  uint8_t menu;
  uint8_t game_on = 0;
  uint8_t btns = 1, i;

  while (!game_on) {
    if (!arduboy.nextFrame()) {
      continue;
    }
    arduboy.pollButtons();
        arduboy.drawRect(0, 0, WIDTH, HEIGHT);
        arduboy.setCursor(2, fh - 4);
        arduboy.print(F("MEHu"));
        arduboy.drawFastHLine(0, fh + 2, WIDTH);

        arduboy.setCursor(2, fh * choice + fh + fh - 1);
        arduboy.print(">");

        for (i = 0; i < games_count; i++) {
          arduboy.setCursor(8, fh * i + fh + fh - 1);
          arduboy.print(games[i].name);
        }
    arduboy.display();
    

    btns = buttonsUpdate();
    switch (btns) {
      case BTN_GO_UP:
        choice = (games_count + choice - 1) % games_count;
        break;
      case BTN_GO_DOWN:
        choice = (games_count + choice + 1) % games_count;
        break;
      case BTN_GO_RIGHT:
      case BTN_GO_A:
        game_on = 1;
        break;
    }
  }

  // Reset the display driver
  //  arduboy.initDisplay();
  //  arduboy.setPowerSave(0);
  //  arduboy.setContrast(contrast);

  randomSeed(analogRead(A7) + millis());

  // READ GAME DATA
  // TODO: EEPROM

  // RUN THE GAME

  // Call game with MENU_EXIT to obtain scores
  (*games[choice].play)(&arduboy, game_data, MENU_EXIT, &game_on, &score, &hiScore);

  do {
    // Show the menu
    btns = 1;
    for (;;) {
      if (!arduboy.nextFrame()) {
        continue;
      }
      arduboy.pollButtons();
      arduboy.drawRect(0, 0, WIDTH, HEIGHT);
      arduboy.setCursor(2, 2);
      arduboy.print(games[choice].name);
      arduboy.drawFastHLine(0, 8, WIDTH);
      arduboy.setCursor(2, 11);
      arduboy.print(F(">-CTAPT"));
      arduboy.setCursor(2, 21);
      arduboy.print(F("^-izXOd"));
      if (game_on) {
        arduboy.setCursor(2, 31);
        arduboy.print(F("\\-pPOdalvi"));
      }
      arduboy.setCursor(2, 41);
      arduboy.print(score);

      arduboy.setCursor(2, 51);
      arduboy.print(hiScore);
      arduboy.display();


      btns = buttonsUpdate();
      switch (btns) {
        case BTN_GO_A:
          menu = game_on ? MENU_RESUME : MENU_NEW; goto end_menu;
        case BTN_GO_RIGHT:
          menu = MENU_NEW; goto end_menu;
        case BTN_GO_B:
        case BTN_GO_UP:
          menu = MENU_EXIT; goto end_menu;
        case BTN_GO_DOWN:
          if (game_on) {
            menu = MENU_RESUME; goto end_menu;
          }
          break;
        default:
          break;
      }
    }
end_menu:

    // Start the game
    (*games[choice].play)(&arduboy, game_data, menu, &game_on, &score, &hiScore);

    // SAVE GAME DATA
    // TODO: EEPROM

  } while (menu != MENU_EXIT);
}
