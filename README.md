# inih (INI Not Invented Here)

[![Tests](https://github.com/benhoyt/inih/actions/workflows/tests.yml/badge.svg)](https://github.com/benhoyt/inih/actions/workflows/tests.yml)

**inih (INI Not Invented Here)** is a simple [.INI file](http://en.wikipedia.org/wiki/INI_file) parser written in C. It's only a couple of pages of code, and it was designed to be _small and simple_, so it's good for embedded systems. It's also more or less compatible with Python's [ConfigParser](http://docs.python.org/library/configparser.html) style of .INI files, including RFC 822-style multi-line syntax and `name: value` entries.

To use it, give `ini_parse_string()` a zero-terminated string containing the INI data, and it will call a callback for every `name=value` pair parsed, giving you strings for the section, name, and value. It's done this way ("SAX style") because it works well on low-memory embedded systems, but also because it makes for a KISS implementation. The library also provides `ini_slurp()` as a convenience to read a file into memory before parsing it.

Download a release, browse the source, or read about [how to use inih in a DRY style](http://blog.brush.co.nz/2009/08/xmacros/) with X-Macros.


## Compile-time options ##

You can control various aspects of inih using preprocessor defines:

### Syntax options ###

  * **Multi-line entries:** By default, inih supports multi-line entries in the style of Python's ConfigParser. To disable, add `-DINI_ALLOW_MULTILINE=0`.
  * **UTF-8 BOM:** By default, inih allows a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of INI files. To disable, add `-DINI_ALLOW_BOM=0`.
  * **Start-of-line comments:** By default, inih allows both `;` and `#` to start a comment at the beginning of a line. You can override this by changing `INI_START_COMMENT_PREFIXES`.
  * **Allow no value:** By default, inih treats a name with no value (no `=` or `:` on the line) as an error. To allow names with no values, add `-DINI_ALLOW_NO_VALUE=1`, and inih will call your handler function with value set to NULL.

### Parsing options ###

  * **Stop on first error:** By default, inih keeps parsing the rest of the file after an error. To stop parsing on the first error, add `-DINI_STOP_ON_FIRST_ERROR=1`.
  * **Report line numbers:** By default, the `ini_handler` callback doesn't receive the line number as a parameter. If you need that, add `-DINI_HANDLER_LINENO=1`.
  * **Call handler on new section:** By default, inih only calls the handler on each `name=value` pair. To detect new sections (e.g., the INI file has multiple sections with the same name), add `-DINI_CALL_HANDLER_ON_NEW_SECTION=1`. Your handler function will then be called each time a new section is encountered, with `section` set to the new section name but `name` and `value` set to NULL.

### Memory options ###

  * **Stack vs heap:** By default, inih creates a fixed-sized line buffer on the stack. To allocate on the heap using `malloc` instead, specify `-DINI_USE_STACK=0`.
  * **Maximum line length:** The default maximum line length (for stack or heap) is 200 bytes. To override this, add something like `-DINI_MAX_LINE=1000`. Note that `INI_MAX_LINE` must be 3 more than the longest line (due to `\r`, `\n`, and the NUL).
  * **Initial malloc size:** `INI_INITIAL_ALLOC` specifies the initial malloc size when using the heap. It defaults to 200 bytes.
  * **Allow realloc:** By default when using the heap (`-DINI_USE_STACK=0`), inih allocates a fixed-sized buffer of `INI_INITIAL_ALLOC` bytes. To allow this to grow to `INI_MAX_LINE` bytes, doubling if needed, set `-DINI_ALLOW_REALLOC=1`.

## Simple example in C ##

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ini.h"

typedef struct {
    int version;
    const char* name;
    const char* email;
} configuration;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* config = (configuration*)user;
    #define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

    if      (MATCH("protocol", "version")) config->version = atoi(value);
    else if (MATCH("user", "name"))        config->name = strdup(value);
    else if (MATCH("user", "email"))       config->email = strdup(value);
    else return 1;  /* unknown section/name */
    return 0;
}

int main(void)
{
    configuration config = {0};
    char* contents = ini_slurp("test.ini", NULL);
    int error;

    if (!contents) {
        printf("Can't load 'test.ini'\n");
        return 1;
    }
    error = ini_parse_string(contents, handler, &config);
    free(contents);
    if (error) {
        printf("Parse error on line %d\n", error);
        return 1;
    }
    printf("Loaded: version=%d, name=%s, email=%s\n",
        config.version, config.name, config.email);
    return 0;
}
```


## Differences from ConfigParser ##

Some differences between inih and Python's [ConfigParser](http://docs.python.org/library/configparser.html) standard library module:

* INI name=value pairs given above any section headers are treated as valid items with no section (section name is an empty string). In ConfigParser having no section is an error.
* Line continuations are handled with leading whitespace on continued lines (like ConfigParser). However, instead of concatenating continued lines together, they are treated as separate values for the same key (unlike ConfigParser).


## Platform-specific notes ##

* inih does not perform any file I/O. Windows/Win32 uses UTF-16 filenames natively, so to handle Unicode paths you can call `_wfopen()` to open a file, read its contents into memory, and pass them to `ini_parse_string()`; inih does not include `wchar_t` or Unicode handling.

## Meson notes ##

* The `meson.build` file is not required to use or compile inih, its main purpose is for distributions.
* By default Meson is set up for distro installation, but this behavior can be configured for embedded use cases:
  * with `-Ddefault_library=static` static libraries are built.
  * with `-Ddistro_install=false` libraries, headers and pkg-config files won't be installed.
* All compile-time options are implemented in Meson as well, you can take a look at [meson_options.txt](https://github.com/benhoyt/inih/blob/master/meson_options.txt) for their definition. These won't work if `distro_install` is set to `true`.
* If you want to use inih for programs which may be shipped in a distro, consider linking against the shared library. The pkg-config entry is `inih`.
* In case you use inih as a Meson subproject, you can use the `inih_dep` dependency variable. You might want to set `default_library=static` and `distro_install=false` for the subproject. An official Wrap is provided on [WrapDB](https://wrapdb.mesonbuild.com/inih).
* For packagers: if you want to tag the version in the pkg-config file, you will need to do this downstream. Add `version : '<version_as_int>',` after the `license` tag in the `project()` function and `version : meson.project_version(),` after the `soversion` tag in the `library()` function.

## Using inih with tipi.build

`inih` can be easily used in [tipi.build](https://tipi.build) projects simply by adding the following entry to your `.tipi/deps` (replace `r56` with the latest version tag):

```json
{
    "benhoyt/inih": { "@": "r56" }
}
```

The required include path in your project is:

```c
#include <ini.h>
```

## Building from vcpkg ##

You can build and install inih using [vcpkg](https://github.com/microsoft/vcpkg/) dependency manager:

    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    ./vcpkg install inih

The inih port in vcpkg is kept up to date by microsoft team members and community contributors.
If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.

## Related links ##

* [Conan package for inih](https://github.com/conan-io/conan-center-index/tree/master/recipes/inih) (Conan is a C/C++ package manager)
