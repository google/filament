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

#include "src/tint/lang/wgsl/sem/function.h"

#include "src/tint/lang/wgsl/ast/function.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/must_use_attribute.h"
#include "src/tint/lang/wgsl/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Function);

namespace tint::sem {

Function::Function(const ast::Function* declaration)
    : Base(core::EvaluationStage::kRuntime,
           ast::HasAttribute<ast::MustUseAttribute>(declaration->attributes)),
      declaration_(declaration),
      workgroup_size_{1, 1, 1} {}

Function::~Function() = default;

void Function::AddTransitivelyReferencedGlobal(const sem::GlobalVariable* global) {
    if (transitively_referenced_globals_.Add(global)) {
        for (auto* ref : global->TransitivelyReferencedOverrides()) {
            AddTransitivelyReferencedGlobal(ref);
        }
    }
}

bool Function::HasCallGraphEntryPoint(Symbol symbol) const {
    for (const auto* point : call_graph_entry_points_) {
        if (point->Declaration()->name->symbol == symbol) {
            return true;
        }
    }
    return false;
}

void Function::SetDiagnosticSeverity(wgsl::DiagnosticRule rule, wgsl::DiagnosticSeverity severity) {
    diagnostic_severities_.Add(rule, severity);
}

}  // namespace tint::sem
