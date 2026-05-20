/* Example: parse a simple configuration file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ini.h"

typedef struct
{
    int version;
    char* name;
    char* email;
} configuration;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)
    if (MATCH("protocol", "version")) {
        pconfig->version = atoi(value);
    } else if (MATCH("user", "name")) {
        pconfig->name = strdup(value);
    } else if (MATCH("user", "email")) {
        pconfig->email = strdup(value);
    } else {
        return 1;  /* unknown section/name, error */
    }
    return 0;
}

int main(void)
{
    configuration config = {0};
    char* contents;
    int rc = 0;

    contents = ini_slurp("test.ini", NULL);
    if (!contents) {
        printf("Can't load 'test.ini'\n");
        rc = 1;
    } else if (ini_parse_string(contents, handler, &config) != 0) {
        printf("Can't load 'test.ini'\n");
        rc = 1;
    } else {
        printf("Config loaded from 'test.ini': version=%d, name=%s, email=%s\n",
            config.version,
            config.name ? config.name : "(unset)",
            config.email ? config.email : "(unset)");
    }

    free(contents);
    free(config.name);
    free(config.email);
    return rc;
}
