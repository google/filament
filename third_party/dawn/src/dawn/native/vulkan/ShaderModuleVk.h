// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_SHADERMODULEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_SHADERMODULEVK_H_

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/HashUtils.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/ShaderModule.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

struct ProgrammableStage;

namespace vulkan {

class Device;
class PipelineLayout;

class ShaderModule final : public ShaderModuleBase {
  public:
    struct ModuleAndSpirv {
        VkShaderModule module;
        std::vector<uint32_t> spirv;
        bool hasInputAttachment;
    };

    static ResultOrError<Ref<ShaderModule>> Create(
        Device* device,
        const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
        const std::vector<tint::wgsl::Extension>& internalExtensions,
        ShaderModuleParseResult* parseResult);

    // Caller is responsible for destroying the `VkShaderModule` returned.
    ResultOrError<ModuleAndSpirv> GetHandleAndSpirv(SingleShaderStage stage,
                                                    const ProgrammableStage& programmableStage,
                                                    const PipelineLayout* layout,
                                                    bool emitPointSize,
                                                    const ImmediateConstantMask& pipelineMask);

  private:
    ShaderModule(Device* device,
                 const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                 std::vector<tint::wgsl::Extension> internalExtensions);
    ~ShaderModule() override;
    MaybeError Initialize(ShaderModuleParseResult* parseResult);
    void DestroyImpl() override;
};

}  // namespace vulkan

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_VULKAN_SHADERMODULEVK_H_
