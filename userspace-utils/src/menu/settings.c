#include <stdio.h>
#include <assert.h>

#include "conf.h"
#include "input.h"
#include "menu.h"
#include "screen.h"
#include "update.h"
#include "state.h"

#define NUM_ARR_TO_INT ((numbers[0] * 100) + (numbers[1] * 10) + numbers[2])

static void configureDisplay(void *dummyArg) {
	inputEvent_t ev;
	int curNumberSel, brightness, isInNumber, backSelected, i, firstDraw;
	int numbers[3] = { 0, 0, 0 };
	(void)dummyArg;

	STATE_NoMenuRefresh = true;
	printf("\x1b[2J\x1b[H"); /* Clear the screen */
	fflush(stdout);

	isInNumber = 0;
	curNumberSel = 0;
	backSelected = 0;
	firstDraw = 1;

	/* seperate the brightness out into it's individual digits */
	brightness = SCREEN_Brightness;

	numbers[2] = brightness % 10;
	brightness /= 10;
	numbers[1] = brightness % 10;
	brightness /= 10;
	numbers[0] = brightness % 10;

	while (1) {
		if (firstDraw) {
			ev = INPUT_TYPE_NONE;
			firstDraw = 0;
		}
		else
			ev = INPUT_Wait();

		if (isInNumber) { /* editing a number */
			switch (ev) {
				case INPUT_TYPE_UP: {
					if (curNumberSel == 0 && numbers[0] == 0) {
						/* prevent going over 100 by forcibly setting the other nums to 0 when increasing the first to 1 */
						numbers[0] = 1;
						numbers[1] = 0;
						numbers[2] = 0;
					}
					else if (numbers[0] == 1)
						break; /* nope, can't go over 100, sorry, can't increment any of the digits here */

					else if (numbers[curNumberSel] < 9) numbers[curNumberSel]++;
					break;
				}
				case INPUT_TYPE_DOWN: {
					if (curNumberSel == 0 && numbers[0] == 1) {
						/* set to 99 to prevent accidentally turning off the screen */
						numbers[0] = 0;
						numbers[1] = 9;
						numbers[2] = 9;
					}
					else if (curNumberSel == 1 && numbers[0] == 0 && numbers[1] == 1 && numbers[2] == 0) {
						/*
						 * currently on 2nd number, and the array is { 0, 1, 0 }
						 * going down here would result in a brightness of 0, don't allow it,
						 * instead set the last number to 9 to force a valid brightness
						 */
						numbers[1]--;
						numbers[2] = 9;
						break;
					}
					else if (curNumberSel == 2 && numbers[0] == 0 && numbers[1] == 0 && numbers[2] == 1) {
						/*
						 * currently on last number, and the array is { 0, 0, 1 }
						 * going down here would result in a brightness of 0, don't allow it
						 */
						break;
					}
					else if (numbers[curNumberSel] > 0) numbers[curNumberSel]--;

					break;
				}
				case INPUT_TYPE_SELECT: {
					isInNumber = 0;
					break;
				}
				default:
					break;
			}
		}
		else { /* selecting a number to edit */
			switch (ev) {
				case INPUT_TYPE_UP: {
					if (backSelected) {
						backSelected = 0;
						curNumberSel = 0;
						break;
					}
					if (curNumberSel < 2)
						curNumberSel++;
					break;
				}
				case INPUT_TYPE_DOWN: {
					if (curNumberSel > 0 && !backSelected)
						curNumberSel--;
					else
						backSelected = 1;
					break;
				}
				case INPUT_TYPE_SELECT: {
					if (backSelected) {
						/* Clean up and leave */
						STATE_NoMenuRefresh = 0; /* Allow the menu to work again */
						CONF_Write(); /* Write the new brightness to the config file */
						return;
						/* menu code will redraw the settings menu, no need to MENU_GoBack() here since this isn't technically a menu */
					}

					isInNumber = 1;
					break;
				}
				default:
					break;
			}
		}

		printf("\x1b[2J\x1b[H"); /* Clear the screen */
#ifdef DEBUG_FEATURES
		printf("numbers={ %d, %d, %d }; curNumberSel=%d; isInNumber=%d; backSelected=%d\n", numbers[0], numbers[1], numbers[2], curNumberSel, isInNumber, backSelected);
#endif
		printf("%sBack\x1b[0m\n", backSelected ? "\x1b[1;47m\x1b[30m * " : "   ");
		puts("Brightness:");
		printf(" [  ");
		for (i = 0; i < 3; i++) {
			printf("%s%d\x1b[0m  ", (curNumberSel == i && !backSelected) ? "\x1b[1;47m\x1b[30m" : "", numbers[i]);
		}
		puts("]");

		if (isInNumber) {
			/* print a little arrow under the number to signify that it's selected */
			printf("    ");
			for (i = 0; i < 3; i++)
				printf("%s", (curNumberSel == i) ? "^  " : "   ");
		}

		fflush(stdout);


		assert(numbers[0] == 0 || numbers[0] == 1);
		assert(numbers[1] >= 0 && numbers[1] <= 9);
		assert(numbers[2] >= 0 && numbers[2] <= 9);
		assert(NUM_ARR_TO_INT > 0);
		assert(NUM_ARR_TO_INT <= 100);

		SCREEN_Brightness = NUM_ARR_TO_INT;
		SCREEN_Enable(); /* Update the brightness immediately so the user can see what that value looks like */
	}
	return;
}

menuItem_t settingsMenuItems[] = {
	{"Display", configureDisplay, NULL},
	{"System Update", MENU_GoToSubMenu, &updateMenu},
	{"Back", MENU_GoBack, NULL},
};

menu_t settingsMenu = {
	SWCOMP_HEADER_TITLE "Settings\n" SWCOMP_HEADER_CONTROLS
			  "You will require a keyboard for some of these options.\n",
	settingsMenuItems,
	0,
	sizeof(settingsMenuItems) / sizeof(menuItem_t),
	NULL,
	NULL,
	NULL
};
