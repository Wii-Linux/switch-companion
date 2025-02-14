#include <stdio.h>
#include <string.h>
void UTIL_GetCPUName(char *cpuName) {
	char line[256];
	FILE *file = fopen("/proc/cpuinfo", "r");
	if (!file) {
		perror("Failed to open /proc/cpuinfo");
		return;
	}

	while (fgets(line, sizeof(line), file)) {
		if (strncmp(line, "model name", 10) == 0 || strncmp(line, "Processor", 9) == 0) {
			char *colon = strchr(line, ':');
			if (colon) {
				strcpy(cpuName, colon + 2);
				cpuName[strlen(cpuName) - 1] = '\0';	/* remove newline */
				break;
			}
			break;
		}
	}

	fclose(file);
}

#ifdef CLIENT_APP
int UTIL_GetModelFromCPUInfo(char *model) {
	char line[256];
	FILE *file = fopen("/proc/cpuinfo", "r");
	int ret = 1;
	if (!file) {
		perror("Failed to open /proc/cpuinfo");
		return 1;
	}

	while (fgets(line, sizeof(line), file)) {
		if (strncmp(line, "machine", 7) == 0) {
			char *colon = strchr(line, ':');
			if (colon) {
				strcpy(model, colon + 2);
				model[strlen(model) - 1] = '\0';	/* remove newline */
				ret = 0;
				break;
			}
			break;
		}
	}

	fclose(file);

	return ret;
}
#endif
