#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>

struct program_options {
        bool only_codepoints{false};
        bool timestamp{false};
};

/* Assumes that the buffer is already validated. Does not handle overlong encodings. */
uint32_t utf8_sequence_to_codepoint(uint8_t* buffer, uint8_t length) 
{  
        uint32_t code_point;
        uint32_t first_byte_mask;
        switch (length) {
                case 2: first_byte_mask = 0x1F; break;
                case 3: first_byte_mask = 0x0F; break;
                case 4: first_byte_mask = 0x07; break;
                default: fprintf(stderr, "utf8_sequence_to_codepoint(): Invalid length %d", length); exit(1);
        }
        code_point = (buffer[0] & first_byte_mask);
        for (int i = 1; i < length; i++) code_point = ((code_point << 6) | (uint32_t) (buffer[i] & 0x3F));
        return code_point; 
}

void describe_byte(uint8_t byte, uint8_t* utf8_buffer, uint8_t* utf8_pos, uint8_t* remaining_utf8_continuation_bytes, program_options const& options)
{
        if (!options.only_codepoints) {
                printf("0%03o %3d 0x%02X ", (int) byte, (int) byte, (int) byte);
                switch (byte) {
                        case 0:   printf("Null character		^@	\\0"); break;
                        case 1:   printf("Start of Header		^A	 "); break;
                        case 2:   printf("Start of Text			^B	 "); break;
                        case 3:   printf("End of Text			^C	 "); break;
                        case 4:   printf("End of Transmission		^D	 "); break;
                        case 5:   printf("Enquiry			^E	 "); break;
                        case 6:   printf("Acknowledgement		^F	 "); break;
                        case 7:   printf("Bell				^G	 "); break;
                        case 8:   printf("Backspace			^H	\\b"); break;
                        case 9:   printf("Horizontal tab		^I	\\t"); break;
                        case 10:  printf("Line feed (^J, \\n)"); break;
                        case 11:  printf("Vertical tab			^K	\\v"); break;
                        case 12:  printf("Form feed			^L	\\f"); break;
                        case 13:  printf("Carriage return (^M, \\r"); break;
                        case 14:  printf("Start of Text			^N	  "); break;
                        case 15:  printf("Shift In			^O	  "); break;
                        case 16:  printf("Shift Out			^P	  "); break;
                        case 17:  printf("Device Control 1		^Q	  "); break;
                        case 18:  printf("Device Control 2		^R	  "); break;
                        case 19:  printf("Device Control 3		^S	  "); break;
                        case 20:  printf("Device Control 4		^T	  "); break;
                        case 21:  printf("Negative Acknowledgement	^U	  "); break;
                        case 22:  printf("Synchronous idle		^V	  "); break;
                        case 23:  printf("End of Transmission Block	^W	  "); break;
                        case 24:  printf("Cancel			^X	  "); break;
                        case 25:  printf("End of Medium			^Y	  "); break;
                        case 26:  printf("Substitute			^Z	  "); break;
                        case 27:  printf("Escape '^[' or '\\e'"); break;
                        case 28:  printf("File Separator		^\\	  "); break;
                        case 29:  printf("Record Separator		^]	  "); break;
                        case 30:  printf("Group Separator		^^	  "); break;
                        case 31:  printf("Unit Separator		^_	  "); break;
                        case 127: printf("Delete			^?	  "); break;
                }
        }

	bool invalid_utf8_seq = false; 
	if (byte <= 127) {
		if (byte >= 32 && byte <= 126) {
                        if (options.only_codepoints) {
                                printf("U+%04X", byte);
                        } else {
                                printf("ASCII '%c'", (char) byte);
                        }

                }
		invalid_utf8_seq = (*remaining_utf8_continuation_bytes > 0);
	} else {
		if (byte == /*0b11000000=*/192 || byte == /*0b11000001*/193) {
			printf("UTF-8 invalid byte (would lead to overlong encoding of 0-127 range)");
			invalid_utf8_seq = true;
		} else if (byte >= 245) {
			printf("UTF-8 invalid byte (if used as a starting byte would lead to value > 0x10FFFF)");
			invalid_utf8_seq = true;
		} else if ((byte & /*0b11000000=*/0xc0) == /*0b10000000=*/0x80) {
                        if (!options.only_codepoints) printf("UTF-8 continuation byte 0b10xxxxxx");

			invalid_utf8_seq = (*remaining_utf8_continuation_bytes == 0);
			if (!invalid_utf8_seq) {
				utf8_buffer[(*utf8_pos)++] = byte;
				--*remaining_utf8_continuation_bytes;
				if (*remaining_utf8_continuation_bytes == 0) {
					uint8_t used_length = *utf8_pos;

					utf8_buffer[used_length] = 0;
                                        uint32_t code_point = utf8_sequence_to_codepoint(utf8_buffer, used_length);
                                        if (options.only_codepoints) {
                                                printf("U+%04X", code_point);
                                        } else {
                                                if (code_point > 0x10FFFF) {
                                                        printf(" (resulting out of range value %u", code_point);
                                                } else if (((code_point <= 0x80) && used_length > 1)
                                                                || (code_point < 0x800 && used_length > 2)
                                                                || (code_point < 0x10000 && used_length > 3)) {
                                                        printf(" (resulting overlong encoding of value %u = 0x%04X)", code_point, code_point);
                                                } else {
                                                        printf(" (resulting code point %u = U+%04X = '%s')", code_point, code_point, utf8_buffer);
                                                }
                                        }
					*remaining_utf8_continuation_bytes = 0;
				}
			}
		} else {
			int expect_following;
			if        ((byte & /*0b11100000=*/0xe0) == /*0b11000000=*/0xc0) {
				expect_following = 1;
			} else if ((byte & /*0b11110000=*/0xf0) == /*0b11100000=*/0xe0) {
				expect_following = 2;
			} else if ((byte & /*0b11111000=*/0xf8) == /*0b11110000=*/0xf0) {
				expect_following = 3;
			} else {
				expect_following = -1;
				printf("Invalid UTF-8 byte");
				invalid_utf8_seq = true;
			}
			if (expect_following != -1) {
                                if (!options.only_codepoints) {
                                        printf("UTF-8 leading byte      %s for a %d byte sequence",
                                                        (expect_following == 1) ? "0b110xxxxx" :
                                                        ((expect_following == 2) ? "0b1110xxxx" : "0b11110xxx"),
                                                        expect_following + 1);
                                }
				if (*remaining_utf8_continuation_bytes == 0) {
					*remaining_utf8_continuation_bytes = expect_following;
					*utf8_pos = 1;
					utf8_buffer[0] = byte;
				} else {
					invalid_utf8_seq = true;
				}
			}
		}
	}

	if (invalid_utf8_seq) {
		printf(" (aborting invalid UTF-8 sequence)");
		*remaining_utf8_continuation_bytes = 0;
	}
}

