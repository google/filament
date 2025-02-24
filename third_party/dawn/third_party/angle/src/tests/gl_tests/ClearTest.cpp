//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <variant>

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"
#include "util/random_utils.h"
#include "util/shader_utils.h"
#include "util/test_utils.h"

using namespace angle;

namespace
{

struct FormatTableElement
{
    GLint internalformat;
    GLenum format;
    GLenum type;

    bool isInt() const
    {
        return format == GL_RED_INTEGER || format == GL_RG_INTEGER || format == GL_RGB_INTEGER ||
               format == GL_RGBA_INTEGER;
    }

    // Call isUInt only if isInt is true.
    bool isUInt() const
    {
        return type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT || type == GL_UNSIGNED_INT;
    }
};

using ColorTypes = std::variant<GLColor,
                                GLColorT<int8_t>,
                                GLColor16,
                                GLColorT<int16_t>,
                                GLColor32F,
                                GLColor32I,
                                GLColor32UI,
                                GLushort,
                                GLuint,
                                GLint>;
class ClearTestBase : public ANGLETest<>
{
  protected:
    ClearTestBase()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    void testSetUp() override
    {
        mFBOs.resize(2, 0);
        glGenFramebuffers(2, mFBOs.data());

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        if (!mFBOs.empty())
        {
            glDeleteFramebuffers(static_cast<GLsizei>(mFBOs.size()), mFBOs.data());
        }

        if (!mTextures.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(mTextures.size()), mTextures.data());
        }
    }

    std::vector<GLuint> mFBOs;
    std::vector<GLuint> mTextures;
};

class ClearTest : public ClearTestBase
{};

class ClearTestES3 : public ClearTestBase
{
  protected:
    void verifyDepth(float depthValue, uint32_t size)
    {
        // Use a small shader to verify depth.
        ANGLE_GL_PROGRAM(depthTestProgram, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Blue());
        ANGLE_GL_PROGRAM(depthTestProgramFail, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Red());

        GLboolean hasDepthTest  = GL_FALSE;
        GLboolean hasDepthWrite = GL_TRUE;
        GLint prevDepthFunc     = GL_ALWAYS;

        glGetBooleanv(GL_DEPTH_TEST, &hasDepthTest);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &hasDepthWrite);
        glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);

        if (!hasDepthTest)
        {
            glEnable(GL_DEPTH_TEST);
        }
        if (hasDepthWrite)
        {
            glDepthMask(GL_FALSE);
        }
        glDepthFunc(GL_LESS);
        drawQuad(depthTestProgram, essl1_shaders::PositionAttrib(), depthValue * 2 - 1 - 0.01f);
        drawQuad(depthTestProgramFail, essl1_shaders::PositionAttrib(), depthValue * 2 - 1 + 0.01f);
        if (!hasDepthTest)
        {
            glDisable(GL_DEPTH_TEST);
        }
        if (hasDepthWrite)
        {
            glDepthMask(GL_TRUE);
        }
        glDepthFunc(prevDepthFunc);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::blue, 1);
        EXPECT_PIXEL_COLOR_NEAR(size - 1, 0, GLColor::blue, 1);
        EXPECT_PIXEL_COLOR_NEAR(0, size - 1, GLColor::blue, 1);
        EXPECT_PIXEL_COLOR_NEAR(size - 1, size - 1, GLColor::blue, 1);
    }

    void verifyStencil(uint32_t stencilValue, uint32_t size)
    {
        // Use another small shader to verify stencil.
        ANGLE_GL_PROGRAM(stencilTestProgram, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Green());
        GLboolean hasStencilTest   = GL_FALSE;
        GLint prevStencilFunc      = GL_ALWAYS;
        GLint prevStencilValue     = 0xFF;
        GLint prevStencilRef       = 0xFF;
        GLint prevStencilFail      = GL_KEEP;
        GLint prevStencilDepthFail = GL_KEEP;
        GLint prevStencilDepthPass = GL_KEEP;

        glGetBooleanv(GL_STENCIL_TEST, &hasStencilTest);
        glGetIntegerv(GL_STENCIL_FUNC, &prevStencilFunc);
        glGetIntegerv(GL_STENCIL_VALUE_MASK, &prevStencilValue);
        glGetIntegerv(GL_STENCIL_REF, &prevStencilRef);
        glGetIntegerv(GL_STENCIL_FAIL, &prevStencilFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &prevStencilDepthFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &prevStencilDepthPass);

        if (!hasStencilTest)
        {
            glEnable(GL_STENCIL_TEST);
        }
        glStencilFunc(GL_EQUAL, stencilValue, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        drawQuad(stencilTestProgram, essl1_shaders::PositionAttrib(), 0.0f);
        if (!hasStencilTest)
        {
            glDisable(GL_STENCIL_TEST);
        }
        glStencilFunc(prevStencilFunc, prevStencilValue, prevStencilRef);
        glStencilOp(prevStencilFail, prevStencilDepthFail, prevStencilDepthPass);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::green, 1);
        EXPECT_PIXEL_COLOR_NEAR(size - 1, 0, GLColor::green, 1);
        EXPECT_PIXEL_COLOR_NEAR(0, size - 1, GLColor::green, 1);
        EXPECT_PIXEL_COLOR_NEAR(size - 1, size - 1, GLColor::green, 1);
    }
};

class ClearTestES31 : public ClearTestES3
{};

class ClearTestRGB : public ANGLETest<>
{
  protected:
    ClearTestRGB()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
    }
};

class ClearTestRGB_ES3 : public ClearTestRGB
{};

class ClearTextureEXTTest : public ANGLETest<>
{
  protected:
    ClearTextureEXTTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class ClearTextureEXTTestES31 : public ANGLETest<>
{
  protected:
    ClearTextureEXTTestES31()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        angle::RNG rng;
        mWidth  = rng.randomIntBetween(1, 32);
        mHeight = rng.randomIntBetween(1, 32);
        mDepth  = rng.randomIntBetween(1, 4);

        mLevels = std::log2(std::max(mWidth, mHeight)) + 1;
    }

    // Texture's information.
    int mWidth;
    int mHeight;
    int mDepth;
    int mLevels;

    GLenum mTarget;

    bool mIsArray      = true;
    bool mHasLayer     = true;
    bool mExtraSupport = true;

    GLColor mFullColorRef    = GLColor::red;
    GLColor mPartialColorRef = GLColor::blue;

    // Convert the reference color so that it can be suitable for different type of format.
    ColorTypes convertColorTypeInternal(const GLenum &type, const GLColor &color);
    ColorTypes convertColorType(const GLenum &format, const GLenum &type, GLColor color);
    GLColor getClearColor(GLenum format, GLColor full);

    // Check color results by format type.
    void colorCheckType(const FormatTableElement &fmt,
                        const int &x,
                        const int &y,
                        const int &width,
                        const int &height,
                        GLColor &color);

    bool requiredNorm16(GLint internalformat);

    const std::string getVertexShader(const GLenum &target);
    const std::string getFragmentShader(const GLenum &target, bool isInteger, bool isUInteger);

    // Initialize texture.
    void initTexture(int levelNum, FormatTableElement &fmt, GLTexture &tex);

    // Clear and check results.
    void clearCheckRenderable(int level,
                              int width,
                              int height,
                              int depth,
                              const GLTexture &tex,
                              const FormatTableElement &fmt,
                              GLColor &fullColorRef,
                              ColorTypes &fullColor,
                              GLColor &partialColorRef,
                              ColorTypes &partialColor);
    void clearCheckUnrenderable(int level,
                                int width,
                                int height,
                                int depth,
                                const GLTexture &tex,
                                const FormatTableElement &fmt,
                                GLuint program,
                                const std::vector<int> &loc,
                                GLColor &fullColorRef,
                                ColorTypes &fullColor,
                                GLColor &partialColorRef,
                                ColorTypes &partialColor);
};

ColorTypes ClearTextureEXTTestES31::convertColorTypeInternal(const GLenum &type,
                                                             const GLColor &color)
{
    GLColor32F colorFloat;
    colorFloat.R = gl::normalizedToFloat<uint8_t>(color.R);
    colorFloat.G = gl::normalizedToFloat<uint8_t>(color.G);
    colorFloat.B = gl::normalizedToFloat<uint8_t>(color.B);
    colorFloat.A = gl::normalizedToFloat<uint8_t>(color.A);

    switch (type)
    {
        case GL_UNSIGNED_BYTE:
        {
            return color;
        }
        case GL_BYTE:
        {
            GLColorT<int8_t> retColorByte;
            retColorByte.R = gl::floatToNormalized<int8_t>(colorFloat.R);
            retColorByte.G = gl::floatToNormalized<int8_t>(colorFloat.G);
            retColorByte.B = gl::floatToNormalized<int8_t>(colorFloat.B);
            retColorByte.A = gl::floatToNormalized<int8_t>(colorFloat.A);
            return retColorByte;
        }
        case GL_UNSIGNED_SHORT:
        {
            GLColor16 retColorUShort;
            retColorUShort.R = gl::floatToNormalized<uint16_t>(colorFloat.R);
            retColorUShort.G = gl::floatToNormalized<uint16_t>(colorFloat.G);
            retColorUShort.B = gl::floatToNormalized<uint16_t>(colorFloat.B);
            retColorUShort.A = gl::floatToNormalized<uint16_t>(colorFloat.A);
            return retColorUShort;
        }
        case GL_UNSIGNED_SHORT_5_6_5:
        {
            GLushort retColor565;
            retColor565 = gl::shiftData<5, 11>(gl::floatToNormalized<5, uint16_t>(colorFloat.R)) |
                          gl::shiftData<6, 5>(gl::floatToNormalized<6, uint16_t>(colorFloat.G)) |
                          gl::shiftData<5, 0>(gl::floatToNormalized<5, uint16_t>(colorFloat.B));
            return retColor565;
        }
        case GL_UNSIGNED_SHORT_5_5_5_1:
        {
            GLushort retColor5551;
            retColor5551 = gl::shiftData<5, 11>(gl::floatToNormalized<5, uint16_t>(colorFloat.R)) |
                           gl::shiftData<5, 6>(gl::floatToNormalized<5, uint16_t>(colorFloat.G)) |
                           gl::shiftData<5, 1>(gl::floatToNormalized<5, uint16_t>(colorFloat.B)) |
                           gl::shiftData<1, 0>(gl::floatToNormalized<1, uint16_t>(colorFloat.A));
            return retColor5551;
        }
        case GL_UNSIGNED_SHORT_4_4_4_4:
        {
            GLushort retColor4444;
            retColor4444 = gl::shiftData<4, 12>(gl::floatToNormalized<4, uint16_t>(colorFloat.R)) |
                           gl::shiftData<4, 8>(gl::floatToNormalized<4, uint16_t>(colorFloat.G)) |
                           gl::shiftData<4, 4>(gl::floatToNormalized<4, uint16_t>(colorFloat.B)) |
                           gl::shiftData<4, 0>(gl::floatToNormalized<4, uint16_t>(colorFloat.A));
            return retColor4444;
        }
        case GL_SHORT:
        {
            GLColorT<int16_t> retColorShort;
            retColorShort.R = gl::floatToNormalized<int16_t>(colorFloat.R);
            retColorShort.G = gl::floatToNormalized<int16_t>(colorFloat.G);
            retColorShort.B = gl::floatToNormalized<int16_t>(colorFloat.B);
            retColorShort.A = gl::floatToNormalized<int16_t>(colorFloat.A);
            return retColorShort;
        }
        case GL_UNSIGNED_INT:
        {
            GLColor32UI retColorUInt;
            retColorUInt.R = gl::floatToNormalized<uint32_t>(colorFloat.R);
            retColorUInt.G = gl::floatToNormalized<uint32_t>(colorFloat.G);
            retColorUInt.B = gl::floatToNormalized<uint32_t>(colorFloat.B);
            retColorUInt.A = gl::floatToNormalized<uint32_t>(colorFloat.A);
            return retColorUInt;
        }
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
        {
            GLuint retColor101011;
            retColor101011 = (GLuint)gl::float32ToFloat11(colorFloat.R) |
                             (GLuint)(gl::float32ToFloat11(colorFloat.G) << 11) |
                             (GLuint)(gl::float32ToFloat10(colorFloat.B) << 22);
            return retColor101011;
        }
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        {
            GLuint retColor1010102;
            retColor1010102 = gl::floatToNormalized<10, uint32_t>(colorFloat.R) |
                              (gl::floatToNormalized<10, uint32_t>(colorFloat.G) << 10) |
                              (gl::floatToNormalized<10, uint32_t>(colorFloat.B) << 20) |
                              (gl::floatToNormalized<2, uint32_t>(colorFloat.A) << 30);
            return retColor1010102;
        }
        case GL_UNSIGNED_INT_5_9_9_9_REV:
        {
            GLuint retColor9995;
            retColor9995 = gl::convertRGBFloatsTo999E5(colorFloat.R, colorFloat.G, colorFloat.B);
            return retColor9995;
        }
        case GL_INT:
        {
            GLColor32I retColorInt;
            retColorInt.R = gl::floatToNormalized<int32_t>(colorFloat.R);
            retColorInt.G = gl::floatToNormalized<int32_t>(colorFloat.G);
            retColorInt.B = gl::floatToNormalized<int32_t>(colorFloat.B);
            retColorInt.A = gl::floatToNormalized<int32_t>(colorFloat.A);
            return retColorInt;
        }
        case GL_HALF_FLOAT:
        {
            GLColor16 retColorHFloat;
            retColorHFloat.R = gl::float32ToFloat16(colorFloat.R);
            retColorHFloat.G = gl::float32ToFloat16(colorFloat.G);
            retColorHFloat.B = gl::float32ToFloat16(colorFloat.B);
            retColorHFloat.A = gl::float32ToFloat16(colorFloat.A);
            return retColorHFloat;
        }
        case GL_FLOAT:
        {
            return colorFloat;
        }
        default:
        {
            return color;
        }
    }
}

ColorTypes ClearTextureEXTTestES31::convertColorType(const GLenum &format,
                                                     const GLenum &type,
                                                     GLColor color)
{
    // LUMINANCE, LUMINANCE_ALPHA and ALPHA are special.
    switch (format)
    {
        case GL_LUMINANCE:
        {
            color.G = color.B = color.A = 0;
            break;
        }
        case GL_LUMINANCE_ALPHA:
        {
            color.G = color.A;
            color.B = color.A = 0;
            break;
        }
        case GL_ALPHA:
        {
            color.R = color.A;
            color.G = color.B = color.A = 0;
            break;
        }
        default:
        {
            // Other formats do nothing.
            break;
        }
    }
    return convertColorTypeInternal(type, color);
}

GLColor ClearTextureEXTTestES31::getClearColor(GLenum format, GLColor full)
{
    switch (format)
    {
        case GL_RED_INTEGER:
        case GL_RED:
            return GLColor(full.R, 0, 0, 255u);
        case GL_RG_INTEGER:
        case GL_RG:
            return GLColor(full.R, full.G, 0, 255u);
        case GL_RGB_INTEGER:
        case GL_RGB:
            return GLColor(full.R, full.G, full.B, 255u);
        case GL_RGBA_INTEGER:
        case GL_RGBA:
            return full;
        case GL_LUMINANCE:
            return GLColor(full.R, full.R, full.R, 255u);
        case GL_ALPHA:
            return GLColor(0, 0, 0, full.A);
        case GL_LUMINANCE_ALPHA:
            return GLColor(full.R, full.R, full.R, full.A);
        default:
            EXPECT_TRUE(false);
            return GLColor::white;
    }
}

void ClearTextureEXTTestES31::colorCheckType(const FormatTableElement &fmt,
                                             const int &x,
                                             const int &y,
                                             const int &width,
                                             const int &height,
                                             GLColor &color)
{
    if (fmt.isInt())
    {
        GLColor32UI colorUInt;
        GLColor32I colorInt;
        switch (fmt.type)
        {
            case GL_UNSIGNED_BYTE:
            {
                colorUInt.R = static_cast<uint32_t>(color.R);
                colorUInt.G = static_cast<uint32_t>(color.G);
                colorUInt.B = static_cast<uint32_t>(color.B);
                colorUInt.A = static_cast<uint32_t>(color.A);
                if (fmt.format != GL_RGBA_INTEGER)
                {
                    colorUInt.A = 1;
                }
                EXPECT_PIXEL_RECT32UI_EQ(x, y, width, height, colorUInt);
                return;
            }
            case GL_UNSIGNED_SHORT:
            {
                GLColor16 colorUShort =
                    std::get<GLColor16>(convertColorTypeInternal(GL_UNSIGNED_SHORT, color));
                colorUInt.R = static_cast<uint32_t>(colorUShort.R);
                colorUInt.G = static_cast<uint32_t>(colorUShort.G);
                colorUInt.B = static_cast<uint32_t>(colorUShort.B);
                colorUInt.A = static_cast<uint32_t>(colorUShort.A);
                if (fmt.format != GL_RGBA_INTEGER)
                {
                    colorUInt.A = 1;
                }
                EXPECT_PIXEL_RECT32UI_EQ(x, y, width, height, colorUInt);
                return;
            }
            case GL_UNSIGNED_INT:
            {
                colorUInt = std::get<GLColor32UI>(convertColorTypeInternal(GL_UNSIGNED_INT, color));
                if (fmt.format != GL_RGBA_INTEGER)
                {
                    colorUInt.A = 1;
                }
                EXPECT_PIXEL_RECT32UI_EQ(x, y, width, height, colorUInt);
                return;
            }
            case GL_BYTE:
            {
                GLColorT<int8_t> colorByte =
                    std::get<GLColorT<int8_t>>(convertColorTypeInternal(GL_BYTE, color));
                colorInt.R = static_cast<int32_t>(colorByte.R);
                colorInt.G = static_cast<int32_t>(colorByte.G);
                colorInt.B = static_cast<int32_t>(colorByte.B);
                colorInt.A = static_cast<int32_t>(colorByte.A);
                if (fmt.format != GL_RGBA_INTEGER)
                {
                    colorInt.A = 1;
                }
                EXPECT_PIXEL_RECT32I_EQ(x, y, width, height, colorInt);
                return;
            }
            case GL_SHORT:
            {
                GLColorT<int16_t> colorShort =
                    std::get<GLColorT<int16_t>>(convertColorTypeInternal(GL_SHORT, color));
                colorInt.R = static_cast<int32_t>(colorShort.R);
                colorInt.G = static_cast<int32_t>(colorShort.G);
                colorInt.B = static_cast<int32_t>(colorShort.B);
                colorInt.A = static_cast<int32_t>(colorShort.A);
                if (fmt.format != GL_RGBA_INTEGER)
                {
                    colorInt.A = 1;
                }
                EXPECT_PIXEL_RECT32I_EQ(x, y, width, height, colorInt);
                return;
            }
            case GL_INT:
            {
                colorInt = std::get<GLColor32I>(convertColorTypeInternal(GL_INT, color));
                if (fmt.format != GL_RGBA_INTEGER)
                {
                    colorInt.A = 1;
                }
                EXPECT_PIXEL_RECT32I_EQ(x, y, width, height, colorInt);
                return;
            }
            default:
            {
                UNREACHABLE();
            }
        }
    }
    else
    {
        switch (fmt.type)
        {
            case GL_UNSIGNED_INT_10F_11F_11F_REV:
            case GL_HALF_FLOAT:
            case GL_FLOAT:
            {
                GLColor32F colorFloat =
                    std::get<GLColor32F>(convertColorTypeInternal(GL_FLOAT, color));
                EXPECT_PIXEL_RECT32F_EQ(x, y, width, height, colorFloat);
                return;
            }
            default:
            {
                EXPECT_PIXEL_RECT_EQ(x, y, width, height, color);
                return;
            }
        }
    }
}

bool ClearTextureEXTTestES31::requiredNorm16(GLint internalformat)
{
    switch (internalformat)
    {
        case GL_R16_EXT:
        case GL_RG16_EXT:
        case GL_RGB16_EXT:
        case GL_RGBA16_EXT:
        case GL_R16_SNORM_EXT:
        case GL_RG16_SNORM_EXT:
        case GL_RGB16_SNORM_EXT:
        case GL_RGBA16_SNORM_EXT:
            return true;
        default:
            return false;
    }
}

