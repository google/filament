//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// context_private_call.cpp:
//   Helpers that set/get state that is entirely locally accessed by the context.

#include "libANGLE/context_private_call_autogen.h"

#include "common/debug.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/queryutils.h"

namespace
{
inline angle::Mat4 FixedMatrixToMat4(const GLfixed *m)
{
    angle::Mat4 matrixAsFloat;
    GLfloat *floatData = matrixAsFloat.data();

    for (int i = 0; i < 16; i++)
    {
        floatData[i] = gl::ConvertFixedToFloat(m[i]);
    }

    return matrixAsFloat;
}
}  // namespace

namespace gl
{
inline void ContextPrivateClearColor(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLfloat red,
                                     GLfloat green,
                                     GLfloat blue,
                                     GLfloat alpha)
{
    privateState->setColorClearValue(red, green, blue, alpha);
}

inline void ContextPrivateClearDepthf(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLfloat depth)
{
    privateState->setDepthClearValue(clamp01(depth));
}

inline void ContextPrivateClearStencil(PrivateState *privateState,
                                       PrivateStateCache *privateStateCache,
                                       GLint stencil)
{
    privateState->setStencilClearValue(stencil);
}

inline void ContextPrivateClearColorx(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLfixed red,
                                      GLfixed green,
                                      GLfixed blue,
                                      GLfixed alpha)
{
    ContextPrivateClearColor(privateState, privateStateCache, ConvertFixedToFloat(red),
                             ConvertFixedToFloat(green), ConvertFixedToFloat(blue),
                             ConvertFixedToFloat(alpha));
}

inline void ContextPrivateClearDepthx(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLfixed depth)
{
    ContextPrivateClearDepthf(privateState, privateStateCache, ConvertFixedToFloat(depth));
}

inline void ContextPrivateColorMask(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLboolean red,
                                    GLboolean green,
                                    GLboolean blue,
                                    GLboolean alpha)
{
    privateState->setColorMask(ConvertToBool(red), ConvertToBool(green), ConvertToBool(blue),
                               ConvertToBool(alpha));
    privateStateCache->onColorMaskChange();
}

inline void ContextPrivateColorMaski(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLuint index,
                                     GLboolean r,
                                     GLboolean g,
                                     GLboolean b,
                                     GLboolean a)
{
    privateState->setColorMaskIndexed(ConvertToBool(r), ConvertToBool(g), ConvertToBool(b),
                                      ConvertToBool(a), index);
    privateStateCache->onColorMaskChange();
}

inline void ContextPrivateDepthMask(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLboolean flag)
{
    privateState->setDepthMask(ConvertToBool(flag));
}

inline void ContextPrivateDisable(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  GLenum cap)
{
    privateState->setEnableFeature(cap, false);
    privateStateCache->onCapChange();
}

inline void ContextPrivateDisablei(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   GLenum target,
                                   GLuint index)
{
    privateState->setEnableFeatureIndexed(target, false, index);
    privateStateCache->onCapChange();
}

inline void ContextPrivateEnable(PrivateState *privateState,
                                 PrivateStateCache *privateStateCache,
                                 GLenum cap)
{
    privateState->setEnableFeature(cap, true);
    privateStateCache->onCapChange();
}

inline void ContextPrivateEnablei(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  GLenum target,
                                  GLuint index)
{
    privateState->setEnableFeatureIndexed(target, true, index);
    privateStateCache->onCapChange();
}

inline void ContextPrivateActiveTexture(PrivateState *privateState,
                                        PrivateStateCache *privateStateCache,
                                        GLenum texture)
{
    privateState->setActiveSampler(texture - GL_TEXTURE0);
}

inline void ContextPrivateCullFace(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   CullFaceMode mode)
{
    privateState->setCullMode(mode);
}

inline void ContextPrivateDepthFunc(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLenum func)
{
    privateState->setDepthFunc(func);
}

inline void ContextPrivateDepthRangef(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLfloat zNear,
                                      GLfloat zFar)
{
    privateState->setDepthRange(clamp01(zNear), clamp01(zFar));
}

inline void ContextPrivateDepthRangex(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLfixed zNear,
                                      GLfixed zFar)
{
    ContextPrivateDepthRangef(privateState, privateStateCache, ConvertFixedToFloat(zNear),
                              ConvertFixedToFloat(zFar));
}

inline void ContextPrivateFrontFace(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLenum mode)
{
    privateState->setFrontFace(mode);
}

inline void ContextPrivateLineWidth(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLfloat width)
{
    privateState->setLineWidth(width);
}

inline void ContextPrivateLineWidthx(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLfixed width)
{
    ContextPrivateLineWidth(privateState, privateStateCache, ConvertFixedToFloat(width));
}

inline void ContextPrivatePolygonOffset(PrivateState *privateState,
                                        PrivateStateCache *privateStateCache,
                                        GLfloat factor,
                                        GLfloat units)
{
    privateState->setPolygonOffsetParams(factor, units, 0.0f);
}

inline void ContextPrivatePolygonOffsetClamp(PrivateState *privateState,
                                             PrivateStateCache *privateStateCache,
                                             GLfloat factor,
                                             GLfloat units,
                                             GLfloat clamp)
{
    privateState->setPolygonOffsetParams(factor, units, clamp);
}

inline void ContextPrivatePolygonOffsetx(PrivateState *privateState,
                                         PrivateStateCache *privateStateCache,
                                         GLfixed factor,
                                         GLfixed units)
{
    ContextPrivatePolygonOffsetClamp(privateState, privateStateCache, ConvertFixedToFloat(factor),
                                     ConvertFixedToFloat(units), 0.0f);
}

inline void ContextPrivateSampleCoverage(PrivateState *privateState,
                                         PrivateStateCache *privateStateCache,
                                         GLfloat value,
                                         GLboolean invert)
{
    privateState->setSampleCoverageParams(clamp01(value), ConvertToBool(invert));
}

inline void ContextPrivateSampleCoveragex(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLclampx value,
                                          GLboolean invert)
{
    ContextPrivateSampleCoverage(privateState, privateStateCache, ConvertFixedToFloat(value),
                                 invert);
}

inline void ContextPrivateScissor(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  GLint x,
                                  GLint y,
                                  GLsizei width,
                                  GLsizei height)
{
    privateState->setScissorParams(x, y, width, height);
}

inline void ContextPrivateVertexAttrib1f(PrivateState *privateState,
                                         PrivateStateCache *privateStateCache,
                                         GLuint index,
                                         GLfloat x)
{
    GLfloat vals[4] = {x, 0, 0, 1};
    privateState->setVertexAttribf(index, vals);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttrib1fv(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLuint index,
                                          const GLfloat *values)
{
    GLfloat vals[4] = {values[0], 0, 0, 1};
    privateState->setVertexAttribf(index, vals);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttrib2f(PrivateState *privateState,
                                         PrivateStateCache *privateStateCache,
                                         GLuint index,
                                         GLfloat x,
                                         GLfloat y)
{
    GLfloat vals[4] = {x, y, 0, 1};
    privateState->setVertexAttribf(index, vals);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttrib2fv(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLuint index,
                                          const GLfloat *values)
{
    GLfloat vals[4] = {values[0], values[1], 0, 1};
    privateState->setVertexAttribf(index, vals);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttrib3f(PrivateState *privateState,
                                         PrivateStateCache *privateStateCache,
                                         GLuint index,
                                         GLfloat x,
                                         GLfloat y,
                                         GLfloat z)
{
    GLfloat vals[4] = {x, y, z, 1};
    privateState->setVertexAttribf(index, vals);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttrib3fv(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLuint index,
                                          const GLfloat *values)
{
    GLfloat vals[4] = {values[0], values[1], values[2], 1};
    privateState->setVertexAttribf(index, vals);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttrib4f(PrivateState *privateState,
                                         PrivateStateCache *privateStateCache,
                                         GLuint index,
                                         GLfloat x,
                                         GLfloat y,
                                         GLfloat z,
                                         GLfloat w)
{
    GLfloat vals[4] = {x, y, z, w};
    privateState->setVertexAttribf(index, vals);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttrib4fv(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLuint index,
                                          const GLfloat *values)
{
    privateState->setVertexAttribf(index, values);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttribI4i(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLuint index,
                                          GLint x,
                                          GLint y,
                                          GLint z,
                                          GLint w)
{
    GLint vals[4] = {x, y, z, w};
    privateState->setVertexAttribi(index, vals);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttribI4iv(PrivateState *privateState,
                                           PrivateStateCache *privateStateCache,
                                           GLuint index,
                                           const GLint *values)
{
    privateState->setVertexAttribi(index, values);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttribI4ui(PrivateState *privateState,
                                           PrivateStateCache *privateStateCache,
                                           GLuint index,
                                           GLuint x,
                                           GLuint y,
                                           GLuint z,
                                           GLuint w)
{
    GLuint vals[4] = {x, y, z, w};
    privateState->setVertexAttribu(index, vals);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateVertexAttribI4uiv(PrivateState *privateState,
                                            PrivateStateCache *privateStateCache,
                                            GLuint index,
                                            const GLuint *values)
{
    privateState->setVertexAttribu(index, values);
    privateStateCache->onDefaultVertexAttributeChange();
}

inline void ContextPrivateViewport(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   GLint x,
                                   GLint y,
                                   GLsizei width,
                                   GLsizei height)
{
    privateState->setViewportParams(x, y, width, height);
}

inline void ContextPrivateSampleMaski(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLuint maskNumber,
                                      GLbitfield mask)
{
    privateState->setSampleMaskParams(maskNumber, mask);
}

inline void ContextPrivateMinSampleShading(PrivateState *privateState,
                                           PrivateStateCache *privateStateCache,
                                           GLfloat value)
{
    privateState->setMinSampleShading(value);
}

inline void ContextPrivatePrimitiveBoundingBox(PrivateState *privateState,
                                               PrivateStateCache *privateStateCache,
                                               GLfloat minX,
                                               GLfloat minY,
                                               GLfloat minZ,
                                               GLfloat minW,
                                               GLfloat maxX,
                                               GLfloat maxY,
                                               GLfloat maxZ,
                                               GLfloat maxW)
{
    privateState->setBoundingBox(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
}

inline void ContextPrivateLogicOp(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  LogicalOperation opcode)
{
    privateState->getMutableGLES1State()->setLogicOp(opcode);
}

inline void ContextPrivateLogicOpANGLE(PrivateState *privateState,
                                       PrivateStateCache *privateStateCache,
                                       LogicalOperation opcode)
{
    privateState->setLogicOp(opcode);
}

inline void ContextPrivatePolygonMode(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLenum face,
                                      PolygonMode mode)
{
    ASSERT(face == GL_FRONT_AND_BACK);
    privateState->setPolygonMode(mode);
}

inline void ContextPrivatePolygonModeNV(PrivateState *privateState,
                                        PrivateStateCache *privateStateCache,
                                        GLenum face,
                                        PolygonMode mode)
{
    ContextPrivatePolygonMode(privateState, privateStateCache, face, mode);
}

inline void ContextPrivateProvokingVertex(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          ProvokingVertexConvention provokeMode)
{
    privateState->setProvokingVertex(provokeMode);
}

inline void ContextPrivateCoverageModulation(PrivateState *privateState,
                                             PrivateStateCache *privateStateCache,
                                             GLenum components)
{
    privateState->setCoverageModulation(components);
}

inline void ContextPrivateClipControl(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      ClipOrigin origin,
                                      ClipDepthMode depth)
{
    privateState->setClipControl(origin, depth);
}

inline void ContextPrivateShadingRate(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLenum rate)
{
    privateState->setShadingRate(rate);
}

inline void ContextPrivateBlendColor(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLfloat red,
                                     GLfloat green,
                                     GLfloat blue,
                                     GLfloat alpha)
{
    privateState->setBlendColor(red, green, blue, alpha);
}

inline void ContextPrivateBlendEquation(PrivateState *privateState,
                                        PrivateStateCache *privateStateCache,
                                        GLenum mode)
{
    privateState->setBlendEquation(mode, mode);
    if (privateState->getExtensions().blendEquationAdvancedKHR)
    {
        privateStateCache->onBlendEquationOrFuncChange();
    }
}

inline void ContextPrivateBlendEquationi(PrivateState *privateState,
                                         PrivateStateCache *privateStateCache,
                                         GLuint buf,
                                         GLenum mode)
{
    privateState->setBlendEquationIndexed(mode, mode, buf);
    if (privateState->getExtensions().blendEquationAdvancedKHR)
    {
        privateStateCache->onBlendEquationOrFuncChange();
    }
}

inline void ContextPrivateBlendEquationSeparate(PrivateState *privateState,
                                                PrivateStateCache *privateStateCache,
                                                GLenum modeRGB,
                                                GLenum modeAlpha)
{
    privateState->setBlendEquation(modeRGB, modeAlpha);
    if (privateState->getExtensions().blendEquationAdvancedKHR)
    {
        privateStateCache->onBlendEquationOrFuncChange();
    }
}

inline void ContextPrivateBlendEquationSeparatei(PrivateState *privateState,
                                                 PrivateStateCache *privateStateCache,
                                                 GLuint buf,
                                                 GLenum modeRGB,
                                                 GLenum modeAlpha)
{
    privateState->setBlendEquationIndexed(modeRGB, modeAlpha, buf);
    if (privateState->getExtensions().blendEquationAdvancedKHR)
    {
        privateStateCache->onBlendEquationOrFuncChange();
    }
}

inline void ContextPrivateBlendFunc(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLenum sfactor,
                                    GLenum dfactor)
{
    privateState->setBlendFactors(sfactor, dfactor, sfactor, dfactor);
    if (privateState->getExtensions().blendFuncExtendedEXT)
    {
        privateStateCache->onBlendEquationOrFuncChange();
    }
}

inline void ContextPrivateBlendFunci(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLuint buf,
                                     GLenum src,
                                     GLenum dst)
{
    privateState->setBlendFactorsIndexed(src, dst, src, dst, buf);
    if (privateState->noSimultaneousConstantColorAndAlphaBlendFunc() ||
        privateState->getExtensions().blendFuncExtendedEXT)
    {
        privateStateCache->onBlendEquationOrFuncChange();
    }
}

inline void ContextPrivateBlendFuncSeparate(PrivateState *privateState,
                                            PrivateStateCache *privateStateCache,
                                            GLenum srcRGB,
                                            GLenum dstRGB,
                                            GLenum srcAlpha,
                                            GLenum dstAlpha)
{
    privateState->setBlendFactors(srcRGB, dstRGB, srcAlpha, dstAlpha);
    if (privateState->getExtensions().blendFuncExtendedEXT)
    {
        privateStateCache->onBlendEquationOrFuncChange();
    }
}

inline void ContextPrivateBlendFuncSeparatei(PrivateState *privateState,
                                             PrivateStateCache *privateStateCache,
                                             GLuint buf,
                                             GLenum srcRGB,
                                             GLenum dstRGB,
                                             GLenum srcAlpha,
                                             GLenum dstAlpha)
{
    privateState->setBlendFactorsIndexed(srcRGB, dstRGB, srcAlpha, dstAlpha, buf);
    if (privateState->noSimultaneousConstantColorAndAlphaBlendFunc() ||
        privateState->getExtensions().blendFuncExtendedEXT)
    {
        privateStateCache->onBlendEquationOrFuncChange();
    }
}

inline void ContextPrivateStencilFunc(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLenum func,
                                      GLint ref,
                                      GLuint mask)
{
    ContextPrivateStencilFuncSeparate(privateState, privateStateCache, GL_FRONT_AND_BACK, func, ref,
                                      mask);
}

inline void ContextPrivateStencilFuncSeparate(PrivateState *privateState,
                                              PrivateStateCache *privateStateCache,
                                              GLenum face,
                                              GLenum func,
                                              GLint ref,
                                              GLuint mask)
{
    GLint clampedRef = gl::clamp(ref, 0, std::numeric_limits<uint8_t>::max());
    if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
    {
        privateState->setStencilParams(func, clampedRef, mask);
    }

    if (face == GL_BACK || face == GL_FRONT_AND_BACK)
    {
        privateState->setStencilBackParams(func, clampedRef, mask);
    }

    privateStateCache->onStencilStateChange();
}

inline void ContextPrivateStencilMask(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLuint mask)
{
    ContextPrivateStencilMaskSeparate(privateState, privateStateCache, GL_FRONT_AND_BACK, mask);
}

inline void ContextPrivateStencilMaskSeparate(PrivateState *privateState,
                                              PrivateStateCache *privateStateCache,
                                              GLenum face,
                                              GLuint mask)
{
    if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
    {
        privateState->setStencilWritemask(mask);
    }

    if (face == GL_BACK || face == GL_FRONT_AND_BACK)
    {
        privateState->setStencilBackWritemask(mask);
    }

    privateStateCache->onStencilStateChange();
}

inline void ContextPrivateStencilOp(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLenum fail,
                                    GLenum zfail,
                                    GLenum zpass)
{
    ContextPrivateStencilOpSeparate(privateState, privateStateCache, GL_FRONT_AND_BACK, fail, zfail,
                                    zpass);
}

inline void ContextPrivateStencilOpSeparate(PrivateState *privateState,
                                            PrivateStateCache *privateStateCache,
                                            GLenum face,
                                            GLenum fail,
                                            GLenum zfail,
                                            GLenum zpass)
{
    if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
    {
        privateState->setStencilOperations(fail, zfail, zpass);
    }

    if (face == GL_BACK || face == GL_FRONT_AND_BACK)
    {
        privateState->setStencilBackOperations(fail, zfail, zpass);
    }
}

inline void ContextPrivatePixelStorei(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLenum pname,
                                      GLint param)
{
    switch (pname)
    {
        case GL_UNPACK_ALIGNMENT:
            privateState->setUnpackAlignment(param);
            break;

        case GL_PACK_ALIGNMENT:
            privateState->setPackAlignment(param);
            break;

        case GL_PACK_REVERSE_ROW_ORDER_ANGLE:
            privateState->setPackReverseRowOrder(param != 0);
            break;

        case GL_UNPACK_ROW_LENGTH:
            ASSERT(privateState->getClientMajorVersion() >= 3 ||
                   privateState->getExtensions().unpackSubimageEXT);
            privateState->setUnpackRowLength(param);
            break;

        case GL_UNPACK_IMAGE_HEIGHT:
            ASSERT(privateState->getClientMajorVersion() >= 3);
            privateState->setUnpackImageHeight(param);
            break;

        case GL_UNPACK_SKIP_IMAGES:
            ASSERT(privateState->getClientMajorVersion() >= 3);
            privateState->setUnpackSkipImages(param);
            break;

        case GL_UNPACK_SKIP_ROWS:
            ASSERT((privateState->getClientMajorVersion() >= 3) ||
                   privateState->getExtensions().unpackSubimageEXT);
            privateState->setUnpackSkipRows(param);
            break;

        case GL_UNPACK_SKIP_PIXELS:
            ASSERT((privateState->getClientMajorVersion() >= 3) ||
                   privateState->getExtensions().unpackSubimageEXT);
            privateState->setUnpackSkipPixels(param);
            break;

        case GL_PACK_ROW_LENGTH:
            ASSERT((privateState->getClientMajorVersion() >= 3) ||
                   privateState->getExtensions().packSubimageNV);
            privateState->setPackRowLength(param);
            break;

        case GL_PACK_SKIP_ROWS:
            ASSERT((privateState->getClientMajorVersion() >= 3) ||
                   privateState->getExtensions().packSubimageNV);
            privateState->setPackSkipRows(param);
            break;

        case GL_PACK_SKIP_PIXELS:
            ASSERT((privateState->getClientMajorVersion() >= 3) ||
                   privateState->getExtensions().packSubimageNV);
            privateState->setPackSkipPixels(param);
            break;

        default:
            UNREACHABLE();
            return;
    }
}

inline void ContextPrivateHint(PrivateState *privateState,
                               PrivateStateCache *privateStateCache,
                               GLenum target,
                               GLenum mode)
{
    switch (target)
    {
        case GL_GENERATE_MIPMAP_HINT:
            privateState->setGenerateMipmapHint(mode);
            break;

        case GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES:
            privateState->setFragmentShaderDerivativeHint(mode);
            break;

        case GL_PERSPECTIVE_CORRECTION_HINT:
        case GL_POINT_SMOOTH_HINT:
        case GL_LINE_SMOOTH_HINT:
        case GL_FOG_HINT:
            privateState->getMutableGLES1State()->setHint(target, mode);
            break;
        default:
            UNREACHABLE();
            return;
    }
}

inline GLboolean ContextPrivateIsEnabled(PrivateState *privateState,
                                         PrivateStateCache *privateStateCache,
                                         GLenum cap)
{
    return privateState->getEnableFeature(cap);
}

inline GLboolean ContextPrivateIsEnabledi(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLenum target,
                                          GLuint index)
{
    return privateState->getEnableFeatureIndexed(target, index);
}

inline void ContextPrivatePatchParameteri(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLenum pname,
                                          GLint value)
{
    switch (pname)
    {
        case GL_PATCH_VERTICES:
            privateState->setPatchVertices(value);
            break;
        default:
            break;
    }
}

inline void ContextPrivateAlphaFunc(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    AlphaTestFunc func,
                                    GLfloat ref)
{
    privateState->getMutableGLES1State()->setAlphaTestParameters(func, ref);
}

inline void ContextPrivateAlphaFuncx(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     AlphaTestFunc func,
                                     GLfixed ref)
{
    ContextPrivateAlphaFunc(privateState, privateStateCache, func, ConvertFixedToFloat(ref));
}

inline void ContextPrivateClipPlanef(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLenum p,
                                     const GLfloat *eqn)
{
    privateState->getMutableGLES1State()->setClipPlane(p - GL_CLIP_PLANE0, eqn);
}

inline void ContextPrivateClipPlanex(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLenum plane,
                                     const GLfixed *equation)
{
    const GLfloat equationf[4] = {
        ConvertFixedToFloat(equation[0]),
        ConvertFixedToFloat(equation[1]),
        ConvertFixedToFloat(equation[2]),
        ConvertFixedToFloat(equation[3]),
    };

    ContextPrivateClipPlanef(privateState, privateStateCache, plane, equationf);
}

inline void ContextPrivateColor4f(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  GLfloat red,
                                  GLfloat green,
                                  GLfloat blue,
                                  GLfloat alpha)
{
    privateState->getMutableGLES1State()->setCurrentColor({red, green, blue, alpha});
}

inline void ContextPrivateColor4ub(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   GLubyte red,
                                   GLubyte green,
                                   GLubyte blue,
                                   GLubyte alpha)
{
    ContextPrivateColor4f(privateState, privateStateCache, normalizedToFloat<uint8_t>(red),
                          normalizedToFloat<uint8_t>(green), normalizedToFloat<uint8_t>(blue),
                          normalizedToFloat<uint8_t>(alpha));
}

inline void ContextPrivateColor4x(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  GLfixed red,
                                  GLfixed green,
                                  GLfixed blue,
                                  GLfixed alpha)
{
    ContextPrivateColor4f(privateState, privateStateCache, ConvertFixedToFloat(red),
                          ConvertFixedToFloat(green), ConvertFixedToFloat(blue),
                          ConvertFixedToFloat(alpha));
}

inline void ContextPrivateFogf(PrivateState *privateState,
                               PrivateStateCache *privateStateCache,
                               GLenum pname,
                               GLfloat param)
{
    ContextPrivateFogfv(privateState, privateStateCache, pname, &param);
}

inline void ContextPrivateFogfv(PrivateState *privateState,
                                PrivateStateCache *privateStateCache,
                                GLenum pname,
                                const GLfloat *params)
{
    SetFogParameters(privateState->getMutableGLES1State(), pname, params);
}

inline void ContextPrivateFogx(PrivateState *privateState,
                               PrivateStateCache *privateStateCache,
                               GLenum pname,
                               GLfixed param)
{
    if (GetFogParameterCount(pname) == 1)
    {
        GLfloat paramf = pname == GL_FOG_MODE ? ConvertToGLenum(param) : ConvertFixedToFloat(param);
        ContextPrivateFogfv(privateState, privateStateCache, pname, &paramf);
    }
    else
    {
        UNREACHABLE();
    }
}

inline void ContextPrivateFogxv(PrivateState *privateState,
                                PrivateStateCache *privateStateCache,
                                GLenum pname,
                                const GLfixed *params)
{
    int paramCount = GetFogParameterCount(pname);

    if (paramCount > 0)
    {
        GLfloat paramsf[4];
        for (int i = 0; i < paramCount; i++)
        {
            paramsf[i] =
                pname == GL_FOG_MODE ? ConvertToGLenum(params[i]) : ConvertFixedToFloat(params[i]);
        }
        ContextPrivateFogfv(privateState, privateStateCache, pname, paramsf);
    }
    else
    {
        UNREACHABLE();
    }
}

inline void ContextPrivateFrustumf(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   GLfloat l,
                                   GLfloat r,
                                   GLfloat b,
                                   GLfloat t,
                                   GLfloat n,
                                   GLfloat f)
{
    privateState->getMutableGLES1State()->multMatrix(angle::Mat4::Frustum(l, r, b, t, n, f));
}

inline void ContextPrivateFrustumx(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   GLfixed l,
                                   GLfixed r,
                                   GLfixed b,
                                   GLfixed t,
                                   GLfixed n,
                                   GLfixed f)
{
    ContextPrivateFrustumf(privateState, privateStateCache, ConvertFixedToFloat(l),
                           ConvertFixedToFloat(r), ConvertFixedToFloat(b), ConvertFixedToFloat(t),
                           ConvertFixedToFloat(n), ConvertFixedToFloat(f));
}

inline void ContextPrivateGetClipPlanef(PrivateState *privateState,
                                        PrivateStateCache *privateStateCache,
                                        GLenum plane,
                                        GLfloat *equation)
{
    privateState->gles1().getClipPlane(plane - GL_CLIP_PLANE0, equation);
}

inline void ContextPrivateGetClipPlanex(PrivateState *privateState,
                                        PrivateStateCache *privateStateCache,
                                        GLenum plane,
                                        GLfixed *equation)
{
    GLfloat equationf[4] = {};

    ContextPrivateGetClipPlanef(privateState, privateStateCache, plane, equationf);

    for (int i = 0; i < 4; i++)
    {
        equation[i] = ConvertFloatToFixed(equationf[i]);
    }
}

inline void ContextPrivateGetLightfv(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLenum light,
                                     LightParameter pname,
                                     GLfloat *params)
{
    GetLightParameters(privateState->getMutableGLES1State(), light, pname, params);
}

inline void ContextPrivateGetLightxv(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLenum light,
                                     LightParameter pname,
                                     GLfixed *params)
{
    GLfloat paramsf[4];
    ContextPrivateGetLightfv(privateState, privateStateCache, light, pname, paramsf);

    for (unsigned int i = 0; i < GetLightParameterCount(pname); i++)
    {
        params[i] = ConvertFloatToFixed(paramsf[i]);
    }
}

inline void ContextPrivateGetMaterialfv(PrivateState *privateState,
                                        PrivateStateCache *privateStateCache,
                                        GLenum face,
                                        MaterialParameter pname,
                                        GLfloat *params)
{
    GetMaterialParameters(privateState->getMutableGLES1State(), face, pname, params);
}

inline void ContextPrivateGetMaterialxv(PrivateState *privateState,
                                        PrivateStateCache *privateStateCache,
                                        GLenum face,
                                        MaterialParameter pname,
                                        GLfixed *params)
{
    GLfloat paramsf[4];
    ContextPrivateGetMaterialfv(privateState, privateStateCache, face, pname, paramsf);

    for (unsigned int i = 0; i < GetMaterialParameterCount(pname); i++)
    {
        params[i] = ConvertFloatToFixed(paramsf[i]);
    }
}

inline void ContextPrivateGetTexEnvfv(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      TextureEnvTarget target,
                                      TextureEnvParameter pname,
                                      GLfloat *params)
{
    GetTextureEnv(privateState->getActiveSampler(), privateState->getMutableGLES1State(), target,
                  pname, params);
}

inline void ContextPrivateGetTexEnviv(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      TextureEnvTarget target,
                                      TextureEnvParameter pname,
                                      GLint *params)
{
    GLfloat paramsf[4];
    ContextPrivateGetTexEnvfv(privateState, privateStateCache, target, pname, paramsf);
    ConvertTextureEnvToInt(pname, paramsf, params);
}

inline void ContextPrivateGetTexEnvxv(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      TextureEnvTarget target,
                                      TextureEnvParameter pname,
                                      GLfixed *params)
{
    GLfloat paramsf[4];
    ContextPrivateGetTexEnvfv(privateState, privateStateCache, target, pname, paramsf);
    ConvertTextureEnvToFixed(pname, paramsf, params);
}

inline void ContextPrivateLightModelf(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLenum pname,
                                      GLfloat param)
{
    ContextPrivateLightModelfv(privateState, privateStateCache, pname, &param);
}

inline void ContextPrivateLightModelfv(PrivateState *privateState,
                                       PrivateStateCache *privateStateCache,
                                       GLenum pname,
                                       const GLfloat *params)
{
    SetLightModelParameters(privateState->getMutableGLES1State(), pname, params);
}

inline void ContextPrivateLightModelx(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      GLenum pname,
                                      GLfixed param)
{
    ContextPrivateLightModelf(privateState, privateStateCache, pname, ConvertFixedToFloat(param));
}

inline void ContextPrivateLightModelxv(PrivateState *privateState,
                                       PrivateStateCache *privateStateCache,
                                       GLenum pname,
                                       const GLfixed *param)
{
    GLfloat paramsf[4];

    for (unsigned int i = 0; i < GetLightModelParameterCount(pname); i++)
    {
        paramsf[i] = ConvertFixedToFloat(param[i]);
    }

    ContextPrivateLightModelfv(privateState, privateStateCache, pname, paramsf);
}

inline void ContextPrivateLightf(PrivateState *privateState,
                                 PrivateStateCache *privateStateCache,
                                 GLenum light,
                                 LightParameter pname,
                                 GLfloat param)
{
    ContextPrivateLightfv(privateState, privateStateCache, light, pname, &param);
}

inline void ContextPrivateLightfv(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  GLenum light,
                                  LightParameter pname,
                                  const GLfloat *params)
{
    SetLightParameters(privateState->getMutableGLES1State(), light, pname, params);
}

inline void ContextPrivateLightx(PrivateState *privateState,
                                 PrivateStateCache *privateStateCache,
                                 GLenum light,
                                 LightParameter pname,
                                 GLfixed param)
{
    ContextPrivateLightf(privateState, privateStateCache, light, pname, ConvertFixedToFloat(param));
}

inline void ContextPrivateLightxv(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  GLenum light,
                                  LightParameter pname,
                                  const GLfixed *params)
{
    GLfloat paramsf[4];

    for (unsigned int i = 0; i < GetLightParameterCount(pname); i++)
    {
        paramsf[i] = ConvertFixedToFloat(params[i]);
    }

    ContextPrivateLightfv(privateState, privateStateCache, light, pname, paramsf);
}

inline void ContextPrivateLoadIdentity(PrivateState *privateState,
                                       PrivateStateCache *privateStateCache)
{
    privateState->getMutableGLES1State()->loadMatrix(angle::Mat4());
}

inline void ContextPrivateLoadMatrixf(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      const GLfloat *m)
{
    privateState->getMutableGLES1State()->loadMatrix(angle::Mat4(m));
}

inline void ContextPrivateLoadMatrixx(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      const GLfixed *m)
{
    privateState->getMutableGLES1State()->loadMatrix(FixedMatrixToMat4(m));
}

inline void ContextPrivateMaterialf(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLenum face,
                                    MaterialParameter pname,
                                    GLfloat param)
{
    ContextPrivateMaterialfv(privateState, privateStateCache, face, pname, &param);
}

inline void ContextPrivateMaterialfv(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLenum face,
                                     MaterialParameter pname,
                                     const GLfloat *params)
{
    SetMaterialParameters(privateState->getMutableGLES1State(), face, pname, params);
}

inline void ContextPrivateMaterialx(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLenum face,
                                    MaterialParameter pname,
                                    GLfixed param)
{
    ContextPrivateMaterialf(privateState, privateStateCache, face, pname,
                            ConvertFixedToFloat(param));
}

inline void ContextPrivateMaterialxv(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLenum face,
                                     MaterialParameter pname,
                                     const GLfixed *param)
{
    GLfloat paramsf[4];

    for (unsigned int i = 0; i < GetMaterialParameterCount(pname); i++)
    {
        paramsf[i] = ConvertFixedToFloat(param[i]);
    }

    ContextPrivateMaterialfv(privateState, privateStateCache, face, pname, paramsf);
}

inline void ContextPrivateMatrixMode(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     MatrixType mode)
{
    privateState->getMutableGLES1State()->setMatrixMode(mode);
}

inline void ContextPrivateMultMatrixf(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      const GLfloat *m)
{
    privateState->getMutableGLES1State()->multMatrix(angle::Mat4(m));
}

inline void ContextPrivateMultMatrixx(PrivateState *privateState,
                                      PrivateStateCache *privateStateCache,
                                      const GLfixed *m)
{
    privateState->getMutableGLES1State()->multMatrix(FixedMatrixToMat4(m));
}

inline void ContextPrivateMultiTexCoord4f(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLenum target,
                                          GLfloat s,
                                          GLfloat t,
                                          GLfloat r,
                                          GLfloat q)
{
    unsigned int unit = target - GL_TEXTURE0;
    ASSERT(target >= GL_TEXTURE0 && unit < privateState->getCaps().maxMultitextureUnits);
    privateState->getMutableGLES1State()->setCurrentTextureCoords(unit, {s, t, r, q});
}

inline void ContextPrivateMultiTexCoord4x(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          GLenum texture,
                                          GLfixed s,
                                          GLfixed t,
                                          GLfixed r,
                                          GLfixed q)
{
    ContextPrivateMultiTexCoord4f(privateState, privateStateCache, texture, ConvertFixedToFloat(s),
                                  ConvertFixedToFloat(t), ConvertFixedToFloat(r),
                                  ConvertFixedToFloat(q));
}

inline void ContextPrivateNormal3f(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   GLfloat nx,
                                   GLfloat ny,
                                   GLfloat nz)
{
    privateState->getMutableGLES1State()->setCurrentNormal({nx, ny, nz});
}

inline void ContextPrivateNormal3x(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   GLfixed nx,
                                   GLfixed ny,
                                   GLfixed nz)
{
    ContextPrivateNormal3f(privateState, privateStateCache, ConvertFixedToFloat(nx),
                           ConvertFixedToFloat(ny), ConvertFixedToFloat(nz));
}

inline void ContextPrivateOrthof(PrivateState *privateState,
                                 PrivateStateCache *privateStateCache,
                                 GLfloat left,
                                 GLfloat right,
                                 GLfloat bottom,
                                 GLfloat top,
                                 GLfloat zNear,
                                 GLfloat zFar)
{
    privateState->getMutableGLES1State()->multMatrix(
        angle::Mat4::Ortho(left, right, bottom, top, zNear, zFar));
}

inline void ContextPrivateOrthox(PrivateState *privateState,
                                 PrivateStateCache *privateStateCache,
                                 GLfixed left,
                                 GLfixed right,
                                 GLfixed bottom,
                                 GLfixed top,
                                 GLfixed zNear,
                                 GLfixed zFar)
{
    ContextPrivateOrthof(privateState, privateStateCache, ConvertFixedToFloat(left),
                         ConvertFixedToFloat(right), ConvertFixedToFloat(bottom),
                         ConvertFixedToFloat(top), ConvertFixedToFloat(zNear),
                         ConvertFixedToFloat(zFar));
}

inline void ContextPrivatePointParameterf(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          PointParameter pname,
                                          GLfloat param)
{
    ContextPrivatePointParameterfv(privateState, privateStateCache, pname, &param);
}

inline void ContextPrivatePointParameterfv(PrivateState *privateState,
                                           PrivateStateCache *privateStateCache,
                                           PointParameter pname,
                                           const GLfloat *params)
{
    SetPointParameter(privateState->getMutableGLES1State(), pname, params);
}

inline void ContextPrivatePointParameterx(PrivateState *privateState,
                                          PrivateStateCache *privateStateCache,
                                          PointParameter pname,
                                          GLfixed param)
{
    ContextPrivatePointParameterf(privateState, privateStateCache, pname,
                                  ConvertFixedToFloat(param));
}

inline void ContextPrivatePointParameterxv(PrivateState *privateState,
                                           PrivateStateCache *privateStateCache,
                                           PointParameter pname,
                                           const GLfixed *params)
{
    GLfloat paramsf[4] = {};
    for (unsigned int i = 0; i < GetPointParameterCount(pname); i++)
    {
        paramsf[i] = ConvertFixedToFloat(params[i]);
    }
    ContextPrivatePointParameterfv(privateState, privateStateCache, pname, paramsf);
}

inline void ContextPrivatePointSize(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache,
                                    GLfloat size)
{
    SetPointSize(privateState->getMutableGLES1State(), size);
}

inline void ContextPrivatePointSizex(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLfixed size)
{
    ContextPrivatePointSize(privateState, privateStateCache, ConvertFixedToFloat(size));
}

inline void ContextPrivatePopMatrix(PrivateState *privateState,
                                    PrivateStateCache *privateStateCache)
{
    privateState->getMutableGLES1State()->popMatrix();
}

inline void ContextPrivatePushMatrix(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache)
{
    privateState->getMutableGLES1State()->pushMatrix();
}

inline void ContextPrivateRotatef(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  GLfloat angle,
                                  GLfloat x,
                                  GLfloat y,
                                  GLfloat z)
{
    privateState->getMutableGLES1State()->multMatrix(
        angle::Mat4::Rotate(angle, angle::Vector3(x, y, z)));
}

inline void ContextPrivateRotatex(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  GLfixed angle,
                                  GLfixed x,
                                  GLfixed y,
                                  GLfixed z)
{
    ContextPrivateRotatef(privateState, privateStateCache, ConvertFixedToFloat(angle),
                          ConvertFixedToFloat(x), ConvertFixedToFloat(y), ConvertFixedToFloat(z));
}

inline void ContextPrivateScalef(PrivateState *privateState,
                                 PrivateStateCache *privateStateCache,
                                 GLfloat x,
                                 GLfloat y,
                                 GLfloat z)
{
    privateState->getMutableGLES1State()->multMatrix(angle::Mat4::Scale(angle::Vector3(x, y, z)));
}

inline void ContextPrivateScalex(PrivateState *privateState,
                                 PrivateStateCache *privateStateCache,
                                 GLfixed x,
                                 GLfixed y,
                                 GLfixed z)
{
    ContextPrivateScalef(privateState, privateStateCache, ConvertFixedToFloat(x),
                         ConvertFixedToFloat(y), ConvertFixedToFloat(z));
}

inline void ContextPrivateShadeModel(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     ShadingModel model)
{
    privateState->getMutableGLES1State()->setShadeModel(model);
}

inline void ContextPrivateTexEnvf(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  TextureEnvTarget target,
                                  TextureEnvParameter pname,
                                  GLfloat param)
{
    ContextPrivateTexEnvfv(privateState, privateStateCache, target, pname, &param);
}

inline void ContextPrivateTexEnvfv(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   TextureEnvTarget target,
                                   TextureEnvParameter pname,
                                   const GLfloat *params)
{
    SetTextureEnv(privateState->getActiveSampler(), privateState->getMutableGLES1State(), target,
                  pname, params);
}

inline void ContextPrivateTexEnvi(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  TextureEnvTarget target,
                                  TextureEnvParameter pname,
                                  GLint param)
{
    ContextPrivateTexEnviv(privateState, privateStateCache, target, pname, &param);
}

inline void ContextPrivateTexEnviv(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   TextureEnvTarget target,
                                   TextureEnvParameter pname,
                                   const GLint *params)
{
    GLfloat paramsf[4] = {};
    ConvertTextureEnvFromInt(pname, params, paramsf);
    ContextPrivateTexEnvfv(privateState, privateStateCache, target, pname, paramsf);
}

inline void ContextPrivateTexEnvx(PrivateState *privateState,
                                  PrivateStateCache *privateStateCache,
                                  TextureEnvTarget target,
                                  TextureEnvParameter pname,
                                  GLfixed param)
{
    ContextPrivateTexEnvxv(privateState, privateStateCache, target, pname, &param);
}

inline void ContextPrivateTexEnvxv(PrivateState *privateState,
                                   PrivateStateCache *privateStateCache,
                                   TextureEnvTarget target,
                                   TextureEnvParameter pname,
                                   const GLfixed *params)
{
    GLfloat paramsf[4] = {};
    ConvertTextureEnvFromFixed(pname, params, paramsf);
    ContextPrivateTexEnvfv(privateState, privateStateCache, target, pname, paramsf);
}

inline void ContextPrivateTranslatef(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLfloat x,
                                     GLfloat y,
                                     GLfloat z)
{
    privateState->getMutableGLES1State()->multMatrix(
        angle::Mat4::Translate(angle::Vector3(x, y, z)));
}

inline void ContextPrivateTranslatex(PrivateState *privateState,
                                     PrivateStateCache *privateStateCache,
                                     GLfixed x,
                                     GLfixed y,
                                     GLfixed z)
{
    ContextPrivateTranslatef(privateState, privateStateCache, ConvertFixedToFloat(x),
                             ConvertFixedToFloat(y), ConvertFixedToFloat(z));
}
}  // namespace gl
