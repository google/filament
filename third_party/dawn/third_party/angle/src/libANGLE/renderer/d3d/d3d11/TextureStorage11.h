//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureStorage11.h: Defines the abstract rx::TextureStorage11 class and its concrete derived
// classes TextureStorage11_2D and TextureStorage11_Cube, which act as the interface to the D3D11
// texture.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_TEXTURESTORAGE11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_TEXTURESTORAGE11_H_

#include "libANGLE/Error.h"
#include "libANGLE/Texture.h"
#include "libANGLE/renderer/d3d/TextureStorage.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"

#include <array>
#include <map>

namespace gl
{
class ImageIndex;
}  // namespace gl

namespace rx
{
class EGLImageD3D;
class RenderTargetD3D;
class RenderTarget11;
class Renderer11;
class SwapChain11;
class Image11;
struct Renderer11DeviceCaps;
class TextureStorage11_2DMultisample;

template <typename T>
using CubeFaceArray = std::array<T, gl::kCubeFaceCount>;

struct MultisampledRenderToTextureInfo
{
    MultisampledRenderToTextureInfo(const GLsizei samples,
                                    const gl::ImageIndex &indexSS,
                                    const gl::ImageIndex &indexMS);
    ~MultisampledRenderToTextureInfo();

    // How many samples the multisampled texture contains
    GLsizei samples;
    // This is the image index for the single sampled texture
    // This will hold the relevant level information
    gl::ImageIndex indexSS;
    // This is the image index for the multisampled texture
    // For multisampled indexes, there is no level Index since they should
    // account for the entire level.
    gl::ImageIndex indexMS;
    // True when multisampled texture has been written to and needs to be
    // resolved to the single sampled texture
    bool msTextureNeedsResolve;
    std::unique_ptr<TextureStorage11_2DMultisample> msTex;
};

class TextureStorage11 : public TextureStorage
{
  public:
    ~TextureStorage11() override;

    static DWORD GetTextureBindFlags(GLenum internalFormat,
                                     const Renderer11DeviceCaps &renderer11DeviceCaps,
                                     BindFlags flags);
    static DWORD GetTextureMiscFlags(GLenum internalFormat,
                                     const Renderer11DeviceCaps &renderer11DeviceCaps,
                                     BindFlags flags,
                                     int levels);

    UINT getBindFlags() const;
    UINT getMiscFlags() const;
    const d3d11::Format &getFormatSet() const;
    angle::Result getSRVLevels(const gl::Context *context,
                               GLint baseLevel,
                               GLint maxLevel,
                               bool forceLinearSampler,
                               const d3d11::SharedSRV **outSRV);
    angle::Result generateSwizzles(const gl::Context *context,
                                   const gl::TextureState &textureState);
    void markLevelDirty(int mipLevel);
    void markDirty();

    angle::Result updateSubresourceLevel(const gl::Context *context,
                                         const TextureHelper11 &texture,
                                         unsigned int sourceSubresource,
                                         const gl::ImageIndex &index,
                                         const gl::Box &copyArea);

    angle::Result copySubresourceLevel(const gl::Context *context,
                                       const TextureHelper11 &dstTexture,
                                       unsigned int dstSubresource,
                                       const gl::ImageIndex &index,
                                       const gl::Box &region);

    // TextureStorage virtual functions
    int getTopLevel() const override;
    bool isRenderTarget() const override;
    bool isManaged() const override;
    bool supportsNativeMipmapFunction() const override;
    int getLevelCount() const override;
    bool isUnorderedAccess() const override { return mBindFlags & D3D11_BIND_UNORDERED_ACCESS; }
    bool isMultiplanar(const gl::Context *context) override;
    bool requiresTypelessTextureFormat() const;
    angle::Result generateMipmap(const gl::Context *context,
                                 const gl::ImageIndex &sourceIndex,
                                 const gl::ImageIndex &destIndex) override;
    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;
    angle::Result setData(const gl::Context *context,
                          const gl::ImageIndex &index,
                          ImageD3D *image,
                          const gl::Box *destBox,
                          GLenum type,
                          const gl::PixelUnpackState &unpack,
                          const uint8_t *pixelData) override;
    void invalidateTextures() override;

