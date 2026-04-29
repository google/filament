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

#include "src/tint/lang/spirv/writer/raise/builtin_polyfill.h"

#include <utility>

#include "spirv/unified1/spirv.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/resource_table.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/spirv/ir/binary.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/type/literal.h"
#include "src/tint/lang/spirv/type/sampled_image.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/internal_limits.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer::raise {

namespace {

const spirv::type::Image* ImageFromTexture(core::type::Manager& ty,
                                           const core::type::Texture* tex_ty) {
    auto dim = type::Dim::kD1;
    auto depth = type::Depth::kNotDepth;
    auto arrayed = type::Arrayed::kNonArrayed;
    auto ms = type::Multisampled::kSingleSampled;
    auto sampled = type::Sampled::kSamplingCompatible;
    auto fmt = core::TexelFormat::kUndefined;
    auto access = core::Access::kReadWrite;
    const core::type::Type* sample_ty = ty.f32();

    switch (tex_ty->Dim()) {
        case core::type::TextureDimension::k1d:
            dim = type::Dim::kD1;
            break;
        case core::type::TextureDimension::k2d:
            dim = type::Dim::kD2;
            break;
        case core::type::TextureDimension::k2dArray:
            dim = type::Dim::kD2;
            arrayed = type::Arrayed::kArrayed;
            break;
        case core::type::TextureDimension::k3d:
            dim = type::Dim::kD3;
            break;
        case core::type::TextureDimension::kCube:
            dim = type::Dim::kCube;
            break;
        case core::type::TextureDimension::kCubeArray:
            dim = type::Dim::kCube;
            arrayed = type::Arrayed::kArrayed;
            break;
        default:
            TINT_ICE() << "Invalid texture dimension: " << tex_ty->Dim();
    }

    tint::Switch(
        tex_ty,                                 //
        [&](const core::type::DepthTexture*) {  //
            depth = type::Depth::kDepth;
        },
        [&](const core::type::DepthMultisampledTexture*) {
            depth = type::Depth::kDepth;
            ms = type::Multisampled::kMultisampled;
        },
        [&](const core::type::MultisampledTexture* mt) {
            ms = type::Multisampled::kMultisampled;
            sample_ty = mt->Type();
        },
        [&](const core::type::SampledTexture* st) {
            sampled = type::Sampled::kSamplingCompatible;
            sample_ty = st->Type();
        },
        [&](const core::type::StorageTexture* st) {
            sampled = type::Sampled::kReadWriteOpCompatible;
            fmt = st->TexelFormat();
            sample_ty = st->Type();
            access = st->Access();
        },
        [&](const core::type::TexelBuffer* tb) {
            sampled = type::Sampled::kReadWriteOpCompatible;
            fmt = tb->TexelFormat();
            sample_ty = tb->Type();
            access = tb->Access();
            dim = type::Dim::kBuffer;
        },
        [&](const core::type::InputAttachment* ia) {
            dim = type::Dim::kSubpassData;
            sampled = type::Sampled::kReadWriteOpCompatible;
            sample_ty = ia->Type();
        },
        TINT_ICE_ON_NO_MATCH);

    return ty.Get<type::Image>(sample_ty, dim, depth, arrayed, ms, sampled, fmt, access);
}

/// Returns a replacement type if type replacement is necessary.
/// @param ty the type manager
/// @param type the type to replace
/// @returns the replacement type if replacement needs to happen
const core::type::Type* ReplacementType(core::type::Manager& ty, const core::type::Type* type) {
    return Switch(
        type,
        [&](const core::type::Pointer* ptr) -> const core::type::Type* {
            if (auto* replacement = ReplacementType(ty, ptr->StoreType())) {
                return ty.ptr(ptr->AddressSpace(), replacement, ptr->Access());
            }
            return nullptr;
        },
        [&](const core::type::BindingArray* arr) -> const core::type::Type* {
            if (auto* replacement = ReplacementType(ty, arr->ElemType())) {
                return ty.binding_array(replacement,
                                        arr->Count()->As<core::type::ConstantArrayCount>()->value);
            }
            return nullptr;
        },
        [&](const core::type::ResourceTable* rb) -> const core::type::Type* {
            if (auto* replacement = ReplacementType(ty, rb->GetBindingType())) {
                return ty.Get<core::type::ResourceTable>(replacement);
            }
            return nullptr;
        },
        [&](const core::type::Texture* tex) { return ImageFromTexture(ty, tex); },
        [&](Default) { return nullptr; });
}

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    PolyfillConfig config;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Find the builtins that need replacing.
        Vector<core::ir::CoreBuiltinCall*, 4> worklist;
        Vector<core::ir::Construct*, 4> subgroup_matrix_constructors;

        // Replace types for function parameters if necessary
        for (auto fn : ir.functions) {
            for (auto* param : fn->Params()) {
                if (auto* replacement = ReplacementType(ty, param->Type())) {
                    param->SetType(replacement);
                }
            }
        }

        for (auto* inst : ir.Instructions()) {
            // Replace types for instruction results if necessary
            for (auto* result : inst->Results()) {
                if (auto* replacement = ReplacementType(ty, result->Type())) {
                    result->SetType(replacement);
                }
            }

            if (auto* builtin = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (builtin->Func()) {
                    case core::BuiltinFn::kArrayLength:
                    case core::BuiltinFn::kAtomicAdd:
                    case core::BuiltinFn::kAtomicAnd:
                    case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    case core::BuiltinFn::kAtomicStoreMax:
                    case core::BuiltinFn::kAtomicStoreMin:
                    case core::BuiltinFn::kAtomicExchange:
                    case core::BuiltinFn::kAtomicLoad:
                    case core::BuiltinFn::kAtomicMax:
                    case core::BuiltinFn::kAtomicMin:
                    case core::BuiltinFn::kAtomicOr:
                    case core::BuiltinFn::kAtomicStore:
                    case core::BuiltinFn::kAtomicSub:
                    case core::BuiltinFn::kAtomicXor:
                    case core::BuiltinFn::kDot:
                    case core::BuiltinFn::kDot4I8Packed:
                    case core::BuiltinFn::kDot4U8Packed:
                    case core::BuiltinFn::kQuadBroadcast:
                    case core::BuiltinFn::kSelect:
                    case core::BuiltinFn::kSubgroupBroadcast:
                    case core::BuiltinFn::kSubgroupShuffle:
                    case core::BuiltinFn::kSubgroupShuffleDown:
                    case core::BuiltinFn::kSubgroupShuffleUp:
                    case core::BuiltinFn::kSubgroupShuffleXor:
                    case core::BuiltinFn::kTextureDimensions:
                    case core::BuiltinFn::kTextureGather:
                    case core::BuiltinFn::kTextureGatherCompare:
                    case core::BuiltinFn::kTextureLoad:
                    case core::BuiltinFn::kTextureNumLayers:
                    case core::BuiltinFn::kTextureNumLevels:
                    case core::BuiltinFn::kTextureNumSamples:
                    case core::BuiltinFn::kTextureSample:
                    case core::BuiltinFn::kTextureSampleBias:
                    case core::BuiltinFn::kTextureSampleCompare:
                    case core::BuiltinFn::kTextureSampleCompareLevel:
                    case core::BuiltinFn::kTextureSampleGrad:
                    case core::BuiltinFn::kTextureSampleLevel:
                    case core::BuiltinFn::kTextureStore:
                    case core::BuiltinFn::kInputAttachmentLoad:
                    case core::BuiltinFn::kSubgroupMatrixLoad:
                    case core::BuiltinFn::kSubgroupMatrixStore:
                    case core::BuiltinFn::kSubgroupMatrixMultiply:
                    case core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate:
                    case core::BuiltinFn::kSubgroupMatrixScalarAdd:
                    case core::BuiltinFn::kSubgroupMatrixScalarSubtract:
                    case core::BuiltinFn::kSubgroupMatrixScalarMultiply:
                        worklist.Push(builtin);
                        break;
                    case core::BuiltinFn::kQuantizeToF16:
                        if (builtin->Result()->Type()->Is<core::type::Vector>()) {
                            worklist.Push(builtin);
                        }
                        break;
                    default:
                        break;
                }
            }
            if (auto* construct = inst->As<core::ir::Construct>()) {
                if (auto* sm = construct->Result()->Type()->As<core::type::SubgroupMatrix>()) {
                    if (sm->Type()->IsAnyOf<core::type::I8, core::type::U8>() &&
                        construct->Args().size() > 0) {
                        subgroup_matrix_constructors.Push(construct);
                    }
                }
            }
        }

