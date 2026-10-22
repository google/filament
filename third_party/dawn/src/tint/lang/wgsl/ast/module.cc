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

#include "src/tint/lang/wgsl/ast/module.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/ast/type_decl.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Module);

namespace tint::ast {

Module::Module(NodeID nid, const Source& src) : Base(nid, src) {}

Module::Module(NodeID nid, const Source& src, VectorRef<const Node*> global_decls)
    : Base(nid, src), global_declarations_(std::move(global_decls)) {
    for (auto* decl : global_declarations_) {
        if (decl == nullptr) {
            continue;
        }
        BinGlobalDeclaration(decl);
    }
}

Module::~Module() = default;

const TypeDecl* Module::LookupType(Symbol name) const {
    for (auto* ty : TypeDecls()) {
        if (ty->name->symbol == name) {
            return ty;
        }
    }
    return nullptr;
}

void Module::AddGlobalDeclaration(const tint::ast::Node* decl) {
    BinGlobalDeclaration(decl);
    global_declarations_.Push(decl);
}

void Module::BinGlobalDeclaration(const tint::ast::Node* decl) {
    Switch(
        decl,  //
        [&](const TypeDecl* type) { type_decls_.Push(type); },
        [&](const Function* func) { functions_.Push(func); },
        [&](const Variable* var) { global_variables_.Push(var); },
        [&](const DiagnosticDirective* diagnostic) { diagnostic_directives_.Push(diagnostic); },
        [&](const Enable* enable) { enables_.Push(enable); },
        [&](const ast::Requires* req) { requires_.Push(req); },
        [&](const ConstAssert* assertion) { const_asserts_.Push(assertion); },  //
        TINT_ICE_ON_NO_MATCH);
}

void Module::AddDiagnosticDirective(const DiagnosticDirective* directive) {
    TINT_ASSERT(directive);
    global_declarations_.Push(directive);
    diagnostic_directives_.Push(directive);
}

void Module::AddEnable(const Enable* enable) {
    TINT_ASSERT(enable);
    global_declarations_.Push(enable);
    enables_.Push(enable);
}

void Module::AddRequires(const ast::Requires* req) {
    TINT_ASSERT(req);
    global_declarations_.Push(req);
    requires_.Push(req);
}

void Module::AddGlobalVariable(const Variable* var) {
    TINT_ASSERT(var);
    global_variables_.Push(var);
    global_declarations_.Push(var);
}

void Module::AddConstAssert(const ConstAssert* assertion) {
    TINT_ASSERT(assertion);
    const_asserts_.Push(assertion);
    global_declarations_.Push(assertion);
}

void Module::AddTypeDecl(const TypeDecl* type) {
    TINT_ASSERT(type);
    type_decls_.Push(type);
    global_declarations_.Push(type);
}

void Module::AddFunction(const Function* func) {
    TINT_ASSERT(func);
    functions_.Push(func);
    global_declarations_.Push(func);
}

bool Module::HasOverrides() const {
    for (auto* var : global_variables_) {
        if (var->As<ast::Override>()) {
            return true;
        }
    }
    return false;
}

}  // namespace tint::ast
