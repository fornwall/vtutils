#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#ifndef DATADIR
# define DATADIR "/Users/fornwall/src/gits/vtutils/data"
#endif

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{
        char line_buffer[100];
        int fd = open(DATADIR "/UnicodeData.txt", O_RDONLY);

        struct stat sb;
        fstat(fd, &sb);
        printf("Size: %lu\n", (unsigned long) sb.st_size);

        size_t memblock_len = sb.st_size;
        char const* memblock = (char const*) mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (memblock == MAP_FAILED) handle_error("mmap");

        int start_of_line = 0;
        int offset = 0;
        int current_line = 1;
        while (true) {
                while (offset < memblock_len && memblock[offset] != '\n') offset++;
                int line_length = offset - start_of_line;
                if (line_length + 1 >= sizeof(line_buffer)) {
                        fprintf(stderr, "%s: too large line %d in UnicodeData.txt\n", argv[0], current_line);
                        return 1;
                }
                memcpy(line_buffer, memblock + start_of_line, line_length);
                line_buffer[line_length] = 0;

                printf("LINE: %s\n", line_buffer);

                if (offset == memblock_len) break;
                start_of_line = offset + 1;
                current_line++;
                offset++;
                if (current_line > 20) break;
        }

        return 0;
}

