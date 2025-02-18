// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

// ****** THIS IS A GENERATED FILE, DO NOT EDIT. ******
// Steps to generate:
// 1. Checkout https://github.com/microsoft/lsprotocol
// 2. Install nox: `python -m pip install nox`
// 3. Run command: `python -m nox --session build_lsp`

use rust_decimal::Decimal;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
/// This type allows extending any string enum to support custom values.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum CustomStringEnum<T> {
    /// The value is one of the known enum values.
    Known(T),
    /// The value is custom.
    Custom(String),
}

/// This type allows extending any integer enum to support custom values.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum CustomIntEnum<T> {
    /// The value is one of the known enum values.
    Known(T),
    /// The value is custom.
    Custom(i32),
}

/// This allows a field to have two types.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum OR2<T, U> {
    T(T),
    U(U),
}

/// This allows a field to have three types.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum OR3<T, U, V> {
    T(T),
    U(U),
    V(V),
}

/// This allows a field to have four types.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum OR4<T, U, V, W> {
    T(T),
    U(U),
    V(V),
    W(W),
}

/// This allows a field to have five types.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum OR5<T, U, V, W, X> {
    T(T),
    U(U),
    V(V),
    W(W),
    X(X),
}

/// This allows a field to have six types.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum OR6<T, U, V, W, X, Y> {
    T(T),
    U(U),
    V(V),
    W(W),
    X(X),
    Y(Y),
}

/// This allows a field to have seven types.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum OR7<T, U, V, W, X, Y, Z> {
    T(T),
    U(U),
    V(V),
    W(W),
    X(X),
    Y(Y),
    Z(Z),
}

/// This allows a field to always have null or empty value.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum LSPNull {
    None,
}

/// The LSP any type.
/// Please note that strictly speaking a property with the value `undefined`
/// can't be converted into JSON preserving the property name. However for
/// convenience it is allowed and assumed that all these properties are
/// optional as well.
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum LSPAny {
    String(String),
    Integer(i32),
    UInteger(u32),
    Decimal(Decimal),
    Boolean(bool),
    Object(LSPObject),
    Array(LSPArray),
    Null,
}

/// LSP object definition.
/// @since 3.17.0
type LSPObject = serde_json::Value;

/// LSP arrays.
/// @since 3.17.0
type LSPArray = Vec<LSPAny>;

/// A selection range represents a part of a selection hierarchy. A selection range
/// may have a parent selection range that contains it.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub struct SelectionRange {
    /// The [range][Range] of this selection range.
    pub range: Range,

    /// The parent selection range containing this range. Therefore `parent.range` must contain `this.range`.
    pub parent: Option<Box<SelectionRange>>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum LSPRequestMethods {
    #[serde(rename = "textDocument/implementation")]
    TextDocumentImplementation,
    #[serde(rename = "textDocument/typeDefinition")]
    TextDocumentTypeDefinition,
    #[serde(rename = "workspace/workspaceFolders")]
    WorkspaceWorkspaceFolders,
    #[serde(rename = "workspace/configuration")]
    WorkspaceConfiguration,
    #[serde(rename = "textDocument/documentColor")]
    TextDocumentDocumentColor,
    #[serde(rename = "textDocument/colorPresentation")]
    TextDocumentColorPresentation,
    #[serde(rename = "textDocument/foldingRange")]
    TextDocumentFoldingRange,
    #[serde(rename = "workspace/foldingRange/refresh")]
    WorkspaceFoldingRangeRefresh,
    #[serde(rename = "textDocument/declaration")]
    TextDocumentDeclaration,
    #[serde(rename = "textDocument/selectionRange")]
    TextDocumentSelectionRange,
    #[serde(rename = "window/workDoneProgress/create")]
    WindowWorkDoneProgressCreate,
    #[serde(rename = "textDocument/prepareCallHierarchy")]
    TextDocumentPrepareCallHierarchy,
    #[serde(rename = "callHierarchy/incomingCalls")]
    CallHierarchyIncomingCalls,
    #[serde(rename = "callHierarchy/outgoingCalls")]
    CallHierarchyOutgoingCalls,
    #[serde(rename = "textDocument/semanticTokens/full")]
    TextDocumentSemanticTokensFull,
    #[serde(rename = "textDocument/semanticTokens/full/delta")]
    TextDocumentSemanticTokensFullDelta,
    #[serde(rename = "textDocument/semanticTokens/range")]
    TextDocumentSemanticTokensRange,
    #[serde(rename = "workspace/semanticTokens/refresh")]
    WorkspaceSemanticTokensRefresh,
    #[serde(rename = "window/showDocument")]
    WindowShowDocument,
    #[serde(rename = "textDocument/linkedEditingRange")]
    TextDocumentLinkedEditingRange,
    #[serde(rename = "workspace/willCreateFiles")]
    WorkspaceWillCreateFiles,
    #[serde(rename = "workspace/willRenameFiles")]
    WorkspaceWillRenameFiles,
    #[serde(rename = "workspace/willDeleteFiles")]
    WorkspaceWillDeleteFiles,
    #[serde(rename = "textDocument/moniker")]
    TextDocumentMoniker,
    #[serde(rename = "textDocument/prepareTypeHierarchy")]
    TextDocumentPrepareTypeHierarchy,
    #[serde(rename = "typeHierarchy/supertypes")]
    TypeHierarchySupertypes,
    #[serde(rename = "typeHierarchy/subtypes")]
    TypeHierarchySubtypes,
    #[serde(rename = "textDocument/inlineValue")]
    TextDocumentInlineValue,
    #[serde(rename = "workspace/inlineValue/refresh")]
    WorkspaceInlineValueRefresh,
    #[serde(rename = "textDocument/inlayHint")]
    TextDocumentInlayHint,
    #[serde(rename = "inlayHint/resolve")]
    InlayHintResolve,
    #[serde(rename = "workspace/inlayHint/refresh")]
    WorkspaceInlayHintRefresh,
    #[serde(rename = "textDocument/diagnostic")]
    TextDocumentDiagnostic,
    #[serde(rename = "workspace/diagnostic")]
    WorkspaceDiagnostic,
    #[serde(rename = "workspace/diagnostic/refresh")]
    WorkspaceDiagnosticRefresh,
    #[serde(rename = "textDocument/inlineCompletion")]
    TextDocumentInlineCompletion,
    #[serde(rename = "client/registerCapability")]
    ClientRegisterCapability,
    #[serde(rename = "client/unregisterCapability")]
    ClientUnregisterCapability,
    #[serde(rename = "initialize")]
    Initialize,
    #[serde(rename = "shutdown")]
    Shutdown,
    #[serde(rename = "window/showMessageRequest")]
    WindowShowMessageRequest,
    #[serde(rename = "textDocument/willSaveWaitUntil")]
    TextDocumentWillSaveWaitUntil,
    #[serde(rename = "textDocument/completion")]
    TextDocumentCompletion,
    #[serde(rename = "completionItem/resolve")]
    CompletionItemResolve,
    #[serde(rename = "textDocument/hover")]
    TextDocumentHover,
    #[serde(rename = "textDocument/signatureHelp")]
    TextDocumentSignatureHelp,
    #[serde(rename = "textDocument/definition")]
    TextDocumentDefinition,
    #[serde(rename = "textDocument/references")]
    TextDocumentReferences,
    #[serde(rename = "textDocument/documentHighlight")]
    TextDocumentDocumentHighlight,
    #[serde(rename = "textDocument/documentSymbol")]
    TextDocumentDocumentSymbol,
    #[serde(rename = "textDocument/codeAction")]
    TextDocumentCodeAction,
    #[serde(rename = "codeAction/resolve")]
    CodeActionResolve,
    #[serde(rename = "workspace/symbol")]
    WorkspaceSymbol,
    #[serde(rename = "workspaceSymbol/resolve")]
    WorkspaceSymbolResolve,
    #[serde(rename = "textDocument/codeLens")]
    TextDocumentCodeLens,
    #[serde(rename = "codeLens/resolve")]
    CodeLensResolve,
    #[serde(rename = "workspace/codeLens/refresh")]
    WorkspaceCodeLensRefresh,
    #[serde(rename = "textDocument/documentLink")]
    TextDocumentDocumentLink,
    #[serde(rename = "documentLink/resolve")]
    DocumentLinkResolve,
    #[serde(rename = "textDocument/formatting")]
    TextDocumentFormatting,
    #[serde(rename = "textDocument/rangeFormatting")]
    TextDocumentRangeFormatting,
    #[serde(rename = "textDocument/rangesFormatting")]
    TextDocumentRangesFormatting,
    #[serde(rename = "textDocument/onTypeFormatting")]
    TextDocumentOnTypeFormatting,
    #[serde(rename = "textDocument/rename")]
    TextDocumentRename,
    #[serde(rename = "textDocument/prepareRename")]
    TextDocumentPrepareRename,
    #[serde(rename = "workspace/executeCommand")]
    WorkspaceExecuteCommand,
    #[serde(rename = "workspace/applyEdit")]
    WorkspaceApplyEdit,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum LSPNotificationMethods {
    #[serde(rename = "workspace/didChangeWorkspaceFolders")]
    WorkspaceDidChangeWorkspaceFolders,
    #[serde(rename = "window/workDoneProgress/cancel")]
    WindowWorkDoneProgressCancel,
    #[serde(rename = "workspace/didCreateFiles")]
    WorkspaceDidCreateFiles,
    #[serde(rename = "workspace/didRenameFiles")]
    WorkspaceDidRenameFiles,
    #[serde(rename = "workspace/didDeleteFiles")]
    WorkspaceDidDeleteFiles,
    #[serde(rename = "notebookDocument/didOpen")]
    NotebookDocumentDidOpen,
    #[serde(rename = "notebookDocument/didChange")]
    NotebookDocumentDidChange,
    #[serde(rename = "notebookDocument/didSave")]
    NotebookDocumentDidSave,
    #[serde(rename = "notebookDocument/didClose")]
    NotebookDocumentDidClose,
    #[serde(rename = "initialized")]
    Initialized,
    #[serde(rename = "exit")]
    Exit,
    #[serde(rename = "workspace/didChangeConfiguration")]
    WorkspaceDidChangeConfiguration,
    #[serde(rename = "window/showMessage")]
    WindowShowMessage,
    #[serde(rename = "window/logMessage")]
    WindowLogMessage,
    #[serde(rename = "telemetry/event")]
    TelemetryEvent,
    #[serde(rename = "textDocument/didOpen")]
    TextDocumentDidOpen,
    #[serde(rename = "textDocument/didChange")]
    TextDocumentDidChange,
    #[serde(rename = "textDocument/didClose")]
    TextDocumentDidClose,
    #[serde(rename = "textDocument/didSave")]
    TextDocumentDidSave,
    #[serde(rename = "textDocument/willSave")]
    TextDocumentWillSave,
    #[serde(rename = "workspace/didChangeWatchedFiles")]
    WorkspaceDidChangeWatchedFiles,
    #[serde(rename = "textDocument/publishDiagnostics")]
    TextDocumentPublishDiagnostics,
    #[serde(rename = "$/setTrace")]
    SetTrace,
    #[serde(rename = "$/logTrace")]
    LogTrace,
    #[serde(rename = "$/cancelRequest")]
    CancelRequest,
    #[serde(rename = "$/progress")]
    Progress,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum MessageDirection {
    #[serde(rename = "both")]
    Both,
    #[serde(rename = "clientToServer")]
    ClientToServer,
    #[serde(rename = "serverToClient")]
    ServerToClient,
}

/// A set of predefined token types. This set is not fixed
/// an clients can specify additional token types via the
/// corresponding client capabilities.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum SemanticTokenTypes {
    #[serde(rename = "namespace")]
    Namespace,

    /// Represents a generic type. Acts as a fallback for types which can't be mapped to
    /// a specific type like class or enum.
    #[serde(rename = "type")]
    Type,

    #[serde(rename = "class")]
    Class,

    #[serde(rename = "enum")]
    Enum,

    #[serde(rename = "interface")]
    Interface,

    #[serde(rename = "struct")]
    Struct,

    #[serde(rename = "typeParameter")]
    TypeParameter,

    #[serde(rename = "parameter")]
    Parameter,

    #[serde(rename = "variable")]
    Variable,

    #[serde(rename = "property")]
    Property,

    #[serde(rename = "enumMember")]
    EnumMember,

    #[serde(rename = "event")]
    Event,

    #[serde(rename = "function")]
    Function,

    #[serde(rename = "method")]
    Method,

    #[serde(rename = "macro")]
    Macro,

    #[serde(rename = "keyword")]
    Keyword,

    #[serde(rename = "modifier")]
    Modifier,

    #[serde(rename = "comment")]
    Comment,

    #[serde(rename = "string")]
    String,

    #[serde(rename = "number")]
    Number,

    #[serde(rename = "regexp")]
    Regexp,

    #[serde(rename = "operator")]
    Operator,

    /// @since 3.17.0
    #[serde(rename = "decorator")]
    Decorator,
}

/// A set of predefined token modifiers. This set is not fixed
/// an clients can specify additional token types via the
/// corresponding client capabilities.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum SemanticTokenModifiers {
    #[serde(rename = "declaration")]
    Declaration,

    #[serde(rename = "definition")]
    Definition,

    #[serde(rename = "readonly")]
    Readonly,

    #[serde(rename = "static")]
    Static,

    #[serde(rename = "deprecated")]
    Deprecated,

    #[serde(rename = "abstract")]
    Abstract,

    #[serde(rename = "async")]
    Async,

    #[serde(rename = "modification")]
    Modification,

    #[serde(rename = "documentation")]
    Documentation,

    #[serde(rename = "defaultLibrary")]
    DefaultLibrary,
}

/// The document diagnostic report kinds.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum DocumentDiagnosticReportKind {
    /// A diagnostic report with a full
    /// set of problems.
    #[serde(rename = "full")]
    Full,

    /// A report indicating that the last
    /// returned report is still accurate.
    #[serde(rename = "unchanged")]
    Unchanged,
}

/// Predefined error codes.
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum ErrorCodes {
    ParseError = -32700,

    InvalidRequest = -32600,

    MethodNotFound = -32601,

    InvalidParams = -32602,

    InternalError = -32603,

    /// Error code indicating that a server received a notification or
    /// request before the server has received the `initialize` request.
    ServerNotInitialized = -32002,

    UnknownErrorCode = -32001,
}
impl Serialize for ErrorCodes {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            ErrorCodes::ParseError => serializer.serialize_i32(-32700),
            ErrorCodes::InvalidRequest => serializer.serialize_i32(-32600),
            ErrorCodes::MethodNotFound => serializer.serialize_i32(-32601),
            ErrorCodes::InvalidParams => serializer.serialize_i32(-32602),
            ErrorCodes::InternalError => serializer.serialize_i32(-32603),
            ErrorCodes::ServerNotInitialized => serializer.serialize_i32(-32002),
            ErrorCodes::UnknownErrorCode => serializer.serialize_i32(-32001),
        }
    }
}
impl<'de> Deserialize<'de> for ErrorCodes {
    fn deserialize<D>(deserializer: D) -> Result<ErrorCodes, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            -32700 => Ok(ErrorCodes::ParseError),
            -32600 => Ok(ErrorCodes::InvalidRequest),
            -32601 => Ok(ErrorCodes::MethodNotFound),
            -32602 => Ok(ErrorCodes::InvalidParams),
            -32603 => Ok(ErrorCodes::InternalError),
            -32002 => Ok(ErrorCodes::ServerNotInitialized),
            -32001 => Ok(ErrorCodes::UnknownErrorCode),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

#[derive(PartialEq, Debug, Eq, Clone)]
pub enum LSPErrorCodes {
    /// A request failed but it was syntactically correct, e.g the
    /// method name was known and the parameters were valid. The error
    /// message should contain human readable information about why
    /// the request failed.
    ///
    /// @since 3.17.0
    RequestFailed = -32803,

    /// The server cancelled the request. This error code should
    /// only be used for requests that explicitly support being
    /// server cancellable.
    ///
    /// @since 3.17.0
    ServerCancelled = -32802,

    /// The server detected that the content of a document got
    /// modified outside normal conditions. A server should
    /// NOT send this error code if it detects a content change
    /// in it unprocessed messages. The result even computed
    /// on an older state might still be useful for the client.
    ///
    /// If a client decides that a result is not of any use anymore
    /// the client should cancel the request.
    ContentModified = -32801,

    /// The client has canceled a request and a server as detected
    /// the cancel.
    RequestCancelled = -32800,
}
impl Serialize for LSPErrorCodes {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            LSPErrorCodes::RequestFailed => serializer.serialize_i32(-32803),
            LSPErrorCodes::ServerCancelled => serializer.serialize_i32(-32802),
            LSPErrorCodes::ContentModified => serializer.serialize_i32(-32801),
            LSPErrorCodes::RequestCancelled => serializer.serialize_i32(-32800),
        }
    }
}
impl<'de> Deserialize<'de> for LSPErrorCodes {
    fn deserialize<D>(deserializer: D) -> Result<LSPErrorCodes, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            -32803 => Ok(LSPErrorCodes::RequestFailed),
            -32802 => Ok(LSPErrorCodes::ServerCancelled),
            -32801 => Ok(LSPErrorCodes::ContentModified),
            -32800 => Ok(LSPErrorCodes::RequestCancelled),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// A set of predefined range kinds.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum FoldingRangeKind {
    /// Folding range for a comment
    #[serde(rename = "comment")]
    Comment,

    /// Folding range for an import or include
    #[serde(rename = "imports")]
    Imports,

    /// Folding range for a region (e.g. `#region`)
    #[serde(rename = "region")]
    Region,
}

/// A symbol kind.
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum SymbolKind {
    File = 1,

    Module = 2,

    Namespace = 3,

    Package = 4,

    Class = 5,

    Method = 6,

    Property = 7,

    Field = 8,

    Constructor = 9,

    Enum = 10,

    Interface = 11,

    Function = 12,

    Variable = 13,

    Constant = 14,

    String = 15,

    Number = 16,

    Boolean = 17,

    Array = 18,

    Object = 19,

    Key = 20,

    Null = 21,

    EnumMember = 22,

    Struct = 23,

    Event = 24,

    Operator = 25,

    TypeParameter = 26,
}
impl Serialize for SymbolKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            SymbolKind::File => serializer.serialize_i32(1),
            SymbolKind::Module => serializer.serialize_i32(2),
            SymbolKind::Namespace => serializer.serialize_i32(3),
            SymbolKind::Package => serializer.serialize_i32(4),
            SymbolKind::Class => serializer.serialize_i32(5),
            SymbolKind::Method => serializer.serialize_i32(6),
            SymbolKind::Property => serializer.serialize_i32(7),
            SymbolKind::Field => serializer.serialize_i32(8),
            SymbolKind::Constructor => serializer.serialize_i32(9),
            SymbolKind::Enum => serializer.serialize_i32(10),
            SymbolKind::Interface => serializer.serialize_i32(11),
            SymbolKind::Function => serializer.serialize_i32(12),
            SymbolKind::Variable => serializer.serialize_i32(13),
            SymbolKind::Constant => serializer.serialize_i32(14),
            SymbolKind::String => serializer.serialize_i32(15),
            SymbolKind::Number => serializer.serialize_i32(16),
            SymbolKind::Boolean => serializer.serialize_i32(17),
            SymbolKind::Array => serializer.serialize_i32(18),
            SymbolKind::Object => serializer.serialize_i32(19),
            SymbolKind::Key => serializer.serialize_i32(20),
            SymbolKind::Null => serializer.serialize_i32(21),
            SymbolKind::EnumMember => serializer.serialize_i32(22),
            SymbolKind::Struct => serializer.serialize_i32(23),
            SymbolKind::Event => serializer.serialize_i32(24),
            SymbolKind::Operator => serializer.serialize_i32(25),
            SymbolKind::TypeParameter => serializer.serialize_i32(26),
        }
    }
}
impl<'de> Deserialize<'de> for SymbolKind {
    fn deserialize<D>(deserializer: D) -> Result<SymbolKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(SymbolKind::File),
            2 => Ok(SymbolKind::Module),
            3 => Ok(SymbolKind::Namespace),
            4 => Ok(SymbolKind::Package),
            5 => Ok(SymbolKind::Class),
            6 => Ok(SymbolKind::Method),
            7 => Ok(SymbolKind::Property),
            8 => Ok(SymbolKind::Field),
            9 => Ok(SymbolKind::Constructor),
            10 => Ok(SymbolKind::Enum),
            11 => Ok(SymbolKind::Interface),
            12 => Ok(SymbolKind::Function),
            13 => Ok(SymbolKind::Variable),
            14 => Ok(SymbolKind::Constant),
            15 => Ok(SymbolKind::String),
            16 => Ok(SymbolKind::Number),
            17 => Ok(SymbolKind::Boolean),
            18 => Ok(SymbolKind::Array),
            19 => Ok(SymbolKind::Object),
            20 => Ok(SymbolKind::Key),
            21 => Ok(SymbolKind::Null),
            22 => Ok(SymbolKind::EnumMember),
            23 => Ok(SymbolKind::Struct),
            24 => Ok(SymbolKind::Event),
            25 => Ok(SymbolKind::Operator),
            26 => Ok(SymbolKind::TypeParameter),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// Symbol tags are extra annotations that tweak the rendering of a symbol.
///
/// @since 3.16
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum SymbolTag {
    /// Render a symbol as obsolete, usually using a strike-out.
    Deprecated = 1,
}
impl Serialize for SymbolTag {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            SymbolTag::Deprecated => serializer.serialize_i32(1),
        }
    }
}
impl<'de> Deserialize<'de> for SymbolTag {
    fn deserialize<D>(deserializer: D) -> Result<SymbolTag, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(SymbolTag::Deprecated),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// Moniker uniqueness level to define scope of the moniker.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum UniquenessLevel {
    /// The moniker is only unique inside a document
    #[serde(rename = "document")]
    Document,

    /// The moniker is unique inside a project for which a dump got created
    #[serde(rename = "project")]
    Project,

    /// The moniker is unique inside the group to which a project belongs
    #[serde(rename = "group")]
    Group,

    /// The moniker is unique inside the moniker scheme.
    #[serde(rename = "scheme")]
    Scheme,

    /// The moniker is globally unique
    #[serde(rename = "global")]
    Global,
}

/// The moniker kind.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum MonikerKind {
    /// The moniker represent a symbol that is imported into a project
    #[serde(rename = "import")]
    Import,

    /// The moniker represents a symbol that is exported from a project
    #[serde(rename = "export")]
    Export,

    /// The moniker represents a symbol that is local to a project (e.g. a local
    /// variable of a function, a class not visible outside the project, ...)
    #[serde(rename = "local")]
    Local,
}

