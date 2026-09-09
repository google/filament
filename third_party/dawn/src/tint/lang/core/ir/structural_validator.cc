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

#include "src/tint/lang/core/ir/structural_validator.h"

#include <algorithm>
#include <string_view>

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/referenced_functions.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/array_count.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/buffer.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/function.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u16.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/text_style.h"

namespace tint::core::ir::validator {
namespace {

struct Pending {
    const core::type::Type* type = nullptr;
    const core::type::Type* parent = nullptr;

    bool operator==(const Pending& other) const {
        return type == other.type && parent == other.parent;
    }

    struct Hasher {
        HashCode operator()(const Pending& p) const {
            return HashCombine(Hash(p.type), Hash(p.parent));
        }
    };
};

/// @returns the parent block of @p block
const Block* ParentBlockOf(const Block* block) {
    if (auto* parent = block->Parent()) {
        return parent->Block();
    }
    return nullptr;
}

/// @returns true if @p block directly or transitively holds the instruction @p inst
bool TransitivelyHolds(const Block* block, const Instruction* inst) {
    for (auto* b = inst->Block(); b; b = ParentBlockOf(b)) {
        if (b == block) {
            return true;
        }
    }
    return false;
}

template <typename CTX, typename IMPL>
void WalkTypeAndMembers(CTX& ctx,
                        const core::type::Type* type,
                        const IOAttributes& attr,
                        IMPL&& impl);
/// Helper that walks the members of a struct, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param str the struct to walk the members of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkStructMembers(CTX& ctx, const core::type::Struct* str, IMPL&& impl) {
    for (auto* member : str->Members()) {
        WalkTypeAndMembers(ctx, member->Type(), member->Attributes(), impl);
    }
}

/// Helper that walks an array's element type, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param arr the array to walk the element type of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkArrayElements(CTX& ctx, const core::type::Array* arr, IMPL&& impl) {
    WalkTypeAndMembers(ctx, arr->ElemType(), IOAttributes{}, impl);
}

/// Helper for walking a type that maybe a struct, calling an impl function for the type and each of
/// its members.
/// @param ctx a context object to pass to the implementation function
/// @param type the type to walk
/// @param attr the attributes for @p type
/// @param impl a function with the signature `void(const core::type::Type*, const IOAttributes&,
///             CTX&)` that is called for each type.
template <typename CTX, typename IMPL>
void WalkTypeAndMembers(CTX& ctx,
                        const core::type::Type* type,
                        const IOAttributes& attr,
                        IMPL&& impl) {
    impl(ctx, type, attr);
    tint::Switch(
        type, [&](const core::type::Struct* s) { WalkStructMembers(ctx, s, impl); },
        [&](const core::type::Array* a) { WalkArrayElements(ctx, a, impl); });
}

}  // namespace

Structural::Structural(const Module& ir, diag::List& diagnostics)
    : ir_(ir), diag_(diagnostics), referenced_module_vars_(ir) {}

Structural::~Structural() = default;

Disassembler& Structural::Disassemble() {
    if (!disassembler_) {
        disassembler_.emplace(ir::Disassembler(ir_));
    }
    return *disassembler_;
}

void Structural::Validate() {
    RunStructuralSoundnessChecks();

    CheckForRecursion();
    CheckForOrphanedInstructions();
    CheckStageRestrictedInstructions();
}

void Structural::CheckForRecursion() {
    if (diag_.ContainsErrors()) {
        return;
    }

    ReferencedFunctions<const Module> referenced_functions(ir_);
    for (auto& func : ir_.functions) {
        auto& refs = referenced_functions.TransitiveReferences(func);
        if (refs.Contains(func)) {
            // TODO(434684891): Consider improving this error with more information.
            AddError(func) << "recursive function calls are not allowed";
            return;
        }
    }
}

void Structural::CheckForOrphanedInstructions() {
    if (diag_.ContainsErrors()) {
        return;
    }

    // Check for orphaned instructions.
    for (auto* inst : ir_.Instructions()) {
        if (!visited_instructions_.Contains(inst)) {
            AddError(inst) << "orphaned instruction: " << inst->FriendlyName();
        }
    }
}

void Structural::CheckStageRestrictedInstructions() {
    if (diag_.ContainsErrors()) {
        return;
    }

    // Check for instructions being used in stages that do not support them.
    for (const auto& i : stage_restricted_instructions_) {
        const auto& inst = i.key;
        const auto& stages = i.value;
        const auto* f = ContainingFunction(inst);
        if (f == nullptr) {
            continue;
        }

        if (f->IsEntryPoint() && !stages.Contains(f->Stage())) {
            AddError(inst) << "cannot be used in a " << f->Stage() << " shader";
        } else {
            for (const Function* ep : ContainingEndPoints(f)) {
                if (!stages.Contains(ep->Stage())) {
                    AddError(inst) << "cannot be used in a " << ep->Stage() << " shader";
                }
            }
        }
    }
}

void Structural::RunStructuralSoundnessChecks() {
    scope_stack_.Push();
    TINT_DEFER({
        scope_stack_.Pop();
        TINT_ASSERT(scope_stack_.IsEmpty());
        TINT_ASSERT(tasks_.IsEmpty());
        TINT_ASSERT(control_stack_.IsEmpty());
        TINT_ASSERT(block_stack_.IsEmpty());
    });
    CheckRootBlock(ir_.root_block);

    for (auto& func : ir_.functions) {
        if (!all_functions_.Add(func)) {
            AddError(func) << "function " << NameOf(func) << " added to module multiple times";
        }
        scope_stack_.Add(func);
    }

    for (auto& func : ir_.functions) {
        block_to_function_.Add(func->Block(), func);
        CheckFunction(func);
    }
}

