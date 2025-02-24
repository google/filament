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

#include "src/tint/lang/spirv/writer/printer/printer.h"

#include <string>
#include <utility>

#include "spirv/unified1/GLSL.std.450.h"
#include "spirv/unified1/spirv.h"

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/terminator.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/ir/literal_operand.h"
#include "src/tint/lang/spirv/type/sampled_image.h"
#include "src/tint/lang/spirv/writer/common/binary_writer.h"
#include "src/tint/lang/spirv/writer/common/function.h"
#include "src/tint/lang/spirv/writer/common/module.h"
#include "src/tint/lang/spirv/writer/common/options.h"
#include "src/tint/lang/spirv/writer/raise/builtin_polyfill.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/result/result.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/symbol/symbol.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

constexpr uint32_t kWriterVersion = 1;
constexpr const char* kGLSLstd450 = "GLSL.std.450";

SpvStorageClass StorageClass(core::AddressSpace addrspace) {
    switch (addrspace) {
        case core::AddressSpace::kHandle:
            return SpvStorageClassUniformConstant;
        case core::AddressSpace::kFunction:
            return SpvStorageClassFunction;
        case core::AddressSpace::kIn:
            return SpvStorageClassInput;
        case core::AddressSpace::kPrivate:
            return SpvStorageClassPrivate;
        case core::AddressSpace::kPushConstant:
            return SpvStorageClassPushConstant;
        case core::AddressSpace::kOut:
            return SpvStorageClassOutput;
        case core::AddressSpace::kStorage:
            return SpvStorageClassStorageBuffer;
        case core::AddressSpace::kUniform:
            return SpvStorageClassUniform;
        case core::AddressSpace::kWorkgroup:
            return SpvStorageClassWorkgroup;
        default:
            return SpvStorageClassMax;
    }
}

const core::type::Type* DedupType(const core::type::Type* ty, core::type::Manager& types) {
    return Switch(
        ty,

        // Atomics are not a distinct type in SPIR-V.
        [&](const core::type::Atomic* atomic) { return atomic->Type(); },

        // Depth textures are always declared as sampled textures.
        [&](const core::type::DepthTexture* depth) {
            return types.Get<core::type::SampledTexture>(depth->Dim(), types.f32());
        },
        [&](const core::type::DepthMultisampledTexture* depth) {
            return types.Get<core::type::MultisampledTexture>(depth->Dim(), types.f32());
        },
        [&](const core::type::StorageTexture* st) -> const core::type::Type* {
            return types.Get<core::type::StorageTexture>(st->Dim(), st->TexelFormat(),
                                                         core::Access::kRead, st->Type());
        },

        // Both sampler types are the same in SPIR-V.
        [&](const core::type::Sampler* s) -> const core::type::Type* {
            if (s->IsComparison()) {
                return types.Get<core::type::Sampler>(core::type::SamplerKind::kSampler);
            }
            return s;
        },

        // Dedup a SampledImage if its underlying image will be deduped.
        [&](const type::SampledImage* si) -> const core::type::Type* {
            auto* img = DedupType(si->Image(), types);
            if (img != si->Image()) {
                return types.Get<type::SampledImage>(img);
            }
            return si;
        },

        [&](Default) { return ty; });
}

/// PIMPL class for SPIR-V writer
class Printer {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    /// @param options the printer options
    Printer(core::ir::Module& module, const Options& options)
        : ir_(module), b_(module), options_(options) {
        zero_init_workgroup_memory_ = !options.disable_workgroup_init &&
                                      options.use_zero_initialize_workgroup_memory_extension;
    }

    /// @returns the generated SPIR-V code on success, or failure
    Result<Output> Code() {
        if (auto res = Generate(); res != Success) {
            return res.Failure();
        }

        // Serialize the module into binary SPIR-V.
        BinaryWriter writer;
        writer.WriteHeader(module_.IdBound(), kWriterVersion);
        writer.WriteModule(module_);

        output_.spirv = std::move(writer.Result());
        return output_;
    }

  private:
    core::ir::Module& ir_;
    core::ir::Builder b_;
    Options options_;
    writer::Module module_;
    BinaryWriter writer_;

    Output output_;

    /// A function type used for an OpTypeFunction declaration.
    struct FunctionType {
        uint32_t return_type_id;
        Vector<uint32_t, 4> param_type_ids;

        /// @returns the hash code of the FunctionType
        tint::HashCode HashCode() const {
            auto hash = Hash(return_type_id);
            for (auto& p : param_type_ids) {
                hash = HashCombine(hash, p);
            }
            return hash;
        }

        /// Equality operator for FunctionType.
        bool operator==(const FunctionType& other) const {
            return (param_type_ids == other.param_type_ids) &&
                   (return_type_id == other.return_type_id);
        }
    };

    /// The map of types to their result IDs.
    Hashmap<const core::type::Type*, uint32_t, 8> types_;

    /// The map of function types to their result IDs.
    Hashmap<FunctionType, uint32_t, 8> function_types_;

    /// The map of constants to their result IDs.
    Hashmap<const core::constant::Value*, uint32_t, 16> constants_;

    /// The map of types to the result IDs of their OpConstantNull instructions.
    Hashmap<const core::type::Type*, uint32_t, 4> constant_nulls_;

    /// The map of types to the result IDs of their OpUndef instructions.
    Hashmap<const core::type::Type*, uint32_t, 4> undef_values_;

    /// The map of non-constant values to their result IDs.
    Hashmap<const core::ir::Value*, uint32_t, 8> values_;

    /// The map of blocks to the IDs of their label instructions.
    Hashmap<const core::ir::Block*, uint32_t, 8> block_labels_;

    /// The map of control instructions to the IDs of the label of their SPIR-V merge blocks.
    Hashmap<const core::ir::ControlInstruction*, uint32_t, 8> merge_block_labels_;

    /// The map of extended instruction set names to their result IDs.
    Hashmap<std::string_view, uint32_t, 2> imports_;

    /// The current function that is being emitted.
    Function current_function_;

    /// The merge block for the current if statement
    uint32_t if_merge_label_ = 0;

    /// The header block for the current loop statement
    uint32_t loop_header_label_ = 0;

    /// The merge block for the current loop statement
    uint32_t loop_merge_label_ = 0;

    /// The merge block for the current switch statement
    uint32_t switch_merge_label_ = 0;

    bool zero_init_workgroup_memory_ = false;

    /// Builds the SPIR-V from the IR
    Result<SuccessType> Generate() {
        auto valid = core::ir::ValidateAndDumpIfNeeded(ir_, "spirv.Printer");
        if (valid != Success) {
            return valid.Failure();
        }

        module_.PushCapability(SpvCapabilityShader);

        if (options_.use_vulkan_memory_model) {
            module_.PushExtension("SPV_KHR_vulkan_memory_model");
            module_.PushCapability(SpvCapabilityVulkanMemoryModelKHR);
            // Required for the `Device` scope on atomic operations
            module_.PushCapability(SpvCapabilityVulkanMemoryModelDeviceScopeKHR);
            module_.PushMemoryModel(spv::Op::OpMemoryModel, {U32Operand(SpvAddressingModelLogical),
                                                             U32Operand(SpvMemoryModelVulkanKHR)});
        } else {
            module_.PushMemoryModel(spv::Op::OpMemoryModel, {U32Operand(SpvAddressingModelLogical),
                                                             U32Operand(SpvMemoryModelGLSL450)});
        }

        // Emit module-scope declarations.
        EmitRootBlock(ir_.root_block);

        // Emit functions.
        for (core::ir::Function* func : ir_.functions) {
            auto res = EmitFunction(func);
            if (res != Success) {
                return res;
            }
        }

        return Success;
    }

    /// Convert a builtin to the corresponding SPIR-V enum value, taking into account the target
    /// address space. Adds any capabilities needed for the builtin.
    /// @param builtin the builtin to convert
    /// @param addrspace the address space the builtin is being used in
    /// @returns the enum value of the corresponding SPIR-V builtin
    uint32_t Builtin(core::BuiltinValue builtin, core::AddressSpace addrspace) {
        switch (builtin) {
            case core::BuiltinValue::kPointSize:
                return SpvBuiltInPointSize;
            case core::BuiltinValue::kCullDistance:
                return SpvBuiltInCullDistance;
            case core::BuiltinValue::kFragDepth:
                return SpvBuiltInFragDepth;
            case core::BuiltinValue::kFrontFacing:
                return SpvBuiltInFrontFacing;
            case core::BuiltinValue::kGlobalInvocationId:
                return SpvBuiltInGlobalInvocationId;
            case core::BuiltinValue::kInstanceIndex:
                return SpvBuiltInInstanceIndex;
            case core::BuiltinValue::kLocalInvocationId:
                return SpvBuiltInLocalInvocationId;
            case core::BuiltinValue::kLocalInvocationIndex:
                return SpvBuiltInLocalInvocationIndex;
            case core::BuiltinValue::kNumWorkgroups:
                return SpvBuiltInNumWorkgroups;
            case core::BuiltinValue::kPosition:
                if (addrspace == core::AddressSpace::kOut) {
                    // Vertex output.
                    return SpvBuiltInPosition;
                } else {
                    // Fragment input.
                    return SpvBuiltInFragCoord;
                }
            case core::BuiltinValue::kSampleIndex:
                module_.PushCapability(SpvCapabilitySampleRateShading);
                return SpvBuiltInSampleId;
            case core::BuiltinValue::kSampleMask:
                return SpvBuiltInSampleMask;
            case core::BuiltinValue::kSubgroupInvocationId:
                module_.PushCapability(SpvCapabilityGroupNonUniform);
                return SpvBuiltInSubgroupLocalInvocationId;
            case core::BuiltinValue::kSubgroupSize:
                module_.PushCapability(SpvCapabilityGroupNonUniform);
                return SpvBuiltInSubgroupSize;
            case core::BuiltinValue::kVertexIndex:
                return SpvBuiltInVertexIndex;
            case core::BuiltinValue::kWorkgroupId:
                return SpvBuiltInWorkgroupId;
            case core::BuiltinValue::kClipDistances:
                module_.PushCapability(SpvCapabilityClipDistance);
                return SpvBuiltInClipDistance;
            case core::BuiltinValue::kUndefined:
                return SpvBuiltInMax;
        }
        return SpvBuiltInMax;
    }

    /// Get the result ID of the constant `constant`, emitting its instruction if necessary.
    /// @param constant the constant to get the ID for
    /// @returns the result ID of the constant
    uint32_t Constant(core::ir::Constant* constant) {
        // If it is a literal operand, just return the value.
        if (auto* literal = constant->As<spirv::ir::LiteralOperand>()) {
            return literal->Value()->ValueAs<uint32_t>();
        }

        auto id = Constant(constant->Value());

        // Set the name for the SPIR-V result ID if provided in the module.
        PushName(id, constant);

        return id;
    }

    /// Get the result ID of the constant `constant`, emitting its instruction if necessary.
    /// @param constant the constant to get the ID for
    /// @returns the result ID of the constant
    uint32_t Constant(const core::constant::Value* constant) {
        return constants_.GetOrAdd(constant, [&] {
            auto* ty = constant->Type();

            // Use OpConstantNull for zero-valued composite constants.
            if (!ty->Is<core::type::Scalar>() && constant->AllZero()) {
                return ConstantNull(ty);
            }

            auto id = module_.NextId();
            Switch(
                ty,  //
                [&](const core::type::Bool*) {
                    module_.PushType(constant->ValueAs<bool>() ? spv::Op::OpConstantTrue
                                                               : spv::Op::OpConstantFalse,
                                     {Type(ty), id});
                },
                [&](const core::type::I32*) {
                    module_.PushType(spv::Op::OpConstant, {Type(ty), id, constant->ValueAs<u32>()});
                },
                [&](const core::type::U32*) {
                    module_.PushType(spv::Op::OpConstant,
                                     {Type(ty), id, U32Operand(constant->ValueAs<i32>())});
                },
                [&](const core::type::F32*) {
                    module_.PushType(spv::Op::OpConstant, {Type(ty), id, constant->ValueAs<f32>()});
                },
                [&](const core::type::F16*) {
                    module_.PushType(
                        spv::Op::OpConstant,
                        {Type(ty), id, U32Operand(constant->ValueAs<f16>().BitsRepresentation())});
                },
                [&](const core::type::Vector* vec) {
                    OperandList operands = {Type(ty), id};
                    for (uint32_t i = 0; i < vec->Width(); i++) {
                        operands.push_back(Constant(constant->Index(i)));
                    }
                    module_.PushType(spv::Op::OpConstantComposite, operands);
                },
                [&](const core::type::Matrix* mat) {
                    OperandList operands = {Type(ty), id};
                    for (uint32_t i = 0; i < mat->Columns(); i++) {
                        operands.push_back(Constant(constant->Index(i)));
                    }
                    module_.PushType(spv::Op::OpConstantComposite, operands);
                },
                [&](const core::type::Array* arr) {
                    TINT_ASSERT(arr->ConstantCount());
                    OperandList operands = {Type(ty), id};
                    for (uint32_t i = 0; i < arr->ConstantCount(); i++) {
                        operands.push_back(Constant(constant->Index(i)));
                    }
                    module_.PushType(spv::Op::OpConstantComposite, operands);
                },
                [&](const core::type::Struct* str) {
                    OperandList operands = {Type(ty), id};
                    for (uint32_t i = 0; i < str->Members().Length(); i++) {
                        operands.push_back(Constant(constant->Index(i)));
                    }
                    module_.PushType(spv::Op::OpConstantComposite, operands);
                },  //
                TINT_ICE_ON_NO_MATCH);
            return id;
        });
    }

