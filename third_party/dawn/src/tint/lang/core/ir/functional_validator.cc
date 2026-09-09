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

#include "src/tint/lang/core/ir/functional_validator.h"

#include <utility>

#include "src/tint/lang/core/intrinsic/dialect.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/swizzle_view.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::core::ir::validator {
namespace {

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

/// @returns true if the type or any contained types are of type T
/// @param ty root of the types to walk
template <typename T>
bool ContainsType(const core::type::Type* ty) {
    bool found = false;
    WalkTypeAndMembers(found, ty, IOAttributes{},
                       [&](bool& ctx, const core::type::Type* t, const IOAttributes&) {
                           if (t != nullptr && t->DeepestElement()->Is<T>()) {
                               ctx = true;
                           }
                       });
    return found;
}

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

/// @returns true if @p ty meets the basic function parameter rules (i.e. one of constructible,
///          pointer, handle).
///
/// Note: Does not handle corner cases like if certain properties are present.
bool IsValidFunctionParamType(const core::type::Type* ty) {
    if (ty->IsConstructible() || ty->IsHandle()) {
        return true;
    }

    if (auto* ptr = ty->As<core::type::Pointer>()) {
        return ptr->AddressSpace() != core::AddressSpace::kHandle;
    }
    return false;
}

/// @returns true if @p ty is a non-struct and decorated with @builtin(position), or if it is a
/// struct and one of its members is decorated, otherwise false.
/// @param attr attributes attached to data
/// @param ty type of the data being tested
bool IsPositionPresent(const IOAttributes& attr, const core::type::Type* ty) {
    if (auto* ty_struct = ty->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            if (mem->Attributes().builtin == BuiltinValue::kPosition) {
                return true;
            }
        }
        return false;
    }

    return attr.builtin == BuiltinValue::kPosition;
}

}  // namespace

Functional::Functional(const Module& ir, diag::List& diagnostics, ErrorSource error_source)
    : ir_(ir), diag_(diagnostics), error_source_(error_source), referenced_module_vars_(ir) {}

Functional::~Functional() = default;

void Functional::Validate() {
    CheckRootBlock(ir_.root_block);

    for (const Function* func : ir_.functions) {
        CheckFunction(func);
    }
}

Disassembler& Functional::Disassemble() {
    TINT_ASSERT(error_source_ == ErrorSource::kIr);

    if (!disassembler_) {
        disassembler_.emplace(ir::Disassembler(ir_));
    }
    return *disassembler_;
}

StyledText Functional::NameOf(const core::type::Type* ty) {
    auto name = ty ? ty->FriendlyName() : "undef";
    return StyledText{} << style::Type(name);
}

StyledText Functional::NameOf(const Value* value) {
    if (error_source_ == ErrorSource::kWgsl) {
        return StyledText{} << ir_.NameOf(value).to_str();
    }
    return Disassemble().NameOf(value);
}

uint64_t Functional::ElementsCount(const core::type::Type* root_ty) {
    TINT_ASSERT(root_ty);

    Vector<const core::type::Type*, 16> stack;
    stack.Push(root_ty);

    while (!stack.IsEmpty()) {
        const core::type::Type* ty = stack.Back();

        if (elements_counts_.Contains(ty)) {
            stack.Pop();
            continue;
        }

        bool children_ready = true;
        uint64_t count = 0;

        tint::Switch(
            ty,
            [&](const core::type::Struct* s) {
                for (auto* member : s->Members()) {
                    if (auto res = elements_counts_.Get(member->Type())) {
                        count += *res;
                    } else {
                        stack.Push(member->Type());
                        children_ready = false;
                    }
                }
            },
            [&](const core::type::Array* a) {
                if (auto res = elements_counts_.Get(a->ElemType())) {
                    uint64_t array_count = 0;
                    if (auto* const_count = a->Count()->As<core::type::ConstantArrayCount>()) {
                        array_count = const_count->value;
                    }
                    count = array_count * (*res);
                } else {
                    stack.Push(a->ElemType());
                    children_ready = false;
                }
            },
            [&](const core::type::Matrix* m) {
                if (auto res = elements_counts_.Get(m->ColumnType())) {
                    count = static_cast<uint64_t>(m->Columns()) * (*res);
                } else {
                    stack.Push(m->ColumnType());
                    children_ready = false;
                }
            },
            [&](const core::type::Vector* v) { count = static_cast<uint64_t>(v->Width()); },
            [&](Default) { count = 1; });

        if (children_ready) {
            elements_counts_.Add(ty, count);
            stack.Pop();
        }
    }

    return *elements_counts_.Get(root_ty);
}

Source Functional::SourceOf(const Function* func) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(func);
    }
    return Disassemble().FunctionSource(func);
}

Source Functional::SourceOf(const FunctionParam* param) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(param);
    }
    return Disassemble().FunctionParamSource(param);
}

Source Functional::SourceOf(const Instruction* inst) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(inst);
    }
    return Disassemble().InstructionSource(inst);
}

Source Functional::SourceOf(const Instruction* inst, size_t idx) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(inst->Operands()[idx]);
    }
    return Disassemble().OperandSource(
        Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
}

diag::Diagnostic& Functional::AddError(Source src) {
    auto& diag = diag_.AddError(src);
    if (error_source_ == ErrorSource::kIr) {
        diag.owned_file = Disassemble().File();
    }
    return diag;
}

diag::Diagnostic& Functional::AddError(const Function* func) {
    return AddError(SourceOf(func));
}

diag::Diagnostic& Functional::AddError(const FunctionParam* param) {
    return AddError(SourceOf(param));
}

diag::Diagnostic& Functional::AddError(const Instruction* inst) {
    auto& diag = AddError(SourceOf(inst));
    if (error_source_ == ErrorSource::kWgsl) {
        return diag;
    }

    diag << inst->FriendlyName() << ": ";
    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference,
        // so we need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Functional::AddError(const Instruction* inst, size_t idx) {
    auto& diag = AddError(SourceOf(inst, idx));
    if (error_source_ == ErrorSource::kWgsl) {
        return diag;
    }

    diag << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Functional::AddNote(Source src) {
    auto& diag = diag_.AddNote(src);
    if (error_source_ == ErrorSource::kIr) {
        diag.owned_file = Disassemble().File();
    }
    return diag;
}

diag::Diagnostic& Functional::AddNote(const Block* blk) {
    TINT_ASSERT(error_source_ == ErrorSource::kIr);
    auto src = Disassemble().BlockSource(blk);
    return AddNote(src);
}

diag::Diagnostic& Functional::AddNote(const Function* func) {
    return AddNote(SourceOf(func));
}

diag::Diagnostic& Functional::AddNote(const Instruction* inst) {
    return AddNote(SourceOf(inst));
}

diag::Diagnostic& Functional::AddNote(const Instruction* inst, size_t idx) {
    return AddNote(SourceOf(inst, idx));
}

void Functional::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER({
        block_stack_.Pop();
        TINT_ASSERT(block_stack_.IsEmpty());
    });

    for (auto* inst : *blk) {
        CheckInstruction(inst);
    }
}