diag::Diagnostic& Structural::AddError(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Structural::AddError(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Structural::AddResultError(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Structural::AddError(const Block* blk) {
    auto src = Disassemble().BlockSource(blk);
    return AddError(src);
}

diag::Diagnostic& Structural::AddError(const BlockParam* param) {
    auto src = Disassemble().BlockParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Structural::AddError(const Function* func) {
    auto src = Disassemble().FunctionSource(func);
    return AddError(src);
}

diag::Diagnostic& Structural::AddError(const FunctionParam* param) {
    auto src = Disassemble().FunctionParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Structural::AddError(const CastableBase* base) {
    diag::Diagnostic* diag = nullptr;
    tint::Switch(
        base,  //
        [&](const Block* block) { diag = &AddError(block); },
        [&](const BlockParam* param) { diag = &AddError(param); },
        [&](const Function* fn) { diag = &AddError(fn); },
        [&](const FunctionParam* param) { diag = &AddError(param); },
        [&](const Instruction* inst) { diag = &AddError(inst); },
        [&](const InstructionResult* res) { diag = &AddError(res); });
    TINT_ASSERT(diag);
    return *diag;
}

diag::Diagnostic& Structural::AddNote(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    return AddNote(src);
}

diag::Diagnostic& Structural::AddNote(const Function* func) {
    auto src = Disassemble().FunctionSource(func);
    return AddNote(src);
}

diag::Diagnostic& Structural::AddOperandNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Structural::AddResultNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Structural::AddNote(const Block* blk) {
    auto src = Disassemble().BlockSource(blk);
    return AddNote(src);
}

diag::Diagnostic& Structural::AddError(Source src) {
    auto& diag = diag_.AddError(src);
    diag.owned_file = Disassemble().File();
    return diag;
}

diag::Diagnostic& Structural::AddNote(Source src) {
    auto& diag = diag_.AddNote(src);
    diag.owned_file = Disassemble().File();
    return diag;
}

void Structural::AddDeclarationNote(const CastableBase* decl) {
    tint::Switch(
        decl,  //
        [&](const Block* block) { AddDeclarationNote(block); },
        [&](const BlockParam* param) { AddDeclarationNote(param); },
        [&](const Function* fn) { AddDeclarationNote(fn); },
        [&](const FunctionParam* param) { AddDeclarationNote(param); },
        [&](const Instruction* inst) { AddDeclarationNote(inst); },
        [&](const InstructionResult* res) { AddDeclarationNote(res); });
}

void Structural::AddDeclarationNote(const Block* block) {
    auto src = Disassemble().BlockSource(block);
    if (src.file) {
        AddNote(src) << NameOf(block) << " declared here";
    }
}

void Structural::AddDeclarationNote(const BlockParam* param) {
    auto src = Disassemble().BlockParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Structural::AddDeclarationNote(const Function* fn) {
    AddNote(fn) << NameOf(fn) << " declared here";
}

void Structural::AddDeclarationNote(const FunctionParam* param) {
    auto src = Disassemble().FunctionParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Structural::AddDeclarationNote(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    if (src.file) {
        AddNote(src) << NameOf(inst) << " declared here";
    }
}

void Structural::AddDeclarationNote(const InstructionResult* res) {
    if (auto* inst = res->Instruction()) {
        auto results = inst->Results();
        for (size_t i = 0; i < results.Length(); i++) {
            if (results[i] == res) {
                AddResultNote(res->Instruction(), i) << NameOf(res) << " declared here";
                return;
            }
        }
    }
}

StyledText Structural::NameOf(const CastableBase* decl) {
    return tint::Switch(
        decl,  //
        [&](const core::type::Type* ty) { return NameOf(ty); },
        [&](const Value* value) { return NameOf(value); },
        [&](const Instruction* inst) { return NameOf(inst); },
        [&](const Block* block) { return NameOf(block); },  //
        TINT_ICE_ON_NO_MATCH);
}

StyledText Structural::NameOf(const core::type::Type* ty) {
    auto name = ty ? ty->FriendlyName() : "undef";
    return StyledText{} << style::Type(name);
}

StyledText Structural::NameOf(const Value* value) {
    return Disassemble().NameOf(value);
}

StyledText Structural::NameOf(const Instruction* inst) {
    auto name = inst ? inst->FriendlyName() : "undef";
    return StyledText{} << style::Instruction(name);
}

StyledText Structural::NameOf(const Block* block) {
    auto parent_name = block->Parent() ? block->Parent()->FriendlyName() : "undef";
    return StyledText{} << style::Instruction(parent_name) << " block "
                        << Disassemble().NameOf(block);
}

bool Structural::CheckResult(const Instruction* inst, size_t idx) {
    auto* result = inst->Result(idx);
    if (DAWN_UNLIKELY(result == nullptr)) {
        AddResultError(inst, idx) << "result is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Type() == nullptr)) {
        AddResultError(inst, idx) << "result type is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Instruction() == nullptr)) {
        AddResultError(inst, idx) << "result instruction is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Instruction() != inst)) {
        AddResultError(inst, idx)
            << "result instruction does not match instruction (possible double usage)";
        return false;
    }

    if (!inst->Is<core::ir::Call>() && result->Type()->Is<core::type::Void>()) {
        AddResultError(inst, idx) << "result type cannot be void";
        return false;
    }

    if (inst->Is<core::ir::ControlInstruction>()) {
        if (result->Type()->Is<core::type::Pointer>()) {
            AddResultError(inst, idx) << "result type cannot be a pointer";
            return false;
        }
        if (!result->Type()->IsConstructible()) {
            AddResultError(inst, idx) << "result type must be constructable";
            return false;
        }
    }

    if (result->Type()->Is<core::type::Void>() && ir_.NameOf(result)) {
        AddResultError(inst, idx) << "void results must not have names";
        return false;
    }

    const core::type::Type* ty = result->Type();
    bool check_size = false;

    if (auto* mv = ty->As<core::type::MemoryView>()) {
        if (mv->AddressSpace() == core::AddressSpace::kFunction ||
            mv->AddressSpace() == core::AddressSpace::kPrivate) {
            check_size = true;
            ty = mv->StoreType();
        }
    } else {
        check_size = true;
    }

    if (check_size) {
        if (ty->Size() > tint::internal_limits::kMaxTemporaryStorageSize) {
            AddResultError(inst, idx)
                << "result type size (" << ty->Size() << ") exceeds maximum allowed ("
                << tint::internal_limits::kMaxTemporaryStorageSize << ")";
            return false;
        }
    }

    return true;
}

bool Structural::CheckResults(const ir::Instruction* inst, std::optional<size_t> count = {}) {
    if (count.has_value()) {
        if (DAWN_UNLIKELY(inst->Results().Length() != count.value())) {
            AddError(inst) << "expected exactly " << count.value() << " results, got "
                           << inst->Results().Length();
            return false;
        }
    }

    bool passed = true;
    Hashset<const InstructionResult*, 4> seen_instruction_results;
    for (size_t i = 0; i < inst->Results().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckResult(inst, i))) {
            passed = false;
        }

        if (!seen_instruction_results.Add(inst->Result(i))) {
            AddResultError(inst, i) << "result was seen previously as a result";
            passed = false;
        }
    }
    return passed;
}

bool Structural::CheckOperand(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operand(idx);

    if (DAWN_UNLIKELY(operand == nullptr)) {
        // var and override instructions are allowed to have a nullptr initializers.
        // terminator instructions use nullptr operands to signal 'undef'.
        if (inst->IsAnyOf<Terminator, Var, Override>()) {
            return true;
        }

        AddError(inst, idx) << "operand is undefined";
        return false;
    }

    // ir::Unused is a internal value used by some transforms to track unused entries, and is
    // removed as part of generating an output shader.
    if (DAWN_UNLIKELY(operand->Is<ir::Unused>())) {
        return true;
    }

    if (DAWN_UNLIKELY(operand->Type() == nullptr)) {
        AddError(inst, idx) << "operand type is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->Alive())) {
        AddError(inst, idx) << "operand is not alive";
        return false;
    }

    if (DAWN_UNLIKELY(operand->Is<Constant>() &&
                      operand->Type()->Is<core::type::SubgroupMatrix>())) {
        AddError(inst, idx) << "subgroup_matrix values cannot be constant";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->HasUsage(inst, idx))) {
        AddError(inst, idx) << "operand missing usage";
        return false;
    }

    if (auto fn = operand->As<Function>(); fn && !all_functions_.Contains(fn)) {
        AddError(inst, idx) << NameOf(operand) << " is not part of the module";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->Is<ir::Unused>() && !operand->Is<Constant>() &&
                      !scope_stack_.Contains(operand))) {
        AddError(inst, idx) << NameOf(operand) << " is not in scope";
        AddDeclarationNote(operand);
        return false;
    }

    return true;
}

bool Structural::CheckOperands(const ir::Instruction* inst,
                               size_t min_count,
                               std::optional<size_t> max_count) {
    if (DAWN_UNLIKELY(inst->Operands().Length() < min_count)) {
        if (max_count.has_value()) {
            AddError(inst) << "expected between " << min_count << " and " << max_count.value()
                           << " operands, got " << inst->Operands().Length();
        } else {
            AddError(inst) << "expected at least " << min_count << " operands, got "
                           << inst->Operands().Length();
        }
        return false;
    }

    if (DAWN_UNLIKELY(max_count.has_value() && inst->Operands().Length() > max_count.value())) {
        AddError(inst) << "expected between " << min_count << " and " << max_count.value()
                       << " operands, got " << inst->Operands().Length();
        return false;
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Operands().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckOperand(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Structural::CheckOperands(const ir::Instruction* inst, std::optional<size_t> count = {}) {
    if (count.has_value()) {
        if (DAWN_UNLIKELY(inst->Operands().Length() != count.value())) {
            AddError(inst) << "expected exactly " << count.value() << " operands, got "
                           << inst->Operands().Length();
            return false;
        }
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Operands().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckOperand(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Structural::CheckResultsAndOperandRange(const ir::Instruction* inst,
                                             size_t num_results,
                                             size_t min_operands,
                                             std::optional<size_t> max_operands = {}) {
    // Intentionally avoiding short-circuiting here
    bool results_passed = CheckResults(inst, num_results);
    bool operands_passed = CheckOperands(inst, min_operands, max_operands);
    return results_passed && operands_passed;
}

bool Structural::CheckResultsAndOperands(const ir::Instruction* inst,
                                         size_t num_results,
                                         size_t num_operands) {
    // Intentionally avoiding short-circuiting here
    bool results_passed = CheckResults(inst, num_results);
    bool operands_passed = CheckOperands(inst, num_operands);
    return results_passed && operands_passed;
}

void Structural::CheckType(const core::type::Type* root, std::function<diag::Diagnostic&()> diag) {
    if (root == nullptr) {
        return;
    }

    if (!ir_.properties.Contains(Property::kAllowNonCoreTypes) && !root->IsCore()) {
        diag() << "non-core types not allowed in core IR";
        return;
    }

    if (!validated_types_.Add(root)) {
        return;
    }

    if (!CheckNestDepth(root, diag)) {
        return;
    }

    AddressSpace addrspace = AddressSpace::kUndefined;
    if (auto* mv = root->As<core::type::MemoryView>()) {
        addrspace = mv->AddressSpace();
    }

    Vector<Pending, 8> stack{Pending{root, nullptr}};
    Hashset<Pending, 8, Pending::Hasher> seen{};
    while (!stack.IsEmpty()) {
        auto [ty, parent] = stack.Pop();
        if (!ty) {
            continue;
        }
        if (ty->IsAbstract()) {
            diag() << "abstracts are not permitted";
            return;
        }

        bool chk = tint::Switch(
            ty,  //
            [&](const core::type::Struct* str) { return CheckStruct(str, diag); },
            [&](const core::type::Reference* ref) { return CheckRef(ref, diag, root); },
            [&](const core::type::Pointer* ptr) { return CheckPtr(ptr, diag); },
            [&](const core::type::I8*) { return Check8BitInteger(diag, parent); },
            [&](const core::type::U8*) { return Check8BitInteger(diag, parent); },
            [&](const core::type::U16*) { return Check16BitInteger(diag); },
            [&](const core::type::U64*) { return Check64BitInteger(diag); },
            [&](const core::type::F16*) { return Check16BitFloat(diag); },
            [&](const core::type::Array* arr) { return CheckArray(arr, diag); },
            [&](const core::type::Vector* v) { return CheckVector(v, diag); },
            [&](const core::type::Matrix* m) { return CheckMatrix(m, diag); },
            [&](const core::type::Atomic* a) { return CheckAtomic(a, diag); },
            [&](const core::type::SampledTexture* s) { return CheckSampledTexture(s, diag); },
            [&](const core::type::MultisampledTexture* ms) {
                return CheckMultisampledTexture(ms, diag);
            },
            [&](const core::type::StorageTexture* s) { return CheckStorageTexture(s, diag); },
            [&](const core::type::InputAttachment* i) { return CheckInputAttachment(i, diag); },
            [&](const core::type::SubgroupMatrix* m) {
                return CheckSubgroupMatrix(m, diag, addrspace);
            },
            [&](const core::type::BindingArray* t) {
                return CheckBindingArray(t, diag, addrspace);
            },
            [&](const core::type::Buffer* buf) { return CheckBuffer(buf, diag); },
            [&](const core::type::SwizzleView* sv) { return CheckSwizzleView(sv, diag); },
            [](Default) { return true; });
        if (!chk) {
            return;
        }

        if (auto* view = ty->As<core::type::MemoryView>()) {
            Pending next{view->StoreType(), ty};
            if (seen.Add(next)) {
                stack.Push(next);
            }
            continue;
        }

        // Visit the elements of a composite type.
        auto type_count = ty->Elements();
        if (type_count.type) {
            // Every element has the same type (e.g. array, vector, matrix, ...), so validate that
            // type once if it has not been seen before.
            Pending next{type_count.type, ty};
            if (seen.Add(next)) {
                stack.Push(next);
            }
            continue;
        }

        // Different elements have different types (e.g. a struct), so we need to validate each
        // of them if they have not been seen before.
        for (uint32_t i = 0; i < type_count.count; i++) {
            if (auto* subtype = ty->Element(i)) {
                Pending next{subtype, ty};
                if (seen.Add(next)) {
                    stack.Push(next);
                }
            }
        }
    }
}

bool Structural::CheckNestDepth(const core::type::Type* type,
                                std::function<diag::Diagnostic&()> diag) {
    if (type == nullptr) {
        return true;
    }

    struct Task {
        const core::type::Type* type;
        uint64_t depth;
    };

    Vector<Task, 16> tasks;
    tasks.Push({type, 0});

    while (!tasks.IsEmpty()) {
        auto [cur, depth] = tasks.Pop();

        // Unwrap memory views
        if (auto* view = cur->As<core::type::MemoryView>()) {
            cur = view->StoreType();
        }

        if (cur == nullptr) {
            continue;
        }

        auto max_depth = max_nest_depth_.Get(cur);
        if (max_depth && *max_depth.value >= depth) {
            // A deeper depth has already been tested for this type
            continue;
        }
        max_nest_depth_.Replace(cur, depth);

        if (depth > internal_limits::kMaxNestDepthOfCompositeType) {
            diag() << "type has a nesting depth that exceeds the maximum of "
                   << internal_limits::kMaxNestDepthOfCompositeType;
            return false;
        }

        auto elems = cur->Elements();
        if (elems.count == 0) {
            // Not a composite type, so no further work needed
            continue;
        }

        // cur is a composite type so need to check all of the contained elements
        uint64_t next_depth = depth + 1;
        if (elems.type) {
            // Homogeneous elements, i.e. is an array, vec, etc, so only need to enqueue one type
            tasks.Push({elems.type, next_depth});
        } else {
            // Heterogeneous elements, so need to enqueue each type
            for (uint32_t i = 0; i < elems.count; i++) {
                if (auto* elem_type = cur->Element(i)) {
                    tasks.Push({elem_type, next_depth});
                }
            }
        }
    }

    return true;
}

bool Structural::CheckBuffer(const core::type::Buffer* buf,
                             std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllowBufferTypes)) {
        diag() << "buffer types are not allowed in this context";
        return false;
    }
    if (auto count = buf->ConstantCount()) {
        const bool allow_16_bits = ir_.properties.Contains(Property::kAllow16BitFloats) ||
                                   ir_.properties.Contains(Property::kAllow16BitIntegers);
        const uint32_t divisor = allow_16_bits ? 2 : 4;
        if (count.value() % divisor != 0) {
            diag() << "buffer size must be evenly divisible by " << divisor;
            return false;
        }
    }
    return true;
}

bool Structural::CheckBindingArray(const core::type::BindingArray* ba,
                                   std::function<diag::Diagnostic&()>& diag,
                                   core::AddressSpace addrspace) {
    if (!ba->Count()->Is<core::type::ConstantArrayCount>()) {
        diag() << "binding_array count must be a constant expression";
        return false;
    }

    auto count = ba->Count()->As<core::type::ConstantArrayCount>()->value;
    if (count == 0) {
        diag() << "binding array requires a constant array size > 0";
        return false;
    }

    if (!(addrspace == AddressSpace::kUndefined || addrspace == AddressSpace::kHandle) &&
        !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
        diag() << "invalid address space for binding_array : " << addrspace;
        return false;
    }

    if (!ir_.properties.Contains(Property::kAllowNonCoreTypes)) {
        if (!ba->ElemType()->Is<core::type::SampledTexture>()) {
            diag() << "binding_array element type must be a sampled texture type";
            return false;
        }
    }
    return true;
}

bool Structural::CheckSubgroupMatrix(const core::type::SubgroupMatrix* m,
                                     std::function<diag::Diagnostic&()>& diag,
                                     core::AddressSpace addrspace) {
    if (!m->Type()
             ->IsAnyOf<core::type::F16, core::type::F32, core::type::I8, core::type::I32,
                       core::type::U8, core::type::U32>()) {
        diag() << "invalid subgroup matrix component type: " << NameOf(m->Type());
        return false;
    }
    if (!(addrspace == AddressSpace::kUndefined || addrspace == AddressSpace::kFunction)) {
        diag() << "invalid address space for subgroup matrix : " << addrspace;
        return false;
    }
    return true;
}

bool Structural::CheckInputAttachment(const core::type::InputAttachment* ia,
                                      std::function<diag::Diagnostic&()>& diag) {
    if (!ia->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        diag() << "invalid input attachment component type: " << NameOf(ia->Type());
        return false;
    }
    return true;
}

bool Structural::CheckStorageTexture(const core::type::StorageTexture* storage,
                                     std::function<diag::Diagnostic&()>& diag) {
    switch (storage->Dim()) {
        case core::type::TextureDimension::kCube:
        case core::type::TextureDimension::kCubeArray:
            diag() << "dimension " << style::Literal(ToString(storage->Dim()))
                   << " for storage textures does not in WGSL yet";
            return false;
        case core::type::TextureDimension::kNone:
            diag() << "invalid texture dimension " << style::Literal(ToString(storage->Dim()));
            return false;
        default:
            break;
    }
    return true;
}

bool Structural::CheckMultisampledTexture(const core::type::MultisampledTexture* ms,
                                          std::function<diag::Diagnostic&()>& diag) {
    if (!ms->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        diag() << "invalid multisampled texture sample type: " << NameOf(ms->Type());
        return false;
    }

    switch (ms->Dim()) {
        case core::type::TextureDimension::k2d:
            break;
        default:
            diag() << "invalid multisampled texture dimension: "
                   << style::Literal(ToString(ms->Dim()));
            return false;
    }
    return true;
}

bool Structural::CheckSampledTexture(const core::type::SampledTexture* s,
                                     std::function<diag::Diagnostic&()>& diag) {
    if (!s->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        diag() << "invalid sampled texture sample type: " << NameOf(s->Type());
        return false;
    }
    return true;
}

bool Structural::CheckAtomic(const core::type::Atomic* atom,
                             std::function<diag::Diagnostic&()>& diag) {
    // Prior to lowering we allow for atomic operations on vec2u to support the
    // AtomicVec2UMinMax feature.
    if (auto* vec = atom->Type()->As<core::type::Vector>()) {
        if (vec->Width() == 2 && vec->Type()->Is<core::type::U32>()) {
            return true;
        }
    }

    if (!atom->Type()->IsAnyOf<core::type::I32, core::type::U32, core::type::U64>()) {
        diag() << "atomic subtype must be i32, u32 or u64 type is " << NameOf(atom->Type());
        return false;
    }
    return true;
}

bool Structural::CheckMatrix(const core::type::Matrix* mat,
                             std::function<diag::Diagnostic&()>& diag) {
    if (!mat->Type()->IsFloatScalar()) {
        diag() << "matrix elements, " << NameOf(mat) << ", must be float scalars";
        return false;
    }
    return true;
}

bool Structural::CheckVector(const core::type::Vector* vec,
                             std::function<diag::Diagnostic&()>& diag) {
    if (!vec->Type()->IsScalar()) {
        diag() << "vector elements, " << NameOf(vec) << ", must be scalars";
        return false;
    }
    return true;
}

bool Structural::CheckSwizzleView(const core::type::SwizzleView* sv,
                                  std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllowSwizzleView)) {
        diag() << "swizzle view is not allowed in this module";
        return false;
    }
    if (sv->FromSize() < 2 || sv->FromSize() > 4) {
        diag() << "swizzle view object must be a vector of 2, 3 or 4 elements, got "
               << sv->FromSize();
        return false;
    }
    if (sv->ToSize() < 1 || sv->ToSize() > 4) {
        diag() << "swizzle view result must be 1, 2, 3 or 4 elements, got " << sv->ToSize();
        return false;
    }
    return true;
}

bool Structural::CheckArray(const core::type::Array* arr,
                            std::function<diag::Diagnostic&()>& diag) {
    if (!arr->ElemType()->HasCreationFixedFootprint()) {
        diag() << "array elements, " << NameOf(arr) << ", must have creation-fixed footprint";
        return false;
    }
    if (auto* count = arr->Count()->As<core::type::ConstantArrayCount>()) {
        if (count->value == 0) {
            diag() << "array requires a constant array size > 0";
            return false;
        }
        return true;
    }

    if (auto* val_count = arr->Count()->As<core::ir::type::ValueArrayCount>()) {
        if (!val_count->value) {
            diag() << "ValueArrayCount value is undefined";
            return false;
        }
        if (!val_count->value->Alive()) {
            diag() << "ValueArrayCount value is not alive";
            return false;
        }
        if (!val_count->value->Type()->IsIntegerScalar()) {
            diag() << "ValueArrayCount must be an integer scalar type";
            return false;
        }
        auto* inst_res = val_count->value->As<core::ir::InstructionResult>();
        if (!inst_res) {
            diag() << "ValueArrayCount must be an instruction result";
            return false;
        }
        auto* inst = inst_res->Instruction();
        if (!inst || inst->Block() != ir_.root_block) {
            diag() << "ValueArrayCount must be a module-scoped override expression";
            return false;
        }
    }
    return true;
}

// 8-bit integer types are guarded by the Allow8BitIntegers property.
// They can be used as the component type of a subgroup matrix without the property.
bool Structural::Check8BitInteger(std::function<diag::Diagnostic&()>& diag,
                                  const core::type::Type* parent) {
    if (!Is<core::type::SubgroupMatrix>(parent) &&
        !ir_.properties.Contains(Property::kAllow8BitIntegers)) {
        diag() << "8-bit integer types are not permitted";
        return false;
    }
    return true;
}

// 16-bit integer types are guarded by the Allow16BitIntegers property.
bool Structural::Check16BitInteger(std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllow16BitIntegers)) {
        diag() << "16-bit integer types are not permitted";
        return false;
    }
    return true;
}

// 64-bit integer types are guarded by the Allow64BitIntegers property.
bool Structural::Check64BitInteger(std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllow64BitIntegers)) {
        diag() << "64-bit integer types are not permitted";
        return false;
    }
    return true;
}

// 16-bit float types are guarded by the Allow16BitFloats property.
bool Structural::Check16BitFloat(std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllow16BitFloats)) {
        diag() << "16-bit float types are not permitted";
        return false;
    }
    return true;
}

