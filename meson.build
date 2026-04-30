project('amalgame', 'vala', 'c',
    version : '0.3.0'
)

cc       = meson.get_compiler('c')
glib_dep = dependency('glib-2.0')
gee_dep  = dependency('gee-0.8')

# GC and Math: linked at C level only
# (no Vala VAPI for bdw-gc)
gc_dep = cc.find_library('gc', required : true)
m_dep  = cc.find_library('m',  required : true)

# Vala sources
vala_sources = files(
    # Lexer
    'src/core/lexer/token.vala',
    'src/core/lexer/lexer.vala',

    # Parser
    'src/core/parser/ast.vala',
    'src/core/parser/ast_visitor.vala',
    'src/core/parser/ast_printer.vala',
    'src/core/parser/parser.vala',

    # Analyzer
    'src/core/analyzer/symbol.vala',
    'src/core/analyzer/resolver.vala',
    'src/core/analyzer/typechecker.vala',

    # Generator
    'src/transpiler/generator/c_generator.vala',

    # Diagnostics
    'src/transpiler/diagnostics.vala',

    # Entry point
    'src/main.vala'
)

executable('amc',
    vala_sources,
    dependencies : [
        glib_dep,
        gee_dep,
        gc_dep,
        m_dep
    ]
)
