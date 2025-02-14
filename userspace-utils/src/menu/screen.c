#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int SCREEN_Brightness;

/* percentages of the screen brightness percentage for each dim level (to be multiplied by SCREEN_Brightness) */
static int dimLevelPercentages[5] = { 50, 25, 10, 5, 1 };

static void SCREEN_ApplyBrightness(int percent) {
	/* convert to 0-255 range (SCREEN_Brightness is a 0-100 percentage) */
	int val = (percent * 255) / 100;
	FILE *fp = fopen("/sys/class/backlight/backlight/brightness", "w");
	if (fp == NULL) {
		perror("Failed to open brightness file");
		return;
	}
	fprintf(fp, "%d", val);
	fclose(fp);

	/* if the brightness is 0, disable the screen */
	fp = fopen("/sys/class/backlight/backlight/bl_power", "w");
	if (fp == NULL) {
		perror("Failed to open bl_power file");
		return;
	}
	if (val >= 0)
		fprintf(fp, "0");
	else
		fprintf(fp, "1");
	fclose(fp);
}

void SCREEN_Enable(void) {
	SCREEN_ApplyBrightness(SCREEN_Brightness);
}

void SCREEN_Disable(void) {
	SCREEN_ApplyBrightness(0);
}

void SCREEN_Dim(int level) {
	/* dim according to dimLevelPercentages */
	assert(level >= 0 && level <= 4);

	SCREEN_ApplyBrightness((dimLevelPercentages[level] * SCREEN_Brightness) / 100);
}

void SCREEN_SetBrightness(int percent) {
	assert(percent >= 0 && percent <= 100);
	SCREEN_Brightness = percent;

	/* Apply the brightness immediately */
	SCREEN_ApplyBrightness(SCREEN_Brightness);
}
