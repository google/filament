//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_WGPU_WGPU_HELPERS_H_
#define LIBANGLE_RENDERER_WGPU_WGPU_HELPERS_H_

#include <dawn/webgpu_cpp.h>
#include <stdint.h>
#include <algorithm>

#include "libANGLE/Error.h"
#include "libANGLE/ImageIndex.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{

class ContextWgpu;

namespace webgpu
{

// WebGPU requires copy buffers bytesPerRow to be aligned to 256.
// https://www.w3.org/TR/webgpu/#abstract-opdef-validating-gpuimagecopybuffer
static const GLuint kCopyBufferAlignment = 256;

enum class UpdateSource
{
    Clear,
    Texture,
};

struct ClearUpdate
{
    ClearValues clearValues;
    bool hasDepth;
    bool hasStencil;
};

struct SubresourceUpdate
{
    SubresourceUpdate() {}
    ~SubresourceUpdate() {}

    SubresourceUpdate(UpdateSource targetUpdateSource,
                      gl::LevelIndex newTargetLevel,
                      wgpu::ImageCopyBuffer targetBuffer)
    {
        updateSource = targetUpdateSource;
        textureData  = targetBuffer;
        targetLevel  = newTargetLevel;
    }

    SubresourceUpdate(UpdateSource targetUpdateSource,
                      gl::LevelIndex newTargetLevel,
                      ClearValues clearValues,
                      bool hasDepth,
                      bool hasStencil)
    {
        updateSource = targetUpdateSource;
        targetLevel  = newTargetLevel;
        clearData.clearValues = clearValues;
        clearData.hasDepth    = hasDepth;
        clearData.hasStencil  = hasStencil;
    }

    UpdateSource updateSource;
    ClearUpdate clearData;
    wgpu::ImageCopyBuffer textureData;

    gl::LevelIndex targetLevel;
};

wgpu::TextureDimension toWgpuTextureDimension(gl::TextureType glTextureType);

class ImageHelper
{
  public:
    ImageHelper();
    ~ImageHelper();

    angle::Result initImage(angle::FormatID intendedFormatID,
                            angle::FormatID actualFormatID,
                            wgpu::Device &device,
                            gl::LevelIndex firstAllocatedLevel,
                            wgpu::TextureDescriptor textureDescriptor);
    angle::Result initExternal(angle::FormatID intendedFormatID,
                               angle::FormatID actualFormatID,
                               wgpu::Texture externalTexture);

    angle::Result flushStagedUpdates(ContextWgpu *contextWgpu);
    angle::Result flushSingleLevelUpdates(ContextWgpu *contextWgpu,
                                          gl::LevelIndex levelGL,
                                          ClearValuesArray *deferredClears,
                                          uint32_t deferredClearIndex);

    wgpu::TextureDescriptor createTextureDescriptor(wgpu::TextureUsage usage,
                                                    wgpu::TextureDimension dimension,
                                                    wgpu::Extent3D size,
                                                    wgpu::TextureFormat format,
                                                    std::uint32_t mipLevelCount,
                                                    std::uint32_t sampleCount);

    angle::Result stageTextureUpload(ContextWgpu *contextWgpu,
                                     const webgpu::Format &webgpuFormat,
                                     GLenum type,
                                     const gl::Extents &glExtents,
                                     GLuint inputRowPitch,
                                     GLuint inputDepthPitch,
                                     uint32_t outputRowPitch,
                                     uint32_t outputDepthPitch,
                                     uint32_t allocationSize,
                                     const gl::ImageIndex &index,
                                     const uint8_t *pixels);

    void stageClear(gl::LevelIndex targetLevel,
                    ClearValues clearValues,
                    bool hasDepth,
                    bool hasStencil);

    void removeStagedUpdates(gl::LevelIndex levelToRemove);

    void resetImage();

    static angle::Result getReadPixelsParams(rx::ContextWgpu *contextWgpu,
                                             const gl::PixelPackState &packState,
                                             gl::Buffer *packBuffer,
                                             GLenum format,
                                             GLenum type,
                                             const gl::Rectangle &area,
                                             const gl::Rectangle &clippedArea,
                                             rx::PackPixelsParams *paramsOut,
                                             GLuint *skipBytesOut);

    angle::Result readPixels(rx::ContextWgpu *contextWgpu,
                             const gl::Rectangle &area,
                             const rx::PackPixelsParams &packPixelsParams,
                             void *pixels);

    angle::Result createTextureView(gl::LevelIndex targetLevel,
                                    uint32_t layerIndex,
                                    wgpu::TextureView &textureViewOut);
    LevelIndex toWgpuLevel(gl::LevelIndex levelIndexGl) const;
    gl::LevelIndex toGlLevel(LevelIndex levelIndexWgpu) const;
    bool isTextureLevelInAllocatedImage(gl::LevelIndex textureLevel);
    wgpu::Texture &getTexture() { return mTexture; }
    wgpu::TextureFormat toWgpuTextureFormat() const { return mTextureDescriptor.format; }
    angle::FormatID getIntendedFormatID() { return mIntendedFormatID; }
    angle::FormatID getActualFormatID() { return mActualFormatID; }
    const wgpu::TextureDescriptor &getTextureDescriptor() const { return mTextureDescriptor; }
    gl::LevelIndex getFirstAllocatedLevel() { return mFirstAllocatedLevel; }
    gl::LevelIndex getLastAllocatedLevel();
    uint32_t getLevelCount() { return mTextureDescriptor.mipLevelCount; }
    wgpu::Extent3D getSize() { return mTextureDescriptor.size; }
    bool isInitialized() { return mInitialized; }

  private:
    void appendSubresourceUpdate(gl::LevelIndex level, SubresourceUpdate &&update);
    std::vector<SubresourceUpdate> *getLevelUpdates(gl::LevelIndex level);

    wgpu::Texture mTexture;
    wgpu::TextureDescriptor mTextureDescriptor = {};
    bool mInitialized                          = false;

    gl::LevelIndex mFirstAllocatedLevel = gl::LevelIndex(0);
    angle::FormatID mIntendedFormatID;
    angle::FormatID mActualFormatID;

    std::vector<std::vector<SubresourceUpdate>> mSubresourceQueue;
};
struct BufferMapState
{
    wgpu::MapMode mode;
    size_t offset;
    size_t size;
};

enum class MapAtCreation
{
    No,
    Yes,
};

class BufferHelper : public angle::NonCopyable
{
  public:
    BufferHelper();
    ~BufferHelper();

    bool valid() const { return mBuffer.operator bool(); }
    void reset();

    angle::Result initBuffer(wgpu::Device device,
                             size_t size,
                             wgpu::BufferUsage usage,
                             MapAtCreation mappedAtCreation);

    angle::Result mapImmediate(ContextWgpu *context,
                               wgpu::MapMode mode,
                               size_t offset,
                               size_t size);
    angle::Result unmap();

    uint8_t *getMapWritePointer(size_t offset, size_t size) const;
    const uint8_t *getMapReadPointer(size_t offset, size_t size) const;

    const std::optional<BufferMapState> &getMappedState() const;

    bool canMapForRead() const;
    bool canMapForWrite() const;

    wgpu::Buffer &getBuffer();
    uint64_t requestedSize() const;
    uint64_t actualSize() const;

  private:
    wgpu::Buffer mBuffer;
    size_t mRequestedSize = 0;

    std::optional<BufferMapState> mMappedState;
};

}  // namespace webgpu
}  // namespace rx
#endif  // LIBANGLE_RENDERER_WGPU_WGPU_HELPERS_H_
