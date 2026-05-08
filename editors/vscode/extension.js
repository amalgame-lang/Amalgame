// Amalgame VS Code extension — LSP client.
//
// Spawns the amc binary in `lsp` mode and pipes stdio through
// vscode-languageclient. The server publishes diagnostics on
// every didOpen/didChange (Full sync); this client just renders
// them as squigglies in the editor.
//
// Disable via `"amalgame.enableLsp": false` in settings to fall
// back to syntax-only highlighting (useful when amc isn't built
// or isn't on PATH).

const { workspace, window } = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

let client;

function activate(context) {
    const config = workspace.getConfiguration('amalgame');
    if (!config.get('enableLsp', true)) {
        return;
    }

    const serverPath = config.get('serverPath', 'amc');

    const serverOptions = {
        command: serverPath,
        args: ['lsp'],
        transport: TransportKind.stdio,
    };

    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'amalgame' }],
        // No watcher needed: the v1 server doesn't read workspace
        // files itself, it only sees what the client sends via
        // didOpen / didChange.
        synchronize: {},
        outputChannel: window.createOutputChannel('Amalgame LSP'),
    };

    client = new LanguageClient(
        'amalgame',
        'Amalgame Language Server',
        serverOptions,
        clientOptions
    );

    // Surface spawn errors (wrong serverPath, amc not built, …)
    // in a notification rather than failing silently.
    client.start().catch((err) => {
        window.showErrorMessage(
            `Amalgame LSP failed to start: ${err.message}. ` +
            `Check the "amalgame.serverPath" setting (currently: "${serverPath}").`
        );
    });
}

function deactivate() {
    if (!client) return undefined;
    return client.stop();
}

module.exports = { activate, deactivate };
