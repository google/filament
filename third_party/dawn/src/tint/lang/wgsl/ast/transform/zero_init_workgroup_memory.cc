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

#include "src/tint/lang/wgsl/ast/transform/zero_init_workgroup_memory.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/containers/unique_vector.h"

using namespace tint::core::fluent_types;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::ZeroInitWorkgroupMemory);

namespace tint::ast::transform {
namespace {

bool ShouldRun(const Program& program) {
    for (auto* global : program.AST().GlobalVariables()) {
        if (auto* var = global->As<Var>()) {
            auto* v = program.Sem().Get(var);
            if (v->AddressSpace() == core::AddressSpace::kWorkgroup) {
                return true;
            }
        }
    }
    return false;
}

}  // namespace

using StatementList = tint::Vector<const Statement*, 8>;

/// PIMPL state for the transform
struct ZeroInitWorkgroupMemory::State {
    /// The clone context
    program::CloneContext& ctx;

    /// An alias to *ctx.dst
    ast::Builder& b = *ctx.dst;

    /// The semantic info for the source program.
    const sem::Info& sem = ctx.src->Sem();

    /// The constant size of the workgroup. If 0, then #workgroup_size_expr should
    /// be used instead.
    uint32_t workgroup_size_const = 0;
    /// The size of the workgroup as an expression generator. Use if
    /// #workgroup_size_const is 0.
    std::function<const ast::Expression*()> workgroup_size_expr;

    /// ArrayIndex represents a function on the local invocation index, of
    /// the form: `array_index = (local_invocation_index % modulo) / division`
    struct ArrayIndex {
        /// The RHS of the modulus part of the expression
        uint32_t modulo = 1;
        /// The RHS of the division part of the expression
        uint32_t division = 1;

        /// @returns the hash code of the ArrayIndex
        tint::HashCode HashCode() const { return Hash(modulo, division); }

        /// Equality operator
        /// @param i the ArrayIndex to compare to this ArrayIndex
        /// @returns true if `i` and this ArrayIndex are equal
        bool operator==(const ArrayIndex& i) const {
            return modulo == i.modulo && division == i.division;
        }
    };

    /// A list of unique ArrayIndex
    using ArrayIndices = UniqueVector<ArrayIndex, 4>;

    /// Expression holds information about an expression that is being built for a
    /// statement will zero workgroup values.
    struct Expression {
        /// The AST expression node
        const ast::Expression* expr = nullptr;
        /// The number of iterations required to zero the value
        uint32_t num_iterations = 0;
        /// All array indices used by this expression
        ArrayIndices array_indices;

        /// @returns true if the expr is not null (null usually indicates a failure)
        explicit operator bool() const { return expr != nullptr; }
    };

    /// Statement holds information about a statement that will zero workgroup
    /// values.
    struct Statement {
        /// The AST statement node
        const ast::Statement* stmt;
        /// The number of iterations required to zero the value
        uint32_t num_iterations;
        /// All array indices used by this statement
        ArrayIndices array_indices;
    };

    /// All statements that zero workgroup memory
    std::vector<Statement> statements;

    /// A map of ArrayIndex to the name reserved for the `let` declaration of that
    /// index.
    Hashmap<ArrayIndex, Symbol, 4> array_index_names;

    /// Constructor
    /// @param c the program::CloneContext used for the transform
    explicit State(program::CloneContext& c) : ctx(c) {}

    /// Run inserts the workgroup memory zero-initialization logic at the top of
    /// the given function
    /// @param fn a compute shader entry point function
    void Run(const Function* fn) {
        CalculateWorkgroupSize(GetAttribute<WorkgroupAttribute>(fn->attributes));

        // Generate the workgroup zeroing function
        auto zeroing_fn_name = BuildZeroingFn(fn);
        if (!zeroing_fn_name) {
            return;  // Nothing to do.
        }

        // Get or create the local invocation index parameter on the entry point
        auto local_invocation_index = GetOrCreateLocalInvocationIndex(fn);

        // Prefix the entry point body with a call to the workgroup zeroing function
        ctx.InsertFront(fn->body->statements,
                        b.CallStmt(b.Call(zeroing_fn_name, local_invocation_index)));
    }

