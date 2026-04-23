// ═══════════════════════════════════════════════════════
//  main.vala  -  Point d'entrée du transpileur CODE
// ═══════════════════════════════════════════════════════

using CodeTranspiler.Lexer;

int main(string[] args) {

    if (args.length < 2) {
        stderr.printf("Usage: codec <file.code> [-o output]\n");
        stderr.printf("       codec --version\n");
        stderr.printf("       codec --help\n");
        return 1;
    }

    // Version
    if (args[1] == "--version") {
        stdout.printf("CODE Transpiler v0.1.0 (Bootstrap/Vala)\n");
        return 0;
    }

    string inputFile  = args[1];
    string outputFile = inputFile.replace(".code", ".c");

    // Fichier de sortie custom
    for (int i = 2; i < args.length; i++) {
        if (args[i] == "-o" && i + 1 < args.length) {
            outputFile = args[i + 1];
        }
    }

    // Lecture du fichier source
    string source;
    try {
        FileUtils.get_contents(inputFile, out source);
    } catch (Error e) {
        stderr.printf("Error reading '%s': %s\n", inputFile, e.message);
        return 1;
    }

    stdout.printf("⚙️  Compiling: %s → %s\n", inputFile, outputFile);

    // ── LEXER ──────────────────────────────────────
    var lexer  = new Lexer(source, inputFile);
    var tokens = lexer.Tokenize();

    stdout.printf("✅ Lexer: %d tokens\n", tokens.size);

    // Debug : afficher les tokens
    if (Environment.get_variable("CODE_DEBUG") == "1") {
        foreach (var tok in tokens) {
            stdout.printf("   %s\n", tok.ToString());
        }
    }

    // ── PARSER (à venir) ───────────────────────────
    // var parser = new Parser(tokens);
    // var ast    = parser.Parse();

    // ── ANALYZER (à venir) ─────────────────────────
    // var analyzer = new Analyzer(ast);
    // analyzer.Resolve();

    // ── GENERATOR (à venir) ────────────────────────
    // var generator = new CGenerator(ast);
    // var cCode     = generator.Generate();

    stdout.printf("🚧 Parser/Generator: coming soon...\n");
    return 0;
}
