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

#include "src/tint/lang/hlsl/writer/raise/decompose_uniform_access.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/builtin_call.h"
#include "src/tint/lang/hlsl/ir/ternary.h"
#include "src/tint/utils/result/result.h"

namespace tint::hlsl::writer::raise {
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

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    using VarTypePair = std::pair<core::ir::Var*, const core::type::Type*>;
    /// Maps a struct to the load function
    Hashmap<VarTypePair, core::ir::Function*, 2> var_and_type_to_load_fn_{};

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

            // DecomposeStorageAccess maybe have converte the var pointers into ByteAddressBuffer
            // objects. Since they've been changed, then they're Storage buffers and we don't care
            // about them here.
            auto* var_ty = var->Result(0)->Type()->As<core::type::Pointer>();
            if (!var_ty) {
                continue;
            }

            // Only care about uniform address space variables.
            if (var_ty->AddressSpace() != core::AddressSpace::kUniform) {
                continue;
            }

            var_worklist.Push(var);
        }

        for (auto* var : var_worklist) {
            auto* result = var->Result(0);

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
                Switch(
                    inst,  //
                    [&](core::ir::LoadVectorElement* l) { LoadVectorElement(l, var, od); },
                    [&](core::ir::Load* l) { Load(l, var, od); },
                    [&](core::ir::Access* a) { Access(a, var, a->Object()->Type(), od); },
                    [&](core::ir::Let* let) {
                        // The `let` is, essentially, an alias for the `var` as it's assigned
                        // directly. Gather all the `let` usages into our worklist, and then replace
                        // the `let` with the `var` itself.
                        for (auto& use : let->Result(0)->UsagesSorted()) {
                            usage_worklist.Push(use);
                        }
                        let->Result(0)->ReplaceAllUsesWith(result);
                        let->Destroy();
                    },
                    TINT_ICE_ON_NO_MATCH);
            }

            // Swap the result type of the `var` to the new HLSL result type
            auto array_length = (var_ty->StoreType()->Size() + 15) / 16;
            result->SetType(ty.ptr(var_ty->AddressSpace(), ty.array(ty.vec4<u32>(), array_length),
                                   var_ty->Access()));
        }
    }

    // OffsetData represents an unsigned integer expression.
    // The value is the sum of a const part, held in `byte_offset`, and
    // non-const parts in `byte_offset_expr`.
    struct OffsetData {
        uint32_t byte_offset = 0;
        Vector<core::ir::Value*, 4> byte_offset_expr{};
    };

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
                val = b.Add(ty.u32(), val, expr)->Result(0);
            }
        }

        return val;
    }

    // Converts a byte offset to an index into a vec4u array.
    // After the transform runs the store type of the uniform buffer variable is an
    // array of vec4u.
    // Note, this must be called inside a builder insert block (Append, InsertBefore, etc)
    core::ir::Value* OffsetValueToArrayIndex(core::ir::Value* val) {
        if (auto* cnst = val->As<core::ir::Constant>()) {
            auto v = cnst->Value()->ValueAs<uint32_t>();
            return b.Value(u32(v / 16u));
        }
        return b.Divide(ty.u32(), val, 16_u)->Result(0);
    }

    // Calculates the index of the vec4u element containing the byte at (byte_idx % 16).
    // Assumes the upper bits of byte_idx have already been used to access the correct
    // vec4u array element in the underlying variable.
    core::ir::Value* CalculateVectorOffset(core::ir::Value* byte_idx) {
        if (auto* byte_cnst = byte_idx->As<core::ir::Constant>()) {
            return b.Value(u32((byte_cnst->Value()->ValueAs<uint32_t>() % 16u) / 4u));
        }
        return b.Divide(ty.u32(), b.Modulo(ty.u32(), byte_idx, 16_u), 4_u)->Result(0);
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

        auto update_offset = [&](core::ir::Value* idx_value, uint32_t size) {
            tint::Switch(
                idx_value,
                [&](core::ir::Constant* cnst) {
                    uint32_t idx = cnst->Value()->ValueAs<uint32_t>();
                    offset.byte_offset += size * idx;
                },
                [&](core::ir::Value* val) {
                    b.InsertBefore(a, [&] {
                        offset.byte_offset_expr.Push(
                            b.Multiply(ty.u32(), u32(size), b.InsertConvertIfNeeded(ty.u32(), val))
                                ->Result(0));
                    });
                });
        };

        for (auto* idx_value : a->Indices()) {
            tint::Switch(
                obj_ty,
                [&](const core::type::Vector* v) {
                    update_offset(idx_value, v->Type()->Size());
                    obj_ty = v->Type();
                },
                [&](const core::type::Matrix* m) {
                    update_offset(idx_value, m->ColumnStride());
                    obj_ty = m->ColumnType();
                },
                [&](const core::type::Array* ary) {
                    update_offset(idx_value, ary->Stride());
                    obj_ty = ary->ElemType();
                },
                [&](const core::type::Struct* s) {
                    auto* cnst = idx_value->As<core::ir::Constant>();

                    // A struct index must be a constant
                    TINT_ASSERT(cnst);

                    uint32_t idx = cnst->Value()->ValueAs<uint32_t>();
                    auto* mem = s->Members()[idx];
                    obj_ty = mem->Type();
                    offset.byte_offset += mem->Offset();
                },
                TINT_ICE_ON_NO_MATCH);
        }

        auto usages = a->Result(0)->UsagesSorted();
        while (!usages.IsEmpty()) {
            auto usage = usages.Pop();
            tint::Switch(
                usage.instruction,
                [&](core::ir::Let* let) {
                    // The `let` is essentially an alias to the `access`. So, add the `let`
                    // usages into the usage worklist, and replace the let with the access chain
                    // directly.
                    for (auto& u : let->Result(0)->UsagesSorted()) {
                        usages.Push(u);
                    }
                    let->Result(0)->ReplaceAllUsesWith(a->Result(0));
                    let->Destroy();
                },
                [&](core::ir::Access* sub_access) {
                    // Treat an access chain of the access chain as a continuation of the outer
                    // chain. Pass through the object we stopped at and the current byte_offset
                    // and then restart the access chain replacement for the new access chain.
                    Access(sub_access, var, obj_ty, offset);
                },
                [&](core::ir::Load* ld) {
                    a->Result(0)->RemoveUsage(usage);
                    Load(ld, var, offset);
                },
                [&](core::ir::LoadVectorElement* lve) {
                    a->Result(0)->RemoveUsage(usage);
                    LoadVectorElement(lve, var, offset);
                },
                TINT_ICE_ON_NO_MATCH);
        }
        a->Destroy();
    }

    void Load(core::ir::Load* ld, core::ir::Var* var, OffsetData offset) {
        b.InsertBefore(ld, [&] {
            auto* byte_idx = OffsetToValue(offset);
            auto* result = MakeLoad(ld, var, ld->Result(0)->Type(), byte_idx);
            ld->Result(0)->ReplaceAllUsesWith(result->Result(0));
        });
        ld->Destroy();
    }

    void LoadVectorElement(core::ir::LoadVectorElement* lve,
                           core::ir::Var* var,
                           OffsetData offset) {
        b.InsertBefore(lve, [&] {
            // Add the byte count from the start of the vector to the requested element to the
            // current offset calculation
            auto elem_byte_size = lve->Result(0)->Type()->DeepestElement()->Size();
            if (auto* cnst = lve->Index()->As<core::ir::Constant>()) {
                offset.byte_offset += (cnst->Value()->ValueAs<uint32_t>() * elem_byte_size);
            } else {
                offset.byte_offset_expr.Push(
                    b.Multiply(ty.u32(), b.InsertConvertIfNeeded(ty.u32(), lve->Index()),
                               u32(elem_byte_size))
                        ->Result(0));
            }

            auto* byte_idx = OffsetToValue(offset);
            auto* result = MakeLoad(lve, var, lve->Result(0)->Type(), byte_idx);
            lve->Result(0)->ReplaceAllUsesWith(result->Result(0));
        });
        lve->Destroy();
    }

    // Creates the appropriate load instructions for the given result type.
    core::ir::Instruction* MakeLoad(core::ir::Instruction* inst,
                                    core::ir::Var* var,
                                    const core::type::Type* result_ty,
                                    core::ir::Value* byte_idx) {
        if (result_ty->IsFloatScalar() || result_ty->IsIntegerScalar()) {
            return MakeScalarLoad(var, result_ty, byte_idx);
        }
        if (result_ty->IsScalarVector()) {
            return MakeVectorLoad(var, result_ty->As<core::type::Vector>(), byte_idx);
        }

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
            TINT_ICE_ON_NO_MATCH);
    }

    core::ir::Call* MakeScalarLoad(core::ir::Var* var,
                                   const core::type::Type* result_ty,
                                   core::ir::Value* byte_idx) {
        auto* array_idx = OffsetValueToArrayIndex(byte_idx);
        auto* access = b.Access(ty.ptr(uniform, ty.vec4<u32>()), var, array_idx);

        auto* vec_idx = CalculateVectorOffset(byte_idx);
        core::ir::Instruction* load = b.LoadVectorElement(access, vec_idx);
        if (result_ty->Is<core::type::F16>()) {
            return MakeScalarLoadF16(load, result_ty, byte_idx);
        }
        return b.Bitcast(result_ty, load);
    }

    core::ir::Call* MakeScalarLoadF16(core::ir::Instruction* load,
                                      const core::type::Type* result_ty,
                                      core::ir::Value* byte_idx) {
        // Handle F16
        if (auto* cnst = byte_idx->As<core::ir::Constant>()) {
            if (cnst->Value()->ValueAs<uint32_t>() % 4 != 0) {
                load = b.ShiftRight(ty.u32(), load, 16_u);
            }
        } else {
            auto* false_ = b.Value(16_u);
            auto* true_ = b.Value(0_u);
            auto* cond = b.Equal(ty.bool_(), b.Modulo(ty.u32(), byte_idx, 4_u), 0_u);

            Vector<core::ir::Value*, 3> args{false_, true_, cond->Result(0)};
            auto* shift_amt =
                b.ir.CreateInstruction<hlsl::ir::Ternary>(b.InstructionResult(ty.u32()), args);
            b.Append(shift_amt);

            load = b.ShiftRight(ty.u32(), load, shift_amt);
        }
        load = b.Call<hlsl::ir::BuiltinCall>(ty.f32(), hlsl::BuiltinFn::kF16Tof32, load);
        return b.Convert(result_ty, load);
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
        auto* array_idx = OffsetValueToArrayIndex(byte_idx);
        auto* access = b.Access(ty.ptr(uniform, ty.vec4<u32>()), var, array_idx);

        if (result_ty->DeepestElement()->Is<core::type::F16>()) {
            return MakeVectorLoadF16(access, result_ty, byte_idx);
        }

        TINT_ASSERT(result_ty->DeepestElement()->Size() == 4);
        core::ir::Instruction* load = nullptr;
        if (result_ty->Width() == 4) {
            load = b.Load(access);
        } else if (result_ty->Width() == 3) {
            load = b.Swizzle(ty.vec3<u32>(), b.Load(access), {0, 1, 2});
        } else if (result_ty->Width() == 2) {
            auto* vec_idx = CalculateVectorOffset(byte_idx);
            if (auto* cnst = vec_idx->As<core::ir::Constant>()) {
                if (cnst->Value()->ValueAs<uint32_t>() == 2u) {
                    load = b.Swizzle(ty.vec2<u32>(), b.Load(access), {2, 3});
                } else {
                    load = b.Swizzle(ty.vec2<u32>(), b.Load(access), {0, 1});
                }
            } else {
                auto* ubo = b.Load(access);
                // if vec_idx == 2 -> zw
                auto* sw_lhs = b.Swizzle(ty.vec2<u32>(), ubo, {2, 3});
                // else -> xy
                auto* sw_rhs = b.Swizzle(ty.vec2<u32>(), ubo, {0, 1});
                auto* cond = b.Equal(ty.bool_(), vec_idx, 2_u);

                Vector<core::ir::Value*, 3> args{sw_rhs->Result(0), sw_lhs->Result(0),
                                                 cond->Result(0)};

                load = b.ir.CreateInstruction<hlsl::ir::Ternary>(
                    b.InstructionResult(ty.vec2<u32>()), args);
                b.Append(load);
            }
        } else {
            TINT_UNREACHABLE();
        }
        return b.Bitcast(result_ty, load);
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
    core::ir::Instruction* MakeVectorLoadF16(core::ir::Access* access,
                                             const core::type::Vector* result_ty,
                                             core::ir::Value* byte_idx) {
        // Vec4 ends up being the same as a bitcast of vec2<u32> to a vec4<f16>.
        // A vec3 will be stored as a vec4, so we can bitcast as if we're a vec4
        // and swizzle out the last element.
        if (result_ty->Width() == 3 || result_ty->Width() == 4) {
            core::ir::Instruction* load = nullptr;
            auto* vec_idx = CalculateVectorOffset(byte_idx);  // 0 or 2
            if (auto* cnst = vec_idx->As<core::ir::Constant>()) {
                if (cnst->Value()->ValueAs<uint32_t>() == 2u) {
                    load = b.Swizzle(ty.vec2<u32>(), b.Load(access), {2, 3});
                } else {
                    load = b.Swizzle(ty.vec2<u32>(), b.Load(access), {0, 1});
                }
            } else {
                auto* ubo = b.Load(access);
                // if vec_idx == 2 -> zw
                auto* sw_lhs = b.Swizzle(ty.vec2<u32>(), ubo, {2, 3});
                // else -> xy
                auto* sw_rhs = b.Swizzle(ty.vec2<u32>(), ubo, {0, 1});
                auto* cond = b.Equal(ty.bool_(), vec_idx, 2_u);
                auto args = Vector{sw_rhs->Result(0), sw_lhs->Result(0), cond->Result(0)};
                load = b.ir.CreateInstruction<hlsl::ir::Ternary>(
                    b.InstructionResult(ty.vec2<u32>()), std::move(args));
                b.Append(load);
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
            auto* vec_idx = CalculateVectorOffset(byte_idx);  // 0, 1, 2, or 3
            if (auto* cnst = vec_idx->As<core::ir::Constant>()) {
                const auto vec_idx_val = cnst->Value()->ValueAs<uint32_t>();
                load = b.Swizzle(ty.u32(), b.Load(access), {vec_idx_val});
            } else {
                load = b.Access(ty.u32(), b.Load(access), vec_idx);
            }
            return b.Bitcast(result_ty, load);
        }

        TINT_UNREACHABLE();
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
                    values.Push(MakeLoad(inst, var, mat->ColumnType(), byte_idx)->Result(0));
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
    //     offset = (offset + (i * A->Stride())) / 16
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
                TINT_ASSERT(count);

                b.LoopRange(ty, 0_u, u32(count->value), 1_u, [&](core::ir::Value* idx) {
                    auto* stride = b.Multiply<u32>(idx, u32(arr->Stride()))->Result(0);
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
                    values.Push(MakeLoad(inst, var, mem->Type(), byte_idx)->Result(0));
                }

                b.Return(fn, b.Construct(s, values));
            });

            return fn;
        });
    }
};

}  // namespace

Result<SuccessType> DecomposeUniformAccess(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "hlsl.DecomposeUniformAccess",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowClipDistancesOnF32,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
