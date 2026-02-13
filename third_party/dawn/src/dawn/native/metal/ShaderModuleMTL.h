// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_METAL_SHADERMODULEMTL_H_
#define SRC_DAWN_NATIVE_METAL_SHADERMODULEMTL_H_

#include <memory>
#include <string>
#include <vector>

#include "dawn/native/ShaderModule.h"

#include "dawn/common/NSRef.h"
#include "dawn/native/Error.h"

#import <Metal/Metal.h>

namespace dawn::native {
struct ProgrammableStage;
}

namespace dawn::native::metal {

class Device;
class PipelineLayout;
class RenderPipeline;

class ShaderModule final : public ShaderModuleBase {
  public:
    static ResultOrError<Ref<ShaderModule>> Create(
        Device* device,
        const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
        const std::vector<tint::wgsl::Extension>& internalExtensions,
        ShaderModuleParseResult* parseResult);

    struct MetalFunctionData {
        std::string msl;
        NSPRef<id<MTLFunction>> function;
        bool needsStorageBufferLength;
        std::vector<uint32_t> workgroupAllocations;
        MTLSize localWorkgroupSize;
    };

    MaybeError CreateFunction(SingleShaderStage stage,
                              const ProgrammableStage& programmableStage,
                              const PipelineLayout* layout,
                              MetalFunctionData* out,
                              uint32_t sampleMask = 0xFFFFFFFF,
                              const RenderPipeline* renderPipeline = nullptr);

  private:
    ShaderModule(Device* device,
                 const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                 std::vector<tint::wgsl::Extension> internalExtensions);
    ~ShaderModule() override;

    MaybeError Initialize(ShaderModuleParseResult* parseResult);
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_SHADERMODULEMTL_H_