/// Inlay hint kinds.
///
/// @since 3.17.0
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum InlayHintKind {
    /// An inlay hint that for a type annotation.
    Type = 1,

    /// An inlay hint that is for a parameter.
    Parameter = 2,
}
impl Serialize for InlayHintKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            InlayHintKind::Type => serializer.serialize_i32(1),
            InlayHintKind::Parameter => serializer.serialize_i32(2),
        }
    }
}
impl<'de> Deserialize<'de> for InlayHintKind {
    fn deserialize<D>(deserializer: D) -> Result<InlayHintKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(InlayHintKind::Type),
            2 => Ok(InlayHintKind::Parameter),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// The message type
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum MessageType {
    /// An error message.
    Error = 1,

    /// A warning message.
    Warning = 2,

    /// An information message.
    Info = 3,

    /// A log message.
    Log = 4,

    /// A debug message.
    ///
    /// @since 3.18.0
    Debug = 5,
}
impl Serialize for MessageType {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            MessageType::Error => serializer.serialize_i32(1),
            MessageType::Warning => serializer.serialize_i32(2),
            MessageType::Info => serializer.serialize_i32(3),
            MessageType::Log => serializer.serialize_i32(4),
            MessageType::Debug => serializer.serialize_i32(5),
        }
    }
}
impl<'de> Deserialize<'de> for MessageType {
    fn deserialize<D>(deserializer: D) -> Result<MessageType, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(MessageType::Error),
            2 => Ok(MessageType::Warning),
            3 => Ok(MessageType::Info),
            4 => Ok(MessageType::Log),
            5 => Ok(MessageType::Debug),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// Defines how the host (editor) should sync
/// document changes to the language server.
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum TextDocumentSyncKind {
    /// Documents should not be synced at all.
    None = 0,

    /// Documents are synced by always sending the full content
    /// of the document.
    Full = 1,

    /// Documents are synced by sending the full content on open.
    /// After that only incremental updates to the document are
    /// send.
    Incremental = 2,
}
impl Serialize for TextDocumentSyncKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            TextDocumentSyncKind::None => serializer.serialize_i32(0),
            TextDocumentSyncKind::Full => serializer.serialize_i32(1),
            TextDocumentSyncKind::Incremental => serializer.serialize_i32(2),
        }
    }
}
impl<'de> Deserialize<'de> for TextDocumentSyncKind {
    fn deserialize<D>(deserializer: D) -> Result<TextDocumentSyncKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            0 => Ok(TextDocumentSyncKind::None),
            1 => Ok(TextDocumentSyncKind::Full),
            2 => Ok(TextDocumentSyncKind::Incremental),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// Represents reasons why a text document is saved.
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum TextDocumentSaveReason {
    /// Manually triggered, e.g. by the user pressing save, by starting debugging,
    /// or by an API call.
    Manual = 1,

    /// Automatic after a delay.
    AfterDelay = 2,

    /// When the editor lost focus.
    FocusOut = 3,
}
impl Serialize for TextDocumentSaveReason {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            TextDocumentSaveReason::Manual => serializer.serialize_i32(1),
            TextDocumentSaveReason::AfterDelay => serializer.serialize_i32(2),
            TextDocumentSaveReason::FocusOut => serializer.serialize_i32(3),
        }
    }
}
impl<'de> Deserialize<'de> for TextDocumentSaveReason {
    fn deserialize<D>(deserializer: D) -> Result<TextDocumentSaveReason, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(TextDocumentSaveReason::Manual),
            2 => Ok(TextDocumentSaveReason::AfterDelay),
            3 => Ok(TextDocumentSaveReason::FocusOut),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// The kind of a completion entry.
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum CompletionItemKind {
    Text = 1,

    Method = 2,

    Function = 3,

    Constructor = 4,

    Field = 5,

    Variable = 6,

    Class = 7,

    Interface = 8,

    Module = 9,

    Property = 10,

    Unit = 11,

    Value = 12,

    Enum = 13,

    Keyword = 14,

    Snippet = 15,

    Color = 16,

    File = 17,

    Reference = 18,

    Folder = 19,

    EnumMember = 20,

    Constant = 21,

    Struct = 22,

    Event = 23,

    Operator = 24,

    TypeParameter = 25,
}
impl Serialize for CompletionItemKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            CompletionItemKind::Text => serializer.serialize_i32(1),
            CompletionItemKind::Method => serializer.serialize_i32(2),
            CompletionItemKind::Function => serializer.serialize_i32(3),
            CompletionItemKind::Constructor => serializer.serialize_i32(4),
            CompletionItemKind::Field => serializer.serialize_i32(5),
            CompletionItemKind::Variable => serializer.serialize_i32(6),
            CompletionItemKind::Class => serializer.serialize_i32(7),
            CompletionItemKind::Interface => serializer.serialize_i32(8),
            CompletionItemKind::Module => serializer.serialize_i32(9),
            CompletionItemKind::Property => serializer.serialize_i32(10),
            CompletionItemKind::Unit => serializer.serialize_i32(11),
            CompletionItemKind::Value => serializer.serialize_i32(12),
            CompletionItemKind::Enum => serializer.serialize_i32(13),
            CompletionItemKind::Keyword => serializer.serialize_i32(14),
            CompletionItemKind::Snippet => serializer.serialize_i32(15),
            CompletionItemKind::Color => serializer.serialize_i32(16),
            CompletionItemKind::File => serializer.serialize_i32(17),
            CompletionItemKind::Reference => serializer.serialize_i32(18),
            CompletionItemKind::Folder => serializer.serialize_i32(19),
            CompletionItemKind::EnumMember => serializer.serialize_i32(20),
            CompletionItemKind::Constant => serializer.serialize_i32(21),
            CompletionItemKind::Struct => serializer.serialize_i32(22),
            CompletionItemKind::Event => serializer.serialize_i32(23),
            CompletionItemKind::Operator => serializer.serialize_i32(24),
            CompletionItemKind::TypeParameter => serializer.serialize_i32(25),
        }
    }
}
impl<'de> Deserialize<'de> for CompletionItemKind {
    fn deserialize<D>(deserializer: D) -> Result<CompletionItemKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(CompletionItemKind::Text),
            2 => Ok(CompletionItemKind::Method),
            3 => Ok(CompletionItemKind::Function),
            4 => Ok(CompletionItemKind::Constructor),
            5 => Ok(CompletionItemKind::Field),
            6 => Ok(CompletionItemKind::Variable),
            7 => Ok(CompletionItemKind::Class),
            8 => Ok(CompletionItemKind::Interface),
            9 => Ok(CompletionItemKind::Module),
            10 => Ok(CompletionItemKind::Property),
            11 => Ok(CompletionItemKind::Unit),
            12 => Ok(CompletionItemKind::Value),
            13 => Ok(CompletionItemKind::Enum),
            14 => Ok(CompletionItemKind::Keyword),
            15 => Ok(CompletionItemKind::Snippet),
            16 => Ok(CompletionItemKind::Color),
            17 => Ok(CompletionItemKind::File),
            18 => Ok(CompletionItemKind::Reference),
            19 => Ok(CompletionItemKind::Folder),
            20 => Ok(CompletionItemKind::EnumMember),
            21 => Ok(CompletionItemKind::Constant),
            22 => Ok(CompletionItemKind::Struct),
            23 => Ok(CompletionItemKind::Event),
            24 => Ok(CompletionItemKind::Operator),
            25 => Ok(CompletionItemKind::TypeParameter),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// Completion item tags are extra annotations that tweak the rendering of a completion
/// item.
///
/// @since 3.15.0
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum CompletionItemTag {
    /// Render a completion as obsolete, usually using a strike-out.
    Deprecated = 1,
}
impl Serialize for CompletionItemTag {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            CompletionItemTag::Deprecated => serializer.serialize_i32(1),
        }
    }
}
impl<'de> Deserialize<'de> for CompletionItemTag {
    fn deserialize<D>(deserializer: D) -> Result<CompletionItemTag, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(CompletionItemTag::Deprecated),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// Defines whether the insert text in a completion item should be interpreted as
/// plain text or a snippet.
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum InsertTextFormat {
    /// The primary text to be inserted is treated as a plain string.
    PlainText = 1,

    /// The primary text to be inserted is treated as a snippet.
    ///
    /// A snippet can define tab stops and placeholders with `$1`, `$2`
    /// and `${3:foo}`. `$0` defines the final tab stop, it defaults to
    /// the end of the snippet. Placeholders with equal identifiers are linked,
    /// that is typing in one will update others too.
    ///
    /// See also: https://microsoft.github.io/language-server-protocol/specifications/specification-current/#snippet_syntax
    Snippet = 2,
}
impl Serialize for InsertTextFormat {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            InsertTextFormat::PlainText => serializer.serialize_i32(1),
            InsertTextFormat::Snippet => serializer.serialize_i32(2),
        }
    }
}
impl<'de> Deserialize<'de> for InsertTextFormat {
    fn deserialize<D>(deserializer: D) -> Result<InsertTextFormat, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(InsertTextFormat::PlainText),
            2 => Ok(InsertTextFormat::Snippet),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// How whitespace and indentation is handled during completion
/// item insertion.
///
/// @since 3.16.0
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum InsertTextMode {
    /// The insertion or replace strings is taken as it is. If the
    /// value is multi line the lines below the cursor will be
    /// inserted using the indentation defined in the string value.
    /// The client will not apply any kind of adjustments to the
    /// string.
    AsIs = 1,

    /// The editor adjusts leading whitespace of new lines so that
    /// they match the indentation up to the cursor of the line for
    /// which the item is accepted.
    ///
    /// Consider a line like this: <2tabs><cursor><3tabs>foo. Accepting a
    /// multi line completion item is indented using 2 tabs and all
    /// following lines inserted will be indented using 2 tabs as well.
    AdjustIndentation = 2,
}
impl Serialize for InsertTextMode {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            InsertTextMode::AsIs => serializer.serialize_i32(1),
            InsertTextMode::AdjustIndentation => serializer.serialize_i32(2),
        }
    }
}
impl<'de> Deserialize<'de> for InsertTextMode {
    fn deserialize<D>(deserializer: D) -> Result<InsertTextMode, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(InsertTextMode::AsIs),
            2 => Ok(InsertTextMode::AdjustIndentation),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// A document highlight kind.
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum DocumentHighlightKind {
    /// A textual occurrence.
    Text = 1,

    /// Read-access of a symbol, like reading a variable.
    Read = 2,

    /// Write-access of a symbol, like writing to a variable.
    Write = 3,
}
impl Serialize for DocumentHighlightKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            DocumentHighlightKind::Text => serializer.serialize_i32(1),
            DocumentHighlightKind::Read => serializer.serialize_i32(2),
            DocumentHighlightKind::Write => serializer.serialize_i32(3),
        }
    }
}
impl<'de> Deserialize<'de> for DocumentHighlightKind {
    fn deserialize<D>(deserializer: D) -> Result<DocumentHighlightKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(DocumentHighlightKind::Text),
            2 => Ok(DocumentHighlightKind::Read),
            3 => Ok(DocumentHighlightKind::Write),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// A set of predefined code action kinds
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum CodeActionKind {
    /// Empty kind.
    #[serde(rename = "")]
    Empty,

    /// Base kind for quickfix actions: 'quickfix'
    #[serde(rename = "quickfix")]
    QuickFix,

    /// Base kind for refactoring actions: 'refactor'
    #[serde(rename = "refactor")]
    Refactor,

    /// Base kind for refactoring extraction actions: 'refactor.extract'
    ///
    /// Example extract actions:
    ///
    /// - Extract method
    /// - Extract function
    /// - Extract variable
    /// - Extract interface from class
    /// - ...
    #[serde(rename = "refactor.extract")]
    RefactorExtract,

    /// Base kind for refactoring inline actions: 'refactor.inline'
    ///
    /// Example inline actions:
    ///
    /// - Inline function
    /// - Inline variable
    /// - Inline constant
    /// - ...
    #[serde(rename = "refactor.inline")]
    RefactorInline,

    /// Base kind for refactoring rewrite actions: 'refactor.rewrite'
    ///
    /// Example rewrite actions:
    ///
    /// - Convert JavaScript function to class
    /// - Add or remove parameter
    /// - Encapsulate field
    /// - Make method static
    /// - Move method to base class
    /// - ...
    #[serde(rename = "refactor.rewrite")]
    RefactorRewrite,

    /// Base kind for source actions: `source`
    ///
    /// Source code actions apply to the entire file.
    #[serde(rename = "source")]
    Source,

    /// Base kind for an organize imports source action: `source.organizeImports`
    #[serde(rename = "source.organizeImports")]
    SourceOrganizeImports,

    /// Base kind for auto-fix source actions: `source.fixAll`.
    ///
    /// Fix all actions automatically fix errors that have a clear fix that do not require user input.
    /// They should not suppress errors or perform unsafe fixes such as generating new types or classes.
    ///
    /// @since 3.15.0
    #[serde(rename = "source.fixAll")]
    SourceFixAll,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum TraceValues {
    /// Turn tracing off.
    #[serde(rename = "off")]
    Off,

    /// Trace messages only.
    #[serde(rename = "messages")]
    Messages,

    /// Verbose message tracing.
    #[serde(rename = "verbose")]
    Verbose,
}

/// Describes the content type that a client supports in various
/// result literals like `Hover`, `ParameterInfo` or `CompletionItem`.
///
/// Please note that `MarkupKinds` must not start with a `$`. This kinds
/// are reserved for internal usage.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum MarkupKind {
    /// Plain text is supported as a content format
    #[serde(rename = "plaintext")]
    PlainText,

    /// Markdown is supported as a content format
    #[serde(rename = "markdown")]
    Markdown,
}

/// Describes how an [inline completion provider][InlineCompletionItemProvider] was triggered.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum InlineCompletionTriggerKind {
    /// Completion was triggered explicitly by a user gesture.
    Invoked = 0,

    /// Completion was triggered automatically while editing.
    Automatic = 1,
}
impl Serialize for InlineCompletionTriggerKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            InlineCompletionTriggerKind::Invoked => serializer.serialize_i32(0),
            InlineCompletionTriggerKind::Automatic => serializer.serialize_i32(1),
        }
    }
}
impl<'de> Deserialize<'de> for InlineCompletionTriggerKind {
    fn deserialize<D>(deserializer: D) -> Result<InlineCompletionTriggerKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            0 => Ok(InlineCompletionTriggerKind::Invoked),
            1 => Ok(InlineCompletionTriggerKind::Automatic),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// A set of predefined position encoding kinds.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum PositionEncodingKind {
    /// Character offsets count UTF-8 code units (e.g. bytes).
    #[serde(rename = "utf-8")]
    Utf8,

    /// Character offsets count UTF-16 code units.
    ///
    /// This is the default and must always be supported
    /// by servers
    #[serde(rename = "utf-16")]
    Utf16,

    /// Character offsets count UTF-32 code units.
    ///
    /// Implementation note: these are the same as Unicode codepoints,
    /// so this `PositionEncodingKind` may also be used for an
    /// encoding-agnostic representation of character offsets.
    #[serde(rename = "utf-32")]
    Utf32,
}

/// The file event type
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum FileChangeType {
    /// The file got created.
    Created = 1,

    /// The file got changed.
    Changed = 2,

    /// The file got deleted.
    Deleted = 3,
}
impl Serialize for FileChangeType {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            FileChangeType::Created => serializer.serialize_i32(1),
            FileChangeType::Changed => serializer.serialize_i32(2),
            FileChangeType::Deleted => serializer.serialize_i32(3),
        }
    }
}
impl<'de> Deserialize<'de> for FileChangeType {
    fn deserialize<D>(deserializer: D) -> Result<FileChangeType, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(FileChangeType::Created),
            2 => Ok(FileChangeType::Changed),
            3 => Ok(FileChangeType::Deleted),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

#[derive(PartialEq, Debug, Eq, Clone)]
pub enum WatchKind {
    /// Interested in create events.
    Create = 1,

    /// Interested in change events
    Change = 2,

    /// Interested in delete events
    Delete = 4,
}
impl Serialize for WatchKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            WatchKind::Create => serializer.serialize_i32(1),
            WatchKind::Change => serializer.serialize_i32(2),
            WatchKind::Delete => serializer.serialize_i32(4),
        }
    }
}
impl<'de> Deserialize<'de> for WatchKind {
    fn deserialize<D>(deserializer: D) -> Result<WatchKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(WatchKind::Create),
            2 => Ok(WatchKind::Change),
            4 => Ok(WatchKind::Delete),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// The diagnostic's severity.
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum DiagnosticSeverity {
    /// Reports an error.
    Error = 1,

    /// Reports a warning.
    Warning = 2,

    /// Reports an information.
    Information = 3,

    /// Reports a hint.
    Hint = 4,
}
impl Serialize for DiagnosticSeverity {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            DiagnosticSeverity::Error => serializer.serialize_i32(1),
            DiagnosticSeverity::Warning => serializer.serialize_i32(2),
            DiagnosticSeverity::Information => serializer.serialize_i32(3),
            DiagnosticSeverity::Hint => serializer.serialize_i32(4),
        }
    }
}
impl<'de> Deserialize<'de> for DiagnosticSeverity {
    fn deserialize<D>(deserializer: D) -> Result<DiagnosticSeverity, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(DiagnosticSeverity::Error),
            2 => Ok(DiagnosticSeverity::Warning),
            3 => Ok(DiagnosticSeverity::Information),
            4 => Ok(DiagnosticSeverity::Hint),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// The diagnostic tags.
///
/// @since 3.15.0
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum DiagnosticTag {
    /// Unused or unnecessary code.
    ///
    /// Clients are allowed to render diagnostics with this tag faded out instead of having
    /// an error squiggle.
    Unnecessary = 1,

    /// Deprecated or obsolete code.
    ///
    /// Clients are allowed to rendered diagnostics with this tag strike through.
    Deprecated = 2,
}
impl Serialize for DiagnosticTag {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            DiagnosticTag::Unnecessary => serializer.serialize_i32(1),
            DiagnosticTag::Deprecated => serializer.serialize_i32(2),
        }
    }
}
impl<'de> Deserialize<'de> for DiagnosticTag {
    fn deserialize<D>(deserializer: D) -> Result<DiagnosticTag, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(DiagnosticTag::Unnecessary),
            2 => Ok(DiagnosticTag::Deprecated),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// How a completion was triggered
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum CompletionTriggerKind {
    /// Completion was triggered by typing an identifier (24x7 code
    /// complete), manual invocation (e.g Ctrl+Space) or via API.
    Invoked = 1,

    /// Completion was triggered by a trigger character specified by
    /// the `triggerCharacters` properties of the `CompletionRegistrationOptions`.
    TriggerCharacter = 2,

    /// Completion was re-triggered as current completion list is incomplete
    TriggerForIncompleteCompletions = 3,
}
impl Serialize for CompletionTriggerKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            CompletionTriggerKind::Invoked => serializer.serialize_i32(1),
            CompletionTriggerKind::TriggerCharacter => serializer.serialize_i32(2),
            CompletionTriggerKind::TriggerForIncompleteCompletions => serializer.serialize_i32(3),
        }
    }
}
impl<'de> Deserialize<'de> for CompletionTriggerKind {
    fn deserialize<D>(deserializer: D) -> Result<CompletionTriggerKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(CompletionTriggerKind::Invoked),
            2 => Ok(CompletionTriggerKind::TriggerCharacter),
            3 => Ok(CompletionTriggerKind::TriggerForIncompleteCompletions),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// How a signature help was triggered.
///
/// @since 3.15.0
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum SignatureHelpTriggerKind {
    /// Signature help was invoked manually by the user or by a command.
    Invoked = 1,

    /// Signature help was triggered by a trigger character.
    TriggerCharacter = 2,

    /// Signature help was triggered by the cursor moving or by the document content changing.
    ContentChange = 3,
}
impl Serialize for SignatureHelpTriggerKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            SignatureHelpTriggerKind::Invoked => serializer.serialize_i32(1),
            SignatureHelpTriggerKind::TriggerCharacter => serializer.serialize_i32(2),
            SignatureHelpTriggerKind::ContentChange => serializer.serialize_i32(3),
        }
    }
}
impl<'de> Deserialize<'de> for SignatureHelpTriggerKind {
    fn deserialize<D>(deserializer: D) -> Result<SignatureHelpTriggerKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(SignatureHelpTriggerKind::Invoked),
            2 => Ok(SignatureHelpTriggerKind::TriggerCharacter),
            3 => Ok(SignatureHelpTriggerKind::ContentChange),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// The reason why code actions were requested.
///
/// @since 3.17.0
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum CodeActionTriggerKind {
    /// Code actions were explicitly requested by the user or by an extension.
    Invoked = 1,

    /// Code actions were requested automatically.
    ///
    /// This typically happens when current selection in a file changes, but can
    /// also be triggered when file content changes.
    Automatic = 2,
}
impl Serialize for CodeActionTriggerKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            CodeActionTriggerKind::Invoked => serializer.serialize_i32(1),
            CodeActionTriggerKind::Automatic => serializer.serialize_i32(2),
        }
    }
}
impl<'de> Deserialize<'de> for CodeActionTriggerKind {
    fn deserialize<D>(deserializer: D) -> Result<CodeActionTriggerKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(CodeActionTriggerKind::Invoked),
            2 => Ok(CodeActionTriggerKind::Automatic),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

/// A pattern kind describing if a glob pattern matches a file a folder or
/// both.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum FileOperationPatternKind {
    /// The pattern matches a file only.
    #[serde(rename = "file")]
    File,

    /// The pattern matches a folder only.
    #[serde(rename = "folder")]
    Folder,
}

/// A notebook cell kind.
///
/// @since 3.17.0
#[derive(PartialEq, Debug, Eq, Clone)]
pub enum NotebookCellKind {
    /// A markup-cell is formatted source that is used for display.
    Markup = 1,

    /// A code-cell is source code.
    Code = 2,
}
impl Serialize for NotebookCellKind {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            NotebookCellKind::Markup => serializer.serialize_i32(1),
            NotebookCellKind::Code => serializer.serialize_i32(2),
        }
    }
}
impl<'de> Deserialize<'de> for NotebookCellKind {
    fn deserialize<D>(deserializer: D) -> Result<NotebookCellKind, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(NotebookCellKind::Markup),
            2 => Ok(NotebookCellKind::Code),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum ResourceOperationKind {
    /// Supports creating new files and folders.
    #[serde(rename = "create")]
    Create,

    /// Supports renaming existing files and folders.
    #[serde(rename = "rename")]
    Rename,

    /// Supports deleting existing files and folders.
    #[serde(rename = "delete")]
    Delete,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum FailureHandlingKind {
    /// Applying the workspace change is simply aborted if one of the changes provided
    /// fails. All operations executed before the failing operation stay executed.
    #[serde(rename = "abort")]
    Abort,

    /// All operations are executed transactional. That means they either all
    /// succeed or no changes at all are applied to the workspace.
    #[serde(rename = "transactional")]
    Transactional,

    /// If the workspace edit contains only textual file changes they are executed transactional.
    /// If resource changes (create, rename or delete file) are part of the change the failure
    /// handling strategy is abort.
    #[serde(rename = "textOnlyTransactional")]
    TextOnlyTransactional,

    /// The client tries to undo the operations already executed. But there is no
    /// guarantee that this is succeeding.
    #[serde(rename = "undo")]
    Undo,
}

#[derive(PartialEq, Debug, Eq, Clone)]
pub enum PrepareSupportDefaultBehavior {
    /// The client's default behavior is to select the identifier
    /// according the to language's syntax rule.
    Identifier = 1,
}
impl Serialize for PrepareSupportDefaultBehavior {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self {
            PrepareSupportDefaultBehavior::Identifier => serializer.serialize_i32(1),
        }
    }
}
impl<'de> Deserialize<'de> for PrepareSupportDefaultBehavior {
    fn deserialize<D>(deserializer: D) -> Result<PrepareSupportDefaultBehavior, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let value = i32::deserialize(deserializer)?;
        match value {
            1 => Ok(PrepareSupportDefaultBehavior::Identifier),
            _ => Err(serde::de::Error::custom("Unexpected value")),
        }
    }
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
pub enum TokenFormat {
    #[serde(rename = "relative")]
    Relative,
}

/// The definition of a symbol represented as one or many [locations][Location].
/// For most programming languages there is only one location at which a symbol is
/// defined.
///
/// Servers should prefer returning `DefinitionLink` over `Definition` if supported
/// by the client.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum Definition {
    One(Location),
    Many(Vec<Location>),
}

/// Information about where a symbol is defined.
///
/// Provides additional metadata over normal [location][Location] definitions, including the range of
/// the defining symbol
pub type DefinitionLink = LocationLink;

/// The declaration of a symbol representation as one or many [locations][Location].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum Declaration {
    One(Location),
    Many(Vec<Location>),
}

/// Information about where a symbol is declared.
///
/// Provides additional metadata over normal [location][Location] declarations, including the range of
/// the declaring symbol.
///
/// Servers should prefer returning `DeclarationLink` over `Declaration` if supported
/// by the client.
pub type DeclarationLink = LocationLink;

/// Inline value information can be provided by different means:
/// - directly as a text value (class InlineValueText).
/// - as a name to use for a variable lookup (class InlineValueVariableLookup)
/// - as an evaluatable expression (class InlineValueEvaluatableExpression)
/// The InlineValue types combines all inline value types into one type.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum InlineValue {
    Text(InlineValueText),
    VariableLookup(InlineValueVariableLookup),
    EvaluatableExpression(InlineValueEvaluatableExpression),
}

/// The result of a document diagnostic pull request. A report can
/// either be a full report containing all diagnostics for the
/// requested document or an unchanged report indicating that nothing
/// has changed in terms of diagnostics in comparison to the last
/// pull request.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum DocumentDiagnosticReport {
    Full(RelatedFullDocumentDiagnosticReport),
    Unchanged(RelatedUnchangedDocumentDiagnosticReport),
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum PrepareRenameResult {
    Range(Range),
    PrepareRenamePlaceholder(PrepareRenamePlaceholder),
    DefaultBehavior(PrepareRenameDefaultBehavior),
}

/// A document selector is the combination of one or many document filters.
///
/// @sample `let sel:DocumentSelector = [{ language: 'typescript' }, { language: 'json', pattern: '**∕tsconfig.json' }]`;
///
/// The use of a string as a document filter is deprecated @since 3.16.0.
pub type DocumentSelector = Vec<DocumentFilter>;

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum ProgressToken {
    Int(i32),
    String(String),
}

/// An identifier to refer to a change annotation stored with a workspace edit.
pub type ChangeAnnotationIdentifier = String;

/// A workspace diagnostic document report.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum WorkspaceDocumentDiagnosticReport {
    Full(WorkspaceFullDocumentDiagnosticReport),
    Unchanged(WorkspaceUnchangedDocumentDiagnosticReport),
}

/// An event describing a change to a text document. If only a text is provided
/// it is considered to be the full content of the document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum TextDocumentContentChangeEvent {
    Partial(TextDocumentContentChangePartial),
    Whole(TextDocumentContentChangeWholeDocument),
}

/// MarkedString can be used to render human readable text. It is either a markdown string
/// or a code-block that provides a language and a code snippet. The language identifier
/// is semantically equal to the optional language identifier in fenced code blocks in GitHub
/// issues. See https://help.github.com/articles/creating-and-highlighting-code-blocks/#syntax-highlighting
///
/// The pair of a language and a value is an equivalent to markdown:
/// ```${language}
/// ${value}
/// ```
///
/// Note that markdown strings will be sanitized - that means html will be escaped.
/// @deprecated use MarkupContent instead.
#[deprecated]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum MarkedString {
    String(String),
    MarkedStringWithLanguage(MarkedStringWithLanguage),
}

/// A document filter describes a top level text document or
/// a notebook cell document.
///
/// @since 3.17.0 - proposed support for NotebookCellTextDocumentFilter.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum DocumentFilter {
    TextDocumentFilter(TextDocumentFilter),
    NotebookCell(NotebookCellTextDocumentFilter),
}

/// The glob pattern. Either a string pattern or a relative pattern.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum GlobPattern {
    Pattern(Pattern),
    Relative(RelativePattern),
}

/// A document filter denotes a document by different properties like
/// the [language][`TextDocument::languageId`], the [scheme][`Uri::scheme`] of
/// its resource, or a glob-pattern that is applied to the [path][`TextDocument::fileName`].
///
/// Glob patterns can have the following syntax:
/// - `*` to match one or more characters in a path segment
/// - `?` to match on one character in a path segment
/// - `**` to match any number of path segments, including none
/// - `{}` to group sub patterns into an OR expression. (e.g. `**​/*.{ts,js}` matches all TypeScript and JavaScript files)
/// - `[]` to declare a range of characters to match in a path segment (e.g., `example.[0-9]` to match on `example.0`, `example.1`, …)
/// - `[!...]` to negate a range of characters to match in a path segment (e.g., `example.[!0-9]` to match on `example.a`, `example.b`, but not `example.0`)
///
/// @sample A language filter that applies to typescript files on disk: `{ language: 'typescript', scheme: 'file' }`
/// @sample A language filter that applies to all package.json paths: `{ language: 'json', pattern: '**package.json' }`
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum TextDocumentFilter {
    Language(TextDocumentFilterLanguage),
    Scheme(TextDocumentFilterScheme),
    Pattern(TextDocumentFilterPattern),
}

/// The glob pattern to watch relative to the base path. Glob patterns can have the following syntax:
/// - `*` to match one or more characters in a path segment
/// - `?` to match on one character in a path segment
/// - `**` to match any number of path segments, including none
/// - `{}` to group conditions (e.g. `**​/*.{ts,js}` matches all TypeScript and JavaScript files)
/// - `[]` to declare a range of characters to match in a path segment (e.g., `example.[0-9]` to match on `example.0`, `example.1`, …)
/// - `[!...]` to negate a range of characters to match in a path segment (e.g., `example.[!0-9]` to match on `example.a`, `example.b`, but not `example.0`)
///
/// @since 3.17.0
pub type Pattern = String;

/// A notebook document filter denotes a notebook document by
/// different properties. The properties will be match
/// against the notebook's URI (same as with documents)
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum NotebookDocumentFilter {
    Type(NotebookDocumentFilterNotebookType),
    Scheme(NotebookDocumentFilterScheme),
    Pattern(NotebookDocumentFilterPattern),
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ImplementationParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Represents a location inside a resource, such as a line
/// inside a text file.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Location {
    pub range: Range,

    pub uri: String,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ImplementationRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeDefinitionParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeDefinitionRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

/// A workspace folder inside a client.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceFolder {
    /// The name of the workspace folder. Used to refer to this
    /// workspace folder in the user interface.
    pub name: String,

    /// The associated URI for this workspace folder.
    pub uri: String,
}

/// The parameters of a `workspace/didChangeWorkspaceFolders` notification.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidChangeWorkspaceFoldersParams {
    /// The actual workspace folder change event.
    pub event: WorkspaceFoldersChangeEvent,
}

/// The parameters of a configuration request.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ConfigurationParams {
    pub items: Vec<ConfigurationItem>,
}

/// Parameters for a [DocumentColorRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentColorParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Represents a color range from a document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ColorInformation {
    /// The actual color value for this color range.
    pub color: Color,

    /// The range in the document where this color appears.
    pub range: Range,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentColorRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

/// Parameters for a [ColorPresentationRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ColorPresentationParams {
    /// The color to request presentations for.
    pub color: Color,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The range where the color would be inserted. Serves as a context.
    pub range: Range,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ColorPresentation {
    /// An optional array of additional [text edits][TextEdit] that are applied when
    /// selecting this color presentation. Edits must not overlap with the main [edit][`ColorPresentation::textEdit`] nor with themselves.
    pub additional_text_edits: Option<Vec<TextEdit>>,

    /// The label of this color presentation. It will be shown on the color
    /// picker header. By default this is also the text that is inserted when selecting
    /// this color presentation.
    pub label: String,

    /// An [edit][TextEdit] which is applied to a document when selecting
    /// this presentation for the color.  When `falsy` the [label][`ColorPresentation::label`]
    /// is used.
    pub text_edit: Option<TextEdit>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkDoneProgressOptions {
    pub work_done_progress: Option<bool>,
}

/// General text document registration options.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,
}