    /// Builds a function that zeros all the variables in the workgroup address space transitively
    /// used by @p fn. The built function takes a single `local_invocation_id : u32` parameter
    /// @param fn the entry point function.
    /// @return the name of the workgroup memory zeroing function.
    Symbol BuildZeroingFn(const Function* fn) {
        // Generate a list of statements to zero initialize each of the workgroup storage variables
        // used by `fn`. This will populate #statements.
        auto* func = sem.Get(fn);
        for (auto* var : func->TransitivelyReferencedGlobals()) {
            if (var->AddressSpace() == core::AddressSpace::kWorkgroup) {
                auto get_expr = [&](uint32_t num_values) {
                    auto var_name = ctx.Clone(var->Declaration()->name->symbol);
                    return Expression{b.Expr(var_name), num_values, ArrayIndices{}};
                };
                if (!BuildZeroingStatements(var->Type()->UnwrapRef(), get_expr)) {
                    return Symbol{};
                }
            }
        }

        if (statements.empty()) {
            return Symbol{};  // No workgroup variables to initialize.
        }

        // Take the zeroing statements and bin them by the number of iterations
        // required to zero the workgroup data. We then emit these in blocks,
        // possibly wrapped in if-statements or for-loops.
        std::unordered_map<uint32_t, std::vector<Statement>> stmts_by_num_iterations;
        std::vector<uint32_t> num_sorted_iterations;
        for (auto& s : statements) {
            auto& stmts = stmts_by_num_iterations[s.num_iterations];
            if (stmts.empty()) {
                num_sorted_iterations.emplace_back(s.num_iterations);
            }
            stmts.emplace_back(s);
        }
        std::sort(num_sorted_iterations.begin(), num_sorted_iterations.end());

        auto local_idx = b.Symbols().New("local_idx");

        // Loop over the statements, grouped by num_iterations.
        Vector<const ast::Statement*, 8> init_body;
        for (auto num_iterations : num_sorted_iterations) {
            auto& stmts = stmts_by_num_iterations[num_iterations];

            // Gather all the array indices used by all the statements in the block.
            ArrayIndices array_indices;
            for (auto& s : stmts) {
                for (auto& idx : s.array_indices) {
                    array_indices.Add(idx);
                }
            }

            // Determine the block type used to emit these statements.

            // TODO(crbug.com/tint/2143): Always emit an if statement around zero init, even when
            // workgroup size matches num_iteration, to work around bugs in certain drivers.
            constexpr bool kWorkaroundUnconditionalZeroInitDriverBug = true;

            if (workgroup_size_const == 0 || num_iterations > workgroup_size_const) {
                // Either the workgroup size is dynamic, or smaller than num_iterations.
                // In either case, we need to generate a for loop to ensure we
                // initialize all the array elements.
                //
                //  for (var idx : u32 = local_index;
                //           idx < num_iterations;
                //           idx += workgroup_size) {
                //    ...
                //  }
                auto idx = b.Symbols().New("idx");
                auto* init = b.Decl(b.Var(idx, b.ty.u32(), b.Expr(local_idx)));
                auto* cond = b.LessThan(idx, u32(num_iterations));
                auto* cont = b.Assign(
                    idx, b.Add(idx, workgroup_size_const ? b.Expr(u32(workgroup_size_const))
                                                         : workgroup_size_expr()));

                auto block =
                    DeclareArrayIndices(num_iterations, array_indices, [&] { return b.Expr(idx); });
                for (auto& s : stmts) {
                    block.Push(s.stmt);
                }
                auto* for_loop = b.For(init, cond, cont, b.Block(block));
                init_body.Push(for_loop);
            } else if (num_iterations < workgroup_size_const ||
                       kWorkaroundUnconditionalZeroInitDriverBug) {
                // Workgroup size is a known constant, but is greater than
                // num_iterations. Emit an if statement:
                //
                //  if (local_index < num_iterations) {
                //    ...
                //  }
                auto* cond = b.LessThan(local_idx, u32(num_iterations));
                auto block = DeclareArrayIndices(num_iterations, array_indices,
                                                 [&] { return b.Expr(local_idx); });
                for (auto& s : stmts) {
                    block.Push(s.stmt);
                }
                auto* if_stmt = b.If(cond, b.Block(block));
                init_body.Push(if_stmt);
            } else {
                // Workgroup size exactly equals num_iterations.
                // No need for any conditionals. Just emit a basic block:
                //
                // {
                //    ...
                // }
                auto block = DeclareArrayIndices(num_iterations, array_indices,
                                                 [&] { return b.Expr(local_idx); });
                for (auto& s : stmts) {
                    block.Push(s.stmt);
                }
                init_body.Push(b.Block(std::move(block)));
            }
        }

        // Append a single workgroup barrier after the zero initialization.
        init_body.Push(b.CallStmt(b.Call("workgroupBarrier")));

        // Generate the zero-init function.
        auto name = b.Symbols().New("tint_zero_workgroup_memory");
        b.Func(name, Vector{b.Param(local_idx, b.ty.u32())}, b.ty.void_(),
               b.Block(std::move(init_body)));
        return name;
    }

