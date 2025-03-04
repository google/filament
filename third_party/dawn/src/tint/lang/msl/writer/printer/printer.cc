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

#include "src/tint/lang/msl/writer/printer/printer.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <utility>

#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/constant.h"
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
#include "src/tint/lang/core/ir/ice.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/msl/barrier_type.h"
#include "src/tint/lang/msl/builtin_fn.h"
#include "src/tint/lang/msl/ir/builtin_call.h"
#include "src/tint/lang/msl/ir/component.h"
#include "src/tint/lang/msl/ir/member_builtin_call.h"
#include "src/tint/lang/msl/ir/memory_order.h"
#include "src/tint/lang/msl/type/bias.h"
#include "src/tint/lang/msl/type/gradient.h"
#include "src/tint/lang/msl/type/level.h"
#include "src/tint/lang/msl/writer/common/options.h"
#include "src/tint/lang/msl/writer/common/printer_support.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text_generator.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::msl::writer {
namespace {

/// @returns true if @p ident is an MSL keyword that needs to be avoided
bool IsKeyword(std::string_view ident);

/// PIMPL class for the MSL generator
class Printer : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    explicit Printer(core::ir::Module& module, const Options& options)
        : ir_(module), options_(options) {}

    /// @returns the generated MSL shader
    tint::Result<Output> Generate() {
        auto valid = core::ir::ValidateAndDumpIfNeeded(
            ir_, "msl.Printer",
            core::ir::Capabilities{
                core::ir::Capability::kAllow8BitIntegers,
                core::ir::Capability::kAllow64BitIntegers,
                core::ir::Capability::kAllowPointersAndHandlesInStructures,
                core::ir::Capability::kAllowPrivateVarsInFunctions,
                core::ir::Capability::kAllowAnyLetType,
            });
        if (valid != Success) {
            return std::move(valid.Failure());
        }

        {
            TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
            Line() << "#include <metal_stdlib>";
            Line() << "using namespace metal;";
        }

        // Module-scope declarations should have all been moved into the entry points.
        TINT_ASSERT(ir_.root_block->IsEmpty());

        // Determine which structures will need to be emitted with host-shareable memory layouts.
        FindHostShareableStructs();

        // Emit functions.
        for (auto* func : ir_.DependencyOrderedFunctions()) {
            EmitFunction(func);
        }

        StringStream ss;
        ss << preamble_buffer_.String() << main_buffer_.String();
        result_.msl = ss.str();

        return std::move(result_);
    }

  private:
    /// The result of printing the module.
    Output result_;

    core::ir::Module& ir_;
    /// MSL writer options
    Options options_;

    /// A hashmap of object to name.
    Hashmap<const CastableBase*, std::string, 32> names_;

    /// The buffer holding preamble text
    TextBuffer preamble_buffer_;

    /// Unique name of the 'TINT_INVARIANT' preprocessor define.
    /// Non-empty only if an invariant attribute has been generated.
    std::string invariant_define_name_;

    Hashset<const core::type::Struct*, 16> host_shareable_structs_;
    Hashset<const core::type::Struct*, 4> emitted_structs_;

    /// The current function being emitted
    const core::ir::Function* current_function_ = nullptr;
    /// The current block being emitted
    const core::ir::Block* current_block_ = nullptr;

    /// Unique name of the tint_array<T, N> template.
    /// Non-empty only if the template has been generated.
    std::string array_template_name_;

    /// Block to emit for a continuing
    std::function<void()> emit_continuing_;

    /// @returns the name of the templated `tint_array` helper type, generating it if needed
    const std::string& ArrayTemplateName() {
        if (!array_template_name_.empty()) {
            return array_template_name_;
        }

        array_template_name_ = UniqueIdentifier("tint_array");

        TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
        Line();
        Line() << "template<typename T, size_t N>";
        Line() << "struct " << array_template_name_ << " {";

        {
            ScopedIndent si(current_buffer_);
            Line()
                << "const constant T& operator[](size_t i) const constant { return elements[i]; }";
            for (auto* space : {"device", "thread", "threadgroup"}) {
                Line() << space << " T& operator[](size_t i) " << space
                       << " { return elements[i]; }";
                Line() << "const " << space << " T& operator[](size_t i) const " << space
                       << " { return elements[i]; }";
            }
            Line() << "T elements[N];";
        }
        Line() << "};";

        return array_template_name_;
    }

    /// Find all structures that are used in host-shareable address spaces and mark them as such so
    /// that we know to pad the properly when we emit them.
    void FindHostShareableStructs() {
        // We only look at function parameters of entry points, since this is how binding resources
        // are handled in MSL.
        for (auto func : ir_.functions) {
            if (!func->IsEntryPoint()) {
                continue;
            }
            for (auto* param : func->Params()) {
                auto* ptr = param->Type()->As<core::type::Pointer>();
                if (ptr && core::IsHostShareable(ptr->AddressSpace())) {
                    // Look for structures at any nesting depth of this parameter's type.
                    Vector<const core::type::Type*, 8> type_queue;
                    type_queue.Push(ptr->StoreType());
                    while (!type_queue.IsEmpty()) {
                        auto* next = type_queue.Pop();
                        if (auto* str = next->As<core::type::Struct>()) {
                            // Record this structure as host-shareable.
                            host_shareable_structs_.Add(str);
                            for (auto* member : str->Members()) {
                                type_queue.Push(member->Type());
                            }
                        } else if (auto* arr = next->As<core::type::Array>()) {
                            type_queue.Push(arr->ElemType());
                        }
                    }
                }
            }
        }
    }

    /// Check if a value is emitted as an actual pointer (instead of a reference).
    /// @param value the value to check
    /// @returns true if @p value will be emitted as an actual pointer
    bool IsRealPointer(const core::ir::Value* value) {
        if (value->Is<core::ir::FunctionParam>()) {
            // Pointer parameters are always emitted as actual pointers.
            return true;
        }
        return Switch(
            value->As<core::ir::InstructionResult>()->Instruction(),
            [&](const core::ir::Var*) {
                // Variable declarations are always references.
                return false;
            },
            [&](const core::ir::Let*) {
                // Let declarations capture actual pointers.
                return true;
            },
            [&](const core::ir::Access* a) {
                // Access instruction emission always dereferences the source.
                // We only produce a pointer when extracting a pointer from a composite value.
                return !a->Object()->Type()->Is<core::type::Pointer>() &&
                       a->Result(0)->Type()->Is<core::type::Pointer>();
            });
    }

    /// Emit @p param value, dereferencing it if it is an actual pointer.
    /// @param out the output stream to write to
    /// @param value the value to emit
    template <typename OUT>
    void EmitAndDerefIfNeeded(OUT& out, const core::ir::Value* value) {
        if (value && value->Type()->Is<core::type::Pointer>() && IsRealPointer(value)) {
            out << "(*";
            EmitValue(out, value);
            out << ")";
        } else {
            EmitValue(out, value);
        }
    }

    /// Emit @p param value, taking its address if it is not an actual pointer.
    /// @param out the output stream to write to
    /// @param value the value to emit
    template <typename OUT>
    void EmitAndTakeAddressIfNeeded(OUT& out, const core::ir::Value* value) {
        if (value && value->Type()->Is<core::type::Pointer>() && !IsRealPointer(value)) {
            out << "(&";
            EmitValue(out, value);
            out << ")";
        } else {
            EmitValue(out, value);
        }
    }