    virtual angle::Result getSRVForSampler(const gl::Context *context,
                                           const gl::TextureState &textureState,
                                           const gl::SamplerState &sampler,
                                           const d3d11::SharedSRV **outSRV);
    angle::Result getSRVForImage(const gl::Context *context,
                                 const gl::ImageUnit &imageUnit,
                                 const d3d11::SharedSRV **outSRV);
    angle::Result getUAVForImage(const gl::Context *context,
                                 const gl::ImageUnit &imageUnit,
                                 const d3d11::SharedUAV **outUAV);
    virtual angle::Result getSubresourceIndex(const gl::Context *context,
                                              const gl::ImageIndex &index,
                                              UINT *outSubresourceIndex) const;
    virtual angle::Result getResource(const gl::Context *context,
                                      const TextureHelper11 **outResource)              = 0;
    virtual void associateImage(Image11 *image, const gl::ImageIndex &index)            = 0;
    virtual void disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage) = 0;
    virtual void verifyAssociatedImageValid(const gl::ImageIndex &index,
                                            Image11 *expectedImage)                     = 0;
    virtual angle::Result releaseAssociatedImage(const gl::Context *context,
                                                 const gl::ImageIndex &index,
                                                 Image11 *incomingImage)                = 0;

    GLsizei getRenderToTextureSamples() const override;

  protected:
    TextureStorage11(Renderer11 *renderer,
                     UINT bindFlags,
                     UINT miscFlags,
                     GLenum internalFormat,
                     const std::string &label);
    int getLevelWidth(int mipLevel) const;
    int getLevelHeight(int mipLevel) const;
    int getLevelDepth(int mipLevel) const;

    // Some classes (e.g. TextureStorage11_2D) will override getMippedResource.
    virtual angle::Result getMippedResource(const gl::Context *context,
                                            const TextureHelper11 **outResource);

