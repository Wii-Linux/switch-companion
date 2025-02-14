#include <stdio.h>

#include "state.h"
#include "input.h"
#include "menu.h"

bool STATE_IsRunning;
menu_t *STATE_CurMenu;
bool STATE_NoMenuRefresh = false;

int STATE_Init(void) {
	STATE_IsRunning = true;
	STATE_CurMenu = NULL;
	return 0;
}

void STATE_Run(void) {
	/* wait for input */
	inputEvent_t ev = INPUT_Wait();

	INPUT_DoAction(ev);

	/* redraw */
	MENU_Update(STATE_CurMenu);
}

void STATE_Shutdown(void) {
	return;
}