    /// Looks for an existing `local_invocation_index` parameter on the entry point function @p fn,
    /// or adds a new parameter to the function if it doesn't exist.
    /// @param fn the entry point function.
    /// @return an expression to the `local_invocation_index` parameter.
    const ast::Expression* GetOrCreateLocalInvocationIndex(const Function* fn) {
        // Scan the entry point for an existing local_invocation_index builtin parameter
        std::function<const ast::Expression*()> local_index;
        for (auto* param : fn->params) {
            if (auto* builtin_attr = GetAttribute<BuiltinAttribute>(param->attributes)) {
                if (builtin_attr->builtin == core::BuiltinValue::kLocalInvocationIndex) {
                    return b.Expr(ctx.Clone(param->name->symbol));
                }
            }

            if (auto* str = sem.Get(param)->Type()->As<core::type::Struct>()) {
                for (auto* member : str->Members()) {
                    if (member->Attributes().builtin == core::BuiltinValue::kLocalInvocationIndex) {
                        auto* param_expr = b.Expr(ctx.Clone(param->name->symbol));
                        auto member_name = ctx.Clone(member->Name());
                        return b.MemberAccessor(param_expr, member_name);
                    }
                }
            }
        }

        // No existing local index parameter. Append one to the entry point.
        auto param_name = b.Symbols().New("local_invocation_index");
        auto* local_invocation_index = b.Builtin(core::BuiltinValue::kLocalInvocationIndex);
        auto* param = b.Param(param_name, b.ty.u32(), tint::Vector{local_invocation_index});
        ctx.InsertBack(fn->params, param);
        return b.Expr(param->name->symbol);
    }

    /// BuildZeroingExpr is a function that builds a sub-expression used to zero
    /// workgroup values. `num_values` is the number of elements that the
    /// expression will be used to zero. Returns the expression.
    using BuildZeroingExpr = std::function<Expression(uint32_t num_values)>;

    /// BuildZeroingStatements() generates the statements required to zero
    /// initialize the workgroup storage expression of type `ty`.
    /// @param ty the expression type
    /// @param get_expr a function that builds the AST nodes for the expression.
    /// @returns true on success, false on failure
    [[nodiscard]] bool BuildZeroingStatements(const core::type::Type* ty,
                                              const BuildZeroingExpr& get_expr) {
        if (CanTriviallyZero(ty)) {
            auto var = get_expr(1u);
            if (!var) {
                return false;
            }
            auto* zero_init = b.Call(CreateASTTypeFor(ctx, ty));
            statements.emplace_back(
                Statement{b.Assign(var.expr, zero_init), var.num_iterations, var.array_indices});
            return true;
        }

        if (auto* atomic = ty->As<core::type::Atomic>()) {
            auto* zero_init = b.Call(CreateASTTypeFor(ctx, atomic->Type()));
            auto expr = get_expr(1u);
            if (!expr) {
                return false;
            }
            auto* store = b.Call("atomicStore", b.AddressOf(expr.expr), zero_init);
            statements.emplace_back(
                Statement{b.CallStmt(store), expr.num_iterations, expr.array_indices});
            return true;
        }

        if (auto* str = ty->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                auto name = ctx.Clone(member->Name());
                auto get_member = [&](uint32_t num_values) {
                    auto s = get_expr(num_values);
                    if (!s) {
                        return Expression{};  // error
                    }
                    return Expression{b.MemberAccessor(s.expr, name), s.num_iterations,
                                      s.array_indices};
                };
                if (!BuildZeroingStatements(member->Type(), get_member)) {
                    return false;
                }
            }
            return true;
        }

        if (auto* arr = ty->As<core::type::Array>()) {
            auto get_el = [&](uint32_t num_values) {
                // num_values is the number of values to zero for the element type.
                // The number of iterations required to zero the array and its elements is:
                //      `num_values * arr->Count()`
                // The index for this array is:
                //      `(idx % modulo) / division`
                auto count = arr->ConstantCount();
                if (!count) {
                    ctx.dst->Diagnostics().AddError(Source{})
                        << core::type::Array::kErrExpectedConstantCount;
                    return Expression{};  // error
                }
                auto modulo = num_values * count.value();
                auto division = num_values;
                auto a = get_expr(modulo);
                if (!a) {
                    return Expression{};  // error
                }
                auto array_indices = a.array_indices;
                array_indices.Add(ArrayIndex{modulo, division});
                auto index = array_index_names.GetOrAdd(ArrayIndex{modulo, division},
                                                        [&] { return b.Symbols().New("i"); });
                return Expression{b.IndexAccessor(a.expr, index), a.num_iterations, array_indices};
            };
            return BuildZeroingStatements(arr->ElemType(), get_el);
        }

