// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/helpers/flatten_bindings.h"

#include <utility>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/inspector/inspector.h"

namespace tint::wgsl {

std::optional<Program> FlattenBindings(const Program& program) {
    // TODO(crbug.com/tint/1101): Make this more robust for multiple entry points.
    tint::ast::transform::BindingRemapper::BindingPoints binding_points;
    uint32_t next_buffer_idx = 0;
    uint32_t next_sampler_idx = 0;
    uint32_t next_texture_idx = 0;

    tint::inspector::Inspector inspector(program);
    auto entry_points = inspector.GetEntryPoints();
    for (auto& entry_point : entry_points) {
        auto bindings = inspector.GetResourceBindings(entry_point.name);

        for (auto& binding : bindings) {
            BindingPoint src = {binding.bind_group, binding.binding};
            if (binding_points.count(src)) {
                continue;
            }
            switch (binding.resource_type) {
                case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
                case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
                case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
                    binding_points.emplace(src, BindingPoint{0, next_buffer_idx++});
                    break;
                case tint::inspector::ResourceBinding::ResourceType::kSampler:
                case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
                    binding_points.emplace(src, BindingPoint{0, next_sampler_idx++});
                    break;
                case tint::inspector::ResourceBinding::ResourceType::kSampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
                case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageTexture:
                case tint::inspector::ResourceBinding::ResourceType::kReadWriteStorageTexture:
                case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
                case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
                    binding_points.emplace(src, BindingPoint{0, next_texture_idx++});
                    break;
                case tint::inspector::ResourceBinding::ResourceType::kInputAttachment:
                    // flattening is not supported for input attachments.
                    TINT_UNREACHABLE();
            }
        }
    }

    // Run the binding remapper transform.
    if (!binding_points.empty()) {
        tint::ast::transform::Manager manager;
        tint::ast::transform::DataMap inputs;
        tint::ast::transform::DataMap outputs;
        inputs.Add<tint::ast::transform::BindingRemapper::Remappings>(
            std::move(binding_points), tint::ast::transform::BindingRemapper::AccessControls{},
            /* mayCollide */ true);
        manager.Add<tint::ast::transform::BindingRemapper>();
        return manager.Run(program, inputs, outputs);
    }

    return {};
}

}  // namespace tint::wgsl