const std::string ClearTextureEXTTestES31::getVertexShader(const GLenum &target)
{
    std::string uniform  = "";
    std::string texcoord = "    texcoord = (a_position.xy * 0.5) + 0.5;\n";

    if (target == GL_TEXTURE_2D_MULTISAMPLE || target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
    {
        uniform = "uniform vec2 texsize;\n";
        texcoord =
            "    texcoord = (a_position.xy * 0.5) + 0.5;\n"
            "    texcoord.x = texcoord.x * texsize.x;\n"
            "    texcoord.y = texcoord.y * texsize.y;\n";
    }

    return "#version 310 es\n"
           "out vec2 texcoord;\n"
           "in vec4 a_position;\n" +
           uniform +
           "void main()\n"
           "{\n"
           "    gl_Position = vec4(a_position.xy, 0.0, 1.0);\n" +
           texcoord + "}\n";
}

const std::string ClearTextureEXTTestES31::getFragmentShader(const GLenum &target,
                                                             bool isInteger,
                                                             bool isUInteger)
{
    std::string version  = "";
    std::string sampler  = "";
    std::string uniform  = "";
    std::string mainCode = "";
    std::string ui       = "";
    if (isInteger)
    {
        if (isUInteger)
        {
            ui = "u";
        }
        else
        {
            ui = "i";
        }
    }
    switch (target)
    {
        case GL_TEXTURE_2D:
        {
            sampler  = "uniform highp " + ui + "sampler2D tex;\n";
            mainCode = "    fragColor = vec4(texture(tex, texcoord));\n";
            break;
        }
        case GL_TEXTURE_2D_ARRAY:
        {
            sampler  = "uniform highp " + ui + "sampler2DArray tex;\n";
            uniform  = "uniform int slice;\n";
            mainCode = "    fragColor = vec4(texture(tex, vec3(texcoord, float(slice))));\n";
            break;
        }
        case GL_TEXTURE_3D:
        {
            sampler  = "uniform highp " + ui + "sampler3D tex;\n";
            uniform  = "uniform float slice;\n";
            mainCode = "    fragColor = vec4(texture(tex, vec3(texcoord, slice)));\n";
            break;
        }
        case GL_TEXTURE_CUBE_MAP:
        {
            sampler = sampler = "uniform highp " + ui + "samplerCube tex;\n";
            uniform           = "uniform int cubeFace;\n";
            mainCode =
                "    vec2 scaled = texcoord.xy * 2. - 1.;\n"
                "    vec3 cubecoord = vec3(1, -scaled.yx);\n"
                "    if (cubeFace == 1)\n"
                "        cubecoord = vec3(-1, -scaled.y, scaled.x);\n"
                "    if (cubeFace == 2)\n"
                "        cubecoord = vec3(scaled.x, 1, scaled.y);\n"
                "    if (cubeFace == 3)\n"
                "        cubecoord = vec3(scaled.x, -1, -scaled.y);\n"
                "    if (cubeFace == 4)\n"
                "        cubecoord = vec3(scaled.x, -scaled.y, 1);\n"
                "    if (cubeFace == 5)\n"
                "        cubecoord = vec3(-scaled.xy, -1);\n"
                "    fragColor = vec4(texture(tex, cubecoord));\n";
            break;
        }
        case GL_TEXTURE_CUBE_MAP_ARRAY:
        {
            version = "#extension GL_EXT_texture_cube_map_array : enable\n";
            sampler = "uniform highp " + ui + "samplerCubeArray tex;\n";
            uniform =
                "uniform int cubeFace;\n"
                "uniform int layer;\n";
            mainCode =
                "    vec2 scaled = texcoord.xy * 2. - 1.;\n"
                "    vec3 cubecoord = vec3(1, -scaled.yx);\n"
                "    if (cubeFace == 1)\n"
                "        cubecoord = vec3(-1, -scaled.y, scaled.x);\n"
                "    if (cubeFace == 2)\n"
                "        cubecoord = vec3(scaled.x, 1, scaled.y);\n"
                "    if (cubeFace == 3)\n"
                "        cubecoord = vec3(scaled.x, -1, -scaled.y);\n"
                "    if (cubeFace == 4)\n"
                "        cubecoord = vec3(scaled.x, -scaled.y, 1);\n"
                "    if (cubeFace == 5)\n"
                "        cubecoord = vec3(-scaled.xy, -1);\n"
                "    fragColor = vec4(texture(tex, vec4(cubecoord, layer)));\n";
            break;
        }
        case GL_TEXTURE_2D_MULTISAMPLE:
        {
            sampler  = "uniform highp " + ui + "sampler2DMS tex;\n";
            uniform  = "uniform int s;\n";
            mainCode = "    fragColor = vec4(texelFetch(tex, ivec2(texcoord), s));\n";
            break;
        }
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        {
            version = "#extension GL_OES_texture_storage_multisample_2d_array : require\n";
            sampler = "uniform highp " + ui + "sampler2DMSArray tex;\n";
            uniform =
                "uniform int s;\n"
                "uniform int slice;\n";
            mainCode = "    fragColor = vec4(texelFetch(tex, ivec3(texcoord, slice), s));\n";
            break;
        }
        default:
        {
            UNREACHABLE();
        }
    }

    return "#version 310 es\n" + version + "precision highp float;\n" + sampler + uniform +
           "in vec2 texcoord;\n"
           "out vec4 fragColor;\n"
           "void main()\n"
           "{\n" +
           mainCode + "}\n";
}

void ClearTextureEXTTestES31::initTexture(int levelNum, FormatTableElement &fmt, GLTexture &tex)
{
    // Create a texture widthxheightxdepth with 0x33.
    std::vector<uint8_t> initColor(mWidth * mHeight * mDepth * 4 * sizeof(fmt.type), 0x33);

    glBindTexture(mTarget, tex);
    if (1 == levelNum)
    {
        glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    else
    {
        glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    }
    glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_ERROR(GL_NO_ERROR);

    // For each level, initialize the texture color.
    for (int level = 0; level < levelNum; ++level)
    {
        int w = std::max(mWidth >> level, 1);
        int h = std::max(mHeight >> level, 1);
        int d = (mTarget == GL_TEXTURE_3D ? std::max(mDepth >> level, 1) : mDepth);

        if (mTarget == GL_TEXTURE_2D)
        {
            glTexImage2D(mTarget, level, fmt.internalformat, w, h, 0, fmt.format, fmt.type,
                         initColor.data());
        }
        else if (mTarget == GL_TEXTURE_CUBE_MAP)
        {
            for (int f = 0; f < 6; ++f)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, level, fmt.internalformat, w, h, 0,
                             fmt.format, fmt.type, initColor.data());
            }
        }
        else
        {
            glTexImage3D(mTarget, level, fmt.internalformat, w, h, d, 0, fmt.format, fmt.type,
                         initColor.data());
        }
    }
}

void ClearTextureEXTTestES31::clearCheckRenderable(int level,
                                                   int width,
                                                   int height,
                                                   int depth,
                                                   const GLTexture &tex,
                                                   const FormatTableElement &fmt,
                                                   GLColor &fullColorRef,
                                                   ColorTypes &fullColor,
                                                   GLColor &partialColorRef,
                                                   ColorTypes &partialColor)
{
    // Clear the entire texture.
    glClearTexImageEXT(tex, level, fmt.format, fmt.type, &fullColor);

    for (int z = 0; z < depth; ++z)
    {
        if (mHasLayer)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, level, z);
        }
        else
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, level);
        }
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        colorCheckType(fmt, 0, 0, width, height, fullColorRef);
    }

    // Partial clear the texture.
    int clearXOffset = width >> 2;
    int clearYOffset = height >> 2;
    int clearZOffset = depth >> 2;
    int clearWidth   = std::max(width >> 1, 1);
    int clearHeight  = std::max(height >> 1, 1);
    int clearDepth   = std::max(depth >> 1, 1);

    glClearTexSubImageEXT(tex, level, clearXOffset, clearYOffset, clearZOffset, clearWidth,
                          clearHeight, clearDepth, fmt.format, fmt.type, &partialColor);

    for (int z = 0; z < depth; ++z)
    {
        if (mHasLayer)
        {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, level, z);
        }
        else
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, level);
        }

        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        if (z >= clearZOffset && z < (clearZOffset + clearDepth))
        {
            colorCheckType(fmt, clearXOffset, clearYOffset, clearWidth, clearHeight,
                           partialColorRef);
        }
        else
        {
            colorCheckType(fmt, clearXOffset, clearYOffset, clearWidth, clearHeight, fullColorRef);
        }
    }
}

void ClearTextureEXTTestES31::clearCheckUnrenderable(int level,
                                                     int width,
                                                     int height,
                                                     int depth,
                                                     const GLTexture &tex,
                                                     const FormatTableElement &fmt,
                                                     GLuint program,
                                                     const std::vector<int> &loc,
                                                     GLColor &fullColorRef,
                                                     ColorTypes &fullColor,
                                                     GLColor &partialColorRef,
                                                     ColorTypes &partialColor)
{
    // Clear the entire texture.
    glClearTexImageEXT(tex, level, fmt.format, fmt.type, &fullColor);
    glTexParameteri(mTarget, GL_TEXTURE_BASE_LEVEL, level);

    int sampleNum = 1;
    if (mTarget == GL_TEXTURE_2D_MULTISAMPLE || mTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
    {
        glGetInternalformativ(mTarget, fmt.internalformat, GL_SAMPLES, 1, &sampleNum);
        sampleNum = std::min(sampleNum, 4);
    }

    for (int z = 0; z < depth; ++z)
    {
        if (mHasLayer)
        {
            if (mTarget == GL_TEXTURE_CUBE_MAP_ARRAY)
            {
                glUniform1i(loc[0], z % 6);
                glUniform1i(loc[1], z / 6);
            }
            else if (mTarget == GL_TEXTURE_3D)
            {
                glUniform1f(loc[0], (z + 0.5f) / depth);
            }
            else if (mTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
            {
                glUniform1i(loc[1], z);
            }
            else
            {
                glUniform1i(loc[0], z);
            }
        }

        for (int sample = 0; sample < sampleNum; ++sample)
        {
            if (sampleNum != 1)
            {
                glUniform1i(loc[0], sample);
            }
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
            EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), fullColorRef);
        }
    }

    // Partial clear the texture.
    int clearXOffset = width >> 2;
    int clearYOffset = height >> 2;
    int clearZOffset = depth >> 2;
    int clearWidth   = std::max(width >> 1, 1);
    int clearHeight  = std::max(height >> 1, 1);
    int clearDepth   = std::max(depth >> 1, 1);
    int pixelXOffset =
        static_cast<int>(clearXOffset * getWindowWidth() / static_cast<float>(width) + 0.5);
    int pixelYOffset =
        static_cast<int>(clearYOffset * getWindowHeight() / static_cast<float>(height) + 0.5);
    int pixelWidth =
        static_cast<int>(
            (clearXOffset + clearWidth) * getWindowWidth() / static_cast<float>(width) + 0.5) -
        pixelXOffset;
    int pixelHeight =
        static_cast<int>(
            (clearYOffset + clearHeight) * getWindowHeight() / static_cast<float>(height) + 0.5) -
        pixelYOffset;

    glClearTexSubImageEXT(tex, level, clearXOffset, clearYOffset, clearZOffset, clearWidth,
                          clearHeight, clearDepth, fmt.format, fmt.type, &partialColor);

    for (int z = 0; z < depth; ++z)
    {
        if (mHasLayer)
        {
            if (mTarget == GL_TEXTURE_CUBE_MAP_ARRAY)
            {
                glUniform1i(loc[0], z % 6);
                glUniform1i(loc[1], z / 6);
            }
            else if (mTarget == GL_TEXTURE_3D)
            {
                glUniform1f(loc[0], (z + 0.5f) / depth);
            }
            else if (mTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
            {
                glUniform1i(loc[1], z);
            }
            else
            {
                glUniform1i(loc[0], z);
            }
        }

        for (int sample = 0; sample < sampleNum; ++sample)
        {
            if (sampleNum != 1)
            {
                glUniform1i(loc[0], sample);
            }
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
            if (z >= clearZOffset && z < (clearZOffset + clearDepth))
            {
                EXPECT_PIXEL_RECT_EQ(pixelXOffset, pixelYOffset, pixelWidth, pixelHeight,
                                     partialColorRef);
            }
            else
            {
                EXPECT_PIXEL_RECT_EQ(pixelXOffset, pixelYOffset, pixelWidth, pixelHeight,
                                     fullColorRef);
            }
        }
    }
}

class ClearTextureEXTTestES31Renderable : public ClearTextureEXTTestES31
{
  protected:
    std::vector<FormatTableElement> mFormats = {
        {GL_R8, GL_RED, GL_UNSIGNED_BYTE},
        {GL_R16F, GL_RED, GL_HALF_FLOAT},
        {GL_R16F, GL_RED, GL_FLOAT},
        {GL_R32F, GL_RED, GL_FLOAT},
        {GL_RG8, GL_RG, GL_UNSIGNED_BYTE},
        {GL_RG16F, GL_RG, GL_HALF_FLOAT},
        {GL_RG16F, GL_RG, GL_FLOAT},
        {GL_RG32F, GL_RG, GL_FLOAT},
        {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE},
        {GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5},
        {GL_RGB565, GL_RGB, GL_UNSIGNED_BYTE},
        {GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV},
        {GL_R11F_G11F_B10F, GL_RGB, GL_HALF_FLOAT},
        {GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT},
        {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE},
        {GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_BYTE},
        {GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1},
        {GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV},
        {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},
        {GL_RGBA4, GL_RGBA, GL_UNSIGNED_BYTE},
        {GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4},
        {GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV},
        {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},
        {GL_RGBA16F, GL_RGBA, GL_FLOAT},
        {GL_RGBA32F, GL_RGBA, GL_FLOAT},
        {GL_R16_EXT, GL_RED, GL_UNSIGNED_SHORT},
        {GL_RG16_EXT, GL_RG, GL_UNSIGNED_SHORT},
        {GL_RGBA16_EXT, GL_RGBA, GL_UNSIGNED_SHORT},
        {GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE},
        {GL_R8I, GL_RED_INTEGER, GL_BYTE},
        {GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT},
        {GL_R16I, GL_RED_INTEGER, GL_SHORT},
        {GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT},
        {GL_R32I, GL_RED_INTEGER, GL_INT},
        {GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE},
        {GL_RG8I, GL_RG_INTEGER, GL_BYTE},
        {GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT},
        {GL_RG16I, GL_RG_INTEGER, GL_SHORT},
        {GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT},
        {GL_RG32I, GL_RG_INTEGER, GL_INT},
        {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE},
        {GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE},
        {GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT},
        {GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT},
        {GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT},
        {GL_RGBA32I, GL_RGBA_INTEGER, GL_INT},
    };

    void testRenderable()
    {
        ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                           !IsGLExtensionEnabled("GL_EXT_clear_texture") || !mExtraSupport);

        const auto test = [&](FormatTableElement &fmt) {
            // Update MAX level numbers.
            if (mTarget == GL_TEXTURE_3D)
            {
                mLevels = std::max(mLevels, static_cast<int>(std::log2(mDepth)) + 1);
            }

            GLTexture tex;
            initTexture(mLevels, fmt, tex);

            GLFramebuffer fbo;
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);

            // Calculate specific clear color value.
            GLColor fullColorRef    = getClearColor(fmt.format, mFullColorRef);
            ColorTypes fullColor    = convertColorType(fmt.format, fmt.type, fullColorRef);
            GLColor partialColorRef = getClearColor(fmt.format, mPartialColorRef);
            ColorTypes partialColor = convertColorType(fmt.format, fmt.type, partialColorRef);

            for (int level = 0; level < mLevels; ++level)
            {
                int width  = std::max(mWidth >> level, 1);
                int height = std::max(mHeight >> level, 1);
                int depth  = mIsArray ? mDepth : std::max(mDepth >> level, 1);

                clearCheckRenderable(level, width, height, depth, tex, fmt, fullColorRef, fullColor,
                                     partialColorRef, partialColor);
            }
        };

        bool supportNorm16 = IsGLExtensionEnabled("GL_EXT_texture_norm16");
        for (auto fmt : mFormats)
        {
            if (!supportNorm16 && requiredNorm16(fmt.internalformat))
            {
                continue;
            }
            test(fmt);
        }
    }

    void testMS()
    {
        ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                           !IsGLExtensionEnabled("GL_EXT_clear_texture") || !mExtraSupport);

        const auto test = [&](FormatTableElement &fmt) {
            // Initialize the texture.
            GLTexture tex;
            glBindTexture(mTarget, tex);
            int sampleNum = 0;
            glGetInternalformativ(mTarget, fmt.internalformat, GL_SAMPLES, 1, &sampleNum);
            sampleNum = std::min(sampleNum, 4);
            if (mIsArray)
            {
                glTexStorage3DMultisampleOES(mTarget, sampleNum, fmt.internalformat, mWidth,
                                             mHeight, mDepth, GL_TRUE);
            }
            else
            {
                glTexStorage2DMultisample(mTarget, sampleNum, fmt.internalformat, mWidth, mHeight,
                                          GL_TRUE);
            }
            EXPECT_GL_ERROR(GL_NO_ERROR);
            ANGLE_GL_PROGRAM(initProgram, essl31_shaders::vs::Simple(),
                             essl31_shaders::fs::Green());
            glUseProgram(initProgram);
            drawQuad(initProgram, essl31_shaders::PositionAttrib(), 0.5f);

            // Calculate specific clear color value.
            GLColor fullColorRef    = getClearColor(fmt.format, mFullColorRef);
            ColorTypes fullColor    = convertColorType(fmt.format, fmt.type, fullColorRef);
            GLColor partialColorRef = getClearColor(fmt.format, mPartialColorRef);
            ColorTypes partialColor = convertColorType(fmt.format, fmt.type, partialColorRef);

            ANGLE_GL_PROGRAM(program, getVertexShader(mTarget).c_str(),
                             getFragmentShader(mTarget, fmt.isInt(), fmt.isUInt()).c_str());
            glUseProgram(program);

            std::vector<int> uniformLocs = {0, 0, 0};

            uniformLocs[0] = glGetUniformLocation(program, "s");
            ASSERT_NE(-1, uniformLocs[0]);
            uniformLocs[2] = glGetUniformLocation(program, "texsize");
            ASSERT_NE(-1, uniformLocs[2]);
            glUniform2f(uniformLocs[2], static_cast<float>(mWidth), static_cast<float>(mHeight));
            if (mIsArray)
            {
                uniformLocs[1] = glGetUniformLocation(program, "slice");
                ASSERT_NE(-1, uniformLocs[1]);
            }

            clearCheckUnrenderable(0, mWidth, mHeight, mDepth, tex, fmt, program, uniformLocs,
                                   fullColorRef, fullColor, partialColorRef, partialColor);
        };

        bool supportNorm16 = IsGLExtensionEnabled("GL_EXT_texture_norm16");
        for (auto fmt : mFormats)
        {
            if (!supportNorm16 && requiredNorm16(fmt.internalformat))
            {
                continue;
            }
            test(fmt);
        }
    }
};

class ClearTextureEXTTestES31Unrenderable : public ClearTextureEXTTestES31
{
  protected:
    std::vector<FormatTableElement> mFormats = {
        {GL_RGB16F, GL_RGB, GL_HALF_FLOAT},
        {GL_RGB16F, GL_RGB, GL_FLOAT},
        {GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE},
        {GL_RGB8I, GL_RGB_INTEGER, GL_BYTE},
        {GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT},
        {GL_RGB16I, GL_RGB_INTEGER, GL_SHORT},
        {GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT},
        {GL_RGB32I, GL_RGB_INTEGER, GL_INT},
        {GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE},
        {GL_R8_SNORM, GL_RED, GL_BYTE},
        {GL_RG8_SNORM, GL_RG, GL_BYTE},
        {GL_RGB8_SNORM, GL_RGB, GL_BYTE},
        {GL_RGB32F, GL_RGB, GL_FLOAT},
        {GL_RGBA8_SNORM, GL_RGBA, GL_BYTE},
        {GL_RGB16_EXT, GL_RGB, GL_UNSIGNED_SHORT},
        {GL_R16_SNORM_EXT, GL_RED, GL_SHORT},
        {GL_RG16_SNORM_EXT, GL_RG, GL_SHORT},
        {GL_RGBA16_SNORM_EXT, GL_RGBA, GL_SHORT},
        {GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE},
        {GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE},
        {GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE},
    };

    std::vector<FormatTableElement> mFormatsRGB9E5 = {
        {GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV},
        {GL_RGB9_E5, GL_RGB, GL_HALF_FLOAT},
        {GL_RGB9_E5, GL_RGB, GL_FLOAT},
    };

    void testUnrenderable(std::vector<FormatTableElement> &formats)
    {
        ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                           !IsGLExtensionEnabled("GL_EXT_clear_texture") || !mExtraSupport);

