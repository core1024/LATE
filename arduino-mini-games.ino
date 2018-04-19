#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

const uint8_t vladofont[911] U8G2_FONT_SECTION("vladofont") = 
  "^\2\3\2\3\3\1\4\4\5\5\0\373\0\373\0\373\1)\2o\3v \6\356\361s\2!\10\356"
  "q\245u\60\3\42\10\356\361\324\222\234\14#\14\356\361\224\24)\211\6)\211\0$\12\356\361LI\70"
  "&\321\4%\13\356qH\211\22V\224H\1&\13\356\361hQ\246\224\62%\1'\10\356\361\244\71\13"
  "\0(\10\356q\205i\71\3)\10\356q\305i\61\3*\13\356q$\245\203\64%\25\0+\11\356\361"
  "#\341\230C\0,\7\356\361S\303\24-\7\356\361\303;\1.\7\356\361\63\206\0/\6\356q\366\12"
  "\60\13\356\361L\221\322\42E\23\0\61\10\356q\205jq\2\62\12\356\361LY\246\205\203\2\63\11\356"
  "\361\314\231\34M\0\64\12\356\361\205Z\22\15b\4\65\13\356q\14J:\304\311\20\1\66\12\356\361L"
  "\351\20e\321\4\67\10\356q\14b\265\6\70\12\356\361LY\64e\321\4\71\12\356\361LY\64\244\321"
  "\4:\11\356\361#\71\230C\0;\10\356q\345\264\60\4<\10\356\361\205\345\22\0=\11\356\361\3;"
  "\264#\0>\10\356\361\304\305\42\0?\12\356\361LY\246c\31\0@\13\356\361LY\222,\311\24\2"
  "A\14\356q\205I\224%\203\222%\0B\15\356q\14Q\226\14Q\226\14\21\0C\10\356\361Liy"
  "\2D\15\356q\14Q\226dI\226\14\21\0E\12\356q\14J\272\245\203\2F\12\356q\14J\272\245"
  ")\0G\12\356\361Li\262d\321\4H\15\356qdI\226\14J\226d\11\0I\10\356\361\214iq"
  "\2J\12\356\361\214i\226\204\32\0K\14\356qdIT\311\224(K\0L\10\356q\244\255\203\2M"
  "\13\356qd\211\245[\222%\0N\13\356qd\211\224tR\262\4O\13\356\361LY\222%Y\64\1"
  "P\13\356q\14Q\226\14Q\232\2Q\12\356\361hQ\27MI\0R\13\356q\14Q\226\14Q[\2"
  "S\12\356\361\14I<'C\4T\10\356q\14Z\332\6U\14\356qdI\226dI\26M\0V\14"
  "\356qdI\226dQ\22f\0W\13\356qdI\226\364)\211\0X\13\356qdQ\22V\242,\1"
  "Y\12\356qdQ\22\246\65\0Z\11\356q\14b\343\240\0[\10\356q\211iU\2\134\12\356qg"
  "Q\22\346\20\0]\10\356\361\250\215\32\0^\12\356\361#a\22e\71\0_\10\356\361\263\15\12\0`"
  "\7\356q\305\71\31a\12\356q\250\351\26E\23\0b\14\356q\14J:DY\62D\0c\11\356q"
  "D=\15j\2d\14\356qiI\226D\203\222%\0e\6\355\351\63\2f\12\356q\205SR\32\63"
  "\0g\10\356q\14J\332\12h\6\355\351\63\2i\14\356qdI\244\264HI\226\0j\13\356qI"
  "Y\22)-R\2k\6\355\351\63\2l\14\356q\205I\226dI\224%\0m\6\355\351\63\2n\6"
  "\355\351\63\2o\6\355\351\63\2p\15\356q\14J\226dI\226d\11\0q\15\356\361\14I\26\15I"
  "\226d\11\0r\6\355\351\63\2s\12\356qdI\377\62(\0t\6\355\351\63\2u\12\356qD\225"
  "\312\322S\4v\12\356q$=MI/\0w\6\355\351\63\2x\6\355\351\63\2y\11\356qdQ"
  "\22\266\2z\12\356\361LY&e\321\4{\12\356\361\204C\26\245a\4|\12\356\361ca\224\204!"
  "\0}\12\356\361eC\22\225C\0\0\0\0";

// AbBgdEvzijKlMHOpPCTyfXchswoauqx

//type func(const __FlashStringHelper *progmem_str);

#include <PetitFS.h>
// The SD chip select pin is currently defined as 5
// in pffArduino.h.  Edit pffArduino.h to change the CS pin.

FATFS fs;     /* File system object */

#include "buttons.h"
#include "block.h"

#include "game_bgun.h"
#include "game_tetris.h"
#include "game_snake.h"
#include "game_sudoku.h"

#define BTN_PIN_UP 9
#define BTN_PIN_DOWN 6
#define BTN_PIN_LEFT 8
#define BTN_PIN_RIGHT 7
#define BTN_PIN_A 4
#define BTN_PIN_B 2
#define BTN_PIN_C A3

