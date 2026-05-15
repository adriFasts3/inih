runtest = find_program('runtest.sh', required: false)
if not runtest.found()
  subdir_done()
endif

tests = {
  'multi': { 'args': [] },
  'stop_on_first_error': { 'args': ['-DINI_STOP_ON_FIRST_ERROR=1'] },
  'handler_lineno': { 'args': ['-DINI_HANDLER_LINENO=1'] },
  'string': { 'src': 'unittest_string.c', 'args': [] },
  'call_handler_on_new_section': { 'args': ['-DINI_CALL_HANDLER_ON_NEW_SECTION=1'] },
  'allow_no_value': { 'args': ['-DINI_ALLOW_NO_VALUE=1'] }
}

foreach name, properties : tests
  test_src = 'src' in properties ? properties['src'] : 'unittest.c'
  exe = executable('unittest_' + name, src_inih, test_src, c_args : ['-Wall', properties['args']])
  test('test_' + name, runtest, depends : [exe], args : [files('baseline_' + name + '.txt'), exe.full_path()])
endforeach
