// ─────────────────────────────────────────────────────
//  Amalgame Programming Language
//  Copyright (c) 2026 Bastien MOUGET
//  Licensed under Apache 2.0
//  https://github.com/BastienMOUGET/Amalgame
// ─────────────────────────────────────────────────────

// ═══════════════════════════════════════════════════════
//  symbol.vala  -  Symbol table for the CODE Resolver
//
//  Organisation:
//  1. SymbolKind     — what kind of entity a symbol is
//  2. Symbol         — a named, typed, located entity
//  3. Scope          — a lexical scope owning symbols
//  4. SymbolTable    — the root table; manages scope stack
// ═══════════════════════════════════════════════════════

namespace CodeTranspiler.Analyzer {

    using CodeTranspiler.Ast;


    // ═══════════════════════════════════════════════════
    //  1. SYMBOL KIND
    // ═══════════════════════════════════════════════════

    /**
     * Discriminates every kind of named entity in CODE.
     *
     * Used by the resolver to know what rules apply when
     * a symbol is referenced (e.g. a CLASS cannot be called
     * like a function; a METHOD cannot be used as a type).
     */
    public enum SymbolKind {
        // Type declarations
        SYM_CLASS,          // public class Player { … }
        SYM_INTERFACE,      // public interface IDamageable { … }
        SYM_ENUM,           // public enum Role { … }
        SYM_ENUM_MEMBER,    // Tank, Healer inside an enum
        SYM_RECORD,         // public record Point(float X, float Y)
        SYM_DATA_CLASS,     // public data class Player(…)
        SYM_TRAIT,          // public trait Printable { … }

        // Callable declarations
        SYM_METHOD,         // public void TakeDamage(int amount) { … }
        SYM_CONSTRUCTOR,    // public Player(string name) { … }
        SYM_LAMBDA,         // p => p.IsAlive()

        // Storage declarations
        SYM_FIELD,          // public string Name
        SYM_PROP,           // public float Area { get; set; }
        SYM_LOCAL_VAR,      // let x = 42  /  var y = 0
        SYM_PARAMETER,      // method / lambda parameter

        // Namespace
        SYM_NAMESPACE,      // namespace MyApp.Models

        // Import alias
        SYM_IMPORT      // import Code.IO as IO
    }


    // ═══════════════════════════════════════════════════
    //  2. SYMBOL
    // ═══════════════════════════════════════════════════

    /**
     * A single named entity known to the resolver.
     *
     * Every declaration the resolver encounters (class, method,
     * variable, parameter…) is turned into a Symbol and stored
     * in the appropriate Scope.
     *
     * The `DeclNode` back-reference lets later passes (TypeChecker,
     * CGenerator) retrieve the full AST node from any use-site.
     */
    public class Symbol : Object {

        /** Human-readable name exactly as written in source. */
        public string     Name     { get; set; }

        /** What kind of entity this symbol represents. */
        public SymbolKind Kind     { get; set; }

        /**
         * The AST node that declared this symbol.
         * Null only for built-in primitives injected by the resolver.
         */
        public AstNode?   DeclNode { get; set; }

        /**
         * Resolved type of this symbol, as a string key.
         *
         * Examples:
         *   "int", "string", "Player", "List<Player>", "void"
         *
         * Populated by the TypeChecker in a later pass.
         * The Resolver sets it to "" when unknown.
         */
        public string     TypeKey  { get; set; default = ""; }

        /**
         * Whether this symbol is declared `let` (immutable).
         * Only meaningful for LOCAL_VAR and PARAMETER.
         */
        public bool       IsLet    { get; set; default = false; }

        /**
         * Whether this symbol is `static`.
         * Meaningful for METHOD, FIELD, PROPERTY.
         */
        public bool       IsStatic { get; set; default = false; }

        /**
         * Source location for error messages (filename:line:col).
         * Copied from DeclNode.Location() when available.
         */
        public string     Location { get; set; default = "<builtin>"; }

        public Symbol(string     name,
                      SymbolKind kind,
                      AstNode?   declNode = null) {
            Name     = name;
            Kind     = kind;
            DeclNode = declNode;
            if (declNode != null)
                Location = declNode.Location();
        }

        public string ToString() {
            return "[%s] %s  type=%s  @ %s".printf(
                Kind.to_string(), Name, TypeKey, Location);
        }
    }


