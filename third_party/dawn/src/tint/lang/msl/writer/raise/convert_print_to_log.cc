// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/raise/convert_print_to_log.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/constant/string.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/string.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The entry point that is invoking this print builtin.
    core::ir::Function* entry_point = nullptr;

    /// The private variable that holds the invocation ID.
    core::ir::Var* invocation_id = nullptr;

    /// Process the module.
    void Process() {
        // Look for and replace calls to the print builtin.
        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                if (call->Func() == core::BuiltinFn::kPrint) {
                    Replace(call);
                }
            }
        }
    }

    /// Replace a call to a print builtin.
    /// @param call the print call to replace
    void Replace(core::ir::CoreBuiltinCall* call) {
        SetupGlobals();
        TINT_ASSERT(entry_point != nullptr);

        b.InsertBefore(call, [&] {
            auto* id = b.Load(invocation_id);
            auto* value = call->Args()[0];
            if (value->Type()->DeepestElement()->Is<core::type::Bool>()) {
                value = b.Convert(ty.MatchWidth(ty.i32(), value->Type()), value)->Result();
            }

            auto entry_point_name = ir.NameOf(entry_point).NameView();
            auto line = ir.SourceOf(call).range.begin.line;

            StringStream ss;
            Vector<core::ir::Value*, 5> args;
            args.Push(nullptr);
            switch (entry_point->Stage()) {
                case core::ir::Function::PipelineStage::kCompute:
                    ss << "[ comp " << entry_point_name << ":L" << line
                       << " global_invocation_id(%u, %u, %u) ] ";
                    args.Push(b.Swizzle<u32>(id, Vector{0u})->Result());
                    args.Push(b.Swizzle<u32>(id, Vector{1u})->Result());
                    args.Push(b.Swizzle<u32>(id, Vector{2u})->Result());
                    break;
                case core::ir::Function::PipelineStage::kFragment:
                    ss << "[ frag " << entry_point_name << ":L" << line
                       << " position(%f, %f, %f) ] ";
                    args.Push(b.Swizzle<f32>(id, Vector{0u})->Result());
                    args.Push(b.Swizzle<f32>(id, Vector{1u})->Result());
                    args.Push(b.Swizzle<f32>(id, Vector{2u})->Result());
                    break;
                case core::ir::Function::PipelineStage::kVertex:
                    ss << "[ vert " << entry_point_name << ":L" << line
                       << " instance=%u, vertex=%u ] ";
                    args.Push(b.Swizzle<u32>(id, Vector{0u})->Result());
                    args.Push(b.Swizzle<u32>(id, Vector{1u})->Result());
                    break;
                case core::ir::Function::PipelineStage::kUndefined:
                    TINT_UNREACHABLE();
            }
            args.Push(value);

            // Add the format specifier for the value being printed, and set the format argument.
            ss << "%" << TypeToFormatSpecifier(value->Type());
            args[0] = b.Constant(ir.constant_values.Get(ss.str()));

            b.Call<msl::ir::BuiltinCall>(ty.void_(), msl::BuiltinFn::kOsLog, std::move(args));
        });

        call->Destroy();
    }

    /// Return the format string specifier that corresponds to a type.
    std::string TypeToFormatSpecifier(const core::type::Type* type) {
        return tint::Switch(
            type,                                          //
            [&](const core::type::Bool*) { return "u"; },  //
            [&](const core::type::F16*) { return "f"; },   //
            [&](const core::type::F32*) { return "f"; },   //
            [&](const core::type::I32*) { return "i"; },   //
            [&](const core::type::U32*) { return "u"; },   //
            [&](const core::type::Vector* vec) {
                StringStream ss;
                ss << "v" << vec->Width();
                if (vec->Type()->Is<core::type::F16>()) {
                    ss << "hf";
                } else {
                    ss << "hl" << TypeToFormatSpecifier(vec->Type());
                }
                return ss.str();
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Set up the global structures and variables.
    void SetupGlobals() {
        if (invocation_id) {
            return;
        }

        // Create the invocation ID variable.
        for (auto func : ir.functions) {
            switch (func->Stage()) {
                case core::ir::Function::PipelineStage::kCompute:
                    TINT_ASSERT(entry_point == nullptr);
                    entry_point = func;
                    SetupComputeInvocationId(func);
                    break;
                case core::ir::Function::PipelineStage::kFragment:
                    TINT_ASSERT(entry_point == nullptr);
                    entry_point = func;
                    SetupFragmentInvocationId(func);
                    break;
                case core::ir::Function::PipelineStage::kVertex:
                    TINT_ASSERT(entry_point == nullptr);
                    entry_point = func;
                    SetupVertexInvocationId(func);
                    break;
                case core::ir::Function::PipelineStage::kUndefined:
                    break;
            }
        }
        TINT_ASSERT(invocation_id && entry_point);
    }

    /// Set up the invocation ID for a compute shader.
    /// @param ep the compute shader entry point
    void SetupComputeInvocationId(core::ir::Function* ep) {
        invocation_id =
            b.Var("tint_print_invocation_id", core::AddressSpace::kPrivate, ty.vec3<u32>());
        ir.root_block->Append(invocation_id);

        auto* id = GetBuiltinInput(ep, core::BuiltinValue::kGlobalInvocationId, ty.vec3<u32>());
        b.InsertBefore(ep->Block()->Front(), [&] {  //
            b.Store(invocation_id, id);
        });
    }

    /// Set up the invocation ID for a fragment shader.
    /// @param ep the fragment shader entry point
    void SetupFragmentInvocationId(core::ir::Function* ep) {
        invocation_id =
            b.Var("tint_print_invocation_id", core::AddressSpace::kPrivate, ty.vec3<f32>());
        ir.root_block->Append(invocation_id);

        auto* pos = GetBuiltinInput(ep, core::BuiltinValue::kPosition, ty.vec4<f32>());
        b.InsertBefore(ep->Block()->Front(), [&] {  //
            b.Store(invocation_id, b.Swizzle<vec3<f32>>(pos, Vector{0u, 1u, 2u}));
        });
    }

    /// Set up the invocation ID for a vertex shader.
    /// @param ep the vertex shader entry point
    void SetupVertexInvocationId(core::ir::Function* ep) {
        invocation_id =
            b.Var("tint_print_invocation_id", core::AddressSpace::kPrivate, ty.vec2<u32>());
        ir.root_block->Append(invocation_id);

        auto* instance = GetBuiltinInput(ep, core::BuiltinValue::kInstanceIndex, ty.u32());
        auto* vertex = GetBuiltinInput(ep, core::BuiltinValue::kVertexIndex, ty.u32());
        b.InsertBefore(ep->Block()->Front(), [&] {  //
            b.Store(invocation_id, b.Construct<vec2<u32>>(instance, vertex));
        });
    }

    /// Get or insert a shader builtin input value for an entry point.
    /// @param ep the entry point
    /// @param builtin the builtin attribute
    /// @param type the type of the builtin
    /// @returns the shader builtin input value
    core::ir::Value* GetBuiltinInput(core::ir::Function* ep,
                                     core::BuiltinValue builtin,
                                     const core::type::Type* type) {
        // Check every shader input to see if the builtin already exists.
        for (auto* param : ep->Params()) {
            if (auto* strct = param->Type()->As<core::type::Struct>()) {
                for (auto* member : strct->Members()) {
                    if (member->Attributes().builtin == builtin) {
                        return b.Access(type, param, u32(member->Index()))->Result(0);
                    }
                }
            } else {
                if (param->Builtin() == builtin) {
                    return param;
                }
            }
        }

        // We did not find the builtin, so invent one as a new parameter.
        auto* param = b.FunctionParam(ir.symbols.New().Name(), type);
        param->SetBuiltin(builtin);
        ep->AppendParam(param);
        return param;
    }
};

}  // namespace

Result<SuccessType> ConvertPrintToLog(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "msl.ConvertPrintToLog",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowDuplicateBindings,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
