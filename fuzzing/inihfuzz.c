/* This is a slightly tweaked copy of tests/unittest.c for fuzzing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ini.h"

int User;
char Prev_section[50];

bool dumper(IniState *is, const char *section, const char *name,
            const char *value)
{
    User = *((int*)is->user);
    if (strcmp (section, Prev_section)) {
        printf ("... [%s]\n", section);
        strncpy (Prev_section, section, sizeof(Prev_section));
        Prev_section[sizeof(Prev_section) - 1] = '\0';
    }

    printf ("... %s%s%s;\n", name, value ? "=" : "", value ? value : "");

    if (!value) {
        return true;
    }

    return !(strcmp (name, "user") == 0 && strcmp (value, "parse_error") == 0);
}

void parse(const char* fname) {
    static int u = 100;
    char *contents;
    IniState is = { .handler = dumper, .user = &u };
    bool ok;

    *Prev_section = '\0';
    contents = ini_slurp (fname, NULL);
    if (!contents) {
        ok = false;
        is.lineno = -1;
    } else {
        ok = ini_parse_string (&is, contents);
        free (contents);
    }
    printf ("%s: ok=%d err=%d line=%d user=%d\n",
            fname, ok, is.err, is.lineno, User);
    u++;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf ("usage: inihfuzz file.ini\n");
        return 1;
    }
    parse (argv[1]);
    return 0;
}