    /// Get the result ID of the OpConstantNull instruction for `type`, emitting it if necessary.
    /// @param type the type to get the ID for
    /// @returns the result ID of the OpConstantNull instruction
    uint32_t ConstantNull(const core::type::Type* type) {
        return constant_nulls_.GetOrAdd(type, [&] {
            auto id = module_.NextId();
            module_.PushType(spv::Op::OpConstantNull, {Type(type), id});
            return id;
        });
    }

    /// Get the result ID of the OpUndef instruction with type `ty`, emitting it if necessary.
    /// @param type the type of the undef value
    /// @returns the result ID of the instruction
    uint32_t Undef(const core::type::Type* type) {
        return undef_values_.GetOrAdd(type, [&] {
            auto id = module_.NextId();
            module_.PushType(spv::Op::OpUndef, {Type(type), id});
            return id;
        });
    }

    /// Get the result ID of the type `ty`, emitting a type declaration instruction if necessary.
    /// @param ty the type to get the ID for
    /// @returns the result ID of the type
    uint32_t Type(const core::type::Type* ty) {
        ty = DedupType(ty, ir_.Types());
        return types_.GetOrAdd(ty, [&] {
            auto id = module_.NextId();
            Switch(
                ty,  //
                [&](const core::type::Void*) { module_.PushType(spv::Op::OpTypeVoid, {id}); },
                [&](const core::type::Bool*) { module_.PushType(spv::Op::OpTypeBool, {id}); },
                [&](const core::type::I32*) {
                    module_.PushType(spv::Op::OpTypeInt, {id, 32u, 1u});
                },
                [&](const core::type::U32*) {
                    module_.PushType(spv::Op::OpTypeInt, {id, 32u, 0u});
                },
                [&](const core::type::F32*) {
                    module_.PushType(spv::Op::OpTypeFloat, {id, 32u});
                },
                [&](const core::type::F16*) {
                    module_.PushCapability(SpvCapabilityFloat16);
                    module_.PushCapability(SpvCapabilityUniformAndStorageBuffer16BitAccess);
                    module_.PushCapability(SpvCapabilityStorageBuffer16BitAccess);
                    module_.PushType(spv::Op::OpTypeFloat, {id, 16u});
                },
                [&](const core::type::Vector* vec) {
                    module_.PushType(spv::Op::OpTypeVector, {id, Type(vec->Type()), vec->Width()});
                },
                [&](const core::type::Matrix* mat) {
                    module_.PushType(spv::Op::OpTypeMatrix,
                                     {id, Type(mat->ColumnType()), mat->Columns()});
                },
                [&](const core::type::Array* arr) {
                    if (arr->ConstantCount()) {
                        auto* count = b_.ConstantValue(u32(arr->ConstantCount().value()));
                        module_.PushType(spv::Op::OpTypeArray,
                                         {id, Type(arr->ElemType()), Constant(count)});
                    } else {
                        TINT_ASSERT(arr->Count()->Is<core::type::RuntimeArrayCount>());
                        module_.PushType(spv::Op::OpTypeRuntimeArray, {id, Type(arr->ElemType())});
                    }
                    module_.PushAnnot(spv::Op::OpDecorate,
                                      {id, U32Operand(SpvDecorationArrayStride), arr->Stride()});
                },
                [&](const core::type::Pointer* ptr) {
                    module_.PushType(spv::Op::OpTypePointer,
                                     {id, U32Operand(StorageClass(ptr->AddressSpace())),
                                      Type(ptr->StoreType())});
                },
                [&](const core::type::Struct* str) { EmitStructType(id, str); },
                [&](const core::type::Texture* tex) { EmitTextureType(id, tex); },
                [&](const core::type::Sampler*) { module_.PushType(spv::Op::OpTypeSampler, {id}); },
                [&](const type::SampledImage* s) {
                    module_.PushType(spv::Op::OpTypeSampledImage, {id, Type(s->Image())});
                },  //
                [&](const core::type::SubgroupMatrix* sm) {
                    TINT_ASSERT(options_.use_vulkan_memory_model);
                    auto scope = Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup)));
                    auto cols = Constant(ir_.constant_values.Get(u32(sm->Columns())));
                    auto rows = Constant(ir_.constant_values.Get(u32(sm->Rows())));
                    spv::CooperativeMatrixUse use = spv::CooperativeMatrixUse::Max;
                    switch (sm->Kind()) {
                        case core::SubgroupMatrixKind::kLeft:
                            use = spv::CooperativeMatrixUse::MatrixAKHR;
                            break;
                        case core::SubgroupMatrixKind::kRight:
                            use = spv::CooperativeMatrixUse::MatrixBKHR;
                            break;
                        case core::SubgroupMatrixKind::kResult:
                            use = spv::CooperativeMatrixUse::MatrixAccumulatorKHR;
                            break;
                        case core::SubgroupMatrixKind::kUndefined:
                            TINT_UNREACHABLE();
                    }
                    module_.PushExtension("SPV_KHR_cooperative_matrix");
                    module_.PushCapability(SpvCapabilityCooperativeMatrixKHR);
                    module_.PushType(spv::Op::OpTypeCooperativeMatrixKHR,
                                     {
                                         id,
                                         Type(sm->Type()),
                                         scope,
                                         rows,
                                         cols,
                                         Constant(ir_.constant_values.Get(u32(use))),
                                     });
                },  //
                TINT_ICE_ON_NO_MATCH);
            return id;
        });
    }

    /// Get the result ID of the instruction result `value`, emitting its instruction if necessary.
    /// @param inst the instruction to get the ID for
    /// @returns the result ID of the instruction
    uint32_t Value(core::ir::Instruction* inst) { return Value(inst->Result(0)); }

    /// Get the result ID of the value `value`, emitting its instruction if necessary.
    /// @param value the value to get the ID for
    /// @returns the result ID of the value
    uint32_t Value(core::ir::Value* value) {
        return Switch(
            value,  //
            [&](core::ir::Constant* constant) { return Constant(constant); },
            [&](core::ir::Value*) {
                return values_.GetOrAdd(value, [&] { return module_.NextId(); });
            });
    }

    /// Get the ID of the label for `block`.
    /// @param block the block to get the label ID for
    /// @returns the ID of the block's label
    uint32_t Label(const core::ir::Block* block) {
        return block_labels_.GetOrAdd(block, [&] { return module_.NextId(); });
    }

    /// Emit a struct type.
    /// @param id the result ID to use
    /// @param str the struct type to emit
    void EmitStructType(uint32_t id, const core::type::Struct* str) {
        // Helper to return `type` or a potentially nested array element type within `type` as a
        // matrix type, or nullptr if no such matrix type is present.
        auto get_nested_matrix_type = [&](const core::type::Type* type) {
            while (auto* arr = type->As<core::type::Array>()) {
                type = arr->ElemType();
            }
            return type->As<core::type::Matrix>();
        };

        OperandList operands = {id};
        for (auto* member : str->Members()) {
            operands.push_back(Type(member->Type()));

            // Generate struct member offset decoration.
            module_.PushAnnot(
                spv::Op::OpMemberDecorate,
                {operands[0], member->Index(), U32Operand(SpvDecorationOffset), member->Offset()});

            // Emit matrix layout decorations if necessary.
            if (auto* matrix_type = get_nested_matrix_type(member->Type())) {
                const uint32_t effective_row_count = (matrix_type->Rows() == 2) ? 2 : 4;
                module_.PushAnnot(spv::Op::OpMemberDecorate,
                                  {id, member->Index(), U32Operand(SpvDecorationColMajor)});
                module_.PushAnnot(spv::Op::OpMemberDecorate,
                                  {id, member->Index(), U32Operand(SpvDecorationMatrixStride),
                                   Operand(effective_row_count * matrix_type->Type()->Size())});
            }

            PushMemberName(id, member->Index(), member->Name());
        }
        module_.PushType(spv::Op::OpTypeStruct, std::move(operands));

        // Add a Block decoration if necessary.
        if (str->StructFlags().Contains(core::type::StructFlag::kBlock)) {
            module_.PushAnnot(spv::Op::OpDecorate, {id, U32Operand(SpvDecorationBlock)});
        }

        PushName(id, str->Name());
    }

    /// Emit a texture type.
    /// @param id the result ID to use
    /// @param texture the texture type to emit
    void EmitTextureType(uint32_t id, const core::type::Texture* texture) {
        uint32_t sampled_type = Switch(
            texture,  //
            [&](const core::type::SampledTexture* t) { return Type(t->Type()); },
            [&](const core::type::MultisampledTexture* t) { return Type(t->Type()); },
            [&](const core::type::StorageTexture* t) { return Type(t->Type()); },
            [&](const core::type::InputAttachment* t) { return Type(t->Type()); },  //
            TINT_ICE_ON_NO_MATCH);

        uint32_t dim = SpvDimMax;
        uint32_t array = 0u;
        switch (texture->Dim()) {
            case core::type::TextureDimension::kNone: {
                break;
            }
            case core::type::TextureDimension::k1d: {
                dim = SpvDim1D;
                if (texture->Is<core::type::SampledTexture>()) {
                    module_.PushCapability(SpvCapabilitySampled1D);
                } else if (texture->Is<core::type::StorageTexture>()) {
                    module_.PushCapability(SpvCapabilityImage1D);
                }
                break;
            }
            case core::type::TextureDimension::k2d: {
                if (texture->Is<core::type::InputAttachment>()) {
                    module_.PushCapability(SpvCapabilityInputAttachment);
                    dim = SpvDimSubpassData;
                } else {
                    dim = SpvDim2D;
                }
                break;
            }
            case core::type::TextureDimension::k2dArray: {
                dim = SpvDim2D;
                array = 1u;
                break;
            }
            case core::type::TextureDimension::k3d: {
                dim = SpvDim3D;
                break;
            }
            case core::type::TextureDimension::kCube: {
                dim = SpvDimCube;
                break;
            }
            case core::type::TextureDimension::kCubeArray: {
                dim = SpvDimCube;
                array = 1u;
                if (texture->Is<core::type::SampledTexture>()) {
                    module_.PushCapability(SpvCapabilitySampledCubeArray);
                }
                break;
            }
        }

        // The Vulkan spec says: The "Depth" operand of OpTypeImage is ignored.
        // In SPIRV, 0 means not depth, 1 means depth, and 2 means unknown.
        // Using anything other than 0 is problematic on various Vulkan drivers.
        uint32_t depth = 0u;

        uint32_t ms = 0u;
        if (texture->Is<core::type::MultisampledTexture>()) {
            ms = 1u;
        }

        uint32_t sampled = 2u;
        if (texture->IsAnyOf<core::type::MultisampledTexture, core::type::SampledTexture>()) {
            sampled = 1u;
        }

        uint32_t format = SpvImageFormat_::SpvImageFormatUnknown;
        if (auto* st = texture->As<core::type::StorageTexture>()) {
            format = TexelFormat(st->TexelFormat());
        }

        module_.PushType(spv::Op::OpTypeImage,
                         {id, sampled_type, dim, depth, array, ms, sampled, format});
    }

    /// Emit a function.
    /// @param func the function to emit
    Result<SuccessType> EmitFunction(core::ir::Function* func) {
        if (func->Params().Length() > 255) {
            // Tint transforms may add additional function parameters which can cause a valid input
            // shader to exceed SPIR-V's function parameter limit. There isn't much we can do about
            // this, so just fail gracefully instead of a generating invalid SPIR-V.
            StringStream ss;
            ss << "Function '" << ir_.NameOf(func).Name()
               << "' has more than 255 parameters after running Tint transforms";
            return Failure{ss.str()};
        }

        auto id = Value(func);

        // Emit the function name.
        PushName(id, func);

        // Emit OpEntryPoint and OpExecutionMode declarations if needed.
        if (func->IsEntryPoint()) {
            EmitEntryPoint(func, id);
        }

        // Get the ID for the return type.
        auto return_type_id = Type(func->ReturnType());

        FunctionType function_type{return_type_id, {}};
        InstructionList params;

        // Generate function parameter declarations and add their type IDs to the function
        // signature.
        for (auto* param : func->Params()) {
            auto param_type_id = Type(param->Type());
            auto param_id = Value(param);
            params.push_back(Instruction(spv::Op::OpFunctionParameter, {param_type_id, param_id}));
            function_type.param_type_ids.Push(param_type_id);
            PushName(param_id, param);
        }

        // Get the ID for the function type (creating it if needed).
        auto function_type_id = function_types_.GetOrAdd(function_type, [&] {
            auto func_ty_id = module_.NextId();
            OperandList operands = {func_ty_id, return_type_id};
            operands.insert(operands.end(), function_type.param_type_ids.begin(),
                            function_type.param_type_ids.end());
            module_.PushType(spv::Op::OpTypeFunction, operands);
            return func_ty_id;
        });

        // Declare the function.
        auto decl = Instruction{
            spv::Op::OpFunction,
            {return_type_id, id, U32Operand(SpvFunctionControlMaskNone), function_type_id}};

        // Create a function that we will add instructions to.
        auto entry_block = module_.NextId();
        current_function_ = Function(decl, entry_block, std::move(params));
        TINT_DEFER(current_function_ = Function());

        // Emit the body of the function.
        EmitBlock(func->Block());

        // Add the function to the module.
        module_.PushFunction(current_function_);

        return Success;
    }

    /// Emit entry point declarations for a function.
    /// @param func the function to emit entry point declarations for
    /// @param id the result ID of the function declaration
    void EmitEntryPoint(core::ir::Function* func, uint32_t id) {
        SpvExecutionModel stage = SpvExecutionModelMax;
        switch (func->Stage()) {
            case core::ir::Function::PipelineStage::kCompute: {
                stage = SpvExecutionModelGLCompute;

                auto const_wg_size = func->WorkgroupSizeAsConst();
                TINT_ASSERT(const_wg_size);
                auto wg_size = *const_wg_size;

                // Store the workgroup information away to return from the generator.
                output_.workgroup_info.x = wg_size[0];
                output_.workgroup_info.y = wg_size[1];
                output_.workgroup_info.z = wg_size[2];

                module_.PushExecutionMode(
                    spv::Op::OpExecutionMode,
                    {id, U32Operand(SpvExecutionModeLocalSize), const_wg_size->at(0),
                     const_wg_size->at(1), const_wg_size->at(2)});
                break;
            }
            case core::ir::Function::PipelineStage::kFragment: {
                stage = SpvExecutionModelFragment;
                module_.PushExecutionMode(spv::Op::OpExecutionMode,
                                          {id, U32Operand(SpvExecutionModeOriginUpperLeft)});
                break;
            }
            case core::ir::Function::PipelineStage::kVertex: {
                stage = SpvExecutionModelVertex;
                break;
            }
            case core::ir::Function::PipelineStage::kUndefined:
                TINT_ICE() << "undefined pipeline stage for entry point";
        }

        // Use the remapped entry point name if requested, otherwise use the original name.
        std::string name;
        if (!options_.remapped_entry_point_name.empty()) {
            name = options_.remapped_entry_point_name;
        } else {
            name = ir_.NameOf(func).Name();
        }
        TINT_ASSERT(!name.empty());

        OperandList operands = {U32Operand(stage), id, name};

        // Add the list of all referenced shader IO variables.
        for (auto* global : *ir_.root_block) {
            auto* var = global->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
            if (!(ptr->AddressSpace() == core::AddressSpace::kIn ||
                  ptr->AddressSpace() == core::AddressSpace::kOut)) {
                continue;
            }

            // Determine if this IO variable is used by the entry point.
            bool used = false;
            for (const auto& use : var->Result(0)->UsagesUnsorted()) {
                auto* block = use->instruction->Block();
                while (block->Parent()) {
                    block = block->Parent()->Block();
                }
                if (block == func->Block()) {
                    used = true;
                    break;
                }
            }
            if (!used) {
                continue;
            }
            operands.push_back(Value(var));

            // Add the `DepthReplacing` execution mode if `frag_depth` is used.
            if (var->Attributes().builtin == core::BuiltinValue::kFragDepth) {
                module_.PushExecutionMode(spv::Op::OpExecutionMode,
                                          {id, U32Operand(SpvExecutionModeDepthReplacing)});
            }
        }

        module_.PushEntryPoint(spv::Op::OpEntryPoint, operands);
    }

    /// Emit the root block.
    /// @param root_block the root block to emit
    void EmitRootBlock(core::ir::Block* root_block) {
        for (auto* inst : *root_block) {
            Switch(
                inst,                                         //
                [&](core::ir::Var* v) { EmitGlobalVar(v); },  //
                TINT_ICE_ON_NO_MATCH);
        }
    }

    /// Emit a block, including the initial OpLabel, OpPhis and instructions.
    /// @param block the block to emit
    void EmitBlock(core::ir::Block* block) {
        // Emit the label.
        // Skip if this is the function's entry block, as it will be emitted by the function object.
        if (!current_function_.Instructions().empty()) {
            current_function_.PushInst(spv::Op::OpLabel, {Label(block)});
        }

        // If there are no instructions in the block, it's a dead end, so we shouldn't be able to
        // get here to begin with.
        if (block->IsEmpty()) {
            if (!block->Parent()->Results().IsEmpty()) {
                current_function_.PushInst(spv::Op::OpBranch, {GetMergeLabel(block->Parent())});
            } else {
                current_function_.PushInst(spv::Op::OpUnreachable, {});
            }
            return;
        }

        if (auto* mib = block->As<core::ir::MultiInBlock>()) {
            // Emit all OpPhi nodes for incoming branches to block.
            EmitIncomingPhis(mib);
        }

        // Emit the block's statements.
        EmitBlockInstructions(block);
    }

    /// Emit all OpPhi nodes for incoming branches to @p block.
    /// @param block the block to emit the OpPhis for
    void EmitIncomingPhis(core::ir::MultiInBlock* block) {
        // Emit Phi nodes for all the incoming block parameters
        for (size_t param_idx = 0; param_idx < block->Params().Length(); param_idx++) {
            auto* param = block->Params()[param_idx];
            OperandList ops{Type(param->Type()), Value(param)};

            for (auto* incoming : block->InboundSiblingBranches()) {
                auto* arg = incoming->Args()[param_idx];
                ops.push_back(Value(arg));
                ops.push_back(GetTerminatorBlockLabel(incoming));
            }

            current_function_.PushInst(spv::Op::OpPhi, std::move(ops));
        }
    }

    /// Emit all instructions of @p block.
    /// @param block the block's instructions to emit
    void EmitBlockInstructions(core::ir::Block* block) {
        TINT_ASSERT(!block->IsEmpty());
        for (auto* inst : *block) {
            Switch(
                inst,                                                                 //
                [&](core::ir::Access* a) { EmitAccess(a); },                          //
                [&](core::ir::Bitcast* b) { EmitBitcast(b); },                        //
                [&](core::ir::CoreBinary* b) { EmitBinary(b); },                      //
                [&](core::ir::CoreBuiltinCall* b) { EmitCoreBuiltinCall(b); },        //
                [&](spirv::ir::BuiltinCall* b) { EmitSpirvBuiltinCall(b); },          //
                [&](core::ir::Construct* c) { EmitConstruct(c); },                    //
                [&](core::ir::Convert* c) { EmitConvert(c); },                        //
                [&](core::ir::Load* l) { EmitLoad(l); },                              //
                [&](core::ir::LoadVectorElement* l) { EmitLoadVectorElement(l); },    //
                [&](core::ir::Loop* l) { EmitLoop(l); },                              //
                [&](core::ir::Switch* sw) { EmitSwitch(sw); },                        //
                [&](core::ir::Swizzle* s) { EmitSwizzle(s); },                        //
                [&](core::ir::Store* s) { EmitStore(s); },                            //
                [&](core::ir::StoreVectorElement* s) { EmitStoreVectorElement(s); },  //
                [&](core::ir::UserCall* c) { EmitUserCall(c); },                      //
                [&](core::ir::CoreUnary* u) { EmitUnary(u); },                        //
                [&](core::ir::Var* v) { EmitVar(v); },                                //
                [&](core::ir::Let* l) { EmitLet(l); },                                //
                [&](core::ir::If* i) { EmitIf(i); },                                  //
                [&](core::ir::Terminator* t) { EmitTerminator(t); },                  //
                [&](core::ir::Discard* t) { EmitDiscard(t); },                        //
                TINT_ICE_ON_NO_MATCH);

            // Set the name for the SPIR-V result ID if provided in the module.
            if (inst->Result(0) && !inst->Is<core::ir::Var>()) {
                PushName(Value(inst), inst);
            }
        }
    }

    void EmitDiscard(core::ir::Discard*) {
        if (options_.use_demote_to_helper_invocation_extensions) {
            module_.PushExtension("SPV_EXT_demote_to_helper_invocation");
            module_.PushCapability(SpvCapabilityDemoteToHelperInvocationEXT);
            current_function_.PushInst(spv::Op::OpDemoteToHelperInvocationEXT, {});
        } else {
            // OpKill does not have the same behavioral semantics as demote to helper and will not
            // be conformant. OpKill has also been deprecated and the alternative
            // OpTerminateInvocation also does not have demote to helper semantics.
            TINT_ICE() << "No substitute function for discard";
        }
    }

    /// Emit a terminator instruction.
    /// @param t the terminator instruction to emit
    void EmitTerminator(core::ir::Terminator* t) {
        tint::Switch(  //
            t,         //
            [&](core::ir::Return*) {
                if (!t->Args().IsEmpty()) {
                    TINT_ASSERT(t->Args().Length() == 1u);
                    OperandList operands;
                    operands.push_back(Value(t->Args()[0]));
                    current_function_.PushInst(spv::Op::OpReturnValue, operands);
                } else {
                    current_function_.PushInst(spv::Op::OpReturn, {});
                }
                return;
            },
            [&](core::ir::BreakIf* breakif) {
                current_function_.PushInst(spv::Op::OpBranchConditional,
                                           {
                                               Value(breakif->Condition()),
                                               loop_merge_label_,
                                               loop_header_label_,
                                           });
            },
            [&](core::ir::Continue* cont) {
                current_function_.PushInst(spv::Op::OpBranch, {Label(cont->Loop()->Continuing())});
            },
            [&](core::ir::ExitIf*) {
                current_function_.PushInst(spv::Op::OpBranch, {if_merge_label_});
            },
            [&](core::ir::ExitLoop*) {
                current_function_.PushInst(spv::Op::OpBranch, {loop_merge_label_});
            },
            [&](core::ir::ExitSwitch*) {
                current_function_.PushInst(spv::Op::OpBranch, {switch_merge_label_});
            },
            [&](core::ir::NextIteration*) {
                current_function_.PushInst(spv::Op::OpBranch, {loop_header_label_});
            },
            [&](core::ir::TerminateInvocation*) {
                current_function_.PushInst(spv::Op::OpKill, {});
            },
            [&](core::ir::Unreachable*) {
                current_function_.PushInst(spv::Op::OpUnreachable, {});
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Emit an `if` flow node.
    /// @param i the if node to emit
    void EmitIf(core::ir::If* i) {
        auto* true_block = i->True();
        auto* false_block = i->False();

        // Generate labels for the blocks. We emit the true or false block if it:
        // 1. contains instructions other then the branch, or
        // 2. branches somewhere instead of exiting the loop (e.g. return or break), or
        // 3. the if returns a value
        // Otherwise we skip them and branch straight to the merge block.
        uint32_t merge_label = GetMergeLabel(i);
        TINT_SCOPED_ASSIGNMENT(if_merge_label_, merge_label);

        uint32_t true_label = merge_label;
        uint32_t false_label = merge_label;
        if (true_block->Length() > 1 || !i->Results().IsEmpty() ||
            (true_block->Terminator() && !true_block->Terminator()->Is<core::ir::ExitIf>())) {
            true_label = Label(true_block);
        }
        if (false_block->Length() > 1 || !i->Results().IsEmpty() ||
            (false_block->Terminator() && !false_block->Terminator()->Is<core::ir::ExitIf>())) {
            false_label = Label(false_block);
        }

        // Emit the OpSelectionMerge and OpBranchConditional instructions.
        current_function_.PushInst(spv::Op::OpSelectionMerge,
                                   {merge_label, U32Operand(SpvSelectionControlMaskNone)});
        current_function_.PushInst(spv::Op::OpBranchConditional,
                                   {Value(i->Condition()), true_label, false_label});

        // Emit the `true` and `false` blocks, if they're not being skipped.
        if (true_label != merge_label) {
            EmitBlock(true_block);
        }
        if (false_label != merge_label) {
            EmitBlock(false_block);
        }

        current_function_.PushInst(spv::Op::OpLabel, {merge_label});

        // Emit the OpPhis for the ExitIfs
        EmitExitPhis(i);
    }

    /// Emit an access instruction
    /// @param access the access instruction to emit
    void EmitAccess(core::ir::Access* access) {
        auto* ty = access->Result(0)->Type();

        auto id = Value(access);
        OperandList operands = {Type(ty), id, Value(access->Object())};

        if (ty->Is<core::type::Pointer>()) {
            // Use OpAccessChain for accesses into pointer types.
            for (auto* idx : access->Indices()) {
                operands.push_back(Value(idx));
            }
            current_function_.PushInst(spv::Op::OpAccessChain, std::move(operands));
            return;
        }

        // For non-pointer types, we assume that the indices are constants and use
        // OpCompositeExtract. If we hit a non-constant index into a vector type, use
        // OpVectorExtractDynamic for it.
        auto* source_ty = access->Object()->Type();
        for (auto* idx : access->Indices()) {
            if (auto* constant = idx->As<core::ir::Constant>()) {
                // Push the index to the chain and update the current type.
                auto i = constant->Value()->ValueAs<u32>();
                operands.push_back(i);
                source_ty = source_ty->Element(i);
            } else {
                // The VarForDynamicIndex transform ensures that only value types that are vectors
                // will be dynamically indexed, as we can use OpVectorExtractDynamic for this case.
                TINT_ASSERT(source_ty->Is<core::type::Vector>());

                // If this wasn't the first access in the chain then emit the chain so far as an
                // OpCompositeExtract, creating a new result ID for the resulting vector.
                auto vec_id = Value(access->Object());
                if (operands.size() > 3) {
                    vec_id = module_.NextId();
                    operands[0] = Type(source_ty);
                    operands[1] = vec_id;
                    current_function_.PushInst(spv::Op::OpCompositeExtract, std::move(operands));
                }

                // Now emit the OpVectorExtractDynamic instruction.
                operands = {Type(ty), id, vec_id, Value(idx)};
                current_function_.PushInst(spv::Op::OpVectorExtractDynamic, std::move(operands));
                return;
            }
        }
        current_function_.PushInst(spv::Op::OpCompositeExtract, std::move(operands));
    }

    /// Emit a binary instruction.
    /// @param binary the binary instruction to emit
    void EmitBinary(core::ir::CoreBinary* binary) {
        auto id = Value(binary);
        auto lhs = Value(binary->LHS());
        auto rhs = Value(binary->RHS());
        auto* ty = binary->Result(0)->Type();
        auto* lhs_ty = binary->LHS()->Type();

        // Determine the opcode.
        spv::Op op = spv::Op::Max;
        switch (binary->Op()) {
            case core::BinaryOp::kAdd: {
                op = ty->IsIntegerScalarOrVector() ? spv::Op::OpIAdd : spv::Op::OpFAdd;
                break;
            }
            case core::BinaryOp::kDivide: {
                if (ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpSDiv;
                } else if (ty->IsUnsignedIntegerScalarOrVector()) {
                    op = spv::Op::OpUDiv;
                } else if (ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFDiv;
                }
                break;
            }
            case core::BinaryOp::kMultiply: {
                if (ty->IsIntegerScalarOrVector()) {
                    op = spv::Op::OpIMul;
                } else if (ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFMul;
                }
                break;
            }
            case core::BinaryOp::kSubtract: {
                op = ty->IsIntegerScalarOrVector() ? spv::Op::OpISub : spv::Op::OpFSub;
                break;
            }
            case core::BinaryOp::kModulo: {
                if (ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpSRem;
                } else if (ty->IsUnsignedIntegerScalarOrVector()) {
                    op = spv::Op::OpUMod;
                } else if (ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFRem;
                }
                break;
            }

            case core::BinaryOp::kAnd: {
                if (ty->IsIntegerScalarOrVector()) {
                    op = spv::Op::OpBitwiseAnd;
                } else if (ty->IsBoolScalarOrVector()) {
                    op = spv::Op::OpLogicalAnd;
                }
                break;
            }
            case core::BinaryOp::kOr: {
                if (ty->IsIntegerScalarOrVector()) {
                    op = spv::Op::OpBitwiseOr;
                } else if (ty->IsBoolScalarOrVector()) {
                    op = spv::Op::OpLogicalOr;
                }
                break;
            }
            case core::BinaryOp::kXor: {
                op = spv::Op::OpBitwiseXor;
                break;
            }

            case core::BinaryOp::kShiftLeft: {
                op = spv::Op::OpShiftLeftLogical;
                break;
            }
            case core::BinaryOp::kShiftRight: {
                if (ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpShiftRightArithmetic;
                } else if (ty->IsUnsignedIntegerScalarOrVector()) {
                    op = spv::Op::OpShiftRightLogical;
                }
                break;
            }

            case core::BinaryOp::kEqual: {
                if (lhs_ty->IsBoolScalarOrVector()) {
                    op = spv::Op::OpLogicalEqual;
                } else if (lhs_ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFOrdEqual;
                } else if (lhs_ty->IsIntegerScalarOrVector()) {
                    op = spv::Op::OpIEqual;
                }
                break;
            }
            case core::BinaryOp::kNotEqual: {
                if (lhs_ty->IsBoolScalarOrVector()) {
                    op = spv::Op::OpLogicalNotEqual;
                } else if (lhs_ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFOrdNotEqual;
                } else if (lhs_ty->IsIntegerScalarOrVector()) {
                    op = spv::Op::OpINotEqual;
                }
                break;
            }
            case core::BinaryOp::kGreaterThan: {
                if (lhs_ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFOrdGreaterThan;
                } else if (lhs_ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpSGreaterThan;
                } else if (lhs_ty->IsUnsignedIntegerScalarOrVector()) {
                    op = spv::Op::OpUGreaterThan;
                }
                break;
            }
            case core::BinaryOp::kGreaterThanEqual: {
                if (lhs_ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFOrdGreaterThanEqual;
                } else if (lhs_ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpSGreaterThanEqual;
                } else if (lhs_ty->IsUnsignedIntegerScalarOrVector()) {
                    op = spv::Op::OpUGreaterThanEqual;
                }
                break;
            }
            case core::BinaryOp::kLessThan: {
                if (lhs_ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFOrdLessThan;
                } else if (lhs_ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpSLessThan;
                } else if (lhs_ty->IsUnsignedIntegerScalarOrVector()) {
                    op = spv::Op::OpULessThan;
                }
                break;
            }
            case core::BinaryOp::kLessThanEqual: {
                if (lhs_ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFOrdLessThanEqual;
                } else if (lhs_ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpSLessThanEqual;
                } else if (lhs_ty->IsUnsignedIntegerScalarOrVector()) {
                    op = spv::Op::OpULessThanEqual;
                }
                break;
            }
            default:
                TINT_UNIMPLEMENTED() << binary->Op();
        }

        // Emit the instruction.
        current_function_.PushInst(op, {Type(ty), id, lhs, rhs});
    }

    /// Emit a bitcast instruction.
    /// @param bitcast the bitcast instruction to emit
    void EmitBitcast(core::ir::Bitcast* bitcast) {
        auto* ty = bitcast->Result(0)->Type();
        if (ty == bitcast->Val()->Type()) {
            values_.Add(bitcast->Result(0), Value(bitcast->Val()));
            return;
        }
        current_function_.PushInst(spv::Op::OpBitcast,
                                   {Type(ty), Value(bitcast), Value(bitcast->Val())});
    }

    /// Emit a builtin function call instruction.
    /// @param builtin the builtin call instruction to emit
    void EmitSpirvBuiltinCall(spirv::ir::BuiltinCall* builtin) {
        auto id = Value(builtin);

        OperandList operands;
        if (!builtin->Result(0)->Type()->Is<core::type::Void>()) {
            operands = {Type(builtin->Result(0)->Type()), id};
        }

        spv::Op op = spv::Op::Max;

        auto ext_inst = [&](enum GLSLstd450 inst) {
            op = spv::Op::OpExtInst;
            operands.push_back(ImportGlslStd450());
            operands.push_back(U32Operand(inst));
        };

        switch (builtin->Func()) {
            case spirv::BuiltinFn::kAbs:
                ext_inst(GLSLstd450SAbs);
                break;
            case spirv::BuiltinFn::kArrayLength:
                op = spv::Op::OpArrayLength;
                break;
            case spirv::BuiltinFn::kAtomicIadd:
                op = spv::Op::OpAtomicIAdd;
                break;
            case spirv::BuiltinFn::kAtomicIsub:
                op = spv::Op::OpAtomicISub;
                break;
            case spirv::BuiltinFn::kAtomicAnd:
                op = spv::Op::OpAtomicAnd;
                break;
            case spirv::BuiltinFn::kAtomicCompareExchange:
                op = spv::Op::OpAtomicCompareExchange;
                break;
            case spirv::BuiltinFn::kAtomicExchange:
                op = spv::Op::OpAtomicExchange;
                break;
            case spirv::BuiltinFn::kAtomicLoad:
                op = spv::Op::OpAtomicLoad;
                break;
            case spirv::BuiltinFn::kAtomicOr:
                op = spv::Op::OpAtomicOr;
                break;
            case spirv::BuiltinFn::kAtomicSmax:
                op = spv::Op::OpAtomicSMax;
                break;
            case spirv::BuiltinFn::kAtomicSmin:
                op = spv::Op::OpAtomicSMin;
                break;
            case spirv::BuiltinFn::kAtomicStore:
                op = spv::Op::OpAtomicStore;
                break;
            case spirv::BuiltinFn::kAtomicUmax:
                op = spv::Op::OpAtomicUMax;
                break;
            case spirv::BuiltinFn::kAtomicUmin:
                op = spv::Op::OpAtomicUMin;
                break;
            case spirv::BuiltinFn::kAtomicXor:
                op = spv::Op::OpAtomicXor;
                break;
            case spirv::BuiltinFn::kDot:
                op = spv::Op::OpDot;
                break;
            case spirv::BuiltinFn::kImageDrefGather:
                op = spv::Op::OpImageDrefGather;
                break;
            case spirv::BuiltinFn::kImageFetch:
                op = spv::Op::OpImageFetch;
                break;
            case spirv::BuiltinFn::kImageGather:
                op = spv::Op::OpImageGather;
                break;
            case spirv::BuiltinFn::kImageQuerySize:
                module_.PushCapability(SpvCapabilityImageQuery);
                op = spv::Op::OpImageQuerySize;
                break;
            case spirv::BuiltinFn::kImageQuerySizeLod:
                module_.PushCapability(SpvCapabilityImageQuery);
                op = spv::Op::OpImageQuerySizeLod;
                break;
            case spirv::BuiltinFn::kImageRead:
                op = spv::Op::OpImageRead;
                break;
            case spirv::BuiltinFn::kImageSampleImplicitLod:
                op = spv::Op::OpImageSampleImplicitLod;
                break;
            case spirv::BuiltinFn::kImageSampleExplicitLod:
                op = spv::Op::OpImageSampleExplicitLod;
                break;
            case spirv::BuiltinFn::kImageSampleDrefImplicitLod:
                op = spv::Op::OpImageSampleDrefImplicitLod;
                break;
            case spirv::BuiltinFn::kImageSampleDrefExplicitLod:
                op = spv::Op::OpImageSampleDrefExplicitLod;
                break;
            case spirv::BuiltinFn::kImageWrite:
                op = spv::Op::OpImageWrite;
                break;
            case spirv::BuiltinFn::kMatrixTimesMatrix:
                op = spv::Op::OpMatrixTimesMatrix;
                break;
            case spirv::BuiltinFn::kMatrixTimesScalar:
                op = spv::Op::OpMatrixTimesScalar;
                break;
            case spirv::BuiltinFn::kMatrixTimesVector:
                op = spv::Op::OpMatrixTimesVector;
                break;
            case spirv::BuiltinFn::kModf:
                ext_inst(GLSLstd450Modf);
                break;
            case spirv::BuiltinFn::kFrexp:
                ext_inst(GLSLstd450Frexp);
                break;
            case spirv::BuiltinFn::kSmax:
                ext_inst(GLSLstd450SMax);
                break;
            case spirv::BuiltinFn::kSmin:
                ext_inst(GLSLstd450SMin);
                break;
            case spirv::BuiltinFn::kSclamp:
                ext_inst(GLSLstd450SClamp);
                break;
            case spirv::BuiltinFn::kUmax:
                ext_inst(GLSLstd450UMax);
                break;
            case spirv::BuiltinFn::kUmin:
                ext_inst(GLSLstd450UMin);
                break;
            case spirv::BuiltinFn::kUclamp:
                ext_inst(GLSLstd450UClamp);
                break;
            case spirv::BuiltinFn::kNormalize:
                ext_inst(GLSLstd450Normalize);
                break;
            case spirv::BuiltinFn::kSampledImage:
                op = spv::Op::OpSampledImage;
                break;
            case spirv::BuiltinFn::kSdot:
                module_.PushExtension("SPV_KHR_integer_dot_product");
                module_.PushCapability(SpvCapabilityDotProductKHR);
                module_.PushCapability(SpvCapabilityDotProductInput4x8BitPackedKHR);
                op = spv::Op::OpSDot;
                break;
            case spirv::BuiltinFn::kSelect:
                op = spv::Op::OpSelect;
                break;
            case spirv::BuiltinFn::kSign:
                ext_inst(GLSLstd450SSign);
                break;
            case spirv::BuiltinFn::kInverse:
                ext_inst(GLSLstd450MatrixInverse);
                break;
            case spirv::BuiltinFn::kFindILsb:
                ext_inst(GLSLstd450FindILsb);
                break;
            case spirv::BuiltinFn::kFindSMsb:
                ext_inst(GLSLstd450FindSMsb);
                break;
            case spirv::BuiltinFn::kFindUMsb:
                ext_inst(GLSLstd450FindUMsb);
                break;
            case spirv::BuiltinFn::kRefract:
                ext_inst(GLSLstd450Refract);
                break;
            case spirv::BuiltinFn::kReflect:
                ext_inst(GLSLstd450Reflect);
                break;
            case spirv::BuiltinFn::kFaceForward:
                ext_inst(GLSLstd450FaceForward);
                break;
            case spirv::BuiltinFn::kLdexp:
                ext_inst(GLSLstd450Ldexp);
                break;
            case spirv::BuiltinFn::kUdot:
                module_.PushExtension("SPV_KHR_integer_dot_product");
                module_.PushCapability(SpvCapabilityDotProductKHR);
                module_.PushCapability(SpvCapabilityDotProductInput4x8BitPackedKHR);
                op = spv::Op::OpUDot;
                break;
            case spirv::BuiltinFn::kVectorTimesMatrix:
                op = spv::Op::OpVectorTimesMatrix;
                break;
            case spirv::BuiltinFn::kVectorTimesScalar:
                op = spv::Op::OpVectorTimesScalar;
                break;
            case spirv::BuiltinFn::kCooperativeMatrixLoad:
                op = spv::Op::OpCooperativeMatrixLoadKHR;
                break;
            case spirv::BuiltinFn::kCooperativeMatrixStore:
                op = spv::Op::OpCooperativeMatrixStoreKHR;
                break;
            case spirv::BuiltinFn::kCooperativeMatrixMulAdd:
                op = spv::Op::OpCooperativeMatrixMulAddKHR;
                break;
            case spirv::BuiltinFn::kNone:
                TINT_ICE() << "undefined spirv ir function";
        }

        for (auto* arg : builtin->Args()) {
            operands.push_back(Value(arg));
        }
        current_function_.PushInst(op, operands);
    }

    uint32_t ImportGlslStd450() {
        return imports_.GetOrAdd(kGLSLstd450, [&] {
            // Import the instruction set the first time it is requested.
            auto import = module_.NextId();
            module_.PushExtImport(spv::Op::OpExtInstImport, {import, Operand(kGLSLstd450)});
            return import;
        });
    }

    /// Emit a builtin function call instruction.
    /// @param builtin the builtin call instruction to emit
    void EmitCoreBuiltinCall(core::ir::CoreBuiltinCall* builtin) {
        auto* result_ty = builtin->Result(0)->Type();

        if (builtin->Func() == core::BuiltinFn::kAbs &&
            result_ty->IsUnsignedIntegerScalarOrVector()) {
            // abs() is a no-op for unsigned integers.
            values_.Add(builtin->Result(0), Value(builtin->Args()[0]));
            return;
        }
        if ((builtin->Func() == core::BuiltinFn::kAll ||
             builtin->Func() == core::BuiltinFn::kAny) &&
            builtin->Args()[0]->Type()->Is<core::type::Bool>()) {
            // all() and any() are passthroughs for scalar arguments.
            values_.Add(builtin->Result(0), Value(builtin->Args()[0]));
            return;
        }

        auto id = Value(builtin);

        spv::Op op = spv::Op::Max;
        OperandList operands = {Type(result_ty), id};

        // Helper to set up the opcode and operand list for a GLSL extended instruction.
        auto glsl_ext_inst = [&](enum GLSLstd450 inst) {
            op = spv::Op::OpExtInst;
            operands.push_back(ImportGlslStd450());
            operands.push_back(U32Operand(inst));
        };

        // TODO(crbug.com/359883869): Clean up this emission logic so this flag isn't necessary.
        bool args_emitted = false;

        // Determine the opcode.
        switch (builtin->Func()) {
            case core::BuiltinFn::kAbs:
                if (result_ty->IsFloatScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450FAbs);
                } else if (result_ty->IsSignedIntegerScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450SAbs);
                }
                break;
            case core::BuiltinFn::kAll:
                op = spv::Op::OpAll;
                break;
            case core::BuiltinFn::kAny:
                op = spv::Op::OpAny;
                break;
            case core::BuiltinFn::kAcos:
                glsl_ext_inst(GLSLstd450Acos);
                break;
            case core::BuiltinFn::kAcosh:
                glsl_ext_inst(GLSLstd450Acosh);
                break;
            case core::BuiltinFn::kAsin:
                glsl_ext_inst(GLSLstd450Asin);
                break;
            case core::BuiltinFn::kAsinh:
                glsl_ext_inst(GLSLstd450Asinh);
                break;
            case core::BuiltinFn::kAtan:
                glsl_ext_inst(GLSLstd450Atan);
                break;
            case core::BuiltinFn::kAtan2:
                glsl_ext_inst(GLSLstd450Atan2);
                break;
            case core::BuiltinFn::kAtanh:
                glsl_ext_inst(GLSLstd450Atanh);
                break;
            case core::BuiltinFn::kClamp:
                if (result_ty->IsFloatScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450NClamp);
                } else if (result_ty->IsUnsignedIntegerScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450UClamp);
                } else if (result_ty->IsSignedIntegerScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450SClamp);
                }
                break;
            case core::BuiltinFn::kCeil:
                glsl_ext_inst(GLSLstd450Ceil);
                break;
            case core::BuiltinFn::kCos:
                glsl_ext_inst(GLSLstd450Cos);
                break;
            case core::BuiltinFn::kCosh:
                glsl_ext_inst(GLSLstd450Cosh);
                break;
            case core::BuiltinFn::kCountOneBits:
                op = spv::Op::OpBitCount;
                break;
            case core::BuiltinFn::kCross:
                glsl_ext_inst(GLSLstd450Cross);
                break;
            case core::BuiltinFn::kDegrees:
                glsl_ext_inst(GLSLstd450Degrees);
                break;
            case core::BuiltinFn::kDeterminant:
                glsl_ext_inst(GLSLstd450Determinant);
                break;
            case core::BuiltinFn::kDistance:
                glsl_ext_inst(GLSLstd450Distance);
                break;
            case core::BuiltinFn::kDpdx:
                op = spv::Op::OpDPdx;
                break;
            case core::BuiltinFn::kDpdxCoarse:
                module_.PushCapability(SpvCapabilityDerivativeControl);
                op = spv::Op::OpDPdxCoarse;
                break;
            case core::BuiltinFn::kDpdxFine:
                module_.PushCapability(SpvCapabilityDerivativeControl);
                op = spv::Op::OpDPdxFine;
                break;
            case core::BuiltinFn::kDpdy:
                op = spv::Op::OpDPdy;
                break;
            case core::BuiltinFn::kDpdyCoarse:
                module_.PushCapability(SpvCapabilityDerivativeControl);
                op = spv::Op::OpDPdyCoarse;
                break;
            case core::BuiltinFn::kDpdyFine:
                module_.PushCapability(SpvCapabilityDerivativeControl);
                op = spv::Op::OpDPdyFine;
                break;
            case core::BuiltinFn::kExp:
                glsl_ext_inst(GLSLstd450Exp);
                break;
            case core::BuiltinFn::kExp2:
                glsl_ext_inst(GLSLstd450Exp2);
                break;
            case core::BuiltinFn::kExtractBits:
                op = result_ty->IsSignedIntegerScalarOrVector() ? spv::Op::OpBitFieldSExtract
                                                                : spv::Op::OpBitFieldUExtract;
                break;
            case core::BuiltinFn::kFaceForward:
                glsl_ext_inst(GLSLstd450FaceForward);
                break;
            case core::BuiltinFn::kFloor:
                glsl_ext_inst(GLSLstd450Floor);
                break;
            case core::BuiltinFn::kFma:
                glsl_ext_inst(GLSLstd450Fma);
                break;
            case core::BuiltinFn::kFract:
                glsl_ext_inst(GLSLstd450Fract);
                break;
            case core::BuiltinFn::kFrexp:
                glsl_ext_inst(GLSLstd450FrexpStruct);
                break;
            case core::BuiltinFn::kFwidth:
                op = spv::Op::OpFwidth;
                break;
            case core::BuiltinFn::kFwidthCoarse:
                module_.PushCapability(SpvCapabilityDerivativeControl);
                op = spv::Op::OpFwidthCoarse;
                break;
            case core::BuiltinFn::kFwidthFine:
                module_.PushCapability(SpvCapabilityDerivativeControl);
                op = spv::Op::OpFwidthFine;
                break;
            case core::BuiltinFn::kInsertBits:
                op = spv::Op::OpBitFieldInsert;
                break;
            case core::BuiltinFn::kInverseSqrt:
                glsl_ext_inst(GLSLstd450InverseSqrt);
                break;
            case core::BuiltinFn::kLdexp:
                glsl_ext_inst(GLSLstd450Ldexp);
                break;
            case core::BuiltinFn::kLength:
                glsl_ext_inst(GLSLstd450Length);
                break;
            case core::BuiltinFn::kLog:
                glsl_ext_inst(GLSLstd450Log);
                break;
            case core::BuiltinFn::kLog2:
                glsl_ext_inst(GLSLstd450Log2);
                break;
            case core::BuiltinFn::kMax:
                if (result_ty->IsFloatScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450FMax);
                } else if (result_ty->IsSignedIntegerScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450SMax);
                } else if (result_ty->IsUnsignedIntegerScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450UMax);
                }
                break;
            case core::BuiltinFn::kMin:
                if (result_ty->IsFloatScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450FMin);
                } else if (result_ty->IsSignedIntegerScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450SMin);
                } else if (result_ty->IsUnsignedIntegerScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450UMin);
                }
                break;
            case core::BuiltinFn::kMix:
                glsl_ext_inst(GLSLstd450FMix);
                break;
            case core::BuiltinFn::kModf:
                glsl_ext_inst(GLSLstd450ModfStruct);
                break;
            case core::BuiltinFn::kNormalize:
                glsl_ext_inst(GLSLstd450Normalize);
                break;
            case core::BuiltinFn::kPack2X16Float:
                glsl_ext_inst(GLSLstd450PackHalf2x16);
                break;
            case core::BuiltinFn::kPack2X16Snorm:
                glsl_ext_inst(GLSLstd450PackSnorm2x16);
                break;
            case core::BuiltinFn::kPack2X16Unorm:
                glsl_ext_inst(GLSLstd450PackUnorm2x16);
                break;
            case core::BuiltinFn::kPack4X8Snorm:
                glsl_ext_inst(GLSLstd450PackSnorm4x8);
                break;
            case core::BuiltinFn::kPack4X8Unorm:
                glsl_ext_inst(GLSLstd450PackUnorm4x8);
                break;
            case core::BuiltinFn::kPow:
                glsl_ext_inst(GLSLstd450Pow);
                break;
            case core::BuiltinFn::kQuadBroadcast:
                module_.PushCapability(SpvCapabilityGroupNonUniformQuad);
                op = spv::Op::OpGroupNonUniformQuadBroadcast;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kQuadSwapX:
                module_.PushCapability(SpvCapabilityGroupNonUniformQuad);
                op = spv::Op::OpGroupNonUniformQuadSwap;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(Value(builtin->Args()[0]));
                operands.push_back(Constant(ir_.constant_values.Get(0_u)));  // Direction
                args_emitted = true;
                break;
            case core::BuiltinFn::kQuadSwapY:
                module_.PushCapability(SpvCapabilityGroupNonUniformQuad);
                op = spv::Op::OpGroupNonUniformQuadSwap;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(Value(builtin->Args()[0]));
                operands.push_back(Constant(ir_.constant_values.Get(1_u)));  // Direction
                args_emitted = true;
                break;
            case core::BuiltinFn::kQuadSwapDiagonal:
                module_.PushCapability(SpvCapabilityGroupNonUniformQuad);
                op = spv::Op::OpGroupNonUniformQuadSwap;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(Value(builtin->Args()[0]));
                operands.push_back(Constant(ir_.constant_values.Get(2_u)));  // Direction
                args_emitted = true;
                break;
            case core::BuiltinFn::kQuantizeToF16:
                op = spv::Op::OpQuantizeToF16;
                break;
            case core::BuiltinFn::kRadians:
                glsl_ext_inst(GLSLstd450Radians);
                break;
            case core::BuiltinFn::kReflect:
                glsl_ext_inst(GLSLstd450Reflect);
                break;
            case core::BuiltinFn::kRefract:
                glsl_ext_inst(GLSLstd450Refract);
                break;
            case core::BuiltinFn::kReverseBits:
                op = spv::Op::OpBitReverse;
                break;
            case core::BuiltinFn::kRound:
                glsl_ext_inst(GLSLstd450RoundEven);
                break;
            case core::BuiltinFn::kSign:
                if (result_ty->IsFloatScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450FSign);
                } else if (result_ty->IsSignedIntegerScalarOrVector()) {
                    glsl_ext_inst(GLSLstd450SSign);
                }
                break;
            case core::BuiltinFn::kSin:
                glsl_ext_inst(GLSLstd450Sin);
                break;
            case core::BuiltinFn::kSinh:
                glsl_ext_inst(GLSLstd450Sinh);
                break;
            case core::BuiltinFn::kSmoothstep:
                glsl_ext_inst(GLSLstd450SmoothStep);
                break;
            case core::BuiltinFn::kSqrt:
                glsl_ext_inst(GLSLstd450Sqrt);
                break;
            case core::BuiltinFn::kStep:
                glsl_ext_inst(GLSLstd450Step);
                break;
            case core::BuiltinFn::kStorageBarrier: {
                spv::MemorySemanticsMask memory_mask = spv::MemorySemanticsMask::UniformMemory |
                                                       spv::MemorySemanticsMask::AcquireRelease;
                if (options_.use_vulkan_memory_model) {
                    memory_mask = memory_mask | spv::MemorySemanticsMask::MakeAvailable |
                                  spv::MemorySemanticsMask::MakeVisible;
                }

                op = spv::Op::OpControlBarrier;
                operands.clear();
                operands.push_back(Constant(b_.ConstantValue(u32(spv::Scope::Workgroup))));
                operands.push_back(Constant(b_.ConstantValue(u32(spv::Scope::Workgroup))));
                operands.push_back(Constant(b_.ConstantValue(u32(memory_mask))));
                break;
            }
            case core::BuiltinFn::kSubgroupAdd:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                op = result_ty->IsIntegerScalarOrVector() ? spv::Op::OpGroupNonUniformIAdd
                                                          : spv::Op::OpGroupNonUniformFAdd;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::Reduce)));
                break;
            case core::BuiltinFn::kSubgroupInclusiveAdd:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                op = result_ty->IsIntegerScalarOrVector() ? spv::Op::OpGroupNonUniformIAdd
                                                          : spv::Op::OpGroupNonUniformFAdd;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::InclusiveScan)));
                break;
            case core::BuiltinFn::kSubgroupExclusiveAdd:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                op = result_ty->IsIntegerScalarOrVector() ? spv::Op::OpGroupNonUniformIAdd
                                                          : spv::Op::OpGroupNonUniformFAdd;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::ExclusiveScan)));
                break;
            case core::BuiltinFn::kSubgroupMul:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                op = result_ty->IsIntegerScalarOrVector() ? spv::Op::OpGroupNonUniformIMul
                                                          : spv::Op::OpGroupNonUniformFMul;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::Reduce)));
                break;
            case core::BuiltinFn::kSubgroupInclusiveMul:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                op = result_ty->IsIntegerScalarOrVector() ? spv::Op::OpGroupNonUniformIMul
                                                          : spv::Op::OpGroupNonUniformFMul;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::InclusiveScan)));
                break;
            case core::BuiltinFn::kSubgroupExclusiveMul:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                op = result_ty->IsIntegerScalarOrVector() ? spv::Op::OpGroupNonUniformIMul
                                                          : spv::Op::OpGroupNonUniformFMul;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::ExclusiveScan)));
                break;
            case core::BuiltinFn::kSubgroupBallot:
                module_.PushCapability(SpvCapabilityGroupNonUniformBallot);
                op = spv::Op::OpGroupNonUniformBallot;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kSubgroupElect:
                module_.PushCapability(SpvCapabilityGroupNonUniform);
                op = spv::Op::OpGroupNonUniformElect;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kSubgroupBroadcast:
                module_.PushCapability(SpvCapabilityGroupNonUniformBallot);
                op = spv::Op::OpGroupNonUniformBroadcast;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kSubgroupBroadcastFirst:
                module_.PushCapability(SpvCapabilityGroupNonUniformBallot);
                op = spv::Op::OpGroupNonUniformBroadcastFirst;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kSubgroupShuffle:
                module_.PushCapability(SpvCapabilityGroupNonUniformShuffle);
                op = spv::Op::OpGroupNonUniformShuffle;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kSubgroupShuffleXor:
                module_.PushCapability(SpvCapabilityGroupNonUniformShuffle);
                op = spv::Op::OpGroupNonUniformShuffleXor;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kSubgroupShuffleUp:
                module_.PushCapability(SpvCapabilityGroupNonUniformShuffleRelative);
                op = spv::Op::OpGroupNonUniformShuffleUp;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kSubgroupShuffleDown:
                module_.PushCapability(SpvCapabilityGroupNonUniformShuffleRelative);
                op = spv::Op::OpGroupNonUniformShuffleDown;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kSubgroupAnd:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                op = spv::Op::OpGroupNonUniformBitwiseAnd;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::Reduce)));
                break;
            case core::BuiltinFn::kSubgroupOr:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                op = spv::Op::OpGroupNonUniformBitwiseOr;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::Reduce)));
                break;
            case core::BuiltinFn::kSubgroupXor:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                op = spv::Op::OpGroupNonUniformBitwiseXor;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::Reduce)));
                break;
            case core::BuiltinFn::kSubgroupMin:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                if (result_ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpGroupNonUniformFMin;
                } else if (result_ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpGroupNonUniformSMin;
                } else {
                    TINT_ASSERT(result_ty->IsUnsignedIntegerScalarOrVector());
                    op = spv::Op::OpGroupNonUniformUMin;
                }
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::Reduce)));
                break;
            case core::BuiltinFn::kSubgroupMax:
                module_.PushCapability(SpvCapabilityGroupNonUniformArithmetic);
                if (result_ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpGroupNonUniformFMax;
                } else if (result_ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpGroupNonUniformSMax;
                } else {
                    TINT_ASSERT(result_ty->IsUnsignedIntegerScalarOrVector());
                    op = spv::Op::OpGroupNonUniformUMax;
                }
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                operands.push_back(U32Operand(u32(spv::GroupOperation::Reduce)));
                break;
            case core::BuiltinFn::kSubgroupAll:
                module_.PushCapability(SpvCapabilityGroupNonUniformVote);
                op = spv::Op::OpGroupNonUniformAll;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kSubgroupAny:
                module_.PushCapability(SpvCapabilityGroupNonUniformVote);
                op = spv::Op::OpGroupNonUniformAny;
                operands.push_back(Constant(ir_.constant_values.Get(u32(spv::Scope::Subgroup))));
                break;
            case core::BuiltinFn::kTan:
                glsl_ext_inst(GLSLstd450Tan);
                break;
            case core::BuiltinFn::kTanh:
                glsl_ext_inst(GLSLstd450Tanh);
                break;
            case core::BuiltinFn::kTextureBarrier: {
                spv::MemorySemanticsMask memory_mask = spv::MemorySemanticsMask::ImageMemory |
                                                       spv::MemorySemanticsMask::AcquireRelease;
                if (options_.use_vulkan_memory_model) {
                    memory_mask = memory_mask | spv::MemorySemanticsMask::MakeAvailable |
                                  spv::MemorySemanticsMask::MakeVisible;
                }

                op = spv::Op::OpControlBarrier;
                operands.clear();
                operands.push_back(Constant(b_.ConstantValue(u32(spv::Scope::Workgroup))));
                operands.push_back(Constant(b_.ConstantValue(u32(spv::Scope::Workgroup))));
                operands.push_back(Constant(b_.ConstantValue(u32(memory_mask))));
                break;
            }
            case core::BuiltinFn::kTextureNumLevels:
                module_.PushCapability(SpvCapabilityImageQuery);
                op = spv::Op::OpImageQueryLevels;
                break;
            case core::BuiltinFn::kTextureNumSamples:
                module_.PushCapability(SpvCapabilityImageQuery);
                op = spv::Op::OpImageQuerySamples;
                break;
            case core::BuiltinFn::kTranspose:
                op = spv::Op::OpTranspose;
                break;
            case core::BuiltinFn::kTrunc:
                glsl_ext_inst(GLSLstd450Trunc);
                break;
            case core::BuiltinFn::kUnpack2X16Float:
                glsl_ext_inst(GLSLstd450UnpackHalf2x16);
                break;
            case core::BuiltinFn::kUnpack2X16Snorm:
                glsl_ext_inst(GLSLstd450UnpackSnorm2x16);
                break;
            case core::BuiltinFn::kUnpack2X16Unorm:
                glsl_ext_inst(GLSLstd450UnpackUnorm2x16);
                break;
            case core::BuiltinFn::kUnpack4X8Snorm:
                glsl_ext_inst(GLSLstd450UnpackSnorm4x8);
                break;
            case core::BuiltinFn::kUnpack4X8Unorm:
                glsl_ext_inst(GLSLstd450UnpackUnorm4x8);
                break;
            case core::BuiltinFn::kWorkgroupBarrier: {
                spv::MemorySemanticsMask memory_mask = spv::MemorySemanticsMask::WorkgroupMemory |
                                                       spv::MemorySemanticsMask::AcquireRelease;
                if (options_.use_vulkan_memory_model) {
                    memory_mask = memory_mask | spv::MemorySemanticsMask::MakeAvailable |
                                  spv::MemorySemanticsMask::MakeVisible;
                }

                op = spv::Op::OpControlBarrier;
                operands.clear();
                operands.push_back(Constant(b_.ConstantValue(u32(spv::Scope::Workgroup))));
                operands.push_back(Constant(b_.ConstantValue(u32(spv::Scope::Workgroup))));
                operands.push_back(Constant(b_.ConstantValue(u32(memory_mask))));
                break;
            }
            default:
                TINT_ICE() << "unimplemented builtin function: " << builtin->Func();
        }
        TINT_ASSERT(op != spv::Op::Max);

        // Add the arguments to the builtin call.
        if (!args_emitted) {
            for (auto* arg : builtin->Args()) {
                operands.push_back(Value(arg));
            }
        }

        // Emit the instruction.
        current_function_.PushInst(op, operands);
    }

    /// Emit a construct instruction.
    /// @param construct the construct instruction to emit
    void EmitConstruct(core::ir::Construct* construct) {
        auto* result_ty = construct->Result(0)->Type();

        // If there is just a single argument with the same type as the result, this is an identity
        // constructor and we can just pass through the ID of the argument.
        if (construct->Args().Length() == 1 && result_ty == construct->Args()[0]->Type()) {
            values_.Add(construct->Result(0), Value(construct->Args()[0]));
            return;
        }

        // Zero-value constructors for subgroup matrices are not folded into constants, so
        // special-case them into OpConstantNull here.
        if (result_ty->Is<core::type::SubgroupMatrix>() && construct->Operands().IsEmpty()) {
            values_.Add(construct->Result(0), ConstantNull(result_ty));
            return;
        }

        OperandList operands = {Type(result_ty), Value(construct)};
        for (auto* arg : construct->Args()) {
            operands.push_back(Value(arg));
        }
        current_function_.PushInst(spv::Op::OpCompositeConstruct, std::move(operands));
    }

    /// Emit a convert instruction.
    /// @param convert the convert instruction to emit
    void EmitConvert(core::ir::Convert* convert) {
        auto* res_ty = convert->Result(0)->Type();
        auto* arg_ty = convert->Args()[0]->Type();

        OperandList operands = {Type(convert->Result(0)->Type()), Value(convert)};
        for (auto* arg : convert->Args()) {
            operands.push_back(Value(arg));
        }

        spv::Op op = spv::Op::Max;
        if (res_ty->IsSignedIntegerScalarOrVector() && arg_ty->IsFloatScalarOrVector()) {
            // float to signed int.
            op = spv::Op::OpConvertFToS;
        } else if (res_ty->IsUnsignedIntegerScalarOrVector() && arg_ty->IsFloatScalarOrVector()) {
            // float to unsigned int.
            op = spv::Op::OpConvertFToU;
        } else if (res_ty->IsFloatScalarOrVector() && arg_ty->IsSignedIntegerScalarOrVector()) {
            // signed int to float.
            op = spv::Op::OpConvertSToF;
        } else if (res_ty->IsFloatScalarOrVector() && arg_ty->IsUnsignedIntegerScalarOrVector()) {
            // unsigned int to float.
            op = spv::Op::OpConvertUToF;
        } else if (res_ty->IsFloatScalarOrVector() && arg_ty->IsFloatScalarOrVector() &&
                   res_ty->Size() != arg_ty->Size()) {
            // float to float (different bitwidth).
            op = spv::Op::OpFConvert;
        } else if (res_ty->IsIntegerScalarOrVector() && arg_ty->IsIntegerScalarOrVector() &&
                   res_ty->Size() == arg_ty->Size()) {
            // int to int (same bitwidth, different signedness).
            op = spv::Op::OpBitcast;
        } else if (res_ty->IsBoolScalarOrVector()) {
            if (arg_ty->IsIntegerScalarOrVector()) {
                // int to bool.
                op = spv::Op::OpINotEqual;
            } else {
                // float to bool.
                op = spv::Op::OpFUnordNotEqual;
            }
            operands.push_back(ConstantNull(arg_ty));
        } else if (arg_ty->IsBoolScalarOrVector()) {
            // Select between constant one and zero, splatting them to vectors if necessary.
            core::ir::Constant* one = nullptr;
            core::ir::Constant* zero = nullptr;
            Switch(
                res_ty->DeepestElement(),  //
                [&](const core::type::F32*) {
                    one = b_.Constant(1_f);
                    zero = b_.Constant(0_f);
                },
                [&](const core::type::F16*) {
                    one = b_.Constant(1_h);
                    zero = b_.Constant(0_h);
                },
                [&](const core::type::I32*) {
                    one = b_.Constant(1_i);
                    zero = b_.Constant(0_i);
                },
                [&](const core::type::U32*) {
                    one = b_.Constant(1_u);
                    zero = b_.Constant(0_u);
                });
            TINT_ASSERT(one && zero);

            if (auto* vec = res_ty->As<core::type::Vector>()) {
                // Splat the scalars into vectors.
                one = b_.Splat(vec, one);
                zero = b_.Splat(vec, zero);
            }

            op = spv::Op::OpSelect;
            operands.push_back(Constant(b_.ConstantValue(one)));
            operands.push_back(Constant(b_.ConstantValue(zero)));
        } else {
            TINT_ICE() << "unhandled convert instruction";
        }

        current_function_.PushInst(op, std::move(operands));
    }

    SpvMemoryAccessMask MemoryAccessMaskForPointer(const core::type::Pointer* ptr) {
        TINT_ASSERT(ptr);

        if (options_.use_vulkan_memory_model &&
            (ptr->AddressSpace() == core::AddressSpace::kStorage ||
             ptr->AddressSpace() == core::AddressSpace::kWorkgroup) &&
            ptr->Access() == core::Access::kReadWrite) {
            return SpvMemoryAccessNonPrivatePointerMask;
        }

        return SpvMemoryAccessMaskNone;
    }

    /// Emit a load instruction.
    /// @param load the load instruction to emit
    void EmitLoad(core::ir::Load* load) {
        current_function_.PushInst(spv::Op::OpLoad,
                                   {Type(load->Result(0)->Type()), Value(load), Value(load->From()),
                                    U32Operand(MemoryAccessMaskForPointer(
                                        load->From()->Type()->As<core::type::Pointer>()))});
    }

    /// Emit a load vector element instruction.
    /// @param load the load vector element instruction to emit
    void EmitLoadVectorElement(core::ir::LoadVectorElement* load) {
        auto* vec_ptr_ty = load->From()->Type()->As<core::type::Pointer>();
        auto* el_ty = load->Result(0)->Type();
        auto* el_ptr_ty = ir_.Types().ptr(vec_ptr_ty->AddressSpace(), el_ty, vec_ptr_ty->Access());
        auto el_ptr_id = module_.NextId();
        current_function_.PushInst(
            spv::Op::OpAccessChain,
            {Type(el_ptr_ty), el_ptr_id, Value(load->From()), Value(load->Index())});
        current_function_.PushInst(spv::Op::OpLoad,
                                   {Type(load->Result(0)->Type()), Value(load), el_ptr_id,
                                    U32Operand(MemoryAccessMaskForPointer(vec_ptr_ty))});
    }

    /// Emit a loop instruction.
    /// @param loop the loop instruction to emit
    void EmitLoop(core::ir::Loop* loop) {
        auto init_label = loop->HasInitializer() ? Label(loop->Initializer()) : 0;
        auto body_label = Label(loop->Body());
        auto continuing_label = Label(loop->Continuing());

        auto header_label = module_.NextId();
        TINT_SCOPED_ASSIGNMENT(loop_header_label_, header_label);

        auto merge_label = GetMergeLabel(loop);
        TINT_SCOPED_ASSIGNMENT(loop_merge_label_, merge_label);

        if (init_label != 0) {
            // Emit the loop initializer.
            current_function_.PushInst(spv::Op::OpBranch, {init_label});
            EmitBlock(loop->Initializer());
        } else {
            // No initializer. Branch to body.
            current_function_.PushInst(spv::Op::OpBranch, {header_label});
        }

        // Emit the loop body header, which contains the OpLoopMerge and OpPhis.
        // This then unconditionally branches to body_label
        current_function_.PushInst(spv::Op::OpLabel, {header_label});
        EmitIncomingPhis(loop->Body());
        current_function_.PushInst(spv::Op::OpLoopMerge, {merge_label, continuing_label,
                                                          U32Operand(SpvLoopControlMaskNone)});
        current_function_.PushInst(spv::Op::OpBranch, {body_label});

        // Emit the loop body
        current_function_.PushInst(spv::Op::OpLabel, {body_label});
        EmitBlockInstructions(loop->Body());

        // Emit the loop continuing block.
        if (loop->Continuing()->Terminator()) {
            EmitBlock(loop->Continuing());
        } else {
            // We still need to emit a continuing block with a back-edge, even if it is unreachable.
            current_function_.PushInst(spv::Op::OpLabel, {continuing_label});
            current_function_.PushInst(spv::Op::OpBranch, {header_label});
        }

        // Emit the loop merge block.
        current_function_.PushInst(spv::Op::OpLabel, {merge_label});

        // Emit the OpPhis for the ExitLoops
        EmitExitPhis(loop);
    }

    /// Emit a switch instruction.
    /// @param swtch the switch instruction to emit
    void EmitSwitch(core::ir::Switch* swtch) {
        // Find the default selector. There must be exactly one.
        uint32_t default_label = 0u;
        for (auto& c : swtch->Cases()) {
            for (auto& sel : c.selectors) {
                if (sel.IsDefault()) {
                    default_label = Label(c.block);
                }
            }
        }
        TINT_ASSERT(default_label != 0u);

        // Build the operands to the OpSwitch instruction.
        OperandList switch_operands = {Value(swtch->Condition()), default_label};
        for (auto& c : swtch->Cases()) {
            auto label = Label(c.block);
            for (auto& sel : c.selectors) {
                if (sel.IsDefault()) {
                    continue;
                }
                switch_operands.push_back(sel.val->Value()->ValueAs<uint32_t>());
                switch_operands.push_back(label);
            }
        }

        uint32_t merge_label = GetMergeLabel(swtch);
        TINT_SCOPED_ASSIGNMENT(switch_merge_label_, merge_label);

        // Emit the OpSelectionMerge and OpSwitch instructions.
        current_function_.PushInst(spv::Op::OpSelectionMerge,
                                   {merge_label, U32Operand(SpvSelectionControlMaskNone)});
        current_function_.PushInst(spv::Op::OpSwitch, switch_operands);

        // Emit the cases.
        for (auto& c : swtch->Cases()) {
            EmitBlock(c.block);
        }

        // Emit the switch merge block.
        current_function_.PushInst(spv::Op::OpLabel, {merge_label});

        // Emit the OpPhis for the ExitSwitches
        EmitExitPhis(swtch);
    }

    /// Emit a swizzle instruction.
    /// @param swizzle the swizzle instruction to emit
    void EmitSwizzle(core::ir::Swizzle* swizzle) {
        auto id = Value(swizzle);
        auto obj = Value(swizzle->Object());
        OperandList operands = {Type(swizzle->Result(0)->Type()), id, obj, obj};
        for (auto idx : swizzle->Indices()) {
            operands.push_back(idx);
        }
        current_function_.PushInst(spv::Op::OpVectorShuffle, operands);
    }

    /// Emit a store instruction.
    /// @param store the store instruction to emit
    void EmitStore(core::ir::Store* store) {
        current_function_.PushInst(spv::Op::OpStore,
                                   {Value(store->To()), Value(store->From()),
                                    U32Operand(MemoryAccessMaskForPointer(
                                        store->To()->Type()->As<core::type::Pointer>()))});
    }

    /// Emit a store vector element instruction.
    /// @param store the store vector element instruction to emit
    void EmitStoreVectorElement(core::ir::StoreVectorElement* store) {
        auto* vec_ptr_ty = store->To()->Type()->As<core::type::Pointer>();
        auto* el_ty = store->Value()->Type();
        auto* el_ptr_ty = ir_.Types().ptr(vec_ptr_ty->AddressSpace(), el_ty, vec_ptr_ty->Access());
        auto el_ptr_id = module_.NextId();
        current_function_.PushInst(
            spv::Op::OpAccessChain,
            {Type(el_ptr_ty), el_ptr_id, Value(store->To()), Value(store->Index())});
        current_function_.PushInst(
            spv::Op::OpStore,
            {el_ptr_id, Value(store->Value()), U32Operand(MemoryAccessMaskForPointer(vec_ptr_ty))});
    }

    /// Emit a unary instruction.
    /// @param unary the unary instruction to emit
    void EmitUnary(core::ir::CoreUnary* unary) {
        auto id = Value(unary);
        auto* ty = unary->Result(0)->Type();
        spv::Op op = spv::Op::Max;
        switch (unary->Op()) {
            case core::UnaryOp::kComplement:
                op = spv::Op::OpNot;
                break;
            case core::UnaryOp::kNegation:
                if (ty->IsFloatScalarOrVector()) {
                    op = spv::Op::OpFNegate;
                } else if (ty->IsSignedIntegerScalarOrVector()) {
                    op = spv::Op::OpSNegate;
                }
                break;
            case core::UnaryOp::kNot:
                op = spv::Op::OpLogicalNot;
                break;
            default:
                TINT_UNIMPLEMENTED() << unary->Op();
        }
        current_function_.PushInst(op, {Type(ty), id, Value(unary->Val())});
    }

    /// Emit a user call instruction.
    /// @param call the user call instruction to emit
    void EmitUserCall(core::ir::UserCall* call) {
        auto id = Value(call);
        OperandList operands = {Type(call->Result(0)->Type()), id, Value(call->Target())};
        for (auto* arg : call->Args()) {
            operands.push_back(Value(arg));
        }
        current_function_.PushInst(spv::Op::OpFunctionCall, operands);
    }

    /// Emit IO attributes.
    /// @param id the ID of the variable to decorate
    /// @param attrs the shader IO attrs
    /// @param addrspace the address of the variable
    void EmitIOAttributes(uint32_t id,
                          const core::IOAttributes& attrs,
                          core::AddressSpace addrspace) {
        if (attrs.location) {
            module_.PushAnnot(spv::Op::OpDecorate,
                              {id, U32Operand(SpvDecorationLocation), *attrs.location});
        }
        if (attrs.blend_src) {
            module_.PushAnnot(spv::Op::OpDecorate,
                              {id, U32Operand(SpvDecorationIndex), *attrs.blend_src});
        }
        if (attrs.interpolation) {
            switch (attrs.interpolation->type) {
                case core::InterpolationType::kLinear:
                    module_.PushAnnot(spv::Op::OpDecorate,
                                      {id, U32Operand(SpvDecorationNoPerspective)});
                    break;
                case core::InterpolationType::kFlat:
                    module_.PushAnnot(spv::Op::OpDecorate, {id, U32Operand(SpvDecorationFlat)});
                    break;
                case core::InterpolationType::kPerspective:
                case core::InterpolationType::kUndefined:
                    break;
            }
            switch (attrs.interpolation->sampling) {
                case core::InterpolationSampling::kCentroid:
                    module_.PushAnnot(spv::Op::OpDecorate, {id, U32Operand(SpvDecorationCentroid)});
                    break;
                case core::InterpolationSampling::kSample:
                    module_.PushCapability(SpvCapabilitySampleRateShading);
                    module_.PushAnnot(spv::Op::OpDecorate, {id, U32Operand(SpvDecorationSample)});
                    break;
                case core::InterpolationSampling::kCenter:
                case core::InterpolationSampling::kFirst:
                case core::InterpolationSampling::kEither:
                case core::InterpolationSampling::kUndefined:
                    break;
            }
        }
        if (attrs.builtin) {
            module_.PushAnnot(spv::Op::OpDecorate, {id, U32Operand(SpvDecorationBuiltIn),
                                                    Builtin(*attrs.builtin, addrspace)});
        }
        if (attrs.invariant) {
            module_.PushAnnot(spv::Op::OpDecorate, {id, U32Operand(SpvDecorationInvariant)});
        }
    }

    void EmitGlobalVar(core::ir::Var* var) {
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kWorkgroup) {
            auto* ty = ptr->StoreType();
            uint32_t align = ty->Align();
            uint32_t size = ty->Size();

            // This essentially matches std430 layout rules from GLSL, which are in
            // turn specified as an upper bound for Vulkan layout sizing.
            output_.workgroup_info.storage_size += tint::RoundUp(16u, tint::RoundUp(align, size));
        }
        EmitVar(var);
    }

    /// Emit a var instruction.
    /// @param var the var instruction to emit
    void EmitVar(core::ir::Var* var) {
        auto id = Value(var);
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        auto* store_ty = ptr->StoreType();
        auto ty = Type(ptr);

        switch (ptr->AddressSpace()) {
            case core::AddressSpace::kFunction: {
                TINT_ASSERT(current_function_);
                if (var->Initializer()) {
                    current_function_.PushVar({ty, id, U32Operand(SpvStorageClassFunction)});
                    current_function_.PushInst(spv::Op::OpStore, {id, Value(var->Initializer())});
                } else {
                    current_function_.PushVar(
                        {ty, id, U32Operand(SpvStorageClassFunction), ConstantNull(store_ty)});
                }
                break;
            }
            case core::AddressSpace::kIn: {
                TINT_ASSERT(!current_function_);
                if (store_ty->DeepestElement()->Is<core::type::F16>()) {
                    module_.PushCapability(SpvCapabilityStorageInputOutput16);
                }
                module_.PushType(spv::Op::OpVariable, {ty, id, U32Operand(SpvStorageClassInput)});
                EmitIOAttributes(id, var->Attributes(), core::AddressSpace::kIn);
                break;
            }
            case core::AddressSpace::kPrivate: {
                TINT_ASSERT(!current_function_);
                OperandList operands = {ty, id, U32Operand(SpvStorageClassPrivate)};
                if (var->Initializer()) {
                    TINT_ASSERT(var->Initializer()->Is<core::ir::Constant>());
                    operands.push_back(Value(var->Initializer()));
                } else {
                    operands.push_back(ConstantNull(store_ty));
                }
                module_.PushType(spv::Op::OpVariable, operands);
                break;
            }
            case core::AddressSpace::kPushConstant: {
                TINT_ASSERT(!current_function_);
                module_.PushType(spv::Op::OpVariable,
                                 {ty, id, U32Operand(SpvStorageClassPushConstant)});
                break;
            }
            case core::AddressSpace::kOut: {
                TINT_ASSERT(!current_function_);
                if (store_ty->DeepestElement()->Is<core::type::F16>()) {
                    module_.PushCapability(SpvCapabilityStorageInputOutput16);
                }
                module_.PushType(spv::Op::OpVariable, {ty, id, U32Operand(SpvStorageClassOutput)});
                EmitIOAttributes(id, var->Attributes(), core::AddressSpace::kOut);
                break;
            }
            case core::AddressSpace::kHandle:
            case core::AddressSpace::kStorage:
            case core::AddressSpace::kUniform: {
                TINT_ASSERT(!current_function_);
                module_.PushType(spv::Op::OpVariable,
                                 {ty, id, U32Operand(StorageClass(ptr->AddressSpace()))});
                auto bp = var->BindingPoint().value();
                module_.PushAnnot(spv::Op::OpDecorate,
                                  {id, U32Operand(SpvDecorationDescriptorSet), bp.group});
                module_.PushAnnot(spv::Op::OpDecorate,
                                  {id, U32Operand(SpvDecorationBinding), bp.binding});

                // Add NonReadable and NonWritable decorations to storage textures and buffers.
                auto* st = store_ty->As<core::type::StorageTexture>();
                auto access = st ? st->Access() : ptr->Access();
                if (st || ptr->AddressSpace() != core::AddressSpace::kHandle) {
                    if (access == core::Access::kRead) {
                        module_.PushAnnot(spv::Op::OpDecorate,
                                          {id, U32Operand(SpvDecorationNonWritable)});
                    } else if (access == core::Access::kWrite) {
                        module_.PushAnnot(spv::Op::OpDecorate,
                                          {id, U32Operand(SpvDecorationNonReadable)});
                    }
                }
                if (!options_.use_vulkan_memory_model && access == core::Access::kReadWrite) {
                    module_.PushAnnot(spv::Op::OpDecorate, {id, U32Operand(SpvDecorationCoherent)});
                }

                auto iidx = var->InputAttachmentIndex();
                if (iidx) {
                    TINT_ASSERT(store_ty->Is<core::type::InputAttachment>());
                    module_.PushAnnot(
                        spv::Op::OpDecorate,
                        {id, U32Operand(SpvDecorationInputAttachmentIndex), iidx.value()});
                }
                break;
            }
            case core::AddressSpace::kWorkgroup: {
                TINT_ASSERT(!current_function_);
                OperandList operands = {ty, id, U32Operand(SpvStorageClassWorkgroup)};
                if (zero_init_workgroup_memory_) {
                    // If requested, use the VK_KHR_zero_initialize_workgroup_memory to
                    // zero-initialize the workgroup variable using an null constant initializer.
                    operands.push_back(ConstantNull(store_ty));
                }
                module_.PushType(spv::Op::OpVariable, operands);
                break;
            }
            default: {
                TINT_ICE() << "unimplemented variable address space " << ptr->AddressSpace();
            }
        }

        // Set the name if present.
        PushName(id, var);
    }

    /// Emit a let instruction.
    /// @param let the let instruction to emit
    void EmitLet(core::ir::Let* let) {
        auto id = Value(let->Value());
        values_.Add(let->Result(0), id);
    }

    /// Emit the OpPhis for the given flow control instruction.
    /// @param inst the flow control instruction
    void EmitExitPhis(core::ir::ControlInstruction* inst) {
        struct Branch {
            uint32_t label = 0;
            core::ir::Value* value = nullptr;
            bool operator<(const Branch& other) const { return label < other.label; }
        };

        auto results = inst->Results();
        for (size_t index = 0; index < results.Length(); index++) {
            auto* result = results[index];
            auto* ty = result->Type();

            Vector<Branch, 8> branches;
            branches.Reserve(inst->Exits().Count());
            for (auto& exit : inst->Exits()) {
                branches.Push(Branch{GetTerminatorBlockLabel(exit), exit->Args()[index]});
            }
            branches.Sort();  // Sort the branches by label to ensure deterministic output

            // Also add phi nodes from implicit exit blocks.
            if (inst->Is<core::ir::If>()) {
                inst->ForeachBlock([&](core::ir::Block* block) {
                    if (block->IsEmpty()) {
                        branches.Push(Branch{Label(block), nullptr});
                    }
                });
            }

            OperandList ops{Type(ty), Value(result)};
            for (auto& branch : branches) {
                if (branch.value == nullptr) {
                    ops.push_back(Undef(ty));
                } else {
                    ops.push_back(Value(branch.value));
                }
                ops.push_back(branch.label);
            }
            current_function_.PushInst(spv::Op::OpPhi, std::move(ops));
        }
    }

    /// Get the ID of the label of the merge block for a control instruction.
    /// @param ci the control instruction to get the merge label for
    /// @returns the label ID
    uint32_t GetMergeLabel(core::ir::ControlInstruction* ci) {
        return merge_block_labels_.GetOrAdd(ci, [&] { return module_.NextId(); });
    }

    /// Get the ID of the label of the block that will contain a terminator instruction.
    /// @param t the terminator instruction to get the block label for
    /// @returns the label ID
    uint32_t GetTerminatorBlockLabel(core::ir::Terminator* t) {
        // Walk backwards from `t` until we find a control instruction.
        auto* inst = t->prev.Get();
        while (inst) {
            auto* prev = inst->prev.Get();
            if (auto* ci = inst->As<core::ir::ControlInstruction>()) {
                // This is the last control instruction before `t`, so use its merge block label.
                return GetMergeLabel(ci);
            }
            inst = prev;
        }

        // There were no control instructions before `t`, so use the label of the parent block.
        return Label(t->Block());
    }

    /// Convert a texel format to the corresponding SPIR-V enum value, adding required capabilities.
    /// @param format the format to convert
    /// @returns the enum value of the corresponding SPIR-V texel format
    uint32_t TexelFormat(const core::TexelFormat format) {
        switch (format) {
            case core::TexelFormat::kBgra8Unorm:
                TINT_ICE() << "bgra8unorm should have been polyfilled to rgba8unorm";
            case core::TexelFormat::kR8Unorm:
                module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
                return SpvImageFormatR8;
            case core::TexelFormat::kR32Uint:
                return SpvImageFormatR32ui;
            case core::TexelFormat::kR32Sint:
                return SpvImageFormatR32i;
            case core::TexelFormat::kR32Float:
                return SpvImageFormatR32f;
            case core::TexelFormat::kRgba8Unorm:
                return SpvImageFormatRgba8;
            case core::TexelFormat::kRgba8Snorm:
                return SpvImageFormatRgba8Snorm;
            case core::TexelFormat::kRgba8Uint:
                return SpvImageFormatRgba8ui;
            case core::TexelFormat::kRgba8Sint:
                return SpvImageFormatRgba8i;
            case core::TexelFormat::kRg32Uint:
                module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
                return SpvImageFormatRg32ui;
            case core::TexelFormat::kRg32Sint:
                module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
                return SpvImageFormatRg32i;
            case core::TexelFormat::kRg32Float:
                module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
                return SpvImageFormatRg32f;
            case core::TexelFormat::kRgba16Uint:
                return SpvImageFormatRgba16ui;
            case core::TexelFormat::kRgba16Sint:
                return SpvImageFormatRgba16i;
            case core::TexelFormat::kRgba16Float:
                return SpvImageFormatRgba16f;
            case core::TexelFormat::kRgba32Uint:
                return SpvImageFormatRgba32ui;
            case core::TexelFormat::kRgba32Sint:
                return SpvImageFormatRgba32i;
            case core::TexelFormat::kRgba32Float:
                return SpvImageFormatRgba32f;
            case core::TexelFormat::kUndefined:
                return SpvImageFormatUnknown;
        }
        return SpvImageFormatUnknown;
    }

    /// Set the debug name of an instruction.
    void PushName(uint32_t id, core::ir::Instruction* inst) { PushName(id, ir_.NameOf(inst)); }
    /// Set the debug name of a value.
    void PushName(uint32_t id, core::ir::Value* value) { PushName(id, ir_.NameOf(value)); }
    /// Set the debug name for a SPIR-V ID.
    void PushName(uint32_t id, const Symbol& name) {
        // Only set the name if it is valid and if we are not stripping user identifiers.
        if (name && !options_.strip_all_names) {
            module_.PushDebug(spv::Op::OpName, {id, Operand(name.Name())});
        }
    }
    /// Set the debug member name for a SPIR-V ID.
    void PushMemberName(uint32_t id, uint32_t index, const Symbol& name) {
        // Only set the name if it is valid and if we are not stripping user identifiers.
        if (name && !options_.strip_all_names) {
            module_.PushDebug(spv::Op::OpMemberName, {id, index, Operand(name.Name())});
        }
    }
};

}  // namespace

tint::Result<Output> Print(core::ir::Module& module, const Options& options) {
    return Printer{module, options}.Code();
}

}  // namespace tint::spirv::writer
