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

#include "src/tint/lang/wgsl/ast/builder.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::ast {

Builder::VarOptions::~VarOptions() = default;
Builder::LetOptions::~LetOptions() = default;
Builder::ConstOptions::~ConstOptions() = default;
Builder::OverrideOptions::~OverrideOptions() = default;

Builder::Builder()
    : id_(GenerationID::New()),
      ast_(ast_nodes_.Create<ast::Module>(id_, AllocateNodeID(), Source{})) {}

Builder::Builder(Builder&& rhs)
    : id_(std::move(rhs.id_)),
      last_ast_node_id_(std::move(rhs.last_ast_node_id_)),
      ast_nodes_(std::move(rhs.ast_nodes_)),
      ast_(rhs.ast_),
      symbols_(std::move(rhs.symbols_)),
      diagnostics_(std::move(rhs.diagnostics_)) {
    rhs.MarkAsMoved();
}

Builder::~Builder() = default;

Builder& Builder::operator=(Builder&& rhs) {
    rhs.MarkAsMoved();
    AssertNotMoved();
    id_ = std::move(rhs.id_);
    last_ast_node_id_ = std::move(rhs.last_ast_node_id_);
    ast_nodes_ = std::move(rhs.ast_nodes_);
    ast_ = std::move(rhs.ast_);
    symbols_ = std::move(rhs.symbols_);
    diagnostics_ = std::move(rhs.diagnostics_);

    return *this;
}

bool Builder::IsValid() const {
    return !diagnostics_.ContainsErrors();
}

void Builder::MarkAsMoved() {
    AssertNotMoved();
    moved_ = true;
}

void Builder::AssertNotMoved() const {
    if (DAWN_UNLIKELY(moved_)) {
        TINT_ICE() << "Attempting to use Builder after it has been moved";
    }
}

Builder::TypesBuilder::TypesBuilder(Builder* pb) : builder(pb) {}

const ast::Statement* Builder::WrapInStatement(const ast::Expression* expr) {
    // Create a temporary variable of inferred type from expr.
    return Decl(Let(symbols_.New(), expr));
}

const ast::VariableDeclStatement* Builder::WrapInStatement(const ast::Variable* v) {
    return create<ast::VariableDeclStatement>(v);
}

const ast::Statement* Builder::WrapInStatement(const ast::Statement* stmt) {
    return stmt;
}

const ast::Function* Builder::WrapInFunction(VectorRef<const ast::Statement*> stmts) {
    return Func("test_function", {}, ty.void_(), std::move(stmts),
                Vector{
                    create<ast::StageAttribute>(ast::PipelineStage::kCompute),
                    WorkgroupSize(1_u, 1_u, 1_u),
                });
}

}  // namespace tint::ast