void read_and_echo(program_options const& options) {
	uint8_t remaining_utf8_continuation_bytes = 0;
	uint8_t utf8_pos = 0;
	uint8_t utf8_buffer[5];

	int64_t initial_timestamp = -1;
	if (options.timestamp) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		initial_timestamp = tv.tv_sec*1000 + tv.tv_usec/1000;
	}

	bool first = true;
	while (true) {
		unsigned char read_buffer[256];
		int read_now = read(0, read_buffer, sizeof(read_buffer) - 1);
		if (read_now == 0) {
			return;
		} else if (read_now < 0) {
			perror("read()");
			return;
		}
		if (first) {
			first = false;
		} else {
			printf("\n");
		}
		if (options.timestamp) {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			int64_t elapsed = (tv.tv_sec*1000 + tv.tv_usec/1000) - initial_timestamp;
			printf("%lld ms\n", elapsed);
		}

		for (int i = 0; i < read_now; i++) {
			uint8_t c = read_buffer[i];
			describe_byte(c, utf8_buffer, &utf8_pos, &remaining_utf8_continuation_bytes, options);
			/* Let the user exit on ctrl+c or ctrl+d, or on end of file. */
			printf("\n");
			if (c == 3 || c == 4) return;
		}
		fflush(stdout);
	}
}

void decPrivateMode(unsigned int mode, bool set)
{
	printf("\033[?%u%c", mode, (set ? 'h' : 'l'));
}

