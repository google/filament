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

#include "src/tint/lang/core/ir/transform/array_length_from_uniform.h"

#include <algorithm>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The binding point to use for the uniform buffer.
    BindingPoint ubo_binding;

    /// The map from binding point to the element index which holds the size of that buffer.
    const std::unordered_map<BindingPoint, uint32_t>& bindpoint_to_size_index;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The uniform buffer variable that holds the total size of each storage buffer.
    Var* buffer_sizes_var = nullptr;

    /// The construct instruction that creates the array lengths structure in the entry point.
    Construct* lengths_constructor = nullptr;

    /// A map from an array function parameter to the function parameter that holds its length.
    Hashmap<FunctionParam*, FunctionParam*, 8> array_param_to_length_param{};

    /// A map from a function to the structure that holds all of the array lengths.
    Hashmap<Function*, Value*, 8> function_to_lengths_structure{};

    /// A list of structure members for the array lengths structure.
    Vector<type::Manager::StructMemberDesc, 8> lengths_structure_members{};

    /// A map from a binding point to its index in the array length structure.
    Hashmap<BindingPoint, uint32_t, 8> bindpoint_to_length_member_index{};

    /// An ordered list of binding points that map to the structure members.
    struct BindingPointInfo {
        BindingPoint binding_point{};
        const type::Type* store_type = nullptr;
    };
    Vector<BindingPointInfo, 8> ordered_bindpoints{};

    /// A map from block to its containing function.
    Hashmap<core::ir::Block*, core::ir::Function*, 64> block_to_function{};

    /// Process the module.
    void Process() {
        // Seed the block-to-function map with the function entry blocks.
        // This is used to determine the owning function for any given instruction.
        for (auto& func : ir.functions) {
            block_to_function.Add(func->Block(), func);
        }

        // Look for and replace calls to the array length builtin.
        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<CoreBuiltinCall>()) {
                if (call->Func() == BuiltinFn::kArrayLength) {
                    MaybeReplace(call);
                }
            }
        }

        // Create the lengths structure and update all of the places that need to use it.
        // We can only do this after we have replaced all of the array length callsites, now that we
        // know all of the structure members that we need.
        CreateLengthsStructure();
    }

    /// Replace a call to an array length builtin, if the variable appears in the bindpoint map.
    /// @param call the arrayLength call to replace
    void MaybeReplace(CoreBuiltinCall* call) {
        if (auto* length = GetComputedLength(call->Args()[0], call)) {
            call->Result()->ReplaceAllUsesWith(length);
            call->Destroy();
        }
    }

    /// Get the computed length value for a runtime-sized array pointer.
    /// @param ptr the pointer to the runtime-sized array
    /// @param insertion_point the insertion point for new instructions
    /// @returns the computed length, or nullptr if the original builtin should be used
    Value* GetComputedLength(Value* ptr, Instruction* insertion_point) {
        // Trace back from the value until we reach the originating variable.
        while (true) {
            if (auto* param = ptr->As<FunctionParam>()) {
                // The length of an array pointer passed as a function parameter will be passed as
                // an additional parameter to the function.
                return GetArrayLengthParam(param);
            }

            if (auto* result = ptr->As<InstructionResult>()) {
                if (auto* var = result->Instruction()->As<Var>()) {
                    // We found the originating variable, so compute its array length.
                    return ComputeArrayLength(var, insertion_point);
                }
                if (auto* access = result->Instruction()->As<Access>()) {
                    ptr = access->Object();
                    continue;
                }
                if (auto* let = result->Instruction()->As<Let>()) {
                    ptr = let->Value();
                    continue;
                }
                TINT_UNREACHABLE() << "unhandled source of a storage buffer pointer: "
                                   << result->Instruction()->TypeInfo().name;
            }
            TINT_UNREACHABLE() << "unhandled source of a storage buffer pointer: "
                               << ptr->TypeInfo().name;
        }
    }

    /// Get (or create) the array length parameter that corresponds to an array parameter.
    /// @param array_param the array parameter
    /// @returns the array length parameter
    FunctionParam* GetArrayLengthParam(FunctionParam* array_param) {
        return array_param_to_length_param.GetOrAdd(array_param, [&] {
            // Add a new parameter to receive the array length.
            auto* length = b.FunctionParam<u32>("tint_array_length");
            array_param->Function()->AppendParam(length);

            // Update callsites of this function to pass the array length to it.
            array_param->Function()->ForEachUseUnsorted([&](core::ir::Usage use) {
                if (auto* call = use.instruction->As<core::ir::UserCall>()) {
                    // Get the length of the array in the calling function and pass that.
                    auto* arg = call->Args()[array_param->Index()];
                    auto* len = GetComputedLength(arg, call);
                    if (!len) {
                        // The originating variable was not in the bindpoint map, so we need to call
                        // the original arrayLength builtin as the callee is expecting a value.
                        b.InsertBefore(call, [&] {
                            len = b.Call<u32>(BuiltinFn::kArrayLength, arg)->Result();
                        });
                    }
                    call->AppendArg(len);
                }
            });

            return length;
        });
    }

    /// Get (or create) the array lengths structure for a function.
    /// @param func the function that needs the structure
    /// @returns the array lengths structure
    Value* GetArrayLengthsStructure(Function* func) {
        return function_to_lengths_structure.GetOrAdd(func, [&]() -> Value* {
            if (func->IsEntryPoint()) {
                // Create a placeholder construct instruction for the lengths structure that will be
                // filled in later when we know all of the structure members.
                TINT_ASSERT(lengths_constructor == nullptr);
                lengths_constructor = b.ConstructWithResult(ir.CreateValue<InstructionResult>());
                lengths_constructor->InsertBefore(func->Block()->Front());
                return lengths_constructor->Result();
            }

            // Add a new parameter to receive the array lengths structure.
            // The type is a placeholder and will be filled in later when we create the struct type.
            auto* lengths = b.FunctionParam("tint_array_lengths", nullptr);
            func->AppendParam(lengths);

            // Update callsites of this function to pass the structure to it.
            func->ForEachUseUnsorted([&](core::ir::Usage use) {
                if (auto* call = use.instruction->As<core::ir::UserCall>()) {
                    // Get the structure in the calling function and pass that.
                    auto* caller = ContainingFunction(call);
                    call->AppendArg(GetArrayLengthsStructure(caller));
                }
            });

            return lengths;
        });
    }

    /// Compute the array length of the runtime-sized array that is inside a storage buffer
    /// variable. If the variable's binding point is not found in the bindpoint map, returns nullptr
    /// to indicate that the original arrayLength builtin should be used instead.
    ///
    /// @param var the storage buffer variable that contains the runtime-sized array
    /// @param insertion_point the insertion point for new instructions
    /// @returns the length of the array, or nullptr if the original builtin should be used
    Value* ComputeArrayLength(Var* var, Instruction* insertion_point) {
        auto binding = var->BindingPoint();
        TINT_ASSERT(binding);

        auto idx_it = bindpoint_to_size_index.find(*binding);
        if (idx_it == bindpoint_to_size_index.end()) {
            // If the bindpoint_to_size_index map does not contain an entry for the storage buffer,
            // then we preserve the arrayLength() call.
            return nullptr;
        }

        // Get the index of the structure member that holds the length for this binding point,
        // creating the structure member descriptor if necessary.
        auto member_index = bindpoint_to_length_member_index.GetOrAdd(*binding, [&]() {
            auto index = static_cast<uint32_t>(lengths_structure_members.Length());
            auto name = "tint_array_length_" + std::to_string(binding->group) + "_" +
                        std::to_string(binding->binding);
            lengths_structure_members.Push(type::Manager::StructMemberDesc{
                .name = ir.symbols.Register(name),
                .type = ty.u32(),
            });
            ordered_bindpoints.Push(BindingPointInfo{
                .binding_point = *binding,
                .store_type = var->Result()->Type()->UnwrapPtr(),
            });
            return index;
        });

        // Extract the length from the structure.
        auto* length = b.Access<u32>(GetArrayLengthsStructure(ContainingFunction(insertion_point)),
                                     u32(member_index));
        length->InsertBefore(insertion_point);
        return length->Result();
    }

    /// Get (or create, on first call) the uniform buffer that contains the storage buffer sizes.
    /// @returns the uniform buffer pointer
    Value* BufferSizes() {
        if (buffer_sizes_var) {
            return buffer_sizes_var->Result();
        }

        // Find the largest index declared in the map, in order to determine the number of elements
        // needed in the array of buffer sizes.
        // The buffer sizes will be packed into vec4s to satisfy the 16-byte alignment requirement
        // for array elements in uniform buffers.
        uint32_t max_index = 0;
        for (auto& entry : bindpoint_to_size_index) {
            max_index = std::max(max_index, entry.second);
        }
        uint32_t num_elements = (max_index / 4) + 1;
        b.Append(ir.root_block, [&] {
            buffer_sizes_var = b.Var("tint_storage_buffer_sizes",
                                     ty.ptr<uniform>(ty.array(ty.vec4<u32>(), num_elements)));
        });
        buffer_sizes_var->SetBindingPoint(ubo_binding.group, ubo_binding.binding);
        return buffer_sizes_var->Result();
    }

    /// Create the structure to hold the array lengths and fill in the construct instruction that
    /// sets all of the length values.
    void CreateLengthsStructure() {
        if (lengths_structure_members.IsEmpty()) {
            return;
        }

        // Create the lengths structure.
        auto* lengths_struct = ty.Struct(ir.symbols.New("tint_array_lengths_struct"),
                                         std::move(lengths_structure_members));

        // Update all of the function parameters that need to receive the lengths structure.
        for (auto s : function_to_lengths_structure) {
            if (auto* param = s.value->As<FunctionParam>()) {
                param->SetType(lengths_struct);
            }
        }

        // Insert code at the beginning of the entry point to initialize the array length members.
        if (lengths_constructor == nullptr) {
            return;
        }
        lengths_constructor->Result()->SetType(lengths_struct);
        b.InsertBefore(lengths_constructor->Block()->Front(), [&] {
            Vector<Value*, 8> constructor_values;
            for (auto info : ordered_bindpoints) {
                TINT_ASSERT(bindpoint_to_size_index.contains(info.binding_point));
                TINT_ASSERT(bindpoint_to_length_member_index.Contains(info.binding_point));

                // Load the total storage buffer size from the uniform buffer.
                // The sizes are packed into vec4s to satisfy the 16-byte alignment requirement for
                // array elements in uniform buffers, so we have to find the vector and element that
                // correspond to the index that we want.
                const uint32_t size_index = bindpoint_to_size_index.at(info.binding_point);
                const uint32_t array_index = size_index / 4;
                const uint32_t vec_index = size_index % 4;
                auto* vec_ptr = b.Access<ptr<uniform, vec4<u32>>>(BufferSizes(), u32(array_index));
                auto* total_buffer_size = b.LoadVectorElement(vec_ptr, u32(vec_index))->Result();

                // Calculate actual array length:
                //                total_buffer_size - array_offset
                // array_length = --------------------------------
                //                             array_stride
                auto* array_size = total_buffer_size;
                const type::Array* array_type = nullptr;
                if (auto* str = info.store_type->As<core::type::Struct>()) {
                    // The variable is a struct, so subtract the byte offset of the array member.
                    auto* member = str->Members().Back();
                    array_type = member->Type()->As<core::type::Array>();
                    array_size =
                        b.Subtract<u32>(total_buffer_size, u32(member->Offset()))->Result();
                } else {
                    array_type = info.store_type->As<core::type::Array>();
                }
                TINT_ASSERT(array_type);

                auto* length = b.Divide<u32>(array_size, u32(array_type->Stride()))->Result();
                constructor_values.Push(length);
            }
            lengths_constructor->SetOperands(std::move(constructor_values));
        });
    }

    /// Get the function that contains an instruction.
    /// @param inst the instruction
    /// @returns the function
    core::ir::Function* ContainingFunction(core::ir::Instruction* inst) {
        return block_to_function.GetOrAdd(inst->Block(), [&] {  //
            return ContainingFunction(inst->Block()->Parent());
        });
    }

    /// @returns true if the transformed module needs a storage buffer sizes UBO
    bool NeedsStorageBufferSizes() { return buffer_sizes_var != nullptr; }
};

}  // namespace

Result<ArrayLengthFromUniformResult> ArrayLengthFromUniform(
    Module& ir,
    BindingPoint ubo_binding,
    const std::unordered_map<BindingPoint, uint32_t>& bindpoint_to_size_index) {
    auto validated = ValidateAndDumpIfNeeded(ir, "core.ArrayLengthFromUniform",
                                             kArrayLengthFromUniformCapabilities);
    if (validated != Success) {
        return validated.Failure();
    }

    State state{ir, ubo_binding, bindpoint_to_size_index};
    state.Process();

    ArrayLengthFromUniformResult result;
    result.needs_storage_buffer_sizes = state.NeedsStorageBufferSizes();
    return result;
}

}  // namespace tint::core::ir::transform