void Functional::CheckFunction(const Function* func) {
    CheckEntryPoint(func);

    // void needs to be filtered out, since it isn't constructible, but used in the IR when no
    // return is specified.
    if (DAWN_UNLIKELY(!func->ReturnType()->Is<core::type::Void>() &&
                      !func->ReturnType()->IsConstructible())) {
        AddError(func) << "function return type must be constructible";
    }

    for (auto* param : func->Params()) {
        CheckFunctionParam(param);
    }

    CheckBlock(func->Block());
}

void Functional::CheckEntryPoint(const Function* func) {
    if (!func->IsEntryPoint()) {
        return;
    }

    // Check that there is at most one entry point unless we allow multiple entry points.
    if (!ir_.properties.Contains(Property::kAllowMultipleEntryPoints) &&
        !entry_point_names_.IsEmpty()) {
        AddError(func) << "a module with multiple entry points requires the "
                          "AllowMultipleEntryPoints property";
        return;
    }

    if (DAWN_UNLIKELY(ir_.NameOf(func).Name().empty())) {
        AddError(func) << "entry points must have names";
    } else {
        // Checking the name early, so its usage can be recorded, even if the function is
        // malformed.
        const auto name = ir_.NameOf(func).Name();
        if (!entry_point_names_.Add(name)) {
            AddError(func) << "entry point name " << style::Function(name) << " is not unique";
        }
    }

    Hashset<BindingPoint, 4> binding_points{};
    bool seen_immediate = false;
    for (auto var : referenced_module_vars_.TransitiveReferences(func)) {
        if (!ir_.properties.Contains(Property::kAllowDuplicateBindings) &&
            var->BindingPoint().has_value()) {
            auto bp = var->BindingPoint().value();
            if (!binding_points.Add(bp)) {
                AddError(var) << "found non-unique binding point, " << bp
                              << ", being referenced in entry point, " << NameOf(func);
            }
        }

        const auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
        if (!mv) {
            continue;
        }

        auto address_space = mv->AddressSpace();
        switch (address_space) {
            case AddressSpace::kImmediate:
                if (seen_immediate) {
                    AddError(var) << "multiple user-declared immediate data variables referenced "
                                     "by entry point "
                                  << NameOf(func);
                }
                seen_immediate = true;
                continue;
            case AddressSpace::kWorkgroup:
                if (!func->IsCompute()) {
                    AddError(var) << "workgroup variable cannot be used in a " << func->Stage()
                                  << " shader";
                }
                continue;
            case AddressSpace::kPixelLocal:
                if (!func->IsFragment()) {
                    AddError(var) << "pixel_local variable cannot be used in a " << func->Stage()
                                  << " shader";
                }
                continue;
            case AddressSpace::kIn:
            case AddressSpace::kOut:
                break;
            default:
                continue;
        }
    }

    if (func->IsCompute()) {
        if (DAWN_UNLIKELY(!func->ReturnType()->Is<core::type::Void>())) {
            AddError(func) << "compute entry point must not have a return type, found "
                           << NameOf(func->ReturnType());
        }
    } else if (func->IsVertex()) {
        CheckPositionPresentForVertexOutput(func);
    }
}

void Functional::CheckPositionPresentForVertexOutput(const Function* ep) {
    if (IsPositionPresent(ep->ReturnAttributes(), ep->ReturnType())) {
        return;
    }

    for (const auto& var : referenced_module_vars_.TransitiveReferences(ep)) {
        const auto* ty = var->Result()->Type()->UnwrapPtrOrRef();
        if (!ty) {
            continue;
        }

        const auto attr = var->Attributes();
        if (IsPositionPresent(attr, ty)) {
            if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
                AddError(var) << "position as part of a `var`, it must be part of the return";
                AddNote(ep) << "used in entry point here";
                return;
            }
            return;
        }
    }
    AddError(ep) << "position must be declared on the return of a vertex entry point";
}

void Functional::CheckFunctionParam(const FunctionParam* param) {
    TINT_ASSERT(param->Function() != nullptr);

    bool func_is_entry_point = param->Function()->IsEntryPoint();

    if (!IsValidFunctionParamType(param->Type())) {
        auto ptr_ty = param->Type()->As<core::type::Pointer>();
        bool allowed_ptr_to_handle = ir_.properties.Contains(Property::kAllowPointerToHandle) &&
                                     ptr_ty != nullptr && ptr_ty->StoreType()->IsHandle();

        auto struct_ty = param->Type()->As<core::type::Struct>();
        if (!allowed_ptr_to_handle &&
            (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface) ||
             (struct_ty == nullptr) ||
             struct_ty->Members().Any([](const core::type::StructMember* m) {
                 return !IsValidFunctionParamType(m->Type());
             }))) {
            AddError(param) << "function parameter type, " << NameOf(param->Type())
                            << ", must be constructible, a pointer, or a handle";
        }
    }

    AddressSpace address_space = AddressSpace::kUndefined;
    auto* mv = param->Type()->As<core::type::MemoryView>();
    if (mv) {
        address_space = mv->AddressSpace();
    } else {
        // ModuleScopeVars transform in MSL backends unwraps pointers to handles
        if (param->Type()->IsHandle()) {
            address_space = AddressSpace::kHandle;
        }
    }

    if (address_space == AddressSpace::kPixelLocal) {
        if (!mv->StoreType()->Is<core::type::Struct>()) {
            AddError(param) << "pixel_local param must be of type struct";
        }
    }

    if (func_is_entry_point && !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
        if (param->Type()->Is<core::type::Pointer>()) {
            AddError(param) << "entry point parameters cannot be pointers";
        }

        if (mv && mv->Is<core::type::Pointer>() && address_space == AddressSpace::kWorkgroup) {
            AddError(param) << "input param to entry point cannot be a ptr in the 'workgroup' "
                               "address space";
        }
    }
}

void Functional::CheckBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER({ block_stack_.Pop(); });

    const Instruction* inst = blk->Instructions();
    while (inst != nullptr) {
        CheckInstruction(inst);
        inst = inst->next;
    }
}

