# Example programs compile the parser sources directly (mirrors meson).

foreach(name ini_example ini_dump ini_xmacros)
    add_executable(${name} ${CMAKE_SOURCE_DIR}/ini.c ${name}.c)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR})
    target_compile_options(${name} PRIVATE -Wall)
endforeach()
