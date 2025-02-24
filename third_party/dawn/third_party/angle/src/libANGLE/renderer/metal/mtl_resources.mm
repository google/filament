//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_resources.mm:
//    Implements wrapper classes for Metal's MTLTexture and MTLBuffer.
//

#include "libANGLE/renderer/metal/mtl_resources.h"

#include <TargetConditionals.h>

#include <algorithm>

#include "common/debug.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/mtl_command_buffer.h"
#include "libANGLE/renderer/metal/mtl_context_device.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#include "libANGLE/renderer/metal/mtl_utils.h"

namespace rx
{
namespace mtl
{
namespace
{
inline NSUInteger GetMipSize(NSUInteger baseSize, const MipmapNativeLevel level)
{
    return std::max<NSUInteger>(1, baseSize >> level.get());
}

// Asynchronously synchronize the content of a resource between GPU memory and its CPU cache.
// NOTE: This operation doesn't finish immediately upon function's return.
template <class T>
void InvokeCPUMemSync(ContextMtl *context, mtl::BlitCommandEncoder *blitEncoder, T *resource)
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    if (blitEncoder)
    {
        blitEncoder->synchronizeResource(resource);

        resource->resetCPUReadMemNeedSync();
        resource->setCPUReadMemSyncPending(true);
    }
#endif
}

template <class T>
void EnsureCPUMemWillBeSynced(ContextMtl *context, T *resource)
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    // Make sure GPU & CPU contents are synchronized.
    // NOTE: Only MacOS has separated storage for resource on CPU and GPU and needs explicit
    // synchronization
    if (resource->get().storageMode == MTLStorageModeManaged && resource->isCPUReadMemNeedSync())
    {
        mtl::BlitCommandEncoder *blitEncoder = context->getBlitCommandEncoder();
        InvokeCPUMemSync(context, blitEncoder, resource);
    }
#endif
    resource->resetCPUReadMemNeedSync();
}

MTLResourceOptions resourceOptionsForStorageMode(MTLStorageMode storageMode)
{
    switch (storageMode)
    {
        case MTLStorageModeShared:
            return MTLResourceStorageModeShared;
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
        case MTLStorageModeManaged:
            return MTLResourceStorageModeManaged;
#endif
        case MTLStorageModePrivate:
            return MTLResourceStorageModePrivate;
        case MTLStorageModeMemoryless:
            return MTLResourceStorageModeMemoryless;
#if TARGET_OS_SIMULATOR
        default:
            // TODO(http://anglebug.com/42266474): Remove me once hacked SDKs are fixed.
            UNREACHABLE();
            return MTLResourceStorageModeShared;
#endif
    }
}

}  // namespace

// Resource implementation
Resource::Resource() : mUsageRef(std::make_shared<UsageRef>()) {}

// Share the GPU usage ref with other resource
Resource::Resource(Resource *other) : Resource(other->mUsageRef) {}
Resource::Resource(std::shared_ptr<UsageRef> otherUsageRef) : mUsageRef(std::move(otherUsageRef))
{
    ASSERT(mUsageRef);
}

void Resource::reset()
{
    mUsageRef->cmdBufferQueueSerial = 0;
    resetCPUReadMemDirty();
    resetCPUReadMemNeedSync();
    resetCPUReadMemSyncPending();
}

bool Resource::isBeingUsedByGPU(Context *context) const
{
    return context->cmdQueue().isResourceBeingUsedByGPU(this);
}

bool Resource::hasPendingWorks(Context *context) const
{
    return context->cmdQueue().resourceHasPendingWorks(this);
}

bool Resource::hasPendingRenderWorks(Context *context) const
{
    return context->cmdQueue().resourceHasPendingRenderWorks(this);
}

void Resource::setUsedByCommandBufferWithQueueSerial(uint64_t serial,
                                                     bool writing,
                                                     bool isRenderCommand)
{
    if (writing)
    {
        mUsageRef->cpuReadMemNeedSync = true;
        mUsageRef->cpuReadMemDirty    = true;
    }

    mUsageRef->cmdBufferQueueSerial = std::max(mUsageRef->cmdBufferQueueSerial, serial);

    if (isRenderCommand)
    {
        if (writing)
        {
            mUsageRef->lastWritingRenderEncoderSerial = mUsageRef->cmdBufferQueueSerial;
        }
        else
        {
            mUsageRef->lastReadingRenderEncoderSerial = mUsageRef->cmdBufferQueueSerial;
        }
    }
}

// Texture implemenetation
/** static */
angle::Result Texture::Make2DTexture(ContextMtl *context,
                                     const Format &format,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t mips,
                                     bool renderTargetOnly,
                                     bool allowFormatView,
                                     TextureRef *refOut)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        MTLTextureDescriptor *desc =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format.metalFormat
                                                               width:width
                                                              height:height
                                                           mipmapped:mips == 0 || mips > 1];
        return MakeTexture(context, format, desc, mips, renderTargetOnly, allowFormatView, refOut);
    }  // ANGLE_MTL_OBJC_SCOPE
}

