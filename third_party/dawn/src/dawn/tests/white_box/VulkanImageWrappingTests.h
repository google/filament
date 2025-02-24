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

#ifndef SRC_DAWN_TESTS_WHITE_BOX_VULKANIMAGEWRAPPINGTESTS_H_
#define SRC_DAWN_TESTS_WHITE_BOX_VULKANIMAGEWRAPPINGTESTS_H_

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <vector>

// This must be above all other includes otherwise VulkanBackend.h includes vulkan.h before we had
// time to wrap it with vulkan_platform.h
#include "dawn/common/vulkan_platform.h"

#include "dawn/common/NonCopyable.h"
#include "dawn/native/VulkanBackend.h"

namespace dawn::native::vulkan {

struct ExternalImageDescriptorVkForTesting;
struct ExternalImageExportInfoVkForTesting;

class VulkanImageWrappingTestBackend {
  public:
    virtual ~VulkanImageWrappingTestBackend() = default;

    class ExternalTexture : NonCopyable {
      public:
        virtual ~ExternalTexture() = default;
    };
    class ExternalSemaphore : NonCopyable {
      public:
        virtual ~ExternalSemaphore() = default;
    };

    // Test parameters passed from the wrapping tests so that they can be used by the test
    // backends. The DAWN_TEST_PARAM_STRUCT is not declared here because it is unnecessary but also
    // because it declares a bunch of functions that would cause ODR violations.
    struct TestParams {
        ExternalImageType externalImageType = ExternalImageType::OpaqueFD;
        bool useDedicatedAllocation = false;
        bool detectDedicatedAllocation = false;
    };
    void SetParam(const TestParams& params);
    const TestParams& GetParam() const;
    virtual bool SupportsTestParams(const TestParams& params) const = 0;

    virtual std::unique_ptr<ExternalTexture> CreateTexture(uint32_t width,
                                                           uint32_t height,
                                                           wgpu::TextureFormat format,
                                                           wgpu::TextureUsage usage) = 0;
    virtual wgpu::Texture WrapImage(const wgpu::Device& device,
                                    const ExternalTexture* texture,
                                    const ExternalImageDescriptorVkForTesting& descriptor,
                                    std::vector<std::unique_ptr<ExternalSemaphore>> semaphores) = 0;

    virtual bool ExportImage(const wgpu::Texture& texture,
                             ExternalImageExportInfoVkForTesting* exportInfo) = 0;

    static std::unique_ptr<VulkanImageWrappingTestBackend> Create(const wgpu::Device& device,
                                                                  const TestParams params);

  private:
    TestParams mParams;
};

struct ExternalImageDescriptorVkForTesting : public ExternalImageDescriptorVk {
  public:
    explicit ExternalImageDescriptorVkForTesting(ExternalImageType type)
        : ExternalImageDescriptorVk(type) {}
};

struct ExternalImageExportInfoVkForTesting : public ExternalImageExportInfoVk {
  public:
    explicit ExternalImageExportInfoVkForTesting(ExternalImageType type)
        : ExternalImageExportInfoVk(type) {}
    std::vector<std::unique_ptr<VulkanImageWrappingTestBackend::ExternalSemaphore>> semaphores;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_TESTS_WHITE_BOX_VULKANIMAGEWRAPPINGTESTS_H_
