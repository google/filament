//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureStorage.h: Defines the abstract rx::TextureStorage class.

#ifndef LIBANGLE_RENDERER_D3D_TEXTURESTORAGE_H_
#define LIBANGLE_RENDERER_D3D_TEXTURESTORAGE_H_

#include "common/debug.h"
#include "libANGLE/angletypes.h"

#include <GLES2/gl2.h>
#include <stdint.h>

namespace gl
{
class Context;
class ImageIndex;
struct Box;
struct PixelUnpackState;
}  // namespace gl

namespace angle
{
class Subject;
}  // namespace angle

namespace rx
{
class SwapChainD3D;
class RenderTargetD3D;
class ImageD3D;

// Dirty bit messages from TextureStorage
constexpr size_t kTextureStorageObserverMessageIndex = 0;

class TextureStorage : public angle::Subject
{
  public:
    TextureStorage(const std::string &label) : mKHRDebugLabel(label) {}
    ~TextureStorage() override {}

    virtual angle::Result onDestroy(const gl::Context *context);

    virtual int getTopLevel() const                   = 0;
    virtual bool isRenderTarget() const               = 0;
    virtual bool isUnorderedAccess() const            = 0;
    virtual bool isManaged() const                    = 0;
    virtual bool supportsNativeMipmapFunction() const = 0;
    virtual int getLevelCount() const                 = 0;

    virtual bool isMultiplanar(const gl::Context *context) = 0;

    virtual angle::Result findRenderTarget(const gl::Context *context,
                                           const gl::ImageIndex &index,
                                           GLsizei samples,
                                           RenderTargetD3D **outRT) const = 0;
    virtual angle::Result getRenderTarget(const gl::Context *context,
                                          const gl::ImageIndex &index,
                                          GLsizei samples,
                                          RenderTargetD3D **outRT)        = 0;
    virtual angle::Result generateMipmap(const gl::Context *context,
                                         const gl::ImageIndex &sourceIndex,
                                         const gl::ImageIndex &destIndex) = 0;

    virtual angle::Result copyToStorage(const gl::Context *context,
                                        TextureStorage *destStorage) = 0;
    virtual angle::Result setData(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  ImageD3D *image,
                                  const gl::Box *destBox,
                                  GLenum type,
                                  const gl::PixelUnpackState &unpack,
                                  const uint8_t *pixelData)          = 0;

    // This is a no-op for most implementations of TextureStorage. Some (e.g. TextureStorage11_2D)
    // might override it.
    virtual angle::Result useLevelZeroWorkaroundTexture(const gl::Context *context,
                                                        bool useLevelZeroTexture);

    virtual void invalidateTextures() {}

    // RenderToTexture methods
    virtual angle::Result releaseMultisampledTexStorageForLevel(size_t level);
    virtual angle::Result resolveTexture(const gl::Context *context);
    virtual GLsizei getRenderToTextureSamples() const;

    // Called by outer object when label has changed via KHR_debug extension
    void setLabel(const std::string &newLabel);

  protected:
    virtual void onLabelUpdate() {}

    const angle::Subject *mSubject;
    std::string mKHRDebugLabel;
};

inline angle::Result TextureStorage::onDestroy(const gl::Context *context)
{
    return angle::Result::Continue;
}

inline angle::Result TextureStorage::useLevelZeroWorkaroundTexture(const gl::Context *context,
                                                                   bool useLevelZeroTexture)
{
    return angle::Result::Continue;
}

inline angle::Result TextureStorage::releaseMultisampledTexStorageForLevel(size_t level)
{
    return angle::Result::Continue;
}

inline angle::Result TextureStorage::resolveTexture(const gl::Context *context)
{
    return angle::Result::Continue;
}

inline GLsizei TextureStorage::getRenderToTextureSamples() const
{
    return 0;
}

inline void TextureStorage::setLabel(const std::string &newLabel)
{
    mKHRDebugLabel = newLabel;
    onLabelUpdate();
}

using TexStoragePointer = angle::UniqueObjectPointer<TextureStorage, gl::Context>;

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_TEXTURESTORAGE_H_
