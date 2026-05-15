project('inih',
    ['c'],
    license : 'BSD-3-Clause',
    version : '62',
    default_options : ['c_std=c11'],
    meson_version: '>=0.56.0'
)

#### options ####
arg_static = []
distro_install = get_option('distro_install')

if distro_install
    pkg = import('pkgconfig')
else
    if get_option('allow_no_value')
        arg_static += ['-DINI_ALLOW_NO_VALUE=1']
    endif
    if get_option('stop_on_first_error')
        arg_static += ['-DINI_STOP_ON_FIRST_ERROR=1']
    endif
    if get_option('report_line_numbers')
        arg_static += ['-DINI_HANDLER_LINENO=1']
    endif
    if get_option('call_handler_on_new_section')
        arg_static += ['-DINI_CALL_HANDLER_ON_NEW_SECTION=1']
    endif
endif

#### inih ####
inc_inih = include_directories('.')

src_inih = files('ini.c')

lib_inih = library('inih',
    [src_inih],
    include_directories : inc_inih,
    c_args : arg_static,
    install : distro_install,
    soversion : '0',
    gnu_symbol_visibility: 'hidden'
)

if distro_install
    install_headers('ini.h')

    pkg.generate(lib_inih,
        name : 'inih',
        description : 'simple .INI file parser',
    )
endif

inih_dep = declare_dependency(
    link_with : lib_inih,
    compile_args : arg_static,
    include_directories : inc_inih
)

if get_option('tests')
    subdir('tests')
endif

if get_option('examples')
    subdir('examples')
endif
