// Amalgame VS Code extension — LSP client + DAP adapter.
//
// LSP: spawns `amc lsp` and pipes stdio through vscode-languageclient.
// Server publishes diagnostics on every didOpen/didChange (Full sync);
// this client renders them as squigglies in the editor.
// Disable via `"amalgame.enableLsp": false`.
//
// DAP (v0.3.0+): registers a DebugAdapterDescriptorFactory for the
// `amc` debug type. When VS Code starts a debug session of type
// `amc`, this returns a DebugAdapterExecutable that spawns
// `amc dap` (path overridable via `amalgame.dapServerPath`).
// The package.json contribution carries `program`/`args` as a
// fallback so the debug type still works if the factory fails to
// register.

const os = require('os');
const vscode = require('vscode');
const { workspace, window } = vscode;
const { LanguageClient, TransportKind, Trace } = require('vscode-languageclient/node');

let client;

// Normalise the user-set `amalgame.serverPath`: trim incidental
// whitespace (trivial JSON paste error) and expand a leading `~/`
// to the user's home directory. Without this, child_process.spawn
// fails with ENOENT on values that "look fine" but differ from
// what spawn understands — `spawn` doesn't run a shell, so neither
// trimming nor tilde expansion happens implicitly.
function resolveServerPath(raw) {
    let p = (raw || '').trim();
    if (p.startsWith('~/') || p === '~') {
        p = os.homedir() + p.slice(1);
    }
    return p || 'amc';
}

function activate(context) {
    const config = workspace.getConfiguration('amalgame');

    // ── DAP adapter registration ──────────────────────────
    // Register before the LSP early-return so the debug type
    // works even when the user disables the LSP. The factory
    // reads the current `amalgame.dapServerPath` (falling back
    // to `amalgame.serverPath`, then literal `amc`) every time
    // a debug session starts — so changing the setting takes
    // effect on the next F5 without a reload.
    //
    // Bridge mode (default since amc 0.8.46+, opt-out via
    // `amalgame.dapBridge`: false) translates DAP↔gdb-MI in
    // amc itself so List/Map/Set/Closure values pretty-print and
    // runtime frames are filtered. Phase 7 of the DAP roadmap
    // will flip the amc-side default and drop the `--bridge`
    // gate; until then we pass the flag explicitly.
    const dapChannel = window.createOutputChannel('Amalgame DAP');
    const factory = {
        createDebugAdapterDescriptor(_session, _executable) {
            const cfg = workspace.getConfiguration('amalgame');
            const rawDap = (cfg.get('dapServerPath', '') || '').trim();
            const rawLsp = (cfg.get('serverPath', 'amc') || '').trim();
            const raw = rawDap || rawLsp || 'amc';
            const resolved = resolveServerPath(raw);
            const useBridge = cfg.get('dapBridge', true);
            const showRuntime = cfg.get('dapShowRuntime', false);
            const args = ['dap'];
            if (useBridge) { args.push('--bridge'); }
            if (showRuntime) { args.push('--show-runtime'); }
            dapChannel.appendLine(`[dap] launching adapter: ${resolved} ${args.join(' ')}`);
            return new vscode.DebugAdapterExecutable(resolved, args);
        }
    };
    context.subscriptions.push(
        vscode.debug.registerDebugAdapterDescriptorFactory('amc', factory)
    );
    dapChannel.appendLine('[dap] DebugAdapterDescriptorFactory registered for type "amc"');

    // ── LSP client startup ────────────────────────────────
    if (!config.get('enableLsp', true)) {
        return;
    }

    const rawServerPath = config.get('serverPath', 'amc');
    const serverPath = resolveServerPath(rawServerPath);
    const outputChannel = window.createOutputChannel('Amalgame LSP');
    if (rawServerPath !== serverPath) {
        outputChannel.appendLine(`[ext] activate — serverPath="${serverPath}" (resolved from "${rawServerPath}")`);
    } else {
        outputChannel.appendLine(`[ext] activate — serverPath="${serverPath}"`);
    }

    const serverOptions = {
        command: serverPath,
        args: ['lsp'],
        transport: TransportKind.stdio,
    };

    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'amalgame' }],
        synchronize: {},
        outputChannel,
        traceOutputChannel: outputChannel,
    };

    try {
        client = new LanguageClient(
            'amalgame',
            'Amalgame Language Server',
            serverOptions,
            clientOptions
        );
    } catch (err) {
        outputChannel.appendLine(`[ext] LanguageClient construction failed: ${err.stack || err.message}`);
        window.showErrorMessage(`Amalgame LSP construction failed: ${err.message}`);
        return;
    }

    client.onDidChangeState((e) => {
        outputChannel.appendLine(`[ext] state ${e.oldState} -> ${e.newState}`);
    });

    // Verbose trace — every JSON-RPC message lands in the output
    // channel, useful while the server is still maturing.
    client.setTrace(Trace.Verbose).catch(() => {});

    client.start().then(
        () => outputChannel.appendLine('[ext] client started'),
        (err) => {
            outputChannel.appendLine(`[ext] client.start() rejected: ${err.stack || err.message}`);
            window.showErrorMessage(
                `Amalgame LSP failed to start: ${err.message}. ` +
                `Check the "amalgame.serverPath" setting (currently: "${serverPath}").`
            );
        }
    );
}

function deactivate() {
    if (!client) return undefined;
    return client.stop();
}

module.exports = { activate, deactivate };
