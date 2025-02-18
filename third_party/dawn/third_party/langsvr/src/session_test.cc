// Copyright 2024 The langsvr Authors
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

#include "langsvr/lsp/lsp.h"

#include "langsvr/session.h"

#include <gtest/gtest.h>
#include "langsvr/json/builder.h"
#include "langsvr/lsp/decode.h"

#include "gmock/gmock.h"
#include "langsvr/result.h"

namespace langsvr {
namespace {

Result<lsp::InitializeRequest> GetInitializeRequest() {
    // Meaty real-world JSON request.
    static constexpr std::string_view kJSON =
        R"({"processId":71875,"clientInfo":{"name":"My Awesome Editor","version":"1.2.3"},"locale":"en-gb","rootPath":"/home/bob/src/langsvr","rootUri":"file:///home/bob/src/langsvr","capabilities":{"workspace":{"applyEdit":true,"workspaceEdit":{"documentChanges":true,"resourceOperations":["create","rename","delete"],"failureHandling":"textOnlyTransactional","normalizesLineEndings":true,"changeAnnotationSupport":{"groupsOnLabel":true}},"didChangeConfiguration":{"dynamicRegistration":true},"didChangeWatchedFiles":{"dynamicRegistration":true},"symbol":{"dynamicRegistration":true,"symbolKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26]},"tagSupport":{"valueSet":[1]}},"codeLens":{"refreshSupport":true},"executeCommand":{"dynamicRegistration":true},"configuration":true,"workspaceFolders":true,"semanticTokens":{"refreshSupport":true},"fileOperations":{"dynamicRegistration":true,"didCreate":true,"didRename":true,"didDelete":true,"willCreate":true,"willRename":true,"willDelete":true}},"textDocument":{"publishDiagnostics":{"relatedInformation":true,"versionSupport":false,"tagSupport":{"valueSet":[1,2]},"codeDescriptionSupport":true,"dataSupport":true},"synchronization":{"dynamicRegistration":true,"willSave":true,"willSaveWaitUntil":true,"didSave":true},"completion":{"dynamicRegistration":true,"contextSupport":true,"completionItem":{"snippetSupport":true,"commitCharactersSupport":true,"documentationFormat":["markdown","plaintext"],"deprecatedSupport":true,"preselectSupport":true,"tagSupport":{"valueSet":[1]},"insertReplaceSupport":true,"resolveSupport":{"properties":["documentation","detail","additionalTextEdits"]},"insertTextModeSupport":{"valueSet":[1,2]}},"completionItemKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25]}},"hover":{"dynamicRegistration":true,"contentFormat":["markdown","plaintext"]},"signatureHelp":{"dynamicRegistration":true,"signatureInformation":{"documentationFormat":["markdown","plaintext"],"parameterInformation":{"labelOffsetSupport":true},"activeParameterSupport":true},"contextSupport":true},"definition":{"dynamicRegistration":true,"linkSupport":true},"references":{"dynamicRegistration":true},"documentHighlight":{"dynamicRegistration":true},"documentSymbol":{"dynamicRegistration":true,"symbolKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26]},"hierarchicalDocumentSymbolSupport":true,"tagSupport":{"valueSet":[1]},"labelSupport":true},"codeAction":{"dynamicRegistration":true,"isPreferredSupport":true,"disabledSupport":true,"dataSupport":true,"resolveSupport":{"properties":["edit"]},"codeActionLiteralSupport":{"codeActionKind":{"valueSet":["","quickfix","refactor","refactor.extract","refactor.inline","refactor.rewrite","source","source.organizeImports"]}},"honorsChangeAnnotations":false},"codeLens":{"dynamicRegistration":true},"formatting":{"dynamicRegistration":true},"rangeFormatting":{"dynamicRegistration":true},"onTypeFormatting":{"dynamicRegistration":true},"rename":{"dynamicRegistration":true,"prepareSupport":true,"prepareSupportDefaultBehavior":1,"honorsChangeAnnotations":true},"documentLink":{"dynamicRegistration":true,"tooltipSupport":true},"typeDefinition":{"dynamicRegistration":true,"linkSupport":true},"implementation":{"dynamicRegistration":true,"linkSupport":true},"colorProvider":{"dynamicRegistration":true},"foldingRange":{"dynamicRegistration":true,"rangeLimit":5000,"lineFoldingOnly":true},"declaration":{"dynamicRegistration":true,"linkSupport":true},"selectionRange":{"dynamicRegistration":true},"callHierarchy":{"dynamicRegistration":true},"semanticTokens":{"dynamicRegistration":true,"tokenTypes":["namespace","type","class","enum","interface","struct","typeParameter","parameter","variable","property","enumMember","event","function","method","macro","keyword","modifier","comment","string","number","regexp","operator"],"tokenModifiers":["declaration","definition","readonly","static","deprecated","abstract","async","modification","documentation","defaultLibrary"],"formats":["relative"],"requests":{"range":true,"full":{"delta":true}},"multilineTokenSupport":false,"overlappingTokenSupport":false},"linkedEditingRange":{"dynamicRegistration":true}},"window":{"showMessage":{"messageActionItem":{"additionalPropertiesSupport":true}},"showDocument":{"support":true},"workDoneProgress":true},"general":{"regularExpressions":{"engine":"ECMAScript","version":"ES2020"},"markdown":{"parser":"marked","version":"1.1.0"}}},"trace":"off","workspaceFolders":[{"uri":"file:///home/bob/src/langsvr","name":"langsvr"}]})";
    auto b = json::Builder::Create();
    auto msg = b->Parse(kJSON);
    if (msg != Success) {
        return msg.Failure();
    }
    lsp::InitializeRequest request;
    if (auto res = lsp::Decode(*msg.Get(), request); res != Success) {
        return res.Failure();
    }
    return request;
}

TEST(Session, InitializeRequest_ResultOrFailure) {
    auto request = GetInitializeRequest();
    ASSERT_EQ(request, Success);

    Session server_session;
    Session client_session;
    client_session.SetSender([&](std::string_view msg) { return server_session.Receive(msg); });
    server_session.SetSender([&](std::string_view msg) { return client_session.Receive(msg); });

    bool handler_called = false;

    server_session.Register([&](const lsp::InitializeRequest& init)
                                -> Result<lsp::InitializeResult, lsp::InitializeError> {
        handler_called = true;
        EXPECT_EQ(request, init);
        lsp::InitializeResult res;
        res.capabilities.hover_provider = true;
        return res;
    });

    auto response_future = client_session.Send(request.Get());
    ASSERT_EQ(response_future, Success);
    EXPECT_TRUE(handler_called);

    auto response = response_future.Get().get();
    ASSERT_EQ(response, Success);

    lsp::InitializeResult expected;
    expected.capabilities.hover_provider = true;
    EXPECT_EQ(response.Get(), expected);
}

TEST(Session, InitializeRequest_ResultOnly) {
    auto request = GetInitializeRequest();
    ASSERT_EQ(request, Success);

    Session server_session;
    Session client_session;
    client_session.SetSender([&](std::string_view msg) { return server_session.Receive(msg); });
    server_session.SetSender([&](std::string_view msg) { return client_session.Receive(msg); });

    bool handler_called = false;

    server_session.Register([&](const lsp::InitializeRequest& init) {
        handler_called = true;
        EXPECT_EQ(request, init);
        lsp::InitializeResult res;
        res.capabilities.hover_provider = true;
        return res;
    });

    auto response_future = client_session.Send(request.Get());
    ASSERT_EQ(response_future, Success);
    EXPECT_TRUE(handler_called);

    auto response = response_future.Get().get();
    ASSERT_EQ(response, Success);

    lsp::InitializeResult expected;
    expected.capabilities.hover_provider = true;
    EXPECT_EQ(response.Get(), expected);
}

TEST(Session, InitializeRequest_FailureOnly) {
    auto request = GetInitializeRequest();
    ASSERT_EQ(request, Success);

    Session server_session;
    Session client_session;
    client_session.SetSender([&](std::string_view msg) { return server_session.Receive(msg); });
    server_session.SetSender([&](std::string_view msg) { return client_session.Receive(msg); });

    bool handler_called = false;

    server_session.Register([&](const lsp::InitializeRequest& init) {
        handler_called = true;
        EXPECT_EQ(request, init);
        lsp::InitializeError err;
        err.retry = true;
        return err;
    });

    auto response_future = client_session.Send(request.Get());
    ASSERT_EQ(response_future, Success);
    EXPECT_TRUE(handler_called);

    auto response = response_future.Get().get();
    ASSERT_NE(response, Success);

    lsp::InitializeError expected;
    expected.retry = true;
    EXPECT_EQ(response.Failure(), expected);
}

}  // namespace
}  // namespace langsvr