    /// Emit the function
    /// @param func the function to emit
    void EmitFunction(const core::ir::Function* func) {
        TINT_SCOPED_ASSIGNMENT(current_function_, func);

        Line();
        {
            auto out = Line();

            // Remap the entry point name if requested.
            auto func_name = NameOf(func);
            if (func->IsEntryPoint() && !options_.remapped_entry_point_name.empty()) {
                func_name = options_.remapped_entry_point_name;
                TINT_ASSERT(!IsKeyword(func_name));
            }

            switch (func->Stage()) {
                case core::ir::Function::PipelineStage::kCompute: {
                    out << "kernel ";

                    auto const_wg_size = func->WorkgroupSizeAsConst();
                    TINT_ASSERT(const_wg_size);
                    auto wg_size = *const_wg_size;

                    // Store the workgroup information away to return from the generator.
                    result_.workgroup_info.x = wg_size[0];
                    result_.workgroup_info.y = wg_size[1];
                    result_.workgroup_info.z = wg_size[2];

                    break;
                }
                case core::ir::Function::PipelineStage::kFragment:
                    out << "fragment ";
                    break;
                case core::ir::Function::PipelineStage::kVertex:
                    out << "vertex ";
                    break;
                case core::ir::Function::PipelineStage::kUndefined:
                    break;
            }
            if (func->IsEntryPoint()) {
                result_.workgroup_info.allocations.insert({func_name, {}});
            }

            EmitType(out, func->ReturnType());
            out << " " << func_name << "(";

            size_t i = 0;
            for (auto* param : func->Params()) {
                if (i > 0) {
                    out << ", ";
                }
                ++i;

                EmitType(out, param->Type());
                out << " ";

                // Non-entrypoint pointers are set to `const` for the value
                if (!func->IsEntryPoint() && param->Type()->Is<core::type::Pointer>()) {
                    out << "const ";
                }

                out << NameOf(param);

                if (auto builtin = param->Builtin()) {
                    auto name = BuiltinToAttribute(builtin.value());
                    TINT_ASSERT(!name.empty());
                    out << " [[" << name << "]]";
                }

                if (param->Type()->Is<core::type::Struct>() && func->IsEntryPoint()) {
                    out << " [[stage_in]]";
                }

                auto ptr = param->Type()->As<core::type::Pointer>();
                if (auto binding_point = param->BindingPoint()) {
                    TINT_ASSERT(binding_point->group == 0);
                    if (ptr) {
                        switch (ptr->AddressSpace()) {
                            case core::AddressSpace::kStorage:
                            case core::AddressSpace::kUniform:
                                out << " [[buffer(" << binding_point->binding << ")]]";
                                break;
                            default:
                                TINT_UNREACHABLE() << "invalid address space with binding point: "
                                                   << ptr->AddressSpace();
                        }
                    } else {
                        // Handle types are declared by value instead of by pointer.
                        Switch(
                            param->Type(),
                            [&](const core::type::Texture*) {
                                out << " [[texture(" << binding_point->binding << ")]]";
                            },
                            [&](const core::type::Sampler*) {
                                out << " [[sampler(" << binding_point->binding << ")]]";
                            },
                            TINT_ICE_ON_NO_MATCH);
                    }
                }
                if (ptr && ptr->AddressSpace() == core::AddressSpace::kWorkgroup &&
                    func->Stage() == core::ir::Function::PipelineStage::kCompute) {
                    auto* ty = ptr->StoreType();

                    auto& allocations = result_.workgroup_info.allocations.at(func_name);
                    out << " [[threadgroup(" << allocations.size() << ")]]";
                    allocations.push_back(ty->Size());

                    // Currently type is always a struct, if this changes in the future we'll need
                    // to update this to handle non-struct data as well.
                    TINT_ASSERT(ty->Is<core::type::Struct>());

                    // This essentially matches std430 layout rules from GLSL, which are in
                    // turn specified as an upper bound for Vulkan layout sizing.
                    //
                    // Since Metal is even less specific, we assume Vulkan behavior as a
                    // good-enough approximation everywhere.
                    //
                    // We can't just take the `ty` size here because we've bundled multiple items
                    // into the struct, but this actually needs the non-bundled rounded size. e.g.
                    // 2-f32 values would be 32bytes in the non-struct case but 16 bytes struct
                    // case.
                    for (auto& mem : ty->As<core::type::Struct>()->Members()) {
                        auto mem_ty = mem->Type();
                        uint32_t align = mem_ty->Align();
                        uint32_t size = mem_ty->Size();
                        result_.workgroup_info.storage_size +=
                            tint::RoundUp(16u, tint::RoundUp(align, size));
                    }
                }
            }

            out << ") {";
        }
        {
            ScopedIndent si(current_buffer_);
            EmitBlock(func->Block());
        }

        Line() << "}";
    }

    /// Emit a block
    /// @param block the block to emit
    void EmitBlock(const core::ir::Block* block) { EmitBlockInstructions(block); }

    /// Emit the instructions in a block
    /// @param block the block with the instructions to emit
    void EmitBlockInstructions(const core::ir::Block* block) {
        TINT_SCOPED_ASSIGNMENT(current_block_, block);

        for (auto* inst : *block) {
            Switch(
                inst,                                                                    //
                [&](const core::ir::BreakIf* i) { EmitBreakIf(i); },                     //
                [&](const core::ir::Continue*) { EmitContinue(); },                      //
                [&](const core::ir::Discard*) { EmitDiscard(); },                        //
                [&](const core::ir::ExitIf*) { /* do nothing handled by transform */ },  //
                [&](const core::ir::ExitLoop*) { EmitExitLoop(); },                      //
                [&](const core::ir::ExitSwitch*) { EmitExitSwitch(); },                  //
                [&](const core::ir::If* i) { EmitIf(i); },                               //
                [&](const core::ir::Let* i) { EmitLet(i); },                             //
                [&](const core::ir::Loop* i) { EmitLoop(i); },                           //
                [&](const core::ir::NextIteration*) { /* do nothing */ },                //
                [&](const core::ir::Return* i) { EmitReturn(i); },                       //
                [&](const core::ir::Store* i) { EmitStore(i); },                         //
                [&](const core::ir::Switch* i) { EmitSwitch(i); },                       //
                [&](const core::ir::Unreachable*) { EmitUnreachable(); },                //
                [&](const core::ir::Call* i) { EmitCallStmt(i); },                       //
                [&](const core::ir::Var* i) { EmitVar(i); },                             //
                [&](const core::ir::StoreVectorElement* e) { EmitStoreVectorElement(e); },
                [&](const core::ir::TerminateInvocation*) { EmitDiscard(); },  //

                [&](const core::ir::LoadVectorElement*) { /* inlined */ },  //
                [&](const core::ir::Swizzle*) { /* inlined */ },            //
                [&](const core::ir::Bitcast*) { /* inlined */ },            //
                [&](const core::ir::Binary*) { /* inlined */ },             //
                [&](const core::ir::CoreUnary*) { /* inlined */ },          //
                [&](const core::ir::Load*) { /* inlined */ },               //
                [&](const core::ir::Construct*) { /* inlined */ },          //
                [&](const core::ir::Access*) { /* inlined */ },             //
                TINT_ICE_ON_NO_MATCH);
        }
    }