    virtual angle::Result getSwizzleTexture(const gl::Context *context,
                                            const TextureHelper11 **outTexture)          = 0;
    virtual angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                                 int mipLevel,
                                                 const d3d11::RenderTargetView **outRTV) = 0;

    enum class SRVType
    {
        Sample,
        Blit,
        Stencil
    };
    angle::Result getSRVLevel(const gl::Context *context,
                              int mipLevel,
                              SRVType srvType,
                              const d3d11::SharedSRV **outSRV);

    // Get a version of a depth texture with only depth information, not stencil.
    enum DropStencil
    {
        CREATED,
        ALREADY_EXISTS
    };
    virtual angle::Result ensureDropStencilTexture(const gl::Context *context,
                                                   DropStencil *dropStencilOut);
    angle::Result initDropStencilTexture(const gl::Context *context,
                                         const gl::ImageIndexIterator &it);

    // The baseLevel parameter should *not* have mTopLevel applied.
    virtual angle::Result createSRVForSampler(const gl::Context *context,
                                              int baseLevel,
                                              int mipLevels,
                                              DXGI_FORMAT format,
                                              const TextureHelper11 &texture,
                                              d3d11::SharedSRV *outSRV) = 0;
    virtual angle::Result createSRVForImage(const gl::Context *context,
                                            int level,
                                            DXGI_FORMAT format,
                                            const TextureHelper11 &texture,
                                            d3d11::SharedSRV *outSRV)   = 0;
    virtual angle::Result createUAVForImage(const gl::Context *context,
                                            int level,
                                            DXGI_FORMAT format,
                                            const TextureHelper11 &texture,
                                            d3d11::SharedUAV *outUAV)   = 0;

    void verifySwizzleExists(const gl::SwizzleState &swizzleState);

    // Clear all cached non-swizzle SRVs and invalidate the swizzle cache.
    void clearSRVCache();

    // Helper for resolving MS shadowed texture
    angle::Result resolveTextureHelper(const gl::Context *context, const TextureHelper11 &texture);
    angle::Result releaseMultisampledTexStorageForLevel(size_t level) override;
    angle::Result findMultisampledRenderTarget(const gl::Context *context,
                                               const gl::ImageIndex &index,
                                               GLsizei samples,
                                               RenderTargetD3D **outRT) const;
    angle::Result getMultisampledRenderTarget(const gl::Context *context,
                                              const gl::ImageIndex &index,
                                              GLsizei samples,
                                              RenderTargetD3D **outRT);

    Renderer11 *mRenderer;
    int mTopLevel;
    unsigned int mMipLevels;

    const d3d11::Format &mFormatInfo;
    unsigned int mTextureWidth;
    unsigned int mTextureHeight;
    unsigned int mTextureDepth;

    gl::TexLevelArray<gl::SwizzleState> mSwizzleCache;
    TextureHelper11 mDropStencilTexture;

    std::unique_ptr<MultisampledRenderToTextureInfo> mMSTexInfo;

  private:
    const UINT mBindFlags;
    const UINT mMiscFlags;

    struct SamplerKey
    {
        SamplerKey();
        SamplerKey(int baseLevel,
                   int mipLevels,
                   bool swizzle,
                   bool dropStencil,
                   bool forceLinearSampler);

        bool operator<(const SamplerKey &rhs) const;

        int baseLevel;
        int mipLevels;
        bool swizzle;
        bool dropStencil;
        bool forceLinearSampler;
    };

    angle::Result getCachedOrCreateSRVForSampler(const gl::Context *context,
                                                 const SamplerKey &key,
                                                 const d3d11::SharedSRV **outSRV);

    using SRVCacheForSampler = std::map<SamplerKey, d3d11::SharedSRV>;
    SRVCacheForSampler mSrvCacheForSampler;

    struct ImageKey
    {
        ImageKey();
        ImageKey(int level, bool layered, int layer, GLenum access, GLenum format);
        bool operator<(const ImageKey &rhs) const;
        int level;
        bool layered;
        int layer;
        GLenum access;
        GLenum format;
    };

    angle::Result getCachedOrCreateSRVForImage(const gl::Context *context,
                                               const ImageKey &key,
                                               const d3d11::SharedSRV **outSRV);
    angle::Result getCachedOrCreateUAVForImage(const gl::Context *context,
                                               const ImageKey &key,
                                               const d3d11::SharedUAV **outUAV);

    using SRVCacheForImage = std::map<ImageKey, d3d11::SharedSRV>;
    SRVCacheForImage mSrvCacheForImage;
    using UAVCacheForImage = std::map<ImageKey, d3d11::SharedUAV>;
    UAVCacheForImage mUavCacheForImage;

    gl::TexLevelArray<d3d11::SharedSRV> mLevelSRVs;
    gl::TexLevelArray<d3d11::SharedSRV> mLevelBlitSRVs;
    gl::TexLevelArray<d3d11::SharedSRV> mLevelStencilSRVs;
};

class TextureStorage11_2D : public TextureStorage11
{
  public:
    TextureStorage11_2D(Renderer11 *renderer, SwapChain11 *swapchain, const std::string &label);
    TextureStorage11_2D(Renderer11 *renderer,
                        GLenum internalformat,
                        BindFlags bindFlags,
                        GLsizei width,
                        GLsizei height,
                        int levels,
                        const std::string &label,
                        bool hintLevelZeroOnly = false);
    ~TextureStorage11_2D() override;

    angle::Result onDestroy(const gl::Context *context) override;

    angle::Result getResource(const gl::Context *context,
                              const TextureHelper11 **outResource) override;
    angle::Result getMippedResource(const gl::Context *context,
                                    const TextureHelper11 **outResource) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;

    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;

