/* ini.h example that simply dumps an INI file without comments */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ini.h"

#define MAX_SECTION_LEN 50

static int dumper(void* user, const char* section, const char* name,
                  const char* value) {
	char *prev_section = (char *)user;
    if (strcmp(section, prev_section)) {
        printf("%s[%s]\n", (prev_section[0] ? "\n" : ""), section);
        strncpy(prev_section, section, MAX_SECTION_LEN);
        prev_section[MAX_SECTION_LEN - 1] = '\0';
    }
    printf("%s = %s\n", name, value);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Usage: ini_dump filename.ini\n");
        return 1;
    }

    char *contents = ini_slurp(argv[1], NULL);
    if (!contents) {
        fprintf(stderr, "Can't read '%s'!\n", argv[1]);
        return 2;
    }
    char prev_section[MAX_SECTION_LEN] = "";
    int error = ini_parse_string(contents, dumper, prev_section);
    free(contents);
    if (error) {
        fprintf(stderr, "Bad config file (first error on line %d)!\n", error);
        return 3;
    }
    return 0;
}
