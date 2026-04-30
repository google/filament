// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_METAL_UTILSMETAL_H_
#define SRC_DAWN_NATIVE_METAL_UTILSMETAL_H_

#import <Metal/Metal.h>

#include <string>

#include "absl/container/inlined_vector.h"
#include "dawn/common/NSRef.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/ShaderModuleMTL.h"
#include "dawn/native/metal/TextureMTL.h"

namespace dawn::native {
struct BeginRenderPassCmd;
struct ProgrammableStage;
struct EntryPointMetadata;
enum class SingleShaderStage;
}  // namespace dawn::native

namespace dawn::native::metal {

MTLPixelFormat MetalPixelFormat(const DeviceBase* device, wgpu::TextureFormat format);

NSRef<NSString> MakeDebugName(DeviceBase* device, const char* prefix, std::string_view label = "");

// Templating for setting the label on MTL objects because not all MTL objects are of the same base
// class. For example MTLBuffer and MTLTexture inherit MTLResource, but MTLFunction does not. Note
// that we allow a nullable Metal object because APISetLabel does not currently do any checks on
// backend resources.
template <typename T>
void SetDebugName(DeviceBase* device, T* mtlObj, const char* prefix, std::string_view label = "") {
    if (!device->IsToggleEnabled(Toggle::UseUserDefinedLabelsInBackend)) {
        return;
    }
    if (mtlObj == nullptr) {
        return;
    }
    NSRef<NSString> debugName = MakeDebugName(device, prefix, label);
    [mtlObj setLabel:debugName.Get()];
}

Aspect GetDepthStencilAspects(MTLPixelFormat format);
MTLCompareFunction ToMetalCompareFunction(wgpu::CompareFunction compareFunction);

// Helpers to convert from typed Origin/Extent to the Metal equivalent. Metal always counts in
// texels so there are no overloads with BlockExtent/Origin3D on purpose.
MTLSize ToMTLSize(const TexelExtent3D& extent);
MTLOrigin ToMTLOrigin(const TexelOrigin3D& origin);

// When using argument buffers, we use the compacted BindingIndex directly in MSL, instead of
// remapping to per-resource-type indices (from GetBindingIndexInfo) like we do without argbufs.
inline uint32_t ToMTLArgumentBufferIndex(BindingIndex bindingIndex) {
    return uint32_t(bindingIndex);
}

// For different reasons a WebGPU copy may need to be split into multiple copies for Metal. This
// structure and its associated function `ComputeTextureBufferCopySplit` have the necessary logic
// for the splits.
// They also convert from Dawn's types (with blocks origin/extent and blocks per row/image) to the
// types that Metal expect (TexelExtent/Origin3D as that's what matches MTLSize/Origin's semantics)
struct TextureBufferCopySplit {
    // Avoid allocations except in the worse case. Most cases require at most 3 regions.
    static constexpr uint32_t kNumCommonTextureBufferCopyRegions = 3;

    struct CopyInfo {
        CopyInfo(NSUInteger bufferOffset,
                 NSUInteger bytesPerRow,
                 NSUInteger bytesPerImage,
                 TexelOrigin3D textureOrigin,
                 TexelExtent3D copyExtent)
            : bufferOffset(bufferOffset),
              bytesPerRow(bytesPerRow),
              bytesPerImage(bytesPerImage),
              textureOrigin(textureOrigin),
              copyExtent(copyExtent) {}

        NSUInteger bufferOffset;
        NSUInteger bytesPerRow;
        NSUInteger bytesPerImage;
        TexelOrigin3D textureOrigin;
        TexelExtent3D copyExtent;
    };

    absl::InlinedVector<CopyInfo, kNumCommonTextureBufferCopyRegions> copies;

    auto begin() const { return copies.begin(); }
    auto end() const { return copies.end(); }
    void push_back(const CopyInfo& copyInfo) { copies.push_back(copyInfo); }
};
TextureBufferCopySplit ComputeTextureBufferCopySplit(const Texture* texture,
                                                     uint32_t mipLevel,
                                                     BlockOrigin3D origin,
                                                     BlockExtent3D copyExtent,
                                                     uint64_t bufferSize,
                                                     uint64_t bufferOffset,
                                                     BlockCount blocksPerRow,
                                                     BlockCount rowsPerImage,
                                                     Aspect aspect);

MaybeError EnsureDestinationTextureInitialized(CommandRecordingContext* commandContext,
                                               Texture* texture,
                                               const TextureCopy& dst,
                                               const Extent3D& size);

// Helper functions to encode Metal render passes that take care of multiple workarounds that
// happen at the render pass start and end. Because workarounds wrap the encoding of the render
// pass, the encoding must be entirely done by the `encodeInside` callback.
// At the end of this function, `commandContext` will have no encoder open.
using EncodeInsideRenderPass =
    std::function<MaybeError(id<MTLRenderCommandEncoder>, BeginRenderPassCmd* renderPassCmd)>;
MaybeError EncodeMetalRenderPass(Device* device,
                                 CommandRecordingContext* commandContext,
                                 const RenderPassResourceUsage* resourceUsage,
                                 MTLRenderPassDescriptor* mtlRenderPass,
                                 uint32_t width,
                                 uint32_t height,
                                 EncodeInsideRenderPass encodeInside,
                                 BeginRenderPassCmd* renderPassCmd = nullptr);

void MetalComputePassMakeResourcesResident(DeviceBase* device,
                                           id<MTLComputeCommandEncoder> encoder,
                                           const SyncScopeResourceUsage& resourceUsage);

id<MTLTexture> CreateTextureMtlForPlane(MTLTextureUsage mtlUsage,
                                        const Format& format,
                                        size_t plane,
                                        Device* device,
                                        IOSurfaceRef ioSurface);

MaybeError EncodeEmptyMetalRenderPass(Device* device,
                                      CommandRecordingContext* commandContext,
                                      MTLRenderPassDescriptor* mtlRenderPass,
                                      Extent3D size);

bool SupportCounterSamplingAtCommandBoundary(id<MTLDevice> device);
bool SupportCounterSamplingAtStageBoundary(id<MTLDevice> device);

bool SupportTextureComponentSwizzle(id<MTLDevice> device);

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_UTILSMETAL_H_
