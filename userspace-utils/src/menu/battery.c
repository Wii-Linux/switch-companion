#include <stdio.h>
#include <string.h>
#define BAT_PATH "/sys/class/power_supply/battery"

int BAT_GetPercent(void) {
	/*
	 * the Swtich's battery is always at a static path
	 * grab the /capacity of it, and return it as an int
	 */
	int capacity;
	FILE *fp = fopen(BAT_PATH "/capacity", "r");

	if (fp == NULL)
		return -1;
	if (fscanf(fp, "%d", &capacity) != 1) {
		fclose(fp);
		return -1;
	}
	return capacity;
}

int BAT_GetChargeRate(void) {
	/* grab the /current_now of it, and return it as an int */
	int current;
	FILE *fp = fopen(BAT_PATH "/current_now", "r");
	char status[16];

	if (fp == NULL)
		return -1;
	if (fscanf(fp, "%d", &current) != 1) {
		fclose(fp);
		return -1;
	}
	fclose(fp);

	/*
	 * check the charging status (/status)
	 * if it's "Discharging", and the current is positive,
	 * make it negative
	 */
	fp = fopen(BAT_PATH "/status", "r");
	if (fp == NULL) {
		return current; /* return the current as is if we can't read status */
	}
	if (fscanf(fp, "%15s", status) != 1) {
		fclose(fp);
		return current; /* return the current as is if we can't read status */
	}
	fclose(fp);
	if (strcmp(status, "Discharging") == 0 && current > 0) {
		current = -current; /* make it negative if discharging */
	}

	/* divide by 1000 to convert from microamps to milliamps */
	current /= 1000;
	return current;
}