/** static */
angle::Result Texture::MakeMemoryLess2DMSTexture(ContextMtl *context,
                                                 const Format &format,
                                                 uint32_t width,
                                                 uint32_t height,
                                                 uint32_t samples,
                                                 TextureRef *refOut)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        MTLTextureDescriptor *desc =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format.metalFormat
                                                               width:width
                                                              height:height
                                                           mipmapped:NO];
        desc.textureType = MTLTextureType2DMultisample;
        desc.sampleCount = samples;

        return MakeTexture(context, format, desc, 1, /*renderTargetOnly=*/true,
                           /*allowFormatView=*/false, /*memoryLess=*/true, refOut);
    }  // ANGLE_MTL_OBJC_SCOPE
}
/** static */
angle::Result Texture::MakeCubeTexture(ContextMtl *context,
                                       const Format &format,
                                       uint32_t size,
                                       uint32_t mips,
                                       bool renderTargetOnly,
                                       bool allowFormatView,
                                       TextureRef *refOut)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        MTLTextureDescriptor *desc =
            [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:format.metalFormat
                                                                  size:size
                                                             mipmapped:mips == 0 || mips > 1];

        return MakeTexture(context, format, desc, mips, renderTargetOnly, allowFormatView, refOut);
    }  // ANGLE_MTL_OBJC_SCOPE
}

/** static */
angle::Result Texture::Make2DMSTexture(ContextMtl *context,
                                       const Format &format,
                                       uint32_t width,
                                       uint32_t height,
                                       uint32_t samples,
                                       bool renderTargetOnly,
                                       bool allowFormatView,
                                       TextureRef *refOut)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        AutoObjCPtr<MTLTextureDescriptor *> desc = adoptObjCPtr([MTLTextureDescriptor new]);
        desc.get().textureType                   = MTLTextureType2DMultisample;
        desc.get().pixelFormat                   = format.metalFormat;
        desc.get().width                         = width;
        desc.get().height                        = height;
        desc.get().mipmapLevelCount              = 1;
        desc.get().sampleCount                   = samples;

        return MakeTexture(context, format, desc, 1, renderTargetOnly, allowFormatView, refOut);
    }  // ANGLE_MTL_OBJC_SCOPE
}

/** static */
angle::Result Texture::Make2DArrayTexture(ContextMtl *context,
                                          const Format &format,
                                          uint32_t width,
                                          uint32_t height,
                                          uint32_t mips,
                                          uint32_t arrayLength,
                                          bool renderTargetOnly,
                                          bool allowFormatView,
                                          TextureRef *refOut)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        // Use texture2DDescriptorWithPixelFormat to calculate full range mipmap range:
        MTLTextureDescriptor *desc =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format.metalFormat
                                                               width:width
                                                              height:height
                                                           mipmapped:mips == 0 || mips > 1];

        desc.textureType = MTLTextureType2DArray;
        desc.arrayLength = arrayLength;

        return MakeTexture(context, format, desc, mips, renderTargetOnly, allowFormatView, refOut);
    }  // ANGLE_MTL_OBJC_SCOPE
}

/** static */
angle::Result Texture::Make3DTexture(ContextMtl *context,
                                     const Format &format,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t depth,
                                     uint32_t mips,
                                     bool renderTargetOnly,
                                     bool allowFormatView,
                                     TextureRef *refOut)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        // Use texture2DDescriptorWithPixelFormat to calculate full range mipmap range:
        const uint32_t maxDimen = std::max({width, height, depth});
        MTLTextureDescriptor *desc =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format.metalFormat
                                                               width:maxDimen
                                                              height:maxDimen
                                                           mipmapped:mips == 0 || mips > 1];

        desc.textureType = MTLTextureType3D;
        desc.width       = width;
        desc.height      = height;
        desc.depth       = depth;

        return MakeTexture(context, format, desc, mips, renderTargetOnly, allowFormatView, refOut);
    }  // ANGLE_MTL_OBJC_SCOPE
}

/** static */
angle::Result Texture::MakeTexture(ContextMtl *context,
                                   const Format &mtlFormat,
                                   MTLTextureDescriptor *desc,
                                   uint32_t mips,
                                   bool renderTargetOnly,
                                   bool allowFormatView,
                                   TextureRef *refOut)
{
    return MakeTexture(context, mtlFormat, desc, mips, renderTargetOnly, allowFormatView, false,
                       refOut);
}

