// ─────────────────────────────────────────────────────
//  Amalgame Programming Language
//  Copyright (c) 2026 Bastien MOUGET
//  Licensed under Apache 2.0
//  https://github.com/BastienMOUGET/Amalgame
// ─────────────────────────────────────────────────────

// ═══════════════════════════════════════════════════════
//  main.vala  -  Entry point of the Amalgame transpiler
//
//  Single-file pipeline:
//    source.am → Lexer → Parser → Resolver → TypeChecker → CGenerator → GCC
//
//  Multi-file pipeline (--merge / multiple .am args):
//    file1.am ─┐
//    file2.am ─┼─ Lex+Parse each → Merge AST → Resolver → TypeChecker → CGenerator → GCC
//    file3.am ─┘
// ═══════════════════════════════════════════════════════

using CodeTranspiler.Lexer;
using CodeTranspiler.Ast;
using CodeTranspiler.Parser;
using CodeTranspiler.Analyzer;
using CodeTranspiler.Generator;
using CodeTranspiler;

int main(string[] args) {

    var fmt = new DiagnosticFormatter();

    // ── Version ───────────────────────────────────────
    if (args.length >= 2 && args[1] == "--version") {
        stdout.printf("Amalgame Transpiler v0.4.0\n");
        stdout.printf("  Lexer       : OK\n");
        stdout.printf("  AST         : OK\n");
        stdout.printf("  Parser      : OK\n");
        stdout.printf("  Resolver    : OK\n");
        stdout.printf("  TypeChecker : OK\n");
        stdout.printf("  Generator   : OK\n");
        stdout.printf("  Multi-file  : OK\n");
        return 0;
    }

    if (args.length < 2) {
        stderr.printf("%s\n", fmt.FormatFatal(
            "no input file\nUsage: amc <file.am> [file2.am ...] [-o output] [--lib] [--no-typecheck]"));
        return 1;
    }

    // ── Argument parsing ──────────────────────────────
    var    inputFiles = new Gee.ArrayList<string>();
    string outputFile = "";
    bool   skipTC     = false;
    bool   forceLib   = false;

    for (int i = 1; i < args.length; i++) {
        if (args[i] == "-o" && i + 1 < args.length) {
            outputFile = args[++i];
        } else if (args[i] == "--no-typecheck") {
            skipTC = true;
        } else if (args[i] == "--lib") {
            forceLib = true;
        } else if (!args[i].has_prefix("-")) {
            inputFiles.add(args[i]);
        }
    }

    if (inputFiles.size == 0) {
        stderr.printf("%s\n", fmt.FormatFatal(
            "no input files\nUsage: amc <file.am> [file2.am ...] [-o output] [--lib] [--no-typecheck]"));
        return 1;
    }

    // Default output: based on first file or "out"
    if (outputFile == "") {
        if (inputFiles.size == 1)
            outputFile = inputFiles[0].replace(".am", ".c");
        else
            outputFile = "out.c";
    } else if (!outputFile.has_suffix(".c")) {
        // User gave an exe name like "-o mygame" → use mygame.c
        outputFile = outputFile + ".c";
    }

    bool isMulti = inputFiles.size > 1;

    if (isMulti)
        stdout.printf("Compiling: %d files → %s\n",
                      inputFiles.size, outputFile);
    else
        stdout.printf("Compiling: %s → %s\n",
                      inputFiles[0], outputFile);

    // ── LEX + PARSE each file ─────────────────────────
    var programs = new Gee.ArrayList<ProgramNode>();
    int totalTokens = 0;

    foreach (var inputFile in inputFiles) {
        string source;
        try {
            FileUtils.get_contents(inputFile, out source);
        } catch (Error e) {
            stderr.printf("%s\n", fmt.FormatFatal(
                "cannot read '%s': %s".printf(inputFile, e.message)));
            return 1;
        }

        var lexer  = new CodeTranspiler.Lexer.Lexer(source, inputFile);
        var tokens = lexer.Tokenize();
        totalTokens += tokens.size;

        var parser = new CodeTranspiler.Parser.Parser(tokens, inputFile);
        var parsed = parser.Parse();

        if (!parsed.Success) {
            foreach (var err in parsed.Errors) {
                stderr.printf("%s", fmt.FormatError(
                    "syntax",
                    err.Message,
                    err.Filename,
                    err.Line,
                    err.Column));
            }
            return 1;
        }

        programs.add(parsed.Program);
    }

    stdout.printf("Lexer  OK  %d tokens\n", totalTokens);
    stdout.printf("Parser OK  %d file(s)\n", programs.size);

    // ── MERGE AST ─────────────────────────────────────
    // Merge all ProgramNodes into a single one.
    // Strategy:
    //   - Namespace: use the first file's namespace (or last Program's)
    //   - Imports: union of all imports (deduplicated)
    //   - Declarations: concatenation of all declarations
    ProgramNode merged;

    if (programs.size == 1) {
        merged = programs[0];
    } else {
        merged = _MergePrograms(programs);
    }

    // ── AST DEBUG ─────────────────────────────────────
    if (Environment.get_variable("AMC_DEBUG") == "1") {
        var printer = new AstPrinter();
        stdout.printf("\n=== Merged AST ===\n");
        stdout.printf("%s\n", printer.Print(merged));
    }

    // ── RESOLVER ──────────────────────────────────────
    string primaryFile = inputFiles[inputFiles.size - 1]; // last = entry point
    var resolver = new Resolver(primaryFile);
    var resolved = resolver.Resolve(merged);

    if (!resolved.Success) {
        foreach (var err in resolved.Errors) {
            stderr.printf("%s", fmt.FormatError(
                "resolver",
                err.Message,
                err.Filename,
                err.Line,
                err.Column));
        }
        return 1;
    }
    stdout.printf("Resolver OK  %d symbols\n",
                  resolved.Symbols.Global.AllSymbols().size);

    // ── TYPE CHECKER ──────────────────────────────────
    if (!skipTC) {
        var tc      = new TypeChecker(resolved.Symbols, primaryFile);
        var checked = tc.Check(merged);

        if (!checked.Success) {
            foreach (var err in checked.Errors) {
                bool isWarning = err.Message.has_prefix("[warning]");
                string msg = isWarning
                    ? err.Message.substring("[warning] ".length)
                    : err.Message;
                string formatted = isWarning
                    ? fmt.FormatWarning("typechecker", msg,
                                        err.Filename, err.Line, err.Column)
                    : fmt.FormatError  ("typechecker", msg,
                                        err.Filename, err.Line, err.Column);
                if (isWarning)
                    stdout.printf("%s", formatted);
                else
                    stderr.printf("%s", formatted);
            }
            int realErrors = 0;
            foreach (var err in checked.Errors)
                if (!err.Message.has_prefix("[warning]"))
                    realErrors++;
            if (realErrors > 0) return 1;
        }
        stdout.printf("TypeChecker OK\n");
    } else {
        stdout.printf("TypeChecker -- (skipped)\n");
    }

    // ── C GENERATOR ───────────────────────────────────
    var generator = new CGenerator(primaryFile, resolved.Symbols, forceLib);
    var generated = generator.Generate(merged);

    if (!generated.Success) {
        stderr.printf("%s\n", fmt.FormatFatal(
            "generator: " + generated.Errors));
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
    if (generated.IsLibrary) {
        stdout.printf("\nLibrary ready: %s\n", outputFile);
        stdout.printf("Compile with: gcc -c -I<runtime_dir> %s -lgc -lm\n",
                      outputFile);
        return 0;
    }

    string exeFile  = outputFile.replace(".c", "");

    // Resolve runtime header directory.
    // Priority:
    //   1. AMC_RUNTIME env var (set by installer)
    //   2. Relative to the first input file's location
    //   3. Relative to the output file (single-file fallback)
    string runtimeH = Environment.get_variable("AMC_RUNTIME") ?? "";

    if (runtimeH == "") {
        // Try relative to first input file
        string firstDir = GLib.Path.get_dirname(inputFiles[0]);
        string candidate = GLib.Path.build_filename(
            firstDir, "..", "..", "..", "src", "transpiler", "runtime");
        if (FileUtils.test(
                GLib.Path.build_filename(candidate, "_runtime.h"),
                FileTest.EXISTS)) {
            runtimeH = candidate;
        }
    }

    if (runtimeH == "") {
        // Fallback: relative to output file
        runtimeH = GLib.Path.get_dirname(
                       GLib.Path.get_dirname(
                           GLib.Path.get_dirname(outputFile)))
                   + "/src/transpiler/runtime";
    }

    // Add -lcurl if Amalgame.Net is used
    bool needsCurl = false;
    foreach (var imp in merged.Imports) {
        if (imp.Name.has_prefix("Amalgame.Net")) {
            needsCurl = true;
            break;
        }
    }

    string gccCmd = "gcc -g3 -O0 -I%s %s -lgc -lm%s -o %s"
                    .printf(runtimeH, outputFile,
                            needsCurl ? " -lcurl" : "",
                            exeFile);

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
        stdout.printf("%s\n", fmt.FormatSuccess(exeFile));
    } else {
        stderr.printf("%s\n", fmt.FormatFatal("GCC compilation failed"));
        return 1;
    }

    return 0;
}

