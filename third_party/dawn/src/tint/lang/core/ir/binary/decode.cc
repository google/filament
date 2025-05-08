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

#include "src/tint/lang/core/ir/binary/decode.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/function.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/invalid.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/text_style.h"

TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS();
#include "src/tint/utils/protos/ir/ir.pb.h"
TINT_END_DISABLE_PROTOBUF_WARNINGS();

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir::binary {
namespace {

struct Decoder {
    const pb::Module& mod_in_;

    Module mod_out_{};
    Vector<ir::Block*, 32> blocks_{};
    Vector<const type::Type*, 32> types_{};
    Vector<const core::constant::Value*, 32> constant_values_{};
    Vector<ir::Value*, 32> values_{};
    Builder b{mod_out_};

    Vector<ir::ExitIf*, 32> exit_ifs_{};
    Vector<ir::ExitSwitch*, 32> exit_switches_{};
    Vector<ir::ExitLoop*, 32> exit_loops_{};
    Vector<ir::NextIteration*, 32> next_iterations_{};
    Vector<ir::BreakIf*, 32> break_ifs_{};
    Vector<ir::Continue*, 32> continues_{};

    std::stringstream err_{};
    Hashset<std::string, 4> struct_names_{};

    Result<Module> Decode() {
        {
            const size_t n = static_cast<size_t>(mod_in_.types().size());
            types_.Reserve(n);
            for (auto& type_in : mod_in_.types()) {
                types_.Push(CreateType(type_in));
            }
        }
        {
            const size_t n = static_cast<size_t>(mod_in_.functions().size());
            mod_out_.functions.Reserve(n);
            for (auto& fn_in : mod_in_.functions()) {
                mod_out_.functions.Push(CreateFunction(fn_in));
            }
        }
        {
            const size_t n = static_cast<size_t>(mod_in_.blocks().size());
            blocks_.Reserve(n);
            for (size_t i = 0; i < n; i++) {
                auto id = static_cast<uint32_t>(i);
                if (id == mod_in_.root_block()) {
                    blocks_.Push(mod_out_.root_block);
                } else {
                    auto& block_in = mod_in_.blocks()[static_cast<int>(i)];
                    blocks_.Push(CreateBlock(block_in));
                }
            }
        }
        {
            const size_t n = static_cast<size_t>(mod_in_.constant_values().size());
            constant_values_.Reserve(n);
            for (auto& value_in : mod_in_.constant_values()) {
                constant_values_.Push(CreateConstantValue(value_in));
            }
        }
        {
            const size_t n = static_cast<size_t>(mod_in_.values().size());
            values_.Reserve(n);
            for (auto& value_in : mod_in_.values()) {
                values_.Push(CreateValue(value_in));
            }
        }
        for (size_t i = 0, n = static_cast<size_t>(mod_in_.functions().size()); i < n; i++) {
            PopulateFunction(mod_out_.functions[i], mod_in_.functions()[static_cast<int>(i)]);
        }
        for (size_t i = 0, n = static_cast<size_t>(mod_in_.blocks().size()); i < n; i++) {
            PopulateBlock(blocks_[i], mod_in_.blocks()[static_cast<int>(i)]);
        }

        auto err = err_.str();
        if (!err.empty()) {
            // Note: Its not safe to call InferControlInstruction() with a broken IR.
            return Failure{err};
        }

        if (CheckBlocks()) {
            for (auto* exit : exit_ifs_) {
                InferControlInstruction(exit, &ExitIf::SetIf);
            }
            for (auto* exit : exit_switches_) {
                InferControlInstruction(exit, &ExitSwitch::SetSwitch);
            }
            for (auto* exit : exit_loops_) {
                InferControlInstruction(exit, &ExitLoop::SetLoop);
            }
            for (auto* break_ifs : break_ifs_) {
                InferControlInstruction(break_ifs, &BreakIf::SetLoop);
            }
            for (auto* next_iters : next_iterations_) {
                InferControlInstruction(next_iters, &NextIteration::SetLoop);
            }
            for (auto* cont : continues_) {
                InferControlInstruction(cont, &Continue::SetLoop);
            }
        }

        err = err_.str();
        if (!err.empty()) {
            return Failure{err};
        }
        return std::move(mod_out_);
    }

    /// Errors if @p number is not finite.
    /// @returns @p number if finite, otherwise 0.
    template <typename T>
    Number<T> CheckFinite(Number<T> number) {
        if (DAWN_UNLIKELY(!std::isfinite(number.value))) {
            err_ << "value must be finite\n";
            return Number<T>{};
        }
        return number;
    }

    /// @returns true if all blocks are reachable, acyclic nesting depth is less than or equal to
    /// kMaxBlockDepth.
    bool CheckBlocks() {
        const size_t kMaxBlockDepth = 128;
        Vector<std::pair<const ir::Block*, size_t>, 32> pending;
        pending.Push(std::make_pair(mod_out_.root_block, 0));
        for (auto& fn : mod_out_.functions) {
            pending.Push(std::make_pair(fn->Block(), 0));
        }
        Hashset<const ir::Block*, 32> seen;
        while (!pending.IsEmpty()) {
            const auto block_depth = pending.Pop();
            const auto* block = block_depth.first;
            const size_t depth = block_depth.second;
            if (!seen.Add(block)) {
                err_ << "cyclic nesting of blocks\n";
                return false;
            }
            if (depth > kMaxBlockDepth) {
                err_ << "block nesting exceeds " << kMaxBlockDepth << "\n";
                return false;
            }
            for (auto* inst = block->Instructions(); inst; inst = inst->next) {
                if (auto* ctrl = inst->As<ir::ControlInstruction>()) {
                    ctrl->ForeachBlock([&](const ir::Block* child) {
                        pending.Push(std::make_pair(child, depth + 1));
                    });
                }
            }
        }

        for (auto* block : blocks_) {
            if (!seen.Contains(block)) {
                err_ << "unreachable block\n";
                return false;
            }
        }

        return true;
    }

    template <typename EXIT, typename CTRL_INST>
    void InferControlInstruction(EXIT* exit, void (EXIT::*set)(CTRL_INST*)) {
        for (auto* block = exit->Block(); block;) {
            auto* parent = block->Parent();
            if (!parent) {
                break;
            }
            if (auto* ctrl_inst = parent->template As<CTRL_INST>()) {
                (exit->*set)(ctrl_inst);
                break;
            }
            block = parent->Block();
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Functions
    ////////////////////////////////////////////////////////////////////////////
    ir::Function* CreateFunction(const pb::Function&) {
        auto* result = mod_out_.CreateValue<ir::Function>();
        result->SetType(mod_out_.Types().function());
        return result;
    }

    void PopulateFunction(ir::Function* fn_out, const pb::Function& fn_in) {
        if (!fn_in.name().empty()) {
            if (DAWN_UNLIKELY(fn_in.name().find('\0') != std::string::npos)) {
                err_ << "function name '" << fn_in.name()
                     << "' contains '\\0' before end of the string\n";
            } else {
                mod_out_.SetName(fn_out, fn_in.name());
            }
        }
        fn_out->SetReturnType(Type(fn_in.return_type()));
        if (fn_in.has_pipeline_stage()) {
            fn_out->SetStage(PipelineStage(fn_in.pipeline_stage()));
        }
        if (fn_in.has_workgroup_size()) {
            auto& wg_size_in = fn_in.workgroup_size();
            // TODO(dsinclair): When overrides are supported we should add support for generating
            // override expressions here.
            fn_out->SetWorkgroupSize(Value(wg_size_in.x()), Value(wg_size_in.y()),
                                     Value(wg_size_in.z()));
        }

        Vector<FunctionParam*, 8> params_out;
        for (auto param_in : fn_in.parameters()) {
            auto* param_out = ValueAs<FunctionParam>(param_in);
            if (DAWN_LIKELY(param_out)) {
                params_out.Push(param_out);
            }
        }
        if (fn_in.has_return_location()) {
            fn_out->SetReturnLocation(fn_in.return_location());
        }
        if (fn_in.has_return_interpolation()) {
            fn_out->SetReturnInterpolation(Interpolation(fn_in.return_interpolation()));
        }
        if (fn_in.has_return_builtin()) {
            fn_out->SetReturnBuiltin(BuiltinValue(fn_in.return_builtin()));
        }
        if (fn_in.return_invariant()) {
            fn_out->SetReturnInvariant(true);
        }
        fn_out->SetParams(std::move(params_out));
        fn_out->SetBlock(Block(fn_in.block()));
    }

    ir::Function* Function(uint32_t id) {
        if (DAWN_UNLIKELY(id >= mod_out_.functions.Length())) {
            err_ << "function id " << id << " out of range\n";
            return nullptr;
        }
        return mod_out_.functions[id];
    }

    Function::PipelineStage PipelineStage(pb::PipelineStage stage) {
        switch (stage) {
            case pb::PipelineStage::Compute:
                return Function::PipelineStage::kCompute;
            case pb::PipelineStage::Fragment:
                return Function::PipelineStage::kFragment;
            case pb::PipelineStage::Vertex:
                return Function::PipelineStage::kVertex;

            case pb::PipelineStage::PipelineStage_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::PipelineStage::PipelineStage_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        TINT_ICE() << "unhandled PipelineStage: " << stage;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Blocks
    ////////////////////////////////////////////////////////////////////////////
    ir::Block* CreateBlock(const pb::Block& block_in) {
        return block_in.is_multi_in() ? b.MultiInBlock() : b.Block();
    }

    void PopulateBlock(ir::Block* block_out, const pb::Block& block_in) {
        if (auto* mib = block_out->As<ir::MultiInBlock>()) {
            Vector<ir::BlockParam*, 8> params;
            for (auto param_in : block_in.parameters()) {
                auto* param_out = ValueAs<BlockParam>(param_in);
                if (DAWN_LIKELY(param_out)) {
                    params.Push(param_out);
                }
            }
            mib->SetParams(std::move(params));
        }
        for (auto& inst : block_in.instructions()) {
            block_out->Append(Instruction(inst));
        }
    }

    ir::Block* Block(uint32_t id) {
        if (DAWN_UNLIKELY(id >= blocks_.Length())) {
            err_ << "block id " << id << " out of range\n";
            return b.Block();
        }
        return blocks_[id];
    }

    template <typename T>
    T* BlockAs(uint32_t id) {
        auto* block = Block(id);
        if (auto cast = As<T>(block); DAWN_LIKELY(cast)) {
            return cast;
        }
        err_ << "block " << id << " is " << (block ? block->TypeInfo().name : "<null>")
             << " expected " << TypeInfo::Of<T>().name << "\n";
        return nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Instructions
    ////////////////////////////////////////////////////////////////////////////
    ir::Instruction* Instruction(const pb::Instruction& inst_in) {
        ir::Instruction* inst_out = nullptr;
        switch (inst_in.kind_case()) {
            case pb::Instruction::KindCase::kAccess:
                inst_out = CreateInstructionAccess(inst_in.access());
                break;
            case pb::Instruction::KindCase::kBinary:
                inst_out = CreateInstructionBinary(inst_in.binary());
                break;
            case pb::Instruction::KindCase::kBitcast:
                inst_out = CreateInstructionBitcast(inst_in.bitcast());
                break;
            case pb::Instruction::KindCase::kBreakIf:
                inst_out = CreateInstructionBreakIf(inst_in.break_if());
                break;
            case pb::Instruction::KindCase::kBuiltinCall:
                inst_out = CreateInstructionBuiltinCall(inst_in.builtin_call());
                break;
            case pb::Instruction::KindCase::kConstruct:
                inst_out = CreateInstructionConstruct(inst_in.construct());
                break;
            case pb::Instruction::KindCase::kContinue:
                inst_out = CreateInstructionContinue(inst_in.continue_());
                break;
            case pb::Instruction::KindCase::kConvert:
                inst_out = CreateInstructionConvert(inst_in.convert());
                break;
            case pb::Instruction::KindCase::kExitIf:
                inst_out = CreateInstructionExitIf(inst_in.exit_if());
                break;
            case pb::Instruction::KindCase::kExitLoop:
                inst_out = CreateInstructionExitLoop(inst_in.exit_loop());
                break;
            case pb::Instruction::KindCase::kExitSwitch:
                inst_out = CreateInstructionExitSwitch(inst_in.exit_switch());
                break;
            case pb::Instruction::KindCase::kDiscard:
                inst_out = CreateInstructionDiscard(inst_in.discard());
                break;
            case pb::Instruction::KindCase::kIf:
                inst_out = CreateInstructionIf(inst_in.if_());
                break;
            case pb::Instruction::KindCase::kLet:
                inst_out = CreateInstructionLet(inst_in.let());
                break;
            case pb::Instruction::KindCase::kLoad:
                inst_out = CreateInstructionLoad(inst_in.load());
                break;
            case pb::Instruction::KindCase::kLoadVectorElement:
                inst_out = CreateInstructionLoadVectorElement(inst_in.load_vector_element());
                break;
            case pb::Instruction::KindCase::kLoop:
                inst_out = CreateInstructionLoop(inst_in.loop());
                break;
            case pb::Instruction::KindCase::kNextIteration:
                inst_out = CreateInstructionNextIteration(inst_in.next_iteration());
                break;
            case pb::Instruction::KindCase::kReturn:
                inst_out = CreateInstructionReturn(inst_in.return_());
                break;
            case pb::Instruction::KindCase::kStore:
                inst_out = CreateInstructionStore(inst_in.store());
                break;
            case pb::Instruction::KindCase::kStoreVectorElement:
                inst_out = CreateInstructionStoreVectorElement(inst_in.store_vector_element());
                break;
            case pb::Instruction::KindCase::kSwizzle:
                inst_out = CreateInstructionSwizzle(inst_in.swizzle());
                break;
            case pb::Instruction::KindCase::kSwitch:
                inst_out = CreateInstructionSwitch(inst_in.switch_());
                break;
            case pb::Instruction::KindCase::kUnary:
                inst_out = CreateInstructionUnary(inst_in.unary());
                break;
            case pb::Instruction::KindCase::kUserCall:
                inst_out = CreateInstructionUserCall(inst_in.user_call());
                break;
            case pb::Instruction::KindCase::kVar:
                inst_out = CreateInstructionVar(inst_in.var());
                break;
            case pb::Instruction::KindCase::kUnreachable:
                inst_out = CreateInstructionUnreachable(inst_in.unreachable());
                break;
            case pb::Instruction::KindCase::KIND_NOT_SET:
                break;
        }
        if (!inst_out) {
            err_ << "invalid Instruction.kind: " << std::to_string(inst_in.kind_case()) << "\n";
            return b.Let(mod_out_.Types().invalid());
        }

        TINT_ASSERT(inst_out);

        Vector<ir::Value*, 4> operands;
        for (auto id : inst_in.operands()) {
            operands.Push(Value(id));
        }
        inst_out->SetOperands(std::move(operands));

        Vector<ir::InstructionResult*, 4> results;
        for (auto id : inst_in.results()) {
            results.Push(ValueAs<ir::InstructionResult>(id));
        }
        inst_out->SetResults(std::move(results));

        if (inst_in.has_break_if()) {
            auto num_next_iter_values = inst_in.break_if().num_next_iter_values();
            bool is_valid =
                inst_out->Operands().Length() >= num_next_iter_values + BreakIf::kArgsOperandOffset;
            if (DAWN_LIKELY(is_valid)) {
                static_cast<BreakIf*>(inst_out)->SetNumNextIterValues(
                    inst_in.break_if().num_next_iter_values());
            } else {
                err_ << "invalid value for num_next_iter_values()\n";
            }
        }

        return inst_out;
    }

    ir::Access* CreateInstructionAccess(const pb::InstructionAccess&) {
        return mod_out_.CreateInstruction<ir::Access>();
    }

    ir::CoreBinary* CreateInstructionBinary(const pb::InstructionBinary& binary_in) {
        auto* binary_out = mod_out_.CreateInstruction<ir::CoreBinary>();
        binary_out->SetOp(BinaryOp(binary_in.op()));
        return binary_out;
    }

    ir::Bitcast* CreateInstructionBitcast(const pb::InstructionBitcast&) {
        return mod_out_.CreateInstruction<ir::Bitcast>();
    }

    ir::BreakIf* CreateInstructionBreakIf(const pb::InstructionBreakIf&) {
        auto* break_if_out = mod_out_.CreateInstruction<ir::BreakIf>();
        break_ifs_.Push(break_if_out);
        return break_if_out;
    }

    ir::CoreBuiltinCall* CreateInstructionBuiltinCall(const pb::InstructionBuiltinCall& call_in) {
        auto* call_out = mod_out_.CreateInstruction<ir::CoreBuiltinCall>();
        call_out->SetFunc(BuiltinFn(call_in.builtin()));
        Vector<const core::type::Type*, 1> params;
        for (auto param : call_in.explicit_template_params()) {
            params.Push(Type(param));
        }
        call_out->SetExplicitTemplateParams(params);
        return call_out;
    }

    ir::Construct* CreateInstructionConstruct(const pb::InstructionConstruct&) {
        return mod_out_.CreateInstruction<ir::Construct>();
    }

    ir::Continue* CreateInstructionContinue(const pb::InstructionContinue&) {
        auto* continue_ = mod_out_.CreateInstruction<ir::Continue>();
        continues_.Push(continue_);
        return continue_;
    }

    ir::Convert* CreateInstructionConvert(const pb::InstructionConvert&) {
        return mod_out_.CreateInstruction<ir::Convert>();
    }

    ir::ExitIf* CreateInstructionExitIf(const pb::InstructionExitIf&) {
        auto* exit_out = mod_out_.CreateInstruction<ir::ExitIf>();
        exit_ifs_.Push(exit_out);
        return exit_out;
    }

    ir::ExitLoop* CreateInstructionExitLoop(const pb::InstructionExitLoop&) {
        auto* exit_out = mod_out_.CreateInstruction<ir::ExitLoop>();
        exit_loops_.Push(exit_out);
        return exit_out;
    }

    ir::ExitSwitch* CreateInstructionExitSwitch(const pb::InstructionExitSwitch&) {
        auto* exit_out = mod_out_.CreateInstruction<ir::ExitSwitch>();
        exit_switches_.Push(exit_out);
        return exit_out;
    }

    ir::Discard* CreateInstructionDiscard(const pb::InstructionDiscard&) {
        return mod_out_.CreateInstruction<ir::Discard>();
    }

    ir::If* CreateInstructionIf(const pb::InstructionIf& if_in) {
        auto* if_out = mod_out_.CreateInstruction<ir::If>();
        if_out->SetTrue(if_in.has_true_() ? Block(if_in.true_()) : b.Block());
        if_out->SetFalse(if_in.has_false_() ? Block(if_in.false_()) : b.Block());
        return if_out;
    }

    ir::Let* CreateInstructionLet(const pb::InstructionLet&) {
        return mod_out_.CreateInstruction<ir::Let>();
    }

    ir::Load* CreateInstructionLoad(const pb::InstructionLoad&) {
        return mod_out_.CreateInstruction<ir::Load>();
    }

    ir::LoadVectorElement* CreateInstructionLoadVectorElement(
        const pb::InstructionLoadVectorElement&) {
        return mod_out_.CreateInstruction<ir::LoadVectorElement>();
    }

    ir::Loop* CreateInstructionLoop(const pb::InstructionLoop& loop_in) {
        auto* loop_out = mod_out_.CreateInstruction<ir::Loop>();
        if (loop_in.has_initializer()) {
            loop_out->SetInitializer(Block(loop_in.initializer()));
        } else {
            loop_out->SetInitializer(b.Block());
        }
        loop_out->SetBody(BlockAs<ir::MultiInBlock>(loop_in.body()));
        if (loop_in.has_continuing()) {
            loop_out->SetContinuing(BlockAs<ir::MultiInBlock>(loop_in.continuing()));
        } else {
            loop_out->SetContinuing(b.MultiInBlock());
        }
        return loop_out;
    }

    ir::NextIteration* CreateInstructionNextIteration(const pb::InstructionNextIteration&) {
        auto* next_it_out = mod_out_.CreateInstruction<ir::NextIteration>();
        next_iterations_.Push(next_it_out);
        return next_it_out;
    }

    ir::Return* CreateInstructionReturn(const pb::InstructionReturn&) {
        return mod_out_.CreateInstruction<ir::Return>();
    }

    ir::Store* CreateInstructionStore(const pb::InstructionStore&) {
        return mod_out_.CreateInstruction<ir::Store>();
    }

    ir::StoreVectorElement* CreateInstructionStoreVectorElement(
        const pb::InstructionStoreVectorElement&) {
        return mod_out_.CreateInstruction<ir::StoreVectorElement>();
    }

    ir::Swizzle* CreateInstructionSwizzle(const pb::InstructionSwizzle& swizzle_in) {
        auto* swizzle_out = mod_out_.CreateInstruction<ir::Swizzle>();
        Vector<uint32_t, 4> indices;
        for (auto idx : swizzle_in.indices()) {
            indices.Push(idx);
        }
        swizzle_out->SetIndices(indices);
        return swizzle_out;
    }

    ir::Switch* CreateInstructionSwitch(const pb::InstructionSwitch& switch_in) {
        auto* switch_out = mod_out_.CreateInstruction<ir::Switch>();
        for (auto& case_in : switch_in.cases()) {
            ir::Switch::Case case_out{};
            case_out.block = Block(case_in.block());
            case_out.block->SetParent(switch_out);
            for (auto selector_in : case_in.selectors()) {
                ir::Switch::CaseSelector selector_out{};
                selector_out.val = Constant(selector_in);
                case_out.selectors.Push(std::move(selector_out));
            }
            if (case_in.is_default()) {
                ir::Switch::CaseSelector selector_out{};
                case_out.selectors.Push(std::move(selector_out));
            }
            switch_out->Cases().Push(std::move(case_out));
        }
        return switch_out;
    }

    ir::CoreUnary* CreateInstructionUnary(const pb::InstructionUnary& unary_in) {
        auto* unary_out = mod_out_.CreateInstruction<ir::CoreUnary>();
        unary_out->SetOp(UnaryOp(unary_in.op()));
        return unary_out;
    }

    ir::UserCall* CreateInstructionUserCall(const pb::InstructionUserCall&) {
        return mod_out_.CreateInstruction<ir::UserCall>();
    }

    ir::Var* CreateInstructionVar(const pb::InstructionVar& var_in) {
        auto* var_out = mod_out_.CreateInstruction<ir::Var>();
        if (var_in.has_binding_point()) {
            auto& bp_in = var_in.binding_point();
            var_out->SetBindingPoint(bp_in.group(), bp_in.binding());
        }
        if (var_in.has_input_attachment_index()) {
            var_out->SetInputAttachmentIndex(var_in.input_attachment_index());
        }
        return var_out;
    }

    ir::Unreachable* CreateInstructionUnreachable(const pb::InstructionUnreachable&) {
        return b.Unreachable();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////
    const type::Type* CreateType(const pb::Type type_in) {
        switch (type_in.kind_case()) {
            case pb::Type::KindCase::kBasic:
                return CreateTypeBasic(type_in.basic());
            case pb::Type::KindCase::kVector:
                return CreateTypeVector(type_in.vector());
            case pb::Type::KindCase::kMatrix:
                return CreateTypeMatrix(type_in.matrix());
            case pb::Type::KindCase::kPointer:
                return CreateTypePointer(type_in.pointer());
            case pb::Type::KindCase::kStruct:
                return CreateTypeStruct(type_in.struct_());
            case pb::Type::KindCase::kAtomic:
                return CreateTypeAtomic(type_in.atomic());
            case pb::Type::KindCase::kArray:
                return CreateTypeArray(type_in.array());
            case pb::Type::KindCase::kDepthTexture:
                return CreateTypeDepthTexture(type_in.depth_texture());
            case pb::Type::KindCase::kSampledTexture:
                return CreateTypeSampledTexture(type_in.sampled_texture());
            case pb::Type::KindCase::kMultisampledTexture:
                return CreateTypeMultisampledTexture(type_in.multisampled_texture());
            case pb::Type::KindCase::kDepthMultisampledTexture:
                return CreateTypeDepthMultisampledTexture(type_in.depth_multisampled_texture());
            case pb::Type::KindCase::kStorageTexture:
                return CreateTypeStorageTexture(type_in.storage_texture());
            case pb::Type::KindCase::kExternalTexture:
                return CreateTypeExternalTexture(type_in.external_texture());
            case pb::Type::KindCase::kSampler:
                return CreateTypeSampler(type_in.sampler());
            case pb::Type::KindCase::kInputAttachment:
                return CreateTypeInputAttachment(type_in.input_attachment());
            case pb::Type::KindCase::kSubgroupMatrixLeft:
                return CreateTypeSubgroupMatrix(SubgroupMatrixKind::kLeft,
                                                type_in.subgroup_matrix_left());
            case pb::Type::KindCase::kSubgroupMatrixRight:
                return CreateTypeSubgroupMatrix(SubgroupMatrixKind::kRight,
                                                type_in.subgroup_matrix_right());
            case pb::Type::KindCase::kSubgroupMatrixResult:
                return CreateTypeSubgroupMatrix(SubgroupMatrixKind::kResult,
                                                type_in.subgroup_matrix_result());
            case pb::Type::KindCase::kBuiltinStruct:
                return CreateTypeBuiltinStruct(type_in.builtin_struct());
            case pb::Type::KindCase::KIND_NOT_SET:
                break;
        }

        err_ << "invalid Type.kind: " << std::to_string(type_in.kind_case()) << "\n";
        return mod_out_.Types().invalid();
    }

    const type::Type* CreateTypeBasic(pb::TypeBasic basic_in) {
        switch (basic_in) {
            case pb::TypeBasic::void_:
                return mod_out_.Types().Get<void>();
            case pb::TypeBasic::bool_:
                return mod_out_.Types().Get<bool>();
            case pb::TypeBasic::i32:
                return mod_out_.Types().Get<i32>();
            case pb::TypeBasic::u32:
                return mod_out_.Types().Get<u32>();
            case pb::TypeBasic::f32:
                return mod_out_.Types().Get<f32>();
            case pb::TypeBasic::f16:
                return mod_out_.Types().Get<f16>();

            case pb::TypeBasic::TypeBasic_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::TypeBasic::TypeBasic_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }

        err_ << "invalid TypeBasic: " << std::to_string(basic_in) << "\n";
        return mod_out_.Types().invalid();
    }

    const type::Type* CreateTypeVector(const pb::TypeVector& vector_in) {
        const auto width = vector_in.width();
        if (DAWN_UNLIKELY(width < 2 || width > 4)) {
            err_ << "invalid vector width\n";
            return mod_out_.Types().invalid();
        }
        auto* el_ty = Type(vector_in.element_type());
        return mod_out_.Types().vec(el_ty, vector_in.width());
    }

    const type::Type* CreateTypeMatrix(const pb::TypeMatrix& matrix_in) {
        const auto rows = matrix_in.num_rows();
        const auto cols = matrix_in.num_columns();
        if (DAWN_UNLIKELY(rows < 2 || rows > 4 || cols < 2 || cols > 4)) {
            err_ << "invalid matrix dimensions\n";
            return mod_out_.Types().invalid();
        }
        auto* el_ty = Type(matrix_in.element_type());
        auto* column_ty = mod_out_.Types().vec(el_ty, matrix_in.num_rows());
        return mod_out_.Types().mat(column_ty, matrix_in.num_columns());
    }

    const type::Pointer* CreateTypePointer(const pb::TypePointer& pointer_in) {
        auto address_space = AddressSpace(pointer_in.address_space());
        auto* store_ty = Type(pointer_in.store_type());
        auto access = AccessControl(pointer_in.access());
        return mod_out_.Types().ptr(address_space, store_ty, access);
    }

    const type::Type* CreateTypeStruct(const pb::TypeStruct& struct_in) {
        auto struct_name = struct_in.name();
        if (DAWN_UNLIKELY(struct_name.empty())) {
            err_ << "struct must have a name\n";
            return mod_out_.Types().invalid();
        }

        if (DAWN_UNLIKELY(struct_name.find('\0') != std::string::npos)) {
            err_ << "structure name '" << struct_name
                 << "' contains '\\0' before end of the string\n";
            return mod_out_.Types().invalid();
        }

        if (!struct_names_.Add(struct_name)) {
            err_ << "duplicate struct name: " << struct_name << "\n";
            return mod_out_.Types().invalid();
        }

        Vector<const core::type::StructMember*, 8> members_out;
        uint32_t offset = 0;
        for (auto& member_in : struct_in.member()) {
            auto member_name = member_in.name();
            if (DAWN_UNLIKELY(member_name.empty())) {
                err_ << "struct member must have a name\n";
                return mod_out_.Types().invalid();
            }

            if (DAWN_UNLIKELY(member_name.find('\0') != std::string::npos)) {
                err_ << "member name '" << member_name
                     << "' contains '\\0' before end of the string\n";
                return mod_out_.Types().invalid();
            }

            auto symbol = mod_out_.symbols.Register(member_name);
            auto* type = Type(member_in.type());
            auto index = static_cast<uint32_t>(members_out.Length());
            auto align = member_in.align();
            auto size = member_in.size();
            if (DAWN_UNLIKELY(align == 0)) {
                err_ << "struct member must have non-zero alignment\n";
                align = 1;
            }
            if (DAWN_UNLIKELY(size == 0)) {
                err_ << "struct member must have non-zero size\n";
                size = 1;
            }
            core::IOAttributes attributes_out{};
            if (member_in.has_attributes()) {
                auto& attributes_in = member_in.attributes();
                if (attributes_in.has_location()) {
                    attributes_out.location = attributes_in.location();
                }
                if (attributes_in.has_blend_src()) {
                    attributes_out.blend_src = attributes_in.blend_src();
                }
                if (attributes_in.has_color()) {
                    attributes_out.color = attributes_in.color();
                }
                if (attributes_in.has_builtin()) {
                    attributes_out.builtin = BuiltinValue(attributes_in.builtin());
                }
                if (attributes_in.has_interpolation()) {
                    auto& interpolation_in = attributes_in.interpolation();
                    attributes_out.interpolation = Interpolation(interpolation_in);
                }
                attributes_out.invariant = attributes_in.invariant();
            }
            offset = RoundUp(align, offset);
            auto* member_out = mod_out_.Types().Get<core::type::StructMember>(
                symbol, type, index, offset, align, size, std::move(attributes_out));
            offset += size;
            members_out.Push(member_out);
        }
        if (DAWN_UNLIKELY(members_out.IsEmpty())) {
            err_ << "struct requires at least one member\n";
            return mod_out_.Types().invalid();
        }
        auto name = mod_out_.symbols.Register(struct_name);
        return mod_out_.Types().Struct(name, std::move(members_out));
    }

    const type::Atomic* CreateTypeAtomic(const pb::TypeAtomic& atomic_in) {
        return mod_out_.Types().atomic(Type(atomic_in.type()));
    }

    const type::Type* CreateTypeArray(const pb::TypeArray& array_in) {
        auto* element = Type(array_in.element());
        uint32_t stride = array_in.stride();
        uint32_t count = array_in.count();
        if (element->Align() == 0 || element->Size() == 0) {
            err_ << "cannot create an array of an unsized type\n";
            return mod_out_.Types().invalid();
        }
        uint32_t implicit_stride = tint::RoundUp(element->Align(), element->Size());
        if (stride < implicit_stride) {
            err_ << "array element stride is smaller than the implicit stride\n";
            return mod_out_.Types().invalid();
        }
        if (count >= internal_limits::kMaxArrayElementCount) {
            err_ << "array count (" << count << ") must be less than "
                 << internal_limits::kMaxArrayElementCount << "\n";
            return mod_out_.Types().invalid();
        }

        return count > 0 ? mod_out_.Types().array(element, count, stride)
                         : mod_out_.Types().runtime_array(element, stride);
    }

    const type::Type* CreateTypeDepthTexture(const pb::TypeDepthTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        if (!type::DepthTexture::IsValidDimension(dimension)) {
            err_ << "invalid DepthTexture dimension\n";
            return mod_out_.Types().invalid();
        }
        return mod_out_.Types().Get<type::DepthTexture>(dimension);
    }

    const type::SampledTexture* CreateTypeSampledTexture(const pb::TypeSampledTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        auto sub_type = Type(texture_in.sub_type());
        return mod_out_.Types().Get<type::SampledTexture>(dimension, sub_type);
    }

    const type::MultisampledTexture* CreateTypeMultisampledTexture(
        const pb::TypeMultisampledTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        auto sub_type = Type(texture_in.sub_type());
        return mod_out_.Types().Get<type::MultisampledTexture>(dimension, sub_type);
    }

    const type::Type* CreateTypeDepthMultisampledTexture(
        const pb::TypeDepthMultisampledTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        if (!type::DepthMultisampledTexture::IsValidDimension(dimension)) {
            err_ << "invalid DepthMultisampledTexture dimension\n";
            return mod_out_.Types().invalid();
        }
        return mod_out_.Types().Get<type::DepthMultisampledTexture>(dimension);
    }

    const type::StorageTexture* CreateTypeStorageTexture(const pb::TypeStorageTexture& texture_in) {
        auto dimension = TextureDimension(texture_in.dimension());
        auto texel_format = TexelFormat(texture_in.texel_format());
        auto access = AccessControl(texture_in.access());
        return mod_out_.Types().Get<type::StorageTexture>(
            dimension, texel_format, access,
            type::StorageTexture::SubtypeFor(texel_format, b.ir.Types()));
    }

    const type::ExternalTexture* CreateTypeExternalTexture(const pb::TypeExternalTexture&) {
        return mod_out_.Types().Get<type::ExternalTexture>();
    }

    const type::Sampler* CreateTypeSampler(const pb::TypeSampler& sampler_in) {
        auto kind = SamplerKind(sampler_in.kind());
        return mod_out_.Types().Get<type::Sampler>(kind);
    }

    const type::InputAttachment* CreateTypeInputAttachment(
        const pb::TypeInputAttachment& input_in) {
        auto sub_type = Type(input_in.sub_type());
        return mod_out_.Types().Get<type::InputAttachment>(sub_type);
    }

    const type::SubgroupMatrix* CreateTypeSubgroupMatrix(
        SubgroupMatrixKind kind,
        const pb::TypeSubgroupMatrix& subgroup_matrix) {
        return mod_out_.Types().Get<type::SubgroupMatrix>(kind, Type(subgroup_matrix.sub_type()),
                                                          subgroup_matrix.columns(),
                                                          subgroup_matrix.rows());
    }

    const type::Type* CreateTypeBuiltinStruct(pb::TypeBuiltinStruct builtin_struct_in) {
        auto& ty = mod_out_.Types();
        switch (builtin_struct_in) {
            case pb::TypeBuiltinStruct::AtomicCompareExchangeResultI32:
                return type::CreateAtomicCompareExchangeResult(ty, mod_out_.symbols, ty.i32());
            case pb::TypeBuiltinStruct::AtomicCompareExchangeResultU32:
                return type::CreateAtomicCompareExchangeResult(ty, mod_out_.symbols, ty.u32());
            case pb::TypeBuiltinStruct::FrexpResultF16:
                return type::CreateFrexpResult(ty, mod_out_.symbols, ty.f16());
            case pb::TypeBuiltinStruct::FrexpResultF32:
                return type::CreateFrexpResult(ty, mod_out_.symbols, ty.f32());
            case pb::TypeBuiltinStruct::FrexpResultVec2F16:
                return type::CreateFrexpResult(ty, mod_out_.symbols, ty.vec2<f16>());
            case pb::TypeBuiltinStruct::FrexpResultVec2F32:
                return type::CreateFrexpResult(ty, mod_out_.symbols, ty.vec2<f32>());
            case pb::TypeBuiltinStruct::FrexpResultVec3F16:
                return type::CreateFrexpResult(ty, mod_out_.symbols, ty.vec3<f16>());
            case pb::TypeBuiltinStruct::FrexpResultVec3F32:
                return type::CreateFrexpResult(ty, mod_out_.symbols, ty.vec3<f32>());
            case pb::TypeBuiltinStruct::FrexpResultVec4F16:
                return type::CreateFrexpResult(ty, mod_out_.symbols, ty.vec4<f16>());
            case pb::TypeBuiltinStruct::FrexpResultVec4F32:
                return type::CreateFrexpResult(ty, mod_out_.symbols, ty.vec4<f32>());
            case pb::TypeBuiltinStruct::ModfResultF16:
                return type::CreateModfResult(ty, mod_out_.symbols, ty.f16());
            case pb::TypeBuiltinStruct::ModfResultF32:
                return type::CreateModfResult(ty, mod_out_.symbols, ty.f32());
            case pb::TypeBuiltinStruct::ModfResultVec2F16:
                return type::CreateModfResult(ty, mod_out_.symbols, ty.vec2<f16>());
            case pb::TypeBuiltinStruct::ModfResultVec2F32:
                return type::CreateModfResult(ty, mod_out_.symbols, ty.vec2<f32>());
            case pb::TypeBuiltinStruct::ModfResultVec3F16:
                return type::CreateModfResult(ty, mod_out_.symbols, ty.vec2<f16>());
            case pb::TypeBuiltinStruct::ModfResultVec3F32:
                return type::CreateModfResult(ty, mod_out_.symbols, ty.vec3<f32>());
            case pb::TypeBuiltinStruct::ModfResultVec4F16:
                return type::CreateModfResult(ty, mod_out_.symbols, ty.vec2<f16>());
            case pb::TypeBuiltinStruct::ModfResultVec4F32:
                return type::CreateModfResult(ty, mod_out_.symbols, ty.vec4<f32>());

            case pb::TypeBuiltinStruct::TypeBuiltinStruct_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::TypeBuiltinStruct::TypeBuiltinStruct_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }

        err_ << "invalid TypeBuiltinStruct: " << std::to_string(builtin_struct_in) << "\n";
        return mod_out_.Types().invalid();
    }

    const type::Type* Type(size_t id) {
        if (DAWN_UNLIKELY(id >= types_.Length())) {
            err_ << "type id " << id << " out of range\n";
            return mod_out_.Types().invalid();
        }
        return types_[id];
    }

    ////////////////////////////////////////////////////////////////////////////
    // Values
    ////////////////////////////////////////////////////////////////////////////
    ir::Value* CreateValue(const pb::Value& value_in) {
        ir::Value* value_out = nullptr;
        switch (value_in.kind_case()) {
            case pb::Value::KindCase::kFunction:
                value_out = Function(value_in.function());
                break;
            case pb::Value::KindCase::kInstructionResult:
                value_out = InstructionResult(value_in.instruction_result());
                break;
            case pb::Value::KindCase::kFunctionParameter:
                value_out = FunctionParameter(value_in.function_parameter());
                break;
            case pb::Value::KindCase::kBlockParameter:
                value_out = BlockParameter(value_in.block_parameter());
                break;
            case pb::Value::KindCase::kConstant:
                value_out = Constant(value_in.constant());
                break;
            case pb::Value::KindCase::KIND_NOT_SET:
                break;
        }

        if (!value_out) {
            err_ << "invalid value kind: " << std::to_string(value_in.kind_case()) << "\n";
            return b.InvalidConstant();
        }

        return value_out;
    }

    ir::InstructionResult* InstructionResult(const pb::InstructionResult& res_in) {
        auto* type = Type(res_in.type());
        auto* res_out = b.InstructionResult(type);
        if (!res_in.name().empty()) {
            if (DAWN_UNLIKELY(res_in.name().find('\0') != std::string::npos)) {
                err_ << "result name '" << res_in.name()
                     << "' contains '\\0' before end of the string\n";
                return nullptr;
            }
            mod_out_.SetName(res_out, res_in.name());
        }
        return res_out;
    }

    ir::FunctionParam* FunctionParameter(const pb::FunctionParameter& param_in) {
        auto* type = Type(param_in.type());
        auto* param_out = b.FunctionParam(type);
        if (!param_in.name().empty()) {
            if (DAWN_UNLIKELY(param_in.name().find('\0') != std::string::npos)) {
                err_ << "param name '" << param_in.name()
                     << "' contains '\\0' before end of the string\n";
                return nullptr;
            }
            mod_out_.SetName(param_out, param_in.name());
        }

        if (param_in.has_attributes()) {
            auto& attrs_in = param_in.attributes();
            if (attrs_in.has_binding_point()) {
                auto& bp_in = attrs_in.binding_point();
                param_out->SetBindingPoint(bp_in.group(), bp_in.binding());
            }
            if (attrs_in.has_location()) {
                param_out->SetLocation(attrs_in.location());
            }
            if (attrs_in.has_color()) {
                param_out->SetColor(attrs_in.color());
            }
            if (attrs_in.has_interpolation()) {
                param_out->SetInterpolation(Interpolation(attrs_in.interpolation()));
            }
            if (attrs_in.has_builtin()) {
                param_out->SetBuiltin(BuiltinValue(attrs_in.builtin()));
            }
            if (attrs_in.invariant()) {
                param_out->SetInvariant(true);
            }
        }

        return param_out;
    }

    ir::BlockParam* BlockParameter(const pb::BlockParameter& param_in) {
        auto* type = Type(param_in.type());
        auto* param_out = b.BlockParam(type);
        if (!param_in.name().empty()) {
            if (DAWN_UNLIKELY(param_in.name().find('\0') != std::string::npos)) {
                err_ << "param name '" << param_in.name()
                     << "' contains '\\0' before end of the string\n";
                return nullptr;
            }
            mod_out_.SetName(param_out, param_in.name());
        }
        return param_out;
    }

    ir::Constant* Constant(uint32_t value_id) { return b.Constant(ConstantValue(value_id)); }

    ir::Value* Value(uint32_t id) {
        if (DAWN_UNLIKELY(id > values_.Length())) {
            err_ << "value id " << id << " out of range\n";
            return nullptr;
        }
        return id > 0 ? values_[id - 1] : nullptr;
    }

    template <typename T>
    T* ValueAs(uint32_t id) {
        auto* value = Value(id);
        if (auto cast = As<T>(value); DAWN_LIKELY(cast)) {
            return cast;
        }
        err_ << "value " << id << " is " << (value ? value->TypeInfo().name : "<null>")
             << " expected " << TypeInfo::Of<T>().name << "\n";
        return nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////
    // ConstantValues
    ////////////////////////////////////////////////////////////////////////////
    const core::constant::Value* CreateConstantValue(const pb::ConstantValue& value_in) {
        switch (value_in.kind_case()) {
            case pb::ConstantValue::KindCase::kScalar:
                return CreateConstantScalar(value_in.scalar());
            case pb::ConstantValue::KindCase::kComposite:
                return CreateConstantComposite(value_in.composite());
            case pb::ConstantValue::KindCase::kSplat:
                return CreateConstantSplat(value_in.splat());
            case pb::ConstantValue::KindCase::KIND_NOT_SET:
                break;
        }
        err_ << "invalid ConstantValue.kind: " << std::to_string(value_in.kind_case()) << "\n";
        return b.InvalidConstant()->Value();
    }

    const core::constant::Value* CreateConstantScalar(const pb::ConstantValueScalar& value_in) {
        switch (value_in.kind_case()) {
            case pb::ConstantValueScalar::KindCase::kBool:
                return b.ConstantValue(value_in.bool_());
            case pb::ConstantValueScalar::KindCase::kI32:
                return b.ConstantValue(i32(value_in.i32()));
            case pb::ConstantValueScalar::KindCase::kU32:
                return b.ConstantValue(u32(value_in.u32()));
            case pb::ConstantValueScalar::KindCase::kF32:
                return b.ConstantValue(CheckFinite(f32(value_in.f32())));
            case pb::ConstantValueScalar::KindCase::kF16:
                return b.ConstantValue(CheckFinite(f16(value_in.f16())));
            case pb::ConstantValueScalar::KindCase::KIND_NOT_SET:
                break;
        }
        err_ << "invalid ConstantValueScalar.kind: " << std::to_string(value_in.kind_case())
             << "\n";
        return b.InvalidConstant()->Value();
    }

    const core::constant::Value* CreateConstantComposite(
        const pb::ConstantValueComposite& composite_in) {
        auto* type = Type(composite_in.type());
        auto type_elements = type->Elements();
        size_t num_values = static_cast<size_t>(composite_in.elements().size());
        if (DAWN_UNLIKELY(type_elements.count == 0)) {
            err_ << "cannot create a composite of type " << type->FriendlyName() << "\n";
            return b.InvalidConstant()->Value();
        }
        if (DAWN_UNLIKELY(type_elements.count != num_values)) {
            err_ << "constant composite type " << type->FriendlyName() << " expects "
                 << type_elements.count << " elements, but " << num_values << " values encoded\n";
            return b.InvalidConstant()->Value();
        }
        Vector<const core::constant::Value*, 8> elements_out;
        for (auto element_id : composite_in.elements()) {
            uint32_t i = static_cast<uint32_t>(elements_out.Length());
            auto* value = ConstantValue(element_id);
            if (auto* el_type = type->Element(i); DAWN_UNLIKELY(value->Type() != el_type)) {
                err_ << "constant composite element value type " << value->Type()->FriendlyName()
                     << " does not match element type " << el_type->FriendlyName() << "\n";
                return b.InvalidConstant()->Value();
            }
            elements_out.Push(value);
        }
        return mod_out_.constant_values.Composite(type, std::move(elements_out));
    }

    const core::constant::Value* CreateConstantSplat(const pb::ConstantValueSplat& splat_in) {
        auto* type = Type(splat_in.type());
        uint32_t num_elements = type->Elements().count;
        if (DAWN_UNLIKELY(num_elements == 0)) {
            err_ << "cannot create a splat of type " << type->FriendlyName() << "\n";
            return b.InvalidConstant()->Value();
        }
        if (DAWN_UNLIKELY(num_elements > internal_limits::kMaxArrayConstructorElements)) {
            err_ << "array constructor has excessive number of elements (>"
                 << internal_limits::kMaxArrayConstructorElements << ")\n";
            return b.InvalidConstant()->Value();
        }
        auto* value = ConstantValue(splat_in.elements());
        for (uint32_t i = 0; i < num_elements; i++) {
            auto* el_type = type->Element(i);
            if (DAWN_UNLIKELY(el_type != value->Type())) {
                err_ << "constant splat element value type " << value->Type()->FriendlyName()
                     << " does not match element " << i << " type " << el_type->FriendlyName()
                     << "\n";
                return b.InvalidConstant()->Value();
            }
        }
        return mod_out_.constant_values.Splat(type, value);
    }

    const core::constant::Value* ConstantValue(uint32_t id) {
        if (DAWN_UNLIKELY(id >= constant_values_.Length())) {
            err_ << "constant value id " << id << " out of range\n";
            return b.InvalidConstant()->Value();
        }
        return constant_values_[id];
    }

    ////////////////////////////////////////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////////////////////////////////////////
    core::Interpolation Interpolation(const pb::Interpolation& interpolation_in) {
        core::Interpolation interpolation_out{};
        interpolation_out.type = InterpolationType(interpolation_in.type());
        if (interpolation_in.has_sampling()) {
            interpolation_out.sampling = InterpolationSampling(interpolation_in.sampling());
        }
        return interpolation_out;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Enums
    ////////////////////////////////////////////////////////////////////////////
    core::AddressSpace AddressSpace(pb::AddressSpace in) {
        switch (in) {
            case pb::AddressSpace::function:
                return core::AddressSpace::kFunction;
            case pb::AddressSpace::handle:
                return core::AddressSpace::kHandle;
            case pb::AddressSpace::pixel_local:
                return core::AddressSpace::kPixelLocal;
            case pb::AddressSpace::private_:
                return core::AddressSpace::kPrivate;
            case pb::AddressSpace::push_constant:
                return core::AddressSpace::kPushConstant;
            case pb::AddressSpace::storage:
                return core::AddressSpace::kStorage;
            case pb::AddressSpace::uniform:
                return core::AddressSpace::kUniform;
            case pb::AddressSpace::workgroup:
                return core::AddressSpace::kWorkgroup;

            case pb::AddressSpace::AddressSpace_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::AddressSpace::AddressSpace_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        TINT_ICE() << "invalid AddressSpace: " << in;
    }

    core::Access AccessControl(pb::AccessControl in) {
        switch (in) {
            case pb::AccessControl::read:
                return core::Access::kRead;
            case pb::AccessControl::write:
                return core::Access::kWrite;
            case pb::AccessControl::read_write:
                return core::Access::kReadWrite;

            case pb::AccessControl::AccessControl_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::AccessControl::AccessControl_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        TINT_ICE() << "invalid Access: " << in;
    }

    core::UnaryOp UnaryOp(pb::UnaryOp in) {
        switch (in) {
            case pb::UnaryOp::complement:
                return core::UnaryOp::kComplement;
            case pb::UnaryOp::negation:
                return core::UnaryOp::kNegation;
            case pb::UnaryOp::address_of:
                return core::UnaryOp::kAddressOf;
            case pb::UnaryOp::indirection:
                return core::UnaryOp::kIndirection;
            case pb::UnaryOp::not_:
                return core::UnaryOp::kNot;

            case pb::UnaryOp::UnaryOp_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::UnaryOp::UnaryOp_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        TINT_ICE() << "invalid UnaryOp: " << in;
    }

    core::BinaryOp BinaryOp(pb::BinaryOp in) {
        switch (in) {
            case pb::BinaryOp::add_:
                return core::BinaryOp::kAdd;
            case pb::BinaryOp::subtract:
                return core::BinaryOp::kSubtract;
            case pb::BinaryOp::multiply:
                return core::BinaryOp::kMultiply;
            case pb::BinaryOp::divide:
                return core::BinaryOp::kDivide;
            case pb::BinaryOp::modulo:
                return core::BinaryOp::kModulo;
            case pb::BinaryOp::and_:
                return core::BinaryOp::kAnd;
            case pb::BinaryOp::or_:
                return core::BinaryOp::kOr;
            case pb::BinaryOp::xor_:
                return core::BinaryOp::kXor;
            case pb::BinaryOp::equal:
                return core::BinaryOp::kEqual;
            case pb::BinaryOp::not_equal:
                return core::BinaryOp::kNotEqual;
            case pb::BinaryOp::less_than:
                return core::BinaryOp::kLessThan;
            case pb::BinaryOp::greater_than:
                return core::BinaryOp::kGreaterThan;
            case pb::BinaryOp::less_than_equal:
                return core::BinaryOp::kLessThanEqual;
            case pb::BinaryOp::greater_than_equal:
                return core::BinaryOp::kGreaterThanEqual;
            case pb::BinaryOp::shift_left:
                return core::BinaryOp::kShiftLeft;
            case pb::BinaryOp::shift_right:
                return core::BinaryOp::kShiftRight;
            case pb::BinaryOp::logical_and:
                return core::BinaryOp::kLogicalAnd;
            case pb::BinaryOp::logical_or:
                return core::BinaryOp::kLogicalOr;

            case pb::BinaryOp::BinaryOp_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::BinaryOp::BinaryOp_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        TINT_ICE() << "invalid BinaryOp: " << in;
    }

    core::type::TextureDimension TextureDimension(pb::TextureDimension in) {
        switch (in) {
            case pb::TextureDimension::_1d:
                return core::type::TextureDimension::k1d;
            case pb::TextureDimension::_2d:
                return core::type::TextureDimension::k2d;
            case pb::TextureDimension::_2d_array:
                return core::type::TextureDimension::k2dArray;
            case pb::TextureDimension::_3d:
                return core::type::TextureDimension::k3d;
            case pb::TextureDimension::cube:
                return core::type::TextureDimension::kCube;
            case pb::TextureDimension::cube_array:
                return core::type::TextureDimension::kCubeArray;

            case pb::TextureDimension::TextureDimension_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::TextureDimension::TextureDimension_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }

        TINT_ICE() << "invalid TextureDimension: " << in;
    }

    core::TexelFormat TexelFormat(pb::TexelFormat in) {
        switch (in) {
            case pb::TexelFormat::bgra8_unorm:
                return core::TexelFormat::kBgra8Unorm;
            case pb::TexelFormat::r8_unorm:
                return core::TexelFormat::kR8Unorm;
            case pb::TexelFormat::r32_float:
                return core::TexelFormat::kR32Float;
            case pb::TexelFormat::r32_sint:
                return core::TexelFormat::kR32Sint;
            case pb::TexelFormat::r32_uint:
                return core::TexelFormat::kR32Uint;
            case pb::TexelFormat::rg32_float:
                return core::TexelFormat::kRg32Float;
            case pb::TexelFormat::rg32_sint:
                return core::TexelFormat::kRg32Sint;
            case pb::TexelFormat::rg32_uint:
                return core::TexelFormat::kRg32Uint;
            case pb::TexelFormat::rgba16_float:
                return core::TexelFormat::kRgba16Float;
            case pb::TexelFormat::rgba16_sint:
                return core::TexelFormat::kRgba16Sint;
            case pb::TexelFormat::rgba16_uint:
                return core::TexelFormat::kRgba16Uint;
            case pb::TexelFormat::rgba32_float:
                return core::TexelFormat::kRgba32Float;
            case pb::TexelFormat::rgba32_sint:
                return core::TexelFormat::kRgba32Sint;
            case pb::TexelFormat::rgba32_uint:
                return core::TexelFormat::kRgba32Uint;
            case pb::TexelFormat::rgba8_sint:
                return core::TexelFormat::kRgba8Sint;
            case pb::TexelFormat::rgba8_snorm:
                return core::TexelFormat::kRgba8Snorm;
            case pb::TexelFormat::rgba8_uint:
                return core::TexelFormat::kRgba8Uint;
            case pb::TexelFormat::rgba8_unorm:
                return core::TexelFormat::kRgba8Unorm;

            case pb::TexelFormat::TexelFormat_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::TexelFormat::TexelFormat_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }

        TINT_ICE() << "invalid TexelFormat: " << in;
    }

    core::type::SamplerKind SamplerKind(pb::SamplerKind in) {
        switch (in) {
            case pb::SamplerKind::sampler:
                return core::type::SamplerKind::kSampler;
            case pb::SamplerKind::comparison:
                return core::type::SamplerKind::kComparisonSampler;

            case pb::SamplerKind::SamplerKind_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::SamplerKind::SamplerKind_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }

        TINT_ICE() << "invalid SamplerKind: " << in;
    }

    core::InterpolationType InterpolationType(pb::InterpolationType in) {
        switch (in) {
            case pb::InterpolationType::flat:
                return core::InterpolationType::kFlat;
            case pb::InterpolationType::linear:
                return core::InterpolationType::kLinear;
            case pb::InterpolationType::perspective:
                return core::InterpolationType::kPerspective;

            case pb::InterpolationType::InterpolationType_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::InterpolationType::InterpolationType_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        TINT_ICE() << "invalid InterpolationType: " << in;
    }

    core::InterpolationSampling InterpolationSampling(pb::InterpolationSampling in) {
        switch (in) {
            case pb::InterpolationSampling::center:
                return core::InterpolationSampling::kCenter;
            case pb::InterpolationSampling::centroid:
                return core::InterpolationSampling::kCentroid;
            case pb::InterpolationSampling::sample:
                return core::InterpolationSampling::kSample;
            case pb::InterpolationSampling::first:
                return core::InterpolationSampling::kFirst;
            case pb::InterpolationSampling::either:
                return core::InterpolationSampling::kEither;

            case pb::InterpolationSampling::InterpolationSampling_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::InterpolationSampling::InterpolationSampling_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        TINT_ICE() << "invalid InterpolationSampling: " << in;
    }

    core::BuiltinValue BuiltinValue(pb::BuiltinValue in) {
        switch (in) {
            case pb::BuiltinValue::point_size:
                return core::BuiltinValue::kPointSize;
            case pb::BuiltinValue::cull_distance:
                return core::BuiltinValue::kCullDistance;
            case pb::BuiltinValue::frag_depth:
                return core::BuiltinValue::kFragDepth;
            case pb::BuiltinValue::front_facing:
                return core::BuiltinValue::kFrontFacing;
            case pb::BuiltinValue::global_invocation_id:
                return core::BuiltinValue::kGlobalInvocationId;
            case pb::BuiltinValue::instance_index:
                return core::BuiltinValue::kInstanceIndex;
            case pb::BuiltinValue::local_invocation_id:
                return core::BuiltinValue::kLocalInvocationId;
            case pb::BuiltinValue::local_invocation_index:
                return core::BuiltinValue::kLocalInvocationIndex;
            case pb::BuiltinValue::num_workgroups:
                return core::BuiltinValue::kNumWorkgroups;
            case pb::BuiltinValue::position:
                return core::BuiltinValue::kPosition;
            case pb::BuiltinValue::sample_index:
                return core::BuiltinValue::kSampleIndex;
            case pb::BuiltinValue::sample_mask:
                return core::BuiltinValue::kSampleMask;
            case pb::BuiltinValue::subgroup_invocation_id:
                return core::BuiltinValue::kSubgroupInvocationId;
            case pb::BuiltinValue::subgroup_size:
                return core::BuiltinValue::kSubgroupSize;
            case pb::BuiltinValue::vertex_index:
                return core::BuiltinValue::kVertexIndex;
            case pb::BuiltinValue::workgroup_id:
                return core::BuiltinValue::kWorkgroupId;
            case pb::BuiltinValue::clip_distances:
                return core::BuiltinValue::kClipDistances;
            case pb::BuiltinValue::BuiltinValue_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::BuiltinValue::BuiltinValue_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        TINT_ICE() << "invalid BuiltinValue: " << in;
    }

    core::BuiltinFn BuiltinFn(pb::BuiltinFn in) {
        switch (in) {
            case pb::BuiltinFn::abs:
                return core::BuiltinFn::kAbs;
            case pb::BuiltinFn::acos:
                return core::BuiltinFn::kAcos;
            case pb::BuiltinFn::acosh:
                return core::BuiltinFn::kAcosh;
            case pb::BuiltinFn::all:
                return core::BuiltinFn::kAll;
            case pb::BuiltinFn::any:
                return core::BuiltinFn::kAny;
            case pb::BuiltinFn::array_length:
                return core::BuiltinFn::kArrayLength;
            case pb::BuiltinFn::asin:
                return core::BuiltinFn::kAsin;
            case pb::BuiltinFn::asinh:
                return core::BuiltinFn::kAsinh;
            case pb::BuiltinFn::atan:
                return core::BuiltinFn::kAtan;
            case pb::BuiltinFn::atan2:
                return core::BuiltinFn::kAtan2;
            case pb::BuiltinFn::atanh:
                return core::BuiltinFn::kAtanh;
            case pb::BuiltinFn::ceil:
                return core::BuiltinFn::kCeil;
            case pb::BuiltinFn::clamp:
                return core::BuiltinFn::kClamp;
            case pb::BuiltinFn::cos:
                return core::BuiltinFn::kCos;
            case pb::BuiltinFn::cosh:
                return core::BuiltinFn::kCosh;
            case pb::BuiltinFn::count_leading_zeros:
                return core::BuiltinFn::kCountLeadingZeros;
            case pb::BuiltinFn::count_one_bits:
                return core::BuiltinFn::kCountOneBits;
            case pb::BuiltinFn::count_trailing_zeros:
                return core::BuiltinFn::kCountTrailingZeros;
            case pb::BuiltinFn::cross:
                return core::BuiltinFn::kCross;
            case pb::BuiltinFn::degrees:
                return core::BuiltinFn::kDegrees;
            case pb::BuiltinFn::determinant:
                return core::BuiltinFn::kDeterminant;
            case pb::BuiltinFn::distance:
                return core::BuiltinFn::kDistance;
            case pb::BuiltinFn::dot:
                return core::BuiltinFn::kDot;
            case pb::BuiltinFn::dot4i8_packed:
                return core::BuiltinFn::kDot4I8Packed;
            case pb::BuiltinFn::dot4u8_packed:
                return core::BuiltinFn::kDot4U8Packed;
            case pb::BuiltinFn::dpdx:
                return core::BuiltinFn::kDpdx;
            case pb::BuiltinFn::dpdx_coarse:
                return core::BuiltinFn::kDpdxCoarse;
            case pb::BuiltinFn::dpdx_fine:
                return core::BuiltinFn::kDpdxFine;
            case pb::BuiltinFn::dpdy:
                return core::BuiltinFn::kDpdy;
            case pb::BuiltinFn::dpdy_coarse:
                return core::BuiltinFn::kDpdyCoarse;
            case pb::BuiltinFn::dpdy_fine:
                return core::BuiltinFn::kDpdyFine;
            case pb::BuiltinFn::exp:
                return core::BuiltinFn::kExp;
            case pb::BuiltinFn::exp2:
                return core::BuiltinFn::kExp2;
            case pb::BuiltinFn::extract_bits:
                return core::BuiltinFn::kExtractBits;
            case pb::BuiltinFn::face_forward:
                return core::BuiltinFn::kFaceForward;
            case pb::BuiltinFn::first_leading_bit:
                return core::BuiltinFn::kFirstLeadingBit;
            case pb::BuiltinFn::first_trailing_bit:
                return core::BuiltinFn::kFirstTrailingBit;
            case pb::BuiltinFn::floor:
                return core::BuiltinFn::kFloor;
            case pb::BuiltinFn::fma:
                return core::BuiltinFn::kFma;
            case pb::BuiltinFn::fract:
                return core::BuiltinFn::kFract;
            case pb::BuiltinFn::frexp:
                return core::BuiltinFn::kFrexp;
            case pb::BuiltinFn::fwidth:
                return core::BuiltinFn::kFwidth;
            case pb::BuiltinFn::fwidth_coarse:
                return core::BuiltinFn::kFwidthCoarse;
            case pb::BuiltinFn::fwidth_fine:
                return core::BuiltinFn::kFwidthFine;
            case pb::BuiltinFn::insert_bits:
                return core::BuiltinFn::kInsertBits;
            case pb::BuiltinFn::inverse_sqrt:
                return core::BuiltinFn::kInverseSqrt;
            case pb::BuiltinFn::ldexp:
                return core::BuiltinFn::kLdexp;
            case pb::BuiltinFn::length:
                return core::BuiltinFn::kLength;
            case pb::BuiltinFn::log:
                return core::BuiltinFn::kLog;
            case pb::BuiltinFn::log2:
                return core::BuiltinFn::kLog2;
            case pb::BuiltinFn::max:
                return core::BuiltinFn::kMax;
            case pb::BuiltinFn::min:
                return core::BuiltinFn::kMin;
            case pb::BuiltinFn::mix:
                return core::BuiltinFn::kMix;
            case pb::BuiltinFn::modf:
                return core::BuiltinFn::kModf;
            case pb::BuiltinFn::normalize:
                return core::BuiltinFn::kNormalize;
            case pb::BuiltinFn::pack2x16_float:
                return core::BuiltinFn::kPack2X16Float;
            case pb::BuiltinFn::pack2x16_snorm:
                return core::BuiltinFn::kPack2X16Snorm;
            case pb::BuiltinFn::pack2x16_unorm:
                return core::BuiltinFn::kPack2X16Unorm;
            case pb::BuiltinFn::pack4x8_snorm:
                return core::BuiltinFn::kPack4X8Snorm;
            case pb::BuiltinFn::pack4x8_unorm:
                return core::BuiltinFn::kPack4X8Unorm;
            case pb::BuiltinFn::pack4xi8:
                return core::BuiltinFn::kPack4XI8;
            case pb::BuiltinFn::pack4xu8:
                return core::BuiltinFn::kPack4XU8;
            case pb::BuiltinFn::pack4xi8_clamp:
                return core::BuiltinFn::kPack4XI8Clamp;
            case pb::BuiltinFn::pack4xu8_clamp:
                return core::BuiltinFn::kPack4XU8Clamp;
            case pb::BuiltinFn::pow:
                return core::BuiltinFn::kPow;
            case pb::BuiltinFn::quantize_to_f16:
                return core::BuiltinFn::kQuantizeToF16;
            case pb::BuiltinFn::radians:
                return core::BuiltinFn::kRadians;
            case pb::BuiltinFn::reflect:
                return core::BuiltinFn::kReflect;
            case pb::BuiltinFn::refract:
                return core::BuiltinFn::kRefract;
            case pb::BuiltinFn::reverse_bits:
                return core::BuiltinFn::kReverseBits;
            case pb::BuiltinFn::round:
                return core::BuiltinFn::kRound;
            case pb::BuiltinFn::saturate:
                return core::BuiltinFn::kSaturate;
            case pb::BuiltinFn::select:
                return core::BuiltinFn::kSelect;
            case pb::BuiltinFn::sign:
                return core::BuiltinFn::kSign;
            case pb::BuiltinFn::sin:
                return core::BuiltinFn::kSin;
            case pb::BuiltinFn::sinh:
                return core::BuiltinFn::kSinh;
            case pb::BuiltinFn::smoothstep:
                return core::BuiltinFn::kSmoothstep;
            case pb::BuiltinFn::sqrt:
                return core::BuiltinFn::kSqrt;
            case pb::BuiltinFn::step:
                return core::BuiltinFn::kStep;
            case pb::BuiltinFn::storage_barrier:
                return core::BuiltinFn::kStorageBarrier;
            case pb::BuiltinFn::tan:
                return core::BuiltinFn::kTan;
            case pb::BuiltinFn::tanh:
                return core::BuiltinFn::kTanh;
            case pb::BuiltinFn::transpose:
                return core::BuiltinFn::kTranspose;
            case pb::BuiltinFn::trunc:
                return core::BuiltinFn::kTrunc;
            case pb::BuiltinFn::unpack2x16_float:
                return core::BuiltinFn::kUnpack2X16Float;
            case pb::BuiltinFn::unpack2x16_snorm:
                return core::BuiltinFn::kUnpack2X16Snorm;
            case pb::BuiltinFn::unpack2x16_unorm:
                return core::BuiltinFn::kUnpack2X16Unorm;
            case pb::BuiltinFn::unpack4x8_snorm:
                return core::BuiltinFn::kUnpack4X8Snorm;
            case pb::BuiltinFn::unpack4x8_unorm:
                return core::BuiltinFn::kUnpack4X8Unorm;
            case pb::BuiltinFn::unpack4xi8:
                return core::BuiltinFn::kUnpack4XI8;
            case pb::BuiltinFn::unpack4xu8:
                return core::BuiltinFn::kUnpack4XU8;
            case pb::BuiltinFn::workgroup_barrier:
                return core::BuiltinFn::kWorkgroupBarrier;
            case pb::BuiltinFn::texture_barrier:
                return core::BuiltinFn::kTextureBarrier;
            case pb::BuiltinFn::texture_dimensions:
                return core::BuiltinFn::kTextureDimensions;
            case pb::BuiltinFn::texture_gather:
                return core::BuiltinFn::kTextureGather;
            case pb::BuiltinFn::texture_gather_compare:
                return core::BuiltinFn::kTextureGatherCompare;
            case pb::BuiltinFn::texture_num_layers:
                return core::BuiltinFn::kTextureNumLayers;
            case pb::BuiltinFn::texture_num_levels:
                return core::BuiltinFn::kTextureNumLevels;
            case pb::BuiltinFn::texture_num_samples:
                return core::BuiltinFn::kTextureNumSamples;
            case pb::BuiltinFn::texture_sample:
                return core::BuiltinFn::kTextureSample;
            case pb::BuiltinFn::texture_sample_bias:
                return core::BuiltinFn::kTextureSampleBias;
            case pb::BuiltinFn::texture_sample_compare:
                return core::BuiltinFn::kTextureSampleCompare;
            case pb::BuiltinFn::texture_sample_compare_level:
                return core::BuiltinFn::kTextureSampleCompareLevel;
            case pb::BuiltinFn::texture_sample_grad:
                return core::BuiltinFn::kTextureSampleGrad;
            case pb::BuiltinFn::texture_sample_level:
                return core::BuiltinFn::kTextureSampleLevel;
            case pb::BuiltinFn::texture_sample_base_clamp_to_edge:
                return core::BuiltinFn::kTextureSampleBaseClampToEdge;
            case pb::BuiltinFn::texture_store:
                return core::BuiltinFn::kTextureStore;
            case pb::BuiltinFn::texture_load:
                return core::BuiltinFn::kTextureLoad;
            case pb::BuiltinFn::atomic_load:
                return core::BuiltinFn::kAtomicLoad;
            case pb::BuiltinFn::atomic_store:
                return core::BuiltinFn::kAtomicStore;
            case pb::BuiltinFn::atomic_add:
                return core::BuiltinFn::kAtomicAdd;
            case pb::BuiltinFn::atomic_sub:
                return core::BuiltinFn::kAtomicSub;
            case pb::BuiltinFn::atomic_max:
                return core::BuiltinFn::kAtomicMax;
            case pb::BuiltinFn::atomic_min:
                return core::BuiltinFn::kAtomicMin;
            case pb::BuiltinFn::atomic_and:
                return core::BuiltinFn::kAtomicAnd;
            case pb::BuiltinFn::atomic_or:
                return core::BuiltinFn::kAtomicOr;
            case pb::BuiltinFn::atomic_xor:
                return core::BuiltinFn::kAtomicXor;
            case pb::BuiltinFn::atomic_exchange:
                return core::BuiltinFn::kAtomicExchange;
            case pb::BuiltinFn::atomic_compare_exchange_weak:
                return core::BuiltinFn::kAtomicCompareExchangeWeak;
            case pb::BuiltinFn::subgroup_ballot:
                return core::BuiltinFn::kSubgroupBallot;
            case pb::BuiltinFn::subgroup_elect:
                return core::BuiltinFn::kSubgroupElect;
            case pb::BuiltinFn::subgroup_broadcast:
                return core::BuiltinFn::kSubgroupBroadcast;
            case pb::BuiltinFn::subgroup_broadcast_first:
                return core::BuiltinFn::kSubgroupBroadcastFirst;
            case pb::BuiltinFn::subgroup_shuffle:
                return core::BuiltinFn::kSubgroupShuffle;
            case pb::BuiltinFn::subgroup_shuffle_xor:
                return core::BuiltinFn::kSubgroupShuffleXor;
            case pb::BuiltinFn::subgroup_shuffle_up:
                return core::BuiltinFn::kSubgroupShuffleUp;
            case pb::BuiltinFn::subgroup_shuffle_down:
                return core::BuiltinFn::kSubgroupShuffleDown;
            case pb::BuiltinFn::input_attachment_load:
                return core::BuiltinFn::kInputAttachmentLoad;
            case pb::BuiltinFn::subgroup_add:
                return core::BuiltinFn::kSubgroupAdd;
            case pb::BuiltinFn::subgroup_inclusive_add:
                return core::BuiltinFn::kSubgroupInclusiveAdd;
            case pb::BuiltinFn::subgroup_exclusive_add:
                return core::BuiltinFn::kSubgroupExclusiveAdd;
            case pb::BuiltinFn::subgroup_mul:
                return core::BuiltinFn::kSubgroupMul;
            case pb::BuiltinFn::subgroup_inclusive_mul:
                return core::BuiltinFn::kSubgroupInclusiveMul;
            case pb::BuiltinFn::subgroup_exclusive_mul:
                return core::BuiltinFn::kSubgroupExclusiveMul;
            case pb::BuiltinFn::subgroup_and:
                return core::BuiltinFn::kSubgroupAnd;
            case pb::BuiltinFn::subgroup_or:
                return core::BuiltinFn::kSubgroupOr;
            case pb::BuiltinFn::subgroup_xor:
                return core::BuiltinFn::kSubgroupXor;
            case pb::BuiltinFn::subgroup_min:
                return core::BuiltinFn::kSubgroupMin;
            case pb::BuiltinFn::subgroup_max:
                return core::BuiltinFn::kSubgroupMax;
            case pb::BuiltinFn::subgroup_all:
                return core::BuiltinFn::kSubgroupAll;
            case pb::BuiltinFn::subgroup_any:
                return core::BuiltinFn::kSubgroupAny;
            case pb::BuiltinFn::quad_broadcast:
                return core::BuiltinFn::kQuadBroadcast;
            case pb::BuiltinFn::quad_swap_x:
                return core::BuiltinFn::kQuadSwapX;
            case pb::BuiltinFn::quad_swap_y:
                return core::BuiltinFn::kQuadSwapY;
            case pb::BuiltinFn::quad_swap_diagonal:
                return core::BuiltinFn::kQuadSwapDiagonal;
            case pb::BuiltinFn::subgroup_matrix_load:
                return core::BuiltinFn::kSubgroupMatrixLoad;
            case pb::BuiltinFn::subgroup_matrix_store:
                return core::BuiltinFn::kSubgroupMatrixStore;
            case pb::BuiltinFn::subgroup_matrix_multiply:
                return core::BuiltinFn::kSubgroupMatrixMultiply;
            case pb::BuiltinFn::subgroup_matrix_multiply_accumulate:
                return core::BuiltinFn::kSubgroupMatrixMultiply;

            case pb::BuiltinFn::BuiltinFn_INT_MIN_SENTINEL_DO_NOT_USE_:
            case pb::BuiltinFn::BuiltinFn_INT_MAX_SENTINEL_DO_NOT_USE_:
                break;
        }
        TINT_ICE() << "invalid BuiltinFn: " << in;
    }
};

}  // namespace

Result<Module> Decode(Slice<const std::byte> encoded) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    pb::Module mod_in;
    if (!mod_in.ParseFromArray(encoded.data, static_cast<int>(encoded.len))) {
        return Failure{"failed to deserialize protobuf"};
    }

    return Decode(mod_in);
}

Result<Module> Decode(const pb::Module& mod_in) {
    return Decoder{mod_in}.Decode();
}

}  // namespace tint::core::ir::binary
