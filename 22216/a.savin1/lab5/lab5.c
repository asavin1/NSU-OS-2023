#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 1024

void build(int fd, off_t **offsets, size_t *num_lines) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    off_t current_offset = 0;
    off_t *line_offsets = NULL;
    size_t line_count = 0;
    size_t line_capacity = 1;

    line_offsets = malloc(line_capacity * sizeof(off_t));
    if (!line_offsets) {
        perror("malloc failed");
        exit(-1);
    }

    line_offsets[line_count++] = 0;

    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\n') {
                if (line_count >= line_capacity) {
                    line_capacity *= 2;
                    line_offsets = realloc(line_offsets, line_capacity * sizeof(off_t));
                    if (!line_offsets) {
                        perror("realloc failed");
                        exit(-1);
                    }
                }
                line_offsets[line_count++] = current_offset + i + 1;
            }
        }
        current_offset += bytes_read;
    }

    if (bytes_read < 0) {
	perror("read failed");
        free(line_offsets);
        exit(-1);
    }

    if (line_count == 0 || line_offsets[line_count - 1] != current_offset) {
        if (line_count >= line_capacity) {
            line_capacity *= 2;
            line_offsets = realloc(line_offsets, line_capacity * sizeof(off_t));
            if (!line_offsets) {
                perror("realloc failed");
                exit(-1);
            }
        }
        line_offsets[line_count++] = current_offset;
    }

    *offsets = line_offsets;
    *num_lines = line_count - 1;
}

char *read_line(int fd, off_t offset) {
    char *line = NULL;
    size_t line_length = 0;
    size_t buffer_size = BUFFER_SIZE;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    if (lseek(fd, offset, SEEK_SET) < 0) {
        perror("lseek failed");
        exit(-1);
    }

    while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {
        char *newline = memchr(buffer, '\n', bytes_read);

        if (newline) {
            size_t line_part_length = newline - buffer + 1;
            line = realloc(line, line_length + line_part_length + 1);
            if (!line) {
                perror("realloc failed");
                exit(-1);
            }
            memcpy(line + line_length, buffer, line_part_length);
            line_length += line_part_length;
            line[line_length] = '\0';
            return line;
        }

        line = realloc(line, line_length + bytes_read);
        if (!line) {
            perror("realloc failed");
            exit(-1);
        }
        memcpy(line + line_length, buffer, bytes_read);
        line_length += bytes_read;
    }

    if (bytes_read < 0) {
        perror("read failed");
        free(line);
        exit(-1);
    }

    if (line_length > 0) {
        line = realloc(line, line_length + 1);
        if (!line) {
            perror("realloc failed");
            exit(-1);
        }
        line[line_length] = '\0';
        return line;
    }

    free(line);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        return -1;
    }

    off_t *line_offsets;
    size_t num_lines;

    build(fd, &line_offsets, &num_lines);

    while (1) {
        int line_number;
        printf("enter line number (0 to exit): ");
        if (scanf("%d", &line_number) != 1) {
            fprintf(stderr, "invalid input\n");
            close(fd);
            return -1;
        }
        if (line_number == 0) {
            break;
        }
        if (line_number < 1 || line_number > num_lines) {
            fprintf(stderr, "invalid line number\n");
            continue;
        }
        char *line = read_line(fd, line_offsets[line_number - 1]);
        if (line) {
            printf("%s", line);
            free(line);
        } else {
            fprintf(stderr, "error reading line\n");
        }
    }

    free(line_offsets);
    close(fd);
    return 0;
}

