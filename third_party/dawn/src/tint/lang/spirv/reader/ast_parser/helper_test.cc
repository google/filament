// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/wgsl/writer/ast_printer/ast_printer.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::spirv::reader::ast_parser::test {

// Default to not dumping the SPIR-V assembly.
bool ASTParserWrapperForTest::dump_successfully_converted_spirv_ = false;

ASTParserWrapperForTest::ASTParserWrapperForTest(const std::vector<uint32_t>& input)
    : impl_(input) {}

ASTParserWrapperForTest::~ASTParserWrapperForTest() {
    if (dump_successfully_converted_spirv_ && !skip_dumping_spirv_ && !impl_.spv_binary().empty() &&
        impl_.success()) {
        std::string disassembly = Disassemble(impl_.spv_binary());
        std::cout << "BEGIN ConvertedOk:\n" << disassembly << "\nEND ConvertedOk\n";
    }
}

std::string ToString(const Program& program) {
    wgsl::writer::ASTPrinter writer(program);
    writer.Generate();

    if (!writer.Diagnostics().empty()) {
        return "WGSL writer error: " + writer.Diagnostics().Str();
    }
    return writer.Result();
}

std::string ToString(const Program& program, VectorRef<const ast::Statement*> stmts) {
    wgsl::writer::ASTPrinter writer(program);
    for (const auto* stmt : stmts) {
        writer.EmitStatement(stmt);
    }
    if (!writer.Diagnostics().empty()) {
        return "WGSL writer error: " + writer.Diagnostics().Str();
    }
    return writer.Result();
}

std::string ToString(const Program& program, const ast::Node* node) {
    wgsl::writer::ASTPrinter writer(program);
    return Switch(
        node,
        [&](const ast::Expression* expr) {
            StringStream out;
            writer.EmitExpression(out, expr);
            if (!writer.Diagnostics().empty()) {
                return "WGSL writer error: " + writer.Diagnostics().Str();
            }
            return out.str();
        },
        [&](const ast::Statement* stmt) {
            writer.EmitStatement(stmt);
            if (!writer.Diagnostics().empty()) {
                return "WGSL writer error: " + writer.Diagnostics().Str();
            }
            return writer.Result();
        },
        [&](const ast::Identifier* ident) { return ident->symbol.Name(); },  //
        TINT_ICE_ON_NO_MATCH);
}

}  // namespace tint::spirv::reader::ast_parser::test
