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

#include "src/tint/lang/core/ir/binary/encode.h"

#include <sstream>
#include <string>
#include <utility>

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/builtin_type.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/core_unary.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function_param.h"
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
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
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
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/rtti/switch.h"

TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS();
#include "src/tint/utils/protos/ir/ir.pb.h"
TINT_END_DISABLE_PROTOBUF_WARNINGS();

namespace tint::core::ir::binary {
namespace {
struct Encoder {
    const Module& mod_in_;
    pb::Module& mod_out_;

    Hashmap<const core::ir::Function*, uint32_t, 32> functions_{};
    Hashmap<const core::ir::Block*, uint32_t, 32> blocks_{};
    Hashmap<const core::type::Type*, uint32_t, 32> types_{};
    Hashmap<const core::ir::Value*, uint32_t, 32> values_{};
    Hashmap<const core::constant::Value*, uint32_t, 32> constant_values_{};

    std::stringstream err_{};

    Result<SuccessType> Encode() {
        // Encode all user-declared structures first. This is to ensure that the IR disassembly
        // (which prints structure types first) does not reorder after encoding and decoding.
        for (auto* ty : mod_in_.Types()) {
            if (auto* str = ty->As<core::type::Struct>()) {
                Type(str);
            }
        }
        Vector<pb::Function*, 8> fns_out;
        for (auto& fn_in : mod_in_.functions) {
            uint32_t id = static_cast<uint32_t>(mod_out_.functions().size());
            fns_out.Push(mod_out_.add_functions());
            functions_.Add(fn_in, id);
        }
        for (size_t i = 0, n = mod_in_.functions.Length(); i < n; i++) {
            PopulateFunction(fns_out[i], mod_in_.functions[i]);
        }
        mod_out_.set_root_block(Block(mod_in_.root_block));

        auto err = err_.str();
        if (!err.empty()) {
            return Failure{err};
        }
        return Success;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////
    void PopulateFunction(pb::Function* fn_out, const ir::Function* fn_in) {
        if (auto name = mod_in_.NameOf(fn_in)) {
            fn_out->set_name(name.Name());
        }
        fn_out->set_return_type(Type(fn_in->ReturnType()));
        if (fn_in->Stage() != Function::PipelineStage::kUndefined) {
            fn_out->set_pipeline_stage(PipelineStage(fn_in->Stage()));
        }
        if (auto wg_size_in = fn_in->WorkgroupSize()) {
            auto& wg_size_out = *fn_out->mutable_workgroup_size();
            wg_size_out.set_x(Value((*wg_size_in)[0]));
            wg_size_out.set_y(Value((*wg_size_in)[1]));
            wg_size_out.set_z(Value((*wg_size_in)[2]));
        }
        for (auto* param_in : fn_in->Params()) {
            fn_out->add_parameters(Value(param_in));
        }
        if (auto ret_loc_in = fn_in->ReturnLocation()) {
            fn_out->set_return_location(*ret_loc_in);
        }
        if (auto ret_interp_in = fn_in->ReturnInterpolation()) {
            auto& ret_interp_out = *fn_out->mutable_return_interpolation();
            Interpolation(ret_interp_out, *ret_interp_in);
        }
        if (auto builtin_in = fn_in->ReturnBuiltin()) {
            fn_out->set_return_builtin(BuiltinValue(*builtin_in));
        }
        if (fn_in->ReturnInvariant()) {
            fn_out->set_return_invariant(true);
        }
        fn_out->set_block(Block(fn_in->Block()));
    }

    uint32_t Function(const ir::Function* fn_in) { return *functions_.Get(fn_in); }

    pb::PipelineStage PipelineStage(Function::PipelineStage stage) {
        switch (stage) {
            case Function::PipelineStage::kCompute:
                return pb::PipelineStage::Compute;
            case Function::PipelineStage::kFragment:
                return pb::PipelineStage::Fragment;
            case Function::PipelineStage::kVertex:
                return pb::PipelineStage::Vertex;
            case Function::PipelineStage::kUndefined:
                break;
        }
        TINT_ICE() << "unhandled PipelineStage: " << stage;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Blocks
    ////////////////////////////////////////////////////////////////////////////
    uint32_t Block(const ir::Block* block_in) {
        TINT_ASSERT(block_in != nullptr);

        return blocks_.GetOrAdd(block_in, [&]() -> uint32_t {
            auto id = static_cast<uint32_t>(mod_out_.blocks().size());
            auto& block_out = *mod_out_.add_blocks();
            for (auto* inst : *block_in) {
                Instruction(*block_out.add_instructions(), inst);
            }
            if (auto* mib = block_in->As<ir::MultiInBlock>()) {
                block_out.set_is_multi_in(true);
                for (auto* param : mib->Params()) {
                    block_out.add_parameters(Value(param));
                }
            }
            return id;
        });
    }

    ////////////////////////////////////////////////////////////////////////////
    // Instructions
    ////////////////////////////////////////////////////////////////////////////
    void Instruction(pb::Instruction& inst_out, const ir::Instruction* inst_in) {
        tint::Switch(
            inst_in,  //
            [&](const ir::Access* i) { InstructionAccess(*inst_out.mutable_access(), i); },
            [&](const ir::Bitcast* i) { InstructionBitcast(*inst_out.mutable_bitcast(), i); },
            [&](const ir::BreakIf* i) { InstructionBreakIf(*inst_out.mutable_break_if(), i); },
            [&](const ir::CoreBinary* i) { InstructionBinary(*inst_out.mutable_binary(), i); },
            [&](const ir::CoreBuiltinCall* i) {
                InstructionBuiltinCall(*inst_out.mutable_builtin_call(), i);
            },
            [&](const ir::CoreUnary* i) { InstructionUnary(*inst_out.mutable_unary(), i); },
            [&](const ir::Construct* i) { InstructionConstruct(*inst_out.mutable_construct(), i); },
            [&](const ir::Continue* i) { InstructionContinue(*inst_out.mutable_continue_(), i); },
            [&](const ir::Convert* i) { InstructionConvert(*inst_out.mutable_convert(), i); },
            [&](const ir::Discard* i) { InstructionDiscard(*inst_out.mutable_discard(), i); },
            [&](const ir::ExitIf* i) { InstructionExitIf(*inst_out.mutable_exit_if(), i); },
            [&](const ir::ExitLoop* i) { InstructionExitLoop(*inst_out.mutable_exit_loop(), i); },
            [&](const ir::ExitSwitch* i) {
                InstructionExitSwitch(*inst_out.mutable_exit_switch(), i);
            },
            [&](const ir::If* i) { InstructionIf(*inst_out.mutable_if_(), i); },
            [&](const ir::Let* i) { InstructionLet(*inst_out.mutable_let(), i); },
            [&](const ir::Load* i) { InstructionLoad(*inst_out.mutable_load(), i); },
            [&](const ir::LoadVectorElement* i) {
                InstructionLoadVectorElement(*inst_out.mutable_load_vector_element(), i);
            },
            [&](const ir::Loop* i) { InstructionLoop(*inst_out.mutable_loop(), i); },
            [&](const ir::NextIteration* i) {
                InstructionNextIteration(*inst_out.mutable_next_iteration(), i);
            },
            [&](const ir::Return* i) { InstructionReturn(*inst_out.mutable_return_(), i); },
            [&](const ir::Store* i) { InstructionStore(*inst_out.mutable_store(), i); },
            [&](const ir::StoreVectorElement* i) {
                InstructionStoreVectorElement(*inst_out.mutable_store_vector_element(), i);
            },
            [&](const ir::Switch* i) { InstructionSwitch(*inst_out.mutable_switch_(), i); },
            [&](const ir::Swizzle* i) { InstructionSwizzle(*inst_out.mutable_swizzle(), i); },
            [&](const ir::UserCall* i) { InstructionUserCall(*inst_out.mutable_user_call(), i); },
            [&](const ir::Var* i) { InstructionVar(*inst_out.mutable_var(), i); },
            [&](const ir::Unreachable* i) {
                InstructionUnreachable(*inst_out.mutable_unreachable(), i);
            },
            TINT_ICE_ON_NO_MATCH);
        for (auto* operand : inst_in->Operands()) {
            inst_out.add_operands(Value(operand));
        }
        for (auto* result : inst_in->Results()) {
            inst_out.add_results(Value(result));
        }
    }

    void InstructionAccess(pb::InstructionAccess&, const ir::Access*) {}

    void InstructionBinary(pb::InstructionBinary& binary_out, const ir::CoreBinary* binary_in) {
        binary_out.set_op(BinaryOp(binary_in->Op()));
    }

    void InstructionBitcast(pb::InstructionBitcast&, const ir::Bitcast*) {}

    void InstructionBreakIf(pb::InstructionBreakIf& breakif_out, const ir::BreakIf* breakif_in) {
        auto num_next_iter_values = static_cast<uint32_t>(breakif_in->NextIterValues().Length());
        breakif_out.set_num_next_iter_values(num_next_iter_values);
    }

    void InstructionBuiltinCall(pb::InstructionBuiltinCall& call_out,
                                const ir::CoreBuiltinCall* call_in) {
        call_out.set_builtin(BuiltinFn(call_in->Func()));
        for (auto* param : call_in->ExplicitTemplateParams()) {
            call_out.add_explicit_template_params(Type(param));
        }
    }

    void InstructionConstruct(pb::InstructionConstruct&, const ir::Construct*) {}

    void InstructionContinue(pb::InstructionContinue&, const ir::Continue*) {}

    void InstructionConvert(pb::InstructionConvert&, const ir::Convert*) {}

    void InstructionIf(pb::InstructionIf& if_out, const ir::If* if_in) {
        if (auto* block = if_in->True()) {
            if_out.set_true_(Block(block));
        }
        if (auto* block = if_in->False()) {
            if_out.set_false_(Block(block));
        }
    }

    void InstructionDiscard(pb::InstructionDiscard&, const ir::Discard*) {}

    void InstructionExitIf(pb::InstructionExitIf&, const ir::ExitIf*) {}

    void InstructionExitLoop(pb::InstructionExitLoop&, const ir::ExitLoop*) {}

    void InstructionExitSwitch(pb::InstructionExitSwitch&, const ir::ExitSwitch*) {}

    void InstructionLet(pb::InstructionLet&, const ir::Let*) {}

    void InstructionLoad(pb::InstructionLoad&, const ir::Load*) {}

    void InstructionLoadVectorElement(pb::InstructionLoadVectorElement&,
                                      const ir::LoadVectorElement*) {}

    void InstructionLoop(pb::InstructionLoop& loop_out, const ir::Loop* loop_in) {
        if (loop_in->HasInitializer()) {
            loop_out.set_initializer(Block(loop_in->Initializer()));
        }
        loop_out.set_body(Block(loop_in->Body()));
        if (loop_in->HasContinuing()) {
            loop_out.set_continuing(Block(loop_in->Continuing()));
        }
    }

    void InstructionNextIteration(pb::InstructionNextIteration&, const ir::NextIteration*) {}

    void InstructionReturn(pb::InstructionReturn&, const ir::Return*) {}

    void InstructionStore(pb::InstructionStore&, const ir::Store*) {}

    void InstructionStoreVectorElement(pb::InstructionStoreVectorElement&,
                                       const ir::StoreVectorElement*) {}

    void InstructionSwizzle(pb::InstructionSwizzle& swizzle_out, const ir::Swizzle* swizzle_in) {
        for (auto idx : swizzle_in->Indices()) {
            swizzle_out.add_indices(idx);
        }
    }

    void InstructionSwitch(pb::InstructionSwitch& switch_out, const ir::Switch* switch_in) {
        for (auto& case_in : switch_in->Cases()) {
            auto& case_out = *switch_out.add_cases();
            case_out.set_block(Block(case_in.block));
            for (auto& selector_in : case_in.selectors) {
                if (selector_in.IsDefault()) {
                    case_out.set_is_default(true);
                } else {
                    case_out.add_selectors(ConstantValue(selector_in.val->Value()));
                }
            }
        }
    }

    void InstructionUnary(pb::InstructionUnary& unary_out, const ir::CoreUnary* unary_in) {
        unary_out.set_op(UnaryOp(unary_in->Op()));
    }

    void InstructionUserCall(pb::InstructionUserCall&, const ir::UserCall*) {}

    void InstructionVar(pb::InstructionVar& var_out, const ir::Var* var_in) {
        if (auto bp_in = var_in->BindingPoint()) {
            auto& bp_out = *var_out.mutable_binding_point();
            BindingPoint(bp_out, *bp_in);
        }
        if (auto iidx_in = var_in->InputAttachmentIndex()) {
            var_out.set_input_attachment_index(iidx_in.value());
        }
    }

    void InstructionUnreachable(pb::InstructionUnreachable&, const ir::Unreachable*) {}

    ////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////
    uint32_t Type(const core::type::Type* type_in) {
        TINT_ASSERT(type_in != nullptr);
        return types_.GetOrAdd(type_in, [&]() -> uint32_t {
            pb::Type type_out;
            tint::Switch(
                type_in,  //
                [&](const core::type::Void*) { type_out.set_basic(pb::TypeBasic::void_); },
                [&](const core::type::Bool*) { type_out.set_basic(pb::TypeBasic::bool_); },
                [&](const core::type::I32*) { type_out.set_basic(pb::TypeBasic::i32); },
                [&](const core::type::U32*) { type_out.set_basic(pb::TypeBasic::u32); },
                [&](const core::type::F32*) { type_out.set_basic(pb::TypeBasic::f32); },
                [&](const core::type::F16*) { type_out.set_basic(pb::TypeBasic::f16); },
                [&](const core::type::Vector* v) { TypeVector(*type_out.mutable_vector(), v); },
                [&](const core::type::Matrix* m) { TypeMatrix(*type_out.mutable_matrix(), m); },
                [&](const core::type::Pointer* m) { TypePointer(*type_out.mutable_pointer(), m); },
                [&](const core::type::Struct* s) { TypeStruct(*type_out.mutable_struct_(), s); },
                [&](const core::type::Atomic* a) { TypeAtomic(*type_out.mutable_atomic(), a); },
                [&](const core::type::Array* m) { TypeArray(*type_out.mutable_array(), m); },
                [&](const core::type::DepthTexture* t) {
                    TypeDepthTexture(*type_out.mutable_depth_texture(), t);
                },
                [&](const core::type::SampledTexture* t) {
                    TypeSampledTexture(*type_out.mutable_sampled_texture(), t);
                },
                [&](const core::type::MultisampledTexture* t) {
                    TypeMultisampledTexture(*type_out.mutable_multisampled_texture(), t);
                },
                [&](const core::type::DepthMultisampledTexture* t) {
                    TypeDepthMultisampledTexture(*type_out.mutable_depth_multisampled_texture(), t);
                },
                [&](const core::type::StorageTexture* t) {
                    TypeStorageTexture(*type_out.mutable_storage_texture(), t);
                },
                [&](const core::type::ExternalTexture* t) {
                    TypeExternalTexture(*type_out.mutable_external_texture(), t);
                },
                [&](const core::type::Sampler* s) { TypeSampler(*type_out.mutable_sampler(), s); },
                [&](const core::type::InputAttachment* i) {
                    TypeInputAttachment(*type_out.mutable_input_attachment(), i);
                },
                [&]([[maybe_unused]] const core::type::SubgroupMatrix* s) {
                    switch (s->Kind()) {
                        case core::SubgroupMatrixKind::kLeft:
                            TypeSubgroupMatrix(*type_out.mutable_subgroup_matrix_left(), s);
                            break;
                        case core::SubgroupMatrixKind::kRight:
                            TypeSubgroupMatrix(*type_out.mutable_subgroup_matrix_right(), s);
                            break;
                        case core::SubgroupMatrixKind::kResult:
                            TypeSubgroupMatrix(*type_out.mutable_subgroup_matrix_result(), s);
                            break;
                        default:
                            TINT_ICE() << "invalid subgroup matrix kind: " << ToString(s->Kind());
                    }
                },
                TINT_ICE_ON_NO_MATCH);

            mod_out_.mutable_types()->Add(std::move(type_out));
            return static_cast<uint32_t>(mod_out_.types().size() - 1);
        });
    }

    void TypeVector(pb::TypeVector& vector_out, const core::type::Vector* vector_in) {
        vector_out.set_width(vector_in->Width());
        vector_out.set_element_type(Type(vector_in->Type()));
    }

    void TypeMatrix(pb::TypeMatrix& matrix_out, const core::type::Matrix* matrix_in) {
        matrix_out.set_num_columns(matrix_in->Columns());
        matrix_out.set_num_rows(matrix_in->Rows());
        matrix_out.set_element_type(Type(matrix_in->Type()));
    }

    void TypePointer(pb::TypePointer& pointer_out, const core::type::Pointer* pointer_in) {
        pointer_out.set_address_space(AddressSpace(pointer_in->AddressSpace()));
        pointer_out.set_store_type(Type(pointer_in->StoreType()));
        pointer_out.set_access(AccessControl(pointer_in->Access()));
    }

    void TypeStruct(pb::TypeStruct& struct_out, const core::type::Struct* struct_in) {
        struct_out.set_name(struct_in->Name().Name());
        for (auto* member_in : struct_in->Members()) {
            auto& member_out = *struct_out.add_member();
            member_out.set_name(member_in->Name().Name());
            member_out.set_type(Type(member_in->Type()));
            member_out.set_size(member_in->Size());
            member_out.set_align(member_in->Align());

            auto& attrs_in = member_in->Attributes();
            if (attrs_in.location) {
                member_out.mutable_attributes()->set_location(*attrs_in.location);
            }
            if (attrs_in.blend_src) {
                member_out.mutable_attributes()->set_blend_src(*attrs_in.blend_src);
            }
            if (attrs_in.color) {
                member_out.mutable_attributes()->set_color(*attrs_in.color);
            }
            if (attrs_in.builtin) {
                member_out.mutable_attributes()->set_builtin(BuiltinValue(*attrs_in.builtin));
            }
            if (auto& interpolation_in = attrs_in.interpolation) {
                auto& interpolation_out = *member_out.mutable_attributes()->mutable_interpolation();
                Interpolation(interpolation_out, *interpolation_in);
            }
            if (attrs_in.invariant) {
                member_out.mutable_attributes()->set_invariant(true);
            }
        }
    }

    void TypeAtomic(pb::TypeAtomic& atomic_out, const core::type::Atomic* atomic_in) {
        atomic_out.set_type(Type(atomic_in->Type()));
    }

    void TypeArray(pb::TypeArray& array_out, const core::type::Array* array_in) {
        array_out.set_element(Type(array_in->ElemType()));
        array_out.set_stride(array_in->Stride());
        tint::Switch(
            array_in->Count(),  //
            [&](const core::type::ConstantArrayCount* c) {
                array_out.set_count(c->value);
                if (c->value >= internal_limits::kMaxArrayElementCount) {
                    err_ << "array count (" << c->value << ") must be less than "
                         << internal_limits::kMaxArrayElementCount << "\n";
                }
            },
            [&](const core::type::RuntimeArrayCount*) { array_out.set_count(0); },
            TINT_ICE_ON_NO_MATCH);
    }

    void TypeDepthTexture(pb::TypeDepthTexture& texture_out,
                          const core::type::DepthTexture* texture_in) {
        texture_out.set_dimension(TextureDimension(texture_in->Dim()));
    }

    void TypeSampledTexture(pb::TypeSampledTexture& texture_out,
                            const core::type::SampledTexture* texture_in) {
        texture_out.set_dimension(TextureDimension(texture_in->Dim()));
        texture_out.set_sub_type(Type(texture_in->Type()));
    }

    void TypeMultisampledTexture(pb::TypeMultisampledTexture& texture_out,
                                 const core::type::MultisampledTexture* texture_in) {
        texture_out.set_dimension(TextureDimension(texture_in->Dim()));
        texture_out.set_sub_type(Type(texture_in->Type()));
    }

    void TypeDepthMultisampledTexture(pb::TypeDepthMultisampledTexture& texture_out,
                                      const core::type::DepthMultisampledTexture* texture_in) {
        texture_out.set_dimension(TextureDimension(texture_in->Dim()));
    }

    void TypeStorageTexture(pb::TypeStorageTexture& texture_out,
                            const core::type::StorageTexture* texture_in) {
        texture_out.set_dimension(TextureDimension(texture_in->Dim()));
        texture_out.set_texel_format(TexelFormat(texture_in->TexelFormat()));
        texture_out.set_access(AccessControl(texture_in->Access()));
    }

    void TypeExternalTexture(pb::TypeExternalTexture&, const core::type::ExternalTexture*) {}
    void TypeInputAttachment(pb::TypeInputAttachment& input_attachment_out,
                             const core::type::InputAttachment* input_attachment_in) {
        input_attachment_out.set_sub_type(Type(input_attachment_in->Type()));
    }

    void TypeSampler(pb::TypeSampler& sampler_out, const core::type::Sampler* sampler_in) {
        sampler_out.set_kind(SamplerKind(sampler_in->Kind()));
    }

    void TypeSubgroupMatrix(pb::TypeSubgroupMatrix& subgroup_matrix_out,
                            const core::type::SubgroupMatrix* subgroup_matrix_in) {
        subgroup_matrix_out.set_sub_type(Type(subgroup_matrix_in->Type()));
        subgroup_matrix_out.set_columns(subgroup_matrix_in->Columns());
        subgroup_matrix_out.set_rows(subgroup_matrix_in->Rows());
    }

    [[maybe_unused]] void TypeBuitinStruct(pb::Type& builtin_struct_out,
                                           const core::type::Struct* builtin_struct_in) {
        auto name = builtin_struct_in->Name().NameView();
        auto builtin = ParseBuiltinType(name);
        switch (builtin) {
            case BuiltinType::kAtomicCompareExchangeResultI32:
                builtin_struct_out.set_builtin_struct(
                    pb::TypeBuiltinStruct::AtomicCompareExchangeResultI32);
                break;
            case BuiltinType::kAtomicCompareExchangeResultU32:
                builtin_struct_out.set_builtin_struct(
                    pb::TypeBuiltinStruct::AtomicCompareExchangeResultU32);
                break;
            case BuiltinType::kFrexpResultF16:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::FrexpResultF16);
                break;
            case BuiltinType::kFrexpResultF32:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::FrexpResultF32);
                break;
            case BuiltinType::kFrexpResultVec2F16:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::FrexpResultVec2F16);
                break;
            case BuiltinType::kFrexpResultVec2F32:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::FrexpResultVec2F32);
                break;
            case BuiltinType::kFrexpResultVec3F16:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::FrexpResultVec3F16);
                break;
            case BuiltinType::kFrexpResultVec3F32:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::FrexpResultVec3F32);
                break;
            case BuiltinType::kFrexpResultVec4F16:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::FrexpResultVec4F16);
                break;
            case BuiltinType::kFrexpResultVec4F32:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::FrexpResultVec4F32);
                break;
            case BuiltinType::kModfResultF16:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::ModfResultF16);
                break;
            case BuiltinType::kModfResultF32:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::ModfResultF32);
                break;
            case BuiltinType::kModfResultVec2F16:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::ModfResultVec2F16);
                break;
            case BuiltinType::kModfResultVec2F32:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::ModfResultVec2F32);
                break;
            case BuiltinType::kModfResultVec3F16:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::ModfResultVec3F16);
                break;
            case BuiltinType::kModfResultVec3F32:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::ModfResultVec3F32);
                break;
            case BuiltinType::kModfResultVec4F16:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::ModfResultVec4F16);
                break;
            case BuiltinType::kModfResultVec4F32:
                builtin_struct_out.set_builtin_struct(pb::TypeBuiltinStruct::ModfResultVec4F32);
                break;
            default:
                TINT_ICE() << "unhandled builtin struct " << name;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Values
    ////////////////////////////////////////////////////////////////////////////
    uint32_t Value(const ir::Value* value_in) {
        if (!value_in) {
            return 0;
        }
        return values_.GetOrAdd(value_in, [&] {
            auto& value_out = *mod_out_.add_values();
            auto id = static_cast<uint32_t>(mod_out_.values().size());

            tint::Switch(
                value_in,
                [&](const ir::InstructionResult* v) {
                    InstructionResult(*value_out.mutable_instruction_result(), v);
                },
                [&](const ir::FunctionParam* v) {
                    FunctionParameter(*value_out.mutable_function_parameter(), v);
                },
                [&](const ir::BlockParam* v) {
                    BlockParameter(*value_out.mutable_block_parameter(), v);
                },
                [&](const ir::Function* v) { value_out.set_function(Function(v)); },
                [&](const ir::Constant* v) { value_out.set_constant(ConstantValue(v->Value())); },
                TINT_ICE_ON_NO_MATCH);

            return id;
        });
    }

    void InstructionResult(pb::InstructionResult& res_out, const ir::InstructionResult* res_in) {
        res_out.set_type(Type(res_in->Type()));
        if (auto name = mod_in_.NameOf(res_in); name.IsValid()) {
            res_out.set_name(name.Name());
        }
    }

    void FunctionParameter(pb::FunctionParameter& param_out, const ir::FunctionParam* param_in) {
        param_out.set_type(Type(param_in->Type()));
        if (auto name = mod_in_.NameOf(param_in); name.IsValid()) {
            param_out.set_name(name.Name());
        }
        if (auto bp_in = param_in->BindingPoint()) {
            auto& bp_out = *param_out.mutable_attributes()->mutable_binding_point();
            BindingPoint(bp_out, *bp_in);
        }
        if (auto location_in = param_in->Location()) {
            param_out.mutable_attributes()->set_location(*location_in);
        }
        if (auto color_in = param_in->Color()) {
            param_out.mutable_attributes()->set_color(*color_in);
        }
        if (auto interpolation_in = param_in->Interpolation()) {
            auto& interpolation_out = *param_out.mutable_attributes()->mutable_interpolation();
            Interpolation(interpolation_out, *interpolation_in);
        }
        if (auto builtin_in = param_in->Builtin()) {
            param_out.mutable_attributes()->set_builtin(BuiltinValue(*builtin_in));
        }
        if (param_in->Invariant()) {
            param_out.mutable_attributes()->set_invariant(true);
        }
    }

    void BlockParameter(pb::BlockParameter& param_out, const ir::BlockParam* param_in) {
        param_out.set_type(Type(param_in->Type()));
        if (auto name = mod_in_.NameOf(param_in); name.IsValid()) {
            param_out.set_name(name.Name());
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // ConstantValues
    ////////////////////////////////////////////////////////////////////////////
    uint32_t ConstantValue(const core::constant::Value* constant_in) {
        TINT_ASSERT(constant_in != nullptr);
        return constant_values_.GetOrAdd(constant_in, [&] {
            pb::ConstantValue constant_out;
            tint::Switch(
                constant_in,  //
                [&](const core::constant::Scalar<bool>* b) {
                    constant_out.mutable_scalar()->set_bool_(b->value);
                },
                [&](const core::constant::Scalar<core::i32>* i32) {
                    constant_out.mutable_scalar()->set_i32(i32->value);
                },
                [&](const core::constant::Scalar<core::u32>* u32) {
                    constant_out.mutable_scalar()->set_u32(u32->value);
                },
                [&](const core::constant::Scalar<core::f32>* f32) {
                    constant_out.mutable_scalar()->set_f32(f32->value);
                },
                [&](const core::constant::Scalar<core::f16>* f16) {
                    constant_out.mutable_scalar()->set_f16(f16->value);
                },
                [&](const core::constant::Composite* composite) {
                    ConstantValueComposite(*constant_out.mutable_composite(), composite);
                },
                [&](const core::constant::Splat* splat) {
                    ConstantValueSplat(*constant_out.mutable_splat(), splat);
                },
                TINT_ICE_ON_NO_MATCH);

            mod_out_.mutable_constant_values()->Add(std::move(constant_out));
            return static_cast<uint32_t>(mod_out_.constant_values().size() - 1);
        });
    }

    void ConstantValueComposite(pb::ConstantValueComposite& composite_out,
                                const core::constant::Composite* composite_in) {
        composite_out.set_type(Type(composite_in->type));
        for (auto* el : composite_in->elements) {
            composite_out.add_elements(ConstantValue(el));
        }
    }

    void ConstantValueSplat(pb::ConstantValueSplat& splat_out,
                            const core::constant::Splat* splat_in) {
        splat_out.set_type(Type(splat_in->type));
        if (DAWN_UNLIKELY(splat_in->count > internal_limits::kMaxArrayConstructorElements)) {
            err_ << "array constructor has excessive number of elements (>"
                 << internal_limits::kMaxArrayConstructorElements << ")\n";
        }
        splat_out.set_elements(ConstantValue(splat_in->el));
        splat_out.set_count(static_cast<uint32_t>(splat_in->count));
    }

    ////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////////////////////////////////////////
    void Interpolation(pb::Interpolation& interpolation_out,
                       const core::Interpolation& interpolation_in) {
        interpolation_out.set_type(InterpolationType(interpolation_in.type));
        if (interpolation_in.sampling != InterpolationSampling::kUndefined) {
            interpolation_out.set_sampling(InterpolationSampling(interpolation_in.sampling));
        }
    }

    void BindingPoint(pb::BindingPoint& binding_point_out, const BindingPoint& binding_point_in) {
        binding_point_out.set_group(binding_point_in.group);
        binding_point_out.set_binding(binding_point_in.binding);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Enums
    ////////////////////////////////////////////////////////////////////////////
    pb::AddressSpace AddressSpace(core::AddressSpace in) {
        switch (in) {
            case core::AddressSpace::kFunction:
                return pb::AddressSpace::function;
            case core::AddressSpace::kHandle:
                return pb::AddressSpace::handle;
            case core::AddressSpace::kPixelLocal:
                return pb::AddressSpace::pixel_local;
            case core::AddressSpace::kPrivate:
                return pb::AddressSpace::private_;
            case core::AddressSpace::kPushConstant:
                return pb::AddressSpace::push_constant;
            case core::AddressSpace::kStorage:
                return pb::AddressSpace::storage;
            case core::AddressSpace::kUniform:
                return pb::AddressSpace::uniform;
            case core::AddressSpace::kWorkgroup:
                return pb::AddressSpace::workgroup;

            case core::AddressSpace::kUndefined:
            case core::AddressSpace::kIn:
            case core::AddressSpace::kOut:
                break;
        }
        TINT_ICE() << "invalid AddressSpace: " << in;
    }

    pb::AccessControl AccessControl(core::Access in) {
        switch (in) {
            case core::Access::kRead:
                return pb::AccessControl::read;
            case core::Access::kWrite:
                return pb::AccessControl::write;
            case core::Access::kReadWrite:
                return pb::AccessControl::read_write;
            case core::Access::kUndefined:
                break;
        }
        TINT_ICE() << "invalid Access: " << in;
    }

    pb::UnaryOp UnaryOp(core::UnaryOp in) {
        switch (in) {
            case core::UnaryOp::kComplement:
                return pb::UnaryOp::complement;
            case core::UnaryOp::kNegation:
                return pb::UnaryOp::negation;
            case core::UnaryOp::kAddressOf:
                return pb::UnaryOp::address_of;
            case core::UnaryOp::kIndirection:
                return pb::UnaryOp::indirection;
            case core::UnaryOp::kNot:
                return pb::UnaryOp::not_;
        }
        TINT_ICE() << "invalid UnaryOp: " << in;
    }

    pb::BinaryOp BinaryOp(core::BinaryOp in) {
        switch (in) {
            case core::BinaryOp::kAdd:
                return pb::BinaryOp::add_;
            case core::BinaryOp::kSubtract:
                return pb::BinaryOp::subtract;
            case core::BinaryOp::kMultiply:
                return pb::BinaryOp::multiply;
            case core::BinaryOp::kDivide:
                return pb::BinaryOp::divide;
            case core::BinaryOp::kModulo:
                return pb::BinaryOp::modulo;
            case core::BinaryOp::kAnd:
                return pb::BinaryOp::and_;
            case core::BinaryOp::kOr:
                return pb::BinaryOp::or_;
            case core::BinaryOp::kXor:
                return pb::BinaryOp::xor_;
            case core::BinaryOp::kEqual:
                return pb::BinaryOp::equal;
            case core::BinaryOp::kNotEqual:
                return pb::BinaryOp::not_equal;
            case core::BinaryOp::kLessThan:
                return pb::BinaryOp::less_than;
            case core::BinaryOp::kGreaterThan:
                return pb::BinaryOp::greater_than;
            case core::BinaryOp::kLessThanEqual:
                return pb::BinaryOp::less_than_equal;
            case core::BinaryOp::kGreaterThanEqual:
                return pb::BinaryOp::greater_than_equal;
            case core::BinaryOp::kShiftLeft:
                return pb::BinaryOp::shift_left;
            case core::BinaryOp::kShiftRight:
                return pb::BinaryOp::shift_right;
            case core::BinaryOp::kLogicalAnd:
                return pb::BinaryOp::logical_and;
            case core::BinaryOp::kLogicalOr:
                return pb::BinaryOp::logical_or;
        }

        TINT_ICE() << "invalid BinaryOp: " << in;
    }

    pb::TextureDimension TextureDimension(core::type::TextureDimension in) {
        switch (in) {
            case core::type::TextureDimension::k1d:
                return pb::TextureDimension::_1d;
            case core::type::TextureDimension::k2d:
                return pb::TextureDimension::_2d;
            case core::type::TextureDimension::k2dArray:
                return pb::TextureDimension::_2d_array;
            case core::type::TextureDimension::k3d:
                return pb::TextureDimension::_3d;
            case core::type::TextureDimension::kCube:
                return pb::TextureDimension::cube;
            case core::type::TextureDimension::kCubeArray:
                return pb::TextureDimension::cube_array;
            case core::type::TextureDimension::kNone:
                break;
        }

        TINT_ICE() << "invalid TextureDimension: " << in;
    }

    pb::TexelFormat TexelFormat(core::TexelFormat in) {
        switch (in) {
            case core::TexelFormat::kBgra8Unorm:
                return pb::TexelFormat::bgra8_unorm;
            case core::TexelFormat::kR32Float:
                return pb::TexelFormat::r32_float;
            case core::TexelFormat::kR32Sint:
                return pb::TexelFormat::r32_sint;
            case core::TexelFormat::kR32Uint:
                return pb::TexelFormat::r32_uint;
            case core::TexelFormat::kR8Unorm:
                return pb::TexelFormat::r8_unorm;
            case core::TexelFormat::kRg32Float:
                return pb::TexelFormat::rg32_float;
            case core::TexelFormat::kRg32Sint:
                return pb::TexelFormat::rg32_sint;
            case core::TexelFormat::kRg32Uint:
                return pb::TexelFormat::rg32_uint;
            case core::TexelFormat::kRgba16Float:
                return pb::TexelFormat::rgba16_float;
            case core::TexelFormat::kRgba16Sint:
                return pb::TexelFormat::rgba16_sint;
            case core::TexelFormat::kRgba16Uint:
                return pb::TexelFormat::rgba16_uint;
            case core::TexelFormat::kRgba32Float:
                return pb::TexelFormat::rgba32_float;
            case core::TexelFormat::kRgba32Sint:
                return pb::TexelFormat::rgba32_sint;
            case core::TexelFormat::kRgba32Uint:
                return pb::TexelFormat::rgba32_uint;
            case core::TexelFormat::kRgba8Sint:
                return pb::TexelFormat::rgba8_sint;
            case core::TexelFormat::kRgba8Snorm:
                return pb::TexelFormat::rgba8_snorm;
            case core::TexelFormat::kRgba8Uint:
                return pb::TexelFormat::rgba8_uint;
            case core::TexelFormat::kRgba8Unorm:
                return pb::TexelFormat::rgba8_unorm;
            case core::TexelFormat::kUndefined:
                break;
        }

        TINT_ICE() << "invalid TexelFormat: " << in;
    }

    pb::SamplerKind SamplerKind(core::type::SamplerKind in) {
        switch (in) {
            case core::type::SamplerKind::kSampler:
                return pb::SamplerKind::sampler;
            case core::type::SamplerKind::kComparisonSampler:
                return pb::SamplerKind::comparison;
        }

        TINT_ICE() << "invalid SamplerKind: " << in;
    }

    pb::InterpolationType InterpolationType(core::InterpolationType in) {
        switch (in) {
            case core::InterpolationType::kFlat:
                return pb::InterpolationType::flat;
            case core::InterpolationType::kLinear:
                return pb::InterpolationType::linear;
            case core::InterpolationType::kPerspective:
                return pb::InterpolationType::perspective;
            case core::InterpolationType::kUndefined:
                break;
        }
        TINT_ICE() << "invalid InterpolationType: " << in;
    }

    pb::InterpolationSampling InterpolationSampling(core::InterpolationSampling in) {
        switch (in) {
            case core::InterpolationSampling::kCenter:
                return pb::InterpolationSampling::center;
            case core::InterpolationSampling::kCentroid:
                return pb::InterpolationSampling::centroid;
            case core::InterpolationSampling::kSample:
                return pb::InterpolationSampling::sample;
            case core::InterpolationSampling::kFirst:
                return pb::InterpolationSampling::first;
            case core::InterpolationSampling::kEither:
                return pb::InterpolationSampling::either;
            case core::InterpolationSampling::kUndefined:
                break;
        }
        TINT_ICE() << "invalid InterpolationSampling: " << in;
    }

    pb::BuiltinValue BuiltinValue(core::BuiltinValue in) {
        switch (in) {
            case core::BuiltinValue::kPointSize:
                return pb::BuiltinValue::point_size;
            case core::BuiltinValue::kCullDistance:
                return pb::BuiltinValue::cull_distance;
            case core::BuiltinValue::kFragDepth:
                return pb::BuiltinValue::frag_depth;
            case core::BuiltinValue::kFrontFacing:
                return pb::BuiltinValue::front_facing;
            case core::BuiltinValue::kGlobalInvocationId:
                return pb::BuiltinValue::global_invocation_id;
            case core::BuiltinValue::kInstanceIndex:
                return pb::BuiltinValue::instance_index;
            case core::BuiltinValue::kLocalInvocationId:
                return pb::BuiltinValue::local_invocation_id;
            case core::BuiltinValue::kLocalInvocationIndex:
                return pb::BuiltinValue::local_invocation_index;
            case core::BuiltinValue::kNumWorkgroups:
                return pb::BuiltinValue::num_workgroups;
            case core::BuiltinValue::kPosition:
                return pb::BuiltinValue::position;
            case core::BuiltinValue::kSampleIndex:
                return pb::BuiltinValue::sample_index;
            case core::BuiltinValue::kSampleMask:
                return pb::BuiltinValue::sample_mask;
            case core::BuiltinValue::kSubgroupInvocationId:
                return pb::BuiltinValue::subgroup_invocation_id;
            case core::BuiltinValue::kSubgroupSize:
                return pb::BuiltinValue::subgroup_size;
            case core::BuiltinValue::kVertexIndex:
                return pb::BuiltinValue::vertex_index;
            case core::BuiltinValue::kWorkgroupId:
                return pb::BuiltinValue::workgroup_id;
            case core::BuiltinValue::kClipDistances:
                return pb::BuiltinValue::clip_distances;
            case core::BuiltinValue::kUndefined:
                break;
        }
        TINT_ICE() << "invalid BuiltinValue: " << in;
    }

    pb::BuiltinFn BuiltinFn(core::BuiltinFn in) {
        switch (in) {
            case core::BuiltinFn::kAbs:
                return pb::BuiltinFn::abs;
            case core::BuiltinFn::kAcos:
                return pb::BuiltinFn::acos;
            case core::BuiltinFn::kAcosh:
                return pb::BuiltinFn::acosh;
            case core::BuiltinFn::kAll:
                return pb::BuiltinFn::all;
            case core::BuiltinFn::kAny:
                return pb::BuiltinFn::any;
            case core::BuiltinFn::kArrayLength:
                return pb::BuiltinFn::array_length;
            case core::BuiltinFn::kAsin:
                return pb::BuiltinFn::asin;
            case core::BuiltinFn::kAsinh:
                return pb::BuiltinFn::asinh;
            case core::BuiltinFn::kAtan:
                return pb::BuiltinFn::atan;
            case core::BuiltinFn::kAtan2:
                return pb::BuiltinFn::atan2;
            case core::BuiltinFn::kAtanh:
                return pb::BuiltinFn::atanh;
            case core::BuiltinFn::kCeil:
                return pb::BuiltinFn::ceil;
            case core::BuiltinFn::kClamp:
                return pb::BuiltinFn::clamp;
            case core::BuiltinFn::kCos:
                return pb::BuiltinFn::cos;
            case core::BuiltinFn::kCosh:
                return pb::BuiltinFn::cosh;
            case core::BuiltinFn::kCountLeadingZeros:
                return pb::BuiltinFn::count_leading_zeros;
            case core::BuiltinFn::kCountOneBits:
                return pb::BuiltinFn::count_one_bits;
            case core::BuiltinFn::kCountTrailingZeros:
                return pb::BuiltinFn::count_trailing_zeros;
            case core::BuiltinFn::kCross:
                return pb::BuiltinFn::cross;
            case core::BuiltinFn::kDegrees:
                return pb::BuiltinFn::degrees;
            case core::BuiltinFn::kDeterminant:
                return pb::BuiltinFn::determinant;
            case core::BuiltinFn::kDistance:
                return pb::BuiltinFn::distance;
            case core::BuiltinFn::kDot:
                return pb::BuiltinFn::dot;
            case core::BuiltinFn::kDot4I8Packed:
                return pb::BuiltinFn::dot4i8_packed;
            case core::BuiltinFn::kDot4U8Packed:
                return pb::BuiltinFn::dot4u8_packed;
            case core::BuiltinFn::kDpdx:
                return pb::BuiltinFn::dpdx;
            case core::BuiltinFn::kDpdxCoarse:
                return pb::BuiltinFn::dpdx_coarse;
            case core::BuiltinFn::kDpdxFine:
                return pb::BuiltinFn::dpdx_fine;
            case core::BuiltinFn::kDpdy:
                return pb::BuiltinFn::dpdy;
            case core::BuiltinFn::kDpdyCoarse:
                return pb::BuiltinFn::dpdy_coarse;
            case core::BuiltinFn::kDpdyFine:
                return pb::BuiltinFn::dpdy_fine;
            case core::BuiltinFn::kExp:
                return pb::BuiltinFn::exp;
            case core::BuiltinFn::kExp2:
                return pb::BuiltinFn::exp2;
            case core::BuiltinFn::kExtractBits:
                return pb::BuiltinFn::extract_bits;
            case core::BuiltinFn::kFaceForward:
                return pb::BuiltinFn::face_forward;
            case core::BuiltinFn::kFirstLeadingBit:
                return pb::BuiltinFn::first_leading_bit;
            case core::BuiltinFn::kFirstTrailingBit:
                return pb::BuiltinFn::first_trailing_bit;
            case core::BuiltinFn::kFloor:
                return pb::BuiltinFn::floor;
            case core::BuiltinFn::kFma:
                return pb::BuiltinFn::fma;
            case core::BuiltinFn::kFract:
                return pb::BuiltinFn::fract;
            case core::BuiltinFn::kFrexp:
                return pb::BuiltinFn::frexp;
            case core::BuiltinFn::kFwidth:
                return pb::BuiltinFn::fwidth;
            case core::BuiltinFn::kFwidthCoarse:
                return pb::BuiltinFn::fwidth_coarse;
            case core::BuiltinFn::kFwidthFine:
                return pb::BuiltinFn::fwidth_fine;
            case core::BuiltinFn::kInsertBits:
                return pb::BuiltinFn::insert_bits;
            case core::BuiltinFn::kInverseSqrt:
                return pb::BuiltinFn::inverse_sqrt;
            case core::BuiltinFn::kLdexp:
                return pb::BuiltinFn::ldexp;
            case core::BuiltinFn::kLength:
                return pb::BuiltinFn::length;
            case core::BuiltinFn::kLog:
                return pb::BuiltinFn::log;
            case core::BuiltinFn::kLog2:
                return pb::BuiltinFn::log2;
            case core::BuiltinFn::kMax:
                return pb::BuiltinFn::max;
            case core::BuiltinFn::kMin:
                return pb::BuiltinFn::min;
            case core::BuiltinFn::kMix:
                return pb::BuiltinFn::mix;
            case core::BuiltinFn::kModf:
                return pb::BuiltinFn::modf;
            case core::BuiltinFn::kNormalize:
                return pb::BuiltinFn::normalize;
            case core::BuiltinFn::kPack2X16Float:
                return pb::BuiltinFn::pack2x16_float;
            case core::BuiltinFn::kPack2X16Snorm:
                return pb::BuiltinFn::pack2x16_snorm;
            case core::BuiltinFn::kPack2X16Unorm:
                return pb::BuiltinFn::pack2x16_unorm;
            case core::BuiltinFn::kPack4X8Snorm:
                return pb::BuiltinFn::pack4x8_snorm;
            case core::BuiltinFn::kPack4X8Unorm:
                return pb::BuiltinFn::pack4x8_unorm;
            case core::BuiltinFn::kPack4XI8:
                return pb::BuiltinFn::pack4xi8;
            case core::BuiltinFn::kPack4XU8:
                return pb::BuiltinFn::pack4xu8;
            case core::BuiltinFn::kPack4XI8Clamp:
                return pb::BuiltinFn::pack4xi8_clamp;
            case core::BuiltinFn::kPack4XU8Clamp:
                return pb::BuiltinFn::pack4xu8_clamp;
            case core::BuiltinFn::kPow:
                return pb::BuiltinFn::pow;
            case core::BuiltinFn::kQuantizeToF16:
                return pb::BuiltinFn::quantize_to_f16;
            case core::BuiltinFn::kRadians:
                return pb::BuiltinFn::radians;
            case core::BuiltinFn::kReflect:
                return pb::BuiltinFn::reflect;
            case core::BuiltinFn::kRefract:
                return pb::BuiltinFn::refract;
            case core::BuiltinFn::kReverseBits:
                return pb::BuiltinFn::reverse_bits;
            case core::BuiltinFn::kRound:
                return pb::BuiltinFn::round;
            case core::BuiltinFn::kSaturate:
                return pb::BuiltinFn::saturate;
            case core::BuiltinFn::kSelect:
                return pb::BuiltinFn::select;
            case core::BuiltinFn::kSign:
                return pb::BuiltinFn::sign;
            case core::BuiltinFn::kSin:
                return pb::BuiltinFn::sin;
            case core::BuiltinFn::kSinh:
                return pb::BuiltinFn::sinh;
            case core::BuiltinFn::kSmoothstep:
                return pb::BuiltinFn::smoothstep;
            case core::BuiltinFn::kSqrt:
                return pb::BuiltinFn::sqrt;
            case core::BuiltinFn::kStep:
                return pb::BuiltinFn::step;
            case core::BuiltinFn::kStorageBarrier:
                return pb::BuiltinFn::storage_barrier;
            case core::BuiltinFn::kTan:
                return pb::BuiltinFn::tan;
            case core::BuiltinFn::kTanh:
                return pb::BuiltinFn::tanh;
            case core::BuiltinFn::kTranspose:
                return pb::BuiltinFn::transpose;
            case core::BuiltinFn::kTrunc:
                return pb::BuiltinFn::trunc;
            case core::BuiltinFn::kUnpack2X16Float:
                return pb::BuiltinFn::unpack2x16_float;
            case core::BuiltinFn::kUnpack2X16Snorm:
                return pb::BuiltinFn::unpack2x16_snorm;
            case core::BuiltinFn::kUnpack2X16Unorm:
                return pb::BuiltinFn::unpack2x16_unorm;
            case core::BuiltinFn::kUnpack4X8Snorm:
                return pb::BuiltinFn::unpack4x8_snorm;
            case core::BuiltinFn::kUnpack4X8Unorm:
                return pb::BuiltinFn::unpack4x8_unorm;
            case core::BuiltinFn::kUnpack4XI8:
                return pb::BuiltinFn::unpack4xi8;
            case core::BuiltinFn::kUnpack4XU8:
                return pb::BuiltinFn::unpack4xu8;
            case core::BuiltinFn::kWorkgroupBarrier:
                return pb::BuiltinFn::workgroup_barrier;
            case core::BuiltinFn::kTextureBarrier:
                return pb::BuiltinFn::texture_barrier;
            case core::BuiltinFn::kTextureDimensions:
                return pb::BuiltinFn::texture_dimensions;
            case core::BuiltinFn::kTextureGather:
                return pb::BuiltinFn::texture_gather;
            case core::BuiltinFn::kTextureGatherCompare:
                return pb::BuiltinFn::texture_gather_compare;
            case core::BuiltinFn::kTextureNumLayers:
                return pb::BuiltinFn::texture_num_layers;
            case core::BuiltinFn::kTextureNumLevels:
                return pb::BuiltinFn::texture_num_levels;
            case core::BuiltinFn::kTextureNumSamples:
                return pb::BuiltinFn::texture_num_samples;
            case core::BuiltinFn::kTextureSample:
                return pb::BuiltinFn::texture_sample;
            case core::BuiltinFn::kTextureSampleBias:
                return pb::BuiltinFn::texture_sample_bias;
            case core::BuiltinFn::kTextureSampleCompare:
                return pb::BuiltinFn::texture_sample_compare;
            case core::BuiltinFn::kTextureSampleCompareLevel:
                return pb::BuiltinFn::texture_sample_compare_level;
            case core::BuiltinFn::kTextureSampleGrad:
                return pb::BuiltinFn::texture_sample_grad;
            case core::BuiltinFn::kTextureSampleLevel:
                return pb::BuiltinFn::texture_sample_level;
            case core::BuiltinFn::kTextureSampleBaseClampToEdge:
                return pb::BuiltinFn::texture_sample_base_clamp_to_edge;
            case core::BuiltinFn::kTextureStore:
                return pb::BuiltinFn::texture_store;
            case core::BuiltinFn::kTextureLoad:
                return pb::BuiltinFn::texture_load;
            case core::BuiltinFn::kAtomicLoad:
                return pb::BuiltinFn::atomic_load;
            case core::BuiltinFn::kAtomicStore:
                return pb::BuiltinFn::atomic_store;
            case core::BuiltinFn::kAtomicAdd:
                return pb::BuiltinFn::atomic_add;
            case core::BuiltinFn::kAtomicSub:
                return pb::BuiltinFn::atomic_sub;
            case core::BuiltinFn::kAtomicMax:
                return pb::BuiltinFn::atomic_max;
            case core::BuiltinFn::kAtomicMin:
                return pb::BuiltinFn::atomic_min;
            case core::BuiltinFn::kAtomicAnd:
                return pb::BuiltinFn::atomic_and;
            case core::BuiltinFn::kAtomicOr:
                return pb::BuiltinFn::atomic_or;
            case core::BuiltinFn::kAtomicXor:
                return pb::BuiltinFn::atomic_xor;
            case core::BuiltinFn::kAtomicExchange:
                return pb::BuiltinFn::atomic_exchange;
            case core::BuiltinFn::kAtomicCompareExchangeWeak:
                return pb::BuiltinFn::atomic_compare_exchange_weak;
            case core::BuiltinFn::kSubgroupBallot:
                return pb::BuiltinFn::subgroup_ballot;
            case core::BuiltinFn::kSubgroupElect:
                return pb::BuiltinFn::subgroup_elect;
            case core::BuiltinFn::kSubgroupBroadcast:
                return pb::BuiltinFn::subgroup_broadcast;
            case core::BuiltinFn::kSubgroupBroadcastFirst:
                return pb::BuiltinFn::subgroup_broadcast_first;
            case core::BuiltinFn::kSubgroupShuffle:
                return pb::BuiltinFn::subgroup_shuffle;
            case core::BuiltinFn::kSubgroupShuffleXor:
                return pb::BuiltinFn::subgroup_shuffle_xor;
            case core::BuiltinFn::kSubgroupShuffleUp:
                return pb::BuiltinFn::subgroup_shuffle_up;
            case core::BuiltinFn::kSubgroupShuffleDown:
                return pb::BuiltinFn::subgroup_shuffle_down;
            case core::BuiltinFn::kInputAttachmentLoad:
                return pb::BuiltinFn::input_attachment_load;
            case core::BuiltinFn::kSubgroupAdd:
                return pb::BuiltinFn::subgroup_add;
            case core::BuiltinFn::kSubgroupInclusiveAdd:
                return pb::BuiltinFn::subgroup_inclusive_add;
            case core::BuiltinFn::kSubgroupExclusiveAdd:
                return pb::BuiltinFn::subgroup_exclusive_add;
            case core::BuiltinFn::kSubgroupMul:
                return pb::BuiltinFn::subgroup_mul;
            case core::BuiltinFn::kSubgroupInclusiveMul:
                return pb::BuiltinFn::subgroup_inclusive_mul;
            case core::BuiltinFn::kSubgroupExclusiveMul:
                return pb::BuiltinFn::subgroup_exclusive_mul;
            case core::BuiltinFn::kSubgroupAnd:
                return pb::BuiltinFn::subgroup_and;
            case core::BuiltinFn::kSubgroupOr:
                return pb::BuiltinFn::subgroup_or;
            case core::BuiltinFn::kSubgroupXor:
                return pb::BuiltinFn::subgroup_xor;
            case core::BuiltinFn::kSubgroupMin:
                return pb::BuiltinFn::subgroup_min;
            case core::BuiltinFn::kSubgroupMax:
                return pb::BuiltinFn::subgroup_max;
            case core::BuiltinFn::kSubgroupAll:
                return pb::BuiltinFn::subgroup_all;
            case core::BuiltinFn::kSubgroupAny:
                return pb::BuiltinFn::subgroup_any;
            case core::BuiltinFn::kQuadBroadcast:
                return pb::BuiltinFn::quad_broadcast;
            case core::BuiltinFn::kQuadSwapX:
                return pb::BuiltinFn::quad_swap_x;
            case core::BuiltinFn::kQuadSwapY:
                return pb::BuiltinFn::quad_swap_y;
            case core::BuiltinFn::kQuadSwapDiagonal:
                return pb::BuiltinFn::quad_swap_diagonal;
            case core::BuiltinFn::kSubgroupMatrixLoad:
                return pb::BuiltinFn::subgroup_matrix_load;
            case core::BuiltinFn::kSubgroupMatrixStore:
                return pb::BuiltinFn::subgroup_matrix_store;
            case core::BuiltinFn::kSubgroupMatrixMultiply:
                return pb::BuiltinFn::subgroup_matrix_multiply;
            case core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate:
                return pb::BuiltinFn::subgroup_matrix_multiply_accumulate;
            case core::BuiltinFn::kNone:
                break;
        }
        TINT_ICE() << "invalid BuiltinFn: " << in;
    }
};

}  // namespace

Result<std::unique_ptr<pb::Module>> EncodeToProto(const Module& mod_in) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    pb::Module mod_out;
    auto res = Encoder{mod_in, mod_out}.Encode();
    if (res != Success) {
        return res.Failure();
    }

    return std::make_unique<pb::Module>(mod_out);
}

Result<Vector<std::byte, 0>> EncodeToBinary(const Module& mod_in) {
    auto mod_out = EncodeToProto(mod_in);
    if (mod_out != Success) {
        return mod_out.Failure();
    }

    Vector<std::byte, 0> buffer;
    size_t len = mod_out.Get()->ByteSizeLong();
    buffer.Resize(len);
    if (len > 0) {
        if (!mod_out.Get()->SerializeToArray(&buffer[0], static_cast<int>(len))) {
            return Failure{"failed to serialize protobuf"};
        }
    }
    return buffer;
}

}  // namespace tint::core::ir::binary
