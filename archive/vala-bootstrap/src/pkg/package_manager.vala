// ─────────────────────────────────────────────────────────────
//  Amalgame Package Manager
//  Handles: init, add, build, list
//
//  amalgame.json format:
//  {
//    "name": "my-project",
//    "version": "1.0.0",
//    "description": "...",
//    "main": "src/main.am",
//    "sources": ["src/*.am"],
//    "dependencies": {
//      "github:user/repo": "1.0.0"
//    }
//  }
// ─────────────────────────────────────────────────────────────

using GLib;

namespace CodeTranspiler {

public class PackageManager : Object {

    private string  _projectDir;
    private string  _manifestPath;
    private string  _packagesDir;

    // ANSI colors (reuse from DiagnosticFormatter logic)
    private const string RESET  = "\x1b[0m";
    private const string BOLD   = "\x1b[1m";
    private const string GREEN  = "\x1b[1;32m";
    private const string RED    = "\x1b[1;31m";
    private const string CYAN   = "\x1b[1;36m";
    private const string YELLOW = "\x1b[1;33m";

    public PackageManager(string projectDir) {
        _projectDir   = projectDir;
        _manifestPath = Path.build_filename(projectDir, "amalgame.json");
        _packagesDir  = Path.build_filename(projectDir, "packages");
    }

    // ── Public commands ───────────────────────────────────

    public int Init(string? name) {
        if (FileUtils.test(_manifestPath, FileTest.EXISTS)) {
            _info("amalgame.json already exists.");
            return 0;
        }

        string projName = name ?? Path.get_basename(_projectDir);
        string manifest = """
{
  "name": "%s",
  "version": "0.1.0",
  "description": "",
  "main": "src/main.am",
  "sources": ["src/*.am"],
  "dependencies": {}
}
""".printf(projName).strip();

        try {
            // Create src/ directory
            string srcDir = Path.build_filename(_projectDir, "src");
            if (!FileUtils.test(srcDir, FileTest.EXISTS))
                DirUtils.create(srcDir, 0755);

            // Write manifest
            FileUtils.set_contents(_manifestPath, manifest + "\n");

            // Create starter main.am if it doesn't exist
            string mainAm = Path.build_filename(srcDir, "main.am");
            if (!FileUtils.test(mainAm, FileTest.EXISTS)) {
                string starter = """
namespace %s

public class Program {
    public static void Main(string[] args) {
        Console.WriteLine("Hello from %s!")
    }
}
""".printf(_ToPascalCase(projName), projName).strip();
                FileUtils.set_contents(mainAm, starter + "\n");
            }

            _success("Initialized project '%s'".printf(projName));
            stdout.printf("  Created : amalgame.json\n");
            stdout.printf("  Created : src/main.am\n");
            stdout.printf("\n");
            stdout.printf("  Run %samc pkg build%s to compile\n",
                          BOLD, RESET);
            return 0;
        } catch (Error e) {
            _error("Init failed: " + e.message);
            return 1;
        }
    }

    public int Add(string dep) {
        // dep format: "github:user/repo" or "github:user/repo@tag"
        string pkg  = dep;
        string ver  = "latest";

        if ("@" in dep) {
            string[] parts = dep.split("@");
            pkg = parts[0];
            ver = parts[1];
        }

        _info("Adding %s@%s ...".printf(pkg, ver));

        // Parse source
        if (!pkg.has_prefix("github:")) {
            _error("Only 'github:user/repo' format is supported for now.");
            return 1;
        }

        string ghPath = pkg.substring("github:".length);
        string[] parts = ghPath.split("/");
        if (parts.length < 2) {
            _error("Invalid package: '%s'. Use 'github:user/repo'".printf(pkg));
            return 1;
        }

        string user     = parts[0];
        string repoName = parts[1];
        string destDir  = Path.build_filename(_packagesDir, user, repoName);

        // Create packages dir
        if (DirUtils.create_with_parents(destDir, 0755) != 0) {
            _error("Cannot create packages dir: %s".printf(destDir));
            return 1;
        }

        // Clone via git
        string cloneUrl = "https://github.com/%s/%s.git"
                          .printf(user, repoName);
        _info("Cloning %s ...".printf(cloneUrl));

        string gitCmd = ver == "latest"
            ? "git clone --depth 1 \"%s\" \"%s\"".printf(cloneUrl, destDir)
            : "git clone --depth 1 --branch \"%s\" \"%s\" \"%s\""
              .printf(ver, cloneUrl, destDir);

        int ret = _Shell(gitCmd);
        if (ret != 0) {
            _error("Failed to clone %s".printf(cloneUrl));
            return 1;
        }

        // Update manifest
        _UpdateManifest(pkg, ver);

        _success("Added %s@%s".printf(pkg, ver));
        stdout.printf("  Location: packages/%s/%s\n", user, repoName);
        return 0;
    }