bool Structural::CheckPtr(const core::type::Pointer* ptr,
                          std::function<diag::Diagnostic&()>& diag) {
    if (ptr->StoreType()->Is<core::type::Void>()) {
        diag() << "pointers to void are not permitted";
        return false;
    }

    if (ptr->AddressSpace() == AddressSpace::kUniform ||
        ptr->AddressSpace() == AddressSpace::kHandle ||
        ptr->AddressSpace() == core::AddressSpace::kImmediate) {
        if (ptr->Access() != core::Access::kRead) {
            diag() << ToString(ptr->AddressSpace()) << " pointers must be read access";
            return false;
        }
    }

    if (ptr->AddressSpace() == AddressSpace::kWorkgroup ||
        ptr->AddressSpace() == AddressSpace::kFunction ||
        ptr->AddressSpace() == AddressSpace::kPrivate) {
        if (ptr->Access() != core::Access::kReadWrite) {
            diag() << ToString(ptr->AddressSpace()) << " pointers must be read_write access";
            return false;
        }
    }

    if (ptr->AddressSpace() == AddressSpace::kHandle) {
        if (!ptr->StoreType()->IsHandle()) {
            diag() << "the 'handle' address space can only be used for handle types";
            return false;
        }
    } else if (ptr->StoreType()->IsHandle()) {
        diag() << "handle types can only be declared in the 'handle' address space";
        return false;
    }

    if (ptr->StoreType()->Is<core::type::Pointer>()) {
        diag() << "pointers to pointers are not allowed";
        return false;
    }

    if (ptr->StoreType()->Is<core::type::Buffer>()) {
        if (ptr->AddressSpace() != AddressSpace::kWorkgroup &&
            ptr->AddressSpace() != AddressSpace::kStorage &&
            ptr->AddressSpace() != AddressSpace::kUniform) {
            diag() << "buffer types are not allowed in the '" << ToString(ptr->AddressSpace())
                   << "' address space";
            return false;
        }
    }

    return true;
}

bool Structural::CheckRef(const core::type::Reference* ref,
                          std::function<diag::Diagnostic&()>& diag,
                          const core::type::Type* root) {
    if (ref->StoreType()->Is<core::type::Void>()) {
        diag() << "references to void are not permitted";
        return false;
    }

    // Reference types are guarded by the AllowRefTypes property.
    if (!ir_.properties.Contains(Property::kAllowRefTypes)) {
        diag() << "reference types are not permitted here";
        return false;
    }
    // If they are allowed, reference types still cannot be nested.
    if (ref != root) {
        diag() << "nested reference types are not permitted";
        return false;
    }
    return true;
}

bool Structural::CheckStruct(const core::type::Struct* str,
                             std::function<diag::Diagnostic&()>& diag) {
    uint32_t cur_offset = 0;
    for (auto* member : str->Members()) {
        if (member->Type()->Is<core::type::Void>()) {
            diag() << "struct member " << member->Index() << " cannot have void type";
            return false;
        }
        if (member->Type()->Is<core::type::Buffer>()) {
            diag() << "struct member " << member->Index() << " cannot have buffer type";
            return false;
        }

        if (!CheckStructMemberAttributes(member, diag)) {
            return false;
        }

        if (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
            if (member->Type()->Is<core::type::Pointer>()) {
                diag() << "struct member " << member->Index() << " cannot be a pointer type";
                return false;
            }

            if (member->Type()->Is<core::type::Texture>()) {
                diag() << "struct member " << member->Index() << " cannot be a texture type";
                return false;
            }

            if (member->Type()->Is<core::type::Sampler>()) {
                diag() << "struct member " << member->Index() << " cannot be a sampler type";
                return false;
            }
        }

        if (auto* arr = member->Type()->As<core::type::Array>();
            arr && arr->Count()->Is<core::type::RuntimeArrayCount>()) {
            if (member != str->Members().Back()) {
                diag() << "runtime-sized arrays can only be the last member of a "
                          "struct";
                return false;
            }
        }

        if (member->Align() == 0) {
            diag() << "struct member must not have an alignment of 0";
            return false;
        }
        if (!tint::IsPowerOfTwo(member->Align())) {
            diag() << "struct member alignment must be a power of 2";
            return false;
        }

        if (member->Type()->Align() == 0) {
            diag() << "struct member type must not have an alignment of 0";
            return false;
        }
        if (!tint::IsPowerOfTwo(member->Type()->Align())) {
            diag() << "struct member type alignment must be a power of 2";
            return false;
        }
        if (ir_.properties.Contains(Property::kAllowStructMatrixDecorations)) {
            if (member->RowMajor() || member->HasMatrixStride()) {
                const core::type::Type* base_ty = member->Type();
                while (auto* arr = base_ty->As<core::type::Array>()) {
                    base_ty = arr->ElemType();
                }
                if (!base_ty->Is<core::type::Matrix>()) {
                    if (member->RowMajor()) {
                        diag() << "RowMajor attribute can only be applied to a matrix or an array "
                                  "of matrices";
                    } else {
                        diag() << "MatrixStride attribute can only be applied to a matrix or an "
                                  "array of matrices";
                    }
                    return false;
                }
            }
        } else {
            if (member->RowMajor()) {
                diag() << "Row major annotation not allowed on structures";
                return false;
            }
            if (member->HasMatrixStride()) {
                diag() << "Matrix stride annotation not allowed on structures";
                return false;
            }
        }

        // TODO(448608979): Remove guard once updated to handle RowMajor correctly
        if (!member->RowMajor()) {
            if (member->Size() < member->Type()->Size()) {
                diag() << "struct member " << member->Index() << " with size=" << member->Size()
                       << " must be at least as large as the type with size "
                       << member->Type()->Size();
                return false;
            }

            if (member->Align() % member->Type()->Align() != 0) {
                diag() << "struct member alignment (" << member->Align()
                       << ") must be divisible by type alignment (" << member->Type()->Align()
                       << ")";
                return false;
            }
        }

        cur_offset += (member->Offset() - cur_offset) + member->MinimumRequiredSize();
    }
    if (str->Size() < cur_offset) {
        diag() << "struct size (" << str->Size() << ") is smaller than the end of the last member ("
               << cur_offset << ")";
        return false;
    }

    return true;
}

