// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/const_param_validator.h"

#include <cstdint>
#include <limits>
#include <string>
#include <utility>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/buffer.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir {

namespace {

/// The core IR const param validator.
class ConstParamValidator {
  public:
    /// Create a core const param validator
    /// @param mod the module to be validated
    explicit ConstParamValidator(Module& mod);

    /// Destructor
    ~ConstParamValidator();

    /// Runs the const param validator over the module provided during construction
    /// @returns success or failure
    Result<SuccessType> Run();

    void CheckSubgroupCall(const CoreBuiltinCall* call);
    void CheckBuiltinCall(const BuiltinCall* call);
    void CheckCoreBinaryCall(const CoreBinary* inst);
    void CheckExtractBitsCall(const CoreBuiltinCall* call);
    void CheckInsertBitsCall(const CoreBuiltinCall* call);
    void CheckLdexpCall(const CoreBuiltinCall* call);
    void CheckQuantizeToF16(const CoreBuiltinCall* call);
    void CheckClampCall(const CoreBuiltinCall* call);
    void CheckPack2x16float(const CoreBuiltinCall* call);
    void CheckSmoothstepCall(const CoreBuiltinCall* call);
    void CheckBinaryDivModCall(const CoreBinary* call);
    void CheckBinaryShiftCall(const CoreBinary* call);
    void CheckBuffersAndMatrices(const Var* var);

    struct UseInfo {
        Usage use;
        /// Variable/buffer size
        uint32_t storage_size;
        /// Accumulated offset to the pointer
        uint32_t offset;
        /// Pointed to size
        uint32_t pointer_size;
    };

    bool CheckBufferView(const CoreBuiltinCall* call, const Var* var, uint32_t buffer_size);
    bool CheckSubgroupMatrixMemory(const CoreBuiltinCall* call,
                                   const Var* var,
                                   const UseInfo& info);

    diag::Diagnostic& AddError(const Instruction& inst);
    diag::Diagnostic& AddNote(const Instruction& inst);

