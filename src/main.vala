// ═══════════════════════════════════════════════════════
//  main.vala  -  Point d'entrée du transpileur CODE
// ═══════════════════════════════════════════════════════
using CodeTranspiler.Lexer;
using CodeTranspiler.Ast;
using CodeTranspiler.Parser;
using CodeTranspiler.Generator;

int main(string[] args) {

    if (args.length >= 2 && args[1] == "--version") {
        stdout.printf("CODE Transpiler v0.1.0\n");
        stdout.printf("  Lexer     : OK\n");
        stdout.printf("  AST       : OK\n");
        stdout.printf("  Parser    : OK\n");
        stdout.printf("  Generator : OK\n");
        return 0;
    }

    if (args.length < 2) {
        stderr.printf("Usage: codec <file.code> [-o output.c]\n");
        return 1;
    }

    string inputFile  = args[1];
    string outputFile = inputFile.replace(".code", ".c");

    for (int i = 2; i < args.length; i++) {
        if (args[i] == "-o" && i + 1 < args.length) {
            outputFile = args[i + 1];
        }
    }

    string source;
    try {
        FileUtils.get_contents(inputFile, out source);
    } catch (Error e) {
        stderr.printf("Error: %s\n", e.message);
        return 1;
    }

    stdout.printf("Compiling: %s → %s\n",
                   inputFile, outputFile);

    // LEXER
    var lexer  = new CodeTranspiler.Lexer.Lexer(
                     source, inputFile);
    var tokens = lexer.Tokenize();
    stdout.printf("Lexer  OK : %d tokens\n", tokens.size);

    // PARSER
    var parser = new CodeTranspiler.Parser.Parser(
                     tokens, inputFile);
    var parsed = parser.Parse();

    if (!parsed.Success) {
        foreach (var err in parsed.Errors) {
            stderr.printf("%s\n", err.ToString());
        }
        return 1;
    }
    stdout.printf("Parser OK !\n");

    // AST DEBUG
    if (Environment.get_variable("CODE_DEBUG") == "1") {
        var printer = new AstPrinter();
        stdout.printf("\n=== AST ===\n");
        stdout.printf("%s\n", printer.Print(parsed.Program));
    }

    // GENERATEUR C
    var generator = new CGenerator(inputFile);
    var generated = generator.Generate(parsed.Program);

    if (!generated.Success) {
        stderr.printf("Generator error: %s\n",
                       generated.Errors);
        return 1;
    }

    // Écrire le fichier .c
    try {
        FileUtils.set_contents(outputFile, generated.CCode);
    } catch (Error e) {
        stderr.printf("Write error: %s\n", e.message);
        return 1;
    }

    stdout.printf("Generator OK : %s\n", outputFile);

    // Compiler avec GCC automatiquement
    string exeFile = outputFile.replace(".c", "");
    string runtimeH = GLib.Path.get_dirname(
                          GLib.Path.get_dirname(
                              GLib.Path.get_dirname(
                                  outputFile)))
                      + "/src/transpiler/runtime";

    string gccCmd = "gcc -g3 -O0 -I%s %s -lgc -lm -o %s"
                    .printf(runtimeH, outputFile, exeFile);

    stdout.printf("GCC: %s\n", gccCmd);
    int ret = 0;
    try {
        string[] cmd_args = {"bash", "-c", gccCmd, null};
        GLib.Process.spawn_sync(
            null, cmd_args, null,
            GLib.SpawnFlags.SEARCH_PATH,
            null, null, null, out ret
        );
    } catch (Error e) {
        stderr.printf("GCC error: %s\n", e.message);
        ret = 1;
    }

    if (ret == 0) {
        stdout.printf("Build OK : %s\n", exeFile);
        stdout.printf("\nRun: ./%s\n", exeFile);
    } else {
        stderr.printf("GCC failed !\n");
        return 1;
    }

    return 0;
}
