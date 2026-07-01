# Thin Makefile wrapper around the CMake build.
#
# All build logic lives in CMakeLists.txt; this just maps the classic targets
# and option knobs onto CMake invocations.
#
# Options (override on the command line, e.g. `make TESTS=0`):
#   DISTRO_INSTALL=1   install shared lib, headers and pkg-config entry (default 1)
#   TESTS=1            build the test suite (default 1)
#   EXAMPLES=0         build the example programs (default 0)
#
# Standard variables honored: CC, CFLAGS, DESTDIR, PREFIX, LIBDIR, INCLUDEDIR.

BUILD_DIR ?= build

DISTRO_INSTALL ?= 1
TESTS          ?= 1
EXAMPLES       ?= 0

onoff = $(if $(filter 1,$(1)),ON,OFF)

CMAKE_FLAGS := \
	-DINIH_DISTRO_INSTALL=$(call onoff,$(DISTRO_INSTALL)) \
	-DINIH_TESTS=$(call onoff,$(TESTS)) \
	-DINIH_EXAMPLES=$(call onoff,$(EXAMPLES))

ifdef PREFIX
    CMAKE_FLAGS += -DCMAKE_INSTALL_PREFIX=$(PREFIX)
endif
ifdef LIBDIR
    CMAKE_FLAGS += -DCMAKE_INSTALL_LIBDIR=$(LIBDIR)
endif
ifdef INCLUDEDIR
    CMAKE_FLAGS += -DCMAKE_INSTALL_INCLUDEDIR=$(INCLUDEDIR)
endif
# Only forward CC when the user actually set it (not make's built-in default).
ifneq ($(origin CC),default)
    CMAKE_FLAGS += -DCMAKE_C_COMPILER=$(CC)
endif
ifdef CFLAGS
    CMAKE_FLAGS += -DCMAKE_C_FLAGS=$(CFLAGS)
endif

.PHONY: all clean install test check examples configure dist

# Reconfigure on every build so option changes take effect; CMake makes this
# cheap and idempotent when nothing changed.
configure:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)

all: configure
	cmake --build $(BUILD_DIR)

examples:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS) -DINIH_EXAMPLES=ON
	cmake --build $(BUILD_DIR) --target ini_example ini_dump ini_xmacros

test check: configure
	cmake --build $(BUILD_DIR)
	ctest --test-dir $(BUILD_DIR) --output-on-failure

install: all
	DESTDIR=$(DESTDIR) cmake --install $(BUILD_DIR)

# Roll a versioned source tarball (inih-<version>.tar.gz) via CPack.
dist: configure
	cpack --config $(BUILD_DIR)/CPackSourceConfig.cmake -B $(CURDIR)
	rm -rf $(CURDIR)/_CPack_Packages   # CPack's working dir; keep only the tarball

mrproper clean:
	rm -rf $(BUILD_DIR)
