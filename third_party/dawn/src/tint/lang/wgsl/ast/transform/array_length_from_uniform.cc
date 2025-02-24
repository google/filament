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

#include "src/tint/lang/wgsl/ast/transform/array_length_from_uniform.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/unary_op.h"
#include "src/tint/lang/wgsl/ast/expression.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/variable.h"
#include "src/tint/lang/wgsl/builtin_fn.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/array.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/expression.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/text_style.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::ArrayLengthFromUniform);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::ArrayLengthFromUniform::Config);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::ArrayLengthFromUniform::Result);

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::ast::transform {
namespace {

bool ShouldRun(const Program& program) {
    for (auto* fn : program.AST().Functions()) {
        if (auto* sem_fn = program.Sem().Get(fn)) {
            for (auto* builtin : sem_fn->DirectlyCalledBuiltins()) {
                if (builtin->Fn() == wgsl::BuiltinFn::kArrayLength) {
                    return true;
                }
            }
        }
    }
    return false;
}

}  // namespace

ArrayLengthFromUniform::ArrayLengthFromUniform() = default;
ArrayLengthFromUniform::~ArrayLengthFromUniform() = default;

/// PIMPL state for the transform
struct ArrayLengthFromUniform::State {
    /// Constructor
    /// @param program the source program
    /// @param in the input transform data
    /// @param out the output transform data
    State(const Program& program, const DataMap& in, DataMap& out)
        : src(program), outputs(out), cfg(in.Get<Config>()) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        if (cfg == nullptr) {
            b.Diagnostics().AddError(Source{}) << "missing transform data for "
                                               << tint::TypeInfo::Of<ArrayLengthFromUniform>().name;
            return resolver::Resolve(b);
        }

        if (cfg->bindpoint_to_size_index.empty() || !ShouldRun(src)) {
            return SkipTransform;
        }

        // Create the name of the array lengths uniform variable.
        array_lengths_var = b.Symbols().New("tint_array_lengths");

        // Replace all the arrayLength() calls.
        for (auto* fn : src.AST().Functions()) {
            if (auto* sem_fn = sem.Get(fn)) {
                for (auto* call : sem_fn->DirectCalls()) {
                    if (auto* target = call->Target()->As<sem::BuiltinFn>()) {
                        if (target->Fn() == wgsl::BuiltinFn::kArrayLength) {
                            ReplaceArrayLengthCall(call);
                        }
                    }
                }
            }
        }

        // Add the necessary array-length arguments to all the newly created array-length
        // parameters.
        while (!len_params_needing_args.IsEmpty()) {
            AddArrayLengthArguments(len_params_needing_args.Pop());
        }

        // Add the tint_array_lengths module-scope uniform variable.
        AddArrayLengthsUniformVar();

        outputs.Add<Result>(used_size_indices);

        ctx.Clone();
        return resolver::Resolve(b);
    }

  private:
    // Replaces the arrayLength() builtin call with an array-length expression passed via a uniform
    // buffer.
    void ReplaceArrayLengthCall(const sem::Call* call) {
        if (auto* replacement = ArrayLengthOf(call->Arguments()[0])) {
            ctx.Replace(call->Declaration(), replacement);
        }
    }

    /// @returns an AST expression that is equal to the arrayLength() of the runtime-sized array
    /// accessed by the pointer expression @p expr, or nullptr on error or if the array is not in
    /// the Config::bindpoint_to_size_index map.
    const ast::Expression* ArrayLengthOf(const sem::Expression* expr) {
        const ast::Expression* len = nullptr;
        while (expr) {
            expr = Switch(
                expr,  //
                [&](const sem::VariableUser* user) {
                    len = ArrayLengthOf(user->Variable());
                    return nullptr;
                },
                [&](const sem::MemberAccessorExpression* access) {
                    return access->Object();  // Follow the object
                },
                [&](const sem::Expression* e) {
                    return Switch(
                        e->Declaration(),  //
                        [&](const ast::UnaryOpExpression* unary) -> const sem::Expression* {
                            switch (unary->op) {
                                case core::UnaryOp::kAddressOf:
                                case core::UnaryOp::kIndirection:
                                    return sem.Get(unary->expr);  // Follow the object
                                default:
                                    TINT_ICE() << "unexpected unary op: " << unary->op;
                            }
                        },
                        TINT_ICE_ON_NO_MATCH);
                },
                TINT_ICE_ON_NO_MATCH);
        }
        return len;
    }

    /// @returns an AST expression that is equal to the arrayLength() of the runtime-sized array
    /// held by the module-scope variable or parameter @p var, or nullptr on error or if the array
    /// is not in the Config::bindpoint_to_size_index map.
    const ast::Expression* ArrayLengthOf(const sem::Variable* var) {
        return Switch(
            var,  //
            [&](const sem::GlobalVariable* global) { return ArrayLengthOf(global); },
            [&](const sem::Parameter* param) { return ArrayLengthOf(param); },
            TINT_ICE_ON_NO_MATCH);
    }

    /// @returns an AST expression that is equal to the arrayLength() of the runtime-sized array
    /// held by the module scope variable @p global, or nullptr on error or if the array is not in
    /// the Config::bindpoint_to_size_index map.
    const ast::Expression* ArrayLengthOf(const sem::GlobalVariable* global) {
        auto binding = global->Attributes().binding_point;
        TINT_ASSERT(binding);

        auto idx_it = cfg->bindpoint_to_size_index.find(*binding);
        if (idx_it == cfg->bindpoint_to_size_index.end()) {
            // If the bindpoint_to_size_index map does not contain an entry for the storage buffer,
            // then we preserve the arrayLength() call.
            return nullptr;
        }

        uint32_t size_index = idx_it->second;
        used_size_indices.insert(size_index);

        // Load the total storage buffer size from the UBO.
        uint32_t array_index = size_index / 4;
        auto* vec_expr = b.IndexAccessor(
            b.MemberAccessor(array_lengths_var, kArrayLengthsMemberName), u32(array_index));
        uint32_t vec_index = size_index % 4;
        auto* total_storage_buffer_size = b.IndexAccessor(vec_expr, u32(vec_index));

        // Calculate actual array length
        //                total_storage_buffer_size - array_offset
        // array_length = ----------------------------------------
        //                             array_stride
        const Expression* total_size = total_storage_buffer_size;
        if (DAWN_UNLIKELY(global->Type()->Is<core::type::Pointer>())) {
            TINT_ICE() << "storage buffer variable should not be a pointer. "
                          "These should have been removed by the SimplifyPointers transform";
        }
        auto* storage_buffer_type = global->Type()->UnwrapRef();
        const core::type::Array* array_type = nullptr;
        if (auto* str = storage_buffer_type->As<core::type::Struct>()) {
            // The variable is a struct, so subtract the byte offset of the
            // array member.
            auto* array_member_sem = str->Members().Back();
            array_type = array_member_sem->Type()->As<core::type::Array>();
            total_size = b.Sub(total_storage_buffer_size, u32(array_member_sem->Offset()));
        } else if (auto* arr = storage_buffer_type->As<core::type::Array>()) {
            array_type = arr;
        } else {
            TINT_ICE() << "expected form of arrayLength argument to be &array_var or "
                          "&struct_var.array_member";
        }
        return b.Div(total_size, u32(array_type->Stride()));
    }

    /// @returns an AST expression that is equal to the arrayLength() of the runtime-sized array
    /// held by the object pointed to by the pointer parameter @p param.
    const ast::Expression* ArrayLengthOf(const sem::Parameter* param) {
        // Pointer originates from a parameter.
        // Add a new array length parameter to the function, and use that.
        auto len_name = param_lengths.GetOrAdd(param, [&] {
            auto* fn = param->Owner()->As<sem::Function>();
            auto name = b.Symbols().New(param->Declaration()->name->symbol.Name() + "_length");
            auto* len_param = b.Param(name, b.ty.u32());
            ctx.InsertAfter(fn->Declaration()->params, param->Declaration(), len_param);
            len_params_needing_args.Add(param);
            return name;
        });
        return b.Expr(len_name);
    }

    /// Constructs the uniform buffer variable that will hold the array lengths.
    void AddArrayLengthsUniformVar() {
        // Calculate the highest index in the array lengths array
        uint32_t highest_index = 0;
        for (auto idx : used_size_indices) {
            if (idx > highest_index) {
                highest_index = idx;
            }
        }

        // Emit an array<vec4<u32>, N>, where N is 1/4 number of elements.
        // We do this because UBOs require an element stride that is 16-byte aligned.
        auto* buffer_size_struct =
            b.Structure(b.Symbols().New("TintArrayLengths"),
                        tint::Vector{
                            b.Member(kArrayLengthsMemberName,
                                     b.ty.array(b.ty.vec4<u32>(), u32((highest_index / 4) + 1))),
                        });
        b.GlobalVar(array_lengths_var, b.ty.Of(buffer_size_struct), core::AddressSpace::kUniform,
                    b.Group(AInt(cfg->ubo_binding.group)),
                    b.Binding(AInt(cfg->ubo_binding.binding)));
    }

    /// Adds an additional array-length argument to all the calls to the function that owns the
    /// pointer parameter @p param. This may add new entries to #len_params_needing_args.
    void AddArrayLengthArguments(const sem::Parameter* param) {
        auto* fn = param->Owner()->As<sem::Function>();
        for (auto* call : fn->CallSites()) {
            auto* arg = call->Arguments()[param->Index()];
            if (auto* len = ArrayLengthOf(arg); len) {
                ctx.InsertAfter(call->Declaration()->args, arg->Declaration(), len);
            } else {
                // Callee expects an array length, but there's no binding for it.
                // Call arrayLength() at the call-site.
                len = b.Call(wgsl::BuiltinFn::kArrayLength, ctx.Clone(arg->Declaration()));
                ctx.InsertAfter(call->Declaration()->args, arg->Declaration(), len);
            }
        }
    }

    /// Name of the array-lengths struct member that holds all the array lengths.
    static constexpr std::string_view kArrayLengthsMemberName = "array_lengths";

    /// The source program
    const Program& src;
    /// The transform outputs
    DataMap& outputs;
    /// The transform config
    const Config* const cfg;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};
    /// Alias to src.Sem()
    const sem::Info& sem = src.Sem();
    /// Name of the uniform buffer variable that holds the array lengths
    Symbol array_lengths_var;
    /// A map of pointer-parameter to the name of the new array-length parameter.
    Hashmap<const sem::Parameter*, Symbol, 8> param_lengths;
    /// Indices into the uniform buffer array indices that are statically used.
    std::unordered_set<uint32_t> used_size_indices;
    /// A vector of array-length parameters which need corresponding array-length arguments for all
    /// callsites.
    UniqueVector<const sem::Parameter*, 8> len_params_needing_args;
};

Transform::ApplyResult ArrayLengthFromUniform::Apply(const Program& src,
                                                     const DataMap& inputs,
                                                     DataMap& outputs) const {
    return State{src, inputs, outputs}.Run();
}

ArrayLengthFromUniform::Config::Config() = default;
ArrayLengthFromUniform::Config::Config(BindingPoint ubo_bp) : ubo_binding(ubo_bp) {}
ArrayLengthFromUniform::Config::Config(const Config&) = default;
ArrayLengthFromUniform::Config& ArrayLengthFromUniform::Config::operator=(const Config&) = default;
ArrayLengthFromUniform::Config::~Config() = default;

ArrayLengthFromUniform::Result::Result(std::unordered_set<uint32_t> used_size_indices_in)
    : used_size_indices(std::move(used_size_indices_in)) {}
ArrayLengthFromUniform::Result::Result(const Result&) = default;
ArrayLengthFromUniform::Result::~Result() = default;

}  // namespace tint::ast::transform
