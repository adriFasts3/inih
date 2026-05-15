#!/usr/bin/env bash

CC="${CC:-gcc} -Wall"

$CC ../ini.c unittest.c -o unittest_multi
./unittest_multi > baseline_multi.txt
rm -f unittest_multi

$CC ../ini.c -DINI_STOP_ON_FIRST_ERROR=1 unittest.c -o unittest_stop_on_first_error
./unittest_stop_on_first_error > baseline_stop_on_first_error.txt
rm -f unittest_stop_on_first_error

$CC ../ini.c -DINI_HANDLER_LINENO=1 unittest.c -o unittest_handler_lineno
./unittest_handler_lineno > baseline_handler_lineno.txt
rm -f unittest_handler_lineno

$CC ../ini.c unittest_string.c -o unittest_string
./unittest_string > baseline_string.txt
rm -f unittest_string

$CC ../ini.c -DINI_CALL_HANDLER_ON_NEW_SECTION=1 unittest.c -o unittest_call_handler_on_new_section
./unittest_call_handler_on_new_section > baseline_call_handler_on_new_section.txt
rm -f unittest_call_handler_on_new_section

$CC ../ini.c -DINI_ALLOW_NO_VALUE=1 unittest.c -o unittest_allow_no_value
./unittest_allow_no_value > baseline_allow_no_value.txt
rm -f unittest_allow_no_value
