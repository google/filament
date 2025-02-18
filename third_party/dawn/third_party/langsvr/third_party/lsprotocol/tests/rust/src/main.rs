// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#[cfg(test)]
mod tests {
    use glob::glob;
    use lsprotocol::*;
    use serde::Deserialize;
    use std::fs;

    fn get_all_json_files(root: &str) -> Vec<String> {
        let pattern = format!("{}/*.json", root);
        glob(&pattern)
            .expect("Failed to read glob pattern")
            .filter_map(Result::ok)
            .map(|path| path.to_str().unwrap().to_string())
            .collect()
    }

    fn validate_type<T: for<'de> Deserialize<'de>>(result_type: &str, data: &str) {
        match serde_json::from_str::<T>(data) {
            Ok(_) => assert_eq!(
                result_type, "True",
                "Expected error, but succeeded deserializing:\r\n{}",
                data
            ),
            Err(e) => assert_eq!(
                result_type,
                "False",
                "Expected success, but failed deserializing: {}\r\nJSON data:\r\n{}",
                e.to_string(),
                data
            ),
        }
    }

    fn validate(lsp_type: &str, result_type: &str, data: &str) {
        println!("Validating: {}", lsp_type);
        match lsp_type {
            // Sample Generated Code:
            // "CallHierarchyIncomingCallsRequest" => {
            //     return validate_type::<CallHierarchyIncomingCallsRequest>(result_type, data)
            // }
            // GENERATED_TEST_CODE:start
            "TextDocumentImplementationRequest" => {
                return validate_type::<TextDocumentImplementationRequest>(result_type, data)
            }
            "TextDocumentTypeDefinitionRequest" => {
                return validate_type::<TextDocumentTypeDefinitionRequest>(result_type, data)
            }
            "WorkspaceWorkspaceFoldersRequest" => {
                return validate_type::<WorkspaceWorkspaceFoldersRequest>(result_type, data)
            }
            "WorkspaceConfigurationRequest" => {
                return validate_type::<WorkspaceConfigurationRequest>(result_type, data)
            }
            "TextDocumentDocumentColorRequest" => {
                return validate_type::<TextDocumentDocumentColorRequest>(result_type, data)
            }
            "TextDocumentColorPresentationRequest" => {
                return validate_type::<TextDocumentColorPresentationRequest>(result_type, data)
            }
            "TextDocumentFoldingRangeRequest" => {
                return validate_type::<TextDocumentFoldingRangeRequest>(result_type, data)
            }
            "WorkspaceFoldingRangeRefreshRequest" => {
                return validate_type::<WorkspaceFoldingRangeRefreshRequest>(result_type, data)
            }
            "TextDocumentDeclarationRequest" => {
                return validate_type::<TextDocumentDeclarationRequest>(result_type, data)
            }
            "TextDocumentSelectionRangeRequest" => {
                return validate_type::<TextDocumentSelectionRangeRequest>(result_type, data)
            }
            "WindowWorkDoneProgressCreateRequest" => {
                return validate_type::<WindowWorkDoneProgressCreateRequest>(result_type, data)
            }
            "TextDocumentPrepareCallHierarchyRequest" => {
                return validate_type::<TextDocumentPrepareCallHierarchyRequest>(result_type, data)
            }
            "CallHierarchyIncomingCallsRequest" => {
                return validate_type::<CallHierarchyIncomingCallsRequest>(result_type, data)
            }
            "CallHierarchyOutgoingCallsRequest" => {
                return validate_type::<CallHierarchyOutgoingCallsRequest>(result_type, data)
            }
            "TextDocumentSemanticTokensFullRequest" => {
                return validate_type::<TextDocumentSemanticTokensFullRequest>(result_type, data)
            }
            "TextDocumentSemanticTokensFullDeltaRequest" => {
                return validate_type::<TextDocumentSemanticTokensFullDeltaRequest>(
                    result_type,
                    data,
                )
            }
            "TextDocumentSemanticTokensRangeRequest" => {
                return validate_type::<TextDocumentSemanticTokensRangeRequest>(result_type, data)
            }
            "WorkspaceSemanticTokensRefreshRequest" => {
                return validate_type::<WorkspaceSemanticTokensRefreshRequest>(result_type, data)
            }
            "WindowShowDocumentRequest" => {
                return validate_type::<WindowShowDocumentRequest>(result_type, data)
            }
            "TextDocumentLinkedEditingRangeRequest" => {
                return validate_type::<TextDocumentLinkedEditingRangeRequest>(result_type, data)
            }
            "WorkspaceWillCreateFilesRequest" => {
                return validate_type::<WorkspaceWillCreateFilesRequest>(result_type, data)
            }
            "WorkspaceWillRenameFilesRequest" => {
                return validate_type::<WorkspaceWillRenameFilesRequest>(result_type, data)
            }
            "WorkspaceWillDeleteFilesRequest" => {
                return validate_type::<WorkspaceWillDeleteFilesRequest>(result_type, data)
            }
            "TextDocumentMonikerRequest" => {
                return validate_type::<TextDocumentMonikerRequest>(result_type, data)
            }
            "TextDocumentPrepareTypeHierarchyRequest" => {
                return validate_type::<TextDocumentPrepareTypeHierarchyRequest>(result_type, data)
            }
            "TypeHierarchySupertypesRequest" => {
                return validate_type::<TypeHierarchySupertypesRequest>(result_type, data)
            }
            "TypeHierarchySubtypesRequest" => {
                return validate_type::<TypeHierarchySubtypesRequest>(result_type, data)
            }
            "TextDocumentInlineValueRequest" => {
                return validate_type::<TextDocumentInlineValueRequest>(result_type, data)
            }
            "WorkspaceInlineValueRefreshRequest" => {
                return validate_type::<WorkspaceInlineValueRefreshRequest>(result_type, data)
            }
            "TextDocumentInlayHintRequest" => {
                return validate_type::<TextDocumentInlayHintRequest>(result_type, data)
            }
            "InlayHintResolveRequest" => {
                return validate_type::<InlayHintResolveRequest>(result_type, data)
            }
            "WorkspaceInlayHintRefreshRequest" => {
                return validate_type::<WorkspaceInlayHintRefreshRequest>(result_type, data)
            }
            "TextDocumentDiagnosticRequest" => {
                return validate_type::<TextDocumentDiagnosticRequest>(result_type, data)
            }
            "WorkspaceDiagnosticRequest" => {
                return validate_type::<WorkspaceDiagnosticRequest>(result_type, data)
            }
            "WorkspaceDiagnosticRefreshRequest" => {
                return validate_type::<WorkspaceDiagnosticRefreshRequest>(result_type, data)
            }
            "TextDocumentInlineCompletionRequest" => {
                return validate_type::<TextDocumentInlineCompletionRequest>(result_type, data)
            }
            "ClientRegisterCapabilityRequest" => {
                return validate_type::<ClientRegisterCapabilityRequest>(result_type, data)
            }
            "ClientUnregisterCapabilityRequest" => {
                return validate_type::<ClientUnregisterCapabilityRequest>(result_type, data)
            }
            "InitializeRequest" => return validate_type::<InitializeRequest>(result_type, data),
            "ShutdownRequest" => return validate_type::<ShutdownRequest>(result_type, data),
            "WindowShowMessageRequestRequest" => {
                return validate_type::<WindowShowMessageRequestRequest>(result_type, data)
            }
            "TextDocumentWillSaveWaitUntilRequest" => {
                return validate_type::<TextDocumentWillSaveWaitUntilRequest>(result_type, data)
            }
            "TextDocumentCompletionRequest" => {
                return validate_type::<TextDocumentCompletionRequest>(result_type, data)
            }
            "CompletionItemResolveRequest" => {
                return validate_type::<CompletionItemResolveRequest>(result_type, data)
            }
            "TextDocumentHoverRequest" => {
                return validate_type::<TextDocumentHoverRequest>(result_type, data)
            }
            "TextDocumentSignatureHelpRequest" => {
                return validate_type::<TextDocumentSignatureHelpRequest>(result_type, data)
            }
            "TextDocumentDefinitionRequest" => {
                return validate_type::<TextDocumentDefinitionRequest>(result_type, data)
            }
            "TextDocumentReferencesRequest" => {
                return validate_type::<TextDocumentReferencesRequest>(result_type, data)
            }
            "TextDocumentDocumentHighlightRequest" => {
                return validate_type::<TextDocumentDocumentHighlightRequest>(result_type, data)
            }
            "TextDocumentDocumentSymbolRequest" => {
                return validate_type::<TextDocumentDocumentSymbolRequest>(result_type, data)
            }
            "TextDocumentCodeActionRequest" => {
                return validate_type::<TextDocumentCodeActionRequest>(result_type, data)
            }
            "CodeActionResolveRequest" => {
                return validate_type::<CodeActionResolveRequest>(result_type, data)
            }
            "WorkspaceSymbolRequest" => {
                return validate_type::<WorkspaceSymbolRequest>(result_type, data)
            }
            "WorkspaceSymbolResolveRequest" => {
                return validate_type::<WorkspaceSymbolResolveRequest>(result_type, data)
            }
            "TextDocumentCodeLensRequest" => {
                return validate_type::<TextDocumentCodeLensRequest>(result_type, data)
            }
            "CodeLensResolveRequest" => {
                return validate_type::<CodeLensResolveRequest>(result_type, data)
            }
            "WorkspaceCodeLensRefreshRequest" => {
                return validate_type::<WorkspaceCodeLensRefreshRequest>(result_type, data)
            }
            "TextDocumentDocumentLinkRequest" => {
                return validate_type::<TextDocumentDocumentLinkRequest>(result_type, data)
            }
            "DocumentLinkResolveRequest" => {
                return validate_type::<DocumentLinkResolveRequest>(result_type, data)
            }
            "TextDocumentFormattingRequest" => {
                return validate_type::<TextDocumentFormattingRequest>(result_type, data)
            }
            "TextDocumentRangeFormattingRequest" => {
                return validate_type::<TextDocumentRangeFormattingRequest>(result_type, data)
            }
            "TextDocumentRangesFormattingRequest" => {
                return validate_type::<TextDocumentRangesFormattingRequest>(result_type, data)
            }
            "TextDocumentOnTypeFormattingRequest" => {
                return validate_type::<TextDocumentOnTypeFormattingRequest>(result_type, data)
            }
            "TextDocumentRenameRequest" => {
                return validate_type::<TextDocumentRenameRequest>(result_type, data)
            }
            "TextDocumentPrepareRenameRequest" => {
                return validate_type::<TextDocumentPrepareRenameRequest>(result_type, data)
            }
            "WorkspaceExecuteCommandRequest" => {
                return validate_type::<WorkspaceExecuteCommandRequest>(result_type, data)
            }
            "WorkspaceApplyEditRequest" => {
                return validate_type::<WorkspaceApplyEditRequest>(result_type, data)
            }
            "WorkspaceDidChangeWorkspaceFoldersNotification" => {
                return validate_type::<WorkspaceDidChangeWorkspaceFoldersNotification>(
                    result_type,
                    data,
                )
            }
            "WindowWorkDoneProgressCancelNotification" => {
                return validate_type::<WindowWorkDoneProgressCancelNotification>(result_type, data)
            }
            "WorkspaceDidCreateFilesNotification" => {
                return validate_type::<WorkspaceDidCreateFilesNotification>(result_type, data)
            }
            "WorkspaceDidRenameFilesNotification" => {
                return validate_type::<WorkspaceDidRenameFilesNotification>(result_type, data)
            }
            "WorkspaceDidDeleteFilesNotification" => {
                return validate_type::<WorkspaceDidDeleteFilesNotification>(result_type, data)
            }
            "NotebookDocumentDidOpenNotification" => {
                return validate_type::<NotebookDocumentDidOpenNotification>(result_type, data)
            }
            "NotebookDocumentDidChangeNotification" => {
                return validate_type::<NotebookDocumentDidChangeNotification>(result_type, data)
            }
            "NotebookDocumentDidSaveNotification" => {
                return validate_type::<NotebookDocumentDidSaveNotification>(result_type, data)
            }
            "NotebookDocumentDidCloseNotification" => {
                return validate_type::<NotebookDocumentDidCloseNotification>(result_type, data)
            }
            "InitializedNotification" => {
                return validate_type::<InitializedNotification>(result_type, data)
            }
            "ExitNotification" => return validate_type::<ExitNotification>(result_type, data),
            "WorkspaceDidChangeConfigurationNotification" => {
                return validate_type::<WorkspaceDidChangeConfigurationNotification>(
                    result_type,
                    data,
                )
            }
            "WindowShowMessageNotification" => {
                return validate_type::<WindowShowMessageNotification>(result_type, data)
            }
            "WindowLogMessageNotification" => {
                return validate_type::<WindowLogMessageNotification>(result_type, data)
            }
            "TelemetryEventNotification" => {
                return validate_type::<TelemetryEventNotification>(result_type, data)
            }
            "TextDocumentDidOpenNotification" => {
                return validate_type::<TextDocumentDidOpenNotification>(result_type, data)
            }
            "TextDocumentDidChangeNotification" => {
                return validate_type::<TextDocumentDidChangeNotification>(result_type, data)
            }
            "TextDocumentDidCloseNotification" => {
                return validate_type::<TextDocumentDidCloseNotification>(result_type, data)
            }
            "TextDocumentDidSaveNotification" => {
                return validate_type::<TextDocumentDidSaveNotification>(result_type, data)
            }
            "TextDocumentWillSaveNotification" => {
                return validate_type::<TextDocumentWillSaveNotification>(result_type, data)
            }
            "WorkspaceDidChangeWatchedFilesNotification" => {
                return validate_type::<WorkspaceDidChangeWatchedFilesNotification>(
                    result_type,
                    data,
                )
            }
            "TextDocumentPublishDiagnosticsNotification" => {
                return validate_type::<TextDocumentPublishDiagnosticsNotification>(
                    result_type,
                    data,
                )
            }
            "SetTraceNotification" => {
                return validate_type::<SetTraceNotification>(result_type, data)
            }
            "LogTraceNotification" => {
                return validate_type::<LogTraceNotification>(result_type, data)
            }
            "CancelRequestNotification" => {
                return validate_type::<CancelRequestNotification>(result_type, data)
            }
            "ProgressNotification" => {
                return validate_type::<ProgressNotification>(result_type, data)
            }
            // GENERATED_TEST_CODE:end
            _ => (),
        }
    }

    fn validate_file(file_path: &str) {
        let basename = std::path::Path::new(&file_path)
            .file_name()
            .unwrap()
            .to_str()
            .unwrap();
        let type_name_result_type: Vec<&str> = basename.split("-").collect();
        let lsp_type = type_name_result_type[0];
        let result_type = type_name_result_type[1];
        let data = &fs::read_to_string(file_path).unwrap();

        validate(&lsp_type, &result_type, &data);
    }

    #[test]

    fn test_generated_data() {
        println!("Running generated data tests");
        let cwd = std::env::current_dir()
            .unwrap()
            .join("../../packages/testdata");
        let env_value = std::env::var("LSP_TEST_DATA_PATH")
            .unwrap_or_else(|_| cwd.to_str().unwrap().to_string());
        println!("TEST_DATA_ROOT: {}", env_value);
        for json_file in get_all_json_files(env_value.as_str()) {
            validate_file(&json_file);
        }
    }
}

fn main() {
    // Use data from test error report here to debug
    // let json_data = "";

    // Update the type here to debug
    // serde_json::from_str::<lsprotocol::TextDocumentCompletionRequest>(json_data).unwrap();
}