/// Parameters for a [FoldingRangeRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FoldingRangeParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Represents a folding range. To be valid, start and end line must be bigger than zero and smaller
/// than the number of lines in the document. Clients are free to ignore invalid ranges.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FoldingRange {
    /// The text that the client should show when the specified range is
    /// collapsed. If not defined or not supported by the client, a default
    /// will be chosen by the client.
    ///
    /// @since 3.17.0
    pub collapsed_text: Option<String>,

    /// The zero-based character offset before the folded range ends. If not defined, defaults to the length of the end line.
    pub end_character: Option<u32>,

    /// The zero-based end line of the range to fold. The folded area ends with the line's last character.
    /// To be valid, the end must be zero or larger and smaller than the number of lines in the document.
    pub end_line: u32,

    /// Describes the kind of the folding range such as `comment' or 'region'. The kind
    /// is used to categorize folding ranges and used by commands like 'Fold all comments'.
    /// See [FoldingRangeKind] for an enumeration of standardized kinds.
    pub kind: Option<CustomStringEnum<FoldingRangeKind>>,

    /// The zero-based character offset from where the folded range starts. If not defined, defaults to the length of the start line.
    pub start_character: Option<u32>,

    /// The zero-based start line of the range to fold. The folded area starts after the line's last character.
    /// To be valid, the end must be zero or larger and smaller than the number of lines in the document.
    pub start_line: u32,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FoldingRangeRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DeclarationParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DeclarationRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

/// A parameter literal used in selection range requests.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SelectionRangeParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The positions inside the text document.
    pub positions: Vec<Position>,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SelectionRangeRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkDoneProgressCreateParams {
    /// The token to be used to report progress.
    pub token: ProgressToken,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkDoneProgressCancelParams {
    /// The token to be used to report progress.
    pub token: ProgressToken,
}

/// The parameter of a `textDocument/prepareCallHierarchy` request.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyPrepareParams {
    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Represents programming constructs like functions or constructors in the context
/// of call hierarchy.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyItem {
    /// A data entry field that is preserved between a call hierarchy prepare and
    /// incoming calls or outgoing calls requests.
    pub data: Option<LSPAny>,

    /// More detail for this item, e.g. the signature of a function.
    pub detail: Option<String>,

    /// The kind of this item.
    pub kind: SymbolKind,

    /// The name of this item.
    pub name: String,

    /// The range enclosing this symbol not including leading/trailing whitespace but everything else, e.g. comments and code.
    pub range: Range,

    /// The range that should be selected and revealed when this symbol is being picked, e.g. the name of a function.
    /// Must be contained by the [`range`][`CallHierarchyItem::range`].
    pub selection_range: Range,

    /// Tags for this item.
    pub tags: Option<Vec<SymbolTag>>,

    /// The resource identifier of this item.
    pub uri: String,
}

/// Call hierarchy options used during static or dynamic registration.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

/// The parameter of a `callHierarchy/incomingCalls` request.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyIncomingCallsParams {
    pub item: CallHierarchyItem,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Represents an incoming call, e.g. a caller of a method or constructor.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyIncomingCall {
    /// The item that makes the call.
    pub from: CallHierarchyItem,

    /// The ranges at which the calls appear. This is relative to the caller
    /// denoted by [`this.from`][`CallHierarchyIncomingCall::from`].
    pub from_ranges: Vec<Range>,
}

/// The parameter of a `callHierarchy/outgoingCalls` request.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyOutgoingCallsParams {
    pub item: CallHierarchyItem,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Represents an outgoing call, e.g. calling a getter from a method or a method from a constructor etc.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyOutgoingCall {
    /// The range at which this item is called. This is the range relative to the caller, e.g the item
    /// passed to [`provideCallHierarchyOutgoingCalls`][`CallHierarchyItemProvider::provideCallHierarchyOutgoingCalls`]
    /// and not [`this.to`][`CallHierarchyOutgoingCall::to`].
    pub from_ranges: Vec<Range>,

    /// The item that is called.
    pub to: CallHierarchyItem,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokens {
    /// The actual tokens.
    pub data: Vec<u32>,

    /// An optional result id. If provided and clients support delta updating
    /// the client will include the result id in the next semantic token request.
    /// A server can then instead of computing all semantic tokens again simply
    /// send a delta.
    pub result_id: Option<String>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensPartialResult {
    pub data: Vec<u32>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// Server supports providing semantic tokens for a full document.
    pub full: Option<OR2<bool, SemanticTokensFullDelta>>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    /// The legend used by the server
    pub legend: SemanticTokensLegend,

    /// Server supports providing semantic tokens for a specific range
    /// of a document.
    pub range: Option<OR2<bool, LSPObject>>,

    pub work_done_progress: Option<bool>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensDeltaParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The result id of a previous response. The result Id can either point to a full response
    /// or a delta response depending on what was received last.
    pub previous_result_id: String,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensDelta {
    /// The semantic token edits to transform a previous result into a new result.
    pub edits: Vec<SemanticTokensEdit>,

    pub result_id: Option<String>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensDeltaPartialResult {
    pub edits: Vec<SemanticTokensEdit>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensRangeParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The range the semantic tokens are requested for.
    pub range: Range,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Params to show a resource in the UI.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ShowDocumentParams {
    /// Indicates to show the resource in an external program.
    /// To show, for example, `https://code.visualstudio.com/`
    /// in the default WEB browser set `external` to `true`.
    pub external: Option<bool>,

    /// An optional selection range if the document is a text
    /// document. Clients might ignore the property if an
    /// external program is started or the file is not a text
    /// file.
    pub selection: Option<Range>,

    /// An optional property to indicate whether the editor
    /// showing the document should take focus or not.
    /// Clients might ignore this property if an external
    /// program is started.
    pub take_focus: Option<bool>,

    /// The uri to show.
    pub uri: String,
}

/// The result of a showDocument request.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ShowDocumentResult {
    /// A boolean indicating if the show was successful.
    pub success: bool,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LinkedEditingRangeParams {
    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// The result of a linked editing range request.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LinkedEditingRanges {
    /// A list of ranges that can be edited together. The ranges must have
    /// identical length and contain identical text content. The ranges cannot overlap.
    pub ranges: Vec<Range>,

    /// An optional word pattern (regular expression) that describes valid contents for
    /// the given ranges. If no pattern is provided, the client configuration's word
    /// pattern will be used.
    pub word_pattern: Option<String>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LinkedEditingRangeRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

/// The parameters sent in notifications/requests for user-initiated creation of
/// files.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CreateFilesParams {
    /// An array of all files/folders created in this operation.
    pub files: Vec<FileCreate>,
}

/// A workspace edit represents changes to many resources managed in the workspace. The edit
/// should either provide `changes` or `documentChanges`. If documentChanges are present
/// they are preferred over `changes` if the client can handle versioned document edits.
///
/// Since version 3.13.0 a workspace edit can contain resource operations as well. If resource
/// operations are present clients need to execute the operations in the order in which they
/// are provided. So a workspace edit for example can consist of the following two changes:
/// (1) a create file a.txt and (2) a text document edit which insert text into file a.txt.
///
/// An invalid sequence (e.g. (1) delete file a.txt and (2) insert text into file a.txt) will
/// cause failure of the operation. How the client recovers from the failure is described by
/// the client capability: `workspace.workspaceEdit.failureHandling`
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceEdit {
    /// A map of change annotations that can be referenced in `AnnotatedTextEdit`s or create, rename and
    /// delete file / folder operations.
    ///
    /// Whether clients honor this property depends on the client capability `workspace.changeAnnotationSupport`.
    ///
    /// @since 3.16.0
    pub change_annotations: Option<HashMap<ChangeAnnotationIdentifier, ChangeAnnotation>>,

    /// Holds changes to existing resources.
    pub changes: Option<HashMap<String, Vec<TextEdit>>>,

    /// Depending on the client capability `workspace.workspaceEdit.resourceOperations` document changes
    /// are either an array of `TextDocumentEdit`s to express changes to n different text documents
    /// where each text document edit addresses a specific version of a text document. Or it can contain
    /// above `TextDocumentEdit`s mixed with create, rename and delete file / folder operations.
    ///
    /// Whether a client supports versioned document edits is expressed via
    /// `workspace.workspaceEdit.documentChanges` client capability.
    ///
    /// If a client neither supports `documentChanges` nor `workspace.workspaceEdit.resourceOperations` then
    /// only plain `TextEdit`s using the `changes` property are supported.
    pub document_changes: Option<Vec<OR4<TextDocumentEdit, CreateFile, RenameFile, DeleteFile>>>,
}

/// The options to register for file operations.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileOperationRegistrationOptions {
    /// The actual filters.
    pub filters: Vec<FileOperationFilter>,
}

/// The parameters sent in notifications/requests for user-initiated renames of
/// files.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RenameFilesParams {
    /// An array of all files/folders renamed in this operation. When a folder is renamed, only
    /// the folder will be included, and not its children.
    pub files: Vec<FileRename>,
}

/// The parameters sent in notifications/requests for user-initiated deletes of
/// files.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DeleteFilesParams {
    /// An array of all files/folders deleted in this operation.
    pub files: Vec<FileDelete>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct MonikerParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Moniker definition to match LSIF 0.5 moniker definition.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Moniker {
    /// The identifier of the moniker. The value is opaque in LSIF however
    /// schema owners are allowed to define the structure if they want.
    pub identifier: String,

    /// The moniker kind if known.
    pub kind: Option<MonikerKind>,

    /// The scheme of the moniker. For example tsc or .Net
    pub scheme: String,

    /// The scope in which the moniker is unique
    pub unique: UniquenessLevel,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct MonikerRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    pub work_done_progress: Option<bool>,
}

/// The parameter of a `textDocument/prepareTypeHierarchy` request.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchyPrepareParams {
    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchyItem {
    /// A data entry field that is preserved between a type hierarchy prepare and
    /// supertypes or subtypes requests. It could also be used to identify the
    /// type hierarchy in the server, helping improve the performance on
    /// resolving supertypes and subtypes.
    pub data: Option<LSPAny>,

    /// More detail for this item, e.g. the signature of a function.
    pub detail: Option<String>,

    /// The kind of this item.
    pub kind: SymbolKind,

    /// The name of this item.
    pub name: String,

    /// The range enclosing this symbol not including leading/trailing whitespace
    /// but everything else, e.g. comments and code.
    pub range: Range,

    /// The range that should be selected and revealed when this symbol is being
    /// picked, e.g. the name of a function. Must be contained by the
    /// [`range`][`TypeHierarchyItem::range`].
    pub selection_range: Range,

    /// Tags for this item.
    pub tags: Option<Vec<SymbolTag>>,

    /// The resource identifier of this item.
    pub uri: String,
}

/// Type hierarchy options used during static or dynamic registration.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchyRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

/// The parameter of a `typeHierarchy/supertypes` request.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchySupertypesParams {
    pub item: TypeHierarchyItem,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// The parameter of a `typeHierarchy/subtypes` request.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchySubtypesParams {
    pub item: TypeHierarchyItem,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// A parameter literal used in inline value requests.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineValueParams {
    /// Additional information about the context in which inline values were
    /// requested.
    pub context: InlineValueContext,

    /// The document range for which inline values should be computed.
    pub range: Range,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Inline value options used during static or dynamic registration.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineValueRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

/// A parameter literal used in inlay hint requests.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlayHintParams {
    /// The document range for which inlay hints should be computed.
    pub range: Range,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Inlay hint information.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlayHint {
    /// A data entry field that is preserved on an inlay hint between
    /// a `textDocument/inlayHint` and a `inlayHint/resolve` request.
    pub data: Option<LSPAny>,

    /// The kind of this hint. Can be omitted in which case the client
    /// should fall back to a reasonable default.
    pub kind: Option<InlayHintKind>,

    /// The label of this hint. A human readable string or an array of
    /// InlayHintLabelPart label parts.
    ///
    /// *Note* that neither the string nor the label part can be empty.
    pub label: OR2<String, Vec<InlayHintLabelPart>>,

    /// Render padding before the hint.
    ///
    /// Note: Padding should use the editor's background color, not the
    /// background color of the hint itself. That means padding can be used
    /// to visually align/separate an inlay hint.
    pub padding_left: Option<bool>,

    /// Render padding after the hint.
    ///
    /// Note: Padding should use the editor's background color, not the
    /// background color of the hint itself. That means padding can be used
    /// to visually align/separate an inlay hint.
    pub padding_right: Option<bool>,

    /// The position of this hint.
    pub position: Position,

    /// Optional text edits that are performed when accepting this inlay hint.
    ///
    /// *Note* that edits are expected to change the document so that the inlay
    /// hint (or its nearest variant) is now part of the document and the inlay
    /// hint itself is now obsolete.
    pub text_edits: Option<Vec<TextEdit>>,

    /// The tooltip text when you hover over this item.
    pub tooltip: Option<OR2<String, MarkupContent>>,
}

/// Inlay hint options used during static or dynamic registration.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlayHintRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    /// The server provides support to resolve additional
    /// information for an inlay hint item.
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// Parameters of the document diagnostic request.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentDiagnosticParams {
    /// The additional identifier  provided during registration.
    pub identifier: Option<String>,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The result id of a previous response if provided.
    pub previous_result_id: Option<String>,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// A partial result for a document diagnostic report.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentDiagnosticReportPartialResult {
    pub related_documents:
        HashMap<String, OR2<FullDocumentDiagnosticReport, UnchangedDocumentDiagnosticReport>>,
}

/// Cancellation data returned from a diagnostic request.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DiagnosticServerCancellationData {
    pub retrigger_request: bool,
}

/// Diagnostic registration options.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DiagnosticRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    /// An optional identifier under which the diagnostics are
    /// managed by the client.
    pub identifier: Option<String>,

    /// Whether the language has inter file dependencies meaning that
    /// editing code in one file can result in a different diagnostic
    /// set in another file. Inter file dependencies are common for
    /// most programming languages and typically uncommon for linters.
    pub inter_file_dependencies: bool,

    pub work_done_progress: Option<bool>,

    /// The server provides support for workspace diagnostics as well.
    pub workspace_diagnostics: bool,
}

/// Parameters of the workspace diagnostic request.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDiagnosticParams {
    /// The additional identifier provided during registration.
    pub identifier: Option<String>,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The currently known diagnostic reports with their
    /// previous result ids.
    pub previous_result_ids: Vec<PreviousResultId>,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// A workspace diagnostic report.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDiagnosticReport {
    pub items: Vec<WorkspaceDocumentDiagnosticReport>,
}

/// A partial result for a workspace diagnostic report.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDiagnosticReportPartialResult {
    pub items: Vec<WorkspaceDocumentDiagnosticReport>,
}

/// The params sent in an open notebook document notification.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidOpenNotebookDocumentParams {
    /// The text documents that represent the content
    /// of a notebook cell.
    pub cell_text_documents: Vec<TextDocumentItem>,

    /// The notebook document that got opened.
    pub notebook_document: NotebookDocument,
}

/// The params sent in a change notebook document notification.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidChangeNotebookDocumentParams {
    /// The actual changes to the notebook document.
    ///
    /// The changes describe single state changes to the notebook document.
    /// So if there are two changes c1 (at array index 0) and c2 (at array
    /// index 1) for a notebook in state S then c1 moves the notebook from
    /// S to S' and c2 from S' to S''. So c1 is computed on the state S and
    /// c2 is computed on the state S'.
    ///
    /// To mirror the content of a notebook using change events use the following approach:
    /// - start with the same initial content
    /// - apply the 'notebookDocument/didChange' notifications in the order you receive them.
    /// - apply the `NotebookChangeEvent`s in a single notification in the order
    ///   you receive them.
    pub change: NotebookDocumentChangeEvent,

    /// The notebook document that did change. The version number points
    /// to the version after all provided changes have been applied. If
    /// only the text document content of a cell changes the notebook version
    /// doesn't necessarily have to change.
    pub notebook_document: VersionedNotebookDocumentIdentifier,
}

/// The params sent in a save notebook document notification.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidSaveNotebookDocumentParams {
    /// The notebook document that got saved.
    pub notebook_document: NotebookDocumentIdentifier,
}

/// The params sent in a close notebook document notification.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidCloseNotebookDocumentParams {
    /// The text documents that represent the content
    /// of a notebook cell that got closed.
    pub cell_text_documents: Vec<TextDocumentIdentifier>,

    /// The notebook document that got closed.
    pub notebook_document: NotebookDocumentIdentifier,
}

/// A parameter literal used in inline completion requests.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineCompletionParams {
    /// Additional information about the context in which inline completions were
    /// requested.
    pub context: InlineCompletionContext,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Represents a collection of [inline completion items][InlineCompletionItem] to be presented in the editor.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineCompletionList {
    /// The inline completion items
    pub items: Vec<InlineCompletionItem>,
}

/// An inline completion item represents a text snippet that is proposed inline to complete text that is being typed.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineCompletionItem {
    /// An optional [Command] that is executed *after* inserting this completion.
    pub command: Option<Command>,

    /// A text that is used to decide if this inline completion should be shown. When `falsy` the [`InlineCompletionItem::insertText`] is used.
    pub filter_text: Option<String>,

    /// The text to replace the range with. Must be set.
    pub insert_text: OR2<String, StringValue>,

    /// The range to replace. Must begin and end on the same line.
    pub range: Option<Range>,
}

/// Inline completion options used during static or dynamic registration.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineCompletionRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    pub work_done_progress: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RegistrationParams {
    pub registrations: Vec<Registration>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct UnregistrationParams {
    pub unregisterations: Vec<Unregistration>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InitializeParams {
    /// The capabilities provided by the client (editor or tool)
    pub capabilities: ClientCapabilities,

    /// Information about the client
    ///
    /// @since 3.15.0
    pub client_info: Option<ClientInfo>,

    /// User provided initialization options.
    pub initialization_options: Option<LSPAny>,

    /// The locale the client is currently showing the user interface
    /// in. This must not necessarily be the locale of the operating
    /// system.
    ///
    /// Uses IETF language tags as the value's syntax
    /// (See https://en.wikipedia.org/wiki/IETF_language_tag)
    ///
    /// @since 3.16.0
    pub locale: Option<String>,

    /// The process Id of the parent process that started
    /// the server.
    ///
    /// Is `null` if the process has not been started by another process.
    /// If the parent process is not alive then the server should exit.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub process_id: Option<i32>,

    /// The rootPath of the workspace. Is null
    /// if no folder is open.
    ///
    /// @deprecated in favour of rootUri.
    #[deprecated]
    pub root_path: Option<String>,

    /// The rootUri of the workspace. Is null if no
    /// folder is open. If both `rootPath` and `rootUri` are set
    /// `rootUri` wins.
    ///
    /// @deprecated in favour of workspaceFolders.
    #[deprecated]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub root_uri: Option<String>,

    /// The initial trace setting. If omitted trace is disabled ('off').
    pub trace: Option<TraceValues>,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,

    /// The workspace folders configured in the client when the server starts.
    ///
    /// This property is only available if the client supports workspace folders.
    /// It can be `null` if the client supports workspace folders but none are
    /// configured.
    ///
    /// @since 3.6.0
    pub workspace_folders: Option<Vec<WorkspaceFolder>>,
}

/// The result returned from an initialize request.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InitializeResult {
    /// The capabilities the language server provides.
    pub capabilities: ServerCapabilities,

    /// Information about the server.
    ///
    /// @since 3.15.0
    pub server_info: Option<ServerInfo>,
}

/// The data type of the ResponseError if the
/// initialize request fails.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InitializeError {
    /// Indicates whether the client execute the following retry logic:
    /// (1) show the message provided by the ResponseError to the user
    /// (2) user selects retry or cancel
    /// (3) if user selected retry the initialize method is sent again.
    pub retry: bool,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InitializedParams {}

/// The parameters of a change configuration notification.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidChangeConfigurationParams {
    /// The actual changed settings
    pub settings: LSPAny,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidChangeConfigurationRegistrationOptions {
    pub section: Option<OR2<String, Vec<String>>>,
}

/// The parameters of a notification message.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ShowMessageParams {
    /// The actual message.
    pub message: String,

    /// The message type. See [MessageType]
    #[serde(rename = "type")]
    pub type_: MessageType,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ShowMessageRequestParams {
    /// The message action items to present.
    pub actions: Option<Vec<MessageActionItem>>,

    /// The actual message.
    pub message: String,

    /// The message type. See [MessageType]
    #[serde(rename = "type")]
    pub type_: MessageType,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct MessageActionItem {
    /// A short title like 'Retry', 'Open Log' etc.
    pub title: String,
}

/// The log message parameters.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LogMessageParams {
    /// The actual message.
    pub message: String,

    /// The message type. See [MessageType]
    #[serde(rename = "type")]
    pub type_: MessageType,
}

/// The parameters sent in an open text document notification
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidOpenTextDocumentParams {
    /// The document that was opened.
    pub text_document: TextDocumentItem,
}

/// The change text document notification's parameters.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidChangeTextDocumentParams {
    /// The actual content changes. The content changes describe single state changes
    /// to the document. So if there are two content changes c1 (at array index 0) and
    /// c2 (at array index 1) for a document in state S then c1 moves the document from
    /// S to S' and c2 from S' to S''. So c1 is computed on the state S and c2 is computed
    /// on the state S'.
    ///
    /// To mirror the content of a document using change events use the following approach:
    /// - start with the same initial content
    /// - apply the 'textDocument/didChange' notifications in the order you receive them.
    /// - apply the `TextDocumentContentChangeEvent`s in a single notification in the order
    ///   you receive them.
    pub content_changes: Vec<TextDocumentContentChangeEvent>,

    /// The document that did change. The version number points
    /// to the version after all provided content changes have
    /// been applied.
    pub text_document: VersionedTextDocumentIdentifier,
}

/// Describe options to be used when registered for text document change events.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentChangeRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// How documents are synced to the server.
    pub sync_kind: TextDocumentSyncKind,
}

/// The parameters sent in a close text document notification
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidCloseTextDocumentParams {
    /// The document that was closed.
    pub text_document: TextDocumentIdentifier,
}

/// The parameters sent in a save text document notification
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidSaveTextDocumentParams {
    /// Optional the content when saved. Depends on the includeText value
    /// when the save notification was requested.
    pub text: Option<String>,

    /// The document that was saved.
    pub text_document: TextDocumentIdentifier,
}

/// Save registration options.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSaveRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The client is supposed to include the content on save.
    pub include_text: Option<bool>,
}

/// The parameters sent in a will save text document notification.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WillSaveTextDocumentParams {
    /// The 'TextDocumentSaveReason'.
    pub reason: TextDocumentSaveReason,

    /// The document that will be saved.
    pub text_document: TextDocumentIdentifier,
}

/// A text edit applicable to a text document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextEdit {
    /// The string to be inserted. For delete operations use an
    /// empty string.
    pub new_text: String,

    /// The range of the text document to be manipulated. To insert
    /// text into a document create a range where start === end.
    pub range: Range,
}

/// The watched files change notification's parameters.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidChangeWatchedFilesParams {
    /// The actual file events.
    pub changes: Vec<FileEvent>,
}

/// Describe options to be used when registered for text document change events.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidChangeWatchedFilesRegistrationOptions {
    /// The watchers to register.
    pub watchers: Vec<FileSystemWatcher>,
}

/// The publish diagnostic notification's parameters.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct PublishDiagnosticsParams {
    /// An array of diagnostic information items.
    pub diagnostics: Vec<Diagnostic>,

    /// The URI for which diagnostic information is reported.
    pub uri: String,

    /// Optional the version number of the document the diagnostics are published for.
    ///
    /// @since 3.15.0
    pub version: Option<i32>,
}

/// Completion parameters
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionParams {
    /// The completion context. This is only available it the client specifies
    /// to send this using the client capability `textDocument.completion.contextSupport === true`
    pub context: Option<CompletionContext>,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// A completion item represents a text snippet that is
/// proposed to complete text that is being typed.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionItem {
    /// An optional array of additional [text edits][TextEdit] that are applied when
    /// selecting this completion. Edits must not overlap (including the same insert position)
    /// with the main [edit][`CompletionItem::textEdit`] nor with themselves.
    ///
    /// Additional text edits should be used to change text unrelated to the current cursor position
    /// (for example adding an import statement at the top of the file if the completion item will
    /// insert an unqualified type).
    pub additional_text_edits: Option<Vec<TextEdit>>,

    /// An optional [command][Command] that is executed *after* inserting this completion. *Note* that
    /// additional modifications to the current document should be described with the
    /// [additionalTextEdits][`CompletionItem::additionalTextEdits`]-property.
    pub command: Option<Command>,

    /// An optional set of characters that when pressed while this completion is active will accept it first and
    /// then type that character. *Note* that all commit characters should have `length=1` and that superfluous
    /// characters will be ignored.
    pub commit_characters: Option<Vec<String>>,

    /// A data entry field that is preserved on a completion item between a
    /// [CompletionRequest] and a [CompletionResolveRequest].
    pub data: Option<LSPAny>,

    /// Indicates if this item is deprecated.
    /// @deprecated Use `tags` instead.
    #[deprecated]
    pub deprecated: Option<bool>,

    /// A human-readable string with additional information
    /// about this item, like type or symbol information.
    pub detail: Option<String>,

    /// A human-readable string that represents a doc-comment.
    pub documentation: Option<OR2<String, MarkupContent>>,

    /// A string that should be used when filtering a set of
    /// completion items. When `falsy` the [label][`CompletionItem::label`]
    /// is used.
    pub filter_text: Option<String>,

    /// A string that should be inserted into a document when selecting
    /// this completion. When `falsy` the [label][`CompletionItem::label`]
    /// is used.
    ///
    /// The `insertText` is subject to interpretation by the client side.
    /// Some tools might not take the string literally. For example
    /// VS Code when code complete is requested in this example
    /// `con<cursor position>` and a completion item with an `insertText` of
    /// `console` is provided it will only insert `sole`. Therefore it is
    /// recommended to use `textEdit` instead since it avoids additional client
    /// side interpretation.
    pub insert_text: Option<String>,

    /// The format of the insert text. The format applies to both the
    /// `insertText` property and the `newText` property of a provided
    /// `textEdit`. If omitted defaults to `InsertTextFormat.PlainText`.
    ///
    /// Please note that the insertTextFormat doesn't apply to
    /// `additionalTextEdits`.
    pub insert_text_format: Option<InsertTextFormat>,

    /// How whitespace and indentation is handled during completion
    /// item insertion. If not provided the clients default value depends on
    /// the `textDocument.completion.insertTextMode` client capability.
    ///
    /// @since 3.16.0
    pub insert_text_mode: Option<InsertTextMode>,

    /// The kind of this completion item. Based of the kind
    /// an icon is chosen by the editor.
    pub kind: Option<CompletionItemKind>,

    /// The label of this completion item.
    ///
    /// The label property is also by default the text that
    /// is inserted when selecting this completion.
    ///
    /// If label details are provided the label itself should
    /// be an unqualified name of the completion item.
    pub label: String,

    /// Additional details for the label
    ///
    /// @since 3.17.0
    pub label_details: Option<CompletionItemLabelDetails>,

    /// Select this item when showing.
    ///
    /// *Note* that only one completion item can be selected and that the
    /// tool / client decides which item that is. The rule is that the *first*
    /// item of those that match best is selected.
    pub preselect: Option<bool>,

    /// A string that should be used when comparing this item
    /// with other items. When `falsy` the [label][`CompletionItem::label`]
    /// is used.
    pub sort_text: Option<String>,

    /// Tags for this completion item.
    ///
    /// @since 3.15.0
    pub tags: Option<Vec<CompletionItemTag>>,

    /// An [edit][TextEdit] which is applied to a document when selecting
    /// this completion. When an edit is provided the value of
    /// [insertText][`CompletionItem::insertText`] is ignored.
    ///
    /// Most editors support two different operations when accepting a completion
    /// item. One is to insert a completion text and the other is to replace an
    /// existing text with a completion text. Since this can usually not be
    /// predetermined by a server it can report both ranges. Clients need to
    /// signal support for `InsertReplaceEdits` via the
    /// `textDocument.completion.insertReplaceSupport` client capability
    /// property.
    ///
    /// *Note 1:* The text edit's range as well as both ranges from an insert
    /// replace edit must be a [single line] and they must contain the position
    /// at which completion has been requested.
    /// *Note 2:* If an `InsertReplaceEdit` is returned the edit's insert range
    /// must be a prefix of the edit's replace range, that means it must be
    /// contained and starting at the same position.
    ///
    /// @since 3.16.0 additional type `InsertReplaceEdit`
    pub text_edit: Option<OR2<TextEdit, InsertReplaceEdit>>,

    /// The edit text used if the completion item is part of a CompletionList and
    /// CompletionList defines an item default for the text edit range.
    ///
    /// Clients will only honor this property if they opt into completion list
    /// item defaults using the capability `completionList.itemDefaults`.
    ///
    /// If not provided and a list's default range is provided the label
    /// property is used as a text.
    ///
    /// @since 3.17.0
    pub text_edit_text: Option<String>,
}