void Functional::CheckInstruction(const Instruction* inst) {
    tint::Switch(
        inst,                                                              //
        [&](const Access* a) { CheckAccess(a); },                          //
        [&](const Binary* b) { CheckBinary(b); },                          //
        [&](const Call* c) { CheckCall(c); },                              //
        [&](const If* if_) { CheckIf(if_); },                              //
        [&](const Let* l) { CheckLet(l); },                                //
        [&](const Load* load) { CheckLoad(load); },                        //
        [&](const LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
        [&](const Loop* l) { CheckLoop(l); },                              //
        [&](const Override* o) { CheckOverride(o); },                      //
        [&](const Phony*) {},                                              //
        [&](const Store* s) { CheckStore(s); },                            //
        [&](const StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
        [&](const Switch* s) { CheckSwitch(s); },                          //
        [&](const Swizzle* s) { CheckSwizzle(s); },                        //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Var* var) { CheckVar(var); },                            //
        TINT_ICE_ON_NO_MATCH);

    // Only check alignment if the instruction passed previous checks.
    if (!diag_.ContainsErrors()) {
        CheckAlignment(inst);
    }
}

void Functional::CheckAlignment(const Instruction* inst) {
    auto align = inst->Alignment();
    if (align.has_value()) {
        if (inst->GetSideEffects().Size() == 0) {
            AddError(inst) << "alignment can only be set on memory instructions";
        }

        auto align_val = align.value();
        if (!tint::IsPowerOfTwo(align_val)) {
            AddError(inst) << "alignment (" << align_val << ") must be a power of 2";
        }

        if (align_val > 256) {
            AddError(inst) << "alignment (" << align_val << ") must be less than or equal to 256";
        }

        // TODO(b/544359162): Which other instructions should be checked?
        uint32_t natural_align = tint::Switch(
            inst,  //
            [&](const Load* ld) { return ld->Result()->Type()->Align(); },
            [&](const Store* st) { return st->From()->Type()->Align(); },
            [&](const LoadVectorElement* lve) { return lve->Result()->Type()->Align(); },
            [&](const StoreVectorElement* sve) { return sve->Value()->Type()->Align(); },
            [&](const CoreBuiltinCall* call) {
                switch (call->Func()) {
                    case BuiltinFn::kSubgroupMatrixLoad:
                    case BuiltinFn::kSubgroupMatrixStore:
                        return call->Args()[0]->Type()->UnwrapPtr()->Align();
                    default:
                        return 0u;
                }
            },
            [&](Default) { return 0u; });

        if (align_val <= natural_align) {
            AddError(inst) << "alignment (" << align_val
                           << ") must be greater than natural alignment (" << natural_align << ")";
        }
    }
}

void Functional::CheckOverride(const Override* o) {
    if (o->OverrideId().has_value()) {
        if (!seen_override_ids_.Add(o->OverrideId().value())) {
            AddError(o) << "duplicate override id encountered: " << o->OverrideId().value().value;
            return;
        }
    }

    if (!o->Result()->Type()->IsScalar()) {
        AddError(o) << "override type " << NameOf(o->Result()->Type()) << " is not a scalar";
        return;
    }

    if (o->Initializer() && o->Initializer()->Type() != o->Result()->Type()) {
        AddError(o) << "override type " << NameOf(o->Result()->Type())
                    << " does not match initializer type " << NameOf(o->Initializer()->Type());
        return;
    }

    if (!o->OverrideId().has_value() && (o->Initializer() == nullptr)) {
        AddError(o) << "must have an id or an initializer";
        return;
    }
}

void Functional::CheckVar(const Var* var) {
    auto* result_type = var->Result()->Type();
    auto* mv = result_type->As<core::type::MemoryView>();
    if (!mv) {
        AddError(var) << "result type " << NameOf(result_type)
                      << " must be a pointer or a reference";
        return;
    }

    bool generates_initializer = var->Initializer() != nullptr ||
                                 mv->AddressSpace() == core::AddressSpace::kPrivate ||
                                 mv->AddressSpace() == core::AddressSpace::kFunction;
    if (generates_initializer) {
        if (ElementsCount(result_type->UnwrapPtrOrRef()) >
            internal_limits::kMaxArrayConstructorElements) {
            AddError(var) << "type has excessive number of elements (>"
                          << internal_limits::kMaxArrayConstructorElements
                          << ") for an initializer";
            return;
        }
    }

    // Check that initializer and result type match
    if (var->Initializer()) {
        if (mv->AddressSpace() != AddressSpace::kFunction &&
            mv->AddressSpace() != AddressSpace::kPrivate &&
            mv->AddressSpace() != AddressSpace::kOut) {
            AddError(var) << "only variables in the function, private, or __out address space may "
                             "be initialized";
            return;
        }

        if (var->Initializer()->Type() != result_type->UnwrapPtrOrRef()) {
            AddError(var) << "initializer type " << NameOf(var->Initializer()->Type())
                          << " does not match store type " << NameOf(result_type->UnwrapPtrOrRef());
            return;
        }
    }

    if (var->Block() == ir_.root_block && mv->AddressSpace() == AddressSpace::kFunction) {
        AddError(var) << "vars in the 'function' address space must be in a function scope";
        return;
    }
    if (var->Block() != ir_.root_block && mv->AddressSpace() != AddressSpace::kFunction) {
        if (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface) ||
            mv->AddressSpace() != AddressSpace::kPrivate) {
            AddError(var) << "vars in a function scope must be in the 'function' address space";
            return;
        }
    }

    if (mv->AddressSpace() != AddressSpace::kStorage &&
        mv->AddressSpace() != AddressSpace::kHandle) {
        if (mv->AddressSpace() == AddressSpace::kWorkgroup ||
            !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
            if (!mv->StoreType()->HasFixedFootprint()) {
                AddError(var) << "vars not in the 'storage' or 'handle' address spaces "
                                 "must have a fixed footprint";
                return;
            }
        }
    }

    if (ContainsType<core::type::Atomic>(mv->StoreType())) {
        bool is_workgroup = mv->AddressSpace() == AddressSpace::kWorkgroup;
        bool is_read_write_storage = mv->AddressSpace() == AddressSpace::kStorage &&
                                     mv->Access() == core::Access::kReadWrite;
        if (!is_workgroup && !is_read_write_storage) {
            AddError(var)
                << "atomic types may only be used by 'workspace' or read write 'storage' variables";
            return;
        }
    }

    if (var->InputAttachmentIndex().has_value()) {
        if (mv->AddressSpace() != AddressSpace::kHandle) {
            AddError(var) << "'@input_attachment_index' is not valid for non-handle var";
            return;
        }
        if (!ir_.properties.Contains(Property::kAllowAnyInputAttachmentIndexType) &&
            !mv->UnwrapPtrOrRef()->Is<core::type::InputAttachment>()) {
            AddError(var)
                << "'@input_attachment_index' is only valid for 'input_attachment' type var";
            return;
        }
    }

    if (mv->AddressSpace() == AddressSpace::kStorage) {
        if (mv->StoreType() && !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'storage' address space must be host-shareable";
            return;
        }
        if (mv->Access() != core::Access::kReadWrite && mv->Access() != core::Access::kRead) {
            AddError(var)
                << "vars in the 'storage' address space must have access 'read' or 'read-write'";
            return;
        }
    } else if (mv->AddressSpace() == AddressSpace::kUniform) {
        if (!(mv->StoreType()->IsConstructible() || mv->StoreType()->Is<core::type::Buffer>()) ||
            !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'uniform' address space must be host-shareable and "
                             "constructible or a buffer";
            return;
        }
    } else if (mv->AddressSpace() == AddressSpace::kImmediate) {
        if (mv->StoreType() && !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'immediate' address space must be host-shareable";
            return;
        }
    } else if (mv->AddressSpace() == core::AddressSpace::kPixelLocal) {
        if (var->Block() == ir_.root_block) {
            if (!mv->StoreType()->Is<core::type::Struct>()) {
                AddError(var) << "pixel_local var must be of type struct";
                return;
            }
        }
    }
}

void Functional::CheckLet(const Let* l) {
    auto* result_ty = l->Result()->Type();
    if (ElementsCount(result_ty) > internal_limits::kMaxArrayConstructorElements) {
        AddError(l) << "type has excessive number of elements (>"
                    << internal_limits::kMaxArrayConstructorElements << ") for an initializer";
        return;
    }
    auto* value_ty = l->Value()->Type();
    if (value_ty != result_ty) {
        AddError(l) << "result type " << NameOf(l->Result()->Type())
                    << " does not match value type " << NameOf(l->Value()->Type());
    }

    if (ir_.properties.Contains(Property::kAllowAnyLetType)) {
        if (value_ty->Is<core::type::Void>()) {
            AddError(l) << "value type cannot be void";
        }
        return;
    }

    if (!value_ty->IsConstructible() && !value_ty->Is<core::type::Pointer>()) {
        AddError(l) << "value type, " << NameOf(value_ty)
                    << ", must be a concrete constructible type or a pointer type";
    }

    if (auto* ptr = result_ty->As<core::type::Pointer>()) {
        if (ptr->AddressSpace() == AddressSpace::kHandle &&
            !ir_.properties.Contains(Property::kAllowPointerToHandle)) {
            AddError(l) << "handle pointer cannot be captured in a let";
        }
    } else if (!result_ty->IsConstructible()) {
        AddError(l) << "result type, " << NameOf(result_ty)
                    << ", must be a concrete constructible type or a pointer type";
    }
}

void Functional::CheckConstruct(const Construct* construct) {
    auto* result_type = construct->Result()->Type();
    if (ElementsCount(result_type) > internal_limits::kMaxArrayConstructorElements) {
        AddError(construct) << "type has excessive number of elements (>"
                            << internal_limits::kMaxArrayConstructorElements
                            << ") for an initializer";
        return;
    }
    if (!result_type->IsConstructible()) {
        // We only allow `construct` to create non-constructible types when they are structures that
        // contain pointers and handle types, with the corresponding property enabled.
        if (!(result_type->Is<core::type::Struct>() &&
              ir_.properties.Contains(Property::kAllowMslEntryPointInterface))) {
            AddError(construct) << "type is not constructible";
            return;
        }
    }

    auto args = construct->Args();

    // Zero-value constructors are valid for all constructible types.
    if (args.empty()) {
        return;
    }

    // Check that type type of each argument matches the expected element type of the composite.
    auto check_args_match_elements = [&] {
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i]->Is<ir::Unused>()) {
                continue;
            }
            auto* expected_type = result_type->Element(static_cast<uint32_t>(i));
            if (args[i]->Type() != expected_type) {
                AddError(construct, Construct::kArgsOperandOffset + i)
                    << "type " << NameOf(args[i]->Type()) << " of argument " << i
                    << " does not match expected type " << NameOf(expected_type);
            }
        }
    };

    if (result_type->Is<core::type::Scalar>()) {
        // The only valid non-zero scalar constructor is the identity operation.
        if (args.size() > 1) {
            AddError(construct) << "scalar construct must not have more than one argument";
        }
        if (args[0]->Type() != result_type) {
            AddError(construct, 0u) << "scalar construct argument type " << NameOf(args[0]->Type())
                                    << " does not match result type " << NameOf(result_type);
        }
        return;
    }

    if (auto* sg_mat = result_type->As<core::type::SubgroupMatrix>()) {
        if (args.size() > 1) {
            AddError(construct) << "subgroup matrix construct must not have more than 1 argument";
            return;
        }

        // 8-bit integer matrices use 32-bit shader scalar types in WGSL.
        // Some backends may support 8-bit integers, in which case they would pass an 8-bit
        // type for the constructor value instead.
        const core::type::Type* scalar_ty = sg_mat->Type();
        if (scalar_ty->Is<core::type::I8>()) {
            scalar_ty = type_mgr_.i32();
        } else if (scalar_ty->Is<core::type::U8>()) {
            scalar_ty = type_mgr_.u32();
        }
        if (args[0]->Type() != scalar_ty && args[0]->Type() != sg_mat->Type()) {
            AddError(construct) << "subgroup matrix construct argument type "
                                << NameOf(args[0]->Type())
                                << " does not match matrix shader scalar type "
                                << NameOf(scalar_ty);
        }
        return;
    }

    if (auto* arr = result_type->As<core::type::Array>()) {
        if (args.size() != arr->ConstantCount()) {
            AddError(construct) << "array has " << arr->ConstantCount().value()
                                << " elements, but construct provides " << args.size()
                                << " arguments";
            return;
        }
        check_args_match_elements();
        return;
    }

    if (auto* str = As<core::type::Struct>(result_type)) {
        auto members = str->Members();
        if (args.size() != str->Members().Length()) {
            AddError(construct) << "structure has " << members.Length()
                                << " members, but construct provides " << args.size()
                                << " arguments";
            return;
        }
        check_args_match_elements();
    }

    auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
    auto arg_types = Transform<4>(args, [&](auto* v) { return v->Type(); });
    if (auto* vec = result_type->As<core::type::Vector>()) {
        auto ctor_conv = intrinsic::VectorCtorConv(vec->Width());
        auto match = table.Lookup(ctor_conv, Vector<TemplateParameter, 1>{vec->Type()},
                                  std::move(arg_types), core::EvaluationStage::kConstant);
        if (match != Success ||
            !match->info->flags.Contains(intrinsic::OverloadFlag::kIsConstructor) ||
            vec->Type() != arg_types[0]->DeepestElement()) {
            AddError(construct) << "no matching overload for " << vec->FriendlyName()
                                << " constructor";
        }
        return;
    }

    if (auto* mat = result_type->As<core::type::Matrix>()) {
        auto ctor_conv = intrinsic::MatrixCtorConv(mat->Columns(), mat->Rows());
        auto match = table.Lookup(ctor_conv, Vector<TemplateParameter, 1>{mat->Type()},
                                  std::move(arg_types), core::EvaluationStage::kConstant);
        if (match != Success ||
            !match->info->flags.Contains(intrinsic::OverloadFlag::kIsConstructor)) {
            AddError(construct) << "no matching overload for " << mat->FriendlyName()
                                << " constructor";
        }
        return;
    }
}

