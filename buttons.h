#pragma once

#include <Arduino.h>
#define BTN_NONE 0
#define BTN_GO_UP 1
#define BTN_GO_DOWN 2
#define BTN_GO_LEFT 4
#define BTN_GO_RIGHT 8
#define BTN_GO_A 16
#define BTN_GO_B 32
#define BTN_GO_C 64

#define MENU_EXIT 0
#define MENU_NEW 1
#define MENU_RESUME 2

#define buttonIs(haystack, needle) !!((haystack & needle) == needle)

void buttonsSetup(const uint8_t setRot, const uint8_t pinA, const uint8_t pinRight, const uint8_t pinLeft, const uint8_t pinUp, const uint8_t pinDown, const uint8_t pinB, const uint8_t pinC);
void buttonsRotate(uint8_t setRot);
uint8_t buttonsUpdate(void);