/// Represents a collection of [completion items][CompletionItem] to be presented
/// in the editor.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionList {
    /// This list it not complete. Further typing results in recomputing this list.
    ///
    /// Recomputed lists have all their items replaced (not appended) in the
    /// incomplete completion sessions.
    pub is_incomplete: bool,

    /// In many cases the items of an actual completion result share the same
    /// value for properties like `commitCharacters` or the range of a text
    /// edit. A completion list can therefore define item defaults which will
    /// be used if a completion item itself doesn't specify the value.
    ///
    /// If a completion list specifies a default value and a completion item
    /// also specifies a corresponding value the one from the item is used.
    ///
    /// Servers are only allowed to return default values if the client
    /// signals support for this via the `completionList.itemDefaults`
    /// capability.
    ///
    /// @since 3.17.0
    pub item_defaults: Option<CompletionItemDefaults>,

    /// The completion items.
    pub items: Vec<CompletionItem>,
}

/// Registration options for a [CompletionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionRegistrationOptions {
    /// The list of all possible characters that commit a completion. This field can be used
    /// if clients don't support individual commit characters per completion item. See
    /// `ClientCapabilities.textDocument.completion.completionItem.commitCharactersSupport`
    ///
    /// If a server provides both `allCommitCharacters` and commit characters on an individual
    /// completion item the ones on the completion item win.
    ///
    /// @since 3.2.0
    pub all_commit_characters: Option<Vec<String>>,

    /// The server supports the following `CompletionItem` specific
    /// capabilities.
    ///
    /// @since 3.17.0
    pub completion_item: Option<ServerCompletionItemOptions>,

    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The server provides support to resolve additional
    /// information for a completion item.
    pub resolve_provider: Option<bool>,

    /// Most tools trigger completion request automatically without explicitly requesting
    /// it using a keyboard shortcut (e.g. Ctrl+Space). Typically they do so when the user
    /// starts to type an identifier. For example if the user types `c` in a JavaScript file
    /// code complete will automatically pop up present `console` besides others as a
    /// completion item. Characters that make up identifiers don't need to be listed here.
    ///
    /// If code complete should automatically be trigger on characters not being valid inside
    /// an identifier (for example `.` in JavaScript) list them in `triggerCharacters`.
    pub trigger_characters: Option<Vec<String>>,

    pub work_done_progress: Option<bool>,
}

/// Parameters for a [HoverRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct HoverParams {
    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// The result of a hover request.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Hover {
    /// The hover's content
    pub contents: OR3<MarkupContent, MarkedString, Vec<MarkedString>>,

    /// An optional range inside the text document that is used to
    /// visualize the hover, e.g. by changing the background color.
    pub range: Option<Range>,
}

/// Registration options for a [HoverRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct HoverRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    pub work_done_progress: Option<bool>,
}

/// Parameters for a [SignatureHelpRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SignatureHelpParams {
    /// The signature help context. This is only available if the client specifies
    /// to send this using the client capability `textDocument.signatureHelp.contextSupport === true`
    ///
    /// @since 3.15.0
    pub context: Option<SignatureHelpContext>,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Signature help represents the signature of something
/// callable. There can be multiple signature but only one
/// active and only one active parameter.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SignatureHelp {
    /// The active parameter of the active signature. If omitted or the value
    /// lies outside the range of `signatures[activeSignature].parameters`
    /// defaults to 0 if the active signature has parameters. If
    /// the active signature has no parameters it is ignored.
    /// In future version of the protocol this property might become
    /// mandatory to better express the active parameter if the
    /// active signature does have any.
    pub active_parameter: Option<u32>,

    /// The active signature. If omitted or the value lies outside the
    /// range of `signatures` the value defaults to zero or is ignored if
    /// the `SignatureHelp` has no signatures.
    ///
    /// Whenever possible implementors should make an active decision about
    /// the active signature and shouldn't rely on a default value.
    ///
    /// In future version of the protocol this property might become
    /// mandatory to better express this.
    pub active_signature: Option<u32>,

    /// One or more signatures.
    pub signatures: Vec<SignatureInformation>,
}

/// Registration options for a [SignatureHelpRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SignatureHelpRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// List of characters that re-trigger signature help.
    ///
    /// These trigger characters are only active when signature help is already showing. All trigger characters
    /// are also counted as re-trigger characters.
    ///
    /// @since 3.15.0
    pub retrigger_characters: Option<Vec<String>>,

    /// List of characters that trigger signature help automatically.
    pub trigger_characters: Option<Vec<String>>,

    pub work_done_progress: Option<bool>,
}

/// Parameters for a [DefinitionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DefinitionParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Registration options for a [DefinitionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DefinitionRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    pub work_done_progress: Option<bool>,
}

/// Parameters for a [ReferencesRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ReferenceParams {
    pub context: ReferenceContext,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Registration options for a [ReferencesRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ReferenceRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    pub work_done_progress: Option<bool>,
}

/// Parameters for a [DocumentHighlightRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentHighlightParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// A document highlight is a range inside a text document which deserves
/// special attention. Usually a document highlight is visualized by changing
/// the background color of its range.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentHighlight {
    /// The highlight kind, default is [text][`DocumentHighlightKind::Text`].
    pub kind: Option<DocumentHighlightKind>,

    /// The range this highlight applies to.
    pub range: Range,
}

/// Registration options for a [DocumentHighlightRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentHighlightRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    pub work_done_progress: Option<bool>,
}

/// Parameters for a [DocumentSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentSymbolParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Represents information about programming constructs like variables, classes,
/// interfaces etc.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SymbolInformation {
    /// The name of the symbol containing this symbol. This information is for
    /// user interface purposes (e.g. to render a qualifier in the user interface
    /// if necessary). It can't be used to re-infer a hierarchy for the document
    /// symbols.
    pub container_name: Option<String>,

    /// Indicates if this symbol is deprecated.
    ///
    /// @deprecated Use tags instead
    #[deprecated]
    pub deprecated: Option<bool>,

    /// The kind of this symbol.
    pub kind: SymbolKind,

    /// The location of this symbol. The location's range is used by a tool
    /// to reveal the location in the editor. If the symbol is selected in the
    /// tool the range's start information is used to position the cursor. So
    /// the range usually spans more than the actual symbol's name and does
    /// normally include things like visibility modifiers.
    ///
    /// The range doesn't have to denote a node range in the sense of an abstract
    /// syntax tree. It can therefore not be used to re-construct a hierarchy of
    /// the symbols.
    pub location: Location,

    /// The name of this symbol.
    pub name: String,

    /// Tags for this symbol.
    ///
    /// @since 3.16.0
    pub tags: Option<Vec<SymbolTag>>,
}

/// Represents programming constructs like variables, classes, interfaces etc.
/// that appear in a document. Document symbols can be hierarchical and they
/// have two ranges: one that encloses its definition and one that points to
/// its most interesting range, e.g. the range of an identifier.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentSymbol {
    /// Children of this symbol, e.g. properties of a class.
    pub children: Option<Vec<DocumentSymbol>>,

    /// Indicates if this symbol is deprecated.
    ///
    /// @deprecated Use tags instead
    #[deprecated]
    pub deprecated: Option<bool>,

    /// More detail for this symbol, e.g the signature of a function.
    pub detail: Option<String>,

    /// The kind of this symbol.
    pub kind: SymbolKind,

    /// The name of this symbol. Will be displayed in the user interface and therefore must not be
    /// an empty string or a string only consisting of white spaces.
    pub name: String,

    /// The range enclosing this symbol not including leading/trailing whitespace but everything else
    /// like comments. This information is typically used to determine if the clients cursor is
    /// inside the symbol to reveal in the symbol in the UI.
    pub range: Range,

    /// The range that should be selected and revealed when this symbol is being picked, e.g the name of a function.
    /// Must be contained by the `range`.
    pub selection_range: Range,

    /// Tags for this document symbol.
    ///
    /// @since 3.16.0
    pub tags: Option<Vec<SymbolTag>>,
}

/// Registration options for a [DocumentSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentSymbolRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// A human-readable string that is shown when multiple outlines trees
    /// are shown for the same document.
    ///
    /// @since 3.16.0
    pub label: Option<String>,

    pub work_done_progress: Option<bool>,
}

/// The parameters of a [CodeActionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeActionParams {
    /// Context carrying additional information.
    pub context: CodeActionContext,

    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The range for which the command was invoked.
    pub range: Range,

    /// The document in which the command was invoked.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Represents a reference to a command. Provides a title which
/// will be used to represent a command in the UI and, optionally,
/// an array of arguments which will be passed to the command handler
/// function when invoked.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Command {
    /// Arguments that the command handler should be
    /// invoked with.
    pub arguments: Option<Vec<LSPAny>>,

    /// The identifier of the actual command handler.
    pub command: String,

    /// Title of the command, like `save`.
    pub title: String,
}

/// A code action represents a change that can be performed in code, e.g. to fix a problem or
/// to refactor code.
///
/// A CodeAction must set either `edit` and/or a `command`. If both are supplied, the `edit` is applied first, then the `command` is executed.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeAction {
    /// A command this code action executes. If a code action
    /// provides an edit and a command, first the edit is
    /// executed and then the command.
    pub command: Option<Command>,

    /// A data entry field that is preserved on a code action between
    /// a `textDocument/codeAction` and a `codeAction/resolve` request.
    ///
    /// @since 3.16.0
    pub data: Option<LSPAny>,

    /// The diagnostics that this code action resolves.
    pub diagnostics: Option<Vec<Diagnostic>>,

    /// Marks that the code action cannot currently be applied.
    ///
    /// Clients should follow the following guidelines regarding disabled code actions:
    ///
    ///   - Disabled code actions are not shown in automatic [lightbulbs](https://code.visualstudio.com/docs/editor/editingevolved#_code-action)
    ///     code action menus.
    ///
    ///   - Disabled actions are shown as faded out in the code action menu when the user requests a more specific type
    ///     of code action, such as refactorings.
    ///
    ///   - If the user has a [keybinding](https://code.visualstudio.com/docs/editor/refactoring#_keybindings-for-code-actions)
    ///     that auto applies a code action and only disabled code actions are returned, the client should show the user an
    ///     error message with `reason` in the editor.
    ///
    /// @since 3.16.0
    pub disabled: Option<CodeActionDisabled>,

    /// The workspace edit this code action performs.
    pub edit: Option<WorkspaceEdit>,

    /// Marks this as a preferred action. Preferred actions are used by the `auto fix` command and can be targeted
    /// by keybindings.
    ///
    /// A quick fix should be marked preferred if it properly addresses the underlying error.
    /// A refactoring should be marked preferred if it is the most reasonable choice of actions to take.
    ///
    /// @since 3.15.0
    pub is_preferred: Option<bool>,

    /// The kind of the code action.
    ///
    /// Used to filter code actions.
    pub kind: Option<CustomStringEnum<CodeActionKind>>,

    /// A short, human-readable, title for this code action.
    pub title: String,
}

/// Registration options for a [CodeActionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeActionRegistrationOptions {
    /// CodeActionKinds that this server may return.
    ///
    /// The list of kinds may be generic, such as `CodeActionKind.Refactor`, or the server
    /// may list out every specific kind they provide.
    pub code_action_kinds: Option<Vec<CustomStringEnum<CodeActionKind>>>,

    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// The server provides support to resolve additional
    /// information for a code action.
    ///
    /// @since 3.16.0
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// The parameters of a [WorkspaceSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSymbolParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// A query string to filter symbols by. Clients may send an empty
    /// string here to request all symbols.
    pub query: String,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// A special workspace symbol that supports locations without a range.
///
/// See also SymbolInformation.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSymbol {
    /// The name of the symbol containing this symbol. This information is for
    /// user interface purposes (e.g. to render a qualifier in the user interface
    /// if necessary). It can't be used to re-infer a hierarchy for the document
    /// symbols.
    pub container_name: Option<String>,

    /// A data entry field that is preserved on a workspace symbol between a
    /// workspace symbol request and a workspace symbol resolve request.
    pub data: Option<LSPAny>,

    /// The kind of this symbol.
    pub kind: SymbolKind,

    /// The location of the symbol. Whether a server is allowed to
    /// return a location without a range depends on the client
    /// capability `workspace.symbol.resolveSupport`.
    ///
    /// See SymbolInformation#location for more details.
    pub location: OR2<Location, LocationUriOnly>,

    /// The name of this symbol.
    pub name: String,

    /// Tags for this symbol.
    ///
    /// @since 3.16.0
    pub tags: Option<Vec<SymbolTag>>,
}

/// Registration options for a [WorkspaceSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSymbolRegistrationOptions {
    /// The server provides support to resolve additional
    /// information for a workspace symbol.
    ///
    /// @since 3.17.0
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// The parameters of a [CodeLensRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeLensParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The document to request code lens for.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// A code lens represents a [command][Command] that should be shown along with
/// source text, like the number of references, a way to run tests, etc.
///
/// A code lens is _unresolved_ when no command is associated to it. For performance
/// reasons the creation of a code lens and resolving should be done in two stages.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeLens {
    /// The command this code lens represents.
    pub command: Option<Command>,

    /// A data entry field that is preserved on a code lens item between
    /// a [CodeLensRequest] and a [CodeLensResolveRequest]
    pub data: Option<LSPAny>,

    /// The range in which this code lens is valid. Should only span a single line.
    pub range: Range,
}

/// Registration options for a [CodeLensRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeLensRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// Code lens has a resolve provider as well.
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// The parameters of a [DocumentLinkRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentLinkParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,

    /// The document to provide document links for.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// A document link is a range in a text document that links to an internal or external resource, like another
/// text document or a web site.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentLink {
    /// A data entry field that is preserved on a document link between a
    /// DocumentLinkRequest and a DocumentLinkResolveRequest.
    pub data: Option<LSPAny>,

    /// The range this link applies to.
    pub range: Range,

    /// The uri this link points to. If missing a resolve request is sent later.
    pub target: Option<String>,

    /// The tooltip text when you hover over this link.
    ///
    /// If a tooltip is provided, is will be displayed in a string that includes instructions on how to
    /// trigger the link, such as `{0} (ctrl + click)`. The specific instructions vary depending on OS,
    /// user settings, and localization.
    ///
    /// @since 3.15.0
    pub tooltip: Option<String>,
}

/// Registration options for a [DocumentLinkRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentLinkRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// Document links have a resolve provider as well.
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// The parameters of a [DocumentFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentFormattingParams {
    /// The format options.
    pub options: FormattingOptions,

    /// The document to format.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Registration options for a [DocumentFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentFormattingRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    pub work_done_progress: Option<bool>,
}

/// The parameters of a [DocumentRangeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentRangeFormattingParams {
    /// The format options
    pub options: FormattingOptions,

    /// The range to format
    pub range: Range,

    /// The document to format.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Registration options for a [DocumentRangeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentRangeFormattingRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// Whether the server supports formatting multiple ranges at once.
    ///
    /// @since 3.18.0
    /// @proposed
    #[cfg(feature = "proposed")]
    pub ranges_support: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// The parameters of a [DocumentRangesFormattingRequest].
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentRangesFormattingParams {
    /// The format options
    pub options: FormattingOptions,

    /// The ranges to format
    pub ranges: Vec<Range>,

    /// The document to format.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// The parameters of a [DocumentOnTypeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentOnTypeFormattingParams {
    /// The character that has been typed that triggered the formatting
    /// on type request. That is not necessarily the last character that
    /// got inserted into the document since the client could auto insert
    /// characters as well (e.g. like automatic brace completion).
    pub ch: String,

    /// The formatting options.
    pub options: FormattingOptions,

    /// The position around which the on type formatting should happen.
    /// This is not necessarily the exact position where the character denoted
    /// by the property `ch` got typed.
    pub position: Position,

    /// The document to format.
    pub text_document: TextDocumentIdentifier,
}

/// Registration options for a [DocumentOnTypeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentOnTypeFormattingRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// A character on which formatting should be triggered, like `{`.
    pub first_trigger_character: String,

    /// More trigger characters.
    pub more_trigger_character: Option<Vec<String>>,
}

/// The parameters of a [RenameRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RenameParams {
    /// The new name of the symbol. If the given name is not valid the
    /// request must return a [ResponseError] with an
    /// appropriate message set.
    pub new_name: String,

    /// The position at which this request was sent.
    pub position: Position,

    /// The document to rename.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Registration options for a [RenameRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RenameRegistrationOptions {
    /// A document selector to identify the scope of the registration. If set to null
    /// the document selector provided on the client side will be used.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub document_selector: Option<DocumentSelector>,

    /// Renames should be checked and tested before being executed.
    ///
    /// @since version 3.12.0
    pub prepare_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct PrepareRenameParams {
    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// The parameters of a [ExecuteCommandRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ExecuteCommandParams {
    /// Arguments that the command should be invoked with.
    pub arguments: Option<Vec<LSPAny>>,

    /// The identifier of the actual command handler.
    pub command: String,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

/// Registration options for a [ExecuteCommandRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ExecuteCommandRegistrationOptions {
    /// The commands to be executed on the server
    pub commands: Vec<String>,

    pub work_done_progress: Option<bool>,
}

/// The parameters passed via an apply workspace edit request.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ApplyWorkspaceEditParams {
    /// The edits to apply.
    pub edit: WorkspaceEdit,

    /// An optional label of the workspace edit. This label is
    /// presented in the user interface for example on an undo
    /// stack to undo the workspace edit.
    pub label: Option<String>,
}

/// The result returned from the apply workspace edit request.
///
/// @since 3.17 renamed from ApplyWorkspaceEditResponse
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ApplyWorkspaceEditResult {
    /// Indicates whether the edit was applied or not.
    pub applied: bool,

    /// Depending on the client's failure handling strategy `failedChange` might
    /// contain the index of the change that failed. This property is only available
    /// if the client signals a `failureHandlingStrategy` in its client capabilities.
    pub failed_change: Option<u32>,

    /// An optional textual description for why the edit was not applied.
    /// This may be used by the server for diagnostic logging or to provide
    /// a suitable error for a request that triggered the edit.
    pub failure_reason: Option<String>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkDoneProgressBegin {
    /// Controls if a cancel button should show to allow the user to cancel the
    /// long running operation. Clients that don't support cancellation are allowed
    /// to ignore the setting.
    pub cancellable: Option<bool>,

    pub kind: String,

    /// Optional, more detailed associated progress message. Contains
    /// complementary information to the `title`.
    ///
    /// Examples: "3/25 files", "project/src/module2", "node_modules/some_dep".
    /// If unset, the previous progress message (if any) is still valid.
    pub message: Option<String>,

    /// Optional progress percentage to display (value 100 is considered 100%).
    /// If not provided infinite progress is assumed and clients are allowed
    /// to ignore the `percentage` value in subsequent in report notifications.
    ///
    /// The value should be steadily rising. Clients are free to ignore values
    /// that are not following this rule. The value range is [0, 100].
    pub percentage: Option<u32>,

    /// Mandatory title of the progress operation. Used to briefly inform about
    /// the kind of operation being performed.
    ///
    /// Examples: "Indexing" or "Linking dependencies".
    pub title: String,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkDoneProgressReport {
    /// Controls enablement state of a cancel button.
    ///
    /// Clients that don't support cancellation or don't support controlling the button's
    /// enablement state are allowed to ignore the property.
    pub cancellable: Option<bool>,

    pub kind: String,

    /// Optional, more detailed associated progress message. Contains
    /// complementary information to the `title`.
    ///
    /// Examples: "3/25 files", "project/src/module2", "node_modules/some_dep".
    /// If unset, the previous progress message (if any) is still valid.
    pub message: Option<String>,

    /// Optional progress percentage to display (value 100 is considered 100%).
    /// If not provided infinite progress is assumed and clients are allowed
    /// to ignore the `percentage` value in subsequent in report notifications.
    ///
    /// The value should be steadily rising. Clients are free to ignore values
    /// that are not following this rule. The value range is [0, 100]
    pub percentage: Option<u32>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkDoneProgressEnd {
    pub kind: String,

    /// Optional, a final message indicating to for example indicate the outcome
    /// of the operation.
    pub message: Option<String>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SetTraceParams {
    pub value: TraceValues,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LogTraceParams {
    pub message: String,

    pub verbose: Option<String>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CancelParams {
    /// The request id to cancel.
    pub id: OR2<i32, String>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ProgressParams {
    /// The progress token provided by the client or server.
    pub token: ProgressToken,

    /// The progress data.
    pub value: LSPAny,
}

/// A parameter literal used in requests to pass a text document and a position inside that
/// document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentPositionParams {
    /// The position inside the text document.
    pub position: Position,

    /// The text document.
    pub text_document: TextDocumentIdentifier,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkDoneProgressParams {
    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct PartialResultParams {
    /// An optional token that a server can use to report partial results (e.g. streaming) to
    /// the client.
    pub partial_result_token: Option<ProgressToken>,
}

/// Represents the connection of two locations. Provides additional metadata over normal [locations][Location],
/// including an origin range.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LocationLink {
    /// Span of the origin of this link.
    ///
    /// Used as the underlined span for mouse interaction. Defaults to the word range at
    /// the definition position.
    pub origin_selection_range: Option<Range>,

    /// The full target range of this link. If the target for example is a symbol then target range is the
    /// range enclosing this symbol not including leading/trailing whitespace but everything else
    /// like comments. This information is typically used to highlight the range in the editor.
    pub target_range: Range,

    /// The range that should be selected and revealed when this link is being followed, e.g the name of a function.
    /// Must be contained by the `targetRange`. See also `DocumentSymbol#range`
    pub target_selection_range: Range,

    /// The target resource identifier of this link.
    pub target_uri: String,
}

/// A range in a text document expressed as (zero-based) start and end positions.
///
/// If you want to specify a range that contains a line including the line ending
/// character(s) then use an end position denoting the start of the next line.
/// For example:
/// ```ts
/// {
///     start: { line: 5, character: 23 }
///     end : { line 6, character : 0 }
/// }
/// ```
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Range {
    /// The range's end position.
    pub end: Position,

    /// The range's start position.
    pub start: Position,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ImplementationOptions {
    pub work_done_progress: Option<bool>,
}

/// Static registration options to be returned in the initialize
/// request.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct StaticRegistrationOptions {
    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeDefinitionOptions {
    pub work_done_progress: Option<bool>,
}

/// The workspace folder change event.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceFoldersChangeEvent {
    /// The array of added workspace folders
    pub added: Vec<WorkspaceFolder>,

    /// The array of the removed workspace folders
    pub removed: Vec<WorkspaceFolder>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ConfigurationItem {
    /// The scope to get the configuration section for.
    pub scope_uri: Option<String>,

    /// The configuration section asked for.
    pub section: Option<String>,
}

/// A literal to identify a text document in the client.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentIdentifier {
    /// The text document's uri.
    pub uri: String,
}

/// Represents a color in RGBA space.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Color {
    /// The alpha component of this color in the range [0-1].
    pub alpha: Decimal,

    /// The blue component of this color in the range [0-1].
    pub blue: Decimal,

    /// The green component of this color in the range [0-1].
    pub green: Decimal,

    /// The red component of this color in the range [0-1].
    pub red: Decimal,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentColorOptions {
    pub work_done_progress: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FoldingRangeOptions {
    pub work_done_progress: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DeclarationOptions {
    pub work_done_progress: Option<bool>,
}

/// Position in a text document expressed as zero-based line and character
/// offset. Prior to 3.17 the offsets were always based on a UTF-16 string
/// representation. So a string of the form `a𐐀b` the character offset of the
/// character `a` is 0, the character offset of `𐐀` is 1 and the character
/// offset of b is 3 since `𐐀` is represented using two code units in UTF-16.
/// Since 3.17 clients and servers can agree on a different string encoding
/// representation (e.g. UTF-8). The client announces it's supported encoding
/// via the client capability [`general.positionEncodings`](https://microsoft.github.io/language-server-protocol/specifications/specification-current/#clientCapabilities).
/// The value is an array of position encodings the client supports, with
/// decreasing preference (e.g. the encoding at index `0` is the most preferred
/// one). To stay backwards compatible the only mandatory encoding is UTF-16
/// represented via the string `utf-16`. The server can pick one of the
/// encodings offered by the client and signals that encoding back to the
/// client via the initialize result's property
/// [`capabilities.positionEncoding`](https://microsoft.github.io/language-server-protocol/specifications/specification-current/#serverCapabilities). If the string value
/// `utf-16` is missing from the client's capability `general.positionEncodings`
/// servers can safely assume that the client supports UTF-16. If the server
/// omits the position encoding in its initialize result the encoding defaults
/// to the string value `utf-16`. Implementation considerations: since the
/// conversion from one encoding into another requires the content of the
/// file / line the conversion is best done where the file is read which is
/// usually on the server side.
///
/// Positions are line end character agnostic. So you can not specify a position
/// that denotes `\r|\n` or `\n|` where `|` represents the character offset.
///
/// @since 3.17.0 - support for negotiated position encoding.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Position {
    /// Character offset on a line in a document (zero-based).
    ///
    /// The meaning of this offset is determined by the negotiated
    /// `PositionEncodingKind`.
    ///
    /// If the character value is greater than the line length it defaults back to the
    /// line length.
    pub character: u32,

    /// Line position in a document (zero-based).
    ///
    /// If a line number is greater than the number of lines in a document, it defaults back to the number of lines in the document.
    /// If a line number is negative, it defaults to 0.
    pub line: u32,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SelectionRangeOptions {
    pub work_done_progress: Option<bool>,
}

/// Call hierarchy options used during static registration.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyOptions {
    pub work_done_progress: Option<bool>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensOptions {
    /// Server supports providing semantic tokens for a full document.
    pub full: Option<OR2<bool, SemanticTokensFullDelta>>,

    /// The legend used by the server
    pub legend: SemanticTokensLegend,

    /// Server supports providing semantic tokens for a specific range
    /// of a document.
    pub range: Option<OR2<bool, LSPObject>>,

    pub work_done_progress: Option<bool>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensEdit {
    /// The elements to insert.
    pub data: Option<Vec<u32>>,

    /// The count of elements to remove.
    pub delete_count: u32,

    /// The start offset of the edit.
    pub start: u32,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LinkedEditingRangeOptions {
    pub work_done_progress: Option<bool>,
}

/// Represents information on a file/folder create.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileCreate {
    /// A file:// URI for the location of the file/folder being created.
    pub uri: String,
}

/// Describes textual changes on a text document. A TextDocumentEdit describes all changes
/// on a document version Si and after they are applied move the document to version Si+1.
/// So the creator of a TextDocumentEdit doesn't need to sort the array of edits or do any
/// kind of ordering. However the edits must be non overlapping.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentEdit {
    /// The edits to be applied.
    ///
    /// @since 3.16.0 - support for AnnotatedTextEdit. This is guarded using a
    /// client capability.
    pub edits: Vec<OR2<TextEdit, AnnotatedTextEdit>>,

    /// The text document to change.
    pub text_document: OptionalVersionedTextDocumentIdentifier,
}

/// Create file operation.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CreateFile {
    /// An optional annotation identifier describing the operation.
    ///
    /// @since 3.16.0
    pub annotation_id: Option<ChangeAnnotationIdentifier>,

    /// A create
    pub kind: String,

    /// Additional options
    pub options: Option<CreateFileOptions>,

    /// The resource to create.
    pub uri: String,
}

/// Rename file operation
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RenameFile {
    /// An optional annotation identifier describing the operation.
    ///
    /// @since 3.16.0
    pub annotation_id: Option<ChangeAnnotationIdentifier>,

    /// A rename
    pub kind: String,

    /// The new location.
    pub new_uri: String,

    /// The old (existing) location.
    pub old_uri: String,

    /// Rename options.
    pub options: Option<RenameFileOptions>,
}

/// Delete file operation
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DeleteFile {
    /// An optional annotation identifier describing the operation.
    ///
    /// @since 3.16.0
    pub annotation_id: Option<ChangeAnnotationIdentifier>,

    /// A delete
    pub kind: String,

    /// Delete options.
    pub options: Option<DeleteFileOptions>,

    /// The file to delete.
    pub uri: String,
}

/// Additional information that describes document changes.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ChangeAnnotation {
    /// A human-readable string which is rendered less prominent in
    /// the user interface.
    pub description: Option<String>,

    /// A human-readable string describing the actual change. The string
    /// is rendered prominent in the user interface.
    pub label: String,

    /// A flag which indicates that user confirmation is needed
    /// before applying the change.
    pub needs_confirmation: Option<bool>,
}

/// A filter to describe in which file operation requests or notifications
/// the server is interested in receiving.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileOperationFilter {
    /// The actual file operation pattern.
    pub pattern: FileOperationPattern,

    /// A Uri scheme like `file` or `untitled`.
    pub scheme: Option<String>,
}

/// Represents information on a file/folder rename.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileRename {
    /// A file:// URI for the new location of the file/folder being renamed.
    pub new_uri: String,

    /// A file:// URI for the original location of the file/folder being renamed.
    pub old_uri: String,
}

/// Represents information on a file/folder delete.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileDelete {
    /// A file:// URI for the location of the file/folder being deleted.
    pub uri: String,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct MonikerOptions {
    pub work_done_progress: Option<bool>,
}

/// Type hierarchy options used during static registration.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchyOptions {
    pub work_done_progress: Option<bool>,
}

/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineValueContext {
    /// The stack frame (as a DAP Id) where the execution has stopped.
    pub frame_id: i32,

    /// The document range where execution has stopped.
    /// Typically the end position of the range denotes the line where the inline values are shown.
    pub stopped_location: Range,
}

/// Provide inline value as text.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineValueText {
    /// The document range for which the inline value applies.
    pub range: Range,

    /// The text of the inline value.
    pub text: String,
}

/// Provide inline value through a variable lookup.
/// If only a range is specified, the variable name will be extracted from the underlying document.
/// An optional variable name can be used to override the extracted name.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineValueVariableLookup {
    /// How to perform the lookup.
    pub case_sensitive_lookup: bool,

    /// The document range for which the inline value applies.
    /// The range is used to extract the variable name from the underlying document.
    pub range: Range,

    /// If specified the name of the variable to look up.
    pub variable_name: Option<String>,
}

/// Provide an inline value through an expression evaluation.
/// If only a range is specified, the expression will be extracted from the underlying document.
/// An optional expression can be used to override the extracted expression.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineValueEvaluatableExpression {
    /// If specified the expression overrides the extracted expression.
    pub expression: Option<String>,

    /// The document range for which the inline value applies.
    /// The range is used to extract the evaluatable expression from the underlying document.
    pub range: Range,
}

/// Inline value options used during static registration.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineValueOptions {
    pub work_done_progress: Option<bool>,
}