void Functional::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                            //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },              //
        [&](const Construct* c) { CheckConstruct(c); },                  //
        [&](const Convert* c) { CheckConvert(c); },                      //
        [&](const Discard*) {},                                          //
        [&](const MemberBuiltinCall* c) { CheckMemberBuiltinCall(c); },  //
        [&](const UserCall* c) { CheckUserCall(c); },                    //
        [&](Default) { /* Validation of custom IR instructions */ });
}

void Functional::CheckAccess(const Access* a) {
    auto* obj_view = a->Object()->Type()->As<core::type::MemoryView>();
    auto* ty = obj_view ? obj_view->StoreType() : a->Object()->Type();

    enum Kind : uint8_t {
        kPtr,
        kRef,
        kValue,
    };
    const Kind in_kind = tint::Switch(
        a->Object()->Type(),                                 //
        [&](const core::type::Pointer*) { return kPtr; },    //
        [&](const core::type::Reference*) { return kRef; },  //
        [&](Default) { return kValue; });

    auto desc_of = [&](Kind kind, const core::type::Type* type) {
        switch (kind) {
            case kPtr:
                return StyledText{}
                       << style::Type("ptr<", obj_view->AddressSpace(), ", ", type->FriendlyName(),
                                      ", ", obj_view->Access(), ">");
            case kRef:
                return StyledText{}
                       << style::Type("ref<", obj_view->AddressSpace(), ", ", type->FriendlyName(),
                                      ", ", obj_view->Access(), ">");
            default:
                return NameOf(type);
        }
    };

    for (size_t i = 0; i < a->Indices().size(); i++) {
        auto err = [&]() -> diag::Diagnostic& {
            return AddError(a, i + Access::kIndicesOperandOffset);
        };

        auto* index = a->Indices()[i];
        if (DAWN_UNLIKELY((!index->Type()->IsAnyOf<core::type::I32, core::type::U32>()))) {
            err() << "index type " << NameOf(index->Type()) << " must be i32 or u32";
            return;
        }

        if (!ir_.properties.Contains(Property::kAllowVectorElementPointer)) {
            if (in_kind != kValue && ty->Is<core::type::Vector>()) {
                err() << "cannot obtain address of vector element";
                return;
            }
        }

        if (auto* const_index = index->As<ir::Constant>()) {
            auto* value = const_index->Value();
            if (value->Type()->IsSignedIntegerScalar()) {
                // index is a signed integer scalar. Check that the index isn't negative.
                // If the index is unsigned, we can skip this.
                auto idx = value->ValueAs<AInt>();
                if (DAWN_UNLIKELY(idx < 0)) {
                    err() << "constant index must be positive, got " << idx;
                    return;
                }
            }

            auto idx = value->ValueAs<uint32_t>();
            auto* el = ty->Element(idx);
            if (DAWN_UNLIKELY(!el)) {
                // Is index in bounds?
                if (auto el_count = ty->Elements().count; el_count != 0 && idx >= el_count) {
                    err() << "index out of bounds for type " << desc_of(in_kind, ty);
                    AddNote(a, i + Access::kIndicesOperandOffset)
                        << "acceptable range: [0.." << (el_count - 1) << "]";
                    return;
                }
                err() << "type " << desc_of(in_kind, ty) << " cannot be indexed";
                return;
            }
            ty = el;
        } else {
            auto* el = ty->Elements().type;
            if (DAWN_UNLIKELY(!el)) {
                err() << "type " << desc_of(in_kind, ty) << " cannot be dynamically indexed";
                return;
            }
            ty = el;
        }
    }

    auto* want = a->Result()->Type();
    auto* want_view = want->As<core::type::MemoryView>();
    bool ok = true;
    if (obj_view) {
        // Pointer source always means pointer result.
        ok = (want_view != nullptr) && ty == want_view->StoreType();
        if (ok) {
            // Also check that the address space and access modes match.
            bool base_is_ptr = obj_view->IsAnyOf<core::type::Pointer, core::type::SwizzleView>();
            ok =
                base_is_ptr == want_view->IsAnyOf<core::type::Pointer, core::type::SwizzleView>() &&
                obj_view->AddressSpace() == want_view->AddressSpace() &&
                obj_view->Access() == want_view->Access();
        }
    } else {
        // Otherwise, result types should exactly match.
        ok = ty == want;
    }
    if (DAWN_UNLIKELY(!ok)) {
        AddError(a) << "result of access chain is type " << desc_of(in_kind, ty)
                    << " but instruction type is " << NameOf(want);
    }
}

