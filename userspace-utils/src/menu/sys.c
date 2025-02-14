#include <stdlib.h>

/* sadly there is not a more direct way to do this */
/* while also bringing down processes and saving data. */
void SYS_DoRestart(void *arg) {
	(void)arg; /* unused */
	system("reboot");
	return;
}

void SYS_DoShutdown(void *arg) {
	(void)arg; /* unused */
	system("poweroff");
	return;
}
