#include "game_bb.h"

#define HERO_LEVEL 45
#define HERO_OFFSET 4
#define HERO_HEIGHT 10
#define BONUS_OFFSET 5
#define BRIDGE_LEVEL 54
#define PLATFORM_LEVEL 55

struct platform_t {
	int8_t x;
	int8_t width;
	int8_t bridge;
};

struct data_t {
	uint8_t gameOn;
	uint32_t score;
	uint32_t hiScore;
	uint8_t bonus;
	struct platform_t platformCurr;
	struct platform_t platformNext;
};

static Arduboy2 *gr;

const uint8_t spriteMap[][5] PROGMEM = {
   {0b11100, 0b11101, 0b00010, 0b11101, 0b11100}
};


static struct data_t *data;

static uint8_t bridge;
static uint8_t bonus;

static int8_t platform_end(struct platform_t platform) {
	return platform.x + platform.width - 1;
}

static void platform_random(struct platform_t prev, struct platform_t *platform) {
	platform->x = platform_end(prev) + random(2, BRIDGE_LEVEL - 20);
	platform->width = random(5, 15);
	platform->bridge = 0;
}

static void platform_copy(struct platform_t src, struct platform_t *dest) {
	dest->x = src.x;
	dest->width = src.width;
	dest->bridge = src.bridge;
}

static void platform_new(void) {
	struct platform_t platformFuture;
	uint8_t distance;
	bonus = 0;

	data->platformNext.bridge =
		platform_end(data->platformCurr) + bridge - data->platformNext.x;

	platform_copy(data->platformNext, &data->platformCurr);
	data->platformCurr.x = bridge;
	platform_random(data->platformCurr, &data->platformNext);
	data->platformNext.bridge = bridge = 0;
	distance = data->platformNext.x - platform_end(data->platformCurr);
	if(distance > BONUS_OFFSET + 3 && random(2) == 0) {
		bonus = platform_end(data->platformCurr) + 2 + random(distance - BONUS_OFFSET - 2);
	}
}

static void display_platform(struct platform_t platform, uint8_t color) {
	uint8_t bridge_start;
	gr->fillRect(platform.x, PLATFORM_LEVEL, platform.width, 9, color);
	gr->drawPixel(platform.x + platform.width / 2, PLATFORM_LEVEL+2, BLACK);
	if(platform.bridge) {
		bridge_start = 0;
		gr->drawFastHLine(bridge_start,
		BRIDGE_LEVEL,
		platform.x + platform.bridge - bridge_start, color);
	}
}


static void display_floor(void) {
	display_platform(data->platformCurr, WHITE);
	display_platform(data->platformNext, WHITE);
}

static void display_hero(int8_t state, int8_t x, int8_t y, uint8_t color) {
	int8_t foot = y + 8;
	if(state & 4) {
		y += HERO_HEIGHT;
		foot = y;
		gr->drawPixel(x, foot, color);
		gr->drawPixel(x + 2, foot, color);
	}
	gr->drawBitmap(x, y + 2, gameBBLogo, 3, 7, color);
	if(state & 1) {
		gr->drawPixel(x + 2, foot, BLACK);
		gr->drawPixel(x + 3, foot, color);
	}
	if(state & 2) {
		gr->drawPixel(x, foot, BLACK);
		gr->drawPixel(x - 1, foot, color);
	}
}

static void display_bridge(uint8_t color) {
	gr->drawFastHLine(platform_end(data->platformCurr),
		BRIDGE_LEVEL, bridge, color);
}

static void display_background(void) {
	gr->clear();
	// The score
	gr->drawBitmap(64+29, 1, cupBmp, 8, 8, WHITE);
	gr->fillRect(65, 2, 28, 5, BLACK);
	drawNumber(gr, 65, 2, data->score, WHITE, 7);
	gr->fillRect(102, 2, 26, 5, BLACK);
	drawNumber(gr, 102, 2, data->hiScore, WHITE, 0);
	gr->drawBitmap(64+30, 10, spriteMap[0], BONUS_OFFSET, BONUS_OFFSET, WHITE);
	drawNumber(gr, 102, 10, data->bonus, WHITE, 0);

	if(bonus) {
		gr->drawBitmap(bonus, PLATFORM_LEVEL + 2,
			spriteMap[0], BONUS_OFFSET, BONUS_OFFSET, WHITE);
	}

	if(bridge) {
		display_bridge(WHITE);
	}

	display_floor();
}

static void hero_fall(int8_t x, int8_t y) {
	while(y < HEIGHT) {
		if (!gr->nextFrame()) {
			continue;
		}
		display_background();
		display_hero(4 | (y % 3), x, y, WHITE);
		gr->display();
		gr->idle();
		y++;
	}
}

static void hero_score(int8_t up) {
	data->score += up;
	if(data->score > data->hiScore) {
		data->hiScore = data->score;
	}
}


static int8_t hero_walk(int8_t from, int8_t to) {
	int8_t bridge_end;
	int8_t hero_state = 0;
	int8_t claim_bonus = 0;
	int8_t i = from;
	while(i < to /* && ! gr->justPressed(B_BUTTON) */) {
		if (!gr->nextFrame()) {
			continue;
		}
		gr->pollButtons();
		if(buttonPressed(gr) && i + HERO_OFFSET + HERO_OFFSET <= data->platformNext.x) {
			hero_state = hero_state ? 0 : 4;
		}
		display_background();
		display_hero((i % 3) | hero_state, i, HERO_LEVEL, WHITE);
		gr->display();

		// Game over?
		bridge_end = platform_end(data->platformCurr) + bridge;
		// TODO: simplify this condition
		if((i + HERO_OFFSET >= bridge_end &&
			(data->platformNext.x >= bridge_end ||
			bridge_end - 1 > platform_end(data->platformNext))) ||
			(hero_state &&
			i + HERO_OFFSET + HERO_OFFSET > data->platformNext.x)) {
			data->gameOn = 0;
			hero_fall(i + HERO_OFFSET + (hero_state ? -1 : 1),
				HERO_LEVEL - (hero_state ? 0 : HERO_HEIGHT));
			return 0;
		}

		// Pick bonus
		if(hero_state && i + HERO_OFFSET >= bonus && i <= bonus + BONUS_OFFSET) {
			bonus = 0;
			claim_bonus = 1;
		}
		gr->idle();
		i++;
	}
	return claim_bonus;
}