angle::Result Texture::MakeTexture(ContextMtl *context,
                                   const Format &mtlFormat,
                                   MTLTextureDescriptor *desc,
                                   uint32_t mips,
                                   bool renderTargetOnly,
                                   bool allowFormatView,
                                   bool memoryLess,
                                   TextureRef *refOut)
{
    if (desc.pixelFormat == MTLPixelFormatInvalid)
    {
        return angle::Result::Stop;
    }

    ASSERT(refOut);
    Texture *newTexture =
        new Texture(context, desc, mips, renderTargetOnly, allowFormatView, memoryLess);
    ANGLE_CHECK_GL_ALLOC(context, newTexture->valid());
    refOut->reset(newTexture);

    if (!mtlFormat.hasDepthAndStencilBits())
    {
        refOut->get()->setColorWritableMask(GetEmulatedColorWriteMask(mtlFormat));
    }

    size_t estimatedBytes = EstimateTextureSizeInBytes(
        mtlFormat, desc.width, desc.height, desc.depth, desc.sampleCount, desc.mipmapLevelCount);
    if (refOut)
    {
        refOut->get()->setEstimatedByteSize(memoryLess ? 0 : estimatedBytes);
    }

    return angle::Result::Continue;
}

angle::Result Texture::MakeTexture(ContextMtl *context,
                                   const Format &mtlFormat,
                                   MTLTextureDescriptor *desc,
                                   IOSurfaceRef surfaceRef,
                                   NSUInteger slice,
                                   bool renderTargetOnly,
                                   TextureRef *refOut)
{

    ASSERT(refOut);
    Texture *newTexture = new Texture(context, desc, surfaceRef, slice, renderTargetOnly);
    ANGLE_CHECK_GL_ALLOC(context, newTexture->valid());
    refOut->reset(newTexture);
    if (!mtlFormat.hasDepthAndStencilBits())
    {
        refOut->get()->setColorWritableMask(GetEmulatedColorWriteMask(mtlFormat));
    }

    size_t estimatedBytes = EstimateTextureSizeInBytes(
        mtlFormat, desc.width, desc.height, desc.depth, desc.sampleCount, desc.mipmapLevelCount);
    refOut->get()->setEstimatedByteSize(estimatedBytes);

    return angle::Result::Continue;
}

bool needMultisampleColorFormatShaderReadWorkaround(ContextMtl *context, MTLTextureDescriptor *desc)
{
    return desc.sampleCount > 1 &&
           context->getDisplay()
               ->getFeatures()
               .multisampleColorFormatShaderReadWorkaround.enabled &&
           context->getNativeFormatCaps(desc.pixelFormat).colorRenderable;
}

/** static */
TextureRef Texture::MakeFromMetal(id<MTLTexture> metalTexture)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        return TextureRef(new Texture(metalTexture));
    }
}

Texture::Texture(id<MTLTexture> metalTexture)
    : mColorWritableMask(std::make_shared<MTLColorWriteMask>(MTLColorWriteMaskAll))
{
    set(metalTexture);
}

Texture::Texture(std::shared_ptr<UsageRef> usageRef,
                 id<MTLTexture> metalTexture,
                 std::shared_ptr<MTLColorWriteMask> colorWriteMask)
    : Resource(std::move(usageRef)), mColorWritableMask(std::move(colorWriteMask))
{
    set(metalTexture);
}

Texture::Texture(ContextMtl *context,
                 MTLTextureDescriptor *desc,
                 uint32_t mips,
                 bool renderTargetOnly,
                 bool allowFormatView)
    : Texture(context, desc, mips, renderTargetOnly, allowFormatView, false)
{}

Texture::Texture(ContextMtl *context,
                 MTLTextureDescriptor *desc,
                 uint32_t mips,
                 bool renderTargetOnly,
                 bool allowFormatView,
                 bool memoryLess)
    : mColorWritableMask(std::make_shared<MTLColorWriteMask>(MTLColorWriteMaskAll))
{
    ANGLE_MTL_OBJC_SCOPE
    {
        const mtl::ContextDevice &metalDevice = context->getMetalDevice();

        if (mips > 1 && mips < desc.mipmapLevelCount)
        {
            desc.mipmapLevelCount = mips;
        }

        // Every texture will support being rendered for now
        desc.usage = 0;

        if (context->getNativeFormatCaps(desc.pixelFormat).isRenderable())
        {
            desc.usage |= MTLTextureUsageRenderTarget;
        }

        if (memoryLess)
        {
            if (context->getDisplay()->supportsAppleGPUFamily(1))
            {
                desc.resourceOptions = MTLResourceStorageModeMemoryless;
            }
            else
            {
                desc.resourceOptions = MTLResourceStorageModePrivate;
            }

            // Regardless of whether MTLResourceStorageModeMemoryless is used or not, we disable
            // Load/Store on this texture.
            mShouldNotLoadStore = true;
        }
        else if (context->getNativeFormatCaps(desc.pixelFormat).depthRenderable ||
                 desc.textureType == MTLTextureType2DMultisample)
        {
            // Metal doesn't support host access to depth stencil texture's data
            desc.resourceOptions = MTLResourceStorageModePrivate;
        }

        if (!renderTargetOnly || needMultisampleColorFormatShaderReadWorkaround(context, desc))
        {
            desc.usage = desc.usage | MTLTextureUsageShaderRead;
            if (context->getNativeFormatCaps(desc.pixelFormat).writable)
            {
                desc.usage = desc.usage | MTLTextureUsageShaderWrite;
            }
        }
        if (desc.pixelFormat == MTLPixelFormatDepth32Float_Stencil8)
        {
            ASSERT(allowFormatView || memoryLess);
        }
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
        if (desc.pixelFormat == MTLPixelFormatDepth24Unorm_Stencil8)
        {
            ASSERT(allowFormatView || memoryLess);
        }
#endif

        if (allowFormatView)
        {
            desc.usage = desc.usage | MTLTextureUsagePixelFormatView;
        }

        set(metalDevice.newTextureWithDescriptor(desc));
        mCreationDesc = std::move(desc);
    }
}

