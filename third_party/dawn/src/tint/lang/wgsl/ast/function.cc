// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/function.h"

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/type.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Function);

namespace tint::ast {

Function::Function(NodeID nid,
                   const Source& src,
                   const Identifier* n,
                   VectorRef<const Parameter*> parameters,
                   Type return_ty,
                   const BlockStatement* b,
                   VectorRef<const Attribute*> attrs,
                   VectorRef<const Attribute*> return_type_attrs)
    : Base(nid, src),
      name(n),
      params(std::move(parameters)),
      return_type(return_ty),
      body(b),
      attributes(std::move(attrs)),
      return_type_attributes(std::move(return_type_attrs)) {
    TINT_ASSERT(name);
    if (name) {
        TINT_ASSERT(!name->Is<TemplatedIdentifier>());
    }
    for (auto* param : params) {
        TINT_ASSERT(tint::Is<Parameter>(param));
    }
    for (auto* attr : attributes) {
        TINT_ASSERT(attr);
    }
    for (auto* attr : return_type_attributes) {
        TINT_ASSERT(attr);
    }
}

Function::~Function() = default;

PipelineStage Function::PipelineStage() const {
    if (auto* stage = GetAttribute<StageAttribute>(attributes)) {
        return stage->stage;
    }
    return PipelineStage::kNone;
}

const Function* FunctionList::Find(Symbol sym) const {
    for (auto* func : *this) {
        if (func->name->symbol == sym) {
            return func;
        }
    }
    return nullptr;
}

const Function* FunctionList::Find(Symbol sym, PipelineStage stage) const {
    for (auto* func : *this) {
        if (func->name->symbol == sym && func->PipelineStage() == stage) {
            return func;
        }
    }
    return nullptr;
}

bool FunctionList::HasStage(ast::PipelineStage stage) const {
    for (auto* func : *this) {
        if (func->PipelineStage() == stage) {
            return true;
        }
    }
    return false;
}

}  // namespace tint::ast
