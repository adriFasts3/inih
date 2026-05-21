/* inih -- tests

This works simply by dumping a bunch of info to standard output, which is
redirected to an output file (baseline_*.txt) and checked into the Git
repository. This baseline file is the test output, so the idea is to check it
once, and if it changes -- look at the diff and see which tests failed.

See unittest.sh for how to run this.

*/

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
    if (!name || strcmp (section, Prev_section)) {
        printf ("... [%s]\n", section);
        strncpy (Prev_section, section, sizeof(Prev_section));
        Prev_section[sizeof(Prev_section) - 1] = '\0';
    }
    if (!name) {
        return true;
    }

    printf ("... %s%s%s;  line %d\n", name, value ? "=" : "",
            value ? value : "", is->lineno);

    if (!value) {
        /* Line had no '=' */
        return true;
    }

    return !(strcmp (name, "user") == 0 && strcmp (value, "parse_error") == 0);
}

void parse(const char *fname) {
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

int main(void)
{
    parse ("no_file.ini");
    parse ("normal.ini");
    parse ("bad_section.ini");
    parse ("user_error.ini");
    parse ("duplicate_sections.ini");
    parse ("no_value.ini");
    parse ("name_only_after_error.ini");
    return 0;
}
