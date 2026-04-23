// ═══════════════════════════════════════════════════════
//  main.vala  -  Point d'entrée du transpileur CODE
// ═══════════════════════════════════════════════════════

using CodeTranspiler.Lexer;
using CodeTranspiler.Ast;
using CodeTranspiler.Parser;

int main(string[] args) {

    if (args.length >= 2 && args[1] == "--version") {
        stdout.printf("CODE Transpiler v0.1.0\n");
        stdout.printf("  Lexer  : OK\n");
        stdout.printf("  AST    : OK\n");
        stdout.printf("  Parser : OK\n");
        return 0;
    }

    if (args.length < 2) {
        stderr.printf("Usage: codec <file.code>\n");
        return 1;
    }

    string inputFile = args[1];
    string source;

    try {
        FileUtils.get_contents(inputFile, out source);
    } catch (Error e) {
        stderr.printf("Error: %s\n", e.message);
        return 1;
    }

    stdout.printf("Compiling: %s\n", inputFile);

    // LEXER
    var lexer  = new CodeTranspiler.Lexer.Lexer(source, inputFile);
    var tokens = lexer.Tokenize();
    stdout.printf("Lexer  OK : %d tokens\n", tokens.size);

    // PARSER
    var parser = new Parser(tokens, inputFile);
    var result = parser.Parse();

    if (!result.Success) {
        foreach (var err in result.Errors) {
            stderr.printf("%s\n", err.ToString());
        }
        return 1;
    }

    stdout.printf("Parser OK !\n");

    // AST Printer (debug)
    if (Environment.get_variable("CODE_DEBUG") == "1") {
        var printer = new AstPrinter();
        stdout.printf("\n=== AST ===\n");
        stdout.printf(printer.Print(result.Program));
    }

    stdout.printf("Generator : coming soon\n");
    return 0;
}
