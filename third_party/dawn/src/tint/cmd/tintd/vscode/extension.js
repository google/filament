// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

var path = require('path');
var vscode = require('vscode');
var langClient = require('vscode-languageclient');

var LanguageClient = langClient.LanguageClient;

// activate() is called when the extension is activated
function activate(context) {
    let serverModule = path.join(context.extensionPath, 'tintd');
    let debugOptions = {};

    // If the extension is launched in debug mode then the debug server options are used
    // Otherwise the run options are used
    let serverOptions = {
        run: { command: serverModule, transport: langClient.stdio },
        debug: { command: serverModule, transport: langClient.stdio, options: debugOptions }
    }

    // Options to control the language client
    let clientOptions = {
        documentSelector: ['wgsl'],
        synchronize: {
            // Synchronize the setting section 'wgsl' to the server
            configurationSection: 'wgsl',
            // Notify the server about file changes to .wgsl files contained in the workspace
            fileEvents: vscode.workspace.createFileSystemWatcher('**/*.wgsl')
        }
    }

    // Create the language client and start the client.
    let disposable = new LanguageClient('wgsl', serverOptions, clientOptions).start();

    // Push the disposable to the context's subscriptions so that the
    // client can be deactivated on extension deactivation
    context.subscriptions.push(disposable);

    // Set the language configuration here instead of a language configuration
    // file to work around https://github.com/microsoft/vscode/issues/42649.
    vscode.languages.setLanguageConfiguration("wgsl", {
        comments: { "lineComment": "//" },
    });
}
exports.activate = activate;

// this method is called when your extension is deactivated
function deactivate() {
}
exports.deactivate = deactivate;
