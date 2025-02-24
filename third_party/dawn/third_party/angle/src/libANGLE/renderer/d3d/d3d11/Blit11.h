//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Blit11.cpp: Texture copy utility class.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_BLIT11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_BLIT11_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

#include <map>

namespace rx
{
class Renderer11;

class Blit11 : angle::NonCopyable
{
  public:
    explicit Blit11(Renderer11 *renderer);
    ~Blit11();

    angle::Result swizzleTexture(const gl::Context *context,
                                 const d3d11::SharedSRV &source,
                                 const d3d11::RenderTargetView &dest,
                                 const gl::Extents &size,
                                 const gl::SwizzleState &swizzleTarget);

    // Set destTypeForDownsampling to GL_NONE to skip downsampling
    angle::Result copyTexture(const gl::Context *context,
                              const d3d11::SharedSRV &source,
                              const gl::Box &sourceArea,
                              const gl::Extents &sourceSize,
                              GLenum sourceFormat,
                              const d3d11::RenderTargetView &dest,
                              const gl::Box &destArea,
                              const gl::Extents &destSize,
                              const gl::Rectangle *scissor,
                              GLenum destFormat,
                              GLenum destTypeForDownsampling,
                              GLenum filter,
                              bool maskOffAlpha,
                              bool unpackPremultiplyAlpha,
                              bool unpackUnmultiplyAlpha);

    angle::Result copyStencil(const gl::Context *context,
                              const TextureHelper11 &source,
                              unsigned int sourceSubresource,
                              const gl::Box &sourceArea,
                              const gl::Extents &sourceSize,
                              const TextureHelper11 &dest,
                              unsigned int destSubresource,
                              const gl::Box &destArea,
                              const gl::Extents &destSize,
                              const gl::Rectangle *scissor);

    angle::Result copyDepth(const gl::Context *context,
                            const d3d11::SharedSRV &source,
                            const gl::Box &sourceArea,
                            const gl::Extents &sourceSize,
                            const d3d11::DepthStencilView &dest,
                            const gl::Box &destArea,
                            const gl::Extents &destSize,
                            const gl::Rectangle *scissor);

    angle::Result copyDepthStencil(const gl::Context *context,
                                   const TextureHelper11 &source,
                                   unsigned int sourceSubresource,
                                   const gl::Box &sourceArea,
                                   const gl::Extents &sourceSize,
                                   const TextureHelper11 &dest,
                                   unsigned int destSubresource,
                                   const gl::Box &destArea,
                                   const gl::Extents &destSize,
                                   const gl::Rectangle *scissor);

    angle::Result resolveDepth(const gl::Context *context,
                               RenderTarget11 *depth,
                               TextureHelper11 *textureOut);

    angle::Result resolveStencil(const gl::Context *context,
                                 RenderTarget11 *depthStencil,
                                 bool alsoDepth,
                                 TextureHelper11 *textureOut);

    using BlitConvertFunction = void(const gl::Box &sourceArea,
                                     const gl::Box &destArea,
                                     const gl::Rectangle &clipRect,
                                     const gl::Extents &sourceSize,
                                     unsigned int sourceRowPitch,
                                     unsigned int destRowPitch,
                                     ptrdiff_t readOffset,
                                     ptrdiff_t writeOffset,
                                     size_t copySize,
                                     size_t srcPixelStride,
                                     size_t destPixelStride,
                                     const uint8_t *sourceData,
                                     uint8_t *destData);

  private:
    enum BlitShaderOperation : unsigned int;
    enum BlitShaderType : unsigned int;
    enum SwizzleShaderType
    {
        SWIZZLESHADER_INVALID,
        SWIZZLESHADER_2D_FLOAT,
        SWIZZLESHADER_2D_UINT,
        SWIZZLESHADER_2D_INT,
        SWIZZLESHADER_CUBE_FLOAT,
        SWIZZLESHADER_CUBE_UINT,
        SWIZZLESHADER_CUBE_INT,
        SWIZZLESHADER_3D_FLOAT,
        SWIZZLESHADER_3D_UINT,
        SWIZZLESHADER_3D_INT,
        SWIZZLESHADER_ARRAY_FLOAT,
        SWIZZLESHADER_ARRAY_UINT,
        SWIZZLESHADER_ARRAY_INT,
    };

    typedef void (*WriteVertexFunction)(const gl::Box &sourceArea,
                                        const gl::Extents &sourceSize,
                                        const gl::Box &destArea,
                                        const gl::Extents &destSize,
                                        void *outVertices,
                                        unsigned int *outStride,
                                        unsigned int *outVertexCount,
                                        D3D11_PRIMITIVE_TOPOLOGY *outTopology);

    enum ShaderDimension
    {
        SHADER_INVALID,
        SHADER_2D,
        SHADER_3D,
        SHADER_2DARRAY
    };

    struct Shader
    {
        Shader();
        Shader(Shader &&other);
        ~Shader();
        Shader &operator=(Shader &&other);

        ShaderDimension dimension;
        d3d11::PixelShader pixelShader;
    };

    struct ShaderSupport
    {
        const d3d11::InputLayout *inputLayout;
        const d3d11::VertexShader *vertexShader;
        const d3d11::GeometryShader *geometryShader;
        WriteVertexFunction vertexWriteFunction;
    };

    angle::Result initResources(const gl::Context *context);

    angle::Result getShaderSupport(const gl::Context *context,
                                   const Shader &shader,
                                   ShaderSupport *supportOut);

