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
bool handler(IniState *is, const char *section, const char *name,
             const char *value)
{
    config *cfg = (config *)is->user;

    if (0) ;
    #define CFG(s, n, default) else if (strcmp (section, #s) == 0 && \
        strcmp (name, #n) == 0) { free (cfg->s##_##n); cfg->s##_##n = strdup (value); return true; }
    #include "config.def"
    else
        return false;  /* unknown section/name, error */
}

/* print all the variables in the config, one per line */
void dump_config(config *cfg)
{
    #define CFG(s, n, default) printf ("%s_%s = %s\n", #s, #n, cfg->s##_##n);
    #include "config.def"
}

int main(void)
{
    char *contents;
    IniState is = { .handler = handler, .user = &Config };
    int rc = 0;

    #define CFG(s, n, default) Config.s##_##n = strdup (default);
    #include "config.def"

    contents = ini_slurp ("test.ini", NULL);
    if (!contents) {
        printf ("Can't load 'test.ini', using defaults\n");
    } else if (!ini_parse_string (&is, contents)) {
        printf ("Parse error in 'test.ini' (line %d)\n", is.lineno);
        rc = 1;
    }
    free (contents);
    dump_config (&Config);

    #define CFG(s, n, default) free (Config.s##_##n);
    #include "config.def"
    return rc;
}
