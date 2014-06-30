// Defining _XOPEN_SOURCE is necessary for sigaction(2) on linux
#define _XOPEN_SOURCE
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define VTSIZE_SIZE_ALL 1
#define VTSIZE_SIZE_ONLY_ROWS 2
#define VTSIZE_SIZE_ONLY_COLS 3

static int what;

static void print_terminal_size()
{
	struct winsize termSize;
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char*) &termSize) < 0) perror("ioctl() failed");
	switch (what) {
		case VTSIZE_SIZE_ONLY_ROWS:
			printf("%d\n", (int) termSize.ws_row);
			break;
		case VTSIZE_SIZE_ONLY_COLS:
			printf("%d\n", (int) termSize.ws_col);
			break;
		default:
			printf("%d %d\n", (int) termSize.ws_row, (int) termSize.ws_col);
			break;
	}
}

int main(int argc, char** argv) 
{
	bool watch = false;
	what = VTSIZE_SIZE_ALL;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0) {
			if (what == VTSIZE_SIZE_ONLY_ROWS) {
				fprintf(stderr, "%s: cannot combine -c and -r\n", argv[0]);
				return 1;
			}
			what = VTSIZE_SIZE_ONLY_COLS;
		} else if (strcmp(argv[i], "-r") == 0) {
			if (what == VTSIZE_SIZE_ONLY_COLS) {
				fprintf(stderr, "%s: cannot combine -c and -r\n", argv[0]);
				return 1;
			}
			what = VTSIZE_SIZE_ONLY_ROWS;
		} else if (strcmp(argv[i], "-w") == 0) {
			watch = true;
		} else {
			fprintf(stderr, "%s: illegal option '%s'\nusage: %s [-c] [-r] [-w]\n", argv[0], argv[i], argv[0]);
			return 1;
		}

	}
	if (!isatty(STDIN_FILENO)) {
		fprintf(stderr, "stdin is not a TTY\n");
		return 0;
	}

	print_terminal_size();

	if (watch) {
		struct sigaction newSigAction= { .sa_handler = print_terminal_size, .sa_mask = 0, .sa_flags = 0 };
		struct sigaction oldSigAction;
		if (sigaction(SIGWINCH, &newSigAction, &oldSigAction) < 0) {
			perror("sigaction() failed");
			return 1;
		}
		// Sleep forever, catching signals as required.
		while (1) sleep(100);
	}
}