void Structural::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER(block_stack_.Pop());

    Hashset<const core::ir::Value*, 8> pipeline_evaluatable{};

    auto add_evaluatable = [&](const Instruction* inst, const bool is_creatable) {
        if (auto* res = inst->Result(0); res != nullptr && is_creatable) {
            pipeline_evaluatable.Add(res);
        }
    };

    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "instruction in root block does not have root block as parent";
            continue;
        }

        auto is_pipeline_creatable = true;
        for (auto* op : inst->Operands()) {
            if (!op) {
                continue;
            }
            if (op->Is<core::ir::Constant>()) {
                continue;
            }
            if (pipeline_evaluatable.Contains(op)) {
                continue;
            }
            is_pipeline_creatable = false;
            break;
        }

        if (!is_pipeline_creatable) {
            AddError(inst) << "instruction is not evaluatable at pipeline creation time";
        }

        tint::Switch(
            inst,  //
            [&](const core::ir::Override* o) {
                if (ir_.properties.Contains(Property::kAllowOverrides)) {
                    CheckInstruction(o);
                    add_evaluatable(o, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const core::ir::Var* var) { CheckInstruction(var); },
            [&](const core::ir::Let* let) {
                if (ir_.properties.Contains(Property::kAllowModuleScopeLets)) {
                    CheckInstruction(let);
                    add_evaluatable(let, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const core::ir::Construct* c) {
                if (ir_.properties.Contains(Property::kAllowModuleScopeLets) ||
                    ir_.properties.Contains(Property::kAllowOverrides)) {
                    CheckInstruction(c);
                    CheckOnlyUsedInRootBlock(inst);
                    add_evaluatable(c, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](Default) {
                // Note, this validation around kAllowOverrides is looser than it could be. There
                // are only certain expressions and builtins which can be used in an override, which
                // currently isn't checked.
                if (ir_.properties.Contains(Property::kAllowOverrides) &&
                    inst->IsAnyOf<core::ir::Unary, core::ir::Binary, core::ir::BuiltinCall,
                                  core::ir::Convert, core::ir::Swizzle, core::ir::Access,
                                  core::ir::ConstExprIf>()) {
                    CheckInstruction(inst);
                    // If overrides are allowed we can have certain regular instructions in the root
                    // block, with the caveat that those instructions can _only_ be used in the root
                    // block.
                    CheckOnlyUsedInRootBlock(inst);
                    add_evaluatable(inst, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            });

        // Process tasks queued by CheckInstruction (like AddResults) before moving to next
        // instruction.
        ProcessTasks();
    }
}

void Structural::CheckOnlyUsedInRootBlock(const Instruction* inst) {
    if (inst->Result(0)) {
        for (auto& usage : inst->Result(0)->UsagesSorted()) {
            if (usage.instruction->Block() != ir_.root_block) {
                AddError(inst) << "root block: instruction used outside of root block "
                               << inst->TypeInfo().name;
            }
        }
    }

    CheckInstruction(inst);
}

void Structural::CheckFunction(const Function* func) {
    // Scope holds the parameters and block
    scope_stack_.Push();
    TINT_DEFER(scope_stack_.Pop());

    // The recursion checks require this to be true as it will be asserted by the
    // referenced_functions helper.
    func->ForEachUseUnsorted([&](const Usage& use) {
        if (use.instruction->As<UserCall>() || use.instruction->As<Return>()) {
            return;
        }
        AddError(use.instruction, use.operand_index) << "function may not be used as a operand";
    });

    if (!func->Type() || !func->Type()->Is<core::type::Function>()) {
        AddError(func) << "functions must have type '<function>'";
        return;
    }

    // Note: This is not a validator error because Function::SetBlock() asserts that the block is
    // not null, and the disassembler will crash if this is not null. This should only be hit due
    // to some sort of corruption, not a bad shader/programmer error.
    TINT_ASSERT(func->Block()) << "root block for function is undefined";

    if (func->Block()->Is<ir::MultiInBlock>()) {
        AddError(func) << "root block for function cannot be a multi-in block";
        return;
    }

    Hashset<const FunctionParam*, 4> param_set{};
    for (auto* param : func->Params()) {
        if (!CheckFunctionParam(func, param, param_set)) {
            return;
        }

        scope_stack_.Add(param);
    }

    // TODO(516717234): Move to functional
    CheckType(func->ReturnType(), [&]() -> diag::Diagnostic& { return AddError(func); });

    // TODO(516717234): Determine what below to move to function.

    ValidateIOAttributes(func);
    CheckWorkgroupSize(func);
    CheckSubgroupSize(func);

    CheckEntryPoint(func);

    QueueBlock(func->Block());
    ProcessTasks();
}

void Structural::CheckEntryPoint(const Function* func) {
    if (!func->IsEntryPoint()) {
        return;
    }

    ValidateShaderIOAnnotations(func, func->ReturnType(), std::nullopt, func->ReturnAttributes(),
                                ShaderIOKind::kResultValue);

    WalkTypeAndMembers(func, func->ReturnType(), func->ReturnAttributes(),
                       [this](const Function* f, const core::type::Type* t, const IOAttributes&) {
                           CheckNotBool(f, t, "entry point returns can not be 'bool'");
                       });

    for (auto var : referenced_module_vars_.TransitiveReferences(func)) {
        const auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
        const auto* ty = var->Result()->Type()->UnwrapPtrOrRef();
        const auto attr = var->Attributes();
        if (!mv || !ty) {
            continue;
        }

        switch (mv->AddressSpace()) {
            case AddressSpace::kIn:
            case AddressSpace::kOut:
                break;
            default:
                continue;
        }

        if (func->IsFragment() && mv->AddressSpace() == AddressSpace::kIn) {
            WalkTypeAndMembers(var, ty, attr, [this](const auto* v, const auto* t, const auto& a) {
                CheckFrontFacingIfBool(v, a, t,
                                       "input address space values referenced by fragment shaders "
                                       "can only be 'bool' if decorated with "
                                       "@builtin(front_facing)");
            });
        } else {
            WalkTypeAndMembers(var, ty, attr, [this](const auto* v, const auto* t, const auto&) {
                CheckNotBool(v, t,
                             "IO address space values referenced by shader entry points can "
                             "only be 'bool' if in the input space, used only by fragment "
                             "shaders and decorated with @builtin(front_facing)");
            });
        }
    }
}

bool Structural::CheckFunctionParam(const Function* func,
                                    const FunctionParam* param,
                                    Hashset<const FunctionParam*, 4>& param_set) {
    if (!param->Alive()) {
        AddError(param) << "destroyed parameter found in function parameter list";
        return false;
    }

    if (!param_set.Add(param)) {
        AddError(param) << "function parameter is not unique";
        return false;
    }

    if (!param->Type()) {
        AddError(param) << "function parameter has nullptr type";
        return false;
    }

    if (!param->Function()) {
        AddError(param) << "function parameter has nullptr parent function";
        return false;
    }

    if (param->Function() != func) {
        AddError(param) << "function parameter has incorrect parent function";
        AddNote(param->Function()) << "parent function declared here";
        return false;
    }

    // TODO(516717234): Move to functional
    CheckType(param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); });

    // TODO(516717234): Move to functional
    if (func->IsFragment()) {
        WalkTypeAndMembers(param, param->Type(), param->Attributes(),
                           [this](const auto* p, const auto* t, const auto& a) {
                               CheckFrontFacingIfBool(
                                   p, a, t,
                                   "fragment entry point params can only be a bool if "
                                   "decorated with @builtin(front_facing)");
                           });
    } else if (func->IsEntryPoint()) {
        WalkTypeAndMembers(
            param, param->Type(), param->Attributes(),
            [this](const auto* p, const auto* t, const auto&) {
                CheckNotBool(p, t, "entry point params can only be a bool for fragment shaders");
            });
    }

    // TODO(516717234): Move to functional
    if (func->IsEntryPoint()) {
        ValidateShaderIOAnnotations(param, param->Type(), param->BindingPoint(),
                                    param->Attributes(), ShaderIOKind::kInputParam);
    } else {
        if (param->BindingPoint().has_value()) {
            AddError(param) << "input param to non-entry point function has a binding point set";
            return false;
        }

        if (param->Builtin().has_value()) {
            AddError(param) << "builtins can only be decorated on entry point params";
            return false;
        }
    }
    return true;
}

void Structural::ValidateIOAttributes(const Function* func) {
    const auto stage = func->Stage();
    struct Task {
        const CastableBase* anchor;
        const core::type::Type* type;
        const IOAttributes& attr;
        IODirection dir;
        ShaderIOKind io_kind;
    };
    Vector<Task, 16> tasks;

    // Gather parameters.
    for (auto* param : func->Params()) {
        tasks.Push({param, param->Type(), param->Attributes(), IODirection::kInput,
                    ShaderIOKind::kInputParam});
    }

    // Gather return value.
    tasks.Push({func, func->ReturnType(), func->ReturnAttributes(), IODirection::kOutput,
                ShaderIOKind::kResultValue});

    // Gather referenced module variables.
    for (auto* var : referenced_module_vars_.TransitiveReferences(func)) {
        auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
        if (mv == nullptr) {
            continue;
        }
        if (mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut ||
            mv->AddressSpace() == AddressSpace::kHandle) {
            tasks.Push({var, mv->StoreType(), var->Attributes(),
                        validator::IODirectionFor(mv->AddressSpace()),
                        ShaderIOKind::kModuleScopeVar});
        }
    }

    if (stage != Function::PipelineStage::kUndefined) {
        // Shared context for blend_src and location validation
        BlendSrcContext input_ctx{func->Stage(), {}, {}, nullptr, IODirection::kInput};
        BlendSrcContext output_ctx{func->Stage(), {}, {}, nullptr, IODirection::kOutput};

        // First pass: pre-populate location hashes for blend_src.
        for (const auto& task : tasks) {
            auto& ctx = task.dir == IODirection::kInput ? input_ctx : output_ctx;
            WalkTypeAndMembers(
                ctx, task.type, task.attr,
                [task](BlendSrcContext& c, const core::type::Type*, const IOAttributes& a) {
                    if (a.blend_src.has_value() && a.location.has_value()) {
                        c.locations.Add(a.location.value(), task.anchor);
                    }
                });
        }

        // Second pass: validate blend_src usages.
        for (const auto& task : tasks) {
            auto& ctx = task.dir == IODirection::kInput ? input_ctx : output_ctx;
            CheckBlendSrc(ctx, task.anchor, task.type, task.attr);
        }

        if (!output_ctx.blend_srcs.IsEmpty()) {
            if (output_ctx.blend_srcs.Count() != 2) {
                AddError(func) << "if any @blend_src is used on an output, then @blend_src(0) and "
                                  "@blend_src(1) must be used";
            }
        }

        // Third pass: validate all non-blend_src location usages.
        for (const auto& task : tasks) {
            if (task.dir == IODirection::kInput) {
                CheckLocation(input_ctx.locations, task.anchor, task.attr, func->Stage(), task.type,
                              task.dir);
            } else if (task.dir == IODirection::kOutput) {
                CheckLocation(output_ctx.locations, task.anchor, task.attr, func->Stage(),
                              task.type, task.dir);
            }
        }
    }

    // Validate all the interpolation usages.
    for (const auto& task : tasks) {
        CheckInterpolation(task.anchor, task.type, task.attr, stage, task.dir);
    }

    if (stage != Function::PipelineStage::kUndefined) {
        // Validate all the binding_point usages, and ensure things that require binding_point have
        // them.
        for (const auto& task : tasks) {
            CheckBindingPoint(task.anchor, task.type, task.attr, task.io_kind);
        }
    }

    IOAttributeContext impl_ctx{.input_builtins = {}, .output_builtins = {}};
    // Validate all remaining attributes on IO objects
    for (const auto& task : tasks) {
        ValidateIOAttributesImpl(impl_ctx, task.anchor, task.type, task.attr, stage, task.dir,
                                 task.io_kind);
    }
}

void Structural::ValidateIOAttributesImpl(IOAttributeContext& ctx,
                                          const CastableBase* msg_anchor,
                                          const core::type::Type* ty,
                                          const IOAttributes& attr,
                                          Function::PipelineStage stage,
                                          IODirection dir,
                                          ShaderIOKind io_kind) {
    bool skip_builtins = ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO) &&
                         io_kind == ShaderIOKind::kModuleScopeVar;
    const IOAttributeUsage usage = IOAttributeUsageFor(stage, dir);
    WalkTypeAndMembers(
        *this, ty, attr,
        [&ctx, msg_anchor, usage, io_kind, skip_builtins, dir](
            Structural& v, const core::type::Type* t, const IOAttributes& a) {
            const auto checkers = IOAttributeCheckersFor(a, skip_builtins);
            if (checkers.IsEmpty()) {
                return;
            }

            if (a.builtin.has_value() && !skip_builtins &&
                usage != IOAttributeUsage::kUndefinedUsage) {
                const auto& builtin = a.builtin.value();

                uint32_t count = 0;
                switch (dir) {
                    case IODirection::kInput:
                        count = ++(ctx.input_builtins.GetOrAddZeroEntry(builtin).value);
                        break;
                    case IODirection::kOutput:
                        count = ++(ctx.output_builtins.GetOrAddZeroEntry(builtin).value);
                        break;
                    default:
                        // This shouldn't ever happen, but this will get caught later in the
                        // checker, so just ignoring
                        break;
                }
                if (v.ir_.properties.Contains(Property::kAllowClipDistancesOnF32ScalarAndVector) &&
                    builtin == BuiltinValue::kClipDistances) {
                    if (count > 2) {
                        v.AddError(msg_anchor)
                            << "too many instances of builtin 'clip_distances' on entry point "
                            << ToString(dir)
                            << ", only two allowed with 'kAllowClipDistancesOnF32ScalarAndVector' "
                               "property enabled";
                    }
                } else {
                    if (count > 1) {
                        v.AddError(msg_anchor)
                            << "duplicate instance of builtin '" << ToString(builtin)
                            << "' on entry point " << ToString(dir)
                            << ", must be unique per entry point i/o direction";
                    }
                }
            }

            auto failed = tint::Hashset<const IOAttributeChecker*, 4>();

            if (usage != IOAttributeUsage::kUndefinedUsage) {
                for (const auto* checker : checkers) {
                    if (!checker->valid_usages.Contains(usage)) {
                        failed.Add(checker);

                        std::stringstream msg;
                        msg << ToString(checker->kind) << " IO attributes cannot be declared for a "
                            << ToString(usage) << ". ";
                        if (checker->valid_usages.Size() == 1) {
                            const auto& u = *checker->valid_usages.begin();
                            msg << "They can only be used for a " << ToString(u) << ".";
                        } else {
                            msg << "They can only be used for " << ToString(checker->valid_usages);
                        }
                        v.AddError(msg_anchor) << msg.str();
                    }
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (!checker->valid_io_kinds.Contains(io_kind)) {
                    failed.Add(checker);

                    std::stringstream msg;
                    msg << ToString(checker->kind) << " IO attributes cannot be declared on a "
                        << ToString(io_kind) << ". ";
                    if (checker->valid_io_kinds.Size() == 1) {
                        const auto& k = *checker->valid_io_kinds.begin();
                        msg << "They can only be used on a " << ToString(k) << ".";
                    } else {
                        msg << "They can only be used on " << ToString(checker->valid_io_kinds);
                    }
                    v.AddError(msg_anchor) << msg.str();
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (!checker->type_check(t, v.ir_.properties)) {
                    failed.Add(checker);
                    v.AddError(msg_anchor) << ToString(checker->kind) << " " << checker->type_error;
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (auto res = checker->check(t, a, v.ir_.properties, usage); res != Success) {
                    failed.Add(checker);
                    v.AddError(msg_anchor) << res.Failure();
                }
            }
        });
}

void Structural::CheckFrontFacingIfBool(const CastableBase* msg_anchor,
                                        const IOAttributes& attr,
                                        const core::type::Type* ty,
                                        const std::string& err) {
    if (ty->Is<core::type::Bool>() && attr.builtin != BuiltinValue::kFrontFacing) {
        AddError(msg_anchor) << err;
    }
}

void Structural::CheckNotBool(const CastableBase* msg_anchor,
                              const core::type::Type* ty,
                              const std::string& err) {
    if (ty->Is<core::type::Bool>()) {
        AddError(msg_anchor) << err;
    }
}

void Structural::CheckWorkgroupSize(const Function* func) {
    if (!func->IsCompute()) {
        if (func->WorkgroupSize().has_value()) {
            AddError(func) << "@workgroup_size only valid on compute entry point";
        }
        return;
    }

    if (!func->WorkgroupSize().has_value()) {
        AddError(func) << "compute entry point requires @workgroup_size";
        return;
    }

    auto workgroup_sizes = func->WorkgroupSize().value();
    // The number parameters cannot be checked here, since it is stored internally as a 3 element
    // array, so will always have 3 elements at this point.
    TINT_ASSERT(workgroup_sizes.size() == 3);

    uint64_t total_size = 1;

    std::optional<const core::type::Type*> sizes_ty;
    for (auto* size : workgroup_sizes) {
        if (!size || !size->Type()) {
            AddError(func) << "a @workgroup_size param is undefined or missing a type";
            return;
        }

        auto* ty = size->Type();
        if (!ty->IsAnyOf<core::type::I32, core::type::U32>()) {
            AddError(func) << "@workgroup_size params must be an 'i32' or 'u32', received "
                           << NameOf(ty);
            return;
        }

        if (!sizes_ty.has_value()) {
            sizes_ty = ty;
        }

        if (sizes_ty != ty) {
            AddError(func) << "@workgroup_size params must be all 'i32's or all 'u32's";
            return;
        }

        if (auto* c = size->As<ir::Constant>()) {
            if (c->Value()->ValueAs<int64_t>() <= 0) {
                AddError(func) << "@workgroup_size params must be greater than 0";
                return;
            }
            total_size *= c->Value()->ValueAs<uint64_t>();

            constexpr uint64_t kMaxGridSize = 0xffffffff;
            if (total_size > kMaxGridSize) {
                AddError(func) << "workgroup grid size cannot exceed 0x" << std::hex
                               << kMaxGridSize;
            }
            continue;
        }

        if (!ir_.properties.Contains(Property::kAllowOverrides)) {
            AddError(func) << "@workgroup_size param is not a constant value, and IR property "
                              "'AllowOverrides' is not enabled";
            return;
        }

        if (auto* r = size->As<ir::InstructionResult>()) {
            if (!r->Instruction()) {
                AddError(func) << "instruction for @workgroup_size param is not defined";
                return;
            }

            if (r->Instruction()->Block() != ir_.root_block) {
                AddError(func) << "@workgroup_size param defined by non-module scope value";
                return;
            }

            // Since above, it is already checked if the value is in the root block, it is assumed
            // to be pipeline creatable here, i.e. const/override or derived from consts and
            // overrides.
            // If that is not true, that indicates an issue in CheckRootBlock().
            continue;
        }

        AddError(func) << "@workgroup_size must be an InstructionResult or a Constant";
    }
}

void Structural::CheckSubgroupSize(const Function* func) {
    // @subgroup_size is optional
    if (!func->SubgroupSize().has_value()) {
        return;
    }

    if (!func->IsCompute()) {
        AddError(func) << "@subgroup_size only valid on compute entry point";
        return;
    }

    auto subgroup_size = func->SubgroupSize().value();
    if (subgroup_size == nullptr) {
        AddError(func) << "a @subgroup_size param must have a value";
        return;
    }

    if (!subgroup_size->Type()) {
        AddError(func) << "a @subgroup_size param is missing a type";
        return;
    }

    auto* ty = subgroup_size->Type();
    if (!ty->IsAnyOf<core::type::I32, core::type::U32>()) {
        AddError(func) << "@subgroup_size param must be an 'i32' or 'u32', received " << NameOf(ty);
        return;
    }

    if (auto* c = subgroup_size->As<ir::Constant>()) {
        int64_t value = c->Value()->ValueAs<int64_t>();
        if (value <= 0) {
            AddError(func) << "@subgroup_size param must be greater than 0";
            return;
        }

        if (!IsPowerOfTwo<int64_t>(value)) {
            AddError(func) << "@subgroup_size param must be a power of 2";
            return;
        }

        return;
    }

    if (!ir_.properties.Contains(Property::kAllowOverrides)) {
        AddError(func) << "@subgroup_size param is not a constant value, and IR property "
                          "'AllowOverrides' is not enabled";
        return;
    }

    if (auto* r = subgroup_size->As<ir::InstructionResult>()) {
        if (!r->Instruction()) {
            AddError(func) << "instruction for @subgroup_size param is not defined";
            return;
        }

        if (r->Instruction()->Block() != ir_.root_block) {
            AddError(func) << "@subgroup_size param defined by non-module scope value";
            return;
        }

        if (r->Instruction()->Is<core::ir::Override>()) {
            return;
        }
    }

    AddError(func) << "@subgroup_size must be an InstructionResult or a Constant";
}

void Structural::ProcessTasks() {
    while (!tasks_.IsEmpty()) {
        tasks_.Pop()();
    }
}

void Structural::QueueBlock(const Block* blk) {
    tasks_.Push([this] { EndBlock(); });
    tasks_.Push([this, blk] { BeginBlock(blk); });
}

void Structural::BeginBlock(const Block* blk) {
    scope_stack_.Push();
    block_stack_.Push(blk);

    if (auto* mb = blk->As<MultiInBlock>()) {
        for (auto* param : mb->Params()) {
            if (!param->Alive()) {
                AddError(param) << "destroyed parameter found in block parameter list";
                return;
            }
            if (!param->Block()) {
                AddError(param) << "block parameter has nullptr parent block";
                return;
            } else if (param->Block() != mb) {
                AddError(param) << "block parameter has incorrect parent block";
                AddNote(param->Block()) << "parent block declared here";
                return;
            }

            CheckType(param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); });

            if (param->Type()->Is<core::type::Void>()) {
                AddError(param) << "block parameter type cannot be void";
            }
            if (param->Type()->Is<core::type::Reference>()) {
                AddError(param) << "block parameter type cannot be a reference";
            }

            scope_stack_.Add(param);
        }
    }

    if (!blk->Terminator()) {
        AddError(blk) << "block does not end in a terminator instruction";
    }

    // Validate the instructions w.r.t. the parent block
    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "block instruction does not have same block as parent";
            AddNote(blk) << "in block";
        }
    }

    // Enqueue validation of the instructions of the block
    if (!blk->IsEmpty()) {
        QueueInstructions(blk->Instructions());
    }
}

void Structural::EndBlock() {
    scope_stack_.Pop();
    block_stack_.Pop();
}

void Structural::QueueInstructions(const Instruction* inst) {
    if (diag_.ContainsErrors()) {
        return;
    }

    tasks_.Push([this, inst] {
        // Tasks are processed LIFO, so push the next instruction to the stack before checking the
        // current instruction, which may need to add more blocks to the stack itself.
        if (inst->next) {
            QueueInstructions(inst->next);
        }
        CheckInstruction(inst);
    });
}

void Structural::CheckInstruction(const Instruction* inst) {
    visited_instructions_.Add(inst);
    if (!inst->Alive()) {
        AddError(inst) << "destroyed instruction found in instruction list";
        return;
    }

    auto results = inst->Results();
    for (size_t i = 0; i < results.Length(); ++i) {
        auto* res = results[i];
        if (!res) {
            continue;
        }

        CheckType(res->Type(), [&]() -> diag::Diagnostic& { return AddResultError(inst, i); });
    }

    auto ops = inst->Operands();
    for (size_t i = 0; i < ops.Length(); ++i) {
        auto* op = ops[i];
        if (!op) {
            continue;
        }

        CheckType(op->Type(), [&]() -> diag::Diagnostic& { return AddError(inst, i); });
    }

    // Push a task to add the results to the scope.
    // This ensures that for control instructions, the results are only added to the scope
    // after their nested blocks have been evaluated (since tasks are processed LIFO).
    tasks_.Push([this, inst] {
        for (auto* result : inst->Results()) {
            if (result) {
                scope_stack_.Add(result);
            }
        }
    });

    tint::Switch(
        inst,                                                              //
        [&](const Access* a) { CheckAccess(a); },                          //
        [&](const Binary* b) { CheckBinary(b); },                          //
        [&](const Call* c) { CheckCall(c); },                              //
        [&](const If* if_) { CheckIf(if_); },                              //
        [&](const Let* let) { CheckLet(let); },                            //
        [&](const Load* load) { CheckLoad(load); },                        //
        [&](const LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
        [&](const Loop* l) { CheckLoop(l); },                              //
        [&](const Phony* p) { CheckPhony(p); },                            //
        [&](const Store* s) { CheckStore(s); },                            //
        [&](const StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
        [&](const Switch* s) { CheckSwitch(s); },                          //
        [&](const Swizzle* s) { CheckSwizzle(s); },                        //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Override* o) { CheckOverride(o); },                      //
        [&](const Var* var) { CheckVar(var); },                            //
        TINT_ICE_ON_NO_MATCH);
}

void Structural::CheckOverride(const Override* o) {
    if (!CheckResultsAndOperands(o, Override::kNumResults, Override::kNumOperands)) {
        return;
    }

    if (o->Block() != ir_.root_block) {
        AddError(o) << "override must be declared at module scope";
    }
}

void Structural::CheckVar(const Var* var) {
    if (!CheckResultsAndOperands(var, Var::kNumResults, Var::kNumOperands)) {
        return;
    }

    // TODO(516717234): Remove when ValidateShaderIOAnnotations are moved to function validator
    auto* result_type = var->Result()->Type();
    auto* mv = result_type->As<core::type::MemoryView>();
    if (!mv) {
        AddError(var) << "result type " << NameOf(result_type)
                      << " must be a pointer or a reference";
        return;
    }
    const core::ir::type::ValueArrayCount* count = nullptr;
    if (auto* ary = result_type->UnwrapPtr()->As<core::type::Array>()) {
        count = ary->Count()->As<core::ir::type::ValueArrayCount>();
    } else if (auto* buf = result_type->UnwrapPtr()->As<core::type::Buffer>()) {
        count = buf->Count()->As<core::ir::type::ValueArrayCount>();
    }

    if (count) {
        if (!scope_stack_.Contains(count->value)) {
            AddError(var) << NameOf(count->value) << " is not in scope";
        }
    }

    if (var->Initializer()) {
        if (!CheckOperand(var, ir::Var::kInitializerOperandOffset)) {
            return;
        }
    }

    // TODO(516717234): Move to functional validator
    CheckBindingPoint(var, var->Result(0)->Type(), var->Attributes(),
                      ShaderIOKind::kModuleScopeVar);

    auto address_space = mv->AddressSpace();
    if (address_space != AddressSpace::kIn && address_space != AddressSpace::kOut) {
        CheckInterpolation(var, mv->StoreType(), var->Attributes(),
                           Function::PipelineStage::kUndefined, IODirection::kResource);
    }

    // TODO(516717234): Move to functional validator
    if (var->Block() == ir_.root_block) {
        if (mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut) {
            ValidateShaderIOAnnotations(var, var->Result()->Type(), var->BindingPoint(),
                                        var->Attributes(), ShaderIOKind::kModuleScopeVar);
        }
    }
}

const ir::Function* Structural::ContainingFunction(const ir::Instruction* inst) {
    if (inst->Block() == ir_.root_block) {
        return nullptr;
    }

    return block_to_function_.GetOrAdd(inst->Block(), [&] {  //
        return ContainingFunction(inst->Block()->Parent());
    });
}

Hashset<const ir::Function*, 4> Structural::ContainingEndPoints(const ir::Function* f) {
    if (!f) {
        return {};
    }

    Hashset<const ir::Function*, 4> result{};
    Hashset<const ir::Function*, 4> visited{f};

    auto call_sites = user_func_calls_.GetOr(f, Hashset<const ir::UserCall*, 4>()).Vector();
    while (!call_sites.IsEmpty()) {
        auto call_site = call_sites.Pop();
        auto calling_function = ContainingFunction(call_site);
        if (!calling_function) {
            continue;
        }

        if (visited.Contains(calling_function)) {
            continue;
        }
        visited.Add(calling_function);

        if (calling_function->IsEntryPoint()) {
            result.Add(calling_function);
        }

        for (auto new_call_sites : user_func_calls_.GetOr(f, Hashset<const ir::UserCall*, 4>())) {
            call_sites.Push(new_call_sites);
        }
    }

    return result;
}

void Structural::CheckBlendSrc(BlendSrcContext& ctx,
                               const CastableBase* target,
                               const core::type::Type* ty,
                               const IOAttributes& attr) {
    if (attr.blend_src.has_value()) {
        if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
            AddError(target) << "blend_src cannot be used on non-struct-member types";
        }
        CheckBlendSrcImpl(ctx, target, ty, attr);
    }

    if (auto* s = ty->As<core::type::Struct>()) {
        if (s->Members().Any([](auto* m) { return m->Attributes().blend_src.has_value(); })) {
            auto location_count = 0u;
            for (const auto* mem : s->Members()) {
                auto& mem_attr = mem->Attributes();
                if (mem_attr.location.has_value()) {
                    location_count++;
                }
                CheckBlendSrcImpl(ctx, target, mem->Type(), mem_attr);
            }

            if (location_count != 2) {
                AddError(target)
                    << "structs with blend_src members must have exactly 2 members with "
                       "location annotations";
            }
            return;
        }
    }

    // Reject blend_src on nested members
    if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
        WalkTypeAndMembers(
            ctx, ty, attr,
            [&target, this](BlendSrcContext&, const core::type::Type*, const IOAttributes& a) {
                if (a.blend_src.has_value()) {
                    AddError(target)
                        << "blend_src cannot be used on members of non-top level structs";
                }
            });
    }
}

void Structural::CheckBlendSrcImpl(BlendSrcContext& ctx,
                                   const CastableBase* target,
                                   const core::type::Type* ty,
                                   const IOAttributes& attr) {
    if (!attr.blend_src.has_value()) {
        return;
    }

    auto bs_val = attr.blend_src.value();
    if (bs_val != 0 && bs_val != 1) {
        AddError(target) << "blend_src value must be 0 or 1";
    }
    if (!ctx.blend_srcs.Add(bs_val)) {
        AddError(target) << "duplicate blend_src(" << bs_val << ") on entry point "
                         << ToString(ctx.dir);
    }

    if (ctx.dir != IODirection::kOutput || ctx.stage != Function::PipelineStage::kFragment) {
        AddError(target) << "blend_src can only be used on fragment shader outputs";
        return;
    }
    if (!attr.location.has_value() || attr.location.value() != 0) {
        AddError(target) << "struct members with blend_src must be located at 0";
    }

    if (!ctx.blend_src_type) {
        if (!ty->IsNumericScalarOrVector()) {
            AddError(target) << "blend_src must be a numeric scalar or vector, but has type "
                             << ty->FriendlyName();
        }
        ctx.blend_src_type = ty;
    } else if (ctx.blend_src_type != ty) {
        AddError(target) << "blend_src type " << ty->FriendlyName()
                         << " does not match other blend_src type "
                         << ctx.blend_src_type->FriendlyName();
    }
}

void Structural::CheckLocation(Hashmap<uint32_t, const CastableBase*, 4>& locations,
                               const CastableBase* target,
                               const IOAttributes& attr,
                               const Function::PipelineStage stage,
                               const core::type::Type* type,
                               const IODirection dir) {
    struct WalkContext {
        Structural* validator;
        Hashmap<uint32_t, const CastableBase*, 4>& locations;
        const CastableBase* target;
        const Function::PipelineStage stage;
        const IODirection dir;
    };
    WalkContext ctx{this, locations, target, stage, dir};

    WalkTypeAndMembers(
        ctx, type, attr,
        [](WalkContext& context, const core::type::Type* ty, const IOAttributes& attribute) {
            if (ty->Is<core::type::Struct>()) {
                return;
            }

            if (attribute.blend_src) {
                // locations associated with a blend_src usage should already be
                // pre-populated in locations
                return;
            }

            if (attribute.location.has_value()) {
                if (context.stage == Function::PipelineStage::kCompute &&
                    context.dir == IODirection::kInput) {
                    context.validator->AddError(context.target)
                        << "location attribute is not valid for compute shader inputs";
                }

                auto loc = attribute.location.value();
                if (const auto conflict = context.locations.Get(loc)) {
                    context.validator->AddError(context.target)
                        << "duplicate location(" << loc << ") on entry point "
                        << ToString(context.dir);
                    context.validator->AddDeclarationNote(*conflict.value);
                } else {
                    context.locations.Add(loc, context.target);
                }
            }
        });
}

void Structural::CheckInterpolation(const CastableBase* anchor,
                                    const core::type::Type* ty,
                                    const IOAttributes& attr,
                                    const Function::PipelineStage stage,
                                    const IODirection dir) {
    if (!ty) {
        return;
    }

    bool ctx = false;

    WalkTypeAndMembers(
        ctx, ty, attr,
        [this, anchor, stage, dir](bool& in_location_composite, const core::type::Type* t,
                                   const IOAttributes& a) {
            bool has_location = a.location.has_value() || in_location_composite;
            if (!has_location) {
                if (auto* str = t->As<core::type::Struct>()) {
                    has_location |= str->Members().All(
                        [](const auto* mem) { return mem->Attributes().location.has_value(); });
                }
            }

            if (a.interpolation.has_value()) {
                has_location |= (ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO) &&
                                 a.builtin.has_value());

                if (!ir_.properties.Contains(Property::kAllowLocationForNumericComposites) &&
                    t->As<core::type::Struct>()) {
                    AddError(anchor) << "interpolation cannot be applied to a struct without "
                                        "'kAllowLocationForNumericComposites' property";
                }

                if (t->IsIntegerScalarOrVector()) {
                    if (a.interpolation.value().type != InterpolationType::kFlat) {
                        AddError(anchor)
                            << "interpolation attribute type must be flat for integral types";
                    }
                }

                auto interp_type = a.interpolation.value().type;
                auto interp_sampling = a.interpolation.value().sampling;
                if (interp_sampling != InterpolationSampling::kUndefined) {
                    switch (interp_type) {
                        case InterpolationType::kFlat:
                            if (interp_sampling != InterpolationSampling::kFirst &&
                                interp_sampling != InterpolationSampling::kEither) {
                                AddError(anchor) << "flat interpolation can only use 'first', "
                                                    "'either' or undefined sampling parameters";
                            }
                            break;
                        case InterpolationType::kLinear:
                        case InterpolationType::kPerspective:
                            if (interp_sampling != InterpolationSampling::kCenter &&
                                interp_sampling != InterpolationSampling::kCentroid &&
                                interp_sampling != InterpolationSampling::kSample) {
                                AddError(anchor) << "linear and perspective interpolation can only "
                                                    "use 'center', 'centroid', 'sample', or "
                                                    "undefined sampling parameters";
                            }
                            break;
                        case InterpolationType::kUndefined:
                            AddError(anchor) << "undefined interpolation should on have an "
                                                "undefined sampling parameter";
                            break;
                        default:
                            TINT_UNREACHABLE();
                    }
                }

                if (!has_location) {
                    if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
                        AddError(anchor) << "interpolation attribute requires a location attribute";
                    } else {
                        AddError(anchor) << "interpolation attribute requires a location attribute "
                                            "(or location-like shader I/O annotation)";
                    }
                }
            } else if (has_location && t->IsIntegerScalarOrVector()) {
                // Integral vertex outputs and fragment inputs require flat interpolation.
                const bool needs_flat =
                    (stage == Function::PipelineStage::kVertex && dir == IODirection::kOutput) ||
                    (stage == Function::PipelineStage::kFragment && dir == IODirection::kInput);
                if (needs_flat) {
                    AddError(anchor) << "integral user-defined inputs and outputs must have an "
                                        "@interpolate(flat) attribute";
                }
            }

            if (t->IsAnyOf<core::type::Array, core::type::Struct>()) {
                in_location_composite |= a.location.has_value();
            }
        });
}

