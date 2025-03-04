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

#ifndef SRC_TINT_LANG_GLSL_IR_COMBINED_TEXTURE_SAMPLER_VAR_H_
#define SRC_TINT_LANG_GLSL_IR_COMBINED_TEXTURE_SAMPLER_VAR_H_

#include <string>

#include "src/tint/lang/core/ir/var.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::glsl::ir {

/// A combined texture sampler variable instruction in the IR.
class CombinedTextureSamplerVar final : public Castable<CombinedTextureSamplerVar, core::ir::Var> {
  public:
    /// Constructor
    /// @param id the instruction id
    /// @param result the result value
    /// @param texture_bp the texture binding point
    /// @param sampler_bp the sampler binding point
    CombinedTextureSamplerVar(Id id,
                              core::ir::InstructionResult* result,
                              tint::BindingPoint texture_bp,
                              tint::BindingPoint sampler_bp);

    ~CombinedTextureSamplerVar() override;

    /// @returns the texture binding point
    tint::BindingPoint TextureBindingPoint() {
        auto bp = BindingPoint();
        TINT_ASSERT(bp);
        return *bp;
    }

    /// @returns the sampler binding point
    tint::BindingPoint SamplerBindingPoint() { return sampler_binding_point_; }

    /// @copydoc core::ir::Instruction::Clone()
    CombinedTextureSamplerVar* Clone(core::ir::CloneContext& ctx) override;

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "combined_texture_sampler"; }

  private:
    tint::BindingPoint sampler_binding_point_;
};

}  // namespace tint::glsl::ir

#endif  // SRC_TINT_LANG_GLSL_IR_COMBINED_TEXTURE_SAMPLER_VAR_H_
