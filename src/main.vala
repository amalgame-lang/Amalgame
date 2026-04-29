// ─────────────────────────────────────────────────────
//  Amalgame Programming Language
//  Copyright (c) 2026 Bastien MOUGET
//  Licensed under Apache 2.0
//  https://github.com/BastienMOUGET/Amalgame
// ─────────────────────────────────────────────────────

// ═══════════════════════════════════════════════════════
//  main.vala  -  Entry point of the Amalgame transpiler
//
//  Pipeline:
//    source.am
//      → Lexer       → tokens
//      → Parser      → AST
//      → Resolver    → AST + SymbolTable
//      → TypeChecker → AST annotated
//      → CGenerator  → output.c
//      → GCC         → native executable
// ═══════════════════════════════════════════════════════

using CodeTranspiler.Lexer;
using CodeTranspiler.Ast;
using CodeTranspiler.Parser;
using CodeTranspiler.Analyzer;
using CodeTranspiler.Generator;

int main(string[] args) {

    if (args.length >= 2 && args[1] == "--version") {
        stdout.printf("Amalgame Transpiler v0.3.0\n");
        stdout.printf("  Lexer       : OK\n");
        stdout.printf("  AST         : OK\n");
        stdout.printf("  Parser      : OK\n");
        stdout.printf("  Resolver    : OK\n");
        stdout.printf("  TypeChecker : OK\n");
        stdout.printf("  Generator   : OK\n");
        return 0;
    }

    if (args.length < 2) {
        stderr.printf("Usage: amc <file.am> [-o output.c] [--lib] [--no-typecheck]\n");
        return 1;
    }

    string inputFile   = args[1];
    string outputFile  = inputFile.replace(".am", ".c");
    bool   skipTC      = false;
    bool   forceLib    = false;

    for (int i = 2; i < args.length; i++) {
        if (args[i] == "-o" && i + 1 < args.length) {
            outputFile = args[i + 1];
        } else if (args[i] == "--no-typecheck") {
            skipTC = true;
        } else if (args[i] == "--lib") {
            forceLib = true;
        }
    }

    string source;
    try {
        FileUtils.get_contents(inputFile, out source);
    } catch (Error e) {
        stderr.printf("Error: %s\n", e.message);
        return 1;
    }

    stdout.printf("Compiling: %s → %s\n", inputFile, outputFile);

    // ── LEXER ─────────────────────────────────────────
    var lexer  = new CodeTranspiler.Lexer.Lexer(source, inputFile);
    var tokens = lexer.Tokenize();
    stdout.printf("Lexer       OK : %d tokens\n", tokens.size);

    // ── PARSER ────────────────────────────────────────
    var parser = new CodeTranspiler.Parser.Parser(tokens, inputFile);
    var parsed = parser.Parse();

    if (!parsed.Success) {
        foreach (var err in parsed.Errors)
            stderr.printf("%s\n", err.ToString());
        return 1;
    }
    stdout.printf("Parser      OK\n");

    // ── AST DEBUG ─────────────────────────────────────
    if (Environment.get_variable("AMC_DEBUG") == "1") {
        var printer = new AstPrinter();
        stdout.printf("\n=== AST ===\n");
        stdout.printf("%s\n", printer.Print(parsed.Program));
    }

    // ── RESOLVER ──────────────────────────────────────
    var resolver = new Resolver(inputFile);
    var resolved = resolver.Resolve(parsed.Program);

    if (!resolved.Success) {
        foreach (var err in resolved.Errors)
            stderr.printf("%s\n", err.ToString());
        return 1;
    }
    stdout.printf("Resolver    OK : %d symbols\n",
                  resolved.Symbols.Global.AllSymbols().size);

    // ── TYPE CHECKER ──────────────────────────────────
    if (!skipTC) {
        var tc      = new TypeChecker(resolved.Symbols, inputFile);
        var checked = tc.Check(parsed.Program);

        if (!checked.Success) {
            foreach (var err in checked.Errors) {
                if (err.Message.has_prefix("[warning]"))
                    stdout.printf("%s\n", err.ToString());
                else
                    stderr.printf("%s\n", err.ToString());
            }
            int realErrors = 0;
            foreach (var err in checked.Errors)
                if (!err.Message.has_prefix("[warning]"))
                    realErrors++;
            if (realErrors > 0) {
                stderr.printf("TypeChecker: %d error(s)\n", realErrors);
                return 1;
            }
        }
        stdout.printf("TypeChecker OK\n");
    } else {
        stdout.printf("TypeChecker -- (skipped)\n");
    }

    // ── C GENERATOR ───────────────────────────────────
    var generator = new CGenerator(inputFile, resolved.Symbols, forceLib);
    var generated = generator.Generate(parsed.Program);

    if (!generated.Success) {
        stderr.printf("Generator error: %s\n", generated.Errors);
        return 1;
    }

    try {
        FileUtils.set_contents(outputFile, generated.CCode);
    } catch (Error e) {
        stderr.printf("Write error: %s\n", e.message);
        return 1;
    }

    string mode = generated.IsLibrary ? "Library" : "Executable";
    stdout.printf("Generator   OK : %s [%s]\n", outputFile, mode);

    // ── GCC ───────────────────────────────────────────
    // Library mode: only produce .c — no executable
    if (generated.IsLibrary) {
        stdout.printf("\nLibrary ready: %s\n", outputFile);
        stdout.printf("Compile with: gcc -c -I<runtime_dir> %s -lgc -lm\n",
                      outputFile);
        return 0;
    }

    string exeFile  = outputFile.replace(".c", "");
    string runtimeH = GLib.Path.get_dirname(
                          GLib.Path.get_dirname(
                              GLib.Path.get_dirname(outputFile)))
                      + "/src/transpiler/runtime";

    string gccCmd = "gcc -g3 -O0 -I%s %s -lgc -lm -o %s"
                    .printf(runtimeH, outputFile, exeFile);

    stdout.printf("GCC         : %s\n", gccCmd);
    int ret = 0;
    try {
        string[] cmd_args = { "bash", "-c", gccCmd, null };
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
        stdout.printf("Build       OK : %s\n", exeFile);
        stdout.printf("\nRun: ./%s\n", exeFile);
    } else {
        stderr.printf("GCC failed!\n");
        return 1;
    }

    return 0;
}