void Structural::CheckBindingPoint(const CastableBase* anchor,
                                   const core::type::Type* ty,
                                   const IOAttributes& attr,
                                   const ShaderIOKind& io_kind) {
    const auto& binding_point = attr.binding_point;
    auto address_space = AddressSpace::kUndefined;
    if (const auto* mv = ty->As<core::type::MemoryView>()) {
        address_space = mv->AddressSpace();
    } else {
        // ModuleScopeVars transform in MSL backends unwraps pointers to handles
        if (ty->IsHandle()) {
            address_space = AddressSpace::kHandle;
        }
    }

    if (binding_point.has_value() && io_kind != ShaderIOKind::kModuleScopeVar &&
        !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
        AddError(anchor) << "binding_points are only valid on resource variables";
    }

    switch (address_space) {
        case AddressSpace::kHandle:
            if (!binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " resource requires a binding point";
            }
            break;
        case AddressSpace::kStorage:
        case AddressSpace::kUniform:
            if (!binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " resource requires a binding point";
            }
            break;
        default:
            if (binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " non-resource cannot have a binding point";
            }
            break;
    }
}

void Structural::ValidateShaderIOAnnotations(const CastableBase* msg_anchor,
                                             const core::type::Type* ty,
                                             const std::optional<BindingPoint>& binding_point,
                                             const IOAttributes& attr,
                                             ShaderIOKind kind) {
    EnumSet<IOAnnotation> annotations;

    // Since there is no entries in the set at this point, this should never fail.
    TINT_ASSERT(AddIOAnnotationsFromIOAttributes(annotations, attr) == Success);

    if (binding_point.has_value()) {
        annotations.Add(IOAnnotation::kBindingPoint);
    }

    if (auto* mv = ty->As<core::type::MemoryView>()) {
        if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
            annotations.Add(IOAnnotation::kWorkgroup);
        }
    }

    if (ty->Is<core::type::Void>()) {
        if (!annotations.Empty()) {
            AddError(msg_anchor) << ToString(kind) << " with void type should never be annotated";
        }
        return;  // Early return because later rules assume non-void types.
    }

    if (attr.location.has_value()) {
        if (ir_.properties.Contains(Property::kAllowLocationForNumericComposites)) {
            std::function<bool(const core::type::Type*)> is_numeric =
                [&is_numeric](const core::type::Type* t) -> bool {
                t = t->UnwrapPtrOrRef();
                bool result = false;
                tint::Switch(
                    t,
                    [&](const core::type::Struct* s) {
                        for (auto* m : s->Members()) {
                            if (!is_numeric(m->Type())) {
                                return;
                            }
                        }
                        result = true;
                    },
                    [&](Default) {
                        auto* e = t->DeepestElement()->UnwrapPtrOrRef();
                        tint::Switch(
                            e,  //
                            [&](const core::type::Struct* s) { result = is_numeric(s); },
                            [&](Default) { result = e->IsNumericScalarOrVector(); });
                    });
                return result;
            };
            if (!is_numeric(ty)) {
                AddError(msg_anchor)
                    << ToString(kind)
                    << " with a location attribute must contain only numeric elements "
                    << ty->FriendlyName();
                return;
            }
        } else {
            if (!ty->UnwrapPtrOrRef()->IsNumericScalarOrVector()) {
                AddError(msg_anchor) << ToString(kind)
                                     << " with a location attribute must be a numeric scalar or "
                                        "vector, but has type "
                                     << ty->FriendlyName();
                return;
            }
        }
    }

    if (auto* ty_struct = ty->UnwrapPtrOrRef()->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            EnumSet<IOAnnotation> mem_annotations = annotations;
            auto add_result = AddIOAnnotationsFromIOAttributes(mem_annotations, mem->Attributes());
            if (add_result != Success) {
                AddError(msg_anchor)
                    << ToString(kind)
                    << " struct member has same IO annotation, as top-level struct, '"
                    << ToString(add_result.Failure()) << "'";
                return;
            }

            if (!CheckStructMemberAttributes(mem, [&]() -> diag::Diagnostic& {
                    return AddError(msg_anchor) << ToString(kind) << " ";
                })) {
                return;
            }

            if (ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
                if (auto* mv = mem->Type()->As<core::type::MemoryView>()) {
                    if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
                        mem_annotations.Add(IOAnnotation::kWorkgroup);
                    }
                }
            }

            if (mem_annotations.Empty()) {
                AddError(msg_anchor) << ToString(kind)
                                     << " struct members must have at least one IO annotation, "
                                        "e.g. a binding point, a location, etc";
            } else if (mem_annotations.Size() > 1) {
                AddError(msg_anchor)
                    << ToString(kind) << " struct member has more than one IO annotation, "
                    << ToString(mem_annotations);
            }
        }
    } else {
        if (annotations.Empty()) {
            if (!(ir_.properties.Contains(Property::kAllowUnannotatedModuleIOVariables) &&
                  kind == ShaderIOKind::kModuleScopeVar)) {
                AddError(msg_anchor) << ToString(kind)
                                     << " must have at least one IO annotation, e.g. a binding "
                                        "point, a location, etc";
            }
        } else if (annotations.Size() > 1) {
            AddError(msg_anchor) << ToString(kind) << " has more than one IO annotation, "
                                 << ToString(annotations);
        }
    }
}

