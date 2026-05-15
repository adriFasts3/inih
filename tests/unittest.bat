@call tcc ..\ini.c -I..\ -run unittest.c > baseline_multi.txt
@call tcc ..\ini.c -I..\ -DINI_STOP_ON_FIRST_ERROR=1 -run unittest.c > baseline_stop_on_first_error.txt
@call tcc ..\ini.c -I..\ -DINI_HANDLER_LINENO=1 -run unittest.c > baseline_handler_lineno.txt
@call tcc ..\ini.c -I..\ -run unittest_string.c > baseline_string.txt
@call tcc ..\ini.c -I..\ -DINI_CALL_HANDLER_ON_NEW_SECTION=1 -run unittest.c > baseline_call_handler_on_new_section.txt
@call tcc ..\ini.c -I..\ -DINI_ALLOW_NO_VALUE=1 -run unittest.c > baseline_allow_no_value.txt
