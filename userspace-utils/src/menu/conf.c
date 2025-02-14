#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "featuresMenu.h"
#include "screen.h"

#define CONF_MAGIC "WL-SWCOMP CONFIG -- DO NOT MODIFY ANY PART OF THIS FILE DIRECTLY\n"
#define CONF_PATH "/boot/switchroot/wii-linux-switch-companion/config.txt"


static int CONF_Reset(void);
int CONF_Init(void) {
	char scratch[256];
	int scratchInt;
	char *ret;

	FILE *fp = fopen(CONF_PATH, "r");
	if (fp == NULL) {
		return CONF_Reset();
	}

	/* Line 1: magic value */
	ret = fgets(scratch, sizeof(scratch), fp);
	if (!ret || strncmp(scratch, CONF_MAGIC, sizeof(CONF_MAGIC) - 1) != 0) {
		fclose(fp);
		return CONF_Reset();
	}

	/* Line 2: RAMDisk enabled */
	ret = fgets(scratch, sizeof(scratch), fp);
	if (!ret) {
		fclose(fp);
		return CONF_Reset();
	}

	scratchInt = strtoul(scratch, NULL, 10);
	if (scratchInt < 0 || scratchInt > 1) {
		fclose(fp);
		return CONF_Reset();
	}
	ramdiskEnabled = scratchInt;

	/* Line 3: Networking enabled */
	ret = fgets(scratch, sizeof(scratch), fp);
	if (!ret) {
		fclose(fp);
		return CONF_Reset();
	}

	scratchInt = strtoul(scratch, NULL, 10);
	if (scratchInt < 0 || scratchInt > 1) {
		fclose(fp);
		return CONF_Reset();
	}
	networkingEnabled = scratchInt;

	/* Line 4: Unused feature 1 enabled */
	ret = fgets(scratch, sizeof(scratch), fp);
	if (!ret) {
		fclose(fp);
		return CONF_Reset();
	}

	scratchInt = strtoul(scratch, NULL, 10);
	if (scratchInt < 0 || scratchInt > 1) {
		fclose(fp);
		return CONF_Reset();
	}

	/* Line 5: Unused feature 2 enabled */
	ret = fgets(scratch, sizeof(scratch), fp);
	if (!ret) {
		fclose(fp);
		return CONF_Reset();
	}

	scratchInt = strtoul(scratch, NULL, 10);
	if (scratchInt < 0 || scratchInt > 1) {
		fclose(fp);
		return CONF_Reset();
	}

	/* Line 6: Screen brightness */
	ret = fgets(scratch, sizeof(scratch), fp);
	if (!ret) {
		fclose(fp);
		return CONF_Reset();
	}

	scratchInt = strtoul(scratch, NULL, 10);
	if (scratchInt < 0 || scratchInt > 100) {
		fclose(fp);
		return CONF_Reset();
	}

	SCREEN_Brightness = scratchInt;
	SCREEN_Enable();

	fclose(fp);
	return 0;
}

int CONF_Write(void) {
	FILE *fp = fopen(CONF_PATH, "w");
	if (fp == NULL) {
		return -1;  /* Error opening file */
	}

	fprintf(fp, CONF_MAGIC);
	fprintf(fp, "%d\n", ramdiskEnabled);
	fprintf(fp, "%d\n", networkingEnabled);
	fprintf(fp, "0\n"); /* Unused feature 1 */
	fprintf(fp, "0\n"); /* Unused feature 2 */
	fprintf(fp, "%d\n", SCREEN_Brightness);

	fclose(fp);
	return 0;
}

static int CONF_Reset(void) {
	ramdiskEnabled = 0;
	networkingEnabled = 0;
	SCREEN_Brightness = 100;  /* Default brightness */

	SCREEN_Enable();  /* Apply default brightness */
	return CONF_Write();  /* Write default configuration */
}