    static BlitShaderOperation getBlitShaderOperation(GLenum destinationFormat,
                                                      GLenum sourceFormat,
                                                      bool isSrcSigned,
                                                      bool isDestSigned,
                                                      bool unpackPremultiplyAlpha,
                                                      bool unpackUnmultiplyAlpha,
                                                      GLenum destTypeForDownsampling);

    static BlitShaderType getBlitShaderType(BlitShaderOperation operation,
                                            ShaderDimension dimension);

    static SwizzleShaderType GetSwizzleShaderType(GLenum type, D3D11_SRV_DIMENSION dimensionality);

    angle::Result copyDepthStencilImpl(const gl::Context *context,
                                       const TextureHelper11 &source,
                                       unsigned int sourceSubresource,
                                       const gl::Box &sourceArea,
                                       const gl::Extents &sourceSize,
                                       const TextureHelper11 &dest,
                                       unsigned int destSubresource,
                                       const gl::Box &destArea,
                                       const gl::Extents &destSize,
                                       const gl::Rectangle *scissor,
                                       bool stencilOnly);

    angle::Result copyAndConvertImpl(const gl::Context *context,
                                     const TextureHelper11 &source,
                                     unsigned int sourceSubresource,
                                     const gl::Box &sourceArea,
                                     const gl::Extents &sourceSize,
                                     const TextureHelper11 &destStaging,
                                     const gl::Box &destArea,
                                     const gl::Extents &destSize,
                                     const gl::Rectangle *scissor,
                                     size_t readOffset,
                                     size_t writeOffset,
                                     size_t copySize,
                                     size_t srcPixelStride,
                                     size_t destPixelStride,
                                     BlitConvertFunction *convertFunction);

    angle::Result copyAndConvert(const gl::Context *context,
                                 const TextureHelper11 &source,
                                 unsigned int sourceSubresource,
                                 const gl::Box &sourceArea,
                                 const gl::Extents &sourceSize,
                                 const TextureHelper11 &dest,
                                 unsigned int destSubresource,
                                 const gl::Box &destArea,
                                 const gl::Extents &destSize,
                                 const gl::Rectangle *scissor,
                                 size_t readOffset,
                                 size_t writeOffset,
                                 size_t copySize,
                                 size_t srcPixelStride,
                                 size_t destPixelStride,
                                 BlitConvertFunction *convertFunction);

    angle::Result mapBlitShader(const gl::Context *context, BlitShaderType blitShaderType);
    angle::Result addBlitShaderToMap(const gl::Context *context,
                                     BlitShaderType blitShaderType,
                                     ShaderDimension dimension,
                                     const ShaderData &shaderData,
                                     const char *name);

    angle::Result getBlitShader(const gl::Context *context,
                                GLenum destFormat,
                                GLenum sourceFormat,
                                bool isSrcSigned,
                                bool isDestSigned,
                                bool unpackPremultiplyAlpha,
                                bool unpackUnmultiplyAlpha,
                                GLenum destTypeForDownsampling,
                                ShaderDimension dimension,
                                const Shader **shaderOut);
    angle::Result getSwizzleShader(const gl::Context *context,
                                   GLenum type,
                                   D3D11_SRV_DIMENSION viewDimension,
                                   const Shader **shaderOut);

    angle::Result addSwizzleShaderToMap(const gl::Context *context,
                                        SwizzleShaderType swizzleShaderType,
                                        ShaderDimension dimension,
                                        const ShaderData &shaderData,
                                        const char *name);

    void clearShaderMap();
    void releaseResolveDepthStencilResources();
    angle::Result initResolveDepthOnly(const gl::Context *context,
                                       const d3d11::Format &format,
                                       const gl::Extents &extents);
    angle::Result initResolveDepthStencil(const gl::Context *context, const gl::Extents &extents);

    Renderer11 *mRenderer;

    std::map<BlitShaderType, Shader> mBlitShaderMap;
    std::map<SwizzleShaderType, Shader> mSwizzleShaderMap;

    bool mResourcesInitialized;
    d3d11::Buffer mVertexBuffer;
    d3d11::SamplerState mPointSampler;
    d3d11::SamplerState mLinearSampler;
    d3d11::RasterizerState mScissorEnabledRasterizerState;
    d3d11::RasterizerState mScissorDisabledRasterizerState;
    d3d11::DepthStencilState mDepthStencilState;

    d3d11::LazyInputLayout mQuad2DIL;
    d3d11::LazyShader<ID3D11VertexShader> mQuad2DVS;
    d3d11::LazyShader<ID3D11PixelShader> mDepthPS;

    d3d11::LazyInputLayout mQuad3DIL;
    d3d11::LazyShader<ID3D11VertexShader> mQuad3DVS;
    d3d11::LazyShader<ID3D11GeometryShader> mQuad3DGS;

    d3d11::LazyBlendState mAlphaMaskBlendState;

    d3d11::Buffer mSwizzleCB;

    d3d11::LazyShader<ID3D11VertexShader> mResolveDepthStencilVS;
    d3d11::LazyShader<ID3D11PixelShader> mResolveDepthPS;
    d3d11::LazyShader<ID3D11PixelShader> mResolveDepthStencilPS;
    d3d11::LazyShader<ID3D11PixelShader> mResolveStencilPS;
    d3d11::ShaderResourceView mStencilSRV;
    TextureHelper11 mResolvedDepthStencil;
    d3d11::RenderTargetView mResolvedDepthStencilRTView;
    TextureHelper11 mResolvedDepth;
    d3d11::DepthStencilView mResolvedDepthDSView;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_BLIT11_H_
