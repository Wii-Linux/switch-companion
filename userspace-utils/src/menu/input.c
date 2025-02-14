#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <termios.h>
#include <time.h>

#include "input.h"
#include "state.h"
#include "menu.h"
#include "screen.h"

static int pwrButtonFd;
static int volButtonFd;
static int touchFd;

static int kbdFds[MAX_KBD_DEVICES];
static int numKbdFds = 0;
static struct pollfd fds[3 + MAX_KBD_DEVICES];
/* static char kbdBuf[128]; */
static struct termios oldTerm;
static int currentlyTouched = 0;
static int touchCounter = 0;
static time_t lastTouch = 0;

#define TEST_KEY(k) (keybits[(k)/8] & (1 << ((k)%8)))
static int isKeyboard(const char *devPath) {
	unsigned long evbits;
	unsigned char keybits[KEY_MAX/8 + 1];
    int fd = open(devPath, O_RDONLY);
    if (fd < 0) return 0;

    evbits = 0;
    if (ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), &evbits) < 0) {
        close(fd);
        return 0;
    }

    /* Must support EV_KEY */
    if (!(evbits & (1 << EV_KEY))) {
        close(fd);
        return 0;
    }


    memset(keybits, 0, sizeof(keybits));
    if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybits)), keybits) < 0) {
        close(fd);
        return 0;
    }

    if (TEST_KEY(KEY_A) && TEST_KEY(KEY_ENTER)) {
        close(fd);
        return 1;  /* looks like a keyboard */
    }

    close(fd);
    return 0;
}


int INPUT_Init(void) {
	struct dirent *dp;
	DIR *dir;
	struct termios newTerm;

	pwrButtonFd = open(PWR_BUTTON, O_RDONLY);
	if (pwrButtonFd < 0) {
		perror("Error opening power button device");
		return 1;
	}

	volButtonFd = open(VOL_BUTTON, O_RDONLY);
	if (volButtonFd < 0) {
		perror("Error opening volume button device");
		close(pwrButtonFd);
		return 1;
	}

	touchFd = open(TOUCH_DEVICE, O_RDONLY);
	if (touchFd < 0) {
		perror("Error opening touchscreen device");
		close(pwrButtonFd);
		close(volButtonFd);
		return 1;
	}

	/* Set up poll structs for power and volume buttons and the touchscreen */
	fds[0].fd = pwrButtonFd;
	fds[0].events = POLLIN;

	fds[1].fd = volButtonFd;
	fds[1].events = POLLIN;

	fds[2].fd = touchFd;
	fds[2].events = POLLIN;

	/* check for keyboards */
	dir = opendir("/dev/input");

    while ((dp = readdir(dir)) != NULL) {
        if (strncmp(dp->d_name, "event", 5) == 0) {
            char fullpath[268]; /* d_name is 256 bytes */
            sprintf(fullpath, "/dev/input/%s", dp->d_name);

            if (isKeyboard(fullpath) && numKbdFds < MAX_KBD_DEVICES) {
                int fd = open(fullpath, O_RDONLY | O_NONBLOCK);
                if (fd >= 0) {
                    kbdFds[numKbdFds++] = fd;
                    fds[3 + numKbdFds - 1].fd = fd;
                    fds[3 + numKbdFds - 1].events = POLLIN;
                }
            }
        }
    }
    closedir(dir);


	/* save old terminal info */
	tcgetattr(STDIN_FILENO, &oldTerm);

	/* set it up so we can properly recieve arrow keys properly without needing a newline */
	/* breaks volume buttons??? */
	newTerm = oldTerm;
	newTerm.c_lflag &= ~(ICANON | ECHO);
	newTerm.c_cc[VMIN] = 1;
	newTerm.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &newTerm);

	return 0;
}

void INPUT_Shutdown(void) {
	if (pwrButtonFd >= 0) {
		close(pwrButtonFd);
	}
	if (volButtonFd >= 0) {
		close(volButtonFd);
	}
	if (touchFd >= 0) {
		close(touchFd);
	}

	/* restore old terminal settings */
	tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);
}

