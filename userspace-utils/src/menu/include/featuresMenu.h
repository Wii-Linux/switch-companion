#ifndef FEATURESMENU_H
#define FEATURESMENU_H

#include <menu.h>
extern menu_t featuresMenu;
extern int ramdiskEnabled;
extern int networkingEnabled;

extern void FEAT_Setup(int ramdisk, int networking);
extern int FEAT_Apply(void);
#endif
