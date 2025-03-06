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

#ifndef SRC_DAWN_NATIVE_METAL_SHAREDTEXTUREMEMORYMTL_H_
#define SRC_DAWN_NATIVE_METAL_SHAREDTEXTUREMEMORYMTL_H_

#include <IOSurface/IOSurfaceRef.h>
#import <Metal/Metal.h>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "dawn/common/CoreFoundationRef.h"
#include "dawn/common/NSRef.h"
#include "dawn/native/Error.h"
#include "dawn/native/SharedTextureMemory.h"
#include "dawn/native/Subresource.h"

namespace dawn::native::metal {

class Device;
class CommandRecordingContext;

class SharedTextureMemory final : public SharedTextureMemoryBase {
  public:
    static ResultOrError<Ref<SharedTextureMemory>> Create(
        Device* device,
        StringView label,
        const SharedTextureMemoryIOSurfaceDescriptor* descriptor);

    IOSurfaceRef GetIOSurface() const;
    const absl::InlinedVector<NSPRef<id<MTLTexture>>, kMaxPlanesPerFormat>& GetMtlPlaneTextures()
        const;
    MTLTextureUsage GetMtlTextureUsage() const;
    MTLPixelFormat GetMtlPixelFormat() const;

  private:
    SharedTextureMemory(Device* device,
                        StringView label,
                        const SharedTextureMemoryProperties& properties,
                        IOSurfaceRef ioSurface,
                        MTLPixelFormat mtlFormat,
                        MTLTextureUsage mtlUsage);
    // Performs initialization of the base class followed by Metal-specific
    // initialization.
    MaybeError Initialize();

    void DestroyImpl() override;

    ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const UnpackedPtr<TextureDescriptor>& descriptor) override;
    MaybeError BeginAccessImpl(TextureBase* texture,
                               const UnpackedPtr<BeginAccessDescriptor>& descriptor) override;
    ResultOrError<FenceAndSignalValue> EndAccessImpl(TextureBase* texture,
                                                     ExecutionSerial lastUsageSerial,
                                                     UnpackedPtr<EndAccessState>& state) override;
    MaybeError CreateMtlTextures();

    CFRef<IOSurfaceRef> mIOSurface;
    const MTLPixelFormat mMtlFormat;
    const MTLTextureUsage mMtlUsage;
    absl::InlinedVector<NSPRef<id<MTLTexture>>, kMaxPlanesPerFormat> mMtlPlaneTextures;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_SHAREDTEXTUREMEMORYMTL_H_