        const auto test = [&](FormatTableElement &fmt) {
            // Update MAX level numbers.
            if (mTarget == GL_TEXTURE_3D)
            {
                mLevels = std::max(mLevels, static_cast<int>(std::log2(mDepth)) + 1);
            }

            GLTexture tex;
            initTexture(mLevels, fmt, tex);

            // Calculate specific clear color value.
            GLColor fullColorRef    = getClearColor(fmt.format, mFullColorRef);
            ColorTypes fullColor    = convertColorType(fmt.format, fmt.type, fullColorRef);
            GLColor partialColorRef = getClearColor(fmt.format, mPartialColorRef);
            ColorTypes partialColor = convertColorType(fmt.format, fmt.type, partialColorRef);

            ANGLE_GL_PROGRAM(program, getVertexShader(mTarget).c_str(),
                             getFragmentShader(mTarget, fmt.isInt(), fmt.isUInt()).c_str());
            glUseProgram(program);

            std::vector<int> uniformLocs = {0, 0};
            if (mTarget == GL_TEXTURE_2D_ARRAY || mTarget == GL_TEXTURE_3D)
            {
                uniformLocs[0] = glGetUniformLocation(program, "slice");
                ASSERT_NE(-1, uniformLocs[0]);
            }
            else if (mTarget == GL_TEXTURE_CUBE_MAP)
            {
                uniformLocs[0] = glGetUniformLocation(program, "cubeFace");
            }
            else if (mTarget == GL_TEXTURE_CUBE_MAP_ARRAY)
            {
                uniformLocs[0] = glGetUniformLocation(program, "cubeFace");
                ASSERT_NE(-1, uniformLocs[0]);
                uniformLocs[1] = glGetUniformLocation(program, "layer");
                ASSERT_NE(-1, uniformLocs[1]);
            }

            // For each level, clear the texture.
            for (int level = 0; level < mLevels; ++level)
            {
                int width  = std::max(mWidth >> level, 1);
                int height = std::max(mHeight >> level, 1);
                int depth  = mIsArray ? mDepth : std::max(mDepth >> level, 1);

                clearCheckUnrenderable(level, width, height, depth, tex, fmt, program, uniformLocs,
                                       fullColorRef, fullColor, partialColorRef, partialColor);
            }
        };

        bool supportNorm16 = IsGLExtensionEnabled("GL_EXT_texture_norm16");
        for (auto fmt : formats)
        {
            if (!supportNorm16 && requiredNorm16(fmt.internalformat))
            {
                continue;
            }
            test(fmt);
        }
    }
};

// Each int parameter can have three values: don't clear, clear, or masked clear.  The bool
// parameter controls scissor.
using MaskedScissoredClearVariationsTestParams =
    std::tuple<angle::PlatformParameters, int, int, int, bool>;

void ParseMaskedScissoredClearVariationsTestParams(
    const MaskedScissoredClearVariationsTestParams &params,
    bool *clearColor,
    bool *clearDepth,
    bool *clearStencil,
    bool *maskColor,
    bool *maskDepth,
    bool *maskStencil,
    bool *scissor)
{
    int colorClearInfo   = std::get<1>(params);
    int depthClearInfo   = std::get<2>(params);
    int stencilClearInfo = std::get<3>(params);

    *clearColor   = colorClearInfo > 0;
    *clearDepth   = depthClearInfo > 0;
    *clearStencil = stencilClearInfo > 0;

    *maskColor   = colorClearInfo > 1;
    *maskDepth   = depthClearInfo > 1;
    *maskStencil = stencilClearInfo > 1;

    *scissor = std::get<4>(params);
}

std::string MaskedScissoredClearVariationsTestPrint(
    const ::testing::TestParamInfo<MaskedScissoredClearVariationsTestParams> &paramsInfo)
{
    const MaskedScissoredClearVariationsTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params);

    bool clearColor, clearDepth, clearStencil;
    bool maskColor, maskDepth, maskStencil;
    bool scissor;

    ParseMaskedScissoredClearVariationsTestParams(params, &clearColor, &clearDepth, &clearStencil,
                                                  &maskColor, &maskDepth, &maskStencil, &scissor);

    if (scissor || clearColor || clearDepth || clearStencil || maskColor || maskDepth ||
        maskStencil)
    {
        out << "_";
    }

    if (scissor)
    {
        out << "_scissored";
    }

    if (clearColor || clearDepth || clearStencil)
    {
        out << "_clear_";
        if (clearColor)
        {
            out << "c";
        }
        if (clearDepth)
        {
            out << "d";
        }
        if (clearStencil)
        {
            out << "s";
        }
    }

    if (maskColor || maskDepth || maskStencil)
    {
        out << "_mask_";
        if (maskColor)
        {
            out << "c";
        }
        if (maskDepth)
        {
            out << "d";
        }
        if (maskStencil)
        {
            out << "s";
        }
    }

    return out.str();
}

class MaskedScissoredClearTestBase : public ANGLETest<MaskedScissoredClearVariationsTestParams>
{
  protected:
    MaskedScissoredClearTestBase()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    void maskedScissoredColorDepthStencilClear(
        const MaskedScissoredClearVariationsTestParams &params);

    bool mHasDepth   = true;
    bool mHasStencil = true;
};

class MaskedScissoredClearTest : public MaskedScissoredClearTestBase
{};

// Overrides a feature to force emulation of stencil-only and depth-only formats with a packed
// depth/stencil format
class VulkanClearTest : public MaskedScissoredClearTestBase
{
  protected:
    void testSetUp() override
    {
        glBindTexture(GL_TEXTURE_2D, mColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);

        // Setup Color/Stencil FBO with a stencil format that's emulated with packed depth/stencil.
        glBindFramebuffer(GL_FRAMEBUFFER, mColorStencilFBO);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTexture,
                               0);
        glBindRenderbuffer(GL_RENDERBUFFER, mStencilTexture);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, getWindowWidth(),
                              getWindowHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  mStencilTexture);

        ASSERT_GL_NO_ERROR();

        // Note: GL_DEPTH_COMPONENT24 is not allowed in GLES2.
        if (getClientMajorVersion() >= 3)
        {
            // Setup Color/Depth FBO with a depth format that's emulated with packed depth/stencil.
            glBindFramebuffer(GL_FRAMEBUFFER, mColorDepthFBO);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   mColorTexture, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthTexture);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, getWindowWidth(),
                                  getWindowHeight());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                      mDepthTexture);
        }

        ASSERT_GL_NO_ERROR();
    }

    void bindColorStencilFBO()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mColorStencilFBO);
        mHasDepth = false;
    }

    void bindColorDepthFBO()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mColorDepthFBO);
        mHasStencil = false;
    }

  private:
    GLFramebuffer mColorStencilFBO;
    GLFramebuffer mColorDepthFBO;
    GLTexture mColorTexture;
    GLRenderbuffer mDepthTexture;
    GLRenderbuffer mStencilTexture;
};

// Test clearing the default framebuffer
TEST_P(ClearTest, DefaultFramebuffer)
{
    glClearColor(0.25f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 128, 128, 1.0);
}

// Test clearing the default framebuffer with scissor and mask
// This forces down path that uses draw to do clear
TEST_P(ClearTest, EmptyScissor)
{
    // These configs have bug that fails this test.
    // These configs are unmaintained so skipping.
    ANGLE_SKIP_TEST_IF(IsIntel() && IsD3D9());
    ANGLE_SKIP_TEST_IF(IsIntel() && IsMac() && IsOpenGL());
    glClearColor(0.25f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(-10, 0, 5, 5);
    glClearColor(0.5f, 0.25f, 0.75f, 0.5f);
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 128, 255, 1.0);
}

// Test clearing the RGB default framebuffer and verify that the alpha channel is not cleared
TEST_P(ClearTestRGB, DefaultFramebufferRGB)
{
    // Some GPUs don't support RGB format default framebuffer,
    // so skip if the back buffer has alpha bits.
    EGLWindow *window          = getEGLWindow();
    EGLDisplay display         = window->getDisplay();
    EGLConfig config           = window->getConfig();
    EGLint backbufferAlphaBits = 0;
    eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &backbufferAlphaBits);
    ANGLE_SKIP_TEST_IF(backbufferAlphaBits != 0);

    glClearColor(0.25f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 128, 255, 1.0);
}

// Invalidate the RGB default framebuffer and verify that the alpha channel is not cleared, and
// stays set after drawing.
TEST_P(ClearTestRGB_ES3, InvalidateDefaultFramebufferRGB)
{
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    // Some GPUs don't support RGB format default framebuffer,
    // so skip if the back buffer has alpha bits.
    EGLWindow *window          = getEGLWindow();
    EGLDisplay display         = window->getDisplay();
    EGLConfig config           = window->getConfig();
    EGLint backbufferAlphaBits = 0;
    eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &backbufferAlphaBits);
    ANGLE_SKIP_TEST_IF(backbufferAlphaBits != 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Verify that even though Alpha is cleared to 0.0 for this RGB FBO, it should be read back as
    // 1.0, since the glReadPixels() is issued with GL_RGBA.
    // OpenGL ES 3.2 spec:
    // 16.1.3 Obtaining Pixels from the Framebuffer
    // If G, B, or A values are not present in the internal format, they are taken to be zero,
    // zero, and one respectively.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    const GLenum discards[] = {GL_COLOR};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);

    // Don't explicitly clear, but draw blue (make sure alpha is not cleared)
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Draw with a shader that outputs alpha=0.5. Readback and ensure that alpha=1.
TEST_P(ClearTestRGB_ES3, ShaderOutputsAlphaVerifyReadingAlphaIsOne)
{
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(blueProgram);

    // Some GPUs don't support RGB format default framebuffer,
    // so skip if the back buffer has alpha bits.
    EGLWindow *window          = getEGLWindow();
    EGLDisplay display         = window->getDisplay();
    EGLConfig config           = window->getConfig();
    EGLint backbufferAlphaBits = 0;
    eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &backbufferAlphaBits);
    ANGLE_SKIP_TEST_IF(backbufferAlphaBits != 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    GLint colorUniformLocation =
        glGetUniformLocation(blueProgram, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 0.5f);
    ASSERT_GL_NO_ERROR();

    // Don't explicitly clear, but draw blue (make sure alpha is not cleared)
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Test clearing a RGBA8 Framebuffer
TEST_P(ClearTest, RGBA8Framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture texture;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_NEAR(0, 0, 128, 128, 128, 128, 1.0);
}

// Test uploading a texture and then clearing a RGBA8 Framebuffer
TEST_P(ClearTest, TextureUploadAndRGBA8Framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture texture;

    constexpr uint32_t kSize = 16;
    std::vector<GLColor> pixelData(kSize * kSize, GLColor::blue);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelData.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    EXPECT_PIXEL_NEAR(0, 0, 0, 0, 255, 255, 1.0);

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_NEAR(0, 0, 128, 128, 128, 128, 1.0);
}

// Test to validate that we can go from an RGBA framebuffer attachment, to an RGB one and still
// have a correct behavior after.
TEST_P(ClearTest, ChangeFramebufferAttachmentFromRGBAtoRGB)
{
    // http://anglebug.com/40096508
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsAdreno() && IsOpenGLES());

    // http://anglebug.com/40644765
    ANGLE_SKIP_TEST_IF(IsMac() && IsDesktopOpenGL() && IsIntel());

    ANGLE_GL_PROGRAM(program, angle::essl1_shaders::vs::Simple(),
                     angle::essl1_shaders::fs::UniformColor());
    setupQuadVertexBuffer(0.5f, 1.0f);
    glUseProgram(program);
    GLint positionLocation = glGetAttribLocation(program, angle::essl1_shaders::PositionAttrib());
    ASSERT_NE(positionLocation, -1);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 1.0f, 0.5f);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Initially clear to black.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Clear with masked color.
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE);
    glClearColor(0.5f, 0.5f, 0.5f, 0.75f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // So far so good, we have an RGBA framebuffer that we've cleared to 0.5 everywhere.
    EXPECT_PIXEL_NEAR(0, 0, 128, 0, 128, 192, 1.0);

    // In the Vulkan backend, RGB textures are emulated with an RGBA texture format
    // underneath and we keep a special mask to know that we shouldn't touch the alpha
    // channel when we have that emulated texture. This test exists to validate that
    // this mask gets updated correctly when the framebuffer attachment changes.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::magenta);
}

// Test clearing a RGB8 Framebuffer with a color mask.
TEST_P(ClearTest, RGB8WithMaskFramebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture texture;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glClearColor(0.2f, 0.4f, 0.6f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Since there's no alpha, we expect to get 255 back instead of the clear value (204).
    EXPECT_PIXEL_NEAR(0, 0, 51, 102, 153, 255, 1.0);

    glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);
    glClearColor(0.1f, 0.3f, 0.5f, 0.7f);
    glClear(GL_COLOR_BUFFER_BIT);

    // The blue channel was masked so its value should be unchanged.
    EXPECT_PIXEL_NEAR(0, 0, 26, 77, 153, 255, 1.0);

    // Restore default.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

TEST_P(ClearTest, ClearIssue)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClearDepthf(0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, 16, 16);

    EXPECT_GL_NO_ERROR();

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    EXPECT_GL_NO_ERROR();

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Regression test for a bug where "glClearDepthf"'s argument was not clamped
// In GLES 2 they where declared as GLclampf and the behaviour is the same in GLES 3.2
TEST_P(ClearTest, ClearIsClamped)
{
    glClearDepthf(5.0f);

    GLfloat clear_depth;
    glGetFloatv(GL_DEPTH_CLEAR_VALUE, &clear_depth);
    EXPECT_EQ(1.0f, clear_depth);
}

// Regression test for a bug where "glDepthRangef"'s arguments were not clamped
// In GLES 2 they where declared as GLclampf and the behaviour is the same in GLES 3.2
TEST_P(ClearTest, DepthRangefIsClamped)
{
    glDepthRangef(1.1f, -4.0f);

    GLfloat depth_range[2];
    glGetFloatv(GL_DEPTH_RANGE, depth_range);
    EXPECT_EQ(1.0f, depth_range[0]);
    EXPECT_EQ(0.0f, depth_range[1]);
}

// Test scissored clears on Depth16
TEST_P(ClearTest, Depth16Scissored)
{
    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    constexpr int kRenderbufferSize = 64;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kRenderbufferSize,
                          kRenderbufferSize);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

    glClearDepthf(0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    constexpr int kNumSteps = 13;
    for (int ndx = 1; ndx < kNumSteps; ndx++)
    {
        float perc = static_cast<float>(ndx) / static_cast<float>(kNumSteps);
        glScissor(0, 0, static_cast<int>(kRenderbufferSize * perc),
                  static_cast<int>(kRenderbufferSize * perc));
        glClearDepthf(perc);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
}

// Test scissored clears on Stencil8
TEST_P(ClearTest, Stencil8Scissored)
{
    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    constexpr int kRenderbufferSize = 64;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, kRenderbufferSize, kRenderbufferSize);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    constexpr int kNumSteps = 13;
    for (int ndx = 1; ndx < kNumSteps; ndx++)
    {
        float perc = static_cast<float>(ndx) / static_cast<float>(kNumSteps);
        glScissor(0, 0, static_cast<int>(kRenderbufferSize * perc),
                  static_cast<int>(kRenderbufferSize * perc));
        glClearStencil(static_cast<int>(perc * 255.0f));
        glClear(GL_STENCIL_BUFFER_BIT);
    }
}

// Covers a bug in the Vulkan back-end where starting a new command buffer in
// the masked clear would not trigger descriptor sets to be re-bound.
TEST_P(ClearTest, MaskedClearThenDrawWithUniform)
{
    // Initialize a program with a uniform.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);

    GLint uniLoc = glGetUniformLocation(program, essl1_shaders::ColorUniform());
    ASSERT_NE(-1, uniLoc);
    glUniform4f(uniLoc, 0.0f, 1.0f, 0.0f, 1.0f);

    // Initialize position attribute.
    GLint posLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);
    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    // Initialize a simple FBO.
    constexpr GLsizei kSize = 2;
    GLTexture clearTexture;
    glBindTexture(GL_TEXTURE_2D, clearTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, clearTexture, 0);

    glViewport(0, 0, kSize, kSize);

    // Clear and draw to flush out dirty bits.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Flush to trigger a new serial.
    glFlush();

    // Enable color mask and draw again to trigger the bug.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Clear with a mask to verify that masked clear is done properly
// (can't use inline or RenderOp clear when some color channels are masked)
TEST_P(ClearTestES3, ClearPlusMaskDrawAndClear)
{
    // Initialize a program with a uniform.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);

    GLint uniLoc = glGetUniformLocation(program, essl1_shaders::ColorUniform());
    ASSERT_NE(-1, uniLoc);
    glUniform4f(uniLoc, 0.0f, 1.0f, 0.0f, 1.0f);

    // Initialize position attribute.
    GLint posLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);
    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    // Initialize a simple FBO.
    constexpr GLsizei kSize = 2;
    GLTexture clearTexture;
    glBindTexture(GL_TEXTURE_2D, clearTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, clearTexture, 0);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, kSize, kSize);

    // Clear and draw to flush out dirty bits.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw green rectangle
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Enable color mask and draw again to trigger the bug.
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw purple-ish rectangle, green should be masked off
    glUniform4f(uniLoc, 1.0f, 0.25f, 1.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that clearing all buffers through glClearColor followed by a clear of a specific buffer
// clears to the correct values.
TEST_P(ClearTestES3, ClearMultipleAttachmentsFollowedBySpecificOne)
{
    constexpr uint32_t kSize            = 16;
    constexpr uint32_t kAttachmentCount = 4;
    std::vector<unsigned char> pixelData(kSize * kSize * 4, 255);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures[kAttachmentCount];
    GLenum drawBuffers[kAttachmentCount];
    GLColor clearValues[kAttachmentCount];

    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixelData.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i],
                               0);
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

        clearValues[i].R = static_cast<GLubyte>(1 + i * 20);
        clearValues[i].G = static_cast<GLubyte>(7 + i * 20);
        clearValues[i].B = static_cast<GLubyte>(12 + i * 20);
        clearValues[i].A = static_cast<GLubyte>(16 + i * 20);
    }

    glDrawBuffers(kAttachmentCount, drawBuffers);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    // Clear all targets.
    angle::Vector4 clearColor = clearValues[0].toNormalizedVector();
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Clear odd targets individually.
    for (uint32_t i = 1; i < kAttachmentCount; i += 2)
    {
        clearColor = clearValues[i].toNormalizedVector();
        glClearBufferfv(GL_COLOR, i, clearColor.data());
    }

    // Even attachments should be cleared to color 0, while odd attachments are cleared to their
    // respective color.
    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        ASSERT_GL_NO_ERROR();

        uint32_t clearIndex   = i % 2 == 0 ? 0 : i;
        const GLColor &expect = clearValues[clearIndex];

        EXPECT_PIXEL_COLOR_EQ(0, 0, expect);
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, expect);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, expect);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, expect);
    }
}

// Test that clearing each render target individually works.  In the Vulkan backend, this should be
// done in a single render pass.
TEST_P(ClearTestES3, ClearMultipleAttachmentsIndividually)
{
    // http://anglebug.com/40096714
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    constexpr uint32_t kSize             = 16;
    constexpr uint32_t kAttachmentCount  = 2;
    constexpr float kDepthClearValue     = 0.125f;
    constexpr int32_t kStencilClearValue = 0x67;
    std::vector<unsigned char> pixelData(kSize * kSize * 4, 255);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures[kAttachmentCount];
    GLRenderbuffer depthStencil;
    GLenum drawBuffers[kAttachmentCount];
    GLColor clearValues[kAttachmentCount];

    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixelData.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i],
                               0);
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

        clearValues[i].R = static_cast<GLubyte>(1 + i * 20);
        clearValues[i].G = static_cast<GLubyte>(7 + i * 20);
        clearValues[i].B = static_cast<GLubyte>(12 + i * 20);
        clearValues[i].A = static_cast<GLubyte>(16 + i * 20);
    }

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    glDrawBuffers(kAttachmentCount, drawBuffers);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glClearBufferfv(GL_COLOR, i, clearValues[i].toNormalizedVector().data());
    }

    glClearBufferfv(GL_DEPTH, 0, &kDepthClearValue);
    glClearBufferiv(GL_STENCIL, 0, &kStencilClearValue);
    ASSERT_GL_NO_ERROR();

    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        ASSERT_GL_NO_ERROR();

        const GLColor &expect = clearValues[i];

        EXPECT_PIXEL_COLOR_EQ(0, 0, expect);
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, expect);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, expect);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, expect);
    }

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    for (uint32_t i = 1; i < kAttachmentCount; ++i)
        drawBuffers[i] = GL_NONE;
    glDrawBuffers(kAttachmentCount, drawBuffers);

    verifyDepth(kDepthClearValue, kSize);
    verifyStencil(kStencilClearValue, kSize);
}

