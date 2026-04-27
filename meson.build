project('code-transpiler', 'vala', 'c',
    version : '0.1.0'
)

cc       = meson.get_compiler('c')
glib_dep = dependency('glib-2.0')
gee_dep  = dependency('gee-0.8')

# GC et Math : lies au niveau C uniquement
# (pas de VAPI Vala pour bdw-gc)
gc_dep = cc.find_library('gc', required : true)
m_dep  = cc.find_library('m',  required : true)

# Sources Vala
vala_sources = files(
    'src/core/lexer/token.vala',
    'src/core/lexer/lexer.vala',
    'src/core/parser/ast.vala',
    'src/core/parser/ast_visitor.vala',
    'src/core/parser/ast_printer.vala',
    'src/core/parser/parser.vala',
    'src/transpiler/generator/c_generator.vala',
    'src/main.vala'
)

executable('codec',
    vala_sources,
    dependencies : [
        glib_dep,
        gee_dep,
        gc_dep,
        m_dep
    ]
)