int main(int argc, char** argv)
{
	int verbose_flag;
	struct option long_options[] = {
		/* These options set a flag. */
		{"verbose",       no_argument,       &verbose_flag, 1},
		{"brief",         no_argument,       &verbose_flag, 0},
		/* These options don't set a flag.  We distinguish them by their indices. */
		{"alternate-screen", no_argument,       0, 'a'},
		{"app-cursors",      no_argument,       0, 'c'},
		{"help",    	     no_argument, 	0, 'h'},
		{"app-keypad",       no_argument,       0, 'k'},
		{"line-input",       no_argument,       0, 'l'},
		{"mouse",            no_argument,       0, 'm'},
		{"keep-newlines",    required_argument, 0, 'n'},
		{"timestamp",        no_argument,       0, 't'},
		{0, 0, 0, 0}
	};

	bool alternate_screen = false;
	bool set_cursor_app = false;
	bool set_keypad_app = false;
	bool line_input = false;
	bool raw_newlines = false;
        struct program_options options;
        bool mouse_mode = false;
        char const* send_option = NULL;

	while (true) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "achklmns:tu", long_options, &option_index);
		if (c == -1) break;
		switch (c) {
			case 't': options.timestamp = true; break;
			case 'a': alternate_screen = true; break;
			case 'c': set_cursor_app = true; break;
			case 'k': set_keypad_app = true; break;
			case 'l': line_input = true; break;
			case 'm': mouse_mode = true; break;
			case 'n': raw_newlines = true; break;
			case 's': send_option = optarg; break;
			case 'u': options.only_codepoints = true; break;
			default:
				  fprintf(stderr, "usage: %s [-c] [-k] [-l] [-m mousemode] [-n]\n"
						  "  -c    Set Application Cursor Keys (DECCKM)\n"
						  "  -k    Set Application keypad (DECNKM)\n"
						  "  -l    Enable line input mode instead of raw\n"
						  "  -m    Enable terminal mouse tracking\n"
						  "  -n    Do not translate CR or NL in terminal driver\n"
						  "  -s    Send the specified escape sequence (such as $'\\033[...')\n"
						  "  -t    Show a timestamp after each input read\n"
                                                  "  -u    Only show unicode code points\n"
						  , argv[0]);
				  return 1;
		}
	}


	struct termios vt_orig;
	struct termios vt_new;
	bool is_tty = isatty(STDIN_FILENO);
	if (is_tty) {
		tcgetattr(0, &vt_orig);
		vt_new = vt_orig;
		vt_new.c_cc[VMIN] = 1; 	// Minimum number of characters for noncanonical read (MIN).
		vt_new.c_cc[VTIME] = 0; 	// Timeout in deciseconds for noncanonical read (TIME).
		// echo off, canonical mode off, extended input processing off, signal chars off:
		vt_new.c_lflag &= ~((line_input ? 0 :  (ECHO | ICANON))
				| IEXTEN | ISIG);
		if (raw_newlines) {
			vt_new.c_oflag &= ~OCRNL;
			vt_new.c_iflag &= ~ICRNL;
		}
		tcsetattr(STDIN_FILENO, TCSANOW, &vt_new);
	} else if (set_cursor_app || set_keypad_app || mouse_mode || alternate_screen) {
		fprintf(stderr, "Not connected to a terminal %d, %d, %d, %d!\n",
				(int) set_cursor_app,
				(int) set_keypad_app,
				(int) mouse_mode,
				(int) alternate_screen
		       );
		return 1;
	}

	if (send_option != NULL) { printf("%s", send_option); }

	if (set_cursor_app) decPrivateMode(1, true);
	if (set_keypad_app) decPrivateMode(66, true);
	if (mouse_mode) { decPrivateMode(1000, true); decPrivateMode(1002, true); decPrivateMode(1006, true); }
	if (alternate_screen) decPrivateMode(1049, true);
	fflush(stdout);

	read_and_echo(options);

	// Cleanup.
	if (set_cursor_app) decPrivateMode(1, false);
	if (set_keypad_app) decPrivateMode(66, false);
	if (mouse_mode > 0) { decPrivateMode(1000, false); decPrivateMode(1006, false); }
	if (alternate_screen) decPrivateMode(1049, false);
	if (is_tty) tcsetattr(0, TCSANOW, &vt_orig);

	return 0;
}