bool Structural::CheckStructMemberAttributes(const core::type::StructMember* member,
                                             std::function<diag::Diagnostic&()> make_diag) {
    const auto checkers = IOAttributeCheckersFor(member->Attributes(), /*skip_builtins*/ false);
    for (const auto* checker : checkers) {
        auto res = checker->check(member->Type(), member->Attributes(), ir_.properties,
                                  IOAttributeUsage::kUndefinedUsage);
        if (res != Success) {
            make_diag() << res.Failure();
            return false;
        }
        if (!checker->type_check(member->Type(), ir_.properties)) {
            make_diag() << ToString(checker->kind) << " " << checker->type_error;
            return false;
        }
    }

    if (member->Attributes().location.has_value()) {
        if (ir_.properties.Contains(Property::kAllowLocationForNumericComposites)) {
            if (!member->Type()->UnwrapPtrOrRef()->IsNumericScalarOrVector() &&
                !member->Type()->UnwrapPtrOrRef()->Is<core::type::Struct>()) {
                make_diag() << "struct member with a location attribute must be a numeric scalar, "
                               "a numeric vector or a struct, but has type "
                            << member->Type()->FriendlyName();
                return false;
            }
        } else {
            if (!member->Type()->UnwrapPtrOrRef()->IsNumericScalarOrVector()) {
                make_diag() << "struct member with a location attribute must be "
                               "a numeric scalar or vector, but has type "
                            << member->Type()->FriendlyName();
                return false;
            }
        }
    }
    return true;
}
void Structural::CheckLet(const Let* l) {
    CheckResultsAndOperands(l, Let::kNumResults, Let::kNumOperands);
}

