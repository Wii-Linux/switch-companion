#ifndef SCREEN_H
#define SCREEN_H

extern int SCREEN_Brightness;

extern void SCREEN_Enable(void);
extern void SCREEN_Disable(void);
/*
 * Dim the screen to a certain level (0 - 4)
 */
extern void SCREEN_Dim(int level);

/*
 * Set the screen brightness to a percentage (0 - 100)
 * This will override any dimming settings.
 */
extern void SCREEN_SetBrightness(int percent);

#endif
