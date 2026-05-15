/* inih -- simple .INI file parser

SPDX-License-Identifier: BSD-3-Clause

Copyright (C) 2009-2025, Ben Hoyt

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ini.h"

#define MAX_SECTION 50

/* Strip whitespace chars off end of given string, in place. end must be a
   pointer to the NUL terminator at the end of the string. Return s. */
static char* ini_rstrip(char* s, char* end)
{
    while (end > s && isspace((unsigned char)(*--end)))
        *end = '\0';
    return s;
}

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

/* Similar to strncpy, but ensures dest (size bytes) is
   NUL-terminated, and doesn't pad with NULs. */
static char* ini_strncpy0(char* dest, const char* src, size_t size)
{
    /* Could use strncpy internally, but it causes gcc warnings (see issue #91) */
    size_t i;
    for (i = 0; i < size - 1 && src[i]; i++)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}

/* Copy at most cap-1 bytes from the input string into buf, stopping at a
   newline. NUL-terminates buf and advances the input cursor (pptr, pleft).
   Returns the number of bytes written (excluding the NUL). */
static size_t ini_read_line(char* buf, size_t cap,
                            const char** pptr, size_t* pleft)
{
    const char* p = *pptr;
    size_t n = *pleft;
    size_t i = 0;
    char c;

    while (i < cap - 1 && n > 0) {
        c = *p++;
        n--;
        buf[i++] = c;
        if (c == '\n')
            break;
    }
    buf[i] = '\0';
    *pptr = p;
    *pleft = n;
    return i;
}

/* See documentation in header file. */
int ini_parse_string(const char* string, ini_handler handler, void* user)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
    size_t max_line = INI_MAX_LINE;
#else
    char* line;
    size_t max_line = INI_INITIAL_ALLOC;
#endif
#if INI_ALLOW_REALLOC && !INI_USE_STACK
    char* new_line;
#endif
    char section[MAX_SECTION] = "";

    const char* ptr;
    size_t num_left;
    size_t offset;
    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;
    char abyss[16];  /* Used to consume input when a line is too long. */
    size_t abyss_len;

    assert(string != NULL);
    assert(handler != NULL);

    ptr = string;
    num_left = strlen(string);

#if !INI_USE_STACK
    line = (char*)malloc(INI_INITIAL_ALLOC);
    if (!line) {
        return -2;
    }
#endif

#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
#else
#define HANDLER(u, s, n, v) handler(u, s, n, v)
#endif

    /* Scan through input line by line */
    while ((offset = ini_read_line(line, max_line, &ptr, &num_left)) > 0) {
#if INI_ALLOW_REALLOC && !INI_USE_STACK
        while (max_line < INI_MAX_LINE &&
               offset == max_line - 1 && line[offset - 1] != '\n') {
            max_line *= 2;
            if (max_line > INI_MAX_LINE)
                max_line = INI_MAX_LINE;
            new_line = realloc(line, max_line);
            if (!new_line) {
                free(line);
                return -2;
            }
            line = new_line;
            offset += ini_read_line(line + offset, max_line - offset,
                                    &ptr, &num_left);
            if (num_left == 0)
                break;
        }
#endif

        lineno++;

        /* If line exceeded INI_MAX_LINE bytes, discard till end of line. */
        if (offset == max_line - 1 && line[offset - 1] != '\n') {
            while ((abyss_len = ini_read_line(abyss, sizeof(abyss),
                                              &ptr, &num_left)) > 0) {
                if (!error)
                    error = lineno;
                if (abyss[abyss_len - 1] == '\n')
                    break;
            }
        }

        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = ini_rstrip(ini_lskip(start), line + offset);

        if (strchr(INI_START_COMMENT_PREFIXES, *start)) {
            /* Start-of-line comment */
        }
        else if (*start == '[') {
            /* A "[section]" line */
            end = ini_find_chars(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                ini_strncpy0(section, start + 1, sizeof(section));
#if INI_CALL_HANDLER_ON_NEW_SECTION
                if (HANDLER(user, section, NULL, NULL) && !error)
                    error = lineno;
#endif
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start) {
            /* Not a comment, must be a name[=:]value pair */
            end = ini_find_chars(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = ini_rstrip(start, end);
                value = end + 1;
                end = value + strlen(value);
                value = ini_lskip(value);
                ini_rstrip(value, end);

                /* Valid name[=:]value pair found, call handler */
                if (HANDLER(user, section, name, value) && !error)
                    error = lineno;
            }
            else {
                /* No '=' or ':' found on name[=:]value line */
#if INI_ALLOW_NO_VALUE
                *end = '\0';
                name = ini_rstrip(start, end);
                if (HANDLER(user, section, name, NULL) && !error)
                    error = lineno;
#else
                if (!error)
                    error = lineno;
#endif
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#if !INI_USE_STACK
    free(line);
#endif

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
