#include <stdio.h>

#include "input.h"
#include "menu.h"
#include "update.h"

/* hack needed to be able to cram everything into a void * */
typedef struct {
	updateType_t updateType;
} updInfo_t;

static updInfo_t updInfoNet = {UPD_TYPE_NET};
static updInfo_t updInfoWii = {UPD_TYPE_WII};
static updInfo_t updInfoPC = {UPD_TYPE_PC};



static void updateSystem(void *_info) {
	inputEvent_t ev;
	updInfo_t *info = (updInfo_t *)_info;
	updateType_t type = info->updateType;
	(void)type;

	puts("TODO");

	puts("Press any key/button to return.");

	ev = INPUT_Wait();
	(void)ev;

	return;
}

menuItem_t updateMenuItems[] = {
	{"Update via network", updateSystem, (void *)&updInfoNet},
	{"Update via USB (connected to Wii-Linux)", updateSystem, (void *)&updInfoWii},
	{"Update via USB (connected to PC)", updateSystem, (void *)&updInfoPC},
	{"Back", MENU_GoBack, NULL},
};
menu_t updateMenu = {
	SWCOMP_HEADER_TITLE "System Update\n" SWCOMP_HEADER_CONTROLS
			  "Please select your desired update type.\n",
	updateMenuItems,
	0,
	sizeof(updateMenuItems) / sizeof(menuItem_t),
	NULL,
	NULL,
	NULL
};