// Test that clearing multiple attachments in the presence of a color mask, scissor or both
// correctly clears all the attachments.
TEST_P(ClearTestES3, MaskedScissoredClearMultipleAttachments)
{
    constexpr uint32_t kSize            = 16;
    constexpr uint32_t kAttachmentCount = 2;
    std::vector<unsigned char> pixelData(kSize * kSize * 4, 255);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures[kAttachmentCount];
    GLenum drawBuffers[kAttachmentCount];

    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixelData.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i],
                               0);
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glDrawBuffers(kAttachmentCount, drawBuffers);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    // Masked clear
    GLColor clearColorMasked(31, 63, 255, 191);
    angle::Vector4 clearColor = GLColor(31, 63, 127, 191).toNormalizedVector();

    glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // All attachments should be cleared, with the blue channel untouched
    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, clearColorMasked);
    }

    // Masked scissored clear
    GLColor clearColorMaskedScissored(63, 127, 255, 31);
    clearColor = GLColor(63, 127, 191, 31).toNormalizedVector();

    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glEnable(GL_SCISSOR_TEST);
    glScissor(kSize / 6, kSize / 6, kSize / 3, kSize / 3);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // The corners should keep the previous value while the center is cleared, except its blue
    // channel.
    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(kSize / 3, 2 * kSize / 3, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3, kSize / 3, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3, 2 * kSize / 3, clearColorMasked);

        EXPECT_PIXEL_COLOR_EQ(kSize / 3, kSize / 3, clearColorMaskedScissored);
    }

    // Scissored clear
    GLColor clearColorScissored(127, 191, 31, 63);
    clearColor = GLColor(127, 191, 31, 63).toNormalizedVector();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // The corners should keep the old value while all channels of the center are cleared.
    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(kSize / 3, 2 * kSize / 3, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3, kSize / 3, clearColorMasked);
        EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3, 2 * kSize / 3, clearColorMasked);

        EXPECT_PIXEL_COLOR_EQ(kSize / 3, kSize / 3, clearColorScissored);
    }
}

// Test clearing multiple attachments in the presence of an indexed color mask.
TEST_P(ClearTestES3, MaskedIndexedClearMultipleAttachments)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_draw_buffers_indexed"));

    constexpr uint32_t kSize            = 16;
    constexpr uint32_t kAttachmentCount = 4;
    std::vector<unsigned char> pixelData(kSize * kSize * 4, 255);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures[kAttachmentCount];
    GLenum drawBuffers[kAttachmentCount];

    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixelData.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i],
                               0);
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glDrawBuffers(kAttachmentCount, drawBuffers);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    // Masked clear
    GLColor clearColorMasked(31, 63, 255, 191);
    angle::Vector4 clearColor = GLColor(31, 63, 127, 191).toNormalizedVector();

    // Block blue channel for all attachements
    glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);

    // Unblock blue channel for attachments 0 and 1
    glColorMaskiOES(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glColorMaskiOES(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // All attachments should be cleared, with the blue channel untouched for all attachments but 1.
    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        ASSERT_GL_NO_ERROR();

        const GLColor attachmentColor = (i > 1) ? clearColorMasked : clearColor;
        EXPECT_PIXEL_COLOR_EQ(0, 0, attachmentColor);
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, attachmentColor);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, attachmentColor);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, attachmentColor);
        EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, attachmentColor);
    }
}

// Test that clearing multiple attachments of different nature (float, int and uint) in the
// presence of a color mask works correctly.  In the Vulkan backend, this exercises clearWithDraw
// and the relevant internal shaders.
TEST_P(ClearTestES3, MaskedClearHeterogeneousAttachments)
{
    // http://anglebug.com/40096714
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    constexpr uint32_t kSize                              = 16;
    constexpr uint32_t kAttachmentCount                   = 3;
    constexpr float kDepthClearValue                      = 0.256f;
    constexpr int32_t kStencilClearValue                  = 0x1D;
    constexpr GLenum kAttachmentFormats[kAttachmentCount] = {
        GL_RGBA8,
        GL_RGBA8I,
        GL_RGBA8UI,
    };
    constexpr GLenum kDataFormats[kAttachmentCount] = {
        GL_RGBA,
        GL_RGBA_INTEGER,
        GL_RGBA_INTEGER,
    };
    constexpr GLenum kDataTypes[kAttachmentCount] = {
        GL_UNSIGNED_BYTE,
        GL_BYTE,
        GL_UNSIGNED_BYTE,
    };

    std::vector<unsigned char> pixelData(kSize * kSize * 4, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures[kAttachmentCount];
    GLRenderbuffer depthStencil;
    GLenum drawBuffers[kAttachmentCount];

    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, kAttachmentFormats[i], kSize, kSize, 0, kDataFormats[i],
                     kDataTypes[i], pixelData.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i],
                               0);
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    glDrawBuffers(kAttachmentCount, drawBuffers);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);

    // Mask out red for all clears
    glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);

    glClearBufferfv(GL_DEPTH, 0, &kDepthClearValue);
    glClearBufferiv(GL_STENCIL, 0, &kStencilClearValue);

    GLColor clearValuef = {25, 50, 75, 100};
    glClearBufferfv(GL_COLOR, 0, clearValuef.toNormalizedVector().data());

    int clearValuei[4] = {10, -20, 30, -40};
    glClearBufferiv(GL_COLOR, 1, clearValuei);

    uint32_t clearValueui[4] = {50, 60, 70, 80};
    glClearBufferuiv(GL_COLOR, 2, clearValueui);

    ASSERT_GL_NO_ERROR();

    {
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        ASSERT_GL_NO_ERROR();

        GLColor expect = clearValuef;
        expect.R       = 0;

        EXPECT_PIXEL_COLOR_EQ(0, 0, expect);
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, expect);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, expect);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, expect);
    }

    {
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_8I(0, 0, 0, clearValuei[1], clearValuei[2], clearValuei[3]);
        EXPECT_PIXEL_8I(0, kSize - 1, 0, clearValuei[1], clearValuei[2], clearValuei[3]);
        EXPECT_PIXEL_8I(kSize - 1, 0, 0, clearValuei[1], clearValuei[2], clearValuei[3]);
        EXPECT_PIXEL_8I(kSize - 1, kSize - 1, 0, clearValuei[1], clearValuei[2], clearValuei[3]);
    }

    {
        glReadBuffer(GL_COLOR_ATTACHMENT2);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_8UI(0, 0, 0, clearValueui[1], clearValueui[2], clearValueui[3]);
        EXPECT_PIXEL_8UI(0, kSize - 1, 0, clearValueui[1], clearValueui[2], clearValueui[3]);
        EXPECT_PIXEL_8UI(kSize - 1, 0, 0, clearValueui[1], clearValueui[2], clearValueui[3]);
        EXPECT_PIXEL_8UI(kSize - 1, kSize - 1, 0, clearValueui[1], clearValueui[2],
                         clearValueui[3]);
    }

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    for (uint32_t i = 1; i < kAttachmentCount; ++i)
        drawBuffers[i] = GL_NONE;
    glDrawBuffers(kAttachmentCount, drawBuffers);

    verifyDepth(kDepthClearValue, kSize);
    verifyStencil(kStencilClearValue, kSize);
}

// Test that clearing multiple attachments of different nature (float, int and uint) in the
// presence of a scissor test works correctly.  In the Vulkan backend, this exercises clearWithDraw
// and the relevant internal shaders.
TEST_P(ClearTestES3, ScissoredClearHeterogeneousAttachments)
{
    // http://anglebug.com/40096714
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    // http://anglebug.com/42263682
    ANGLE_SKIP_TEST_IF(IsWindows() && (IsOpenGL() || IsD3D11()) && IsAMD());

    // http://anglebug.com/42263790
    ANGLE_SKIP_TEST_IF(IsWindows7() && IsD3D11() && IsNVIDIA());

    constexpr uint32_t kSize                              = 16;
    constexpr uint32_t kHalfSize                          = kSize / 2;
    constexpr uint32_t kAttachmentCount                   = 3;
    constexpr float kDepthClearValue                      = 0.256f;
    constexpr int32_t kStencilClearValue                  = 0x1D;
    constexpr GLenum kAttachmentFormats[kAttachmentCount] = {
        GL_RGBA8,
        GL_RGBA8I,
        GL_RGBA8UI,
    };
    constexpr GLenum kDataFormats[kAttachmentCount] = {
        GL_RGBA,
        GL_RGBA_INTEGER,
        GL_RGBA_INTEGER,
    };
    constexpr GLenum kDataTypes[kAttachmentCount] = {
        GL_UNSIGNED_BYTE,
        GL_BYTE,
        GL_UNSIGNED_BYTE,
    };

    std::vector<unsigned char> pixelData(kSize * kSize * 4, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures[kAttachmentCount];
    GLRenderbuffer depthStencil;
    GLenum drawBuffers[kAttachmentCount];

    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, kAttachmentFormats[i], kSize, kSize, 0, kDataFormats[i],
                     kDataTypes[i], pixelData.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i],
                               0);
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    glDrawBuffers(kAttachmentCount, drawBuffers);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);

    // Enable scissor test
    glScissor(0, 0, kHalfSize, kHalfSize);
    glEnable(GL_SCISSOR_TEST);

    GLColor clearValuef         = {25, 50, 75, 100};
    angle::Vector4 clearValuefv = clearValuef.toNormalizedVector();

    glClearColor(clearValuefv.x(), clearValuefv.y(), clearValuefv.z(), clearValuefv.w());
    glClearDepthf(kDepthClearValue);

    // clear stencil.
    glClearBufferiv(GL_STENCIL, 0, &kStencilClearValue);

    // clear float color attachment & depth together
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // clear integer attachment.
    int clearValuei[4] = {10, -20, 30, -40};
    glClearBufferiv(GL_COLOR, 1, clearValuei);

    // clear unsigned integer attachment
    uint32_t clearValueui[4] = {50, 60, 70, 80};
    glClearBufferuiv(GL_COLOR, 2, clearValueui);

    ASSERT_GL_NO_ERROR();

    {
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        ASSERT_GL_NO_ERROR();

        GLColor expect = clearValuef;

        EXPECT_PIXEL_COLOR_EQ(0, 0, expect);
        EXPECT_PIXEL_COLOR_EQ(0, kHalfSize - 1, expect);
        EXPECT_PIXEL_COLOR_EQ(kHalfSize - 1, 0, expect);
        EXPECT_PIXEL_COLOR_EQ(kHalfSize - 1, kHalfSize - 1, expect);
        EXPECT_PIXEL_EQ(kHalfSize + 1, kHalfSize + 1, 0, 0, 0, 0);
    }

    {
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_8I(0, 0, clearValuei[0], clearValuei[1], clearValuei[2], clearValuei[3]);
        EXPECT_PIXEL_8I(0, kHalfSize - 1, clearValuei[0], clearValuei[1], clearValuei[2],
                        clearValuei[3]);
        EXPECT_PIXEL_8I(kHalfSize - 1, 0, clearValuei[0], clearValuei[1], clearValuei[2],
                        clearValuei[3]);
        EXPECT_PIXEL_8I(kHalfSize - 1, kHalfSize - 1, clearValuei[0], clearValuei[1],
                        clearValuei[2], clearValuei[3]);
        EXPECT_PIXEL_8I(kHalfSize + 1, kHalfSize + 1, 0, 0, 0, 0);
    }

    {
        glReadBuffer(GL_COLOR_ATTACHMENT2);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_8UI(0, 0, clearValueui[0], clearValueui[1], clearValueui[2], clearValueui[3]);
        EXPECT_PIXEL_8UI(0, kHalfSize - 1, clearValueui[0], clearValueui[1], clearValueui[2],
                         clearValueui[3]);
        EXPECT_PIXEL_8UI(kHalfSize - 1, 0, clearValueui[0], clearValueui[1], clearValueui[2],
                         clearValueui[3]);
        EXPECT_PIXEL_8UI(kHalfSize - 1, kHalfSize - 1, clearValueui[0], clearValueui[1],
                         clearValueui[2], clearValueui[3]);
        EXPECT_PIXEL_8UI(kHalfSize + 1, kHalfSize + 1, 0, 0, 0, 0);
    }

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    for (uint32_t i = 1; i < kAttachmentCount; ++i)
        drawBuffers[i] = GL_NONE;
    glDrawBuffers(kAttachmentCount, drawBuffers);

    verifyDepth(kDepthClearValue, kHalfSize);
    verifyStencil(kStencilClearValue, kHalfSize);
}

// This tests a bug where in a masked clear when calling "ClearBuffer", we would
// mistakenly clear every channel (including the masked-out ones)
TEST_P(ClearTestES3, MaskedClearBufferBug)
{
    unsigned char pixelData[] = {255, 255, 255, 255};

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures[2];

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[1], 0);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 255, 255, 255);

    float clearValue[]   = {0, 0.5f, 0.5f, 1.0f};
    GLenum drawBuffers[] = {GL_NONE, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);
    glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);
    glClearBufferfv(GL_COLOR, 1, clearValue);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 255, 255, 255);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_NEAR(0, 0, 0, 127, 255, 255, 1);
}

// Test that stencil clears works if reference and write mask have no common bits.  The write mask
// is the only thing that dictates which bits should be written to, and this is a regression test
// for a bug where the clear was no-oped if the (reference & writemask) == 0 instead of just
// writemask == 0.
TEST_P(ClearTestES3, ClearStencilWithNonOverlappingWriteMaskAndReferenceBits)
{
    constexpr uint32_t kSize = 16;

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures;
    GLRenderbuffer depthStencil;

    glBindTexture(GL_TEXTURE_2D, textures);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    // Initialize the stencil buffer.
    glClearDepthf(0);
    glClearStencil(0xEC);

    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    verifyStencil(0xEC, kSize);

    // Clear the color buffer again to make sure there are no stale data.
    glClearColor(0.25, 0.5, 0.75, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_NEAR(0, 0, 63, 127, 191, 255, 1.0);

    // Set the stencil write mask to 0xF0
    glStencilMask(0xF0);

    // Set the stencil reference to 0x0F.  It shouldn't matter
    glStencilFunc(GL_EQUAL, 0x55, 0x0F);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);

    // Clear stencil again.  Only the top four bits should be written.
    const GLint kStencilClearValue = 0x59;
    glClearBufferiv(GL_STENCIL, 0, &kStencilClearValue);
    verifyStencil(0x5C, kSize);
}

// Regression test for a serial tracking bug.
TEST_P(ClearTestES3, BadFBOSerialBug)
{
    // First make a simple framebuffer, and clear it to green
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures[2];

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    float clearValues1[] = {0.0f, 1.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, clearValues1);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Next make a second framebuffer, and draw it to red
    // (Triggers bad applied render target serial)
    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    ASSERT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);

    glDrawBuffers(1, drawBuffers);

    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Check that the first framebuffer is still green.
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that SRGB framebuffers clear to the linearized clear color
TEST_P(ClearTestES3, SRGBClear)
{
    // First make a simple framebuffer, and clear it
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture texture;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_NEAR(0, 0, 188, 188, 188, 128, 1.0);
}

// Test that framebuffers with mixed SRGB/Linear attachments clear to the correct color for each
// attachment
TEST_P(ClearTestES3, MixedSRGBClear)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);

    GLTexture textures[2];

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[1], 0);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    // Clear both textures
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);

    // Check value of texture0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);
    EXPECT_PIXEL_NEAR(0, 0, 188, 188, 188, 128, 1.0);

    // Check value of texture1
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);
    EXPECT_PIXEL_NEAR(0, 0, 128, 128, 128, 128, 1.0);
}

// This test covers a D3D11 bug where calling ClearRenderTargetView sometimes wouldn't sync
// before a draw call. The test draws small quads to a larger FBO (the default back buffer).
// Before each blit to the back buffer it clears the quad to a certain color using
// ClearBufferfv to give a solid color. The sync problem goes away if we insert a call to
// flush or finish after ClearBufferfv or each draw.
TEST_P(ClearTestES3, RepeatedClear)
{
    // Fails on 431.02 driver. http://anglebug.com/40644697
    ANGLE_SKIP_TEST_IF(IsWindows() && IsNVIDIA() && IsVulkan());
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    constexpr char kVS[] =
        "#version 300 es\n"
        "in highp vec2 position;\n"
        "out highp vec2 v_coord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    vec2 texCoord = (position * 0.5) + 0.5;\n"
        "    v_coord = texCoord;\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "in highp vec2 v_coord;\n"
        "out highp vec4 color;\n"
        "uniform sampler2D tex;\n"
        "void main()\n"
        "{\n"
        "    color = texture(tex, v_coord);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    mTextures.resize(1, 0);
    glGenTextures(1, mTextures.data());

    GLenum format           = GL_RGBA8;
    const int numRowsCols   = 3;
    const int cellSize      = 32;
    const int fboSize       = cellSize;
    const int backFBOSize   = cellSize * numRowsCols;
    const float fmtValueMin = 0.0f;
    const float fmtValueMax = 1.0f;

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, format, fboSize, fboSize);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);
    ASSERT_GL_NO_ERROR();

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // larger fbo bound -- clear to transparent black
    glUseProgram(program);
    GLint uniLoc = glGetUniformLocation(program, "tex");
    ASSERT_NE(-1, uniLoc);
    glUniform1i(uniLoc, 0);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);

    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);

    glUseProgram(program);

    for (int cellY = 0; cellY < numRowsCols; cellY++)
    {
        for (int cellX = 0; cellX < numRowsCols; cellX++)
        {
            int seed            = cellX + cellY * numRowsCols;
            const Vector4 color = RandomVec4(seed, fmtValueMin, fmtValueMax);

            glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);
            glClearBufferfv(GL_COLOR, 0, color.data());

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Method 1: Set viewport and draw full-viewport quad
            glViewport(cellX * cellSize, cellY * cellSize, cellSize, cellSize);
            drawQuad(program, "position", 0.5f);

            // Uncommenting the glFinish call seems to make the test pass.
            // glFinish();
        }
    }

    std::vector<GLColor> pixelData(backFBOSize * backFBOSize);
    glReadPixels(0, 0, backFBOSize, backFBOSize, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

    for (int cellY = 0; cellY < numRowsCols; cellY++)
    {
        for (int cellX = 0; cellX < numRowsCols; cellX++)
        {
            int seed            = cellX + cellY * numRowsCols;
            const Vector4 color = RandomVec4(seed, fmtValueMin, fmtValueMax);
            GLColor expectedColor(color);

            int testN = cellX * cellSize + cellY * backFBOSize * cellSize + backFBOSize + 1;
            GLColor actualColor = pixelData[testN];
            EXPECT_NEAR(expectedColor.R, actualColor.R, 1);
            EXPECT_NEAR(expectedColor.G, actualColor.G, 1);
            EXPECT_NEAR(expectedColor.B, actualColor.B, 1);
            EXPECT_NEAR(expectedColor.A, actualColor.A, 1);
        }
    }

    ASSERT_GL_NO_ERROR();
}

// Test that clearing RGB8 attachments work when verified through sampling.
TEST_P(ClearTestES3, ClearRGB8)
{
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, 1, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the texture through framebuffer.
    const GLubyte kClearColor[] = {63, 127, 191, 55};
    glClearColor(kClearColor[0] / 255.0f, kClearColor[1] / 255.0f, kClearColor[2] / 255.0f,
                 kClearColor[3] / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Sample from it and verify clear is done.
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(), essl3_shaders::fs::Texture2DLod());
    glUseProgram(program);
    GLint textureLocation = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLocation);
    GLint lodLocation = glGetUniformLocation(program, essl3_shaders::LodUniform());
    ASSERT_NE(-1, lodLocation);

    glUniform1i(textureLocation, 0);
    glUniform1f(lodLocation, 0);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_NEAR(0, 0, kClearColor[0], kClearColor[1], kClearColor[2], 255, 1);
    ASSERT_GL_NO_ERROR();
}

// Test that clearing RGB8 attachments from a 2D texture array does not cause
// VUID-VkImageMemoryBarrier-oldLayout-01197
TEST_P(ClearTestES3, TextureArrayRGB8)
{
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, 1, 1, 2);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 0);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex, 0, 1);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, &bufs[0]);

    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);

    EXPECT_GL_NO_ERROR();
}