    void associateImage(Image11 *image, const gl::ImageIndex &index) override;
    void disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage) override;
    void verifyAssociatedImageValid(const gl::ImageIndex &index, Image11 *expectedImage) override;
    angle::Result releaseAssociatedImage(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         Image11 *incomingImage) override;

    angle::Result useLevelZeroWorkaroundTexture(const gl::Context *context,
                                                bool useLevelZeroTexture) override;
    void onLabelUpdate() override;

  protected:
    angle::Result getSwizzleTexture(const gl::Context *context,
                                    const TextureHelper11 **outTexture) override;
    angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                         int mipLevel,
                                         const d3d11::RenderTargetView **outRTV) override;

    angle::Result ensureDropStencilTexture(const gl::Context *context,
                                           DropStencil *dropStencilOut) override;

    angle::Result ensureTextureExists(const gl::Context *context, int mipLevels);

    angle::Result resolveTexture(const gl::Context *context) override;

  private:
    angle::Result createSRVForSampler(const gl::Context *context,
                                      int baseLevel,
                                      int mipLevels,
                                      DXGI_FORMAT format,
                                      const TextureHelper11 &texture,
                                      d3d11::SharedSRV *outSRV) override;
    angle::Result createSRVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedSRV *outSRV) override;
    angle::Result createUAVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedUAV *outUAV) override;

    TextureHelper11 mTexture;
    gl::TexLevelArray<std::unique_ptr<RenderTarget11>> mRenderTarget;
    bool mHasKeyedMutex;

    // These are members related to the zero max-LOD workaround.
    // D3D11 Feature Level 9_3 can't disable mipmaps on a mipmapped texture (i.e. solely sample from
    // level zero). These members are used to work around this limitation. Usually only mTexture XOR
    // mLevelZeroTexture will exist. For example, if an app creates a texture with only one level,
    // then 9_3 will only create mLevelZeroTexture. However, in some scenarios, both textures have
    // to be created. This incurs additional memory overhead. One example of this is an application
    // that creates a texture, calls glGenerateMipmap, and then disables mipmaps on the texture. A
    // more likely example is an app that creates an empty texture, renders to it, and then calls
    // glGenerateMipmap
    // TODO: In this rendering scenario, release the mLevelZeroTexture after mTexture has been
    // created to save memory.
    TextureHelper11 mLevelZeroTexture;
    std::unique_ptr<RenderTarget11> mLevelZeroRenderTarget;
    bool mUseLevelZeroTexture;

    // Swizzle-related variables
    TextureHelper11 mSwizzleTexture;
    gl::TexLevelArray<d3d11::RenderTargetView> mSwizzleRenderTargets;

    gl::TexLevelArray<Image11 *> mAssociatedImages;
};

class TextureStorage11_External : public TextureStorage11
{
  public:
    TextureStorage11_External(Renderer11 *renderer,
                              egl::Stream *stream,
                              const egl::Stream::GLTextureDescription &glDesc,
                              const std::string &label);
    ~TextureStorage11_External() override;

    angle::Result onDestroy(const gl::Context *context) override;

    angle::Result getResource(const gl::Context *context,
                              const TextureHelper11 **outResource) override;
    angle::Result getMippedResource(const gl::Context *context,
                                    const TextureHelper11 **outResource) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;

    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;

    void associateImage(Image11 *image, const gl::ImageIndex &index) override;
    void disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage) override;
    void verifyAssociatedImageValid(const gl::ImageIndex &index, Image11 *expectedImage) override;
    angle::Result releaseAssociatedImage(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         Image11 *incomingImage) override;
    void onLabelUpdate() override;

  protected:
    angle::Result getSwizzleTexture(const gl::Context *context,
                                    const TextureHelper11 **outTexture) override;
    angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                         int mipLevel,
                                         const d3d11::RenderTargetView **outRTV) override;

  private:
    angle::Result createSRVForSampler(const gl::Context *context,
                                      int baseLevel,
                                      int mipLevels,
                                      DXGI_FORMAT format,
                                      const TextureHelper11 &texture,
                                      d3d11::SharedSRV *outSRV) override;
    angle::Result createSRVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedSRV *outSRV) override;
    angle::Result createUAVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedUAV *outUAV) override;

    TextureHelper11 mTexture;
    int mSubresourceIndex;
    bool mHasKeyedMutex;

    Image11 *mAssociatedImage;
};

// A base class for texture storage classes where the associated images are not changed, nor are
// they accessible as images in GLES3.1+ shaders.
class TextureStorage11ImmutableBase : public TextureStorage11
{
  public:
    TextureStorage11ImmutableBase(Renderer11 *renderer,
                                  UINT bindFlags,
                                  UINT miscFlags,
                                  GLenum internalFormat,
                                  const std::string &label);

