#pragma once

#include <Arduboy2.h>

// LEFT_BUTTON, RIGHT_BUTTON, UP_BUTTON, DOWN_BUTTON, A_BUTTON, B_BUTTON 

#define BTN_NONE 0
#define BTN_GO_UP UP_BUTTON
#define BTN_GO_DOWN DOWN_BUTTON
#define BTN_GO_LEFT LEFT_BUTTON
#define BTN_GO_RIGHT RIGHT_BUTTON
#define BTN_GO_A B_BUTTON
#define BTN_GO_B A_BUTTON

#define MENU_EXIT 0
#define MENU_NEW 1
#define MENU_RESUME 2

#define buttonIs(haystack, needle) !!((haystack) & (needle))

void buttonsSetup(Arduboy2 *ab);
uint8_t buttonsUpdate(void);
