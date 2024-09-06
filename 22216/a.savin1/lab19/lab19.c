#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#include <unistd.h>


int main() {
    long name_max = pathconf(".", _PC_NAME_MAX);
    if (name_max == -1) {
        perror("failed to get max length of filename");
        return -1;
    }
    char pattern[name_max + 1];

    printf("enter a template for the filename: ");
    if (fgets(pattern, sizeof(pattern), stdin) == NULL) {
        perror("fgets failed");
        return -1;
    }

    size_t len = strlen(pattern);
    if (len > 0 && pattern[len - 1] == '\n') {
        pattern[len - 1] = '\0';
    }

    if (strchr(pattern, '/') != NULL) {
        fprintf(stderr, "pattern can not contain /\n");
        return -1;
    }

    struct dirent *entry;
    DIR *dir;

    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir failed");
        return -1;
    }

    int found = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (fnmatch(pattern, entry->d_name, 0) == 0) {
            printf("%s\n", entry->d_name);
            found = 1;
        }
    }
    closedir(dir);

    if (!found) {
        printf("failed to find files matching the template: %s\n", pattern);
    }

    return 0;
}
