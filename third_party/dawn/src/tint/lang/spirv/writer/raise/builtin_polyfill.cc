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
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/ir/image_from_texture.h"
#include "src/tint/lang/spirv/ir/literal_operand.h"
#include "src/tint/lang/spirv/type/sampled_image.h"
#include "src/tint/utils/ice/ice.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// If we should use the vulkan memory model
    bool use_vulkan_memory_model = false;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Find the builtins that need replacing.
        Vector<core::ir::CoreBuiltinCall*, 4> worklist;

        // Convert function parameters to `spirv::type::Image` if necessary
        for (auto fn : ir.functions) {
            for (auto* param : fn->Params()) {
                if (auto* tex = param->Type()->As<core::type::Texture>()) {
                    param->SetType(ir::ImageFromTexture(ty, tex));
                }
            }
        }

        for (auto* inst : ir.Instructions()) {
            // Convert instruction results to `spirv::type::Image` if necessary
            if (!inst->Results().IsEmpty()) {
                if (auto* res = inst->Result(0)->As<core::ir::InstructionResult>()) {
                    // Watch for pointers, which would be wrapping any texture on a `var`
                    if (auto* tex = res->Type()->UnwrapPtr()->As<core::type::Texture>()) {
                        auto* tex_ty = ir::ImageFromTexture(ty, tex);
                        const core::type::Type* res_ty = tex_ty;
                        if (auto* orig_ptr = res->Type()->As<core::type::Pointer>()) {
                            res_ty = ty.ptr(orig_ptr->AddressSpace(), res_ty, orig_ptr->Access());
                        }
                        res->SetType(res_ty);
                    }
                }
            }

            if (auto* builtin = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (builtin->Func()) {
                    case core::BuiltinFn::kArrayLength:
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
                    case core::BuiltinFn::kDot:
                    case core::BuiltinFn::kDot4I8Packed:
                    case core::BuiltinFn::kDot4U8Packed:
                    case core::BuiltinFn::kQuadBroadcast:
                    case core::BuiltinFn::kSelect:
                    case core::BuiltinFn::kSubgroupBroadcast:
                    case core::BuiltinFn::kSubgroupShuffle:
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
                    SubgroupShuffle(builtin);
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
                default:
                    break;
            }
        }
    }

    /// Create a literal operand.
    /// @param value the literal value
    /// @returns the literal operand
    spirv::ir::LiteralOperand* Literal(u32 value) {
        return ir.CreateValue<spirv::ir::LiteralOperand>(b.ConstantValue(value));
    }

    /// Handle an `arrayLength()` builtin.
    /// @param builtin the builtin call instruction
    void ArrayLength(core::ir::CoreBuiltinCall* builtin) {
        // Strip away any let instructions to get to the original struct member access instruction.
        auto* ptr = builtin->Args()[0]->As<core::ir::InstructionResult>();
        while (auto* let = tint::As<core::ir::Let>(ptr->Instruction())) {
            ptr = let->Value()->As<core::ir::InstructionResult>();
        }
        TINT_ASSERT(ptr);

        auto* access = ptr->Instruction()->As<core::ir::Access>();
        TINT_ASSERT(access);
        TINT_ASSERT(access->Indices().Length() == 1u);
        TINT_ASSERT(access->Object()->Type()->UnwrapPtr()->Is<core::type::Struct>());
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
                    TINT_UNREACHABLE() << "unhandled atomic address space";
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
                auto* compare = b.Equal(ty.bool_(), original, cmp);
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
                TINT_UNREACHABLE() << "unhandled atomic builtin";
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
                    auto* mul = b.Multiply(elty, e1, e2);
                    if (sum) {
                        sum = b.Add(elty, sum, mul);
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

        // If the condition is scalar and the objects are vectors, we need to splat the condition
        // into a vector of the same size.
        // TODO(jrprice): We don't need to do this if we're targeting SPIR-V 1.4 or newer.
        auto* vec = builtin->Result()->Type()->As<core::type::Vector>();
        if (vec && args[0]->Type()->Is<core::type::Scalar>()) {
            Vector<core::ir::Value*, 4> elements;
            elements.Resize(vec->Width(), args[0]);

            auto* construct = b.Construct(ty.vec(ty.bool_(), vec->Width()), std::move(elements));
            construct->InsertBefore(builtin);
            args[0] = construct->Result();
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
        if (use_vulkan_memory_model) {
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
            return arg_idx < builtin->Args().Length() ? builtin->Args()[arg_idx++] : nullptr;
        };

        auto* texture = next_arg();
        auto* sampler = next_arg();
        auto* coords = next_arg();
        auto* texture_ty = texture->Type()->As<spirv::type::Image>();

        // Use OpSampledImage to create an OpTypeSampledImage object.
        auto* sampled_image = b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.Get<type::SampledImage>(texture_ty), spirv::BuiltinFn::kSampledImage,
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
                function = spirv::BuiltinFn::kImageSampleDrefImplicitLod;
                depth = next_arg();
                operands.offset = next_arg();
                break;
            case core::BuiltinFn::kTextureSampleCompareLevel:
                function = spirv::BuiltinFn::kImageSampleDrefExplicitLod;
                depth = next_arg();
                operands.lod = b.Constant(0_f);
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
                TINT_UNREACHABLE() << "unhandled texture sample builtin";
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
        auto* result_ty = depth ? static_cast<const core::type::Type*>(ty.f32()) : ty.vec4<f32>();
        core::ir::Instruction* result =
            b.Call<spirv::ir::BuiltinCall>(result_ty, function, std::move(function_args));
        result->InsertBefore(builtin);

        // If this is not a depth comparison but we are sampling a depth texture, extract the first
        // component to get the scalar f32 that SPIR-V expects.
        if (!depth && texture_ty->GetDepth() == type::Depth::kDepth) {
            result = b.Access(ty.f32(), result, 0_u);
            result->InsertBefore(builtin);
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
            return arg_idx < builtin->Args().Length() ? builtin->Args()[arg_idx++] : nullptr;
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
            ty.Get<type::SampledImage>(texture_ty), spirv::BuiltinFn::kSampledImage,
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
                TINT_UNIMPLEMENTED() << "unhandled texture gather builtin";
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
            return arg_idx < builtin->Args().Length() ? builtin->Args()[arg_idx++] : nullptr;
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
            return arg_idx < builtin->Args().Length() ? builtin->Args()[arg_idx++] : nullptr;
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
            return arg_idx < builtin->Args().Length() ? builtin->Args()[arg_idx++] : nullptr;
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
        core::ir::Instruction* result =
            b.Call<spirv::ir::BuiltinCall>(result_ty, function, std::move(function_args));
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
        auto* texture_call =
            b.Call<spirv::ir::BuiltinCall>(ty.vec3<u32>(), function, std::move(function_args));
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
        TINT_ASSERT(vec);

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
        TINT_ASSERT(builtin->Args().Length() == 1);

        auto* texture = builtin->Args()[0];
        // coords for input_attachment are always (0, 0)
        auto* coords = b.Composite(ty.vec2<i32>(), 0_i, 0_i);

        // Start building the argument list for the builtin.
        // The first two operands are always the texture and then the coordinates.
        Vector<core::ir::Value*, 8> builtin_args;
        builtin_args.Push(texture);
        builtin_args.Push(coords);

        // Call the builtin.
        // The result is always a vec4 in SPIR-V.
        auto* result_ty = builtin->Result()->Type();
        TINT_ASSERT(result_ty->Is<core::type::Vector>());

        core::ir::Instruction* result = b.Call<spirv::ir::BuiltinCall>(
            result_ty, spirv::BuiltinFn::kImageRead, std::move(builtin_args));
        result->InsertBefore(builtin);

        result->SetResult(builtin->DetachResult());
        builtin->Destroy();
    }

    /// Handle a SubgroupShuffle() builtin.
    /// @param builtin the builtin call instruction
    void SubgroupShuffle(core::ir::CoreBuiltinCall* builtin) {
        TINT_ASSERT(builtin->Args().Length() == 2);
        auto* id = builtin->Args()[1];

        // Id must be an unsigned integer scalar, so bitcast if necessary.
        if (id->Type()->IsSignedIntegerScalar()) {
            auto* cast = b.Bitcast(ty.u32(), id);
            cast->InsertBefore(builtin);
            builtin->SetArg(1, cast->Result());
        }
    }

    /// Handle a SubgroupBroadcast() builtin.
    /// @param builtin the builtin call instruction
    void SubgroupBroadcast(core::ir::CoreBuiltinCall* builtin) {
        TINT_ASSERT(builtin->Args().Length() == 2);
        auto* id = builtin->Args()[1];
        TINT_ASSERT(id->Is<core::ir::Constant>());

        // For const signed int IDs, compile-time convert to u32 to maintain constness.
        if (id->Type()->IsSignedIntegerScalar()) {
            builtin->SetArg(1, b.Constant(id->As<core::ir::Constant>()->Value()->ValueAs<u32>()));
        }
    }

    /// Handle a QuadBroadcast() builtin.
    /// @param builtin the builtin call instruction
    void QuadBroadcast(core::ir::CoreBuiltinCall* builtin) {
        TINT_ASSERT(builtin->Args().Length() == 2);
        auto* id = builtin->Args()[1];
        TINT_ASSERT(id->Is<core::ir::Constant>());

        // For const signed int IDs, compile-time convert to u32 to maintain constness.
        if (id->Type()->IsSignedIntegerScalar()) {
            builtin->SetArg(1, b.Constant(id->As<core::ir::Constant>()->Value()->ValueAs<u32>()));
        }
    }

    /// Replace a subgroupMatrixLoad builtin.
    /// @param builtin the builtin call instruction
    void SubgroupMatrixLoad(core::ir::CoreBuiltinCall* builtin) {
        b.InsertBefore(builtin, [&] {
            auto* result_ty = builtin->Result()->Type();
            auto* p = builtin->Args()[0];
            auto* offset = builtin->Args()[1];
            auto* col_major = builtin->Args()[2]->As<core::ir::Constant>();
            auto* stride = builtin->Args()[3];

            auto* ptr = p->Type()->As<core::type::Pointer>();
            auto* arr = ptr->StoreType()->As<core::type::Array>();

            // Make a pointer to the first element of the array that we will load from.
            auto* elem_ptr = ty.ptr(ptr->AddressSpace(), arr->ElemType(), ptr->Access());
            auto* src = b.Access(elem_ptr, p, offset);

            auto* layout = b.Constant(u32(col_major->Value()->ValueAs<bool>()
                                              ? SpvCooperativeMatrixLayoutColumnMajorKHR
                                              : SpvCooperativeMatrixLayoutRowMajorKHR));
            auto* memory_operand = Literal(u32(SpvMemoryAccessNonPrivatePointerMask));

            auto* call = b.CallWithResult<spirv::ir::BuiltinCall>(
                builtin->DetachResult(), spirv::BuiltinFn::kCooperativeMatrixLoad, src, layout,
                stride, memory_operand);
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
            auto* col_major = builtin->Args()[3]->As<core::ir::Constant>();
            auto* stride = builtin->Args()[4];

            auto* ptr = p->Type()->As<core::type::Pointer>();
            auto* arr = ptr->StoreType()->As<core::type::Array>();

            // Make a pointer to the first element of the array that we will write to.
            auto* elem_ptr = ty.ptr(ptr->AddressSpace(), arr->ElemType(), ptr->Access());
            auto* dst = b.Access(elem_ptr, p, offset);

            auto* layout = b.Constant(u32(col_major->Value()->ValueAs<bool>()
                                              ? SpvCooperativeMatrixLayoutColumnMajorKHR
                                              : SpvCooperativeMatrixLayoutRowMajorKHR));
            auto* memory_operand = Literal(u32(SpvMemoryAccessNonPrivatePointerMask));

            b.Call<spirv::ir::BuiltinCall>(ty.void_(), spirv::BuiltinFn::kCooperativeMatrixStore,
                                           dst, value, layout, stride, memory_operand);
        });
        builtin->Destroy();
    }

    /// Generate the literal operand for a subgroup matrix multiply instruction.
    /// @param input_ty the type of the input matrices
    /// @param result_ty the type of the result matrix
    /// @returns the literal operands
    ir::LiteralOperand* SubgroupMatrixMultiplyOperands(
        const core::type::SubgroupMatrix* input_ty,
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
};

}  // namespace

Result<SuccessType> BuiltinPolyfill(core::ir::Module& ir, bool use_vulkan_memory_model) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.BuiltinPolyfill");
    if (result != Success) {
        return result.Failure();
    }

    State{ir, use_vulkan_memory_model}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
