# inih (INI Not Invented Here)

[![Tests](https://github.com/benhoyt/inih/actions/workflows/tests.yml/badge.svg)](https://github.com/benhoyt/inih/actions/workflows/tests.yml)

**inih (INI Not Invented Here)** is a simple [.INI file](http://en.wikipedia.org/wiki/INI_file) parser written in C. It's only a couple of pages of code, and it was designed to be _small and simple_, so it's good for embedded systems. It also supports Python [ConfigParser](http://docs.python.org/library/configparser.html)-style `name: value` entries.

To use it, give `ini_parse_string()` a zero-terminated, writable string containing the INI data, and it will call a callback for every `name=value` pair parsed, giving you strings for the section, name, and value. The buffer is parsed in place (no copies), so there are no fixed limits on line, section, name, or value lengths. It's done this way ("SAX style") because it works well on low-memory embedded systems, but also because it makes for a KISS implementation. The library also provides `ini_slurp()` as a convenience to read a file into memory before parsing it.

Download a release, browse the source, or read about [how to use inih in a DRY style](http://blog.brush.co.nz/2009/08/xmacros/) with X-Macros.


## Compile-time options ##

You can control various aspects of inih using preprocessor defines:

### Syntax options ###

  * **Allow no value:** By default, inih treats a name with no value (no `=` or `:` on the line) as an error. To allow names with no values, add `-DINI_ALLOW_NO_VALUE=1`, and inih will call your handler function with value set to NULL.

### Parsing options ###

  * **Stop on first error:** By default, inih keeps parsing the rest of the file after an error. To stop parsing on the first error, add `-DINI_STOP_ON_FIRST_ERROR=1`.
  * **Report line numbers:** By default, the `ini_handler` callback doesn't receive the line number as a parameter. If you need that, add `-DINI_HANDLER_LINENO=1`.
  * **Call handler on new section:** By default, inih only calls the handler on each `name=value` pair. To detect new sections (e.g., the INI file has multiple sections with the same name), add `-DINI_CALL_HANDLER_ON_NEW_SECTION=1`. Your handler function will then be called each time a new section is encountered, with `section` set to the new section name but `name` and `value` set to NULL.

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
* Multi-line value continuations are not supported; each `name=value` must fit on a single line.


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
