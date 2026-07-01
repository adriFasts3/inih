# inih (INI Not Invented Here)

This is a fork and rewrite of [Ben Hoyt's inih](https://github.com/benhoyt/inih), released under the same BSD-3-Clause license. It lives in a separate repository (not a GitHub fork) but preserves the original commit history.

## Why this fork?

The goal is a smaller, simpler, single-purpose `.INI` parser. The codebase has been stripped down to one job: SAX-style in-place parsing of a writable string. In particular, this fork:

- Removes the C++ wrapper — C only.
- Removes the file and stream parsers, custom allocators, and the internal line buffer; the caller owns the buffer and there are no fixed line, section, name, or value length limits.
- Removes Windows-specific code paths.
- Removes all compile-time options (multiline values, inline comments, line-number reporting, stop-on-first-error, allow-no-value, call-handler-on-new-section). Behavior is fixed.
- Drops Python ConfigParser-style `name: value` syntax; only `name = value` is supported.
- Drops BOM handling.
- Treats trailing characters after a section's closing `]` as an error.
- Uses consistent return codes via an `IniState` struct rather than a return-value-overloaded `int`.

If you need any of the features above, use [the original project](https://github.com/benhoyt/inih) — it is still actively maintained.

**inih (INI Not Invented Here)** is a simple [.INI file](http://en.wikipedia.org/wiki/INI_file) parser written in C. It's only a couple of pages of code, and it was designed to be _small and simple_, so it's good for embedded systems. Lines beginning with `;` or `#` are treated as comments.

To use it, give `ini_parse_string()` a zero-terminated, writable string containing the INI data, and it will call a callback for every `name=value` pair parsed, giving you strings for the section, name, and value. The buffer is parsed in place (no copies), so there are no fixed limits on line, section, name, or value lengths. It's done this way ("SAX style") because it works well on low-memory embedded systems, but also because it makes for a KISS implementation. The library also provides `ini_slurp()` as a convenience to read a file into memory before parsing it.

See [how to use inih in a DRY style](http://blog.brush.co.nz/2009/08/xmacros/) with X-Macros (and `examples/ini_xmacros.c`).


## API

```c
typedef bool (*IniHandler)(struct IniState *is, const char *section,
                           const char *name, const char *value);

typedef enum {
    INI_OK = 0,
    INI_ERROR_INVALID_ARGS = 1,
    INI_ERROR_MISSING_BRACKET = 2,
    INI_ERROR_INVALID_DEF = 3,
    INI_ERROR_TRAILING_CHARS = 4,
    INI_ERROR_UNKNOWN = 5,
} IniError;

typedef struct IniState {
    IniHandler handler;
    void *user;
    IniError err;
    int lineno;
} IniState;

bool ini_parse_string(IniState *is, char *string);
char *ini_slurp(const char *filename, size_t *size);
```

Set `is->handler` (required) and `is->user` (optional) before calling `ini_parse_string()`. The parser modifies `string` in place: section, name, and value pointers passed to the handler point into the buffer. On return, `is->lineno` holds the last line processed and `is->err` holds the error code. The handler returns `true` to continue parsing or `false` to stop.


## Simple example in C

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ini.h"

typedef struct {
    int version;
    char *name;
    char *email;
} configuration;

static bool handler(IniState *is, const char *section, const char *name,
                    const char *value)
{
    configuration *config = (configuration *)is->user;
    #define MATCH(s, n) (strcmp (section, s) == 0 && strcmp (name, n) == 0)

    if (!value) {
        return false;  /* this config requires values for all known keys */
    }
    if (MATCH ("protocol", "version")) {
        config->version = atoi (value);
    } else if (MATCH ("user", "name")) {
        config->name = strdup (value);
    } else if (MATCH ("user", "email")) {
        config->email = strdup (value);
    } else {
        return false;  /* unknown section/name */
    }
    return true;
}

int main(void)
{
    configuration config = {0};
    IniState is = { .handler = handler, .user = &config };
    char *contents = ini_slurp ("test.ini", NULL);

    if (!contents) {
        printf ("Can't load 'test.ini'\n");
        return 1;
    }
    bool ok = ini_parse_string (&is, contents);
    free (contents);
    if (!ok) {
        printf ("Parse error on line %d (err=%d)\n", is.lineno, is.err);
        return 1;
    }
    printf ("Loaded: version=%d, name=%s, email=%s\n",
        config.version, config.name, config.email);
    free (config.name);
    free (config.email);
    return 0;
}
```


## Differences from ConfigParser

Some differences between inih and Python's [ConfigParser](http://docs.python.org/library/configparser.html) standard library module:

* `name=value` pairs given above any section header are treated as valid items with no section (section name is an empty string). In ConfigParser having no section is an error.
* Only `=` is accepted as the name/value separator; ConfigParser's `:` separator is not supported.
* Multi-line value continuations are not supported; each `name=value` must fit on a single line.
* Inline comments are not supported. A `;` or `#` only starts a comment at the beginning of a line (after optional leading whitespace). Anywhere else, it is part of the value.


## Building

A `CMakeLists.txt`, a thin `Makefile` wrapper around it, and a `meson.build` are provided. The `Makefile` is the simplest entry point — it drives CMake under the hood, so it needs `cmake` installed. Run `make` to build the shared library, `make test` to run the test suite, `make EXAMPLES=1 examples` to build the examples, and `make install` to install (set `PREFIX`, `DESTDIR`, etc. as usual — see the `Makefile` header comment for the full list of options).

You can also invoke CMake directly, e.g. `cmake -S . -B build && cmake --build build`; the options are `-DINIH_DISTRO_INSTALL`, `-DINIH_TESTS`, and `-DINIH_EXAMPLES`.

### Cleaning across build systems

Because both a CMake and a Meson build can coexist in the tree, each build's clean also sweeps the *other* system's build directories (detected by their marker files, whatever the directory is named):

* `make clean` removes the CMake build dir **and** any Meson build dirs.
* Meson reserves the `clean` target name and offers no hook onto `ninja clean`, so the CMake sweep is a separate `clean-cmake` target — run both together: `ninja -C <builddir> clean clean-cmake`.

### Meson notes

* The `meson.build` file is not required to use or compile inih, its main purpose is for distributions.
* By default Meson is set up for distro installation, but this behavior can be configured for embedded use cases:
  * with `-Ddefault_library=static` static libraries are built.
  * with `-Ddistro_install=false` libraries, headers and pkg-config files won't be installed.
* If you want to use inih for programs which may be shipped in a distro, consider linking against the shared library. The pkg-config entry is `inih`.
* In case you use inih as a Meson subproject, you can use the `inih_dep` dependency variable. You might want to set `default_library=static` and `distro_install=false` for the subproject.
