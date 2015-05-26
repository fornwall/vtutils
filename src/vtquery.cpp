#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>

void request_termcap(char const* s)
{
	printf("\033P+q"); // Device Control String (DCS) and a +
	for (int i = 0; s[i] != 0; i++) {
		printf("%02x", (int) s[i]);
	}
	printf("\033\\"); // String Terminator (ST)
	fflush(stdout);
}

void print_color(char const* str) {
	printf("\033[1;31m%s\033[0m\n", str);
}

void read_and_echo() {
	char buffer[65];
	int read_now = read(0, buffer, sizeof(buffer)-1);
	if (read_now < 7 || buffer[0] != 033 || buffer[1] != 'P' || !(buffer[2] == '1' || buffer[2] == '0') || buffer[3] != '+' || buffer[4] != 'r') {
		fprintf(stderr, "Invalid %d byte response, not 'DCS + q XX XX ST'\n", read_now);
		return;
	}
	if (buffer[2] == '0') {
		printf("Terminal responded with invalid request\n");
		return;
	}

	for (int i = 5; i + 1 < read_now && buffer[i] != 033; i += 2) {
		int char_value;
		if (buffer[i] == '=') {
			i--;
			printf("=");
		} else {
			sscanf(buffer + i, "%02x", &char_value);
			if (char_value == 033) {
				printf("<ESC>");
			} else {
				printf("%c", (char) char_value);
			}
		}
	}
	printf("\n");
}

void decPrivateMode(unsigned int mode, bool set)
{
	printf("\033[?%u%c", mode, (set ? 'h' : 'l'));
}

int main(int argc, char** argv)
{
	struct option long_options[] = {
		{"app-cursors",      no_argument,       0, 'c'},
		{"help",    	     no_argument, 	0, 'h'},
		{"app-keypad",       no_argument,       0, 'k'},
		{0, 0, 0, 0}
	};
	bool set_cursor_app = false;
	bool set_keypad_app = false;

	int option_index = 0;
	while (true) {
		int c = getopt_long(argc, argv, "achklmns:tu", long_options, &option_index);
		if (c == -1) break;
		switch (c) {
			case 'c': set_cursor_app = true; break;
			case 'k': set_keypad_app = true; break;
			default:
				  fprintf(stderr, "usage: %s [-c] [-k] [name]\n"
						  "  -c    Set Application Cursor Keys (DECCKM)\n"
						  "  -k    Set Application keypad (DECNKM)\n"
						  , argv[0]);
				  return 1;
		}
	}
        argc -= optind;
        argv += optind;

	struct termios vt_orig;
	tcgetattr(0, &vt_orig); /* get the current settings */
	struct termios trm = vt_orig;
	trm.c_cc[VMIN] = 1; 	// Minimum number of characters for noncanonical read (MIN).
	trm.c_cc[VTIME] = 0; 	// Timeout in deciseconds for noncanonical read (TIME).
	// echo off, canonical mode off, extended input processing off, signal chars off:
	trm.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	tcsetattr(STDIN_FILENO, TCSANOW, &trm);

	if (set_cursor_app) decPrivateMode(1, true);
	if (set_keypad_app) decPrivateMode(66, true);
	fflush(stdout);

	request_termcap(argv[0]);
	read_and_echo();

	if (set_cursor_app) decPrivateMode(1, false);
	if (set_keypad_app) decPrivateMode(66, false);

	tcsetattr(0, TCSANOW, &vt_orig);
	fflush(stdout);

	return 0;
}