    // ═══════════════════════════════════════════════════
    //  3. SCOPE
    // ═══════════════════════════════════════════════════

    /**
     * A lexical scope: a flat map of name → Symbol.
     *
     * Scopes are arranged in a parent chain:
     *
     *   global (builtins + top-level types)
     *     └─ namespace scope
     *          └─ class scope   (fields, methods, properties)
     *               └─ method scope  (parameters)
     *                    └─ block scope  (local vars, foreach var)
     *                         └─ nested block scope …
     *
     * Lookup walks the chain upward until a match is found or
     * the global scope is exhausted.
     */
    public class Scope : Object {

        /** Human-readable label for debug output. */
        public string  Label  { get; set; }

        /** The enclosing scope, or null for the global scope. */
        public weak Scope? Parent { get; set; }

        /** Symbols declared directly in this scope. */
        private Gee.HashMap<string, Symbol> _symbols;

        public Scope(string label, Scope? parent = null) {
            Label   = label;
            Parent  = parent;
            _symbols = new Gee.HashMap<string, Symbol>();
        }

        // ── Declaration ────────────────────────────────

        /**
         * Declare a symbol in this scope.
         *
         * Returns false (duplicate) if the name is already taken
         * in *this* scope (shadowing an outer scope is allowed).
         */
        public bool Declare(Symbol symbol) {
            if (_symbols.has_key(symbol.Name))
                return false;
            _symbols[symbol.Name] = symbol;
            return true;
        }

        // ── Lookup ─────────────────────────────────────

        /**
         * Look up a name in this scope only (no parent walk).
         */
        public Symbol? LookupLocal(string name) {
            return _symbols.has_key(name) ? _symbols[name] : null;
        }

        /**
         * Look up a name, walking the parent chain.
         * Returns the first match or null.
         */
        public Symbol? Lookup(string name) {
            Scope? current = this;
            while (current != null) {
                var sym = current.LookupLocal(name);
                if (sym != null) return sym;
                current = current.Parent;
            }
            return null;
        }

        /**
         * Returns all symbols declared in this scope.
         * Useful for IDE features (completion list).
         */
        public Gee.Collection<Symbol> AllSymbols() {
            return _symbols.values;
        }

        // ── Debug ──────────────────────────────────────

        public string Dump(int indent = 0) {
            var sb  = new StringBuilder();
            var pad = string.nfill(indent * 2, ' ');
            sb.append("%sScope[%s]\n".printf(pad, Label));
            foreach (var sym in _symbols.values)
                sb.append("%s  %s\n".printf(pad, sym.ToString()));
            return sb.str;
        }
    }


    // ═══════════════════════════════════════════════════
    //  4. SYMBOL TABLE
    // ═══════════════════════════════════════════════════

    /**
     * The root symbol table; manages the scope stack and
     * provides the public API used by the Resolver.
     *
     * Usage pattern inside the Resolver:
     *
     *   _table.PushScope("class:Player");
     *   // … visit class members, declare symbols …
     *   _table.PopScope();
     *
     * Built-in primitive types are pre-loaded so the resolver
     * can validate usages of int, string, bool… without special
     * cases spread across the code.
     */
    public class SymbolTable : Object {

        /** The global (outermost) scope. */
        public Scope Global { get; private set; }

        /** The scope currently being visited. */
        public Scope Current { get; private set; }

        /** Stack of scopes (Current == stack top). */
        private Gee.ArrayList<Scope> _stack;

        public SymbolTable() {
            Global  = new Scope("global");
            Current = Global;
            _stack  = new Gee.ArrayList<Scope>();
            _stack.add(Global);

            _RegisterBuiltins();
        }

        // ── Scope management ───────────────────────────

        /**
         * Open a new child scope of the current scope.
         * All subsequent Declare() calls go into this scope.
         */
        public void PushScope(string label) {
            var scope = new Scope(label, Current);
            _stack.add(scope);
            Current = scope;
        }

        /**
         * Close the current scope and return to its parent.
         * Asserts that we never pop the global scope.
         */
        public void PopScope() {
            assert(_stack.size > 1);
            _stack.remove_at(_stack.size - 1);
            Current = _stack[_stack.size - 1];
        }

        // ── Declaration ────────────────────────────────

        /**
         * Declare a symbol in the *current* scope.
         * Returns false if the name is already taken in this scope.
         */
        public bool Declare(Symbol symbol) {
            return Current.Declare(symbol);
        }