void Functional::CheckBinary(const Binary* b) {
    intrinsic::Context context{b->TableData(), type_mgr_, symbols_};

    auto overload =
        core::intrinsic::LookupBinary(context, b->Op(), b->LHS()->Type(), b->RHS()->Type(),
                                      core::EvaluationStage::kRuntime, /* is_compound */ false);
    if (overload != Success) {
        AddError(b) << overload.Failure();
        return;
    }

    auto* result = b->Result(0);
    TINT_ASSERT(result);

    if (overload->return_type != result->Type()) {
        AddError(b) << "result value type " << NameOf(result->Type()) << " does not match "
                    << style::Instruction(b->Op()) << " result type "
                    << NameOf(overload->return_type);
    }
}

void Functional::CheckIf(const If* if_) {
    if (!if_->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(if_, If::kConditionOperandOffset) << "condition type must be 'bool'";
    }

    CheckBlock(if_->True());
    CheckBlock(if_->False());
}

bool Functional::CanLoad(const core::type::Type* ty) {
    return tint::Switch(
        ty,  //
        [&](const core::type::Array* arr) {
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                return false;
            }
            return CanLoad(arr->Elements().type);
        },
        [&](const core::type::Struct* str) {
            for (auto* member : str->Members()) {
                if (member->Type()->Is<core::type::Pointer>() &&
                    ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
                    continue;
                }
                if (!CanLoad(member->Type())) {
                    return false;
                }
            }
            return true;
        },
        [&](Default) { return ty->IsConstructible() || ty->IsHandle(); });
}

void Functional::CheckLoad(const Load* l) {
    const Value* from = l->From();
    TINT_ASSERT(from);

    auto* mv = from->Type()->As<core::type::MemoryView>();
    if (!mv) {
        AddError(l, Load::kFromOperandOffset)
            << "load source operand " << NameOf(from->Type()) << " is not a memory view";
        return;
    }

    if (mv->Access() != core::Access::kRead && mv->Access() != core::Access::kReadWrite) {
        AddError(l, Load::kFromOperandOffset)
            << "load source operand has a non-readable access type, "
            << style::Literal(ToString(mv->Access()));
        return;
    }

    if (l->Result()->Type() != mv->StoreType()) {
        AddError(l, Load::kFromOperandOffset)
            << "result type " << NameOf(l->Result()->Type()) << " does not match source store type "
            << NameOf(mv->StoreType());
    }

    if (!CanLoad(mv->StoreType())) {
        AddError(l, Load::kFromOperandOffset)
            << "type " << NameOf(mv->StoreType()) << " cannot be loaded";
        return;
    }
}

void Functional::CheckStore(const Store* s) {
    const core::ir::Value* from = s->From();
    const core::ir::Value* to = s->To();
    TINT_ASSERT(from != nullptr);
    TINT_ASSERT(to != nullptr);

    auto* mv = As<core::type::MemoryView>(to->Type());
    if (!mv) {
        AddError(s, Store::kToOperandOffset)
            << "store target operand " << NameOf(to->Type()) << " is not a memory view";
        return;
    }

    if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
        AddError(s, Store::kToOperandOffset)
            << "store target operand has a non-writeable access type, "
            << style::Literal(ToString(mv->Access()));
        return;
    }

    const core::type::Type* value_type = from->Type();
    const core::type::Type* store_type = mv->StoreType();
    if (value_type != store_type) {
        AddError(s, Store::kFromOperandOffset)
            << "value type " << NameOf(value_type) << " does not match store type "
            << NameOf(store_type);
        return;
    }

    if (!store_type->IsConstructible()) {
        AddError(s) << "store type " << NameOf(store_type) << " is not constructible";
        return;
    }
}

