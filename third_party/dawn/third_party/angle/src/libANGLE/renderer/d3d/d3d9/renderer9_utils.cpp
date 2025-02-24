//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderer9_utils.cpp: Conversion functions and other utility routines
// specific to the D3D9 renderer.

#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"

#include "common/debug.h"
#include "common/mathutil.h"

#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d9/RenderTarget9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/driver_utils.h"
#include "platform/PlatformMethods.h"
#include "platform/autogen/FeaturesD3D_autogen.h"

namespace rx
{

namespace gl_d3d9
{

D3DCMPFUNC ConvertComparison(GLenum comparison)
{
    D3DCMPFUNC d3dComp = D3DCMP_ALWAYS;
    switch (comparison)
    {
        case GL_NEVER:
            d3dComp = D3DCMP_NEVER;
            break;
        case GL_ALWAYS:
            d3dComp = D3DCMP_ALWAYS;
            break;
        case GL_LESS:
            d3dComp = D3DCMP_LESS;
            break;
        case GL_LEQUAL:
            d3dComp = D3DCMP_LESSEQUAL;
            break;
        case GL_EQUAL:
            d3dComp = D3DCMP_EQUAL;
            break;
        case GL_GREATER:
            d3dComp = D3DCMP_GREATER;
            break;
        case GL_GEQUAL:
            d3dComp = D3DCMP_GREATEREQUAL;
            break;
        case GL_NOTEQUAL:
            d3dComp = D3DCMP_NOTEQUAL;
            break;
        default:
            UNREACHABLE();
    }

    return d3dComp;
}

D3DCOLOR ConvertColor(gl::ColorF color)
{
    return D3DCOLOR_RGBA(gl::unorm<8>(color.red), gl::unorm<8>(color.green),
                         gl::unorm<8>(color.blue), gl::unorm<8>(color.alpha));
}

D3DBLEND ConvertBlendFunc(GLenum blend)
{
    D3DBLEND d3dBlend = D3DBLEND_ZERO;

    switch (blend)
    {
        case GL_ZERO:
            d3dBlend = D3DBLEND_ZERO;
            break;
        case GL_ONE:
            d3dBlend = D3DBLEND_ONE;
            break;
        case GL_SRC_COLOR:
            d3dBlend = D3DBLEND_SRCCOLOR;
            break;
        case GL_ONE_MINUS_SRC_COLOR:
            d3dBlend = D3DBLEND_INVSRCCOLOR;
            break;
        case GL_DST_COLOR:
            d3dBlend = D3DBLEND_DESTCOLOR;
            break;
        case GL_ONE_MINUS_DST_COLOR:
            d3dBlend = D3DBLEND_INVDESTCOLOR;
            break;
        case GL_SRC_ALPHA:
            d3dBlend = D3DBLEND_SRCALPHA;
            break;
        case GL_ONE_MINUS_SRC_ALPHA:
            d3dBlend = D3DBLEND_INVSRCALPHA;
            break;
        case GL_DST_ALPHA:
            d3dBlend = D3DBLEND_DESTALPHA;
            break;
        case GL_ONE_MINUS_DST_ALPHA:
            d3dBlend = D3DBLEND_INVDESTALPHA;
            break;
        case GL_CONSTANT_COLOR:
            d3dBlend = D3DBLEND_BLENDFACTOR;
            break;
        case GL_ONE_MINUS_CONSTANT_COLOR:
            d3dBlend = D3DBLEND_INVBLENDFACTOR;
            break;
        case GL_CONSTANT_ALPHA:
            d3dBlend = D3DBLEND_BLENDFACTOR;
            break;
        case GL_ONE_MINUS_CONSTANT_ALPHA:
            d3dBlend = D3DBLEND_INVBLENDFACTOR;
            break;
        case GL_SRC_ALPHA_SATURATE:
            d3dBlend = D3DBLEND_SRCALPHASAT;
            break;
        default:
            UNREACHABLE();
    }

    return d3dBlend;
}

D3DBLENDOP ConvertBlendOp(GLenum blendOp)
{
    D3DBLENDOP d3dBlendOp = D3DBLENDOP_ADD;

    switch (blendOp)
    {
        case GL_FUNC_ADD:
            d3dBlendOp = D3DBLENDOP_ADD;
            break;
        case GL_FUNC_SUBTRACT:
            d3dBlendOp = D3DBLENDOP_SUBTRACT;
            break;
        case GL_FUNC_REVERSE_SUBTRACT:
            d3dBlendOp = D3DBLENDOP_REVSUBTRACT;
            break;
        case GL_MIN_EXT:
            d3dBlendOp = D3DBLENDOP_MIN;
            break;
        case GL_MAX_EXT:
            d3dBlendOp = D3DBLENDOP_MAX;
            break;
        default:
            UNREACHABLE();
    }

    return d3dBlendOp;
}

D3DSTENCILOP ConvertStencilOp(GLenum stencilOp)
{
    D3DSTENCILOP d3dStencilOp = D3DSTENCILOP_KEEP;

    switch (stencilOp)
    {
        case GL_ZERO:
            d3dStencilOp = D3DSTENCILOP_ZERO;
            break;
        case GL_KEEP:
            d3dStencilOp = D3DSTENCILOP_KEEP;
            break;
        case GL_REPLACE:
            d3dStencilOp = D3DSTENCILOP_REPLACE;
            break;
        case GL_INCR:
            d3dStencilOp = D3DSTENCILOP_INCRSAT;
            break;
        case GL_DECR:
            d3dStencilOp = D3DSTENCILOP_DECRSAT;
            break;
        case GL_INVERT:
            d3dStencilOp = D3DSTENCILOP_INVERT;
            break;
        case GL_INCR_WRAP:
            d3dStencilOp = D3DSTENCILOP_INCR;
            break;
        case GL_DECR_WRAP:
            d3dStencilOp = D3DSTENCILOP_DECR;
            break;
        default:
            UNREACHABLE();
    }

    return d3dStencilOp;
}

D3DTEXTUREADDRESS ConvertTextureWrap(GLenum wrap)
{
    D3DTEXTUREADDRESS d3dWrap = D3DTADDRESS_WRAP;

    switch (wrap)
    {
        case GL_REPEAT:
            d3dWrap = D3DTADDRESS_WRAP;
            break;
        case GL_MIRRORED_REPEAT:
            d3dWrap = D3DTADDRESS_MIRROR;
            break;
        case GL_CLAMP_TO_EDGE:
            d3dWrap = D3DTADDRESS_CLAMP;
            break;
        case GL_CLAMP_TO_BORDER:
            d3dWrap = D3DTADDRESS_BORDER;
            break;
        case GL_MIRROR_CLAMP_TO_EDGE_EXT:
            d3dWrap = D3DTADDRESS_MIRRORONCE;
            break;
        default:
            UNREACHABLE();
    }

    return d3dWrap;
}

D3DCULL ConvertCullMode(gl::CullFaceMode cullFace, GLenum frontFace)
{
    D3DCULL cull = D3DCULL_CCW;
    switch (cullFace)
    {
        case gl::CullFaceMode::Front:
            cull = (frontFace == GL_CCW ? D3DCULL_CW : D3DCULL_CCW);
            break;
        case gl::CullFaceMode::Back:
            cull = (frontFace == GL_CCW ? D3DCULL_CCW : D3DCULL_CW);
            break;
        case gl::CullFaceMode::FrontAndBack:
            cull = D3DCULL_NONE;  // culling will be handled during draw
            break;
        default:
            UNREACHABLE();
    }

    return cull;
}

D3DCUBEMAP_FACES ConvertCubeFace(gl::TextureTarget cubeFace)
{
    D3DCUBEMAP_FACES face = D3DCUBEMAP_FACE_POSITIVE_X;

    switch (cubeFace)
    {
        case gl::TextureTarget::CubeMapPositiveX:
            face = D3DCUBEMAP_FACE_POSITIVE_X;
            break;
        case gl::TextureTarget::CubeMapNegativeX:
            face = D3DCUBEMAP_FACE_NEGATIVE_X;
            break;
        case gl::TextureTarget::CubeMapPositiveY:
            face = D3DCUBEMAP_FACE_POSITIVE_Y;
            break;
        case gl::TextureTarget::CubeMapNegativeY:
            face = D3DCUBEMAP_FACE_NEGATIVE_Y;
            break;
        case gl::TextureTarget::CubeMapPositiveZ:
            face = D3DCUBEMAP_FACE_POSITIVE_Z;
            break;
        case gl::TextureTarget::CubeMapNegativeZ:
            face = D3DCUBEMAP_FACE_NEGATIVE_Z;
            break;
        default:
            UNREACHABLE();
    }

    return face;
}

DWORD ConvertColorMask(bool red, bool green, bool blue, bool alpha)
{
    return (red ? D3DCOLORWRITEENABLE_RED : 0) | (green ? D3DCOLORWRITEENABLE_GREEN : 0) |
           (blue ? D3DCOLORWRITEENABLE_BLUE : 0) | (alpha ? D3DCOLORWRITEENABLE_ALPHA : 0);
}

D3DTEXTUREFILTERTYPE ConvertMagFilter(GLenum magFilter, float maxAnisotropy)
{
    if (maxAnisotropy > 1.0f)
    {
        return D3DTEXF_ANISOTROPIC;
    }

    D3DTEXTUREFILTERTYPE d3dMagFilter = D3DTEXF_POINT;
    switch (magFilter)
    {
        case GL_NEAREST:
            d3dMagFilter = D3DTEXF_POINT;
            break;
        case GL_LINEAR:
            d3dMagFilter = D3DTEXF_LINEAR;
            break;
        default:
            UNREACHABLE();
    }

    return d3dMagFilter;
}

void ConvertMinFilter(GLenum minFilter,
                      D3DTEXTUREFILTERTYPE *d3dMinFilter,
                      D3DTEXTUREFILTERTYPE *d3dMipFilter,
                      float *d3dLodBias,
                      float maxAnisotropy,
                      size_t baseLevel)
{
    switch (minFilter)
    {
        case GL_NEAREST:
            *d3dMinFilter = D3DTEXF_POINT;
            *d3dMipFilter = D3DTEXF_NONE;
            break;
        case GL_LINEAR:
            *d3dMinFilter = D3DTEXF_LINEAR;
            *d3dMipFilter = D3DTEXF_NONE;
            break;
        case GL_NEAREST_MIPMAP_NEAREST:
            *d3dMinFilter = D3DTEXF_POINT;
            *d3dMipFilter = D3DTEXF_POINT;
            break;
        case GL_LINEAR_MIPMAP_NEAREST:
            *d3dMinFilter = D3DTEXF_LINEAR;
            *d3dMipFilter = D3DTEXF_POINT;
            break;
        case GL_NEAREST_MIPMAP_LINEAR:
            *d3dMinFilter = D3DTEXF_POINT;
            *d3dMipFilter = D3DTEXF_LINEAR;
            break;
        case GL_LINEAR_MIPMAP_LINEAR:
            *d3dMinFilter = D3DTEXF_LINEAR;
            *d3dMipFilter = D3DTEXF_LINEAR;
            break;
        default:
            *d3dMinFilter = D3DTEXF_POINT;
            *d3dMipFilter = D3DTEXF_NONE;
            UNREACHABLE();
    }

    // Disabling mipmapping will always sample from level 0 of the texture. It is possible to work
    // around this by modifying D3DSAMP_MAXMIPLEVEL to force a specific mip level to become the
    // lowest sampled mip level and using a large negative value for D3DSAMP_MIPMAPLODBIAS to
    // ensure that only the base mip level is sampled.
    if (baseLevel > 0 && *d3dMipFilter == D3DTEXF_NONE)
    {
        *d3dMipFilter = D3DTEXF_POINT;
        *d3dLodBias   = -static_cast<float>(gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    }
    else
    {
        *d3dLodBias = 0.0f;
    }

    if (maxAnisotropy > 1.0f)
    {
        *d3dMinFilter = D3DTEXF_ANISOTROPIC;
    }
}

D3DQUERYTYPE ConvertQueryType(gl::QueryType type)
{
    switch (type)
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
            return D3DQUERYTYPE_OCCLUSION;
        case gl::QueryType::CommandsCompleted:
            return D3DQUERYTYPE_EVENT;
        default:
            UNREACHABLE();
            return static_cast<D3DQUERYTYPE>(0);
    }
}

D3DMULTISAMPLE_TYPE GetMultisampleType(GLuint samples)
{
    return (samples > 1) ? static_cast<D3DMULTISAMPLE_TYPE>(samples) : D3DMULTISAMPLE_NONE;
}

}  // namespace gl_d3d9

namespace d3d9_gl
{

unsigned int GetReservedVaryingVectors()
{
    // We reserve two registers for "dx_Position" and "gl_Position". The spec says they
    // don't count towards the varying limit, so we must make space for them. We also
    // reserve the last register since it can only pass a PSIZE, and not any arbitrary
    // varying.
    return 3;
}

unsigned int GetReservedVertexUniformVectors()
{
    return 3;  // dx_ViewCoords, dx_ViewAdjust and dx_DepthRange.
}

unsigned int GetReservedFragmentUniformVectors()
{
    return 4;  // dx_ViewCoords, dx_DepthFront, dx_DepthRange, dx_FragCoordoffset.
}

GLsizei GetSamplesCount(D3DMULTISAMPLE_TYPE type)
{
    return (type != D3DMULTISAMPLE_NONMASKABLE) ? type : 0;
}

bool IsFormatChannelEquivalent(D3DFORMAT d3dformat, GLenum format)
{
    GLenum internalFormat  = d3d9::GetD3DFormatInfo(d3dformat).info().glInternalFormat;
    GLenum convertedFormat = gl::GetSizedInternalFormatInfo(internalFormat).format;
    return convertedFormat == format;
}

static gl::TextureCaps GenerateTextureFormatCaps(GLenum internalFormat,
                                                 IDirect3D9 *d3d9,
                                                 D3DDEVTYPE deviceType,
                                                 UINT adapter,
                                                 D3DFORMAT adapterFormat)
{
    gl::TextureCaps textureCaps;

    const d3d9::TextureFormat &d3dFormatInfo = d3d9::GetTextureFormatInfo(internalFormat);
    const gl::InternalFormat &formatInfo     = gl::GetSizedInternalFormatInfo(internalFormat);

    if (d3dFormatInfo.texFormat != D3DFMT_UNKNOWN)
    {
        if (formatInfo.depthBits > 0 || formatInfo.stencilBits > 0)
        {
            textureCaps.texturable = SUCCEEDED(d3d9->CheckDeviceFormat(
                adapter, deviceType, adapterFormat, 0, D3DRTYPE_TEXTURE, d3dFormatInfo.texFormat));
        }
        else
        {
            textureCaps.texturable =
                SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, adapterFormat, 0,
                                                  D3DRTYPE_TEXTURE, d3dFormatInfo.texFormat)) &&
                SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, adapterFormat, 0,
                                                  D3DRTYPE_CUBETEXTURE, d3dFormatInfo.texFormat));
            if (textureCaps.texturable && (formatInfo.colorEncoding == GL_SRGB))
            {
                textureCaps.texturable =
                    SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, adapterFormat,
                                                      D3DUSAGE_QUERY_SRGBREAD, D3DRTYPE_TEXTURE,
                                                      d3dFormatInfo.texFormat)) &&
                    SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, adapterFormat,
                                                      D3DUSAGE_QUERY_SRGBREAD, D3DRTYPE_CUBETEXTURE,
                                                      d3dFormatInfo.texFormat));
            }
        }

        textureCaps.filterable = SUCCEEDED(
            d3d9->CheckDeviceFormat(adapter, deviceType, adapterFormat, D3DUSAGE_QUERY_FILTER,
                                    D3DRTYPE_TEXTURE, d3dFormatInfo.texFormat));
    }

    if (d3dFormatInfo.renderFormat != D3DFMT_UNKNOWN)
    {
        textureCaps.textureAttachment = SUCCEEDED(
            d3d9->CheckDeviceFormat(adapter, deviceType, adapterFormat, D3DUSAGE_RENDERTARGET,
                                    D3DRTYPE_TEXTURE, d3dFormatInfo.renderFormat));
        if (textureCaps.textureAttachment && (formatInfo.colorEncoding == GL_SRGB))
        {
            textureCaps.textureAttachment = SUCCEEDED(d3d9->CheckDeviceFormat(
                adapter, deviceType, adapterFormat, D3DUSAGE_QUERY_SRGBWRITE, D3DRTYPE_TEXTURE,
                d3dFormatInfo.renderFormat));
        }

        if ((formatInfo.depthBits > 0 || formatInfo.stencilBits > 0) &&
            !textureCaps.textureAttachment)
        {
            textureCaps.textureAttachment = SUCCEEDED(
                d3d9->CheckDeviceFormat(adapter, deviceType, adapterFormat, D3DUSAGE_DEPTHSTENCIL,
                                        D3DRTYPE_TEXTURE, d3dFormatInfo.renderFormat));
        }
        textureCaps.renderbuffer = textureCaps.textureAttachment;
        textureCaps.blendable    = textureCaps.renderbuffer;

        textureCaps.sampleCounts.insert(1);
        for (unsigned int i = D3DMULTISAMPLE_2_SAMPLES; i <= D3DMULTISAMPLE_16_SAMPLES; i++)
        {
            D3DMULTISAMPLE_TYPE multisampleType = D3DMULTISAMPLE_TYPE(i);

            HRESULT result = d3d9->CheckDeviceMultiSampleType(
                adapter, deviceType, d3dFormatInfo.renderFormat, TRUE, multisampleType, nullptr);
            if (SUCCEEDED(result))
            {
                textureCaps.sampleCounts.insert(i);
            }
        }
    }

    return textureCaps;
}

