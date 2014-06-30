#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTROL_CSI "\033["
#define CONTROL_OSC "\033]"
#define CONTROL_ST "\033\\"
#define CONTROL_BEL "\07"

enum class text_parameter { both = 0, tab = 1, window = 2 };

void die_explaining(char* program_name) {
	fprintf(stderr, "usage: %s [-p parameter] title\n"
			"\t-p [both/icon/tab/window] what parameter to set\n"
			, program_name);
	exit(1);
}

int main(int argc, char** argv) {
	text_parameter parameter_to_set = text_parameter::both;
	char* program_name = argv[0];

	struct option long_options[] = {
		{"param",            required_argument, 0, 'p'},
		{0, 0, 0, 0}
	};
	int option_index = 0;
	while (true) {
		int c = getopt_long(argc, argv, "p:", long_options, &option_index);
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
			case 'p': 
				if (strcmp("icon", optarg) == 0 || strcmp("tab", optarg) == 0) {
					parameter_to_set = text_parameter::tab;
					break;
				} else if (strcmp("window", optarg) == 0) {
					parameter_to_set = text_parameter::window;
					break;
				} else if (strcmp("both", optarg) == 0) {
					parameter_to_set = text_parameter::both;
					break;
				} else {
					// fallthough
				}
			default:
				die_explaining(program_name);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1) die_explaining(program_name);

	char new_title[1024];
	int offset = 0;
	for (int i = 0; i < argc; i++) {
		int argv_len = strlen(argv[i]);
		memcpy(new_title + offset, argv[i], argv_len);
		offset += argv_len;
		if (i != argc - 1) {
			new_title[offset] = ' ';
			offset++;
		}
	}

	new_title[offset] = '\0';
	printf(CONTROL_OSC "%d;%s" CONTROL_BEL, int(parameter_to_set), new_title);
	fflush(stdout);
	return 0;
}

