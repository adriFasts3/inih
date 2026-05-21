/* Example: parse a simple configuration file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ini.h"

typedef struct
{
    int version;
    char *name;
    char *email;
} configuration;

static bool handler(IniState *is, const char *section, const char *name,
                    const char *value)
{
    configuration *pconfig = (configuration*)is->user;

    #define MATCH(s, n) (strcmp (section, s) == 0 && strcmp (name, n) == 0)
    if (MATCH ("protocol", "version")) {
        pconfig->version = atoi (value);
    } else if (MATCH ("user", "name")) {
        pconfig->name = strdup (value);
    } else if (MATCH ("user", "email")) {
        pconfig->email = strdup (value);
    } else {
        return false;  /* unknown section/name, error */
    }
    return true;
}

int main(void)
{
    configuration config = {0};
    char *contents;
    IniState is = { .handler = handler, .user = &config };
    int rc = 0;

    contents = ini_slurp ("test.ini", NULL);
    if (!contents) {
        printf ("Can't load 'test.ini'\n");
        rc = 1;
    } else if (!ini_parse_string (&is, contents)) {
        printf ("Can't parse 'test.ini' (line %d)\n", is.lineno);
        rc = 1;
    } else {
        printf ("Config loaded from 'test.ini': version=%d, name=%s, email=%s\n",
            config.version,
            config.name ? config.name : "(unset)",
            config.email ? config.email : "(unset)");
    }

    free (contents);
    free (config.name);
    free (config.email);
    return rc;
}