Texture::Texture(ContextMtl *context,
                 MTLTextureDescriptor *desc,
                 IOSurfaceRef iosurface,
                 NSUInteger plane,
                 bool renderTargetOnly)
    : mColorWritableMask(std::make_shared<MTLColorWriteMask>(MTLColorWriteMaskAll))
{
    ANGLE_MTL_OBJC_SCOPE
    {
        const mtl::ContextDevice &metalDevice = context->getMetalDevice();

        // Every texture will support being rendered for now
        desc.usage = MTLTextureUsagePixelFormatView;

        if (context->getNativeFormatCaps(desc.pixelFormat).isRenderable())
        {
            desc.usage |= MTLTextureUsageRenderTarget;
        }

#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
        desc.resourceOptions = MTLResourceStorageModeManaged;
#else
        desc.resourceOptions = MTLResourceStorageModeShared;
#endif

        if (!renderTargetOnly)
        {
            desc.usage = desc.usage | MTLTextureUsageShaderRead;
            if (context->getNativeFormatCaps(desc.pixelFormat).writable)
            {
                desc.usage = desc.usage | MTLTextureUsageShaderWrite;
            }
        }
        set(metalDevice.newTextureWithDescriptor(desc, iosurface, plane));
    }
}

Texture::Texture(Texture *original, MTLPixelFormat pixelFormat)
    : Resource(original),
      mColorWritableMask(original->mColorWritableMask)  // Share color write mask property
{
    ANGLE_MTL_OBJC_SCOPE
    {
        set(adoptObjCPtr([original->get() newTextureViewWithPixelFormat:pixelFormat]));
        // Texture views consume no additional memory
        mEstimatedByteSize = 0;
    }
}

Texture::Texture(Texture *original,
                 MTLPixelFormat pixelFormat,
                 MTLTextureType textureType,
                 NSRange levels,
                 NSRange slices)
    : Resource(original),
      mColorWritableMask(original->mColorWritableMask)  // Share color write mask property
{
    ANGLE_MTL_OBJC_SCOPE
    {
        set(adoptObjCPtr([original->get() newTextureViewWithPixelFormat:pixelFormat
                                                            textureType:textureType
                                                                 levels:levels
                                                                 slices:slices]));
        // Texture views consume no additional memory
        mEstimatedByteSize = 0;
    }
}

Texture::Texture(Texture *original,
                 MTLPixelFormat pixelFormat,
                 MTLTextureType textureType,
                 NSRange levels,
                 NSRange slices,
                 const MTLTextureSwizzleChannels &swizzle)
    : Resource(original),
      mColorWritableMask(original->mColorWritableMask)  // Share color write mask property
{
    ANGLE_MTL_OBJC_SCOPE
    {
        set(adoptObjCPtr([original->get() newTextureViewWithPixelFormat:pixelFormat
                                                            textureType:textureType
                                                                 levels:levels
                                                                 slices:slices
                                                                swizzle:swizzle]));

        // Texture views consume no additional memory
        mEstimatedByteSize = 0;
    }
}

void Texture::syncContent(ContextMtl *context, mtl::BlitCommandEncoder *blitEncoder)
{
    InvokeCPUMemSync(context, blitEncoder, this);
}

void Texture::syncContentIfNeeded(ContextMtl *context)
{
    EnsureCPUMemWillBeSynced(context, this);
}

bool Texture::isCPUAccessible() const
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    if (get().storageMode == MTLStorageModeManaged)
    {
        return true;
    }
#endif
    return get().storageMode == MTLStorageModeShared;
}