const core::type::Type* Functional::GetVectorPtrElementType(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operands()[idx];
    TINT_ASSERT(operand) << "missing element operand";

    auto* type = operand->Type();
    TINT_ASSERT(type) << "missing operand type";

    auto* memory_view_ty = type->As<core::type::MemoryView>();
    if (DAWN_LIKELY(memory_view_ty)) {
        auto* vec_ty = memory_view_ty->StoreType()->As<core::type::Vector>();
        if (DAWN_LIKELY(vec_ty)) {
            return vec_ty->Type();
        }
    }

    AddError(inst, idx) << "operand " << NameOf(type) << " must be a pointer to a vector";
    return nullptr;
}

void Functional::CheckLoadVectorElement(const LoadVectorElement* l) {
    const core::type::Type* el_ty =
        GetVectorPtrElementType(l, LoadVectorElement::kFromOperandOffset);
    if (!el_ty) {
        return;
    }

    auto* res = l->Result(0);
    if (res->Type() != el_ty) {
        AddError(l) << "result type " << NameOf(res->Type())
                    << " does not match vector pointer element type " << NameOf(el_ty);
        return;
    }

    if (!l->Index()->Type()->IsIntegerScalar()) {
        AddError(l, LoadVectorElement::kIndexOperandOffset)
            << "load vector element index must be an integer scalar";
    }
    if (auto* c = l->Index()->As<core::ir::Constant>()) {
        uint32_t val = c->Value()->ValueAs<uint32_t>();

        const core::type::Vector* vec_ty =
            l->From()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
        TINT_ASSERT(vec_ty);

        if (val >= vec_ty->Width()) {
            AddError(l, LoadVectorElement::kIndexOperandOffset)
                << "load vector element index must be in range [0, " << (vec_ty->Width() - 1)
                << "]";
        }
    }
}

void Functional::CheckStoreVectorElement(const StoreVectorElement* s) {
    const core::type::Type* el_ty =
        GetVectorPtrElementType(s, StoreVectorElement::kToOperandOffset);
    if (!el_ty) {
        return;
    }
    auto* value = s->Value();
    if (value->Type() != el_ty) {
        AddError(s, StoreVectorElement::kValueOperandOffset)
            << "value type " << NameOf(value->Type())
            << " does not match vector pointer element type " << NameOf(el_ty);
        return;
    }

    // The `GetVectorPtrElementType` has already validated that the pointer exists.
    const core::type::MemoryView* mv = s->To()->Type()->As<core::type::MemoryView>();
    if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
        AddError(s, StoreVectorElement::kToOperandOffset)
            << "store_vector_element target operand has a non-writeable access type, "
            << style::Literal(ToString(mv->Access()));
        return;
    }

    if (!s->Index()->Type()->IsIntegerScalar()) {
        AddError(s, StoreVectorElement::kIndexOperandOffset)
            << "store vector element index must be an integer scalar";
    }

    const core::ir::Constant* c = s->Index()->As<core::ir::Constant>();
    if (c == nullptr) {
        return;
    }

    uint32_t val = c->Value()->ValueAs<uint32_t>();
    const core::type::Vector* vec_ty = s->To()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
    TINT_ASSERT(vec_ty);

    if (val >= vec_ty->Width()) {
        AddError(s, StoreVectorElement::kIndexOperandOffset)
            << "store vector element index must be in range [0, " << (vec_ty->Width() - 1) << "]";
    }
}

void Functional::CheckLoop(const Loop* l) {
    CheckBlock(l->Initializer());
    CheckLoopBody(l);
    CheckLoopContinuing(l);

    first_continues_.Remove(l);
}

void Functional::CheckLoopBody(const Loop* loop) {
    CheckBlock(loop->Body());
}

void Functional::CheckLoopContinuing(const Loop* loop) {
    // Ensure that values used in the loop continuing are not from the loop body, after a continue
    // instruction.
    auto* first_continue = first_continues_.GetOr(loop, nullptr);
    if (first_continue != nullptr) {
        // Find the instruction in the body block that is or holds the first continue instruction.
        const Instruction* holds_continue = first_continue;
        while (holds_continue && holds_continue->Block() &&
               holds_continue->Block() != loop->Body()) {
            holds_continue = holds_continue->Block()->Parent();
        }

        auto check_usage = [&](Usage use) {
            if (TransitivelyHolds(loop->Continuing(), use.instruction)) {
                AddError(use.instruction, use.operand_index)
                    << NameOf(use.instruction->Operands()[use.operand_index])
                    << " cannot be used in continuing block as it is declared after the first "
                    << style::Instruction("continue") << " in the loop's body";
                AddNote(first_continue) << "loop body's first " << style::Instruction("continue");
            }
        };

        // Check that all subsequent instruction values are not used in the continuing block.
        for (auto* inst = holds_continue; inst; inst = inst->next) {
            for (auto* result : inst->Results()) {
                result->ForEachUseUnsorted(check_usage);
            }
        }
    }

    CheckBlock(loop->Continuing());
}

void Functional::CheckTerminator(const Terminator* b) {
    tint::Switch(
        b,                                                 //
        [&](const ir::BreakIf* i) { CheckBreakIf(i); },    //
        [&](const ir::Continue* c) { CheckContinue(c); },  //
        [&](const ir::Exit* e) { CheckExit(e); },          //
        [&](const ir::NextIteration*) {},                  //
        [&](const ir::Return* ret) { CheckReturn(ret); },  //
        [&](const ir::TerminateInvocation*) {},            //
        [&](const ir::Unreachable*) {},                    //
        TINT_ICE_ON_NO_MATCH);
}

void Functional::CheckContinue(const Continue* c) {
    auto* loop = c->Loop();
    TINT_ASSERT(loop);

    first_continues_.Add(loop, c);
}

void Functional::CheckSwitch(const Switch* s) {
    if (!s->Condition()->Type()->IsIntegerScalar()) {
        auto* cond_ty = s->Condition() ? s->Condition()->Type() : nullptr;
        AddError(s, Switch::kConditionOperandOffset)
            << "condition type " << NameOf(cond_ty) << " must be an integer scalar";
    }

    bool found_default = false;
    for (auto& case_ : s->Cases()) {
        CheckBlock(case_.block);

        for (const auto& sel : case_.selectors) {
            if (sel.IsDefault()) {
                if (found_default) {
                    AddError(s) << "multiple default selectors in switch";
                }
                found_default = true;
            } else if (!sel.val->Type()->IsIntegerScalar()) {
                AddError(s) << "case selector type " << NameOf(sel.val->Type())
                            << " must be an integer scalar";
            } else if (sel.val->Type() != s->Condition()->Type()) {
                AddError(s) << "case selector type " << NameOf(sel.val->Type())
                            << " must match the switch condition type "
                            << NameOf(s->Condition()->Type());
            }
        }
    }

    if (!found_default) {
        AddError(s) << "missing default case for switch";
    }
}

