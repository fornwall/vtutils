#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vtutils.h"

void request_termcap(char const* s)
{
	printf("\033P+q"); // Device Control String (DCS) and a +
	for (int i = 0; s[i] != 0; i++) {
		printf("%02x", (int) s[i]);
	}
	printf("\033\\"); // String Terminator (ST)
	fflush(stdout);
}

void read_and_echo()
{
	char buffer[65];
	int read_now = read(0, buffer, sizeof(buffer)-1);
	bool has_seen_equal = false;
	char hex_buffer[3] = {' ', ' ', '\0'};
	int hex_buffer_idx = 0;
	if (read_now > 0) {
		for (int i = 0; i < read_now; i++) {
			char c = buffer[i];
			if (c == '=') {
				has_seen_equal = true;
			} else if (has_seen_equal) {
				if (c == '\\') {
					printf("\n");
					return;
				}
				hex_buffer[hex_buffer_idx++] = c;
				if (hex_buffer_idx == 2) {
					long int value = strtol(hex_buffer, NULL, 16);
					if (value < 32) {
						if (value == 27) {
							printf("\\e");
						} else {
							printf("<%d>", (int) value);
						}
					} else {
						printf("%c", (char) value);
					}
					hex_buffer_idx = 0;
				}
			}
		}
	}
	printf("\n");
}

int main(int argc, char** argv)
{
	if (argc != 2 || (strlen(argv[1]) == 0) || argv[1][0] == '-') {
		fprintf(stderr, "Use: [appcursor=true] [appkeypad=true] %s [capability]\n", argv[0]);
		return 1;
	}
	TerminalRawMode rawMode;

	bool set_cursor_app = getenv("appcursor") != NULL;
	bool set_keypad_app = getenv("appkeypad") != NULL;

	if (set_cursor_app) printf("\033[?1h"); // CSI ? 1 h, Application Cursor Keys
	if (set_keypad_app) printf("\033[?66h"); // CSI ? 1 h, Application Cursor Keys

	request_termcap(argv[1]);
	read_and_echo();

	if (set_cursor_app) printf("\033[?1l");
	if (set_keypad_app) printf("\033[?66l"); // CSI ? 1 h, Application Cursor Keys

	return 0;
}