bool Texture::isShaderReadable() const
{
    return get().usage & MTLTextureUsageShaderRead;
}

bool Texture::isShaderWritable() const
{
    return get().usage & MTLTextureUsageShaderWrite;
}

bool Texture::supportFormatView() const
{
    return get().usage & MTLTextureUsagePixelFormatView;
}

void Texture::replace2DRegion(ContextMtl *context,
                              const MTLRegion &region,
                              const MipmapNativeLevel &mipmapLevel,
                              uint32_t slice,
                              const uint8_t *data,
                              size_t bytesPerRow)
{
    ASSERT(region.size.depth == 1);
    replaceRegion(context, region, mipmapLevel, slice, data, bytesPerRow, 0);
}

void Texture::replaceRegion(ContextMtl *context,
                            const MTLRegion &region,
                            const MipmapNativeLevel &mipmapLevel,
                            uint32_t slice,
                            const uint8_t *data,
                            size_t bytesPerRow,
                            size_t bytesPer2DImage)
{
    if (mipmapLevel.get() >= this->mipmapLevels())
    {
        return;
    }

    ASSERT(isCPUAccessible());

    CommandQueue &cmdQueue = context->cmdQueue();

    syncContentIfNeeded(context);

    // NOTE(hqle): what if multiple contexts on multiple threads are using this texture?
    if (this->isBeingUsedByGPU(context))
    {
        context->flushCommandBuffer(mtl::NoWait);
    }

    cmdQueue.ensureResourceReadyForCPU(this);

    if (textureType() != MTLTextureType3D)
    {
        bytesPer2DImage = 0;
    }

    [get() replaceRegion:region
             mipmapLevel:mipmapLevel.get()
                   slice:slice
               withBytes:data
             bytesPerRow:bytesPerRow
           bytesPerImage:bytesPer2DImage];
}

void Texture::getBytes(ContextMtl *context,
                       size_t bytesPerRow,
                       size_t bytesPer2DInage,
                       const MTLRegion &region,
                       const MipmapNativeLevel &mipmapLevel,
                       uint32_t slice,
                       uint8_t *dataOut)
{
    ASSERT(isCPUAccessible());

    CommandQueue &cmdQueue = context->cmdQueue();

    syncContentIfNeeded(context);

    // NOTE(hqle): what if multiple contexts on multiple threads are using this texture?
    if (this->isBeingUsedByGPU(context))
    {
        context->flushCommandBuffer(mtl::NoWait);
    }

    cmdQueue.ensureResourceReadyForCPU(this);

    [get() getBytes:dataOut
          bytesPerRow:bytesPerRow
        bytesPerImage:bytesPer2DInage
           fromRegion:region
          mipmapLevel:mipmapLevel.get()
                slice:slice];
}

TextureRef Texture::createCubeFaceView(uint32_t face)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        switch (textureType())
        {
            case MTLTextureTypeCube:
                return TextureRef(new Texture(this, pixelFormat(), MTLTextureType2D,
                                              NSMakeRange(0, mipmapLevels()),
                                              NSMakeRange(face, 1)));
            default:
                UNREACHABLE();
                return nullptr;
        }
    }
}

TextureRef Texture::createSliceMipView(uint32_t slice, const MipmapNativeLevel &level)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        switch (textureType())
        {
            case MTLTextureTypeCube:
            case MTLTextureType2D:
            case MTLTextureType2DArray:
                return TextureRef(new Texture(this, pixelFormat(), MTLTextureType2D,
                                              NSMakeRange(level.get(), 1), NSMakeRange(slice, 1)));
            default:
                UNREACHABLE();
                return nullptr;
        }
    }
}

TextureRef Texture::createMipView(const MipmapNativeLevel &level)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        NSUInteger slices = cubeFacesOrArrayLength();
        return TextureRef(new Texture(this, pixelFormat(), textureType(),
                                      NSMakeRange(level.get(), 1), NSMakeRange(0, slices)));
    }
}

TextureRef Texture::createMipsView(const MipmapNativeLevel &baseLevel, uint32_t levels)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        NSUInteger slices = cubeFacesOrArrayLength();
        return TextureRef(new Texture(this, pixelFormat(), textureType(),
                                      NSMakeRange(baseLevel.get(), levels),
                                      NSMakeRange(0, slices)));
    }
}

TextureRef Texture::createViewWithDifferentFormat(MTLPixelFormat format)
{
    ASSERT(supportFormatView());
    return TextureRef(new Texture(this, format));
}

TextureRef Texture::createShaderImageView2D(const MipmapNativeLevel &level,
                                            int layer,
                                            MTLPixelFormat format)
{
    ASSERT(isShaderReadable());
    ASSERT(isShaderWritable());
    ASSERT(format == pixelFormat() || supportFormatView());
    ASSERT(textureType() != MTLTextureType3D);
    return TextureRef(new Texture(this, format, MTLTextureType2D, NSMakeRange(level.get(), 1),
                                  NSMakeRange(layer, 1)));
}