void Structural::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                            //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },              //
        [&](const MemberBuiltinCall* c) { CheckMemberBuiltinCall(c); },  //
        [&](const Construct* c) { CheckConstruct(c); },                  //
        [&](const Convert* c) { CheckConvert(c); },                      //
        [&](const Discard* d) {                                          //
            stage_restricted_instructions_.Add(
                d, SupportedStages{Function::PipelineStage::kFragment});        //
            CheckDiscard(d);                                                    //
        },                                                                      //
        [&](const UserCall* c) {                                                //
            if (c->Target()) {                                                  //
                auto calls =                                                    //
                    user_func_calls_.GetOr(c->Target(),                         //
                                           Hashset<const ir::UserCall*, 4>{});  //
                calls.Add(c);                                                   //
                user_func_calls_.Replace(c->Target(), calls);                   //
            }
            CheckUserCall(c);
        },
        [&](Default) {
            // Validation of custom IR instructions
        });
}

void Structural::CheckBuiltinCall(const BuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    if (!CheckResults(call, BuiltinCall::kNumResults) || !CheckOperands(call)) {
        return;
    }

    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });

    intrinsic::Context context{call->TableData(), type_mgr_, symbols_};
    auto builtin = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                             call->ExplicitTemplateParams(), args,
                                             core::EvaluationStage::kRuntime);
    if (builtin != Success) {
        AddError(call) << builtin.Failure();
        return;
    }

    // Track the stages that this builtin call is limited to, so that we can check them against the
    // entry points that they are used from.
    SupportedStages stages;
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsComputePipeline)) {
        stages.Add(Function::PipelineStage::kCompute);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsFragmentPipeline)) {
        stages.Add(Function::PipelineStage::kFragment);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsVertexPipeline)) {
        stages.Add(Function::PipelineStage::kVertex);
    }
    stage_restricted_instructions_.Add(call, stages);

    const core::ir::CoreBuiltinCall* bc = call->As<CoreBuiltinCall>();
    if (bc == nullptr) {
        return;
    }
    CheckCoreBuiltinCall(bc);
}

void Structural::CheckCoreBuiltinCall(const CoreBuiltinCall* call) {
    if (ir_.properties.Contains(Property::kDisallowVectorMinMaxClamp)) {
        switch (call->Func()) {
            case core::BuiltinFn::kClamp:
            case core::BuiltinFn::kMax:
            case core::BuiltinFn::kMin:
                if (call->Result()->Type()->Is<core::type::Vector>()) {
                    AddError(call) << "vector " << call->FriendlyName()
                                   << " disallowed by the DisallowVectorMinMaxClamp property";
                }
                break;
            default:
                break;
        }
    }
    if (ir_.properties.Contains(Property::kAllowBufferTypes)) {
        switch (call->Func()) {
            case core::BuiltinFn::kBufferArrayView:
                if (call->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
                    AddError(call)
                        << call->FriendlyName() << " result type must not have a fixed footprint";
                }
                break;
            default:
                break;
        }
    }
}

void Structural::CheckMemberBuiltinCall(const MemberBuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    CheckResults(call, MemberBuiltinCall::kNumResults) || !CheckOperands(call);
}

void Structural::CheckConstruct(const Construct* construct) {
    CheckResultsAndOperandRange(construct, Construct::kNumResults, Construct::kMinOperands);
}

void Structural::CheckConvert(const Convert* convert) {
    CheckResultsAndOperands(convert, Convert::kNumResults, Convert::kNumOperands);
}

void Structural::CheckDiscard(const tint::core::ir::Discard* discard) {
    CheckResultsAndOperands(discard, Discard::kNumResults, Discard::kNumOperands);
}

void Structural::CheckUserCall(const UserCall* call) {
    CheckResultsAndOperandRange(call, UserCall::kNumResults, UserCall::kMinOperands);

    if (!call->Target()) {
        AddError(call, UserCall::kFunctionOperandOffset) << "target not defined or not a function";
        return;
    }
}

void Structural::CheckAccess(const Access* a) {
    CheckResultsAndOperandRange(a, Access::kNumResults, Access::kMinNumOperands);
}

void Structural::CheckBinary(const Binary* b) {
    if (!CheckResultsAndOperands(b, Binary::kNumResults, Binary::kNumOperands)) {
        return;
    }
    if (b->Op() == core::BinaryOp::kLogicalAnd) {
        AddError(b) << "logical-and is not valid in the IR";
        return;
    }
    if (b->Op() == core::BinaryOp::kLogicalOr) {
        AddError(b) << "logical-or is not valid in the IR";
        return;
    }
}

