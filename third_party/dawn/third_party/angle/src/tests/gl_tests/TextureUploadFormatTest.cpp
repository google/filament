//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Texture upload format tests:
//   Test all texture unpack/upload formats for sampling correctness.
//

#include "common/mathutil.h"
#include "image_util/copyimage.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class TextureUploadFormatTest : public ANGLETest<>
{};

struct TexFormat final
{
    GLenum internalFormat;
    GLenum unpackFormat;
    GLenum unpackType;

    TexFormat() = delete;
    TexFormat(GLenum internalFormat, GLenum unpackFormat, GLenum unpackType)
        : internalFormat(internalFormat), unpackFormat(unpackFormat), unpackType(unpackType)
    {}

    uint8_t bytesPerPixel() const
    {
        uint8_t bytesPerChannel;
        switch (unpackType)
        {
            case GL_UNSIGNED_SHORT_5_6_5:
            case GL_UNSIGNED_SHORT_4_4_4_4:
            case GL_UNSIGNED_SHORT_5_5_5_1:
                return 2;

            case GL_UNSIGNED_INT_2_10_10_10_REV:
            case GL_UNSIGNED_INT_24_8:
            case GL_UNSIGNED_INT_10F_11F_11F_REV:
            case GL_UNSIGNED_INT_5_9_9_9_REV:
                return 4;

            case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
                return 8;

            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
                bytesPerChannel = 1;
                break;

            case GL_UNSIGNED_SHORT:
            case GL_SHORT:
            case GL_HALF_FLOAT:
            case GL_HALF_FLOAT_OES:
                bytesPerChannel = 2;
                break;

            case GL_UNSIGNED_INT:
            case GL_INT:
            case GL_FLOAT:
                bytesPerChannel = 4;
                break;

            default:
                assert(false);
                return 0;
        }

        switch (unpackFormat)
        {
            case GL_RGBA:
            case GL_RGBA_INTEGER:
                return bytesPerChannel * 4;

            case GL_RGB:
            case GL_RGB_INTEGER:
                return bytesPerChannel * 3;

            case GL_RG:
            case GL_RG_INTEGER:
            case GL_LUMINANCE_ALPHA:
                return bytesPerChannel * 2;

            case GL_RED:
            case GL_RED_INTEGER:
            case GL_LUMINANCE:
            case GL_ALPHA:
            case GL_DEPTH_COMPONENT:
                return bytesPerChannel * 1;

            default:
                assert(false);
                return 0;
        }
    }
};

template <const uint8_t bits>
constexpr uint32_t EncodeNormUint(const float val)
{
    return static_cast<uint32_t>(val * static_cast<float>(UINT32_MAX >> (32 - bits)) +
                                 0.5f);  // round-half-up
}

}  // anonymous namespace

namespace
{

template <typename DestT, typename SrcT, size_t SrcN>
void ZeroAndCopy(DestT &dest, const SrcT (&src)[SrcN])
{
    dest.fill(0);
    memcpy(dest.data(), src, sizeof(SrcT) * SrcN);
}

std::string EnumStr(const GLenum v)
{
    std::stringstream ret;
    ret << "0x" << std::hex << v;
    return ret.str();
}

template <typename ColorT, typename DestT>
void EncodeThenZeroAndCopy(DestT &dest, const float srcVals[4])
{
    ColorF srcValsF(srcVals[0], srcVals[1], srcVals[2], srcVals[3]);

    ColorT encoded;
    ColorT::writeColor(&encoded, &srcValsF);

    dest.fill(0);
    memcpy(dest.data(), &encoded, sizeof(ColorT));
}
}  // anonymous namespace