/// An inlay hint label part allows for interactive and composite labels
/// of inlay hints.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlayHintLabelPart {
    /// An optional command for this label part.
    ///
    /// Depending on the client capability `inlayHint.resolveSupport` clients
    /// might resolve this property late using the resolve request.
    pub command: Option<Command>,

    /// An optional source code location that represents this
    /// label part.
    ///
    /// The editor will use this location for the hover and for code navigation
    /// features: This part will become a clickable link that resolves to the
    /// definition of the symbol at the given location (not necessarily the
    /// location itself), it shows the hover that shows at the given location,
    /// and it shows a context menu with further code navigation commands.
    ///
    /// Depending on the client capability `inlayHint.resolveSupport` clients
    /// might resolve this property late using the resolve request.
    pub location: Option<Location>,

    /// The tooltip text when you hover over this label part. Depending on
    /// the client capability `inlayHint.resolveSupport` clients might resolve
    /// this property late using the resolve request.
    pub tooltip: Option<OR2<String, MarkupContent>>,

    /// The value of this label part.
    pub value: String,
}

/// A `MarkupContent` literal represents a string value which content is interpreted base on its
/// kind flag. Currently the protocol supports `plaintext` and `markdown` as markup kinds.
///
/// If the kind is `markdown` then the value can contain fenced code blocks like in GitHub issues.
/// See https://help.github.com/articles/creating-and-highlighting-code-blocks/#syntax-highlighting
///
/// Here is an example how such a string can be constructed using JavaScript / TypeScript:
/// ```ts
/// let markdown: MarkdownContent = {
///  kind: MarkupKind.Markdown,
///  value: [
///    '# Header',
///    'Some text',
///    '```typescript',
///    'someCode();',
///    '```'
///  ].join('\n')
/// };
/// ```
///
/// *Please Note* that clients might sanitize the return markdown. A client could decide to
/// remove HTML from the markdown to avoid script execution.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct MarkupContent {
    /// The type of the Markup
    pub kind: MarkupKind,

    /// The content itself
    pub value: String,
}

/// Inlay hint options used during static registration.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlayHintOptions {
    /// The server provides support to resolve additional
    /// information for an inlay hint item.
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// A full diagnostic report with a set of related documents.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RelatedFullDocumentDiagnosticReport {
    /// The actual items.
    pub items: Vec<Diagnostic>,

    /// A full document diagnostic report.
    pub kind: String,

    /// Diagnostics of related documents. This information is useful
    /// in programming languages where code in a file A can generate
    /// diagnostics in a file B which A depends on. An example of
    /// such a language is C/C++ where marco definitions in a file
    /// a.cpp and result in errors in a header file b.hpp.
    ///
    /// @since 3.17.0
    pub related_documents: Option<
        HashMap<String, OR2<FullDocumentDiagnosticReport, UnchangedDocumentDiagnosticReport>>,
    >,

    /// An optional result id. If provided it will
    /// be sent on the next diagnostic request for the
    /// same document.
    pub result_id: Option<String>,
}

/// An unchanged diagnostic report with a set of related documents.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RelatedUnchangedDocumentDiagnosticReport {
    /// A document diagnostic report indicating
    /// no changes to the last result. A server can
    /// only return `unchanged` if result ids are
    /// provided.
    pub kind: String,

    /// Diagnostics of related documents. This information is useful
    /// in programming languages where code in a file A can generate
    /// diagnostics in a file B which A depends on. An example of
    /// such a language is C/C++ where marco definitions in a file
    /// a.cpp and result in errors in a header file b.hpp.
    ///
    /// @since 3.17.0
    pub related_documents: Option<
        HashMap<String, OR2<FullDocumentDiagnosticReport, UnchangedDocumentDiagnosticReport>>,
    >,

    /// A result id which will be sent on the next
    /// diagnostic request for the same document.
    pub result_id: String,
}

/// A diagnostic report with a full set of problems.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FullDocumentDiagnosticReport {
    /// The actual items.
    pub items: Vec<Diagnostic>,

    /// A full document diagnostic report.
    pub kind: String,

    /// An optional result id. If provided it will
    /// be sent on the next diagnostic request for the
    /// same document.
    pub result_id: Option<String>,
}

/// A diagnostic report indicating that the last returned
/// report is still accurate.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct UnchangedDocumentDiagnosticReport {
    /// A document diagnostic report indicating
    /// no changes to the last result. A server can
    /// only return `unchanged` if result ids are
    /// provided.
    pub kind: String,

    /// A result id which will be sent on the next
    /// diagnostic request for the same document.
    pub result_id: String,
}

/// Diagnostic options.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DiagnosticOptions {
    /// An optional identifier under which the diagnostics are
    /// managed by the client.
    pub identifier: Option<String>,

    /// Whether the language has inter file dependencies meaning that
    /// editing code in one file can result in a different diagnostic
    /// set in another file. Inter file dependencies are common for
    /// most programming languages and typically uncommon for linters.
    pub inter_file_dependencies: bool,

    pub work_done_progress: Option<bool>,

    /// The server provides support for workspace diagnostics as well.
    pub workspace_diagnostics: bool,
}

/// A previous result id in a workspace pull request.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct PreviousResultId {
    /// The URI for which the client knowns a
    /// result id.
    pub uri: String,

    /// The value of the previous result id.
    pub value: String,
}

/// A notebook document.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocument {
    /// The cells of a notebook.
    pub cells: Vec<NotebookCell>,

    /// Additional metadata stored with the notebook
    /// document.
    ///
    /// Note: should always be an object literal (e.g. LSPObject)
    pub metadata: Option<LSPObject>,

    /// The type of the notebook.
    pub notebook_type: String,

    /// The notebook document's uri.
    pub uri: String,

    /// The version number of this document (it will increase after each
    /// change, including undo/redo).
    pub version: i32,
}

/// An item to transfer a text document from the client to the
/// server.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentItem {
    /// The text document's language identifier.
    pub language_id: String,

    /// The content of the opened text document.
    pub text: String,

    /// The text document's uri.
    pub uri: String,

    /// The version number of this document (it will increase after each
    /// change, including undo/redo).
    pub version: i32,
}

/// A versioned notebook document identifier.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct VersionedNotebookDocumentIdentifier {
    /// The notebook document's uri.
    pub uri: String,

    /// The version number of this notebook document.
    pub version: i32,
}

/// A change event for a notebook document.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentChangeEvent {
    /// Changes to cells
    pub cells: Option<NotebookDocumentCellChanges>,

    /// The changed meta data if any.
    ///
    /// Note: should always be an object literal (e.g. LSPObject)
    pub metadata: Option<LSPObject>,
}

/// A literal to identify a notebook document in the client.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentIdentifier {
    /// The notebook document's uri.
    pub uri: String,
}

/// Provides information about the context in which an inline completion was requested.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineCompletionContext {
    /// Provides information about the currently selected item in the autocomplete widget if it is visible.
    pub selected_completion_info: Option<SelectedCompletionInfo>,

    /// Describes how the inline completion was triggered.
    pub trigger_kind: InlineCompletionTriggerKind,
}

/// A string value used as a snippet is a template which allows to insert text
/// and to control the editor cursor when insertion happens.
///
/// A snippet can define tab stops and placeholders with `$1`, `$2`
/// and `${3:foo}`. `$0` defines the final tab stop, it defaults to
/// the end of the snippet. Variables are defined with `$name` and
/// `${name:default value}`.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct StringValue {
    /// The kind of string value.
    pub kind: String,

    /// The snippet string.
    pub value: String,
}

/// Inline completion options used during static registration.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineCompletionOptions {
    pub work_done_progress: Option<bool>,
}

/// General parameters to register for a notification or to register a provider.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Registration {
    /// The id used to register the request. The id can be used to deregister
    /// the request again.
    pub id: String,

    /// The method / capability to register for.
    pub method: String,

    /// Options necessary for the registration.
    pub register_options: Option<LSPAny>,
}

/// General parameters to unregister a request or notification.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Unregistration {
    /// The id used to unregister the request or notification. Usually an id
    /// provided during the register request.
    pub id: String,

    /// The method to unregister for.
    pub method: String,
}

/// The initialize parameters
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct _InitializeParams {
    /// The capabilities provided by the client (editor or tool)
    pub capabilities: ClientCapabilities,

    /// Information about the client
    ///
    /// @since 3.15.0
    pub client_info: Option<ClientInfo>,

    /// User provided initialization options.
    pub initialization_options: Option<LSPAny>,

    /// The locale the client is currently showing the user interface
    /// in. This must not necessarily be the locale of the operating
    /// system.
    ///
    /// Uses IETF language tags as the value's syntax
    /// (See https://en.wikipedia.org/wiki/IETF_language_tag)
    ///
    /// @since 3.16.0
    pub locale: Option<String>,

    /// The process Id of the parent process that started
    /// the server.
    ///
    /// Is `null` if the process has not been started by another process.
    /// If the parent process is not alive then the server should exit.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub process_id: Option<i32>,

    /// The rootPath of the workspace. Is null
    /// if no folder is open.
    ///
    /// @deprecated in favour of rootUri.
    #[deprecated]
    pub root_path: Option<String>,

    /// The rootUri of the workspace. Is null if no
    /// folder is open. If both `rootPath` and `rootUri` are set
    /// `rootUri` wins.
    ///
    /// @deprecated in favour of workspaceFolders.
    #[deprecated]
    #[serde(skip_serializing_if = "Option::is_none")]
    pub root_uri: Option<String>,

    /// The initial trace setting. If omitted trace is disabled ('off').
    pub trace: Option<TraceValues>,

    /// An optional token that a server can use to report work done progress.
    pub work_done_token: Option<ProgressToken>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceFoldersInitializeParams {
    /// The workspace folders configured in the client when the server starts.
    ///
    /// This property is only available if the client supports workspace folders.
    /// It can be `null` if the client supports workspace folders but none are
    /// configured.
    ///
    /// @since 3.6.0
    pub workspace_folders: Option<Vec<WorkspaceFolder>>,
}

/// Defines the capabilities provided by a language
/// server.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ServerCapabilities {
    /// The server provides call hierarchy support.
    ///
    /// @since 3.16.0
    pub call_hierarchy_provider:
        Option<OR3<bool, CallHierarchyOptions, CallHierarchyRegistrationOptions>>,

    /// The server provides code actions. CodeActionOptions may only be
    /// specified if the client states that it supports
    /// `codeActionLiteralSupport` in its initial `initialize` request.
    pub code_action_provider: Option<OR2<bool, CodeActionOptions>>,

    /// The server provides code lens.
    pub code_lens_provider: Option<CodeLensOptions>,

    /// The server provides color provider support.
    pub color_provider: Option<OR3<bool, DocumentColorOptions, DocumentColorRegistrationOptions>>,

    /// The server provides completion support.
    pub completion_provider: Option<CompletionOptions>,

    /// The server provides Goto Declaration support.
    pub declaration_provider: Option<OR3<bool, DeclarationOptions, DeclarationRegistrationOptions>>,

    /// The server provides goto definition support.
    pub definition_provider: Option<OR2<bool, DefinitionOptions>>,

    /// The server has support for pull model diagnostics.
    ///
    /// @since 3.17.0
    pub diagnostic_provider: Option<OR2<DiagnosticOptions, DiagnosticRegistrationOptions>>,

    /// The server provides document formatting.
    pub document_formatting_provider: Option<OR2<bool, DocumentFormattingOptions>>,

    /// The server provides document highlight support.
    pub document_highlight_provider: Option<OR2<bool, DocumentHighlightOptions>>,

    /// The server provides document link support.
    pub document_link_provider: Option<DocumentLinkOptions>,

    /// The server provides document formatting on typing.
    pub document_on_type_formatting_provider: Option<DocumentOnTypeFormattingOptions>,

    /// The server provides document range formatting.
    pub document_range_formatting_provider: Option<OR2<bool, DocumentRangeFormattingOptions>>,

    /// The server provides document symbol support.
    pub document_symbol_provider: Option<OR2<bool, DocumentSymbolOptions>>,

    /// The server provides execute command support.
    pub execute_command_provider: Option<ExecuteCommandOptions>,

    /// Experimental server capabilities.
    pub experimental: Option<LSPAny>,

    /// The server provides folding provider support.
    pub folding_range_provider:
        Option<OR3<bool, FoldingRangeOptions, FoldingRangeRegistrationOptions>>,

    /// The server provides hover support.
    pub hover_provider: Option<OR2<bool, HoverOptions>>,

    /// The server provides Goto Implementation support.
    pub implementation_provider:
        Option<OR3<bool, ImplementationOptions, ImplementationRegistrationOptions>>,

    /// The server provides inlay hints.
    ///
    /// @since 3.17.0
    pub inlay_hint_provider: Option<OR3<bool, InlayHintOptions, InlayHintRegistrationOptions>>,

    /// Inline completion options used during static registration.
    ///
    /// @since 3.18.0
    /// @proposed
    #[cfg(feature = "proposed")]
    pub inline_completion_provider: Option<OR2<bool, InlineCompletionOptions>>,

    /// The server provides inline values.
    ///
    /// @since 3.17.0
    pub inline_value_provider:
        Option<OR3<bool, InlineValueOptions, InlineValueRegistrationOptions>>,

    /// The server provides linked editing range support.
    ///
    /// @since 3.16.0
    pub linked_editing_range_provider:
        Option<OR3<bool, LinkedEditingRangeOptions, LinkedEditingRangeRegistrationOptions>>,

    /// The server provides moniker support.
    ///
    /// @since 3.16.0
    pub moniker_provider: Option<OR3<bool, MonikerOptions, MonikerRegistrationOptions>>,

    /// Defines how notebook documents are synced.
    ///
    /// @since 3.17.0
    pub notebook_document_sync:
        Option<OR2<NotebookDocumentSyncOptions, NotebookDocumentSyncRegistrationOptions>>,

    /// The position encoding the server picked from the encodings offered
    /// by the client via the client capability `general.positionEncodings`.
    ///
    /// If the client didn't provide any position encodings the only valid
    /// value that a server can return is 'utf-16'.
    ///
    /// If omitted it defaults to 'utf-16'.
    ///
    /// @since 3.17.0
    pub position_encoding: Option<CustomStringEnum<PositionEncodingKind>>,

    /// The server provides find references support.
    pub references_provider: Option<OR2<bool, ReferenceOptions>>,

    /// The server provides rename support. RenameOptions may only be
    /// specified if the client states that it supports
    /// `prepareSupport` in its initial `initialize` request.
    pub rename_provider: Option<OR2<bool, RenameOptions>>,

    /// The server provides selection range support.
    pub selection_range_provider:
        Option<OR3<bool, SelectionRangeOptions, SelectionRangeRegistrationOptions>>,

    /// The server provides semantic tokens support.
    ///
    /// @since 3.16.0
    pub semantic_tokens_provider:
        Option<OR2<SemanticTokensOptions, SemanticTokensRegistrationOptions>>,

    /// The server provides signature help support.
    pub signature_help_provider: Option<SignatureHelpOptions>,

    /// Defines how text documents are synced. Is either a detailed structure
    /// defining each notification or for backwards compatibility the
    /// TextDocumentSyncKind number.
    pub text_document_sync: Option<OR2<TextDocumentSyncOptions, TextDocumentSyncKind>>,

    /// The server provides Goto Type Definition support.
    pub type_definition_provider:
        Option<OR3<bool, TypeDefinitionOptions, TypeDefinitionRegistrationOptions>>,

    /// The server provides type hierarchy support.
    ///
    /// @since 3.17.0
    pub type_hierarchy_provider:
        Option<OR3<bool, TypeHierarchyOptions, TypeHierarchyRegistrationOptions>>,

    /// Workspace specific server capabilities.
    pub workspace: Option<WorkspaceOptions>,

    /// The server provides workspace symbol support.
    pub workspace_symbol_provider: Option<OR2<bool, WorkspaceSymbolOptions>>,
}

/// Information about the server
///
/// @since 3.15.0
/// @since 3.18.0 ServerInfo type name added.
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ServerInfo {
    /// The name of the server as defined by the server.
    pub name: String,

    /// The server's version as defined by the server.
    pub version: Option<String>,
}

/// A text document identifier to denote a specific version of a text document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct VersionedTextDocumentIdentifier {
    /// The text document's uri.
    pub uri: String,

    /// The version number of this document.
    pub version: i32,
}

/// Save options.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SaveOptions {
    /// The client is supposed to include the content on save.
    pub include_text: Option<bool>,
}

/// An event describing a file change.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileEvent {
    /// The change type.
    #[serde(rename = "type")]
    pub type_: FileChangeType,

    /// The file's uri.
    pub uri: String,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileSystemWatcher {
    /// The glob pattern to watch. See [glob pattern][GlobPattern] for more detail.
    ///
    /// @since 3.17.0 support for relative patterns.
    pub glob_pattern: GlobPattern,

    /// The kind of events of interest. If omitted it defaults
    /// to WatchKind.Create | WatchKind.Change | WatchKind.Delete
    /// which is 7.
    pub kind: Option<CustomIntEnum<WatchKind>>,
}

/// Represents a diagnostic, such as a compiler error or warning. Diagnostic objects
/// are only valid in the scope of a resource.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct Diagnostic {
    /// The diagnostic's code, which usually appear in the user interface.
    pub code: Option<OR2<i32, String>>,

    /// An optional property to describe the error code.
    /// Requires the code field (above) to be present/not null.
    ///
    /// @since 3.16.0
    pub code_description: Option<CodeDescription>,

    /// A data entry field that is preserved between a `textDocument/publishDiagnostics`
    /// notification and `textDocument/codeAction` request.
    ///
    /// @since 3.16.0
    pub data: Option<LSPAny>,

    /// The diagnostic's message. It usually appears in the user interface
    pub message: String,

    /// The range at which the message applies
    pub range: Range,

    /// An array of related diagnostic information, e.g. when symbol-names within
    /// a scope collide all definitions can be marked via this property.
    pub related_information: Option<Vec<DiagnosticRelatedInformation>>,

    /// The diagnostic's severity. Can be omitted. If omitted it is up to the
    /// client to interpret diagnostics as error, warning, info or hint.
    pub severity: Option<DiagnosticSeverity>,

    /// A human-readable string describing the source of this
    /// diagnostic, e.g. 'typescript' or 'super lint'. It usually
    /// appears in the user interface.
    pub source: Option<String>,

    /// Additional metadata about the diagnostic.
    ///
    /// @since 3.15.0
    pub tags: Option<Vec<DiagnosticTag>>,
}

/// Contains additional information about the context in which a completion request is triggered.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionContext {
    /// The trigger character (a single character) that has trigger code complete.
    /// Is undefined if `triggerKind !== CompletionTriggerKind.TriggerCharacter`
    pub trigger_character: Option<String>,

    /// How the completion was triggered.
    pub trigger_kind: CompletionTriggerKind,
}

/// Additional details for a completion item label.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionItemLabelDetails {
    /// An optional string which is rendered less prominently after [`CompletionItem::detail`]. Should be used
    /// for fully qualified names and file paths.
    pub description: Option<String>,

    /// An optional string which is rendered less prominently directly after [label][`CompletionItem::label`],
    /// without any spacing. Should be used for function signatures and type annotations.
    pub detail: Option<String>,
}

/// A special text edit to provide an insert and a replace operation.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InsertReplaceEdit {
    /// The range if the insert is requested
    pub insert: Range,

    /// The string to be inserted.
    pub new_text: String,

    /// The range if the replace is requested.
    pub replace: Range,
}

/// In many cases the items of an actual completion result share the same
/// value for properties like `commitCharacters` or the range of a text
/// edit. A completion list can therefore define item defaults which will
/// be used if a completion item itself doesn't specify the value.
///
/// If a completion list specifies a default value and a completion item
/// also specifies a corresponding value the one from the item is used.
///
/// Servers are only allowed to return default values if the client
/// signals support for this via the `completionList.itemDefaults`
/// capability.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionItemDefaults {
    /// A default commit character set.
    ///
    /// @since 3.17.0
    pub commit_characters: Option<Vec<String>>,

    /// A default data value.
    ///
    /// @since 3.17.0
    pub data: Option<LSPAny>,

    /// A default edit range.
    ///
    /// @since 3.17.0
    pub edit_range: Option<OR2<Range, EditRangeWithInsertReplace>>,

    /// A default insert text format.
    ///
    /// @since 3.17.0
    pub insert_text_format: Option<InsertTextFormat>,

    /// A default insert text mode.
    ///
    /// @since 3.17.0
    pub insert_text_mode: Option<InsertTextMode>,
}

/// Completion options.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionOptions {
    /// The list of all possible characters that commit a completion. This field can be used
    /// if clients don't support individual commit characters per completion item. See
    /// `ClientCapabilities.textDocument.completion.completionItem.commitCharactersSupport`
    ///
    /// If a server provides both `allCommitCharacters` and commit characters on an individual
    /// completion item the ones on the completion item win.
    ///
    /// @since 3.2.0
    pub all_commit_characters: Option<Vec<String>>,

    /// The server supports the following `CompletionItem` specific
    /// capabilities.
    ///
    /// @since 3.17.0
    pub completion_item: Option<ServerCompletionItemOptions>,

    /// The server provides support to resolve additional
    /// information for a completion item.
    pub resolve_provider: Option<bool>,

    /// Most tools trigger completion request automatically without explicitly requesting
    /// it using a keyboard shortcut (e.g. Ctrl+Space). Typically they do so when the user
    /// starts to type an identifier. For example if the user types `c` in a JavaScript file
    /// code complete will automatically pop up present `console` besides others as a
    /// completion item. Characters that make up identifiers don't need to be listed here.
    ///
    /// If code complete should automatically be trigger on characters not being valid inside
    /// an identifier (for example `.` in JavaScript) list them in `triggerCharacters`.
    pub trigger_characters: Option<Vec<String>>,

    pub work_done_progress: Option<bool>,
}

/// Hover options.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct HoverOptions {
    pub work_done_progress: Option<bool>,
}

/// Additional information about the context in which a signature help request was triggered.
///
/// @since 3.15.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SignatureHelpContext {
    /// The currently active `SignatureHelp`.
    ///
    /// The `activeSignatureHelp` has its `SignatureHelp.activeSignature` field updated based on
    /// the user navigating through available signatures.
    pub active_signature_help: Option<SignatureHelp>,

    /// `true` if signature help was already showing when it was triggered.
    ///
    /// Retriggers occurs when the signature help is already active and can be caused by actions such as
    /// typing a trigger character, a cursor move, or document content changes.
    pub is_retrigger: bool,

    /// Character that caused signature help to be triggered.
    ///
    /// This is undefined when `triggerKind !== SignatureHelpTriggerKind.TriggerCharacter`
    pub trigger_character: Option<String>,

    /// Action that caused signature help to be triggered.
    pub trigger_kind: SignatureHelpTriggerKind,
}

/// Represents the signature of something callable. A signature
/// can have a label, like a function-name, a doc-comment, and
/// a set of parameters.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SignatureInformation {
    /// The index of the active parameter.
    ///
    /// If provided, this is used in place of `SignatureHelp.activeParameter`.
    ///
    /// @since 3.16.0
    pub active_parameter: Option<u32>,

    /// The human-readable doc-comment of this signature. Will be shown
    /// in the UI but can be omitted.
    pub documentation: Option<OR2<String, MarkupContent>>,

    /// The label of this signature. Will be shown in
    /// the UI.
    pub label: String,

    /// The parameters of this signature.
    pub parameters: Option<Vec<ParameterInformation>>,
}

/// Server Capabilities for a [SignatureHelpRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SignatureHelpOptions {
    /// List of characters that re-trigger signature help.
    ///
    /// These trigger characters are only active when signature help is already showing. All trigger characters
    /// are also counted as re-trigger characters.
    ///
    /// @since 3.15.0
    pub retrigger_characters: Option<Vec<String>>,

    /// List of characters that trigger signature help automatically.
    pub trigger_characters: Option<Vec<String>>,

    pub work_done_progress: Option<bool>,
}

/// Server Capabilities for a [DefinitionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DefinitionOptions {
    pub work_done_progress: Option<bool>,
}