        // Replace the builtins that we found.
        for (auto* builtin : worklist) {
            switch (builtin->Func()) {
                case core::BuiltinFn::kArrayLength:
                    ArrayLength(builtin);
                    break;
                case core::BuiltinFn::kAtomicAdd:
                case core::BuiltinFn::kAtomicAnd:
                case core::BuiltinFn::kAtomicCompareExchangeWeak:
                case core::BuiltinFn::kAtomicExchange:
                case core::BuiltinFn::kAtomicLoad:
                case core::BuiltinFn::kAtomicMax:
                case core::BuiltinFn::kAtomicMin:
                case core::BuiltinFn::kAtomicOr:
                case core::BuiltinFn::kAtomicStore:
                case core::BuiltinFn::kAtomicSub:
                case core::BuiltinFn::kAtomicXor:
                case core::BuiltinFn::kAtomicStoreMax:
                case core::BuiltinFn::kAtomicStoreMin:
                    Atomic(builtin);
                    break;
                case core::BuiltinFn::kDot:
                    Dot(builtin);
                    break;
                case core::BuiltinFn::kDot4I8Packed:
                case core::BuiltinFn::kDot4U8Packed:
                    DotPacked4x8(builtin);
                    break;
                case core::BuiltinFn::kQuadBroadcast:
                    QuadBroadcast(builtin);
                    break;
                case core::BuiltinFn::kSelect:
                    Select(builtin);
                    break;
                case core::BuiltinFn::kSubgroupBroadcast:
                    SubgroupBroadcast(builtin);
                    break;
                case core::BuiltinFn::kSubgroupShuffle:
                case core::BuiltinFn::kSubgroupShuffleDown:
                case core::BuiltinFn::kSubgroupShuffleUp:
                case core::BuiltinFn::kSubgroupShuffleXor:
                    SubgroupShuffle(builtin, config.subgroup_shuffle_clamped);
                    break;
                case core::BuiltinFn::kTextureDimensions:
                    TextureDimensions(builtin);
                    break;
                case core::BuiltinFn::kTextureGather:
                case core::BuiltinFn::kTextureGatherCompare:
                    TextureGather(builtin);
                    break;
                case core::BuiltinFn::kTextureLoad:
                    TextureLoad(builtin);
                    break;
                case core::BuiltinFn::kTextureNumLayers:
                    TextureNumLayers(builtin);
                    break;
                case core::BuiltinFn::kTextureNumLevels:
                    TextureNumLevels(builtin);
                    break;
                case core::BuiltinFn::kTextureNumSamples:
                    TextureNumSamples(builtin);
                    break;
                case core::BuiltinFn::kTextureSample:
                case core::BuiltinFn::kTextureSampleBias:
                case core::BuiltinFn::kTextureSampleCompare:
                case core::BuiltinFn::kTextureSampleCompareLevel:
                case core::BuiltinFn::kTextureSampleGrad:
                case core::BuiltinFn::kTextureSampleLevel:
                    TextureSample(builtin);
                    break;
                case core::BuiltinFn::kTextureStore:
                    TextureStore(builtin);
                    break;
                case core::BuiltinFn::kQuantizeToF16:
                    QuantizeToF16Vec(builtin);
                    break;
                case core::BuiltinFn::kInputAttachmentLoad:
                    InputAttachmentLoad(builtin);
                    break;
                case core::BuiltinFn::kSubgroupMatrixLoad:
                    SubgroupMatrixLoad(builtin);
                    break;
                case core::BuiltinFn::kSubgroupMatrixStore:
                    SubgroupMatrixStore(builtin);
                    break;
                case core::BuiltinFn::kSubgroupMatrixMultiply:
                    SubgroupMatrixMultiply(builtin);
                    break;
                case core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate:
                    SubgroupMatrixMultiplyAccumulate(builtin);
                    break;
                case core::BuiltinFn::kSubgroupMatrixScalarAdd:
                    SubgroupMatrixScalar(builtin, core::BinaryOp::kAdd);
                    break;
                case core::BuiltinFn::kSubgroupMatrixScalarSubtract:
                    SubgroupMatrixScalar(builtin, core::BinaryOp::kSubtract);
                    break;
                case core::BuiltinFn::kSubgroupMatrixScalarMultiply:
                    SubgroupMatrixScalar(builtin, core::BinaryOp::kMultiply);
                    break;
                default:
                    break;
            }
        }

