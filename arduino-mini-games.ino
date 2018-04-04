#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//type func(const __FlashStringHelper *progmem_str);

#include "PetitFS.h"
// The SD chip select pin is currently defined as 5
// in pffArduino.h.  Edit pffArduino.h to change the CS pin.

FATFS fs;     /* File system object */

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

struct game_t {
  char *name;
  void (*play)(U8G2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore);
};

uint8_t contrast = 127;
//U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ A1, /* dc=*/ A2, /* reset=*/ A0);     // Nokia 5110 Display
//U8G2_PCD8544_84X48_2_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ A1, /* dc=*/ A2, /* reset=*/ A0);     // Nokia 5110 Display
U8G2_SH1106_128X64_NONAME_2_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ A0, /* dc=*/ A1, /* reset=*/ A2);
//U8G2_ST7567_JLX12864_2_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ A1, /* dc=*/ A2, /* reset=*/ A0);
//uint8_t contrast = 110;

const uint8_t game_data_sz = 240;
uint8_t game_data[game_data_sz];

const uint8_t games_count = 3;
struct game_t games[games_count];

uint8_t rot = 1;

uint8_t choice = 0;

uint8_t fh;

void rotateDisplay(uint8_t r) {
  switch (r) {
    case 0:
      u8g2.setDisplayRotation(U8G2_R0);
      break;
    case 1:
      u8g2.setDisplayRotation(U8G2_R1);
      break;
    case 2:
      u8g2.setDisplayRotation(U8G2_R2);
      break;
    case 3:
      u8g2.setDisplayRotation(U8G2_R3);
      break;
  }
  buttonsRotate(r);
  blocksSetup(&u8g2);
}


void setup() {
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);

  //  games[0].name = (char *)"Clock";
  //  games[0].play = &gameClock;
  games[0].name  = (char *)"BGUN";
  games[0].play = &gameBGun;
  games[1].name  = (char *)"TETRIS";
  games[1].play = &gameTetris;
  games[2].name  = (char *)"SNAKE";
  games[2].play = &gameSnake;

  buttonsSetup(0, BTN_PIN_A, BTN_PIN_RIGHT, BTN_PIN_LEFT, BTN_PIN_UP, BTN_PIN_DOWN, BTN_PIN_B, BTN_PIN_C);
  u8g2.begin(BTN_PIN_A, BTN_PIN_RIGHT, BTN_PIN_LEFT, BTN_PIN_UP, BTN_PIN_DOWN, BTN_PIN_B);
  u8g2.setContrast(contrast);
  //u8g2.setFont(u8g2_font_5x7_tr);
  //u8g2.setFont(u8g2_font_lucasfont_alternate_tr);
  //u8g2.setFont(u8g2_font_mozart_nbp_tr);
  u8g2.setFont(u8g2_font_chikita_tr);
  fh = BLOCK_LINE;
  rotateDisplay(rot);

  for (;;) {
    if (pf_mount(&fs)) {
      u8g2.firstPage();
      do {
        u8g2.setCursor(2, fh + 1);
        u8g2.print(F("SD Error"));
      } while (u8g2.nextPage());
      while (!buttonsUpdate());
    } else {
      break;
    }
  }
}

void loop() {
  uint32_t score, hiScore;
  uint8_t menu;
  uint8_t game_on = 0;

  while (!game_on) {
    uint8_t btns, i;
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
      case BTN_GO_B:
        rot = (rot + 2) % 4;
        rotateDisplay(rot);
        break;
      default:
        break;
    }

    u8g2.firstPage();
    do {
      u8g2.drawFrame(0, 0, u8g2.getWidth(), u8g2.getHeight());
      u8g2.setCursor(2, fh + 1);
      u8g2.print(F("MAIN MENU"));
      u8g2.drawHLine(0, fh + 2, u8g2.getWidth());

      u8g2.drawGlyph(2, fh * choice + fh + fh + 4, '>');

      for (i = 0; i < games_count; i++) {
        u8g2.drawStr(8, fh * i + fh + fh + 4, games[i].name);
      }
    } while (u8g2.nextPage());
  }

  // Reset the display driver
  //  u8g2.initDisplay();
  //  u8g2.setPowerSave(0);
  //  u8g2.setContrast(contrast);

  randomSeed(analogRead(A7) + millis());

  // READ GAME DATA
  memset(game_data, 0, sizeof(game_data));

  UINT nr;
  if (!pf_open(games[choice].name)) {
    pf_read(game_data, sizeof(game_data), &nr);
  }

  // RUN THE GAME

  // Call game with MENU_EXIT to obtain scores
  (*games[choice].play)(&u8g2, game_data, MENU_EXIT, &game_on, &score, &hiScore);

  do {
    // Show the menu
    for (;;) {
      u8g2.firstPage();
      do {
        u8g2.drawFrame(0, 0, u8g2.getWidth(), u8g2.getHeight());
        u8g2.setCursor(2, 9);
        u8g2.print(games[choice].name);
        u8g2.drawHLine(0, 11, u8g2.getWidth());
        u8g2.setCursor(2, 21);
        u8g2.print(F("> - Start"));
        u8g2.setCursor(2, 31);
        u8g2.print(F("< - Exit"));
        if (game_on) {
          u8g2.setCursor(2, 41);
          u8g2.print(F("v - Resume"));
        }
        u8g2.setCursor(2, 51);
        u8g2.print(score);

        u8g2.setCursor(2, 61);
        u8g2.print(hiScore);
      } while (u8g2.nextPage());
      switch (buttonsUpdate()) {
        case BTN_GO_A:
          menu = game_on ? MENU_RESUME : MENU_NEW; goto end_menu;
        case BTN_GO_RIGHT:
          menu = MENU_NEW; goto end_menu;
        case BTN_GO_B:
        case BTN_GO_LEFT:
          menu = MENU_EXIT; goto end_menu;
        case BTN_GO_DOWN:
          if (game_on) {
            menu = MENU_RESUME; goto end_menu;
          }
          break;
        default:
          continue;
      }
    }
end_menu:

    // Start the game
    (*games[choice].play)(&u8g2, game_data, menu, &game_on, &score, &hiScore);

    // SAVE GAME DATA
    if (!pf_open(games[choice].name)) {
      pf_lseek(0);
      pf_write(game_data, sizeof(game_data), &nr);
      pf_write(0, 0, &nr);
    }
  } while (menu != MENU_EXIT);
}
