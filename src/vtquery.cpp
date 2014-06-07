#include <unistd.h>
#include <stdio.h>
#include "vtutils.h"

void request_termcap(char const* s)
{
	printf("*** Device Control (DCS) for '%s'- 'ESC P + q ", s);
	for (int i = 0; s[i] != 0; i++) { 
		if (i != 0) printf(" ");
		printf("%02x", (int) s[i]);
	}
	printf(" ESC \\'");
	printf("\033P+q"); // Device Control String (DCS) and a +
	for (int i = 0; s[i] != 0; i++) {
		printf("%02x", (int) s[i]);
	}
	printf("\033\\"); // String Terminator (ST)
	printf("\nResponse:");
	fflush(stdout);
}

void print_color(char const* str) {
	printf("\033[1;31m%s\033[0m\n", str);
}

void read_and_echo() {
	char buffer[65];
	int read_now = read(0, buffer, sizeof(buffer)-1);
	if (read_now > 0) {
		for (int i = 0; i < read_now; i++) {
			char c = buffer[i];
			if (c < 32) {
				printf(" <%d> ", (int) c);
			} else {
				printf(" %c ", c);
			}
		}
		printf("\n");
		fflush(stdout);
	}
}

int main(int argc, char** argv)
{
	TerminalRawMode rawMode;

	printf("*** Send Device Attributes (Primary DA) - 'ESC [ 0 c'\033[0c\nResponse:"); fflush(stdout); 
	read_and_echo();

	printf("*** Send Device Attributes (Secondary DA) - 'ESC [ > 0 c'\033[>0c\nResponse:"); fflush(stdout); 
	read_and_echo();

	printf("*** Request DEC private mode (DECRQM) - 'ESC [ ? 1 $ p'\033[?1$p\nResponse:"); fflush(stdout);
	read_and_echo();

	//print_color("HELLO WORLD");
	request_termcap("ku");
	read_and_echo();

	printf("*** (Setting Application Cursor Keys)\n");
	printf("\033[?1h"); // CSI ? 1 h, Application Cursor Keys
	fflush(stdout);

	printf("*** Request DEC private mode (DECRQM) - 'ESC [ ? 1 $ p'\033[?1$p\nResponse:"); fflush(stdout);
	read_and_echo();

	request_termcap("ku");
	read_and_echo();

	printf("*** (Setting Normal Cursor Keys)\n");
	printf("\033[?1l"); // CSI ? 1 l, Normal Cursor Keys
	fflush(stdout);

	request_termcap("Co");
	read_and_echo();

	request_termcap("colors");
	read_and_echo();

	request_termcap("TN");
	read_and_echo();

	request_termcap("name");
	read_and_echo();

	request_termcap("#2");
	read_and_echo();

	request_termcap("#4");
	read_and_echo();

	request_termcap("%i");
	read_and_echo();

	request_termcap("*7");
	read_and_echo();

	request_termcap("kB");
	read_and_echo();

	request_termcap("K1");
	read_and_echo();

	//printf("\033P");
	printf("Bye\n");
	return 0;
}
