//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_device.h:
//    Defines the wrapper class for Metal's MTLDevice per context.
//

#ifndef LIBANGLE_RENDERER_METAL_CONTEXT_DEVICE_H_
#define LIBANGLE_RENDERER_METAL_CONTEXT_DEVICE_H_

#import <Metal/Metal.h>
#import <mach/mach_types.h>

#include "common/apple/apple_platform.h"
#include "libANGLE/renderer/metal/mtl_common.h"

namespace rx
{
namespace mtl
{

class ContextDevice final : public WrappedObject<id<MTLDevice>>, angle::NonCopyable
{
  public:
    ContextDevice(GLint ownershipIdentity);
    ~ContextDevice();
    inline void set(id<MTLDevice> metalDevice) { ParentClass::set(metalDevice); }

    AutoObjCPtr<id<MTLSamplerState>> newSamplerStateWithDescriptor(
        MTLSamplerDescriptor *descriptor) const;

    AutoObjCPtr<id<MTLTexture>> newTextureWithDescriptor(MTLTextureDescriptor *descriptor) const;
    AutoObjCPtr<id<MTLTexture>> newTextureWithDescriptor(MTLTextureDescriptor *descriptor,
                                                         IOSurfaceRef iosurface,
                                                         NSUInteger plane) const;

    AutoObjCPtr<id<MTLBuffer>> newBufferWithLength(NSUInteger length,
                                                   MTLResourceOptions options) const;
    AutoObjCPtr<id<MTLBuffer>> newBufferWithBytes(const void *pointer,
                                                  NSUInteger length,
                                                  MTLResourceOptions options) const;

    AutoObjCPtr<id<MTLComputePipelineState>> newComputePipelineStateWithFunction(
        id<MTLFunction> computeFunction,
        __autoreleasing NSError **error) const;
    AutoObjCPtr<id<MTLRenderPipelineState>> newRenderPipelineStateWithDescriptor(
        MTLRenderPipelineDescriptor *descriptor,
        __autoreleasing NSError **error) const;

    AutoObjCPtr<id<MTLDepthStencilState>> newDepthStencilStateWithDescriptor(
        MTLDepthStencilDescriptor *descriptor) const;

    AutoObjCPtr<id<MTLSharedEvent>> newSharedEvent() const;
    AutoObjCPtr<id<MTLEvent>> newEvent() const;

    void setOwnerWithIdentity(id<MTLResource> resource) const;
    bool hasUnifiedMemory() const;

  private:
    using ParentClass = WrappedObject<id<MTLDevice>>;

#if ANGLE_USE_METAL_OWNERSHIP_IDENTITY
    task_id_token_t mOwnershipIdentity = TASK_ID_TOKEN_NULL;
#endif
};

}  // namespace mtl
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_CONTEXT_DEVICE_H_ */
