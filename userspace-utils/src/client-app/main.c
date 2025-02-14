#include <stddef.h>
#include "device.h"
#include "com.h"

int main(void) {
	DEV_GatherDeviceInfo();
	COM_Init();
	DEV_DisplayDeviceInfo(NULL);

	return 0;
}
