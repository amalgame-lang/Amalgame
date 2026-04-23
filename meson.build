project('code-transpiler', 'vala', 'c',
    version : '0.1.0'
)

glib_dep = dependency('glib-2.0')
gee_dep  = dependency('gee-0.8')

sources = files(
    'src/core/lexer/token.vala',
    'src/core/lexer/lexer.vala',
    'src/core/parser/ast.vala',
    'src/core/parser/ast_visitor.vala',
    'src/core/parser/ast_printer.vala',
    'src/main.vala'
)

executable('codec',
    sources,
    dependencies : [glib_dep, gee_dep]
)