void MaskedScissoredClearTestBase::maskedScissoredColorDepthStencilClear(
    const MaskedScissoredClearVariationsTestParams &params)
{
    // Flaky on Android Nexus 5x and Pixel 2, possible Qualcomm driver bug.
    // TODO(jmadill): Re-enable when possible. http://anglebug.com/42261257
    ANGLE_SKIP_TEST_IF(IsOpenGLES() && IsAndroid());

    const int w      = getWindowWidth();
    const int h      = getWindowHeight();
    const int wthird = w / 3;
    const int hthird = h / 3;

    constexpr float kPreClearDepth     = 0.9f;
    constexpr float kClearDepth        = 0.5f;
    constexpr uint8_t kPreClearStencil = 0xFF;
    constexpr uint8_t kClearStencil    = 0x16;
    constexpr uint8_t kStencilMask     = 0x59;
    constexpr uint8_t kMaskedClearStencil =
        (kPreClearStencil & ~kStencilMask) | (kClearStencil & kStencilMask);

    bool clearColor, clearDepth, clearStencil;
    bool maskColor, maskDepth, maskStencil;
    bool scissor;

    ParseMaskedScissoredClearVariationsTestParams(params, &clearColor, &clearDepth, &clearStencil,
                                                  &maskColor, &maskDepth, &maskStencil, &scissor);

    // Clear to a random color, 0.9 depth and 0x00 stencil
    Vector4 color1(0.1f, 0.2f, 0.3f, 0.4f);
    GLColor color1RGB(color1);

    glClearColor(color1[0], color1[1], color1[2], color1[3]);
    glClearDepthf(kPreClearDepth);
    glClearStencil(kPreClearStencil);

    if (!clearColor)
    {
        // If not asked to clear color, clear it anyway, but individually.  The clear value is
        // still used to verify that the depth/stencil clear happened correctly.  This allows
        // testing for depth/stencil-only clear implementations.
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glClear((clearColor ? GL_COLOR_BUFFER_BIT : 0) | (clearDepth ? GL_DEPTH_BUFFER_BIT : 0) |
            (clearStencil ? GL_STENCIL_BUFFER_BIT : 0));
    ASSERT_GL_NO_ERROR();

    // Verify color was cleared correctly.
    EXPECT_PIXEL_COLOR_NEAR(0, 0, color1RGB, 1);

    if (scissor)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(wthird / 2, hthird / 2, wthird, hthird);
    }

    // Use color and stencil masks to clear to a second color, 0.5 depth and 0x59 stencil.
    Vector4 color2(0.2f, 0.4f, 0.6f, 0.8f);
    GLColor color2RGB(color2);
    glClearColor(color2[0], color2[1], color2[2], color2[3]);
    glClearDepthf(kClearDepth);
    glClearStencil(kClearStencil);
    if (maskColor)
    {
        glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
    }
    if (maskDepth)
    {
        glDepthMask(GL_FALSE);
    }
    if (maskStencil)
    {
        glStencilMask(kStencilMask);
    }
    glClear((clearColor ? GL_COLOR_BUFFER_BIT : 0) | (clearDepth ? GL_DEPTH_BUFFER_BIT : 0) |
            (clearStencil ? GL_STENCIL_BUFFER_BIT : 0));
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilMask(0xFF);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);
    ASSERT_GL_NO_ERROR();

    GLColor color2MaskedRGB(color2RGB[0], color1RGB[1], color2RGB[2], color1RGB[3]);

    // If not clearing color, the original color should be left both in the center and corners.  If
    // using a scissor, the corners should be left to the original color, while the center is
    // possibly changed.  If using a mask, the center (and corners if not scissored), changes to
    // the masked results.
    GLColor expectedCenterColorRGB = !clearColor ? color1RGB
                                     : maskColor ? color2MaskedRGB
                                                 : color2RGB;
    GLColor expectedCornerColorRGB = scissor ? color1RGB : expectedCenterColorRGB;

    // Verify second clear color mask worked as expected.
    EXPECT_PIXEL_COLOR_NEAR(wthird, hthird, expectedCenterColorRGB, 1);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, expectedCornerColorRGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, 0, expectedCornerColorRGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, h - 1, expectedCornerColorRGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, h - 1, expectedCornerColorRGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(wthird, 2 * hthird, expectedCornerColorRGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(2 * wthird, hthird, expectedCornerColorRGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(2 * wthird, 2 * hthird, expectedCornerColorRGB, 1);

    // If there is depth, but depth is not asked to be cleared, the depth buffer contains garbage,
    // so no particular behavior can be expected.
    if (clearDepth || !mHasDepth)
    {
        // We use a small shader to verify depth.
        ANGLE_GL_PROGRAM(depthTestProgram, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Blue());
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(maskDepth ? GL_GREATER : GL_EQUAL);
        // - If depth is cleared, but it's masked, kPreClearDepth should be in the depth buffer.
        // - If depth is cleared, but it's not masked, kClearDepth should be in the depth buffer.
        // - If depth is not cleared, the if above ensures there is no depth buffer at all,
        //   which means depth test will always pass.
        drawQuad(depthTestProgram, essl1_shaders::PositionAttrib(), maskDepth ? 1.0f : 0.0f);
        glDisable(GL_DEPTH_TEST);
        ASSERT_GL_NO_ERROR();

        // Either way, we expect blue to be written to the center.
        expectedCenterColorRGB = GLColor::blue;
        // If there is no depth, depth test always passes so the whole image must be blue.  Same if
        // depth write is masked.
        expectedCornerColorRGB =
            mHasDepth && scissor && !maskDepth ? expectedCornerColorRGB : GLColor::blue;

        EXPECT_PIXEL_COLOR_NEAR(wthird, hthird, expectedCenterColorRGB, 1);

        EXPECT_PIXEL_COLOR_NEAR(0, 0, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(w - 1, 0, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(0, h - 1, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(w - 1, h - 1, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(wthird, 2 * hthird, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(2 * wthird, hthird, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(2 * wthird, 2 * hthird, expectedCornerColorRGB, 1);
    }

    // If there is stencil, but it's not asked to be cleared, there is similarly no expectation.
    if (clearStencil || !mHasStencil)
    {
        // And another small shader to verify stencil.
        ANGLE_GL_PROGRAM(stencilTestProgram, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Green());
        glEnable(GL_STENCIL_TEST);
        // - If stencil is cleared, but it's masked, kMaskedClearStencil should be in the stencil
        //   buffer.
        // - If stencil is cleared, but it's not masked, kClearStencil should be in the stencil
        //   buffer.
        // - If stencil is not cleared, the if above ensures there is no stencil buffer at all,
        //   which means stencil test will always pass.
        glStencilFunc(GL_EQUAL, maskStencil ? kMaskedClearStencil : kClearStencil, 0xFF);
        drawQuad(stencilTestProgram, essl1_shaders::PositionAttrib(), 0.0f);
        glDisable(GL_STENCIL_TEST);
        ASSERT_GL_NO_ERROR();

        // Either way, we expect green to be written to the center.
        expectedCenterColorRGB = GLColor::green;
        // If there is no stencil, stencil test always passes so the whole image must be green.
        expectedCornerColorRGB = mHasStencil && scissor ? expectedCornerColorRGB : GLColor::green;

        EXPECT_PIXEL_COLOR_NEAR(wthird, hthird, expectedCenterColorRGB, 1);

        EXPECT_PIXEL_COLOR_NEAR(0, 0, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(w - 1, 0, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(0, h - 1, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(w - 1, h - 1, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(wthird, 2 * hthird, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(2 * wthird, hthird, expectedCornerColorRGB, 1);
        EXPECT_PIXEL_COLOR_NEAR(2 * wthird, 2 * hthird, expectedCornerColorRGB, 1);
    }
}

// Tests combinations of color, depth, stencil clears with or without masks or scissor.
TEST_P(MaskedScissoredClearTest, Test)
{
    maskedScissoredColorDepthStencilClear(GetParam());
}

// Tests combinations of color, depth, stencil clears with or without masks or scissor.
//
// This uses depth/stencil attachments that are single-channel, but are emulated with a format
// that has both channels.
TEST_P(VulkanClearTest, Test)
{
    bool clearColor, clearDepth, clearStencil;
    bool maskColor, maskDepth, maskStencil;
    bool scissor;

    ParseMaskedScissoredClearVariationsTestParams(GetParam(), &clearColor, &clearDepth,
                                                  &clearStencil, &maskColor, &maskDepth,
                                                  &maskStencil, &scissor);

    // We only care about clearing depth xor stencil.
    if (clearDepth == clearStencil)
    {
        return;
    }

    if (clearDepth)
    {
        // Creating a depth-only renderbuffer is an ES3 feature.
        ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
        bindColorDepthFBO();
    }
    else
    {
        bindColorStencilFBO();
    }

    maskedScissoredColorDepthStencilClear(GetParam());
}

// Tests that clearing a non existing attachment works.
TEST_P(ClearTest, ClearColorThenClearNonExistingDepthStencil)
{
    constexpr GLsizei kSize = 16;

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear color.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Clear depth/stencil.
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Read back color.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Tests that clearing a non existing attachment works.
TEST_P(ClearTestES3, ClearDepthStencilThenClearNonExistingColor)
{
    constexpr GLsizei kSize = 16;

    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear depth/stencil.
    glClearDepthf(1.0f);
    glClearStencil(0xAA);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Clear color.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
}

// Test that just clearing a nonexistent drawbuffer of the default framebuffer doesn't cause an
// assert.
TEST_P(ClearTestES3, ClearBuffer1OnDefaultFramebufferNoAssert)
{
    std::vector<GLuint> testUint(4);
    glClearBufferuiv(GL_COLOR, 1, testUint.data());
    std::vector<GLint> testInt(4);
    glClearBufferiv(GL_COLOR, 1, testInt.data());
    std::vector<GLfloat> testFloat(4);
    glClearBufferfv(GL_COLOR, 1, testFloat.data());
    EXPECT_GL_NO_ERROR();
}

// Clears many small concentric rectangles using scissor regions.
TEST_P(ClearTest, InceptionScissorClears)
{
    angle::RNG rng;

    constexpr GLuint kSize = 16;

    // Create a square user FBO so we have more control over the dimensions.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, kSize, kSize);

    // Draw small concentric squares using scissor.
    std::vector<GLColor> expectedColors;
    for (GLuint index = 0; index < (kSize - 1) / 2; index++)
    {
        // Do the first clear without the scissor.
        if (index > 0)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(index, index, kSize - (index * 2), kSize - (index * 2));
        }

        GLColor color = RandomColor(&rng);
        expectedColors.push_back(color);
        Vector4 floatColor = color.toNormalizedVector();
        glClearColor(floatColor[0], floatColor[1], floatColor[2], floatColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> actualColors(expectedColors.size());
    glReadPixels(0, kSize / 2, actualColors.size(), 1, GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());

    EXPECT_EQ(expectedColors, actualColors);
}

// Clears many small concentric rectangles using scissor regions.
TEST_P(ClearTest, DrawThenInceptionScissorClears)
{
    angle::RNG rng;

    constexpr GLuint kSize = 16;

    // Create a square user FBO so we have more control over the dimensions.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, kSize, kSize);

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw small concentric squares using scissor.
    std::vector<GLColor> expectedColors;
    for (GLuint index = 0; index < (kSize - 1) / 2; index++)
    {
        // Do the first clear without the scissor.
        if (index > 0)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(index, index, kSize - (index * 2), kSize - (index * 2));
        }

        GLColor color = RandomColor(&rng);
        expectedColors.push_back(color);
        Vector4 floatColor = color.toNormalizedVector();
        glClearColor(floatColor[0], floatColor[1], floatColor[2], floatColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> actualColors(expectedColors.size());
    glReadPixels(0, kSize / 2, actualColors.size(), 1, GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());

    EXPECT_EQ(expectedColors, actualColors);
}

// Test that clearBuffer with disabled non-zero drawbuffer or disabled read source doesn't cause an
// assert.
TEST_P(ClearTestES3, ClearDisabledNonZeroAttachmentNoAssert)
{
    // http://anglebug.com/40644728
    ANGLE_SKIP_TEST_IF(IsMac() && IsDesktopOpenGL());

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    GLRenderbuffer rb;
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 16, 16);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rb);
    glDrawBuffers(0, nullptr);
    glReadBuffer(GL_NONE);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    float clearColorf[4] = {0.5, 0.5, 0.5, 0.5};
    glClearBufferfv(GL_COLOR, 1, clearColorf);

    GLuint clearColorui[4] = {255, 255, 255, 255};
    glClearBufferuiv(GL_COLOR, 1, clearColorui);

    GLint clearColori[4] = {-127, -127, -127, -127};
    glClearBufferiv(GL_COLOR, 1, clearColori);

    EXPECT_GL_NO_ERROR();
}

// Test that having a framebuffer with maximum number of attachments and clearing color, depth and
// stencil works.
TEST_P(ClearTestES3, ClearMaxAttachments)
{
    // http://anglebug.com/40644728
    ANGLE_SKIP_TEST_IF(IsMac() && IsDesktopOpenGL());
    // http://anglebug.com/42263935
    ANGLE_SKIP_TEST_IF(IsAMD() && IsD3D11());

    constexpr GLsizei kSize = 16;

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    ASSERT_GE(maxDrawBuffers, 4);

    // Setup framebuffer.
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    std::vector<GLRenderbuffer> color(maxDrawBuffers);
    std::vector<GLenum> drawBuffers(maxDrawBuffers);

    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, color[colorIndex]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorIndex,
                                  GL_RENDERBUFFER, color[colorIndex]);

        drawBuffers[colorIndex] = GL_COLOR_ATTACHMENT0 + colorIndex;
    }

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    EXPECT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glDrawBuffers(maxDrawBuffers, drawBuffers.data());

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify that every color attachment is cleared correctly.
    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + colorIndex);

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::red);
    }

    // Verify that depth and stencil attachments are cleared correctly.
    GLFramebuffer fbVerify;
    glBindFramebuffer(GL_FRAMEBUFFER, fbVerify);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    // If depth is not cleared to 1, rendering would fail.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // If stencil is not cleared to 0x55, rendering would fail.
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    // Draw green.
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.95f);

    // Verify that green was drawn.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::green);
}

// Test that having a framebuffer with maximum number of attachments and clearing color, depth and
// stencil after a draw call works.
TEST_P(ClearTestES3, ClearMaxAttachmentsAfterDraw)
{
    // http://anglebug.com/40644728
    ANGLE_SKIP_TEST_IF(IsMac() && IsDesktopOpenGL());

    constexpr GLsizei kSize = 16;

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    ASSERT_GE(maxDrawBuffers, 4);

    // Setup framebuffer.
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    std::vector<GLRenderbuffer> color(maxDrawBuffers);
    std::vector<GLenum> drawBuffers(maxDrawBuffers);

    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, color[colorIndex]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorIndex,
                                  GL_RENDERBUFFER, color[colorIndex]);

        drawBuffers[colorIndex] = GL_COLOR_ATTACHMENT0 + colorIndex;
    }

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    EXPECT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glDrawBuffers(maxDrawBuffers, drawBuffers.data());

    // Issue a draw call to render blue, depth=0 and stencil 0x3C to the attachments.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x3C, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    // Generate shader for this framebuffer.
    std::stringstream strstr;
    strstr << "#version 300 es\n"
              "precision highp float;\n";
    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        strstr << "layout(location = " << colorIndex << ") out vec4 value" << colorIndex << ";\n";
    }
    strstr << "void main()\n"
              "{\n";
    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        strstr << "value" << colorIndex << " = vec4(0.0f, 0.0f, 1.0f, 1.0f);\n";
    }
    strstr << "}\n";

    ANGLE_GL_PROGRAM(drawMRT, essl3_shaders::vs::Simple(), strstr.str().c_str());
    drawQuad(drawMRT, essl3_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify that every color attachment is cleared correctly.
    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + colorIndex);

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red) << colorIndex;
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::red) << colorIndex;
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::red) << colorIndex;
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::red) << colorIndex;
    }

    // Verify that depth and stencil attachments are cleared correctly.
    GLFramebuffer fbVerify;
    glBindFramebuffer(GL_FRAMEBUFFER, fbVerify);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    // If depth is not cleared to 1, rendering would fail.
    glDepthFunc(GL_LESS);

    // If stencil is not cleared to 0x55, rendering would fail.
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Draw green.
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.95f);

    // Verify that green was drawn.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::green);
}

// Test that mixed masked clear works after clear.
TEST_P(ClearTestES3, ClearThenMixedMaskedClear)
{
    constexpr GLsizei kSize = 16;

    // Setup framebuffer.
    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    EXPECT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear color and depth/stencil
    glClearColor(0.1f, 1.0f, 0.0f, 0.7f);
    glClearDepthf(0.0f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Clear again, but with color and stencil masked
    glClearColor(1.0f, 0.2f, 0.6f, 1.0f);
    glClearDepthf(1.0f);
    glClearStencil(0x3C);
    glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
    glStencilMask(0xF0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Issue a draw call to verify color, depth and stencil.

    // If depth is not cleared to 1, rendering would fail.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // If stencil is not cleared to 0x35, rendering would fail.
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x35, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Blend half-transparent blue into the color buffer.
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Verify that the color buffer is now gray
    const GLColor kExpected(127, 127, 127, 191);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kExpected, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, kSize - 1, kExpected, 1);
    EXPECT_PIXEL_COLOR_NEAR(kSize - 1, 0, kExpected, 1);
    EXPECT_PIXEL_COLOR_NEAR(kSize - 1, kSize - 1, kExpected, 1);
}

// Test that clearing stencil after a draw call works.
TEST_P(ClearTestES3, ClearStencilAfterDraw)
{
    // http://anglebug.com/40644728
    ANGLE_SKIP_TEST_IF(IsMac() && IsDesktopOpenGL());

    constexpr GLsizei kSize = 16;

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    ASSERT_GE(maxDrawBuffers, 4);

    // Setup framebuffer.
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    std::vector<GLRenderbuffer> color(maxDrawBuffers);
    std::vector<GLenum> drawBuffers(maxDrawBuffers);

    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, color[colorIndex]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorIndex,
                                  GL_RENDERBUFFER, color[colorIndex]);

        drawBuffers[colorIndex] = GL_COLOR_ATTACHMENT0 + colorIndex;
    }

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    EXPECT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glDrawBuffers(maxDrawBuffers, drawBuffers.data());

    // Issue a draw call to render blue and stencil 0x3C to the attachments.
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x3C, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    // Generate shader for this framebuffer.
    std::stringstream strstr;
    strstr << "#version 300 es\n"
              "precision highp float;\n";
    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        strstr << "layout(location = " << colorIndex << ") out vec4 value" << colorIndex << ";\n";
    }
    strstr << "void main()\n"
              "{\n";
    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        strstr << "value" << colorIndex << " = vec4(0.0f, 0.0f, 1.0f, 1.0f);\n";
    }
    strstr << "}\n";

    ANGLE_GL_PROGRAM(drawMRT, essl3_shaders::vs::Simple(), strstr.str().c_str());
    drawQuad(drawMRT, essl3_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify that every color attachment is cleared correctly.
    for (GLint colorIndex = 0; colorIndex < maxDrawBuffers; ++colorIndex)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + colorIndex);

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::red);
    }

    // Verify that depth and stencil attachments are cleared correctly.
    GLFramebuffer fbVerify;
    glBindFramebuffer(GL_FRAMEBUFFER, fbVerify);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);

    // If stencil is not cleared to 0x55, rendering would fail.
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Draw green.
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.95f);

    // Verify that green was drawn.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::green);
}

