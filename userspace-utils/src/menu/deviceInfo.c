#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "util.h"
#include "com.h"
#ifndef CLIENT_APP
#include "input.h"
#include "state.h"
#endif

#define MODEL_PATH "/sys/firmware/devicetree/base/model"

void DEV_GatherDeviceInfo(void) {
	FILE *file;
	int cores;
	int ram;
	struct utsname uts;
#ifdef CLIENT_APP
	int ret;
	char line[256];
	char *start, *end;
	comDeviceInfo_t *targetInfo = &comClientDeviceInfo;
#else
	char alpineRel[64];
	comDeviceInfo_t *targetInfo = &comHostDeviceInfo;
#endif

	/* get the model name from MODEL_PATH */
#ifdef CLIENT_APP
	ret = UTIL_GetModelFromCPUInfo(comClientDeviceInfo.deviceModel);
	if (ret) {
#endif
		/* get the model name from MODEL_PATH */
		file = fopen(MODEL_PATH, "r");
		if (!file) {
			perror("Failed to open " MODEL_PATH);
			strcpy(comClientDeviceInfo.deviceModel, "Unknown");
			goto outModel;
		}
		fgets(comClientDeviceInfo.deviceModel, sizeof(comClientDeviceInfo.deviceModel), file);
		fclose(file);
#ifdef CLIENT_APP
	}
#endif
outModel:

	/* get number of CPUs and CPU name */
	cores = sysconf(_SC_NPROCESSORS_ONLN);
	UTIL_GetCPUName(targetInfo->cpuName);

	strcat(targetInfo->cpuName, " [");
	sprintf(targetInfo->cpuName + strlen(targetInfo->cpuName), "%dc]", cores);

	/* get RAM Size */
	ram = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / 1024 / 1024;
	sprintf(targetInfo->memSize, "%d MB", ram);

	/* get kernel version */
	if (uname(&uts) < 0) {
		perror("uname");
		return;
	}
	strcpy(targetInfo->kernelVer, uts.release);

#ifdef CLIENT_APP
	/* get distro from /etc/os-release */
	file = fopen("/etc/os-release", "r");
	if (!file) {
		perror("Failed to open /etc/os-release");
		return;
	}
	strcpy(targetInfo->distroName, "Unknown Distro");
	while (fgets(line, sizeof(line), file)) {
		if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
			start = strchr(line, '"');
			end = strrchr(line, '"');
			if (start && end && start < end) {
				*end = '\0'; /* remove trailing quote */
				strcpy(targetInfo->distroName, start + 1);
				break;
			}
		}
	}
#else
	/* get Alpine Linux release */
	file = fopen("/etc/alpine-release", "r");
	if (!file) {
		perror("Failed to open /etc/alpine-release");
		return;
	}
	fgets(alpineRel, sizeof(alpineRel), file);
	fclose(file);

	strcpy(targetInfo->distroName, "Alpine Linux (WL-SWCOMP) ");
	strcat(targetInfo->distroName, alpineRel);
#endif

	strcpy(targetInfo->cpuArch, uts.machine);
}

static void printDeviceInfo(void) {
	int ret;

	puts("Please wait, handshaking...");
	ret = COM_Handshake();

	puts("Device Info:");
	puts("- Host device:");
	printf("  - Name: %s\n", comHostDeviceInfo.deviceModel);
	printf("  - CPU: %s (arch: %s)\n", comHostDeviceInfo.cpuName, comHostDeviceInfo.cpuArch);
	printf("  - RAM: %s\n", comHostDeviceInfo.memSize);
	printf("  - Kernel Version: %s\n", comHostDeviceInfo.kernelVer);
	printf("  - Alpine Release: %s\n", comHostDeviceInfo.distroName);

	puts("- Target device info:");
	if (ret != HANDSHAKE_OK) {
		printf("  - Error %d during handshake, cannot gather info\n", ret);
		return;
	}

	printf("  - Name: %s\n", comClientDeviceInfo.deviceModel);
	printf("  - CPU: %s (arch: %s)\n", comClientDeviceInfo.cpuName, comClientDeviceInfo.cpuArch);
	printf("  - RAM: %s\n", comClientDeviceInfo.memSize);
	printf("  - Kernel Version: %s\n", comClientDeviceInfo.kernelVer);
	printf("  - Distro: %s\n", comClientDeviceInfo.distroName);
}

#ifndef CLIENT_APP
void DEV_DisplayDeviceInfo(void *dummyArg) {
	inputEvent_t ev;

	(void)dummyArg;
	STATE_NoMenuRefresh = true;

	printf("\x1b[1;1H\x1b[2J");
	printDeviceInfo();
	puts("Press any key/button to return.");
	ev = INPUT_Wait();
	(void)ev;

	STATE_NoMenuRefresh = false;

	return;
}
#else
void DEV_DisplayDeviceInfo(void *dummyArg) {
	(void)dummyArg;

	printDeviceInfo();
}
#endif
