# Makefile port of meson.build for inih
#
# Options (override on the command line, e.g. `make TESTS=0`):
#   DISTRO_INSTALL=1   install shared lib, headers and pkg-config entry (default 1)
#   TESTS=1            build the test suite (default 1)
#   EXAMPLES=0         build the example programs (default 0)
#
# Standard variables: CC, CFLAGS, LDFLAGS, DESTDIR, PREFIX, LIBDIR, INCLUDEDIR.

NAME       := inih
VERSION    := 62
SOVERSION  := 0

DISTRO_INSTALL ?= 1
TESTS          ?= 1
EXAMPLES       ?= 0

PREFIX     ?= /usr/local
LIBDIR     ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include
PKGCONFDIR ?= $(LIBDIR)/pkgconfig

CC      ?= cc
CSTD    ?= -std=c11
CFLAGS  ?= -O2
WARN    := -Wall
PIC     := -fPIC
VIS     := -fvisibility=hidden

ALL_CFLAGS := $(CSTD) $(CFLAGS) $(WARN) $(PIC) $(VIS) -I.

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SHLIB_EXT     := dylib
    SHLIB         := lib$(NAME).$(SHLIB_EXT)
    SHLIB_SONAME  := lib$(NAME).$(SOVERSION).$(SHLIB_EXT)
    SHLIB_REAL    := lib$(NAME).$(SOVERSION).$(SHLIB_EXT)
    SHLIB_LDFLAGS := -dynamiclib -install_name $(LIBDIR)/$(SHLIB_SONAME) \
                     -compatibility_version $(SOVERSION) \
                     -current_version $(SOVERSION)
else
    SHLIB_EXT     := so
    SHLIB         := lib$(NAME).$(SHLIB_EXT)
    SHLIB_SONAME  := $(SHLIB).$(SOVERSION)
    SHLIB_REAL    := $(SHLIB_SONAME)
    SHLIB_LDFLAGS := -shared -Wl,-soname,$(SHLIB_SONAME)
endif

SRC_INIH := ini.c
OBJ_INIH := ini.o

TEST_NAMES    := multi string
TEST_BINS     := $(addprefix tests/unittest_,$(TEST_NAMES))
EXAMPLE_NAMES := ini_example ini_dump ini_xmacros
EXAMPLE_BINS  := $(addprefix examples/,$(EXAMPLE_NAMES))

PC_FILE := $(NAME).pc

BUILT := $(SHLIB_REAL) $(SHLIB)
ifeq ($(DISTRO_INSTALL),1)
    BUILT += $(PC_FILE)
endif
ifeq ($(TESTS),1)
    BUILT += $(TEST_BINS)
endif
ifeq ($(EXAMPLES),1)
    BUILT += $(EXAMPLE_BINS)
endif

.PHONY: all clean install test check examples
all: $(BUILT)

$(OBJ_INIH): $(SRC_INIH) ini.h
	$(CC) $(ALL_CFLAGS) -c $< -o $@

$(SHLIB_REAL): $(OBJ_INIH)
	$(CC) $(SHLIB_LDFLAGS) -o $@ $^ $(LDFLAGS)

$(SHLIB): $(SHLIB_REAL)
	ln -sf $(SHLIB_REAL) $@

$(PC_FILE): Makefile
	@printf '%s\n' \
	  'prefix=$(PREFIX)' \
	  'exec_prefix=$${prefix}' \
	  'libdir=$(LIBDIR)' \
	  'includedir=$(INCLUDEDIR)' \
	  '' \
	  'Name: $(NAME)' \
	  'Description: simple .INI file parser' \
	  'Version: $(VERSION)' \
	  'Libs: -L$${libdir} -l$(NAME)' \
	  'Cflags: -I$${includedir}' > $@

# Tests link the parser statically (mirrors meson, which compiles src_inih
# into each test binary).
tests/unittest_multi: tests/unittest.c $(SRC_INIH) ini.h
	$(CC) $(CSTD) $(CFLAGS) $(WARN) -I. $(SRC_INIH) $< -o $@ $(LDFLAGS)

tests/unittest_string: tests/unittest_string.c $(SRC_INIH) ini.h
	$(CC) $(CSTD) $(CFLAGS) $(WARN) -I. $(SRC_INIH) $< -o $@ $(LDFLAGS)

examples/%: examples/%.c $(SRC_INIH) ini.h
	$(CC) $(CSTD) $(CFLAGS) $(WARN) -I. $(SRC_INIH) $< -o $@ $(LDFLAGS)

examples: $(EXAMPLE_BINS)

test check: $(TEST_BINS)
	@status=0; here=$$(pwd); \
	for name in $(TEST_NAMES); do \
	    sh tests/runtest.sh $$here/tests/baseline_$$name.txt $$here/tests/unittest_$$name \
	        && echo "PASS test_$$name" \
	        || { echo "FAIL test_$$name"; status=1; }; \
	done; \
	exit $$status

install: all
ifneq ($(DISTRO_INSTALL),1)
	@echo "install requires DISTRO_INSTALL=1" >&2; exit 1
else
	install -d $(DESTDIR)$(LIBDIR) $(DESTDIR)$(INCLUDEDIR) $(DESTDIR)$(PKGCONFDIR)
	install -m 0755 $(SHLIB_REAL) $(DESTDIR)$(LIBDIR)/$(SHLIB_REAL)
ifneq ($(SHLIB_REAL),$(SHLIB))
	ln -sf $(SHLIB_REAL) $(DESTDIR)$(LIBDIR)/$(SHLIB)
endif
	install -m 0644 ini.h $(DESTDIR)$(INCLUDEDIR)/ini.h
	install -m 0644 $(PC_FILE) $(DESTDIR)$(PKGCONFDIR)/$(PC_FILE)
endif

clean:
	rm -f $(OBJ_INIH) $(SHLIB) $(SHLIB_REAL) $(PC_FILE) \
	      $(TEST_BINS) $(EXAMPLE_BINS)
