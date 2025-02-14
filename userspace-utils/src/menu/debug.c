#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include "menu.h"
#include "screen.h"

typedef struct {
	bool launchShell;
	int code;
} exitCode_t;

static void DEBUG_CleanupAndExit(void *_exitCode) {
	exitCode_t *exitCode = (exitCode_t *)_exitCode;
	if (exitCode->launchShell) {
		open(".debugShell", O_CREAT | O_WRONLY, 0666);
	}

	exit(exitCode->code);
}

static exitCode_t codeZero = {false, 0};
static exitCode_t codeOne = {false, 1};
static exitCode_t codeZeroShell = {true, 0};

static void DEBUG_ScreenEnable(void *arg) {
	(void)arg;
	SCREEN_Enable();
}

static void DEBUG_ScreenDisable(void *arg) {
	(void)arg;
	SCREEN_Disable();
}

static void DEBUG_ScreenBrightness(void *arg) {
	int brightness = (int)(size_t)arg;
	SCREEN_SetBrightness(brightness);
}

static void DEBUG_ScreenDim(void *arg) {
	int level = (int)(size_t)arg;
	SCREEN_Dim(level);
}

menuItem_t debugMenuItems[] = {
	{"Exit App (exit code 0)", DEBUG_CleanupAndExit, (void *)&codeZero},
	{"Exit App (exit code 1)", DEBUG_CleanupAndExit, (void *)&codeOne},
	{"Exit App (exit code 0, set flag to launch debug shell)", DEBUG_CleanupAndExit, (void *)&codeZeroShell},
	{"SCREEN_Enable()", DEBUG_ScreenEnable, NULL},
	{"SCREEN_Disable()", DEBUG_ScreenDisable, NULL},
	{"SCREEN_Dim(0)", DEBUG_ScreenDim, (void *)0},
	{"SCREEN_Dim(1)", DEBUG_ScreenDim, (void *)1},
	{"SCREEN_Dim(2)", DEBUG_ScreenDim, (void *)2},
	{"SCREEN_Dim(3)", DEBUG_ScreenDim, (void *)3},
	{"SCREEN_Dim(4)", DEBUG_ScreenDim, (void *)4},
	{"SCREEN_SetBrightness(75)", DEBUG_ScreenBrightness, (void *)75},
	{"SCREEN_SetBrightness(50)", DEBUG_ScreenBrightness, (void *)50},
	{"SCREEN_SetBrightness(25)", DEBUG_ScreenBrightness, (void *)25},
	{"Back", MENU_GoBack, NULL},
};

menu_t debugMenu = {
	SWCOMP_HEADER_TITLE "Debugging Settings\n" SWCOMP_HEADER_CONTROLS,
	debugMenuItems,
	0,
	sizeof(debugMenuItems) / sizeof(menuItem_t),
	NULL,
	NULL,
	NULL
};
