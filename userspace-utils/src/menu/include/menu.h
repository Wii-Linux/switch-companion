#ifndef MENU_H
#define MENU_H
#include <stdbool.h>

typedef struct {
	char *name;
	void (*action)(void *param);
	void *actionParam;
} menuItem_t;

typedef struct {
	char *header;
	menuItem_t *items;
	int selection;
	int numItems;
	void (*setup)(void);
	void (*cleanup)(void);
	char *footer;
} menu_t;

extern int MENU_Init(void);
extern void MENU_Shutdown(void);
extern void _MENU_GoToSubMenu(menu_t *newMenu, bool pushOldToStack);
static __attribute__((unused)) void MENU_GoToSubMenu(void *newMenu) {
	_MENU_GoToSubMenu(newMenu, true);
}
extern void MENU_Update(menu_t *menu);
extern menuItem_t *MENU_GetCurrentSelected(menu_t *menu);
extern void MENU_GoBack(void *dummyArg);

#define SWCOMP_HEADER_TITLE "Wii-Linux Switch Companion (" SWCOMP_VERSION ") - "
#define SWCOMP_HEADER_CONTROLS "Use the volume buttons to move up/down, and the power button to select.\n" \
      		  "You can also use a keyboard.\n\n"

#endif
