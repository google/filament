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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <IOSurface/IOSurface.h>
#include <webgpu/webgpu_cpp.h>

#import <Metal/Metal.h>

#include "dawn/common/CoreFoundationRef.h"
#include "dawn/common/NSRef.h"
#include "dawn/native/metal/Forward.h"
#include "dawn/native/metal/SharedTextureMemoryMTL.h"
#include "dawn/tests/white_box/SharedTextureMemoryTests.h"

namespace dawn {
namespace {

void AddIntegerValue(CFMutableDictionaryRef dictionary, const CFStringRef key, int32_t value) {
    auto number = AcquireCFRef(CFNumberCreate(nullptr, kCFNumberSInt32Type, &value));
    CFDictionaryAddValue(dictionary, key, number.Get());
}

wgpu::SharedTextureMemory CreateSharedTextureMemoryHelper(const wgpu::Device& device,
                                                          bool allowStorageBinding = true) {
    auto dict = AcquireCFRef(CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    AddIntegerValue(dict.Get(), kIOSurfaceWidth, 16);
    AddIntegerValue(dict.Get(), kIOSurfaceHeight, 16);
    AddIntegerValue(dict.Get(), kIOSurfacePixelFormat, kCVPixelFormatType_32RGBA);
    AddIntegerValue(dict.Get(), kIOSurfaceBytesPerElement, 4);

    wgpu::SharedTextureMemoryIOSurfaceDescriptor ioSurfaceDesc;
    ioSurfaceDesc.ioSurface = IOSurfaceCreate(dict.Get());
    ioSurfaceDesc.allowStorageBinding = allowStorageBinding;

    wgpu::SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &ioSurfaceDesc;

    return device.ImportSharedTextureMemory(&desc);
}

class Backend : public SharedTextureMemoryTestBackend {
  public:
    static Backend* GetInstance() {
        static Backend b;
        return &b;
    }

    std::string Name() const override { return "IOSurface"; }

    std::vector<wgpu::FeatureName> RequiredFeatures(const wgpu::Adapter& device) const override {
        std::vector<wgpu::FeatureName> features = {wgpu::FeatureName::SharedTextureMemoryIOSurface,
                                                   wgpu::FeatureName::SharedFenceMTLSharedEvent,
                                                   wgpu::FeatureName::DawnMultiPlanarFormats};

        if (device.HasFeature(wgpu::FeatureName::MultiPlanarFormatNv12a)) {
            features.push_back(wgpu::FeatureName::MultiPlanarFormatNv12a);
        }
        if (device.HasFeature(wgpu::FeatureName::MultiPlanarFormatNv16)) {
            features.push_back(wgpu::FeatureName::MultiPlanarFormatNv16);
        }
        if (device.HasFeature(wgpu::FeatureName::MultiPlanarFormatNv24)) {
            features.push_back(wgpu::FeatureName::MultiPlanarFormatNv24);
        }
        if (device.HasFeature(wgpu::FeatureName::MultiPlanarFormatP010)) {
            features.push_back(wgpu::FeatureName::MultiPlanarFormatP010);
        }
        if (device.HasFeature(wgpu::FeatureName::MultiPlanarFormatP210)) {
            features.push_back(wgpu::FeatureName::MultiPlanarFormatP210);
        }
        if (device.HasFeature(wgpu::FeatureName::MultiPlanarFormatP410)) {
            features.push_back(wgpu::FeatureName::MultiPlanarFormatP410);
        }
        if (device.HasFeature(wgpu::FeatureName::Unorm16TextureFormats)) {
            features.push_back(wgpu::FeatureName::Unorm16TextureFormats);
        }

        return features;
    }

    // Create one basic shared texture memory. It should support most operations.
    wgpu::SharedTextureMemory CreateSharedTextureMemory(const wgpu::Device& device,
                                                        int layerCount) override {
        return CreateSharedTextureMemoryHelper(device);
    }

    std::vector<std::vector<wgpu::SharedTextureMemory>> CreatePerDeviceSharedTextureMemories(
        const std::vector<wgpu::Device>& devices,
        int layerCount) override {
        std::vector<std::vector<wgpu::SharedTextureMemory>> memories;

        struct IOSurfaceFormat {
            uint32_t format;
            uint32_t bytesPerElement;
            wgpu::FeatureName requiredFeature = wgpu::FeatureName(0u);
        };
        const std::array<IOSurfaceFormat, 17> kFormats{
            {{kCVPixelFormatType_64RGBAHalf, 8},
             {kCVPixelFormatType_TwoComponent16Half, 4},
             {kCVPixelFormatType_OneComponent16Half, 2},
             {kCVPixelFormatType_TwoComponent16, 4, wgpu::FeatureName::Unorm16TextureFormats},
             {kCVPixelFormatType_OneComponent16, 2, wgpu::FeatureName::Unorm16TextureFormats},
             {kCVPixelFormatType_ARGB2101010LEPacked, 4},
             {kCVPixelFormatType_32RGBA, 4},
             {kCVPixelFormatType_32BGRA, 4},
             {kCVPixelFormatType_TwoComponent8, 2},
             {kCVPixelFormatType_OneComponent8, 1},
             // Below bytes per element isn't correct.
             {kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange, 4},
             {kCVPixelFormatType_422YpCbCr8BiPlanarVideoRange, 4,
              wgpu::FeatureName::MultiPlanarFormatNv16},
             {kCVPixelFormatType_444YpCbCr8BiPlanarVideoRange, 4,
              wgpu::FeatureName::MultiPlanarFormatNv24},
             {kCVPixelFormatType_420YpCbCr8VideoRange_8A_TriPlanar, 4,
              wgpu::FeatureName::MultiPlanarFormatNv12a},
             {kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange, 8,
              wgpu::FeatureName::MultiPlanarFormatP010},
             {kCVPixelFormatType_422YpCbCr10BiPlanarVideoRange, 8,
              wgpu::FeatureName::MultiPlanarFormatP210},
             {kCVPixelFormatType_444YpCbCr10BiPlanarVideoRange, 8,
              wgpu::FeatureName::MultiPlanarFormatP410}}};

        for (auto f : kFormats) {
            for (uint32_t size : {4, 64}) {
                auto dict = AcquireCFRef(CFDictionaryCreateMutable(
                    kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
                    &kCFTypeDictionaryValueCallBacks));
                AddIntegerValue(dict.Get(), kIOSurfaceWidth, size);
                AddIntegerValue(dict.Get(), kIOSurfaceHeight, size);
                AddIntegerValue(dict.Get(), kIOSurfacePixelFormat, f.format);
                AddIntegerValue(dict.Get(), kIOSurfaceBytesPerElement, f.bytesPerElement);

                wgpu::SharedTextureMemoryIOSurfaceDescriptor ioSurfaceDesc;
                ioSurfaceDesc.ioSurface = IOSurfaceCreate(dict.Get());
                ioSurfaceDesc.allowStorageBinding = true;

                // Internally, the CV enums are defined as their fourcc values. Cast to that and use
                // it as the label. The fourcc value is a four-character name that can be
                // interpreted as a 32-bit integer enum ('ABGR', 'r011', etc.)
                std::string label = std::string(reinterpret_cast<char*>(&f.format), 4) + " " +
                                    std::to_string(size) + "x" + std::to_string(size);
                wgpu::SharedTextureMemoryDescriptor desc;
                desc.label = label.c_str();
                desc.nextInChain = &ioSurfaceDesc;

                std::vector<wgpu::SharedTextureMemory> perDeviceMemories;
                for (auto& device : devices) {
                    if (f.requiredFeature != wgpu::FeatureName(0u) &&
                        !device.HasFeature(f.requiredFeature)) {
                        continue;
                    }

                    perDeviceMemories.push_back(device.ImportSharedTextureMemory(&desc));
                }

                if (!perDeviceMemories.empty()) {
                    memories.push_back(std::move(perDeviceMemories));
                }
            }
        }

        return memories;
    }
};

// Test that a shared event can be imported, and then exported.
TEST_P(SharedTextureMemoryTests, SharedFenceSuccessfulImportExport) {
    auto mtlDevice = AcquireNSPRef(MTLCreateSystemDefaultDevice());
    auto sharedEvent = AcquireNSPRef([*mtlDevice newSharedEvent]);

    wgpu::SharedFenceMTLSharedEventDescriptor sharedEventDesc;
    sharedEventDesc.sharedEvent = static_cast<void*>(*sharedEvent);

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedEventDesc;

    wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc);

    // Release the Metal objects. They should be retained by the implementation.
    mtlDevice = nil;
    sharedEvent = nil;

    wgpu::SharedFenceMTLSharedEventExportInfo sharedEventInfo;
    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.nextInChain = &sharedEventInfo;
    fence.ExportInfo(&exportInfo);

    // The exported event should be the same as the imported one.
    EXPECT_EQ(sharedEventInfo.sharedEvent, sharedEventDesc.sharedEvent);
    EXPECT_EQ(exportInfo.type, wgpu::SharedFenceType::MTLSharedEvent);
}

// Test that it is an error to import a shared fence without enabling the feature.
TEST_P(SharedTextureMemoryNoFeatureTests, SharedFenceImportWithoutFeature) {
    auto mtlDevice = AcquireNSPRef(MTLCreateSystemDefaultDevice());
    auto sharedEvent = AcquireNSPRef([*mtlDevice newSharedEvent]);

    wgpu::SharedFenceMTLSharedEventDescriptor sharedEventDesc;
    sharedEventDesc.sharedEvent = static_cast<void*>(*sharedEvent);

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedEventDesc;

    ASSERT_DEVICE_ERROR_MSG(wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc),
                            testing::HasSubstr("MTLSharedEvent is not enabled"));
}

// Test that it is an error to import a shared fence with a null MTLSharedEvent
TEST_P(SharedTextureMemoryTests, SharedFenceImportMTLSharedEventMissing) {
    wgpu::SharedFenceMTLSharedEventDescriptor sharedEventDesc;
    sharedEventDesc.sharedEvent = nullptr;

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedEventDesc;

    ASSERT_DEVICE_ERROR_MSG(wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc),
                            testing::HasSubstr("missing"));
}