    void associateImage(Image11 *image, const gl::ImageIndex &index) override;
    void disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage) override;
    void verifyAssociatedImageValid(const gl::ImageIndex &index, Image11 *expectedImage) override;
    angle::Result releaseAssociatedImage(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         Image11 *incomingImage) override;

    angle::Result createSRVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedSRV *outSRV) override;
    angle::Result createUAVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedUAV *outUAV) override;
};

class TextureStorage11_EGLImage final : public TextureStorage11ImmutableBase
{
  public:
    TextureStorage11_EGLImage(Renderer11 *renderer,
                              EGLImageD3D *eglImage,
                              RenderTarget11 *renderTarget11,
                              const std::string &label);
    ~TextureStorage11_EGLImage() override;

    angle::Result onDestroy(const gl::Context *context) override;

    angle::Result getSubresourceIndex(const gl::Context *context,
                                      const gl::ImageIndex &index,
                                      UINT *outSubresourceIndex) const override;
    angle::Result getResource(const gl::Context *context,
                              const TextureHelper11 **outResource) override;
    angle::Result getSRVForSampler(const gl::Context *context,
                                   const gl::TextureState &textureState,
                                   const gl::SamplerState &sampler,
                                   const d3d11::SharedSRV **outSRV) override;
    angle::Result getMippedResource(const gl::Context *context,
                                    const TextureHelper11 **outResource) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;

    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;

    angle::Result useLevelZeroWorkaroundTexture(const gl::Context *context,
                                                bool useLevelZeroTexture) override;
    void onLabelUpdate() override;

    void associateImage(Image11 *image, const gl::ImageIndex &index) override;
    void disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage) override;
    void verifyAssociatedImageValid(const gl::ImageIndex &index, Image11 *expectedImage) override;
    angle::Result releaseAssociatedImage(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         Image11 *incomingImage) override;

  protected:
    angle::Result getSwizzleTexture(const gl::Context *context,
                                    const TextureHelper11 **outTexture) override;
    angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                         int mipLevel,
                                         const d3d11::RenderTargetView **outRTV) override;

  private:
    // Check if the EGL image's render target has been updated due to orphaning and delete
    // any SRVs and other resources based on the image's old render target.
    angle::Result checkForUpdatedRenderTarget(const gl::Context *context);

    angle::Result createSRVForSampler(const gl::Context *context,
                                      int baseLevel,
                                      int mipLevels,
                                      DXGI_FORMAT format,
                                      const TextureHelper11 &texture,
                                      d3d11::SharedSRV *outSRV) override;

    angle::Result getImageRenderTarget(const gl::Context *context, RenderTarget11 **outRT) const;

    EGLImageD3D *mImage;
    uintptr_t mCurrentRenderTarget;

    // Swizzle-related variables
    TextureHelper11 mSwizzleTexture;
    std::vector<d3d11::RenderTargetView> mSwizzleRenderTargets;

    Image11 *mAssociatedImage;
};

class TextureStorage11_Cube : public TextureStorage11
{
  public:
    TextureStorage11_Cube(Renderer11 *renderer,
                          GLenum internalformat,
                          BindFlags bindFlags,
                          int size,
                          int levels,
                          bool hintLevelZeroOnly,
                          const std::string &label);
    ~TextureStorage11_Cube() override;

    angle::Result onDestroy(const gl::Context *context) override;

    angle::Result getSubresourceIndex(const gl::Context *context,
                                      const gl::ImageIndex &index,
                                      UINT *outSubresourceIndex) const override;

    angle::Result getResource(const gl::Context *context,
                              const TextureHelper11 **outResource) override;
    angle::Result getMippedResource(const gl::Context *context,
                                    const TextureHelper11 **outResource) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;

    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;

