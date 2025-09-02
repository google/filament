// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/ast_lower/pass_workgroup_id_as_argument.h"

#include <utility>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/utils/containers/hashmap.h"

using namespace tint::core::fluent_types;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::PassWorkgroupIdAsArgument);

namespace tint::spirv::reader {

/// PIMPL state for the transform.
struct PassWorkgroupIdAsArgument::State {
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};
    /// The semantic info.
    const sem::Info& sem = src.Sem();

    /// Map from function to the name of its workgroup_id parameter.
    Hashmap<const ast::Function*, Symbol, 8> func_to_param;
    /// The identifier expressions that have been replaced, other than
    /// the assignments in entry points.
    Hashset<const ast::Expression*, 16> replaced_idexpr;

    /// Constructor
    /// @param program the source program
    explicit State(const Program& program) : src(program) {}

    /// Runs the transform.
    /// @returns the new program
    ApplyResult Run() {
        struct Root {
            const ast::Function* entry_point;
            const ast::Parameter* param;
        };

        // Process all entry points in the module, looking for workgroup_id builtin parameters.
        bool made_changes = false;
        Vector<Root, 4> roots;
        for (auto* func : src.AST().Functions()) {
            if (func->IsEntryPoint()) {
                for (auto* param : func->params) {
                    if (auto* builtin =
                            ast::GetAttribute<ast::BuiltinAttribute>(param->attributes)) {
                        if (builtin->builtin == core::BuiltinValue::kWorkgroupId) {
                            roots.Emplace(func, param);
                        }
                    }
                }
            }
        }
        // Record the name of the parameter for all entry point functions
        // that use the workgroup size builtin.  Do this before traversing
        // call sites of uses of the global wgsize variable.
        for (const auto& root : roots) {
            // Record the name of the parameter for the entry point function.
            func_to_param.Add(root.entry_point, ctx.Clone(root.param->name->symbol));
            made_changes = true;
        }
        for (const auto& root : roots) {
            ProcessBuiltin(root.entry_point, root.param);
        }

        if (!made_changes) {
            return SkipTransform;
        }

        ctx.Clone();
        return resolver::Resolve(b);
    }

    /// Process a workgroup_id builtin.
    /// @param ep the entry point function
    /// @param builtin the builtin parameter
    void ProcessBuiltin(const ast::Function* ep, const ast::Parameter* builtin) {
        // The reader should only produce a single use of the parameter which assigns to a global.
        const auto& users = sem.Get(builtin)->Users();
        TINT_ASSERT(users.Length() == 1u);
        auto* assign = users[0]->Stmt()->Declaration()->As<ast::AssignmentStatement>();
        auto& stmts =
            sem.Get(assign)->Parent()->Declaration()->As<ast::BlockStatement>()->statements;
        auto* rhs = assign->rhs;
        if (auto* call = sem.Get<sem::Call>(rhs)) {
            if (auto* builtin_fn = call->Target()->As<sem::BuiltinFn>()) {
                if (builtin_fn->Fn() == wgsl::BuiltinFn::kBitcast) {
                    // The RHS may be bitcast to a signed integer, so we capture that bitcast.
                    auto let = b.Symbols().New("tint_wgid_bitcast");
                    auto* val = call->Arguments()[0]->Declaration();
                    ctx.InsertBefore(stmts, assign, b.Decl(b.Let(let, ctx.Clone(rhs))));
                    func_to_param.Replace(ep, let);
                    rhs = val;
                }
            }
        }
        TINT_ASSERT(assign && rhs == users[0]->Declaration());
        auto* lhs = sem.GetVal(assign->lhs)->As<sem::VariableUser>();
        TINT_ASSERT(lhs && lhs->Variable()->AddressSpace() == core::AddressSpace::kPrivate);

        // Replace all references to the global variable with a function parameter.
        for (auto* user : lhs->Variable()->Users()) {
            if (user == lhs) {
                // Skip the assignment, which will be removed.
                continue;
            }
            auto* using_func = user->Stmt()->Function()->Declaration();
            if (using_func->IsEntryPoint() && using_func != ep) {
                // The other entry point will update its own uses.
                continue;
            }

            // This use might be reachable from more than one entry point.
            // Only replace it once. If we replaced it every time, then
            // all but the last replacement identifier expressions would
            // be orphaned.
            if (replaced_idexpr.Add(user->Declaration())) {
                auto param = GetParameter(using_func, lhs->Variable()->Declaration()->type);
                ctx.Replace(user->Declaration(), b.Expr(param));
            }
        }

        // Remove the global variable and the assignment to it.
        ctx.Remove(src.AST().GlobalDeclarations(), lhs->Variable()->Declaration());
        ctx.Remove(stmts, assign);
    }

    /// Get the workgroup_id parameter for a function, creating it and updating callsites if needed.
    /// @param func the function
    /// @param type the type of the parameter
    /// @returns the name of the parameter
    Symbol GetParameter(const ast::Function* func, const ast::Type& type) {
        return func_to_param.GetOrAdd(func, [&] {
            // Append a new parameter to the function.
            auto name = b.Symbols().New("tint_wgid");
            ctx.InsertBack(func->params, b.Param(name, ctx.Clone(type)));

            // Recursively update all callsites to pass the workgroup_id as an argument.
            for (auto* callsite : sem.Get(func)->CallSites()) {
                auto caller_func = callsite->Stmt()->Function()->Declaration();
                auto param = GetParameter(caller_func, type);
                ctx.InsertBack(callsite->Declaration()->args, b.Expr(param));
            }

            return name;
        });
    }
};

PassWorkgroupIdAsArgument::PassWorkgroupIdAsArgument() = default;

PassWorkgroupIdAsArgument::~PassWorkgroupIdAsArgument() = default;

ast::transform::Transform::ApplyResult PassWorkgroupIdAsArgument::Apply(
    const Program& src,
    const ast::transform::DataMap&,
    ast::transform::DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::spirv::reader