struct game_t {
  const __FlashStringHelper *name;
  char *file;
  void (*play)(U8G2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore);
};

uint8_t contrast = 127;
//U8G2_PCD8544_84X48_2_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ A1, /* dc=*/ A2, /* reset=*/ A0);     // Nokia 5110 Display
U8G2_SH1106_128X64_NONAME_2_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ A0, /* dc=*/ A1, /* reset=*/ A2);
//U8G2_ST7567_JLX12864_2_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ A0, /* dc=*/ A1, /* reset=*/ A2);
//uint8_t contrast = 110;

const uint8_t game_data_sz = 240;
uint8_t game_data[game_data_sz];

const uint8_t games_count = 4;
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
  games[0].name = F("CTPElbA");
  games[0].file = (char *)"BGUN";
  games[0].play = &gameBGun;
  games[1].name = F("TETPiC");
  games[1].file = (char *)"TETRIS";
  games[1].play = &gameTetris;
  games[2].name = F("zMiq");
  games[2].file = (char *)"SNAKE";
  games[2].play = &gameSnake;
  games[3].name = F("CydOKy");
  games[3].file = (char *)"SUDOKU";
  games[3].play = &gameSudoku;

  buttonsSetup(0, BTN_PIN_A, BTN_PIN_RIGHT, BTN_PIN_LEFT, BTN_PIN_UP, BTN_PIN_DOWN, BTN_PIN_B, BTN_PIN_C);
  u8g2.begin(BTN_PIN_A, BTN_PIN_RIGHT, BTN_PIN_LEFT, BTN_PIN_UP, BTN_PIN_DOWN, BTN_PIN_B);
  u8g2.setContrast(contrast);
  //u8g2.setFont(u8g2_font_5x7_tr);
  //u8g2.setFont(u8g2_font_lucasfont_alternate_tr);
  //u8g2.setFont(u8g2_font_mozart_nbp_tr);
  //u8g2.setFont(u8g2_font_chikita_tr);
  u8g2.setFont(vladofont);
  fh = BLOCK_LINE;
  rotateDisplay(rot);

  for (;;) {
    if (pf_mount(&fs)) {
      u8g2.firstPage();
      do {
        u8g2.drawFrame(0, 0, u8g2.getWidth(), u8g2.getHeight());
        u8g2.setCursor(2, fh + 1);
        u8g2.print(F("SD gPEsKA"));
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
  uint8_t btns = 1, i;

  while (!game_on) {
    if(btns) {
      u8g2.firstPage();
      do {
        u8g2.drawFrame(0, 0, u8g2.getWidth(), u8g2.getHeight());
        u8g2.setCursor(2, fh - 4);
        u8g2.print(F("MEHu"));
        u8g2.drawHLine(0, fh + 2, u8g2.getWidth());

        u8g2.drawGlyph(2, fh * choice + fh + fh - 1, '>');

        for (i = 0; i < games_count; i++) {
          u8g2.setCursor(8, fh * i + fh + fh - 1);
          u8g2.print(games[i].name);
        }
      } while (u8g2.nextPage());
    }

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
  }

  // Reset the display driver
  //  u8g2.initDisplay();
  //  u8g2.setPowerSave(0);
  //  u8g2.setContrast(contrast);

  randomSeed(analogRead(A7) + millis());

  // READ GAME DATA
  memset(game_data, 0, sizeof(game_data));

  UINT nr;
  if (!pf_open(games[choice].file)) {
    pf_read(game_data, sizeof(game_data), &nr);
  }

  // RUN THE GAME

  // Call game with MENU_EXIT to obtain scores
  (*games[choice].play)(&u8g2, game_data, MENU_EXIT, &game_on, &score, &hiScore);

  do {
    // Show the menu
    btns = 1;
    for (;;) {
      if(btns) {
        u8g2.firstPage();
        do {
          u8g2.drawFrame(0, 0, u8g2.getWidth(), u8g2.getHeight());
          u8g2.setCursor(2, 2);
          u8g2.print(games[choice].name);
          u8g2.drawHLine(0, 8, u8g2.getWidth());
          u8g2.setCursor(2, 11);
          u8g2.print(F(">-CTAPT"));
          u8g2.setCursor(2, 21);
          u8g2.print(F("^-izXOd"));
          if (game_on) {
            u8g2.setCursor(2, 31);
            u8g2.print(F("\\-pPOdalvi"));
          }
          u8g2.setCursor(2, 41);
          u8g2.print(score);

          u8g2.setCursor(2, 51);
          u8g2.print(hiScore);
        } while (u8g2.nextPage());
      }

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
    (*games[choice].play)(&u8g2, game_data, menu, &game_on, &score, &hiScore);

    // SAVE GAME DATA
    if (!pf_open(games[choice].file)) {
      pf_lseek(0);
      pf_write(game_data, sizeof(game_data), &nr);
      pf_write(0, 0, &nr);
    }
  } while (menu != MENU_EXIT);
}
