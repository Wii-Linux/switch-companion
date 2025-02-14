#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "com.h"
#include "input.h"
#include "state.h"
#include "featuresMenu.h"
#include "device.h"
#include "settings.h"
#ifdef DEBUG_FEATURES
#include "debug.h"
#endif
#include "sys.h"
#include "menu.h"
#include "conf.h"

static menuItem_t mainMenuItems[] = {
	{"Toggle Features", MENU_GoToSubMenu, &featuresMenu},
	{"Device Info", DEV_DisplayDeviceInfo, NULL},
	{"Settings", MENU_GoToSubMenu, &settingsMenu},
	#ifdef DEBUG_FEATURES
	{"**Debugging Features**", MENU_GoToSubMenu, &debugMenu},
	#endif
	{"Restart", SYS_DoRestart, NULL},
	{"Shutdown", SYS_DoShutdown, NULL},
};

static menu_t mainMenu = {
	SWCOMP_HEADER_TITLE "Main Menu\n" SWCOMP_HEADER_CONTROLS,
	mainMenuItems,
	0,
	sizeof(mainMenuItems) / sizeof(menuItem_t),
	NULL,
	NULL,
	NULL
};

int main(int argc, char *argv[]) {
	int ret;
	(void)argc;
	(void)argv;

	ret = INPUT_Init();
	if (ret != 0) {
		printf("Input initialization failed (%d)!\n", ret);
		return 1;
	}

	ret = STATE_Init();
	if (ret != 0) {
		printf("State machine initialization failed (%d)!\n", ret);
		return 1;
	}

	ret = CONF_Init();
	if (ret != 0) {
		printf("Configuration initialization failed (%d)!\n", ret);
		return 1;
	}

	ret = MENU_Init();
	if (ret != 0) {
		printf("Menu initialization failed (%d)!\n", ret);
		return 1;
	}

	ret = FEAT_Apply();
	if (ret != 0) {
		printf("Feature application failed (%d)!\n", ret);
		return 1;
	}

	DEV_GatherDeviceInfo();
	ret = COM_Init();
	if (ret != 0) {
		printf("Communication initialization failed (%d)!\n", ret);
		return 1;
	}

	/* set the initial menu */
	_MENU_GoToSubMenu(&mainMenu, false);

	/* first menu draw */
	MENU_Update(STATE_CurMenu);

	/* main loop */
	while (STATE_IsRunning) {
		STATE_Run();
	}

	/* cleanup before we exit */
	STATE_Shutdown();
	INPUT_Shutdown();

	return 0;
}
