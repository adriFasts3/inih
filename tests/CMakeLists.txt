# Tests compile the parser sources directly into each binary (mirrors meson,
# which passes src_inih into every test executable).

find_program(RUNTEST runtest.sh PATHS ${CMAKE_CURRENT_SOURCE_DIR} NO_DEFAULT_PATH)
if(NOT RUNTEST)
    message(STATUS "runtest.sh not found; skipping tests")
    return()
endif()

# name -> test source
set(test_multi_src  unittest.c)
set(test_string_src unittest_string.c)

foreach(name multi string)
    add_executable(unittest_${name} ${CMAKE_SOURCE_DIR}/ini.c ${test_${name}_src})
    target_include_directories(unittest_${name} PRIVATE ${CMAKE_SOURCE_DIR})
    target_compile_options(unittest_${name} PRIVATE -Wall)

    add_test(
        NAME test_${name}
        COMMAND ${RUNTEST}
                ${CMAKE_CURRENT_SOURCE_DIR}/baseline_${name}.txt
                $<TARGET_FILE:unittest_${name}>
    )
endforeach()
