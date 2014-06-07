#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	bool human_readable = argc == 3 && (strcmp(argv[1], "-d") == 0);
	if (argc < 2 || (argc == 2 && human_readable)) {
		fprintf(stderr, "usage: %s [-d] codepoint ...\n", argv[0]);
		return 1;
	}

	int start_index = human_readable ? 2 : 1;

	for (int i = start_index; i < argc; i++) {
		long c = strtol(argv[i], NULL, 0);
		if (c == 0 || c >= 0x110000) {
			fprintf(stderr, "Error: Invalid code point '%s' - specify a number in the range [1-1114112]\n", argv[i]);
			return 1;
		}
		if (c >= 0xD800 && c <= 0xDFFF) {
			fprintf(stderr, "Error: Invalid surrogate code code point '%s'\n", argv[i]);
			return 1;
		}
	}

	for (int i = start_index; i < argc; i++) {
		long c = strtol(argv[i], NULL, 0);
		uint8_t buffer[5];
		uint8_t* b = buffer;

		if (c < 0x80) {
			*b++ = c;
		} else if (c<0x800) {
			*b++ = 192+c/64;
			*b++=128+c%64;
		} else if (c >= 0xD800 && c <= 0xDFFF) {
			fprintf(stderr, "Error: Surrogate code point reserved use by UTF-16: '%s'.\n", argv[i]);
			return 1;
		} else if (c<0x10000) {
			*b++=224+c/4096;
			*b++=128+c/64%64;
			*b++=128+c%64;
		} else if (c<0x110000) {
			*b++=240+c/262144;
			*b++=128+c/4096%64;
			*b++=128+c/64%64;
			*b++=128+c%64;
		} else {
			fprintf(stderr, "Error\n");
			return 1;
		}
		uint64_t len = b - buffer;
		b[len] = 0;
		if (human_readable) {
			for (int j = 0; buffer[j] != 0; j++) {
				printf("%s0x%X", (j == 0 ? "" : " "), (int) buffer[j]);
			}
		} else {
			printf("%s", buffer);
		}
	}
	if (isatty(0)) printf("\n");
	return 0;
}


