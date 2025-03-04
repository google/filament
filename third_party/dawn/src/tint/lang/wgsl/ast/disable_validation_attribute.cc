// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/ast/clone_context.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::DisableValidationAttribute);

namespace tint::ast {

DisableValidationAttribute::DisableValidationAttribute(GenerationID pid,
                                                       NodeID nid,
                                                       DisabledValidation val)
    : Base(pid, nid, tint::Empty), validation(val) {}

DisableValidationAttribute::~DisableValidationAttribute() = default;

std::string DisableValidationAttribute::InternalName() const {
    switch (validation) {
        case DisabledValidation::kFunctionHasNoBody:
            return "disable_validation__function_has_no_body";
        case DisabledValidation::kBindingPointCollision:
            return "disable_validation__binding_point_collision";
        case DisabledValidation::kIgnoreAddressSpace:
            return "disable_validation__ignore_address_space";
        case DisabledValidation::kEntryPointParameter:
            return "disable_validation__entry_point_parameter";
        case DisabledValidation::kFunctionParameter:
            return "disable_validation__function_parameter";
        case DisabledValidation::kIgnoreStrideAttribute:
            return "disable_validation__ignore_stride";
        case DisabledValidation::kIgnoreInvalidPointerArgument:
            return "disable_validation__ignore_invalid_pointer_argument";
        case DisabledValidation::kIgnorePointerAliasing:
            return "disable_validation__ignore_pointer_aliasing";
        case DisabledValidation::kIgnoreStructMemberLimit:
            return "disable_validation__ignore_struct_member";
        case DisabledValidation::kIgnoreClipDistancesType:
            return "disable_validation__ignore_clip_distances_type";
    }
    return "<invalid>";
}

const DisableValidationAttribute* DisableValidationAttribute::Clone(CloneContext& ctx) const {
    return ctx.dst->ASTNodes().Create<DisableValidationAttribute>(
        ctx.dst->ID(), ctx.dst->AllocateNodeID(), validation);
}

}  // namespace tint::ast
