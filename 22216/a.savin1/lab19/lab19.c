#define  _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>

void closedir_or_exit(DIR *dir) {
    if (closedir(dir) == -1) {
	perror("closedir failed");
	exit(-1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <pattern>\n", argv[0]);
	return -1;
    }

    if (strchr(argv[1], '/') != NULL) {
	fprintf(stderr, "pattern can not contain /\n");
	return -1;
    }

    DIR *dir;

    dir = opendir(".");
    if (dir == NULL) {
 	perror("opendir failed");
	return -1;
    }

    int found = 0;
    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(dir)) != NULL) {
	if (fnmatch(argv[1], entry->d_name, 0) == 0) {
	    printf("%s\n", entry->d_name);
	    found = 1;
	}
    }

    if (errno != 0) {
   	perror("readdir failed");
  	closedir_or_exit(dir);
 	return -1;
    }

    if (!found) {
	printf("failed to find files matching the template: %s\n", argv[1]);
    }

    closedir_or_exit(dir);
    return 0;
}
