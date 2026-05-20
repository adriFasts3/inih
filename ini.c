/* inih -- simple .INI file parser

SPDX-License-Identifier: BSD-3-Clause

Copyright (C) 2009-2025, Ben Hoyt

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ini.h"

/* See documentation in header file. */
int ini_parse_string(char* string, ini_handler handler, void* user)
{
    const char* section = "";
    char* p = string;
    int lineno = 0;
    int error = 0;

    assert(string != NULL);
    assert(handler != NULL);

#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
#else
#define HANDLER(u, s, n, v) handler(u, s, n, v)
#endif

    while (*p) {
        char* line_start = p;
        char* line_end;
        char* line_eol = strchr(p, '\n');

        lineno++;
        if (line_eol)
            p = line_eol + 1;
        else
            p = line_eol = p + strlen(p);

        /* Trim leading and trailing whitespace; NUL-terminate. */
        while (line_start < line_eol && isspace((unsigned char)*line_start))
            line_start++;
        line_end = line_eol;
        while (line_end > line_start && isspace((unsigned char)line_end[-1]))
            line_end--;
        *line_end = '\0';

        if (*line_start == '[') {
            /* A "[section]" line */
            char* close = strchr(line_start + 1, ']');
            if (close) {
                *close = '\0';
                section = line_start + 1;
#if INI_CALL_HANDLER_ON_NEW_SECTION
                if (HANDLER(user, section, NULL, NULL) && !error)
                    error = lineno;
#endif
            }
            else if (!error)
                error = lineno; /* No ']' on section line */
        }
        else if (*line_start && *line_start != ';' && *line_start != '#') {
            /* Not a comment, must be a name[=:]value pair */
            char* sep = strpbrk(line_start, "=:");
            if (sep) {
                char* name_end = sep;
                char* value = sep + 1;
                while (name_end > line_start &&
                       isspace((unsigned char)name_end[-1]))
                    name_end--;
                *name_end = '\0';
                while (isspace((unsigned char)*value))
                    value++;
                if (HANDLER(user, section, line_start, value) && !error)
                    error = lineno;
            }
#if INI_ALLOW_NO_VALUE
            else if (HANDLER(user, section, line_start, NULL) && !error)
                error = lineno;
#else
            else if (!error)
                error = lineno; /* No '=' or ':' on name[=:]value line */
#endif
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#undef HANDLER
    return error;
}

/* See documentation in header file. */
char* ini_slurp(const char* filename, size_t* size)
{
    FILE* f;
    char* buf = NULL;
    long len;
    size_t n = 0;

    assert(filename != NULL);

    f = fopen(filename, "rb");
    if (!f)
        return NULL;
    if (fseek(f, 0, SEEK_END) == 0 && (len = ftell(f)) >= 0 &&
            (buf = malloc((size_t)len + 1)) != NULL) {
        rewind(f);
        n = fread(buf, 1, (size_t)len, f);
        buf[n] = '\0';
    }
    fclose(f);
    if (size)
        *size = n;
    return buf;
}