        /**
         * Declare a symbol in the *global* scope.
         * Used during the first pass (top-level type collection).
         */
        public bool DeclareGlobal(Symbol symbol) {
            return Global.Declare(symbol);
        }

        // ── Lookup ─────────────────────────────────────

        /**
         * Look up a name starting from the current scope,
         * walking up the parent chain.
         */
        public Symbol? Lookup(string name) {
            return Current.Lookup(name);
        }

        /**
         * Look up a name in the global scope only.
         * Used to check if a type is known.
         */
        public Symbol? LookupGlobal(string name) {
            return Global.LookupLocal(name);
        }

        // ── "Did you mean?" helper ─────────────────────

        /**
         * Returns the closest declared name to `name` using
         * Levenshtein distance, searching from the current scope
         * upward. Returns null if no candidate is close enough.
         *
         * Used to power "did you mean 'player'?" suggestions in
         * resolver error messages.
         */
        public string? DidYouMean(string name) {
            string? best     = null;
            int     bestDist = int.MAX;

            Scope? scope = Current;
            while (scope != null) {
                foreach (var sym in scope.AllSymbols()) {
                    int d = _Levenshtein(name, sym.Name);
                    // Accept suggestions within distance 3
                    if (d < bestDist && d <= 3) {
                        bestDist = d;
                        best     = sym.Name;
                    }
                }
                scope = scope.Parent;
            }
            return best;
        }

        // ── Dump ───────────────────────────────────────

        /**
         * Returns a textual dump of the entire global scope.
         * Useful for debugging the first resolver pass.
         */
        public string DumpGlobal() {
            return Global.Dump();
        }

        // ── Private helpers ────────────────────────────

        /**
         * Pre-populate the global scope with CODE's built-in
         * primitive types and the special `void` pseudo-type.
         *
         * These symbols have no DeclNode (they are not written
         * in any .code source file) and their TypeKey is their
         * own name.
         */
        private void _RegisterBuiltins() {
            string[] primitives = {
                "int", "float", "double",
                "string", "bool", "void",
                "i8", "i16", "i32", "i64",
                "u8", "u16", "u32", "u64",
                "f32", "f64",
                "char", "byte",
                "object"
            };

            foreach (var name in primitives) {
                var sym      = new Symbol(name, SymbolKind.SYM_CLASS);
                sym.TypeKey  = name;
                sym.Location = "<builtin>";
                Global.Declare(sym);
            }

            // Built-in generic container stubs.
            // Full definitions will come from the standard library;
            // these stubs let the resolver accept usages before
            // stdlib is wired in.
            string[] generics = {
                "List", "Map", "Set",
                "Option", "Result",
                "Task", "Func", "Action"
            };
            foreach (var name in generics) {
                var sym      = new Symbol(name, SymbolKind.SYM_CLASS);
                sym.TypeKey  = name;
                sym.Location = "<builtin>";
                Global.Declare(sym);
            }

            // Built-in Console namespace stub
            var console      = new Symbol("Console", SymbolKind.SYM_CLASS);
            console.TypeKey  = "Console";
            console.Location = "<builtin>";
            Global.Declare(console);

            // Built-in Math namespace stub
            var math         = new Symbol("Math", SymbolKind.SYM_CLASS);
            math.TypeKey     = "Math";
            math.Location    = "<builtin>";
            Global.Declare(math);
        }

        /**
         * Iterative Levenshtein distance between two strings.
         * O(m*n) time, O(min(m,n)) space.
         */
        private int _Levenshtein(string a, string b) {
            int la = (int) a.length;
            int lb = (int) b.length;

            if (la == 0) return lb;
            if (lb == 0) return la;

            // Use the shorter string as the column dimension
            if (la < lb) return _Levenshtein(b, a);

            int[] prev = new int[lb + 1];
            int[] curr = new int[lb + 1];

            for (int j = 0; j <= lb; j++)
                prev[j] = j;

            for (int i = 1; i <= la; i++) {
                curr[0] = i;
                for (int j = 1; j <= lb; j++) {
                    int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
                    curr[j]  = int.min(
                        int.min(curr[j-1] + 1, prev[j] + 1),
                        prev[j-1] + cost
                    );
                }
                // Swap
                int[] tmp = prev;
                prev = curr;
                curr = tmp;
            }
            return prev[lb];
        }
    }
}