        // Replace non-zero subgroup matrix constructors that use 8-bit component types.
        // SPIR-V requires that the value passed to OpCompositeConstruct is an 8-bit value.
        for (auto* construct : subgroup_matrix_constructors) {
            auto* sm_ty = construct->Result()->Type()->As<core::type::SubgroupMatrix>();
            TINT_IR_ASSERT(ir, construct->Args().size() == 1u);
            TINT_IR_ASSERT(ir, sm_ty);
            auto* value = construct->Args()[0];
            b.InsertBefore(construct, [&] {
                if (sm_ty->Type()->Is<core::type::I8>()) {
                    value = b.CallExplicit<spirv::ir::BuiltinCall>(
                                 ty.i8(), spirv::BuiltinFn::kSConvert, Vector{ty.i8()},
                                 b.Clamp(value, -128_i, 127_i))
                                ->Result();
                } else if (sm_ty->Type()->Is<core::type::U8>()) {
                    value = b.CallExplicit<spirv::ir::BuiltinCall>(
                                 ty.u8(), spirv::BuiltinFn::kUConvert, Vector{ty.u8()},
                                 b.Clamp(value, 0_u, 255_u))
                                ->Result();
                }
            });
            construct->SetArg(0, value);
        }
    }

    /// Create a literal operand.
    /// @param value the literal value
    /// @returns the literal operand
    core::ir::Value* Literal(u32 value) {
        return b.Constant(
            ir.constant_values.Get<core::constant::Scalar<u32>>(ty.Get<type::Literal>(), value));
    }

    /// Handle an `arrayLength()` builtin.
    /// @param builtin the builtin call instruction
    void ArrayLength(core::ir::CoreBuiltinCall* builtin) {
        // Strip away any let instructions to get to the original struct member access instruction.
        auto* ptr = builtin->Args()[0]->As<core::ir::InstructionResult>();
        while (auto* let = tint::As<core::ir::Let>(ptr->Instruction())) {
            ptr = let->Value()->As<core::ir::InstructionResult>();
        }
        TINT_IR_ASSERT(ir, ptr);

        auto* access = ptr->Instruction()->As<core::ir::Access>();
        TINT_IR_ASSERT(ir, access);
        TINT_IR_ASSERT(ir, access->Indices().size() == 1u);
        TINT_IR_ASSERT(ir, access->Object()->Type()->UnwrapPtr()->Is<core::type::Struct>());
        auto* const_idx = access->Indices()[0]->As<core::ir::Constant>();

        // Replace the builtin call with a call to the spirv.array_length intrinsic.
        auto* call = b.CallWithResult<spirv::ir::BuiltinCall>(
            builtin->DetachResult(), spirv::BuiltinFn::kArrayLength,
            Vector{access->Object(), Literal(u32(const_idx->Value()->ValueAs<uint32_t>()))});
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Handle an atomic*() builtin.
    /// @param builtin the builtin call instruction
    void Atomic(core::ir::CoreBuiltinCall* builtin) {
        auto* result_ty = builtin->Result()->Type();

        auto* pointer = builtin->Args()[0];
        auto* memory = [&]() -> core::ir::Value* {
            switch (pointer->Type()->As<core::type::Pointer>()->AddressSpace()) {
                case core::AddressSpace::kWorkgroup:
                    return b.Constant(u32(SpvScopeWorkgroup));
                case core::AddressSpace::kStorage:
                    return b.Constant(u32(SpvScopeDevice));
                default:
                    TINT_IR_UNREACHABLE(ir) << "unhandled atomic address space";
            }
        }();
        auto* memory_semantics = b.Constant(u32(SpvMemorySemanticsMaskNone));

        // Helper to build the builtin call with the common operands.
        auto build = [&](enum spirv::BuiltinFn builtin_fn) {
            return b.CallWithResult<spirv::ir::BuiltinCall>(builtin->DetachResult(), builtin_fn,
                                                            pointer, memory, memory_semantics);
        };

        // Create the replacement call instruction.
        core::ir::Call* call = nullptr;
        switch (builtin->Func()) {
            case core::BuiltinFn::kAtomicAdd:
                call = build(spirv::BuiltinFn::kAtomicIAdd);
                call->AppendArg(builtin->Args()[1]);
                break;
            case core::BuiltinFn::kAtomicAnd:
                call = build(spirv::BuiltinFn::kAtomicAnd);
                call->AppendArg(builtin->Args()[1]);
                break;
            case core::BuiltinFn::kAtomicCompareExchangeWeak: {
                auto* cmp = builtin->Args()[1];
                auto* value = builtin->Args()[2];
                auto* int_ty = value->Type();
                call =
                    b.Call<spirv::ir::BuiltinCall>(int_ty, spirv::BuiltinFn::kAtomicCompareExchange,
                                                   pointer, memory, memory_semantics);
                call->AppendArg(memory_semantics);
                call->AppendArg(value);
                call->AppendArg(cmp);
                call->InsertBefore(builtin);

                // Compare the original value to the comparator to see if an exchange happened.
                auto* original = call->Result();
                auto* compare = b.Equal(original, cmp);
                compare->InsertBefore(builtin);

                // Construct the atomicCompareExchange result structure.
                call = b.ConstructWithResult(builtin->DetachResult(),
                                             Vector{original, compare->Result()});
                break;
            }
            case core::BuiltinFn::kAtomicExchange:
                call = build(spirv::BuiltinFn::kAtomicExchange);
                call->AppendArg(builtin->Args()[1]);
                break;
            case core::BuiltinFn::kAtomicLoad:
                call = build(spirv::BuiltinFn::kAtomicLoad);
                break;
            case core::BuiltinFn::kAtomicOr:
                call = build(spirv::BuiltinFn::kAtomicOr);
                call->AppendArg(builtin->Args()[1]);
                break;
            case core::BuiltinFn::kAtomicMax:
                if (result_ty->IsSignedIntegerScalar()) {
                    call = build(spirv::BuiltinFn::kAtomicSMax);
                } else {
                    call = build(spirv::BuiltinFn::kAtomicUMax);
                }
                call->AppendArg(builtin->Args()[1]);
                break;
            case core::BuiltinFn::kAtomicMin:
                if (result_ty->IsSignedIntegerScalar()) {
                    call = build(spirv::BuiltinFn::kAtomicSMin);
                } else {
                    call = build(spirv::BuiltinFn::kAtomicUMin);
                }
                call->AppendArg(builtin->Args()[1]);
                break;
            case core::BuiltinFn::kAtomicStoreMax: {
                call = build(spirv::BuiltinFn::kAtomicUMax);
                call->AppendArg(builtin->Args()[1]);
                call->Result()->SetType(ty.u64());
                break;
            }
            case core::BuiltinFn::kAtomicStoreMin:
                call = build(spirv::BuiltinFn::kAtomicUMin);
                call->AppendArg(builtin->Args()[1]);
                call->Result()->SetType(ty.u64());
                break;
            case core::BuiltinFn::kAtomicStore:
                call = build(spirv::BuiltinFn::kAtomicStore);
                call->AppendArg(builtin->Args()[1]);
                break;
            case core::BuiltinFn::kAtomicSub:
                call = build(spirv::BuiltinFn::kAtomicISub);
                call->AppendArg(builtin->Args()[1]);
                break;
            case core::BuiltinFn::kAtomicXor:
                call = build(spirv::BuiltinFn::kAtomicXor);
                call->AppendArg(builtin->Args()[1]);
                break;
            default:
                TINT_IR_UNREACHABLE(ir) << "unhandled atomic builtin";
        }

        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Handle a `dot()` builtin.
    /// @param builtin the builtin call instruction
    void Dot(core::ir::CoreBuiltinCall* builtin) {
        // OpDot only supports floating point operands, so we need to polyfill the integer case.
        // TODO(crbug.com/tint/1267): If SPV_KHR_integer_dot_product is supported, use that instead.
        if (builtin->Result()->Type()->IsIntegerScalar()) {
            core::ir::Instruction* sum = nullptr;

            auto* v1 = builtin->Args()[0];
            auto* v2 = builtin->Args()[1];
            auto* vec = v1->Type()->As<core::type::Vector>();
            auto* elty = vec->Type();
            for (uint32_t i = 0; i < vec->Width(); i++) {
                b.InsertBefore(builtin, [&] {
                    auto* e1 = b.Access(elty, v1, u32(i));
                    auto* e2 = b.Access(elty, v2, u32(i));
                    auto* mul = b.Multiply(e1, e2);
                    if (sum) {
                        sum = b.Add(sum, mul);
                    } else {
                        sum = mul;
                    }
                });
            }
            sum->SetResult(builtin->DetachResult());
            builtin->Destroy();
            return;
        }

        // Replace the builtin call with a call to the spirv.dot intrinsic.
        auto args = Vector<core::ir::Value*, 4>(builtin->Args());
        auto* call = b.CallWithResult<spirv::ir::BuiltinCall>(
            builtin->DetachResult(), spirv::BuiltinFn::kDot, std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Handle a `dot4{I,U}8Packed()` builtin.
    /// @param builtin the builtin call instruction
    void DotPacked4x8(core::ir::CoreBuiltinCall* builtin) {
        // Replace the builtin call with a call to the spirv.{s,u}dot intrinsic.
        auto is_signed = builtin->Func() == core::BuiltinFn::kDot4I8Packed;
        auto inst = is_signed ? spirv::BuiltinFn::kSDot : spirv::BuiltinFn::kUDot;

        auto args = Vector<core::ir::Value*, 3>(builtin->Args());
        args.Push(Literal(u32(SpvPackedVectorFormatPackedVectorFormat4x8Bit)));

        auto* call = b.CallWithResult<spirv::ir::BuiltinCall>(builtin->DetachResult(), inst,
                                                              std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Handle a `select()` builtin.
    /// @param builtin the builtin call instruction
    void Select(core::ir::CoreBuiltinCall* builtin) {
        // Argument order is different in SPIR-V: (condition, true_operand, false_operand).
        Vector<core::ir::Value*, 4> args = {
            builtin->Args()[2],
            builtin->Args()[1],
            builtin->Args()[0],
        };

        if (config.version < SpvVersion::kSpv14) {
            // If the condition is scalar and the objects are vectors, we need to splat the
            // condition into a vector of the same size.
            auto* vec = builtin->Result()->Type()->As<core::type::Vector>();
            if (vec && args[0]->Type()->Is<core::type::Scalar>()) {
                Vector<core::ir::Value*, 4> elements;
                elements.Resize(vec->Width(), args[0]);

                auto* construct =
                    b.Construct(ty.vec(ty.bool_(), vec->Width()), std::move(elements));
                construct->InsertBefore(builtin);
                args[0] = construct->Result();
            }
        }

        // Replace the builtin call with a call to the spirv.select intrinsic.
        auto* call = b.CallWithResult<spirv::ir::BuiltinCall>(
            builtin->DetachResult(), spirv::BuiltinFn::kSelect, std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// ImageOperands represents the optional image operands for an image instruction.
    struct ImageOperands {
        /// Bias
        core::ir::Value* bias = nullptr;
        /// Lod
        core::ir::Value* lod = nullptr;
        /// Grad (dx)
        core::ir::Value* ddx = nullptr;
        /// Grad (dy)
        core::ir::Value* ddy = nullptr;
        /// ConstOffset
        core::ir::Value* offset = nullptr;
        /// Sample
        core::ir::Value* sample = nullptr;
    };

    /// Append optional image operands to an image intrinsic argument list.
    /// @param operands the operands
    /// @param args the argument list
    /// @param insertion_point the insertion point for new instructions
    /// @param requires_float_lod true if the lod needs to be a floating point value
    void AppendImageOperands(ImageOperands& operands,
                             Vector<core::ir::Value*, 8>& args,
                             core::ir::CoreBuiltinCall* insertion_point,
                             bool requires_float_lod) {
        // Add a placeholder argument for the image operand mask, which we will fill in when we have
        // processed the image operands.
        uint32_t image_operand_mask = 0u;
        size_t mask_idx = args.Length();
        args.Push(nullptr);

        // Append the NonPrivateTexel flag to Read/Write storage textures when we load/store them.
        if (config.use_vulkan_memory_model) {
            if (insertion_point->Func() == core::BuiltinFn::kTextureLoad ||
                insertion_point->Func() == core::BuiltinFn::kTextureStore) {
                if (auto* st = insertion_point->Args()[0]->Type()->As<spirv::type::Image>()) {
                    if (st->GetTexelFormat() != core::TexelFormat::kUndefined &&
                        st->GetAccess() == core::Access::kReadWrite) {
                        image_operand_mask |= SpvImageOperandsNonPrivateTexelMask;
                    }
                }
            }
        }

        // Add each of the optional image operands if used, updating the image operand mask.
        if (operands.bias) {
            image_operand_mask |= SpvImageOperandsBiasMask;
            args.Push(operands.bias);
        }
        if (operands.lod) {
            image_operand_mask |= SpvImageOperandsLodMask;
            if (requires_float_lod && operands.lod->Type()->IsIntegerScalar()) {
                auto* convert = b.Convert(ty.f32(), operands.lod);
                convert->InsertBefore(insertion_point);
                operands.lod = convert->Result();
            }
            args.Push(operands.lod);
        }
        if (operands.ddx) {
            image_operand_mask |= SpvImageOperandsGradMask;
            args.Push(operands.ddx);
            args.Push(operands.ddy);
        }
        if (operands.offset) {
            image_operand_mask |= SpvImageOperandsConstOffsetMask;
            args.Push(operands.offset);
        }
        if (operands.sample) {
            image_operand_mask |= SpvImageOperandsSampleMask;
            args.Push(operands.sample);
        }

        // Replace the image operand mask with the final mask value, as a literal operand.
        args[mask_idx] = Literal(u32(image_operand_mask));
    }

    /// Append an array index to a coordinate vector.
    /// @param coords the coordinate vector
    /// @param array_idx the array index
    /// @param insertion_point the insertion point for new instructions
    /// @returns the modified coordinate vector
    core::ir::Value* AppendArrayIndex(core::ir::Value* coords,
                                      core::ir::Value* array_idx,
                                      core::ir::Instruction* insertion_point) {
        auto* vec = coords->Type()->As<core::type::Vector>();
        auto* element_ty = vec->Type();

        // Convert the index to match the coordinate type if needed.
        if (array_idx->Type() != element_ty) {
            auto* array_idx_converted = b.Convert(element_ty, array_idx);
            array_idx_converted->InsertBefore(insertion_point);
            array_idx = array_idx_converted->Result();
        }

        // Construct a new coordinate vector.
        auto num_coords = vec->Width();
        auto* coord_ty = ty.vec(element_ty, num_coords + 1);
        auto* construct = b.Construct(coord_ty, Vector{coords, array_idx});
        construct->InsertBefore(insertion_point);
        return construct->Result();
    }

    /// Handle a textureSample*() builtin.
    /// @param builtin the builtin call instruction
    void TextureSample(core::ir::CoreBuiltinCall* builtin) {
        // Helper to get the next argument from the call, or nullptr if there are no more arguments.
        uint32_t arg_idx = 0;
        auto next_arg = [&]() {
            return arg_idx < builtin->Args().size() ? builtin->Args()[arg_idx++] : nullptr;
        };

        auto* texture = next_arg();
        auto* sampler = next_arg();
        auto* coords = next_arg();
        auto* texture_ty = texture->Type()->As<spirv::type::Image>();

        const bool is_depth_3d_cube_array = texture_ty->GetArrayed() == type::Arrayed::kArrayed &&
                                            texture_ty->GetDepth() == type::Depth::kDepth &&
                                            texture_ty->GetDim() == type::Dim::kCube;
        const bool polyfill_depth_cube_array =
            config.texture_sample_compare_depth_cube_array && is_depth_3d_cube_array &&
            (builtin->Func() == core::BuiltinFn::kTextureSampleCompare ||
             builtin->Func() == core::BuiltinFn::kTextureSampleCompareLevel);

        // Use OpSampledImage to create an OpTypeSampledImage object.
        auto* sampled_image = b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.Get<type::SampledImage>(texture_ty), spirv::BuiltinFn::kOpSampledImage,
            Vector{texture_ty}, Vector{texture, sampler});
        sampled_image->InsertBefore(builtin);

        // Append the array index to the coordinates if provided.
        auto* array_idx =
            texture_ty->GetArrayed() == type::Arrayed::kArrayed ? next_arg() : nullptr;
        if (array_idx) {
            coords = AppendArrayIndex(coords, array_idx, builtin);
        }

        // Determine which SPIR-V function to use and which optional image operands are needed.
        enum spirv::BuiltinFn function = BuiltinFn::kNone;
        core::ir::Value* depth = nullptr;
        ImageOperands operands;
        switch (builtin->Func()) {
            case core::BuiltinFn::kTextureSample:
                function = spirv::BuiltinFn::kImageSampleImplicitLod;
                operands.offset = next_arg();
                break;
            case core::BuiltinFn::kTextureSampleBias:
                function = spirv::BuiltinFn::kImageSampleImplicitLod;
                operands.bias = next_arg();
                operands.offset = next_arg();
                break;
            case core::BuiltinFn::kTextureSampleCompare:
                function = polyfill_depth_cube_array
                               ? spirv::BuiltinFn::kImageDrefGather
                               : spirv::BuiltinFn::kImageSampleDrefImplicitLod;
                depth = next_arg();
                operands.offset = next_arg();
                break;
            case core::BuiltinFn::kTextureSampleCompareLevel:
                function = polyfill_depth_cube_array
                               ? spirv::BuiltinFn::kImageDrefGather
                               : spirv::BuiltinFn::kImageSampleDrefExplicitLod;
                depth = next_arg();
                if (!polyfill_depth_cube_array) {
                    operands.lod = b.Constant(0_f);
                }
                operands.offset = next_arg();
                break;
            case core::BuiltinFn::kTextureSampleGrad:
                function = spirv::BuiltinFn::kImageSampleExplicitLod;
                operands.ddx = next_arg();
                operands.ddy = next_arg();
                operands.offset = next_arg();
                break;
            case core::BuiltinFn::kTextureSampleLevel:
                function = spirv::BuiltinFn::kImageSampleExplicitLod;
                operands.lod = next_arg();
                operands.offset = next_arg();
                break;
            default:
                TINT_IR_UNREACHABLE(ir) << "unhandled texture sample builtin";
        }

        // Start building the argument list for the function.
        // The first two operands are always the sampled image and then the coordinates, followed by
        // the depth reference if used.
        Vector<core::ir::Value*, 8> function_args;
        function_args.Push(sampled_image->Result());
        function_args.Push(coords);
        if (depth) {
            function_args.Push(depth);
        }

        // Add the optional image operands, if any.
        AppendImageOperands(operands, function_args, builtin, /* requires_float_lod */ true);

        // Call the function.
        // If this is a depth comparison, the result is always f32, otherwise vec4f.
        auto* result_ty = (depth && !polyfill_depth_cube_array)
                              ? static_cast<const core::type::Type*>(ty.f32())
                              : ty.vec4f();

        core::ir::Instruction* result =
            b.Call<spirv::ir::BuiltinCall>(result_ty, function, std::move(function_args));
        result->InsertBefore(builtin);

        // If this is not a depth comparison but we are sampling a depth texture, extract the first
        // component to get the scalar f32 that SPIR-V expects.
        if (!depth && texture_ty->GetDepth() == type::Depth::kDepth) {
            result = b.Access(ty.f32(), result, 0_u);
            result->InsertBefore(builtin);
        }

        if (polyfill_depth_cube_array) {
            core::ir::Instruction* close_to_pcf_result = nullptr;
            b.InsertAfter(result, [&] {
                // This is an imperfect polyfill for builtin intrinsic to do PCF style shadows.
                // See: crbug.com/467015399
                // To do a complete polyfill we would have to properly do bilinear interpolation of
                // the TextureGatherCompare result which we do not do as there is no trivial way to
                // do it for a cubemap.

                // We do a textureGatherCompare and then dot with a vec4f(0.25) to get the average
                // result. This will give PCF-like shadows but they will not be as smooth as the
                // result from the original TextureSampleCompare. We also only sample mip0 which is
                // identical to TextureSampleCompareLevel but not TextureSampleCompare.
                close_to_pcf_result =
                    b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kDot, result,
                                                   b.Splat(ty.vec4f(), b.Constant(0.25_f)));
            });

            close_to_pcf_result->SetResult(builtin->DetachResult());
            builtin->Destroy();
            return;
        }

        result->SetResult(builtin->DetachResult());
        builtin->Destroy();
    }

    /// Handle a textureGather*() builtin.
    /// @param builtin the builtin call instruction
    void TextureGather(core::ir::CoreBuiltinCall* builtin) {
        // Helper to get the next argument from the call, or nullptr if there are no more arguments.
        uint32_t arg_idx = 0;
        auto next_arg = [&]() {
            return arg_idx < builtin->Args().size() ? builtin->Args()[arg_idx++] : nullptr;
        };

        auto* component = next_arg();
        if (!component->Type()->IsIntegerScalar()) {
            // The first argument wasn't the component, so it must be the texture instead.
            // Use constant zero for the component.
            component = b.Constant(0_u);
            arg_idx--;
        }
        auto* texture = next_arg();
        auto* sampler = next_arg();
        auto* coords = next_arg();
        auto* texture_ty = texture->Type()->As<spirv::type::Image>();

        // Use OpSampledImage to create an OpTypeSampledImage object.
        auto* sampled_image = b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.Get<type::SampledImage>(texture_ty), spirv::BuiltinFn::kOpSampledImage,
            Vector{texture_ty}, Vector{texture, sampler});
        sampled_image->InsertBefore(builtin);

        // Append the array index to the coordinates if provided.
        auto* array_idx =
            texture_ty->GetArrayed() == type::Arrayed::kArrayed ? next_arg() : nullptr;
        if (array_idx) {
            coords = AppendArrayIndex(coords, array_idx, builtin);
        }

        // Determine which SPIR-V function to use and which optional image operands are needed.
        enum spirv::BuiltinFn function = BuiltinFn::kNone;
        core::ir::Value* depth = nullptr;
        ImageOperands operands;
        switch (builtin->Func()) {
            case core::BuiltinFn::kTextureGather:
                function = spirv::BuiltinFn::kImageGather;
                operands.offset = next_arg();
                break;
            case core::BuiltinFn::kTextureGatherCompare:
                function = spirv::BuiltinFn::kImageDrefGather;
                depth = next_arg();
                operands.offset = next_arg();
                break;
            default:
                TINT_IR_UNIMPLEMENTED(ir) << "unhandled texture gather builtin";
        }

        // Start building the argument list for the function.
        // The first two operands are always the sampled image and then the coordinates, followed by
        // either the depth reference or the component.
        Vector<core::ir::Value*, 8> function_args;
        function_args.Push(sampled_image->Result());
        function_args.Push(coords);
        if (depth) {
            function_args.Push(depth);
        } else {
            function_args.Push(component);
        }

        // Add the optional image operands, if any.
        AppendImageOperands(operands, function_args, builtin, /* requires_float_lod */ true);

        // Call the function.
        auto* texture_call = b.CallWithResult<spirv::ir::BuiltinCall>(
            builtin->DetachResult(), function, std::move(function_args));
        texture_call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Handle a textureLoad() builtin.
    /// @param builtin the builtin call instruction
    void TextureLoad(core::ir::CoreBuiltinCall* builtin) {
        // Helper to get the next argument from the call, or nullptr if there are no more arguments.
        uint32_t arg_idx = 0;
        auto next_arg = [&]() {
            return arg_idx < builtin->Args().size() ? builtin->Args()[arg_idx++] : nullptr;
        };

        auto* texture = next_arg();
        auto* coords = next_arg();
        auto* texture_ty = texture->Type()->As<spirv::type::Image>();

        // Append the array index to the coordinates if provided.
        auto* array_idx =
            texture_ty->GetArrayed() == type::Arrayed::kArrayed ? next_arg() : nullptr;
        if (array_idx) {
            coords = AppendArrayIndex(coords, array_idx, builtin);
        }

        // Start building the argument list for the builtin.
        // The first two operands are always the texture and then the coordinates.
        Vector<core::ir::Value*, 8> builtin_args;
        builtin_args.Push(texture);
        builtin_args.Push(coords);

        // Add the optional image operands, if any.
        ImageOperands operands;
        if (texture_ty->GetMultisampled() == type::Multisampled::kMultisampled) {
            operands.sample = next_arg();
        } else {
            operands.lod = next_arg();
        }
        AppendImageOperands(operands, builtin_args, builtin, /* requires_float_lod */ false);

        // Call the builtin.
        // The result is always a vec4 in SPIR-V.
        auto* result_ty = builtin->Result()->Type();
        bool expects_scalar_result = result_ty->Is<core::type::Scalar>();
        if (expects_scalar_result) {
            result_ty = ty.vec4(result_ty);
        }
        auto kind = texture_ty->GetSampled() == type::Sampled::kSamplingCompatible
                        ? spirv::BuiltinFn::kImageFetch
                        : spirv::BuiltinFn::kImageRead;
        core::ir::Instruction* result =
            b.Call<spirv::ir::BuiltinCall>(result_ty, kind, std::move(builtin_args));
        result->InsertBefore(builtin);

        // If we are expecting a scalar result, extract the first component.
        if (expects_scalar_result) {
            result = b.Access(ty.f32(), result, 0_u);
            result->InsertBefore(builtin);
        }

        result->SetResult(builtin->DetachResult());
        builtin->Destroy();
    }

    /// Handle a textureStore() builtin.
    /// @param builtin the builtin call instruction
    void TextureStore(core::ir::CoreBuiltinCall* builtin) {
        // Helper to get the next argument from the call, or nullptr if there are no more arguments.
        uint32_t arg_idx = 0;
        auto next_arg = [&]() {
            return arg_idx < builtin->Args().size() ? builtin->Args()[arg_idx++] : nullptr;
        };

        auto* texture = next_arg();
        auto* coords = next_arg();
        auto* texture_ty = texture->Type()->As<spirv::type::Image>();

        // Append the array index to the coordinates if provided.
        auto* array_idx =
            texture_ty->GetArrayed() == type::Arrayed::kArrayed ? next_arg() : nullptr;
        if (array_idx) {
            coords = AppendArrayIndex(coords, array_idx, builtin);
        }

        auto* texel = next_arg();

        // Start building the argument list for the function.
        // The first two operands are always the texture and then the coordinates.
        Vector<core::ir::Value*, 8> function_args;
        function_args.Push(texture);
        function_args.Push(coords);
        function_args.Push(texel);

        ImageOperands operands;
        AppendImageOperands(operands, function_args, builtin, /* requires_float_lod */ false);

        // Call the function.
        auto* texture_call = b.Call<spirv::ir::BuiltinCall>(
            ty.void_(), spirv::BuiltinFn::kImageWrite, std::move(function_args));
        texture_call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Handle a textureDimensions() builtin.
    /// @param builtin the builtin call instruction
    void TextureDimensions(core::ir::CoreBuiltinCall* builtin) {
        // Helper to get the next argument from the call, or nullptr if there are no more arguments.
        uint32_t arg_idx = 0;
        auto next_arg = [&]() {
            return arg_idx < builtin->Args().size() ? builtin->Args()[arg_idx++] : nullptr;
        };

        auto* texture = next_arg();
        auto* texture_ty = texture->Type()->As<spirv::type::Image>();

        Vector<core::ir::Value*, 8> function_args;
        function_args.Push(texture);

        // Determine which SPIR-V function to use, and add the Lod argument if needed.
        enum spirv::BuiltinFn function;
        if (texture_ty->GetMultisampled() == type::Multisampled::kMultisampled ||
            texture_ty->GetTexelFormat() != core::TexelFormat::kUndefined) {
            function = spirv::BuiltinFn::kImageQuerySize;
        } else {
            function = spirv::BuiltinFn::kImageQuerySizeLod;
            if (auto* lod = next_arg()) {
                function_args.Push(lod);
            } else {
                // Lod wasn't explicit, so assume 0.
                function_args.Push(b.Constant(0_u));
            }
        }

        // Add an extra component to the result vector for arrayed textures.
        auto* result_ty = builtin->Result()->Type();
        bool is_arrayed = texture_ty->GetArrayed() == type::Arrayed::kArrayed;
        if (is_arrayed) {
            auto* vec = result_ty->As<core::type::Vector>();
            result_ty = ty.vec(vec->Type(), vec->Width() + 1);
        }

        // Call the function.
        core::ir::Instruction* result = b.CallExplicit<spirv::ir::BuiltinCall>(
            result_ty, function, Vector{ty.u32()}, std::move(function_args));
        result->InsertBefore(builtin);

        // Swizzle the first two components from the result for arrayed textures.
        if (is_arrayed) {
            result = b.Swizzle(builtin->Result()->Type(), result, {0, 1});
            result->InsertBefore(builtin);
        }

        result->SetResult(builtin->DetachResult());
        builtin->Destroy();
    }

    /// Handle a textureNumLevels() builtin.
    /// @param builtin the builtin call instruction
    void TextureNumLevels(core::ir::CoreBuiltinCall* builtin) {
        auto args = builtin->Args();

        b.InsertBefore(builtin, [&] {
            // Call the function.
            auto* res_ty = builtin->Result()->Type();
            b.CallExplicitWithResult<spirv::ir::BuiltinCall>(builtin->DetachResult(),
                                                             spirv::BuiltinFn::kImageQueryLevels,
                                                             Vector{res_ty}, Vector{args[0]});
        });
        builtin->Destroy();
    }

    /// Handle a textureNumSamples() builtin.
    /// @param builtin the builtin call instruction
    void TextureNumSamples(core::ir::CoreBuiltinCall* builtin) {
        auto args = builtin->Args();

        b.InsertBefore(builtin, [&] {
            // Call the function.
            auto* res_ty = builtin->Result()->Type();
            b.CallExplicitWithResult<spirv::ir::BuiltinCall>(builtin->DetachResult(),
                                                             spirv::BuiltinFn::kImageQuerySamples,
                                                             Vector{res_ty}, Vector{args[0]});
        });
        builtin->Destroy();
    }

    /// Handle a textureNumLayers() builtin.
    /// @param builtin the builtin call instruction
    void TextureNumLayers(core::ir::CoreBuiltinCall* builtin) {
        auto* texture = builtin->Args()[0];
        auto* texture_ty = texture->Type()->As<spirv::type::Image>();

        Vector<core::ir::Value*, 2> function_args;
        function_args.Push(texture);

        // Determine which SPIR-V function to use, and add the Lod argument if needed.
        enum spirv::BuiltinFn function;
        if (texture_ty->GetMultisampled() == type::Multisampled::kMultisampled ||
            texture_ty->GetTexelFormat() != core::TexelFormat::kUndefined) {
            function = spirv::BuiltinFn::kImageQuerySize;
        } else {
            function = spirv::BuiltinFn::kImageQuerySizeLod;
            function_args.Push(b.Constant(0_u));
        }

        // Call the function.
        auto* texture_call = b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec3u(), function, Vector{ty.u32()}, std::move(function_args));
        texture_call->InsertBefore(builtin);

        // Extract the third component to get the number of array layers.
        auto* extract = b.AccessWithResult(builtin->DetachResult(), texture_call->Result(), 2_u);
        extract->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Scalarize the vector form of a `quantizeToF16()` builtin.
    /// See crbug.com/tint/1741.
    /// @param builtin the builtin call instruction
    void QuantizeToF16Vec(core::ir::CoreBuiltinCall* builtin) {
        auto* arg = builtin->Args()[0];
        auto* vec = arg->Type()->As<core::type::Vector>();
        TINT_IR_ASSERT(ir, vec);

        // Replace the builtin call with a call to the spirv.dot intrinsic.
        Vector<core::ir::Value*, 4> args;
        for (uint32_t i = 0; i < vec->Width(); i++) {
            auto* el = b.Access(ty.f32(), arg, u32(i));
            auto* scalar_call = b.Call(ty.f32(), core::BuiltinFn::kQuantizeToF16, el);
            args.Push(scalar_call->Result());
            el->InsertBefore(builtin);
            scalar_call->InsertBefore(builtin);
        }
        auto* construct = b.ConstructWithResult(builtin->DetachResult(), std::move(args));
        construct->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Handle an inputAttachmentLoad() builtin.
    /// @param builtin the builtin call instruction
    void InputAttachmentLoad(core::ir::CoreBuiltinCall* builtin) {
        TINT_IR_ASSERT(ir, builtin->Args().size() == 1);

        auto* texture = builtin->Args()[0];
        // coords for input_attachment are always (0, 0)
        auto* coords = b.Composite(ty.vec2i(), 0_i, 0_i);

        // Start building the argument list for the builtin.
        // The first two operands are always the texture and then the coordinates.
        Vector<core::ir::Value*, 8> builtin_args;
        builtin_args.Push(texture);
        builtin_args.Push(coords);

        // Call the builtin.
        // The result is always a vec4 in SPIR-V.
        auto* result_ty = builtin->Result()->Type();
        TINT_IR_ASSERT(ir, result_ty->Is<core::type::Vector>());

        core::ir::Instruction* result = b.Call<spirv::ir::BuiltinCall>(
            result_ty, spirv::BuiltinFn::kImageRead, std::move(builtin_args));
        result->InsertBefore(builtin);

        result->SetResult(builtin->DetachResult());
        builtin->Destroy();
    }

    /// Handles SubgroupShuffle(), SubgroupShuffleDown(), SubgroupShuffleUp(), SubgroupShuffleXor()
    /// builtins.
    /// @param builtin the builtin call instruction
    void SubgroupShuffle(core::ir::CoreBuiltinCall* builtin, bool clamp_subgroup_shuffle) {
        TINT_IR_ASSERT(ir, builtin->Args().size() == 2);
        // The second argument is either 'id' , 'delta', or 'mask'.
        // All must be bound by [0, 128)
        auto* arg2 = builtin->Args()[1];
        // arg2 must be an unsigned integer scalar, so bitcast if necessary.
        if (arg2->Type()->IsSignedIntegerScalar()) {
            auto* cast = b.Bitcast(ty.u32(), arg2);
            cast->InsertBefore(builtin);
            builtin->SetArg(1, cast->Result());
        }

        /// Polyfill a `subgroupShuffleX` builtin call with one that has clamped the arg2 param
        if (clamp_subgroup_shuffle) {
            auto* shuffle_id = builtin->Args()[1];
            auto* mask_max_subgroup_size =
                b.Constant(core::u32(tint::internal_limits::kMaxSubgroupSize - 1));
            b.InsertBefore(builtin, [&] {
                auto* clamp_via_masking_and = b.And(shuffle_id, mask_max_subgroup_size);
                builtin->SetArg(1, clamp_via_masking_and->Result());
            });
        }
    }

    /// Handle a SubgroupBroadcast() builtin.
    /// @param builtin the builtin call instruction
    void SubgroupBroadcast(core::ir::CoreBuiltinCall* builtin) {
        TINT_IR_ASSERT(ir, builtin->Args().size() == 2);
        auto* id = builtin->Args()[1];
        TINT_IR_ASSERT(ir, id->Is<core::ir::Constant>());

        // For const signed int IDs, compile-time convert to u32 to maintain constness.
        if (id->Type()->IsSignedIntegerScalar()) {
            builtin->SetArg(1, b.Constant(id->As<core::ir::Constant>()->Value()->ValueAs<u32>()));
        }
    }

    /// Handle a QuadBroadcast() builtin.
    /// @param builtin the builtin call instruction
    void QuadBroadcast(core::ir::CoreBuiltinCall* builtin) {
        TINT_IR_ASSERT(ir, builtin->Args().size() == 2);
        auto* id = builtin->Args()[1];
        TINT_IR_ASSERT(ir, id->Is<core::ir::Constant>());

        // For const signed int IDs, compile-time convert to u32 to maintain constness.
        if (id->Type()->IsSignedIntegerScalar()) {
            builtin->SetArg(1, b.Constant(id->As<core::ir::Constant>()->Value()->ValueAs<u32>()));
        }
    }

    /// Replace a subgroupMatrixLoad builtin.
    /// @param builtin the builtin call instruction
    void SubgroupMatrixLoad(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* result_ty = builtin->Result()->Type()->As<core::type::SubgroupMatrix>();
            auto* p = builtin->Args()[0];
            auto* offset = builtin->Args()[1];
            auto* col_major = builtin->Args()[2]->As<core::ir::Constant>();
            auto* stride = builtin->Args()[3];

            auto* ptr = p->Type()->As<core::type::Pointer>();
            auto* arr = ptr->StoreType()->As<core::type::Array>();

            auto* layout = b.Constant(u32(col_major->Value()->ValueAs<bool>()
                                              ? SpvCooperativeMatrixLayoutColumnMajorKHR
                                              : SpvCooperativeMatrixLayoutRowMajorKHR));
            auto* memory_operand = Literal(u32(SpvMemoryAccessNonPrivatePointerMask));

            // In SPIR-V `stride` and `offset` are related to the type of the input pointer, while
            // in WGSL they both mean the number of elements. When the subgroup matrix element type
            // is `i8` or `u8`, and the input array type is `i32` or `u32`, we need to convert the
            // `stride` and `offset` in WGSL into the ones in SPIR-V by dividing them with 4.
            auto* applied_stride = stride;
            auto* applied_offset = offset;
            if (result_ty->Type()->Size() == 1u && arr->ElemType()->Size() == 4u) {
                if (!config.cooperative_matrix_stride_is_matrix_elements) {
                    auto* applied_stride_binary =
                        b.Binary(core::BinaryOp::kDivide, stride->Type(), stride, u32(4));
                    applied_stride = applied_stride_binary->Result();
                }

                auto* applied_offset_binary =
                    b.Binary(core::BinaryOp::kDivide, offset->Type(), offset, u32(4));
                applied_offset = applied_offset_binary->Result();
            }

            // Make a pointer to the first element of the array that we will load from.
            auto* elem_ptr = ty.ptr(ptr->AddressSpace(), arr->ElemType(), ptr->Access());
            auto* src = b.Access(elem_ptr, p, applied_offset);

            auto* call = b.CallWithResult<spirv::ir::BuiltinCall>(
                builtin->DetachResult(), spirv::BuiltinFn::kCooperativeMatrixLoad, src, layout,
                applied_stride, memory_operand);
            call->SetExplicitTemplateParams(Vector{result_ty});
        });
        builtin->Destroy();
    }

    /// Replace a subgroupMatrixStore builtin.
    /// @param builtin the builtin call instruction
    void SubgroupMatrixStore(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* p = builtin->Args()[0];
            auto* offset = builtin->Args()[1];
            auto* value = builtin->Args()[2];
            auto* value_type = value->Type()->As<core::type::SubgroupMatrix>();

            auto* col_major = builtin->Args()[3]->As<core::ir::Constant>();
            auto* stride = builtin->Args()[4];

            auto* ptr = p->Type()->As<core::type::Pointer>();
            auto* arr = ptr->StoreType()->As<core::type::Array>();

            // In SPIR-V `stride` and `offset` are related to the type of the input pointer, while
            // in WGSL they both mean the number of elements. When the subgroup matrix element type
            // is `i8` or `u8`, and the input array type is `i32` or `u32`, we need to convert the
            // `stride` and `offset` in WGSL into the ones in SPIR-V by dividing them with 4.
            auto* applied_stride = stride;
            auto* applied_offset = offset;
            if (value_type->Type()->Size() == 1u && arr->ElemType()->Size() == 4u) {
                if (!config.cooperative_matrix_stride_is_matrix_elements) {
                    auto* applied_stride_binary =
                        b.Binary(core::BinaryOp::kDivide, stride->Type(), stride, u32(4));
                    applied_stride = applied_stride_binary->Result();
                }

                auto* applied_offset_binary =
                    b.Binary(core::BinaryOp::kDivide, offset->Type(), offset, u32(4));
                applied_offset = applied_offset_binary->Result();
            }

            // Make a pointer to the first element of the array that we will write to.
            auto* elem_ptr = ty.ptr(ptr->AddressSpace(), arr->ElemType(), ptr->Access());
            auto* dst = b.Access(elem_ptr, p, applied_offset);

            auto* layout = b.Constant(u32(col_major->Value()->ValueAs<bool>()
                                              ? SpvCooperativeMatrixLayoutColumnMajorKHR
                                              : SpvCooperativeMatrixLayoutRowMajorKHR));
            auto* memory_operand = Literal(u32(SpvMemoryAccessNonPrivatePointerMask));

            b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kCooperativeMatrixStore,
                                           dst, value, layout, applied_stride, memory_operand);
        });
        builtin->Destroy();
    }

    /// Generate the literal operand for a subgroup matrix multiply instruction.
    /// @param input_ty the type of the input matrices
    /// @param result_ty the type of the result matrix
    /// @returns the literal operands
    core::ir::Value* SubgroupMatrixMultiplyOperands(const core::type::SubgroupMatrix* input_ty,
                                                    const core::type::SubgroupMatrix* result_ty) {
        uint32_t operands = SpvCooperativeMatrixOperandsMaskNone;
        if (input_ty->Type()->IsSignedIntegerScalar()) {
            operands |= SpvCooperativeMatrixOperandsMatrixASignedComponentsKHRMask;
            operands |= SpvCooperativeMatrixOperandsMatrixBSignedComponentsKHRMask;
        }
        if (result_ty->Type()->IsSignedIntegerScalar()) {
            operands |= SpvCooperativeMatrixOperandsMatrixCSignedComponentsKHRMask;
            operands |= SpvCooperativeMatrixOperandsMatrixResultSignedComponentsKHRMask;
        }
        return Literal(u32(operands));
    }

    /// Replace a subgroupMatrixMultiply builtin.
    /// @param builtin the builtin call instruction
    void SubgroupMatrixMultiply(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            // SPIR-V only provides a multiply-accumulate instruction, so construct a zero-valued
            // matrix to accumulate into.
            auto* result_ty = builtin->Result()->Type()->As<core::type::SubgroupMatrix>();
            auto* left = builtin->Args()[0];
            auto* right = builtin->Args()[1];
            auto* acc = b.Construct(result_ty);
            auto* operands = SubgroupMatrixMultiplyOperands(
                left->Type()->As<core::type::SubgroupMatrix>(), result_ty);
            b.CallWithResult<spirv::ir::BuiltinCall>(builtin->DetachResult(),
                                                     spirv::BuiltinFn::kCooperativeMatrixMulAdd,
                                                     left, right, acc, operands);
        });
        builtin->Destroy();
    }

    /// Replace a subgroupMatrixMultiplyAccumulate builtin.
    /// @param builtin the builtin call instruction
    void SubgroupMatrixMultiplyAccumulate(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* left = builtin->Args()[0];
            auto* right = builtin->Args()[1];
            auto* acc = builtin->Args()[2];
            auto* operands =
                SubgroupMatrixMultiplyOperands(left->Type()->As<core::type::SubgroupMatrix>(),
                                               acc->Type()->As<core::type::SubgroupMatrix>());
            b.CallWithResult<spirv::ir::BuiltinCall>(builtin->DetachResult(),
                                                     spirv::BuiltinFn::kCooperativeMatrixMulAdd,
                                                     left, right, acc, operands);
        });
        builtin->Destroy();
    }

    /// Replace a subgroupMatrixScalar builtin.
    /// @param builtin the builtin call instruction
    /// @param op the operation to perform
    void SubgroupMatrixScalar(core::ir::CoreBuiltinCall* builtin, core::BinaryOp op) {
        b.InsertBefore(builtin, [&] {
            auto* mat = builtin->Args()[0];
            auto* scalar = builtin->Args()[1];

            auto* sm_ty = mat->Type()->As<core::type::SubgroupMatrix>();
            if (sm_ty->Type()->Is<core::type::I8>()) {
                scalar = b.CallExplicit<spirv::ir::BuiltinCall>(
                              ty.i8(), spirv::BuiltinFn::kSConvert, Vector{ty.i8()},
                              b.Clamp(scalar, -128_i, 127_i))
                             ->Result();
            } else if (sm_ty->Type()->Is<core::type::U8>()) {
                scalar = b.CallExplicit<spirv::ir::BuiltinCall>(
                              ty.u8(), spirv::BuiltinFn::kUConvert, Vector{ty.u8()},
                              b.Clamp(scalar, 0_u, 255_u))
                             ->Result();
            }

            auto* scalar_mat = b.Construct(sm_ty, scalar);
            b.BinaryWithResult<spirv::ir::Binary>(builtin->DetachResult(), op, mat, scalar_mat);
        });
        builtin->Destroy();
    }
};

}  // namespace

Result<SuccessType> BuiltinPolyfill(core::ir::Module& ir, PolyfillConfig config) {
    AssertValid(ir,
                core::ir::Capabilities{
                    core::ir::Capability::kAllow8BitIntegers,
                    core::ir::Capability::kAllowDuplicateBindings,
                    core::ir::Capability::kAllowNonCoreTypes,
                },
                "before spirv.BuiltinPolyfill");

    State{ir, config}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