    void EmitValue(StringStream& out, const core::ir::Value* v) {
        Switch(
            v,                                                           //
            [&](const core::ir::Constant* c) { EmitConstant(out, c); },  //
            [&](const core::ir::InstructionResult* r) {
                Switch(
                    r->Instruction(),                                                    //
                    [&](const core::ir::Binary* b) { EmitBinary(out, b); },              //
                    [&](const core::ir::CoreUnary* u) { EmitUnary(out, u); },            //
                    [&](const core::ir::Convert* b) { EmitConvert(out, b); },            //
                    [&](const core::ir::Let* l) { out << NameOf(l->Result(0)); },        //
                    [&](const core::ir::Load* l) { EmitLoad(out, l); },                  //
                    [&](const core::ir::Construct* c) { EmitConstruct(out, c); },        //
                    [&](const core::ir::Var* var) { out << NameOf(var->Result(0)); },    //
                    [&](const core::ir::Bitcast* b) { EmitBitcast(out, b); },            //
                    [&](const core::ir::Access* a) { EmitAccess(out, a); },              //
                    [&](const msl::ir::BuiltinCall* c) { EmitMslBuiltinCall(out, c); },  //
                    [&](const msl::ir::MemberBuiltinCall* c) {
                        EmitMslMemberBuiltinCall(out, c);
                    },                                                                         //
                    [&](const core::ir::CoreBuiltinCall* c) { EmitCoreBuiltinCall(out, c); },  //
                    [&](const core::ir::UserCall* c) { EmitUserCall(out, c); },                //
                    [&](const core::ir::LoadVectorElement* e) {
                        EmitLoadVectorElement(out, e);
                    },                                                         //
                    [&](const core::ir::Swizzle* s) { EmitSwizzle(out, s); },  //
                    TINT_ICE_ON_NO_MATCH);
            },                                                            //
            [&](const core::ir::FunctionParam* p) { out << NameOf(p); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void EmitUnary(StringStream& out, const core::ir::CoreUnary* u) {
        switch (u->Op()) {
            case core::UnaryOp::kNegation:
                out << "-";
                break;
            case core::UnaryOp::kComplement:
                out << "~";
                break;
            case core::UnaryOp::kNot:
                out << "!";
                break;
            default:
                TINT_UNIMPLEMENTED() << u->Op();
        }
        out << "(";
        EmitValue(out, u->Val());
        out << ")";
    }

    /// Emit a binary instruction
    /// @param b the binary instruction
    void EmitBinary(StringStream& out, const core::ir::Binary* b) {
        auto kind = [&] {
            switch (b->Op()) {
                case core::BinaryOp::kAdd:
                    return "+";
                case core::BinaryOp::kSubtract:
                    return "-";
                case core::BinaryOp::kMultiply:
                    return "*";
                case core::BinaryOp::kDivide:
                    return "/";
                case core::BinaryOp::kModulo:
                    return "%";
                case core::BinaryOp::kAnd:
                    return "&";
                case core::BinaryOp::kOr:
                    return "|";
                case core::BinaryOp::kXor:
                    return "^";
                case core::BinaryOp::kEqual:
                    return "==";
                case core::BinaryOp::kNotEqual:
                    return "!=";
                case core::BinaryOp::kLessThan:
                    return "<";
                case core::BinaryOp::kGreaterThan:
                    return ">";
                case core::BinaryOp::kLessThanEqual:
                    return "<=";
                case core::BinaryOp::kGreaterThanEqual:
                    return ">=";
                case core::BinaryOp::kShiftLeft:
                    return "<<";
                case core::BinaryOp::kShiftRight:
                    return ">>";
                case core::BinaryOp::kLogicalAnd:
                    return "&&";
                case core::BinaryOp::kLogicalOr:
                    return "||";
            }
            return "<error>";
        };

        out << "(";
        EmitValue(out, b->LHS());
        out << " " << kind() << " ";
        EmitValue(out, b->RHS());
        out << ")";
    }

    /// Emit a convert instruction
    /// @param c the convert instruction
    void EmitConvert(StringStream& out, const core::ir::Convert* c) {
        EmitType(out, c->Result(0)->Type());
        out << "(";
        EmitValue(out, c->Operand(0));
        out << ")";
    }

    /// Emit a var instruction
    /// @param v the var instruction
    void EmitVar(const core::ir::Var* v) {
        auto out = Line();

        auto* ptr = v->Result(0)->Type()->As<core::type::Pointer>();
        TINT_ASSERT(ptr);

        auto space = ptr->AddressSpace();
        switch (space) {
            case core::AddressSpace::kFunction:
            case core::AddressSpace::kHandle:
                break;
            case core::AddressSpace::kPrivate:
                out << "thread ";
                break;
            case core::AddressSpace::kWorkgroup:
                out << "threadgroup ";
                break;
            default:
                TINT_IR_ICE(ir_) << "unhandled variable address space";
        }

        EmitType(out, ptr->UnwrapPtr());
        out << " " << NameOf(v->Result(0));

        if (v->Initializer()) {
            out << " = ";
            EmitValue(out, v->Initializer());
        } else if (space == core::AddressSpace::kPrivate ||
                   space == core::AddressSpace::kFunction) {
            out << " = ";
            EmitZeroValue(out, ptr->UnwrapPtr());
        }
        out << ";";
    }

    /// Emit a let instruction
    /// @param l the let instruction
    void EmitLet(const core::ir::Let* l) {
        auto out = Line();
        EmitType(out, l->Result(0)->Type());
        out << " const " << NameOf(l->Result(0)) << " = ";
        EmitAndTakeAddressIfNeeded(out, l->Value());
        out << ";";
    }

    void EmitExitLoop() { Line() << "break;"; }

    void EmitBreakIf(const core::ir::BreakIf* b) {
        auto out = Line();
        out << "if (";
        EmitValue(out, b->Condition());
        out << ") { break; }";
    }

    void EmitContinue() {
        if (emit_continuing_) {
            emit_continuing_();
        }
        Line() << "continue;";
    }

    void EmitLoop(const core::ir::Loop* l) {
        // Note, we can't just emit the continuing inside a conditional at the top of the loop
        // because any variable declared in the block must be visible to the continuing.
        //
        // loop {
        //   var a = 3;
        //   continue {
        //     let y = a;
        //   }
        // }

        auto emit_continuing = [&] {
            Line() << "{";
            {
                const ScopedIndent si(current_buffer_);
                EmitBlock(l->Continuing());
            }
            Line() << "}";
        };
        TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);

        Line() << "{";
        {
            ScopedIndent init(current_buffer_);
            EmitBlock(l->Initializer());

            Line() << "while(true) {";
            {
                ScopedIndent si(current_buffer_);
                EmitBlock(l->Body());
            }
            Line() << "}";
        }
        Line() << "}";
    }

    void EmitExitSwitch() { Line() << "break;"; }

    void EmitSwitch(const core::ir::Switch* s) {
        {
            auto out = Line();
            out << "switch(";
            EmitValue(out, s->Condition());
            out << ") {";
        }
        {
            ScopedIndent blk(current_buffer_);
            for (auto& case_ : s->Cases()) {
                for (auto& sel : case_.selectors) {
                    if (sel.IsDefault()) {
                        Line() << "default:";
                    } else {
                        auto out = Line();
                        out << "case ";
                        EmitValue(out, sel.val);
                        out << ":";
                    }
                }
                Line() << "{";
                {
                    ScopedIndent ci(current_buffer_);
                    EmitBlock(case_.block);
                }
                Line() << "}";
            }
        }
        Line() << "}";
    }

    void IdxToComponent(StringStream& out, uint32_t idx) {
        switch (idx) {
            case 0:
                out << "x";
                break;
            case 1:
                out << "y";
                break;
            case 2:
                out << "z";
                break;
            case 3:
                out << "w";
                break;
            default:
                TINT_UNREACHABLE() << "invalid index for component";
        }
    }

    void EmitSwizzle(StringStream& out, const core::ir::Swizzle* swizzle) {
        EmitValue(out, swizzle->Object());
        out << ".";
        for (const auto i : swizzle->Indices()) {
            IdxToComponent(out, i);
        }
    }

    void EmitVectorAccess(StringStream& out, const core::ir::Value* index) {
        if (auto* cnst = index->As<core::ir::Constant>()) {
            out << ".";
            IdxToComponent(out, cnst->Value()->ValueAs<uint32_t>());
        } else {
            out << "[";
            EmitValue(out, index);
            out << "]";
        }
    }

    void EmitStoreVectorElement(const core::ir::StoreVectorElement* s) {
        auto out = Line();

        EmitAndDerefIfNeeded(out, s->To());
        EmitVectorAccess(out, s->Index());
        out << " = ";
        EmitValue(out, s->Value());
        out << ";";
    }

    void EmitLoadVectorElement(StringStream& out, const core::ir::LoadVectorElement* l) {
        EmitAndDerefIfNeeded(out, l->From());
        EmitVectorAccess(out, l->Index());
    }

    /// Emit an if instruction
    /// @param if_ the if instruction
    void EmitIf(const core::ir::If* if_) {
        {
            auto out = Line();
            out << "if (";
            EmitValue(out, if_->Condition());
            out << ") {";
        }

        {
            ScopedIndent si(current_buffer_);
            EmitBlockInstructions(if_->True());
        }

        if (if_->False() && !if_->False()->IsEmpty()) {
            Line() << "} else {";

            ScopedIndent si(current_buffer_);
            EmitBlockInstructions(if_->False());
        }

        Line() << "}";
    }

    /// Emit a return instruction
    /// @param r the return instruction
    void EmitReturn(const core::ir::Return* r) {
        // If this return has no arguments and the current block is for the function which is
        // being returned, skip the return.
        if (current_block_ == current_function_->Block() && r->Args().IsEmpty()) {
            return;
        }

        auto out = Line();
        out << "return";
        if (!r->Args().IsEmpty()) {
            out << " ";
            EmitValue(out, r->Args().Front());
        }
        out << ";";
    }

    /// Emit an unreachable instruction
    void EmitUnreachable() {
        Line() << "/* unreachable */";
        if (!current_function_->ReturnType()->Is<core::type::Void>()) {
            // If this is inside a non-void function, emit a return statement to avoid potential
            // errors due to missing return statements.
            auto out = Line();
            out << "return ";
            EmitZeroValue(out, current_function_->ReturnType());
            out << ";";
        }
    }

    /// Emit a discard instruction
    void EmitDiscard() { Line() << "discard_fragment();"; }

    /// Emit a load
    void EmitLoad(StringStream& out, const core::ir::Load* l) {
        EmitAndDerefIfNeeded(out, l->From());
    }

    /// Emit a store
    void EmitStore(const core::ir::Store* s) {
        auto out = Line();

        EmitAndDerefIfNeeded(out, s->To());
        out << " = ";
        EmitValue(out, s->From());
        out << ";";
    }

    /// Emit a bitcast instruction
    void EmitBitcast(StringStream& out, const core::ir::Bitcast* b) {
        out << "as_type<";
        EmitType(out, b->Result(0)->Type());
        out << ">(";
        EmitValue(out, b->Val());
        out << ")";
    }

    /// Emit an accessor
    void EmitAccess(StringStream& out, const core::ir::Access* a) {
        EmitAndDerefIfNeeded(out, a->Object());

        auto* current_type = a->Object()->Type();
        for (auto* index : a->Indices()) {
            TINT_ASSERT(current_type);

            current_type = current_type->UnwrapPtr();
            Switch(
                current_type,  //
                [&](const core::type::Struct* s) {
                    auto* c = index->As<core::ir::Constant>();
                    auto* member = s->Members()[c->Value()->ValueAs<uint32_t>()];
                    out << "." << NameOf(member);
                    current_type = member->Type();
                },
                [&](const core::type::Vector*) {  //
                    EmitVectorAccess(out, index);
                },
                [&](Default) {
                    out << "[";
                    EmitValue(out, index);
                    out << "]";
                    current_type = current_type->Element(0);
                });
        }
    }

    void EmitCallStmt(const core::ir::Call* c) {
        if (!c->Result(0)->IsUsed()) {
            auto out = Line();
            EmitValue(out, c->Result(0));
            out << ";";
        }
    }

    void EmitMslBuiltinCall(StringStream& out, const msl::ir::BuiltinCall* c) {
        if (c->Func() == msl::BuiltinFn::kThreadgroupBarrier) {
            auto flags = c->Args()[0]->As<core::ir::Constant>()->Value()->ValueAs<uint8_t>();
            out << "threadgroup_barrier(";
            bool emitted_flag = false;

            auto emit = [&](BarrierType type, const std::string& name) {
                if ((flags & type) != type) {
                    return;
                }

                if (emitted_flag) {
                    out << " | ";
                }
                emitted_flag = true;
                out << "mem_flags::mem_" << name;
            };
            emit(BarrierType::kDevice, "device");
            emit(BarrierType::kThreadGroup, "threadgroup");
            emit(BarrierType::kTexture, "texture");

            out << ")";
            return;
        } else if (c->Func() == msl::BuiltinFn::kSimdBallot) {
            out << "as_type<uint2>((simd_vote::vote_t)simd_ballot(";
            EmitValue(out, c->Args()[0]);
            out << "))";
            return;
        } else if (c->Func() == msl::BuiltinFn::kConvert) {
            EmitType(out, c->Result(0)->Type());
            out << "(";
            EmitValue(out, c->Operand(0));
            out << ")";
            return;
        }

        out << c->Func() << "(";
        bool needs_comma = false;
        for (const auto* arg : c->Args()) {
            if (needs_comma) {
                out << ", ";
            }
            EmitAndTakeAddressIfNeeded(out, arg);
            needs_comma = true;
        }
        out << ")";
    }

    void EmitMslMemberBuiltinCall(StringStream& out, const msl::ir::MemberBuiltinCall* c) {
        if (c->Func() == BuiltinFn::kFence) {
            // If this is a fence builtin, we need to `const_cast<>` the object to remove the
            // `const` qualifier. We do this to work around an MSL bug that prevents us from being
            // able to use texture fence intrinsics when texture handles are stored inside
            // const-qualified structures (see crbug.com/365570202).
            out << "const_cast<";
            EmitType(out, c->Object()->Type());
            out << "thread &>(";
            EmitValue(out, c->Object());
            out << ")";
        } else {
            EmitValue(out, c->Object());
        }

        out << "." << c->Func() << "(";
        bool needs_comma = false;
        for (const auto* arg : c->Args()) {
            if (needs_comma) {
                out << ", ";
            }
            EmitAndTakeAddressIfNeeded(out, arg);
            needs_comma = true;
        }
        out << ")";
    }

    void EmitCoreBuiltinCall(StringStream& out, const core::ir::CoreBuiltinCall* c) {
        EmitCoreBuiltinName(out, c->Func());
        out << "(";

        size_t i = 0;
        for (const auto* arg : c->Args()) {
            if (i > 0) {
                out << ", ";
            }
            ++i;

            EmitAndTakeAddressIfNeeded(out, arg);
        }
        out << ")";
    }

    void EmitCoreBuiltinName(StringStream& out, core::BuiltinFn func) {
        switch (func) {
            case core::BuiltinFn::kAbs:
            case core::BuiltinFn::kAcos:
            case core::BuiltinFn::kAcosh:
            case core::BuiltinFn::kAll:
            case core::BuiltinFn::kAny:
            case core::BuiltinFn::kAsin:
            case core::BuiltinFn::kAsinh:
            case core::BuiltinFn::kAtan2:
            case core::BuiltinFn::kAtan:
            case core::BuiltinFn::kAtanh:
            case core::BuiltinFn::kCeil:
            case core::BuiltinFn::kClamp:
            case core::BuiltinFn::kCos:
            case core::BuiltinFn::kCosh:
            case core::BuiltinFn::kCross:
            case core::BuiltinFn::kDeterminant:
            case core::BuiltinFn::kExp2:
            case core::BuiltinFn::kExp:
            case core::BuiltinFn::kFloor:
            case core::BuiltinFn::kFma:
            case core::BuiltinFn::kFract:
            case core::BuiltinFn::kLdexp:
            case core::BuiltinFn::kLog2:
            case core::BuiltinFn::kLog:
            case core::BuiltinFn::kMax:
            case core::BuiltinFn::kMin:
            case core::BuiltinFn::kMix:
            case core::BuiltinFn::kNormalize:
            case core::BuiltinFn::kReflect:
            case core::BuiltinFn::kRefract:
            case core::BuiltinFn::kSaturate:
            case core::BuiltinFn::kSelect:
            case core::BuiltinFn::kSin:
            case core::BuiltinFn::kSinh:
            case core::BuiltinFn::kSqrt:
            case core::BuiltinFn::kStep:
            case core::BuiltinFn::kTan:
            case core::BuiltinFn::kTanh:
            case core::BuiltinFn::kTranspose:
            case core::BuiltinFn::kTrunc:
                out << func;
                break;
            case core::BuiltinFn::kPow:
                out << "powr";
                break;
            case core::BuiltinFn::kCountLeadingZeros:
                out << "clz";
                break;
            case core::BuiltinFn::kCountOneBits:
                out << "popcount";
                break;
            case core::BuiltinFn::kCountTrailingZeros:
                out << "ctz";
                break;
            case core::BuiltinFn::kDpdx:
            case core::BuiltinFn::kDpdxCoarse:
            case core::BuiltinFn::kDpdxFine:
                out << "dfdx";
                break;
            case core::BuiltinFn::kDpdy:
            case core::BuiltinFn::kDpdyCoarse:
            case core::BuiltinFn::kDpdyFine:
                out << "dfdy";
                break;
            case core::BuiltinFn::kExtractBits:
                out << "extract_bits";
                break;
            case core::BuiltinFn::kInsertBits:
                out << "insert_bits";
                break;
            case core::BuiltinFn::kFwidth:
            case core::BuiltinFn::kFwidthCoarse:
                out << "fwidth";
                break;
            case core::BuiltinFn::kFaceForward:
                out << "faceforward";
                break;
            case core::BuiltinFn::kPack4X8Snorm:
                out << "pack_float_to_snorm4x8";
                break;
            case core::BuiltinFn::kPack4X8Unorm:
                out << "pack_float_to_unorm4x8";
                break;
            case core::BuiltinFn::kPack2X16Snorm:
                out << "pack_float_to_snorm2x16";
                break;
            case core::BuiltinFn::kPack2X16Unorm:
                out << "pack_float_to_unorm2x16";
                break;
            case core::BuiltinFn::kQuadBroadcast:
                out << "quad_broadcast";
                break;
            case core::BuiltinFn::kReverseBits:
                out << "reverse_bits";
                break;
            case core::BuiltinFn::kRound:
                out << "rint";
                break;
            case core::BuiltinFn::kSmoothstep:
                out << "smoothstep";
                break;
            case core::BuiltinFn::kSubgroupElect:
                out << "simd_is_first";
                break;
            case core::BuiltinFn::kSubgroupBroadcast:
                out << "simd_broadcast";
                break;
            case core::BuiltinFn::kSubgroupBroadcastFirst:
                out << "simd_broadcast_first";
                break;
            case core::BuiltinFn::kSubgroupShuffle:
                out << "simd_shuffle";
                break;
            case core::BuiltinFn::kSubgroupShuffleXor:
                out << "simd_shuffle_xor";
                break;
            case core::BuiltinFn::kSubgroupShuffleUp:
                out << "simd_shuffle_up";
                break;
            case core::BuiltinFn::kSubgroupShuffleDown:
                out << "simd_shuffle_down";
                break;
            case core::BuiltinFn::kSubgroupAdd:
                out << "simd_sum";
                break;
            case core::BuiltinFn::kSubgroupInclusiveAdd:
                out << "simd_prefix_inclusive_sum";
                break;
            case core::BuiltinFn::kSubgroupExclusiveAdd:
                out << "simd_prefix_exclusive_sum";
                break;
            case core::BuiltinFn::kSubgroupMul:
                out << "simd_product";
                break;
            case core::BuiltinFn::kSubgroupInclusiveMul:
                out << "simd_prefix_inclusive_product";
                break;
            case core::BuiltinFn::kSubgroupExclusiveMul:
                out << "simd_prefix_exclusive_product";
                break;
            case core::BuiltinFn::kSubgroupAnd:
                out << "simd_and";
                break;
            case core::BuiltinFn::kSubgroupOr:
                out << "simd_or";
                break;
            case core::BuiltinFn::kSubgroupXor:
                out << "simd_xor";
                break;
            case core::BuiltinFn::kSubgroupMin:
                out << "simd_min";
                break;
            case core::BuiltinFn::kSubgroupMax:
                out << "simd_max";
                break;
            case core::BuiltinFn::kSubgroupAll:
                out << "simd_all";
                break;
            case core::BuiltinFn::kSubgroupAny:
                out << "simd_any";
                break;
            case core::BuiltinFn::kInverseSqrt:
                out << "rsqrt";
                break;
            case core::BuiltinFn::kUnpack4X8Snorm:
                out << "unpack_snorm4x8_to_float";
                break;
            case core::BuiltinFn::kUnpack4X8Unorm:
                out << "unpack_unorm4x8_to_float";
                break;
            case core::BuiltinFn::kUnpack2X16Snorm:
                out << "unpack_snorm2x16_to_float";
                break;
            case core::BuiltinFn::kUnpack2X16Unorm:
                out << "unpack_unorm2x16_to_float";
                break;
            default:
                TINT_UNREACHABLE() << "unhandled: " << func;
        }
    }

    /// Emits a user call instruction
    void EmitUserCall(StringStream& out, const core::ir::UserCall* c) {
        out << NameOf(c->Target()) << "(";
        size_t i = 0;
        for (const auto* arg : c->Args()) {
            if (i > 0) {
                out << ", ";
            }
            ++i;

            EmitAndTakeAddressIfNeeded(out, arg);
        }
        out << ")";
    }

    /// Emit a constructor
    void EmitConstruct(StringStream& out, const core::ir::Construct* c) {
        Switch(
            c->Result(0)->Type(),
            [&](const core::type::Array*) {
                EmitType(out, c->Result(0)->Type());
                out << "{";
                size_t i = 0;
                for (auto* arg : c->Args()) {
                    if (i > 0) {
                        out << ", ";
                    }
                    EmitValue(out, arg);
                    i++;
                }
                out << "}";
            },
            [&](const core::type::Struct* struct_ty) {
                EmitStructType(struct_ty);
                out << StructName(struct_ty);
                out << "{";
                size_t i = 0;
                bool needs_comma = false;
                for (auto* arg : c->Args()) {
                    if (arg->Is<tint::core::ir::Unused>()) {
                        // Skip `unused` values.
                        i++;
                        continue;
                    }
                    if (needs_comma) {
                        out << ", ";
                    }
                    // Emit field designators for structures so that we can skip padding members and
                    // arguments that are `undef` or `unused` values.
                    auto name = NameOf(struct_ty->Members()[i]);
                    out << "." << name << "=";
                    EmitAndTakeAddressIfNeeded(out, arg);
                    needs_comma = true;
                    i++;
                }
                out << "}";
            },
            [&](Default) {
                EmitType(out, c->Result(0)->Type());
                out << "(";
                size_t i = 0;
                for (auto* arg : c->Args()) {
                    if (i > 0) {
                        out << ", ";
                    }
                    EmitValue(out, arg);
                    i++;
                }
                out << ")";
            });
    }

    /// Handles generating a address space
    /// @param out the output of the type stream
    /// @param sc the address space to generate
    void EmitAddressSpace(StringStream& out, core::AddressSpace sc) {
        switch (sc) {
            case core::AddressSpace::kFunction:
            case core::AddressSpace::kPrivate:
                out << "thread";
                break;
            case core::AddressSpace::kWorkgroup:
                out << "threadgroup";
                break;
            case core::AddressSpace::kStorage:
                out << "device";
                break;
            case core::AddressSpace::kUniform:
                out << "constant";
                break;
            default:
                TINT_IR_ICE(ir_) << "unhandled address space: " << sc;
        }
    }

    /// Emit a type
    /// @param out the stream to emit too
    /// @param ty the type to emit
    void EmitType(StringStream& out, const core::type::Type* ty) {
        tint::Switch(
            ty,                                               //
            [&](const core::type::Bool*) { out << "bool"; },  //
            [&](const core::type::Void*) { out << "void"; },  //
            [&](const core::type::F32*) { out << "float"; },  //
            [&](const core::type::F16*) { out << "half"; },   //
            [&](const core::type::I32*) { out << "int"; },    //
            [&](const core::type::U32*) { out << "uint"; },   //
            [&](const core::type::U64*) { out << "ulong"; },  //
            [&](const core::type::I8*) { out << "char"; },    //
            [&](const core::type::U8*) { out << "uchar"; },   //
            [&](const core::type::Array* arr) { EmitArrayType(out, arr); },
            [&](const core::type::Vector* vec) { EmitVectorType(out, vec); },
            [&](const core::type::Matrix* mat) { EmitMatrixType(out, mat); },
            [&](const core::type::Atomic* atomic) { EmitAtomicType(out, atomic); },
            [&](const core::type::Pointer* ptr) { EmitPointerType(out, ptr); },
            [&](const core::type::Sampler*) { out << "sampler"; },  //
            [&](const core::type::Texture* tex) { EmitTextureType(out, tex); },
            [&](const core::type::Struct* str) {
                out << StructName(str);

                TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);
                EmitStructType(str);
            },  //

            // MSL builtin types.
            [&](const msl::type::Bias*) { out << "bias"; },  //
            [&](const msl::type::Gradient* g) {
                out << "gradient";
                switch (g->Dim()) {
                    case type::Gradient::Dim::k2d:
                        out << "2d";
                        break;
                    case type::Gradient::Dim::k3d:
                        out << "3d";
                        break;
                    case type::Gradient::Dim::kCube:
                        out << "cube";
                        break;
                }
            },                                                 //
            [&](const msl::type::Level*) { out << "level"; },  //
            [&](const core::type::SubgroupMatrix* sm) {
                TINT_ASSERT((sm->Type()->IsAnyOf<core::type::F32, core::type::F16>()));
                TINT_ASSERT(sm->Columns() == 8);
                TINT_ASSERT(sm->Rows() == 8);

                out << "simdgroup_";
                EmitType(out, sm->Type());
                out << sm->Columns() << "x" << sm->Rows();
            },

            TINT_ICE_ON_NO_MATCH);
    }

    /// Handles generating a pointer declaration
    /// @param out the output stream
    /// @param ptr the pointer to emit
    void EmitPointerType(StringStream& out, const core::type::Pointer* ptr) {
        if (ptr->Access() == core::Access::kRead) {
            out << "const ";
        }
        EmitAddressSpace(out, ptr->AddressSpace());
        out << " ";
        EmitType(out, ptr->StoreType());
        out << "*";
    }

    /// Handles generating an atomic declaration
    /// @param out the output stream
    /// @param atomic the atomic to emit
    void EmitAtomicType(StringStream& out, const core::type::Atomic* atomic) {
        if (atomic->Type()->Is<core::type::I32>()) {
            out << "atomic_int";
            return;
        }
        if (DAWN_LIKELY(atomic->Type()->Is<core::type::U32>())) {
            out << "atomic_uint";
            return;
        }
        TINT_ICE() << "unhandled atomic type " << atomic->Type()->FriendlyName();
    }

    /// Handles generating an array declaration
    /// @param out the output stream
    /// @param arr the array to emit
    void EmitArrayType(StringStream& out, const core::type::Array* arr) {
        out << ArrayTemplateName() << "<";
        EmitType(out, arr->ElemType());
        out << ", ";
        if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
            out << "1";
        } else {
            auto count = arr->ConstantCount();
            if (!count) {
                TINT_IR_ICE(ir_) << core::type::Array::kErrExpectedConstantCount;
            }
            out << count.value();
        }
        out << ">";
    }

