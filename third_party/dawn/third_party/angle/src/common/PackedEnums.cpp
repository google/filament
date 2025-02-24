// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PackedGLEnums.cpp:
//   Declares ANGLE-specific enums classes for GLEnum and functions operating
//   on them.

#include "common/PackedEnums.h"

#include "common/utilities.h"

namespace gl
{

TextureType TextureTargetToType(TextureTarget target)
{
    switch (target)
    {
        case TextureTarget::CubeMapNegativeX:
        case TextureTarget::CubeMapNegativeY:
        case TextureTarget::CubeMapNegativeZ:
        case TextureTarget::CubeMapPositiveX:
        case TextureTarget::CubeMapPositiveY:
        case TextureTarget::CubeMapPositiveZ:
            return TextureType::CubeMap;
        case TextureTarget::CubeMapArray:
            return TextureType::CubeMapArray;
        case TextureTarget::External:
            return TextureType::External;
        case TextureTarget::Rectangle:
            return TextureType::Rectangle;
        case TextureTarget::_2D:
            return TextureType::_2D;
        case TextureTarget::_2DArray:
            return TextureType::_2DArray;
        case TextureTarget::_2DMultisample:
            return TextureType::_2DMultisample;
        case TextureTarget::_2DMultisampleArray:
            return TextureType::_2DMultisampleArray;
        case TextureTarget::_3D:
            return TextureType::_3D;
        case TextureTarget::VideoImage:
            return TextureType::VideoImage;
        case TextureTarget::Buffer:
            return TextureType::Buffer;
        case TextureTarget::InvalidEnum:
            return TextureType::InvalidEnum;
        default:
            UNREACHABLE();
            return TextureType::InvalidEnum;
    }
}

bool IsCubeMapFaceTarget(TextureTarget target)
{
    return TextureTargetToType(target) == TextureType::CubeMap;
}

TextureTarget NonCubeTextureTypeToTarget(TextureType type)
{
    switch (type)
    {
        case TextureType::External:
            return TextureTarget::External;
        case TextureType::Rectangle:
            return TextureTarget::Rectangle;
        case TextureType::_2D:
            return TextureTarget::_2D;
        case TextureType::_2DArray:
            return TextureTarget::_2DArray;
        case TextureType::_2DMultisample:
            return TextureTarget::_2DMultisample;
        case TextureType::_2DMultisampleArray:
            return TextureTarget::_2DMultisampleArray;
        case TextureType::_3D:
            return TextureTarget::_3D;
        case TextureType::CubeMapArray:
            return TextureTarget::CubeMapArray;
        case TextureType::VideoImage:
            return TextureTarget::VideoImage;
        case TextureType::Buffer:
            return TextureTarget::Buffer;
        default:
            UNREACHABLE();
            return TextureTarget::InvalidEnum;
    }
}

// Check that we can do arithmetic on TextureTarget to convert from / to cube map faces
static_assert(static_cast<uint8_t>(TextureTarget::CubeMapNegativeX) -
                      static_cast<uint8_t>(TextureTarget::CubeMapPositiveX) ==
                  1u,
              "");
static_assert(static_cast<uint8_t>(TextureTarget::CubeMapPositiveY) -
                      static_cast<uint8_t>(TextureTarget::CubeMapPositiveX) ==
                  2u,
              "");
static_assert(static_cast<uint8_t>(TextureTarget::CubeMapNegativeY) -
                      static_cast<uint8_t>(TextureTarget::CubeMapPositiveX) ==
                  3u,
              "");
static_assert(static_cast<uint8_t>(TextureTarget::CubeMapPositiveZ) -
                      static_cast<uint8_t>(TextureTarget::CubeMapPositiveX) ==
                  4u,
              "");
static_assert(static_cast<uint8_t>(TextureTarget::CubeMapNegativeZ) -
                      static_cast<uint8_t>(TextureTarget::CubeMapPositiveX) ==
                  5u,
              "");

TextureTarget CubeFaceIndexToTextureTarget(size_t face)
{
    ASSERT(face < 6u);
    return static_cast<TextureTarget>(static_cast<uint8_t>(TextureTarget::CubeMapPositiveX) + face);
}

size_t CubeMapTextureTargetToFaceIndex(TextureTarget target)
{
    ASSERT(IsCubeMapFaceTarget(target));
    return static_cast<uint8_t>(target) - static_cast<uint8_t>(TextureTarget::CubeMapPositiveX);
}

TextureType SamplerTypeToTextureType(GLenum samplerType)
{
    switch (samplerType)
    {
        case GL_SAMPLER_2D:
        case GL_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_SAMPLER_2D_SHADOW:
            return TextureType::_2D;

        case GL_SAMPLER_EXTERNAL_OES:
        case GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT:
            return TextureType::External;

        case GL_SAMPLER_CUBE:
        case GL_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_SHADOW:
            return TextureType::CubeMap;

        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
            return TextureType::CubeMapArray;

        case GL_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
            return TextureType::_2DArray;

        case GL_SAMPLER_3D:
        case GL_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
            return TextureType::_3D;

        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
            return TextureType::_2DMultisample;

        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
            return TextureType::_2DMultisampleArray;

        case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
            return TextureType::Buffer;

        case GL_SAMPLER_2D_RECT_ANGLE:
            return TextureType::Rectangle;

        case GL_SAMPLER_VIDEO_IMAGE_WEBGL:
            return TextureType::VideoImage;

        default:
            UNREACHABLE();
            return TextureType::InvalidEnum;
    }
}

TextureType ImageTypeToTextureType(GLenum imageType)
{
    switch (imageType)
    {
        case GL_IMAGE_2D:
        case GL_INT_IMAGE_2D:
        case GL_UNSIGNED_INT_IMAGE_2D:
            return TextureType::_2D;

        case GL_IMAGE_CUBE:
        case GL_INT_IMAGE_CUBE:
        case GL_UNSIGNED_INT_IMAGE_CUBE:
            return TextureType::CubeMap;

        case GL_IMAGE_CUBE_MAP_ARRAY:
        case GL_INT_IMAGE_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
            return TextureType::CubeMapArray;

        case GL_IMAGE_2D_ARRAY:
        case GL_INT_IMAGE_2D_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
            return TextureType::_2DArray;

        case GL_IMAGE_3D:
        case GL_INT_IMAGE_3D:
        case GL_UNSIGNED_INT_IMAGE_3D:
            return TextureType::_3D;

        case GL_IMAGE_BUFFER:
        case GL_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_IMAGE_BUFFER:
            return TextureType::Buffer;

        default:
            UNREACHABLE();
            return TextureType::InvalidEnum;
    }
}

bool IsMultisampled(TextureType type)
{
    switch (type)
    {
        case TextureType::_2DMultisample:
        case TextureType::_2DMultisampleArray:
            return true;
        default:
            return false;
    }
}

bool IsArrayTextureType(TextureType type)
{
    switch (type)
    {
        case TextureType::_2DArray:
        case TextureType::_2DMultisampleArray:
        case TextureType::CubeMapArray:
            return true;
        default:
            return false;
    }
}

bool IsLayeredTextureType(TextureType type)
{
    switch (type)
    {
        case TextureType::_2DArray:
        case TextureType::_2DMultisampleArray:
        case TextureType::_3D:
        case TextureType::CubeMap:
        case TextureType::CubeMapArray:
            return true;
        default:
            return false;
    }
}

bool IsStaticBufferUsage(BufferUsage useage)
{
    switch (useage)
    {
        case BufferUsage::StaticCopy:
        case BufferUsage::StaticDraw:
        case BufferUsage::StaticRead:
            return true;
        default:
            return false;
    }
}

std::ostream &operator<<(std::ostream &os, PrimitiveMode value)
{
    switch (value)
    {
        case PrimitiveMode::LineLoop:
            os << "GL_LINE_LOOP";
            break;
        case PrimitiveMode::Lines:
            os << "GL_LINES";
            break;
        case PrimitiveMode::LinesAdjacency:
            os << "GL_LINES_ADJACENCY";
            break;
        case PrimitiveMode::LineStrip:
            os << "GL_LINE_STRIP";
            break;
        case PrimitiveMode::LineStripAdjacency:
            os << "GL_LINE_STRIP_ADJANCENCY";
            break;
        case PrimitiveMode::Patches:
            os << "GL_PATCHES";
            break;
        case PrimitiveMode::Points:
            os << "GL_POINTS";
            break;
        case PrimitiveMode::TriangleFan:
            os << "GL_TRIANGLE_FAN";
            break;
        case PrimitiveMode::Triangles:
            os << "GL_TRIANGLES";
            break;
        case PrimitiveMode::TrianglesAdjacency:
            os << "GL_TRIANGLES_ADJANCENCY";
            break;
        case PrimitiveMode::TriangleStrip:
            os << "GL_TRIANGLE_STRIP";
            break;
        case PrimitiveMode::TriangleStripAdjacency:
            os << "GL_TRIANGLE_STRIP_ADJACENCY";
            break;
        default:
            os << "GL_INVALID_ENUM";
            break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, DrawElementsType value)
{
    switch (value)
    {
        case DrawElementsType::UnsignedByte:
            os << "GL_UNSIGNED_BYTE";
            break;
        case DrawElementsType::UnsignedShort:
            os << "GL_UNSIGNED_SHORT";
            break;
        case DrawElementsType::UnsignedInt:
            os << "GL_UNSIGNED_INT";
            break;
        default:
            os << "GL_INVALID_ENUM";
            break;
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, BlendEquationType value)
{
    switch (value)
    {
        case BlendEquationType::Add:
            os << "GL_FUNC_ADD";
            break;
        case BlendEquationType::Min:
            os << "GL_MIN";
            break;
        case BlendEquationType::Max:
            os << "GL_MAX";
            break;
        case BlendEquationType::Subtract:
            os << "GL_FUNC_SUBTRACT";
            break;
        case BlendEquationType::ReverseSubtract:
            os << "GL_FUNC_REVERSE_SUBTRACT";
            break;
        case BlendEquationType::Multiply:
            os << "GL_MULTIPLY_KHR";
            break;
        case BlendEquationType::Screen:
            os << "GL_SCREEN_KHR";
            break;
        case BlendEquationType::Overlay:
            os << "GL_OVERLAY_KHR";
            break;
        case BlendEquationType::Darken:
            os << "GL_DARKEN_KHR";
            break;
        case BlendEquationType::Lighten:
            os << "GL_LIGHTEN_KHR";
            break;
        case BlendEquationType::Colordodge:
            os << "GL_COLORDODGE_KHR";
            break;
        case BlendEquationType::Colorburn:
            os << "GL_COLORBURN_KHR";
            break;
        case BlendEquationType::Hardlight:
            os << "GL_HARDLIGHT_KHR";
            break;
        case BlendEquationType::Softlight:
            os << "GL_SOFTLIGHT_KHR";
            break;
        case BlendEquationType::Difference:
            os << "GL_DIFFERENCE_KHR";
            break;
        case BlendEquationType::Exclusion:
            os << "GL_EXCLUSION_KHR";
            break;
        case BlendEquationType::HslHue:
            os << "GL_HSL_HUE_KHR";
            break;
        case BlendEquationType::HslSaturation:
            os << "GL_HSL_SATURATION_KHR";
            break;
        case BlendEquationType::HslColor:
            os << "GL_HSL_COLOR_KHR";
            break;
        case BlendEquationType::HslLuminosity:
            os << "GL_HSL_LUMINOSITY_KHR";
            break;
        default:
            os << "GL_INVALID_ENUM";
            break;
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, BlendFactorType value)
{
    switch (value)
    {
        case BlendFactorType::Zero:
            os << "GL_ZERO";
            break;
        case BlendFactorType::One:
            os << "GL_ONE";
            break;
        case BlendFactorType::SrcColor:
            os << "GL_SRC_COLOR";
            break;
        case BlendFactorType::OneMinusSrcColor:
            os << "GL_ONE_MINUS_SRC_COLOR";
            break;
        case BlendFactorType::SrcAlpha:
            os << "GL_SRC_ALPHA";
            break;
        case BlendFactorType::OneMinusSrcAlpha:
            os << "GL_ONE_MINUS_SRC_ALPHA";
            break;
        case BlendFactorType::DstAlpha:
            os << "GL_DST_ALPHA";
            break;
        case BlendFactorType::OneMinusDstAlpha:
            os << "GL_ONE_MINUS_DST_ALPHA";
            break;
        case BlendFactorType::DstColor:
            os << "GL_DST_COLOR";
            break;
        case BlendFactorType::OneMinusDstColor:
            os << "GL_ONE_MINUS_DST_COLOR";
            break;
        case BlendFactorType::SrcAlphaSaturate:
            os << "GL_SRC_ALPHA_SATURATE";
            break;
        case BlendFactorType::ConstantColor:
            os << "GL_CONSTANT_COLOR";
            break;
        case BlendFactorType::OneMinusConstantColor:
            os << "GL_ONE_MINUS_CONSTANT_COLOR";
            break;
        case BlendFactorType::ConstantAlpha:
            os << "GL_CONSTANT_ALPHA";
            break;
        case BlendFactorType::OneMinusConstantAlpha:
            os << "GL_ONE_MINUS_CONSTANT_ALPHA";
            break;
        case BlendFactorType::Src1Alpha:
            os << "GL_SRC1_ALPHA_EXT";
            break;
        case BlendFactorType::Src1Color:
            os << "GL_SRC1_COLOR_EXT";
            break;
        case BlendFactorType::OneMinusSrc1Color:
            os << "GL_ONE_MINUS_SRC1_COLOR_EXT";
            break;
        case BlendFactorType::OneMinusSrc1Alpha:
            os << "GL_ONE_MINUS_SRC1_ALPHA_EXT";
            break;
        default:
            os << "GL_INVALID_ENUM";
            break;
    }

    return os;
}

std::ostream &operator<<(std::ostream &os, VertexAttribType value)
{
    switch (value)
    {
        case VertexAttribType::Byte:
            os << "GL_BYTE";
            break;
        case VertexAttribType::Fixed:
            os << "GL_FIXED";
            break;
        case VertexAttribType::Float:
            os << "GL_FLOAT";
            break;
        case VertexAttribType::HalfFloat:
            os << "GL_HALF_FLOAT";
            break;
        case VertexAttribType::HalfFloatOES:
            os << "GL_HALF_FLOAT_OES";
            break;
        case VertexAttribType::Int:
            os << "GL_INT";
            break;
        case VertexAttribType::Int2101010:
            os << "GL_INT_2_10_10_10_REV";
            break;
        case VertexAttribType::Int1010102:
            os << "GL_INT_10_10_10_2_OES";
            break;
        case VertexAttribType::Short:
            os << "GL_SHORT";
            break;
        case VertexAttribType::UnsignedByte:
            os << "GL_UNSIGNED_BYTE";
            break;
        case VertexAttribType::UnsignedInt:
            os << "GL_UNSIGNED_INT";
            break;
        case VertexAttribType::UnsignedInt2101010:
            os << "GL_UNSIGNED_INT_2_10_10_10_REV";
            break;
        case VertexAttribType::UnsignedInt1010102:
            os << "GL_UNSIGNED_INT_10_10_10_2_OES";
            break;
        case VertexAttribType::UnsignedShort:
            os << "GL_UNSIGNED_SHORT";
            break;
        default:
            os << "GL_INVALID_ENUM";
            break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, TessEvaluationType value)
{
    switch (value)
    {
        case TessEvaluationType::Triangles:
            os << "GL_TRIANGLES";
            break;
        case TessEvaluationType::Quads:
            os << "GL_QUADS";
            break;
        case TessEvaluationType::Isolines:
            os << "GL_ISOLINES";
            break;
        case TessEvaluationType::EqualSpacing:
            os << "GL_EQUAL";
            break;
        case TessEvaluationType::FractionalEvenSpacing:
            os << "GL_FRACTIONAL_EVEN";
            break;
        case TessEvaluationType::FractionalOddSpacing:
            os << "GL_FRACTIONAL_ODD";
            break;
        case TessEvaluationType::Cw:
            os << "GL_CW";
            break;
        case TessEvaluationType::Ccw:
            os << "GL_CCW";
            break;
        case TessEvaluationType::PointMode:
            os << "GL_TESS_GEN_POINT_MODE";
            break;
        default:
            os << "GL_INVALID_ENUM";
            break;
    }
    return os;
}

const char *ShaderTypeToString(ShaderType shaderType)
{
    constexpr ShaderMap<const char *> kShaderTypeNameMap = {
        {ShaderType::Vertex, "Vertex"},
        {ShaderType::TessControl, "Tessellation control"},
        {ShaderType::TessEvaluation, "Tessellation evaluation"},
        {ShaderType::Geometry, "Geometry"},
        {ShaderType::Fragment, "Fragment"},
        {ShaderType::Compute, "Compute"}};
    return kShaderTypeNameMap[shaderType];
}

bool operator<(const UniformLocation &lhs, const UniformLocation &rhs)
{
    return lhs.value < rhs.value;
}

bool IsEmulatedCompressedFormat(GLenum format)
{
    // TODO(anglebug.com/42264702): Check for all formats ANGLE will use to emulate a compressed
    // texture
    return format == GL_RGBA || format == GL_RG || format == GL_RED;
}
}  // namespace gl

namespace egl
{
MessageType ErrorCodeToMessageType(EGLint errorCode)
{
    switch (errorCode)
    {
        case EGL_BAD_ALLOC:
        case EGL_CONTEXT_LOST:
        case EGL_NOT_INITIALIZED:
            return MessageType::Critical;

        case EGL_BAD_ACCESS:
        case EGL_BAD_ATTRIBUTE:
        case EGL_BAD_CONFIG:
        case EGL_BAD_CONTEXT:
        case EGL_BAD_CURRENT_SURFACE:
        case EGL_BAD_DISPLAY:
        case EGL_BAD_MATCH:
        case EGL_BAD_NATIVE_PIXMAP:
        case EGL_BAD_NATIVE_WINDOW:
        case EGL_BAD_PARAMETER:
        case EGL_BAD_SURFACE:
        case EGL_BAD_STREAM_KHR:
        case EGL_BAD_STATE_KHR:
        case EGL_BAD_DEVICE_EXT:
            return MessageType::Error;

        case EGL_SUCCESS:
        default:
            UNREACHABLE();
            return MessageType::InvalidEnum;
    }
}
}  // namespace egl

namespace egl_gl
{

gl::TextureTarget EGLCubeMapTargetToCubeMapTarget(EGLenum eglTarget)
{
    ASSERT(egl::IsCubeMapTextureTarget(eglTarget));
    return gl::CubeFaceIndexToTextureTarget(egl::CubeMapTextureTargetToLayerIndex(eglTarget));
}

gl::TextureTarget EGLImageTargetToTextureTarget(EGLenum eglTarget)
{
    switch (eglTarget)
    {
        case EGL_GL_TEXTURE_2D_KHR:
            return gl::TextureTarget::_2D;

        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
            return EGLCubeMapTargetToCubeMapTarget(eglTarget);

        case EGL_GL_TEXTURE_3D_KHR:
            return gl::TextureTarget::_3D;

        default:
            UNREACHABLE();
            return gl::TextureTarget::InvalidEnum;
    }
}

gl::TextureType EGLTextureTargetToTextureType(EGLenum eglTarget)
{
    switch (eglTarget)
    {
        case EGL_TEXTURE_2D:
            return gl::TextureType::_2D;

        case EGL_TEXTURE_RECTANGLE_ANGLE:
            return gl::TextureType::Rectangle;

        default:
            UNREACHABLE();
            return gl::TextureType::InvalidEnum;
    }
}
}  // namespace egl_gl
