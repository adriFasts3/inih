# CMake port of meson.build for inih
#
# Options (override with -D on the command line, e.g. -DINIH_TESTS=OFF):
#   INIH_DISTRO_INSTALL=ON   install shared lib, headers and pkg-config entry
#   INIH_TESTS=ON            build the test suite (noisy)
#   INIH_EXAMPLES=OFF        build the example programs

cmake_minimum_required(VERSION 3.13)

project(inih
    VERSION "0.1.0"
    DESCRIPTION "simple .INI file parser"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

#### options ####
option(INIH_DISTRO_INSTALL "install shared libs, headers and pkg-config entries" ON)
option(INIH_EXAMPLES "build the example programs" OFF)
option(INIH_TESTS "build the test suite (noisy)" ON)

#### inih ####
add_library(inih SHARED ini.c)
add_library(inih::inih ALIAS inih)

target_include_directories(inih PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(inih PROPERTIES
    SOVERSION 0
    C_VISIBILITY_PRESET hidden
)

if(INIH_DISTRO_INSTALL)
    include(GNUInstallDirs)

    install(TARGETS inih
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(FILES ini.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    configure_file(inih.pc.in inih.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/inih.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()

if(INIH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(INIH_EXAMPLES)
    add_subdirectory(examples)
endif()

#### source tarball (`make dist` / `cpack --config build/CPackSourceConfig.cmake`) ####
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES
    "/\\\\.git/"
    "/build/"                 # CMake build dir
    "CMakeCache\\\\.txt"      # stray CMake build dirs
    "/meson-private/"         # Meson build dirs
    "\\\\.sw[po]$"            # editor swap files
    "\\\\.o$"
    "\\\\.d$"
)
include(CPack)
