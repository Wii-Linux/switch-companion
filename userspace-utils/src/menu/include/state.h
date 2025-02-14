#ifndef STATE_H
#define STATE_H
#include <stdbool.h>
#include <menu.h>

extern bool STATE_IsRunning;
extern menu_t *STATE_CurMenu;
extern bool STATE_NoMenuRefresh;

extern int STATE_Init();
extern void STATE_Run();
extern void STATE_Shutdown();
#endif