/// Value-object that contains additional information when
/// requesting references.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ReferenceContext {
    /// Include the declaration of the current symbol.
    pub include_declaration: bool,
}

/// Reference options.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ReferenceOptions {
    pub work_done_progress: Option<bool>,
}

/// Provider options for a [DocumentHighlightRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentHighlightOptions {
    pub work_done_progress: Option<bool>,
}

/// A base for all symbol information.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct BaseSymbolInformation {
    /// The name of the symbol containing this symbol. This information is for
    /// user interface purposes (e.g. to render a qualifier in the user interface
    /// if necessary). It can't be used to re-infer a hierarchy for the document
    /// symbols.
    pub container_name: Option<String>,

    /// The kind of this symbol.
    pub kind: SymbolKind,

    /// The name of this symbol.
    pub name: String,

    /// Tags for this symbol.
    ///
    /// @since 3.16.0
    pub tags: Option<Vec<SymbolTag>>,
}

/// Provider options for a [DocumentSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentSymbolOptions {
    /// A human-readable string that is shown when multiple outlines trees
    /// are shown for the same document.
    ///
    /// @since 3.16.0
    pub label: Option<String>,

    pub work_done_progress: Option<bool>,
}

/// Contains additional diagnostic information about the context in which
/// a [code action][`CodeActionProvider::provideCodeActions`] is run.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeActionContext {
    /// An array of diagnostics known on the client side overlapping the range provided to the
    /// `textDocument/codeAction` request. They are provided so that the server knows which
    /// errors are currently presented to the user for the given range. There is no guarantee
    /// that these accurately reflect the error state of the resource. The primary parameter
    /// to compute code actions is the provided range.
    pub diagnostics: Vec<Diagnostic>,

    /// Requested kind of actions to return.
    ///
    /// Actions not of this kind are filtered out by the client before being shown. So servers
    /// can omit computing them.
    pub only: Option<Vec<CustomStringEnum<CodeActionKind>>>,

    /// The reason why code actions were requested.
    ///
    /// @since 3.17.0
    pub trigger_kind: Option<CodeActionTriggerKind>,
}

/// Captures why the code action is currently disabled.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeActionDisabled {
    /// Human readable description of why the code action is currently disabled.
    ///
    /// This is displayed in the code actions UI.
    pub reason: String,
}

/// Provider options for a [CodeActionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeActionOptions {
    /// CodeActionKinds that this server may return.
    ///
    /// The list of kinds may be generic, such as `CodeActionKind.Refactor`, or the server
    /// may list out every specific kind they provide.
    pub code_action_kinds: Option<Vec<CustomStringEnum<CodeActionKind>>>,

    /// The server provides support to resolve additional
    /// information for a code action.
    ///
    /// @since 3.16.0
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// Location with only uri and does not include range.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LocationUriOnly {
    pub uri: String,
}

/// Server capabilities for a [WorkspaceSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSymbolOptions {
    /// The server provides support to resolve additional
    /// information for a workspace symbol.
    ///
    /// @since 3.17.0
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// Code Lens provider options of a [CodeLensRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeLensOptions {
    /// Code lens has a resolve provider as well.
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// Provider options for a [DocumentLinkRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentLinkOptions {
    /// Document links have a resolve provider as well.
    pub resolve_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// Value-object describing what options formatting should use.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FormattingOptions {
    /// Insert a newline character at the end of the file if one does not exist.
    ///
    /// @since 3.15.0
    pub insert_final_newline: Option<bool>,

    /// Prefer spaces over tabs.
    pub insert_spaces: bool,

    /// Size of a tab in spaces.
    pub tab_size: u32,

    /// Trim all newlines after the final newline at the end of the file.
    ///
    /// @since 3.15.0
    pub trim_final_newlines: Option<bool>,

    /// Trim trailing whitespace on a line.
    ///
    /// @since 3.15.0
    pub trim_trailing_whitespace: Option<bool>,
}

/// Provider options for a [DocumentFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentFormattingOptions {
    pub work_done_progress: Option<bool>,
}

/// Provider options for a [DocumentRangeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentRangeFormattingOptions {
    /// Whether the server supports formatting multiple ranges at once.
    ///
    /// @since 3.18.0
    /// @proposed
    #[cfg(feature = "proposed")]
    pub ranges_support: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// Provider options for a [DocumentOnTypeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentOnTypeFormattingOptions {
    /// A character on which formatting should be triggered, like `{`.
    pub first_trigger_character: String,

    /// More trigger characters.
    pub more_trigger_character: Option<Vec<String>>,
}

/// Provider options for a [RenameRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RenameOptions {
    /// Renames should be checked and tested before being executed.
    ///
    /// @since version 3.12.0
    pub prepare_provider: Option<bool>,

    pub work_done_progress: Option<bool>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct PrepareRenamePlaceholder {
    pub placeholder: String,

    pub range: Range,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct PrepareRenameDefaultBehavior {
    pub default_behavior: bool,
}

/// The server capabilities of a [ExecuteCommandRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ExecuteCommandOptions {
    /// The commands to be executed on the server
    pub commands: Vec<String>,

    pub work_done_progress: Option<bool>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensLegend {
    /// The token modifiers a server uses.
    pub token_modifiers: Vec<String>,

    /// The token types a server uses.
    pub token_types: Vec<String>,
}

/// Semantic tokens options to support deltas for full documents
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensFullDelta {
    /// The server supports deltas for full documents.
    pub delta: Option<bool>,
}

/// A text document identifier to optionally denote a specific version of a text document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct OptionalVersionedTextDocumentIdentifier {
    /// The text document's uri.
    pub uri: String,

    /// The version number of this document. If a versioned text document identifier
    /// is sent from the server to the client and the file is not open in the editor
    /// (the server has not received an open notification before) the server can send
    /// `null` to indicate that the version is unknown and the content on disk is the
    /// truth (as specified with document content ownership).
    #[serde(skip_serializing_if = "Option::is_none")]
    pub version: Option<i32>,
}

/// A special text edit with an additional change annotation.
///
/// @since 3.16.0.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct AnnotatedTextEdit {
    /// The actual identifier of the change annotation
    pub annotation_id: ChangeAnnotationIdentifier,

    /// The string to be inserted. For delete operations use an
    /// empty string.
    pub new_text: String,

    /// The range of the text document to be manipulated. To insert
    /// text into a document create a range where start === end.
    pub range: Range,
}

/// A generic resource operation.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ResourceOperation {
    /// An optional annotation identifier describing the operation.
    ///
    /// @since 3.16.0
    pub annotation_id: Option<ChangeAnnotationIdentifier>,

    /// The resource operation kind.
    pub kind: String,
}

/// Options to create a file.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CreateFileOptions {
    /// Ignore if exists.
    pub ignore_if_exists: Option<bool>,

    /// Overwrite existing file. Overwrite wins over `ignoreIfExists`
    pub overwrite: Option<bool>,
}

/// Rename file options
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RenameFileOptions {
    /// Ignores if target exists.
    pub ignore_if_exists: Option<bool>,

    /// Overwrite target if existing. Overwrite wins over `ignoreIfExists`
    pub overwrite: Option<bool>,
}

/// Delete file options
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DeleteFileOptions {
    /// Ignore the operation if the file doesn't exist.
    pub ignore_if_not_exists: Option<bool>,

    /// Delete the content recursively if a folder is denoted.
    pub recursive: Option<bool>,
}

/// A pattern to describe in which file operation requests or notifications
/// the server is interested in receiving.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileOperationPattern {
    /// The glob pattern to match. Glob patterns can have the following syntax:
    /// - `*` to match one or more characters in a path segment
    /// - `?` to match on one character in a path segment
    /// - `**` to match any number of path segments, including none
    /// - `{}` to group sub patterns into an OR expression. (e.g. `**​/*.{ts,js}` matches all TypeScript and JavaScript files)
    /// - `[]` to declare a range of characters to match in a path segment (e.g., `example.[0-9]` to match on `example.0`, `example.1`, …)
    /// - `[!...]` to negate a range of characters to match in a path segment (e.g., `example.[!0-9]` to match on `example.a`, `example.b`, but not `example.0`)
    pub glob: String,

    /// Whether to match files or folders with this pattern.
    ///
    /// Matches both if undefined.
    pub matches: Option<FileOperationPatternKind>,

    /// Additional options used during matching.
    pub options: Option<FileOperationPatternOptions>,
}

/// A full document diagnostic report for a workspace diagnostic result.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceFullDocumentDiagnosticReport {
    /// The actual items.
    pub items: Vec<Diagnostic>,

    /// A full document diagnostic report.
    pub kind: String,

    /// An optional result id. If provided it will
    /// be sent on the next diagnostic request for the
    /// same document.
    pub result_id: Option<String>,

    /// The URI for which diagnostic information is reported.
    pub uri: String,

    /// The version number for which the diagnostics are reported.
    /// If the document is not marked as open `null` can be provided.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub version: Option<i32>,
}

/// An unchanged document diagnostic report for a workspace diagnostic result.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceUnchangedDocumentDiagnosticReport {
    /// A document diagnostic report indicating
    /// no changes to the last result. A server can
    /// only return `unchanged` if result ids are
    /// provided.
    pub kind: String,

    /// A result id which will be sent on the next
    /// diagnostic request for the same document.
    pub result_id: String,

    /// The URI for which diagnostic information is reported.
    pub uri: String,

    /// The version number for which the diagnostics are reported.
    /// If the document is not marked as open `null` can be provided.
    #[serde(skip_serializing_if = "Option::is_none")]
    pub version: Option<i32>,
}

/// A notebook cell.
///
/// A cell's document URI must be unique across ALL notebook
/// cells and can therefore be used to uniquely identify a
/// notebook cell or the cell's text document.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookCell {
    /// The URI of the cell's text document
    /// content.
    pub document: String,

    /// Additional execution summary information
    /// if supported by the client.
    pub execution_summary: Option<ExecutionSummary>,

    /// The cell's kind
    pub kind: NotebookCellKind,

    /// Additional metadata stored with the cell.
    ///
    /// Note: should always be an object literal (e.g. LSPObject)
    pub metadata: Option<LSPObject>,
}

/// Cell changes to a notebook document.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentCellChanges {
    /// Changes to notebook cells properties like its
    /// kind, execution summary or metadata.
    pub data: Option<Vec<NotebookCell>>,

    /// Changes to the cell structure to add or
    /// remove cells.
    pub structure: Option<NotebookDocumentCellChangeStructure>,

    /// Changes to the text content of notebook cells.
    pub text_content: Option<Vec<NotebookDocumentCellContentChanges>>,
}

/// Describes the currently selected completion item.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SelectedCompletionInfo {
    /// The range that will be replaced if this completion item is accepted.
    pub range: Range,

    /// The text the range will be replaced with if this completion is accepted.
    pub text: String,
}

/// Information about the client
///
/// @since 3.15.0
/// @since 3.18.0 ClientInfo type name added.
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientInfo {
    /// The name of the client as defined by the client.
    pub name: String,

    /// The client's version as defined by the client.
    pub version: Option<String>,
}

/// Defines the capabilities provided by the client.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientCapabilities {
    /// Experimental client capabilities.
    pub experimental: Option<LSPAny>,

    /// General client capabilities.
    ///
    /// @since 3.16.0
    pub general: Option<GeneralClientCapabilities>,

    /// Capabilities specific to the notebook document support.
    ///
    /// @since 3.17.0
    pub notebook_document: Option<NotebookDocumentClientCapabilities>,

    /// Text document specific client capabilities.
    pub text_document: Option<TextDocumentClientCapabilities>,

    /// Window specific client capabilities.
    pub window: Option<WindowClientCapabilities>,

    /// Workspace specific client capabilities.
    pub workspace: Option<WorkspaceClientCapabilities>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSyncOptions {
    /// Change notifications are sent to the server. See TextDocumentSyncKind.None, TextDocumentSyncKind.Full
    /// and TextDocumentSyncKind.Incremental. If omitted it defaults to TextDocumentSyncKind.None.
    pub change: Option<TextDocumentSyncKind>,

    /// Open and close notifications are sent to the server. If omitted open close notification should not
    /// be sent.
    pub open_close: Option<bool>,

    /// If present save notifications are sent to the server. If omitted the notification should not be
    /// sent.
    pub save: Option<OR2<bool, SaveOptions>>,

    /// If present will save notifications are sent to the server. If omitted the notification should not be
    /// sent.
    pub will_save: Option<bool>,

    /// If present will save wait until requests are sent to the server. If omitted the request should not be
    /// sent.
    pub will_save_wait_until: Option<bool>,
}

/// Options specific to a notebook plus its cells
/// to be synced to the server.
///
/// If a selector provides a notebook document
/// filter but no cell selector all cells of a
/// matching notebook document will be synced.
///
/// If a selector provides no notebook document
/// filter but only a cell selector all notebook
/// document that contain at least one matching
/// cell will be synced.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentSyncOptions {
    /// The notebooks to be synced
    pub notebook_selector:
        Vec<OR2<NotebookDocumentFilterWithNotebook, NotebookDocumentFilterWithCells>>,

    /// Whether save notification should be forwarded to
    /// the server. Will only be honored if mode === `notebook`.
    pub save: Option<bool>,
}

/// Registration options specific to a notebook.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentSyncRegistrationOptions {
    /// The id used to register the request. The id can be used to deregister
    /// the request again. See also Registration#id.
    pub id: Option<String>,

    /// The notebooks to be synced
    pub notebook_selector:
        Vec<OR2<NotebookDocumentFilterWithNotebook, NotebookDocumentFilterWithCells>>,

    /// Whether save notification should be forwarded to
    /// the server. Will only be honored if mode === `notebook`.
    pub save: Option<bool>,
}

/// Defines workspace specific capabilities of the server.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceOptions {
    /// The server is interested in notifications/requests for operations on files.
    ///
    /// @since 3.16.0
    pub file_operations: Option<FileOperationOptions>,

    /// The server supports workspace folder.
    ///
    /// @since 3.6.0
    pub workspace_folders: Option<WorkspaceFoldersServerCapabilities>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentContentChangePartial {
    /// The range of the document that changed.
    pub range: Range,

    /// The optional length of the range that got replaced.
    ///
    /// @deprecated use range instead.
    #[deprecated]
    pub range_length: Option<u32>,

    /// The new text for the provided range.
    pub text: String,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentContentChangeWholeDocument {
    /// The new text of the whole document.
    pub text: String,
}

/// Structure to capture a description for an error code.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeDescription {
    /// An URI to open with more information about the diagnostic error.
    pub href: String,
}

/// Represents a related message and source code location for a diagnostic. This should be
/// used to point to code locations that cause or related to a diagnostics, e.g when duplicating
/// a symbol in a scope.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DiagnosticRelatedInformation {
    /// The location of this related diagnostic information.
    pub location: Location,

    /// The message of this related diagnostic information.
    pub message: String,
}

/// Edit range variant that includes ranges for insert and replace operations.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct EditRangeWithInsertReplace {
    pub insert: Range,

    pub replace: Range,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ServerCompletionItemOptions {
    /// The server has support for completion item label
    /// details (see also `CompletionItemLabelDetails`) when
    /// receiving a completion item in a resolve call.
    ///
    /// @since 3.17.0
    pub label_details_support: Option<bool>,
}

/// @since 3.18.0
/// @proposed
/// @deprecated use MarkupContent instead.
#[deprecated]
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct MarkedStringWithLanguage {
    pub language: String,

    pub value: String,
}

/// Represents a parameter of a callable-signature. A parameter can
/// have a label and a doc-comment.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ParameterInformation {
    /// The human-readable doc-comment of this parameter. Will be shown
    /// in the UI but can be omitted.
    pub documentation: Option<OR2<String, MarkupContent>>,

    /// The label of this parameter information.
    ///
    /// Either a string or an inclusive start and exclusive end offsets within its containing
    /// signature label. (see SignatureInformation.label). The offsets are based on a UTF-16
    /// string representation as `Position` and `Range` does.
    ///
    /// *Note*: a label of type string should be a substring of its containing signature label.
    /// Its intended use case is to highlight the parameter label part in the `SignatureInformation.label`.
    pub label: OR2<String, (u32, u32)>,
}

/// A notebook cell text document filter denotes a cell text
/// document by different properties.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookCellTextDocumentFilter {
    /// A language id like `python`.
    ///
    /// Will be matched against the language id of the
    /// notebook cell document. '*' matches every language.
    pub language: Option<String>,

    /// A filter that matches against the notebook
    /// containing the notebook cell. If a string
    /// value is provided it matches against the
    /// notebook type. '*' matches every notebook.
    pub notebook: OR2<String, NotebookDocumentFilter>,
}

/// Matching options for the file operation pattern.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileOperationPatternOptions {
    /// The pattern should be matched ignoring casing.
    pub ignore_case: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ExecutionSummary {
    /// A strict monotonically increasing value
    /// indicating the execution order of a cell
    /// inside a notebook.
    pub execution_order: u32,

    /// Whether the execution was successful or
    /// not if known by the client.
    pub success: Option<bool>,
}

/// Structural changes to cells in a notebook document.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentCellChangeStructure {
    /// The change to the cell array.
    pub array: NotebookCellArrayChange,

    /// Additional closed cell text documents.
    pub did_close: Option<Vec<TextDocumentIdentifier>>,

    /// Additional opened cell text documents.
    pub did_open: Option<Vec<TextDocumentItem>>,
}

/// Content changes to a cell in a notebook document.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentCellContentChanges {
    pub changes: Vec<TextDocumentContentChangeEvent>,

    pub document: VersionedTextDocumentIdentifier,
}

/// Workspace specific client capabilities.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceClientCapabilities {
    /// The client supports applying batch edits
    /// to the workspace by supporting the request
    /// 'workspace/applyEdit'
    pub apply_edit: Option<bool>,

    /// Capabilities specific to the code lens requests scoped to the
    /// workspace.
    ///
    /// @since 3.16.0.
    pub code_lens: Option<CodeLensWorkspaceClientCapabilities>,

    /// The client supports `workspace/configuration` requests.
    ///
    /// @since 3.6.0
    pub configuration: Option<bool>,

    /// Capabilities specific to the diagnostic requests scoped to the
    /// workspace.
    ///
    /// @since 3.17.0.
    pub diagnostics: Option<DiagnosticWorkspaceClientCapabilities>,

    /// Capabilities specific to the `workspace/didChangeConfiguration` notification.
    pub did_change_configuration: Option<DidChangeConfigurationClientCapabilities>,

    /// Capabilities specific to the `workspace/didChangeWatchedFiles` notification.
    pub did_change_watched_files: Option<DidChangeWatchedFilesClientCapabilities>,

    /// Capabilities specific to the `workspace/executeCommand` request.
    pub execute_command: Option<ExecuteCommandClientCapabilities>,

    /// The client has support for file notifications/requests for user operations on files.
    ///
    /// Since 3.16.0
    pub file_operations: Option<FileOperationClientCapabilities>,

    /// Capabilities specific to the folding range requests scoped to the workspace.
    ///
    /// @since 3.18.0
    /// @proposed
    #[cfg(feature = "proposed")]
    pub folding_range: Option<FoldingRangeWorkspaceClientCapabilities>,

    /// Capabilities specific to the inlay hint requests scoped to the
    /// workspace.
    ///
    /// @since 3.17.0.
    pub inlay_hint: Option<InlayHintWorkspaceClientCapabilities>,

    /// Capabilities specific to the inline values requests scoped to the
    /// workspace.
    ///
    /// @since 3.17.0.
    pub inline_value: Option<InlineValueWorkspaceClientCapabilities>,

    /// Capabilities specific to the semantic token requests scoped to the
    /// workspace.
    ///
    /// @since 3.16.0.
    pub semantic_tokens: Option<SemanticTokensWorkspaceClientCapabilities>,

    /// Capabilities specific to the `workspace/symbol` request.
    pub symbol: Option<WorkspaceSymbolClientCapabilities>,

    /// Capabilities specific to `WorkspaceEdit`s.
    pub workspace_edit: Option<WorkspaceEditClientCapabilities>,

    /// The client has support for workspace folders.
    ///
    /// @since 3.6.0
    pub workspace_folders: Option<bool>,
}

/// Text document specific client capabilities.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentClientCapabilities {
    /// Capabilities specific to the various call hierarchy requests.
    ///
    /// @since 3.16.0
    pub call_hierarchy: Option<CallHierarchyClientCapabilities>,

    /// Capabilities specific to the `textDocument/codeAction` request.
    pub code_action: Option<CodeActionClientCapabilities>,

    /// Capabilities specific to the `textDocument/codeLens` request.
    pub code_lens: Option<CodeLensClientCapabilities>,

    /// Capabilities specific to the `textDocument/documentColor` and the
    /// `textDocument/colorPresentation` request.
    ///
    /// @since 3.6.0
    pub color_provider: Option<DocumentColorClientCapabilities>,

    /// Capabilities specific to the `textDocument/completion` request.
    pub completion: Option<CompletionClientCapabilities>,

    /// Capabilities specific to the `textDocument/declaration` request.
    ///
    /// @since 3.14.0
    pub declaration: Option<DeclarationClientCapabilities>,

    /// Capabilities specific to the `textDocument/definition` request.
    pub definition: Option<DefinitionClientCapabilities>,

    /// Capabilities specific to the diagnostic pull model.
    ///
    /// @since 3.17.0
    pub diagnostic: Option<DiagnosticClientCapabilities>,

    /// Capabilities specific to the `textDocument/documentHighlight` request.
    pub document_highlight: Option<DocumentHighlightClientCapabilities>,

    /// Capabilities specific to the `textDocument/documentLink` request.
    pub document_link: Option<DocumentLinkClientCapabilities>,

    /// Capabilities specific to the `textDocument/documentSymbol` request.
    pub document_symbol: Option<DocumentSymbolClientCapabilities>,

    /// Capabilities specific to the `textDocument/foldingRange` request.
    ///
    /// @since 3.10.0
    pub folding_range: Option<FoldingRangeClientCapabilities>,

    /// Capabilities specific to the `textDocument/formatting` request.
    pub formatting: Option<DocumentFormattingClientCapabilities>,

    /// Capabilities specific to the `textDocument/hover` request.
    pub hover: Option<HoverClientCapabilities>,

    /// Capabilities specific to the `textDocument/implementation` request.
    ///
    /// @since 3.6.0
    pub implementation: Option<ImplementationClientCapabilities>,

    /// Capabilities specific to the `textDocument/inlayHint` request.
    ///
    /// @since 3.17.0
    pub inlay_hint: Option<InlayHintClientCapabilities>,

    /// Client capabilities specific to inline completions.
    ///
    /// @since 3.18.0
    /// @proposed
    #[cfg(feature = "proposed")]
    pub inline_completion: Option<InlineCompletionClientCapabilities>,

    /// Capabilities specific to the `textDocument/inlineValue` request.
    ///
    /// @since 3.17.0
    pub inline_value: Option<InlineValueClientCapabilities>,

    /// Capabilities specific to the `textDocument/linkedEditingRange` request.
    ///
    /// @since 3.16.0
    pub linked_editing_range: Option<LinkedEditingRangeClientCapabilities>,

    /// Client capabilities specific to the `textDocument/moniker` request.
    ///
    /// @since 3.16.0
    pub moniker: Option<MonikerClientCapabilities>,

    /// Capabilities specific to the `textDocument/onTypeFormatting` request.
    pub on_type_formatting: Option<DocumentOnTypeFormattingClientCapabilities>,

    /// Capabilities specific to the `textDocument/publishDiagnostics` notification.
    pub publish_diagnostics: Option<PublishDiagnosticsClientCapabilities>,

    /// Capabilities specific to the `textDocument/rangeFormatting` request.
    pub range_formatting: Option<DocumentRangeFormattingClientCapabilities>,

    /// Capabilities specific to the `textDocument/references` request.
    pub references: Option<ReferenceClientCapabilities>,

    /// Capabilities specific to the `textDocument/rename` request.
    pub rename: Option<RenameClientCapabilities>,

    /// Capabilities specific to the `textDocument/selectionRange` request.
    ///
    /// @since 3.15.0
    pub selection_range: Option<SelectionRangeClientCapabilities>,

    /// Capabilities specific to the various semantic token request.
    ///
    /// @since 3.16.0
    pub semantic_tokens: Option<SemanticTokensClientCapabilities>,

    /// Capabilities specific to the `textDocument/signatureHelp` request.
    pub signature_help: Option<SignatureHelpClientCapabilities>,

    /// Defines which synchronization capabilities the client supports.
    pub synchronization: Option<TextDocumentSyncClientCapabilities>,

    /// Capabilities specific to the `textDocument/typeDefinition` request.
    ///
    /// @since 3.6.0
    pub type_definition: Option<TypeDefinitionClientCapabilities>,

    /// Capabilities specific to the various type hierarchy requests.
    ///
    /// @since 3.17.0
    pub type_hierarchy: Option<TypeHierarchyClientCapabilities>,
}

/// Capabilities specific to the notebook document support.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentClientCapabilities {
    /// Capabilities specific to notebook document synchronization
    ///
    /// @since 3.17.0
    pub synchronization: NotebookDocumentSyncClientCapabilities,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowClientCapabilities {
    /// Capabilities specific to the showDocument request.
    ///
    /// @since 3.16.0
    pub show_document: Option<ShowDocumentClientCapabilities>,

    /// Capabilities specific to the showMessage request.
    ///
    /// @since 3.16.0
    pub show_message: Option<ShowMessageRequestClientCapabilities>,

    /// It indicates whether the client supports server initiated
    /// progress using the `window/workDoneProgress/create` request.
    ///
    /// The capability also controls Whether client supports handling
    /// of progress notifications. If set servers are allowed to report a
    /// `workDoneProgress` property in the request specific server
    /// capabilities.
    ///
    /// @since 3.15.0
    pub work_done_progress: Option<bool>,
}

/// General client capabilities.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct GeneralClientCapabilities {
    /// Client capabilities specific to the client's markdown parser.
    ///
    /// @since 3.16.0
    pub markdown: Option<MarkdownClientCapabilities>,

    /// The position encodings supported by the client. Client and server
    /// have to agree on the same position encoding to ensure that offsets
    /// (e.g. character position in a line) are interpreted the same on both
    /// sides.
    ///
    /// To keep the protocol backwards compatible the following applies: if
    /// the value 'utf-16' is missing from the array of position encodings
    /// servers can assume that the client supports UTF-16. UTF-16 is
    /// therefore a mandatory encoding.
    ///
    /// If omitted it defaults to ['utf-16'].
    ///
    /// Implementation considerations: since the conversion from one encoding
    /// into another requires the content of the file / line the conversion
    /// is best done where the file is read which is usually on the server
    /// side.
    ///
    /// @since 3.17.0
    pub position_encodings: Option<Vec<CustomStringEnum<PositionEncodingKind>>>,

    /// Client capabilities specific to regular expressions.
    ///
    /// @since 3.16.0
    pub regular_expressions: Option<RegularExpressionsClientCapabilities>,

    /// Client capability that signals how the client
    /// handles stale requests (e.g. a request
    /// for which the client will not process the response
    /// anymore since the information is outdated).
    ///
    /// @since 3.17.0
    pub stale_request_support: Option<StaleRequestSupportOptions>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentFilterWithNotebook {
    /// The cells of the matching notebook to be synced.
    pub cells: Option<Vec<NotebookCellLanguage>>,

    /// The notebook to be synced If a string
    /// value is provided it matches against the
    /// notebook type. '*' matches every notebook.
    pub notebook: OR2<String, NotebookDocumentFilter>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentFilterWithCells {
    /// The cells of the matching notebook to be synced.
    pub cells: Vec<NotebookCellLanguage>,

    /// The notebook to be synced If a string
    /// value is provided it matches against the
    /// notebook type. '*' matches every notebook.
    pub notebook: Option<OR2<String, NotebookDocumentFilter>>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceFoldersServerCapabilities {
    /// Whether the server wants to receive workspace folder
    /// change notifications.
    ///
    /// If a string is provided the string is treated as an ID
    /// under which the notification is registered on the client
    /// side. The ID can be used to unregister for these events
    /// using the `client/unregisterCapability` request.
    pub change_notifications: Option<OR2<String, bool>>,

    /// The server has support for workspace folders
    pub supported: Option<bool>,
}

/// Options for notifications/requests for user operations on files.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileOperationOptions {
    /// The server is interested in receiving didCreateFiles notifications.
    pub did_create: Option<FileOperationRegistrationOptions>,

    /// The server is interested in receiving didDeleteFiles file notifications.
    pub did_delete: Option<FileOperationRegistrationOptions>,

    /// The server is interested in receiving didRenameFiles notifications.
    pub did_rename: Option<FileOperationRegistrationOptions>,

    /// The server is interested in receiving willCreateFiles requests.
    pub will_create: Option<FileOperationRegistrationOptions>,

    /// The server is interested in receiving willDeleteFiles file requests.
    pub will_delete: Option<FileOperationRegistrationOptions>,

    /// The server is interested in receiving willRenameFiles requests.
    pub will_rename: Option<FileOperationRegistrationOptions>,
}

/// A relative pattern is a helper to construct glob patterns that are matched
/// relatively to a base URI. The common value for a `baseUri` is a workspace
/// folder root, but it can be another absolute URI as well.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RelativePattern {
    /// A workspace folder or a base URI to which this pattern will be matched
    /// against relatively.
    pub base_uri: OR2<WorkspaceFolder, String>,

    /// The actual glob pattern;
    pub pattern: Pattern,
}

