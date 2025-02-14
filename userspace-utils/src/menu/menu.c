#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "menu.h"
#include "state.h"
#include "battery.h"

static struct winsize termSz;
static menu_t menuStack[10];
static int menuStackIndex = 0;

int MENU_Init(void) {
	ioctl(0, TIOCGWINSZ, &termSz);
	return 0;
}

void MENU_Shutdown(void) {
	/* don't need to do anything here (yet) */
	return;
}

static void batStatus(void) {
	const char* rateColor;
	char status[64];
	int statusLen, col;
	int percent = BAT_GetPercent();
    int chargeRate = BAT_GetChargeRate();

    /* Determine color for battery percentage */
    const char* percentColor;
    if (percent >= 50)
        percentColor = "\033[32m"; /* Green */
    else if (percent >= 20)
        percentColor = "\033[33m"; /* Yellow */
    else
        percentColor = "\033[31m"; /* Red */

    /* Normalize charge rate to 0 if idle */
    if (chargeRate > -25 && chargeRate < 25)
        chargeRate = 0;

    /* Determine color for charge rate */
    if (chargeRate > 0)
        rateColor = "\033[32m"; /* Green */
    else if (chargeRate < 0)
        rateColor = "\033[31m"; /* Red */
    else
        rateColor = "\033[37m"; /* White (idle) */

    /* Compose status string */
    sprintf(status,
             "Battery: %s%d%%\033[0m, %s%+dmA\033[0m",
             percentColor, percent,
             rateColor, chargeRate);

    /* Move cursor to top-right and print */
    statusLen = (int)strlen(status) - (strlen("\033[0m") * 2) - strlen(percentColor) - strlen(rateColor);
    col = (termSz.ws_col - statusLen) > 0 ? (termSz.ws_col - statusLen) : 0;

    printf("\033[1;%dH%s\n", col + 1, status);  /* +1 since columns are 1-based */
    fflush(stdout);
}

void MENU_Update(menu_t *menu) {
	int i;

	/* clear the screen */
	printf("\033[2J\033[1;1H");
	fflush(stdout);

	/* sanity checks */
	assert(menu != NULL);
	assert(menu->items != NULL);
	assert(&menu->items[0] != NULL);
	assert(menu->numItems > 0);

	/* print header */
	if (menu->header != NULL) {
		printf("%s", menu->header);
	}

	/* print the menu */
	for (i = 0; i < menu->numItems; i++) {
		if (i == menu->selection) {
			printf("\x1b[1;47m\x1b[30m * ");
		}
		else {
			printf("   ");
		}
		printf("%s\x1b[0m\n", menu->items[i].name);
	}

	/* print footer */
	if (menu->footer != NULL) {
		printf("%s", menu->footer);
	}

	/* print battery status */
	batStatus();
}

menuItem_t *MENU_GetCurrentSelected(menu_t *menu) {
	return &menu->items[menu->selection];
}

void _MENU_GoToSubMenu(menu_t *newMenu, bool pushOldToStack) {
	if (pushOldToStack) {
		assert(menuStackIndex < 10);
		menuStack[menuStackIndex] = *STATE_CurMenu;
		menuStackIndex++;
	}

	STATE_CurMenu = newMenu;
	if (STATE_CurMenu->setup != NULL) {
		STATE_CurMenu->setup();
	}
}

void MENU_GoBack(void *dummyArg) {
	(void)dummyArg;
	assert(menuStackIndex > 0);

	menuStackIndex--;

	/* run the cleanup function if it exists */
	if (STATE_CurMenu->cleanup != NULL) {
		STATE_CurMenu->cleanup();
	}

	STATE_CurMenu = &menuStack[menuStackIndex];
}
