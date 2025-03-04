// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_TESTS_WHITE_BOX_SHAREDTEXTUREMEMORYTESTS_H_
#define SRC_DAWN_TESTS_WHITE_BOX_SHAREDTEXTUREMEMORYTESTS_H_

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/tests/DawnTest.h"

namespace dawn {

class SharedTextureMemoryTestBackend {
  public:
    virtual void SetUp() {}
    virtual void TearDown() {}

    // The name used in gtest parameterization. Names of backends must be unique.
    virtual std::string Name() const = 0;

    // The required features for testing this backend.
    virtual std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter& device) const = 0;

    // Create one basic shared texture memory. It should support most operations.
    virtual wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                                int layerCount) = 0;

    // Create a variety of valid SharedTextureMemory for testing, one on each device.
    // Backends should return all interesting types of shared texture memory here, including
    // different sizes, formats, memory types, etc.
    // The inner vector is a vector of the same memory imported to each device.
    virtual std::vector<std::vector<wgpu::SharedTextureMemory>>
    CreatePerDeviceSharedTextureMemories(const std::vector<wgpu::Device>& devices,
                                         int layerCount) = 0;

    // Import `fence` which may have been created on some other device, onto `importingDevice`.
    wgpu::SharedFence ImportFenceTo(const wgpu::Device& importingDevice,
                                    const wgpu::SharedFence& fence);

    // Shorthand version of `CreatePerDeviceSharedTextureMemories` that creates memories on a single
    // device.
    std::vector<wgpu::SharedTextureMemory> CreateSharedTextureMemories(wgpu::Device& device,
                                                                       int layerCount);

    // Wrapper around CreateSharedTextureMemories() that restricts the returned
    // vector to only the single-planar instances.
    std::vector<wgpu::SharedTextureMemory> CreateSinglePlanarSharedTextureMemories(
        wgpu::Device& device,
        int layerCount);

    // Wrapper around CreatePerDeviceSharedTextureMemories that filters the memories by
    // usage to ensure they have `requiredUsage`.
    std::vector<std::vector<wgpu::SharedTextureMemory>>
    CreatePerDeviceSharedTextureMemoriesFilterByUsage(const std::vector<wgpu::Device>& devices,
                                                      wgpu::TextureUsage requiredUsage,
                                                      int layerCount);

    // Return true if the test should always use the same device.
    // Some interop paths require the same underyling backend device.
    virtual bool UseSameDevice() const { return false; }

    // Whether or not the backing supports concurrent reads. This is
    // a property of the underlying API (keyed mutex, vk binary semaphore),
    // so it is concurrent reads across disjoint Dawn devices - not concurrent
    // reads on the same Dawn device.
    virtual bool SupportsConcurrentRead() const { return true; }

    // Opaque object which holds backend-specific begin access state.
    class BackendBeginState {
      public:
        virtual ~BackendBeginState() = default;
    };

    // Opaque object which holds backend-specific end access state.
    class BackendEndState {
      public:
        virtual ~BackendEndState() = default;
    };

    // Create backend-specific begin access state suitable for initial import and chain it on
    // `beginDesc` overwriting all chains. Backend-specific state should be allocated inside of the
    // returned `BackendBeginState`.
    virtual std::unique_ptr<BackendBeginState> ChainInitialBeginState(
        wgpu::SharedTextureMemoryBeginAccessDescriptor* beginDesc) {
        beginDesc->nextInChain = nullptr;
        return std::make_unique<BackendBeginState>();
    }

    // Create backend-specific end access state and chain it on `endState` overwriting all chains.
    // Backend-specific state should be allocated inside of the returned `BackendEndState`.
    virtual std::unique_ptr<BackendEndState> ChainEndState(
        wgpu::SharedTextureMemoryEndAccessState* endState) {
        endState->nextInChain = nullptr;
        return std::make_unique<BackendEndState>();
    }

    // Create backend-specific begin access state that is suitable following a prior export using
    // the backend-specific information written in the provided `endState`. Chain the being state on
    // `beignDesc` overwriting all chains. Backend-specific state should be allocated inside of the
    // returned `BackendBeginState`.
    virtual std::unique_ptr<BackendBeginState> ChainBeginState(
        wgpu::SharedTextureMemoryBeginAccessDescriptor* beginDesc,
        const wgpu::SharedTextureMemoryEndAccessState& endState) {
        beginDesc->nextInChain = nullptr;
        return std::make_unique<BackendBeginState>();
    }
};

class SharedTextureMemoryTestVulkanBackend : public SharedTextureMemoryTestBackend {
  public:
    std::unique_ptr<BackendBeginState> ChainInitialBeginState(
        wgpu::SharedTextureMemoryBeginAccessDescriptor* beginDesc) override;

    std::unique_ptr<BackendEndState> ChainEndState(
        wgpu::SharedTextureMemoryEndAccessState* endState) override;

    std::unique_ptr<BackendBeginState> ChainBeginState(
        wgpu::SharedTextureMemoryBeginAccessDescriptor* beginDesc,
        const wgpu::SharedTextureMemoryEndAccessState& endState) override;
};

inline std::ostream& operator<<(std::ostream& o, SharedTextureMemoryTestBackend* backend) {
    o << backend->Name();
    return o;
}

using Backend = SharedTextureMemoryTestBackend*;
using LayerCount = int;
DAWN_TEST_PARAM_STRUCT(SharedTextureMemoryTestParams, Backend, LayerCount);

class SharedTextureMemoryNoFeatureTests : public DawnTestWithParams<SharedTextureMemoryTestParams> {
  protected:
    void SetUp() override;
    void TearDown() override;
};

class SharedTextureMemoryTests : public DawnTestWithParams<SharedTextureMemoryTestParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override;

    void SetUp() override;
    void TearDown() override;

    wgpu::Device CreateDevice();

    void UseInRenderPass(wgpu::Device& deviceObj, wgpu::Texture& texture);
    void UseInCopy(wgpu::Device& deviceObj, wgpu::Texture& texture);

    wgpu::CommandBuffer MakeFourColorsClearCommandBuffer(wgpu::Device& deviceObj,
                                                         wgpu::Texture& texture);
    wgpu::CommandBuffer MakeFourColorsComputeCommandBuffer(wgpu::Device& deviceObj,
                                                           wgpu::Texture& texture);
    void WriteFourColorsToRGBA8Texture(wgpu::Device& deviceObj, wgpu::Texture& texture);
    std::pair<wgpu::CommandBuffer, wgpu::Texture> MakeCheckBySamplingCommandBuffer(
        wgpu::Device& deviceObj,
        wgpu::Texture& texture);
    std::pair<wgpu::CommandBuffer, wgpu::Texture> MakeCheckBySamplingTwoTexturesCommandBuffer(
        wgpu::Texture& texture0,
        wgpu::Texture& texture1);
    std::pair<wgpu::CommandBuffer, wgpu::Texture> MakeCheckBySamplingTexture2DArrayCommandBuffer(
        wgpu::Device& deviceObj,
        wgpu::Texture& texture);
    void CheckFourColors(wgpu::Device& deviceObj,
                         wgpu::TextureFormat format,
                         wgpu::Texture& colorTarget);
};

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_WHITE_BOX_SHAREDTEXTUREMEMORYTESTS_H_
