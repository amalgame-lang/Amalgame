// ─────────────────────────────────────────────────────────────
//  Amalgame — Diagnostic Formatter
//  Produces readable, colorized compiler error messages.
//
//  Output style (inspired by Rust/Swift):
//
//  error[resolver]: unknown identifier 'Directon'
//    --> tests/samples/enums.am:12:15
//     |
//  12 |     let d = Directon.North
//     |             ^^^^^^^ did you mean 'Direction'?
//     |
//
//  Colors are disabled when stdout/stderr is not a TTY
//  or when NO_COLOR env var is set.
// ─────────────────────────────────────────────────────────────

using CodeTranspiler;

namespace CodeTranspiler {

public class DiagnosticFormatter : Object {

    // ── ANSI colors ──────────────────────────────────────────
    private bool _color;

    private const string RESET  = "\x1b[0m";
    private const string BOLD   = "\x1b[1m";
    private const string DIM    = "\x1b[2m";

    // Error = red bold, Warning = yellow bold, Note = cyan
    private const string RED    = "\x1b[1;31m";
    private const string YELLOW = "\x1b[1;33m";
    private const string CYAN   = "\x1b[1;36m";
    private const string BLUE   = "\x1b[1;34m";
    private const string GREEN  = "\x1b[1;32m";
    private const string GREY   = "\x1b[2;37m";

    public DiagnosticFormatter() {
        // Enable color unless NO_COLOR is set or TERM is dumb/empty
        string? noColor = GLib.Environment.get_variable("NO_COLOR");
        string? term    = GLib.Environment.get_variable("TERM");
        string? forceColor = GLib.Environment.get_variable("FORCE_COLOR");

        if (forceColor != null) {
            _color = true;
        } else if (noColor != null || term == "dumb" || term == null) {
            _color = false;
        } else {
            // Heuristic: color if TERM is set to something real
            _color = (term != "");
        }
    }

    // ── Public API ───────────────────────────────────────────

    /**
     * Format a parse/resolver/typechecker error from raw fields.
     * Returns a fully formatted string ready to print to stderr.
     */
    public string FormatError(string kind,
                               string message,
                               string filename,
                               int    line,
                               int    col,
                               string? sourceText = null) {
        string safeFile = (filename != null && filename.length > 0)
                          ? filename : "<unknown>";
        return _Format("error", kind, message, safeFile,
                       line, col, sourceText);
    }

    public string FormatWarning(string kind,
                                 string message,
                                 string filename,
                                 int    line,
                                 int    col,
                                 string? sourceText = null) {
        string safeFile = (filename != null && filename.length > 0)
                          ? filename : "<unknown>";
        return _Format("warning", kind, message, safeFile,
                       line, col, sourceText);
    }

    /**
     * Format a stage banner shown at startup.
     * e.g. "Lexer       OK : 272 tokens"
     */
    public string FormatStage(string name, bool ok, string detail) {
        if (!_color)
            return "%s %s : %s".printf(
                name.printf("%-12s"), ok ? "OK" : "!!", detail);

        string icon  = ok ? GREEN + "✓" + RESET : RED + "✗" + RESET;
        string label = BOLD + "%-12s".printf(name) + RESET;
        return "%s %s  %s".printf(icon, label, detail);
    }

    /**
     * Format a build success line.
     */
    public string FormatSuccess(string exeFile) {
        if (!_color)
            return "Build OK : " + exeFile;
        return GREEN + BOLD + "Build OK" + RESET +
               " : " + BOLD + exeFile + RESET;
    }

    /**
     * Format a fatal/internal error (not tied to a source location).
     */
    public string FormatFatal(string message) {
        if (!_color)
            return "error: " + message;
        return RED + BOLD + "error" + RESET + ": " + message;
    }

    // ── Private helpers ──────────────────────────────────────

    private string _Format(string severity,
                            string kind,
                            string message,
                            string filename,
                            int    line,
                            int    col,
                            string? sourceText) {
        var sb = new StringBuilder();

        // Line 1: "error[resolver]: message"
        string sevColor = severity == "error"   ? RED
                        : severity == "warning" ? YELLOW
                        :                         CYAN;

        if (_color) {
            sb.append(sevColor);
            sb.append(severity);
            sb.append(RESET);
            if (kind != "") {
                sb.append(GREY);
                sb.append("[");
                sb.append(kind);
                sb.append("]");
                sb.append(RESET);
            }
            sb.append(": ");
            sb.append(BOLD);
            sb.append(message);
            sb.append(RESET);
        } else {
            sb.append(severity);
            if (kind != "") {
                sb.append("[");
                sb.append(kind);
                sb.append("]");
            }
            sb.append(": ");
            sb.append(message);
        }
        sb.append("\n");

        // Line 2: "  --> filename:line:col"
        string arrow = _color ? (BLUE + BOLD + "-->" + RESET) : "-->";
        sb.append("  %s %s:%d:%d\n".printf(arrow, filename, line, col));

        // Source context (if available)
        if (sourceText != null && line > 0) {
            _AppendSourceContext(sb, sourceText, line, col, sevColor);
        } else {
            string pipe = _color ? (BLUE + BOLD + " |" + RESET) : " |";
            sb.append(pipe);
            sb.append("\n");
        }

        return sb.str;
    }

    /**
     * Try to extract and display the source line with a caret.
     * Falls back gracefully if the line doesn't exist.
     */
    private void _AppendSourceContext(StringBuilder sb,
                                       string        source,
                                       int           line,
                                       int           col,
                                       string        sevColor) {
        string pipe    = _color ? (BLUE + BOLD + " |" + RESET) : " |";
        string lineNum = line.to_string();
        string pad     = string.nfill(lineNum.length, ' ');

        string[] lines = source.split("\n");
        string   src   = (line >= 1 && line <= lines.length)
                         ? lines[line - 1] : "";

        // Blank separator
        sb.append("%s %s\n".printf(pad, pipe));

        // Source line
        if (_color) {
            sb.append(BLUE + BOLD + lineNum + RESET);
            sb.append(" %s ".printf(pipe));
            sb.append(src);
        } else {
            sb.append(lineNum);
            sb.append(" | ");
            sb.append(src);
        }
        sb.append("\n");

        // Caret line
        if (col > 0) {
            int caretPos = int.min(col - 1, src.length);
            string caretPad = string.nfill(caretPos, ' ');
            sb.append("%s %s ".printf(pad, pipe));
            if (_color) {
                sb.append(sevColor);
                sb.append(caretPad);
                sb.append("^");
                sb.append(RESET);
            } else {
                sb.append(caretPad);
                sb.append("^");
            }
            sb.append("\n");
        }

        // Trailing separator
        sb.append("%s %s\n".printf(pad, pipe));
    }
}

} // namespace CodeTranspiler
