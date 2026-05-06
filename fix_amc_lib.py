#!/usr/bin/env python3
"""Post-process amc_lib.c: fix what the Vala CGen gets wrong."""
import re

path = "src/amalgame/amc_lib.c"
with open(path) as f:
    content = f.read()

changes = 0

# Fix 1: struct primitive fields
old_struct = """struct _Amalgame_Compiler_AmalgameCompiler {
    Amalgame_Compiler_DiagnosticFormatter* Diag;
};"""
new_struct = """struct _Amalgame_Compiler_AmalgameCompiler {
    Amalgame_Compiler_DiagnosticFormatter* Diag;
    code_bool IsLib;
    code_bool CheckOnly;
    code_bool Verbose;
    i64 ExitCode;
};"""
if old_struct in content:
    content = content.replace(old_struct, new_struct, 1)
    changes += 1
    print("struct fields added")

# Fix 2: remove 'static' from AmalgameCompiler method forward declarations
# These must be visible to amc_main.c (external linkage)
methods = ["SetLib", "SetCheckOnly", "SetVerbose", "SetColor", "GetExitCode", "Run"]
for m in methods:
    old = f"static void Amalgame_Compiler_AmalgameCompiler_{m}("
    new = f"void Amalgame_Compiler_AmalgameCompiler_{m}("
    if old in content:
        content = content.replace(old, new)
        changes += 1

old_get = "static i64 Amalgame_Compiler_AmalgameCompiler_GetExitCode("
new_get = "i64 Amalgame_Compiler_AmalgameCompiler_GetExitCode("
if old_get in content:
    content = content.replace(old_get, new_get)
    changes += 1

old_run = "static void Amalgame_Compiler_AmalgameCompiler_Run("
new_run = "void Amalgame_Compiler_AmalgameCompiler_Run("
if old_run in content:
    content = content.replace(old_run, new_run)
    changes += 1

print(f"static removed from {changes-1} method declarations")

# Fix 3: constructor initialization
if "self->IsLib = 0;" not in content:
    pat = r'(self->Diag = Amalgame_Compiler_DiagnosticFormatter_new\(\);)([\s\S]*?)(return self;\n\})'
    def add_inits(m):
        return (m.group(1) + m.group(2) +
                "    self->IsLib = 0;\n    self->CheckOnly = 0;\n    self->Verbose = 0;\n    self->ExitCode = 0;\n    " +
                m.group(3))
    new_content = re.sub(pat, add_inits, content, count=1)
    if new_content != content:
        content = new_content
        changes += 1
        print("constructor inits added")

with open(path, "w") as f:
    f.write(content)
print(f"Done: {changes} fix(es) applied")
