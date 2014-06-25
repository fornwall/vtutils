#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

long parse_code_point(char* s)
{
        if (strlen(s) > 2 && s[0] == 'U' && s[1] == '+') {
                s[0] = '0';
                s[1] = 'x';
        }
        return strtol(s, NULL, 0);
}

void die_explaining(char** argv)
{
        fprintf(stderr, "usage: %s [-h] [-s] codepoint [codepoints]\n", argv[0]);
        exit(1);
}

int main(int argc, char** argv)
{
        bool allow_surrogates = false;
        bool human_readable = false;

        int getopt_result;
        opterr = 0;
        while ((getopt_result = getopt (argc, argv, "hs")) != -1)
                switch (getopt_result) {
                        case 'h':
                                human_readable = true;
                                break;
                        case 's':
                                allow_surrogates = true;
                                break;
                        case '?':
                                fprintf(stderr, "%s: illegal option -- %c\n", argv[0], optopt);
                                die_explaining(argv);
                                return 1;
                        default:
                                die_explaining(argv);
                }

        if (argc == optind) die_explaining(argv);

        for (int i = optind; i < argc; i++) {
                long c = parse_code_point(argv[i]);
                if (c < 0 || c > 1114111) {
                        // 1114111 (= 2^16 + 1024^2 - 1) is the max code point:
                        //   2^16: From 16 bit
                        //   1024^2: From combining the 1024 high surrogates with the 1024 low
                        //   -1: Zero based
                        fprintf(stderr, "%s: invalid code point '%s' - specify a number in the range [1-1114111]\n", argv[0], argv[i]);
                        return 1;
                }
                if (c >= 0xD800 && c <= 0xDBFF) {
                        fprintf(stderr, "%s: invalid high surrogate %s\n", argv[0], argv[i]);
                        return 1;
                }
                if (c >= 0xDC00 && c <= 0xDFFF) {
                        fprintf(stderr, "%s: invalid low surrogate %s\n", argv[0], argv[i]);
                        return 1;
                }
        }

        for (int i = optind; i < argc; i++) {
                long code_point = strtol(argv[i], NULL, 0);
                uint8_t buffer[5];
                uint8_t buffer_length = 0;

                if (code_point <= /* 7 bits */ 0b1111111) {
                        buffer_length = 1;
                        buffer[0] = code_point;
                } else if (code_point <= /* 11 bits */ 0b11111111111) {
                        buffer_length = 2;
                        // 110xxxxx leading five bits, followed by a 10xxxxxx byte
                        buffer[0] = 0b11000000 | (code_point >> 6);
                        buffer[1] = 0b10000000 | (code_point & 0b00111111);
                } else if (code_point <= /* 16 bits */ 0b1111111111111111) {
                        buffer_length = 3;
                        // 1110xxxx leading four bits, bollowed by two 10xxxxxx bytes
                        buffer[0] = 0b11100000 | (code_point >> 12);
                        buffer[1] = 0b10000000 | ((code_point >> 6) & 0b00111111);
                        buffer[2] = 0b10000000 | (code_point & 0b00111111);
                } else if (code_point <= /* 21 bits */ 0b111111111111111111111) {
                        buffer_length = 4;
                        // 11110xxx leading three bits, bollowed by three 10xxxxxx bytes
                        buffer[0] = 0b11110000 | (code_point >> 18);
                        buffer[1] = 0b10000000 | ((code_point >> 12) & 0b00111111);
                        buffer[2] = 0b10000000 | ((code_point >> 6) & 0b00111111);
                        buffer[3] = 0b10000000 | (code_point & 0b00111111);
                }
                buffer[buffer_length] = 0;
                if (human_readable) {
                        for (int j = 0; buffer[j] != 0; j++) {
                                printf("%s0x%X", ((j == 0 && i == optind) ? "" : " "), (int) buffer[j]);
                        }
                } else {
                        printf("%s", buffer);
                }
        }
        if (isatty(1)) printf("\n");
        return 0;
}


