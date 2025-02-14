#ifndef INPUT_H
#define INPUT_H
#define MAX_KBD_DEVICES 8
#define PWR_BUTTON_NAME "platform-7000d000.i2c-platform-max77620-onoff-event"
#define VOL_BUTTON_NAME "platform-gpio-keys-event"
#define TOUCH_NAME "platform-7000c500.i2c-event"
#define PWR_BUTTON "/dev/input/by-path/" PWR_BUTTON_NAME
#define VOL_BUTTON "/dev/input/by-path/" VOL_BUTTON_NAME
#define TOUCH_DEVICE "/dev/input/by-path/" TOUCH_NAME

typedef enum {
	INPUT_TYPE_NONE,
	INPUT_TYPE_SELECT,
	INPUT_TYPE_UP,
	INPUT_TYPE_DOWN,
	INPUT_TYPE_KEYBOARD,
	INPUT_TYPE_ERROR = -1
} inputEvent_t;


extern int INPUT_Init();
extern void INPUT_Shutdown();
extern inputEvent_t INPUT_Wait();
extern void INPUT_DoAction(inputEvent_t act);

#endif
