/* ini.h example that simply dumps an INI file without comments */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ini.h"

#define MAX_SECTION_LEN 50

static bool dumper(IniState *is, const char *section, const char *name,
                   const char *value) {
    char *prev_section = (char *)is->user;
    if (strcmp (section, prev_section)) {
        printf ("%s[%s]\n", (prev_section[0] ? "\n" : ""), section);
        strncpy (prev_section, section, MAX_SECTION_LEN);
        prev_section[MAX_SECTION_LEN - 1] = '\0';
    }
    printf ("%s = %s\n", name, value ? value : "");
    return true;
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf (stderr, "Usage: ini_dump filename.ini\n");
        return 1;
    }

    char *contents = ini_slurp (argv[1], NULL);
    if (!contents) {
        fprintf (stderr, "Can't read '%s'!\n", argv[1]);
        return 2;
    }
    char prev_section[MAX_SECTION_LEN] = "";
    IniState is = { .handler = dumper, .user = prev_section };
    bool ok = ini_parse_string (&is, contents);
    free (contents);
    if (!ok) {
        fprintf (stderr, "Bad config file (first error on line %d)!\n", is.lineno);
        return 3;
    }
    return 0;
}