    void associateImage(Image11 *image, const gl::ImageIndex &index) override;
    void disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage) override;
    void verifyAssociatedImageValid(const gl::ImageIndex &index, Image11 *expectedImage) override;
    angle::Result releaseAssociatedImage(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         Image11 *incomingImage) override;

    angle::Result useLevelZeroWorkaroundTexture(const gl::Context *context,
                                                bool useLevelZeroTexture) override;
    void onLabelUpdate() override;

  protected:
    angle::Result getSwizzleTexture(const gl::Context *context,
                                    const TextureHelper11 **outTexture) override;
    angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                         int mipLevel,
                                         const d3d11::RenderTargetView **outRTV) override;

    angle::Result ensureDropStencilTexture(const gl::Context *context,
                                           DropStencil *dropStencilOut) override;

    angle::Result ensureTextureExists(const gl::Context *context, int mipLevels);

    angle::Result resolveTexture(const gl::Context *context) override;

  private:
    angle::Result createSRVForSampler(const gl::Context *context,
                                      int baseLevel,
                                      int mipLevels,
                                      DXGI_FORMAT format,
                                      const TextureHelper11 &texture,
                                      d3d11::SharedSRV *outSRV) override;
    angle::Result createSRVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedSRV *outSRV) override;
    angle::Result createUAVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedUAV *outUAV) override;
    angle::Result createRenderTargetSRV(const gl::Context *context,
                                        const TextureHelper11 &texture,
                                        const gl::ImageIndex &index,
                                        DXGI_FORMAT resourceFormat,
                                        d3d11::SharedSRV *srv) const;

    TextureHelper11 mTexture;
    CubeFaceArray<gl::TexLevelArray<std::unique_ptr<RenderTarget11>>> mRenderTarget;

    // Level-zero workaround members. See TextureStorage11_2D's workaround members for a
    // description.
    TextureHelper11 mLevelZeroTexture;
    CubeFaceArray<std::unique_ptr<RenderTarget11>> mLevelZeroRenderTarget;
    bool mUseLevelZeroTexture;

    TextureHelper11 mSwizzleTexture;
    gl::TexLevelArray<d3d11::RenderTargetView> mSwizzleRenderTargets;

    CubeFaceArray<gl::TexLevelArray<Image11 *>> mAssociatedImages;
};

class TextureStorage11_3D : public TextureStorage11
{
  public:
    TextureStorage11_3D(Renderer11 *renderer,
                        GLenum internalformat,
                        BindFlags bindFlags,
                        GLsizei width,
                        GLsizei height,
                        GLsizei depth,
                        int levels,
                        const std::string &label);
    ~TextureStorage11_3D() override;

    angle::Result onDestroy(const gl::Context *context) override;

    angle::Result getResource(const gl::Context *context,
                              const TextureHelper11 **outResource) override;

    // Handles both layer and non-layer RTs
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;

    void associateImage(Image11 *image, const gl::ImageIndex &index) override;
    void disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage) override;
    void verifyAssociatedImageValid(const gl::ImageIndex &index, Image11 *expectedImage) override;
    angle::Result releaseAssociatedImage(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         Image11 *incomingImage) override;
    void onLabelUpdate() override;

  protected:
    angle::Result getSwizzleTexture(const gl::Context *context,
                                    const TextureHelper11 **outTexture) override;
    angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                         int mipLevel,
                                         const d3d11::RenderTargetView **outRTV) override;

  private:
    angle::Result createSRVForSampler(const gl::Context *context,
                                      int baseLevel,
                                      int mipLevels,
                                      DXGI_FORMAT format,
                                      const TextureHelper11 &texture,
                                      d3d11::SharedSRV *outSRV) override;
    angle::Result createSRVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedSRV *outSRV) override;
    angle::Result createUAVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedUAV *outUAV) override;

    typedef std::pair<int, int> LevelLayerKey;
    std::map<LevelLayerKey, std::unique_ptr<RenderTarget11>> mLevelLayerRenderTargets;

    gl::TexLevelArray<std::unique_ptr<RenderTarget11>> mLevelRenderTargets;

    TextureHelper11 mTexture;
    TextureHelper11 mSwizzleTexture;
    gl::TexLevelArray<d3d11::RenderTargetView> mSwizzleRenderTargets;

    gl::TexLevelArray<Image11 *> mAssociatedImages;
};