// Test exporting info from a shared fence with no chained struct.
// It should be valid and the fence type is exported.
TEST_P(SharedTextureMemoryTests, SharedFenceExportInfoNoChainedStruct) {
    auto mtlDevice = AcquireNSPRef(MTLCreateSystemDefaultDevice());
    auto sharedEvent = AcquireNSPRef([*mtlDevice newSharedEvent]);

    wgpu::SharedFenceMTLSharedEventDescriptor sharedEventDesc;
    sharedEventDesc.sharedEvent = static_cast<void*>(*sharedEvent);

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedEventDesc;

    wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc);

    // Test no chained struct.
    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.nextInChain = nullptr;

    fence.ExportInfo(&exportInfo);
    EXPECT_EQ(exportInfo.type, wgpu::SharedFenceType::MTLSharedEvent);
}

// Test exporting info from a shared fence with an invalid chained struct.
// It should not be valid, but the fence type should still be exported.
TEST_P(SharedTextureMemoryTests, SharedFenceExportInfoInvalidChainedStruct) {
    auto mtlDevice = AcquireNSPRef(MTLCreateSystemDefaultDevice());
    auto sharedEvent = AcquireNSPRef([*mtlDevice newSharedEvent]);

    wgpu::SharedFenceMTLSharedEventDescriptor sharedEventDesc;
    sharedEventDesc.sharedEvent = static_cast<void*>(*sharedEvent);

    wgpu::SharedFenceDescriptor fenceDesc;
    fenceDesc.nextInChain = &sharedEventDesc;

    wgpu::SharedFence fence = device.ImportSharedFence(&fenceDesc);

    wgpu::ChainedStructOut otherStruct;
    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.nextInChain = &otherStruct;

    ASSERT_DEVICE_ERROR(fence.ExportInfo(&exportInfo));
}