        TINT_UNREACHABLE() << "could not zero workgroup type: " << ty->FriendlyName();
    }

    /// DeclareArrayIndices returns a list of statements that contain the `let`
    /// declarations for all of the ArrayIndices.
    /// @param num_iterations the number of iterations for the block
    /// @param array_indices the list of array indices to generate `let`
    ///        declarations for
    /// @param iteration a function that returns the index of the current
    ///         iteration.
    /// @returns the list of `let` statements that declare the array indices
    StatementList DeclareArrayIndices(uint32_t num_iterations,
                                      const ArrayIndices& array_indices,
                                      const std::function<const ast::Expression*()>& iteration) {
        StatementList stmts;
        std::map<Symbol, ArrayIndex> indices_by_name;
        for (auto index : array_indices) {
            auto name = array_index_names.Get(index);
            auto* mod = (num_iterations > index.modulo)
                            ? b.create<BinaryExpression>(core::BinaryOp::kModulo, iteration(),
                                                         b.Expr(u32(index.modulo)))
                            : iteration();
            auto* div = (index.division != 1u) ? b.Div(mod, u32(index.division)) : mod;
            auto* decl = b.Decl(b.Let(*name, b.ty.u32(), div));
            stmts.Push(decl);
        }
        return stmts;
    }

    /// CalculateWorkgroupSize initializes the members #workgroup_size_const and
    /// #workgroup_size_expr with the linear workgroup size.
    /// @param attr the workgroup attribute applied to the entry point function
    void CalculateWorkgroupSize(const WorkgroupAttribute* attr) {
        bool is_signed = false;
        workgroup_size_const = 1u;
        workgroup_size_expr = nullptr;
        for (auto* expr : attr->Values()) {
            if (!expr) {
                continue;
            }
            if (auto* c = sem.GetVal(expr)->ConstantValue()) {
                workgroup_size_const *= c->ValueAs<AInt>();
                continue;
            }
            // Constant value could not be found. Build expression instead.
            workgroup_size_expr = [this, expr, size = workgroup_size_expr] {
                auto* e = ctx.Clone(expr);
                if (ctx.src->TypeOf(expr)->UnwrapRef()->Is<core::type::I32>()) {
                    e = b.Call<u32>(e);
                }
                return size ? b.Mul(size(), e) : e;
            };
        }
        if (workgroup_size_expr) {
            if (workgroup_size_const != 1) {
                // Fold workgroup_size_const in to workgroup_size_expr
                workgroup_size_expr = [this, is_signed, const_size = workgroup_size_const,
                                       expr_size = workgroup_size_expr] {
                    return is_signed ? b.Mul(expr_size(), i32(const_size))
                                     : b.Mul(expr_size(), u32(const_size));
                };
            }
            // Indicate that workgroup_size_expr should be used instead of the
            // constant.
            workgroup_size_const = 0;
        }
    }

    /// @returns true if a variable with store type `ty` can be efficiently zeroed
    /// by assignment of a value constructor without operands. If CanTriviallyZero() returns false,
    /// then the type needs to be initialized by decomposing the initialization into multiple
    /// sub-initializations.
    /// @param ty the type to inspect
    bool CanTriviallyZero(const core::type::Type* ty) {
        if (ty->Is<core::type::Atomic>()) {
            return false;
        }
        if (auto* str = ty->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                if (!CanTriviallyZero(member->Type())) {
                    return false;
                }
            }
        }
        if (ty->Is<core::type::Array>()) {
            return false;
        }
        // True for all other storable types
        return true;
    }
};

ZeroInitWorkgroupMemory::ZeroInitWorkgroupMemory() = default;

ZeroInitWorkgroupMemory::~ZeroInitWorkgroupMemory() = default;

Transform::ApplyResult ZeroInitWorkgroupMemory::Apply(const Program& src,
                                                      const DataMap&,
                                                      DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    for (auto* fn : src.AST().Functions()) {
        if (fn->PipelineStage() == PipelineStage::kCompute) {
            State{ctx}.Run(fn);
        }
    }

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::ast::transform
