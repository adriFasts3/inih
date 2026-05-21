runtest = find_program('runtest.sh', required: false)
if not runtest.found()
  subdir_done()
endif

tests = {
  'multi': { 'args': [] },
  'string': { 'src': 'unittest_string.c', 'args': [] },
}

foreach name, properties : tests
  test_src = 'src' in properties ? properties['src'] : 'unittest.c'
  exe = executable('unittest_' + name, src_inih, test_src, c_args : ['-Wall', properties['args']])
  test('test_' + name, runtest, depends : [exe], args : [files('baseline_' + name + '.txt'), exe.full_path()])
endforeach
