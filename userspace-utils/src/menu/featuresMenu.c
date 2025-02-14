#include <stdio.h>
#include <stdlib.h>

#include "com.h"
#include "input.h"
#include "menu.h"
#include "state.h"
#include "conf.h"

static char *ramdiskStr[2] = { "RAMDisk [Disabled]", "RAMDisk [Enabled]" };
static char *networkingStr[2] = { "Networking [Disabled]", "Networking [Enabled]" };

int ramdiskEnabled = 0;
int networkingEnabled = 0;
static void settingsMenuSetup(void);

int FEAT_Apply(void) {
	COM_Shutdown();
	if (system("cd /usr/libexec/usb; ./teardown.sh"))
		return -1;

	/* Determine which script to call based on the new state */
	if (ramdiskEnabled && networkingEnabled) { /* RAMDisk + USB Ethernet + Communication Channel */
		if (system("cd /usr/libexec/usb; ./ramdisk+eth.sh"))
			return -2;
		goto out;
	} else if (ramdiskEnabled) { /* RAMDisk + Communication Channel */
		if (system("cd /usr/libexec/usb; ./ramdisk.sh"))
			return -2;
		goto out;
	} else if (networkingEnabled) { /* USB Ethernet + Communication Channel */
		if (system("cd /usr/libexec/usb; ./eth.sh"))
			return -2;
		goto out;
	}
	else if (!ramdiskEnabled && !networkingEnabled) { /* just Communication Channel (no features enabled) */
		if (system("cd /usr/libexec/usb; ./com.sh"))
			return -2;
		goto out;
	}

out:
	if (COM_Init())
		return -3;
	return 0;
}


static void toggle(void *arg) {
	int item = (int)(size_t)arg;
	int ret;
	inputEvent_t ev;

	STATE_NoMenuRefresh = true;
	printf("\x1b[2J\x1b[1;1H"); /* Clear the screen */
	fflush(stdout);

	/* Toggle the selected item */
	if (item == 1) {
		ramdiskEnabled = !ramdiskEnabled;
	} else if (item == 2) {
		networkingEnabled = !networkingEnabled;
	}
	switch (FEAT_Apply()) {
		case 0:
			puts("Changes applied successfully.");
			break;
		case -1:
			puts("Failed to stop USB services.");
			break;
		case -2:
			puts("Failed to apply changes.");
			break;
		case -3:
			puts("Failed to reinitialize communication channel.");
			break;
		default:
			puts("Unknown error occurred.");
			break;
	}

	ret = CONF_Write();
	if (ret != 0) {
		puts("Failed to save configuration.");
	} else {
		puts("Configuration saved.");
	}

	puts("Press any key/button to return.");

	ev = INPUT_Wait();
	(void)ev;

	settingsMenuSetup();
	return;
}
menuItem_t featuresMenuItems[] = {
	{NULL, toggle, (void *)1},
	{NULL, toggle, (void *)2},
	{"Back", MENU_GoBack, NULL},
};

static void settingsMenuSetup(void) {
	STATE_NoMenuRefresh = false;
	featuresMenuItems[0].name = ramdiskStr[ramdiskEnabled];
	featuresMenuItems[1].name = networkingStr[networkingEnabled];
}

menu_t featuresMenu = {
	SWCOMP_HEADER_TITLE "Features\n" SWCOMP_HEADER_CONTROLS,
	featuresMenuItems,
	0,
	sizeof(featuresMenuItems) / sizeof(menuItem_t),
	settingsMenuSetup,
	NULL,
	NULL
};

void FEAT_Setup(int ramdisk, int networking) {
	ramdiskEnabled = ramdisk;
	networkingEnabled = networking;
}