static void rise_bridge(int8_t bridge_x) {
	bridge = 0;
	while(! buttonReleased(gr)) {
		if (!gr->nextFrame()) {
			continue;
		}
		gr->pollButtons();
		bridge += bridge < BRIDGE_LEVEL;
		gr->drawFastVLine(bridge_x,
			BRIDGE_LEVEL - bridge + 1, bridge, WHITE);
		gr->display();
		gr->idle();
	}
	gr->drawFastVLine(bridge_x,
		BRIDGE_LEVEL - bridge + 1, bridge, BLACK);
}

static void place_bridge(int8_t bridge_x) {
	uint8_t bridge_short = (data->platformNext.x >= platform_end(data->platformCurr) + bridge);
	for(float d = 0;d < PI/(bridge_short ? 1 : 2);) {
		int8_t x, y;
		if (!gr->nextFrame()) {
			continue;
		}
		y = BRIDGE_LEVEL + sin(d + PI + PI / 2) * bridge;
		x = bridge_x + cos(d + PI + PI / 2) * bridge;
		gr->drawLine(bridge_x, BRIDGE_LEVEL, x, y, WHITE);
		gr->display();
		gr->drawLine(bridge_x, BRIDGE_LEVEL, x, y, BLACK);
		d+=PI / 15;
		gr->idle();
	}
	bridge = bridge_short ? 0 : bridge;
}

static void bonus_up(int8_t claim_bonus) {
	if(! claim_bonus) return;
	int8_t y = HERO_LEVEL - 2;
	int8_t hero_pos = platform_end(data->platformNext) - HERO_OFFSET;
	while(! buttonPressed(gr) && y > 0) {
		if (!gr->nextFrame()) {
			continue;
		}
		gr->pollButtons();

		display_background();
		gr->drawFastHLine(hero_pos - 4, y + 2, 3);
		gr->drawFastVLine(hero_pos - 3, y + 1, 3);
		drawNumber(gr, hero_pos, y, claim_bonus, WHITE, 0);
		display_hero(0, hero_pos, HERO_LEVEL, WHITE);
		y -= (HERO_LEVEL - y) / 2;
		gr->display();
		gr->idle();
	}
}

static void build_bridge(void) {
	int8_t claim_bonus;
	int8_t bridge_x = platform_end(data->platformCurr);

	rise_bridge(bridge_x);
	place_bridge(bridge_x);

	// Walk over the bridge
	claim_bonus = hero_walk(bridge_x - HERO_OFFSET,
		max(platform_end(data->platformNext) - HERO_OFFSET,
		bridge_x + bridge));

	if(data->gameOn) {
		claim_bonus += bridge_x + bridge - 1 == data->platformNext.x + data->platformNext.width / 2;
		data->bonus += claim_bonus;
		bonus_up(claim_bonus);
		hero_score(claim_bonus + 1);
	}
}

static void bonus_for_live(void) {
	if(data->bonus < 5) return;
	gr->fillRoundRect(4, 16, 120, 32, 5, BLACK);
	gr->drawRoundRect(4, 16, 120, 32, 5, WHITE);
	gr->setCursor(7, 19);
	gr->print(F("Spend 5 bonus"));
	gr->setCursor(7, 27);
	gr->print(F("points to continue?"));

	arduboy.drawBitmap(7, 38, aBmp, 7, 7, WHITE);
	arduboy.setCursor(17, 38);
	arduboy.print(F("Accept"));

	arduboy.drawBitmap(66, 38, bBmp, 7, 7, WHITE);
	arduboy.setCursor(76, 38);
	arduboy.print(F("Decline"));

	gr->display();
	for(;;) {
		if (!gr->nextFrame()) {
			continue;
		}
		gr->pollButtons();
		if(gr->justPressed(A_BUTTON)) {
			data->bonus -= 5;
			data->gameOn = 1;
			bridge = 0;
			return;
		}
		if(gr->justPressed(B_BUTTON)) {
			return;
		}
	}
}

static void game_new(void) {
	data->gameOn = 1;
	data->score = 0;
	data->platformCurr.x = -19;
	data->platformCurr.width = 36;
	platform_random(data->platformCurr, &data->platformNext);
	data->platformCurr.bridge = 0;
	data->platformNext.bridge = 0;
}

static void game_on(void) {
	bonus = bridge = 0;
	while(data->gameOn && ! gr->justPressed(B_BUTTON)) {
		if (!gr->nextFrame()) {
			continue;
		}
		gr->pollButtons();
		display_background();
		display_hero(0, platform_end(data->platformCurr) - HERO_OFFSET,
			HERO_LEVEL, WHITE);
		if(buttonPressed(gr) && ! gr->justPressed(B_BUTTON)) {
			build_bridge();
			if(data->gameOn) {
				// Place new platform
				platform_new();
			} else {
				bonus_for_live();
			}
		}
		gr->display();
		gr->idle();
	}
}


void gameBB(Arduboy2 *sgr, uint8_t *gdat, uint8_t menu, uint8_t *gameOn, uint32_t *score, uint32_t *hiScore) {
	gr = sgr;
	data = (struct data_t *)gdat;

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