    /// Handles generating a vector declaration
    /// @param out the output stream
    /// @param vec the vector to emit
    void EmitVectorType(StringStream& out, const core::type::Vector* vec) {
        if (vec->Packed()) {
            out << "packed_";
        }
        EmitType(out, vec->Type());
        out << vec->Width();
    }

    /// Handles generating a matrix declaration
    /// @param out the output stream
    /// @param mat the matrix to emit
    void EmitMatrixType(StringStream& out, const core::type::Matrix* mat) {
        EmitType(out, mat->Type());
        out << mat->Columns() << "x" << mat->Rows();
    }

    /// Handles generating a texture declaration
    /// @param out the output stream
    /// @param tex the texture to emit
    void EmitTextureType(StringStream& out, const core::type::Texture* tex) {
        if (DAWN_UNLIKELY(tex->Is<core::type::ExternalTexture>())) {
            TINT_IR_ICE(ir_) << "Multiplanar external texture transform was not run.";
        }

        if (tex->IsAnyOf<core::type::DepthTexture, core::type::DepthMultisampledTexture>()) {
            out << "depth";
        } else {
            out << "texture";
        }

        switch (tex->Dim()) {
            case core::type::TextureDimension::k1d:
                out << "1d";
                break;
            case core::type::TextureDimension::k2d:
                out << "2d";
                break;
            case core::type::TextureDimension::k2dArray:
                out << "2d_array";
                break;
            case core::type::TextureDimension::k3d:
                out << "3d";
                break;
            case core::type::TextureDimension::kCube:
                out << "cube";
                break;
            case core::type::TextureDimension::kCubeArray:
                out << "cube_array";
                break;
            default:
                TINT_IR_ICE(ir_) << "invalid texture dimensions";
        }
        if (tex->IsAnyOf<core::type::MultisampledTexture, core::type::DepthMultisampledTexture>()) {
            out << "_ms";
        }
        out << "<";
        TINT_DEFER(out << ">");

        tint::Switch(
            tex,  //
            [&](const core::type::DepthTexture*) { out << "float, access::sample"; },
            [&](const core::type::DepthMultisampledTexture*) { out << "float, access::read"; },
            [&](const core::type::StorageTexture* storage) {
                EmitType(out, storage->Type());
                out << ", ";

                std::string access_str;
                if (storage->Access() == core::Access::kRead) {
                    out << "access::read";
                } else if (storage->Access() == core::Access::kReadWrite) {
                    out << "access::read_write";
                } else if (storage->Access() == core::Access::kWrite) {
                    out << "access::write";
                } else {
                    TINT_IR_ICE(ir_) << "invalid access control for storage texture";
                }
            },
            [&](const core::type::MultisampledTexture* ms) {
                EmitType(out, ms->Type());
                out << ", access::read";
            },
            [&](const core::type::SampledTexture* sampled) {
                EmitType(out, sampled->Type());
                out << ", access::sample";
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Handles generating a struct declaration. If the structure has already been emitted, then
    /// this function will simply return without emitting anything.
    /// @param str the struct to generate
    void EmitStructType(const core::type::Struct* str) {
        if (!emitted_structs_.Add(str)) {
            return;
        }

        // This does not append directly to the preamble because a struct may require other
        // structs, or the array template, to get emitted before it. So, the struct emits into a
        // temporary text buffer, then anything it depends on will emit to the preamble first,
        // and then it copies the text buffer into the preamble.
        TextBuffer str_buf;
        Line(&str_buf) << "\n" << "struct " << StructName(str) << " {";

        bool is_host_shareable = host_shareable_structs_.Contains(str);

        // Emits a `/* 0xnnnn */` byte offset comment for a struct member.
        auto add_byte_offset_comment = [&](StringStream& out, uint32_t offset) {
            std::ios_base::fmtflags saved_flag_state(out.flags());
            out << "/* 0x" << std::hex << std::setfill('0') << std::setw(4) << offset << " */ ";
            out.flags(saved_flag_state);
        };

        auto add_padding = [&](uint32_t size, uint32_t msl_offset) {
            std::string name;
            do {
                name = UniqueIdentifier("tint_pad");
            } while (str->FindMember(ir_.symbols.Get(name)));

            auto out = Line(&str_buf);
            add_byte_offset_comment(out, msl_offset);
            out << ArrayTemplateName() << "<int8_t, " << size << "> " << name << ";";
        };

        str_buf.IncrementIndent();

        uint32_t msl_offset = 0;
        for (auto* mem : str->Members()) {
            auto out = Line(&str_buf);
            auto mem_name = NameOf(mem);
            auto ir_offset = mem->Offset();

            if (is_host_shareable) {
                if (DAWN_UNLIKELY(ir_offset < msl_offset)) {
                    // Unimplementable layout
                    TINT_IR_ICE(ir_) << "Structure member offset (" << ir_offset
                                     << ") is behind MSL offset (" << msl_offset << ")";
                }

                // Generate padding if required
                if (auto padding = ir_offset - msl_offset) {
                    add_padding(padding, msl_offset);
                    msl_offset += padding;
                }

                add_byte_offset_comment(out, msl_offset);
            }

            auto* ty = mem->Type();

            // The clip distances builtin is an array, but needs to be emitted as a C-style array
            // instead of using Tint's array wrapper. Additionally, the builtin attribute needs to
            // be emitted after the member name and before the array count.
            if (mem->Attributes().builtin == core::BuiltinValue::kClipDistances) {
                auto* arr = ty->As<core::type::Array>();
                out << "float " << mem_name << " [[clip_distance]] ["
                    << arr->ConstantCount().value_or(0) << "];";
                continue;
            }

            EmitType(out, ty);
            out << " " << mem_name;

            // Emit attributes
            auto& attributes = mem->Attributes();

            if (auto builtin = attributes.builtin) {
                auto name = BuiltinToAttribute(builtin.value());
                if (name.empty()) {
                    TINT_IR_ICE(ir_) << "unknown builtin";
                }
                out << " [[" << name << "]]";
            }

            if (auto location = attributes.location) {
                auto& pipeline_stage_uses = str->PipelineStageUses();
                if (DAWN_UNLIKELY(pipeline_stage_uses.Count() != 1)) {
                    TINT_IR_ICE(ir_) << "invalid entry point IO struct uses";
                }

                if (pipeline_stage_uses.Contains(core::type::PipelineStageUsage::kVertexInput)) {
                    out << " [[attribute(" << location.value() << ")]]";
                } else if (pipeline_stage_uses.Contains(
                               core::type::PipelineStageUsage::kVertexOutput)) {
                    out << " [[user(locn" << location.value() << ")]]";
                } else if (pipeline_stage_uses.Contains(
                               core::type::PipelineStageUsage::kFragmentInput)) {
                    out << " [[user(locn" << location.value() << ")]]";
                } else if (DAWN_LIKELY(pipeline_stage_uses.Contains(
                               core::type::PipelineStageUsage::kFragmentOutput))) {
                    out << " [[color(" << location.value() << ")]]";
                    if (auto blend_src = attributes.blend_src) {
                        out << " [[index(" << blend_src.value() << ")]]";
                    }
                } else {
                    TINT_IR_ICE(ir_) << "invalid use of location decoration";
                }
            }

            if (auto color = attributes.color) {
                out << " [[color(" << color.value() << ")]]";
            }

            if (auto interpolation = attributes.interpolation) {
                auto name = InterpolationToAttribute(interpolation->type, interpolation->sampling);
                if (name.empty()) {
                    TINT_IR_ICE(ir_) << "unknown interpolation attribute";
                }
                out << " [[" << name << "]]";
            }

            if (attributes.invariant) {
                if (invariant_define_name_.empty()) {
                    invariant_define_name_ = UniqueIdentifier("TINT_INVARIANT");
                    result_.has_invariant_attribute = true;

                    // 'invariant' attribute requires MSL 2.1 or higher.
                    // WGSL can ignore the invariant attribute on pre MSL 2.1 devices.
                    // See: https://github.com/gpuweb/gpuweb/issues/893#issuecomment-745537465
                    Line(&preamble_buffer_);
                    Line(&preamble_buffer_) << "#if __METAL_VERSION__ >= 210";
                    Line(&preamble_buffer_)
                        << "#define " << invariant_define_name_ << " [[invariant]]";
                    Line(&preamble_buffer_) << "#else";
                    Line(&preamble_buffer_) << "#define " << invariant_define_name_;
                    Line(&preamble_buffer_) << "#endif";
                    Line(&preamble_buffer_);
                }
                out << " " << invariant_define_name_;
            }

            out << ";";

            if (is_host_shareable) {
                // Calculate new MSL offset
                auto size_align = MslPackedTypeSizeAndAlign(ty);
                if (DAWN_UNLIKELY(msl_offset % size_align.align)) {
                    TINT_IR_ICE(ir_) << "Misaligned MSL structure member " << mem_name << " : "
                                     << ty->FriendlyName() << " offset: " << msl_offset
                                     << " align: " << size_align.align;
                }
                msl_offset += size_align.size;
            }
        }

        if (is_host_shareable && str->Size() != msl_offset) {
            add_padding(str->Size() - msl_offset, msl_offset);
        }

        str_buf.DecrementIndent();
        Line(&str_buf) << "};";

        preamble_buffer_.Append(str_buf);
    }

    /// Handles core::ir::Constant values
    /// @param out the stream to write the constant too
    /// @param c the constant to emit
    void EmitConstant(StringStream& out, const core::ir::Constant* c) {
        // Special cases for enum values.
        if (auto* order = c->As<msl::ir::Component>()) {
            switch (order->Value()->ValueAs<uint32_t>()) {
                case 0:
                    out << "component::x";
                    break;
                case 1:
                    out << "component::y";
                    break;
                case 2:
                    out << "component::z";
                    break;
                case 3:
                    out << "component::w";
                    break;
                default:
                    TINT_UNREACHABLE();
            }
            return;
        }
        if (auto* order = c->As<msl::ir::MemoryOrder>()) {
            TINT_ASSERT(order->Value()->ValueAs<u32>() ==
                        static_cast<u32>(std::memory_order_relaxed));
            out << "memory_order_relaxed";
            return;
        }

        EmitConstant(out, c->Value());
    }

    /// Handles core::constant::Value values
    /// @param out the stream to write the constant too
    /// @param c the constant to emit
    void EmitConstant(StringStream& out, const core::constant::Value* c) {
        auto emit_values = [&](uint32_t count) {
            for (size_t i = 0; i < count; i++) {
                if (i > 0) {
                    out << ", ";
                }
                EmitConstant(out, c->Index(i));
            }
        };

        tint::Switch(
            c->Type(),  //
            [&](const core::type::Bool*) { out << (c->ValueAs<bool>() ? "true" : "false"); },
            [&](const core::type::I32*) { PrintI32(out, c->ValueAs<i32>()); },
            [&](const core::type::U32*) { out << c->ValueAs<u32>() << "u"; },
            [&](const core::type::U64*) { out << c->ValueAs<u64>() << "ul"; },
            [&](const core::type::F32*) { PrintF32(out, c->ValueAs<f32>()); },
            [&](const core::type::F16*) { PrintF16(out, c->ValueAs<f16>()); },
            [&](const core::type::Vector* v) {
                EmitType(out, v);

                ScopedParen sp(out);
                if (auto* splat = c->As<core::constant::Splat>()) {
                    EmitConstant(out, splat->el);
                    return;
                }
                emit_values(v->Width());
            },
            [&](const core::type::Matrix* m) {
                EmitType(out, m);
                ScopedParen sp(out);
                emit_values(m->Columns());
            },
            [&](const core::type::Array* a) {
                EmitType(out, a);
                out << "{";
                TINT_DEFER(out << "}");

                if (c->AllZero()) {
                    return;
                }

                auto count = a->ConstantCount();
                if (!count) {
                    TINT_IR_ICE(ir_) << core::type::Array::kErrExpectedConstantCount;
                }
                emit_values(*count);
            },
            [&](const core::type::Struct* s) {
                EmitStructType(s);
                out << StructName(s) << "{";
                TINT_DEFER(out << "}");

                if (c->AllZero()) {
                    return;
                }

                auto members = s->Members();
                for (size_t i = 0; i < members.Length(); i++) {
                    if (i > 0) {
                        out << ", ";
                    }
                    out << "." << NameOf(members[i]) << "=";
                    EmitConstant(out, c->Index(i));
                }
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Emits the zero value for the given type
    /// @param out the stream to emit too
    /// @param ty the type
    void EmitZeroValue(StringStream& out, const core::type::Type* ty) {
        Switch(
            ty, [&](const core::type::Bool*) { out << "false"; },                     //
            [&](const core::type::F16*) { out << "0.0h"; },                           //
            [&](const core::type::F32*) { out << "0.0f"; },                           //
            [&](const core::type::I32*) { out << "0"; },                              //
            [&](const core::type::U32*) { out << "0u"; },                             //
            [&](const core::type::U64*) { out << "0u"; },                             //
            [&](const core::type::Vector* vec) { EmitZeroValue(out, vec->Type()); },  //
            [&](const core::type::Matrix* mat) {
                EmitType(out, mat);

                ScopedParen sp(out);
                EmitZeroValue(out, mat->Type());
            },
            [&](const core::type::Array*) { out << "{}"; },   //
            [&](const core::type::Struct*) { out << "{}"; },  //
            [&](const core::type::SubgroupMatrix* sm) {
                out << "make_filled_simdgroup_matrix<";
                EmitType(out, sm->Type());
                out << ", " << sm->Columns() << ", " << sm->Rows() << ">(";
                EmitZeroValue(out, sm->Type());
                out << ")";
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// @returns `true` if @p ident should be renamed
    bool ShouldRename(std::string_view ident) {
        return options_.strip_all_names || IsKeyword(ident) || !tint::utf8::IsASCII(ident);
    }

    /// @param s the structure
    /// @returns the name of the structure, taking special care of builtin structures that start
    /// with double underscores. If the structure is a builtin, then the returned name will be a
    /// unique name without the leading underscores.
    std::string StructName(const core::type::Struct* s) {
        return names_.GetOrAdd(s, [&] {
            auto name = s->Name().Name();
            if (HasPrefix(name, "__")) {
                name = UniqueIdentifier(name.substr(2));
            }
            if (ShouldRename(name)) {
                return UniqueIdentifier("tint_struct");
            }
            return name;
        });
    }

    /// @param m the struct member
    /// @returns the name to use for the struct member
    std::string NameOf(const core::type::StructMember* m) {
        return names_.GetOrAdd(m, [&] {
            auto name = m->Name().Name();
            if (ShouldRename(name)) {
                return UniqueIdentifier("tint_member");
            }
            return name;
        });
    }

    /// @param value the value to get the name of
    /// @returns the name of the given value, creating a new unique name if the value is unnamed in
    /// the module.
    std::string NameOf(const core::ir::Value* value) {
        return names_.GetOrAdd(value, [&] {
            auto sym = ir_.NameOf(value);
            if (!sym || ShouldRename(sym.NameView())) {
                return UniqueIdentifier("v");
            }
            return sym.Name();
        });
    }

    /// @return a new, unique identifier with the given prefix.
    /// @param prefix prefix to apply to the generated identifier
    std::string UniqueIdentifier(const std::string& prefix) {
        return ir_.symbols.New(prefix).Name();
    }
};

// This list is used for a binary search and must be kept in sorted order.
const char* const kReservedKeywordsMSL[] = {
    "HUGE_VALF",
    "HUGE_VALH",
    "INFINITY",
    "MAXFLOAT",
    "MAXHALF",
    "M_1_PI_F",
    "M_1_PI_H",
    "M_2_PI_F",
    "M_2_PI_H",
    "M_2_SQRTPI_F",
    "M_2_SQRTPI_H",
    "M_E_F",
    "M_E_H",
    "M_LN10_F",
    "M_LN10_H",
    "M_LN2_F",
    "M_LN2_H",
    "M_LOG10E_F",
    "M_LOG10E_H",
    "M_LOG2E_F",
    "M_LOG2E_H",
    "M_PI_2_F",
    "M_PI_2_H",
    "M_PI_4_F",
    "M_PI_4_H",
    "M_PI_F",
    "M_PI_H",
    "M_SQRT1_2_F",
    "M_SQRT1_2_H",
    "M_SQRT2_F",
    "M_SQRT2_H",
    "NAN",
    "access",
    "alignas",
    "alignof",
    "and",
    "and_eq",
    "array",
    "array_ref",
    "as_type",
    "asm",
    "atomic",
    "atomic_bool",
    "atomic_int",
    "atomic_uint",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "bool2",
    "bool3",
    "bool4",
    "break",
    "buffer",
    "case",
    "catch",
    "char",
    "char16_t",
    "char2",
    "char3",
    "char32_t",
    "char4",
    "class",
    "compl",
    "const",
    "const_cast",
    "const_reference",
    "constant",
    "constexpr",
    "continue",
    "decltype",
    "default",
    "delete",
    "depth2d",
    "depth2d_array",
    "depth2d_ms",
    "depth2d_ms_array",
    "depthcube",
    "depthcube_array",
    "device",
    "discard_fragment",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "extern",
    "false",
    "final",
    "float",
    "float2",
    "float2x2",
    "float2x3",
    "float2x4",
    "float3",
    "float3x2",
    "float3x3",
    "float3x4",
    "float4",
    "float4x2",
    "float4x3",
    "float4x4",
    "for",
    "fragment",
    "friend",
    "goto",
    "half",
    "half2",
    "half2x2",
    "half2x3",
    "half2x4",
    "half3",
    "half3x2",
    "half3x3",
    "half3x4",
    "half4",
    "half4x2",
    "half4x3",
    "half4x4",
    "if",
    "imageblock",
    "infinity",
    "inline",
    "int",
    "int16_t",
    "int2",
    "int3",
    "int32_t",
    "int4",
    "int64_t",
    "int8_t",
    "kernel",
    "long",
    "long2",
    "long3",
    "long4",
    "main",
    "matrix",
    "metal",
    "mutable",
    "namespace",
    "new",
    "noexcept",
    "not",
    "not_eq",
    "nullptr",
    "operator",
    "or",
    "or_eq",
    "override",
    "packed_bool2",
    "packed_bool3",
    "packed_bool4",
    "packed_char2",
    "packed_char3",
    "packed_char4",
    "packed_float2",
    "packed_float3",
    "packed_float4",
    "packed_half2",
    "packed_half3",
    "packed_half4",
    "packed_int2",
    "packed_int3",
    "packed_int4",
    "packed_short2",
    "packed_short3",
    "packed_short4",
    "packed_uchar2",
    "packed_uchar3",
    "packed_uchar4",
    "packed_uint2",
    "packed_uint3",
    "packed_uint4",
    "packed_ushort2",
    "packed_ushort3",
    "packed_ushort4",
    "patch_control_point",
    "private",
    "protected",
    "ptrdiff_t",
    "public",
    "r16snorm",
    "r16unorm",
    "r8unorm",
    "reference",
    "register",
    "reinterpret_cast",
    "return",
    "rg11b10f",
    "rg16snorm",
    "rg16unorm",
    "rg8snorm",
    "rg8unorm",
    "rgb10a2",
    "rgb9e5",
    "rgba16snorm",
    "rgba16unorm",
    "rgba8snorm",
    "rgba8unorm",
    "sampler",
    "short",
    "short2",
    "short3",
    "short4",
    "signed",
    "size_t",
    "sizeof",
    "srgba8unorm",
    "static",
    "static_assert",
    "static_cast",
    "struct",
    "switch",
    "template",
    "texture",
    "texture1d",
    "texture1d_array",
    "texture2d",
    "texture2d_array",
    "texture2d_ms",
    "texture2d_ms_array",
    "texture3d",
    "texture_buffer",
    "texturecube",
    "texturecube_array",
    "this",
    "thread",
    "thread_local",
    "threadgroup",
    "threadgroup_imageblock",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "uchar",
    "uchar2",
    "uchar3",
    "uchar4",
    "uint",
    "uint16_t",
    "uint2",
    "uint3",
    "uint32_t",
    "uint4",
    "uint64_t",
    "uint8_t",
    "ulong2",
    "ulong3",
    "ulong4",
    "uniform",
    "union",
    "unsigned",
    "ushort",
    "ushort2",
    "ushort3",
    "ushort4",
    "using",
    "vec",
    "vertex",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
};
bool IsKeyword(std::string_view ident) {
    return std::binary_search(std::begin(kReservedKeywordsMSL), std::end(kReservedKeywordsMSL),
                              ident);
}

}  // namespace

Result<Output> Print(core::ir::Module& module, const Options& options) {
    return Printer{module, options}.Generate();
}

}  // namespace tint::msl::writer