class TextureStorage11_2DArray : public TextureStorage11
{
  public:
    TextureStorage11_2DArray(Renderer11 *renderer,
                             GLenum internalformat,
                             BindFlags bindFlags,
                             GLsizei width,
                             GLsizei height,
                             GLsizei depth,
                             int levels,
                             const std::string &label);
    ~TextureStorage11_2DArray() override;

    angle::Result onDestroy(const gl::Context *context) override;

    angle::Result getResource(const gl::Context *context,
                              const TextureHelper11 **outResource) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;

    void associateImage(Image11 *image, const gl::ImageIndex &index) override;
    void disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage) override;
    void verifyAssociatedImageValid(const gl::ImageIndex &index, Image11 *expectedImage) override;
    angle::Result releaseAssociatedImage(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         Image11 *incomingImage) override;
    void onLabelUpdate() override;

    struct LevelLayerRangeKey
    {
        LevelLayerRangeKey(int mipLevelIn, int layerIn, int numLayersIn)
            : mipLevel(mipLevelIn), layer(layerIn), numLayers(numLayersIn)
        {}
        bool operator<(const LevelLayerRangeKey &other) const
        {
            if (mipLevel != other.mipLevel)
            {
                return mipLevel < other.mipLevel;
            }
            if (layer != other.layer)
            {
                return layer < other.layer;
            }
            return numLayers < other.numLayers;
        }
        int mipLevel;
        int layer;
        int numLayers;
    };

  protected:
    angle::Result getSwizzleTexture(const gl::Context *context,
                                    const TextureHelper11 **outTexture) override;
    angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                         int mipLevel,
                                         const d3d11::RenderTargetView **outRTV) override;

    angle::Result ensureDropStencilTexture(const gl::Context *context,
                                           DropStencil *dropStencilOut) override;

  private:
    angle::Result createSRVForSampler(const gl::Context *context,
                                      int baseLevel,
                                      int mipLevels,
                                      DXGI_FORMAT format,
                                      const TextureHelper11 &texture,
                                      d3d11::SharedSRV *outSRV) override;
    angle::Result createSRVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedSRV *outSRV) override;
    angle::Result createUAVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedUAV *outUAV) override;
    angle::Result createRenderTargetSRV(const gl::Context *context,
                                        const TextureHelper11 &texture,
                                        const gl::ImageIndex &index,
                                        DXGI_FORMAT resourceFormat,
                                        d3d11::SharedSRV *srv) const;

    std::map<LevelLayerRangeKey, std::unique_ptr<RenderTarget11>> mRenderTargets;

    TextureHelper11 mTexture;

    TextureHelper11 mSwizzleTexture;
    gl::TexLevelArray<d3d11::RenderTargetView> mSwizzleRenderTargets;

    typedef std::map<LevelLayerRangeKey, Image11 *> ImageMap;
    ImageMap mAssociatedImages;
};

class TextureStorage11_2DMultisample final : public TextureStorage11ImmutableBase
{
  public:
    TextureStorage11_2DMultisample(Renderer11 *renderer,
                                   GLenum internalformat,
                                   GLsizei width,
                                   GLsizei height,
                                   int levels,
                                   int samples,
                                   bool fixedSampleLocations,
                                   const std::string &label);
    ~TextureStorage11_2DMultisample() override;

    angle::Result onDestroy(const gl::Context *context) override;

    angle::Result getResource(const gl::Context *context,
                              const TextureHelper11 **outResource) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;

    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;
    void onLabelUpdate() override;

  protected:
    angle::Result getSwizzleTexture(const gl::Context *context,
                                    const TextureHelper11 **outTexture) override;
    angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                         int mipLevel,
                                         const d3d11::RenderTargetView **outRTV) override;

    angle::Result ensureDropStencilTexture(const gl::Context *context,
                                           DropStencil *dropStencilOut) override;

    angle::Result ensureTextureExists(const gl::Context *context, int mipLevels);

  private:
    angle::Result createSRVForSampler(const gl::Context *context,
                                      int baseLevel,
                                      int mipLevels,
                                      DXGI_FORMAT format,
                                      const TextureHelper11 &texture,
                                      d3d11::SharedSRV *outSRV) override;

    TextureHelper11 mTexture;
    std::unique_ptr<RenderTarget11> mRenderTarget;

    unsigned int mSamples;
    GLboolean mFixedSampleLocations;
};