// Test that mid-render pass clearing of mixed used and unused color attachments works.
TEST_P(ClearTestES3, MixedRenderPassClearMixedUsedUnusedAttachments)
{
    // http://anglebug.com/40644728
    ANGLE_SKIP_TEST_IF(IsMac() && IsDesktopOpenGL());

    constexpr GLsizei kSize = 16;

    // Setup framebuffer.
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    GLRenderbuffer color[2];

    for (GLint colorIndex = 0; colorIndex < 2; ++colorIndex)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, color[colorIndex]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorIndex,
                                  GL_RENDERBUFFER, color[colorIndex]);
    }
    EXPECT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Disable color attachment 0.
    GLenum drawBuffers[] = {GL_NONE, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    // Draw into color attachment 1
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 color0;
layout(location = 1) out vec4 color1;
void main()
{
    color0 = vec4(0, 0, 1, 1);
    color1 = vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(drawMRT, essl3_shaders::vs::Simple(), kFS);
    drawQuad(drawMRT, essl3_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Color attachment 0 is now uninitialized, while color attachment 1 is red.
    // Re-enable color attachment 0 and clear both attachments to green.
    drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(2, drawBuffers);

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Verify that both color attachments are now green.
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_GL_NO_ERROR();
}

// Test that draw without state change after masked clear works
TEST_P(ClearTestES3, DrawClearThenDrawWithoutStateChange)
{
    swapBuffers();
    constexpr GLsizei kSize = 16;

    // Setup framebuffer.
    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
    EXPECT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear color initially.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Mask color.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Initialize position attribute.
    GLint posLoc = glGetAttribLocation(drawColor, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);
    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    // Draw red.
    glViewport(0, 0, kSize, kSize);
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 0.5f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Clear to green without any state change.
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw red again without any state change.
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Verify that the color buffer is now red
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::red);
}

// Test that clear stencil value is correctly masked to 8 bits.
TEST_P(ClearTest, ClearStencilMask)
{
    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    EXPECT_EQ(stencilBits, 8);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(drawColor);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Clear stencil value must be masked to 0x42
    glClearStencil(0x142);
    glClear(GL_STENCIL_BUFFER_BIT);

    // Check that the stencil test works as expected
    glEnable(GL_STENCIL_TEST);

    // Negative case
    glStencilFunc(GL_NOTEQUAL, 0x42, 0xFF);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Positive case
    glStencilFunc(GL_EQUAL, 0x42, 0xFF);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// Test that glClearBufferiv correctly masks the clear stencil value.
TEST_P(ClearTestES3, ClearBufferivStencilMask)
{
    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    EXPECT_EQ(stencilBits, 8);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(drawColor);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Clear stencil value must be masked to 0x42
    const GLint kStencilClearValue = 0x142;
    glClearBufferiv(GL_STENCIL, 0, &kStencilClearValue);

    // Check that the stencil test works as expected
    glEnable(GL_STENCIL_TEST);

    // Negative case
    glStencilFunc(GL_NOTEQUAL, 0x42, 0xFF);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Positive case
    glStencilFunc(GL_EQUAL, 0x42, 0xFF);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// Test that glClearBufferfi correctly masks the clear stencil value.
TEST_P(ClearTestES3, ClearBufferfiStencilMask)
{
    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    EXPECT_EQ(stencilBits, 8);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(drawColor);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Clear stencil value must be masked to 0x42
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.5f, 0x142);

    // Check that the stencil test works as expected
    glEnable(GL_STENCIL_TEST);

    // Negative case
    glStencilFunc(GL_NOTEQUAL, 0x42, 0xFF);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Positive case
    glStencilFunc(GL_EQUAL, 0x42, 0xFF);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// Test that glClearBufferfi works when stencil attachment is not present.
TEST_P(ClearTestES3, ClearBufferfiNoStencilAttachment)
{
    constexpr GLsizei kSize = 16;

    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kSize, kSize);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    // Clear depth/stencil with glClearBufferfi.  Note that the stencil attachment doesn't exist.
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.5f, 0x55);
    EXPECT_GL_NO_ERROR();

    // Verify depth is cleared correctly.
    verifyDepth(0.5f, kSize);
}

// Test that scissored clear followed by non-scissored draw works.  Ensures that when scissor size
// is expanded, the clear operation remains limited to the scissor region.  Written to catch
// potential future bugs if loadOp=CLEAR is used in the Vulkan backend for a small render pass and
// then the render area is mistakenly enlarged.
TEST_P(ClearTest, ScissoredClearThenNonScissoredDraw)
{
    constexpr GLsizei kSize = 16;
    const std::vector<GLColor> kInitialData(kSize * kSize, GLColor::red);

    // Setup framebuffer.  Initialize color with red.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kInitialData.data());

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    EXPECT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Issue a scissored clear to green.
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glScissor(kSize / 2, 0, kSize / 2, kSize);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    // Expand the scissor and blend blue into the framebuffer.
    glScissor(0, 0, kSize, kSize);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    ANGLE_GL_PROGRAM(drawBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(drawBlue, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Verify that the left half is magenta, and the right half is cyan.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2 - 1, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2 - 1, kSize - 1, GLColor::magenta);

    EXPECT_PIXEL_COLOR_EQ(kSize / 2, 0, GLColor::cyan);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize - 1, GLColor::cyan);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::cyan);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::cyan);
}

// Test that clear followed by a scissored masked clear works.
TEST_P(ClearTest, ClearThenScissoredMaskedClear)
{
    constexpr GLsizei kSize = 16;

    // Setup framebuffer
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    EXPECT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear to red.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Mask red and clear to green with a scissor
    glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
    glScissor(0, 0, kSize / 2, kSize);
    glEnable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Verify that the left half is yellow, and the right half is red.
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize / 2, kSize, GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ(kSize / 2, 0, kSize / 2, kSize, GLColor::red);
}

// Test that a scissored stencil clear followed by a full clear works.
TEST_P(ClearTestES3, StencilScissoredClearThenFullClear)
{
    constexpr GLsizei kSize = 128;

    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    EXPECT_EQ(stencilBits, 8);

    // Clear stencil value must be masked to 0x42
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.5f, 0x142);

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Shrink the render area.
    glScissor(kSize / 2, 0, kSize / 2, kSize);
    glEnable(GL_SCISSOR_TEST);

    // Clear stencil.
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.5f, 0x64);

    // Grow the render area.
    glScissor(0, 0, kSize, kSize);
    glEnable(GL_SCISSOR_TEST);

    // Check that the stencil test works as expected
    glEnable(GL_STENCIL_TEST);

    // Scissored region is green, outside is red (clear color)
    glStencilFunc(GL_EQUAL, 0x64, 0xFF);
    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    glUseProgram(drawGreen);
    drawQuad(drawGreen, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize / 2, kSize, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(kSize / 2, 0, kSize / 2, kSize, GLColor::green);

    // Outside scissored region is blue.
    glStencilFunc(GL_EQUAL, 0x42, 0xFF);
    ANGLE_GL_PROGRAM(drawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());
    glUseProgram(drawBlue);
    drawQuad(drawBlue, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize / 2, kSize, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(kSize / 2, 0, kSize / 2, kSize, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// This is a test that must be verified visually.
//
// Tests that clear of the default framebuffer applies to the window.
TEST_P(ClearTest, DISABLED_ClearReachesWindow)
{
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    // Draw blue.
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);
    swapBuffers();

    // Use glClear to clear to red.  Regression test for the Vulkan backend where this clear
    // remained "deferred" and didn't make it to the window on swap.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    swapBuffers();

    // Wait for visual verification.
    angle::Sleep(2000);
}

// Tests that masked clear after a no-op framebuffer binding change with an open render pass works.
TEST_P(ClearTest, DrawThenChangeFBOBindingAndBackThenMaskedClear)
{
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    // Draw blue.
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);

    // Change framebuffer and back
    glBindFramebuffer(GL_FRAMEBUFFER, mFBOs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Masked clear
    glColorMask(1, 0, 0, 1);
    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
}

// Test that clearing slices of a 3D texture and reading them back works.
TEST_P(ClearTestES3, ClearAndReadPixels3DTexture)
{
    constexpr uint32_t kWidth  = 128;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth  = 7;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, kWidth, kHeight, kDepth);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

    std::array<GLColor, kDepth> clearColors = {
        GLColor::red,  GLColor::green,   GLColor::blue,  GLColor::yellow,
        GLColor::cyan, GLColor::magenta, GLColor::white,
    };

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        glClearBufferfv(GL_COLOR, 0, clearColors[z].toNormalizedVector().data());
    }

    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        EXPECT_PIXEL_COLOR_EQ(0, 0, clearColors[z]);
    }
}

// Test that clearing stencil with zero first byte in mask doesn't crash.
TEST_P(ClearTestES3, ClearStencilZeroFirstByteMask)
{
    glStencilMask(0xe7d6a900);
    glClear(GL_STENCIL_BUFFER_BIT);
}

// Same test as ClearStencilZeroFirstByteMask, but using glClearBufferiv.
TEST_P(ClearTestES3, ClearBufferStencilZeroFirstByteMask)
{
    glStencilMask(0xe7d6a900);
    const GLint kStencilClearValue = 0x55;
    glClearBufferiv(GL_STENCIL, 0, &kStencilClearValue);
}

// Test that mid render pass clear after draw sets the render pass size correctly.
TEST_P(ClearTestES3, ScissoredDrawThenFullClear)
{
    const int w = getWindowWidth();
    const int h = getWindowHeight();

    // Use viewport to imply scissor on the draw call
    glViewport(w / 4, h / 4, w / 2, h / 2);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    // Mid-render-pass clear without scissor or viewport change, which covers the whole framebuffer.
    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::yellow);
}

// Test that mid render pass clear after masked clear sets the render pass size correctly.
TEST_P(ClearTestES3, MaskedScissoredClearThenFullClear)
{
    const int w = getWindowWidth();
    const int h = getWindowHeight();

    // Use viewport to imply a small scissor on (non-existing) draw calls.  This is important to
    // make sure render area that's derived from scissor+viewport for draw calls doesn't
    // accidentally fix render area derived from scissor for clear calls.
    glViewport(w / 2, h / 2, 1, 1);

    glEnable(GL_SCISSOR_TEST);
    glScissor(w / 4, h / 4, w / 2, h / 2);
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
    glClearColor(0.13, 0.38, 0.87, 0.65);
    glClear(GL_COLOR_BUFFER_BIT);

    // Mid-render-pass clear without scissor, which covers the whole framebuffer.
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::yellow);
}

// Test that mid render pass masked clear after masked clear sets the render pass size correctly.
TEST_P(ClearTestES3, MaskedScissoredClearThenFullMaskedClear)
{
    const int w = getWindowWidth();
    const int h = getWindowHeight();

    // Make sure the framebuffer is initialized.
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Use viewport to imply a small scissor on (non-existing) draw calls  This is important to
    // make sure render area that's derived from scissor+viewport for draw calls doesn't
    // accidentally fix render area derived from scissor for clear calls.
    glViewport(w / 2, h / 2, 1, 1);

    glEnable(GL_SCISSOR_TEST);
    glScissor(w / 4, h / 4, w / 2, h / 2);
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Mid-render-pass clear without scissor, which covers the whole framebuffer.
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_RECT_EQ(0, 0, w / 4, h, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(w / 4, 0, w / 2, h / 4, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(w / 4, 3 * h / 4, w / 2, h / 4, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(3 * w / 4, 0, w / 4, h, GLColor::green);

    EXPECT_PIXEL_RECT_EQ(w / 4, h / 4, w / 2, h / 2, GLColor::yellow);
}

// Test that reclearing color to the same value works.
TEST_P(ClearTestES3, RepeatedColorClear)
{
    glClearColor(1, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing depth to the same value works.
TEST_P(ClearTestES3, RepeatedDepthClear)
{
    glClearDepthf(0.25f);
    glClear(GL_DEPTH_BUFFER_BIT);

    verifyDepth(0.25f, 1);

    glClear(GL_DEPTH_BUFFER_BIT);

    verifyDepth(0.25f, 1);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing stencil to the same value works.
TEST_P(ClearTestES3, RepeatedStencilClear)
{
    glClearStencil(0xE4);
    glClear(GL_STENCIL_BUFFER_BIT);
    verifyStencil(0xE4, 1);

    glClear(GL_STENCIL_BUFFER_BIT);
    verifyStencil(0xE4, 1);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing color to the same value works if color was written to in between with a draw
// call.
TEST_P(ClearTestES3, RepeatedColorClearWithDrawInBetween)
{
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing depth to the same value works if depth was written to in between with a draw
// call.
TEST_P(ClearTestES3, RepeatedDepthClearWithDrawInBetween)
{
    glClearDepthf(0.25f);
    glClear(GL_DEPTH_BUFFER_BIT);

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.75f);

    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);
    verifyDepth(0.25f, 1);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing stencil to the same value works if stencil was written to in between with a
// draw call.
TEST_P(ClearTestES3, RepeatedStencilClearWithDrawInBetween)
{
    glClearStencil(0xE4);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 0x3C, 0xFF);
    glEnable(GL_STENCIL_TEST);

    glClear(GL_STENCIL_BUFFER_BIT);
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.75f);

    glClear(GL_STENCIL_BUFFER_BIT);
    verifyStencil(0xE4, 1);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing color to the same value works if color was written to in between with
// glCopyTexSubImage2D.
TEST_P(ClearTestES3, RepeatedColorClearWithCopyInBetween)
{
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the texture
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Replace the framebuffer texture
    GLTexture color2;
    glBindTexture(GL_TEXTURE_2D, color2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the new texture and copy it to the old texture
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, color);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, getWindowWidth(), getWindowHeight());

    // Attach the original texture back to the framebuffer and verify the copy.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Clear to the original value and make sure it's applied.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing color to the same value works if color was written to in between with a
// blit.
TEST_P(ClearTestES3, RepeatedColorClearWithBlitInBetween)
{
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the texture
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Create another framebuffer as blit src
    GLTexture color2;
    glBindTexture(GL_TEXTURE_2D, color2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the new framebuffer and blit it to the old one
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Verify the copy is done correctly.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Clear to the original value and make sure it's applied.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing depth to the same value works if depth was written to in between with a
// blit.
TEST_P(ClearTestES3, RepeatedDepthClearWithBlitInBetween)
{
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, getWindowWidth(),
                          getWindowHeight());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear depth
    glClearDepthf(0.25f);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    verifyDepth(0.25f, 1);

    // Create another framebuffer as blit src
    GLRenderbuffer depth2;
    glBindRenderbuffer(GL_RENDERBUFFER, depth2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, getWindowWidth(),
                          getWindowHeight());

    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the new framebuffer and blit it to the old one
    glClearDepthf(0.75f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // Verify the copy is done correctly.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    verifyDepth(0.75f, 1);

    // Clear to the original value and make sure it's applied.
    glClearDepthf(0.25f);
    glClear(GL_DEPTH_BUFFER_BIT);
    verifyDepth(0.25f, 1);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing stencil to the same value works if stencil was written to in between with a
// blit.
TEST_P(ClearTestES3, RepeatedStencilClearWithBlitInBetween)
{
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    GLRenderbuffer stencil;
    glBindRenderbuffer(GL_RENDERBUFFER, stencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, getWindowWidth(), getWindowHeight());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear stencil
    glClearStencil(0xE4);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    verifyStencil(0xE4, 1);

    // Create another framebuffer as blit src
    GLRenderbuffer stencil2;
    glBindRenderbuffer(GL_RENDERBUFFER, stencil2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, getWindowWidth(), getWindowHeight());

    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the new framebuffer and blit it to the old one
    glClearStencil(0x35);
    glClear(GL_STENCIL_BUFFER_BIT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    // Verify the copy is done correctly.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    verifyStencil(0x35, 1);

    // Clear to the original value and make sure it's applied.
    glClearStencil(0xE4);
    glClear(GL_STENCIL_BUFFER_BIT);
    verifyStencil(0xE4, 1);

    ASSERT_GL_NO_ERROR();
}

// Test that reclearing color to the same value works if color was written to in between with a
// compute shader.
TEST_P(ClearTestES31, RepeatedColorClearWithDispatchInBetween)
{
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the texture
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Write to the texture with a compute shader.
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba8) uniform highp writeonly image2D imageOut;
void main()
{
    imageStore(imageOut, ivec2(gl_GlobalInvocationID.xy), vec4(0, 1, 0, 1));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);
    glBindImageTexture(0, color, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    glDispatchCompute(getWindowWidth(), getWindowHeight(), 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    // Verify the compute shader overwrites the image correctly.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Clear to the original value and make sure it's applied.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test that clearing a 3D image bound to a layered framebuffer works using only attachment 0.
TEST_P(ClearTestES31, Bind3DTextureAndClearUsingAttachment0)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr uint32_t kSize            = 16;
    constexpr uint32_t kAttachmentCount = 4;
    std::vector<GLColor> pixelData(kSize * kSize * kAttachmentCount, GLColor::white);

    GLTexture texture3D;
    glBindTexture(GL_TEXTURE_3D, texture3D);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, kSize, kSize, kAttachmentCount, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixelData.data());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_LAYERS, kAttachmentCount);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture3D, 0);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture3D, 0, i);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        ASSERT_GL_NO_ERROR();
    }
}

// Test that clearing a 3D image bound to a layered framebuffer works using multiple attachments.
TEST_P(ClearTestES31, Bind3DTextureAndClearUsingMultipleAttachments)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr uint32_t kSize            = 16;
    constexpr uint32_t kAttachmentCount = 4;
    std::vector<GLColor> pixelData(kSize * kSize * kAttachmentCount, GLColor::white);

    GLTexture texture3D;
    glBindTexture(GL_TEXTURE_3D, texture3D);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, kSize, kSize, kAttachmentCount, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixelData.data());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_LAYERS, kAttachmentCount);
    glFramebufferTextureEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture3D, 0);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    const GLenum usedAttachment[kAttachmentCount] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                     GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT1};
    for (uint32_t i = 0; i < kAttachmentCount; ++i)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, usedAttachment[i], texture3D, 0, i);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        ASSERT_GL_NO_ERROR();
    }
}

// Test that reclearing depth to the same value works if depth is blit after clear, and depth is
// modified in between with a draw call.
TEST_P(ClearTestES3, RepeatedDepthClearWithBlitAfterClearAndDrawInBetween)
{
    glClearDepthf(0.25f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Make sure clear is flushed.
    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, getWindowWidth(),
                          getWindowHeight());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // Draw to depth, and break the render pass.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.75f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Clear back to the original value
    glClear(GL_DEPTH_BUFFER_BIT);

    // Blit again.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // Make sure the cleared value is in destination, not the modified value.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
    verifyDepth(0.25f, 1);

    ASSERT_GL_NO_ERROR();
}