TextureRef Texture::createViewWithCompatibleFormat(MTLPixelFormat format)
{
    return TextureRef(new Texture(this, format));
}

TextureRef Texture::createMipsSwizzleView(const MipmapNativeLevel &baseLevel,
                                          uint32_t levels,
                                          MTLPixelFormat format,
                                          const MTLTextureSwizzleChannels &swizzle)
{
    return TextureRef(new Texture(this, format, textureType(), NSMakeRange(baseLevel.get(), levels),
                                  NSMakeRange(0, cubeFacesOrArrayLength()), swizzle));
}

MTLPixelFormat Texture::pixelFormat() const
{
    return get().pixelFormat;
}

MTLTextureType Texture::textureType() const
{
    return get().textureType;
}

uint32_t Texture::mipmapLevels() const
{
    return static_cast<uint32_t>(get().mipmapLevelCount);
}

uint32_t Texture::arrayLength() const
{
    return static_cast<uint32_t>(get().arrayLength);
}

uint32_t Texture::cubeFaces() const
{
    if (textureType() == MTLTextureTypeCube)
    {
        return 6;
    }
    return 1;
}

uint32_t Texture::cubeFacesOrArrayLength() const
{
    if (textureType() == MTLTextureTypeCube)
    {
        return 6;
    }
    return arrayLength();
}

uint32_t Texture::width(const MipmapNativeLevel &level) const
{
    return static_cast<uint32_t>(GetMipSize(get().width, level));
}

uint32_t Texture::height(const MipmapNativeLevel &level) const
{
    return static_cast<uint32_t>(GetMipSize(get().height, level));
}

uint32_t Texture::depth(const MipmapNativeLevel &level) const
{
    return static_cast<uint32_t>(GetMipSize(get().depth, level));
}

gl::Extents Texture::size(const MipmapNativeLevel &level) const
{
    gl::Extents re;

    re.width  = width(level);
    re.height = height(level);
    re.depth  = depth(level);

    return re;
}

gl::Extents Texture::size(const ImageNativeIndex &index) const
{
    gl::Extents extents = size(index.getNativeLevel());

    if (index.hasLayer())
    {
        extents.depth = 1;
    }

    return extents;
}

uint32_t Texture::samples() const
{
    return static_cast<uint32_t>(get().sampleCount);
}

bool Texture::hasIOSurface() const
{
    return (get().iosurface) != nullptr;
}

bool Texture::sameTypeAndDimemsionsAs(const TextureRef &other) const
{
    return textureType() == other->textureType() && pixelFormat() == other->pixelFormat() &&
           mipmapLevels() == other->mipmapLevels() &&
           cubeFacesOrArrayLength() == other->cubeFacesOrArrayLength() &&
           widthAt0() == other->widthAt0() && heightAt0() == other->heightAt0() &&
           depthAt0() == other->depthAt0();
}

angle::Result Texture::resize(ContextMtl *context, uint32_t width, uint32_t height)
{
    // Resizing texture view is not supported.
    ASSERT(mCreationDesc);

    ANGLE_MTL_OBJC_SCOPE
    {
        AutoObjCPtr<MTLTextureDescriptor *> newDesc =
            adoptObjCPtr<MTLTextureDescriptor>([mCreationDesc.get() copy]);
        newDesc.get().width           = width;
        newDesc.get().height          = height;
        auto newTexture               = context->getMetalDevice().newTextureWithDescriptor(newDesc);
        ANGLE_CHECK_GL_ALLOC(context, newTexture);
        mCreationDesc = std::move(newDesc);
        set(std::move(newTexture));
        // Reset reference counter
        Resource::reset();
    }

    return angle::Result::Continue;
}

TextureRef Texture::getLinearColorView()
{
    if (mLinearColorView)
    {
        return mLinearColorView;
    }

    switch (pixelFormat())
    {
        case MTLPixelFormatRGBA8Unorm_sRGB:
            mLinearColorView = createViewWithCompatibleFormat(MTLPixelFormatRGBA8Unorm);
            break;
        case MTLPixelFormatBGRA8Unorm_sRGB:
            mLinearColorView = createViewWithCompatibleFormat(MTLPixelFormatBGRA8Unorm);
            break;
        default:
            // NOTE(hqle): Not all sRGB formats are supported yet.
            UNREACHABLE();
    }

    return mLinearColorView;
}

