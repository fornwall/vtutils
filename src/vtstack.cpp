#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTROL_CSI "\033["
#define CONTROL_OSC "\033]"
#define CONTROL_ST "\033\\"
#define CONTROL_BEL "\07"

enum class text_parameter { icon = 1, window = 2 };

void die_explaining(char* program_name) {
	fprintf(stderr, "usage: %s\n" 
			"\t-i, --icon         operate on icon instead of window title\n"
			"\t-r, --restore     restore title from stack\n"
			"\t-s, --save        save title on stack\n"
			, program_name);
	exit(1);
}

int main(int argc, char** argv) {
	text_parameter parameter_to_set = text_parameter::window;
	char* program_name = argv[0];
	bool do_save = false;
	bool do_restore = false;

	struct option long_options[] = {
		{"icon", required_argument, 0, 'i'},
		{"restore", no_argument, 0, 'r'},
		{"save", no_argument, 0, 's'},
		{0, 0, 0, 0}
	};
	int option_index = 0;
	while (true) {
		int c = getopt_long(argc, argv, "irc", long_options, &option_index);
		if (c == -1) break;
		switch (c) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				printf ("option %s", long_options[option_index].name);
				if (optarg) printf (" with arg %s", optarg);
				printf ("\n");
				break;
			case 'i': 
				parameter_to_set = text_parameter::icon;
				break;
			case 'r':
				do_restore = true;
				break;
			case 's':
				do_save = true;
				break;
			default:
				die_explaining(program_name);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 0) die_explaining(program_name);

	if (do_save == do_restore) die_explaining(program_name);

	printf(CONTROL_CSI "%d;%dt", (do_save ? 22 : 23), int(parameter_to_set));
	fflush(stdout);
	return 0;
}

