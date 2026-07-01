/* inih -- tests for ini_parse_string() */

#define _POSIX_C_SOURCE 200809L  /* for strdup() under strict -std=c11 */

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
    printf ("... %s=%s;\n", name, value ? value : "");
    return true;
}

void parse(const char *name, const char *string) {
    static int u = 100;
    char *buf;
    IniState is = { .handler = dumper, .user = &u };
    bool ok;

    *Prev_section = '\0';
    buf = strdup (string);
    ok = ini_parse_string (&is, buf);
    free (buf);
    printf ("%s: ok=%d err=%d line=%d user=%d\n",
            name, ok, is.err, is.lineno, User);
    u++;
}

int main(void)
{
    parse ("empty string", "");
    parse ("basic", "[section]\nfoo = bar\nbazz = buzz quxx");
    parse ("crlf", "[section]\r\nhello = world\r\nforty_two = 42\r\n");
    parse ("error", "[s]\na=1\nb\nc=3");
    return 0;
}