    public int List() {
        var manifest = _LoadManifest();
        if (manifest == null) return 1;

        stdout.printf("\n%s%s%s — %s\n".printf(
            BOLD, manifest.name, RESET, manifest.version));

        if (manifest.dependencies.size == 0) {
            stdout.printf("  (no dependencies)\n\n");
            return 0;
        }

        stdout.printf("\nDependencies:\n");
        foreach (var kv in manifest.dependencies.entries) {
            bool installed = _IsInstalled(kv.key);
            string icon    = installed
                ? GREEN + "✓" + RESET
                : YELLOW + "?" + RESET;
            stdout.printf("  %s  %s  %s%s%s\n",
                          icon, kv.key,
                          installed ? "" : YELLOW,
                          kv.value,
                          installed ? "" : RESET);
        }
        stdout.printf("\n");
        return 0;
    }

    public int Build(string? amcPath) {
        var manifest = _LoadManifest();
        if (manifest == null) return 1;

        _info("Building '%s' v%s ...".printf(
              manifest.name, manifest.version));

        // Collect source files
        var files = new Gee.ArrayList<string>();
        foreach (var pattern in manifest.sources) {
            _GlobFiles(pattern, files);
        }

        if (files.size == 0) {
            _error("No source files found. Check 'sources' in amalgame.json.");
            return 1;
        }

        _info("Found %d source file(s)".printf(files.size));

        // Build amc command
        string amc = amcPath ?? "amc";
        string outName = manifest.name.replace(" ", "_");
        var    cmd = new StringBuilder();
        cmd.append(amc);
        foreach (var f in files)
            cmd.append(" \"%s\"".printf(f));
        cmd.append(" -o \"%s\"".printf(outName));

        stdout.printf("  %s$ %s%s\n", CYAN, cmd.str, RESET);

        int ret = _Shell(cmd.str);
        if (ret == 0) {
            _success("Built: ./%s".printf(outName));
        } else {
            _error("Build failed.");
        }
        return ret;
    }

    public int Install() {
        var manifest = _LoadManifest();
        if (manifest == null) return 1;

        if (manifest.dependencies.size == 0) {
            _info("No dependencies to install.");
            return 0;
        }

        int errors = 0;
        foreach (var kv in manifest.dependencies.entries) {
            if (!_IsInstalled(kv.key)) {
                stdout.printf("  Installing %s@%s ...\n", kv.key, kv.value);
                int r = Add("%s@%s".printf(kv.key, kv.value));
                if (r != 0) errors++;
            } else {
                _info("Already installed: %s".printf(kv.key));
            }
        }
        return errors > 0 ? 1 : 0;
    }

    // ── Private helpers ───────────────────────────────────

    private class Manifest : Object {
        public string name        { get; set; default = "project"; }
        public string version     { get; set; default = "0.1.0"; }
        public string description { get; set; default = ""; }
        public string main        { get; set; default = "src/main.am"; }
        public Gee.ArrayList<string> sources { get; set; }
        public Gee.HashMap<string, string> dependencies { get; set; }

        public Manifest() {
            sources      = new Gee.ArrayList<string>();
            dependencies = new Gee.HashMap<string, string>();
        }
    }

    private Manifest? _LoadManifest() {
        if (!FileUtils.test(_manifestPath, FileTest.EXISTS)) {
            _error("No amalgame.json found. Run 'amc pkg init' first.");
            return null;
        }

        string content;
        try {
            FileUtils.get_contents(_manifestPath, out content);
        } catch (Error e) {
            _error("Cannot read amalgame.json: " + e.message);
            return null;
        }

        var m = new Manifest();

        // Simple JSON parsing — extract key-value pairs
        m.name        = _JsonStr(content, "name")        ?? "project";
        m.version     = _JsonStr(content, "version")     ?? "0.1.0";
        m.description = _JsonStr(content, "description") ?? "";
        m.main        = _JsonStr(content, "main")        ?? "src/main.am";

        // Parse sources array
        int srcStart = content.index_of("\"sources\"");
        if (srcStart >= 0) {
            int arrStart = content.index_of("[", srcStart);
            int arrEnd   = content.index_of("]", arrStart);
            if (arrStart >= 0 && arrEnd > arrStart) {
                string arr = content[arrStart+1:arrEnd];
                foreach (var item in arr.split(",")) {
                    string s = item.strip().replace("\"", "");
                    if (s.length > 0) m.sources.add(s);
                }
            }
        }
        if (m.sources.size == 0)
            m.sources.add("src/*.am");

        // Parse dependencies object
        int depStart = content.index_of("\"dependencies\"");
        if (depStart >= 0) {
            int objStart = content.index_of("{", depStart);
            int objEnd   = content.index_of("}", objStart);
            if (objStart >= 0 && objEnd > objStart) {
                string obj = content[objStart+1:objEnd];
                foreach (var item in obj.split(",")) {
                    string[] kv = item.split(":");
                    if (kv.length >= 2) {
                        string k = kv[0].strip().replace("\"", "");
                        // Reconstruct value (may contain ":" from github:user/repo)
                        string v = "";
                        for (int i = 1; i < kv.length; i++) {
                            if (i > 1) v += ":";
                            v += kv[i];
                        }
                        v = v.strip().replace("\"", "");
                        if (k.length > 0)
                            m.dependencies[k] = v;
                    }
                }
            }
        }

        return m;
    }

