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

// File containing utilities common to both types of pipelines. However adding a base class doesn't
// work as well as one would hope because we would want to use diamond inheritance and that's to
// brittle.

#ifndef SRC_DAWN_NATIVE_VULKAN_PIPELINEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_PIPELINEVK_H_

#include <utility>

#include "dawn/native/vulkan/PipelineLayoutVk.h"

namespace dawn::native::vulkan {

// The pipeline layouts are specialized per-pipeline to use a tight bound on the number of push
// constants used, so store both handles (the pipeline and specialized layout) together.
struct PipelineHandles {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
};

// Both types of pipelines need similar specialization information.
struct CommonPipelineSpecialization {
    PipelineLayout::Specialization layout;
    absl::flat_hash_set<APIBindPoint> ycbcrExternalTextures = {};

    template <typename H>
    friend H AbslHashValue(H h, const CommonPipelineSpecialization& s) {
        return H::combine(std::move(h), s.layout, s.ycbcrExternalTextures);
    }
    bool operator==(const CommonPipelineSpecialization& other) const = default;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_PIPELINEVK_H_