  private:
    Module& mod_;
    constant::Eval const_eval_;
    diag::List diagnostics_;
};

ConstParamValidator::ConstParamValidator(Module& mod)
    : mod_(mod), const_eval_(mod.constant_values, diagnostics_) {}

ConstParamValidator::~ConstParamValidator() = default;

diag::Diagnostic& ConstParamValidator::AddError(const Instruction& inst) {
    auto src = mod_.SourceOf(&inst);
    return diagnostics_.AddError(src);
}

diag::Diagnostic& ConstParamValidator::AddNote(const Instruction& inst) {
    auto src = mod_.SourceOf(&inst);
    return diagnostics_.AddNote(src);
}

const constant::Value* GetConstArg(const CoreBuiltinCall* call, uint32_t param_index) {
    if (call->Args().size() <= param_index) {
        return nullptr;
    }
    if (call->Args()[param_index] == nullptr) {
        return nullptr;
    }
    if (!call->Args()[param_index]->Is<ir::Constant>()) {
        return nullptr;
    }
    return call->Args()[param_index]->As<ir::Constant>()->Value();
}

void ConstParamValidator::CheckExtractBitsCall(const CoreBuiltinCall* call) {
    // This can be u32/i32 or vector of those types.
    auto* param0 = call->Args()[0];
    auto* const_val_offset = GetConstArg(call, 1);
    auto* const_val_count = GetConstArg(call, 2);
    if (const_val_count && const_val_offset) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, const_val_offset, const_val_count};
        [[maybe_unused]] auto result =
            const_eval_.extractBits(param0->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckInsertBitsCall(const CoreBuiltinCall* call) {
    // This can be u32/i32 or vector of those types.
    auto* param0 = call->Args()[0];
    auto* const_val_offset = GetConstArg(call, 2);
    auto* const_val_count = GetConstArg(call, 3);
    if (const_val_count && const_val_offset) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, zero, const_val_offset, const_val_count};
        [[maybe_unused]] auto result =
            const_eval_.insertBits(param0->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckLdexpCall(const CoreBuiltinCall* call) {
    auto* param0 = call->Args()[0];
    if (auto const_val = GetConstArg(call, 1)) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, const_val};
        [[maybe_unused]] auto result =
            const_eval_.ldexp(param0->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckQuantizeToF16(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 0)) {
        [[maybe_unused]] auto result = const_eval_.quantizeToF16(
            call->Result()->Type(), Vector{const_val}, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckPack2x16float(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 0)) {
        [[maybe_unused]] auto result = const_eval_.pack2x16float(
            call->Result()->Type(), Vector{const_val}, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckSubgroupCall(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 1)) {
        auto as_aint = const_val->ValueAs<AInt>();
        // User friendly param name.
        std::string paramName = "sourceLaneIndex";
        switch (call->Func()) {
            case core::BuiltinFn::kSubgroupShuffleXor:
                paramName = "mask";
                break;
            case core::BuiltinFn::kSubgroupShuffleUp:
            case core::BuiltinFn::kSubgroupShuffleDown:
                paramName = "delta";
                break;
            default:
                break;
        }

        if (as_aint >= tint::internal_limits::kMaxSubgroupSize) {
            AddError(*call) << "The " << paramName << " argument of " << call->FriendlyName()
                            << " must be less than " << tint::internal_limits::kMaxSubgroupSize;
        } else if (as_aint < 0) {
            AddError(*call) << "The " << paramName << " argument of " << call->FriendlyName()
                            << " must be greater than or equal to zero";
        }
    }
}

void ConstParamValidator::CheckClampCall(const CoreBuiltinCall* call) {
    auto* const_val_low = GetConstArg(call, 1);
    auto* const_val_high = GetConstArg(call, 2);
    if (const_val_low && const_val_high) {
        auto fakeArgs = Vector{const_val_low, const_val_low, const_val_high};
        [[maybe_unused]] auto result =
            const_eval_.clamp(call->Result()->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckSmoothstepCall(const CoreBuiltinCall* call) {
    auto* const_val_low = GetConstArg(call, 0);
    auto* const_val_high = GetConstArg(call, 1);
    if (const_val_low && const_val_high) {
        auto fakeArgs = Vector{const_val_low, const_val_high, const_val_high};
        [[maybe_unused]] auto result =
            const_eval_.smoothstep(call->Result()->Type(), fakeArgs, mod_.SourceOf(call));
    }
}

void ConstParamValidator::CheckBuiltinCall(const BuiltinCall* inst) {
    if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
        switch (call->Func()) {
            case core::BuiltinFn::kSubgroupShuffle:
            case core::BuiltinFn::kSubgroupShuffleXor:
            case core::BuiltinFn::kSubgroupShuffleUp:
            case core::BuiltinFn::kSubgroupShuffleDown:
                CheckSubgroupCall(call);
                break;
            case core::BuiltinFn::kExtractBits:
                CheckExtractBitsCall(call);
                break;
            case core::BuiltinFn::kInsertBits:
                CheckInsertBitsCall(call);
                break;
            case core::BuiltinFn::kLdexp:
                CheckLdexpCall(call);
                break;
            case core::BuiltinFn::kClamp:
                CheckClampCall(call);
                break;
            case core::BuiltinFn::kSmoothstep:
                CheckSmoothstepCall(call);
                break;
            case core::BuiltinFn::kQuantizeToF16:
                CheckQuantizeToF16(call);
                break;
            case core::BuiltinFn::kPack2X16Float:
                CheckPack2x16float(call);
                break;
            default:
                break;
        }
    }
}

void ConstParamValidator::CheckBinaryDivModCall(const CoreBinary* call) {
    // Integer division by zero should be checked for the partial evaluation case (only rhs
    // is const). FP division by zero is only invalid when the whole expression is
    // constant-evaluated.
    if (call->RHS()->Type()->IsIntegerScalarOrVector()) {
        auto rhs_constant = call->RHS()->As<ir::Constant>();
        if (rhs_constant && rhs_constant->Value()->AnyZero()) {
            AddError(*call) << "integer division by zero is invalid";
        }
    }
}

void ConstParamValidator::CheckBinaryShiftCall(const CoreBinary* call) {
    // If lhs value is a concrete type, and rhs is a const-expression greater than or equal
    // to the bit width of lhs, then it is a shader-creation error.
    const auto* elem_type = call->LHS()->Type()->DeepestElement();
    const uint32_t bit_width = elem_type->Size() * 8;
    if (auto* rhs_val_as_const = call->RHS()->As<ir::Constant>()) {
        auto* rhs_as_value = rhs_val_as_const->Value();
        for (size_t i = 0, n = rhs_as_value->NumElements(); i < n; i++) {
            auto* shift_val = n == 1 ? rhs_as_value : rhs_as_value->Index(i);
            if (shift_val->ValueAs<u32>() >= bit_width) {
                AddError(*call) << "shift "
                                << (call->Op() == core::BinaryOp::kShiftLeft ? "left" : "right")
                                << " value must be less than the bit width of the lhs, which is "
                                << bit_width;
                break;
            }
        }
    }
}

void ConstParamValidator::CheckCoreBinaryCall(const CoreBinary* call) {
    switch (call->Op()) {
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
            CheckBinaryDivModCall(call);
            break;
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            CheckBinaryShiftCall(call);
            break;
        default:
            break;
    }
}

bool ConstParamValidator::CheckBufferView(const CoreBuiltinCall* call,
                                          const Var* var,
                                          uint32_t buffer_size) {
    // Calculate the minimum type size.
    auto* store_ty = call->Result()->Type()->UnwrapPtr();
    uint64_t ty_required_size = 0;
    uint64_t ty_offset = 0;
    uint64_t ty_stride = 0;
    if (store_ty->HasFixedFootprint()) {
        ty_required_size = store_ty->Size();
    } else if (auto* str = store_ty->As<core::type::Struct>()) {
        auto* last = str->Members().Back();
        auto* arr_ty = last->Type()->As<core::type::Array>();
        ty_offset = last->Offset();
        ty_stride = arr_ty->ImplicitStride();
        ty_required_size = ty_offset + ty_stride;
    } else {
        ty_stride = store_ty->As<core::type::Array>()->ImplicitStride();
        ty_required_size = ty_stride;
    }

    // Error conditions:
    // For both bufferView and bufferArrayView:
    // * ty_required_size + offset < buffer_size
    // * offset % store_ty->Align() != 0
    // For bufferArrayView
    // * size + offset < buffer_size
    // * size < ty_required_size
    // * (size - offset) % stride != 0
    //
    // Also error if any addition overflows a uint32_t.

    uint64_t offset_val = 0;
    if (auto* const_offset = call->Args()[1]->As<Constant>()) {
        if (const_offset->Type()->IsSignedIntegerScalar()) {
            if (const_offset->Value()->ValueAs<int32_t>() < 0) {
                AddError(*call) << call->FriendlyName() << " offset must be greater than 0";
                return false;
            }
        }
        offset_val = const_offset->Value()->ValueAs<uint64_t>();
    }

    if (offset_val + ty_required_size > std::numeric_limits<uint32_t>::max()) {
        AddError(*call) << call->FriendlyName() << " requires a size beyond 32 bits";
        return false;
    }

    if (buffer_size > 0 && buffer_size < offset_val + ty_required_size) {
        AddError(*var) << "invalid buffer size (" << buffer_size << " bytes) when used with "
                       << call->FriendlyName() << " (" << offset_val + ty_required_size
                       << " bytes required)";
        return false;
    }

    if (offset_val % store_ty->Align() != 0) {
        AddError(*call) << call->FriendlyName() << " offset (" << offset_val
                        << " bytes) must be a multiple of result alignment (" << store_ty->Align()
                        << " bytes)";
        return false;
    }

    if (call->Func() == BuiltinFn::kBufferView) {
        return true;
    }

    uint64_t size_val = 0;
    if (auto* const_size = call->Args()[2]->As<Constant>()) {
        if (const_size->Type()->IsSignedIntegerScalar()) {
            if (const_size->Value()->ValueAs<int32_t>() < 0) {
                AddError(*call) << call->FriendlyName() << " size must be greater than 0";
                return false;
            }
        }
        size_val = const_size->Value()->ValueAs<uint64_t>();
        if (size_val == 0) {
            AddError(*call) << call->FriendlyName() << " cannot be 0 sized";
            return false;
        }
    }

    if (offset_val + size_val > std::numeric_limits<uint32_t>::max()) {
        AddError(*call) << call->FriendlyName() << " requires a size beyond 32 bits";
        return false;
    }

    if (buffer_size > 0 && buffer_size < size_val + offset_val) {
        AddError(*var) << "invalid buffer size (" << buffer_size << " bytes) when used with "
                       << call->FriendlyName() << " (" << size_val + offset_val
                       << " bytes required)";
        return false;
    }

    if (size_val > 0 && size_val < ty_required_size) {
        AddError(*call) << call->FriendlyName() << " has invalid size (" << size_val
                        << " bytes, requires " << ty_required_size << " bytes)";
        return false;
    }

    if (size_val > 0 && ((size_val - ty_offset) % ty_stride != 0)) {
        AddError(*call) << call->FriendlyName() << " size (" << size_val
                        << " bytes) minus type offset (" << ty_offset
                        << " bytes) must be a multiple of the type stride (" << ty_stride
                        << " bytes)";
        return false;
    }

    return true;
}

bool ConstParamValidator::CheckSubgroupMatrixMemory(const CoreBuiltinCall* call,
                                                    const Var* var,
                                                    const UseInfo& info) {
    const bool is_load = call->Func() == BuiltinFn::kSubgroupMatrixLoad;
    bool col_major = false;
    auto* offset_arg = call->Args()[1];
    const Value* stride_arg = nullptr;
    if (is_load) {
        col_major = std::get<Majorness>(call->ExplicitTemplateParams()[1]) == Majorness::kColMajor;
        stride_arg = call->Args()[2];
    } else {
        col_major = std::get<Majorness>(call->ExplicitTemplateParams()[0]) == Majorness::kColMajor;
        stride_arg = call->Args()[3];
    }
    auto* ty = is_load ? call->Result()->Type() : call->Args()[2]->Type();
    auto* mat_ty = ty->As<core::type::SubgroupMatrix>();
    auto* ele_ty = mat_ty->Type();

    auto* array_ty = call->Args()[0]->Type()->UnwrapPtr()->As<core::type::Array>();
    const uint32_t array_stride = array_ty->ImplicitStride();

    // Error conditions:
    // * stride is less than minimal required stride
    // * pointed to memory is smaller than matrix requires
    // * variable memory is smaller than total required
    //
    // Also if any calculation overflows 32 bits.

    const uint32_t major_size = col_major ? mat_ty->Columns() : mat_ty->Rows();
    const uint32_t minor_size = col_major ? mat_ty->Rows() : mat_ty->Columns();

    uint64_t offset = 0;
    if (auto* const_offset = offset_arg->As<Constant>()) {
        // Offset is array elements of shader scalar type.
        offset = const_offset->Value()->ValueAs<uint64_t>() * array_stride;

        if (offset > std::numeric_limits<uint32_t>::max()) {
            AddError(*call) << call->FriendlyName() << " has an offset exceeding 32 bits";
            return false;
        }
    }
    uint32_t min_stride = minor_size * ele_ty->Size();
    uint64_t stride = 0;
    if (auto* const_stride = stride_arg->As<Constant>()) {
        // Stride is in array elements of shader scalar type.
        stride = const_stride->Value()->ValueAs<uint64_t>() * array_stride;

        if (stride > std::numeric_limits<uint32_t>::max()) {
            AddError(*call) << call->FriendlyName() << " has a stride exceeding 32 bits";
            return false;
        }
        if (stride < min_stride) {
            AddError(*call) << call->FriendlyName() << " stride (" << stride
                            << " bytes) must be greater or equal to " << min_stride << " bytes";
            return false;
        }
    } else {
        stride = min_stride;
    }

    // Note: Offset and stride are in bytes.
    uint64_t mat_required_size = offset + static_cast<uint64_t>(stride) * (major_size - 1) +
                                 static_cast<uint64_t>(minor_size) * ele_ty->Size();
    // Round up to array element size.
    mat_required_size = RoundUp(static_cast<uint64_t>(array_stride), mat_required_size);
    if (mat_required_size > std::numeric_limits<uint32_t>::max()) {
        AddError(*call) << call->FriendlyName() << " has a memory requirement exceeding 32 bits";
        return false;
    }

    if (info.pointer_size > 0 && info.pointer_size < mat_required_size) {
        AddError(*call) << call->FriendlyName() << " requires more memory (" << mat_required_size
                        << " bytes) than pointed to (" << info.pointer_size << " bytes)";
        return false;
    }

    uint64_t mem_required_size = mat_required_size + info.offset;
    if (mem_required_size > std::numeric_limits<uint32_t>::max()) {
        AddError(*call) << " has a total memory requirement exceeding 32 bits";
        return false;
    }

    if (info.storage_size > 0 && info.storage_size < mem_required_size) {
        AddError(*var) << "invalid storage size (" << info.storage_size << " bytes) when used with "
                       << call->FriendlyName() << " (" << mem_required_size << " bytes required)";
        AddNote(*call) << call->FriendlyName() << " here";
        return false;
    }

    return true;
}

void ConstParamValidator::CheckBuffersAndMatrices(const Var* var) {
    uint32_t var_size = 0;
    if (var->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
        var_size = var->Result()->Type()->UnwrapPtr()->Size();
    }

    Vector<UseInfo, 4> uses;
    for (auto& u : var->Result()->UsagesSorted()) {
        uses.Push({u, var_size, 0, 0});
    }
    while (!uses.IsEmpty()) {
        auto info = uses.Pop();
        diag::Diagnostic error;
        bool errored = tint::Switch(
            info.use.instruction,
            [&](const Let* let) {
                for (auto& u : let->Result()->UsagesSorted()) {
                    uses.Push({u, info.storage_size, info.offset, info.pointer_size});
                }
                return false;
            },
            [&](const UserCall* user) {
                // If the buffer size is decreased at a function boundary, use that size
                // instead.
                auto* target = user->Target();
                auto* param = target->Params()[info.use.operand_index - user->ArgsOperandOffset()];
                auto* param_buffer_ty = param->Type()->UnwrapPtr()->As<core::type::Buffer>();
                uint32_t next_size = param_buffer_ty && param_buffer_ty->Size() > 0
                                         ? param_buffer_ty->Size()
                                         : info.storage_size;
                for (auto& u : param->UsagesSorted()) {
                    uses.Push({u, next_size, info.offset, info.pointer_size});
                }
                return false;
            },
            [&](const CoreBuiltinCall* call) {
                if (call->Func() == BuiltinFn::kBufferView ||
                    call->Func() == BuiltinFn::kBufferArrayView) {
                    if (!CheckBufferView(call, var, info.storage_size)) {
                        return true;
                    }

                    uint32_t offset = 0;
                    if (auto* const_offset = call->Args()[1]->As<Constant>()) {
                        offset = const_offset->Value()->ValueAs<uint32_t>();
                    }
                    uint32_t pointer_size = 0;
                    if (call->Func() == BuiltinFn::kBufferArrayView) {
                        if (auto* const_size = call->Args()[2]->As<Constant>()) {
                            pointer_size = const_size->Value()->ValueAs<uint32_t>();
                        }
                    } else if (call->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
                        // Use the bufferView result size if it has a fixed size.
                        pointer_size = call->Result()->Type()->UnwrapPtr()->Size();
                    }

                    // Keep tracing to catch subgroupMatrixLoad/Store transitive uses.
                    for (auto& u : call->Result()->UsagesSorted()) {
                        uses.Push({u, info.storage_size, offset, pointer_size});
                    }
                } else if (call->Func() == BuiltinFn::kSubgroupMatrixLoad ||
                           call->Func() == BuiltinFn::kSubgroupMatrixStore) {
                    if (!CheckSubgroupMatrixMemory(call, var, info)) {
                        return true;
                    }
                }

                return false;
            },
            [&](const Access* access) {
                auto* obj_ty = access->Object()->Type()->UnwrapPtr();

                uint32_t offset = 0;
                for (auto* idx : access->Indices()) {
                    uint32_t idx_value = 0;
                    if (auto* const_idx = idx->As<Constant>()) {
                        idx_value = const_idx->Value()->ValueAs<uint32_t>();
                    }
                    // Matrix and vector can't be hit on the way to a subgroupMatrix and access
                    // won't be hit at all on the way to buffer[Array]View so we only handle
                    // structure and array here.
                    tint::Switch(
                        obj_ty,  //
                        [&](const core::type::Array* ary) {
                            obj_ty = ary->ElemType();
                            offset += idx_value * ary->ImplicitStride();
                        },
                        [&](const core::type::Struct* s) {
                            auto* mem = s->Members()[idx_value];
                            obj_ty = mem->Type();
                            offset += mem->Offset();
                        },
                        [&](Default) {});
                }

                uint32_t pointer_size = info.pointer_size;
                if (access->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
                    // If the result has a fixed size, update pointer size.
                    pointer_size = access->Result()->Type()->UnwrapPtr()->Size();
                }

                for (auto& u : access->Result()->UsagesSorted()) {
                    // Accumulate the offset.
                    uses.Push({u, info.storage_size, info.offset + offset, pointer_size});
                }

                return false;
            },
            [&](Default) { return false; });
        if (errored) {
            return;
        }
    }
}

Result<SuccessType> ConstParamValidator::Run() {
    auto instructions = this->mod_.Instructions();

    for (auto inst : instructions) {
        tint::Switch(
            inst,                                                  //
            [&](const BuiltinCall* c) { CheckBuiltinCall(c); },    //
            [&](const CoreBinary* c) { CheckCoreBinaryCall(c); },  //
            [&](Default) {});
    }

    for (auto* inst : *this->mod_.root_block) {
        if (auto* var = inst->As<Var>()) {
            CheckBuffersAndMatrices(var);
        }
    }

    if (diagnostics_.ContainsErrors()) {
        return Failure{diagnostics_.Str()};
    }

    return Success;
}

}  // namespace

Result<SuccessType> ValidateConstParam(Module& mod) {
    ConstParamValidator v(mod);
    return v.Run();
}

}  // namespace tint::core::ir