TextureRef Texture::getReadableCopy(ContextMtl *context,
                                    mtl::BlitCommandEncoder *encoder,
                                    const uint32_t levelToCopy,
                                    const uint32_t sliceToCopy,
                                    const MTLRegion &areaToCopy)
{
    gl::Extents firstLevelSize = size(kZeroNativeMipLevel);
    if (!mReadCopy || mReadCopy->get().width < static_cast<size_t>(firstLevelSize.width) ||
        mReadCopy->get().height < static_cast<size_t>(firstLevelSize.height))
    {
        // Create a texture that big enough to store the first level data and any smaller level
        ANGLE_MTL_OBJC_SCOPE
        {
            AutoObjCPtr<MTLTextureDescriptor *> desc = adoptObjCPtr([MTLTextureDescriptor new]);
            desc.get().textureType                   = get().textureType;
            desc.get().pixelFormat                   = get().pixelFormat;
            desc.get().width                         = firstLevelSize.width;
            desc.get().height                        = firstLevelSize.height;
            desc.get().depth                         = 1;
            desc.get().arrayLength                   = 1;
            desc.get().resourceOptions               = MTLResourceStorageModePrivate;
            desc.get().sampleCount                   = get().sampleCount;
            desc.get().usage = MTLTextureUsageShaderRead | MTLTextureUsagePixelFormatView;
            mReadCopy.reset(new Texture(context->getMetalDevice().newTextureWithDescriptor(desc)));
        }  // ANGLE_MTL_OBJC_SCOPE
    }

    ASSERT(encoder);

    encoder->copyTexture(shared_from_this(), sliceToCopy, mtl::MipmapNativeLevel(levelToCopy),
                         mReadCopy, 0, mtl::kZeroNativeMipLevel, 1, 1);

    return mReadCopy;
}

void Texture::releaseReadableCopy()
{
    mReadCopy = nullptr;
}

TextureRef Texture::getStencilView()
{
    if (mStencilView)
    {
        return mStencilView;
    }

    switch (pixelFormat())
    {
        case MTLPixelFormatStencil8:
        case MTLPixelFormatX32_Stencil8:
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
        case MTLPixelFormatX24_Stencil8:
#endif
            // This texture is already a stencil texture. Return its ref directly.
            return shared_from_this();
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
        case MTLPixelFormatDepth24Unorm_Stencil8:
            mStencilView = createViewWithDifferentFormat(MTLPixelFormatX24_Stencil8);
            break;
#endif
        case MTLPixelFormatDepth32Float_Stencil8:
            mStencilView = createViewWithDifferentFormat(MTLPixelFormatX32_Stencil8);
            break;
        default:
            UNREACHABLE();
    }

    return mStencilView;
}

TextureRef Texture::parentTexture()
{
    if (mParentTexture)
    {
        return mParentTexture;
    }

    if (!get().parentTexture)
    {
        // Doesn't have parent.
        return nullptr;
    }

    // Lazily construct parent's Texture object from parent's MTLTexture.
    // Note that the constructed Texture object is not the same as the same original object that
    // creates this view. However, it will share the same usageRef and MTLTexture with the
    // original Texture object. We do this to avoid cyclic reference between original Texture
    // and its view.
    //
    // For example, the original Texture object might keep a ref to its stencil view. Had we
    // kept the original object's ref in the stencil view, there would have been a cyclic
    // reference.
    //
    // This is OK because even though the Texture objects are not the same, they refer to same
    // MTLTexture and usageRef.
    mParentTexture.reset(new Texture(mUsageRef, get().parentTexture, mColorWritableMask));

    return mParentTexture;
}
MipmapNativeLevel Texture::parentRelativeLevel()
{
    return mtl::GetNativeMipLevel(static_cast<uint32_t>(get().parentRelativeLevel), 0);
}
uint32_t Texture::parentRelativeSlice()
{
    return static_cast<uint32_t>(get().parentRelativeSlice);
}

void Texture::set(id<MTLTexture> metalTexture)
{
    ParentClass::set(metalTexture);
    // Reset stencil view
    mStencilView     = nullptr;
    mLinearColorView = nullptr;

    mParentTexture = nullptr;
}

// Buffer implementation

MTLStorageMode Buffer::getStorageModeForSharedBuffer(ContextMtl *contextMtl)
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    if (ANGLE_UNLIKELY(contextMtl->getDisplay()->getFeatures().forceBufferGPUStorage.enabled))
    {
        return MTLStorageModeManaged;
    }
#endif
    return MTLStorageModeShared;
}

