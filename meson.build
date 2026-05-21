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
