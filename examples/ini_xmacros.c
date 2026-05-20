/* Parse a configuration file into a struct using X-Macros */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ini.h"

/* define the config struct type */
typedef struct {
    #define CFG(s, n, default) char *s##_##n;
    #include "config.def"
} config;

/* zero-initialized; defaults are strdup'd in main so all fields are heap-owned */
config Config;

/* process a line of the INI file, storing valid values into config struct */
int handler(void *user, const char *section, const char *name,
            const char *value)
{
    config *cfg = (config *)user;

    if (0) ;
    #define CFG(s, n, default) else if (strcmp(section, #s) == 0 && \
        strcmp(name, #n) == 0) { free(cfg->s##_##n); cfg->s##_##n = strdup(value); return 0; }
    #include "config.def"
    else
        return 1;  /* unknown section/name, error */
}

/* print all the variables in the config, one per line */
void dump_config(config *cfg)
{
    #define CFG(s, n, default) printf("%s_%s = %s\n", #s, #n, cfg->s##_##n);
    #include "config.def"
}

int main(void)
{
    char* contents;
    int rc = 0;

    #define CFG(s, n, default) Config.s##_##n = strdup(default);
    #include "config.def"

    contents = ini_slurp("test.ini", NULL);
    if (!contents) {
        printf("Can't load 'test.ini', using defaults\n");
    } else if (ini_parse_string(contents, handler, &Config) != 0) {
        printf("Parse error in 'test.ini'\n");
        rc = 1;
    }
    free(contents);
    dump_config(&Config);

    #define CFG(s, n, default) free(Config.s##_##n);
    #include "config.def"
    return rc;
}