    private void _UpdateManifest(string pkg, string ver) {
        try {
            string content;
            FileUtils.get_contents(_manifestPath, out content);

            // Find dependencies section and add entry
            int depIdx = content.index_of("\"dependencies\"");
            if (depIdx < 0) return;

            int objStart = content.index_of("{", depIdx);
            int objEnd   = content.index_of("}", objStart);
            if (objStart < 0 || objEnd < 0) return;

            string depEntry = "\n    \"%s\": \"%s\"".printf(pkg, ver);
            string inner    = content[objStart+1:objEnd].strip();

            string newDeps;
            if (inner.length == 0)
                newDeps = "{%s\n  }".printf(depEntry);
            else
                newDeps = "{\n    %s,%s\n  }".printf(
                    inner, depEntry);

            string newContent = content[0:objStart] + newDeps +
                                content[objEnd+1:content.length];
            FileUtils.set_contents(_manifestPath, newContent);
        } catch (Error e) {
            _info("Warning: could not update amalgame.json: " + e.message);
        }
    }

    private bool _IsInstalled(string pkg) {
        if (!pkg.has_prefix("github:")) return false;
        string path = pkg.substring("github:".length).replace("/", Path.DIR_SEPARATOR_S);
        string dir  = Path.build_filename(_packagesDir, path);
        return FileUtils.test(dir, FileTest.IS_DIR);
    }

    private void _GlobFiles(string pattern, Gee.ArrayList<string> files) {
        // Handle "src/*.am" glob pattern
        string dir  = Path.get_dirname(pattern);
        string ext  = "*.am";
        if (pattern.contains("*"))
            ext = Path.get_basename(pattern);
        else {
            // Exact file
            string full = Path.build_filename(_projectDir, pattern);
            if (FileUtils.test(full, FileTest.EXISTS))
                files.add(full);
            return;
        }

        string fullDir = Path.build_filename(_projectDir, dir);
        try {
            var d = Dir.open(fullDir);
            string? name = null;
            while ((name = d.read_name()) != null) {
                if (name.has_suffix(".am")) {
                    string full = Path.build_filename(fullDir, name);
                    if (!files.contains(full))
                        files.add(full);
                }
            }
        } catch (Error e) {
            // Directory doesn't exist yet — ignore
        }
    }

    private string? _JsonStr(string json, string key) {
        string pattern = "\"%s\"".printf(key);
        int idx = json.index_of(pattern);
        if (idx < 0) return null;
        int colon = json.index_of(":", idx);
        if (colon < 0) return null;
        int q1 = json.index_of("\"", colon + 1);
        if (q1 < 0) return null;
        int q2 = json.index_of("\"", q1 + 1);
        if (q2 < 0) return null;
        return json[q1+1:q2];
    }

    private string _ToPascalCase(string s) {
        var sb = new StringBuilder();
        bool cap = true;
        foreach (char c in s.to_utf8()) {
            if (c == '-' || c == '_' || c == ' ') { cap = true; continue; }
            sb.append_c(cap ? c.toupper() : c);
            cap = false;
        }
        return sb.str;
    }

    private int _Shell(string cmd) {
        int ret = 0;
        try {
            string[] argv = { "bash", "-c", cmd, null };
            GLib.Process.spawn_sync(
                _projectDir, argv, null,
                GLib.SpawnFlags.SEARCH_PATH,
                null, null, null, out ret);
        } catch (Error e) {
            _error("Shell error: " + e.message);
            ret = 1;
        }
        return ret;
    }

    private void _info(string msg) {
        stdout.printf("  %s→%s %s\n", CYAN, RESET, msg);
    }

    private void _success(string msg) {
        stdout.printf("  %s✓%s %s%s%s\n", GREEN, RESET, BOLD, msg, RESET);
    }

    private void _error(string msg) {
        stderr.printf("  %s✗%s %s\n", RED, RESET, msg);
    }
}

} // namespace CodeTranspiler
