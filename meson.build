project('amalgame', 'vala', 'c',
    version : '0.8.0'
)

cc       = meson.get_compiler('c')
glib_dep = dependency('glib-2.0')
gee_dep  = dependency('gee-0.8')

# GC and Math: linked at C level only
# (no Vala VAPI for bdw-gc)
gc_dep = cc.find_library('gc', required : true)
m_dep  = cc.find_library('m',  required : true)

# Vala sources — archived bootstrap compiler.
# The active compiler is the self-hosted ./amc (built via ./build_amc.sh).
# This Vala build is kept as a recovery path for cold-start bootstrapping.
vala_sources = files(
    # Lexer
    'archive/vala-bootstrap/src/core/lexer/token.vala',
    'archive/vala-bootstrap/src/core/lexer/lexer.vala',

    # Parser
    'archive/vala-bootstrap/src/core/parser/ast.vala',
    'archive/vala-bootstrap/src/core/parser/ast_visitor.vala',
    'archive/vala-bootstrap/src/core/parser/ast_printer.vala',
    'archive/vala-bootstrap/src/core/parser/parser.vala',

    # Analyzer
    'archive/vala-bootstrap/src/core/analyzer/symbol.vala',
    'archive/vala-bootstrap/src/core/analyzer/resolver.vala',
    'archive/vala-bootstrap/src/core/analyzer/typechecker.vala',

    # Generator
    'archive/vala-bootstrap/src/transpiler/generator/c_generator.vala',

    # Diagnostics
    'archive/vala-bootstrap/src/transpiler/diagnostics.vala',

    # Package manager
    'archive/vala-bootstrap/src/pkg/package_manager.vala',

    # Entry point
    'archive/vala-bootstrap/src/main.vala'
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
