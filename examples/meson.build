examples = ['ini_example', 'ini_dump', 'ini_xmacros']

foreach name : examples
  executable(name, src_inih, name + '.c',
    include_directories : inc_inih,
    c_args : [arg_static, extra_args, '-Wall'])
endforeach
