#ifndef UPDATE_H
#define UPDATE_H
#include <menu.h>

typedef enum {
	UPD_TYPE_NET,
	UPD_TYPE_WII,
	UPD_TYPE_PC
} updateType_t;

extern menu_t updateMenu;

#endif