/**
 * Merge multiple ProgramNodes into one.
 *
 * - Namespace  : the last file's namespace wins (entry point)
 * - Imports    : union, deduplicated by name
 * - Declarations: all declarations concatenated in order
 */
ProgramNode _MergePrograms(Gee.ArrayList<ProgramNode> programs) {

    // Use the last program's namespace (entry point wins)
    ProgramNode result = programs[programs.size - 1];

    // Collect all declarations from all files into the last program
    // (keeping the entry point's namespace and imports as base)
    var seen = new Gee.HashSet<string>();

    // First: collect imports from all files (deduplicated)
    var allImports = new Gee.ArrayList<ImportNode>();
    foreach (var prog in programs) {
        foreach (var imp in prog.Imports) {
            if (!seen.contains(imp.Name)) {
                seen.add(imp.Name);
                allImports.add(imp);
            }
        }
    }
    result.Imports.clear();
    foreach (var imp in allImports)
        result.Imports.add(imp);

    // Then: merge all declarations (preserve order: libs first, entry last)
    var allDecls = new Gee.ArrayList<AstNode>();
    for (int i = 0; i < programs.size - 1; i++) {
        foreach (var decl in programs[i].Declarations)
            allDecls.add(decl);
    }
    // Entry point declarations last
    foreach (var decl in programs[programs.size - 1].Declarations)
        allDecls.add(decl);

    result.Declarations.clear();
    foreach (var decl in allDecls)
        result.Declarations.add(decl);

    return result;
}
