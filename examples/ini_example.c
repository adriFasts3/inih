/* Example: parse a simple configuration file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ini.h"

typedef struct
{
    int version;
    const char* name;
    const char* email;
} configuration;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
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

int main(int argc, char* argv[])
{
    configuration config;
    char* contents;

    config.version = 0;  /* set defaults */
    config.name = NULL;
    config.email = NULL;

    contents = ini_slurp("test.ini", NULL);
    if (!contents) {
        printf("Can't load 'test.ini'\n");
        return 1;
    }
    if (ini_parse_string(contents, handler, &config) < 0) {
        free(contents);
        printf("Can't load 'test.ini'\n");
        return 1;
    }
    free(contents);
    printf("Config loaded from 'test.ini': version=%d, name=%s, email=%s\n",
        config.version, config.name, config.email);

    if (config.name)
        free((void*)config.name);
    if (config.email)
        free((void*)config.email);

    return 0;
}