inputEvent_t INPUT_Wait(void) {
	int ret, i;
	int counter = 0;
	int dimmed = 0;
	int disabled = 0;
	struct input_event ev;

	while (1) {
		ret = poll(fds, 3 + numKbdFds, 5000);  /* wait for up to 5 seconds */

		if (ret < 0) {
			perror("poll");
			return INPUT_TYPE_ERROR;
		}

		/* Check for events */
		if (fds[0].revents & POLLIN) {
			read(pwrButtonFd, &ev, sizeof(ev));

			#ifdef INPUT_DEBUG
			printf("Event from power button: type=%d code=%d value=%d\n", ev.type, ev.code, ev.value);
			#endif

			if (ev.code == KEY_SLEEP && ev.value == 1) {
				/* Reset state on input */
				if (dimmed || disabled) {
					SCREEN_Enable();
					dimmed = 0;
					disabled = 0;
					counter = 0;
				}
				return INPUT_TYPE_SELECT;
			}
		}

		if (fds[1].revents & POLLIN) {
			read(volButtonFd, &ev, sizeof(ev));

			#ifdef INPUT_DEBUG
			printf("Event from volume buttons: type=%d code=%d value=%d\n", ev.type, ev.code, ev.value);
			#endif

			if ((ev.code == KEY_VOLUMEUP || ev.code == KEY_VOLUMEDOWN) && ev.value == 1) {
				/* Reset state on input */
				if (dimmed || disabled) {
					SCREEN_Enable();
					disabled = 0;
					dimmed = 0;
					counter = 0;
				}
				return (ev.code == KEY_VOLUMEUP) ? INPUT_TYPE_UP : INPUT_TYPE_DOWN;
			}
		}

		if (fds[2].revents & POLLIN) {
			time_t curTime = time(NULL);
			read(touchFd, &ev, sizeof(ev));

			#ifdef INPUT_DEBUG
			printf("Event from touchscreen: type=%d code=%d value=%d\n", ev.type, ev.code, ev.value);
			#endif

			if ((ev.code == ABS_MT_TRACKING_ID) && ev.value >= 0) {
				/* Reset state on tap */
				if (dimmed || disabled) {
					SCREEN_Enable();
					disabled = 0;
					dimmed = 0;
					counter = 0;
				}
				if (!currentlyTouched) {
					currentlyTouched = 1;

					/* If the last touch was more than 30 seconds ago, reset the touch counter */
					if (curTime - lastTouch > 30) {
						touchCounter = 0;
					}
					lastTouch = curTime;
					touchCounter++;
				}

				if (touchCounter == 10) {
					touchCounter = 0;
					if (!STATE_NoMenuRefresh) {
						puts("\x1b[1;1H\x1b[2J\x1b[1;31mThis app does not support touch input, stop trying.\x1b[0m\n");
						sleep(5);
						MENU_Update(STATE_CurMenu);
					}
				}
			}
			if ((ev.code == ABS_MT_TRACKING_ID) && ev.value == -1) {
				currentlyTouched = 0;
			}
		}

		for (i = 3; i < 2 + MAX_KBD_DEVICES; i++) {
			if (fds[i].revents & POLLIN) {
				read(kbdFds[i - 2], &ev, sizeof(ev));

				#ifdef INPUT_DEBUG
				printf("Event from keyboard: type=%d code=%d value=%d\n", ev.type, ev.code, ev.value);
				#endif

				if (ev.value == 1 && (ev.code == KEY_DOWN || ev.code == KEY_UP || ev.code == KEY_ENTER)) {
					/* Reset state on input */
					if (dimmed) {
						SCREEN_Enable();
						dimmed = 0;
						counter = 0;
					}
					if (disabled) {
						SCREEN_Enable();
						disabled = 0;
						counter = 0;
					}
					if (ev.code == KEY_DOWN) return INPUT_TYPE_DOWN;
					if (ev.code == KEY_UP) return INPUT_TYPE_UP;
					if (ev.code == KEY_ENTER) return INPUT_TYPE_SELECT;
				}
			}
		}

		/* No input received, increment counter */
		if (ret == 0) {
			counter++;

			/* Handle dimming and disabling screen */
			if (counter >= 10 && counter < 15) {
				SCREEN_Dim(counter - 10);
				dimmed = 1;
			} else if (counter == 15) {
				SCREEN_Disable();
				dimmed = 0;
				disabled = 1;
			}

			/* Update menu only if poll timer expired */
			if (STATE_CurMenu && !STATE_NoMenuRefresh) {
				MENU_Update(STATE_CurMenu);
			}
		}

		/* If screen is disabled, block indefinitely */
		if (disabled) {
			ret = poll(fds, 3, -1);  /* block indefinitely */
			if (ret < 0) {
				perror("poll");
				return INPUT_TYPE_ERROR;
			}
			counter = 0;  /* Reset counter on input */
			SCREEN_Enable();
		}
	}
}

void INPUT_DoAction(inputEvent_t act) {
	switch (act) {
		case INPUT_TYPE_SELECT: {
			menuItem_t *item = MENU_GetCurrentSelected(STATE_CurMenu);
			item->action(item->actionParam);
			break;
		}
		case INPUT_TYPE_UP: {
			if (STATE_CurMenu->selection > 0) {
				STATE_CurMenu->selection--;
			}
			break;
		}
		case INPUT_TYPE_DOWN: {
			if (STATE_CurMenu->selection < STATE_CurMenu->numItems - 1) {
				STATE_CurMenu->selection++;
			}
			break;
		}
		case INPUT_TYPE_KEYBOARD:
			break;
		case INPUT_TYPE_ERROR:
			break;
		case INPUT_TYPE_NONE:
	 		break;
	}
}