/// A document filter where `language` is required field.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentFilterLanguage {
    /// A language id, like `typescript`.
    pub language: String,

    /// A glob pattern, like **​/*.{ts,js}. See TextDocumentFilter for examples.
    pub pattern: Option<String>,

    /// A Uri [scheme][`Uri::scheme`], like `file` or `untitled`.
    pub scheme: Option<String>,
}

/// A document filter where `scheme` is required field.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentFilterScheme {
    /// A language id, like `typescript`.
    pub language: Option<String>,

    /// A glob pattern, like **​/*.{ts,js}. See TextDocumentFilter for examples.
    pub pattern: Option<String>,

    /// A Uri [scheme][`Uri::scheme`], like `file` or `untitled`.
    pub scheme: String,
}

/// A document filter where `pattern` is required field.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentFilterPattern {
    /// A language id, like `typescript`.
    pub language: Option<String>,

    /// A glob pattern, like **​/*.{ts,js}. See TextDocumentFilter for examples.
    pub pattern: String,

    /// A Uri [scheme][`Uri::scheme`], like `file` or `untitled`.
    pub scheme: Option<String>,
}

/// A change describing how to move a `NotebookCell`
/// array from state S to S'.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookCellArrayChange {
    /// The new cells, if any
    pub cells: Option<Vec<NotebookCell>>,

    /// The deleted cells
    pub delete_count: u32,

    /// The start oftest of the cell that changed.
    pub start: u32,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceEditClientCapabilities {
    /// Whether the client in general supports change annotations on text edits,
    /// create file, rename file and delete file changes.
    ///
    /// @since 3.16.0
    pub change_annotation_support: Option<ChangeAnnotationsSupportOptions>,

    /// The client supports versioned document changes in `WorkspaceEdit`s
    pub document_changes: Option<bool>,

    /// The failure handling strategy of a client if applying the workspace edit
    /// fails.
    ///
    /// @since 3.13.0
    pub failure_handling: Option<FailureHandlingKind>,

    /// Whether the client normalizes line endings to the client specific
    /// setting.
    /// If set to `true` the client will normalize line ending characters
    /// in a workspace edit to the client-specified new line
    /// character.
    ///
    /// @since 3.16.0
    pub normalizes_line_endings: Option<bool>,

    /// The resource operations the client supports. Clients should at least
    /// support 'create', 'rename' and 'delete' files and folders.
    ///
    /// @since 3.13.0
    pub resource_operations: Option<Vec<ResourceOperationKind>>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidChangeConfigurationClientCapabilities {
    /// Did change configuration notification supports dynamic registration.
    pub dynamic_registration: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DidChangeWatchedFilesClientCapabilities {
    /// Did change watched files notification supports dynamic registration. Please note
    /// that the current protocol doesn't support static configuration for file changes
    /// from the server side.
    pub dynamic_registration: Option<bool>,

    /// Whether the client has support for [relative pattern][RelativePattern]
    /// or not.
    ///
    /// @since 3.17.0
    pub relative_pattern_support: Option<bool>,
}

/// Client capabilities for a [WorkspaceSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSymbolClientCapabilities {
    /// Symbol request supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// The client support partial workspace symbols. The client will send the
    /// request `workspaceSymbol/resolve` to the server to resolve additional
    /// properties.
    ///
    /// @since 3.17.0
    pub resolve_support: Option<ClientSymbolResolveOptions>,

    /// Specific capabilities for the `SymbolKind` in the `workspace/symbol` request.
    pub symbol_kind: Option<ClientSymbolKindOptions>,

    /// The client supports tags on `SymbolInformation`.
    /// Clients supporting tags have to handle unknown tags gracefully.
    ///
    /// @since 3.16.0
    pub tag_support: Option<ClientSymbolTagOptions>,
}

/// The client capabilities of a [ExecuteCommandRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ExecuteCommandClientCapabilities {
    /// Execute command supports dynamic registration.
    pub dynamic_registration: Option<bool>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensWorkspaceClientCapabilities {
    /// Whether the client implementation supports a refresh request sent from
    /// the server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// semantic tokens currently shown. It should be used with absolute care
    /// and is useful for situation where a server for example detects a project
    /// wide change that requires such a calculation.
    pub refresh_support: Option<bool>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeLensWorkspaceClientCapabilities {
    /// Whether the client implementation supports a refresh request sent from the
    /// server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// code lenses currently shown. It should be used with absolute care and is
    /// useful for situation where a server for example detect a project wide
    /// change that requires such a calculation.
    pub refresh_support: Option<bool>,
}

/// Capabilities relating to events from file operations by the user in the client.
///
/// These events do not come from the file system, they come from user operations
/// like renaming a file in the UI.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FileOperationClientCapabilities {
    /// The client has support for sending didCreateFiles notifications.
    pub did_create: Option<bool>,

    /// The client has support for sending didDeleteFiles notifications.
    pub did_delete: Option<bool>,

    /// The client has support for sending didRenameFiles notifications.
    pub did_rename: Option<bool>,

    /// Whether the client supports dynamic registration for file requests/notifications.
    pub dynamic_registration: Option<bool>,

    /// The client has support for sending willCreateFiles requests.
    pub will_create: Option<bool>,

    /// The client has support for sending willDeleteFiles requests.
    pub will_delete: Option<bool>,

    /// The client has support for sending willRenameFiles requests.
    pub will_rename: Option<bool>,
}

/// Client workspace capabilities specific to inline values.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineValueWorkspaceClientCapabilities {
    /// Whether the client implementation supports a refresh request sent from the
    /// server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// inline values currently shown. It should be used with absolute care and is
    /// useful for situation where a server for example detects a project wide
    /// change that requires such a calculation.
    pub refresh_support: Option<bool>,
}

/// Client workspace capabilities specific to inlay hints.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlayHintWorkspaceClientCapabilities {
    /// Whether the client implementation supports a refresh request sent from
    /// the server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// inlay hints currently shown. It should be used with absolute care and
    /// is useful for situation where a server for example detects a project wide
    /// change that requires such a calculation.
    pub refresh_support: Option<bool>,
}

/// Workspace client capabilities specific to diagnostic pull requests.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DiagnosticWorkspaceClientCapabilities {
    /// Whether the client implementation supports a refresh request sent from
    /// the server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// pulled diagnostics currently shown. It should be used with absolute care and
    /// is useful for situation where a server for example detects a project wide
    /// change that requires such a calculation.
    pub refresh_support: Option<bool>,
}

/// Client workspace capabilities specific to folding ranges
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FoldingRangeWorkspaceClientCapabilities {
    /// Whether the client implementation supports a refresh request sent from the
    /// server to the client.
    ///
    /// Note that this event is global and will force the client to refresh all
    /// folding ranges currently shown. It should be used with absolute care and is
    /// useful for situation where a server for example detects a project wide
    /// change that requires such a calculation.
    ///
    /// @since 3.18.0
    /// @proposed
    #[cfg(feature = "proposed")]
    pub refresh_support: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSyncClientCapabilities {
    /// The client supports did save notifications.
    pub did_save: Option<bool>,

    /// Whether text document synchronization supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// The client supports sending will save notifications.
    pub will_save: Option<bool>,

    /// The client supports sending a will save request and
    /// waits for a response providing text edits which will
    /// be applied to the document before it is saved.
    pub will_save_wait_until: Option<bool>,
}

/// Completion client capabilities
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionClientCapabilities {
    /// The client supports the following `CompletionItem` specific
    /// capabilities.
    pub completion_item: Option<ClientCompletionItemOptions>,

    pub completion_item_kind: Option<ClientCompletionItemOptionsKind>,

    /// The client supports the following `CompletionList` specific
    /// capabilities.
    ///
    /// @since 3.17.0
    pub completion_list: Option<CompletionListCapabilities>,

    /// The client supports to send additional context information for a
    /// `textDocument/completion` request.
    pub context_support: Option<bool>,

    /// Whether completion supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// Defines how the client handles whitespace and indentation
    /// when accepting a completion item that uses multi line
    /// text in either `insertText` or `textEdit`.
    ///
    /// @since 3.17.0
    pub insert_text_mode: Option<InsertTextMode>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct HoverClientCapabilities {
    /// Client supports the following content formats for the content
    /// property. The order describes the preferred format of the client.
    pub content_format: Option<Vec<MarkupKind>>,

    /// Whether hover supports dynamic registration.
    pub dynamic_registration: Option<bool>,
}

/// Client Capabilities for a [SignatureHelpRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SignatureHelpClientCapabilities {
    /// The client supports to send additional context information for a
    /// `textDocument/signatureHelp` request. A client that opts into
    /// contextSupport will also support the `retriggerCharacters` on
    /// `SignatureHelpOptions`.
    ///
    /// @since 3.15.0
    pub context_support: Option<bool>,

    /// Whether signature help supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// The client supports the following `SignatureInformation`
    /// specific properties.
    pub signature_information: Option<ClientSignatureInformationOptions>,
}

/// @since 3.14.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DeclarationClientCapabilities {
    /// Whether declaration supports dynamic registration. If this is set to `true`
    /// the client supports the new `DeclarationRegistrationOptions` return value
    /// for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,

    /// The client supports additional metadata in the form of declaration links.
    pub link_support: Option<bool>,
}

/// Client Capabilities for a [DefinitionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DefinitionClientCapabilities {
    /// Whether definition supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// The client supports additional metadata in the form of definition links.
    ///
    /// @since 3.14.0
    pub link_support: Option<bool>,
}

/// Since 3.6.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeDefinitionClientCapabilities {
    /// Whether implementation supports dynamic registration. If this is set to `true`
    /// the client supports the new `TypeDefinitionRegistrationOptions` return value
    /// for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,

    /// The client supports additional metadata in the form of definition links.
    ///
    /// Since 3.14.0
    pub link_support: Option<bool>,
}

/// @since 3.6.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ImplementationClientCapabilities {
    /// Whether implementation supports dynamic registration. If this is set to `true`
    /// the client supports the new `ImplementationRegistrationOptions` return value
    /// for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,

    /// The client supports additional metadata in the form of definition links.
    ///
    /// @since 3.14.0
    pub link_support: Option<bool>,
}

/// Client Capabilities for a [ReferencesRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ReferenceClientCapabilities {
    /// Whether references supports dynamic registration.
    pub dynamic_registration: Option<bool>,
}

/// Client Capabilities for a [DocumentHighlightRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentHighlightClientCapabilities {
    /// Whether document highlight supports dynamic registration.
    pub dynamic_registration: Option<bool>,
}

/// Client Capabilities for a [DocumentSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentSymbolClientCapabilities {
    /// Whether document symbol supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// The client supports hierarchical document symbols.
    pub hierarchical_document_symbol_support: Option<bool>,

    /// The client supports an additional label presented in the UI when
    /// registering a document symbol provider.
    ///
    /// @since 3.16.0
    pub label_support: Option<bool>,

    /// Specific capabilities for the `SymbolKind` in the
    /// `textDocument/documentSymbol` request.
    pub symbol_kind: Option<ClientSymbolKindOptions>,

    /// The client supports tags on `SymbolInformation`. Tags are supported on
    /// `DocumentSymbol` if `hierarchicalDocumentSymbolSupport` is set to true.
    /// Clients supporting tags have to handle unknown tags gracefully.
    ///
    /// @since 3.16.0
    pub tag_support: Option<ClientSymbolTagOptions>,
}

/// The Client Capabilities of a [CodeActionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeActionClientCapabilities {
    /// The client support code action literals of type `CodeAction` as a valid
    /// response of the `textDocument/codeAction` request. If the property is not
    /// set the request can only return `Command` literals.
    ///
    /// @since 3.8.0
    pub code_action_literal_support: Option<ClientCodeActionLiteralOptions>,

    /// Whether code action supports the `data` property which is
    /// preserved between a `textDocument/codeAction` and a
    /// `codeAction/resolve` request.
    ///
    /// @since 3.16.0
    pub data_support: Option<bool>,

    /// Whether code action supports the `disabled` property.
    ///
    /// @since 3.16.0
    pub disabled_support: Option<bool>,

    /// Whether code action supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// Whether the client honors the change annotations in
    /// text edits and resource operations returned via the
    /// `CodeAction#edit` property by for example presenting
    /// the workspace edit in the user interface and asking
    /// for confirmation.
    ///
    /// @since 3.16.0
    pub honors_change_annotations: Option<bool>,

    /// Whether code action supports the `isPreferred` property.
    ///
    /// @since 3.15.0
    pub is_preferred_support: Option<bool>,

    /// Whether the client supports resolving additional code action
    /// properties via a separate `codeAction/resolve` request.
    ///
    /// @since 3.16.0
    pub resolve_support: Option<ClientCodeActionResolveOptions>,
}

/// The client capabilities  of a [CodeLensRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeLensClientCapabilities {
    /// Whether code lens supports dynamic registration.
    pub dynamic_registration: Option<bool>,
}

/// The client capabilities of a [DocumentLinkRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentLinkClientCapabilities {
    /// Whether document link supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// Whether the client supports the `tooltip` property on `DocumentLink`.
    ///
    /// @since 3.15.0
    pub tooltip_support: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentColorClientCapabilities {
    /// Whether implementation supports dynamic registration. If this is set to `true`
    /// the client supports the new `DocumentColorRegistrationOptions` return value
    /// for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,
}

/// Client capabilities of a [DocumentFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentFormattingClientCapabilities {
    /// Whether formatting supports dynamic registration.
    pub dynamic_registration: Option<bool>,
}

/// Client capabilities of a [DocumentRangeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentRangeFormattingClientCapabilities {
    /// Whether range formatting supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// Whether the client supports formatting multiple ranges at once.
    ///
    /// @since 3.18.0
    /// @proposed
    #[cfg(feature = "proposed")]
    pub ranges_support: Option<bool>,
}

/// Client capabilities of a [DocumentOnTypeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentOnTypeFormattingClientCapabilities {
    /// Whether on type formatting supports dynamic registration.
    pub dynamic_registration: Option<bool>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RenameClientCapabilities {
    /// Whether rename supports dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// Whether the client honors the change annotations in
    /// text edits and resource operations returned via the
    /// rename request's workspace edit by for example presenting
    /// the workspace edit in the user interface and asking
    /// for confirmation.
    ///
    /// @since 3.16.0
    pub honors_change_annotations: Option<bool>,

    /// Client supports testing for validity of rename operations
    /// before execution.
    ///
    /// @since 3.12.0
    pub prepare_support: Option<bool>,

    /// Client supports the default behavior result.
    ///
    /// The value indicates the default behavior used by the
    /// client.
    ///
    /// @since 3.16.0
    pub prepare_support_default_behavior: Option<PrepareSupportDefaultBehavior>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct FoldingRangeClientCapabilities {
    /// Whether implementation supports dynamic registration for folding range
    /// providers. If this is set to `true` the client supports the new
    /// `FoldingRangeRegistrationOptions` return value for the corresponding
    /// server capability as well.
    pub dynamic_registration: Option<bool>,

    /// Specific options for the folding range.
    ///
    /// @since 3.17.0
    pub folding_range: Option<ClientFoldingRangeOptions>,

    /// Specific options for the folding range kind.
    ///
    /// @since 3.17.0
    pub folding_range_kind: Option<ClientFoldingRangeKindOptions>,

    /// If set, the client signals that it only supports folding complete lines.
    /// If set, client will ignore specified `startCharacter` and `endCharacter`
    /// properties in a FoldingRange.
    pub line_folding_only: Option<bool>,

    /// The maximum number of folding ranges that the client prefers to receive
    /// per document. The value serves as a hint, servers are free to follow the
    /// limit.
    pub range_limit: Option<u32>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SelectionRangeClientCapabilities {
    /// Whether implementation supports dynamic registration for selection range providers. If this is set to `true`
    /// the client supports the new `SelectionRangeRegistrationOptions` return value for the corresponding server
    /// capability as well.
    pub dynamic_registration: Option<bool>,
}

/// The publish diagnostic client capabilities.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct PublishDiagnosticsClientCapabilities {
    /// Client supports a codeDescription property
    ///
    /// @since 3.16.0
    pub code_description_support: Option<bool>,

    /// Whether code action supports the `data` property which is
    /// preserved between a `textDocument/publishDiagnostics` and
    /// `textDocument/codeAction` request.
    ///
    /// @since 3.16.0
    pub data_support: Option<bool>,

    /// Whether the clients accepts diagnostics with related information.
    pub related_information: Option<bool>,

    /// Client supports the tag property to provide meta data about a diagnostic.
    /// Clients supporting tags have to handle unknown tags gracefully.
    ///
    /// @since 3.15.0
    pub tag_support: Option<ClientDiagnosticsTagOptions>,

    /// Whether the client interprets the version property of the
    /// `textDocument/publishDiagnostics` notification's parameter.
    ///
    /// @since 3.15.0
    pub version_support: Option<bool>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyClientCapabilities {
    /// Whether implementation supports dynamic registration. If this is set to `true`
    /// the client supports the new `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SemanticTokensClientCapabilities {
    /// Whether the client uses semantic tokens to augment existing
    /// syntax tokens. If set to `true` client side created syntax
    /// tokens and semantic tokens are both used for colorization. If
    /// set to `false` the client only uses the returned semantic tokens
    /// for colorization.
    ///
    /// If the value is `undefined` then the client behavior is not
    /// specified.
    ///
    /// @since 3.17.0
    pub augments_syntax_tokens: Option<bool>,

    /// Whether implementation supports dynamic registration. If this is set to `true`
    /// the client supports the new `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,

    /// The token formats the clients supports.
    pub formats: Vec<TokenFormat>,

    /// Whether the client supports tokens that can span multiple lines.
    pub multiline_token_support: Option<bool>,

    /// Whether the client supports tokens that can overlap each other.
    pub overlapping_token_support: Option<bool>,

    /// Which requests the client supports and might send to the server
    /// depending on the server's capability. Please note that clients might not
    /// show semantic tokens or degrade some of the user experience if a range
    /// or full request is advertised by the client but not provided by the
    /// server. If for example the client capability `requests.full` and
    /// `request.range` are both set to true but the server only provides a
    /// range provider the client might not render a minimap correctly or might
    /// even decide to not show any semantic tokens at all.
    pub requests: ClientSemanticTokensRequestOptions,

    /// Whether the client allows the server to actively cancel a
    /// semantic token request, e.g. supports returning
    /// LSPErrorCodes.ServerCancelled. If a server does the client
    /// needs to retrigger the request.
    ///
    /// @since 3.17.0
    pub server_cancel_support: Option<bool>,

    /// The token modifiers that the client supports.
    pub token_modifiers: Vec<String>,

    /// The token types that the client supports.
    pub token_types: Vec<String>,
}

/// Client capabilities for the linked editing range request.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LinkedEditingRangeClientCapabilities {
    /// Whether implementation supports dynamic registration. If this is set to `true`
    /// the client supports the new `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,
}

/// Client capabilities specific to the moniker request.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct MonikerClientCapabilities {
    /// Whether moniker supports dynamic registration. If this is set to `true`
    /// the client supports the new `MonikerRegistrationOptions` return value
    /// for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,
}

/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchyClientCapabilities {
    /// Whether implementation supports dynamic registration. If this is set to `true`
    /// the client supports the new `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,
}

/// Client capabilities specific to inline values.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineValueClientCapabilities {
    /// Whether implementation supports dynamic registration for inline value providers.
    pub dynamic_registration: Option<bool>,
}

/// Inlay hint client capabilities.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlayHintClientCapabilities {
    /// Whether inlay hints support dynamic registration.
    pub dynamic_registration: Option<bool>,

    /// Indicates which properties a client can resolve lazily on an inlay
    /// hint.
    pub resolve_support: Option<ClientInlayHintResolveOptions>,
}

/// Client capabilities specific to diagnostic pull requests.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DiagnosticClientCapabilities {
    /// Whether implementation supports dynamic registration. If this is set to `true`
    /// the client supports the new `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,

    /// Whether the clients supports related documents for document diagnostic pulls.
    pub related_document_support: Option<bool>,
}

/// Client capabilities specific to inline completions.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlineCompletionClientCapabilities {
    /// Whether implementation supports dynamic registration for inline completion providers.
    pub dynamic_registration: Option<bool>,
}

/// Notebook specific client capabilities.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentSyncClientCapabilities {
    /// Whether implementation supports dynamic registration. If this is
    /// set to `true` the client supports the new
    /// `(TextDocumentRegistrationOptions & StaticRegistrationOptions)`
    /// return value for the corresponding server capability as well.
    pub dynamic_registration: Option<bool>,

    /// The client supports sending execution summary data per cell.
    pub execution_summary_support: Option<bool>,
}

/// Show message request client capabilities
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ShowMessageRequestClientCapabilities {
    /// Capabilities specific to the `MessageActionItem` type.
    pub message_action_item: Option<ClientShowMessageActionItemOptions>,
}

/// Client capabilities for the showDocument request.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ShowDocumentClientCapabilities {
    /// The client has support for the showDocument
    /// request.
    pub support: bool,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct StaleRequestSupportOptions {
    /// The client will actively cancel the request.
    pub cancel: bool,

    /// The list of requests for which the client
    /// will retry the request if it receives a
    /// response with error code `ContentModified`
    pub retry_on_content_modified: Vec<String>,
}

/// Client capabilities specific to regular expressions.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct RegularExpressionsClientCapabilities {
    /// The engine's name.
    pub engine: String,

    /// The engine's version.
    pub version: Option<String>,
}

/// Client capabilities specific to the used markdown parser.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct MarkdownClientCapabilities {
    /// A list of HTML tags that the client allows / supports in
    /// Markdown.
    ///
    /// @since 3.17.0
    pub allowed_tags: Option<Vec<String>>,

    /// The name of the parser.
    pub parser: String,

    /// The version of the parser.
    pub version: Option<String>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookCellLanguage {
    pub language: String,
}

/// A notebook document filter where `notebookType` is required field.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentFilterNotebookType {
    /// The type of the enclosing notebook.
    pub notebook_type: String,

    /// A glob pattern.
    pub pattern: Option<String>,

    /// A Uri [scheme][`Uri::scheme`], like `file` or `untitled`.
    pub scheme: Option<String>,
}

/// A notebook document filter where `scheme` is required field.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentFilterScheme {
    /// The type of the enclosing notebook.
    pub notebook_type: Option<String>,

    /// A glob pattern.
    pub pattern: Option<String>,

    /// A Uri [scheme][`Uri::scheme`], like `file` or `untitled`.
    pub scheme: String,
}

