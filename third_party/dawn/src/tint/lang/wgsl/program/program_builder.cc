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

#include "src/tint/lang/wgsl/program/program_builder.h"

#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint {

ProgramBuilder::ProgramBuilder() = default;

ProgramBuilder::ProgramBuilder(ProgramBuilder&& rhs)
    : Builder(std::move(rhs)),
      constants(std::move(rhs.constants)),
      sem_nodes_(std::move(rhs.sem_nodes_)),
      sem_(std::move(rhs.sem_)) {}

ProgramBuilder::~ProgramBuilder() = default;

ProgramBuilder& ProgramBuilder::operator=(ProgramBuilder&& rhs) {
    *static_cast<Builder*>(this) = std::move(rhs);
    constants = std::move(rhs.constants);
    sem_nodes_ = std::move(rhs.sem_nodes_);
    sem_ = std::move(rhs.sem_);
    return *this;
}

ProgramBuilder ProgramBuilder::Wrap(const Program& program) {
    ProgramBuilder builder;
    builder.id_ = program.ID();
    builder.last_ast_node_id_ = program.HighestASTNodeID();
    builder.constants = core::constant::Manager::Wrap(program.Constants());
    builder.ast_ =
        builder.create<ast::Module>(program.AST().source, program.AST().GlobalDeclarations());
    builder.sem_ = sem::Info::Wrap(program.Sem());
    builder.symbols_ = SymbolTable::Wrap(program.Symbols());
    builder.diagnostics_ = program.Diagnostics();
    return builder;
}

void ProgramBuilder::AssertNotMoved() const {
    if (DAWN_UNLIKELY(moved_)) {
        TINT_ICE() << "Attempting to use ProgramBuilder after it has been moved";
    }
}

const core::type::Type* ProgramBuilder::TypeOf(const ast::Expression* expr) const {
    return tint::Switch(
        Sem().Get(expr),  //
        [](const sem::ValueExpression* e) { return e->Type(); },
        [](const sem::TypeExpression* e) { return e->Type(); });
}

const core::type::Type* ProgramBuilder::TypeOf(const ast::Variable* var) const {
    auto* sem = Sem().Get(var);
    return sem ? sem->Type() : nullptr;
}

const core::type::Type* ProgramBuilder::TypeOf(const ast::TypeDecl* type_decl) const {
    return Sem().Get(type_decl);
}

}  // namespace tint
