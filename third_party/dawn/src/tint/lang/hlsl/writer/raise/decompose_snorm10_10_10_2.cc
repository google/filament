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

#include "src/tint/lang/hlsl/writer/raise/decompose_snorm10_10_10_2.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

struct State {
    core::ir::Module& ir;
    const std::vector<uint32_t>& locations;
    core::ir::Builder b{ir};
    core::type::Manager& ty{ir.Types()};

    struct DecodeResult {
        core::ir::InstructionResult* value = nullptr;
        core::ir::Instruction* multiply_inst = nullptr;
    };

    void Process() {
        std::unordered_set<uint32_t> emulated_locs(locations.begin(), locations.end());

        for (auto* func : ir.functions) {
            if (!func->IsVertex()) {
                continue;
            }

            for (auto* param : func->Params()) {
                auto* struct_ty = param->Type()->As<core::type::Struct>();
                // Vertex shader parameters are always structs after ShaderIO
                TINT_ASSERT(struct_ty);

                for (auto* member : struct_ty->Members()) {
                    auto member_loc = member->Attributes().location;
                    if (!member_loc.has_value() || !emulated_locs.contains(member_loc.value())) {
                        continue;
                    }
                    uint32_t member_index = member->Index();

                    param->ForEachUseUnsorted([&](core::ir::Usage u) {
                        auto* access = u.instruction->As<core::ir::Access>();
                        if (!access || access->Indices().size() != 1) {
                            return;
                        }
                        auto* const_idx = access->Indices()[0]->As<core::ir::Constant>();
                        if (!const_idx || const_idx->Value()->ValueAs<uint32_t>() != member_index) {
                            return;
                        }

                        // Decode the access locally close to its usage to minimize register
                        // liveness/pressure. Duplicate access decodes will be merged later by
                        // Common Subexpression Elimination (CSE) and instruction deduplication
                        // at the downstream native shader compilers
                        auto* access_val = access->Result();
                        b.InsertAfter(access, [&] {
                            auto decoded = Decode(access_val);

                            // Replace all uses of the access result with the decoded result, except
                            // the use in the decoding logic's first instruction. We manually
                            // iterate a copy of the usages rather than using
                            // Value::ReplaceAllUsesWith(replacer) because the latter expects every
                            // usage to be replaced with a different value, and would infinite loop
                            // if we returned the original value for the decoder's first
                            // instruction.
                            auto usages = access_val->UsagesSorted();
                            for (auto& use : usages) {
                                if (use.instruction != decoded.multiply_inst) {
                                    use.instruction->SetOperand(use.operand_index, decoded.value);
                                }
                            }
                        });
                    });
                }
            }
        }
    }

    DecodeResult Decode(core::ir::Value* input) {
        // Reconstruct integer values:
        auto* scaled = b.Multiply(input, b.Composite<vec4f>(1023_f, 1023_f, 1023_f, 3_f));
        auto* rounded = b.Call<vec4f>(core::BuiltinFn::kRound, scaled);

        // Sign-extend:
        // The shift left values (22, 22, 22, 30) here differ from vertex pulling's
        // (22, 12, 2, 0) because the input components have already been unpacked into separate
        // vector lanes by the hardware fetcher before reaching the shader. We only need to
        // sign-extend each lane's value (10 bits for XYZ, 2 bits for W) in-place.
        auto* s32s = b.Convert<vec4i>(rounded);
        auto* shl = b.ShiftLeft(s32s, b.Composite<vec4u>(22_u, 22_u, 22_u, 30_u));
        auto* shr = b.ShiftRight(shl, b.Composite<vec4u>(22_u, 22_u, 22_u, 30_u));

        // Normalize:
        auto* normalized =
            b.Divide(b.Convert<vec4f>(shr), b.Composite<vec4f>(511_f, 511_f, 511_f, 1_f));
        auto* decoded = b.Call<vec4f>(core::BuiltinFn::kMax, normalized, b.Splat<vec4f>(-1_f));

        return {decoded->Result(), scaled};
    }
};

}  // namespace

Result<SuccessType> DecomposeSnorm10_10_10_2(core::ir::Module& ir,
                                             const std::vector<uint32_t>& locations) {
    AssertValid(ir, "before hlsl.DecomposeSnorm10_10_10_2");

    if (!locations.empty()) {
        State{ir, locations}.Process();
    }

    return Success;
}

}  // namespace tint::hlsl::writer::raise