TEST_P(SharedTextureMemoryTests, DisallowStorageBinding) {
    wgpu::SharedTextureMemory memory =
        CreateSharedTextureMemoryHelper(device, /*allowStorageBinding=*/false);

    wgpu::SharedTextureMemoryProperties properties;
    memory.GetProperties(&properties);

    EXPECT_FALSE(properties.usage & wgpu::TextureUsage::StorageBinding);

    const dawn::native::metal::SharedTextureMemory* memoryMtl =
        dawn::native::metal::ToBackend(dawn::native::FromAPI(memory.Get()));

    EXPECT_FALSE(memoryMtl->GetMtlTextureUsage() & MTLTextureUsageShaderWrite);
    EXPECT_TRUE(memoryMtl->GetMtlPlaneTextures()[0]);
    EXPECT_EQ(memoryMtl->GetMtlPlaneTextures()[0].Get().usage, memoryMtl->GetMtlTextureUsage());
}

DAWN_INSTANTIATE_PREFIXED_TEST_P(Metal,
                                 SharedTextureMemoryNoFeatureTests,
                                 {MetalBackend()},
                                 {Backend::GetInstance()},
                                 {1});

DAWN_INSTANTIATE_PREFIXED_TEST_P(Metal,
                                 SharedTextureMemoryTests,
                                 {MetalBackend()},
                                 {Backend::GetInstance()},
                                 {1});

}  // anonymous namespace
}  // namespace dawn