// Upload (1,2,5,3) to integer formats, and (1,2,5,3)/8.0 to float formats.
// Draw a point into a 1x1 renderbuffer and readback the result for comparison with expectations.
// Test all internalFormat/unpackFormat/unpackType combinations from ES3.0.
TEST_P(TextureUploadFormatTest, All)
{
    ANGLE_SKIP_TEST_IF(IsD3D9());

    constexpr char kVertShaderES2[]     = R"(
        void main()
        {
            gl_PointSize = 1.0;
            gl_Position = vec4(0, 0, 0, 1);
        })";
    constexpr char kFragShader_Floats[] = R"(
        precision mediump float;
        uniform sampler2D uTex;

        void main()
        {
            gl_FragColor = texture2D(uTex, vec2(0,0));
        })";
    ANGLE_GL_PROGRAM(floatsProg, kVertShaderES2, kFragShader_Floats);

    glDisable(GL_DITHER);

    ASSERT_GL_NO_ERROR();

    // Create the 1x1 framebuffer

    GLRenderbuffer backbufferRB;
    glBindRenderbuffer(GL_RENDERBUFFER, backbufferRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    GLFramebuffer backbufferFB;
    glBindFramebuffer(GL_FRAMEBUFFER, backbufferFB);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, backbufferRB);
    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glViewport(0, 0, 1, 1);

    // Create and bind our test texture

    GLTexture testTex;
    glBindTexture(GL_TEXTURE_2D, testTex);
    // Must be nearest because some texture formats aren't filterable!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    ASSERT_GL_NO_ERROR();

    // Initialize our test variables

    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    const bool hasSubrectUploads = !glGetError();

    constexpr uint8_t srcIntVals[4] = {1u, 2u, 5u, 3u};
    constexpr float srcVals[4] = {srcIntVals[0] / 8.0f, srcIntVals[1] / 8.0f, srcIntVals[2] / 8.0f,
                                  srcIntVals[3] / 8.0f};
    constexpr uint8_t refVals[4] = {static_cast<uint8_t>(EncodeNormUint<8>(srcVals[0])),
                                    static_cast<uint8_t>(EncodeNormUint<8>(srcVals[1])),
                                    static_cast<uint8_t>(EncodeNormUint<8>(srcVals[2])),
                                    static_cast<uint8_t>(EncodeNormUint<8>(srcVals[3]))};

    // Test a format with the specified data

    const auto fnTestData = [&](const TexFormat &format, const void *const data, const GLColor &err,
                                const char *const info) {
        ASSERT_GL_NO_ERROR();
        glTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, 1, 1, 0, format.unpackFormat,
                     format.unpackType, data);
        const auto uploadErr = glGetError();
        if (uploadErr)  // Format might not be supported. (e.g. on ES2)
            return;

        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, 1);

        const auto actual = ReadColor(0, 0);

        GLColor expected;
        std::optional<GLColor> alternativeExpected;
        switch (format.unpackFormat)
        {
            case GL_RGBA:
            case GL_RGBA_INTEGER:
                expected = {refVals[0], refVals[1], refVals[2], refVals[3]};
                break;
            case GL_RGB:
                expected = {refVals[0], refVals[1], refVals[2], 255};
                break;
            case GL_RG:
                expected = {refVals[0], refVals[1], 0, 255};
                break;
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_STENCIL:
                expected = {refVals[0], 0, 0, 255};
                // The green and blue channels are undefined. Some backends treat these textures are
                // luminance while others return 0 in g/b channels.
                alternativeExpected = {refVals[0], refVals[0], refVals[0], 255};
                break;
            case GL_RED:
                expected = {refVals[0], 0, 0, 255};
                break;

            case GL_RGB_INTEGER:
                expected = {refVals[0], refVals[1], refVals[2], refVals[0]};
                break;
            case GL_RG_INTEGER:
                expected = {refVals[0], refVals[1], 0, refVals[0]};
                break;
            case GL_RED_INTEGER:
                expected = {refVals[0], 0, 0, refVals[0]};
                break;

            case GL_LUMINANCE_ALPHA:
                expected = {refVals[0], refVals[0], refVals[0], refVals[1]};
                break;
            case GL_LUMINANCE:
                expected = {refVals[0], refVals[0], refVals[0], 255};
                break;
            case GL_ALPHA:
                expected = {0, 0, 0, refVals[0]};
                break;

            default:
                assert(false);
        }

        ASSERT_GL_NO_ERROR();
        auto result = actual.ExpectNear(expected, err);
        if (!result && alternativeExpected.has_value())
        {
            result = actual.ExpectNear(*alternativeExpected, err);
        }
        EXPECT_TRUE(result) << " [" << EnumStr(format.internalFormat) << "/"
                            << EnumStr(format.unpackFormat) << "/" << EnumStr(format.unpackType)
                            << " " << info << "]";
    };

    // Provide buffers for test data, and a func to run the test on both the data directly, and on
    // a basic subrect selection to ensure pixel byte size is calculated correctly.
    // Possible todo here is to add tests to ensure stride calculation.

    std::array<uint8_t, sizeof(float) * 4> srcBuffer;

    std::array<uint8_t, srcBuffer.size() * 2> subrectBuffer;
    const auto fnTest = [&](const TexFormat &format, const GLColor &err) {
        fnTestData(format, srcBuffer.data(), err, "simple");

        if (!hasSubrectUploads)
            return;

        const auto bytesPerPixel = format.bytesPerPixel();

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 1);

        subrectBuffer.fill(0);
        memcpy(subrectBuffer.data() + bytesPerPixel, srcBuffer.data(), bytesPerPixel);
        fnTestData(format, subrectBuffer.data(), err, "subrect");

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    };

    // Test All The Formats, organized by unpack format and type.
    // (Combos from GLES 3.0.5 p111-112: Table 3.2: "Valid combinations of format, type, and sized
    // internalformat.")

    // Start with normalized ints
    glUseProgram(floatsProg);

    // RGBA+UNSIGNED_BYTE
    {
        constexpr uint8_t src[] = {static_cast<uint8_t>(EncodeNormUint<8>(srcVals[0])),
                                   static_cast<uint8_t>(EncodeNormUint<8>(srcVals[1])),
                                   static_cast<uint8_t>(EncodeNormUint<8>(srcVals[2])),
                                   static_cast<uint8_t>(EncodeNormUint<8>(srcVals[3]))};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_BYTE), {8, 8, 8, 255});
        fnTest(TexFormat(GL_RGBA4, GL_RGBA, GL_UNSIGNED_BYTE), {16, 16, 16, 16});

        fnTest(TexFormat(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE), {1, 1, 1, 0});
        fnTest(TexFormat(GL_RGB565, GL_RGB, GL_UNSIGNED_BYTE), {8, 4, 8, 0});

        fnTest(TexFormat(GL_RG8, GL_RG, GL_UNSIGNED_BYTE), {1, 1, 0, 0});

        fnTest(TexFormat(GL_R8, GL_RED, GL_UNSIGNED_BYTE), {1, 0, 0, 0});

        fnTest(TexFormat(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE), {1, 1, 1, 0});
        fnTest(TexFormat(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE), {1, 1, 1, 1});
        fnTest(TexFormat(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE), {1, 1, 1, 0});
        fnTest(TexFormat(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE), {0, 0, 0, 1});
        if (IsGLExtensionEnabled("GL_OES_required_internalformat"))
        {
            fnTest(TexFormat(GL_LUMINANCE4_ALPHA4_OES, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE),
                   {4, 4, 4, 4});
            fnTest(TexFormat(GL_LUMINANCE8_ALPHA8_OES, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE),
                   {1, 1, 1, 1});
            fnTest(TexFormat(GL_LUMINANCE8_OES, GL_LUMINANCE, GL_UNSIGNED_BYTE), {1, 1, 1, 0});
            fnTest(TexFormat(GL_ALPHA8_OES, GL_ALPHA, GL_UNSIGNED_BYTE), {0, 0, 0, 1});
        }
    }

    // RGBA+BYTE
    {
        constexpr uint8_t src[] = {static_cast<uint8_t>(EncodeNormUint<7>(srcVals[0])),
                                   static_cast<uint8_t>(EncodeNormUint<7>(srcVals[1])),
                                   static_cast<uint8_t>(EncodeNormUint<7>(srcVals[2])),
                                   static_cast<uint8_t>(EncodeNormUint<7>(srcVals[3]))};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA8_SNORM, GL_RGBA, GL_BYTE), {2, 2, 2, 2});
        fnTest(TexFormat(GL_RGB8_SNORM, GL_RGB, GL_BYTE), {2, 2, 2, 0});
        fnTest(TexFormat(GL_RG8_SNORM, GL_RG, GL_BYTE), {2, 2, 0, 0});
        fnTest(TexFormat(GL_R8_SNORM, GL_RED, GL_BYTE), {2, 0, 0, 0});
    }

    // RGB+UNSIGNED_SHORT_5_6_5
    {
        constexpr uint16_t src[] = {static_cast<uint16_t>((EncodeNormUint<5>(srcVals[0]) << 11) |
                                                          (EncodeNormUint<6>(srcVals[1]) << 5) |
                                                          (EncodeNormUint<5>(srcVals[2]) << 0))};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5), {8, 4, 8, 0});
        fnTest(TexFormat(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5), {8, 4, 8, 0});
    }

    // RGBA+UNSIGNED_SHORT_4_4_4_4
    {
        constexpr uint16_t src[] = {static_cast<uint16_t>(
            (EncodeNormUint<4>(srcVals[0]) << 12) | (EncodeNormUint<4>(srcVals[1]) << 8) |
            (EncodeNormUint<4>(srcVals[2]) << 4) | (EncodeNormUint<4>(srcVals[3]) << 0))};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4), {16, 16, 16, 16});
        fnTest(TexFormat(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4), {16, 16, 16, 16});
    }

    // RGBA+UNSIGNED_SHORT_5_5_5_1
    {
        constexpr uint16_t src[] = {static_cast<uint16_t>(
            (EncodeNormUint<5>(srcVals[0]) << 11) | (EncodeNormUint<5>(srcVals[1]) << 6) |
            (EncodeNormUint<5>(srcVals[2]) << 1) | (EncodeNormUint<1>(srcVals[3]) << 0))};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1), {8, 8, 8, 255});
        fnTest(TexFormat(GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1), {8, 8, 8, 255});
    }

    // RGBA+UNSIGNED_INT_2_10_10_10_REV
    {
        constexpr uint32_t src[] = {
            (EncodeNormUint<10>(srcVals[0]) << 0) | (EncodeNormUint<10>(srcVals[1]) << 10) |
            (EncodeNormUint<10>(srcVals[2]) << 20) | (EncodeNormUint<2>(srcVals[3]) << 30)};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV), {1, 1, 1, 128});
        fnTest(TexFormat(GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV), {8, 8, 8, 255});
        fnTest(TexFormat(GL_RGBA, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV), {1, 1, 1, 128});
    }

    // RGB+UNSIGNED_INT_2_10_10_10_REV
    {
        constexpr uint32_t src[] = {
            (EncodeNormUint<10>(srcVals[0]) << 0) | (EncodeNormUint<10>(srcVals[1]) << 10) |
            (EncodeNormUint<10>(srcVals[2]) << 20) | (EncodeNormUint<2>(srcVals[3]) << 30)};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGB, GL_RGB, GL_UNSIGNED_INT_2_10_10_10_REV), {1, 1, 1, 0});
    }

    // DEPTH_COMPONENT+UNSIGNED_SHORT
    {
        const uint16_t src[] = {static_cast<uint16_t>(EncodeNormUint<16>(srcVals[0]))};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT),
               {1, 0, 0, 0});
    }

    // DEPTH_COMPONENT+UNSIGNED_INT
    {
        constexpr uint32_t src[] = {EncodeNormUint<32>(srcVals[0])};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT), {1, 0, 0, 0});
        fnTest(TexFormat(GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT), {1, 0, 0, 0});
    }

    // DEPTH_STENCIL+UNSIGNED_INT_24_8
    {
        // Drop stencil.
        constexpr uint32_t src[] = {EncodeNormUint<24>(srcVals[0]) << 8};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8),
               {1, 0, 0, 0});
    }

    if (getClientMajorVersion() < 3)
        return;

    constexpr char kVertShaderES3[]    = R"(#version 300 es
        void main()
        {
            gl_PointSize = 1.0;
            gl_Position = vec4(0, 0, 0, 1);
        })";
    constexpr char kFragShader_Ints[]  = R"(#version 300 es
        precision mediump float;
        uniform highp isampler2D uTex;
        out vec4 oFragColor;

        void main()
        {
            oFragColor = vec4(texture(uTex, vec2(0,0))) / 8.0;
        })";
    constexpr char kFragShader_Uints[] = R"(#version 300 es
        precision mediump float;
        uniform highp usampler2D uTex;
        out vec4 oFragColor;

        void main()
        {
            oFragColor = vec4(texture(uTex, vec2(0,0))) / 8.0;
        })";
    ANGLE_GL_PROGRAM(intsProg, kVertShaderES3, kFragShader_Ints);
    ANGLE_GL_PROGRAM(uintsProg, kVertShaderES3, kFragShader_Uints);

    // Non-normalized ints
    glUseProgram(intsProg);

    // RGBA_INTEGER+UNSIGNED_BYTE
    {
        constexpr uint8_t src[4] = {srcIntVals[0], srcIntVals[1], srcIntVals[2], srcIntVals[3]};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGB8I, GL_RGB_INTEGER, GL_BYTE), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RG8I, GL_RG_INTEGER, GL_BYTE), {1, 1, 1, 1});
        fnTest(TexFormat(GL_R8I, GL_RED_INTEGER, GL_BYTE), {1, 1, 1, 1});
    }

    // RGBA_INTEGER+UNSIGNED_SHORT
    {
        constexpr uint16_t src[4] = {srcIntVals[0], srcIntVals[1], srcIntVals[2], srcIntVals[3]};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGB16I, GL_RGB_INTEGER, GL_SHORT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RG16I, GL_RG_INTEGER, GL_SHORT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_R16I, GL_RED_INTEGER, GL_SHORT), {1, 1, 1, 1});
    }

    // RGBA_INTEGER+UNSIGNED_INT
    {
        constexpr uint32_t src[4] = {srcIntVals[0], srcIntVals[1], srcIntVals[2], srcIntVals[3]};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA32I, GL_RGBA_INTEGER, GL_INT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGB32I, GL_RGB_INTEGER, GL_INT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RG32I, GL_RG_INTEGER, GL_INT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_R32I, GL_RED_INTEGER, GL_INT), {1, 1, 1, 1});
    }

    // Non-normalized uints
    glUseProgram(uintsProg);

    // RGBA_INTEGER+UNSIGNED_BYTE
    {
        constexpr uint8_t src[4] = {srcIntVals[0], srcIntVals[1], srcIntVals[2], srcIntVals[3]};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE), {1, 1, 1, 1});
        fnTest(TexFormat(GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE), {1, 1, 1, 1});
    }

    // RGBA_INTEGER+UNSIGNED_SHORT
    {
        constexpr uint16_t src[4] = {srcIntVals[0], srcIntVals[1], srcIntVals[2], srcIntVals[3]};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT), {1, 1, 1, 1});
    }

    // RGBA_INTEGER+UNSIGNED_INT
    {
        constexpr uint32_t src[4] = {srcIntVals[0], srcIntVals[1], srcIntVals[2], srcIntVals[3]};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT), {1, 1, 1, 1});
    }

    // RGBA_INTEGER+UNSIGNED_INT_2_10_10_10_REV
    {
        constexpr uint32_t src[] = {static_cast<uint32_t>(srcIntVals[0] << 0) |
                                    static_cast<uint32_t>(srcIntVals[1] << 10) |
                                    static_cast<uint32_t>(srcIntVals[2] << 20) |
                                    static_cast<uint32_t>(srcIntVals[3] << 30)};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV),
               {1, 1, 1, 1});
    }

    // True floats
    glUseProgram(floatsProg);

    // RGBA+HALF_FLOAT
    {
        EncodeThenZeroAndCopy<R16G16B16A16F>(srcBuffer, srcVals);

        fnTest(TexFormat(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT), {1, 1, 1, 1});

        fnTest(TexFormat(GL_RGB16F, GL_RGB, GL_HALF_FLOAT), {1, 1, 1, 0});
        fnTest(TexFormat(GL_R11F_G11F_B10F, GL_RGB, GL_HALF_FLOAT), {1, 1, 1, 0});
        fnTest(TexFormat(GL_RGB9_E5, GL_RGB, GL_HALF_FLOAT), {1, 1, 1, 0});

        fnTest(TexFormat(GL_RG16F, GL_RG, GL_HALF_FLOAT), {1, 1, 0, 0});

        fnTest(TexFormat(GL_R16F, GL_RED, GL_HALF_FLOAT), {1, 0, 0, 0});

        fnTest(TexFormat(GL_RGBA, GL_RGBA, GL_HALF_FLOAT_OES), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGB, GL_RGB, GL_HALF_FLOAT_OES), {1, 1, 1, 0});
        fnTest(TexFormat(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT_OES), {1, 1, 1, 1});
        fnTest(TexFormat(GL_LUMINANCE, GL_LUMINANCE, GL_HALF_FLOAT_OES), {1, 1, 1, 0});
        fnTest(TexFormat(GL_ALPHA, GL_ALPHA, GL_HALF_FLOAT_OES), {0, 0, 0, 1});
    }

    // RGBA+FLOAT
    {
        ZeroAndCopy(srcBuffer, srcVals);

        fnTest(TexFormat(GL_RGBA32F, GL_RGBA, GL_FLOAT), {1, 1, 1, 1});
        fnTest(TexFormat(GL_RGBA16F, GL_RGBA, GL_FLOAT), {1, 1, 1, 1});

        fnTest(TexFormat(GL_RGB32F, GL_RGB, GL_FLOAT), {1, 1, 1, 0});
        fnTest(TexFormat(GL_RGB16F, GL_RGB, GL_FLOAT), {1, 1, 1, 0});
        fnTest(TexFormat(GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT), {1, 1, 1, 0});
        fnTest(TexFormat(GL_RGB9_E5, GL_RGB, GL_FLOAT), {1, 1, 1, 0});

        fnTest(TexFormat(GL_RG32F, GL_RG, GL_FLOAT), {1, 1, 0, 0});
        fnTest(TexFormat(GL_RG16F, GL_RG, GL_FLOAT), {1, 1, 0, 0});

        fnTest(TexFormat(GL_R32F, GL_RED, GL_FLOAT), {1, 0, 0, 0});
        fnTest(TexFormat(GL_R16F, GL_RED, GL_FLOAT), {1, 0, 0, 0});
    }

    // UNSIGNED_INT_10F_11F_11F_REV
    {
        EncodeThenZeroAndCopy<R11G11B10F>(srcBuffer, srcVals);

        fnTest(TexFormat(GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV), {1, 1, 1, 0});
    }

    // UNSIGNED_INT_5_9_9_9_REV
    {
        EncodeThenZeroAndCopy<R9G9B9E5>(srcBuffer, srcVals);

        fnTest(TexFormat(GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV), {1, 1, 1, 0});
    }

    // DEPTH_COMPONENT+FLOAT
    {
        // Skip stencil.
        constexpr float src[] = {srcVals[0], 0};
        ZeroAndCopy(srcBuffer, src);

        fnTest(TexFormat(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT), {1, 0, 0, 0});
        fnTest(TexFormat(GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV),
               {1, 0, 0, 0});
    }

    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(TextureUploadFormatTest);
