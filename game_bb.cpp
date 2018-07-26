#include "game_bb.h"

#define HERO_LEVEL 45
#define HERO_OFFSET 4
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

static struct data_t *data;

static uint8_t bridge;
static uint8_t hero;
struct platform_t platformFuture;

static int8_t platform_end(struct platform_t platform) {
	return platform.x + platform.width - 1;
}

static void platform_random(struct platform_t prev, struct platform_t *platform) {
	platform->x = platform_end(prev) + random(2, BRIDGE_LEVEL - 10);
	platform->width = random(5, 30);
	platform->bridge = 0;
}

static void platform_copy(struct platform_t src, struct platform_t *dest) {
	dest->x = src.x;
	dest->width = src.width;
	dest->bridge = src.bridge;
}

static void display_platform(struct platform_t platform, uint8_t color) {
	uint8_t bridge_start;
	gr->fillRect(platform.x, PLATFORM_LEVEL, platform.width, 9, color);
	gr->drawPixel(platform.x + platform.width / 2, PLATFORM_LEVEL, BLACK);
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

static void display_hero(int8_t state, int8_t x, int8_t j, uint8_t color) {
	int8_t y = 45;
	int8_t foot = y + 8;
	if(j) {
		y += 10;
		foot = y;
		gr->drawPixel(x, foot, color);
		gr->drawPixel(x + 2, foot, color);
	}
	gr->drawBitmap(x, y + 2, gameBBLogo, 3, 7, color);
	if(state == 1) {
		gr->drawPixel(x + 2, foot, BLACK);
		gr->drawPixel(x + 3, foot, color);
	}
	if(state == 2) {
		gr->drawPixel(x, foot, BLACK);
		gr->drawPixel(x - 1, foot, color);
	}
}

static void display_background(void) {
	// The score
	gr->drawBitmap(64+29, 1, cupBmp, 8, 8, WHITE);
	gr->fillRect(65, 2, 28, 5, BLACK);
	drawNumber(gr, 65, 2, data->score, WHITE, 7);
	gr->fillRect(102, 2, 26, 5, BLACK);
	drawNumber(gr, 102, 2, data->hiScore, WHITE, 0);

	display_floor();
}

static void display_bridge(uint8_t color) {
	gr->drawFastHLine(platform_end(data->platformCurr),
		BRIDGE_LEVEL, bridge, color);
}

static void hero_walk(int8_t from, int8_t to) {
	int8_t i = from;
	while(i < to /* && ! gr->justPressed(B_BUTTON) */) {
		if (!gr->nextFrame()) {
			continue;
		}
		gr->pollButtons();

		display_hero(i % 3, i, gr->pressed(DOWN_BUTTON), WHITE);
		gr->display();
		display_hero(i % 3, i, gr->pressed(DOWN_BUTTON), BLACK);

		// Game over?
		if(platform_end(data->platformCurr) + bridge <= i &&
			platform_end(data->platformCurr) + bridge <= data->platformNext.x) {
			data->gameOn = 0;
			break;
		}
		gr->idle();
		i++;
	}
	data->score++;
	if(data->score > data->hiScore) {
		data->hiScore = data->score;
	}
}

static void build_bridge(void) {
	bridge = 0;
	while(!gr->justReleased(A_BUTTON)) {
		if (!gr->nextFrame()) {
			continue;
		}
		gr->pollButtons();
		bridge += bridge < BRIDGE_LEVEL;
		gr->drawFastVLine(platform_end(data->platformCurr),
			BRIDGE_LEVEL - bridge + 1, bridge, WHITE);
		gr->display();
		gr->idle();
	}
	gr->drawFastVLine(platform_end(data->platformCurr),
		BRIDGE_LEVEL - bridge + 1, bridge, BLACK);
	display_bridge(WHITE);

	hero_walk(platform_end(data->platformCurr) - HERO_OFFSET,
		platform_end(data->platformNext) - HERO_OFFSET);

	data->platformNext.bridge =
		platform_end(data->platformCurr) + bridge - data->platformNext.x;

	platform_copy(data->platformNext, &data->platformCurr);
	data->platformCurr.x = 32 - data->platformCurr.width / 2;
	platform_random(data->platformCurr, &data->platformNext);
	data->platformNext.bridge = bridge = 0;
}


static void game_new(void) {
	data->gameOn = 1;
	bridge = data->score = 0;
	data->platformCurr.x = -19;
	data->platformCurr.width = 36;
	platform_random(data->platformCurr, &data->platformNext);
	data->platformCurr.bridge = 0;
	data->platformNext.bridge = 0;
}

static void game_on(void) {
	bridge = 0;
	while(data->gameOn && ! gr->justPressed(B_BUTTON)) {
		if (!gr->nextFrame()) {
			continue;
		}
		gr->pollButtons();
		gr->clear();
		display_background();
		display_hero(0, platform_end(data->platformCurr) - HERO_OFFSET,
			0, WHITE);
		if(gr->justPressed(A_BUTTON)) {
			build_bridge();
		}
		if(bridge) {
			display_bridge(WHITE);
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
