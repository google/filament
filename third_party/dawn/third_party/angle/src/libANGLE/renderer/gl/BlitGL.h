//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BlitGL.h: Defines the BlitGL class, a helper for blitting textures

#ifndef LIBANGLE_RENDERER_GL_BLITGL_H_
#define LIBANGLE_RENDERER_GL_BLITGL_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/gl/formatutilsgl.h"

#include <map>

namespace angle
{
struct FeaturesGL;
}  // namespace angle

namespace gl
{
class Framebuffer;
class ImageIndex;
}  // namespace gl

namespace rx
{

class FramebufferGL;
class FunctionsGL;
class RenderbufferGL;
class StateManagerGL;
class TextureGL;
struct VertexArrayStateGL;

class BlitGL : angle::NonCopyable
{
  public:
    BlitGL(const FunctionsGL *functions,
           const angle::FeaturesGL &features,
           StateManagerGL *stateManager);
    ~BlitGL();

    angle::Result copyImageToLUMAWorkaroundTexture(const gl::Context *context,
                                                   GLuint texture,
                                                   gl::TextureType textureType,
                                                   gl::TextureTarget target,
                                                   GLenum lumaFormat,
                                                   size_t level,
                                                   const gl::Rectangle &sourceArea,
                                                   GLenum internalFormat,
                                                   gl::Framebuffer *source);

    angle::Result copySubImageToLUMAWorkaroundTexture(const gl::Context *context,
                                                      GLuint texture,
                                                      gl::TextureType textureType,
                                                      gl::TextureTarget target,
                                                      GLenum lumaFormat,
                                                      size_t level,
                                                      const gl::Offset &destOffset,
                                                      const gl::Rectangle &sourceArea,
                                                      gl::Framebuffer *source);

    angle::Result blitColorBufferWithShader(const gl::Context *context,
                                            const gl::Framebuffer *source,
                                            const gl::Framebuffer *dest,
                                            const gl::Rectangle &sourceArea,
                                            const gl::Rectangle &destArea,
                                            GLenum filter,
                                            bool writeAlpha);

    angle::Result blitColorBufferWithShader(const gl::Context *context,
                                            const gl::Framebuffer *source,
                                            const GLuint destFramebuffer,
                                            const gl::Rectangle &sourceArea,
                                            const gl::Rectangle &destArea,
                                            GLenum filter,
                                            bool writeAlpha);

    angle::Result blitColorBufferWithShader(const gl::Context *context,
                                            const gl::Framebuffer *source,
                                            const GLuint destTexture,
                                            const gl::TextureTarget destTarget,
                                            const size_t destLevel,
                                            const gl::Rectangle &sourceArea,
                                            const gl::Rectangle &destArea,
                                            GLenum filter,
                                            bool writeAlpha);

    angle::Result copySubTexture(const gl::Context *context,
                                 TextureGL *source,
                                 size_t sourceLevel,
                                 GLenum sourceComponentType,
                                 GLuint destID,
                                 gl::TextureTarget destTarget,
                                 size_t destLevel,
                                 GLenum destComponentType,
                                 const gl::Extents &sourceSize,
                                 const gl::Rectangle &sourceArea,
                                 const gl::Offset &destOffset,
                                 bool needsLumaWorkaround,
                                 GLenum lumaFormat,
                                 bool unpackFlipY,
                                 bool unpackPremultiplyAlpha,
                                 bool unpackUnmultiplyAlpha,
                                 bool transformLinearToSrgb,
                                 bool *copySucceededOut);

    angle::Result copySubTextureCPUReadback(const gl::Context *context,
                                            TextureGL *source,
                                            size_t sourceLevel,
                                            GLenum sourceSizedInternalFormat,
                                            TextureGL *dest,
                                            gl::TextureTarget destTarget,
                                            size_t destLevel,
                                            GLenum destFormat,
                                            GLenum destType,
                                            const gl::Extents &sourceSize,
                                            const gl::Rectangle &sourceArea,
                                            const gl::Offset &destOffset,
                                            bool needsLumaWorkaround,
                                            GLenum lumaFormat,
                                            bool unpackFlipY,
                                            bool unpackPremultiplyAlpha,
                                            bool unpackUnmultiplyAlpha);

    angle::Result copyTexSubImage(const gl::Context *context,
                                  TextureGL *source,
                                  size_t sourceLevel,
                                  TextureGL *dest,
                                  gl::TextureTarget destTarget,
                                  size_t destLevel,
                                  const gl::Rectangle &sourceArea,
                                  const gl::Offset &destOffset,
                                  bool *copySucceededOut);

    angle::Result clearRenderableTexture(const gl::Context *context,
                                         TextureGL *source,
                                         GLenum sizedInternalFormat,
                                         int numTextureLayers,
                                         const gl::ImageIndex &imageIndex,
                                         bool *clearSucceededOut);

    angle::Result clearRenderbuffer(const gl::Context *context,
                                    RenderbufferGL *source,
                                    GLenum sizedInternalFormat);

    angle::Result clearFramebuffer(const gl::Context *context,
                                   const gl::DrawBufferMask &colorAttachments,
                                   bool depthClear,
                                   bool stencilClear,
                                   FramebufferGL *source);

    angle::Result clearRenderableTextureAlphaToOne(const gl::Context *context,
                                                   GLuint texture,
                                                   gl::TextureTarget target,
                                                   size_t level);

    angle::Result generateMipmap(const gl::Context *context,
                                 TextureGL *source,
                                 GLuint baseLevel,
                                 GLuint levelCount,
                                 const gl::Extents &sourceBaseLevelSize,
                                 const nativegl::TexImageFormat &format);
    angle::Result generateSRGBMipmap(const gl::Context *context,
                                     TextureGL *source,
                                     GLuint baseLevel,
                                     GLuint levelCount,
                                     const gl::Extents &sourceBaseLevelSize);

    angle::Result initializeResources(const gl::Context *context);

  private:
    angle::Result orphanScratchTextures(const gl::Context *context);
    angle::Result setScratchTextureParameter(const gl::Context *context,
                                             GLenum param,
                                             GLenum value);
    angle::Result setVAOState(const gl::Context *context);
    angle::Result initializeVAOState(const gl::Context *context);

    const FunctionsGL *mFunctions;
    const angle::FeaturesGL &mFeatures;
    StateManagerGL *mStateManager;

    struct BlitProgram
    {
        GLuint program                      = 0;
        GLint sourceTextureLocation         = -1;
        GLint scaleLocation                 = -1;
        GLint offsetLocation                = -1;
        GLint multiplyAlphaLocation         = -1;
        GLint unMultiplyAlphaLocation       = -1;
        GLint transformLinearToSrgbLocation = -1;
    };

    angle::Result getBlitProgram(const gl::Context *context,
                                 gl::TextureType sourceTextureType,
                                 GLenum sourceComponentType,
                                 GLenum destComponentType,
                                 BlitProgram **program);

    bool mResourcesInitialized = false;

    // SourceType, SourceComponentType, DestComponentType
    using BlitProgramType = std::tuple<gl::TextureType, GLenum, GLenum>;
    std::map<BlitProgramType, BlitProgram> mBlitPrograms;

    GLuint mScratchTextures[2] = {0};
    GLuint mScratchFBO         = 0;

    GLuint mVAO                   = 0;
    VertexArrayStateGL *mVAOState = nullptr;
    bool mOwnsVAOState            = false;

    const GLuint mTexcoordAttribLocation = 0;
    GLuint mVertexBuffer                 = 0;

    nativegl::TexImageFormat mSRGBMipmapGenerationFormat;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_BLITGL_H_