void GenerateCaps(IDirect3D9 *d3d9,
                  IDirect3DDevice9 *device,
                  D3DDEVTYPE deviceType,
                  UINT adapter,
                  gl::Caps *caps,
                  gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions,
                  gl::Limitations *limitations)
{
    D3DCAPS9 deviceCaps;
    if (FAILED(d3d9->GetDeviceCaps(adapter, deviceType, &deviceCaps)))
    {
        // Can't continue with out device caps
        return;
    }

    D3DDISPLAYMODE currentDisplayMode;
    d3d9->GetAdapterDisplayMode(adapter, &currentDisplayMode);

    GLuint maxSamples = 0;
    for (GLenum internalFormat : gl::GetAllSizedInternalFormats())
    {
        gl::TextureCaps textureCaps = GenerateTextureFormatCaps(internalFormat, d3d9, deviceType,
                                                                adapter, currentDisplayMode.Format);
        textureCapsMap->insert(internalFormat, textureCaps);

        maxSamples = std::max(maxSamples, textureCaps.getMaxSamples());
    }

    // GL core feature limits
    caps->maxElementIndex = static_cast<GLint64>(std::numeric_limits<unsigned int>::max());

    // 3D textures are unimplemented in D3D9
    caps->max3DTextureSize = 1;

    // Only one limit in GL, use the minimum dimension
    caps->max2DTextureSize = std::min(deviceCaps.MaxTextureWidth, deviceCaps.MaxTextureHeight);

    // D3D treats cube maps as a special case of 2D textures
    caps->maxCubeMapTextureSize = caps->max2DTextureSize;

    // Array textures are not available in D3D9
    caps->maxArrayTextureLayers = 1;

    // ES3-only feature
    caps->maxLODBias = 0.0f;

    // No specific limits on render target size, maximum 2D texture size is equivalent
    caps->maxRenderbufferSize = caps->max2DTextureSize;

    // Draw buffers are not supported in D3D9
    caps->maxDrawBuffers      = 1;
    caps->maxColorAttachments = 1;

    // No specific limits on viewport size, maximum 2D texture size is equivalent
    caps->maxViewportWidth  = caps->max2DTextureSize;
    caps->maxViewportHeight = caps->maxViewportWidth;

    // Point size is clamped to 1.0f when the shader model is less than 3
    caps->minAliasedPointSize = 1.0f;
    caps->maxAliasedPointSize =
        ((D3DSHADER_VERSION_MAJOR(deviceCaps.PixelShaderVersion) >= 3) ? deviceCaps.MaxPointSize
                                                                       : 1.0f);

    // Wide lines not supported
    caps->minAliasedLineWidth = 1.0f;
    caps->maxAliasedLineWidth = 1.0f;

    // Primitive count limits (unused in ES2)
    caps->maxElementsIndices  = 0;
    caps->maxElementsVertices = 0;

    // Program and shader binary formats (no supported shader binary formats)
    caps->programBinaryFormats.push_back(GL_PROGRAM_BINARY_ANGLE);

    caps->vertexHighpFloat.setIEEEFloat();
    caps->vertexMediumpFloat.setIEEEFloat();
    caps->vertexLowpFloat.setIEEEFloat();
    caps->fragmentHighpFloat.setIEEEFloat();
    caps->fragmentMediumpFloat.setIEEEFloat();
    caps->fragmentLowpFloat.setIEEEFloat();

    // Some (most) hardware only supports single-precision floating-point numbers,
    // which can accurately represent integers up to +/-16777216
    caps->vertexHighpInt.setSimulatedInt(24);
    caps->vertexMediumpInt.setSimulatedInt(24);
    caps->vertexLowpInt.setSimulatedInt(24);
    caps->fragmentHighpInt.setSimulatedInt(24);
    caps->fragmentMediumpInt.setSimulatedInt(24);
    caps->fragmentLowpInt.setSimulatedInt(24);

    // WaitSync is ES3-only, set to zero
    caps->maxServerWaitTimeout = 0;

    // Vertex shader limits
    caps->maxVertexAttributes = 16;
    // Vertex Attrib Binding not supported.
    caps->maxVertexAttribBindings = caps->maxVertexAttributes;

    const size_t MAX_VERTEX_CONSTANT_VECTORS_D3D9 = 256;
    caps->maxVertexUniformVectors =
        MAX_VERTEX_CONSTANT_VECTORS_D3D9 - GetReservedVertexUniformVectors();
    caps->maxShaderUniformComponents[gl::ShaderType::Vertex] = caps->maxVertexUniformVectors * 4;

    caps->maxShaderUniformBlocks[gl::ShaderType::Vertex] = 0;

    // SM3 only supports 12 output variables, but the special 12th register is only for PSIZE.
    const unsigned int MAX_VERTEX_OUTPUT_VECTORS_SM3 = 12 - GetReservedVaryingVectors();
    const unsigned int MAX_VERTEX_OUTPUT_VECTORS_SM2 = 10 - GetReservedVaryingVectors();
    caps->maxVertexOutputComponents =
        ((deviceCaps.VertexShaderVersion >= D3DVS_VERSION(3, 0)) ? MAX_VERTEX_OUTPUT_VECTORS_SM3
                                                                 : MAX_VERTEX_OUTPUT_VECTORS_SM2) *
        4;

    // Only Direct3D 10 ready devices support all the necessary vertex texture formats.
    // We test this using D3D9 by checking support for the R16F format.
    if (deviceCaps.VertexShaderVersion >= D3DVS_VERSION(3, 0) &&
        SUCCEEDED(d3d9->CheckDeviceFormat(adapter, deviceType, currentDisplayMode.Format,
                                          D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE,
                                          D3DFMT_R16F)))
    {
        const size_t MAX_TEXTURE_IMAGE_UNITS_VTF_SM3             = 4;
        caps->maxShaderTextureImageUnits[gl::ShaderType::Vertex] = MAX_TEXTURE_IMAGE_UNITS_VTF_SM3;
    }
    else
    {
        caps->maxShaderTextureImageUnits[gl::ShaderType::Vertex] = 0;
    }

    // Fragment shader limits
    const size_t MAX_PIXEL_CONSTANT_VECTORS_SM3 = 224;
    const size_t MAX_PIXEL_CONSTANT_VECTORS_SM2 = 32;
    caps->maxFragmentUniformVectors =
        ((deviceCaps.PixelShaderVersion >= D3DPS_VERSION(3, 0)) ? MAX_PIXEL_CONSTANT_VECTORS_SM3
                                                                : MAX_PIXEL_CONSTANT_VECTORS_SM2) -
        GetReservedFragmentUniformVectors();
    caps->maxShaderUniformComponents[gl::ShaderType::Fragment] =
        caps->maxFragmentUniformVectors * 4;
    caps->maxShaderUniformBlocks[gl::ShaderType::Fragment]     = 0;
    caps->maxFragmentInputComponents                           = caps->maxVertexOutputComponents;
    caps->maxShaderTextureImageUnits[gl::ShaderType::Fragment] = 16;
    caps->minProgramTexelOffset                                = 0;
    caps->maxProgramTexelOffset                                = 0;

    // Aggregate shader limits (unused in ES2)
    caps->maxUniformBufferBindings                                     = 0;
    caps->maxUniformBlockSize                                          = 0;
    caps->uniformBufferOffsetAlignment                                 = 0;
    caps->maxCombinedUniformBlocks                                     = 0;
    caps->maxCombinedShaderUniformComponents[gl::ShaderType::Vertex]   = 0;
    caps->maxCombinedShaderUniformComponents[gl::ShaderType::Fragment] = 0;
    caps->maxVaryingComponents                                         = 0;

    // Aggregate shader limits
    caps->maxVaryingVectors            = caps->maxVertexOutputComponents / 4;
    caps->maxCombinedTextureImageUnits = caps->maxShaderTextureImageUnits[gl::ShaderType::Vertex] +
                                         caps->maxShaderTextureImageUnits[gl::ShaderType::Fragment];

    // Transform feedback limits
    caps->maxTransformFeedbackInterleavedComponents = 0;
    caps->maxTransformFeedbackSeparateAttributes    = 0;
    caps->maxTransformFeedbackSeparateComponents    = 0;

    // Multisample limits
    caps->maxSamples = maxSamples;

    // GL extension support
    extensions->setTextureExtensionSupport(*textureCapsMap);
    extensions->elementIndexUintOES = deviceCaps.MaxVertexIndex >= (1 << 16);
    extensions->getProgramBinaryOES = true;
    extensions->rgb8Rgba8OES        = true;
    extensions->readFormatBgraEXT   = true;
    extensions->pixelBufferObjectNV = false;
    extensions->mapbufferOES        = false;
    extensions->mapBufferRangeEXT   = false;

    // D3D does not allow depth textures to have more than one mipmap level OES_depth_texture
    // allows for that so we can't implement full support with the D3D9 back end.
    extensions->depthTextureOES = false;

    // textureRgEXT is emulated and not performant.
    extensions->textureRgEXT = false;

    // GL_KHR_parallel_shader_compile
    extensions->parallelShaderCompileKHR = true;

    D3DADAPTER_IDENTIFIER9 adapterId = {};
    if (SUCCEEDED(d3d9->GetAdapterIdentifier(adapter, 0, &adapterId)))
    {
        // ATI cards on XP have problems with non-power-of-two textures.
        extensions->textureNpotOES =
            !(deviceCaps.TextureCaps & D3DPTEXTURECAPS_POW2) &&
            !(deviceCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP_POW2) &&
            !(deviceCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) &&
            !(!IsWindowsVistaOrLater() && IsAMD(adapterId.VendorId));

        // Disable depth texture support on AMD cards (See ANGLE issue 839)
        if (IsAMD(adapterId.VendorId))
        {
            extensions->depthTextureANGLE = false;
            extensions->depthTextureOES   = false;
        }
    }
    else
    {
        extensions->textureNpotOES = false;
    }

    extensions->drawBuffersEXT    = false;
    extensions->textureStorageEXT = true;

    // Must support a minimum of 2:1 anisotropy for max anisotropy to be considered supported, per
    // the spec
    extensions->textureFilterAnisotropicEXT =
        (deviceCaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0 && deviceCaps.MaxAnisotropy >= 2;
    caps->maxTextureAnisotropy = static_cast<GLfloat>(deviceCaps.MaxAnisotropy);

    // Check occlusion query support by trying to create one
    IDirect3DQuery9 *occlusionQuery = nullptr;
    extensions->occlusionQueryBooleanEXT =
        SUCCEEDED(device->CreateQuery(D3DQUERYTYPE_OCCLUSION, &occlusionQuery)) && occlusionQuery;
    SafeRelease(occlusionQuery);

    // Check event query support by trying to create one
    IDirect3DQuery9 *eventQuery = nullptr;
    extensions->fenceNV =
        SUCCEEDED(device->CreateQuery(D3DQUERYTYPE_EVENT, &eventQuery)) && eventQuery;
    SafeRelease(eventQuery);

    extensions->disjointTimerQueryEXT = false;
    extensions->robustnessEXT         = true;
    extensions->robustnessKHR         = true;
    // It seems that only DirectX 10 and higher enforce the well-defined behavior of always
    // returning zero values when out-of-bounds reads. See
    // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_robustness.txt
    extensions->robustBufferAccessBehaviorKHR = false;
    extensions->blendMinmaxEXT                = true;
    // Although according to
    // https://docs.microsoft.com/en-us/windows/desktop/direct3ddxgi/format-support-for-direct3d-feature-level-9-1-hardware
    // D3D9 doesn't have full blending capability for RGBA32F. But turns out it could provide
    // correct blending result in reality. As a result of some regression reports by client app, we
    // decided to turn floatBlendEXT on for D3D9
    extensions->floatBlendEXT               = true;
    extensions->framebufferBlitANGLE        = true;
    extensions->framebufferMultisampleANGLE = true;
    extensions->instancedArraysANGLE        = deviceCaps.PixelShaderVersion >= D3DPS_VERSION(3, 0);
    // D3D9 requires at least one attribute that has a divisor of 0, which isn't required by the EXT
    // extension
    extensions->instancedArraysEXT       = false;
    extensions->packReverseRowOrderANGLE = true;
    extensions->standardDerivativesOES =
        (deviceCaps.PS20Caps.Caps & D3DPS20CAPS_GRADIENTINSTRUCTIONS) != 0;
    extensions->shaderTextureLodEXT         = true;
    extensions->fragDepthEXT                = true;
    extensions->textureUsageANGLE           = true;
    extensions->translatedShaderSourceANGLE = true;
    extensions->fboRenderMipmapOES          = true;
    extensions->textureMirrorClampToEdgeEXT = true;
    extensions->discardFramebufferEXT = false;  // It would be valid to set this to true, since
                                                // glDiscardFramebufferEXT is just a hint
    extensions->colorBufferFloatEXT   = false;
    extensions->debugMarkerEXT        = true;
    extensions->EGLImageOES           = true;
    extensions->EGLImageExternalOES   = true;
    extensions->unpackSubimageEXT     = true;
    extensions->packSubimageNV        = true;
    extensions->syncQueryCHROMIUM     = extensions->fenceNV;
    extensions->copyTextureCHROMIUM   = true;
    extensions->textureBorderClampEXT = true;
    extensions->textureBorderClampOES = true;
    extensions->videoTextureWEBGL     = true;

    // D3D9 has no concept of separate masks and refs for front and back faces in the depth stencil
    // state.
    limitations->noSeparateStencilRefsAndMasks = true;

    // D3D9 cannot support constant color and alpha blend funcs together
    limitations->noSimultaneousConstantColorAndAlphaBlendFunc = true;

    // D3D9 cannot support unclamped constant blend color
    limitations->noUnclampedBlendColor = true;

    // D3D9 cannot support packing more than one variable to a single varying.
    // TODO(jmadill): Implement more sophisticated component packing in D3D9.
    limitations->noFlexibleVaryingPacking = true;

    // D3D9 does not support vertex attribute aliasing
    limitations->noVertexAttributeAliasing = true;

    // D3D9 does not support compressed textures where the base mip level is not a multiple of 4
    limitations->compressedBaseMipLevelMultipleOfFour = true;
}

}  // namespace d3d9_gl

