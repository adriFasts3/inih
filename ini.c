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

/* Return pointer to first non-whitespace char in given string. */
static char* ini_lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char (of chars) in given string, or pointer to
   NUL at end of string if not found. */
static char* ini_find_chars(const char* s, const char* chars)
{
    while (*s && !strchr(chars, *s)) {
        s++;
    }
    return (char*)s;
}

/* See documentation in header file. */
int ini_parse_string(char* string, ini_handler handler, void* user)
{
    char* p;
    char* line_start;
    char* line_end;
    char* line_eol;
    char saved;
    char* section = (char*)"";
    char* sep;
    char* name;
    char* value;
    char* name_end;
    char* close;
    int lineno = 0;
    int error = 0;

    assert(string != NULL);
    assert(handler != NULL);

#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
#else
#define HANDLER(u, s, n, v) handler(u, s, n, v)
#endif

    p = string;

    while (*p) {
        lineno++;

        /* Find end-of-line: a single strchr does the job. */
        line_eol = strchr(p, '\n');
        if (line_eol) {
            line_start = p;
            p = line_eol + 1;
        }
        else {
            line_start = p;
            line_eol = p + strlen(p);
            p = line_eol;
        }

        /* Skip leading whitespace in place. */
        while (line_start < line_eol && isspace((unsigned char)*line_start))
            line_start++;

        /* Find end of content (strip trailing whitespace, incl. '\r'). */
        line_end = line_eol;
        while (line_end > line_start && isspace((unsigned char)line_end[-1]))
            line_end--;

        /* NUL-terminate the trimmed line; remember the byte to restore later. */
        saved = *line_end;
        *line_end = '\0';

        if (*line_start == '\0' ||
                *line_start == ';' || *line_start == '#') {
            /* Blank line or start-of-line comment */
        }
        else if (*line_start == '[') {
            /* A "[section]" line */
            close = ini_find_chars(line_start + 1, "]");
            if (*close == ']') {
                *close = '\0';
                section = line_start + 1;
#if INI_CALL_HANDLER_ON_NEW_SECTION
                if (HANDLER(user, section, NULL, NULL) && !error)
                    error = lineno;
#endif
            }
            else if (!error) {
                /* No ']' on section line */
                error = lineno;
            }
        }
        else {
            /* Not a comment, must be a name[=:]value pair */
            sep = ini_find_chars(line_start, "=:");
            if (*sep == '=' || *sep == ':') {
                /* Trim trailing whitespace from name in place. */
                name_end = sep;
                while (name_end > line_start &&
                       isspace((unsigned char)name_end[-1])) {
                    name_end--;
                }
                *name_end = '\0';
                name = line_start;
                value = ini_lskip(sep + 1);
                if (HANDLER(user, section, name, value) && !error)
                    error = lineno;
            }
#if INI_ALLOW_NO_VALUE
            else {
                if (HANDLER(user, section, line_start, NULL) && !error)
                    error = lineno;
            }
#else
            else if (!error) {
                /* No '=' or ':' on name[=:]value line */
                error = lineno;
            }
#endif
        }

        /* Restore the line-end byte so the caller sees the buffer with line
           breaks intact (internal separators like '=' and ']' stay NUL'd: the
           section pointer must remain valid across handler calls). */
        *line_end = saved;

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

    return error;
}

/* See documentation in header file. */
char* ini_slurp(const char* filename, size_t* size)
{
    FILE* f;
    char* buf;
    long len;
    size_t n;

    assert(filename != NULL);

    f = fopen(filename, "rb");
    if (!f)
        return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    len = ftell(f);
    if (len < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    buf = (char*)malloc((size_t)len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    n = fread(buf, 1, (size_t)len, f);
    buf[n] = '\0';
    fclose(f);
    if (size)
        *size = n;
    return buf;
}
