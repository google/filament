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

#include "src/tint/lang/glsl/writer/raise/offset_first_index.h"

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::glsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The configuration options.
    const OffsetFirstIndexConfig& config;

    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// Process the module.
    void Process() {
        // Find module-scope `in` variables that have `instance_index` or `vertex_index` builtins.
        for (auto* global : *ir.root_block) {
            auto* var = global->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto builtin = var->Attributes().builtin;
            if (builtin == core::BuiltinValue::kInstanceIndex) {
                if (config.first_instance_offset) {
                    AddOffset(var, *config.first_instance_offset);
                }
            }
            if (builtin == core::BuiltinValue::kVertexIndex) {
                if (config.first_vertex_offset) {
                    AddOffset(var, *config.first_vertex_offset);
                }
            }
        }
    }

    /// Add an offset to the value loaded from @p var.
    /// @param var the variable that contains the builtin value
    /// @param push_constant_offset the offset in the push constants where the offset is stored
    void AddOffset(core::ir::Var* var, uint32_t push_constant_offset) {
        // ShaderIO transforms these input builtins such that they are loaded a single time and then
        // converted to u32. We add the offset to the result of the conversion.
        auto* load = GetSingularUse<core::ir::Load>(var);
        auto* index = load->Result(0)->Type()->Is<core::type::U32>()
                          ? load
                          : GetSingularUse<core::ir::Convert>(load);

        // Replace users of the original load with the result of the offset calculation.
        auto* offset_index = b.InstructionResult<u32>();
        index->Result(0)->ReplaceAllUsesWith(offset_index);

        // Load the offset from the push constant structure and add it to the index.
        b.InsertAfter(index, [&] {
            auto* push_constants = config.push_constant_layout.var;
            auto idx = u32(config.push_constant_layout.IndexOf(push_constant_offset));
            auto* offset = b.Load(b.Access<ptr<push_constant, u32>>(push_constants, idx));
            b.AddWithResult(offset_index, index, offset);
        });
    }

    /// Assert that @p inst has a single use and that it is of type @p T, and return that use.
    /// @param inst the instruction to get the use for
    /// @returns the use
    template <typename T>
    core::ir::Instruction* GetSingularUse(core::ir::Instruction* inst) {
        auto& usages = inst->Result(0)->UsagesUnsorted();
        TINT_ASSERT(usages.Count() == 1);
        auto* index = usages.begin()->instruction->As<T>();
        TINT_ASSERT(index);
        return index;
    }
};

}  // namespace

Result<SuccessType> OffsetFirstIndex(core::ir::Module& ir, const OffsetFirstIndexConfig& config) {
    auto result = ValidateAndDumpIfNeeded(ir, "glsl.OffsetFirstIndex",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowHandleVarsWithoutBindings,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::glsl::writer::raise