namespace d3d9
{

GLuint ComputeBlockSize(D3DFORMAT format, GLuint width, GLuint height)
{
    const D3DFormat &d3dFormatInfo = d3d9::GetD3DFormatInfo(format);
    GLuint numBlocksWide  = (width + d3dFormatInfo.blockWidth - 1) / d3dFormatInfo.blockWidth;
    GLuint numBlocksHight = (height + d3dFormatInfo.blockHeight - 1) / d3dFormatInfo.blockHeight;
    return (d3dFormatInfo.pixelBytes * numBlocksWide * numBlocksHight);
}

void MakeValidSize(bool isImage,
                   D3DFORMAT format,
                   GLsizei *requestWidth,
                   GLsizei *requestHeight,
                   int *levelOffset)
{
    const D3DFormat &d3dFormatInfo = d3d9::GetD3DFormatInfo(format);

    int upsampleCount = 0;
    // Don't expand the size of full textures that are at least (blockWidth x blockHeight) already.
    if (isImage || *requestWidth < static_cast<GLsizei>(d3dFormatInfo.blockWidth) ||
        *requestHeight < static_cast<GLsizei>(d3dFormatInfo.blockHeight))
    {
        while (*requestWidth % d3dFormatInfo.blockWidth != 0 ||
               *requestHeight % d3dFormatInfo.blockHeight != 0)
        {
            *requestWidth <<= 1;
            *requestHeight <<= 1;
            upsampleCount++;
        }
    }
    *levelOffset = upsampleCount;
}

void InitializeFeatures(angle::FeaturesD3D *features, DWORD vendorID)
{
    ANGLE_FEATURE_CONDITION(features, mrtPerfWorkaround, true);
    ANGLE_FEATURE_CONDITION(features, setDataFasterThanImageUpload, false);

    // TODO(jmadill): Disable workaround when we have a fixed compiler DLL.
    ANGLE_FEATURE_CONDITION(features, expandIntegerPowExpressions, true);

    // crbug.com/1011627 Turn this on for D3D9.
    ANGLE_FEATURE_CONDITION(features, allowClearForRobustResourceInit, true);

    ANGLE_FEATURE_CONDITION(features, borderColorSrgb, IsNvidia(vendorID));

    // D3D9 shader models have limited support for looping, so the Appendix A
    // index/loop limitations are necessary. Workarounds that are needed to
    // support dynamic indexing of vectors on HLSL also don't work on D3D9.
    ANGLE_FEATURE_CONDITION(features, supportsNonConstantLoopIndexing, false);
}

void InitializeFrontendFeatures(angle::FrontendFeatures *features, DWORD vendorID)
{
    // The D3D backend's handling of compile and link is thread-safe
    ANGLE_FEATURE_CONDITION(features, compileJobIsThreadSafe, true);
    ANGLE_FEATURE_CONDITION(features, linkJobIsThreadSafe, true);
}
}  // namespace d3d9

}  // namespace rx
