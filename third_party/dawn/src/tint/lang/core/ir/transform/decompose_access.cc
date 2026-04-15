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

#include "src/tint/lang/core/ir/transform/decompose_access.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
///
/// Each uniform buffer variable is rewritten so its store type is an array of vec4u
/// elements.  Accesses to original store type contents are rewritten to pick out the
/// right set of vec4u elements and then use bitcasts as needed to construct the originally
/// loaded value.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The transform options.
    const DecomposeAccessOptions& options;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    using VarTypePair = std::pair<core::ir::Var*, const core::type::Type*>;
    /// Maps a variable and type to the load function
    Hashmap<VarTypePair, core::ir::Function*, 2> var_and_type_to_load_fn_{};
    /// Maps a variable and type to the store function
    Hashmap<VarTypePair, core::ir::Function*, 2> var_and_type_to_store_fn_{};

    const type::Type* base_ty_ = nullptr;
    const type::Pointer* base_ptr_ty_ = nullptr;

    /// Process the module.
    void Process() {
        Vector<core::ir::Var*, 4> var_worklist;
        for (auto* inst : *ir.root_block) {
            // Allow this to run before or after PromoteInitializers by handling non-var root_block
            // entries
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            // DecomposeStorageAccess maybe have converted the var pointers into ByteAddressBuffer
            // objects. Since they've been changed, then they're Storage buffers and we don't care
            // about them here.
            auto* var_ty = var->Result()->Type()->As<core::type::Pointer>();
            if (!var_ty) {
                continue;
            }

            // Always decompose buffer types, otherwise depend on the options.
            if (var_ty->StoreType()->Is<type::Buffer>()) {
                var_worklist.Push(var);
            } else if ((var_ty->AddressSpace() == AddressSpace::kStorage && options.storage) ||
                       (var_ty->AddressSpace() == AddressSpace::kUniform && options.uniform) ||
                       (var_ty->AddressSpace() == AddressSpace::kWorkgroup && options.workgroup) ||
                       (var_ty->AddressSpace() == AddressSpace::kImmediate && options.immediate)) {
                // Atomics could be handled in some backends, but not in a generic way.
                if (!ContainsAtomic(var_ty->StoreType())) {
                    var_worklist.Push(var);
                }
            }
        }

        for (auto* var : var_worklist) {
            auto* result = var->Result();
            SetBaseEleType(var);

            auto usage_worklist = result->UsagesSorted();
            auto* var_ty = result->Type()->As<core::type::Pointer>();
            while (!usage_worklist.IsEmpty()) {
                auto usage = usage_worklist.Pop();
                auto* inst = usage.instruction;

                // Load instructions can be destroyed by the replacing access function
                if (!inst->Alive()) {
                    continue;
                }

                OffsetData od{};
                tint::Switch(
                    inst,  //
                    [&](core::ir::LoadVectorElement* l) { LoadVectorElement(l, var, od); },
                    [&](core::ir::StoreVectorElement* s) { StoreVectorElement(s, var, od); },
                    [&](core::ir::Load* l) { Load(l, var, od); },
                    [&](core::ir::Store* s) { Store(s, var, od); },
                    [&](core::ir::Access* a) { Access(a, var, a->Object()->Type(), od); },
                    [&](core::ir::Let* let) {
                        // The `let` is, essentially, an alias for the `var` as it's assigned
                        // directly. Gather all the `let` usages into our worklist, and then replace
                        // the `let` with the `var` itself.
                        for (auto& use : let->Result()->UsagesSorted()) {
                            usage_worklist.Push(use);
                        }
                        let->Result()->ReplaceAllUsesWith(result);
                        let->Destroy();
                    },
                    [&](core::ir::CoreBuiltinCall* call) {
                        switch (call->Func()) {
                            case core::BuiltinFn::kArrayLength:
                                ArrayLength(call, var, var_ty->StoreType(), {});
                                break;
                            case core::BuiltinFn::kBufferLength:
                                BufferLength(call, var, var_ty->StoreType());
                                break;
                            case core::BuiltinFn::kBufferView:
                                BufferView(call, var, var_ty->StoreType(), {});
                                break;
                            default:
                                TINT_IR_UNREACHABLE(ir);
                        }
                    },
                    TINT_ICE_ON_NO_MATCH);
            }

            auto HasRuntimeSize = [&](const type::Type* type) {
                return tint::Switch(
                    type,
                    [&](const type::Array* arr) {
                        return arr->Count()->Is<type::RuntimeArrayCount>();
                    },
                    [&](const type::Buffer* buf) {
                        return buf->Count()->Is<type::RuntimeArrayCount>();
                    },
                    [&](const type::Struct* str) {
                        auto* last =
                            str->Element(static_cast<uint32_t>(str->Members().Length()) - 1);
                        if (auto* arr = last->As<type::Array>()) {
                            return arr->Count()->Is<type::RuntimeArrayCount>();
                        }
                        return false;
                    },
                    [&](Default) { return false; });
            };

            // Swap the result type of the `var` to the new result type
            const type::Array* array_ty = nullptr;
            if (HasRuntimeSize(var_ty->StoreType())) {
                // Use a runtime-sized array of the base type.
                array_ty = ty.runtime_array(BaseEleType());
            } else {
                auto array_length = NumBaseElements(var_ty->StoreType());

                array_length =
                    std::max(array_length, options.minimum_array_size / BaseEleType()->Size());
                array_ty = ty.array(BaseEleType(), array_length);
            }
            result->SetType(ty.ptr(var_ty->AddressSpace(), array_ty, var_ty->Access()));
        }
    }

    const type::Type* BaseEleType() { return base_ty_; }

    const type::Pointer* BaseEleTypePtr() { return base_ptr_ty_; }

    // Returns the number of BaseEleType elements need to represent `type` rounded up.
    uint32_t NumBaseElements(const type::Type* type) {
        return (type->Size() + BaseEleType()->Size() - 1) / BaseEleType()->Size();
    }

    // OffsetData represents an unsigned integer expression.
    // The value is the sum of a const part, held in `byte_offset`, and
    // non-const parts in `byte_offset_expr`.
    struct OffsetData {
        uint32_t byte_offset = 0;
        Vector<core::ir::Value*, 4> byte_offset_expr{};
    };

    bool ContainsAtomic(const type::Type* type) const {
        return tint::Switch(
            type,  //
            [&](const type::Atomic*) { return true; },
            [&](const type::Array* array_ty) { return ContainsAtomic(array_ty->ElemType()); },
            [&](const type::Struct* struct_ty) {
                for (auto* member : struct_ty->Members()) {
                    if (ContainsAtomic(member->Type())) {
                        return true;
                    }
                }
                return false;
            },
            [&](Default) { return false; });
    }

    uint32_t SmallestElementSize(const type::Type* type) {
        return tint::Switch(
            type,  //
            [&](const type::Scalar* scalar) { return scalar->Size(); },
            [&](const type::Vector* vector) {
                if (vector->Width() == 3) {
                    return vector->Type()->Size();
                }
                return type->Size();
            },
            [&](const type::Matrix* matrix) { return SmallestElementSize(matrix->ColumnType()); },
            [&](const type::Array* array) { return SmallestElementSize(array->ElemType()); },
            [&](const type::Struct* str) {
                uint32_t size = std::numeric_limits<uint32_t>::max();
                for (auto* member : str->Members()) {
                    size = std::min(size, SmallestElementSize(member->Type()));
                }
                return size;
            },
            TINT_ICE_ON_NO_MATCH);
    }

    // Returns the smallest access size (load/store) in bytes for `var` up to a max of `16` bytes.
    uint32_t SmallestAccessSize(core::ir::Var* var) {
        uint32_t size = std::numeric_limits<uint32_t>::max();
        auto worklist = var->Result()->UsagesSorted();
        while (!worklist.IsEmpty()) {
            auto usage = worklist.Pop();
            auto* use = usage.instruction;

            size = std::min(
                size,
                tint::Switch(
                    use,
                    [&](core::ir::LoadVectorElement* lve) { return lve->Result()->Type()->Size(); },
                    [&](core::ir::StoreVectorElement* sve) { return sve->Value()->Type()->Size(); },
                    [&](core::ir::Load* load) {
                        return SmallestElementSize(load->Result()->Type());
                    },
                    [&](core::ir::Store* store) {
                        return SmallestElementSize(store->From()->Type());
                    },
                    [&](core::ir::Instruction* inst) {
                        bool bufferView = false;
                        if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                            bufferView = call->Func() == core::BuiltinFn::kBufferView;
                        }
                        if (inst->IsAnyOf<core::ir::Access, core::ir::Let>() || bufferView) {
                            for (auto u : inst->Result()->UsagesSorted()) {
                                worklist.Push(u);
                            }
                        }
                        if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                            if (call->Func() == core::BuiltinFn::kArrayLength) {
                                auto* ptr_ty = call->Args()[0]->Type()->As<type::Pointer>();
                                return SmallestElementSize(ptr_ty->StoreType());
                            }
                        }
                        return size;
                    }));
        }
        // No need to be larger than a vec4u.
        return std::min(size, 16u);
    }

    // Sets the base type based on the smallest access size.
    // Possible values are: u16, u32, vec2u, and vec4u.
    void SetBaseEleType(core::ir::Var* var) {
        uint32_t size = 0;
        auto* var_ty = var->Result()->Type()->As<core::type::Pointer>();
        if (var_ty->AddressSpace() == AddressSpace::kUniform) {
            size = 16u;
        } else if (var_ty->AddressSpace() == AddressSpace::kImmediate) {
            size = 4u;
        } else {
            size = SmallestAccessSize(var);
        }

        if (size == 2 || size == 6) {
            // 6 == vec3h so we must use a 2-byte type to be safe.
            base_ty_ = ty.u16();
        } else if (size < 8 || size == 12) {
            // 12 == vec3u so we must use a 4-byte type to be safe.
            base_ty_ = ty.u32();
        } else if (size < 16) {
            base_ty_ = ty.vec2u();
        } else if (size == 16) {
            base_ty_ = ty.vec4u();
        } else {
            TINT_UNREACHABLE();
        }

        base_ptr_ty_ = ty.ptr(var_ty->AddressSpace(), base_ty_, var_ty->Access());
    }

    // Note, must be called inside a builder insert block (Append, InsertBefore, etc)
    core::ir::Value* OffsetToValue(OffsetData offset) {
        core::ir::Value* val = nullptr;

        // If the offset is zero, skip setting val. This way, we won't add `0 +` and create useless
        // addition expressions, but if the offset is zero, and there are no expressions, make sure
        // we return the 0 value.
        if (offset.byte_offset != 0) {
            val = b.Value(u32(offset.byte_offset));
        } else if (offset.byte_offset_expr.IsEmpty()) {
            return b.Value(0_u);
        }

        for (core::ir::Value* expr : offset.byte_offset_expr) {
            if (!val) {
                val = expr;
            } else {
                val = b.Add(val, expr)->Result();
            }
        }

        return val;
    }

    // Converts a byte offset to an index into an array of base type.
    // After the transform runs the store type of the uniform buffer variable is an
    // array of vec4u.
    // Note, this must be called inside a builder insert block (Append, InsertBefore, etc)
    core::ir::Value* OffsetValueToArrayIndex(core::ir::Value* val) {
        if (auto* cnst = val->As<core::ir::Constant>()) {
            auto v = cnst->Value()->ValueAs<uint32_t>();
            return b.Value(u32(v / BaseEleType()->Size()));
        }
        return b.Divide(val, u32(BaseEleType()->Size()))->Result();
    }

    // Calculates the index of the vector element containing the byte at (byte_idx %
    // src_ty->Size()). Assumes the upper bits of byte_idx have already been used to access the
    // correct vector array element in the underlying variable.
    core::ir::Value* CalculateVectorOffset(core::ir::Value* byte_idx, const type::Vector* src_ty) {
        if (auto* byte_cnst = byte_idx->As<core::ir::Constant>()) {
            return b.Value(u32((byte_cnst->Value()->ValueAs<uint32_t>() % src_ty->Size()) /
                               src_ty->Type()->Size()));
        }
        // Note: Using bitwise-and and shift instead of modulo and divide here was necessary to
        // avoid an FXC miscompile. See https://crbug.com/454366353.
        return b
            .ShiftRight(b.And(byte_idx, b.Constant(u32(src_ty->Size() - 1))),
                        u32(log2(src_ty->Type()->Size())))
            ->Result();
    }

    // Note, must be called inside a builder insert block (Append, InsertBefore, etc)
    void UpdateOffsetData(core::ir::Value* v, uint32_t elm_size, OffsetData* offset) {
        tint::Switch(
            v,  //
            [&](core::ir::Constant* idx_value) {
                offset->byte_offset += idx_value->Value()->ValueAs<uint32_t>() * elm_size;
            },
            [&](core::ir::Value* val) {
                auto* idx = val;
                idx = b.InsertConvertIfNeeded(ty.u32(), val);
                offset->byte_offset_expr.Push(b.Multiply(idx, u32(elm_size))->Result());
            },
            TINT_ICE_ON_NO_MATCH);
    }

    void Access(core::ir::Access* a,
                core::ir::Var* var,
                const core::type::Type* obj_ty,
                OffsetData offset) {
        // Note, because we recurse through the `access` helper, the object passed in isn't
        // necessarily the originating `var` object, but maybe a partially resolved access chain
        // object.
        if (auto* view = obj_ty->As<core::type::MemoryView>()) {
            obj_ty = view->StoreType();
        }

        for (auto* idx_value : a->Indices()) {
            tint::Switch(
                obj_ty,
                [&](const core::type::Vector* v) {
                    b.InsertBefore(
                        a, [&] { UpdateOffsetData(idx_value, v->Type()->Size(), &offset); });
                    obj_ty = v->Type();
                },
                [&](const core::type::Matrix* m) {
                    b.InsertBefore(
                        a, [&] { UpdateOffsetData(idx_value, m->ColumnStride(), &offset); });
                    obj_ty = m->ColumnType();
                },
                [&](const core::type::Array* ary) {
                    b.InsertBefore(
                        a, [&] { UpdateOffsetData(idx_value, ary->ImplicitStride(), &offset); });
                    obj_ty = ary->ElemType();
                },
                [&](const core::type::Struct* s) {
                    auto* cnst = idx_value->As<core::ir::Constant>();

                    // A struct index must be a constant
                    TINT_IR_ASSERT(ir, cnst);

                    uint32_t idx = cnst->Value()->ValueAs<uint32_t>();
                    auto* mem = s->Members()[idx];
                    obj_ty = mem->Type();
                    offset.byte_offset += mem->Offset();
                },
                TINT_ICE_ON_NO_MATCH);
        }

        AccessUses(a, var, obj_ty, offset);
    }

    void BufferView(core::ir::CoreBuiltinCall* call,
                    core::ir::Var* var,
                    const type::Type* obj_type,
                    OffsetData offset) {
        auto* offset_arg = call->Args()[1];
        b.InsertBefore(call, [&] { UpdateOffsetData(offset_arg, 1, &offset); });
        obj_type = call->Result()->Type()->As<type::Pointer>()->StoreType();

        AccessUses(call, var, obj_type, offset);
    }

    void AccessUses(core::ir::Instruction* inst,
                    core::ir::Var* var,
                    const type::Type* obj_ty,
                    OffsetData offset) {
        auto usages = inst->Result()->UsagesSorted();
        while (!usages.IsEmpty()) {
            auto usage = usages.Pop();
            tint::Switch(
                usage.instruction,
                [&](core::ir::Let* let) {
                    // The `let` is essentially an alias to the `access`. So, add the `let`
                    // usages into the usage worklist, and replace the let with the access chain
                    // directly.
                    for (auto& u : let->Result()->UsagesSorted()) {
                        usages.Push(u);
                    }
                    let->Result()->ReplaceAllUsesWith(inst->Result());
                    let->Destroy();
                },
                [&](core::ir::Access* sub_access) {
                    // Treat an access chain of the access chain as a continuation of the outer
                    // chain. Pass through the object we stopped at and the current byte_offset
                    // and then restart the access chain replacement for the new access chain.
                    Access(sub_access, var, obj_ty, offset);
                },
                [&](core::ir::Load* ld) {
                    inst->Result()->RemoveUsage(usage);
                    Load(ld, var, offset);
                },
                [&](core::ir::Store* st) {
                    inst->Result()->RemoveUsage(usage);
                    Store(st, var, offset);
                },
                [&](core::ir::LoadVectorElement* lve) {
                    inst->Result()->RemoveUsage(usage);
                    LoadVectorElement(lve, var, offset);
                },
                [&](core::ir::StoreVectorElement* sve) {
                    inst->Result()->RemoveUsage(usage);
                    StoreVectorElement(sve, var, offset);
                },
                [&](core::ir::CoreBuiltinCall* call) {
                    // Note: we don't need to check for bufferView and bufferLength as they cannot
                    // be encountered after an access.
                    if (call->Func() == core::BuiltinFn::kArrayLength) {
                        ArrayLength(call, var, obj_ty, offset);
                    }
                },
                TINT_ICE_ON_NO_MATCH);
        }
        inst->Destroy();
    }

    void Load(core::ir::Load* ld, core::ir::Var* var, OffsetData offset) {
        b.InsertBefore(ld, [&] {
            auto* byte_idx = OffsetToValue(offset);
            auto* result = MakeLoad(ld, var, ld->Result()->Type(), byte_idx);
            ld->Result()->ReplaceAllUsesWith(result->Result());
        });
        ld->Destroy();
    }

    void LoadVectorElement(core::ir::LoadVectorElement* lve,
                           core::ir::Var* var,
                           OffsetData offset) {
        b.InsertBefore(lve, [&] {
            // Add the byte count from the start of the vector to the requested element to the
            // current offset calculation
            auto elem_byte_size = lve->Result()->Type()->DeepestElement()->Size();
            if (auto* cnst = lve->Index()->As<core::ir::Constant>()) {
                offset.byte_offset += (cnst->Value()->ValueAs<uint32_t>() * elem_byte_size);
            } else {
                offset.byte_offset_expr.Push(
                    b.Multiply(b.InsertConvertIfNeeded(ty.u32(), lve->Index()), u32(elem_byte_size))
                        ->Result());
            }

            auto* byte_idx = OffsetToValue(offset);
            auto* result = MakeLoad(lve, var, lve->Result()->Type(), byte_idx);
            lve->Result()->ReplaceAllUsesWith(result->Result());
        });
        lve->Destroy();
    }

    // Creates the appropriate load instructions for the given result type.
    core::ir::Instruction* MakeLoad(core::ir::Instruction* inst,
                                    core::ir::Var* var,
                                    const core::type::Type* result_ty,
                                    core::ir::Value* byte_idx) {
        return tint::Switch(
            result_ty,
            [&](const core::type::Struct* s) {
                auto* fn = GetLoadFunctionFor(inst, var, s);
                return b.Call(fn, byte_idx);
            },
            [&](const core::type::Matrix* m) {
                auto* fn = GetLoadFunctionFor(inst, var, m);
                return b.Call(fn, byte_idx);
            },
            [&](const core::type::Array* a) {
                auto* fn = GetLoadFunctionFor(inst, var, a);
                return b.Call(fn, byte_idx);
            },
            [&](const core::type::Vector* v) { return MakeVectorLoad(var, v, byte_idx); },
            [&](const core::type::Scalar* s) { return MakeScalarLoad(var, s, byte_idx); },
            TINT_ICE_ON_NO_MATCH);
    }

    // Returns a vector of `num_loads` load instructions of BaseEleType from `var` starting from
    // `start_idx`.
    // Note, must be called inside a builder insert block (Append, InsertBefore, etc)
    Vector<Instruction*, 4> MakeNLoadInsts(Var* var, Value* start_idx, uint32_t num_loads) {
        Vector<Instruction*, 4> loads;
        auto* idx = start_idx;
        for (uint32_t i = 0; i < num_loads; i++) {
            if (i > 0) {
                if (auto* cnst = idx->As<Constant>()) {
                    idx = b.Constant(u32(cnst->Value()->ValueAs<uint32_t>() + 1));
                } else {
                    idx = b.Add(idx, b.Constant(1_u))->Result();
                }
            }
            auto* access = b.Access(BaseEleTypePtr(), var, idx);
            loads.Push(b.Load(access));
        }
        return loads;
    }

    // Returns a vector of `num_loads` load values of BaseEleType from `var` starting from
    // `start_idx`.
    // Note, must be called inside a builder insert block (Append, InsertBefore, etc)
    Vector<Value*, 4> MakeNLoads(Var* var, Value* start_idx, uint32_t num_loads) {
        auto insts = MakeNLoadInsts(var, start_idx, num_loads);
        Vector<Value*, 4> vals;
        vals.Reserve(insts.Length());
        for (auto* inst : insts) {
            vals.Push(inst->Result());
        }
        return vals;
    }

    // Returns a value that is appropriately bitcasted or converted as necessary.
    // Note, must be called inside a builder insert block (Append, InsertBefore, etc)
    core::ir::Value* BitcastOrConvertIfNeeded(const core::type::Type* result_ty,
                                              core::ir::Value* from) {
        Value* value = from;
        if (result_ty->DeepestElement()->Is<type::Bool>()) {
            auto* new_ty = ty.MatchWidth(ty.u32(), result_ty);
            value = b.InsertBitcastIfNeeded(new_ty, from);
            return b.Convert(result_ty, value)->Result();
        }
        if (from->Type()->DeepestElement()->Is<type::Bool>()) {
            auto* new_ty = ty.MatchWidth(ty.u32(), from->Type());
            value = b.Convert(new_ty, from)->Result();
        }
        return b.InsertBitcastIfNeeded(result_ty, value);
    }

    // Returns an instruction that is appropriately bitcasted or converted as necessary.
    // Note, must be called inside a builder insert block (Append, InsertBefore, etc)
    core::ir::Instruction* BitcastOrConvertIfNeeded(const core::type::Type* result_ty,
                                                    core::ir::Instruction* from) {
        Instruction* inst = from;
        if (result_ty->DeepestElement()->Is<type::Bool>()) {
            auto* new_ty = ty.MatchWidth(ty.u32(), result_ty);
            inst = b.InsertBitcastIfNeeded(new_ty, from);
            return b.Convert(result_ty, inst);
        }
        if (from->Result()->Type()->DeepestElement()->Is<type::Bool>()) {
            auto* new_ty = ty.MatchWidth(ty.u32(), from->Result()->Type());
            inst = b.Convert(new_ty, from);
        }
        return b.InsertBitcastIfNeeded(result_ty, inst);
    }

    core::ir::Instruction* MakeScalarLoad(core::ir::Var* var,
                                          const core::type::Type* result_ty,
                                          core::ir::Value* byte_idx) {
        // Number of array elements needed to load the scalar.
        auto num_array_eles = NumBaseElements(result_ty);
        auto* array_idx = OffsetValueToArrayIndex(byte_idx);
        if (num_array_eles > 1) {
            // This should only happen when the base type is u16 and a 4-byte scalar is being
            // loaded.
            TINT_IR_ASSERT(ir, num_array_eles == 2);
            auto* vec_ty = ty.vec(BaseEleType(), num_array_eles);
            auto loads = MakeNLoads(var, array_idx, num_array_eles);
            auto* construct = b.Construct(vec_ty, loads);
            return BitcastOrConvertIfNeeded(result_ty, construct);
        }

        auto* access = b.Access(BaseEleTypePtr(), var, array_idx);

        ir::Instruction* load = nullptr;
        if (auto* vec_ty = BaseEleType()->As<type::Vector>()) {
            auto* vec_idx = CalculateVectorOffset(byte_idx, vec_ty);
            load = b.LoadVectorElement(access, vec_idx);
        } else {
            load = b.Load(access);
        }

        if (result_ty->Size() < load->Result()->Type()->Size()) {
            return ExtractScalar2Bytes(load, result_ty, byte_idx);
        }
        return BitcastOrConvertIfNeeded(result_ty, load);
    }

    // Currently this could only be f16, but in the future that will not be true.
    core::ir::Access* ExtractScalar2Bytes(core::ir::Instruction* load,
                                          const type::Type* result_ty,
                                          core::ir::Value* byte_idx) {
        // We will bitcast the load to a vector of result_ty and then extract the element that we
        // want.
        const uint32_t load_size = load->Result()->Type()->Size();
        uint32_t num_eles = load_size / result_ty->Size();
        const type::Type* vec_ty = ty.vec(result_ty, num_eles);
        core::ir::Value* element_index = nullptr;
        if (auto* cnst = byte_idx->As<core::ir::Constant>()) {
            if (cnst->Value()->ValueAs<uint32_t>() % 4 == 0) {
                element_index = b.Constant(0_u);
            } else {
                element_index = b.Constant(1_u);
            }
        } else {
            auto* false_ = b.Value(1_u);
            auto* true_ = b.Value(0_u);
            auto* cond = b.Equal(b.Modulo(byte_idx, 4_u), 0_u);

            Vector<core::ir::Value*, 3> args{false_, true_, cond->Result()};
            element_index = b.Call(ty.u32(), core::BuiltinFn::kSelect, args)->Result();
        }

        auto* bitcast = b.Bitcast(vec_ty, load);
        return b.Access(result_ty, bitcast, element_index);
    }

    // When loading a vector we have to take the alignment into account to determine which part of
    // the `uint4` to load. A `vec`  of `u32`, `f32` or `i32` has an alignment requirement of
    // a multiple of 8-bytes (`f16` is 4-bytes). So, this means we'll have memory like:
    //
    // Byte: | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
    // Value:|      v1       |        v2     |        v3       |          v4       |
    // Scalar Index:   0                 1               2                   3
    //
    // Start with a byte address which is `offset + (column * columnStride)`, the array index is
    // `byte_address / 16`. This gives us the `uint4` which contains our values. We can then
    // calculate the vector offset as `(byte_address % 16) / 4`.
    //
    // * A 4-element row we access all 4-elements at the `array_idx`
    // * A 3-element row we access `array_idx` at `.xyz` as it must be padded to a vec4.
    // * A 2-element row, we have to decide if we want the `xy` or `zw` element. We have a minimum
    //   alignment of 8-bytes as per the WGSL spec. So if the `vector_idx != 2` is `0` then we
    //   access the `.xy` component, otherwise it is in the `.zw` component.
    core::ir::Instruction* MakeVectorLoad(core::ir::Var* var,
                                          const core::type::Vector* result_ty,
                                          core::ir::Value* byte_idx) {
        if (result_ty->DeepestElement()->Size() == 2) {
            return MakeVectorLoadF16(var, result_ty, byte_idx);
        }

        auto* array_idx = OffsetValueToArrayIndex(byte_idx);
        uint32_t num_loads = NumBaseElements(result_ty);
        auto loads = MakeNLoadInsts(var, array_idx, num_loads);

        if (BaseEleType()->Size() < result_ty->DeepestElement()->Size()) {
            TINT_IR_ASSERT(ir, BaseEleType() == ty.u16());
            // Need to step up the size first.
            Vector<Instruction*, 4> new_loads;
            for (uint32_t i = 0; i < num_loads; i += 2) {
                Vector<Value*, 2> args;
                args.Push(loads[i]->Result());
                args.Push(loads[i + 1]->Result());
                new_loads.Push(
                    b.Bitcast(ty.u32(), b.Construct(ty.vec2(BaseEleType()), args)->Result()));
            }
            std::swap(loads, new_loads);
            num_loads /= 2;
        }

        Instruction* value = nullptr;
        Vector<Value*, 4> construct_args;
        for (auto* l : loads) {
            construct_args.Push(l->Result());
        }
        if (loads[0]->Result()->Type() == ty.u32()) {
            value = b.Construct(ty.vec(ty.u32(), static_cast<uint32_t>(loads.Length())),
                                construct_args);
        } else if (loads[0]->Result()->Type() == ty.vec2u()) {
            if (loads.Length() > 1) {
                value = b.Construct(ty.vec4u(), construct_args);
            } else {
                value = loads[0];
            }
        } else {
            TINT_IR_ASSERT(ir, loads[0]->Result()->Type() == ty.vec4u());
            value = loads[0];
        }

        TINT_IR_ASSERT(ir, result_ty->DeepestElement()->Size() == 4);
        core::ir::Instruction* load = nullptr;
        if (result_ty->Width() == 4) {
            load = value;
        } else if (result_ty->Width() == 3) {
            load = b.Swizzle(ty.vec3u(), value, {0, 1, 2});
        } else if (result_ty->Width() == 2) {
            if (value->Result()->Type()->Size() == result_ty->Size()) {
                load = value;
            } else {
                auto* vec_idx = CalculateVectorOffset(byte_idx, ty.vec4u());
                if (auto* cnst = vec_idx->As<core::ir::Constant>()) {
                    if (cnst->Value()->ValueAs<uint32_t>() == 2u) {
                        load = b.Swizzle(ty.vec2u(), value, {2, 3});
                    } else {
                        load = b.Swizzle(ty.vec2u(), value, {0, 1});
                    }
                } else {
                    auto* ubo = value;
                    // if vec_idx == 2 -> zw
                    auto* sw_lhs = b.Swizzle(ty.vec2u(), ubo, {2, 3});
                    // else -> xy
                    auto* sw_rhs = b.Swizzle(ty.vec2u(), ubo, {0, 1});
                    auto* cond = b.Equal(vec_idx, 2_u);

                    Vector<core::ir::Value*, 3> args{sw_rhs->Result(), sw_lhs->Result(),
                                                     cond->Result()};

                    load = b.Call(ty.vec2u(), core::BuiltinFn::kSelect, args);
                }
            }
        } else {
            TINT_IR_UNREACHABLE(ir);
        }
        return BitcastOrConvertIfNeeded(result_ty, load);
    }

    // Returns the instruction for getting a vector-of-f16 value of type `result_ty`
    // out of the pointer-to-vec4u value `access`. Get the components starting
    // at byte offset (byte_idx % 16).
    // A `vec4` or `vec3` of `f16` has 8-byte alignment, while a `vec2` of `f16` has 4-byte
    // alignment. So, this means we'll have memory like:
    //
    // Byte:     |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 |
    // Scalar Index:       0         |         1         |         2         |         3         |
    // vec4<u32>:|         x         |         y         |         z         |         w         |
    // vec4<f16>:|   x     |    y    |    z    |    w    |   x     |    y    |    z    |    w    |
    // vec3<f16>:|   x     |    y    |    z    |         |   x     |    y    |    z    |         |
    // vec2<f16>:|   x     |    y    |    x    |    y    |   x     |    y    |    x    |    y    |
    core::ir::Instruction* MakeVectorLoadF16(core::ir::Var* var,
                                             const core::type::Vector* result_ty,
                                             core::ir::Value* byte_idx) {
        auto* array_idx = OffsetValueToArrayIndex(byte_idx);
        uint32_t num_loads = NumBaseElements(result_ty);
        auto loads = MakeNLoads(var, array_idx, num_loads);

        // Since this vector has 2-byte elements, there are only a few possibilities:
        // 1. Base type is u16
        //    - scalar loads equal to number of vector elements
        // 2. Base type is u32
        //    - 1 or 2 u32 loads
        //    - Note: vec3h is not possible here
        // 3. Base type is vec2u
        //    - 1 load of vec2u
        //    - Note: only vec4h possible here
        // 4. Base type is vec4u
        //    - 1 load of vec4u
        //    - Note: vec3h is not possible here, but vec2h is
        if (BaseEleType() == ty.u16()) {
            auto* vec_ty = ty.vec(ty.u16(), num_loads);
            auto* construct = b.Construct(vec_ty, loads);
            if (vec_ty != result_ty) {
                return b.Bitcast(result_ty, construct->Result());
            }
            return construct;
        } else if (BaseEleType() == ty.u32()) {
            if (result_ty->Width() == 2) {
                return b.Bitcast(result_ty, loads[0]);
            }
            TINT_IR_ASSERT(ir, result_ty->Width() == 4);
            auto* construct = b.Construct(ty.vec2u(), loads);
            return b.Bitcast(result_ty, construct->Result());
        } else if (BaseEleType() == ty.vec2u()) {
            TINT_IR_ASSERT(ir, result_ty->Width() == 4);
            TINT_IR_ASSERT(ir, (result_ty->DeepestElement()->IsAnyOf<type::F16, type::U16>()));
            return b.Bitcast(result_ty, loads[0]);
        }
        TINT_IR_ASSERT(ir, BaseEleType() == ty.vec4u());
        TINT_IR_ASSERT(ir, loads.Length() == 1);

        // Vec4 ends up being the same as a bitcast of vec2<u32> to a vec4<f16>.
        // A vec3 will be stored as a vec4, so we can bitcast as if we're a vec4
        // and swizzle out the last element.
        if (result_ty->Width() == 3 || result_ty->Width() == 4) {
            core::ir::Instruction* load = nullptr;
            auto* vec_idx = CalculateVectorOffset(byte_idx, ty.vec4u());  // 0 or 2
            if (auto* cnst = vec_idx->As<core::ir::Constant>()) {
                if (cnst->Value()->ValueAs<uint32_t>() == 2u) {
                    load = b.Swizzle(ty.vec2u(), loads[0], {2, 3});
                } else {
                    load = b.Swizzle(ty.vec2u(), loads[0], {0, 1});
                }
            } else {
                auto* ubo = loads[0];
                // if vec_idx == 2 -> zw
                auto* sw_lhs = b.Swizzle(ty.vec2u(), ubo, {2, 3});
                // else -> xy
                auto* sw_rhs = b.Swizzle(ty.vec2u(), ubo, {0, 1});
                auto* cond = b.Equal(vec_idx, 2_u);
                auto args = Vector{sw_rhs->Result(), sw_lhs->Result(), cond->Result()};
                load = b.Call(ty.vec2u(), core::BuiltinFn::kSelect, std::move(args));
            }
            if (result_ty->Width() == 3) {
                auto* bc = b.Bitcast(ty.vec4(result_ty->Type()), load);
                return b.Swizzle(result_ty, bc, {0, 1, 2});
            }
            return b.Bitcast(result_ty, load);
        }

        // Vec2 ends up being the same as a bitcast u32 to vec2<f16>
        if (result_ty->Width() == 2) {
            core::ir::Instruction* load = nullptr;
            auto* vec_idx = CalculateVectorOffset(byte_idx, ty.vec4u());  // 0, 1, 2, or 3
            if (auto* cnst = vec_idx->As<core::ir::Constant>()) {
                const auto vec_idx_val = cnst->Value()->ValueAs<uint32_t>();
                load = b.Swizzle(ty.u32(), loads[0], {vec_idx_val});
            } else {
                load = b.Access(ty.u32(), loads[0], vec_idx);
            }
            return b.Bitcast(result_ty, load);
        }

        TINT_IR_UNREACHABLE(ir);
    }

    // Creates a load function for the given `var` and `matrix` combination. Essentially creates
    // a function similar to:
    //
    // fn custom_load_M(offset: u32) {
    //   const uint scalar_offset = ((offset + 0u)) / 4;
    //   const uint scalar_offset_1 = ((offset + (1 * ColumnStride))) / 4;
    //   const uint scalar_offset_2 = ((offset + (2 * ColumnStride))) / 4;
    //   const uint scalar_offset_3 = ((offset + (3 * ColumnStride)))) / 4;
    //   return float4x4(
    //       asfloat(v[scalar_offset / 4]),
    //       asfloat(v[scalar_offset_1 / 4]),
    //       asfloat(v[scalar_offset_2 / 4]),
    //      asfloat(v[scalar_offset_3 / 4])
    //   );
    // }
    core::ir::Function* GetLoadFunctionFor(core::ir::Instruction* inst,
                                           core::ir::Var* var,
                                           const core::type::Matrix* mat) {
        return var_and_type_to_load_fn_.GetOrAdd(VarTypePair{var, mat}, [&] {
            auto* start_byte_offset = b.FunctionParam("start_byte_offset", ty.u32());
            auto* fn = b.Function(mat);
            fn->SetParams({start_byte_offset});

            b.Append(fn->Block(), [&] {
                Vector<core::ir::Value*, 4> values;
                for (size_t i = 0; i < mat->Columns(); ++i) {
                    uint32_t stride = static_cast<uint32_t>(i * mat->ColumnStride());

                    OffsetData od{stride, {start_byte_offset}};
                    auto* byte_idx = OffsetToValue(od);
                    values.Push(MakeLoad(inst, var, mat->ColumnType(), byte_idx)->Result());
                }
                b.Return(fn, b.Construct(mat, values));
            });

            return fn;
        });
    }

    // Creates a load function for the given `var` and `array` combination. Essentially creates
    // a function similar to:
    //
    // fn custom_load_A(offset: u32) {
    //   A a = A();
    //   u32 i = 0;
    //   loop {
    //     if (i >= A length) {
    //       break;
    //     }
    //     offset = (offset + (i * A->ImplicitStride())) / 16
    //     a[i] = cast(v[offset].xyz)
    //     i = i + 1;
    //   }
    //   return a;
    // }
    core::ir::Function* GetLoadFunctionFor(core::ir::Instruction* inst,
                                           core::ir::Var* var,
                                           const core::type::Array* arr) {
        return var_and_type_to_load_fn_.GetOrAdd(VarTypePair{var, arr}, [&] {
            auto* start_byte_offset = b.FunctionParam("start_byte_offset", ty.u32());
            auto* fn = b.Function(arr);
            fn->SetParams({start_byte_offset});

            b.Append(fn->Block(), [&] {
                auto* result_arr = b.Var<function>("a", b.Zero(arr));

                auto* count = arr->Count()->As<core::type::ConstantArrayCount>();
                TINT_IR_ASSERT(ir, count);

                b.LoopRange(0_u, u32(count->value), 1_u, [&](core::ir::Value* idx) {
                    auto* stride = b.Multiply(idx, u32(arr->ImplicitStride()))->Result();
                    OffsetData od{0, {start_byte_offset, stride}};
                    auto* byte_idx = OffsetToValue(od);
                    auto* access = b.Access(ty.ptr<function>(arr->ElemType()), result_arr, idx);
                    b.Store(access, MakeLoad(inst, var, arr->ElemType(), byte_idx));
                });

                b.Return(fn, b.Load(result_arr));
            });

            return fn;
        });
    }

    // Creates a load function for the given `var` and `struct` combination. Essentially creates
    // a function similar to:
    //
    // fn custom_load_S(start_offset: u32) {
    //   let a = load object at (start_offset + member_offset)
    //   let b = load object at (start_offset + member 1 offset);
    //   ...
    //   return S(a, b, ..., z);
    // }
    core::ir::Function* GetLoadFunctionFor(core::ir::Instruction* inst,
                                           core::ir::Var* var,
                                           const core::type::Struct* s) {
        return var_and_type_to_load_fn_.GetOrAdd(VarTypePair{var, s}, [&] {
            auto* start_byte_offset = b.FunctionParam("start_byte_offset", ty.u32());
            auto* fn = b.Function(s);
            fn->SetParams({start_byte_offset});

            b.Append(fn->Block(), [&] {
                Vector<core::ir::Value*, 4> values;
                for (const auto* mem : s->Members()) {
                    uint32_t stride = static_cast<uint32_t>(mem->Offset());

                    OffsetData od{stride, {start_byte_offset}};
                    auto* byte_idx = OffsetToValue(od);
                    values.Push(MakeLoad(inst, var, mem->Type(), byte_idx)->Result());
                }

                b.Return(fn, b.Construct(s, values));
            });

            return fn;
        });
    }

    // Updates an arrayLength call to account for `offset`.
    // The builtin call returns the length of the whole variable. The array type may have changed
    // too, so we must account for that difference. Any offset must be subtracted from that value to
    // provide the same result as before the transform. Offsets can come from two sources:
    //
    // 1. An offset from an access into a structure.
    // 2. An offset (possibly runtime value) from a bufferView call.
    void ArrayLength(core::ir::CoreBuiltinCall* call,
                     core::ir::Var* var,
                     const type::Type* type,
                     OffsetData offset) {
        auto* array_ty = type->As<type::Array>();
        TINT_IR_ASSERT(ir, array_ty && array_ty->Count()->Is<core::type::RuntimeArrayCount>());

        // The arrayLength of the transformed variable will always be a multiple of the base type.
        TINT_IR_ASSERT(ir, array_ty->ElemType()->Size() / BaseEleType()->Size() >= 1);
        const uint32_t ratio = array_ty->ImplicitStride() / BaseEleType()->Size();

        // Given:
        // b = bufferView var, offset1
        // a = access b, offset2
        // l = arrayLength a
        //
        // Transformed:
        // l = arrayLength var
        // o = add offset1, bytes(offset2)
        // s = sub l, o
        // d = div s, ratio
        b.InsertBefore(call, [&] {
            // Re-create the arrayLength call to simplify RAUW below.
            auto* len = b.Call(ty.u32(), BuiltinFn::kArrayLength, var);

            Value* value = nullptr;
            ir::Instruction* inst = len;
            // bufferView calls may have introduced a single runtime offset value.
            TINT_IR_ASSERT(ir, offset.byte_offset_expr.Length() <= 1);
            if (offset.byte_offset > 0 && offset.byte_offset_expr.Length() > 0) {
                value = b.Add(offset.byte_offset_expr[0], u32(offset.byte_offset))->Result();
            } else if (offset.byte_offset_expr.Length() > 0) {
                value = offset.byte_offset_expr[0];
            } else if (offset.byte_offset > 0) {
                value = b.Constant(u32(offset.byte_offset / BaseEleType()->Size()));
            }
            if (value && offset.byte_offset_expr.Length() > 0) {
                value = b.Divide(value, u32(BaseEleType()->Size()))->Result();
            }
            if (value) {
                inst = b.Subtract(inst, value);
            }
            if (ratio != 1u) {
                inst = b.Divide(inst, u32(ratio));
            }
            call->Result()->ReplaceAllUsesWith(inst->Result());
        });
        call->Destroy();
    }

    // bufferLength calls are replaced with arrayLength calls for the entire var size with two
    // special cases:
    //
    // 1. The bufferLength has a third operand added by DirectVariableAccess that represents the
    //    lowest limit encountered.
    // 2. The buffer is sized and we can use that directly from the type.
    void BufferLength(core::ir::CoreBuiltinCall* call, core::ir::Var* var, const type::Type* type) {
        auto* buffer_ty = type->As<core::type::Buffer>();
        TINT_IR_ASSERT(ir, buffer_ty && (buffer_ty->Count()->Is<type::RuntimeArrayCount>() ||
                                         buffer_ty->Count()->Is<type::ConstantArrayCount>()));

        if (call->Args().size() > 1) {
            // Direct variable access encoded a lower limit.
            call->Result()->ReplaceAllUsesWith(call->Args()[1]);
        } else if (auto* cnst = buffer_ty->Count()->As<type::ConstantArrayCount>()) {
            call->Result()->ReplaceAllUsesWith(b.Constant(u32(cnst->value)));
        } else {
            TINT_IR_ASSERT(ir, buffer_ty->Count()->Is<type::RuntimeArrayCount>());
            b.InsertBefore(call, [&] {
                // arrayLength(var) * BaseEleType()->Size()
                ir::Instruction* inst = b.Call(ty.u32(), core::BuiltinFn::kArrayLength, var);
                inst = b.Multiply(inst, u32(BaseEleType()->Size()));
                call->Result()->ReplaceAllUsesWith(inst->Result());
            });
        }
        call->Destroy();
    }

    // Store the scalar value `from` at `byte_idx` offset.
    //
    // The transform should never select a base type that is not an even multiple of the smallest
    // storage size, so there are two cases here:
    //
    // 1. Multiple stores are needed to store `from` (e.g. base type u16 storing a u32).
    // 2. The base type size matches `from`'s size.
    void MakeScalarStore(core::ir::Var* var, core::ir::Value* from, core::ir::Value* byte_idx) {
        auto* st_ty = from->Type();
        // Number of array elements need to store the scalar.
        auto num_array_eles = NumBaseElements(st_ty);
        auto* array_idx = OffsetValueToArrayIndex(byte_idx);
        if (num_array_eles > 1) {
            // This should only happen if the base type is u16 and a 4-byte scalar is being stored.
            TINT_IR_ASSERT(ir, num_array_eles == 2);
            TINT_IR_ASSERT(ir, BaseEleType()->Is<type::U16>());
            auto* vec_ty = ty.vec(BaseEleType(), num_array_eles);
            auto* cast = BitcastOrConvertIfNeeded(vec_ty, from);
            for (uint32_t i = 0; i < num_array_eles; i++) {
                auto* access = b.Access(BaseEleTypePtr(), var, array_idx);
                b.Store(access, b.Access(BaseEleType(), cast, u32(i)));
                if (i < num_array_eles - 1) {
                    if (auto* cnst = array_idx->As<Constant>()) {
                        array_idx = b.Constant(u32(cnst->Value()->ValueAs<uint32_t>() + 1));
                    } else {
                        array_idx = b.Add(array_idx, 1_u)->Result();
                    }
                }
            }
            return;
        }

        // For stores, we know that the base type size must be less than or equal to the store size
        // (or we'd have picked a smaller base type). Above we eliminated the smaller size case.
        TINT_IR_ASSERT(ir, st_ty->Size() == BaseEleType()->Size());
        Value* value = BitcastOrConvertIfNeeded(BaseEleType(), from);
        auto* access = b.Access(BaseEleTypePtr(), var, array_idx);
        b.Store(access, value);
    }

    // Store the vector `from` at `byte_idx` offset.
    //
    // The two main cases to consider are:
    //
    // 1. The base type is a vector.
    //    i.  If a single store is needed, we simply cast the result if necessary
    //    ii. Multiple stores are needed, break the vector in sub-vectors and store those.
    // 2. The base type is a scalar.
    //    i.  If the vector element is smaller than base type (e.g., vec4h with u32 base),
    //        bitcast the entire vector to u32(s) and store.
    //    ii. Otherwise, store each element at successive indices.
    void MakeVectorStore(core::ir::Var* var, core::ir::Value* from, core::ir::Value* byte_idx) {
        auto* st_ty = from->Type()->As<type::Vector>();
        // Number of array elements need to store the scalar.
        auto num_array_eles = NumBaseElements(st_ty);
        auto* array_idx = OffsetValueToArrayIndex(byte_idx);

        // We're storing a vector so we need break down `from` into appropriate `BaseEleType()`
        // bits. The base type size will be less than or equal to the store size, but may be a
        // scalar or a vector.
        if (BaseEleType()->Is<type::Vector>()) {
            // | Base type | Possible store sizes            | # Array Ele |
            // | vec2u     | vec2u, vec4u, vec4h (NOT vec3u) | 1 or 2      |
            // | vec4u     | vec4u                           | 1           |
            if (num_array_eles == 1) {
                Value* value = BitcastOrConvertIfNeeded(BaseEleType(), from);
                auto* access = b.Access(BaseEleTypePtr(), var, array_idx);
                b.Store(access, value);
            } else {
                auto* sub_vec_ty = ty.vec2(st_ty->DeepestElement());
                Instruction* first = b.Swizzle(sub_vec_ty, from, {0, 1});
                first = BitcastOrConvertIfNeeded(BaseEleType(), first);
                auto* access = b.Access(BaseEleTypePtr(), var, array_idx);
                b.Store(access, first);

                if (auto* cnst = array_idx->As<Constant>()) {
                    array_idx = b.Constant(u32(cnst->Value()->ValueAs<uint32_t>() + 1));
                } else {
                    array_idx = b.Add(array_idx, 1_u)->Result();
                }
                Instruction* second = b.Swizzle(sub_vec_ty, from, {2, 3});
                second = BitcastOrConvertIfNeeded(BaseEleType(), second);
                access = b.Access(BaseEleTypePtr(), var, array_idx);
                b.Store(access, second);
            }
        } else {
            auto* st_ele_ty = st_ty->DeepestElement();

            // Case A: Vector element is smaller than base type.
            // This occurs when storing vec<N, f16> but the base type is u32.
            if (st_ele_ty->Size() < BaseEleType()->Size()) {
                TINT_IR_ASSERT(ir, BaseEleType()->Is<type::U32>());
                TINT_IR_ASSERT(ir, st_ele_ty->Size() == 2);  // f16 or similar 2-byte type

                // vec3<f16> forces a u16 base type, so width must be even here.
                TINT_IR_ASSERT(ir, st_ty->Width() % 2 == 0);

                // vec2h -> bitcast to u32, vec4h -> bitcast to vec2u
                uint32_t num_u32s = (st_ty->Width() == 2) ? 1 : 2;
                Value* cast_val = (num_u32s == 1) ? b.Bitcast(ty.u32(), from)->Result()
                                                  : b.Bitcast(ty.vec2u(), from)->Result();

                for (uint32_t i = 0; i < num_u32s; i++) {
                    Value* elem =
                        (num_u32s == 1) ? cast_val : b.Access(ty.u32(), cast_val, u32(i))->Result();
                    auto* access = b.Access(BaseEleTypePtr(), var, array_idx);
                    b.Store(access, elem);
                    if (i < num_u32s - 1) {
                        if (auto* cnst = array_idx->As<Constant>()) {
                            array_idx = b.Constant(u32(cnst->Value()->ValueAs<uint32_t>() + 1));
                        } else {
                            array_idx = b.Add(array_idx, 1_u)->Result();
                        }
                    }
                }
                return;
            }

            // Case B: Vector element is equal to or larger than base type.
            // ratio == 1: element size == base type size.
            //   e.g. vec4<f32> with u32 base  → 4× u32  (bitcast each f32)
            // ratio == 2: element size is 2× base type size.
            //   e.g. vec4<f32> with u16 base  → 8× u16 (bitcast vec4<f32> to vec2<u32>, then
            //   bitcast each u32 to 2× u16)
            uint32_t ratio = st_ele_ty->Size() / BaseEleType()->Size();
            TINT_IR_ASSERT(ir, ratio == 1 || ratio == 2);
            for (uint32_t i = 0; i < num_array_eles; i++) {
                Instruction* value = b.Access(st_ele_ty, from, u32(i / ratio));
                if (ratio == 2) {
                    value = BitcastOrConvertIfNeeded(ty.vec2(BaseEleType()), value);
                    uint32_t sub_idx = i % 2;
                    value = b.Access(BaseEleType(), value, u32(sub_idx));
                } else if (st_ele_ty != BaseEleType()) {
                    value = BitcastOrConvertIfNeeded(BaseEleType(), value);
                }
                auto* access = b.Access(BaseEleTypePtr(), var, array_idx);
                b.Store(access, value);
                if (i < num_array_eles - 1) {
                    if (auto* cnst = array_idx->As<Constant>()) {
                        array_idx = b.Constant(u32(cnst->Value()->ValueAs<uint32_t>() + 1));
                    } else {
                        array_idx = b.Add(array_idx, 1_u)->Result();
                    }
                }
            }
        }
    }

    // Generate a store of `from` at `byte_idx` for the appropriate type.
    void MakeStore(core::ir::Instruction* inst,
                   core::ir::Var* var,
                   core::ir::Value* from,
                   core::ir::Value* byte_idx) {
        tint::Switch(
            from->Type(),  //
            [&](const type::Struct* s) {
                auto* fn = GetStoreFunctionFor(inst, var, s);
                b.Call(fn, byte_idx, from);
            },
            [&](const type::Matrix* m) {
                auto* fn = GetStoreFunctionFor(inst, var, m);
                b.Call(fn, byte_idx, from);
            },
            [&](const type::Array* a) {
                auto* fn = GetStoreFunctionFor(inst, var, a);
                b.Call(fn, byte_idx, from);
            },
            [&](const type::Vector*) { MakeVectorStore(var, from, byte_idx); },
            [&](const type::Scalar*) { MakeScalarStore(var, from, byte_idx); },
            TINT_ICE_ON_NO_MATCH);
    }

    void Store(core::ir::Store* s, core::ir::Var* var, OffsetData offset) {
        b.InsertBefore(s, [&] {
            auto* byte_idx = OffsetToValue(offset);
            MakeStore(s, var, s->From(), byte_idx);
        });
        s->Destroy();
    }

    void StoreVectorElement(core::ir::StoreVectorElement* s,
                            core::ir::Var* var,
                            OffsetData offset) {
        b.InsertBefore(s, [&] {
            UpdateOffsetData(s->Index(), s->Value()->Type()->Size(), &offset);
            auto* byte_idx = OffsetToValue(offset);
            MakeStore(s, var, s->Value(), byte_idx);
        });
        s->Destroy();
    }

    // Create a store function for the given `var` and `struct` combination. Essentially creates a
    // function similar to:
    //
    // fn custom_store_S(start_offset:u32, object:S) {
    //   store object at (start_offset + member 0 offset)
    //   store object at (start_offset + member 1 offset)
    //   ...
    //   store object at (start_offset + member n-1 offset)
    // }
    core::ir::Function* GetStoreFunctionFor(core::ir::Instruction* inst,
                                            core::ir::Var* var,
                                            const core::type::Struct* s) {
        return var_and_type_to_store_fn_.GetOrAdd(VarTypePair{var, s}, [&] {
            auto* start_byte_offset = b.FunctionParam("start_byte_offset", ty.u32());
            auto* object = b.FunctionParam("object", s);
            auto* fn = b.Function(ty.void_());
            fn->SetParams({start_byte_offset, object});

            b.Append(fn->Block(), [&] {
                for (auto* member : s->Members()) {
                    uint32_t mem_offset = static_cast<uint32_t>(member->Offset());
                    OffsetData offset{mem_offset, {start_byte_offset}};
                    auto* byte_idx = OffsetToValue(offset);
                    auto* from = b.Access(member->Type(), object, u32(member->Index()))->Result();
                    MakeStore(inst, var, from, byte_idx);
                }
                b.Return(fn);
            });

            return fn;
        });
    }

    // Create a store function for the given `var` and `matrix` combination. Essentially creates a
    // function similar to:
    //
    // fn custom_store_M(start_offset:u32, object:M) {
    //   store object[0] at offset (start_offset + 0)
    //   store object[1] at offset (start_offset + (1 * ColumnStride))
    //   store object[2] at offset (start_offset + (2 * ColumnStride))
    //   store object[3] at offset (start_offset + (3 * ColumnStride))
    // }
    core::ir::Function* GetStoreFunctionFor(core::ir::Instruction* inst,
                                            core::ir::Var* var,
                                            const core::type::Matrix* m) {
        return var_and_type_to_store_fn_.GetOrAdd(VarTypePair{var, m}, [&] {
            auto* start_byte_offset = b.FunctionParam("start_byte_offset", ty.u32());
            auto* object = b.FunctionParam("object", m);
            auto* fn = b.Function(ty.void_());
            fn->SetParams({start_byte_offset, object});

            b.Append(fn->Block(), [&] {
                for (uint32_t c = 0; c < m->Columns(); c++) {
                    uint32_t vec_offset = static_cast<uint32_t>(c * m->ColumnStride());
                    OffsetData offset{vec_offset, {start_byte_offset}};
                    auto* byte_idx = OffsetToValue(offset);
                    auto* from = b.Access(m->ColumnType(), object, u32(c))->Result();
                    MakeStore(inst, var, from, byte_idx);
                }
                b.Return(fn);
            });

            return fn;
        });
    }

    // Create a store function for the given `var` and `array` combination. Essentially creates a
    // function similar to:
    //
    // fn custom_store_A(start_offset:u32, object:A) {
    // }
    core::ir::Function* GetStoreFunctionFor(core::ir::Instruction* inst,
                                            core::ir::Var* var,
                                            const core::type::Array* a) {
        return var_and_type_to_store_fn_.GetOrAdd(VarTypePair{var, a}, [&] {
            auto* start_byte_offset = b.FunctionParam("start_byte_offset", ty.u32());
            auto* object = b.FunctionParam("object", a);
            auto* fn = b.Function(ty.void_());
            fn->SetParams({start_byte_offset, object});

            b.Append(fn->Block(), [&] {
                auto* count = a->Count()->As<core::type::ConstantArrayCount>();
                TINT_IR_ASSERT(ir, count);

                b.LoopRange(0_u, u32(count->value), 1_u, [&](core::ir::Value* idx) {
                    auto* stride = b.Multiply(idx, u32(a->ImplicitStride()))->Result();
                    OffsetData od{0, {start_byte_offset, stride}};
                    auto* byte_idx = OffsetToValue(od);
                    auto* from = b.Access(a->ElemType(), object, idx)->Result();
                    MakeStore(inst, var, from, byte_idx);
                });

                b.Return(fn);
            });

            return fn;
        });
    }
};

}  // namespace

Result<SuccessType> DecomposeAccess(core::ir::Module& ir, const DecomposeAccessOptions& options) {
    core::ir::AssertValid(ir,
                          core::ir::Capabilities{
                              core::ir::Capability::kAllow8BitIntegers,
                              core::ir::Capability::kAllow16BitIntegers,
                              core::ir::Capability::kAllowHandleVarsWithoutBindings,
                              core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector,
                              core::ir::Capability::kAllowDuplicateBindings,
                              core::ir::Capability::kAllowNonCoreTypes,
                              core::ir::Capability::kLoosenValidationForShaderIO,
                          },
                          "before core.DecomposeUniformAccess");

    State{ir, options}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