void Functional::CheckSwizzle(const Swizzle* s) {
    auto* result_ty = s->Result()->Type();

    auto* obj_ty = s->Object()->Type();
    auto* src_mv = obj_ty->As<core::type::MemoryView>();
    auto* src_vec =
        src_mv ? src_mv->StoreType()->As<core::type::Vector>() : obj_ty->As<core::type::Vector>();
    if (!src_vec) {
        AddError(s) << "object of swizzle, " << NameOf(s->Object()) << ", is not a vector, "
                    << NameOf(s->Object()->Type());
        return;
    }

    auto indices = s->Indices();
    if (indices.Length() < Swizzle::kMinNumIndices) {
        AddError(s) << "expected at least " << Swizzle::kMinNumIndices << " indices";
        return;
    }

    if (indices.Length() > Swizzle::kMaxNumIndices) {
        AddError(s) << "expected at most " << Swizzle::kMaxNumIndices << " indices";
        return;
    }

    auto elem_count = src_vec->Elements().count;
    for (auto& idx : indices) {
        if (idx > Swizzle::kMaxIndexValue || idx >= elem_count) {
            AddError(s) << "invalid index value";
            return;
        }
    }

    auto* elem_ty = src_vec->Elements().type;
    auto* expected_store_ty = type_mgr_.MatchWidth(elem_ty, indices.Length());
    const core::type::Type* expected_ty = nullptr;
    if (src_mv) {
        expected_ty = type_mgr_.Get<core::type::SwizzleView>(
            src_mv->AddressSpace(), expected_store_ty, src_mv->Access(), src_vec->Width(),
            static_cast<uint32_t>(indices.Length()));
    } else {
        expected_ty = expected_store_ty;
    }

    if (result_ty != expected_ty) {
        AddError(s) << "result type " << NameOf(result_ty) << " does not match expected type, "
                    << NameOf(expected_ty);
        return;
    }
}

void Functional::CheckUnary(const Unary* u) {
    TINT_ASSERT(u->Val());

    intrinsic::Context context{u->TableData(), type_mgr_, symbols_};
    auto overload = core::intrinsic::LookupUnary(context, u->Op(), u->Val()->Type(),
                                                 core::EvaluationStage::kRuntime);
    if (overload != Success) {
        AddError(u) << overload.Failure();
        return;
    }

    const core::ir::Value* result = u->Result(0);
    if (overload->return_type != result->Type()) {
        AddError(u) << "result value type " << NameOf(result->Type()) << " does not match "
                    << style::Instruction(u->Op()) << " result type "
                    << NameOf(overload->return_type);
    }
}

void Functional::CheckBuiltinCall(const BuiltinCall* call) {
    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });

    intrinsic::Context context{call->TableData(), type_mgr_, symbols_};
    auto builtin = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                             call->ExplicitTemplateParams(), args,
                                             core::EvaluationStage::kRuntime);
    if (builtin != Success) {
        AddError(call) << builtin.Failure();
        return;
    }

    TINT_ASSERT(builtin->return_type);

    if (builtin->return_type != call->Result()->Type()) {
        AddError(call) << "call result type " << NameOf(call->Result()->Type())
                       << " does not match builtin return type " << NameOf(builtin->return_type);
        return;
    }

    // Check evaluation stage of parameters that are required to be const-expressions.
    for (uint32_t i = 0; i < builtin->parameters.Length(); i++) {
        const auto& p = builtin->parameters[i];
        const auto* arg = call->Args()[i];
        if (p.is_const && !arg->Is<Constant>()) {
            AddError(call, BuiltinCall::kArgsOperandOffset + i)
                << "the " << style::Variable(p.usage) << " argument must be a constant";
            return;
        }
    }

    const core::ir::CoreBuiltinCall* bc = call->As<CoreBuiltinCall>();
    if (bc == nullptr) {
        return;
    }

    CheckCoreBuiltinCall(bc, builtin.Get());
}

void Functional::CheckCoreBuiltinCall(const CoreBuiltinCall* call,
                                      const core::intrinsic::Overload& overload) {
    auto idx_for_usage = [&](core::ParameterUsage usage) -> std::optional<uint32_t> {
        for (uint32_t i = 0; i < overload.parameters.Length(); ++i) {
            auto& p = overload.parameters[i];
            if (p.usage == usage) {
                return int32_t(i);
            }
        }
        return std::nullopt;
    };

    auto check_arg_in_range = [&](core::ParameterUsage usage, int32_t min, int32_t max) {
        auto idx_opt = idx_for_usage(usage);
        if (!idx_opt.has_value()) {
            return;
        }
        uint32_t idx = idx_opt.value();
        TINT_ASSERT(idx < call->Args().size());

        auto* val = call->Args()[idx];
        auto* const_val = val->As<ir::Constant>();
        TINT_ASSERT(const_val);
        auto* cnst = const_val->Value();

        if (val->Type()->Is<core::type::Vector>()) {
            for (size_t i = 0; i < cnst->NumElements(); i++) {
                auto value = cnst->Index(i)->ValueAs<int32_t>();
                if (value < min || value > max) {
                    AddError(call, idx)
                        << value << " outside range of [" << min << ", " << max << "]";
                    return;
                }
            }
        } else {
            auto value = cnst->ValueAs<int32_t>();
            if (value < min || value > max) {
                AddError(call, idx) << value << " outside range of [" << min << ", " << max << "]";
                return;
            }
        }
    };

    if (core::IsTexture(call->Func())) {
        check_arg_in_range(core::ParameterUsage::kComponent, 0, 3);
        check_arg_in_range(core::ParameterUsage::kOffset, -8, 7);
    }

    if (call->Func() == core::BuiltinFn::kSubgroupMatrixLoad ||
        call->Func() == core::BuiltinFn::kSubgroupMatrixStore) {
        CheckSubgroupMatrixOpOffset(call);
    }
}

void Functional::CheckSubgroupMatrixOpOffset(const CoreBuiltinCall* call) {
    const core::ir::Value* p_arg = call->Args()[0];
    const core::ir::Value* offset_arg = call->Args()[1];

    const core::type::Pointer* ptr_ty = p_arg->Type()->As<core::type::Pointer>();
    TINT_ASSERT(ptr_ty);

    const core::type::Array* arr_ty = ptr_ty->StoreType()->As<core::type::Array>();
    TINT_ASSERT(arr_ty);

    auto const_count = arr_ty->ConstantCount();
    if (!const_count.has_value()) {
        return;
    }

    const core::type::SubgroupMatrix* mat_ty = nullptr;
    if (call->Func() == core::BuiltinFn::kSubgroupMatrixLoad) {
        mat_ty = call->Result()->Type()->As<core::type::SubgroupMatrix>();
    } else if (call->Func() == core::BuiltinFn::kSubgroupMatrixStore) {
        mat_ty = call->Args()[2]->Type()->As<core::type::SubgroupMatrix>();
    }
    TINT_ASSERT(mat_ty);

    auto mat_comp_size = mat_ty->Type()->Size();
    TINT_ASSERT(mat_comp_size > 0);

    auto limit = const_count.value();

    if (auto* offset_const = offset_arg->As<ir::Constant>()) {
        auto* offset_val = offset_const->Value();
        uint32_t offset = 0;
        if (offset_arg->Type()->IsUnsignedIntegerScalar()) {
            offset = offset_val->ValueAs<u32>();
        } else if (offset_arg->Type()->IsSignedIntegerScalar()) {
            auto ival = offset_val->ValueAs<i32>();
            if (ival < 0) {
                AddError(call, 1) << "the offset argument of " << call->Func()
                                  << " must be non-negative";
                return;
            }
            offset = static_cast<uint32_t>(ival);
        }

        if (offset >= limit) {
            AddError(call, 1) << "the offset argument of " << call->Func() << " (" << offset
                              << ") is out of bounds of the array type of size " << limit;
        }
    }
}

