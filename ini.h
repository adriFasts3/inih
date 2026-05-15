/* inih -- simple .INI file parser

SPDX-License-Identifier: BSD-3-Clause

Copyright (C) 2009-2025, Ben Hoyt

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#ifndef INI_H
#define INI_H

#include <stddef.h>

/* Nonzero if ini_handler callback should accept lineno parameter. */
#ifndef INI_HANDLER_LINENO
#define INI_HANDLER_LINENO 0
#endif

/* Visibility for exported symbols (paired with meson's gnu_symbol_visibility:
   'hidden' to keep internal helpers private). */
#ifndef INI_API
#if defined(__GNUC__) && __GNUC__ >= 4
#define INI_API __attribute__ ((visibility ("default")))
#else
#define INI_API
#endif
#endif

/* Typedef for prototype of handler function.

   Note that even though the value parameter has type "const char*", the user
   may cast to "char*" and modify its content, as the value is not used again
   after the call to ini_handler. This is not true of section and name --
   those must not be modified.

   Handler should return 0 on success, nonzero on error.
*/
#if INI_HANDLER_LINENO
typedef int (*ini_handler)(void* user, const char* section,
                           const char* name, const char* value,
                           int lineno);
#else
typedef int (*ini_handler)(void* user, const char* section,
                           const char* name, const char* value);
#endif

/* Parse a zero-terminated, writable string containing INI data. May have
   [section]s, name=value pairs (whitespace stripped), and comments starting
   with ';' (semicolon). Section is "" if name=value pair parsed before any
   section heading. name:value pairs are also supported as a concession to
   Python's configparser.

   The buffer is parsed in place: section, name, and value pointers passed
   to the handler point into `string`. The parser writes NUL terminators at
   token boundaries, then restores the byte at each line end so the buffer's
   line structure is preserved on return.

   For each name=value pair parsed, call handler function with given user
   pointer as well as section, name, and value (data only valid for duration
   of handler call). Handler should return 0 on success, nonzero on error.

   Returns 0 on success, or a positive line number for the first parse error
   (parsing does not stop on first error by default).
*/
INI_API int ini_parse_string(char* string, ini_handler handler, void* user);

/* Read the entire file at `filename` into a freshly malloc'd, NUL-terminated
   buffer suitable for ini_parse_string(). If `size` is non-NULL, the number
   of bytes read (excluding the trailing NUL) is written to *size. Returns
   NULL on file-open, seek, or allocation error. The caller must free() the
   returned buffer. */
INI_API char* ini_slurp(const char* filename, size_t* size);

/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See https://github.com/benhoyt/inih/issues/21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/* Chars that begin a start-of-line comment. Per Python configparser, allow
   both ; and # comments at the start of a line by default. */
#ifndef INI_START_COMMENT_PREFIXES
#define INI_START_COMMENT_PREFIXES ";#"
#endif

/* Stop parsing on first error (default is to keep parsing). */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/* Nonzero to call the handler at the start of each new section (with
   name and value NULL). Default is to only call the handler on
   each name=value pair. */
#ifndef INI_CALL_HANDLER_ON_NEW_SECTION
#define INI_CALL_HANDLER_ON_NEW_SECTION 0
#endif

/* Nonzero to allow a name without a value (no '=' or ':' on the line) and
   call the handler with value NULL in this case. Default is to treat
   no-value lines as an error. */
#ifndef INI_ALLOW_NO_VALUE
#define INI_ALLOW_NO_VALUE 0
#endif

#endif /* INI_H */