void Structural::CheckUnary(const Unary* u) {
    CheckResultsAndOperands(u, Unary::kNumResults, Unary::kNumOperands);
}

void Structural::CheckIf(const If* if_) {
    CheckResults(if_);
    CheckOperands(if_, If::kNumOperands);

    if (if_->False() && if_->False()->Is<core::ir::MultiInBlock>()) {
        AddError(if_) << "if false block must be a block";
    }
    if (if_->True() && if_->True()->Is<core::ir::MultiInBlock>()) {
        AddError(if_) << "if true block must be a block";
    }

    if (auto* constexpr_if = if_->As<core::ir::ConstExprIf>()) {
        if (constexpr_if->Results().Length() != 1) {
            AddError(constexpr_if) << "constexpr_if must have exactly one result";
        } else if (!constexpr_if->Result(0)->Type()->Is<core::type::Bool>()) {
            AddError(constexpr_if) << "constexpr_if result type must be 'bool'";
        }
        if (constexpr_if->False()->IsEmpty()) {
            AddError(constexpr_if) << "constexpr_if must have a false block";
        }
    }

    tasks_.Push([this] { control_stack_.Pop(); });

    if (!if_->False()->IsEmpty()) {
        QueueBlock(if_->False());
    }

    QueueBlock(if_->True());

    tasks_.Push([this, if_] { control_stack_.Push(if_); });
}

void Structural::CheckLoop(const Loop* l) {
    CheckResults(l);
    CheckOperands(l, 0);

    if (l->Initializer()->Is<core::ir::MultiInBlock>()) {
        AddError(l->Initializer()) << "loop initializer must be a block";
    }

    if (!l->Initializer()->IsEmpty()) {
        if (!l->Initializer()->Terminator() ||
            !l->Initializer()->Terminator()->Is<core::ir::NextIteration>()) {
            AddError(l->Initializer()) << "loop initializer must have a NextIteration terminator";
        }
    }

    // Note: Tasks are queued in reverse order of their execution
    tasks_.Push([this] { control_stack_.Pop(); });
    if (!l->Initializer()->IsEmpty()) {
        tasks_.Push([this] { EndBlock(); });
    }
    tasks_.Push([this] { EndBlock(); });
    if (!l->Continuing()->IsEmpty()) {
        tasks_.Push([this, l] {
            if (!l->Continuing()->Terminator()->IsAnyOf<NextIteration, BreakIf>()) {
                AddError(l->Continuing())
                    << "loop continuing terminator can only be next_iteration or break_if";
            }
            EndBlock();
        });
    }

    // ⎡Initializer              ⎤
    // ⎢    ⎡Body               ⎤⎥
    // ⎣    ⎣    [Continuing ]  ⎦⎦

    if (!l->Continuing()->IsEmpty()) {
        tasks_.Push([this, l] { BeginBlock(l->Continuing()); });
    } else if (!l->Continuing()->Params().IsEmpty()) {
        AddError(l) << "loop continuing block has parameters but is empty";
    }

    tasks_.Push([this, l] {
        CheckLoopBody(l);
        BeginBlock(l->Body());
    });
    if (!l->Initializer()->IsEmpty()) {
        tasks_.Push([this, l] { BeginBlock(l->Initializer()); });
    }
    tasks_.Push([this, l] { control_stack_.Push(l); });
}

void Structural::CheckLoopBody(const Loop* loop) {
    // If the body block has parameters, there must be an initializer block.
    if (!loop->Body()->Params().IsEmpty()) {
        if (!loop->HasInitializer()) {
            AddError(loop) << "loop with body block parameters must have an initializer";
        }
    }
}

void Structural::CheckSwitch(const Switch* s) {
    CheckResults(s);
    CheckOperands(s, Switch::kNumOperands);

    tasks_.Push([this] { control_stack_.Pop(); });

    for (auto& cse : s->Cases()) {
        if (cse.selectors.IsEmpty()) {
            AddError(s) << "case does not have any selectors";
        }
        if (cse.block->Is<core::ir::MultiInBlock>()) {
            AddError(s) << "case block must be a block";
        }
        QueueBlock(cse.block);
    }

    tasks_.Push([this, s] { control_stack_.Push(s); });
}

void Structural::CheckSwizzle(const Swizzle* s) {
    CheckResultsAndOperands(s, Swizzle::kNumResults, Swizzle::kNumOperands);
}

void Structural::CheckTerminator(const Terminator* b) {
    // All terminators should have zero results
    if (!CheckResults(b, 0)) {
        return;
    }

    // Operands must be alive and in scope if they are not nullptr.
    if (!CheckOperands(b)) {
        return;
    }

    tint::Switch(
        b,                                                           //
        [&](const ir::BreakIf* i) { CheckBreakIf(i); },              //
        [&](const ir::Continue* c) { CheckContinue(c); },            //
        [&](const ir::Exit* e) { CheckExit(e); },                    //
        [&](const ir::NextIteration* n) { CheckNextIteration(n); },  //
        [&](const ir::Return* ret) { CheckReturn(ret); },            //
        [&](const ir::TerminateInvocation*) {},                      //
        [&](const ir::Unreachable* u) { CheckUnreachable(u); },      //
        TINT_ICE_ON_NO_MATCH);

    if (b->next) {
        AddError(b) << "must be the last instruction in the block";
    }
}

void Structural::CheckBreakIf(const BreakIf* b) {
    auto* loop = b->Loop();
    if (loop == nullptr) {
        AddError(b) << "has no associated loop";
        return;
    }
    if (b->Condition() == nullptr) {
        AddError(b) << "break_if condition cannot be nullptr";
        return;
    }

    auto next_iter_values = b->NextIterValues();
    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(b, b->ArgsOperandOffset(), next_iter_values.size(), body,
                                 body->Params());
    }

    auto exit_values = b->ExitValues();
    CheckOperandsMatchTarget(b, b->ArgsOperandOffset() + next_iter_values.size(),
                             exit_values.size(), loop, loop->Results());
}

void Structural::CheckContinue(const Continue* c) {
    auto* loop = c->Loop();
    if (loop == nullptr) {
        AddError(c) << "has no associated loop";
        return;
    }
    if (!TransitivelyHolds(loop->Body(), c)) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(c) << "must only be called from loop body";
        } else {
            AddError(c) << "called outside of associated loop";
        }
    }

    if (auto* cont = loop->Continuing()) {
        CheckOperandsMatchTarget(c, Continue::kArgsOperandOffset, c->Args().size(), cont,
                                 cont->Params());
    }
}

void Structural::CheckExit(const Exit* e) {
    if (control_stack_.IsEmpty()) {
        AddError(e) << "found outside all control instructions";
        return;
    }
    if (e->ControlInstruction() == nullptr) {
        AddError(e) << "has no parent control instruction";
        return;
    }

    auto args = e->Args();
    CheckOperandsMatchTarget(e, e->ArgsOperandOffset(), args.size(), e->ControlInstruction(),
                             e->ControlInstruction()->Results());

    tint::Switch(
        e,                                                     //
        [&](const ir::ExitIf* i) { CheckExitIf(i); },          //
        [&](const ir::ExitLoop* l) { CheckExitLoop(l); },      //
        [&](const ir::ExitSwitch* s) { CheckExitSwitch(s); },  //
        TINT_ICE_ON_NO_MATCH);
}

void Structural::CheckNextIteration(const NextIteration* n) {
    auto* loop = n->Loop();
    if (loop == nullptr) {
        AddError(n) << "has no associated loop";
        return;
    }

    if (loop->Initializer() != n->Block() && loop->Continuing() != n->Block()) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(n) << "must only be called directly from loop initializer or continuing";
        } else {
            AddError(n) << "called outside of associated loop";
        }
    }

    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(n, NextIteration::kArgsOperandOffset, n->Args().size(), body,
                                 body->Params());
    }
}

void Structural::CheckExitIf(const ExitIf* e) {
    if (control_stack_.Back() != e->If()) {
        AddError(e) << "if target jumps over other control instructions";
        AddNote(control_stack_.Back()) << "first control instruction jumped";
    }
}

void Structural::CheckReturn(const Return* ret) {
    if (!CheckOperands(ret, Return::kMinOperands, Return::kMaxOperands)) {
        return;
    }

    auto* func = ret->Func();
    if (func == nullptr) {
        // Func() returning nullptr after CheckResultsAndOperandRange is due to the first
        // operand being not a function
        AddError(ret) << "expected function for first operand";
        return;
    }

    if (func != ContainingFunction(ret)) {
        AddError(ret) << "function operand does not match containing function";
        return;
    }
}

void Structural::CheckUnreachable(const Unreachable* u) {
    CheckResultsAndOperands(u, Unreachable::kNumResults, Unreachable::kNumOperands);
}

void Structural::CheckControlsAllowingIf(const Exit* exit, const Instruction* control) {
    bool found = false;
    for (auto ctrl : tint::Reverse(control_stack_)) {
        if (ctrl == control) {
            found = true;
            break;
        }
        // A exit switch can step over if instructions, but no others.
        if (!ctrl->Is<ir::If>()) {
            AddError(exit) << control->FriendlyName()
                           << " target jumps over other control instructions";
            AddNote(ctrl) << "first control instruction jumped";
            return;
        }
    }
    if (!found) {
        AddError(exit) << control->FriendlyName() << " not found in parent control instructions";
    }
}

void Structural::CheckExitSwitch(const ExitSwitch* s) {
    CheckControlsAllowingIf(s, s->ControlInstruction());
}

void Structural::CheckExitLoop(const ExitLoop* l) {
    CheckControlsAllowingIf(l, l->ControlInstruction());
}

void Structural::CheckLoad(const Load* l) {
    CheckResultsAndOperands(l, Load::kNumResults, Load::kNumOperands);
}

void Structural::CheckStore(const Store* s) {
    CheckResultsAndOperands(s, Store::kNumResults, Store::kNumOperands);
}

void Structural::CheckLoadVectorElement(const LoadVectorElement* l) {
    CheckResultsAndOperands(l, LoadVectorElement::kNumResults, LoadVectorElement::kNumOperands);
}

void Structural::CheckStoreVectorElement(const StoreVectorElement* s) {
    CheckResultsAndOperands(s, StoreVectorElement::kNumResults, StoreVectorElement::kNumOperands);
}

void Structural::CheckPhony(const Phony* p) {
    if (!ir_.properties.Contains(Property::kAllowPhonyInstructions)) {
        AddError(p) << "missing property 'kAllowPhonyInstructions'";
        return;
    }

    if (!CheckResultsAndOperands(p, Phony::kNumResults, Phony::kNumOperands)) {
        return;
    }
}

void Structural::CheckOperandsMatchTarget(const Instruction* source_inst,
                                          size_t source_operand_offset,
                                          size_t source_operand_count,
                                          const CastableBase* target,
                                          VectorRef<const Value*> target_values) {
    if (source_operand_count != target_values.Length()) {
        auto values = [&](size_t n) { return n == 1 ? " value" : " values"; };
        AddError(source_inst) << "provides " << source_operand_count << values(source_operand_count)
                              << " but " << NameOf(target) << " expects " << target_values.Length()
                              << values(target_values.Length());
        AddDeclarationNote(target);
    }
    size_t count = std::min(source_operand_count, target_values.Length());
    for (size_t i = 0; i < count; i++) {
        auto* source_value = source_inst->Operand(source_operand_offset + i);
        auto* target_value = target_values[i];
        if (!source_value || !target_value) {
            continue;  // Caller should be checking operands are not null
        }
        auto* source_type = source_value->Type();
        auto* target_type = target_value->Type();
        if (source_type != target_type) {
            AddError(source_inst, source_operand_offset + i)
                << "operand with type " << NameOf(source_type) << " does not match "
                << NameOf(target) << " target type " << NameOf(target_type);
            AddDeclarationNote(target_value);
        }
    }
}

}  // namespace tint::core::ir::validator
