/* inih -- simple .INI file parser

SPDX-License-Identifier: BSD-3-Clause

Copyright (C) 2009-2025, Ben Hoyt

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#ifndef INI_H
#define INI_H

#include <stdbool.h>
#include <stddef.h>

struct IniState;

/* Typedef for prototype of handler function.

   Note that even though the value parameter has type "const char*", the user
   may cast to "char*" and modify its content, as the value is not used again
   after the call to IniHandler. This is not true of section and name --
   those must not be modified.

   Handler should return true to continue parsing, false to stop.
*/
typedef bool (*IniHandler)(struct IniState *is, const char *section,
                           const char *name, const char *value);

typedef enum {
	INI_ERROR_UNKNOWN = 0,
	INI_ERROR_INVALID_ARGS = 1,
	INI_ERROR_MISSING_BRACKET = 2,
	INI_ERROR_KVDEF = 3,
} IniError;

typedef struct IniState {
	IniHandler handler;
	void *user;
	IniError err;
	int lineno;
} IniState;

/* Visibility for exported symbols (paired with meson's gnu_symbol_visibility:
   'hidden' to keep internal helpers private). */
#ifndef INI_API
#if defined(__GNUC__) && __GNUC__ >= 4
#define INI_API __attribute__ ((visibility ("default")))
#else
#define INI_API
#endif
#endif


/* Parse a zero-terminated, writable string containing INI data. May have
   [section]s, name=value pairs (whitespace stripped), and comment lines
   starting with ';' (semicolon) or '#' (hash). Section is "" if name=value
   pair parsed before any section heading.

   The buffer is parsed in place and modified: section, name, and value
   pointers passed to the handler point into `string`, with NUL terminators
   written at token and line boundaries. The buffer is not restored.

   Before calling, set `is->handler` (required) and `is->user` (optional).
   On return, `is->lineno` holds the line at which parsing stopped, and
   `is->err` holds the parse error code (if any).

   Returns true on success, false on the first parse error or when the
   handler returns false.
*/
INI_API bool ini_parse_string(IniState *is, char *string);

/* Read the entire file at `filename` into a freshly malloc'd, NUL-terminated
   buffer suitable for ini_parse_string(). If `size` is non-NULL, the number
   of bytes read (excluding the trailing NUL) is written to *size. Returns
   NULL on file-open, seek, allocation, or read error. The caller must free()
   the returned buffer. */
INI_API char *ini_slurp(const char *filename, size_t *size);

#endif /* INI_H */