class TextureStorage11_2DMultisampleArray final : public TextureStorage11ImmutableBase
{
  public:
    TextureStorage11_2DMultisampleArray(Renderer11 *renderer,
                                        GLenum internalformat,
                                        GLsizei width,
                                        GLsizei height,
                                        GLsizei depth,
                                        int levels,
                                        int samples,
                                        bool fixedSampleLocations,
                                        const std::string &label);
    ~TextureStorage11_2DMultisampleArray() override;

    angle::Result onDestroy(const gl::Context *context) override;

    angle::Result getResource(const gl::Context *context,
                              const TextureHelper11 **outResource) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;

    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;
    void onLabelUpdate() override;

  protected:
    angle::Result getSwizzleTexture(const gl::Context *context,
                                    const TextureHelper11 **outTexture) override;
    angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                         int mipLevel,
                                         const d3d11::RenderTargetView **outRTV) override;

    angle::Result ensureDropStencilTexture(const gl::Context *context,
                                           DropStencil *dropStencilOut) override;

    angle::Result ensureTextureExists(const gl::Context *context, int mipLevels);

  private:
    angle::Result createRenderTargetSRV(const gl::Context *context,
                                        const TextureHelper11 &texture,
                                        const gl::ImageIndex &index,
                                        DXGI_FORMAT resourceFormat,
                                        d3d11::SharedSRV *srv) const;

    angle::Result createSRVForSampler(const gl::Context *context,
                                      int baseLevel,
                                      int mipLevels,
                                      DXGI_FORMAT format,
                                      const TextureHelper11 &texture,
                                      d3d11::SharedSRV *outSRV) override;

    TextureHelper11 mTexture;
    std::map<TextureStorage11_2DArray::LevelLayerRangeKey, std::unique_ptr<RenderTarget11>>
        mRenderTargets;

    unsigned int mSamples;
    GLboolean mFixedSampleLocations;
};

class TextureStorage11_Buffer : public TextureStorage11
{
  public:
    TextureStorage11_Buffer(Renderer11 *renderer,
                            const gl::OffsetBindingPointer<gl::Buffer> &buffer,
                            GLenum internalFormat,
                            const std::string &label);
    ~TextureStorage11_Buffer() override;

    angle::Result getResource(const gl::Context *context,
                              const TextureHelper11 **outResource) override;
    angle::Result getMippedResource(const gl::Context *context,
                                    const TextureHelper11 **outResource) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;

    void onLabelUpdate() override;

    void associateImage(Image11 *image, const gl::ImageIndex &index) override;
    void disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage) override;
    void verifyAssociatedImageValid(const gl::ImageIndex &index, Image11 *expectedImage) override;
    angle::Result releaseAssociatedImage(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         Image11 *incomingImage) override;

  protected:
    angle::Result getSwizzleTexture(const gl::Context *context,
                                    const TextureHelper11 **outTexture) override;
    angle::Result getSwizzleRenderTarget(const gl::Context *context,
                                         int mipLevel,
                                         const d3d11::RenderTargetView **outRTV) override;

  private:
    angle::Result createSRVForSampler(const gl::Context *context,
                                      int baseLevel,
                                      int mipLevels,
                                      DXGI_FORMAT format,
                                      const TextureHelper11 &texture,
                                      d3d11::SharedSRV *outSRV) override;
    angle::Result createSRVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedSRV *outSRV) override;
    angle::Result createUAVForImage(const gl::Context *context,
                                    int level,
                                    DXGI_FORMAT format,
                                    const TextureHelper11 &texture,
                                    d3d11::SharedUAV *outUAV) override;

    angle::Result initTexture(const gl::Context *context);

    TextureHelper11 mTexture;
    const gl::OffsetBindingPointer<gl::Buffer> &mBuffer;
    GLint64 mDataSize;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_TEXTURESTORAGE11_H_
