# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

import json
import uuid

import hamcrest
import jsonrpc
import pytest

ID = str(uuid.uuid4())
INITIALIZE_PARAMS = {
    "processId": 1105947,
    "rootPath": "/home/user/src/Personal/jedi-language-server",
    "rootUri": "file:///home/user/src/Personal/jedi-language-server",
    "capabilities": {
        "workspace": {
            "applyEdit": True,
            "workspaceEdit": {
                "documentChanges": True,
                "resourceOperations": ["create", "rename", "delete"],
                "failureHandling": "undo",
                "normalizesLineEndings": True,
                "changeAnnotationSupport": {"groupsOnLabel": False},
            },
            "didChangeConfiguration": {"dynamicRegistration": True},
            "didChangeWatchedFiles": {
                "dynamicRegistration": True,
                "relativePatternSupport": True,
            },
            "codeLens": {"refreshSupport": True},
            "executeCommand": {"dynamicRegistration": True},
            "configuration": True,
            "fileOperations": {
                "dynamicRegistration": True,
                "didCreate": True,
                "didRename": True,
                "didDelete": True,
                "willCreate": True,
                "willRename": True,
                "willDelete": True,
            },
            "semanticTokens": {"refreshSupport": True},
            "inlayHint": {"refreshSupport": True},
            "inlineValue": {"refreshSupport": True},
            "diagnostics": {"refreshSupport": True},
            "symbol": {
                "dynamicRegistration": True,
                "symbolKind": {
                    "valueSet": [
                        1,
                        2,
                        3,
                        4,
                        5,
                        6,
                        7,
                        8,
                        9,
                        10,
                        11,
                        12,
                        13,
                        14,
                        15,
                        16,
                        17,
                        18,
                        19,
                        20,
                        21,
                        22,
                        23,
                        24,
                        25,
                        26,
                    ]
                },
                "tagSupport": {"valueSet": [1]},
                "resolveSupport": {"properties": ["location.range"]},
            },
            "workspaceFolders": True,
        },
        "textDocument": {
            "publishDiagnostics": {
                "relatedInformation": True,
                "versionSupport": True,
                "tagSupport": {"valueSet": [1, 2]},
                "codeDescriptionSupport": True,
                "dataSupport": True,
            },
            "synchronization": {
                "dynamicRegistration": True,
                "willSave": True,
                "willSaveWaitUntil": True,
                "didSave": True,
            },
            "completion": {
                "dynamicRegistration": True,
                "contextSupport": True,
                "completionItem": {
                    "snippetSupport": True,
                    "commitCharactersSupport": True,
                    "documentationFormat": ["markdown", "plaintext"],
                    "deprecatedSupport": True,
                    "preselectSupport": True,
                    "insertReplaceSupport": True,
                    "tagSupport": {"valueSet": [1]},
                    "resolveSupport": {
                        "properties": ["documentation", "detail", "additionalTextEdits"]
                    },
                    "labelDetailsSupport": True,
                    "insertTextModeSupport": {"valueSet": [1, 2]},
                },
                "completionItemKind": {
                    "valueSet": [
                        1,
                        2,
                        3,
                        4,
                        5,
                        6,
                        7,
                        8,
                        9,
                        10,
                        11,
                        12,
                        13,
                        14,
                        15,
                        16,
                        17,
                        18,
                        19,
                        20,
                        21,
                        22,
                        23,
                        24,
                        25,
                    ]
                },
                "insertTextMode": 2,
                "completionList": {
                    "itemDefaults": [
                        "commitCharacters",
                        "editRange",
                        "insertTextFormat",
                        "insertTextMode",
                    ]
                },
            },
            "hover": {
                "dynamicRegistration": True,
                "contentFormat": ["markdown", "plaintext"],
            },
            "signatureHelp": {
                "dynamicRegistration": True,
                "contextSupport": True,
                "signatureInformation": {
                    "documentationFormat": ["markdown", "plaintext"],
                    "activeParameterSupport": True,
                    "parameterInformation": {"labelOffsetSupport": True},
                },
            },
            "references": {"dynamicRegistration": True},
            "definition": {"dynamicRegistration": True, "linkSupport": True},
            "documentHighlight": {"dynamicRegistration": True},
            "documentSymbol": {
                "dynamicRegistration": True,
                "symbolKind": {
                    "valueSet": [
                        1,
                        2,
                        3,
                        4,
                        5,
                        6,
                        7,
                        8,
                        9,
                        10,
                        11,
                        12,
                        13,
                        14,
                        15,
                        16,
                        17,
                        18,
                        19,
                        20,
                        21,
                        22,
                        23,
                        24,
                        25,
                        26,
                    ]
                },
                "hierarchicalDocumentSymbolSupport": True,
                "tagSupport": {"valueSet": [1]},
                "labelSupport": True,
            },
            "codeAction": {
                "dynamicRegistration": True,
                "isPreferredSupport": True,
                "disabledSupport": True,
                "dataSupport": True,
                "honorsChangeAnnotations": False,
                "resolveSupport": {"properties": ["edit"]},
                "codeActionLiteralSupport": {
                    "codeActionKind": {
                        "valueSet": [
                            "",
                            "quickfix",
                            "refactor",
                            "refactor.extract",
                            "refactor.inline",
                            "refactor.rewrite",
                            "source",
                            "source.organizeImports",
                        ]
                    }
                },
            },
            "codeLens": {"dynamicRegistration": True},
            "formatting": {"dynamicRegistration": True},
            "rangeFormatting": {"dynamicRegistration": True},
            "onTypeFormatting": {"dynamicRegistration": True},
            "rename": {
                "dynamicRegistration": True,
                "prepareSupport": True,
                "honorsChangeAnnotations": True,
                "prepareSupportDefaultBehavior": 1,
            },
            "documentLink": {"dynamicRegistration": True, "tooltipSupport": True},
            "typeDefinition": {"dynamicRegistration": True, "linkSupport": True},
            "implementation": {"dynamicRegistration": True, "linkSupport": True},
            "declaration": {"dynamicRegistration": True, "linkSupport": True},
            "colorProvider": {"dynamicRegistration": True},
            "foldingRange": {
                "dynamicRegistration": True,
                "rangeLimit": 5000,
                "lineFoldingOnly": True,
                "foldingRangeKind": {"valueSet": ["comment", "imports", "region"]},
                "foldingRange": {"collapsedText": False},
            },
            "selectionRange": {"dynamicRegistration": True},
            "callHierarchy": {"dynamicRegistration": True},
            "linkedEditingRange": {"dynamicRegistration": True},
            "semanticTokens": {
                "dynamicRegistration": True,
                "tokenTypes": [
                    "namespace",
                    "type",
                    "class",
                    "enum",
                    "interface",
                    "struct",
                    "typeParameter",
                    "parameter",
                    "variable",
                    "property",
                    "enumMember",
                    "event",
                    "function",
                    "method",
                    "macro",
                    "keyword",
                    "modifier",
                    "comment",
                    "string",
                    "number",
                    "regexp",
                    "decorator",
                    "operator",
                ],
                "tokenModifiers": [
                    "declaration",
                    "definition",
                    "readonly",
                    "static",
                    "deprecated",
                    "abstract",
                    "async",
                    "modification",
                    "documentation",
                    "defaultLibrary",
                ],
                "formats": ["relative"],
                "requests": {"range": True, "full": {"delta": True}},
                "multilineTokenSupport": False,
                "overlappingTokenSupport": False,
                "serverCancelSupport": True,
                "augmentsSyntaxTokens": True,
            },
            "inlayHint": {
                "dynamicRegistration": True,
                "resolveSupport": {
                    "properties": [
                        "tooltip",
                        "textEdits",
                        "label.tooltip",
                        "label.location",
                        "label.command",
                    ]
                },
            },
            "inlineValue": {"dynamicRegistration": True},
            "diagnostic": {"dynamicRegistration": True, "relatedDocumentSupport": True},
            "typeHierarchy": {"dynamicRegistration": True},
        },
        "window": {
            "showMessage": {"messageActionItem": {"additionalPropertiesSupport": True}},
            "showDocument": {"support": True},
            "workDoneProgress": True,
        },
        "general": {
            "regularExpressions": {"engine": "ECMAScript", "version": "ES2020"},
            "markdown": {"parser": "marked", "version": "4.0.10"},
            "positionEncodings": ["utf-16"],
            "staleRequestSupport": {
                "cancel": True,
                "retryOnContentModified": [
                    "textDocument/inlayHint",
                    "textDocument/semanticTokens/full",
                    "textDocument/semanticTokens/range",
                    "textDocument/semanticTokens/full/delta",
                ],
            },
        },
    },
    "initializationOptions": {
        "enable": True,
        "startupMessage": False,
        "trace": {"server": "verbose"},
        "jediSettings": {
            "autoImportModules": ["pygls"],
            "caseInsensitiveCompletion": True,
            "debug": False,
        },
        "executable": {"args": [], "command": "jedi-language-server"},
        "codeAction": {
            "nameExtractFunction": "jls_extract_def",
            "nameExtractVariable": "jls_extract_var",
        },
        "completion": {
            "disableSnippets": False,
            "resolveEagerly": False,
            "ignorePatterns": [],
        },
        "diagnostics": {
            "enable": True,
            "didOpen": True,
            "didChange": True,
            "didSave": True,
        },
        "hover": {
            "enable": True,
            "disable": {
                "class": {"all": False, "names": [], "fullNames": []},
                "function": {"all": False, "names": [], "fullNames": []},
                "instance": {"all": False, "names": [], "fullNames": []},
                "keyword": {"all": False, "names": [], "fullNames": []},
                "module": {"all": False, "names": [], "fullNames": []},
                "param": {"all": False, "names": [], "fullNames": []},
                "path": {"all": False, "names": [], "fullNames": []},
                "property": {"all": False, "names": [], "fullNames": []},
                "statement": {"all": False, "names": [], "fullNames": []},
            },
        },
        "workspace": {
            "extraPaths": [],
            "symbols": {
                "maxSymbols": 20,
                "ignoreFolders": [".nox", ".tox", ".venv", "__pycache__", "venv"],
            },
        },
    },
    "trace": "verbose",
    "workspaceFolders": [
        {
            "uri": "file:///home/user/src/Personal/jedi-language-server",
            "name": "jedi-language-server",
        }
    ],
    "locale": "en_US",
    "clientInfo": {"name": "coc.nvim", "version": "0.0.82"},
}


TEST_DATA = [
    {"id": ID, "params": INITIALIZE_PARAMS, "method": "initialize", "jsonrpc": "2.0"},
]


@pytest.mark.parametrize("index", list(range(0, len(TEST_DATA))))
def test_initialize_request_params(index):
    data = TEST_DATA[index]
    data_str = json.dumps(data)
    parsed = jsonrpc.from_json(data_str)
    actual_str = jsonrpc.to_json(parsed)
    actual_data = json.loads(actual_str)
    hamcrest.assert_that(actual_data, hamcrest.is_(data))
