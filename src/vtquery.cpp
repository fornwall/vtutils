#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

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
		if (buffer[0] != 033) fprintf(stderr, "First char wrong (should be escape)\n");
		if (buffer[1] != 'P') fprintf(stderr, "WRONG SECOND\n");
		if (buffer[2] != '1') fprintf(stderr, "WRONG THIRD\n");
		if (buffer[3] != '+') fprintf(stderr, "WRONG FOURTH\n");
		if (buffer[4] != 'r') fprintf(stderr, "WRONG FIFTH\n");
		return;
	}
	if (buffer[2] == '0') {
		printf("Terminal responded with invalid request\n");
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

int main(int argc, char** argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: vtquery <capability>\n");
		return 1;
	}

	struct termios vt_orig;
	tcgetattr(0, &vt_orig); /* get the current settings */
	struct termios trm = vt_orig;
	trm.c_cc[VMIN] = 1; 	// Minimum number of characters for noncanonical read (MIN).
	trm.c_cc[VTIME] = 0; 	// Timeout in deciseconds for noncanonical read (TIME).
	// echo off, canonical mode off, extended input processing off, signal chars off:
	trm.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	tcsetattr(STDIN_FILENO, TCSANOW, &trm);

	request_termcap(argv[1]);
	read_and_echo();

	tcsetattr(0, TCSANOW, &vt_orig);
	fflush(stdout);

	return 0;
}
