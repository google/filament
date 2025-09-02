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

#include "src/tint/lang/spirv/reader/parser/parser.h"

#include <algorithm>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// This header is in an external dependency, so warnings cannot be fixed without upstream changes.
TINT_BEGIN_DISABLE_WARNING(NEWLINE_EOF);
TINT_BEGIN_DISABLE_WARNING(OLD_STYLE_CAST);
TINT_BEGIN_DISABLE_WARNING(SIGN_CONVERSION);
TINT_BEGIN_DISABLE_WARNING(WEAK_VTABLES);
TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
#include "source/opt/build_module.h"
#include "source/opt/resolve_binding_conflicts_pass.h"
#include "source/opt/split_combined_image_sampler_pass.h"
TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
TINT_END_DISABLE_WARNING(WEAK_VTABLES);
TINT_END_DISABLE_WARNING(SIGN_CONVERSION);
TINT_END_DISABLE_WARNING(OLD_STYLE_CAST);
TINT_END_DISABLE_WARNING(NEWLINE_EOF);

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/spirv/builtin_fn.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/type/explicit_layout_array.h"
#include "src/tint/lang/spirv/type/image.h"
#include "src/tint/lang/spirv/type/sampled_image.h"
#include "src/tint/lang/spirv/validate/validate.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::reader {

namespace {

// Stores information for operands which need to be calculated after a block is complete. Because
// a phi can store values which come after it, we can't calculate the value when the `OpPhi` is
// seen.
struct ReplacementValue {
    // The terminator instruction the operand belongs to
    core::ir::Terminator* terminator;
    // The operand index in `terminator` to replace
    uint32_t idx;
    // The SPIR-V value id to create the value from
    uint32_t value_id;
};

/// The SPIR-V environment that we validate against.
constexpr auto kTargetEnv = SPV_ENV_VULKAN_1_1;

/// PIMPL class for SPIR-V parser.
/// Validates the SPIR-V module and then parses it to produce a Tint IR module.
class Parser {
  public:
    explicit Parser(const Options& options) : options_(options) {}
    ~Parser() = default;

    /// @param spirv the SPIR-V binary data
    /// @returns the generated SPIR-V IR module on success, or failure
    Result<core::ir::Module> Run(Slice<const uint32_t> spirv) {
        // Validate the incoming SPIR-V binary.
        auto result = validate::Validate(spirv, kTargetEnv);
        if (result != Success) {
            return result.Failure();
        }

        // Build the SPIR-V tools internal representation of the SPIR-V module.
        spvtools::Context context(kTargetEnv);
        spirv_context_ =
            spvtools::BuildModule(kTargetEnv, context.CContext()->consumer, spirv.data, spirv.len);
        if (!spirv_context_) {
            return Failure("failed to build the internal representation of the module");
        }

        // Run SPIR-V opt transforms to make the input friendlier for the SPIR-V frontend.
        {
            spvtools::opt::SplitCombinedImageSamplerPass pass;
            auto status = pass.Run(spirv_context_.get());
            if (status == spvtools::opt::Pass::Status::Failure) {
                return Failure("failed to run SplitCombinedImageSamplerPass in SPIR-V opt");
            }
        }
        if (options_.sampler_mappings.empty()) {
            spvtools::opt::ResolveBindingConflictsPass pass;
            auto status = pass.Run(spirv_context_.get());
            if (status == spvtools::opt::Pass::Status::Failure) {
                return Failure("failed to run ResolveBindingConflictsPass in SPIR-V opt");
            }
        }

        // Check for unsupported extensions.
        for (const auto& ext : spirv_context_->extensions()) {
            auto name = ext.GetOperand(0).AsString();
            if (name != "SPV_KHR_storage_buffer_storage_class" &&
                name != "SPV_KHR_non_semantic_info" &&     //
                name != "SPV_KHR_terminate_invocation" &&  //
                // TODO(423644565): We assume the barriers are correct. We should check for any
                // operation that makes barrier assumptions that aren't consistent with WGSL and
                // generate the needed barriers.
                name != "SPV_KHR_vulkan_memory_model") {
                return Failure("SPIR-V extension '" + name + "' is not supported");
            }
        }

        // Register imported instruction sets
        for (const auto& import : spirv_context_->ext_inst_imports()) {
            auto name = import.GetInOperand(0).AsString();

            if (name == "GLSL.std.450") {
                glsl_std_450_imports_.insert(import.result_id());
            } else if (name.find("NonSemantic.") == 0) {
                ignored_imports_.insert(import.result_id());
            } else {
                return Failure("Unrecognized extended instruction set: " + name);
            }
        }

        RegisterNames();

        id_stack_.emplace_back();
        {
            TINT_SCOPED_ASSIGNMENT(current_block_, ir_.root_block);
            EmitSpecConstants();
            EmitModuleScopeVariables();
        }

        EmitFunctions();
        EmitEntryPointAttributes();

        RemapSamplers();
        RemapBufferBlockAddressSpace();
        AddRefToOutputsIfNeeded();

        return std::move(ir_);
    }

    // If the spir-v struct was marked as `BufferBlock` then we always treat this as a storage
    // address space. We do this as a post-pass because we have to propagate the change through all
    // the types that derive from the buffer block, e.g. any access needs to change from `Uniform`
    // to `Storage` address space.
    void RemapBufferBlockAddressSpace() {
        for (auto* inst : *ir_.root_block) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            TINT_ASSERT(ptr);

            auto* str = ptr->UnwrapPtr()->As<core::type::Struct>();
            if (!str) {
                continue;
            }
            if (!storage_buffer_types_.contains(str)) {
                continue;
            }
            auto iter = var_to_original_access_mode_.find(var);
            TINT_ASSERT(iter != var_to_original_access_mode_.end());
            auto access_mode = iter->second;

            // Handle the case of the struct members all being marked as `NonWritable`
            if (consider_non_writable_.contains(str)) {
                access_mode = core::Access::kRead;
            }

            var->Result()->SetType(ty_.ptr(core::AddressSpace::kStorage, str, access_mode));
            UpdateUsagesToStorageAddressSpace(var->Result(), access_mode);
        }
    }

    void UpdateUsagesToStorageAddressSpace(core::ir::Value* val, core::Access access_mode) {
        for (auto& usage : val->UsagesUnsorted()) {
            if (usage->instruction->Results().IsEmpty()) {
                continue;
            }

            auto* res = usage->instruction->Result();
            auto* ptr = res->Type()->As<core::type::Pointer>();
            if (!ptr) {
                continue;
            }
            TINT_ASSERT(ptr->AddressSpace() == core::AddressSpace::kUniform);

            res->SetType(ty_.ptr(core::AddressSpace::kStorage, ptr->StoreType(), access_mode));
            UpdateUsagesToStorageAddressSpace(res, access_mode);
        }
    }

    void AddRefToOutputsIfNeeded() {
        for (auto& entry_point : spirv_context_->module()->entry_points()) {
            uint32_t spv_id = entry_point.GetSingleWordInOperand(1);
            TINT_ASSERT(functions_.Contains(spv_id));

            auto* func = *(functions_.Get(spv_id));
            for (uint32_t i = 3; i < entry_point.NumInOperands(); ++i) {
                auto* val = Value(entry_point.GetSingleWordInOperand(i));
                b_.Phony(val)->InsertBefore(func->Block()->Front());
            }
        }
    }

    void RemapSamplers() {
        for (auto* inst : *ir_.root_block) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }
            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            TINT_ASSERT(ptr);

            if (!ptr->StoreType()->As<core::type::Sampler>()) {
                continue;
            }

            TINT_ASSERT(var->BindingPoint().has_value());

            auto bp = var->BindingPoint().value();
            auto used = used_bindings.GetOrAddZero(bp);

            // Only one use is the sampler itself.
            if (used == 1) {
                continue;
            }

            auto& binding = max_binding.GetOrAddZero(bp.group);
            binding += 1;