// Test that gaps in framebuffer attachments do not cause race
// conditions when a clear op is followed by a draw call.
TEST_P(ClearTestES3, DrawAfterClearWithGaps)
{
    constexpr char kVS[] = R"(#version 300 es
precision highp float;
void main() {
    vec2 offset = vec2((gl_VertexID & 1) == 0 ? -1.0 : 1.0, (gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    gl_Position = vec4(offset * 0.125 - 0.5, 0.0, 1.0);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
layout(location=0) out vec4 color0;
layout(location=2) out vec4 color2;
void main() {
  color0 = vec4(1, 0, 1, 1);
  color2 = vec4(1, 1, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    constexpr int kSize = 1024;

    GLRenderbuffer rb0;
    glBindRenderbuffer(GL_RENDERBUFFER, rb0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    GLRenderbuffer rb2;
    glBindRenderbuffer(GL_RENDERBUFFER, rb2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, rb2);

    GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, bufs);
    glReadBuffer(GL_COLOR_ATTACHMENT2);

    glClearColor(0, 1, 0, 1);
    glViewport(0, 0, kSize, kSize);

    // Draw immediately after clear
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    std::vector<GLColor> pixels(kSize * kSize, GLColor::transparentBlack);
    glReadPixels(0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    ASSERT_GL_NO_ERROR();

    for (int y = 0; y < kSize; ++y)
    {
        for (int x = 0; x < kSize; ++x)
        {
            const GLColor color = pixels[y * kSize + x];
            if (x > 192 && x < 319 && y > 192 && y < 319)
            {
                EXPECT_EQ(color, GLColor::yellow) << "at " << x << ", " << y;
            }
            else if (x < 191 || x > 320 || y < 191 || y > 320)
            {
                EXPECT_EQ(color, GLColor::green) << "at " << x << ", " << y;
            }
        }
    }
}

// Test that mid render pass clears work with gaps in locations.
TEST_P(ClearTestES3, MidRenderPassClearWithGaps)
{
    constexpr char kVS[] = R"(#version 300 es
precision highp float;
void main() {
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, gl_VertexID % 2 == 0 ? -1 : 1, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
layout(location=0) out vec4 color0;
layout(location=2) out vec4 color2;
uniform vec4 color0in;
uniform vec4 color1in;
void main() {
  color0 = color0in;
  color2 = color1in;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    const GLint color0InLoc = glGetUniformLocation(program, "color0in");
    const GLint color1InLoc = glGetUniformLocation(program, "color1in");
    ASSERT_NE(color0InLoc, -1);
    ASSERT_NE(color1InLoc, -1);

    constexpr int kSize = 23;

    GLRenderbuffer rb0;
    glBindRenderbuffer(GL_RENDERBUFFER, rb0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    GLRenderbuffer rb2;
    glBindRenderbuffer(GL_RENDERBUFFER, rb2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, rb2);

    GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, bufs);

    glViewport(0, 0, kSize, kSize);

    // Start with a draw call
    glUniform4f(color0InLoc, 0.1, 0.2, 0.3, 0.4);
    glUniform4f(color1InLoc, 0.05, 0.15, 0.25, 0.35);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Clear in the middle of the render pass
    glClearColor(1, 0, 0, 0.6);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw with blend, and verify results
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glUniform4f(color0InLoc, 0, 1, 0, 0.5);
    glUniform4f(color1InLoc, 0, 0, 1, 0.5);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::yellow);

    glReadBuffer(GL_COLOR_ATTACHMENT2);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::magenta);
}

// Test that reclearing stencil to the same value works if stencil is blit after clear, and stencil
// is modified in between with a draw call.
TEST_P(ClearTestES3, RepeatedStencilClearWithBlitAfterClearAndDrawInBetween)
{
    glClearStencil(0xE4);
    glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Make sure clear is flushed.
    GLRenderbuffer stencil;
    glBindRenderbuffer(GL_RENDERBUFFER, stencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, getWindowWidth(),
                          getWindowHeight());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    // Draw to stencil, and break the render pass.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 0x3C, 0xFF);
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.75f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Clear back to the original value
    glClear(GL_STENCIL_BUFFER_BIT);

    // Blit again.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    // Make sure the cleared value is in destination, not the modified value.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
    verifyStencil(0xE4, 1);

    ASSERT_GL_NO_ERROR();
}

// Test basic functionality of clearing a 2D texture with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Clear2D)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the entire texture
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    // Clear each corner to a different color
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexSubImageEXT(tex, 0, 8, 0, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);
    glClearTexSubImageEXT(tex, 0, 0, 8, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::cyan);
    glClearTexSubImageEXT(tex, 0, 8, 8, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::yellow);

    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 8, GLColor::transparentBlack);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 8, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(0, 8, 8, 8, GLColor::cyan);
    EXPECT_PIXEL_RECT_EQ(8, 8, 8, 8, GLColor::yellow);
}

// Test basic functionality of clearing a 2D RGB texture with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Clear2DRGB)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the entire texture
    glClearTexImageEXT(tex, 0, GL_RGB, GL_UNSIGNED_BYTE, &GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    // Clear each corner to a different color
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 8, 8, 1, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glClearTexSubImageEXT(tex, 0, 8, 0, 0, 8, 8, 1, GL_RGB, GL_UNSIGNED_BYTE, &GLColor::blue);
    glClearTexSubImageEXT(tex, 0, 0, 8, 0, 8, 8, 1, GL_RGB, GL_UNSIGNED_BYTE, &GLColor::cyan);
    glClearTexSubImageEXT(tex, 0, 8, 8, 0, 8, 8, 1, GL_RGB, GL_UNSIGNED_BYTE, &GLColor::yellow);

    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 8, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 8, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(0, 8, 8, 8, GLColor::cyan);
    EXPECT_PIXEL_RECT_EQ(8, 8, 8, 8, GLColor::yellow);
}

// Test basic functionality of clearing a part of a 2D texture with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Clear2DSubImage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the center to a different color
    glClearTexSubImageEXT(tex, 0, 4, 4, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::cyan);
    EXPECT_PIXEL_RECT_EQ(4, 4, 8, 8, GLColor::cyan);
}

// Test clearing a 2D RGBA4 texture with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Clear2DRGBA4)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, 16, 16, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,
                 nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the entire texture, then clear each corner to a different color.
    GLushort colorRed    = 0xF00F;
    GLushort colorGreen  = 0x0F0F;
    GLushort colorBlue   = 0x00FF;
    GLushort colorYellow = 0xFF0F;

    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, &colorGreen);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::green);

    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, nullptr);
    glClearTexSubImageEXT(tex, 0, 8, 0, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, &colorBlue);
    glClearTexSubImageEXT(tex, 0, 0, 8, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, &colorRed);
    glClearTexSubImageEXT(tex, 0, 8, 8, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,
                          &colorYellow);

    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 8, GLColor::transparentBlack);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 8, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(0, 8, 8, 8, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(8, 8, 8, 8, GLColor::yellow);
}

// Test clearing a 2D RGB8 Snorm texture with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Clear2DRGB8Snorm)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8_SNORM, 16, 16, 0, GL_RGB, GL_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Clear the entire texture.
    GLint colorGreenRGBSnorm = 0x007F00;
    glClearTexImageEXT(tex, 0, GL_RGB, GL_BYTE, &colorGreenRGBSnorm);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::green);
}

// Test clearing a corner of a 2D RGB8 Snorm texture with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Clear2DRGB8SnormCorner)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 4x4 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8_SNORM, 4, 4, 0, GL_RGB, GL_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Clear one corner of the texture.
    GLint colorGreenRGBSnorm = 0x007F00;
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 2, 2, 1, GL_RGB, GL_BYTE, &colorGreenRGBSnorm);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, GLColor::green);
}

// Test basic functionality of clearing 2D textures with GL_EXT_clear_texture using nullptr.
TEST_P(ClearTextureEXTTest, Clear2DWithNull)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create two 16x16 textures with prior data.
    std::vector<GLColor> redBlock(16 * 16, GLColor::red);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, redBlock.data());

    GLTexture tex2;
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());

    // Clear the RGB texture.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLColor outputColor;
    glClearTexImageEXT(tex, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glReadPixels(0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &outputColor);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::black);

    // Clear the RGBA texture.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearTexImageEXT(tex2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &outputColor);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::transparentBlack);
}

// Test basic functionality of clearing a 2D texture while bound to another.
TEST_P(ClearTextureEXTTest, Clear2DDifferentBinding)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create two 16x16 textures with no data.
    GLTexture tex1;
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLTexture tex2;
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Use clear on both textures while none are bound.
    glBindTexture(GL_TEXTURE_2D, 0);
    glClearTexImageEXT(tex1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glClearTexImageEXT(tex2, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);

    // Bind to one texture while clearing the other.
    glBindTexture(GL_TEXTURE_2D, tex2);
    glClearTexImageEXT(tex1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::green);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);
}

// Test basic functionality of clearing 2D textures without binding to them.
TEST_P(ClearTextureEXTTest, Clear2DNoBinding)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create two 16x16 textures with no data.
    GLTexture tex1;
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLTexture tex2;
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Use clear on both textures while bound to neither.
    glBindTexture(GL_TEXTURE_2D, 0);
    glClearTexImageEXT(tex1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glClearTexImageEXT(tex2, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);
}

// Test basic functionality of clearing a 2D texture with prior staged update.
TEST_P(ClearTextureEXTTest, Clear2DOverwritePriorContent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with some data.
    std::vector<GLColor> redBlock(16 * 16, GLColor::red);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the entire texture.
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);
}

// Test clearing a 2D texture using a depth of zero.
TEST_P(ClearTextureEXTTest, Clear2DWithZeroDepth)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture and fully clear it.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);

    // Clear the 2D texture using depth of zero.
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::green);
}

// Test basic functionality of clearing a 2D texture defined using glTexStorage().
TEST_P(ClearTextureEXTTest, Clear2DTexStorage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 mipmap texture.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 5, GL_RGBA8, 16, 16);

    // Define texture mip levels by clearing them.
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glClearTexImageEXT(tex, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    glClearTexImageEXT(tex, 2, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);
    glClearTexImageEXT(tex, 3, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::magenta);
    ASSERT_GL_NO_ERROR();

    // Bind to framebuffer and verify.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 8, GLColor::green);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, 4, 4, GLColor::blue);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 3);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, 2, 2, GLColor::magenta);
}

// Test that a single full clear for the 3D texture works.
TEST_P(ClearTextureEXTTest, Clear3DSingleFull)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    constexpr uint32_t kWidth  = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kDepth  = 4;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, kWidth, kHeight, kDepth);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

    glClearTexImageEXT(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::white);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::white);
    }
}

// Test that a simple clear for the entire texture works.
TEST_P(ClearTextureEXTTest, Clear3DWhole)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    constexpr uint32_t kWidth  = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kDepth  = 4;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, kWidth, kHeight, kDepth);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

    glClearTexImageEXT(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::white);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::white);
    }

    glClearTexSubImageEXT(texture, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::green);
    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);
    }
}

// Test that clearing slices of a 3D texture and reading them back works.
TEST_P(ClearTextureEXTTest, Clear3DLayers)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    constexpr uint32_t kWidth  = 128;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth  = 7;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, kWidth, kHeight, kDepth);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

    std::array<GLColor, kDepth> clearColors = {
        GLColor::red,  GLColor::green,   GLColor::blue,  GLColor::yellow,
        GLColor::cyan, GLColor::magenta, GLColor::white,
    };

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClearTexImageEXT(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glClearTexSubImageEXT(texture, 0, 0, 0, z, kWidth, kHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                              &clearColors[z]);
    }

    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, clearColors[z]);
    }
}

// Test that clearing slices of a 3D texture with dimensions of zero does not change it.
TEST_P(ClearTextureEXTTest, Clear3DZeroDims)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    constexpr uint32_t kWidth  = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kDepth  = 4;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, kWidth, kHeight, kDepth);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    glClearTexImageEXT(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::white);

    // Dimensions of zero for clear are valid. However, they should not change the texture.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClearTexSubImageEXT(texture, 0, 0, 0, 0, 0, kHeight, kDepth, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::red);
    ASSERT_GL_NO_ERROR();
    glClearTexSubImageEXT(texture, 0, 0, 0, 0, kWidth, 0, kDepth, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::green);
    ASSERT_GL_NO_ERROR();
    glClearTexSubImageEXT(texture, 0, 0, 0, 0, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::blue);
    ASSERT_GL_NO_ERROR();
    glClearTexSubImageEXT(texture, 0, 0, 0, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::white);
    }
}

// Test that clearing blocks of a 3D texture and reading them back works.
TEST_P(ClearTextureEXTTest, Clear3DBlocks)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    constexpr uint32_t kWidth  = 16;
    constexpr uint32_t kHeight = 16;
    constexpr uint32_t kDepth  = 16;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, kWidth, kHeight, kDepth);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

    std::array<GLColor, 8> clearColors = {GLColor::red,    GLColor::green, GLColor::blue,
                                          GLColor::yellow, GLColor::cyan,  GLColor::magenta,
                                          GLColor::white,  GLColor::red};

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for (uint32_t k = 0; k < 2; ++k)
    {
        for (uint32_t j = 0; j < 2; ++j)
        {
            for (uint32_t i = 0; i < 2; ++i)
            {
                glClearTexSubImageEXT(texture, 0, i * kWidth / 2, j * kHeight / 2, k * kDepth / 2,
                                      kWidth / 2, kHeight / 2, kDepth / 2, GL_RGBA,
                                      GL_UNSIGNED_BYTE, &clearColors[(k << 2) + (j << 1) + i]);
            }
        }
    }

    for (uint32_t z = 0; z < kDepth / 2; ++z)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth / 2, kHeight / 2, clearColors[0]);
        EXPECT_PIXEL_RECT_EQ(kWidth / 2, 0, kWidth / 2, kHeight / 2, clearColors[1]);
        EXPECT_PIXEL_RECT_EQ(0, kHeight / 2, kWidth / 2, kHeight / 2, clearColors[2]);
        EXPECT_PIXEL_RECT_EQ(kWidth / 2, kHeight / 2, kWidth / 2, kHeight / 2, clearColors[3]);
    }
    for (uint32_t z = kDepth / 2; z < kDepth; ++z)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth / 2, kHeight / 2, clearColors[4]);
        EXPECT_PIXEL_RECT_EQ(kWidth / 2, 0, kWidth / 2, kHeight / 2, clearColors[5]);
        EXPECT_PIXEL_RECT_EQ(0, kHeight / 2, kWidth / 2, kHeight / 2, clearColors[6]);
        EXPECT_PIXEL_RECT_EQ(kWidth / 2, kHeight / 2, kWidth / 2, kHeight / 2, clearColors[7]);
    }
}

// Test that clearing slices of a 3D texture and reading them back works.
TEST_P(ClearTextureEXTTest, Clear2DArray)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    constexpr uint32_t kWidth  = 64;
    constexpr uint32_t kHeight = 64;
    constexpr uint32_t kDepth  = 4;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, kWidth, kHeight, kDepth);

    std::array<GLColor, kDepth> clearColors = {GLColor::red, GLColor::green, GLColor::blue,
                                               GLColor::yellow};

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClearTexImageEXT(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glClearTexSubImageEXT(texture, 0, 0, 0, z, kWidth, kHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                              &clearColors[z]);
    }

    for (uint32_t z = 0; z < kDepth; ++z)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, z);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, clearColors[z]);
    }
}

// Test that luminance alpha textures are cleared correctly with GL_EXT_clear_texture. Regression
// test for emulated luma formats.
TEST_P(ClearTextureEXTTest, Luma)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    // Clear the entire texture to transparent black and test
    GLubyte luminanceClearValue = 192;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 16, 16, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 nullptr);
    glClearTexImageEXT(tex, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &luminanceClearValue);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(
        0, 0, getWindowWidth(), getWindowHeight(),
        GLColor(luminanceClearValue, luminanceClearValue, luminanceClearValue, 255));

    GLubyte lumaClearValue[2] = {128, 64};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, 16, 16, 0, GL_LUMINANCE_ALPHA,
                 GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(tex, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, &lumaClearValue);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(
        0, 0, getWindowWidth(), getWindowHeight(),
        GLColor(lumaClearValue[0], lumaClearValue[0], lumaClearValue[0], lumaClearValue[1]));
}

// Test that luminance alpha float textures are cleared correctly with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, LumaAlphaFloat)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    GLfloat lumaClearValue[2] = {0.5, 0.25};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, 16, 16, 0, GL_LUMINANCE_ALPHA, GL_FLOAT,
                 nullptr);
    glClearTexImageEXT(tex, 0, GL_LUMINANCE_ALPHA, GL_FLOAT, &lumaClearValue);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0,
                            GLColor(lumaClearValue[0] * 255, lumaClearValue[0] * 255,
                                    lumaClearValue[0] * 255, lumaClearValue[1] * 255),
                            1);
}

// Test that interleaving glClearTexImageEXT and glTexSubImage2D calls produces the correct texture
// data.
TEST_P(ClearTextureEXTTest, InterleavedUploads)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    // Clear the entire texture to transparent black and test
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::transparentBlack);

    // TexSubImage a corner
    std::vector<GLColor> redBlock(8 * 8, GLColor::red);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);

    // Clear and tex sub image together
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 8, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, getWindowHeight() / 2, getWindowWidth() / 2, getWindowHeight() / 2,
                         GLColor::blue);
}

// Test clearing integer textures with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, IntegerTexture)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    GLColor32I rgba32iTestValue(-128, 256, -512, 1024);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, 16, 16, 0, GL_RGBA_INTEGER, GL_INT, nullptr);
    glClearTexImageEXT(tex, 0, GL_RGBA_INTEGER, GL_INT, &rgba32iTestValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_32I_COLOR(0, 0, rgba32iTestValue);

    GLColor32UI rgba32uiTestValue(128, 256, 512, 1024);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, 16, 16, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT,
                 nullptr);
    glClearTexImageEXT(tex, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, &rgba32uiTestValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_32UI_COLOR(0, 0, rgba32uiTestValue);
}

// Test clearing float32 textures with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Float32Texture)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    GLColor32F rgba32fTestValue(0.1, 0.2, 0.3, 0.4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 16, 16, 0, GL_RGBA, GL_FLOAT, nullptr);
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_FLOAT, &rgba32fTestValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_32F_EQ(0, 0, rgba32fTestValue.R, rgba32fTestValue.G, rgba32fTestValue.B,
                        rgba32fTestValue.A);
}

// Test clearing depth textures with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, DepthTexture)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    GLTexture colorTex;
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture depthTex;
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 16, 16, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    const GLuint depthClearZero = 0;
    const GLuint depthClearOne  = std::numeric_limits<GLuint>::max();

    // Draw doesn't pass the depth test. Texture has 0.0.
    glClearTexImageEXT(depthTex, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &depthClearZero);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    // Draw passes the depth test. Texture has 1.0.
    glClearTexImageEXT(depthTex, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &depthClearOne);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);

    // Left side passes, right side fails
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glClearTexSubImageEXT(depthTex, 0, 0, 0, 0, 8, 16, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                          &depthClearZero);
    glClearTexSubImageEXT(depthTex, 0, 8, 0, 0, 8, 16, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                          &depthClearOne);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 16, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 16, GLColor::blue);
}

// Test clearing float depth textures (32-bit) with GL_EXT_clear_texture
TEST_P(ClearTextureEXTTest, Depth32FTexture)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    GLTexture colorTex;
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture depthTex;
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 16, 16, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    const GLfloat depthClearZero = 0.0;
    const GLfloat depthClearOne  = 1.0;

    // Draw doesn't pass the depth test. Texture has 0.0.
    glClearTexImageEXT(depthTex, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &depthClearZero);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    // Draw passes the depth test. Texture has 1.0.
    glClearTexImageEXT(depthTex, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &depthClearOne);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);

    // Left side passes, right side fails
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glClearTexSubImageEXT(depthTex, 0, 0, 0, 0, 8, 16, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                          &depthClearZero);
    glClearTexSubImageEXT(depthTex, 0, 8, 0, 0, 8, 16, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                          &depthClearOne);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 16, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 16, GLColor::blue);
}

// Test clearing 16-bit depth textures with GL_EXT_clear_texture
TEST_P(ClearTextureEXTTest, Depth16Texture)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    GLTexture colorTex;
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture depthTex;
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 16, 16, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_SHORT, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    const GLushort depthClearZero = 0;
    const GLushort depthClearOne  = std::numeric_limits<GLushort>::max();

    // Draw doesn't pass the depth test. Texture has 0.0.
    glClearTexImageEXT(depthTex, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, &depthClearZero);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    // Draw passes the depth test. Texture has 1.0.
    glClearTexImageEXT(depthTex, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, &depthClearOne);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);

    // Left side passes, right side fails
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glClearTexSubImageEXT(depthTex, 0, 0, 0, 0, 8, 16, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,
                          &depthClearZero);
    glClearTexSubImageEXT(depthTex, 0, 8, 0, 0, 8, 16, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,
                          &depthClearOne);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 16, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 16, GLColor::blue);
}

// Test clearing stencil textures with GL_EXT_clear_texture
TEST_P(ClearTextureEXTTest, StencilTexture)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_stencil8"));

    GLTexture colorTex;
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture stencilTex;
    glBindTexture(GL_TEXTURE_2D, stencilTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_STENCIL_INDEX8, 16, 16);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilTex, 0);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0xCC, 0xFF);

    const GLint stencilClearAA = 0xAA;
    const GLint stencilClearEE = 0xEE;

    // Draw doesn't pass the stencil test.
    glClearTexImageEXT(stencilTex, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &stencilClearAA);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    // Draw passes the stencil test.
    glClearTexImageEXT(stencilTex, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &stencilClearEE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);

    // Left side passes, right side fails
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glClearTexSubImageEXT(stencilTex, 0, 0, 0, 0, 8, 16, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
                          &stencilClearAA);
    glClearTexSubImageEXT(stencilTex, 0, 8, 0, 0, 8, 16, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
                          &stencilClearEE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 16, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 16, GLColor::blue);
}

// Test clearing depth24/stencil8 textures with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Depth24Stencil8Texture)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    GLTexture colorTex;
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture dsTex;
    glBindTexture(GL_TEXTURE_2D, dsTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, 16, 16);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, dsTex, 0);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0xCC, 0xFF);

    GLuint dsValue0 = 0x000000AA;
    GLuint dsValue1 = 0x000000EE;
    GLuint dsValue2 = 0xFFFFFFAA;
    GLuint dsValue3 = 0xFFFFFFEE;

    // Draw doesn't pass the test.
    glClearTexImageEXT(dsTex, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, &dsValue0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    // Draw passes the stencil test.
    glClearTexImageEXT(dsTex, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, &dsValue3);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);

    // Left side fails the depth test. Top side fails the stencil test.
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    glClearTexSubImageEXT(dsTex, 0, 0, 0, 0, 8, 8, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                          &dsValue0);
    glClearTexSubImageEXT(dsTex, 0, 0, 8, 0, 8, 8, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                          &dsValue1);
    glClearTexSubImageEXT(dsTex, 0, 8, 0, 0, 8, 8, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                          &dsValue2);
    glClearTexSubImageEXT(dsTex, 0, 8, 8, 0, 8, 8, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                          &dsValue3);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 8, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 16, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(8, 8, 8, 8, GLColor::blue);
}

