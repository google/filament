// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_FUNCTIONAL_VALIDATOR_H_
#define SRC_TINT_LANG_CORE_IR_FUNCTIONAL_VALIDATOR_H_

#include <string>

#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/builtin_call.h"
#include "src/tint/lang/core/ir/call.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/exit.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/member_builtin_call.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::core::ir::validator {

class Functional {
  public:
    enum class ErrorSource {
        kWgsl,
        kIr,
    };

    Functional(const Module& ir, diag::List& diagnostics, ErrorSource error_source);
    ~Functional();

    void Validate();

    Functional(const Functional&) = delete;
    Functional(Functional&&) = delete;
    Functional& operator=(const Functional&) = delete;
    Functional& operator=(Functional&&) = delete;

  private:
    using SourceHelper = std::function<Source()>;

    /// ScopeStack holds a stack of values that are currently in scope
    struct ScopeStack {
        void Push() { stack_.Push({}); }
        void Pop() { stack_.Pop(); }
        void Add(const Value* value) { stack_.Back().Add(value); }
        bool Contains(const Value* value) {
            return stack_.Any([&](auto& v) { return v.Contains(value); });
        }
        bool IsEmpty() const { return stack_.IsEmpty(); }

      private:
        Vector<Hashset<const Value*, 8>, 4> stack_;
    };

    StyledText NameOf(const core::type::Type* ty);
    StyledText NameOf(const Value* value);

    ///
    /// Calculates the total number elements contained in a type, i.e. the number of values required
    /// for an initializer.
    /// @param ty the type to calculate elements for
    /// @returns the count of elements in the type and all of its children
    uint64_t ElementsCount(const core::type::Type* ty);

    Source SourceOf(const Function* func);
    Source SourceOf(const FunctionParam* param);
    Source SourceOf(const Instruction* inst);
    Source SourceOf(const Instruction* inst, size_t idx);

    diag::Diagnostic& AddError(Source src);
    diag::Diagnostic& AddError(const Function* func);
    diag::Diagnostic& AddError(const FunctionParam* param);
    diag::Diagnostic& AddError(const Instruction* inst);
    diag::Diagnostic& AddError(const Instruction* inst, size_t idx);

    diag::Diagnostic& AddNote(Source src);
    diag::Diagnostic& AddNote(const Block* blk);
    diag::Diagnostic& AddNote(const Function* func);
    diag::Diagnostic& AddNote(const Instruction* inst);
    diag::Diagnostic& AddNote(const Instruction* inst, size_t idx);

    ir::Disassembler& Disassemble();

    bool CanLoad(const core::type::Type* ty);
    const core::type::Type* GetVectorPtrElementType(const Instruction* inst, size_t idx);

    void CheckRootBlock(const Block* blk);
    void CheckFunction(const Function* func);
    void CheckFunctionParam(const FunctionParam* param);
    void CheckEntryPoint(const Function* func);
    void CheckPositionPresentForVertexOutput(const Function* ep);
    void CheckBlock(const Block* blk);
    void CheckInstruction(const Instruction* inst);

    // Validates the alignment of the given instruction
    /// @param inst the instruction to validate
    void CheckAlignment(const Instruction* inst);

    void CheckAccess(const Access* a);
    void CheckBinary(const Binary* b);
    void CheckBreakIf(const BreakIf* b);
    void CheckBuiltinCall(const BuiltinCall* call);
    void CheckCall(const Call* call);
    void CheckConstruct(const Construct* construct);
    void CheckContinue(const Continue* c);
    void CheckConvert(const Convert* convert);
    void CheckCoreBuiltinCall(const CoreBuiltinCall* call,
                              const core::intrinsic::Overload& overload);
    void CheckExit(const Exit* e);
    void CheckExitLoop(const ExitLoop* l);
    void CheckIf(const If* if_);
    void CheckLet(const Let* l);
    void CheckLoad(const Load* l);
    void CheckLoadVectorElement(const LoadVectorElement* l);
    void CheckLoop(const Loop* l);
    void CheckLoopBody(const Loop* loop);
    void CheckLoopContinuing(const Loop* loop);
    void CheckMemberBuiltinCall(const MemberBuiltinCall* call);
    void CheckOverride(const Override* o);
    void CheckReturn(const Return* ret);
    void CheckStore(const Store* s);
    void CheckStoreVectorElement(const StoreVectorElement* s);
    void CheckSubgroupMatrixOpOffset(const CoreBuiltinCall* call);
    void CheckSwitch(const Switch* s);
    void CheckSwizzle(const Swizzle* s);
    void CheckTerminator(const Terminator* b);
    void CheckUnary(const Unary* u);
    void CheckUserCall(const UserCall* call);
    void CheckVar(const Var* var);

    const Module& ir_;
    diag::List& diag_;
    ErrorSource error_source_;
    std::optional<ir::Disassembler> disassembler_;  // Use Disassemble()

    SymbolTable symbols_ = SymbolTable::Wrap(ir_.symbols);
    core::type::Manager type_mgr_ = core::type::Manager::Wrap(ir_.Types());
    core::ir::ReferencedModuleVars<const Module> referenced_module_vars_;

    Vector<const Block*, 8> block_stack_;
    Hashset<OverrideId, 8> seen_override_ids_;
    Hashmap<const Loop*, const Continue*, 4> first_continues_;
    Hashset<std::string, 4> entry_point_names_;
    Hashmap<const core::type::Type*, uint64_t, 16> elements_counts_;
};

}  // namespace tint::core::ir::validator

#endif  // SRC_TINT_LANG_CORE_IR_FUNCTIONAL_VALIDATOR_H_
