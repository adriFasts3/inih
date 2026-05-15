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
extra_args = []

if distro_install
    pkg = import('pkgconfig')
else
    if not get_option('utf-8_bom')
        arg_static += ['-DINI_ALLOW_BOM=0']
    endif
    sol_comment_prefix = get_option('start-of-line_comment_prefix')
    if sol_comment_prefix != ';#'
        arg_static += ['-DINI_START_COMMENT_PREFIXES="' + sol_comment_prefix + '"']
    endif
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

if host_machine.system() == 'windows'
    lib = get_option('default_library')
    if lib == 'both'
        error('default_library=both is not supported on Windows')
    elif lib == 'shared'
        extra_args += '-DINI_SHARED_LIB'
        add_project_arguments('-DINI_SHARED_LIB_BUILDING', language: ['c'])
    endif
endif

#### inih ####
inc_inih = include_directories('.')

src_inih = files('ini.c')

lib_inih = library('inih',
    [src_inih],
    include_directories : inc_inih,
    c_args : [arg_static, extra_args],
    install : distro_install,
    soversion : '0',
    gnu_symbol_visibility: 'hidden'
)

if distro_install
    install_headers('ini.h')

    pkg.generate(lib_inih,
        name : 'inih',
        description : 'simple .INI file parser',
        extra_cflags : extra_args,
    )
endif

inih_dep = declare_dependency(
    link_with : lib_inih,
    compile_args : arg_static + extra_args,
    include_directories : inc_inih
)

if get_option('tests')
    subdir('tests')
endif

if get_option('examples')
    subdir('examples')
endif