/// A notebook document filter where `pattern` is required field.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentFilterPattern {
    /// The type of the enclosing notebook.
    pub notebook_type: Option<String>,

    /// A glob pattern.
    pub pattern: String,

    /// A Uri [scheme][`Uri::scheme`], like `file` or `untitled`.
    pub scheme: Option<String>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ChangeAnnotationsSupportOptions {
    /// Whether the client groups edits with equal labels into tree nodes,
    /// for instance all edits labelled with "Changes in Strings" would
    /// be a tree node.
    pub groups_on_label: Option<bool>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientSymbolKindOptions {
    /// The symbol kind values the client supports. When this
    /// property exists the client also guarantees that it will
    /// handle values outside its set gracefully and falls back
    /// to a default value when unknown.
    ///
    /// If this property is not present the client only supports
    /// the symbol kinds from `File` to `Array` as defined in
    /// the initial version of the protocol.
    pub value_set: Option<Vec<SymbolKind>>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientSymbolTagOptions {
    /// The tags supported by the client.
    pub value_set: Vec<SymbolTag>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientSymbolResolveOptions {
    /// The properties that a client can resolve lazily. Usually
    /// `location.range`
    pub properties: Vec<String>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientCompletionItemOptions {
    /// Client supports commit characters on a completion item.
    pub commit_characters_support: Option<bool>,

    /// Client supports the deprecated property on a completion item.
    pub deprecated_support: Option<bool>,

    /// Client supports the following content formats for the documentation
    /// property. The order describes the preferred format of the client.
    pub documentation_format: Option<Vec<MarkupKind>>,

    /// Client support insert replace edit to control different behavior if a
    /// completion item is inserted in the text or should replace text.
    ///
    /// @since 3.16.0
    pub insert_replace_support: Option<bool>,

    /// The client supports the `insertTextMode` property on
    /// a completion item to override the whitespace handling mode
    /// as defined by the client (see `insertTextMode`).
    ///
    /// @since 3.16.0
    pub insert_text_mode_support: Option<ClientCompletionItemInsertTextModeOptions>,

    /// The client has support for completion item label
    /// details (see also `CompletionItemLabelDetails`).
    ///
    /// @since 3.17.0
    pub label_details_support: Option<bool>,

    /// Client supports the preselect property on a completion item.
    pub preselect_support: Option<bool>,

    /// Indicates which properties a client can resolve lazily on a completion
    /// item. Before version 3.16.0 only the predefined properties `documentation`
    /// and `details` could be resolved lazily.
    ///
    /// @since 3.16.0
    pub resolve_support: Option<ClientCompletionItemResolveOptions>,

    /// Client supports snippets as insert text.
    ///
    /// A snippet can define tab stops and placeholders with `$1`, `$2`
    /// and `${3:foo}`. `$0` defines the final tab stop, it defaults to
    /// the end of the snippet. Placeholders with equal identifiers are linked,
    /// that is typing in one will update others too.
    pub snippet_support: Option<bool>,

    /// Client supports the tag property on a completion item. Clients supporting
    /// tags have to handle unknown tags gracefully. Clients especially need to
    /// preserve unknown tags when sending a completion item back to the server in
    /// a resolve call.
    ///
    /// @since 3.15.0
    pub tag_support: Option<CompletionItemTagOptions>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientCompletionItemOptionsKind {
    /// The completion item kind values the client supports. When this
    /// property exists the client also guarantees that it will
    /// handle values outside its set gracefully and falls back
    /// to a default value when unknown.
    ///
    /// If this property is not present the client only supports
    /// the completion items kinds from `Text` to `Reference` as defined in
    /// the initial version of the protocol.
    pub value_set: Option<Vec<CompletionItemKind>>,
}

/// The client supports the following `CompletionList` specific
/// capabilities.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionListCapabilities {
    /// The client supports the following itemDefaults on
    /// a completion list.
    ///
    /// The value lists the supported property names of the
    /// `CompletionList.itemDefaults` object. If omitted
    /// no properties are supported.
    ///
    /// @since 3.17.0
    pub item_defaults: Option<Vec<String>>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientSignatureInformationOptions {
    /// The client supports the `activeParameter` property on `SignatureInformation`
    /// literal.
    ///
    /// @since 3.16.0
    pub active_parameter_support: Option<bool>,

    /// Client supports the following content formats for the documentation
    /// property. The order describes the preferred format of the client.
    pub documentation_format: Option<Vec<MarkupKind>>,

    /// Client capabilities specific to parameter information.
    pub parameter_information: Option<ClientSignatureParameterInformationOptions>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientCodeActionLiteralOptions {
    /// The code action kind is support with the following value
    /// set.
    pub code_action_kind: ClientCodeActionKindOptions,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientCodeActionResolveOptions {
    /// The properties that a client can resolve lazily.
    pub properties: Vec<String>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientFoldingRangeKindOptions {
    /// The folding range kind values the client supports. When this
    /// property exists the client also guarantees that it will
    /// handle values outside its set gracefully and falls back
    /// to a default value when unknown.
    pub value_set: Option<Vec<CustomStringEnum<FoldingRangeKind>>>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientFoldingRangeOptions {
    /// If set, the client signals that it supports setting collapsedText on
    /// folding ranges to display custom labels instead of the default text.
    ///
    /// @since 3.17.0
    pub collapsed_text: Option<bool>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientDiagnosticsTagOptions {
    /// The tags supported by the client.
    pub value_set: Vec<DiagnosticTag>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientSemanticTokensRequestOptions {
    /// The client will send the `textDocument/semanticTokens/full` request if
    /// the server provides a corresponding handler.
    pub full: Option<OR2<bool, ClientSemanticTokensRequestFullDelta>>,

    /// The client will send the `textDocument/semanticTokens/range` request if
    /// the server provides a corresponding handler.
    pub range: Option<OR2<bool, LSPObject>>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientInlayHintResolveOptions {
    /// The properties that a client can resolve lazily.
    pub properties: Vec<String>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientShowMessageActionItemOptions {
    /// Whether the client supports additional attributes which
    /// are preserved and send back to the server in the
    /// request's response.
    pub additional_properties_support: Option<bool>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionItemTagOptions {
    /// The tags supported by the client.
    pub value_set: Vec<CompletionItemTag>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientCompletionItemResolveOptions {
    /// The properties that a client can resolve lazily.
    pub properties: Vec<String>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientCompletionItemInsertTextModeOptions {
    pub value_set: Vec<InsertTextMode>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientSignatureParameterInformationOptions {
    /// The client supports processing label offsets instead of a
    /// simple label string.
    ///
    /// @since 3.14.0
    pub label_offset_support: Option<bool>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientCodeActionKindOptions {
    /// The code action kind values the client supports. When this
    /// property exists the client also guarantees that it will
    /// handle values outside its set gracefully and falls back
    /// to a default value when unknown.
    pub value_set: Vec<CustomStringEnum<CodeActionKind>>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientSemanticTokensRequestFullDelta {
    /// The client will send the `textDocument/semanticTokens/full/delta` request if
    /// the server provides a corresponding handler.
    pub delta: Option<bool>,
}

/// The `workspace/didChangeWorkspaceFolders` notification is sent from the client to the server when the workspace
/// folder configuration changes.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDidChangeWorkspaceFoldersNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidChangeWorkspaceFoldersParams,
}

/// The `window/workDoneProgress/cancel` notification is sent from  the client to the server to cancel a progress
/// initiated on the server side.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowWorkDoneProgressCancelNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: WorkDoneProgressCancelParams,
}

/// The did create files notification is sent from the client to the server when
/// files were created from within the client.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDidCreateFilesNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: CreateFilesParams,
}

/// The did rename files notification is sent from the client to the server when
/// files were renamed from within the client.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDidRenameFilesNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: RenameFilesParams,
}

/// The will delete files request is sent from the client to the server before files are actually
/// deleted as long as the deletion is triggered from within the client.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDidDeleteFilesNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DeleteFilesParams,
}

/// A notification sent when a notebook opens.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentDidOpenNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidOpenNotebookDocumentParams,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentDidChangeNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidChangeNotebookDocumentParams,
}

/// A notification sent when a notebook document is saved.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentDidSaveNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidSaveNotebookDocumentParams,
}

/// A notification sent when a notebook closes.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct NotebookDocumentDidCloseNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidCloseNotebookDocumentParams,
}

/// The initialized notification is sent from the client to the
/// server after the client is fully initialized and the server
/// is allowed to send requests from the server to the client.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InitializedNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: Option<LSPAny>,
}

/// The exit event is sent from the client to the server to
/// ask the server to exit its process.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ExitNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: Option<LSPNull>,
}

/// The configuration change notification is sent from the client to the server
/// when the client's configuration has changed. The notification contains
/// the changed configuration as defined by the language client.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDidChangeConfigurationNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidChangeConfigurationParams,
}

/// The show message notification is sent from a server to a client to ask
/// the client to display a particular message in the user interface.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowShowMessageNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: ShowMessageParams,
}

/// The log message notification is sent from the server to the client to ask
/// the client to log a particular message.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowLogMessageNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: LogMessageParams,
}

/// The telemetry event notification is sent from the server to the client to ask
/// the client to log telemetry data.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TelemetryEventNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: Option<LSPAny>,
}

/// The document open notification is sent from the client to the server to signal
/// newly opened text documents. The document's truth is now managed by the client
/// and the server must not try to read the document's truth using the document's
/// uri. Open in this sense means it is managed by the client. It doesn't necessarily
/// mean that its content is presented in an editor. An open notification must not
/// be sent more than once without a corresponding close notification send before.
/// This means open and close notification must be balanced and the max open count
/// is one.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDidOpenNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidOpenTextDocumentParams,
}

/// The document change notification is sent from the client to the server to signal
/// changes to a text document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDidChangeNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidChangeTextDocumentParams,
}

/// The document close notification is sent from the client to the server when
/// the document got closed in the client. The document's truth now exists where
/// the document's uri points to (e.g. if the document's uri is a file uri the
/// truth now exists on disk). As with the open notification the close notification
/// is about managing the document's content. Receiving a close notification
/// doesn't mean that the document was open in an editor before. A close
/// notification requires a previous open notification to be sent.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDidCloseNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidCloseTextDocumentParams,
}

/// The document save notification is sent from the client to the server when
/// the document got saved in the client.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDidSaveNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidSaveTextDocumentParams,
}

/// A document will save notification is sent from the client to the server before
/// the document is actually saved.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentWillSaveNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: WillSaveTextDocumentParams,
}

/// The watched files notification is sent from the client to the server when
/// the client detects changes to file watched by the language client.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDidChangeWatchedFilesNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: DidChangeWatchedFilesParams,
}

/// Diagnostics notification are sent from the server to the client to signal
/// results of validation runs.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentPublishDiagnosticsNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: PublishDiagnosticsParams,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct SetTraceNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: SetTraceParams,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct LogTraceNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: LogTraceParams,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CancelRequestNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: CancelParams,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ProgressNotification {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPNotificationMethods,

    pub params: ProgressParams,
}

/// An identifier to denote a specific request.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum LSPId {
    Int(i32),
    String(String),
}

/// An identifier to denote a specific response.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(untagged)]
pub enum LSPIdOptional {
    Int(i32),
    String(String),
    None,
}

/// A request to resolve the implementation locations of a symbol at a given text
/// document position. The request's parameter is of type [TextDocumentPositionParams]
/// the response is of type [Definition] or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentImplementationRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: ImplementationParams,
}

/// Response to the [TextDocumentImplementationRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentImplementationResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<OR2<Definition, Vec<DefinitionLink>>>,
}

/// A request to resolve the type definition locations of a symbol at a given text
/// document position. The request's parameter is of type [TextDocumentPositionParams]
/// the response is of type [Definition] or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentTypeDefinitionRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: TypeDefinitionParams,
}

/// Response to the [TextDocumentTypeDefinitionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentTypeDefinitionResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<OR2<Definition, Vec<DefinitionLink>>>,
}

/// The `workspace/workspaceFolders` is sent from the server to the client to fetch the open workspace folders.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceWorkspaceFoldersRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: Option<LSPNull>,
}

/// Response to the [WorkspaceWorkspaceFoldersRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceWorkspaceFoldersResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<WorkspaceFolder>>,
}

/// The 'workspace/configuration' request is sent from the server to the client to fetch a certain
/// configuration setting.
///
/// This pull model replaces the old push model were the client signaled configuration change via an
/// event. If the server still needs to react to configuration changes (since the server caches the
/// result of `workspace/configuration` requests) the server should register for an empty configuration
/// change event and empty the cache if such an event is received.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceConfigurationRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: ConfigurationParams,
}

/// Response to the [WorkspaceConfigurationRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceConfigurationResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: Vec<LSPAny>,
}

/// A request to list all color symbols found in a given text document. The request's
/// parameter is of type [DocumentColorParams] the
/// response is of type {@link ColorInformation ColorInformation[]} or a Thenable
/// that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDocumentColorRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentColorParams,
}

/// Response to the [TextDocumentDocumentColorRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDocumentColorResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: Vec<ColorInformation>,
}

/// A request to list all presentation for a color. The request's
/// parameter is of type [ColorPresentationParams] the
/// response is of type {@link ColorInformation ColorInformation[]} or a Thenable
/// that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentColorPresentationRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: ColorPresentationParams,
}

/// Response to the [TextDocumentColorPresentationRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentColorPresentationResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: Vec<ColorPresentation>,
}

/// A request to provide folding ranges in a document. The request's
/// parameter is of type [FoldingRangeParams], the
/// response is of type [FoldingRangeList] or a Thenable
/// that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentFoldingRangeRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: FoldingRangeParams,
}

/// Response to the [TextDocumentFoldingRangeRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentFoldingRangeResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<FoldingRange>>,
}

/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceFoldingRangeRefreshRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: Option<LSPNull>,
}

/// Response to the [WorkspaceFoldingRangeRefreshRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceFoldingRangeRefreshResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// A request to resolve the type definition locations of a symbol at a given text
/// document position. The request's parameter is of type [TextDocumentPositionParams]
/// the response is of type [Declaration] or a typed array of [DeclarationLink]
/// or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDeclarationRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DeclarationParams,
}

/// Response to the [TextDocumentDeclarationRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDeclarationResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<OR2<Declaration, Vec<DeclarationLink>>>,
}

/// A request to provide selection ranges in a document. The request's
/// parameter is of type [SelectionRangeParams], the
/// response is of type {@link SelectionRange SelectionRange[]} or a Thenable
/// that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSelectionRangeRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: SelectionRangeParams,
}

/// Response to the [TextDocumentSelectionRangeRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSelectionRangeResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<SelectionRange>>,
}

/// The `window/workDoneProgress/create` request is sent from the server to the client to initiate progress
/// reporting from the server.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowWorkDoneProgressCreateRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: WorkDoneProgressCreateParams,
}

/// Response to the [WindowWorkDoneProgressCreateRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowWorkDoneProgressCreateResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// A request to result a `CallHierarchyItem` in a document at a given position.
/// Can be used as an input to an incoming or outgoing call hierarchy.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentPrepareCallHierarchyRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CallHierarchyPrepareParams,
}

/// Response to the [TextDocumentPrepareCallHierarchyRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentPrepareCallHierarchyResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<CallHierarchyItem>>,
}

/// A request to resolve the incoming calls for a given `CallHierarchyItem`.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyIncomingCallsRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CallHierarchyIncomingCallsParams,
}

/// Response to the [CallHierarchyIncomingCallsRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyIncomingCallsResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<CallHierarchyIncomingCall>>,
}

/// A request to resolve the outgoing calls for a given `CallHierarchyItem`.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyOutgoingCallsRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CallHierarchyOutgoingCallsParams,
}

/// Response to the [CallHierarchyOutgoingCallsRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CallHierarchyOutgoingCallsResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<CallHierarchyOutgoingCall>>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSemanticTokensFullRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: SemanticTokensParams,
}

/// Response to the [TextDocumentSemanticTokensFullRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSemanticTokensFullResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<SemanticTokens>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSemanticTokensFullDeltaRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: SemanticTokensDeltaParams,
}

/// Response to the [TextDocumentSemanticTokensFullDeltaRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSemanticTokensFullDeltaResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<OR2<SemanticTokens, SemanticTokensDelta>>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSemanticTokensRangeRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: SemanticTokensRangeParams,
}

/// Response to the [TextDocumentSemanticTokensRangeRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSemanticTokensRangeResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<SemanticTokens>,
}

/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSemanticTokensRefreshRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: Option<LSPNull>,
}

/// Response to the [WorkspaceSemanticTokensRefreshRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSemanticTokensRefreshResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// A request to show a document. This request might open an
/// external program depending on the value of the URI to open.
/// For example a request to open `https://code.visualstudio.com/`
/// will very likely open the URI in a WEB browser.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowShowDocumentRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: ShowDocumentParams,
}

/// Response to the [WindowShowDocumentRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowShowDocumentResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: ShowDocumentResult,
}

/// A request to provide ranges that can be edited together.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentLinkedEditingRangeRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: LinkedEditingRangeParams,
}

/// Response to the [TextDocumentLinkedEditingRangeRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentLinkedEditingRangeResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<LinkedEditingRanges>,
}

/// The will create files request is sent from the client to the server before files are actually
/// created as long as the creation is triggered from within the client.
///
/// The request can return a `WorkspaceEdit` which will be applied to workspace before the
/// files are created. Hence the `WorkspaceEdit` can not manipulate the content of the file
/// to be created.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceWillCreateFilesRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CreateFilesParams,
}

/// Response to the [WorkspaceWillCreateFilesRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceWillCreateFilesResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<WorkspaceEdit>,
}

/// The will rename files request is sent from the client to the server before files are actually
/// renamed as long as the rename is triggered from within the client.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceWillRenameFilesRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: RenameFilesParams,
}

/// Response to the [WorkspaceWillRenameFilesRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceWillRenameFilesResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<WorkspaceEdit>,
}

/// The did delete files notification is sent from the client to the server when
/// files were deleted from within the client.
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceWillDeleteFilesRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DeleteFilesParams,
}

/// Response to the [WorkspaceWillDeleteFilesRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceWillDeleteFilesResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<WorkspaceEdit>,
}

/// A request to get the moniker of a symbol at a given text document position.
/// The request parameter is of type [TextDocumentPositionParams].
/// The response is of type {@link Moniker Moniker[]} or `null`.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentMonikerRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: MonikerParams,
}

/// Response to the [TextDocumentMonikerRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentMonikerResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<Moniker>>,
}

/// A request to result a `TypeHierarchyItem` in a document at a given position.
/// Can be used as an input to a subtypes or supertypes type hierarchy.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentPrepareTypeHierarchyRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: TypeHierarchyPrepareParams,
}

/// Response to the [TextDocumentPrepareTypeHierarchyRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentPrepareTypeHierarchyResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<TypeHierarchyItem>>,
}

/// A request to resolve the supertypes for a given `TypeHierarchyItem`.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchySupertypesRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: TypeHierarchySupertypesParams,
}

/// Response to the [TypeHierarchySupertypesRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchySupertypesResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<TypeHierarchyItem>>,
}

/// A request to resolve the subtypes for a given `TypeHierarchyItem`.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchySubtypesRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: TypeHierarchySubtypesParams,
}

/// Response to the [TypeHierarchySubtypesRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TypeHierarchySubtypesResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<TypeHierarchyItem>>,
}

/// A request to provide inline values in a document. The request's parameter is of
/// type [InlineValueParams], the response is of type
/// {@link InlineValue InlineValue[]} or a Thenable that resolves to such.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentInlineValueRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: InlineValueParams,
}

/// Response to the [TextDocumentInlineValueRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentInlineValueResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<InlineValue>>,
}

/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceInlineValueRefreshRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: Option<LSPNull>,
}

/// Response to the [WorkspaceInlineValueRefreshRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceInlineValueRefreshResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// A request to provide inlay hints in a document. The request's parameter is of
/// type [InlayHintsParams], the response is of type
/// {@link InlayHint InlayHint[]} or a Thenable that resolves to such.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentInlayHintRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: InlayHintParams,
}

/// Response to the [TextDocumentInlayHintRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentInlayHintResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<InlayHint>>,
}

/// A request to resolve additional properties for an inlay hint.
/// The request's parameter is of type [InlayHint], the response is
/// of type [InlayHint] or a Thenable that resolves to such.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlayHintResolveRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: InlayHint,
}

/// Response to the [InlayHintResolveRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InlayHintResolveResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: InlayHint,
}

/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceInlayHintRefreshRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: Option<LSPNull>,
}

/// Response to the [WorkspaceInlayHintRefreshRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceInlayHintRefreshResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// The document diagnostic request definition.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDiagnosticRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentDiagnosticParams,
}

/// Response to the [TextDocumentDiagnosticRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDiagnosticResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: DocumentDiagnosticReport,
}

/// The workspace diagnostic request definition.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDiagnosticRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: WorkspaceDiagnosticParams,
}

/// Response to the [WorkspaceDiagnosticRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDiagnosticResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: WorkspaceDiagnosticReport,
}

/// The diagnostic refresh request definition.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDiagnosticRefreshRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: Option<LSPNull>,
}

/// Response to the [WorkspaceDiagnosticRefreshRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceDiagnosticRefreshResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// A request to provide inline completions in a document. The request's parameter is of
/// type [InlineCompletionParams], the response is of type
/// {@link InlineCompletion InlineCompletion[]} or a Thenable that resolves to such.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentInlineCompletionRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: InlineCompletionParams,
}

/// Response to the [TextDocumentInlineCompletionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentInlineCompletionResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<OR2<InlineCompletionList, Vec<InlineCompletionItem>>>,
}

/// The `client/registerCapability` request is sent from the server to the client to register a new capability
/// handler on the client side.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientRegisterCapabilityRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: RegistrationParams,
}

/// Response to the [ClientRegisterCapabilityRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientRegisterCapabilityResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// The `client/unregisterCapability` request is sent from the server to the client to unregister a previously registered capability
/// handler on the client side.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientUnregisterCapabilityRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: UnregistrationParams,
}

/// Response to the [ClientUnregisterCapabilityRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ClientUnregisterCapabilityResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// The initialize request is sent from the client to the server.
/// It is sent once as the request after starting up the server.
/// The requests parameter is of type [InitializeParams]
/// the response if of type [InitializeResult] of a Thenable that
/// resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InitializeRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: InitializeParams,
}

/// Response to the [InitializeRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct InitializeResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: InitializeResult,
}

/// A shutdown request is sent from the client to the server.
/// It is sent once when the client decides to shutdown the
/// server. The only notification that is sent after a shutdown request
/// is the exit event.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ShutdownRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: Option<LSPNull>,
}

/// Response to the [ShutdownRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct ShutdownResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// The show message request is sent from the server to the client to show a message
/// and a set of options actions to the user.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowShowMessageRequestRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: ShowMessageRequestParams,
}

/// Response to the [WindowShowMessageRequestRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WindowShowMessageRequestResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<MessageActionItem>,
}

/// A document will save request is sent from the client to the server before
/// the document is actually saved. The request can return an array of TextEdits
/// which will be applied to the text document before it is saved. Please note that
/// clients might drop results if computing the text edits took too long or if a
/// server constantly fails on this request. This is done to keep the save fast and
/// reliable.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentWillSaveWaitUntilRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: WillSaveTextDocumentParams,
}

/// Response to the [TextDocumentWillSaveWaitUntilRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentWillSaveWaitUntilResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<TextEdit>>,
}

/// Request to request completion at a given text document position. The request's
/// parameter is of type [TextDocumentPosition] the response
/// is of type {@link CompletionItem CompletionItem[]} or [CompletionList]
/// or a Thenable that resolves to such.
///
/// The request can delay the computation of the [`detail`][`CompletionItem::detail`]
/// and [`documentation`][`CompletionItem::documentation`] properties to the `completionItem/resolve`
/// request. However, properties that are needed for the initial sorting and filtering, like `sortText`,
/// `filterText`, `insertText`, and `textEdit`, must not be changed during resolve.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentCompletionRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CompletionParams,
}

/// Response to the [TextDocumentCompletionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentCompletionResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<OR2<Vec<CompletionItem>, CompletionList>>,
}

/// Request to resolve additional information for a given completion item.The request's
/// parameter is of type [CompletionItem] the response
/// is of type [CompletionItem] or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionItemResolveRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CompletionItem,
}

/// Response to the [CompletionItemResolveRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CompletionItemResolveResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: CompletionItem,
}

/// Request to request hover information at a given text document position. The request's
/// parameter is of type [TextDocumentPosition] the response is of
/// type [Hover] or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentHoverRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: HoverParams,
}

/// Response to the [TextDocumentHoverRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentHoverResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Hover>,
}

#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSignatureHelpRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: SignatureHelpParams,
}

/// Response to the [TextDocumentSignatureHelpRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentSignatureHelpResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<SignatureHelp>,
}

/// A request to resolve the definition location of a symbol at a given text
/// document position. The request's parameter is of type [TextDocumentPosition]
/// the response is of either type [Definition] or a typed array of
/// [DefinitionLink] or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDefinitionRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DefinitionParams,
}

/// Response to the [TextDocumentDefinitionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDefinitionResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<OR2<Definition, Vec<DefinitionLink>>>,
}

/// A request to resolve project-wide references for the symbol denoted
/// by the given text document position. The request's parameter is of
/// type [ReferenceParams] the response is of type
/// {@link Location Location[]} or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentReferencesRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: ReferenceParams,
}

/// Response to the [TextDocumentReferencesRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentReferencesResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<Location>>,
}

/// Request to resolve a [DocumentHighlight] for a given
/// text document position. The request's parameter is of type [TextDocumentPosition]
/// the request response is an array of type [DocumentHighlight]
/// or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDocumentHighlightRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentHighlightParams,
}

/// Response to the [TextDocumentDocumentHighlightRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDocumentHighlightResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<DocumentHighlight>>,
}

/// A request to list all symbols found in a given text document. The request's
/// parameter is of type [TextDocumentIdentifier] the
/// response is of type {@link SymbolInformation SymbolInformation[]} or a Thenable
/// that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDocumentSymbolRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentSymbolParams,
}

/// Response to the [TextDocumentDocumentSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDocumentSymbolResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<OR2<Vec<SymbolInformation>, Vec<DocumentSymbol>>>,
}

/// A request to provide commands for the given text document and range.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentCodeActionRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CodeActionParams,
}

/// Response to the [TextDocumentCodeActionRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentCodeActionResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<OR2<Command, CodeAction>>>,
}

/// Request to resolve additional information for a given code action.The request's
/// parameter is of type [CodeAction] the response
/// is of type [CodeAction] or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeActionResolveRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CodeAction,
}

/// Response to the [CodeActionResolveRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeActionResolveResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: CodeAction,
}

/// A request to list project-wide symbols matching the query string given
/// by the [WorkspaceSymbolParams]. The response is
/// of type {@link SymbolInformation SymbolInformation[]} or a Thenable that
/// resolves to such.
///
/// @since 3.17.0 - support for WorkspaceSymbol in the returned data. Clients
///  need to advertise support for WorkspaceSymbols via the client capability
///  `workspace.symbol.resolveSupport`.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSymbolRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: WorkspaceSymbolParams,
}

/// Response to the [WorkspaceSymbolRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSymbolResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<OR2<Vec<SymbolInformation>, Vec<WorkspaceSymbol>>>,
}

/// A request to resolve the range inside the workspace
/// symbol's location.
///
/// @since 3.17.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSymbolResolveRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: WorkspaceSymbol,
}

/// Response to the [WorkspaceSymbolResolveRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceSymbolResolveResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: WorkspaceSymbol,
}

/// A request to provide code lens for the given text document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentCodeLensRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CodeLensParams,
}

/// Response to the [TextDocumentCodeLensRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentCodeLensResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<CodeLens>>,
}

/// A request to resolve a command for a given code lens.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeLensResolveRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: CodeLens,
}

/// Response to the [CodeLensResolveRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct CodeLensResolveResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: CodeLens,
}

/// A request to refresh all code actions
///
/// @since 3.16.0
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceCodeLensRefreshRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: Option<LSPNull>,
}

/// Response to the [WorkspaceCodeLensRefreshRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceCodeLensRefreshResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: LSPNull,
}

/// A request to provide document links
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDocumentLinkRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentLinkParams,
}

/// Response to the [TextDocumentDocumentLinkRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentDocumentLinkResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<DocumentLink>>,
}

/// Request to resolve additional information for a given document link. The request's
/// parameter is of type [DocumentLink] the response
/// is of type [DocumentLink] or a Thenable that resolves to such.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentLinkResolveRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentLink,
}

/// Response to the [DocumentLinkResolveRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct DocumentLinkResolveResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: DocumentLink,
}

/// A request to format a whole document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentFormattingRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentFormattingParams,
}

/// Response to the [TextDocumentFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentFormattingResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<TextEdit>>,
}

/// A request to format a range in a document.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentRangeFormattingRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentRangeFormattingParams,
}

/// Response to the [TextDocumentRangeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentRangeFormattingResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<TextEdit>>,
}

/// A request to format ranges in a document.
///
/// @since 3.18.0
/// @proposed
#[cfg(feature = "proposed")]
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentRangesFormattingRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentRangesFormattingParams,
}

/// Response to the [TextDocumentRangesFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentRangesFormattingResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<TextEdit>>,
}

/// A request to format a document on type.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentOnTypeFormattingRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: DocumentOnTypeFormattingParams,
}

/// Response to the [TextDocumentOnTypeFormattingRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentOnTypeFormattingResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Vec<TextEdit>>,
}

/// A request to rename a symbol.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentRenameRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: RenameParams,
}

/// Response to the [TextDocumentRenameRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentRenameResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<WorkspaceEdit>,
}

/// A request to test and perform the setup necessary for a rename.
///
/// @since 3.16 - support for default behavior
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentPrepareRenameRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: PrepareRenameParams,
}

/// Response to the [TextDocumentPrepareRenameRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct TextDocumentPrepareRenameResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<PrepareRenameResult>,
}

/// A request send from the client to the server to execute a command. The request might return
/// a workspace edit which the client will apply to the workspace.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceExecuteCommandRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: ExecuteCommandParams,
}

/// Response to the [WorkspaceExecuteCommandRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceExecuteCommandResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<LSPAny>,
}

/// A request sent from the server to the client to modified certain resources.
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceApplyEditRequest {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPId,

    pub params: ApplyWorkspaceEditParams,
}

/// Response to the [WorkspaceApplyEditRequest].
#[derive(Serialize, Deserialize, PartialEq, Debug, Eq, Clone)]
#[serde(rename_all = "camelCase", deny_unknown_fields)]
pub struct WorkspaceApplyEditResponse {
    /// The version of the JSON RPC protocol.
    pub jsonrpc: String,

    /// The method to be invoked.
    pub method: LSPRequestMethods,

    /// The request id.
    pub id: LSPIdOptional,

    pub result: ApplyWorkspaceEditResult,
}
