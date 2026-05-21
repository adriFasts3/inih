/* inih -- simple .INI file parser

SPDX-License-Identifier: BSD-3-Clause

Copyright (C) 2009-2025, Ben Hoyt
Copyright (C) 2026, Adriana Castro <adriana.castro@fasts3.io>

This is a fork and rewrite of Ben Hoyt's original inih.

inih is released under the New BSD license (see LICENSE.txt).

Original project: https://github.com/benhoyt/inih

*/

#include "ini.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Advance past leading whitespace. */
static char *skip_spaces(char *line_start, char *line_end) {
	while (line_start < line_end && isspace ((unsigned char)*line_start)) {
		line_start++;
	}
	return line_start;
}

/* Trim trailing whitespace; NUL-terminate. */
static char *trim_spaces(char *line_start, char *line_end) {
	while (line_end > line_start && isspace ((unsigned char)line_end[-1])) {
		line_end--;
	}
	*line_end = '\0';
	return line_end;
}

/* See documentation in header file. */
bool ini_parse_string(IniState *is, char *string) {
	if (!is || !string || !is->handler) {
		if (is) {
			is->err = INI_ERROR_INVALID_ARGS;
		}
		return false;
	}

	const char *section = "";
	char *p = string;
	is->lineno = 0;
	is->err = INI_ERROR_UNKNOWN;

	while (*p) {
		char *line_start = p;
		char *line_end;
		char *line_eol = strchr (p, '\n');

		is->lineno++;
		if (line_eol) {
			p = line_eol + 1;
		} else {
			p = line_eol = p + strlen (p);
		}

		line_start = skip_spaces (line_start, line_eol);
		line_end = trim_spaces (line_start, line_eol);

		if (*line_start == '[') {
			/* A "[section]" line */
			char *close = strchr (line_start + 1, ']');
			if (close) {
				*close = '\0';
				section = line_start + 1;
			}
			else {
				is->err = INI_ERROR_MISSING_BRACKET;
				return false;
			}
		}
		else if (*line_start && *line_start != ';' && *line_start != '#') {
			/* Not a comment, must be a "name = value" pair */
			char *sep = strchr (line_start, '=');
			if (sep) {
				trim_spaces (line_start, sep);
				char *value = skip_spaces (sep + 1, line_end);
				if (!is->handler (is, section, line_start, value)) {
					is->err = INI_ERROR_INVALID_DEF;
					return false;
				}
			}
			else if (!is->handler (is, section, line_start, NULL)) {
				is->err = INI_ERROR_INVALID_DEF;
				return false;
			}
		}
	}

	is->err = INI_OK;
	return true;
}

/* See documentation in header file. */
char *ini_slurp(const char *filename, size_t *size) {
	char *buf = NULL;
	long len;
	size_t n = 0;

	if (!filename) return NULL;

	FILE *f = fopen (filename, "rb");
	if (!f) {
		return NULL;
	}
	if (fseek (f, 0, SEEK_END) == 0 && (len = ftell (f)) >= 0
			&& (buf = malloc ((size_t)len + 1)) != NULL) {
		rewind (f);
		n = fread (buf, 1, (size_t)len, f);
		if (ferror (f)) {
			free (buf);
			buf = NULL;
			n = 0;
		} else {
			buf[n] = '\0';
		}
	}
	fclose (f);

	if (size) {
		*size = n;
	}
	return buf;
}