// Test clearing depth32/stencil textures with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Depth32FStencilTexture)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    GLTexture colorTex;
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture dsTex;
    glBindTexture(GL_TEXTURE_2D, dsTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH32F_STENCIL8, 16, 16);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, dsTex, 0);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0xCC, 0xFF);

    struct DSClearValue
    {
        GLfloat depth;
        GLuint stencil;
    };

    DSClearValue dsValue0 = {0, 0xAA};
    DSClearValue dsValue1 = {0, 0xEE};
    DSClearValue dsValue2 = {1, 0xAA};
    DSClearValue dsValue3 = {1, 0xEE};

    // Draw doesn't pass the test.
    glClearTexImageEXT(dsTex, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, &dsValue0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    // Draw passes the stencil test.
    glClearTexImageEXT(dsTex, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, &dsValue3);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);

    // Left side fails the depth test. Top side fails the stencil test.
    glClearTexImageEXT(colorTex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    glClearTexSubImageEXT(dsTex, 0, 0, 0, 0, 8, 8, 1, GL_DEPTH_STENCIL,
                          GL_FLOAT_32_UNSIGNED_INT_24_8_REV, &dsValue0);
    glClearTexSubImageEXT(dsTex, 0, 0, 8, 0, 8, 8, 1, GL_DEPTH_STENCIL,
                          GL_FLOAT_32_UNSIGNED_INT_24_8_REV, &dsValue1);
    glClearTexSubImageEXT(dsTex, 0, 8, 0, 0, 8, 8, 1, GL_DEPTH_STENCIL,
                          GL_FLOAT_32_UNSIGNED_INT_24_8_REV, &dsValue2);
    glClearTexSubImageEXT(dsTex, 0, 8, 8, 0, 8, 8, 1, GL_DEPTH_STENCIL,
                          GL_FLOAT_32_UNSIGNED_INT_24_8_REV, &dsValue3);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 8, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 16, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(8, 8, 8, 8, GLColor::blue);
}

// Test clearing different sets of cube map texture faces with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, ClearCubeFaces)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    for (size_t i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Clear the entire texture
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    for (size_t i = 0; i < 6; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex, 0);
        EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);
    }

    // Clear different ranges of faces to different colors:

    // [0, 1] -> green
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 16, 16, 2, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);

    // [1, 2] -> blue (partially overlaps previous clear)
    glClearTexSubImageEXT(tex, 0, 0, 0, 1, 16, 16, 2, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);

    // [3, 5] -> cyan
    glClearTexSubImageEXT(tex, 0, 0, 0, 3, 16, 16, 3, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::cyan);

    // Test the colors
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0,
                           tex, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::green);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1,
                           tex, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2,
                           tex, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3,
                           tex, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::cyan);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4,
                           tex, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::cyan);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5,
                           tex, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::cyan);
}

// Test clearing different sets of cube map array texture layer-faces with GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, ClearCubeMapArray)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_cube_map_array"));

    // Create a 16x16 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, tex);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA, 16, 16, 24, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Clear the entire texture
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    for (size_t i = 0; i < 24; i++)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, i);
        EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);
    }

    // Clear different ranges of faces to different colors:

    // [0, 4] -> green
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 16, 16, 5, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);

    // [4, 6] -> blue (partially overlaps previous clear)
    glClearTexSubImageEXT(tex, 0, 0, 0, 4, 16, 16, 3, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);

    // [12, 17] -> cyan
    glClearTexSubImageEXT(tex, 0, 0, 0, 12, 16, 16, 6, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::cyan);

    // Test the colors
    for (size_t i = 0; i < 4; i++)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, i);
        EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::green);
    }
    for (size_t i = 4; i < 7; i++)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, i);
        EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);
    }
    for (size_t i = 12; i < 18; i++)
    {
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, i);
        EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::cyan);
    }
}

// Test clearing one level, then uploading an update to the same level and then drawing.
TEST_P(ClearTextureEXTTest, ClearOneLevelThenPartialUpdateAndDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with prior data.
    std::vector<GLColor> redBlock(16 * 16, GLColor::red);
    std::vector<GLColor> greenBlock(16 * 16, GLColor::green);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());

    // Clear one level and add another update on top of it.
    glClearTexImageEXT(tex, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, greenBlock.data());

    // Draw.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight(),
                         GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(0, getWindowHeight() / 2, getWindowWidth(), getWindowHeight() / 2,
                         GLColor::blue);
}

// Test drawing, then partially clearing the texture and drawing again.
TEST_P(ClearTextureEXTTest, DrawThenClearPartiallyThenDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with prior data.
    std::vector<GLColor> redBlock(16 * 16, GLColor::red);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());

    // Draw.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);

    // Clear a portion of the texture and draw again.
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::yellow);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight(),
                         GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, getWindowHeight() / 2, getWindowWidth(), getWindowHeight() / 2,
                         GLColor::red);
}

// Test partially clearing a mip level, applying an overlapping update and then drawing with it.
TEST_P(ClearTextureEXTTest, PartialClearThenOverlappingUploadThenDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with prior data.
    std::vector<GLColor> redBlock(16 * 16, GLColor::red);
    std::vector<GLColor> greenBlock(16 * 16, GLColor::green);
    std::vector<GLColor> blueBlock(16 * 16, GLColor::blue);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, blueBlock.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());

    // Clear one level and add another update on top of it.
    glClearTexSubImageEXT(tex, 1, 2, 2, 0, 4, 4, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::yellow);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, greenBlock.data());

    // Draw and verify.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(getWindowWidth() / 4, getWindowWidth() / 2, getWindowWidth() / 2,
                         getWindowHeight() / 4, GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ(getWindowWidth() / 2, getWindowWidth() / 4, getWindowWidth() / 4,
                         getWindowHeight() / 2, GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ((3 * getWindowWidth()) / 4, 0, getWindowWidth() / 4, getWindowHeight(),
                         GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, (3 * getWindowHeight()) / 4, getWindowWidth(), getWindowHeight() / 4,
                         GLColor::red);
    EXPECT_PIXEL_RECT_EQ(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight() / 4,
                         GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, getWindowHeight() / 2, getWindowWidth() / 4, getWindowHeight() / 2,
                         GLColor::red);
}

// Test clearing a mip level and generating mipmap based on its previous level and draw.
TEST_P(ClearTextureEXTTest, ClearLevelThenGenerateMipmapOnBaseThenDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with prior data.
    std::vector<GLColor> redBlock(16 * 16, GLColor::red);
    std::vector<GLColor> greenBlock(16 * 16, GLColor::green);
    std::vector<GLColor> blueBlock(16 * 16, GLColor::blue);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, greenBlock.data());
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, blueBlock.data());

    // Clear level 1 and then generate mipmap for the texture.
    glClearTexImageEXT(tex, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::yellow);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Draw.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
}

// Test clearing a mip level and generating mipmap based on that level and draw.
TEST_P(ClearTextureEXTTest, ClearLevelThenGenerateMipmapOnLevelThenDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Create a 16x16 texture with prior data.
    std::vector<GLColor> redBlock(16 * 16, GLColor::red);
    std::vector<GLColor> greenBlock(16 * 16, GLColor::green);
    std::vector<GLColor> blueBlock(16 * 16, GLColor::blue);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, redBlock.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, greenBlock.data());
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, blueBlock.data());

    // Clear level 1 and then generate mipmap for the texture based on level 1.
    glClearTexImageEXT(tex, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::yellow);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Draw.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::yellow);
}

// Test clearing a large 2D texture.
TEST_P(ClearTextureEXTTest, LargeTexture)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    constexpr GLsizei kTexWidth  = 2048;
    constexpr GLsizei kTexHeight = 2048;
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexWidth, kTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, 0, kTexWidth, kTexHeight, GLColor::red);
}

// Test that clearing works correctly after swizzling the texture components.
TEST_P(ClearTextureEXTTest, ClearDrawSwizzleClearDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Initialize texture and draw.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::cyan);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::cyan);

    // Change swizzling; left rotation of the RGB components.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

    // Clear again and draw.
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::yellow);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::magenta);
}

// Test validation of GL_EXT_clear_texture.
TEST_P(ClearTextureEXTTest, Validation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Texture 0 is invalid
    glClearTexImageEXT(0, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Texture is not the name of a texture object
    glClearTexImageEXT(1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLTexture tex2D;
    glBindTexture(GL_TEXTURE_2D, tex2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Out of bounds
    glClearTexSubImageEXT(tex2D, 0, 1, 0, 0, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClearTexSubImageEXT(tex2D, 0, 0, 1, 0, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClearTexSubImageEXT(tex2D, 1, 0, 0, 0, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClearTexSubImageEXT(tex2D, 0, 0, 0, 1, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClearTexSubImageEXT(tex2D, 0, 0, 0, 0, 16, 16, 2, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Negative offsets
    glClearTexSubImageEXT(tex2D, 0, 4, 4, 0, -2, -2, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLTexture texCube;
    glBindTexture(GL_TEXTURE_CUBE_MAP, texCube);

    // First and third cube faces are specified
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    // Unspecified cube face
    glClearTexSubImageEXT(texCube, 0, 0, 0, 1, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::green);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Cube range with an unspecified face
    glClearTexSubImageEXT(texCube, 0, 0, 0, 0, 16, 16, 3, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::green);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Undefined level
    GLTexture tex1Level;
    glBindTexture(GL_TEXTURE_2D, tex1Level);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(tex1Level, 1, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Compressed texture
    GLTexture tex2DCompressed;
    glBindTexture(GL_TEXTURE_2D, tex2DCompressed);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, 16, 16, 0, 128, nullptr);
    glClearTexImageEXT(tex2DCompressed, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Buffer texture
    if (IsGLExtensionEnabled("GL_EXT_texture_buffer"))
    {
        GLTexture texBuffer;
        glBindTexture(GL_TEXTURE_BUFFER_EXT, texBuffer);
        glClearTexImageEXT(texBuffer, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    // internal format is integer and format does not specify integer data
    GLTexture texInt;
    glBindTexture(GL_TEXTURE_2D, texInt);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(texInt, 0, GL_RGBA, GL_FLOAT, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // internal format is not integer and format does specify integer data
    GLTexture texFloat;
    glBindTexture(GL_TEXTURE_2D, texFloat);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 16, 16, 0, GL_RGBA, GL_FLOAT, nullptr);
    glClearTexImageEXT(texFloat, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // internal format is DEPTH_COMPONENT and format is not DEPTH_COMPONENT
    GLTexture texDepth;
    glBindTexture(GL_TEXTURE_2D, texDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 16, 16, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 nullptr);
    glClearTexImageEXT(texDepth, 0, GL_RGBA, GL_FLOAT, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // internal format is DEPTH_STENCIL and format is not DEPTH_STENCIL
    GLTexture texDepthStencil;
    glBindTexture(GL_TEXTURE_2D, texDepthStencil);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 16, 16, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);
    glClearTexImageEXT(texDepthStencil, 0, GL_RGBA, GL_UNSIGNED_INT_24_8, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // internal format is STENCIL_INDEX and format is not STENCIL_INDEX
    GLTexture texStencil;
    glBindTexture(GL_TEXTURE_2D, texStencil);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, 16, 16, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
                 nullptr);
    glClearTexImageEXT(texStencil, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // internal format is RGBA and the <format> is DEPTH_COMPONENT, STENCIL_INDEX, or DEPTH_STENCIL
    GLTexture texRGBA;
    glBindTexture(GL_TEXTURE_2D, texRGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 16, 16, 0, GL_RGBA, GL_FLOAT, nullptr);
    glClearTexImageEXT(texRGBA, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glClearTexImageEXT(texRGBA, 0, GL_STENCIL_INDEX, GL_FLOAT, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glClearTexImageEXT(texRGBA, 0, GL_DEPTH_STENCIL, GL_FLOAT, &GLColor::red);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Covers a driver bug that leaks color mask state into clear texture ops.
TEST_P(ClearTextureEXTTest, ClearTextureAfterMaskedClearBug)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    // Perform a masked clear
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

    // Create a new texture with data, clear it, and sample
    {
        constexpr uint32_t kSize = 16;
        std::vector<unsigned char> pixelData(kSize * kSize * 4, 255);

        GLTexture tex;
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixelData.data());
        glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    }
}

// Test clearing renderable format textures with GL_EXT_clear_texture for TEXTURE_2D.
TEST_P(ClearTextureEXTTestES31Renderable, Clear2D)
{
    mDepth   = 1;
    mTarget  = GL_TEXTURE_2D;
    mIsArray = mHasLayer = false;
    testRenderable();
}

// Test clearing unrenderable format textures with GL_EXT_clear_texture for TEXTURE_2D.
TEST_P(ClearTextureEXTTestES31Unrenderable, Clear2D)
{
    mDepth   = 1;
    mTarget  = GL_TEXTURE_2D;
    mIsArray = mHasLayer = false;
    testUnrenderable(mFormats);
}

// Test clearing renderable format textures with GL_EXT_clear_texture for TEXTURE_2D_ARRAY.
TEST_P(ClearTextureEXTTestES31Renderable, Clear2DArray)
{
    mTarget = GL_TEXTURE_2D_ARRAY;
    testRenderable();
}

// Test clearing unrenderable format textures with GL_EXT_clear_texture for TEXTURE_2D_ARRAY.
TEST_P(ClearTextureEXTTestES31Unrenderable, Clear2DArray)
{
    mTarget = GL_TEXTURE_2D_ARRAY;
    testUnrenderable(mFormats);
}

// Test clearing renderable format textures with GL_EXT_clear_texture for TEXTURE_3D.
TEST_P(ClearTextureEXTTestES31Renderable, Clear3D)
{
    mTarget  = GL_TEXTURE_3D;
    mIsArray = false;
    testRenderable();
}

// Test clearing unrenderable format textures with GL_EXT_clear_texture for TEXTURE_3D.
TEST_P(ClearTextureEXTTestES31Unrenderable, Clear3D)
{
    mTarget  = GL_TEXTURE_3D;
    mIsArray = false;
    testUnrenderable(mFormats);
}

// Test clearing renderable format textures with GL_EXT_clear_texture for TEXTURE_CUBE_MAP.
TEST_P(ClearTextureEXTTestES31Renderable, ClearCubeMap)
{
    mHeight = mWidth;
    mDepth  = 6;
    mLevels = std::log2(mWidth) + 1;
    mTarget = GL_TEXTURE_CUBE_MAP;
    testRenderable();
}

// Test clearing unrenderable format textures with GL_EXT_clear_texture for TEXTURE_CUBE_MAP.
TEST_P(ClearTextureEXTTestES31Unrenderable, ClearCubeMap)
{
    mHeight = mWidth;
    mDepth  = 6;
    mLevels = std::log2(mWidth) + 1;
    mTarget = GL_TEXTURE_CUBE_MAP;
    testUnrenderable(mFormats);
}

// Test clearing renderable format textures with GL_EXT_clear_texture for
// TEXTURE_CUBE_MAP_ARRAY.
TEST_P(ClearTextureEXTTestES31Renderable, ClearCubeMapArray)
{
    mHeight       = mWidth;
    mDepth        = 2 * 6;
    mLevels       = std::log2(mWidth) + 1;
    mTarget       = GL_TEXTURE_CUBE_MAP_ARRAY;
    mExtraSupport = IsGLExtensionEnabled("GL_EXT_texture_cube_map_array");
    testRenderable();
}

// Test clearing unrenderable format textures with GL_EXT_clear_texture for
// TEXTURE_CUBE_MAP_ARRAY.
TEST_P(ClearTextureEXTTestES31Unrenderable, ClearCubeMapArray)
{
    mHeight       = mWidth;
    mDepth        = 2 * 6;
    mLevels       = std::log2(mWidth) + 1;
    mTarget       = GL_TEXTURE_CUBE_MAP_ARRAY;
    mExtraSupport = IsGLExtensionEnabled("GL_EXT_texture_cube_map_array");
    testUnrenderable(mFormats);
}

// Test clearing GL_RGB9_E5 format.
TEST_P(ClearTextureEXTTestES31Unrenderable, ClearRGB9E5)
{
    // Test for TEXTURE_2D_ARRAY.
    mTarget = GL_TEXTURE_2D_ARRAY;
    testUnrenderable(mFormatsRGB9E5);

    // Test for TEXTURE_3D.
    mTarget  = GL_TEXTURE_3D;
    mIsArray = false;
    testUnrenderable(mFormatsRGB9E5);

    // Test for TEXTURE_2D.
    mDepth   = 1;
    mLevels  = std::log2(std::max(mWidth, mHeight)) + 1;
    mTarget  = GL_TEXTURE_2D;
    mIsArray = mHasLayer = false;
    testUnrenderable(mFormatsRGB9E5);

    // Test for TEXTURE_CUBE_MAP.
    mHeight  = mWidth;
    mDepth   = 6;
    mLevels  = std::log2(mWidth) + 1;
    mTarget  = GL_TEXTURE_CUBE_MAP;
    mIsArray = mHasLayer = true;
    testUnrenderable(mFormatsRGB9E5);

    // Test for TEXTURE_CUBE_MAP_ARRAY.
    mDepth        = 2 * 6;
    mTarget       = GL_TEXTURE_CUBE_MAP_ARRAY;
    mExtraSupport = IsGLExtensionEnabled("GL_EXT_texture_cube_map_array");
    testUnrenderable(mFormatsRGB9E5);
}

// Test clearing unrenderable format textures with GL_EXT_clear_texture for TEXTURE_2D_MULTISAMPLE.
TEST_P(ClearTextureEXTTestES31Renderable, ClearMultisample)
{
    mDepth   = 1;
    mTarget  = GL_TEXTURE_2D_MULTISAMPLE;
    mIsArray = mHasLayer = false;
    testMS();
}

// Test clearing unrenderable format textures with GL_EXT_clear_texture for
// TEXTURE_2D_MULTISAMPLE_ARRAY.
TEST_P(ClearTextureEXTTestES31Renderable, ClearMultisampleArray)
{
    mTarget       = GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES;
    mExtraSupport = EnsureGLExtensionEnabled("GL_OES_texture_storage_multisample_2d_array");
    testMS();
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(
    ClearTest,
    ES3_VULKAN().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN().enable(Feature::PreferDrawClearOverVkCmdClearAttachments),
    ES2_WEBGPU());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ClearTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(
    ClearTestES3,
    ES3_VULKAN().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN().enable(Feature::PreferDrawClearOverVkCmdClearAttachments));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ClearTestES31);
ANGLE_INSTANTIATE_TEST_ES31_AND(
    ClearTestES31,
    ES31_VULKAN().enable(Feature::ForceFallbackFormat),
    ES31_VULKAN().enable(Feature::PreferDrawClearOverVkCmdClearAttachments));

ANGLE_INSTANTIATE_TEST_COMBINE_4(MaskedScissoredClearTest,
                                 MaskedScissoredClearVariationsTestPrint,
                                 testing::Range(0, 3),
                                 testing::Range(0, 3),
                                 testing::Range(0, 3),
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3,
                                 ES3_VULKAN()
                                     .disable(Feature::SupportsExtendedDynamicState)
                                     .disable(Feature::SupportsExtendedDynamicState2),
                                 ES3_VULKAN().disable(Feature::SupportsExtendedDynamicState2));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VulkanClearTest);
ANGLE_INSTANTIATE_TEST_COMBINE_4(VulkanClearTest,
                                 MaskedScissoredClearVariationsTestPrint,
                                 testing::Range(0, 3),
                                 testing::Range(0, 3),
                                 testing::Range(0, 3),
                                 testing::Bool(),
                                 ES2_VULKAN().enable(Feature::ForceFallbackFormat),
                                 ES2_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
                                 ES3_VULKAN().enable(Feature::ForceFallbackFormat),
                                 ES3_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat));

// Not all ANGLE backends support RGB backbuffers
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ClearTestRGB);
ANGLE_INSTANTIATE_TEST(ClearTestRGB,
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_VULKAN(),
                       ES3_VULKAN(),
                       ES2_METAL(),
                       ES3_METAL());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ClearTestRGB_ES3);
ANGLE_INSTANTIATE_TEST(ClearTestRGB_ES3, ES3_D3D11(), ES3_VULKAN(), ES3_METAL());

ANGLE_INSTANTIATE_TEST_ES3(ClearTextureEXTTest);
ANGLE_INSTANTIATE_TEST_ES31(ClearTextureEXTTestES31Renderable);
ANGLE_INSTANTIATE_TEST_ES31(ClearTextureEXTTestES31Unrenderable);

}  // anonymous namespace
