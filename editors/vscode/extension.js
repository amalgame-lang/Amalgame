// Amalgame VS Code extension — LSP client.
//
// Spawns the amc binary in `lsp` mode and pipes stdio through
// vscode-languageclient. The server publishes diagnostics on
// every didOpen/didChange (Full sync); this client renders them
// as squigglies in the editor.
//
// Disable via `"amalgame.enableLsp": false` in settings to fall
// back to syntax-only highlighting.

const { workspace, window } = require('vscode');
const { LanguageClient, TransportKind, Trace } = require('vscode-languageclient/node');

let client;

function activate(context) {
    const config = workspace.getConfiguration('amalgame');
    if (!config.get('enableLsp', true)) {
        return;
    }

    const serverPath = config.get('serverPath', 'amc');
    const outputChannel = window.createOutputChannel('Amalgame LSP');
    outputChannel.appendLine(`[ext] activate — serverPath="${serverPath}"`);

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