MTLStorageMode Buffer::getStorageModeForUsage(ContextMtl *contextMtl, Usage usage)
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    bool hasCpuAccess = false;
    switch (usage)
    {
        case Usage::StaticCopy:
        case Usage::StaticDraw:
        case Usage::StaticRead:
        case Usage::DynamicRead:
        case Usage::StreamRead:
            hasCpuAccess = true;
            break;
        default:
            break;
    }
    const auto &features = contextMtl->getDisplay()->getFeatures();
    if (hasCpuAccess)
    {
        if (features.alwaysUseManagedStorageModeForBuffers.enabled ||
            ANGLE_UNLIKELY(features.forceBufferGPUStorage.enabled))
        {
            return MTLStorageModeManaged;
        }
        return MTLStorageModeShared;
    }
    if (contextMtl->getMetalDevice().hasUnifiedMemory() ||
        features.alwaysUseSharedStorageModeForBuffers.enabled)
    {
        return MTLStorageModeShared;
    }
    return MTLStorageModeManaged;
#else
    ANGLE_UNUSED_VARIABLE(contextMtl);
    ANGLE_UNUSED_VARIABLE(usage);
    return MTLStorageModeShared;
#endif
}

angle::Result Buffer::MakeBuffer(ContextMtl *context,
                                 size_t size,
                                 const uint8_t *data,
                                 BufferRef *bufferOut)
{
    auto storageMode = getStorageModeForUsage(context, Usage::DynamicDraw);
    return MakeBufferWithStorageMode(context, storageMode, size, data, bufferOut);
}

angle::Result Buffer::MakeBufferWithStorageMode(ContextMtl *context,
                                                MTLStorageMode storageMode,
                                                size_t size,
                                                const uint8_t *data,
                                                BufferRef *bufferOut)
{
    bufferOut->reset(new Buffer(context, storageMode, size, data));
    ANGLE_CHECK_GL_ALLOC(context, *bufferOut && (*bufferOut)->get());
    return angle::Result::Continue;
}

Buffer::Buffer(ContextMtl *context, MTLStorageMode storageMode, size_t size, const uint8_t *data)
{
    (void)reset(context, storageMode, size, data);
}

angle::Result Buffer::reset(ContextMtl *context,
                            MTLStorageMode storageMode,
                            size_t size,
                            const uint8_t *data)
{
    auto options = resourceOptionsForStorageMode(storageMode);
    set([&]() -> AutoObjCPtr<id<MTLBuffer>> {
        const mtl::ContextDevice &metalDevice = context->getMetalDevice();
        if (size > [metalDevice maxBufferLength])
        {
            return nullptr;
        }
        if (data)
        {
            return metalDevice.newBufferWithBytes(data, size, options);
        }
        return metalDevice.newBufferWithLength(size, options);
    }());
    // Reset command buffer's reference serial
    Resource::reset();

    return angle::Result::Continue;
}

void Buffer::syncContent(ContextMtl *context, mtl::BlitCommandEncoder *blitEncoder)
{
    InvokeCPUMemSync(context, blitEncoder, this);
}

const uint8_t *Buffer::mapReadOnly(ContextMtl *context)
{
    return mapWithOpt(context, true, false);
}

uint8_t *Buffer::map(ContextMtl *context)
{
    return mapWithOpt(context, false, false);
}

uint8_t *Buffer::mapWithOpt(ContextMtl *context, bool readonly, bool noSync)
{
    mMapReadOnly = readonly;

    if (!noSync && (isCPUReadMemSyncPending() || isCPUReadMemNeedSync() || !readonly))
    {
        CommandQueue &cmdQueue = context->cmdQueue();

        EnsureCPUMemWillBeSynced(context, this);

        if (this->isBeingUsedByGPU(context))
        {
            context->flushCommandBuffer(mtl::NoWait);
        }

        cmdQueue.ensureResourceReadyForCPU(this);
        resetCPUReadMemSyncPending();
    }

    return reinterpret_cast<uint8_t *>([get() contents]);
}

void Buffer::unmap(ContextMtl *context)
{
    flush(context, 0, size());

    // Reset read only flag
    mMapReadOnly = true;
}

void Buffer::unmapNoFlush(ContextMtl *context)
{
    mMapReadOnly = true;
}

void Buffer::unmapAndFlushSubset(ContextMtl *context, size_t offsetWritten, size_t sizeWritten)
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    flush(context, offsetWritten, sizeWritten);
#endif
    mMapReadOnly = true;
}

void Buffer::flush(ContextMtl *context, size_t offsetWritten, size_t sizeWritten)
{
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    if (!mMapReadOnly)
    {
        if (get().storageMode == MTLStorageModeManaged)
        {
            size_t bufferSize  = size();
            size_t startOffset = std::min(offsetWritten, bufferSize);
            size_t endOffset   = std::min(offsetWritten + sizeWritten, bufferSize);
            size_t clampedSize = endOffset - startOffset;
            if (clampedSize > 0)
            {
                [get() didModifyRange:NSMakeRange(startOffset, clampedSize)];
            }
        }
    }
#endif
}

size_t Buffer::size() const
{
    return get().length;
}

MTLStorageMode Buffer::storageMode() const
{
    return get().storageMode;
}
}  // namespace mtl
}  // namespace rx
