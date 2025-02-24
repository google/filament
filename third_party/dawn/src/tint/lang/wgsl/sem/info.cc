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

#include "src/tint/lang/wgsl/sem/info.h"

#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::sem {

Info::Info() = default;

Info::Info(Info&&) = default;

Info::~Info() = default;

Info& Info::operator=(Info&&) = default;

wgsl::DiagnosticSeverity Info::DiagnosticSeverity(const ast::Node* ast_node,
                                                  wgsl::DiagnosticRule rule) const {
    // Get the diagnostic severity modification for a node.
    auto check = [&](auto* node) {
        if (auto severity = node->DiagnosticSeverities().Get(rule)) {
            return *severity;
        }
        return wgsl::DiagnosticSeverity::kUndefined;
    };

    // Get the diagnostic severity modification for a function.
    auto check_func = [&](const sem::Function* func) {
        auto severity = check(func);
        if (severity != wgsl::DiagnosticSeverity::kUndefined) {
            return severity;
        }

        // No severity set on the function, so check the module instead.
        return check(module_);
    };

    // Get the diagnostic severity modification for a statement.
    auto check_stmt = [&](const sem::Statement* stmt) {
        // Walk up the statement hierarchy, checking for diagnostic severity modifications.
        while (true) {
            auto severity = check(stmt);
            if (severity != wgsl::DiagnosticSeverity::kUndefined) {
                return severity;
            }
            if (!stmt->Parent()) {
                break;
            }
            stmt = stmt->Parent();
        }

        // No severity set on the statement, so check the function instead.
        return check_func(stmt->Function());
    };

    // Query the diagnostic severity from the semantic node that corresponds to the AST node.
    auto* sem = Get(ast_node);
    TINT_ASSERT(sem != nullptr);
    auto severity = Switch(
        sem,  //
        [&](const sem::ValueExpression* expr) { return check_stmt(expr->Stmt()); },
        [&](const sem::Statement* stmt) { return check_stmt(stmt); },
        [&](const sem::Function* func) { return check_func(func); },
        [&](Default) {
            // Use the global severity set on the module.
            return check(module_);
        });
    TINT_ASSERT(severity != wgsl::DiagnosticSeverity::kUndefined);
    return severity;
}

}  // namespace tint::sem