            var->SetBindingPoint(bp.group, binding);
        }
    }

    std::optional<uint16_t> GetSpecId(const spvtools::opt::Instruction& inst) {
        auto decos =
            spirv_context_->get_decoration_mgr()->GetDecorationsFor(inst.result_id(), true);
        for (const auto* deco_inst : decos) {
            TINT_ASSERT(deco_inst->opcode() == spv::Op::OpDecorate);

            if (deco_inst->GetSingleWordInOperand(1) ==
                static_cast<uint32_t>(spv::Decoration::SpecId)) {
                return {static_cast<uint16_t>(deco_inst->GetSingleWordInOperand(2))};
            }
        }
        return std::nullopt;
    }

    void CreateOverride(const spvtools::opt::Instruction& inst,
                        core::ir::Value* value,
                        std::optional<uint16_t> spec_id) {
        auto* override_ = b_.Override(Type(inst.type_id()));
        override_->SetInitializer(value);

        if (spec_id.has_value()) {
            override_->SetOverrideId(OverrideId{spec_id.value()});
        }

        Emit(override_, inst.result_id());

        Symbol name = GetSymbolFor(inst.result_id());
        if (name.IsValid()) {
            ir_.SetName(override_, name);
        }
    }

    // Generate a module-scope const declaration for each instruction
    // that is OpSpecConstantTrue, OpSpecConstantFalse, or OpSpecConstant.
    void EmitSpecConstants() {
        for (auto& inst : spirv_context_->types_values()) {
            switch (inst.opcode()) {
                case spv::Op::OpSpecConstantTrue:
                case spv::Op::OpSpecConstantFalse: {
                    auto* value = b_.Value(inst.opcode() == spv::Op::OpSpecConstantTrue);
                    auto spec_id = GetSpecId(inst);

                    if (spec_id.has_value()) {
                        CreateOverride(inst, value, spec_id);
                    } else {
                        // No spec_id means treat this as a constant.
                        AddValue(inst.result_id(), value);
                    }
                    break;
                }
                case spv::Op::OpSpecConstant: {
                    auto literal = inst.GetSingleWordInOperand(0);
                    auto* ty = spirv_context_->get_type_mgr()->GetType(inst.type_id());

                    auto* constant =
                        spirv_context_->get_constant_mgr()->GetConstant(ty, std::vector{literal});

                    core::ir::Value* value = tint::Switch(
                        Type(ty),  //
                        [&](const core::type::I32*) {
                            return b_.Constant(i32(constant->GetS32()));
                        },
                        [&](const core::type::U32*) {
                            return b_.Constant(u32(constant->GetU32()));
                        },
                        [&](const core::type::F32*) {
                            return b_.Constant(f32(constant->GetFloat()));
                        },
                        [&](const core::type::F16*) {
                            auto bits = constant->AsScalarConstant()->GetU32BitValue();
                            return b_.Constant(f16::FromBits(static_cast<uint16_t>(bits)));
                        },
                        TINT_ICE_ON_NO_MATCH);

                    auto spec_id = GetSpecId(inst);
                    CreateOverride(inst, value, spec_id);
                    break;
                }
                case spv::Op::OpSpecConstantOp: {
                    auto op = inst.GetSingleWordInOperand(0);

                    // Store the name away and remove it from the name list.
                    // This keeps any `Emit*` call in the switch below for
                    // gaining the name we want associated to the override.
                    std::string name;
                    auto iter = id_to_name_.find(inst.result_id());
                    if (iter != id_to_name_.end()) {
                        name = iter->second;
                        id_to_name_.erase(inst.result_id());
                    }

                    switch (static_cast<spv::Op>(op)) {
                        case spv::Op::OpBitwiseAnd:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kBitwiseAnd, 3);
                            break;
                        case spv::Op::OpBitwiseOr:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kBitwiseOr, 3);
                            break;
                        case spv::Op::OpBitwiseXor:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kBitwiseXor, 3);
                            break;
                        case spv::Op::OpIEqual:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kEqual, 3);
                            break;
                        case spv::Op::OpINotEqual:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kNotEqual, 3);
                            break;
                        case spv::Op::OpSGreaterThan:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSGreaterThan, 3);
                            break;
                        case spv::Op::OpSGreaterThanEqual:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSGreaterThanEqual, 3);
                            break;
                        case spv::Op::OpSLessThan:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSLessThan, 3);
                            break;
                        case spv::Op::OpSLessThanEqual:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSLessThanEqual, 3);
                            break;
                        case spv::Op::OpUGreaterThan:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kUGreaterThan, 3);
                            break;
                        case spv::Op::OpUGreaterThanEqual:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kUGreaterThanEqual, 3);
                            break;
                        case spv::Op::OpULessThan:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kULessThan, 3);
                            break;
                        case spv::Op::OpULessThanEqual:
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kULessThanEqual, 3);
                            break;
                        case spv::Op::OpLogicalAnd:
                            EmitBinary(inst, core::BinaryOp::kAnd, 3);
                            break;
                        case spv::Op::OpLogicalOr:
                            EmitBinary(inst, core::BinaryOp::kOr, 3);
                            break;
                        case spv::Op::OpLogicalNot:
                            EmitUnary(inst, core::UnaryOp::kNot, 3);
                            break;
                        case spv::Op::OpLogicalEqual:
                            EmitBinary(inst, core::BinaryOp::kEqual, 3);
                            break;
                        case spv::Op::OpLogicalNotEqual:
                            EmitBinary(inst, core::BinaryOp::kNotEqual, 3);
                            break;
                        case spv::Op::OpNot:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kNot, 3);
                            break;
                        case spv::Op::OpSConvert:
                            TINT_ICE() << "can't translate SConvert: WGSL does not have concrete "
                                          "integer types of different widths";
                        case spv::Op::OpUConvert:
                            TINT_ICE() << "can't translate UConvert: WGSL does not have concrete "
                                          "integer types of different widths";
                        case spv::Op::OpFConvert:
                            Emit(b_.Convert(Type(inst.type_id()),
                                            Value(inst.GetSingleWordInOperand(1))),
                                 inst.result_id());
                            break;
                        case spv::Op::OpSNegate:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSNegate, 3);
                            break;
                        case spv::Op::OpIAdd:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kAdd, 3);
                            break;
                        case spv::Op::OpISub:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSub, 3);
                            break;
                        case spv::Op::OpIMul:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kMul, 3);
                            break;
                        case spv::Op::OpSDiv:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSDiv, 3);
                            break;
                        case spv::Op::OpUDiv:
                            EmitBinary(inst, core::BinaryOp::kDivide, 3);
                            break;
                        case spv::Op::OpUMod:
                            EmitBinary(inst, core::BinaryOp::kModulo, 3);
                            break;
                        case spv::Op::OpSMod:
                        case spv::Op::OpSRem:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSMod, 3);
                            break;
                        case spv::Op::OpShiftLeftLogical:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kShiftLeftLogical,
                                                         3);
                            break;
                        case spv::Op::OpShiftRightLogical:
                            EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kShiftRightLogical,
                                                         3);
                            break;
                        case spv::Op::OpShiftRightArithmetic:
                            EmitSpirvExplicitBuiltinCall(
                                inst, spirv::BuiltinFn::kShiftRightArithmetic, 3);
                            break;
                        case spv::Op::OpCompositeExtract:
                            EmitCompositeExtract(inst, 3);
                            break;
                        case spv::Op::OpCompositeInsert:
                            TINT_ICE() << "can't translate OpSpecConstantOp with CompositeInsert: "
                                          "OpSpecConstantOp maps to a WGSL override declaration, "
                                          "but WGSL overrides must have scalar type";
                        case spv::Op::OpVectorShuffle:
                            TINT_ICE() << "can't translate OpSpecConstantOp with VectorShuffle: "
                                          "OpSpecConstantOp maps to a WGSL override declaration, "
                                          "but WGSL overrides must have scalar type";
                        case spv::Op::OpSelect:
                            if (!Type(inst.type_id())->IsScalar()) {
                                TINT_ICE()
                                    << "can't translate OpSpecConstantOp with Select that returns "
                                       "a vector: "
                                       "OpSpecConstantOp maps to a WGSL override declaration, "
                                       "but WGSL overrides must have scalar type";
                            }
                            EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSelect, 3);
                            break;
                        default:
                            TINT_ICE() << "Unknown spec constant operation: " << op;
                    }

                    // Restore the saved name, if any, in order to provide that
                    // name to the override.
                    if (!name.empty()) {
                        id_to_name_.insert({inst.result_id(), name});
                    }

                    CreateOverride(inst, Value(inst.result_id()), std::nullopt);
                    break;
                }
                case spv::Op::OpSpecConstantComposite: {
                    auto spec_id = GetSpecId(inst);
                    if (spec_id.has_value()) {
                        TINT_ICE()
                            << "OpSpecConstantCompositeOp not supported when set with a SpecId";
                    }

                    auto* cnst = SpvConstant(inst.result_id());
                    if (cnst != nullptr) {
                        // The spec constant is made of literals, so it's return as a constant from
                        // SPIR-V Tools Opt. We can just ignore it and let the normal constant
                        // handling take over.
                        break;
                    }

                    Vector<uint32_t, 4> args;
                    args.Reserve(inst.NumInOperands());

                    for (uint32_t i = 0; i < inst.NumInOperands(); ++i) {
                        uint32_t id = inst.GetSingleWordInOperand(i);
                        args.Push(id);
                    }

                    spec_composites_.insert({inst.result_id(), SpecComposite{
                                                                   .type = Type(inst.type_id()),
                                                                   .args = args,
                                                               }});
                    break;
                }
                default:
                    break;
            }
        }
    }

    void RegisterNames() {
        // Register names from OpName
        for (const auto& inst : spirv_context_->debugs2()) {
            switch (inst.opcode()) {
                case spv::Op::OpName: {
                    const auto name = inst.GetInOperand(1).AsString();
                    if (!name.empty()) {
                        id_to_name_[inst.GetSingleWordInOperand(0)] = name;
                    }
                    break;
                }
                case spv::Op::OpMemberName: {
                    const auto name = inst.GetInOperand(2).AsString();
                    if (!name.empty()) {
                        uint32_t struct_id = inst.GetSingleWordInOperand(0);
                        uint32_t member_idx = inst.GetSingleWordInOperand(1);
                        auto iter = struct_to_member_names_.insert({struct_id, {}});
                        auto& members = (*(iter.first)).second;

                        if (members.size() < (member_idx + 1)) {
                            members.resize(member_idx + 1);
                        }
                        members[member_idx] = name;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    /// @param sc a SPIR-V storage class
    /// @returns the Tint address space for a SPIR-V storage class
    core::AddressSpace AddressSpace(spv::StorageClass sc) {
        switch (sc) {
            case spv::StorageClass::Input:
                return core::AddressSpace::kIn;
            case spv::StorageClass::Output:
                return core::AddressSpace::kOut;
            case spv::StorageClass::Function:
                return core::AddressSpace::kFunction;
            case spv::StorageClass::Private:
                return core::AddressSpace::kPrivate;
            case spv::StorageClass::StorageBuffer:
                return core::AddressSpace::kStorage;
            case spv::StorageClass::Uniform:
                return core::AddressSpace::kUniform;
            case spv::StorageClass::UniformConstant:
                return core::AddressSpace::kHandle;
            case spv::StorageClass::Workgroup:
                return core::AddressSpace::kWorkgroup;
            default:
                TINT_UNIMPLEMENTED()
                    << "unhandled SPIR-V storage class: " << static_cast<uint32_t>(sc);
        }
    }

    /// @param b a SPIR-V BuiltIn
    /// @returns the Tint builtin value for a SPIR-V BuiltIn decoration
    core::BuiltinValue Builtin(spv::BuiltIn b) {
        switch (b) {
            case spv::BuiltIn::FragCoord:
                return core::BuiltinValue::kPosition;
            case spv::BuiltIn::FragDepth:
                return core::BuiltinValue::kFragDepth;
            case spv::BuiltIn::FrontFacing:
                return core::BuiltinValue::kFrontFacing;
            case spv::BuiltIn::GlobalInvocationId:
                return core::BuiltinValue::kGlobalInvocationId;
            case spv::BuiltIn::InstanceIndex:
                return core::BuiltinValue::kInstanceIndex;
            case spv::BuiltIn::LocalInvocationId:
                return core::BuiltinValue::kLocalInvocationId;
            case spv::BuiltIn::LocalInvocationIndex:
                return core::BuiltinValue::kLocalInvocationIndex;
            case spv::BuiltIn::NumWorkgroups:
                return core::BuiltinValue::kNumWorkgroups;
            case spv::BuiltIn::PointSize:
                return core::BuiltinValue::kPointSize;
            case spv::BuiltIn::Position:
                return core::BuiltinValue::kPosition;
            case spv::BuiltIn::SampleId:
                return core::BuiltinValue::kSampleIndex;
            case spv::BuiltIn::SampleMask:
                return core::BuiltinValue::kSampleMask;
            case spv::BuiltIn::SubgroupId:
                return core::BuiltinValue::kSubgroupId;
            case spv::BuiltIn::SubgroupSize:
                return core::BuiltinValue::kSubgroupSize;
            case spv::BuiltIn::SubgroupLocalInvocationId:
                return core::BuiltinValue::kSubgroupInvocationId;
            case spv::BuiltIn::VertexIndex:
                return core::BuiltinValue::kVertexIndex;
            case spv::BuiltIn::WorkgroupId:
                return core::BuiltinValue::kWorkgroupId;
            case spv::BuiltIn::ClipDistance:
                return core::BuiltinValue::kClipDistances;
            case spv::BuiltIn::CullDistance:
                return core::BuiltinValue::kCullDistance;
            default:
                TINT_UNIMPLEMENTED() << "unhandled SPIR-V BuiltIn: " << static_cast<uint32_t>(b);
        }
    }

    /// @param type a SPIR-V type object
    /// @param access_mode an optional access mode (for pointers)
    /// @returns a Tint type object
    const core::type::Type* Type(const spvtools::opt::analysis::Type* type,
                                 core::Access access_mode = core::Access::kUndefined) {
        // Only use the access mode for the map key if it is used as part of the type in Tint IR.
        auto key_mode = core::Access::kUndefined;
        if (type->kind() == spvtools::opt::analysis::Type::kImage ||
            type->kind() == spvtools::opt::analysis::Type::kPointer) {
            key_mode = access_mode;
        }

        return types_.GetOrAdd(TypeKey{type, key_mode}, [&]() -> const core::type::Type* {
            uint32_t array_stride = 0;
            bool set_as_storage_buffer = false;
            for (auto& deco : type->decorations()) {
                switch (spv::Decoration(deco[0])) {
                    case spv::Decoration::Block: {
                        // Ignore, just means it's a memory block.
                        break;
                    }
                    case spv::Decoration::BufferBlock: {
                        set_as_storage_buffer = true;
                        break;
                    }
                    case spv::Decoration::ArrayStride: {
                        array_stride = deco[1];
                        break;
                    }
                    default: {
                        TINT_UNIMPLEMENTED() << " unhandled type decoration " << deco[0];
                    }
                }
            }
            // Storage buffer is only set ons structs
            if (set_as_storage_buffer) {
                TINT_ASSERT(type->kind() == spvtools::opt::analysis::Type::kStruct);
            }
            // ArrayStride is only handled on the array type for now
            if (array_stride > 0) {
                TINT_ASSERT(type->kind() == spvtools::opt::analysis::Type::kArray ||
                            type->kind() == spvtools::opt::analysis::Type::kRuntimeArray);
            }

            switch (type->kind()) {
                case spvtools::opt::analysis::Type::kVoid: {
                    return ty_.void_();
                }
                case spvtools::opt::analysis::Type::kBool: {
                    return ty_.bool_();
                }
                case spvtools::opt::analysis::Type::kInteger: {
                    auto* int_ty = type->AsInteger();
                    TINT_ASSERT(int_ty->width() == 32);
                    if (int_ty->IsSigned()) {
                        return ty_.i32();
                    } else {
                        return ty_.u32();
                    }
                }
                case spvtools::opt::analysis::Type::kFloat: {
                    auto* float_ty = type->AsFloat();
                    if (float_ty->width() == 16) {
                        return ty_.f16();
                    } else if (float_ty->width() == 32) {
                        return ty_.f32();
                    } else {
                        TINT_UNREACHABLE()
                            << "unsupported floating point type width: " << float_ty->width();
                    }
                }
                case spvtools::opt::analysis::Type::kVector: {
                    auto* vec_ty = type->AsVector();
                    TINT_ASSERT(vec_ty->element_count() <= 4);
                    return ty_.vec(Type(vec_ty->element_type()), vec_ty->element_count());
                }
                case spvtools::opt::analysis::Type::kMatrix: {
                    auto* mat_ty = type->AsMatrix();
                    TINT_ASSERT(mat_ty->element_count() <= 4);
                    return ty_.mat(As<core::type::Vector>(Type(mat_ty->element_type())),
                                   mat_ty->element_count());
                }
                case spvtools::opt::analysis::Type::kArray: {
                    return EmitArray(type->AsArray(), array_stride);
                }
                case spvtools::opt::analysis::Type::kRuntimeArray: {
                    auto* arr_ty = type->AsRuntimeArray();

                    auto* elem_ty = Type(arr_ty->element_type());
                    uint32_t implicit_stride = tint::RoundUp(elem_ty->Align(), elem_ty->Size());
                    if (array_stride == 0 || array_stride == implicit_stride) {
                        return ty_.runtime_array(elem_ty);
                    }

                    return ty_.Get<spirv::type::ExplicitLayoutArray>(
                        elem_ty, ty_.Get<core::type::RuntimeArrayCount>(), elem_ty->Align(),
                        static_cast<uint32_t>(array_stride), array_stride);
                }
                case spvtools::opt::analysis::Type::kStruct: {
                    const core::type::Struct* str_ty = EmitStruct(type->AsStruct());
                    if (set_as_storage_buffer) {
                        storage_buffer_types_.insert(str_ty);
                    }
                    return str_ty;
                }
                case spvtools::opt::analysis::Type::kPointer: {
                    auto* ptr_ty = type->AsPointer();
                    auto* subtype = Type(ptr_ty->pointee_type(), access_mode);

                    // In a few cases we need to adjust the access mode.
                    //
                    // 1. Handle is always a read pointer
                    // 2. If the SPIR-V type should be considered NonWritable
                    if (subtype->IsHandle() || consider_non_writable_.contains(subtype)) {
                        access_mode = core::Access::kRead;
                    }

                    return ty_.ptr(AddressSpace(ptr_ty->storage_class()), subtype, access_mode);
                }
                case spvtools::opt::analysis::Type::kSampler: {
                    return ty_.sampler();
                }
                case spvtools::opt::analysis::Type::kImage: {
                    auto* img = type->AsImage();

                    auto* sampled_ty = Type(img->sampled_type());
                    auto dim = static_cast<type::Dim>(img->dim());
                    auto depth = static_cast<type::Depth>(img->depth());
                    auto arrayed =
                        img->is_arrayed() ? type::Arrayed::kArrayed : type::Arrayed::kNonArrayed;
                    auto ms = img->is_multisampled() ? type::Multisampled::kMultisampled
                                                     : type::Multisampled::kSingleSampled;
                    auto sampled = static_cast<type::Sampled>(img->sampled());
                    auto texel_format = ToTexelFormat(img->format());

                    // If the access mode is undefined then default to read/write for the image
                    access_mode = access_mode == core::Access::kUndefined ? core::Access::kReadWrite
                                                                          : access_mode;

                    if (img->dim() != spv::Dim::Dim1D && img->dim() != spv::Dim::Dim2D &&
                        img->dim() != spv::Dim::Dim3D && img->dim() != spv::Dim::Cube &&
                        img->dim() != spv::Dim::SubpassData) {
                        TINT_ICE() << "Unsupported texture dimension: "
                                   << static_cast<uint32_t>(img->dim());
                    }
                    if (img->sampled() == 0) {
                        TINT_ICE() << "Unsupported texture sample setting: Known at Runtime";
                    }

                    return ty_.Get<spirv::type::Image>(sampled_ty, dim, depth, arrayed, ms, sampled,
                                                       texel_format, access_mode);
                }
                case spvtools::opt::analysis::Type::kSampledImage: {
                    auto* sampled = type->AsSampledImage();
                    return ty_.Get<spirv::type::SampledImage>(Type(sampled->image_type()));
                }
                default: {
                    TINT_UNIMPLEMENTED() << "unhandled SPIR-V type: " << type->str();
                }
            }
        });
    }

    core::TexelFormat ToTexelFormat(spv::ImageFormat fmt) {
        switch (fmt) {
            case spv::ImageFormat::Unknown:
                return core::TexelFormat::kUndefined;

            // 8 bit channels
            case spv::ImageFormat::Rgba8:
                return core::TexelFormat::kRgba8Unorm;
            case spv::ImageFormat::Rgba8Snorm:
                return core::TexelFormat::kRgba8Snorm;
            case spv::ImageFormat::Rgba8ui:
                return core::TexelFormat::kRgba8Uint;
            case spv::ImageFormat::Rgba8i:
                return core::TexelFormat::kRgba8Sint;
            case spv::ImageFormat::R8:
                return core::TexelFormat::kR8Unorm;

            // 16 bit channels
            case spv::ImageFormat::Rgba16ui:
                return core::TexelFormat::kRgba16Uint;
            case spv::ImageFormat::Rgba16i:
                return core::TexelFormat::kRgba16Sint;
            case spv::ImageFormat::Rgba16f:
                return core::TexelFormat::kRgba16Float;

            // 32 bit channels
            case spv::ImageFormat::R32ui:
                return core::TexelFormat::kR32Uint;
            case spv::ImageFormat::R32i:
                return core::TexelFormat::kR32Sint;
            case spv::ImageFormat::R32f:
                return core::TexelFormat::kR32Float;
            case spv::ImageFormat::Rg32ui:
                return core::TexelFormat::kRg32Uint;
            case spv::ImageFormat::Rg32i:
                return core::TexelFormat::kRg32Sint;
            case spv::ImageFormat::Rg32f:
                return core::TexelFormat::kRg32Float;
            case spv::ImageFormat::Rgba32ui:
                return core::TexelFormat::kRgba32Uint;
            case spv::ImageFormat::Rgba32i:
                return core::TexelFormat::kRgba32Sint;
            case spv::ImageFormat::Rgba32f:
                return core::TexelFormat::kRgba32Float;
            default:
                break;
        }
        TINT_ICE() << "invalid image format: " << int(fmt);
    }

    /// @param id a SPIR-V result ID for a type declaration instruction
    /// @param access_mode an optional access mode (for pointers)
    /// @returns a Tint type object
    const core::type::Type* Type(uint32_t id, core::Access access_mode = core::Access::kUndefined) {
        return Type(spirv_context_->get_type_mgr()->GetType(id), access_mode);
    }

    /// @param arr_ty a SPIR-V array object
    /// @returns a Tint array object
    const core::type::Type* EmitArray(const spvtools::opt::analysis::Array* arr_ty,
                                      uint32_t array_stride) {
        const auto& length = arr_ty->length_info();
        TINT_ASSERT(!length.words.empty());
        if (length.words[0] != spvtools::opt::analysis::Array::LengthInfo::kConstant) {
            TINT_UNIMPLEMENTED() << "specialized array lengths";
        }

        // Get the value from the constant used for the element count.
        const auto* count_const =
            spirv_context_->get_constant_mgr()->FindDeclaredConstant(length.id);
        TINT_ASSERT(count_const);
        const uint64_t count_val = count_const->GetZeroExtendedValue();
        TINT_ASSERT(count_val <= UINT32_MAX);

        auto* elem_ty = Type(arr_ty->element_type());
        uint32_t implicit_stride = tint::RoundUp(elem_ty->Align(), elem_ty->Size());
        if (array_stride == 0 || array_stride == implicit_stride) {
            return ty_.array(elem_ty, static_cast<uint32_t>(count_val));
        }

        return ty_.Get<spirv::type::ExplicitLayoutArray>(
            elem_ty, ty_.Get<core::type::ConstantArrayCount>(static_cast<uint32_t>(count_val)),
            elem_ty->Align(), static_cast<uint32_t>(array_stride * count_val), array_stride);
    }

    /// Calculate the size of a struct member type that has a matrix stride decoration.
    uint32_t GetSizeOfTypeWithMatrixStride(const core::type::Type* type,
                                           uint32_t stride,
                                           bool is_row_major) {
        return tint::Switch(
            type,
            [&](const core::type::Matrix* mat) {
                return stride * (is_row_major ? mat->Rows() : mat->Columns());
            },
            [&](const core::type::Array* arr) {
                auto size = GetSizeOfTypeWithMatrixStride(arr->ElemType(), stride, is_row_major);
                // The size of a runtime-sized array is the size of a single element, so we only
                // have to handle the fixed-sized array case here.
                if (auto count = arr->ConstantCount()) {
                    size *= count.value();
                }
                return size;
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// @param struct_ty a SPIR-V struct object
    /// @returns a Tint struct object
    const core::type::Struct* EmitStruct(const spvtools::opt::analysis::Struct* struct_ty) {
        if (struct_ty->NumberOfComponents() == 0) {
            TINT_ICE() << "empty structures are not supported";
        }

        auto* type_mgr = spirv_context_->get_type_mgr();
        auto struct_id = type_mgr->GetId(struct_ty);

        std::vector<std::string>* member_names = nullptr;
        auto struct_to_member_iter = struct_to_member_names_.find(struct_id);
        if (struct_to_member_iter != struct_to_member_names_.end()) {
            member_names = &((*struct_to_member_iter).second);
        }

        // Build a list of struct members.
        uint32_t current_size = 0u;
        uint32_t member_count = static_cast<uint32_t>(struct_ty->NumberOfComponents());
        uint32_t non_writable_members = 0;
        Vector<core::type::StructMember*, 4> members;
        for (uint32_t i = 0; i < member_count; i++) {
            auto* member_ty = Type(struct_ty->element_types()[i]);
            uint32_t align = std::max<uint32_t>(member_ty->Align(), 1u);
            uint32_t offset = tint::RoundUp(align, current_size);
            core::IOAttributes attributes;
            auto interpolation = [&]() -> core::Interpolation& {
                // Create the interpolation field with the default values on first call.
                if (!attributes.interpolation.has_value()) {
                    attributes.interpolation =
                        core::Interpolation{core::InterpolationType::kPerspective,
                                            core::InterpolationSampling::kUndefined};
                }
                return attributes.interpolation.value();
            };

            bool is_row_major = false;
            uint32_t matrix_stride = 0;
            // Handle member decorations that affect layout or attributes.
            if (struct_ty->element_decorations().count(i)) {
                for (auto& deco : struct_ty->element_decorations().at(i)) {
                    switch (spv::Decoration(deco[0])) {
                        case spv::Decoration::NonWritable:
                            // WGSL doesn't have a non-writable attribute on struct members, but, if
                            // the SPIR-V structure has NonWritable on all members then we treat the
                            // entire structure as non-writable.
                            non_writable_members += 1;
                            break;

                        case spv::Decoration::ColMajor:          // Do nothing, WGSL is column major
                        case spv::Decoration::NonReadable:       // Not supported in WGSL
                        case spv::Decoration::RelaxedPrecision:  // Not supported in WGSL
                            break;
                        case spv::Decoration::Coherent:
                            // Tint has coherent memory semantics, so this is a no-op.
                            break;
                        case spv::Decoration::RowMajor:
                            is_row_major = true;
                            break;
                        case spv::Decoration::MatrixStride:
                            matrix_stride = deco[1];
                            break;
                        case spv::Decoration::Offset:
                            offset = deco[1];
                            break;
                        case spv::Decoration::BuiltIn:
                            attributes.builtin = Builtin(spv::BuiltIn(deco[1]));
                            break;
                        case spv::Decoration::Invariant:
                            attributes.invariant = true;
                            break;
                        case spv::Decoration::Location:
                            attributes.location = deco[1];
                            break;
                        case spv::Decoration::NoPerspective:
                            interpolation().type = core::InterpolationType::kLinear;
                            break;
                        case spv::Decoration::Flat:
                            interpolation().type = core::InterpolationType::kFlat;
                            break;
                        case spv::Decoration::Centroid:
                            interpolation().sampling = core::InterpolationSampling::kCentroid;
                            break;
                        case spv::Decoration::Sample:
                            interpolation().sampling = core::InterpolationSampling::kSample;
                            break;

                        default:
                            TINT_UNIMPLEMENTED() << "unhandled member decoration: " << deco[0];
                    }
                }
            }

            if (member_ty->IsIntegerScalarOrVector() && attributes.location.has_value()) {
                interpolation().type = core::InterpolationType::kFlat;
            }

            Symbol name;
            if (member_names && member_names->size() > i) {
                auto n = (*member_names)[i];
                if (!n.empty()) {
                    name = ir_.symbols.Register(n);
                }
            }
            if (!name.IsValid()) {
                name = ir_.symbols.New();
            }

            uint32_t size = member_ty->Size();
            if (matrix_stride > 0) {
                size = GetSizeOfTypeWithMatrixStride(member_ty, matrix_stride, is_row_major);
            }

            core::type::StructMember* member = ty_.Get<core::type::StructMember>(
                name, member_ty, i, offset, align, size, std::move(attributes));
            if (is_row_major) {
                member->SetRowMajor();
            }
            if (matrix_stride > 0) {
                member->SetMatrixStride(matrix_stride);
            }

            members.Push(member);

            current_size = offset + size;
        }

        Symbol name = GetUniqueSymbolFor(struct_id);
        if (!name.IsValid()) {
            name = ir_.symbols.New();
        }
        auto* strct = ty_.Struct(name, std::move(members));
        if (non_writable_members == member_count) {
            consider_non_writable_.insert(strct);
        }
        return strct;
    }

    Symbol GetUniqueSymbolFor(uint32_t id) {
        auto iter = id_to_name_.find(id);
        if (iter != id_to_name_.end()) {
            return ir_.symbols.New(iter->second);
        }
        return Symbol{};
    }

    Symbol GetSymbolFor(uint32_t id) {
        auto iter = id_to_name_.find(id);
        if (iter != id_to_name_.end()) {
            return ir_.symbols.Register(iter->second);
        }
        return Symbol{};
    }

    /// @param id a SPIR-V result ID for a function declaration instruction
    /// @returns a Tint function object
    core::ir::Function* Function(uint32_t id) {
        return functions_.GetOrAdd(id, [&] { return b_.Function(ty_.void_()); });
    }

    // Passes a value up through control flow to make it visible in an outer scope. Because SPIR-V
    // allows an id to be referenced as long as it's dominated, you can access a variable which is
    // defined inside an if branch for example. In order for that to be accessed in IR, we have to
    // propagate the value as a return of the control instruction (like the if).
    //
    // e.g. if the IR is similar to the following:
    // ```
    // if (b) {
    //    %a:i32 = let 4;
    //    exit_if
    // }
    // %c:i32 = %a + %a;
    // ```
    //
    // We propagate to something like:
    // ```
    // %d:i32 = if (b) {
    //   %a:i32 = let 4;  // The spir-v ID refers to %a at this point
    //   exit_if %a
    // }
    // %c:i32 = %d + %d  // The spir-v ID will now refer to %d instead of %a
    // ```
    //
    // We can end up propagating up through multiple levels, so we can end up with something like:
    // ```
    // %k:i32 = if (true) {
    //   %l:i32 = if (false) {
    //     %m:i32 = if (true) {
    //       %n:i32 = switch 4 {
    //         default: {
    //           %o:i32 = loop {
    //             %a:i32 = let 4;
    //             exit_loop %a
    //           }
    //           exit_switch %o
    //         }
    //       }
    //       exit_if %n
    //     }
    //     exit_if %m
    //   }
    //   exit_if %l
    // }
    // %b:i32 = %k + %k
    // ```
    //
    // @param id the spir-v ID to propagate up
    // @param src the source value being propagated
    core::ir::Value* Propagate(uint32_t id, core::ir::Value* src) {
        // Function params are always in scope so we should never need to propagate.
        if (src->Is<core::ir::FunctionParam>()) {
            return src;
        }

        auto* blk = tint::Switch(
            src,  //
            [&](core::ir::BlockParam* bp) { return bp->Block(); },
            [&](core::ir::InstructionResult* res) { return res->Instruction()->Block(); },
            TINT_ICE_ON_NO_MATCH);

        // Walk up the set of control instructions from the current `blk`. We'll update the `src`
        // instruction as the new result which is to be used for the given SPIR-V `id`. At each
        // control instruction we'll add the current `src` as a result of each exit from the control
        // instruction, making a new result which is available in the parent scope.
        while (blk) {
            if (InBlock(blk)) {
                break;
            }

            core::ir::ControlInstruction* ctrl = nullptr;
            if (auto* mb = blk->As<core::ir::MultiInBlock>()) {
                ctrl = mb->Parent()->As<core::ir::ControlInstruction>();
                TINT_ASSERT(ctrl);

                for (auto exit : ctrl->Exits()) {
                    tint::Switch(
                        exit.Value(),  //
                        [&](core::ir::ExitLoop* el) { el->PushOperand(src); },
                        [&](core::ir::BreakIf* bi) { bi->PushOperand(src); },  //
                        TINT_ICE_ON_NO_MATCH);
                }
            } else {
                TINT_ASSERT(blk->Terminator());

                ctrl = tint::Switch(
                    blk->Terminator(),  //
                    [&](core::ir::ExitIf* ei) { return ei->If(); },
                    [&](core::ir::ExitSwitch* es) { return es->Switch(); },
                    [&](core::ir::ExitLoop* el) { return el->Loop(); },
                    [&](core::ir::NextIteration* ni) {
                        ni->PushOperand(src);
                        return nullptr;
                    },
                    [&](core::ir::Continue* cont) {
                        // The propagation is going through a `continue`. This means
                        // this is the only path to the continuing block, but it also
                        // means we're current in the continuing block. We can't do
                        // normal propagation here, we have to pass a block param
                        // instead.

                        auto* param = b_.BlockParam(src->Type());

                        // We're in the continuing block, so make the block param available in the
                        // scope.
                        id_stack_.back().insert(id);

                        auto* loop = cont->Loop();
                        loop->Continuing()->AddParam(param);
                        cont->PushOperand(src);

                        // Set `src` as the `param` so it's returned as the new value
                        src = param;
                        return nullptr;
                    },                                                      //
                    [&](core::ir::Unreachable*) { return blk->Parent(); },  //
                    TINT_ICE_ON_NO_MATCH);

                if (!ctrl) {
                    break;
                }

                for (auto& exit : ctrl->Exits()) {
                    exit->PushOperand(src);
                }
            }

            // If this control has no exits, then we don't need to add the result through the
            // control as we're jumping over the control to its parent control. (This is an
            // `if` inside a `loop` where the `if` is doing a `break`).
            if (ctrl->Exits().IsEmpty()) {
                TINT_ASSERT(ctrl->Is<core::ir::If>());
                blk = ctrl->Block();
                continue;
            }

            // Add a new result to the control instruction
            ctrl->AddResult(b_.InstructionResult(src->Type()));
            // The source instruction is now the control result we just inserted
            src = ctrl->Results().Back();
            // The SPIR-V ID now refers to the propagated value.
            values_.Replace(id, src);

            blk = ctrl->Block();
        }
        return src;
    }

    // Returns true if the `blk` is the  `parent_blk` or any of the ancestors of parent_blk
    bool IsContainedInBlock(core::ir::Block* parent_blk, core::ir::Block* blk) {
        while (parent_blk) {
            if (parent_blk == blk) {
                return true;
            }

            if (!parent_blk->Parent()) {
                break;
            }
            parent_blk = parent_blk->Parent()->Block();
        }
        return false;
    }

    // Propagate a value out of a control instruction. This isn't the same as the other
    // propagate as we only need to pull it out through a single blocks exit for the control
    // instruction.
    core::ir::Value* PropagateTerm(core::ir::Terminator* term,
                                   uint32_t spv_id_being_propagated,
                                   core::ir::Value* source_value,
                                   core::ir::Block* source_block) {
        auto* current_ctrl = source_block->Parent();
        core::ir::Block* current_blk = source_block;
        core::ir::Value* propagated_value = source_value;
        auto* type = source_value->Type();

        while (true) {
            if (IsContainedInBlock(term->Block(), current_blk)) {
                break;
            }
            TINT_ASSERT(current_ctrl);

            // If the terminator is an exit for this control instruction, we're done as the caller
            // will handle adding the result.
            bool done = false;
            for (auto exit : current_ctrl->Exits()) {
                if (exit.Value() == term) {
                    done = true;
                    break;
                }
            }
            if (done) {
                break;
            }

            auto* res = b_.InstructionResult(type);
            current_ctrl->AddResult(res);

            for (auto exit : current_ctrl->Exits()) {
                if (IsContainedInBlock(exit->Block(), current_blk)) {
                    exit->PushOperand(propagated_value);
                    continue;
                }

                // Special handling for loop-continuing blocks as the body is visible to the
                // continuing so any block params from the body can pass through the exit.
                if (auto* loop = current_ctrl->As<core::ir::Loop>()) {
                    if (exit->Block() == loop->Continuing() &&
                        propagated_value->Is<core::ir::BlockParam>()) {
                        exit->PushOperand(propagated_value);
                    }
                }
            }

            propagated_value = res;
            current_blk = current_ctrl->Block();
            current_ctrl = current_blk->Parent();
        }

        values_.Replace(spv_id_being_propagated, propagated_value);
        return propagated_value;
    }

    // Returns true if the value is a constant value
    bool IdIsConstant(uint32_t id) { return SpvConstant(id) || spec_composites_.contains(id); }

    // Returns true if this value is currently in scope
    bool IdIsInScope(uint32_t id) {
        for (auto iter = id_stack_.rbegin(); iter != id_stack_.rend(); ++iter) {
            if (iter->contains(id)) {
                return true;
            }
        }
        return false;
    }

    // Get the spirv constant for the given `id`. `nullptr` if no constant exists.
    const spvtools::opt::analysis::Constant* SpvConstant(uint32_t id) {
        return spirv_context_->get_constant_mgr()->FindDeclaredConstant(id);
    }

    /// Attempts to retrieve the current Tint IR value for `id`. This ignores scoping for the
    /// variable, if it exists it's returned (or if it's constant it's created). The value will not
    /// propagate up through control instructions.
    ///
    /// @param id a SPIR-V result ID
    /// @returns a Tint value object
    std::optional<core::ir::Value*> ValueNoPropagate(uint32_t id) {
        auto v = values_.Get(id);
        if (v) {
            return *v;
        }

        if (auto* c = SpvConstant(id)) {
            auto* val = b_.Constant(Constant(c));
            values_.Add(id, val);
            return val;
        }

        // If this was a spec composite, then it currently isn't in scope, so we construct
        // a new copy and assign the constant ID to the new construct in this scope.
        auto iter = spec_composites_.find(id);
        if (iter != spec_composites_.end()) {
            Vector<core::ir::Value*, 4> args;
            for (auto arg : iter->second.args) {
                args.Push(Value(arg));
            }

            auto* construct = b_.Construct(iter->second.type, args);
            current_block_->Append(construct);
            AddValue(id, construct->Result());
            return construct->Result();
        }

        return std::nullopt;
    }

    /// Attempts to retrieve the current Tint IR value for `id`. If the value exists and is not in
    /// scope it will propagate the value up through the control instructions.
    ///
    /// @param id a SPIR-V result ID
    /// @returns a Tint value object
    core::ir::Value* Value(uint32_t id, bool add_to_scope = true) {
        auto v = ValueNoPropagate(id);
        TINT_ASSERT(v.has_value());

        if (v.value()->Is<core::ir::Constant>() || IdIsInScope(id)) {
            return v.value();
        }

        auto* new_v = Propagate(id, v.value());
        if (add_to_scope) {
            AddValue(id, new_v);
        }
        return new_v;
    }

    /// Creates the Tint IR constant for the SPIR-V `constant` value.
    ///
    /// @param constant a SPIR-V constant object
    /// @returns a Tint constant value
    const core::constant::Value* Constant(const spvtools::opt::analysis::Constant* constant) {
        // Handle OpConstantNull for all types.
        if (constant->AsNullConstant()) {
            return ir_.constant_values.Zero(Type(constant->type()));
        }

        if (auto* bool_ = constant->AsBoolConstant()) {
            return b_.ConstantValue(bool_->value());
        }
        if (auto* i = constant->AsIntConstant()) {
            auto* int_ty = i->type()->AsInteger();
            TINT_ASSERT(int_ty->width() == 32);
            if (int_ty->IsSigned()) {
                return b_.ConstantValue(i32(i->GetS32BitValue()));
            } else {
                return b_.ConstantValue(u32(i->GetU32BitValue()));
            }
        }
        if (auto* f = constant->AsFloatConstant()) {
            auto* float_ty = f->type()->AsFloat();
            if (float_ty->width() == 16) {
                return b_.ConstantValue(f16::FromBits(static_cast<uint16_t>(f->words()[0])));
            } else if (float_ty->width() == 32) {
                return b_.ConstantValue(f32(f->GetFloat()));
            } else {
                TINT_UNREACHABLE() << "unsupported floating point type width";
            }
        }
        if (auto* v = constant->AsVectorConstant()) {
            Vector<const core::constant::Value*, 4> elements;
            for (auto& el : v->GetComponents()) {
                elements.Push(Constant(el));
            }
            return ir_.constant_values.Composite(Type(v->type()), std::move(elements));
        }
        if (auto* m = constant->AsMatrixConstant()) {
            Vector<const core::constant::Value*, 4> columns;
            for (auto& el : m->GetComponents()) {
                columns.Push(Constant(el));
            }
            return ir_.constant_values.Composite(Type(m->type()), std::move(columns));
        }
        if (auto* a = constant->AsArrayConstant()) {
            Vector<const core::constant::Value*, 16> elements;
            for (auto& el : a->GetComponents()) {
                elements.Push(Constant(el));
            }
            return ir_.constant_values.Composite(Type(a->type()), std::move(elements));
        }
        if (auto* s = constant->AsStructConstant()) {
            Vector<const core::constant::Value*, 16> elements;
            for (auto& el : s->GetComponents()) {
                elements.Push(Constant(el));
            }
            return ir_.constant_values.Composite(Type(s->type()), std::move(elements));
        }
        TINT_UNIMPLEMENTED() << "unhandled constant type";
    }

    /// Register an IR value for a SPIR-V result ID.
    /// @param result_id the SPIR-V result ID
    /// @param value the IR value
    void AddValue(uint32_t result_id, core::ir::Value* value) {
        id_stack_.back().insert(result_id);
        values_.Replace(result_id, value);
    }

    /// Emit an instruction to the current block and associates the result to
    /// the spirv result id.
    /// @param inst the instruction to emit
    /// @param result_id the SPIR-V result ID to register the instruction result for
    void Emit(core::ir::Instruction* inst, uint32_t result_id) {
        current_block_->Append(inst);
        TINT_ASSERT(inst->Results().Length() == 1u);
        AddValue(result_id, inst->Result());

        Symbol name = GetSymbolFor(result_id);
        if (name.IsValid()) {
            ir_.SetName(inst, name);
        }
    }

    /// Emit an instruction to the current block.
    /// @param inst the instruction to emit
    void EmitWithoutSpvResult(core::ir::Instruction* inst) {
        current_block_->Append(inst);
        TINT_ASSERT(inst->Results().Length() == 1u);
    }

    /// Emit an instruction to the current block.
    /// @param inst the instruction to emit
    void EmitWithoutResult(core::ir::Instruction* inst) {
        TINT_ASSERT(inst->Results().IsEmpty());
        current_block_->Append(inst);
    }

    /// Emit the module-scope variables.
    void EmitModuleScopeVariables() {
        for (auto& inst : spirv_context_->module()->types_values()) {
            switch (inst.opcode()) {
                case spv::Op::OpVariable:
                    EmitVar(inst);
                    break;
                case spv::Op::OpUndef:
                    AddValue(inst.result_id(), b_.Zero(Type(inst.type_id())));
                    break;
                default:
                    break;
            }
        }
    }

    /// Emit the functions.
    void EmitFunctions() {
        // Add all the functions in a first pass and then fill in the function bodies. This means
        // the function will exist fixing an issues where calling a function that hasn't been seen
        // generates the wrong signature.
        for (auto& func : *spirv_context_->module()) {
            current_spirv_function_ = &func;

            Vector<core::ir::FunctionParam*, 4> params;
            func.ForEachParam([&](spvtools::opt::Instruction* spirv_param) {
                auto* param = b_.FunctionParam(Type(spirv_param->type_id()));
                values_.Add(spirv_param->result_id(), param);

                Symbol name = GetSymbolFor(spirv_param->result_id());
                if (name.IsValid()) {
                    ir_.SetName(param, name);
                }

                params.Push(param);
            });

            current_function_ = Function(func.result_id());
            current_function_->SetParams(std::move(params));
            current_function_->SetReturnType(Type(func.type_id()));

            Symbol name = GetSymbolFor(func.result_id());
            if (name.IsValid()) {
                ir_.SetName(current_function_, name);
            }

            functions_.Add(func.result_id(), current_function_);
            current_spirv_function_ = nullptr;
        }

        for (auto& func : *spirv_context_->module()) {
            current_spirv_function_ = &func;

            current_function_ = Function(func.result_id());
            EmitBlockParent(current_function_->Block(), *func.entry());

            // No terminator was emitted, that means then end of block is
            // unreachable. Mark as such.
            if (!current_function_->Block()->Terminator()) {
                current_function_->Block()->Append(b_.Unreachable());
            }
            current_spirv_function_ = nullptr;
        }
    }

    /// Emit entry point attributes.
    void EmitEntryPointAttributes() {
        // Handle OpEntryPoint declarations.
        for (auto& entry_point : spirv_context_->module()->entry_points()) {
            auto model = entry_point.GetSingleWordInOperand(0);
            auto* func = Function(entry_point.GetSingleWordInOperand(1));

            // Set the pipeline stage.
            switch (spv::ExecutionModel(model)) {
                case spv::ExecutionModel::GLCompute:
                    func->SetStage(core::ir::Function::PipelineStage::kCompute);
                    break;
                case spv::ExecutionModel::Fragment:
                    func->SetStage(core::ir::Function::PipelineStage::kFragment);
                    break;
                case spv::ExecutionModel::Vertex:
                    func->SetStage(core::ir::Function::PipelineStage::kVertex);
                    break;
                default:
                    TINT_UNIMPLEMENTED() << "unhandled execution model: " << model;
            }

            // Set the entry point name.
            ir_.SetName(func, entry_point.GetOperand(2).AsString());

            if (func->IsCompute()) {
                // Search for `WorkgroupSize` decorated Ids
                for (const spvtools::opt::Instruction& inst :
                     spirv_context_->module()->annotations()) {
                    if (inst.opcode() != spv::Op::OpDecorate ||
                        inst.GetSingleWordInOperand(1) != uint32_t(spv::Decoration::BuiltIn) ||
                        inst.GetSingleWordInOperand(2) != uint32_t(spv::BuiltIn::WorkgroupSize)) {
                        continue;
                    }
                    uint32_t id = inst.GetSingleWordInOperand(0);

                    Vector<core::ir::Value*, 3> args;
                    if (auto* c = SpvConstant(id)) {
                        auto* vals = c->AsVectorConstant();
                        TINT_ASSERT(vals);

                        for (auto& el : vals->GetComponents()) {
                            args.Push(b_.Constant(Constant(el)));
                        }
                    } else {
                        TINT_ASSERT(spec_composites_.contains(id));

                        auto info = spec_composites_[id];
                        TINT_ASSERT(info.args.Length() == 3);

                        for (auto arg : info.args) {
                            args.Push(Value(arg));
                        }
                    }
                    func->SetWorkgroupSize(args[0], args[1], args[2]);

                    break;
                }
            }
        }

        // Handle OpExecutionMode declarations.
        for (auto& execution_mode : spirv_context_->module()->execution_modes()) {
            auto* func = functions_.GetOr(execution_mode.GetSingleWordInOperand(0), nullptr);
            auto mode = execution_mode.GetSingleWordInOperand(1);
            TINT_ASSERT(func);

            switch (spv::ExecutionMode(mode)) {
                case spv::ExecutionMode::LocalSize:
                    func->SetWorkgroupSize(
                        b_.Constant(u32(execution_mode.GetSingleWordInOperand(2))),
                        b_.Constant(u32(execution_mode.GetSingleWordInOperand(3))),
                        b_.Constant(u32(execution_mode.GetSingleWordInOperand(4))));
                    break;
                case spv::ExecutionMode::DepthGreater:
                    TINT_ICE() << "ExecutionMode DepthGreater is not supported in WGSL";
                    /* Ice aborts, so this doesn't fallthrough */
                case spv::ExecutionMode::DepthLess:
                    TINT_ICE() << "ExecutionMode DepthLess is not supported in WGSL";
                    /* Ice aborts, so this doesn't fallthrough */
                case spv::ExecutionMode::DepthUnchanged:
                    TINT_ICE() << "ExecutionMode DepthUnchanged is not supported in WGSL";
                    /* Ice aborts, so this doesn't fallthrough */
                case spv::ExecutionMode::EarlyFragmentTests:
                    TINT_ICE() << "ExecutionMode EarlyFragmentTests is not supported in WGSL";
                    /* Ice aborts, so this doesn't fallthrough */
                case spv::ExecutionMode::DepthReplacing:
                case spv::ExecutionMode::OriginUpperLeft:
                    // These are ignored as they are implicitly supported by Tint IR.
                    break;
                default:
                    TINT_UNIMPLEMENTED() << "unhandled execution mode: " << mode;
            }
        }
    }

    bool InBlock(core::ir::Block* blk) { return current_blocks_.contains(blk); }

    // A block parent is a container for a scope, like a `{}`d section in code. It controls the
    // block addition to the current blocks and the ID stack entry for the block.
    void EmitBlockParent(core::ir::Block* dst, spvtools::opt::BasicBlock& src) {
        TINT_ASSERT(!InBlock(dst));

        id_stack_.emplace_back();
        current_blocks_.insert(dst);

        EmitBlock(dst, src);

        current_blocks_.erase(dst);
        id_stack_.pop_back();
    }

    /// Emit the contents of SPIR-V block @p src into Tint IR block @p dst.
    /// @param dst the Tint IR block to append to
    /// @param src the SPIR-V block to emit
    void EmitBlock(core::ir::Block* dst, spvtools::opt::BasicBlock& src) {
        TINT_SCOPED_ASSIGNMENT(current_block_, dst);

        // Register the merge if this is a header block
        auto* merge_inst = src.GetMergeInst();
        if (merge_inst) {
            auto merge_id = merge_inst->GetSingleWordInOperand(0);
            spirv_merge_id_to_header_id_.insert({merge_id, src.id()});
        }

        values_to_replace_.push_back({});

        // If this is a loop merge block, so we need to treat it as a Loop.
        auto* loop_merge_inst = src.GetLoopMergeInst();
        core::ir::Loop* loop = nullptr;
        if (loop_merge_inst) {
            // Emit the loop instruction into the current block.
            EmitLoop(src);

            // The loop header is a walk stop block, which was created in the emit loop method. Get
            // the loop back so we can change the current insertion block.
            loop = StopWalkingAt(src.id())->As<core::ir::Loop>();
            TINT_ASSERT(loop);

            id_stack_.emplace_back();
            current_blocks_.insert(loop->Body());

            // Now emit the remainder of the block into the loop body. We do this now so the loop
            // emitted above is in the parent block.
            current_block_ = loop->Body();
        }

        // Note, this comes after the loop code since the current block is set to the loop body
        spirv_id_to_block_.insert({src.id(), current_block_});

        ProcessInstructions(src);

        // Add the body terminator if necessary
        if (loop && !loop->Body()->Terminator()) {
            loop->Body()->Append(b_.Unreachable());
        }

        // For any `OpPhi` values we saw, insert their `Value` now. We do this at the end of the
        // processing of instructions because a phi can refer to instructions defined after it
        // in the block.
        auto replace = values_to_replace_.back();
        for (auto& val : replace) {
            auto v = ValueNoPropagate(val.value_id);
            TINT_ASSERT(v.has_value());
            val.terminator->SetOperand(val.idx, v.value());
        }

        values_to_replace_.pop_back();

        if (!loop) {
            return;
        }

        // Emit the continuing block. The continue block is within the scope of the body block,
        // so we don't pop the id stack yet.
        auto continue_id = loop_merge_inst->GetSingleWordInOperand(1);

        // Capture any continue PHi header values before emitting the continue because the value
        // will get re-written to be the continue block param, so we have to capture first.
        auto continue_loop_phi_ids = block_phi_values_[continue_id];
        std::vector<core::ir::Value*> continue_loop_phi_values;
        if (!continue_loop_phi_ids.empty()) {
            continue_loop_phi_values.reserve(continue_loop_phi_ids.size());
            for (auto& id : continue_loop_phi_ids) {
                auto v = ValueNoPropagate(id);
                if (v.has_value()) {
                    continue_loop_phi_values.push_back(v.value());
                }
            }
        }

        EmitContinueBlock(src.id(), continue_id, loop);

        // Remove the body block id stack before emitting the merge block.
        current_blocks_.erase(loop->Body());
        id_stack_.pop_back();

        // If we added phi's to the continuing block, we may have exits from the body which
        // aren't valid.
        if (loop->Continuing()->Params().Length() > 0) {
            for (auto incoming : loop->Continuing()->InboundSiblingBranches()) {
                TINT_ASSERT(incoming->Is<core::ir::Continue>());

                if (incoming->Args().Length() == loop->Continuing()->Params().Length()) {
                    continue;
                }

                // Check if the block this instruction exists in has default phi result that we
                // can append.
                auto inst_to_blk_iter = inst_to_spirv_block_.find(incoming);
                if (inst_to_blk_iter != inst_to_spirv_block_.end()) {
                    uint32_t spirv_blk = inst_to_blk_iter->second;

                    auto phi_values_from_loop_header = block_phi_values_[spirv_blk];

                    // We require a PHI value, but the current block doesn't have any from the
                    // header. In this case, check the `continue` target to see if it has header
                    // provided phi values. If so, use that one.
                    if (phi_values_from_loop_header.empty()) {
                        TINT_ASSERT(!continue_loop_phi_values.empty());

                        for (auto* value : continue_loop_phi_values) {
                            incoming->PushOperand(value);
                        }

                    } else {
                        // If there were phi values, push them to this instruction
                        for (auto value_id : phi_values_from_loop_header) {
                            auto* value = Value(value_id);
                            incoming->PushOperand(value);
                        }
                    }
                } else {
                    TINT_UNREACHABLE() << "unable to provide needed continue arguments";
                }
            }
        }

        // Emit the merge block
        auto merge_id = loop_merge_inst->GetSingleWordInOperand(0);
        const auto& merge_bb = current_spirv_function_->FindBlock(merge_id);
        EmitBlock(dst, *merge_bb);
    }

    void EmitContinueBlock(uint32_t src_id, uint32_t continue_id, core::ir::Loop* loop) {
        // Push id stack entry for the continuing block. We don't use EmitBlockParent to do this
        // because we need the scope to exist until after we process any `continue_blk_phis_`.
        id_stack_.emplace_back();

        // We only need to emit the continuing block if:
        //  a) It is not the loop header
        //  b) It has inbound branches. This works around a case where you can have a continuing
        //     where uses values which are very difficult to propagate, but the continuing is never
        //     reached anyway, so the propagation is useless.
        bool skipped_continue = true;
        if (continue_id != src_id && !loop->Continuing()->InboundSiblingBranches().IsEmpty()) {
            skipped_continue = false;

            const auto& bb_continue = current_spirv_function_->FindBlock(continue_id);

            current_blocks_.insert(loop->Continuing());
            EmitBlock(loop->Continuing(), *bb_continue);
            current_blocks_.erase(loop->Continuing());
        }

        if (!loop->Continuing()->Terminator()) {
            loop->Continuing()->Append(b_.NextIteration(loop));
        }

        // If this continue block needs to pass any `phi` instructions back to the main loop body.
        //
        // We have to do this here because we need to have emitted the loop body before we can get
        // the values used in the continue block.
        auto phis = continue_blk_phis_.find(continue_id);
        if (phis != continue_blk_phis_.end()) {
            for (auto value_id : phis->second) {
                auto* value = skipped_continue ? nullptr : Value(value_id, false);

                tint::Switch(
                    loop->Continuing()->Terminator(),  //
                    [&](core::ir::NextIteration* ni) { ni->PushOperand(value); },
                    [&](core::ir::BreakIf* bi) {
                        // TODO(dsinclair): Need to change the break-if insertion if there happens
                        // to be exit values, but those are rare, so leave this for when we have
                        // test case.
                        TINT_ASSERT(bi->ExitValues().IsEmpty());

                        auto len = bi->NextIterValues().Length();
                        bi->PushOperand(value);
                        bi->SetNumNextIterValues(len + 1);
                    },
                    TINT_ICE_ON_NO_MATCH);
            }
        }

        id_stack_.pop_back();
    }

    void ProcessInstructions(spvtools::opt::BasicBlock& src) {
        for (auto& inst : src) {
            switch (inst.opcode()) {
                case spv::Op::OpNop:
                    break;
                case spv::Op::OpUndef:
                    AddValue(inst.result_id(), b_.Zero(Type(inst.type_id())));
                    break;
                case spv::Op::OpBranch:
                    EmitBranch(inst);
                    break;
                case spv::Op::OpBranchConditional:
                    EmitBranchConditional(src, inst);
                    break;
                case spv::Op::OpSwitch:
                    EmitSwitch(src, inst);
                    break;
                case spv::Op::OpLoopMerge:
                    EmitLoopMerge(src, inst);
                    break;
                case spv::Op::OpSelectionMerge:
                    // Do nothing, the selection merge will be handled in the following
                    // OpBranchCondition or OpSwitch instruction
                    break;
                case spv::Op::OpExtInst:
                    EmitExtInst(inst);
                    break;
                case spv::Op::OpCopyObject:
                    EmitCopyObject(inst);
                    break;
                case spv::Op::OpConvertFToS:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kConvertFToS);
                    break;
                case spv::Op::OpConvertFToU:
                    Emit(b_.Convert(Type(inst.type_id()), Value(inst.GetSingleWordOperand(2))),
                         inst.result_id());
                    break;
                case spv::Op::OpConvertSToF:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kConvertSToF);
                    break;
                case spv::Op::OpConvertUToF:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kConvertUToF);
                    break;
                case spv::Op::OpFConvert:
                    Emit(b_.Convert(Type(inst.type_id()), Value(inst.GetSingleWordOperand(2))),
                         inst.result_id());
                    break;
                case spv::Op::OpBitwiseAnd:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kBitwiseAnd);
                    break;
                case spv::Op::OpBitwiseOr:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kBitwiseOr);
                    break;
                case spv::Op::OpBitwiseXor:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kBitwiseXor);
                    break;
                case spv::Op::OpAccessChain:
                case spv::Op::OpInBoundsAccessChain:
                    EmitAccess(inst);
                    break;
                case spv::Op::OpCompositeInsert:
                    EmitCompositeInsert(inst);
                    break;
                case spv::Op::OpCompositeConstruct:
                    EmitConstruct(inst);
                    break;
                case spv::Op::OpCompositeExtract:
                    EmitCompositeExtract(inst);
                    break;
                case spv::Op::OpVectorInsertDynamic:
                    EmitVectorInsertDynamic(inst);
                    break;
                case spv::Op::OpFAdd:
                    EmitBinary(inst, core::BinaryOp::kAdd);
                    break;
                case spv::Op::OpIAdd:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kAdd);
                    break;
                case spv::Op::OpSDiv:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSDiv);
                    break;
                case spv::Op::OpFDiv:
                case spv::Op::OpUDiv:
                    EmitBinary(inst, core::BinaryOp::kDivide);
                    break;
                case spv::Op::OpIMul:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kMul);
                    break;
                case spv::Op::OpFMul:
                case spv::Op::OpVectorTimesScalar:
                case spv::Op::OpMatrixTimesScalar:
                case spv::Op::OpVectorTimesMatrix:
                case spv::Op::OpMatrixTimesVector:
                case spv::Op::OpMatrixTimesMatrix:
                    EmitBinary(inst, core::BinaryOp::kMultiply);
                    break;
                case spv::Op::OpFRem:
                case spv::Op::OpUMod:
                    EmitBinary(inst, core::BinaryOp::kModulo);
                    break;
                case spv::Op::OpSMod:
                case spv::Op::OpSRem:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSMod);
                    break;
                case spv::Op::OpFSub:
                    EmitBinary(inst, core::BinaryOp::kSubtract);
                    break;
                case spv::Op::OpFOrdEqual:
                    EmitBinary(inst, core::BinaryOp::kEqual);
                    break;
                case spv::Op::OpFOrdNotEqual:
                    EmitBinary(inst, core::BinaryOp::kNotEqual);
                    break;
                case spv::Op::OpFOrdGreaterThan:
                    EmitBinary(inst, core::BinaryOp::kGreaterThan);
                    break;
                case spv::Op::OpFOrdGreaterThanEqual:
                    EmitBinary(inst, core::BinaryOp::kGreaterThanEqual);
                    break;
                case spv::Op::OpFOrdLessThan:
                    EmitBinary(inst, core::BinaryOp::kLessThan);
                    break;
                case spv::Op::OpFOrdLessThanEqual:
                    EmitBinary(inst, core::BinaryOp::kLessThanEqual);
                    break;
                case spv::Op::OpFUnordEqual:
                    EmitInvertedBinary(inst, core::BinaryOp::kNotEqual);
                    break;
                case spv::Op::OpFUnordNotEqual:
                    EmitInvertedBinary(inst, core::BinaryOp::kEqual);
                    break;
                case spv::Op::OpFUnordGreaterThan:
                    EmitInvertedBinary(inst, core::BinaryOp::kLessThanEqual);
                    break;
                case spv::Op::OpFUnordGreaterThanEqual:
                    EmitInvertedBinary(inst, core::BinaryOp::kLessThan);
                    break;
                case spv::Op::OpFUnordLessThan:
                    EmitInvertedBinary(inst, core::BinaryOp::kGreaterThanEqual);
                    break;
                case spv::Op::OpFUnordLessThanEqual:
                    EmitInvertedBinary(inst, core::BinaryOp::kGreaterThan);
                    break;
                case spv::Op::OpIEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kEqual);
                    break;
                case spv::Op::OpINotEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kNotEqual);
                    break;
                case spv::Op::OpSGreaterThan:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSGreaterThan);
                    break;
                case spv::Op::OpSGreaterThanEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSGreaterThanEqual);
                    break;
                case spv::Op::OpSLessThan:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSLessThan);
                    break;
                case spv::Op::OpSLessThanEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSLessThanEqual);
                    break;
                case spv::Op::OpUGreaterThan:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kUGreaterThan);
                    break;
                case spv::Op::OpUGreaterThanEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kUGreaterThanEqual);
                    break;
                case spv::Op::OpULessThan:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kULessThan);
                    break;
                case spv::Op::OpULessThanEqual:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kULessThanEqual);
                    break;
                case spv::Op::OpISub:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSub);
                    break;
                case spv::Op::OpFunctionCall:
                    EmitFunctionCall(inst);
                    break;
                case spv::Op::OpLoad:
                    Emit(b_.Load(Value(inst.GetSingleWordOperand(2))), inst.result_id());
                    break;
                case spv::Op::OpReturn:
                    EmitWithoutResult(b_.Return(current_function_));
                    break;
                case spv::Op::OpReturnValue:
                    EmitWithoutResult(
                        b_.Return(current_function_, Value(inst.GetSingleWordOperand(0))));
                    break;
                case spv::Op::OpStore:
                    EmitWithoutResult(b_.Store(Value(inst.GetSingleWordOperand(0)),
                                               Value(inst.GetSingleWordOperand(1))));
                    break;
                case spv::Op::OpCopyMemory:
                    EmitCopyMemory(inst);
                    break;
                case spv::Op::OpVariable:
                    EmitVar(inst);
                    break;
                case spv::Op::OpUnreachable:
                    EmitWithoutResult(b_.Unreachable());
                    break;
                case spv::Op::OpKill:
                case spv::Op::OpTerminateInvocation:
                    EmitKill(inst);
                    break;
                case spv::Op::OpDot:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDot);
                    break;
                case spv::Op::OpBitCount:
                    EmitBitCount(inst);
                    break;
                case spv::Op::OpBitFieldInsert:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kBitFieldInsert);
                    break;
                case spv::Op::OpBitFieldSExtract:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kBitFieldSExtract);
                    break;
                case spv::Op::OpBitFieldUExtract:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kBitFieldUExtract);
                    break;
                case spv::Op::OpBitReverse:
                    EmitBuiltinCall(inst, core::BuiltinFn::kReverseBits);
                    break;
                case spv::Op::OpAll:
                    EmitBuiltinCall(inst, core::BuiltinFn::kAll);
                    break;
                case spv::Op::OpAny:
                    EmitBuiltinCall(inst, core::BuiltinFn::kAny);
                    break;
                case spv::Op::OpDPdx:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdx);
                    break;
                case spv::Op::OpDPdy:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdy);
                    break;
                case spv::Op::OpFwidth:
                    EmitBuiltinCall(inst, core::BuiltinFn::kFwidth);
                    break;
                case spv::Op::OpDPdxFine:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdxFine);
                    break;
                case spv::Op::OpDPdyFine:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdyFine);
                    break;
                case spv::Op::OpFwidthFine:
                    EmitBuiltinCall(inst, core::BuiltinFn::kFwidthFine);
                    break;
                case spv::Op::OpDPdxCoarse:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdxCoarse);
                    break;
                case spv::Op::OpDPdyCoarse:
                    EmitBuiltinCall(inst, core::BuiltinFn::kDpdyCoarse);
                    break;
                case spv::Op::OpFwidthCoarse:
                    EmitBuiltinCall(inst, core::BuiltinFn::kFwidthCoarse);
                    break;
                case spv::Op::OpLogicalAnd:
                    EmitBinary(inst, core::BinaryOp::kAnd);
                    break;
                case spv::Op::OpLogicalOr:
                    EmitBinary(inst, core::BinaryOp::kOr);
                    break;
                case spv::Op::OpLogicalEqual:
                    EmitBinary(inst, core::BinaryOp::kEqual);
                    break;
                case spv::Op::OpLogicalNotEqual:
                    EmitBinary(inst, core::BinaryOp::kNotEqual);
                    break;
                case spv::Op::OpLogicalNot:
                    EmitUnary(inst, core::UnaryOp::kNot);
                    break;
                case spv::Op::OpFNegate:
                    EmitUnary(inst, core::UnaryOp::kNegation);
                    break;
                case spv::Op::OpNot:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kNot);
                    break;
                case spv::Op::OpShiftLeftLogical:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kShiftLeftLogical);
                    break;
                case spv::Op::OpShiftRightLogical:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kShiftRightLogical);
                    break;
                case spv::Op::OpShiftRightArithmetic:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kShiftRightArithmetic);
                    break;
                case spv::Op::OpBitcast:
                    EmitBitcast(inst);
                    break;
                case spv::Op::OpQuantizeToF16:
                    EmitBuiltinCall(inst, core::BuiltinFn::kQuantizeToF16);
                    break;
                case spv::Op::OpTranspose:
                    EmitBuiltinCall(inst, core::BuiltinFn::kTranspose);
                    break;
                case spv::Op::OpSNegate:
                    EmitSpirvExplicitBuiltinCall(inst, spirv::BuiltinFn::kSNegate);
                    break;
                case spv::Op::OpFMod:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kFMod);
                    break;
                case spv::Op::OpSelect:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kSelect);
                    break;
                case spv::Op::OpVectorExtractDynamic:
                    EmitAccess(inst);
                    break;
                case spv::Op::OpOuterProduct:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kOuterProduct);
                    break;
                case spv::Op::OpVectorShuffle:
                    EmitVectorShuffle(inst);
                    break;
                case spv::Op::OpAtomicStore:
                    EmitAtomicStore(inst);
                    break;
                case spv::Op::OpAtomicLoad:
                    CheckAtomicNotFloat(inst);
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicLoad);
                    break;
                case spv::Op::OpAtomicIAdd:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicIAdd);
                    break;
                case spv::Op::OpAtomicISub:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicISub);
                    break;
                case spv::Op::OpAtomicAnd:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicAnd);
                    break;
                case spv::Op::OpAtomicOr:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicOr);
                    break;
                case spv::Op::OpAtomicXor:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicXor);
                    break;
                case spv::Op::OpAtomicSMin:
                    EmitAtomicSigned(inst, spirv::BuiltinFn::kAtomicSMin);
                    break;
                case spv::Op::OpAtomicUMin:
                    EmitAtomicUnsigned(inst, spirv::BuiltinFn::kAtomicUMin);
                    break;
                case spv::Op::OpAtomicSMax:
                    EmitAtomicSigned(inst, spirv::BuiltinFn::kAtomicSMax);
                    break;
                case spv::Op::OpAtomicUMax:
                    EmitAtomicUnsigned(inst, spirv::BuiltinFn::kAtomicUMax);
                    break;
                case spv::Op::OpAtomicExchange:
                    CheckAtomicNotFloat(inst);
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicExchange);
                    break;
                case spv::Op::OpAtomicCompareExchange:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicCompareExchange);
                    break;
                case spv::Op::OpAtomicIIncrement:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicIIncrement);
                    break;
                case spv::Op::OpAtomicIDecrement:
                    EmitSpirvBuiltinCall(inst, spirv::BuiltinFn::kAtomicIDecrement);
                    break;
                case spv::Op::OpControlBarrier:
                    EmitControlBarrier(inst);
                    break;
                case spv::Op::OpArrayLength:
                    EmitArrayLength(inst);
                    break;
                case spv::Op::OpSampledImage:
                    EmitSampledImage(inst);
                    break;
                case spv::Op::OpImage:
                    EmitImage(inst);
                    break;
                case spv::Op::OpImageFetch:
                    EmitImageFetchOrRead(inst, spirv::BuiltinFn::kImageFetch);
                    break;
                case spv::Op::OpImageRead:
                    EmitImageFetchOrRead(inst, spirv::BuiltinFn::kImageRead);
                    break;
                case spv::Op::OpImageGather:
                    EmitImageGather(inst);
                    break;
                case spv::Op::OpImageQueryLevels:
                    EmitImageQuery(inst, spirv::BuiltinFn::kImageQueryLevels);
                    break;
                case spv::Op::OpImageQuerySamples:
                    EmitImageQuery(inst, spirv::BuiltinFn::kImageQuerySamples);
                    break;
                case spv::Op::OpImageQuerySize:
                    EmitImageQuery(inst, spirv::BuiltinFn::kImageQuerySize);
                    break;
                case spv::Op::OpImageQuerySizeLod:
                    EmitImageQuerySizeLod(inst);
                    break;
                case spv::Op::OpImageSampleExplicitLod:
                    EmitImageSample(inst, spirv::BuiltinFn::kImageSampleExplicitLod);
                    break;
                case spv::Op::OpImageSampleImplicitLod:
                    EmitImageSample(inst, spirv::BuiltinFn::kImageSampleImplicitLod);
                    break;
                case spv::Op::OpImageSampleProjImplicitLod:
                    EmitImageSample(inst, spirv::BuiltinFn::kImageSampleProjImplicitLod);
                    break;
                case spv::Op::OpImageSampleProjExplicitLod:
                    EmitImageSample(inst, spirv::BuiltinFn::kImageSampleProjExplicitLod);
                    break;
                case spv::Op::OpImageWrite:
                    EmitImageWrite(inst);
                    break;
                case spv::Op::OpImageSampleDrefImplicitLod:
                    EmitImageSampleDepth(inst, spirv::BuiltinFn::kImageSampleDrefImplicitLod);
                    break;
                case spv::Op::OpImageSampleDrefExplicitLod:
                    EmitImageSampleDepth(inst, spirv::BuiltinFn::kImageSampleDrefExplicitLod);
                    break;
                case spv::Op::OpImageSampleProjDrefImplicitLod:
                    EmitImageSampleDepth(inst, spirv::BuiltinFn::kImageSampleProjDrefImplicitLod);
                    break;
                case spv::Op::OpImageSampleProjDrefExplicitLod:
                    EmitImageSampleDepth(inst, spirv::BuiltinFn::kImageSampleProjDrefExplicitLod);
                    break;
                case spv::Op::OpImageDrefGather:
                    EmitImageGatherDref(inst);
                    break;
                case spv::Op::OpPhi:
                    EmitPhi(inst);
                    break;

                case spv::Op::OpGroupNonUniformBroadcast:
                    EmitSubgroupBuiltinConstantId(inst,
                                                  spirv::BuiltinFn::kGroupNonUniformBroadcast);
                    break;
                case spv::Op::OpGroupNonUniformQuadBroadcast:
                    EmitSubgroupBuiltinConstantId(inst,
                                                  spirv::BuiltinFn::kGroupNonUniformQuadBroadcast);
                    break;
                case spv::Op::OpGroupNonUniformQuadSwap:
                    EmitSubgroupBuiltinConstantId(inst, spirv::BuiltinFn::kGroupNonUniformQuadSwap);
                    break;
                case spv::Op::OpGroupNonUniformAll:
                    EmitSubgroupBuiltin(inst, core::BuiltinFn::kSubgroupAll);
                    break;
                case spv::Op::OpGroupNonUniformAny:
                    EmitSubgroupBuiltin(inst, core::BuiltinFn::kSubgroupAny);
                    break;
                case spv::Op::OpGroupNonUniformElect:
                    EmitSubgroupBuiltin(inst, core::BuiltinFn::kSubgroupElect);
                    break;
                case spv::Op::OpGroupNonUniformBallot:
                    EmitSubgroupBuiltin(inst, core::BuiltinFn::kSubgroupBallot);
                    break;
                case spv::Op::OpGroupNonUniformBroadcastFirst:
                    EmitSubgroupBuiltin(inst, spirv::BuiltinFn::kGroupNonUniformBroadcastFirst);
                    break;
                case spv::Op::OpGroupNonUniformShuffle:
                    EmitSubgroupBuiltin(inst, spirv::BuiltinFn::kGroupNonUniformShuffle);
                    break;
                case spv::Op::OpGroupNonUniformShuffleXor:
                    EmitSubgroupBuiltin(inst, spirv::BuiltinFn::kGroupNonUniformShuffleXor);
                    break;
                case spv::Op::OpGroupNonUniformShuffleDown:
                    EmitSubgroupBuiltin(inst, spirv::BuiltinFn::kGroupNonUniformShuffleDown);
                    break;
                case spv::Op::OpGroupNonUniformShuffleUp:
                    EmitSubgroupBuiltin(inst, spirv::BuiltinFn::kGroupNonUniformShuffleUp);
                    break;
                case spv::Op::OpGroupNonUniformSMin:
                    EmitSubgroupMinMax(inst, spirv::BuiltinFn::kGroupNonUniformSMin);
                    break;
                case spv::Op::OpGroupNonUniformSMax:
                    EmitSubgroupMinMax(inst, spirv::BuiltinFn::kGroupNonUniformSMax);
                    break;
                case spv::Op::OpGroupNonUniformUMin:
                case spv::Op::OpGroupNonUniformFMin:
                    EmitSubgroupMinMax(inst, core::BuiltinFn::kSubgroupMin);
                    break;
                case spv::Op::OpGroupNonUniformUMax:
                case spv::Op::OpGroupNonUniformFMax:
                    EmitSubgroupMinMax(inst, core::BuiltinFn::kSubgroupMax);
                    break;
                case spv::Op::OpGroupNonUniformIAdd:
                case spv::Op::OpGroupNonUniformFAdd:
                    EmitSubgroupAdd(inst);
                    break;
                case spv::Op::OpGroupNonUniformIMul:
                case spv::Op::OpGroupNonUniformFMul:
                    EmitSubgroupMul(inst);
                    break;
                case spv::Op::OpGroupNonUniformBitwiseAnd:
                    EmitSubgroupBitwise(inst, core::BuiltinFn::kSubgroupAnd);
                    break;
                case spv::Op::OpGroupNonUniformBitwiseOr:
                    EmitSubgroupBitwise(inst, core::BuiltinFn::kSubgroupOr);
                    break;
                case spv::Op::OpGroupNonUniformBitwiseXor:
                    EmitSubgroupBitwise(inst, core::BuiltinFn::kSubgroupXor);
                    break;
                default:
                    TINT_UNIMPLEMENTED()
                        << "unhandled SPIR-V instruction: " << static_cast<uint32_t>(inst.opcode());
            }
        }
    }

    void ValidateScope(spvtools::opt::Instruction& inst) {
        auto scope_val = Value(inst.GetSingleWordInOperand(0));
        auto* cnst = scope_val->As<core::ir::Constant>();
        TINT_ASSERT(cnst);

        uint32_t scope = cnst->Value()->ValueAs<uint32_t>();
        if (static_cast<spv::Scope>(scope) != spv::Scope::Subgroup) {
            TINT_ICE() << "subgroup scope required for GroupNonUniform instructions";
        }
    }

    void EmitSubgroupBitwise(spvtools::opt::Instruction& inst, core::BuiltinFn fn) {
        ValidateScope(inst);

        auto group = inst.GetSingleWordInOperand(1);
        if (static_cast<spv::GroupOperation>(group) != spv::GroupOperation::Reduce) {
            TINT_ICE() << "GroupNonUniformBitwise operations require a Reduce group operation";
        }

        Emit(b_.Call(Type(inst.type_id()), fn, Args(inst, 4)), inst.result_id());
    }

    void EmitSubgroupMul(spvtools::opt::Instruction& inst) {
        ValidateScope(inst);

        core::BuiltinFn fn = core::BuiltinFn::kNone;

        auto group = inst.GetSingleWordInOperand(1);
        if (static_cast<spv::GroupOperation>(group) == spv::GroupOperation::Reduce) {
            fn = core::BuiltinFn::kSubgroupMul;
        } else if (static_cast<spv::GroupOperation>(group) == spv::GroupOperation::InclusiveScan) {
            fn = core::BuiltinFn::kSubgroupInclusiveMul;
        } else if (static_cast<spv::GroupOperation>(group) == spv::GroupOperation::ExclusiveScan) {
            fn = core::BuiltinFn::kSubgroupExclusiveMul;
        } else {
            TINT_ICE() << "GroupNonUniform Mul instruction must have a group of `Reduce`, "
                          "`InclusiveScan`, or `ExclusiveScan`";
        }

        Emit(b_.Call(Type(inst.type_id()), fn, Args(inst, 4)), inst.result_id());
    }

    void EmitSubgroupAdd(spvtools::opt::Instruction& inst) {
        ValidateScope(inst);

        core::BuiltinFn fn = core::BuiltinFn::kNone;

        auto group = inst.GetSingleWordInOperand(1);
        if (static_cast<spv::GroupOperation>(group) == spv::GroupOperation::Reduce) {
            fn = core::BuiltinFn::kSubgroupAdd;
        } else if (static_cast<spv::GroupOperation>(group) == spv::GroupOperation::InclusiveScan) {
            fn = core::BuiltinFn::kSubgroupInclusiveAdd;
        } else if (static_cast<spv::GroupOperation>(group) == spv::GroupOperation::ExclusiveScan) {
            fn = core::BuiltinFn::kSubgroupExclusiveAdd;
        } else {
            TINT_ICE() << "GroupNonUniform Add instruction must have a group of `Reduce`, "
                          "`InclusiveScan`, or `ExclusiveScan`";
        }

        Emit(b_.Call(Type(inst.type_id()), fn, Args(inst, 4)), inst.result_id());
    }

    void EmitSubgroupMinMax(spvtools::opt::Instruction& inst, core::BuiltinFn fn) {
        ValidateScope(inst);

        auto group = inst.GetSingleWordInOperand(1);
        if (static_cast<spv::GroupOperation>(group) != spv::GroupOperation::Reduce) {
            TINT_ICE() << "group operand Reduce required for `Min`/`Max` instructions";
        }

        Emit(b_.Call(Type(inst.type_id()), fn, Args(inst, 4)), inst.result_id());
    }

    void EmitSubgroupMinMax(spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        ValidateScope(inst);

        auto group = inst.GetSingleWordInOperand(1);
        if (static_cast<spv::GroupOperation>(group) != spv::GroupOperation::Reduce) {
            TINT_ICE() << "group operand Reduce required for `Min`/`Max` instructions";
        }

        Emit(
            b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn,                      //
                                            Vector{Value(inst.GetSingleWordInOperand(0)),  //
                                                   b_.Constant(u32(inst.GetSingleWordInOperand(1))),
                                                   Value(inst.GetSingleWordInOperand(2))}),
            inst.result_id());
    }

    void EmitSubgroupBuiltinConstantId(spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        auto id = Value(inst.GetSingleWordInOperand(2));

        // TODO(431054356): Convert core::BuiltinFn::kSubgroupBroadcast non-constant values into a
        // `subgroupShuffle` when we support SPIR-V >= 1.5 source.
        //
        // For QuadBroadcast this will remain an error as there is no WGSL equivalent.
        // For QuadSwap this will remain an error as there is no WGSL equivalent.
        if (!id->Is<core::ir::Constant>()) {
            TINT_ICE() << "non-constant GroupNonUniform `Invocation Id` not supported";
        }

        ValidateScope(inst);
        Emit(b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn, Args(inst, 2)),
             inst.result_id());
    }

    void EmitSubgroupBuiltin(spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        ValidateScope(inst);
        Emit(b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn, Args(inst, 2)),
             inst.result_id());
    }

    void EmitSubgroupBuiltin(spvtools::opt::Instruction& inst, core::BuiltinFn fn) {
        ValidateScope(inst);
        Emit(b_.Call(Type(inst.type_id()), fn, Args(inst, 3)), inst.result_id());
    }

    struct IfBranchValue {
        core::ir::Value* value;
        core::ir::If* if_;
    };

    void EmitPhi(spvtools::opt::Instruction& inst) {
        auto num_ops = inst.NumInOperands();

        // If there are no operands, then the phi is in a unreachable block, ignore it.
        if (num_ops == 0) {
            return;
        }

        // If there are only 2 arguments, that means we came directly from a block, so just emit the
        // value directly.
        if (num_ops == 2) {
            AddValue(inst.result_id(), Value(inst.GetSingleWordInOperand(0)));
            return;
        }

        auto* phi_containing_spirv_block = spirv_context_->get_instr_block(&inst);
        auto iter = spirv_merge_id_to_header_id_.find(phi_containing_spirv_block->id());

        // We're in a merge block for some construct which could be a loop, switch or if.
        if (iter != spirv_merge_id_to_header_id_.end()) {
            auto parent_blk_id = iter->second;
            auto* parent_blk = spirv_context_->get_instr_block(parent_blk_id);

            if (parent_blk->terminator()->opcode() == spv::Op::OpSwitch) {
                EmitPhiInSwitchMerge(inst, parent_blk_id);
                return;
            }
            if (!parent_blk->IsLoopHeader()) {
                EmitPhiInIfMerge(inst, parent_blk_id);
                return;
            }
        }

        // At this point, we must be dealing with an OpPhi in some kind of loop context, the loop
        // header, loop continuing or loop merge construct.
        EmitPhiInLoop(inst);
    }

    void EmitPhiInLoop(spvtools::opt::Instruction& inst) {
        auto* phi_containing_spirv_block = spirv_context_->get_instr_block(&inst);

        auto iter = spirv_merge_id_to_header_id_.find(phi_containing_spirv_block->id());
        // We're in the merge block for the loop
        if (iter != spirv_merge_id_to_header_id_.end()) {
            EmitPhiInLoopMerge(inst);
            return;
        }

        // The OpPhi is in the loop header, which means it's the body of the loop. The OpPhi can
        // receive values from the parent block, in which case we need to push them through the
        // initializer, or from the continuing block, which we will not have emitted yet.
        if (phi_containing_spirv_block->IsLoopHeader()) {
            EmitPhiInLoopHeader(inst, phi_containing_spirv_block);
            return;
        }

        auto containing_loop_header_id = spirv_context_->GetStructuredCFGAnalysis()->ContainingLoop(
            phi_containing_spirv_block->id());
        auto* loop_header_block = spirv_context_->get_instr_block(containing_loop_header_id);
        TINT_ASSERT(loop_header_block);

        auto* loop_merge_inst = loop_header_block->GetLoopMergeInst();
        auto continue_id = loop_merge_inst->GetSingleWordInOperand(1);

        // If the spirv-id to block table contains an entry for the continue id then we must have
        // started emitting the continue.
        bool has_continue_emitted = spirv_id_to_block_.contains(continue_id);
        if (!has_continue_emitted) {
            EmitPhiInLoopBody(inst, loop_merge_inst);
            return;
        }

        // The OpPhi is in the continue block of the loop.
        EmitPhiInLoopContinue(inst);
    }

    void AddOperandToTerminator(core::ir::Terminator* term, uint32_t id) {
        // If the ID is a constant, then we just directly emit it, it isn't an OpPhi value
        if (IdIsConstant(id)) {
            term->PushOperand(Value(id, false));
            return;
        }
        // If we've already seen the value, then we can emit it as this isn't referencing a later
        // value.
        auto val = values_.Get(id);
        if (val) {
            // If we've already seen the value, and it's still in scope, then we can just emit as it
            // isn't referencing a later value.
            if (IdIsInScope(id)) {
                term->PushOperand(Value(id, false));
                return;
            }

            // The value was a block param, that means it was a Phi which we've removed and
            // converted. We need to pass that Phi out of the control instruction, but we only want
            // to add it to exit instructions, not all terminators, so this isn't a full
            // Propagation.
            if (auto* bp = (*val)->As<core::ir::BlockParam>()) {
                auto* v = PropagateTerm(term, id, bp, bp->Block());
                term->PushOperand(v);
                return;
            }
            // The value is defined by an instruction inside an inner block. Propagate it out of
            // that block, then push that propagated value onto this terminator.
            if (auto* res = (*val)->As<core::ir::InstructionResult>()) {
                auto* v = PropagateTerm(term, id, *val, res->Instruction()->Block());
                term->PushOperand(v);
                return;
            }
        }

        // Value isn't known, or isn't in scope, push a placeholder for the operand value. We store
        // away the terminator/index pair along with the required value and then fill it in at the
        // end of the block emission.
        auto operand_idx = term->PushOperand(nullptr);
        values_to_replace_.back().push_back(ReplacementValue{
            .terminator = term,
            .idx = operand_idx,
            .value_id = id,
        });
    }

    void EmitPhiInLoopMerge(spvtools::opt::Instruction& inst) {
        // All of the source blocks should exist as they will either be the loop header, if we
        // happened to jump out early, the body if we break or the continuing if we break-if.
        auto* type = Type(inst.type_id());

        auto* phi_spirv_block = spirv_context_->get_instr_block(&inst);
        auto phi_blk_id = phi_spirv_block->id();

        // The merge target (which is the OpPhi SPIR-V block) is a walk stop block
        auto* loop = StopWalkingAt(phi_spirv_block->id())->As<core::ir::Loop>();
        TINT_ASSERT(loop);

        std::optional<uint32_t> default_value = std::nullopt;

        // The only way into the continue should be through `continue` calls out of the loop body,
        // which should all exist at this point as we emit the body first.
        for (uint32_t i = 0; i < inst.NumInOperands(); i += 2) {
            auto value_id = inst.GetSingleWordInOperand(i);
            auto blk_id = inst.GetSingleWordInOperand(i + 1);

            core::ir::Terminator* term = nullptr;

            // If the basic block ends in a branch conditional, then we really need to update the
            // branch which goes to the current block, not the terminator of the block we emitted
            // into. This difference is because the merge block will _also_ end up in the same IR
            // block and we can't tell the difference if we just get the terminator.
            const auto& bb = current_spirv_function_->FindBlock(blk_id);
            auto* terminator = (*bb).terminator();
            if (terminator->opcode() == spv::Op::OpBranchConditional) {
                uint32_t true_id = terminator->GetSingleWordInOperand(1);
                uint32_t false_id = terminator->GetSingleWordInOperand(2);

                auto iter = branch_conditional_to_if_.find(terminator);
                if (iter != branch_conditional_to_if_.end()) {
                    auto* if_ = iter->second;
                    if (true_id == phi_blk_id) {
                        term = if_->True()->Terminator();
                    } else if (false_id == phi_blk_id) {
                        term = if_->False()->Terminator();
                    }
                }
            }

            if (term == nullptr) {
                auto value_blk_iter = spirv_id_to_block_.find(blk_id);
                auto* value_ir_blk = value_blk_iter->second;
                term = value_ir_blk->Terminator();
            }

            if (term->Is<core::ir::Unreachable>()) {
                default_value = value_id;
                continue;
            }
            // Continue isn't a terminator into a merge. So, we must have had a branch conditional
            // where one branch continued and the other exited. We need to update the exiting block
            // as the terminator.
            if (term->Is<core::ir::Continue>()) {
                auto* if_ = term->prev->As<core::ir::If>();
                TINT_ASSERT(if_);

                if (if_->True()->Terminator()->Is<core::ir::ExitLoop>()) {
                    term = if_->True()->Terminator();
                } else {
                    term = if_->False()->Terminator();
                    TINT_ASSERT(term->Is<core::ir::ExitLoop>());
                }
            }

            AddOperandToTerminator(term, value_id);
        }

        auto* res = b_.InstructionResult(type);
        loop->AddResult(res);
        AddValue(inst.result_id(), res);

        // If there is a default value, then one of the blocks was an unreachable at the end. We
        // need to find exits where we haven't set all the params and add the default.
        if (default_value) {
            auto result_count = loop->Results().Length();
            for (auto iter : loop->Exits()) {
                if (iter->Operands().Length() >= result_count) {
                    continue;
                }

                // We haven't updated the exit yet
                auto* term = iter->As<core::ir::Terminator>();
                TINT_ASSERT(term);

                AddOperandToTerminator(term, default_value.value());
            }
        }
    }

    void EmitPhiInLoopContinue(spvtools::opt::Instruction& inst) {
        auto* type = Type(inst.type_id());

        auto* phi_spirv_block = spirv_context_->get_instr_block(&inst);

        // The continue target is a walk stop block
        auto* loop = StopWalkingAt(phi_spirv_block->id())->As<core::ir::Loop>();
        TINT_ASSERT(loop);

        std::optional<uint32_t> default_value = std::nullopt;

        // The only way into the continue should be through `continue` calls out of the loop body,
        // which should all exist at this point as we emit the body first.
        for (uint32_t i = 0; i < inst.NumInOperands(); i += 2) {
            auto value_id = inst.GetSingleWordInOperand(i);
            auto blk_id = inst.GetSingleWordInOperand(i + 1);

            auto value_blk_iter = spirv_id_to_block_.find(blk_id);
            auto* value_ir_blk = value_blk_iter->second;
            auto* term = value_ir_blk->Terminator();

            if (term->Is<core::ir::Unreachable>()) {
                default_value = value_id;
                continue;
            }

            AddOperandToTerminator(term, value_id);
        }

        // Add the block param to the continuing block
        auto* p = b_.BlockParam(type);
        loop->Continuing()->AddParam(p);
        AddValue(inst.result_id(), p);

        // If there is a default value, then one of the blocks was an unreachable at the end, this
        // can happen if the `OpBranchConditional` has one of the branches to the continue block. We
        // need to find incoming sibling branches where we haven't set all the params and add the
        // default.
        if (default_value) {
            auto param_count = loop->Continuing()->Params().Length();
            for (auto* sibling : loop->Continuing()->InboundSiblingBranches()) {
                if (sibling->Operands().Length() >= param_count) {
                    continue;
                }
                // We haven't updated the sibling yet
                auto* term = sibling->As<core::ir::Terminator>();
                TINT_ASSERT(term);

                AddOperandToTerminator(term, default_value.value());
            }
        }
    }

    void EmitPhiInLoopBody(spvtools::opt::Instruction& inst,
                           spvtools::opt::Instruction* loop_merge_inst) {
        auto continue_id = loop_merge_inst->GetSingleWordInOperand(1);

        auto* loop_header_block = spirv_context_->get_instr_block(loop_merge_inst);

        // The loop continue is a walk stop block. Since we're in the loop body itself we would have
        // added the continue target to the stop blocks when processing the header loop merge.
        auto* loop = StopWalkingAt(continue_id)->As<core::ir::Loop>();
        TINT_ASSERT(loop);

        // The only phi incoming block that makes sense on the body is from the loop header itself.
        // Any other incoming branches must be from unreachable blocks, otherwise it would have had
        // to have come from the loop header which is where the continue target will loop back too,
        // not the body label directly.
        std::optional<core::ir::Value*> value = std::nullopt;
        for (uint32_t i = 0; i < inst.NumInOperands(); i += 2) {
            auto value_id = inst.GetSingleWordInOperand(i);
            auto blk_id = inst.GetSingleWordInOperand(i + 1);

            if (blk_id != loop_header_block->id()) {
                continue;
            }

            value = Value(value_id);
        }
        TINT_ASSERT(value.has_value());
        AddValue(inst.result_id(), value.value());
    }

    void EmitPhiInLoopHeader(spvtools::opt::Instruction& inst,
                             spvtools::opt::BasicBlock* loop_header) {
        // Incoming branches should be from the parent block into the loop, and from the loop
        // continuing back into the header, each with a block_id and a value_id.
        TINT_ASSERT(inst.NumInOperands() == 4);

        auto* type = Type(inst.type_id());

        auto* phi_spirv_block = spirv_context_->get_instr_block(&inst);

        auto* loop_merge_inst = loop_header->GetLoopMergeInst();
        auto continue_id = loop_merge_inst->GetSingleWordInOperand(1);

        // The loop header is a walk stop block.
        auto* loop = StopWalkingAt(phi_spirv_block->id())->As<core::ir::Loop>();
        TINT_ASSERT(loop);

        for (uint32_t i = 0; i < inst.NumInOperands(); i += 2) {
            auto value_id = inst.GetSingleWordInOperand(i);
            auto blk_id = inst.GetSingleWordInOperand(i + 1);

            if (block_phi_values_.contains(blk_id)) {
                block_phi_values_[blk_id].push_back(value_id);
            } else {
                block_phi_values_[blk_id] = {value_id};
            }

            core::ir::Terminator* term = nullptr;
            // The referenced block hasn't been emitted yet (continue blocks have this
            // behaviour). So, store the fact that it needs to return a given value away for
            // when we do emit the block.
            //
            // We check both if we've seen the continue block, which if it's a separate block we
            // won't have seen, or if we have seen it (then it's the header itself) if the block is
            // the continue id.
            if (!spirv_id_to_block_.contains(blk_id) || blk_id == continue_id) {
                // If the continue is a separate target, we'll emit it later so just store this
                // value away to add to the next iteration.
                if (continue_id != phi_spirv_block->id()) {
                    continue_blk_phis_[continue_id].push_back(value_id);
                    continue;
                }

                // The loop header is the terminator, so synthesize a continue blockhand append
                // to that a next iteration.
                if (loop->Continuing()->IsEmpty()) {
                    b_.Append(loop->Continuing(), [&] { term = b_.NextIteration(loop); });
                } else {
                    // With multiple phis we may have already created the continuing
                    // block, so just get the terminator.
                    term = loop->Continuing()->Terminator();
                    TINT_ASSERT(term->Is<core::ir::NextIteration>());
                }

            } else {
                // This is the parent block, push this into the next iteration of the initializer.
                if (loop->Initializer()->IsEmpty()) {
                    b_.Append(loop->Initializer(), [&] { term = b_.NextIteration(loop); });
                } else {
                    term = loop->Initializer()->Terminator();
                    TINT_ASSERT(term->Is<core::ir::NextIteration>());
                }
            }

            AddOperandToTerminator(term, value_id);
        }

        // Add the block param to the body
        auto* p = b_.BlockParam(type);
        loop->Body()->AddParam(p);
        AddValue(inst.result_id(), p);
    }

    // Emit an OpPhi which is inside the merge block for a if.
    void EmitPhiInIfMerge(spvtools::opt::Instruction& inst, uint32_t header_id) {
        auto* type = Type(inst.type_id());
        core::ir::If* ctrl = nullptr;
        std::optional<core::ir::Value*> value_for_default_block = std::nullopt;

        for (uint32_t i = 0; i < inst.NumInOperands(); i += 2) {
            auto value_id = inst.GetSingleWordInOperand(i);
            auto blk_id = inst.GetSingleWordInOperand(i + 1);

            auto value_blk_iter = spirv_id_to_block_.find(blk_id);
            auto* value_ir_blk = value_blk_iter->second;

            // The block refers to the header itself, so it was the true or false branching directly
            // to the merge.
            //
            // We store this away as we may not know the if yet, we need to wait until we get the
            // control instruction and do the work later.
            if (blk_id == header_id) {
                value_for_default_block = Value(value_id);
                continue;
            }

            auto* term = value_ir_blk->Terminator();
            AddOperandToTerminator(term, value_id);

            if (auto* exit = term->As<core::ir::ExitIf>()) {
                if (ctrl) {
                    TINT_ASSERT(ctrl == exit->ControlInstruction());
                } else {
                    ctrl = exit->ControlInstruction()->As<core::ir::If>();
                    TINT_ASSERT(ctrl);
                }
            } else {
                TINT_UNREACHABLE();
            }
        }
        // No control instruction means that both true/false branches are the merge itself, so we
        // can just ignore the if, if we have a default then assign it straight through.
        if (!ctrl) {
            if (value_for_default_block) {
                AddValue(inst.result_id(), value_for_default_block.value());
            }
            return;
        }

        // If the default block needs to be assigned a value
        if (value_for_default_block) {
            core::ir::Terminator* term = nullptr;
            if (ctrl->True()->Terminator()->Operands().Length() <
                ctrl->False()->Terminator()->Operands().Length()) {
                term = ctrl->True()->Terminator();
            } else {
                term = ctrl->False()->Terminator();
            }
            term->PushOperand(value_for_default_block.value());
        }

        // Push the result into the parent
        auto* res = b_.InstructionResult(type);
        ctrl->AddResult(res);
        AddValue(inst.result_id(), res);
    }

    // Emit an OpPhi which is inside the merge block for a switch.
    void EmitPhiInSwitchMerge(spvtools::opt::Instruction& inst, uint32_t header_id) {
        auto* type = Type(inst.type_id());

        core::ir::Switch* ctrl = nullptr;
        std::optional<core::ir::Value*> value_for_default_block = std::nullopt;

        auto phi_blk_id = spirv_context_->get_instr_block(&inst)->id();

        for (uint32_t i = 0; i < inst.NumInOperands(); i += 2) {
            auto value_id = inst.GetSingleWordInOperand(i);
            auto blk_id = inst.GetSingleWordInOperand(i + 1);

            core::ir::Terminator* term = nullptr;

            // If the basic block ends in a branch conditional, then we really need to update the
            // branch which goes to the current block, not the terminator of the block we emitted
            // into. This difference is because the merge block will _also_ end up in the same IR
            // block and we can't tell the difference if we just get the terminator.
            const auto& bb = current_spirv_function_->FindBlock(blk_id);
            auto* terminator = (*bb).terminator();
            if (terminator->opcode() == spv::Op::OpBranchConditional) {
                uint32_t true_id = terminator->GetSingleWordInOperand(1);
                uint32_t false_id = terminator->GetSingleWordInOperand(2);

                auto iter = branch_conditional_to_if_.find(terminator);
                TINT_ASSERT(iter != branch_conditional_to_if_.end());

                auto* if_ = iter->second;
                if (true_id == phi_blk_id) {
                    term = if_->True()->Terminator();
                } else if (false_id == phi_blk_id) {
                    term = if_->False()->Terminator();
                }
            }

            if (term == nullptr) {
                auto value_blk_iter = spirv_id_to_block_.find(blk_id);
                auto* value_ir_blk = value_blk_iter->second;

                // In the case of a switch, the block can refer to the header of the switch, in this
                // case it means we don't have a default block and we jump over the switch itself,
                // so we need to insert this value into the terminator of the default block of the
                // switch.
                //
                // We store this away as we may not know the switch yet, we need to wait until we
                // get the control instruction and do the work later.
                if (blk_id == header_id) {
                    value_for_default_block = Value(value_id);
                    continue;
                }

                term = value_ir_blk->Terminator();
            }

            AddOperandToTerminator(term, value_id);

            if (auto* exit = term->As<core::ir::ExitSwitch>()) {
                if (ctrl) {
                    TINT_ASSERT(ctrl == exit->ControlInstruction());
                } else {
                    ctrl = exit->ControlInstruction()->As<core::ir::Switch>();
                    TINT_ASSERT(ctrl);
                }
            } else {
                TINT_UNREACHABLE();
            }
        }

        // No control instruction means there were no cases, so we can just ignore the switch, if we
        // have a default then assign it straight through.
        if (!ctrl) {
            if (value_for_default_block) {
                AddValue(inst.result_id(), value_for_default_block.value());
            }
            return;
        }

        // If the default block needs to be assigned a value
        if (value_for_default_block) {
            ctrl->DefaultBlock()->Terminator()->PushOperand(value_for_default_block.value());
        }

        // Push the result into the switch
        auto* res = b_.InstructionResult(type);
        ctrl->AddResult(res);
        AddValue(inst.result_id(), res);
    }

    void EmitImage(const spvtools::opt::Instruction& inst) {
        auto* si = Value(inst.GetSingleWordInOperand(0));
        Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(
                 Type(inst.type_id()), spirv::BuiltinFn::kImage,
                 Vector{si->Type()->As<spirv::type::SampledImage>()->Image()}, Args(inst, 2)),
             inst.result_id());
    }

    void EmitSampledImage(const spvtools::opt::Instruction& inst) {
        auto* tex = Value(inst.GetSingleWordInOperand(0));
        Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(Type(inst.type_id()),
                                                     spirv::BuiltinFn::kSampledImage,
                                                     Vector{tex->Type()}, Args(inst, 2)),
             inst.result_id());
    }

    void EmitImageFetchOrRead(const spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        auto sampled_image = Value(inst.GetSingleWordInOperand(0));
        auto* coord = Value(inst.GetSingleWordInOperand(1));

        Vector<core::ir::Value*, 4> args = {sampled_image, coord};

        if (inst.NumInOperands() > 2) {
            uint32_t literal_mask = inst.GetSingleWordInOperand(2);
            args.Push(b_.Constant(u32(literal_mask)));

            if (literal_mask != 0) {
                args.Push(Value(inst.GetSingleWordInOperand(3)));
            }
        } else {
            args.Push(b_.Zero(ty_.u32()));
        }

        Emit(b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn, args), inst.result_id());
    }

    void EmitImageGatherDref(const spvtools::opt::Instruction& inst) {
        auto sampled_image = Value(inst.GetSingleWordInOperand(0));
        auto* coord = Value(inst.GetSingleWordInOperand(1));
        auto* dref = Value(inst.GetSingleWordInOperand(2));

        Vector<core::ir::Value*, 4> args = {sampled_image, coord, dref};

        if (inst.NumInOperands() > 3) {
            uint32_t literal_mask = inst.GetSingleWordInOperand(3);
            args.Push(b_.Constant(u32(literal_mask)));

            if (literal_mask != 0) {
                TINT_ASSERT(static_cast<spv::ImageOperandsMask>(literal_mask) ==
                            spv::ImageOperandsMask::ConstOffset);
                TINT_ASSERT(inst.NumInOperands() > 4);
                args.Push(Value(inst.GetSingleWordInOperand(4)));
            }
        } else {
            args.Push(b_.Zero(ty_.u32()));
        }

        Emit(b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()),
                                             spirv::BuiltinFn::kImageDrefGather, args),
             inst.result_id());
    }

    void EmitImageGather(const spvtools::opt::Instruction& inst) {
        auto sampled_image = Value(inst.GetSingleWordInOperand(0));
        auto* coord = Value(inst.GetSingleWordInOperand(1));
        auto* comp = Value(inst.GetSingleWordInOperand(2));

        Vector<core::ir::Value*, 4> args = {sampled_image, coord, comp};

        if (inst.NumInOperands() > 3) {
            uint32_t literal_mask = inst.GetSingleWordInOperand(3);
            args.Push(b_.Constant(u32(literal_mask)));

            if (literal_mask != 0) {
                TINT_ASSERT(static_cast<spv::ImageOperandsMask>(literal_mask) ==
                            spv::ImageOperandsMask::ConstOffset);
                TINT_ASSERT(inst.NumInOperands() > 4);
                args.Push(Value(inst.GetSingleWordInOperand(4)));
            }
        } else {
            args.Push(b_.Zero(ty_.u32()));
        }

        Emit(b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()), spirv::BuiltinFn::kImageGather,
                                             args),
             inst.result_id());
    }

    void EmitImageSample(const spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        auto sampled_image = Value(inst.GetSingleWordInOperand(0));
        auto* coord = Value(inst.GetSingleWordInOperand(1));

        Vector<core::ir::Value*, 4> args = {sampled_image, coord};

        if (inst.NumInOperands() > 2) {
            uint32_t literal_mask = inst.GetSingleWordInOperand(2);
            args.Push(b_.Constant(u32(literal_mask)));

            if (literal_mask != 0) {
                TINT_ASSERT(inst.NumInOperands() > 3);
            }

            for (uint32_t i = 3; i < inst.NumInOperands(); ++i) {
                args.Push(Value(inst.GetSingleWordInOperand(i)));
            }
        } else {
            args.Push(b_.Zero(ty_.u32()));
        }

        Emit(b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn, args), inst.result_id());
    }

    bool HasLod(uint32_t mask) {
        return (mask & static_cast<uint32_t>(spv::ImageOperandsMask::Lod)) != 0;
    }

    void EmitImageSampleDepth(const spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        auto sampled_image = Value(inst.GetSingleWordInOperand(0));
        auto* coord = Value(inst.GetSingleWordInOperand(1));
        auto* dref = Value(inst.GetSingleWordInOperand(2));

        Vector<core::ir::Value*, 4> args = {sampled_image, coord, dref};

        if (inst.NumInOperands() > 3) {
            uint32_t literal_mask = inst.GetSingleWordInOperand(3);
            args.Push(b_.Constant(u32(literal_mask)));

            if (literal_mask != 0) {
                TINT_ASSERT(inst.NumInOperands() > 4);
            }

            for (uint32_t i = 4; i < inst.NumInOperands(); ++i) {
                args.Push(Value(inst.GetSingleWordInOperand(i)));
            }

            if (HasLod(literal_mask)) {
                core::ir::Value* lod = args[4];
                TINT_ASSERT(lod->Is<core::ir::Constant>());
                TINT_ASSERT(lod->Type()->As<core::type::F32>());

                auto v = lod->As<core::ir::Constant>()->Value()->ValueAs<float>();
                if (v != 0.0f) {
                    TINT_ICE() << "Dref LOD values must be 0.0";
                }
            }
        } else {
            args.Push(b_.Zero(ty_.u32()));
        }

        Emit(b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn, args), inst.result_id());
    }

    void EmitImageWrite(const spvtools::opt::Instruction& inst) {
        auto* image = Value(inst.GetSingleWordInOperand(0));
        auto* coord = Value(inst.GetSingleWordInOperand(1));
        core::ir::Value* texel = Value(inst.GetSingleWordInOperand(2));

        // Our intrinsic has a vec4 type, which matches what WGSL expects. Instead of creating more
        // intrinsic entries, just turn the texel into a vec4.
        auto* texel_ty = texel->Type();
        if (texel_ty->IsScalar()) {
            auto* c = b_.Construct(ty_.vec4(texel_ty), texel);
            EmitWithoutSpvResult(c);
            texel = c->Result();
        } else {
            auto* vec_ty = texel_ty->As<core::type::Vector>();
            TINT_ASSERT(vec_ty);

            core::ir::Instruction* c = nullptr;
            if (vec_ty->Width() == 2) {
                c = b_.Construct(ty_.vec4(vec_ty->Type()), texel, b_.Zero(vec_ty));
            } else if (vec_ty->Width() == 3) {
                c = b_.Construct(ty_.vec4(vec_ty->Type()), texel, b_.Zero(vec_ty->Type()));
            }
            if (c != nullptr) {
                EmitWithoutSpvResult(c);
                texel = c->Result();
            }
        }

        Vector<core::ir::Value*, 4> args = {image, coord, texel};
        if (inst.NumInOperands() > 3) {
            uint32_t literal_mask = inst.GetSingleWordInOperand(3);
            args.Push(b_.Constant(u32(literal_mask)));
            TINT_ASSERT(literal_mask == 0);
        } else {
            args.Push(b_.Zero(ty_.u32()));
        }

        Emit(b_.Call<spirv::ir::BuiltinCall>(ty_.void_(), spirv::BuiltinFn::kImageWrite, args),
             inst.result_id());
    }

    void EmitImageQuery(const spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        auto* image = Value(inst.GetSingleWordInOperand(0));

        auto* ty = Type(inst.type_id());
        Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(ty, fn, Vector{ty->DeepestElement()}, image),
             inst.result_id());
    }

    void EmitImageQuerySizeLod(const spvtools::opt::Instruction& inst) {
        auto* image = Value(inst.GetSingleWordInOperand(0));
        auto* level = Value(inst.GetSingleWordInOperand(1));

        auto* ty = Type(inst.type_id());
        Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(ty, spirv::BuiltinFn::kImageQuerySizeLod,
                                                     Vector{ty->DeepestElement()}, image, level),
             inst.result_id());
    }

    void EmitAtomicSigned(const spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        core::ir::Value* v = Value(inst.GetSingleWordOperand(2));
        TINT_ASSERT(v->Type()->UnwrapPtr()->Is<core::type::I32>());
        EmitSpirvBuiltinCall(inst, fn);
    }

    void EmitAtomicUnsigned(const spvtools::opt::Instruction& inst, spirv::BuiltinFn fn) {
        core::ir::Value* v = Value(inst.GetSingleWordOperand(2));
        TINT_ASSERT(v->Type()->UnwrapPtr()->Is<core::type::U32>());
        EmitSpirvBuiltinCall(inst, fn);
    }

    void EmitArrayLength(const spvtools::opt::Instruction& inst) {
        auto strct = Value(inst.GetSingleWordInOperand(0));
        auto field_index = inst.GetSingleWordInOperand(1);

        auto* ptr = strct->Type()->As<core::type::Pointer>();
        TINT_ASSERT(ptr);

        auto* ty = ptr->StoreType()->As<core::type::Struct>();
        TINT_ASSERT(ty);

        auto* access =
            b_.Access(ty_.ptr(ptr->AddressSpace(), ty->Members().Back()->Type(), ptr->Access()),
                      strct, u32(field_index));
        EmitWithoutSpvResult(access);

        Emit(b_.Call(Type(inst.type_id()), core::BuiltinFn::kArrayLength, Vector{access->Result()}),
             inst.result_id());
    }

    void EmitControlBarrier(const spvtools::opt::Instruction& inst) {
        auto get_constant = [&](uint32_t idx) {
            uint32_t id = inst.GetSingleWordOperand(idx);
            if (auto* constant = spirv_context_->get_constant_mgr()->FindDeclaredConstant(id)) {
                return constant->GetU32();
            }
            TINT_ICE() << "invalid or missing operands for control barrier";
        };

        uint32_t execution = get_constant(0);
        uint32_t memory = get_constant(1);
        uint32_t semantics = get_constant(2);

        if (execution != uint32_t(spv::Scope::Workgroup)) {
            TINT_ICE() << "unsupported control barrier execution scope: "
                       << "expected Workgroup (2), got: " << execution;
        }

        if (semantics & uint32_t(spv::MemorySemanticsMask::AcquireRelease)) {
            semantics &= ~static_cast<uint32_t>(spv::MemorySemanticsMask::AcquireRelease);
        } else {
            TINT_ICE() << "control barrier semantics requires acquire and release";
        }
        if (memory != uint32_t(spv::Scope::Workgroup)) {
            TINT_ICE() << "control barrier requires workgroup memory scope";
        }

        if (semantics & uint32_t(spv::MemorySemanticsMask::WorkgroupMemory)) {
            EmitWithoutSpvResult(b_.Call(ty_.void_(), core::BuiltinFn::kWorkgroupBarrier));
            semantics &= ~static_cast<uint32_t>(spv::MemorySemanticsMask::WorkgroupMemory);
        }

        if (semantics & uint32_t(spv::MemorySemanticsMask::UniformMemory)) {
            EmitWithoutSpvResult(b_.Call(ty_.void_(), core::BuiltinFn::kStorageBarrier));
            semantics &= ~static_cast<uint32_t>(spv::MemorySemanticsMask::UniformMemory);
        }

        if (semantics & uint32_t(spv::MemorySemanticsMask::ImageMemory)) {
            EmitWithoutSpvResult(b_.Call(ty_.void_(), core::BuiltinFn::kTextureBarrier));
            semantics &= ~static_cast<uint32_t>(spv::MemorySemanticsMask::ImageMemory);
        }

        if (semantics) {
            TINT_ICE() << "unsupported control barrier semantics: " << semantics;
        }
    }

    void CheckAtomicNotFloat(const spvtools::opt::Instruction& inst) {
        auto* ty = Type(inst.type_id());
        if (ty->IsFloatScalar()) {
            TINT_ICE() << "Atomic operations on floating point values not supported.";
        }
    }

    void EmitAtomicStore(const spvtools::opt::Instruction& inst) {
        auto* v = Value(inst.GetSingleWordInOperand(0));
        auto* ty = v->Type()->UnwrapPtr();
        if (ty->IsFloatScalar()) {
            TINT_ICE() << "Atomic operations on floating point values not supported.";
        }

        EmitWithoutSpvResult(b_.Call<spirv::ir::BuiltinCall>(
            ty_.void_(), spirv::BuiltinFn::kAtomicStore, Args(inst, 0)));
    }

    void EmitBitcast(const spvtools::opt::Instruction& inst) {
        auto val = Value(inst.GetSingleWordInOperand(0));
        auto ty = Type(inst.type_id());
        Emit(b_.Bitcast(ty, val), inst.result_id());
    }

    core::ir::ControlInstruction* StopWalkingAt(uint32_t id) {
        auto iter = walk_stop_blocks_.find(id);
        if (iter != walk_stop_blocks_.end()) {
            return iter->second;
        }
        return nullptr;
    }

    core::ir::Loop* ContinueTarget(uint32_t id) {
        auto iter = continue_targets_.find(id);
        if (iter != continue_targets_.end()) {
            return iter->second;
        }
        return nullptr;
    }

    void EmitBranch(spvtools::opt::Instruction& inst) {
        auto dest_id = inst.GetSingleWordInOperand(0);

        // Disallow fallthrough
        for (auto& switch_blocks : current_switch_blocks_) {
            if (switch_blocks.count(dest_id) != 0) {
                TINT_ICE() << "switch fallthrough not supported by the SPIR-V reader";
            }
        }

        // The destination is a continuing block, so insert a `continue`
        if (auto* loop = ContinueTarget(dest_id)) {
            auto* new_inst = b_.Continue(loop);
            auto id = spirv_context_->get_instr_block(&inst)->id();
            inst_to_spirv_block_[new_inst] = id;
            EmitWithoutResult(new_inst);
            return;
        }
        // If this is branching to a previous merge block then we're done. It can be a previous
        // merge block in the case of an `if` breaking out of a `switch` or `loop`.
        if (auto* ctrl_inst = StopWalkingAt(dest_id)) {
            if (auto* loop = ctrl_inst->As<core::ir::Loop>()) {
                // Going to the merge in a loop body has to be a break regardless of nesting level.

                if (InBlock(loop->Body()) && !InBlock(loop->Continuing())) {
                    EmitWithoutResult(b_.Exit(ctrl_inst));
                }
            } else if (ctrl_inst->Is<core::ir::Switch>()) {
                EmitWithoutResult(b_.Exit(ctrl_inst));
            }
            return;
        }

        TINT_ASSERT(current_spirv_function_);
        const auto& bb = current_spirv_function_->FindBlock(dest_id);

        EmitBlock(current_block_, *bb);
    }

    // Given a true and false branch find if there is a common convergence point before the merge
    // block.
    std::optional<uint32_t> FindPremergeId(uint32_t true_id,
                                           uint32_t false_id,
                                           std::optional<uint32_t> merge_id) {
        auto* cfg = spirv_context_->cfg();

        // We need a merge block, the true and false to be unique and the true and false to not be
        // the merge.
        if (!merge_id || true_id == false_id || true_id == merge_id || false_id == merge_id) {
            return std::nullopt;
        }

        // Get the list of blocks from the true branch to the merge
        std::list<spvtools::opt::BasicBlock*> true_blocks;
        cfg->ComputeStructuredOrder(
            current_spirv_function_, &*(current_spirv_function_->FindBlock(true_id)),
            &*(current_spirv_function_->FindBlock(merge_id.value())), &true_blocks);

        // Get the list of blocks from the false branch to the merge
        std::list<spvtools::opt::BasicBlock*> false_blocks;
        cfg->ComputeStructuredOrder(
            current_spirv_function_, &*(current_spirv_function_->FindBlock(false_id)),
            &*(current_spirv_function_->FindBlock(merge_id.value())), &false_blocks);

        auto& true_end = true_blocks.back();
        auto& false_end = false_blocks.back();

        // We only consider the block as returning if it didn't return through
        // the merge block. (I.e. it's a direct exit from inside the branch
        // itself.
        bool true_returns = true_end->id() != merge_id && true_end->IsReturn();
        bool false_returns = false_end->id() != merge_id && false_end->IsReturn();
        // If one of the blocks returns but the other doesn't, then we can't
        // have a premerge block.
        if (true_returns != false_returns) {
            return std::nullopt;
        }

        // If they don't return, both blocks must merge to the same place.
        if (!true_returns && (true_end->id() != false_end->id())) {
            return std::nullopt;
        }

        // If these aren't returns, then remove the merge blocks.
        if (!true_returns) {
            true_blocks.pop_back();
            false_blocks.pop_back();
        }

        std::optional<uint32_t> id = std::nullopt;
        while (!true_blocks.empty() && !false_blocks.empty()) {
            auto* tb = true_blocks.back();
            if (tb != false_blocks.back()) {
                break;
            }

            id = tb->id();

            true_blocks.pop_back();
            false_blocks.pop_back();
        }

        // If this is already a stop block, so it can't be a premerge
        if (id.has_value() && walk_stop_blocks_.contains(id.value())) {
            return std::nullopt;
        }
        return id;
    }

    core::ir::ControlInstruction* ExitFor(core::ir::ControlInstruction* ctrl,
                                          core::ir::ControlInstruction* parent) {
        // If you have a BranchConditional inside a BranchConditional where
        // the inner does not have a merge block, it can branch out to the
        // merge of the outer conditional. But, WGSL doesn't allow that, so
        // just treat it as an exit of the inner block.
        if (ctrl->Is<core::ir::If>() && parent->Is<core::ir::If>()) {
            return parent;
        }
        return ctrl;
    }

    core::ir::Instruction* EmitBranchStopBlock(core::ir::ControlInstruction* ctrl,
                                               core::ir::If* if_,
                                               core::ir::Block* blk,
                                               uint32_t target) {
        if (auto* loop = ContinueTarget(target)) {
            auto* cont = b_.Continue(loop);
            blk->Append(cont);
            return cont;
        }

        auto iter = merge_to_premerge_.find(target);
        if (iter != merge_to_premerge_.end()) {
            // Branch to a merge block, but skipping over an expected premerge block
            // so we need a guard.
            if (!iter->second.condition) {
                b_.InsertBefore(iter->second.parent,
                                [&] { iter->second.condition = b_.Var("execute_premerge", true); });
            }
            b_.Append(blk, [&] { b_.Store(iter->second.condition, false); });
        }

        auto* exit = b_.Exit(ExitFor(ctrl, if_));
        blk->Append(exit);
        return exit;
    }

    bool ProcessBranchAsLoopHeader(core::ir::Value* cond, uint32_t true_id, uint32_t false_id) {
        bool true_is_header = loop_headers_.count(true_id) > 0;
        bool false_is_header = loop_headers_.count(false_id) > 0;

        if (!true_is_header && !false_is_header) {
            return false;
        }

        core::ir::Loop* loop = nullptr;
        uint32_t merge_id = 0;

        if (true_is_header) {
            const auto& bb_header = current_spirv_function_->FindBlock(true_id);
            merge_id = (*bb_header).MergeBlockIdIfAny();

            loop = loop_headers_[true_id];

        } else {
            const auto& bb_header = current_spirv_function_->FindBlock(false_id);
            merge_id = (*bb_header).MergeBlockIdIfAny();

            loop = loop_headers_[false_id];
        }
        TINT_ASSERT(merge_id > 0);

        // The only time a loop continuing will be in current blocks is if
        // we're inside the continuing block itself.
        //
        // Note, we may _not_ be in the IR continuing block. This can happen
        // in the case of a SPIR-V loop where the header_id and continue_id
        // are the same. We'll be emitting into the IR body, but branch to
        // the header because that's also the continuing in SPIR-V.
        if (current_blocks_.count(loop->Continuing()) != 0u) {
            if (true_id == merge_id && false_is_header) {
                EmitWithoutResult(b_.BreakIf(loop, cond));
                return true;
            }
            if (false_id == merge_id && true_is_header) {
                auto* val = b_.Not(cond->Type(), cond);
                EmitWithoutSpvResult(val);
                EmitWithoutResult(b_.BreakIf(loop, val));
                return true;
            }
        }
        return false;
    }

    void EmitPremergeBlock(uint32_t merge_id,
                           uint32_t premerge_start_id,
                           core::ir::If* premerge_if_) {
        auto iter = merge_to_premerge_.find(merge_id);
        TINT_ASSERT(iter != merge_to_premerge_.end());

        // If we created a condition guard, we need to swap the premerge `true` condition with
        // the condition variable.
        if (iter->second.condition) {
            auto* premerge_cond = b_.Load(iter->second.condition);
            EmitWithoutSpvResult(premerge_cond);
            premerge_if_->SetOperand(core::ir::If::kConditionOperandOffset,
                                     premerge_cond->Result());
        }
        merge_to_premerge_.erase(iter);

        EmitWithoutResult(premerge_if_);

        const auto& bb_premerge = current_spirv_function_->FindBlock(premerge_start_id);
        EmitBlockParent(premerge_if_->True(), *bb_premerge);
        if (!premerge_if_->True()->Terminator()) {
            premerge_if_->True()->Append(b_.Exit(premerge_if_));
        }

        premerge_if_->False()->Append(b_.Unreachable());
    }

    void EmitIfBranch(uint32_t id, core::ir::If* if_, core::ir::Block* blk) {
        const auto& bb = current_spirv_function_->FindBlock(id);
        EmitBlockParent(blk, *bb);
        if (!blk->Terminator()) {
            blk->Append(b_.Exit(if_));
        }
    }

    void EmitBranchConditional(const spvtools::opt::BasicBlock& bb,
                               const spvtools::opt::Instruction& inst) {
        auto cond = Value(inst.GetSingleWordInOperand(0));
        auto true_id = inst.GetSingleWordInOperand(1);
        auto false_id = inst.GetSingleWordInOperand(2);

        if (ProcessBranchAsLoopHeader(cond, true_id, false_id)) {
            return;
        }

        // If the true and false block are the same, then we change the condition into
        // `cond || true` so that we always take the true block, the false block will be marked
        // unreachable.
        if (true_id == false_id) {
            auto* binary = b_.Binary(core::BinaryOp::kOr, cond->Type(), cond, b_.Constant(true));
            EmitWithoutSpvResult(binary);
            cond = binary->Result();
        }

        auto* if_ = b_.If(cond);
        EmitWithoutResult(if_);

        branch_conditional_to_if_.insert({&inst, if_});

        std::optional<uint32_t> merge_id = std::nullopt;

        auto* merge_inst = bb.GetMergeInst();
        if (bb.GetLoopMergeInst()) {
            // If this is a loop merge block, then the merge instruction is for
            // the loop, not the branch conditional.
            merge_inst = nullptr;
        } else if (merge_inst != nullptr) {
            merge_id = merge_inst->GetSingleWordInOperand(0);
            walk_stop_blocks_.insert({merge_id.value(), if_});
        }

        TINT_ASSERT(current_spirv_function_);

        // Determine if there is a premerge block to handle
        std::optional<uint32_t> premerge_start_id = FindPremergeId(true_id, false_id, merge_id);

        // If we found the start of a premerge, push it onto the merge stack so this ends up being a
        // temporary merge block for the if branches.
        core::ir::If* premerge_if_ = nullptr;
        if (premerge_start_id.has_value()) {
            // Must have a merge to have a premerge
            merge_to_premerge_.insert({merge_id.value(), PremergeInfo{if_, {}}});
            premerge_if_ = b_.If(b_.Constant(true));
            walk_stop_blocks_.insert({premerge_start_id.value(), premerge_if_});
        }
        if (auto* ctrl = StopWalkingAt(true_id)) {
            auto* new_inst = EmitBranchStopBlock(ctrl, if_, if_->True(), true_id);
            inst_to_spirv_block_[new_inst] = bb.id();
        } else {
            EmitIfBranch(true_id, if_, if_->True());
        }

        // Pre-SPIRV 1.6 the true and false blocks could be the same. If that's the case then we
        // will have changed the condition and the false block is now unreachable.
        if (false_id == true_id) {
            if_->False()->Append(b_.Unreachable());
        } else if (auto* ctrl = StopWalkingAt(false_id)) {
            auto* new_inst = EmitBranchStopBlock(ctrl, if_, if_->False(), false_id);
            inst_to_spirv_block_[new_inst] = bb.id();
        } else {
            EmitIfBranch(false_id, if_, if_->False());
        }

        // There was a premerge, remove it from the merge stack and then emit the premerge into an
        // `if true` block in order to maintain re-convergence guarantees. The premerge will contain
        // all the blocks up to the merge block.
        if (premerge_start_id.has_value()) {
            EmitPremergeBlock(merge_id.value(), premerge_start_id.value(), premerge_if_);
        }

        // Emit the merge block if it exists.
        if (merge_id.has_value()) {
            const auto& bb_merge = current_spirv_function_->FindBlock(merge_id.value());
            EmitBlock(current_block_, *bb_merge);
        }
    }

    void EmitLoop(const spvtools::opt::BasicBlock& bb) {
        // This just handles creating the loop itself, the rest of the processing
        // of the continue and merge blocks will be handled when we deal with the
        // LoopMerge instruction itself. We have to setup the loop early in order
        // to capture instructions which come in the header before the LoopMerge.
        auto* loop = b_.Loop();
        EmitWithoutResult(loop);

        // A `loop` header block can also be the merge block for an `if`. In that the case, replace
        // the `if` information in the stop blocks with the loop as this must be the `if` merge
        // block and the `if` is complete.
        walk_stop_blocks_[bb.id()] = loop;
    }

    void EmitLoopMerge(const spvtools::opt::BasicBlock& bb,
                       const spvtools::opt::Instruction& inst) {
        auto merge_id = inst.GetSingleWordInOperand(0);
        auto continue_id = inst.GetSingleWordInOperand(1);
        auto header_id = bb.id();

        // The loop was created in `EmitLoop` and set as the stop block value for
        // the header block. Retrieve the loop from the stop list.
        auto* loop = StopWalkingAt(header_id)->As<core::ir::Loop>();
        TINT_ASSERT(loop);

        loop_headers_.insert({header_id, loop});
        continue_targets_.insert({continue_id, loop});

        // Insert the stop blocks
        walk_stop_blocks_.insert({merge_id, loop});
        if (continue_id != header_id) {
            walk_stop_blocks_.insert({continue_id, loop});
        }

        // The remainder of the loop body will process when we hit the
        // BranchConditional or Branch after the LoopMerge. We're already
        // processing into the loop body from the `EmitLoop` code above so just
        // continue emitting.

        // The merge block will be emitted by the `EmitBlock` code after the
        // instructions in the loop header are emitted.
    }

    void EmitSwitch(const spvtools::opt::BasicBlock& bb, const spvtools::opt::Instruction& inst) {
        auto* selector = Value(inst.GetSingleWordInOperand(0));
        auto default_id = inst.GetSingleWordInOperand(1);

        auto* switch_ = b_.Switch(selector);
        EmitWithoutResult(switch_);

        auto* merge_inst = bb.GetMergeInst();
        TINT_ASSERT(merge_inst);

        auto merge_id = merge_inst->GetSingleWordInOperand(0);
        walk_stop_blocks_.insert({merge_id, switch_});

        current_switch_blocks_.push_back({});
        auto& switch_blocks = current_switch_blocks_.back();

        auto* default_blk = b_.DefaultCase(switch_);
        if (default_id != merge_id) {
            switch_blocks.emplace(default_id);

            const auto& bb_default = current_spirv_function_->FindBlock(default_id);
            EmitBlockParent(default_blk, *bb_default);
        }
        if (!default_blk->Terminator()) {
            default_blk->Append(b_.ExitSwitch(switch_));
        }

        std::unordered_map<uint32_t, core::ir::Switch::Case*> block_id_to_case;
        block_id_to_case[default_id] = &(switch_->Cases().Back());

        for (uint32_t i = 2; i < inst.NumInOperandWords(); i += 2) {
            auto blk_id = inst.GetSingleWordInOperand(i + 1);

            if (blk_id != merge_id) {
                switch_blocks.emplace(blk_id);
            }
        }

        // For each selector.
        for (uint32_t i = 2; i < inst.NumInOperandWords(); i += 2) {
            auto literal = inst.GetSingleWordInOperand(i);
            auto blk_id = inst.GetSingleWordInOperand(i + 1);

            core::ir::Constant* sel = nullptr;
            if (selector->Type()->Is<core::type::I32>()) {
                sel = b_.Constant(i32(literal));
            } else {
                sel = b_.Constant(u32(literal));
            }

            // Determine if we've seen this block and should combine selectors
            auto iter = block_id_to_case.find(blk_id);
            if (iter != block_id_to_case.end()) {
                iter->second->selectors.Push(core::ir::Switch::CaseSelector{sel});
                continue;
            }

            core::ir::Block* blk = b_.Case(switch_, Vector{sel});
            if (blk_id != merge_id) {
                const auto& basic_block = current_spirv_function_->FindBlock(blk_id);
                EmitBlockParent(blk, *basic_block);
            }
            if (!blk->Terminator()) {
                blk->Append(b_.ExitSwitch(switch_));
            }
            block_id_to_case[blk_id] = &(switch_->Cases().Back());
        }

        current_switch_blocks_.pop_back();

        const auto& bb_merge = current_spirv_function_->FindBlock(merge_id);
        EmitBlock(current_block_, *bb_merge);
    }

    Vector<core::ir::Value*, 4> Args(const spvtools::opt::Instruction& inst, uint32_t start) {
        Vector<core::ir::Value*, 4> args;
        for (uint32_t i = start; i < inst.NumOperandWords(); i++) {
            args.Push(Value(inst.GetSingleWordOperand(i)));
        }
        return args;
    }

    void EmitBuiltinCall(const spvtools::opt::Instruction& inst, core::BuiltinFn fn) {
        Emit(b_.Call(Type(inst.type_id()), fn, Args(inst, 2)), inst.result_id());
    }

    void EmitSpirvExplicitBuiltinCall(const spvtools::opt::Instruction& inst,
                                      spirv::BuiltinFn fn,
                                      uint32_t first_operand_idx = 2) {
        Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn,
                                                     Vector{Type(inst.type_id())->DeepestElement()},
                                                     Args(inst, first_operand_idx)),
             inst.result_id());
    }

    void EmitSpirvBuiltinCall(const spvtools::opt::Instruction& inst,
                              spirv::BuiltinFn fn,
                              uint32_t first_operand_idx = 2) {
        Emit(b_.Call<spirv::ir::BuiltinCall>(Type(inst.type_id()), fn,
                                             Args(inst, first_operand_idx)),
             inst.result_id());
    }

    void EmitBitCount(const spvtools::opt::Instruction& inst) {
        auto* res_ty = Type(inst.type_id());
        Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(res_ty, spirv::BuiltinFn::kBitCount,
                                                     Vector{res_ty->DeepestElement()},
                                                     Args(inst, 2)),
             inst.result_id());
    }

    /// @param inst the SPIR-V instruction
    /// Note: This isn't technically correct, but there is no `kill` equivalent in WGSL. The closets
    /// we have is `discard` which maps to `OpDemoteToHelperInvocation` in SPIR-V.
    void EmitKill([[maybe_unused]] const spvtools::opt::Instruction& inst) {
        // TODO(430084563): Log a warning that the `OpKill` or `OpTerminateInvocation` will not have
        // the same semantics as the original SPIR-V.
        EmitWithoutResult(b_.Discard());

        // An `OpKill` is a terminator in SPIR-V. `discard` is not a terminator in WGSL. After the
        // `discard` we inject a `return` for the current function. This is similar in spirit to
        // what `OpKill` does although not totally correct (i.e. we don't early return from calling
        // functions, just the function where `OpKill` was emitted. There are also limited places in
        // which `OpKill` can be used. So, we don't have to worry about it in a `continuing` block
        // because the continuing must end with a branching terminator which `OpKill` does not
        // branch.
        if (current_function_->ReturnType()->Is<core::type::Void>()) {
            EmitWithoutResult(b_.Return(current_function_));
        } else {
            EmitWithoutResult(
                b_.Return(current_function_, b_.Zero(current_function_->ReturnType())));
        }
    }

    /// @param inst the SPIR-V instruction for OpCopyObject
    void EmitCopyObject(const spvtools::opt::Instruction& inst) {
        // Make the result Id a pointer to the original copied value.
        auto* v = Value(inst.GetSingleWordOperand(2));

        if (v->Type()->IsHandle() || v->Type()->Is<spirv::type::SampledImage>()) {
            values_.Add(inst.result_id(), v);
        } else {
            auto* l = b_.Let(v);
            Emit(l, inst.result_id());
        }
    }

    /// @param inst the SPIR-V instruction for OpCopyMemory
    void EmitCopyMemory(const spvtools::opt::Instruction& inst) {
        auto load = b_.Load(Value(inst.GetSingleWordOperand(1)));
        EmitWithoutSpvResult(load);
        EmitWithoutResult(b_.Store(Value(inst.GetSingleWordOperand(0)), load));
    }

    /// @param inst the SPIR-V instruction for OpExtInst
    void EmitExtInst(const spvtools::opt::Instruction& inst) {
        auto inst_set = inst.GetSingleWordInOperand(0);
        if (ignored_imports_.count(inst_set) > 0) {
            // Ignore it but don't error out.
            return;
        }
        if (glsl_std_450_imports_.count(inst_set) > 0) {
            EmitGlslStd450ExtInst(inst);
            return;
        }

        TINT_UNIMPLEMENTED() << "unhandled extended instruction import with ID "
                             << inst.GetSingleWordInOperand(0);
    }

    // Returns the WGSL standard library function for the given GLSL.std.450 extended instruction
    // operation code. This handles GLSL functions which directly translate to the WGSL equivalent.
    // Any non-direct translation is returned as `kNone`.
    core::BuiltinFn GetGlslStd450WgslEquivalentFuncName(uint32_t ext_opcode) {
        switch (ext_opcode) {
            case GLSLstd450Acos:
                return core::BuiltinFn::kAcos;
            case GLSLstd450Acosh:
                return core::BuiltinFn::kAcosh;
            case GLSLstd450Asin:
                return core::BuiltinFn::kAsin;
            case GLSLstd450Asinh:
                return core::BuiltinFn::kAsinh;
            case GLSLstd450Atan:
                return core::BuiltinFn::kAtan;
            case GLSLstd450Atanh:
                return core::BuiltinFn::kAtanh;
            case GLSLstd450Atan2:
                return core::BuiltinFn::kAtan2;
            case GLSLstd450Ceil:
                return core::BuiltinFn::kCeil;
            case GLSLstd450Cos:
                return core::BuiltinFn::kCos;
            case GLSLstd450Cosh:
                return core::BuiltinFn::kCosh;
            case GLSLstd450Cross:
                return core::BuiltinFn::kCross;
            case GLSLstd450Degrees:
                return core::BuiltinFn::kDegrees;
            case GLSLstd450Determinant:
                return core::BuiltinFn::kDeterminant;
            case GLSLstd450Distance:
                return core::BuiltinFn::kDistance;
            case GLSLstd450Exp:
                return core::BuiltinFn::kExp;
            case GLSLstd450Exp2:
                return core::BuiltinFn::kExp2;
            case GLSLstd450FAbs:
                return core::BuiltinFn::kAbs;
            case GLSLstd450FSign:
                return core::BuiltinFn::kSign;
            case GLSLstd450Floor:
                return core::BuiltinFn::kFloor;
            case GLSLstd450Fract:
                return core::BuiltinFn::kFract;
            case GLSLstd450Fma:
                return core::BuiltinFn::kFma;
            case GLSLstd450InverseSqrt:
                return core::BuiltinFn::kInverseSqrt;
            case GLSLstd450Length:
                return core::BuiltinFn::kLength;
            case GLSLstd450Log:
                return core::BuiltinFn::kLog;
            case GLSLstd450Log2:
                return core::BuiltinFn::kLog2;
            case GLSLstd450NClamp:
            case GLSLstd450FClamp:  // FClamp is less prescriptive about NaN operands
                return core::BuiltinFn::kClamp;
            case GLSLstd450ModfStruct:
                return core::BuiltinFn::kModf;
            case GLSLstd450FrexpStruct:
                return core::BuiltinFn::kFrexp;
            case GLSLstd450NMin:
            case GLSLstd450FMin:  // FMin is less prescriptive about NaN operands
                return core::BuiltinFn::kMin;
            case GLSLstd450NMax:
            case GLSLstd450FMax:  // FMax is less prescriptive about NaN operands
                return core::BuiltinFn::kMax;
            case GLSLstd450FMix:
                return core::BuiltinFn::kMix;
            case GLSLstd450PackSnorm4x8:
                return core::BuiltinFn::kPack4X8Snorm;
            case GLSLstd450PackUnorm4x8:
                return core::BuiltinFn::kPack4X8Unorm;
            case GLSLstd450PackSnorm2x16:
                return core::BuiltinFn::kPack2X16Snorm;
            case GLSLstd450PackUnorm2x16:
                return core::BuiltinFn::kPack2X16Unorm;
            case GLSLstd450PackHalf2x16:
                return core::BuiltinFn::kPack2X16Float;
            case GLSLstd450Pow:
                return core::BuiltinFn::kPow;
            case GLSLstd450Radians:
                return core::BuiltinFn::kRadians;
            case GLSLstd450Round:
            case GLSLstd450RoundEven:
                return core::BuiltinFn::kRound;
            case GLSLstd450Sin:
                return core::BuiltinFn::kSin;
            case GLSLstd450Sinh:
                return core::BuiltinFn::kSinh;
            case GLSLstd450SmoothStep:
                return core::BuiltinFn::kSmoothstep;
            case GLSLstd450Sqrt:
                return core::BuiltinFn::kSqrt;
            case GLSLstd450Step:
                return core::BuiltinFn::kStep;
            case GLSLstd450Tan:
                return core::BuiltinFn::kTan;
            case GLSLstd450Tanh:
                return core::BuiltinFn::kTanh;
            case GLSLstd450Trunc:
                return core::BuiltinFn::kTrunc;
            case GLSLstd450UnpackSnorm4x8:
                return core::BuiltinFn::kUnpack4X8Snorm;
            case GLSLstd450UnpackUnorm4x8:
                return core::BuiltinFn::kUnpack4X8Unorm;
            case GLSLstd450UnpackSnorm2x16:
                return core::BuiltinFn::kUnpack2X16Snorm;
            case GLSLstd450UnpackUnorm2x16:
                return core::BuiltinFn::kUnpack2X16Unorm;
            case GLSLstd450UnpackHalf2x16:
                return core::BuiltinFn::kUnpack2X16Float;

            default:
                break;
        }
        return core::BuiltinFn::kNone;
    }

    spirv::BuiltinFn GetGlslStd450SpirvEquivalentFuncName(uint32_t ext_opcode) {
        switch (ext_opcode) {
            case GLSLstd450SAbs:
                return spirv::BuiltinFn::kAbs;
            case GLSLstd450SSign:
                return spirv::BuiltinFn::kSign;
            case GLSLstd450Normalize:
                return spirv::BuiltinFn::kNormalize;
            case GLSLstd450MatrixInverse:
                return spirv::BuiltinFn::kInverse;
            case GLSLstd450SMax:
                return spirv::BuiltinFn::kSMax;
            case GLSLstd450SMin:
                return spirv::BuiltinFn::kSMin;
            case GLSLstd450SClamp:
                return spirv::BuiltinFn::kSClamp;
            case GLSLstd450UMax:
                return spirv::BuiltinFn::kUMax;
            case GLSLstd450UMin:
                return spirv::BuiltinFn::kUMin;
            case GLSLstd450UClamp:
                return spirv::BuiltinFn::kUClamp;
            case GLSLstd450FindILsb:
                return spirv::BuiltinFn::kFindILsb;
            case GLSLstd450FindSMsb:
                return spirv::BuiltinFn::kFindSMsb;
            case GLSLstd450FindUMsb:
                return spirv::BuiltinFn::kFindUMsb;
            case GLSLstd450Refract:
                return spirv::BuiltinFn::kRefract;
            case GLSLstd450Reflect:
                return spirv::BuiltinFn::kReflect;
            case GLSLstd450FaceForward:
                return spirv::BuiltinFn::kFaceForward;
            case GLSLstd450Ldexp:
                return spirv::BuiltinFn::kLdexp;
            case GLSLstd450Modf:
                return spirv::BuiltinFn::kModf;
            case GLSLstd450Frexp:
                return spirv::BuiltinFn::kFrexp;
            default:
                break;
        }
        return spirv::BuiltinFn::kNone;
    }

    Vector<const core::type::Type*, 1> GlslStd450ExplicitParams(uint32_t ext_opcode,
                                                                const core::type::Type* result_ty) {
        if (ext_opcode == GLSLstd450SSign || ext_opcode == GLSLstd450SAbs ||
            ext_opcode == GLSLstd450SMax || ext_opcode == GLSLstd450SMin ||
            ext_opcode == GLSLstd450SClamp || ext_opcode == GLSLstd450UMax ||
            ext_opcode == GLSLstd450UMin || ext_opcode == GLSLstd450UClamp ||
            ext_opcode == GLSLstd450FindILsb || ext_opcode == GLSLstd450FindSMsb ||
            ext_opcode == GLSLstd450FindUMsb) {
            return {result_ty->DeepestElement()};
        }
        return {};
    }

    /// @param inst the SPIR-V instruction for OpAccessChain
    void EmitGlslStd450ExtInst(const spvtools::opt::Instruction& inst) {
        const auto ext_opcode = inst.GetSingleWordInOperand(1);
        auto* spv_ty = Type(inst.type_id());

        Vector<core::ir::Value*, 4> operands;
        // All parameters to GLSL.std.450 extended instructions are IDs.
        for (uint32_t idx = 2; idx < inst.NumInOperands(); ++idx) {
            operands.Push(Value(inst.GetSingleWordInOperand(idx)));
        }

        const auto wgsl_fn = GetGlslStd450WgslEquivalentFuncName(ext_opcode);
        if (wgsl_fn == core::BuiltinFn::kModf) {
            // For `ModfStruct`, which is, essentially, a WGSL `modf` instruction
            // we need some special handling. The result type that we produce
            // must be the SPIR-V type as we don't know how the result is used
            // later. So, we need to make the WGSL query and re-construct an
            // object of the right SPIR-V type. We can't, easily, do this later
            // as we lose the SPIR-V type as soon as we replace the result of the
            // `modf`. So, inline the work here to generate the correct results.

            auto* mem_ty = operands[0]->Type();
            auto* result_ty = core::type::CreateModfResult(ty_, ir_.symbols, mem_ty);

            auto* call = b_.Call(result_ty, wgsl_fn, operands);
            auto* fract = b_.Access(mem_ty, call, 0_u);
            auto* whole = b_.Access(mem_ty, call, 1_u);

            EmitWithoutSpvResult(call);
            EmitWithoutSpvResult(fract);
            EmitWithoutSpvResult(whole);
            Emit(b_.Construct(spv_ty, fract, whole), inst.result_id());
            return;
        }
        if (wgsl_fn == core::BuiltinFn::kFrexp) {
            // For `FrexpStruct`, which is, essentially, a WGSL `frexp`
            // instruction we need some special handling. The result type that we
            // produce must be the SPIR-V type as we don't know how the result is
            // used later. So, we need to make the WGSL query and re-construct an
            // object of the right SPIR-V type. We can't, easily, do this later
            // as we lose the SPIR-V type as soon as we replace the result of the
            // `frexp`. So, inline the work here to generate the correct results.

            auto* mem_ty = operands[0]->Type();
            auto* result_ty = core::type::CreateFrexpResult(ty_, ir_.symbols, mem_ty);

            auto* call = b_.Call(result_ty, wgsl_fn, operands);
            auto* fract = b_.Access(mem_ty, call, 0_u);
            auto* exp = b_.Access(ty_.MatchWidth(ty_.i32(), mem_ty), call, 1_u);
            auto* exp_res = exp->Result();

            EmitWithoutSpvResult(call);
            EmitWithoutSpvResult(fract);
            EmitWithoutSpvResult(exp);

            if (auto* str = spv_ty->As<core::type::Struct>()) {
                auto* exp_ty = str->Members()[1]->Type();
                if (exp_ty->DeepestElement()->IsUnsignedIntegerScalar()) {
                    auto* uexp = b_.Bitcast(exp_ty, exp);
                    exp_res = uexp->Result();
                    EmitWithoutSpvResult(uexp);
                }
            }

            Emit(b_.Construct(spv_ty, fract, exp_res), inst.result_id());
            return;
        }
        if (wgsl_fn != core::BuiltinFn::kNone) {
            Emit(b_.Call(spv_ty, wgsl_fn, operands), inst.result_id());
            return;
        }

        const auto spv_fn = GetGlslStd450SpirvEquivalentFuncName(ext_opcode);
        if (spv_fn != spirv::BuiltinFn::kNone) {
            auto explicit_params = GlslStd450ExplicitParams(ext_opcode, spv_ty);
            Emit(b_.CallExplicit<spirv::ir::BuiltinCall>(spv_ty, spv_fn, explicit_params, operands),
                 inst.result_id());
            return;
        }

        TINT_UNIMPLEMENTED() << "unhandled GLSL.std.450 instruction " << ext_opcode;
    }

    /// @param inst the SPIR-V instruction for OpAccessChain
    void EmitAccess(const spvtools::opt::Instruction& inst) {
        Vector indices = Args(inst, 3);
        auto* base = Value(inst.GetSingleWordOperand(2));

        if (indices.IsEmpty()) {
            // There are no indices, so just forward the base object.
            AddValue(inst.result_id(), base);
            return;
        }

        // Propagate the access mode of the base object.
        auto access_mode = core::Access::kUndefined;
        if (auto* ptr = base->Type()->As<core::type::Pointer>()) {
            access_mode = ptr->Access();
        }

        auto* access = b_.Access(Type(inst.type_id(), access_mode), base, std::move(indices));
        Emit(access, inst.result_id());
    }

    /// @param inst the SPIR-V instruction
    /// @param op the unary operator to use
    void EmitUnary(const spvtools::opt::Instruction& inst,
                   core::UnaryOp op,
                   uint32_t first_operand_idx = 2) {
        auto* val = Value(inst.GetSingleWordOperand(first_operand_idx));
        auto* unary = b_.Unary(op, Type(inst.type_id()), val);
        Emit(unary, inst.result_id());
    }

    /// @param inst the SPIR-V instruction
    /// @param op the binary operator to use
    void EmitBinary(const spvtools::opt::Instruction& inst,
                    core::BinaryOp op,
                    uint32_t first_operand_idx = 2) {
        auto* lhs = Value(inst.GetSingleWordOperand(first_operand_idx));
        auto* rhs = Value(inst.GetSingleWordOperand(first_operand_idx + 1));
        auto* binary = b_.Binary(op, Type(inst.type_id()), lhs, rhs);
        Emit(binary, inst.result_id());
    }

    /// Emits the logical negation of the result of the given SPIR-V instruction.
    /// @param inst the SPIR-V instruction
    /// @param op the binary operator to use
    void EmitInvertedBinary(const spvtools::opt::Instruction& inst, core::BinaryOp op) {
        auto* lhs = Value(inst.GetSingleWordOperand(2));
        auto* rhs = Value(inst.GetSingleWordOperand(3));
        auto* binary = b_.Binary(op, Type(inst.type_id()), lhs, rhs);
        EmitWithoutSpvResult(binary);

        auto* res = b_.Not(Type(inst.type_id()), binary);
        Emit(res, inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpCompositeExtract
    void EmitCompositeExtract(const spvtools::opt::Instruction& inst,
                              uint32_t composite_index = 2) {
        Vector<core::ir::Value*, 4> indices;
        for (uint32_t i = composite_index + 1; i < inst.NumOperandWords(); i++) {
            indices.Push(b_.Constant(u32(inst.GetSingleWordOperand(i))));
        }
        auto* object = Value(inst.GetSingleWordOperand(composite_index));
        auto* access = b_.Access(Type(inst.type_id()), object, std::move(indices));
        Emit(access, inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpCompositeInsert
    void EmitCompositeInsert(const spvtools::opt::Instruction& inst) {
        auto* object = Value(inst.GetSingleWordOperand(2));
        auto* composite = Value(inst.GetSingleWordOperand(3));
        Vector<core::ir::Value*, 4> indices;
        for (uint32_t i = 4; i < inst.NumOperandWords(); i++) {
            indices.Push(b_.Constant(u32(inst.GetSingleWordOperand(i))));
        }

        auto* tmp = b_.Var(ty_.ptr(function, Type(inst.type_id())));
        tmp->SetInitializer(composite);
        auto* ptr_ty = ty_.ptr(function, object->Type());
        auto* access = b_.Access(ptr_ty, tmp, std::move(indices));

        EmitWithoutSpvResult(tmp);
        EmitWithoutSpvResult(access);
        EmitWithoutResult(b_.Store(access, object));
        Emit(b_.Load(tmp), inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpCompositeConstruct
    void EmitConstruct(const spvtools::opt::Instruction& inst) {
        auto* construct = b_.Construct(Type(inst.type_id()), Args(inst, 2));
        Emit(construct, inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpVectorInsertDynamic
    void EmitVectorInsertDynamic(const spvtools::opt::Instruction& inst) {
        auto vector = Value(inst.GetSingleWordOperand(2));
        auto component = Value(inst.GetSingleWordOperand(3));
        auto index = Value(inst.GetSingleWordOperand(4));
        auto* tmp = b_.Var(
            ty_.ptr(core::AddressSpace::kFunction, Type(inst.type_id()), core::Access::kReadWrite));
        tmp->SetInitializer(vector);
        EmitWithoutSpvResult(tmp);
        EmitWithoutResult(b_.StoreVectorElement(tmp, index, component));
        Emit(b_.Load(tmp), inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpVectorShuffle
    void EmitVectorShuffle(const spvtools::opt::Instruction& inst) {
        auto* vector1 = Value(inst.GetSingleWordOperand(2));
        auto* vector2 = Value(inst.GetSingleWordOperand(3));
        auto* result_ty = Type(inst.type_id());
        auto* el_ty = result_ty->DeepestElement();

        uint32_t n1 = vector1->Type()->As<core::type::Vector>()->Width();
        uint32_t n2 = vector2->Type()->As<core::type::Vector>()->Width();

        Vector<core::ir::Value*, 4> swizzles;

        // Track the current swizzle that we are building from consecutive indices that fall within
        // the same vector.
        Vector<uint32_t, 4> current_indices;
        core::ir::Value* current_vector = nullptr;

        // Emit the current swizzle that we have constructed so far, and add it to the list.
        auto flush_swizzle = [&] {
            if (current_vector == nullptr) {
                return;
            }
            auto* swizzle_type = ty_.MatchWidth(el_ty, current_indices.Length());
            auto* swizzle = b_.Swizzle(swizzle_type, current_vector, current_indices);
            EmitWithoutSpvResult(swizzle);
            swizzles.Push(swizzle->Result());
            current_indices.Clear();
        };

        for (uint32_t i = 4; i < inst.NumOperandWords(); i++) {
            uint32_t literal = inst.GetSingleWordOperand(i);

            // Determine which vector this index falls within.
            uint32_t next_index;
            core::ir::Value* next_vector = nullptr;
            if (literal == 0xFFFFFFFF) {
                // Undefined component, so just use the first component of the first vector.
                next_vector = vector1;
                next_index = 0;
            } else if (literal < n1) {
                next_vector = vector1;
                next_index = literal;
            } else if (literal < n1 + n2) {
                next_vector = vector2;
                next_index = literal - n1;
            } else {
                TINT_ICE() << "invalid vector shuffle index";
            }

            // If the vector has changed from the previous index, flush the swizzle.
            if (next_vector != current_vector) {
                flush_swizzle();
            }
            current_vector = next_vector;
            current_indices.Push(next_index);
        }

        flush_swizzle();

        if (swizzles.Length() == 1) {
            // There was only one swizzle, so we can just use it directly.
            AddValue(inst.result_id(), swizzles[0]);
        } else {
            // There were multiple swizzles, so we combine them all to produce the final result.
            Emit(b_.Construct(result_ty, swizzles), inst.result_id());
        }
    }

    /// @param inst the SPIR-V instruction for OpFunctionCall
    void EmitFunctionCall(const spvtools::opt::Instruction& inst) {
        Emit(b_.Call(Function(inst.GetSingleWordInOperand(0)), Args(inst, 3)), inst.result_id());
    }

    /// @param inst the SPIR-V instruction for OpVariable
    void EmitVar(const spvtools::opt::Instruction& inst) {
        // Handle decorations.
        std::optional<uint32_t> group;
        std::optional<uint32_t> binding;
        core::Access access_mode = core::Access::kUndefined;
        core::IOAttributes io_attributes;
        auto interpolation = [&]() -> core::Interpolation& {
            // Create the interpolation field with the default values on first call.
            if (!io_attributes.interpolation.has_value()) {
                io_attributes.interpolation = core::Interpolation{
                    .type = core::InterpolationType::kPerspective,
                    .sampling = core::InterpolationSampling::kUndefined,
                };
            }
            return io_attributes.interpolation.value();
        };
        for (auto* deco :
             spirv_context_->get_decoration_mgr()->GetDecorationsFor(inst.result_id(), false)) {
            auto d = deco->GetSingleWordOperand(1);
            switch (spv::Decoration(d)) {
                case spv::Decoration::RelaxedPrecision:  // WGSL is relaxed precision by default
                    break;
                case spv::Decoration::NonReadable:
                    access_mode = core::Access::kWrite;
                    break;
                case spv::Decoration::NonWritable:
                    access_mode = core::Access::kRead;
                    break;
                case spv::Decoration::DescriptorSet:
                    group = deco->GetSingleWordOperand(2);
                    break;
                case spv::Decoration::Binding:
                    binding = deco->GetSingleWordOperand(2);
                    break;
                case spv::Decoration::BuiltIn:
                    io_attributes.builtin = Builtin(spv::BuiltIn(deco->GetSingleWordOperand(2)));
                    break;
                case spv::Decoration::Invariant:
                    io_attributes.invariant = true;
                    break;
                case spv::Decoration::Location:
                    io_attributes.location = deco->GetSingleWordOperand(2);
                    break;
                case spv::Decoration::NoPerspective:
                    interpolation().type = core::InterpolationType::kLinear;
                    break;
                case spv::Decoration::Flat:
                    interpolation().type = core::InterpolationType::kFlat;
                    break;
                case spv::Decoration::Centroid:
                    interpolation().sampling = core::InterpolationSampling::kCentroid;
                    break;
                case spv::Decoration::Sample:
                    interpolation().sampling = core::InterpolationSampling::kSample;
                    break;
                case spv::Decoration::Index:
                    io_attributes.blend_src = deco->GetSingleWordOperand(2);
                    break;
                case spv::Decoration::Coherent:
                    // Tint has coherent memory semantics, so this is a no-op.
                    break;
                default:
                    TINT_UNIMPLEMENTED() << "unhandled decoration " << d;
            }
        }

        if (io_attributes.interpolation.has_value()) {
            // WGSL requires that '@interpolate(flat)' needs to be paired with '@location', however
            // SPIR-V requires all fragment shader integer Inputs are 'flat'. If the decorations do
            // not contain a spv::Decoration::Location, then remove the interpolation decoration.
            //
            // The `perspective,center` interpolation is the default value if one isn't provided.
            // Just strip it off. This keeps us from accidentally applying interpolation where it
            // isn't permitted, and it isn't necessary.
            if ((io_attributes.interpolation->type == core::InterpolationType::kFlat &&
                 !io_attributes.location.has_value()) ||
                (io_attributes.interpolation->type == core::InterpolationType::kPerspective &&
                 io_attributes.interpolation->sampling == core::InterpolationSampling::kCenter)) {
                io_attributes.interpolation = std::nullopt;
            }
        }

        auto* element_ty = Type(inst.type_id(), access_mode)->As<core::type::Pointer>();
        if (element_ty->AddressSpace() == core::AddressSpace::kOut &&
            element_ty->StoreType()->IsIntegerScalarOrVector()) {
            io_attributes.interpolation = {
                .type = core::InterpolationType::kFlat,
                .sampling = core::InterpolationSampling::kUndefined,
            };
        }

        auto* var = b_.Var(element_ty);
        if (inst.NumOperands() > 3) {
            var->SetInitializer(Value(inst.GetSingleWordOperand(3)));
        }

        if (group || binding) {
            TINT_ASSERT(group && binding);

            // Remap any samplers which match an entry in the sampler mappings
            // table.
            if (element_ty->StoreType()->Is<core::type::Sampler>()) {
                auto it =
                    options_.sampler_mappings.find(BindingPoint{group.value(), binding.value()});
                if (it != options_.sampler_mappings.end()) {
                    auto bp = it->second;
                    group = bp.group;
                    binding = bp.binding;
                }
            }

            auto& grp = max_binding.GetOrAddZero(group.value());
            grp = std::max(grp, binding.value());

            auto& used = used_bindings.GetOrAddZero(BindingPoint{group.value(), binding.value()});
            used += 1;

            io_attributes.binding_point = {group.value(), binding.value()};
        }
        var->SetAttributes(std::move(io_attributes));
        var_to_original_access_mode_.insert({var, access_mode});

        Emit(var, inst.result_id());
    }

  private:
    /// TypeKey describes a SPIR-V type with an access mode.
    struct TypeKey {
        /// The SPIR-V type object.
        const spvtools::opt::analysis::Type* type;
        /// The access mode.
        core::Access access_mode;

        // Equality operator for TypeKey.
        bool operator==(const TypeKey& other) const {
            return type == other.type && access_mode == other.access_mode;
        }

        /// @returns the hash code of the TypeKey
        tint::HashCode HashCode() const { return Hash(type, access_mode); }
    };

    /// The parser options
    const Options& options_;

    /// The generated IR module.
    core::ir::Module ir_;
    /// The Tint IR builder.
    core::ir::Builder b_{ir_};
    /// The Tint type manager.
    core::type::Manager& ty_{ir_.Types()};

    /// The Tint IR function that is currently being emitted.
    core::ir::Function* current_function_ = nullptr;
    /// The Tint IR block that is currently being emitted.
    core::ir::Block* current_block_ = nullptr;
    /// A map from a SPIR-V type declaration to the corresponding Tint type object.
    Hashmap<TypeKey, const core::type::Type*, 16> types_;
    /// A map from a SPIR-V function definition result ID to the corresponding Tint function object.
    Hashmap<uint32_t, core::ir::Function*, 8> functions_;
    /// A map from a SPIR-V result ID to the corresponding Tint value object.
    Hashmap<uint32_t, core::ir::Value*, 8> values_;
    /// Maps a `group` number to the largest seen `binding` value for that group
    Hashmap<uint32_t, uint32_t, 4> max_binding;
    /// A map of binding point to the count of usages
    Hashmap<BindingPoint, uint32_t, 4> used_bindings;

    /// The SPIR-V context containing the SPIR-V tools intermediate representation.
    std::unique_ptr<spvtools::opt::IRContext> spirv_context_;
    /// The current SPIR-V function being emitted
    spvtools::opt::Function* current_spirv_function_ = nullptr;

    // The set of IDs that are imports of the GLSL.std.450 extended instruction sets.
    std::unordered_set<uint32_t> glsl_std_450_imports_;
    // The set of IDs of imports that are ignored. For example, any "NonSemanticInfo." import is
    // ignored.
    std::unordered_set<uint32_t> ignored_imports_;

    // Map of SPIR-V IDs to string names
    std::unordered_map<uint32_t, std::string> id_to_name_;
    // Map of SPIR-V Struct IDs to a list of member string names
    std::unordered_map<uint32_t, std::vector<std::string>> struct_to_member_names_;

    // Set of types which should be considered `NonWritable` even if no decoration is present
    std::unordered_set<const core::type::Type*> consider_non_writable_;

    // Set of SPIR-V block ids where we'll stop a `Branch` instruction walk. These could be merge
    // blocks, premerge blocks, continuing blocks, etc.
    std::unordered_map<uint32_t, core::ir::ControlInstruction*> walk_stop_blocks_;
    // Map of continue target ID to the controlling IR loop.
    std::unordered_map<uint32_t, core::ir::Loop*> continue_targets_;
    // Map of header target ID to the controlling IR loop.
    std::unordered_map<uint32_t, core::ir::Loop*> loop_headers_;

    struct PremergeInfo {
        core::ir::If* parent = nullptr;
        core::ir::Var* condition = nullptr;
    };
    // Map of merge ID to an associated premerge_id, if any
    std::unordered_map<uint32_t, PremergeInfo> merge_to_premerge_;

    std::unordered_set<core::ir::Block*> current_blocks_;

    // For each block, we keep a set of SPIR-V `id`s which are known in that scope.
    std::vector<std::unordered_set<uint32_t>> id_stack_;

    // If we're in a switch, is populated with the IDs of the blocks for each of the switch
    // selectors. This lets us watch for fallthrough when emitting branch instructions.
    std::vector<std::unordered_set<uint32_t>> current_switch_blocks_;

    /// Maps from a spirv-v block id to the corresponding block in the IR
    std::unordered_map<uint32_t, core::ir::Block*> spirv_id_to_block_;

    // Map of continue block id to the phi types which need to be returned by
    // the continue target
    std::unordered_map<uint32_t, std::vector<uint32_t>> continue_blk_phis_;

    // A stack of values which need to be replaced as we finish processing a
    // block. Used to store `phi` information so we can retrieve values which
    // are defined after the `OpPhi` instruction.
    std::vector<std::vector<ReplacementValue>> values_to_replace_;

    // A map of block to phi values returned by the loop header
    std::unordered_map<uint32_t, std::vector<uint32_t>> block_phi_values_;

    // Map of certain instructions back to their originating spirv block
    std::unordered_map<core::ir::Instruction*, uint32_t> inst_to_spirv_block_;

    // Map of merge block id's back to the block which contains the header
    std::unordered_map<uint32_t, uint32_t> spirv_merge_id_to_header_id_;

    // Structure hold spec composite information
    struct SpecComposite {
        // The composite type
        const core::type::Type* type;
        // The composite arguments
        Vector<uint32_t, 4> args;
    };

    // The set of SPIR-V IDs which map to `OpSpecConstantComposite` information
    std::unordered_map<uint32_t, SpecComposite> spec_composites_;

    // Structures which are marked with `BufferBlock` and need to be reported as `Storage` address
    // space
    std::unordered_set<const core::type::Struct*> storage_buffer_types_;

    // Map of `var` to the access mode it was originally created with. This may be different from
    // the current mode if we needed to set a default mode.
    std::unordered_map<core::ir::Var*, core::Access> var_to_original_access_mode_;

    // Map of spir-v branch conditional instructions to the related IR if instruction.
    std::unordered_map<const spvtools::opt::Instruction*, core::ir::If*> branch_conditional_to_if_;
};

}  // namespace

Result<core::ir::Module> Parse(Slice<const uint32_t> spirv, const Options& options) {
    return Parser(options).Run(spirv);
}

}  // namespace tint::spirv::reader