void Functional::CheckMemberBuiltinCall(const MemberBuiltinCall* call) {
    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });
    args.Insert(0, call->Object()->Type());

    intrinsic::Context context{call->TableData(), type_mgr_, symbols_};
    auto result = core::intrinsic::LookupMemberFn(context, call->FriendlyName().c_str(),
                                                  call->FuncId(), call->ExplicitTemplateParams(),
                                                  std::move(args), core::EvaluationStage::kRuntime);
    if (result != Success) {
        AddError(call) << result.Failure();
        return;
    }

    if (result->return_type != call->Result()->Type()) {
        // Note: This is not currently tested in core unittests as there are no concrete
        // MemberBuiltinCall implementations in core IR. This is tested by backend-specific
        // (e.g. HLSL) validation tests.
        AddError(call) << "member call result type " << NameOf(call->Result()->Type())
                       << " does not match builtin return type " << NameOf(result->return_type);
    }
}

void Functional::CheckConvert(const Convert* convert) {
    auto* result_type = convert->Result()->Type();
    auto* value_type = convert->Operand(Convert::kValueOperandOffset)->Type();

    intrinsic::CtorConv conv_ty;
    Vector<TemplateParameter, 1> template_type;
    tint::Switch(
        result_type,                                                             //
        [&](const core::type::I32*) { conv_ty = intrinsic::CtorConv::kI32; },    //
        [&](const core::type::U32*) { conv_ty = intrinsic::CtorConv::kU32; },    //
        [&](const core::type::U64*) { conv_ty = intrinsic::CtorConv::kU64; },    //
        [&](const core::type::F32*) { conv_ty = intrinsic::CtorConv::kF32; },    //
        [&](const core::type::F16*) { conv_ty = intrinsic::CtorConv::kF16; },    //
        [&](const core::type::Bool*) { conv_ty = intrinsic::CtorConv::kBool; },  //
        [&](const core::type::Vector* v) {
            conv_ty = intrinsic::VectorCtorConv(v->Width());
            template_type.Push(v->Type());
        },
        [&](const core::type::Matrix* m) {
            conv_ty = intrinsic::MatrixCtorConv(m->Columns(), m->Rows());
            template_type.Push(m->Type());
        },
        [&](Default) { conv_ty = intrinsic::CtorConv::kNone; });

    if (conv_ty == intrinsic::CtorConv::kNone) {
        AddError(convert) << "not defined for result type, " << NameOf(result_type);
        return;
    }

    auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
    auto match =
        table.Lookup(conv_ty, template_type, Vector{value_type}, core::EvaluationStage::kOverride);
    if (match != Success || !match->info->flags.Contains(intrinsic::OverloadFlag::kIsConverter)) {
        AddError(convert) << "No defined converter for " << NameOf(value_type) << " -> "
                          << NameOf(result_type);
        return;
    }
}

void Functional::CheckUserCall(const UserCall* call) {
    if (call->Target()->IsEntryPoint()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "call target must not have a pipeline stage";
    }

    if (call->Target()->ReturnType() != call->Result()->Type()) {
        AddError(call) << "result type does not match function return type";
        return;
    }

    auto args = call->Args();
    auto params = call->Target()->Params();
    if (args.size() != params.Length()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "function has " << params.Length() << " parameters, but call provides "
            << args.size() << " arguments";
        return;
    }

    for (size_t i = 0; i < args.size(); i++) {
        bool allow_mismatch = false;
        if (auto* arg_buffer_ty = args[i]->Type()->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
            auto* arg_ptr_ty = args[i]->Type()->As<core::type::Pointer>();
            if (auto* param_ptr_ty = params[i]->Type()->As<core::type::Pointer>()) {
                if (auto* param_buffer_ty =
                        param_ptr_ty->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
                    allow_mismatch = arg_ptr_ty->AddressSpace() == param_ptr_ty->AddressSpace() &&
                                     arg_ptr_ty->Access() == param_ptr_ty->Access();
                    const bool both_constant =
                        arg_buffer_ty->Count()->Is<core::type::ConstantArrayCount>() &&
                        param_buffer_ty->Count()->Is<core::type::ConstantArrayCount>();
                    uint32_t arg_size = arg_buffer_ty->ConstantCount().value_or(0);
                    uint32_t param_size = param_buffer_ty->ConstantCount().value_or(0);
                    allow_mismatch &=
                        param_buffer_ty->Count()->Is<core::type::RuntimeArrayCount>() ||
                        (both_constant && param_size < arg_size);
                }
            }
        }
        if (!allow_mismatch && args[i]->Type() != params[i]->Type()) {
            AddError(call, UserCall::kArgsOperandOffset + i)
                << "type " << NameOf(params[i]->Type()) << " of function parameter " << i
                << " does not match argument type " << NameOf(args[i]->Type());
        }
    }
}

void Functional::CheckBreakIf(const BreakIf* b) {
    if (!b->Condition()->Type() || !b->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(b) << "condition must be a 'bool'";
        return;
    }

    auto* loop = b->Loop();
    TINT_ASSERT(loop);

    if (loop->Continuing() != b->Block()) {
        AddError(b) << "must only be called directly from loop continuing";
    }
}

void Functional::CheckExit(const Exit* e) {
    tint::Switch(
        e,                                                 //
        [&](const ir::ExitIf*) {},                         //
        [&](const ir::ExitLoop* l) { CheckExitLoop(l); },  //
        [&](const ir::ExitSwitch*) {},                     //
        TINT_ICE_ON_NO_MATCH);
}

void Functional::CheckExitLoop(const ExitLoop* l) {
    const Instruction* inst = l;
    const Loop* control = l->Loop();
    while (inst) {
        // Found parent loop
        if (inst->Block()->Parent() == control) {
            if (inst->Block() == control->Continuing()) {
                AddError(l) << "loop exit jumps out of continuing block";
                if (control->Continuing() != l->Block()) {
                    AddNote(control->Continuing()) << "in continuing block";
                }
            } else if (inst->Block() == control->Initializer()) {
                AddError(l) << "loop exit not permitted in loop initializer";
                if (control->Initializer() != l->Block()) {
                    AddNote(control->Initializer()) << "in initializer block";
                }
            }
            break;
        }
        inst = inst->Block()->Parent();
    }
}

void Functional::CheckReturn(const Return* ret) {
    auto* func = ret->Func();
    TINT_ASSERT(func);

    if (func->ReturnType()->Is<core::type::Void>()) {
        if (ret->HasValue()) {
            AddError(ret) << "unexpected return value";
        }
        return;
    }

    if (!ret->Value()) {
        AddError(ret) << "expected return value";
    } else if (ret->Value()->Type() != func->ReturnType()) {
        AddError(ret) << "return value type " << NameOf(ret->Value()->Type())
                      << " does not match function return type " << NameOf(func->ReturnType());
    }
}

}  // namespace tint::core::ir::validator
