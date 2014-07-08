#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CONTROL_CSI "\033["
#define CONTROL_OSC "\033]"
#define CONTROL_ST "\033\\"
#define CONTROL_BEL "\07"

enum class text_parameter { both = 0, tab = 1, window = 2 };

void die_explaining(char* program_name) {
        fprintf(stderr, "Usage: %s [-b] [-i] [-s] <title>\n"
                        "   or: %s -r\n"
                        "   or: %s [-t <title>] -w <command-to-execute>\n"
                        "\n"
                        "Options:\n"
                        "  -b, --both               set both icon name and window title\n"
                        "  -i, --icon               set icon name instead of window title\n"
                        "  -r, --restore            restore/pop title from stack\n"
                        "  -s, --save               save/push title to stack for setting new title\n"
                        "  -t, --temporary-title    title while executing\n"
                        "  -w, --while-executing    show title while executing, then restore\n"
                        , program_name, program_name, program_name);
        exit(1);
}

int main(int argc, char** argv) {
	text_parameter parameter_to_set = text_parameter::window;
	char* program_name = argv[0];

	struct option long_options[] = {
		{"both",  required_argument, 0, 'b'},
		{"help",  required_argument, 0, 'h'},
		{"icon",  required_argument, 0, 'i'},
		{"restore", no_argument, 0, 'r'},
		{"save", no_argument, 0, 's'},
		{"temporary-title", no_argument, 0, 't'},
		{"while-executing", no_argument, 0, 'w'},
		{0, 0, 0, 0}
	};
	int option_index = 0;
        bool do_save = false;
        bool do_restore = false;
        bool do_execute = false;
        char const* temporary_title = NULL;
	while (true) {
                int c = getopt_long(argc, argv, "+bhirst:w", long_options, &option_index);
                if (c == -1) break;
                switch (c) {
                        case 'b': 
                                parameter_to_set = text_parameter::both;
                                break;
                        case 'i': 
                                parameter_to_set = text_parameter::tab;
                                break;
                        case 'r':
                                do_restore = true;
                                break;
                        case 's':
                                do_save = true;
                                break;
                        case 't':
                                temporary_title = optarg;
                                break;
                        case 'w':
                                do_execute = true;
                                break;
                        default:
                                die_explaining(program_name);
                }
                if (c == 'w') break;
        }
        argc -= optind;
        argv += optind;

        if (do_restore && do_save) {
                fprintf(stderr, "cannot both save and restore\n");
                return 1;
        } else if (do_restore && argc > 0) {
                fprintf(stderr, "cannot combine restoring with a title\n");
                return 1;
        } else if (argc < 1 && !(do_save || do_restore)) {
                fprintf(stderr, "requires a title\n");
                return 1;
        }

        if (do_execute) do_save = true;

        if (do_save || do_restore) printf(CONTROL_CSI "%d;%dt", (do_save ? 22 : 23), int(parameter_to_set));
        if (do_restore || argc == 0) return 0;

	char new_title_buffer[1024];
        char const* new_title;
        if (temporary_title) {
                new_title = temporary_title;
        } else {
                new_title = new_title_buffer; 
                unsigned int offset = 0;
                for (int i = 0; i < argc; i++) {
                        int argv_len = strlen(argv[i]);
                        if (argv_len + offset > sizeof(new_title_buffer)) {
                                fprintf(stderr, "Too large window title - only %d bytes allowed\n", (int) sizeof(new_title_buffer));
                                return 1;
                        }
                        memcpy(new_title_buffer + offset, argv[i], argv_len);
                        offset += argv_len;
                        if (i != argc - 1) {
                                new_title_buffer[offset] = ' ';
                                offset++;
                        }
                }
                new_title_buffer[offset] = '\0';
        }

	printf(CONTROL_OSC "%d;%s" CONTROL_BEL, int(parameter_to_set), new_title);
	fflush(stdout);

        if (do_execute) {
                pid_t pid = fork();
                if (pid < 0) {
                        perror("fork()");
                        return 1;
                } else if (pid == 0) {
                        execvp(argv[0], argv);
                        fprintf(stderr, "%s - ", argv[0]);
                        perror("");
                        exit(1);
                } else {
                        int status;
                        wait(&status);
                        printf(CONTROL_CSI "23;%dt", int(parameter_to_set));
                        return (WIFEXITED(status)) ? WEXITSTATUS(status) : 1;
                }
        }

	return 0;
}

