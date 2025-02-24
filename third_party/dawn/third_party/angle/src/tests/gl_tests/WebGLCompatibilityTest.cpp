//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WebGLCompatibilityTest.cpp : Tests of the GL_ANGLE_webgl_compatibility extension.

#include "test_utils/ANGLETest.h"

#include "common/mathutil.h"
#include "test_utils/gl_raii.h"

namespace
{

bool ConstantColorAndAlphaBlendFunctions(GLenum first, GLenum second)
{
    return (first == GL_CONSTANT_COLOR || first == GL_ONE_MINUS_CONSTANT_COLOR) &&
           (second == GL_CONSTANT_ALPHA || second == GL_ONE_MINUS_CONSTANT_ALPHA);
}

void CheckBlendFunctions(GLenum src, GLenum dst)
{
    if (ConstantColorAndAlphaBlendFunctions(src, dst) ||
        ConstantColorAndAlphaBlendFunctions(dst, src))
    {
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
    else
    {
        ASSERT_GL_NO_ERROR();
    }
}

// Extensions that affect the ability to use floating point textures
constexpr const char *FloatingPointTextureExtensions[] = {
    "",
    "GL_EXT_texture_storage",
    "GL_OES_texture_half_float",
    "GL_OES_texture_half_float_linear",
    "GL_EXT_color_buffer_half_float",
    "GL_OES_texture_float",
    "GL_OES_texture_float_linear",
    "GL_EXT_color_buffer_float",
    "GL_EXT_float_blend",
    "GL_CHROMIUM_color_buffer_float_rgba",
    "GL_CHROMIUM_color_buffer_float_rgb",
};

}  // namespace

namespace angle
{

class WebGLCompatibilityTest : public ANGLETest<>
{
  protected:
    WebGLCompatibilityTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setWebGLCompatibilityEnabled(true);
    }

    template <typename T>
    void TestFloatTextureFormat(GLenum internalFormat,
                                GLenum format,
                                GLenum type,
                                bool texturingEnabled,
                                bool linearSamplingEnabled,
                                bool renderingEnabled,
                                const T textureData[4],
                                const float floatData[4])
    {
        ASSERT_GL_NO_ERROR();

        constexpr char kVS[] =
            R"(attribute vec4 position;
varying vec2 texcoord;
void main()
{
    gl_Position = vec4(position.xy, 0.0, 1.0);
    texcoord = (position.xy * 0.5) + 0.5;
})";

        constexpr char kFS[] =
            R"(precision mediump float;
uniform sampler2D tex;
uniform vec4 subtractor;
varying vec2 texcoord;
void main()
{
    vec4 color = texture2D(tex, texcoord);
    if (abs(color.r - subtractor.r) +
        abs(color.g - subtractor.g) +
        abs(color.b - subtractor.b) +
        abs(color.a - subtractor.a) < 8.0)
    {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else
    {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
})";

        ANGLE_GL_PROGRAM(samplingProgram, kVS, kFS);
        glUseProgram(samplingProgram);

        // Need RGBA8 renderbuffers for enough precision on the readback
        if (IsGLExtensionRequestable("GL_OES_rgb8_rgba8"))
        {
            glRequestExtensionANGLE("GL_OES_rgb8_rgba8");
        }
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_rgb8_rgba8") &&
                           getClientMajorVersion() < 3);
        ASSERT_GL_NO_ERROR();

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);

        if (internalFormat == format)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 1, 1, 0, format, type, textureData);
        }
        else
        {
            if (getClientMajorVersion() >= 3)
            {
                glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, 1, 1);
            }
            else
            {
                ASSERT_TRUE(IsGLExtensionEnabled("GL_EXT_texture_storage"));
                glTexStorage2DEXT(GL_TEXTURE_2D, 1, internalFormat, 1, 1);
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, format, type, textureData);
        }

        if (!texturingEnabled)
        {
            // Depending on the entry point and client version, different errors may be generated
            ASSERT_GLENUM_NE(GL_NO_ERROR, glGetError());

            // Two errors may be generated in the glTexStorage + glTexSubImage case, clear the
            // second error
            glGetError();

            return;
        }
        ASSERT_GL_NO_ERROR();

        glUniform1i(glGetUniformLocation(samplingProgram, "tex"), 0);
        glUniform4fv(glGetUniformLocation(samplingProgram, "subtractor"), 1, floatData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        drawQuad(samplingProgram, "position", 0.5f, 1.0f, true);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        drawQuad(samplingProgram, "position", 0.5f, 1.0f, true);

        if (linearSamplingEnabled)
        {
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
        }
        else
        {
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (!renderingEnabled)
        {
            EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                             glCheckFramebufferStatus(GL_FRAMEBUFFER));
            return;
        }

        GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (framebufferStatus == GL_FRAMEBUFFER_UNSUPPORTED)
        {
            std::cout << "Framebuffer returned GL_FRAMEBUFFER_UNSUPPORTED, this is legal."
                      << std::endl;
            return;
        }
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, framebufferStatus);

        ANGLE_GL_PROGRAM(renderingProgram, essl1_shaders::vs::Simple(),
                         essl1_shaders::fs::UniformColor());
        glUseProgram(renderingProgram);

        glUniform4fv(glGetUniformLocation(renderingProgram, essl1_shaders::ColorUniform()), 1,
                     floatData);

        drawQuad(renderingProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

        EXPECT_PIXEL_COLOR32F_NEAR(
            0, 0, GLColor32F(floatData[0], floatData[1], floatData[2], floatData[3]), 1.0f);
    }

    void TestExtFloatBlend(GLenum internalFormat, GLenum type, bool shouldBlend)
    {
        constexpr char kVS[] =
            R"(void main()
{
    gl_PointSize = 1.0;
    gl_Position = vec4(0, 0, 0, 1);
})";

        constexpr char kFS[] =
            R"(void main()
{
    gl_FragColor = vec4(0.5, 0, 0, 0);
})";

        ANGLE_GL_PROGRAM(program, kVS, kFS);
        glUseProgram(program);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 1, 1, 0, GL_RGBA, type, nullptr);
        EXPECT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_EGLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_COLOR32F_NEAR(0, 0, GLColor32F(1, 0, 1, 1), 0.001f);

        glDisable(GL_BLEND);
        glDrawArrays(GL_POINTS, 0, 1);
        EXPECT_GL_NO_ERROR();

        glEnable(GL_BLEND);
        glBlendFunc(GL_CONSTANT_COLOR, GL_ZERO);
        glBlendColor(10, 1, 1, 1);
        glViewport(0, 0, 1, 1);
        glDrawArrays(GL_POINTS, 0, 1);
        if (!shouldBlend)
        {
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            return;
        }
        EXPECT_GL_NO_ERROR();

        // Ensure that the stored value reflect the actual platform behavior.
        float storedColor[4];
        glGetFloatv(GL_BLEND_COLOR, storedColor);
        if (storedColor[0] == 10)
        {
            EXPECT_PIXEL_COLOR32F_NEAR(0, 0, GLColor32F(5, 0, 0, 0), 0.001f);
        }
        else
        {
            EXPECT_PIXEL_COLOR32F_NEAR(0, 0, GLColor32F(0.5, 0, 0, 0), 0.001f);
        }

        // Check sure that non-float attachments clamp BLEND_COLOR.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glDrawArrays(GL_POINTS, 0, 1);

        EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0x80, 0, 0, 0), 1);
    }

    void TestDifferentStencilMaskAndRef(GLenum errIfMismatch);

    // Called from RenderingFeedbackLoopWithDrawBuffersEXT.
    void drawBuffersEXTFeedbackLoop(GLuint program,
                                    const std::array<GLenum, 2> &drawBuffers,
                                    GLenum expectedError);

    // Called from RenderingFeedbackLoopWithDrawBuffers.
    void drawBuffersFeedbackLoop(GLuint program,
                                 const std::array<GLenum, 2> &drawBuffers,
                                 GLenum expectedError);

    // Called from Enable[Compressed]TextureFormatExtensions
    void validateTexImageExtensionFormat(GLenum format, const std::string &extName);
    void validateCompressedTexImageExtensionFormat(GLenum format,
                                                   GLsizei width,
                                                   GLsizei height,
                                                   GLsizei blockSize,
                                                   const std::string &extName,
                                                   bool subImageAllowed);

    GLint expectedByteLength(GLenum format, GLsizei width, GLsizei height);
    void testCompressedTexLevelDimension(GLenum format,
                                         GLint level,
                                         GLsizei width,
                                         GLsizei height,
                                         GLsizei expectedByteLength,
                                         GLenum expectedError,
                                         const char *explanation);
    void testCompressedTexImage(GLenum format);
};

class WebGL2CompatibilityTest : public WebGLCompatibilityTest
{};

// Context creation would fail if EGL_ANGLE_create_context_webgl_compatibility was not available so
// the GL extension should always be present
TEST_P(WebGLCompatibilityTest, ExtensionStringExposed)
{
    EXPECT_TRUE(IsGLExtensionEnabled("GL_ANGLE_webgl_compatibility"));
}

// Verify that all extension entry points are available
TEST_P(WebGLCompatibilityTest, EntryPoints)
{
    if (IsGLExtensionEnabled("GL_ANGLE_request_extension"))
    {
        EXPECT_NE(nullptr, eglGetProcAddress("glRequestExtensionANGLE"));
    }
}

// WebGL 1 allows GL_DEPTH_STENCIL_ATTACHMENT as a valid binding point.  Make sure it is usable,
// even in ES2 contexts.
TEST_P(WebGLCompatibilityTest, DepthStencilBindingPoint)
{
    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 32, 32);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffer);

    EXPECT_GL_NO_ERROR();
}

// Test that attempting to enable an extension that doesn't exist generates GL_INVALID_OPERATION
TEST_P(WebGLCompatibilityTest, EnableExtensionValidation)
{
    glRequestExtensionANGLE("invalid_extension_string");
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test enabling the GL_OES_element_index_uint extension
TEST_P(WebGLCompatibilityTest, EnableExtensionUintIndices)
{
    if (getClientMajorVersion() != 2)
    {
        // This test only works on ES2 where uint indices are not available by default
        return;
    }

    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_element_index_uint"));

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    GLuint data[] = {0, 1, 2, 1, 3, 2};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, "void main() { gl_Position = vec4(0, 0, 0, 1); }",
                     "void main() { gl_FragColor = vec4(0, 1, 0, 1); }");
    glUseProgram(program);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_OES_element_index_uint"))
    {
        glRequestExtensionANGLE("GL_OES_element_index_uint");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_OES_element_index_uint"));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_OES_standard_derivatives extension
TEST_P(WebGLCompatibilityTest, EnableExtensionStandardDerivitives)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_standard_derivatives"));

    constexpr char kFS[] =
        R"(#extension GL_OES_standard_derivatives : require
void main() { gl_FragColor = vec4(dFdx(vec2(1.0, 1.0)).x, 1, 0, 1); })";
    ASSERT_EQ(0u, CompileShader(GL_FRAGMENT_SHADER, kFS));

    if (IsGLExtensionRequestable("GL_OES_standard_derivatives"))
    {
        glRequestExtensionANGLE("GL_OES_standard_derivatives");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_OES_standard_derivatives"));

        GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
        ASSERT_NE(0u, shader);
        glDeleteShader(shader);
    }
}

// Test enabling the GL_EXT_shader_texture_lod extension
TEST_P(WebGLCompatibilityTest, EnableExtensionTextureLOD)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_shader_texture_lod"));

    constexpr char kFS[] =
        R"(#extension GL_EXT_shader_texture_lod : require
uniform sampler2D u_texture;
void main() {
    gl_FragColor = texture2DGradEXT(u_texture, vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0,
0.0));
})";
    ASSERT_EQ(0u, CompileShader(GL_FRAGMENT_SHADER, kFS));

    if (IsGLExtensionRequestable("GL_EXT_shader_texture_lod"))
    {
        glRequestExtensionANGLE("GL_EXT_shader_texture_lod");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_EXT_shader_texture_lod"));

        GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
        ASSERT_NE(0u, shader);
        glDeleteShader(shader);
    }
}

// Test enabling the GL_EXT_frag_depth extension
TEST_P(WebGLCompatibilityTest, EnableExtensionFragDepth)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_frag_depth"));

    constexpr char kFS[] =
        R"(#extension GL_EXT_frag_depth : require
void main() {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    gl_FragDepthEXT = 1.0;
})";
    ASSERT_EQ(0u, CompileShader(GL_FRAGMENT_SHADER, kFS));

    if (IsGLExtensionRequestable("GL_EXT_frag_depth"))
    {
        glRequestExtensionANGLE("GL_EXT_frag_depth");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_EXT_frag_depth"));

        GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
        ASSERT_NE(0u, shader);
        glDeleteShader(shader);
    }
}

// Test enabling the GL_EXT_texture_filter_anisotropic extension
TEST_P(WebGLCompatibilityTest, EnableExtensionTextureFilterAnisotropic)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_texture_filter_anisotropic"));

    GLfloat maxAnisotropy = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    ASSERT_GL_NO_ERROR();

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLfloat currentAnisotropy = 0.0f;
    glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &currentAnisotropy);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_EXT_texture_filter_anisotropic"))
    {
        glRequestExtensionANGLE("GL_EXT_texture_filter_anisotropic");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_EXT_texture_filter_anisotropic"));

        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        ASSERT_GL_NO_ERROR();
        EXPECT_GE(maxAnisotropy, 2.0f);

        glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &currentAnisotropy);
        ASSERT_GL_NO_ERROR();
        EXPECT_EQ(1.0f, currentAnisotropy);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f);
        ASSERT_GL_NO_ERROR();
    }
}

// Test enabling the EGL image extensions
TEST_P(WebGLCompatibilityTest, EnableExtensionEGLImage)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_EGL_image"));
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_EGL_image_external"));
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_EGL_image_external_essl3"));
    EXPECT_FALSE(IsGLExtensionEnabled("NV_EGL_stream_consumer_external"));

    constexpr char kFSES2[] =
        R"(#extension GL_OES_EGL_image_external : require
precision highp float;
uniform samplerExternalOES sampler;
void main()
{
    gl_FragColor = texture2D(sampler, vec2(0, 0));
})";
    EXPECT_EQ(0u, CompileShader(GL_FRAGMENT_SHADER, kFSES2));

    constexpr char kFSES3[] =
        R"(#version 300 es
#extension GL_OES_EGL_image_external_essl3 : require
precision highp float;
uniform samplerExternalOES sampler;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(sampler, vec2(0, 0));
})";
    if (getClientMajorVersion() >= 3)
    {
        EXPECT_EQ(0u, CompileShader(GL_FRAGMENT_SHADER, kFSES3));
    }

    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint result;
    glGetIntegerv(GL_TEXTURE_BINDING_EXTERNAL_OES, &result);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_OES_EGL_image_external"))
    {
        glRequestExtensionANGLE("GL_OES_EGL_image_external");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_OES_EGL_image_external"));

        EXPECT_NE(0u, CompileShader(GL_FRAGMENT_SHADER, kFSES2));

        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
        EXPECT_GL_NO_ERROR();

        glGetIntegerv(GL_TEXTURE_BINDING_EXTERNAL_OES, &result);
        EXPECT_GL_NO_ERROR();

        if (getClientMajorVersion() >= 3 &&
            IsGLExtensionRequestable("GL_OES_EGL_image_external_essl3"))
        {
            glRequestExtensionANGLE("GL_OES_EGL_image_external_essl3");
            EXPECT_GL_NO_ERROR();
            EXPECT_TRUE(IsGLExtensionEnabled("GL_OES_EGL_image_external_essl3"));

            EXPECT_NE(0u, CompileShader(GL_FRAGMENT_SHADER, kFSES3));
        }
        else
        {
            EXPECT_EQ(0u, CompileShader(GL_FRAGMENT_SHADER, kFSES3));
        }
    }
}

// Verify that shaders are of a compatible spec when the extension is enabled.
TEST_P(WebGLCompatibilityTest, ExtensionCompilerSpec)
{
    EXPECT_TRUE(IsGLExtensionEnabled("GL_ANGLE_webgl_compatibility"));

    // Use of reserved _webgl prefix should fail when the shader specification is for WebGL.
    constexpr char kVS[] =
        R"(struct Foo {
    int _webgl_bar;
};
void main()
{
    Foo foo = Foo(1);
})";

    // Default fragement shader.
    constexpr char kFS[] =
        R"(void main()
{
    gl_FragColor = vec4(1.0,0.0,0.0,1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
    glDeleteProgram(program);
}

// Test enabling the GL_NV_pixel_buffer_object extension
TEST_P(WebGLCompatibilityTest, EnablePixelBufferObjectExtensions)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_NV_pixel_buffer_object"));
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_mapbuffer"));
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_map_buffer_range"));

    // These extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    // http://anglebug.com/40644771
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    GLBuffer buffer;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_NV_pixel_buffer_object"))
    {
        glRequestExtensionANGLE("GL_NV_pixel_buffer_object");
        EXPECT_GL_NO_ERROR();

        // Create a framebuffer to read from
        GLRenderbuffer renderbuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 1, 1);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderbuffer);
        EXPECT_GL_NO_ERROR();

        glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
        EXPECT_GL_NO_ERROR();

        glBufferData(GL_PIXEL_PACK_BUFFER, 4, nullptr, GL_STATIC_DRAW);
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_EXT_texture_storage extension
TEST_P(WebGLCompatibilityTest, EnableTextureStorage)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_texture_storage"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    GLint result;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_FORMAT, &result);
    if (getClientMajorVersion() >= 3)
    {
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    if (IsGLExtensionRequestable("GL_EXT_texture_storage"))
    {
        glRequestExtensionANGLE("GL_EXT_texture_storage");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_EXT_texture_storage"));

        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_FORMAT, &result);
        EXPECT_GL_NO_ERROR();

        const GLenum alwaysAcceptableFormats[] = {
            GL_ALPHA8_EXT,
            GL_LUMINANCE8_EXT,
            GL_LUMINANCE8_ALPHA8_EXT,
        };
        for (const auto &acceptableFormat : alwaysAcceptableFormats)
        {
            GLTexture localTexture;
            glBindTexture(GL_TEXTURE_2D, localTexture);
            glTexStorage2DEXT(GL_TEXTURE_2D, 1, acceptableFormat, 1, 1);
            EXPECT_GL_NO_ERROR();
        }
    }
}

// Test enabling the GL_OES_mapbuffer and GL_EXT_map_buffer_range extensions
TEST_P(WebGLCompatibilityTest, EnableMapBufferExtensions)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_mapbuffer"));
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_map_buffer_range"));

    // These extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLBuffer buffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4, nullptr, GL_STATIC_DRAW);

    glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glMapBufferRangeEXT(GL_ELEMENT_ARRAY_BUFFER, 0, 4, GL_MAP_WRITE_BIT);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLint access = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_ACCESS_OES, &access);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_OES_mapbuffer"))
    {
        glRequestExtensionANGLE("GL_OES_mapbuffer");
        EXPECT_GL_NO_ERROR();

        glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
        glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER);
        glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_ACCESS_OES, &access);
        EXPECT_GL_NO_ERROR();
    }

    if (IsGLExtensionRequestable("GL_EXT_map_buffer_range"))
    {
        glRequestExtensionANGLE("GL_EXT_map_buffer_range");
        EXPECT_GL_NO_ERROR();

        glMapBufferRangeEXT(GL_ELEMENT_ARRAY_BUFFER, 0, 4, GL_MAP_WRITE_BIT);
        glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER);
        glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_ACCESS_OES, &access);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_OES_fbo_render_mipmap extension
TEST_P(WebGLCompatibilityTest, EnableRenderMipmapExtension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_fbo_render_mipmap"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    if (IsGLExtensionRequestable("GL_OES_fbo_render_mipmap"))
    {
        glRequestExtensionANGLE("GL_OES_fbo_render_mipmap");
        EXPECT_GL_NO_ERROR();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 1);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_EXT_blend_minmax extension
TEST_P(WebGLCompatibilityTest, EnableBlendMinMaxExtension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_blend_minmax"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    glBlendEquation(GL_MIN);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glBlendEquation(GL_MAX);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_EXT_blend_minmax"))
    {
        glRequestExtensionANGLE("GL_EXT_blend_minmax");
        EXPECT_GL_NO_ERROR();

        glBlendEquation(GL_MIN);
        glBlendEquation(GL_MAX);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the query extensions
TEST_P(WebGLCompatibilityTest, EnableQueryExtensions)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));
    EXPECT_FALSE(IsGLExtensionEnabled("GL_CHROMIUM_sync_query"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLQueryEXT badQuery;

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, badQuery);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, badQuery);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, badQuery);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glQueryCounterEXT(GL_TIMESTAMP_EXT, badQuery);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, badQuery);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    if (IsGLExtensionRequestable("GL_EXT_occlusion_query_boolean"))
    {
        glRequestExtensionANGLE("GL_EXT_occlusion_query_boolean");
        EXPECT_GL_NO_ERROR();

        GLQueryEXT query;
        glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
        glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
        EXPECT_GL_NO_ERROR();
    }

    if (IsGLExtensionRequestable("GL_EXT_disjoint_timer_query"))
    {
        glRequestExtensionANGLE("GL_EXT_disjoint_timer_query");
        EXPECT_GL_NO_ERROR();

        GLQueryEXT query1;
        glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query1);
        glEndQueryEXT(GL_TIME_ELAPSED_EXT);
        EXPECT_GL_NO_ERROR();

        GLQueryEXT query2;
        glQueryCounterEXT(query2, GL_TIMESTAMP_EXT);
        EXPECT_GL_NO_ERROR();
    }

    if (IsGLExtensionRequestable("GL_CHROMIUM_sync_query"))
    {
        glRequestExtensionANGLE("GL_CHROMIUM_sync_query");
        EXPECT_GL_NO_ERROR();

        GLQueryEXT query;
        glBeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, query);
        glEndQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_ANGLE_framebuffer_multisample extension
TEST_P(WebGLCompatibilityTest, EnableFramebufferMultisampleExtension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_ANGLE_framebuffer_multisample"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLint maxSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 1, GL_RGBA4, 1, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    if (IsGLExtensionRequestable("GL_ANGLE_framebuffer_multisample"))
    {
        glRequestExtensionANGLE("GL_ANGLE_framebuffer_multisample");
        EXPECT_GL_NO_ERROR();

        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
        EXPECT_GL_NO_ERROR();

        glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, maxSamples, GL_RGBA4, 1, 1);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_ANGLE_instanced_arrays extension
TEST_P(WebGLCompatibilityTest, EnableInstancedArraysExtensionANGLE)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLint divisor = 0;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &divisor);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glVertexAttribDivisorANGLE(0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    if (IsGLExtensionRequestable("GL_ANGLE_instanced_arrays"))
    {
        glRequestExtensionANGLE("GL_ANGLE_instanced_arrays");
        EXPECT_GL_NO_ERROR();

        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &divisor);
        glVertexAttribDivisorANGLE(0, 1);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_EXT_instanced_arrays extension
TEST_P(WebGLCompatibilityTest, EnableInstancedArraysExtensionEXT)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_instanced_arrays"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLint divisor = 0;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &divisor);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glVertexAttribDivisorEXT(0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    if (IsGLExtensionRequestable("GL_EXT_instanced_arrays"))
    {
        glRequestExtensionANGLE("GL_EXT_instanced_arrays");
        EXPECT_GL_NO_ERROR();

        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &divisor);
        glVertexAttribDivisorEXT(0, 1);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_ANGLE_pack_reverse_row_order extension
TEST_P(WebGLCompatibilityTest, EnablePackReverseRowOrderExtension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_ANGLE_pack_reverse_row_order"));

    GLint result = 0;
    glGetIntegerv(GL_PACK_REVERSE_ROW_ORDER_ANGLE, &result);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glPixelStorei(GL_PACK_REVERSE_ROW_ORDER_ANGLE, GL_TRUE);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_ANGLE_pack_reverse_row_order"))
    {
        glRequestExtensionANGLE("GL_ANGLE_pack_reverse_row_order");
        EXPECT_GL_NO_ERROR();

        glGetIntegerv(GL_PACK_REVERSE_ROW_ORDER_ANGLE, &result);
        glPixelStorei(GL_PACK_REVERSE_ROW_ORDER_ANGLE, GL_TRUE);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_EXT_unpack_subimage extension
TEST_P(WebGLCompatibilityTest, EnablePackUnpackSubImageExtension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_EXT_unpack_subimage"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    constexpr GLenum parameters[] = {
        GL_UNPACK_ROW_LENGTH_EXT,
        GL_UNPACK_SKIP_ROWS_EXT,
        GL_UNPACK_SKIP_PIXELS_EXT,
    };

    for (GLenum param : parameters)
    {
        GLint resultI = 0;
        glGetIntegerv(param, &resultI);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        GLfloat resultF = 0.0f;
        glGetFloatv(param, &resultF);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glPixelStorei(param, 0);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    if (IsGLExtensionRequestable("GL_EXT_unpack_subimage"))
    {
        glRequestExtensionANGLE("GL_EXT_unpack_subimage");
        EXPECT_GL_NO_ERROR();

        for (GLenum param : parameters)
        {
            GLint resultI = 0;
            glGetIntegerv(param, &resultI);

            GLfloat resultF = 0.0f;
            glGetFloatv(param, &resultF);

            glPixelStorei(param, 0);

            EXPECT_GL_NO_ERROR();
        }
    }
}

TEST_P(WebGLCompatibilityTest, EnableTextureRectangle)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_ANGLE_texture_rectangle"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, texture);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint minFilter = 0;
    glGetTexParameteriv(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_MIN_FILTER, &minFilter);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_ANGLE_texture_rectangle"))
    {
        glRequestExtensionANGLE("GL_ANGLE_texture_rectangle");
        EXPECT_GL_NO_ERROR();

        EXPECT_TRUE(IsGLExtensionEnabled("GL_ANGLE_texture_rectangle"));

        glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, texture);
        EXPECT_GL_NO_ERROR();

        glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        EXPECT_GL_NO_ERROR();

        glDisableExtensionANGLE("GL_ANGLE_texture_rectangle");
        EXPECT_GL_NO_ERROR();

        EXPECT_FALSE(IsGLExtensionEnabled("GL_ANGLE_texture_rectangle"));

        glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, texture);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Test enabling the GL_NV_pack_subimage extension
TEST_P(WebGLCompatibilityTest, EnablePackPackSubImageExtension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_NV_pack_subimage"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    constexpr GLenum parameters[] = {
        GL_PACK_ROW_LENGTH,
        GL_PACK_SKIP_ROWS,
        GL_PACK_SKIP_PIXELS,
    };

    for (GLenum param : parameters)
    {
        GLint resultI = 0;
        glGetIntegerv(param, &resultI);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        GLfloat resultF = 0.0f;
        glGetFloatv(param, &resultF);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glPixelStorei(param, 0);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    if (IsGLExtensionRequestable("GL_NV_pack_subimage"))
    {
        glRequestExtensionANGLE("GL_NV_pack_subimage");
        EXPECT_GL_NO_ERROR();

        for (GLenum param : parameters)
        {
            GLint resultI = 0;
            glGetIntegerv(param, &resultI);

            GLfloat resultF = 0.0f;
            glGetFloatv(param, &resultF);

            glPixelStorei(param, 0);

            EXPECT_GL_NO_ERROR();
        }
    }
}

TEST_P(WebGLCompatibilityTest, EnableRGB8RGBA8Extension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_rgb8_rgba8"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    EXPECT_GL_NO_ERROR();

    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8_OES, 1, 1);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, 1, 1);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_OES_rgb8_rgba8"))
    {
        glRequestExtensionANGLE("GL_OES_rgb8_rgba8");
        EXPECT_GL_NO_ERROR();

        EXPECT_TRUE(IsGLExtensionEnabled("GL_OES_rgb8_rgba8"));

        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8_OES, 1, 1);
        EXPECT_GL_NO_ERROR();

        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, 1, 1);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_ANGLE_framebuffer_blit extension
TEST_P(WebGLCompatibilityTest, EnableFramebufferBlitExtension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLFramebuffer fbo;

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, fbo);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint result;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_ANGLE, &result);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glBlitFramebufferANGLE(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    if (IsGLExtensionRequestable("GL_ANGLE_framebuffer_blit"))
    {
        glRequestExtensionANGLE("GL_ANGLE_framebuffer_blit");
        EXPECT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, fbo);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_ANGLE, &result);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_OES_get_program_binary extension
TEST_P(WebGLCompatibilityTest, EnableProgramBinaryExtension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_get_program_binary"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLint result           = 0;
    GLint numBinaryFormats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numBinaryFormats);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &result);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    constexpr char kVS[] =
        R"(void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
})";
    constexpr char kFS[] =
        R"(precision highp float;
void main()
{
    gl_FragColor = vec4(1.0);
})";
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &result);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    uint8_t tempArray[512];
    GLenum tempFormat  = 0;
    GLsizei tempLength = 0;
    glGetProgramBinaryOES(program, static_cast<GLsizei>(ArraySize(tempArray)), &tempLength,
                          &tempFormat, tempArray);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    if (IsGLExtensionRequestable("GL_OES_get_program_binary"))
    {
        glRequestExtensionANGLE("GL_OES_get_program_binary");
        EXPECT_GL_NO_ERROR();

        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numBinaryFormats);
        // No use to test further if no binary formats are supported
        ANGLE_SKIP_TEST_IF(numBinaryFormats < 1);

        glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &result);
        EXPECT_GL_NO_ERROR();

        GLint binaryLength = 0;
        glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
        EXPECT_GL_NO_ERROR();

        GLenum binaryFormat;
        GLsizei writeLength = 0;
        std::vector<uint8_t> binary(binaryLength);
        glGetProgramBinaryOES(program, binaryLength, &writeLength, &binaryFormat, binary.data());
        EXPECT_GL_NO_ERROR();

        glProgramBinaryOES(program, binaryFormat, binary.data(), binaryLength);
        EXPECT_GL_NO_ERROR();
    }
}

// Test enabling the GL_OES_vertex_array_object extension
TEST_P(WebGLCompatibilityTest, EnableVertexArrayExtension)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_vertex_array_object"));

    // This extensions become core in in ES3/WebGL2.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() >= 3);

    GLint result = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &result);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Expect that GL_OES_vertex_array_object is always available.  It is implemented in the GL
    // frontend.
    EXPECT_TRUE(IsGLExtensionRequestable("GL_OES_vertex_array_object"));

    glRequestExtensionANGLE("GL_OES_vertex_array_object");
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(IsGLExtensionEnabled("GL_OES_vertex_array_object"));

    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &result);
    EXPECT_GL_NO_ERROR();

    GLuint vao = 0;
    glGenVertexArraysOES(0, &vao);
    EXPECT_GL_NO_ERROR();

    glBindVertexArrayOES(vao);
    EXPECT_GL_NO_ERROR();

    glDeleteVertexArraysOES(1, &vao);
    EXPECT_GL_NO_ERROR();
}

// Verify that the context generates the correct error when the framebuffer attachments are
// different sizes
TEST_P(WebGLCompatibilityTest, FramebufferAttachmentSizeMismatch)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture textures[2];
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 3, 3);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    if (IsGLExtensionRequestable("GL_EXT_draw_buffers"))
    {
        glRequestExtensionANGLE("GL_EXT_draw_buffers");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_EXT_draw_buffers"));

        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[1], 0);
        ASSERT_GL_NO_ERROR();

        ASSERT_GL_NO_ERROR();
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS,
                         glCheckFramebufferStatus(GL_FRAMEBUFFER));

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

        ASSERT_GL_NO_ERROR();
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        ASSERT_GL_NO_ERROR();
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS,
                         glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
}

// Test that client-side array buffers are forbidden in WebGL mode
TEST_P(WebGLCompatibilityTest, ForbidsClientSideArrayBuffer)
{
    constexpr char kVS[] =
        R"(attribute vec3 a_pos;
void main()
{
    gl_Position = vec4(a_pos, 1.0);
})";

    constexpr char kFS[] =
        R"(precision highp float;
void main()
{
    gl_FragColor = vec4(1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint posLocation = glGetAttribLocation(program, "a_pos");
    ASSERT_NE(-1, posLocation);
    glUseProgram(program);

    const auto &vertices = GetQuadVertices();
    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 4, vertices.data());
    glEnableVertexAttribArray(posLocation);

    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that client-side element array buffers are forbidden in WebGL mode
TEST_P(WebGLCompatibilityTest, ForbidsClientSideElementBuffer)
{
    constexpr char kVS[] =
        R"(attribute vec3 a_pos;
void main()
{
    gl_Position = vec4(a_pos, 1.0);
})";

    constexpr char kFS[] =
        R"(precision highp float;
void main()
{
    gl_FragColor = vec4(1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint posLocation = glGetAttribLocation(program, "a_pos");
    ASSERT_NE(-1, posLocation);
    glUseProgram(program);

    const auto &vertices = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);

    ASSERT_GL_NO_ERROR();

    // Use the pointer with value of 1 for indices instead of an actual pointer because WebGL also
    // enforces that the top bit of indices must be 0 (i.e. offset >= 0) and would generate
    // GL_INVALID_VALUE in that case. Using a null pointer gets caught by another check.
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(intptr_t(1)));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that client-side array buffers are forbidden even if the program doesn't use the attribute
TEST_P(WebGLCompatibilityTest, ForbidsClientSideArrayBufferEvenNotUsedOnes)
{
    constexpr char kVS[] =
        R"(void main()
{
    gl_Position = vec4(1.0);
})";

    constexpr char kFS[] =
        R"(precision highp float;
void main()
{
    gl_FragColor = vec4(1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glUseProgram(program);

    const auto &vertices = GetQuadVertices();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4, vertices.data());
    glEnableVertexAttribArray(0);

    ASSERT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that passing a null pixel data pointer to TexSubImage calls generates an INVALID_VALUE error
TEST_P(WebGLCompatibilityTest, NullPixelDataForSubImage)
{
    // glTexSubImage2D
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);

        // TexImage with null data - OK
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        EXPECT_GL_NO_ERROR();

        // TexSubImage with zero size and null data - OK
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        EXPECT_GL_NO_ERROR();

        // TexSubImage with non-zero size and null data - Invalid value
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        EXPECT_GL_ERROR(GL_INVALID_VALUE);
    }

    // glTexSubImage3D
    if (getClientMajorVersion() >= 3)
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_3D, texture);

        // TexImage with null data - OK
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        EXPECT_GL_NO_ERROR();

        // TexSubImage with zero size and null data - OK
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        EXPECT_GL_NO_ERROR();

        // TexSubImage with non-zero size and null data - Invalid value
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        EXPECT_GL_ERROR(GL_INVALID_VALUE);
    }
}

// Tests the WebGL requirement of having the same stencil mask, writemask and ref for front and back
// (when stencil testing is enabled)
void WebGLCompatibilityTest::TestDifferentStencilMaskAndRef(GLenum errIfMismatch)
{
    // Run the test in an FBO to make sure we have some stencil bits.
    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 32, 32);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffer);

    ANGLE_GL_PROGRAM(program, "void main() { gl_Position = vec4(0, 0, 0, 1); }",
                     "void main() { gl_FragColor = vec4(0, 1, 0, 1); }");
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    // Having ref and mask the same for front and back is valid.
    glStencilMask(255);
    glStencilFunc(GL_ALWAYS, 0, 255);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Having a different front - back write mask generates an error.
    glStencilMaskSeparate(GL_FRONT, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(errIfMismatch);

    // Setting both write masks separately to the same value is valid.
    glStencilMaskSeparate(GL_BACK, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Having a different stencil front - back mask generates an error
    glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, 0, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(errIfMismatch);

    // Setting both masks separately to the same value is valid.
    glStencilFuncSeparate(GL_BACK, GL_ALWAYS, 0, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Having a different stencil front - back reference generates an error
    glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, 255, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(errIfMismatch);

    // Setting both references separately to the same value is valid.
    glStencilFuncSeparate(GL_BACK, GL_ALWAYS, 255, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Using different stencil funcs, everything being equal is valid.
    glStencilFuncSeparate(GL_BACK, GL_NEVER, 255, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
}

TEST_P(WebGLCompatibilityTest, StencilTestEnabledDisallowsDifferentStencilMaskAndRef)
{
    glEnable(GL_STENCIL_TEST);
    TestDifferentStencilMaskAndRef(GL_INVALID_OPERATION);
}

TEST_P(WebGLCompatibilityTest, StencilTestDisabledAllowsDifferentStencilMaskAndRef)
{
    glDisable(GL_STENCIL_TEST);
    TestDifferentStencilMaskAndRef(GL_NO_ERROR);
}

// Test that GL_FIXED is forbidden
TEST_P(WebGLCompatibilityTest, ForbidsGLFixed)
{
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 16, nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    ASSERT_GL_NO_ERROR();

    glVertexAttribPointer(0, 1, GL_FIXED, GL_FALSE, 0, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Test the WebGL limit of 255 for the attribute stride
TEST_P(WebGLCompatibilityTest, MaxStride)
{
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 1024, nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 255, nullptr);
    ASSERT_GL_NO_ERROR();

    glVertexAttribPointer(0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 256, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test the checks for OOB reads in the vertex buffers, non-instanced version
TEST_P(WebGLCompatibilityTest, DrawArraysBufferOutOfBoundsNonInstanced)
{
    constexpr char kVS[] =
        R"(attribute float a_pos;
void main()
{
    gl_Position = vec4(a_pos, a_pos, a_pos, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    GLint posLocation = glGetAttribLocation(program, "a_pos");
    ASSERT_NE(-1, posLocation);
    glUseProgram(program);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 16, nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(posLocation);

    // Test touching the last element is valid.
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(12));
    glDrawArrays(GL_POINTS, 0, 4);
    ASSERT_GL_NO_ERROR();

    // Test touching the last element + 1 is invalid.
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(13));
    glDrawArrays(GL_POINTS, 0, 4);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Test touching the last element is valid, using a stride.
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 2,
                          reinterpret_cast<const void *>(9));
    glDrawArrays(GL_POINTS, 0, 4);
    ASSERT_GL_NO_ERROR();

    // Test touching the last element + 1 is invalid, using a stride.
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 2,
                          reinterpret_cast<const void *>(10));
    glDrawArrays(GL_POINTS, 0, 4);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Test any offset is valid if no vertices are drawn.
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(32));
    glDrawArrays(GL_POINTS, 0, 0);
    ASSERT_GL_NO_ERROR();

    // Test a case of overflow that could give a max vertex that's negative
    constexpr GLint kIntMax = std::numeric_limits<GLint>::max();
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_POINTS, kIntMax, kIntMax);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that index values outside of the 32-bit integer range do not read out of bounds
TEST_P(WebGLCompatibilityTest, LargeIndexRange)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_element_index_uint"));

    constexpr char kVS[] =
        R"(attribute vec4 a_Position;
void main()
{
    gl_Position = a_Position;
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    glUseProgram(program);

    glEnableVertexAttribArray(glGetAttribLocation(program, "a_Position"));

    constexpr float kVertexData[] = {
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertexData), kVertexData, GL_STREAM_DRAW);

    constexpr GLuint kMaxIntAsGLuint = static_cast<GLuint>(std::numeric_limits<GLint>::max());
    constexpr GLuint kIndexData[]    = {
        kMaxIntAsGLuint,
        kMaxIntAsGLuint + 1,
        kMaxIntAsGLuint + 2,
        kMaxIntAsGLuint + 3,
    };

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndexData), kIndexData, GL_DYNAMIC_DRAW);

    EXPECT_GL_NO_ERROR();

    // First index is representable as 32-bit int but second is not
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Neither index is representable as 32-bit int
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, reinterpret_cast<void *>(sizeof(GLuint) * 2));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test for drawing with a null index buffer
TEST_P(WebGLCompatibilityTest, NullIndexBuffer)
{
    constexpr char kVS[] =
        R"(attribute float a_pos;
void main()
{
    gl_Position = vec4(a_pos, a_pos, a_pos, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    glUseProgram(program);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_TRIANGLES, 0, GL_UNSIGNED_BYTE, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test the checks for OOB reads in the vertex buffers, instanced version
TEST_P(WebGL2CompatibilityTest, DrawArraysBufferOutOfBoundsInstanced)
{
    constexpr char kVS[] =
        R"(attribute float a_pos;
attribute float a_w;
void main()
{
    gl_Position = vec4(a_pos, a_pos, a_pos, a_w);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    GLint posLocation = glGetAttribLocation(program, "a_pos");
    GLint wLocation   = glGetAttribLocation(program, "a_w");
    ASSERT_NE(-1, posLocation);
    ASSERT_NE(-1, wLocation);
    glUseProgram(program);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 16, nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(posLocation);
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
    glVertexAttribDivisor(posLocation, 0);

    glEnableVertexAttribArray(wLocation);
    glVertexAttribDivisor(wLocation, 1);

    // Test touching the last element is valid.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(12));
    glDrawArraysInstanced(GL_POINTS, 0, 1, 4);
    ASSERT_GL_NO_ERROR();

    // Test touching the last element + 1 is invalid.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(13));
    glDrawArraysInstanced(GL_POINTS, 0, 1, 4);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Test touching the last element is valid, using a stride.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 2,
                          reinterpret_cast<const void *>(9));
    glDrawArraysInstanced(GL_POINTS, 0, 1, 4);
    ASSERT_GL_NO_ERROR();

    // Test touching the last element + 1 is invalid, using a stride.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 2,
                          reinterpret_cast<const void *>(10));
    glDrawArraysInstanced(GL_POINTS, 0, 1, 4);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Test any offset is valid if no vertices are drawn.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(32));
    glDrawArraysInstanced(GL_POINTS, 0, 0, 1);
    ASSERT_GL_NO_ERROR();

    // Test any offset is valid if no primitives are drawn.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(32));
    glDrawArraysInstanced(GL_POINTS, 0, 1, 0);
    ASSERT_GL_NO_ERROR();
}

// Test the checks for OOB reads in the vertex buffers, ANGLE_instanced_arrays version
TEST_P(WebGLCompatibilityTest, DrawArraysBufferOutOfBoundsInstancedANGLE)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionRequestable("GL_ANGLE_instanced_arrays"));
    glRequestExtensionANGLE("GL_ANGLE_instanced_arrays");
    EXPECT_GL_NO_ERROR();

    constexpr char kVS[] =
        R"(attribute float a_pos;
attribute float a_w;
void main()
{
    gl_Position = vec4(a_pos, a_pos, a_pos, a_w);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    GLint posLocation = glGetAttribLocation(program, "a_pos");
    GLint wLocation   = glGetAttribLocation(program, "a_w");
    ASSERT_NE(-1, posLocation);
    ASSERT_NE(-1, wLocation);
    glUseProgram(program);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 16, nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(posLocation);
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
    glVertexAttribDivisorANGLE(posLocation, 0);

    glEnableVertexAttribArray(wLocation);
    glVertexAttribDivisorANGLE(wLocation, 1);

    // Test touching the last element is valid.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(12));
    glDrawArraysInstancedANGLE(GL_POINTS, 0, 1, 4);
    ASSERT_GL_NO_ERROR() << "touching the last element.";

    // Test touching the last element + 1 is invalid.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(13));
    glDrawArraysInstancedANGLE(GL_POINTS, 0, 1, 4);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "touching the last element + 1.";

    // Test touching the last element is valid, using a stride.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 2,
                          reinterpret_cast<const void *>(9));
    glDrawArraysInstancedANGLE(GL_POINTS, 0, 1, 4);
    ASSERT_GL_NO_ERROR() << "touching the last element using a stride.";

    // Test touching the last element + 1 is invalid, using a stride.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 2,
                          reinterpret_cast<const void *>(10));
    glDrawArraysInstancedANGLE(GL_POINTS, 0, 1, 4);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "touching the last element + 1 using a stride.";

    // Test any offset is valid if no vertices are drawn.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(32));
    glDrawArraysInstancedANGLE(GL_POINTS, 0, 0, 1);
    ASSERT_GL_NO_ERROR() << "any offset with no vertices.";

    // Test any offset is valid if no primitives are drawn.
    glVertexAttribPointer(wLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(32));
    glDrawArraysInstancedANGLE(GL_POINTS, 0, 1, 0);
    ASSERT_GL_NO_ERROR() << "any offset with primitives.";
}

// Test the checks for OOB reads in the index buffer
TEST_P(WebGLCompatibilityTest, DrawElementsBufferOutOfBoundsInIndexBuffer)
{
    constexpr char kVS[] =
        R"(attribute float a_pos;
void main()
{
    gl_Position = vec4(a_pos, a_pos, a_pos, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    GLint posLocation = glGetAttribLocation(program, "a_pos");
    ASSERT_NE(-1, posLocation);
    glUseProgram(program);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 16, nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(posLocation);
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, nullptr);

    const uint8_t zeroIndices[] = {0, 0, 0, 0, 0, 0, 0, 0};

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(zeroIndices), zeroIndices, GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // Test touching the last index is valid
    glDrawElements(GL_POINTS, 4, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(4));
    ASSERT_GL_NO_ERROR();

    // Test touching the last + 1 element is invalid
    glDrawElements(GL_POINTS, 4, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(5));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Test any offset if valid if count is zero
    glDrawElements(GL_POINTS, 0, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(42));
    ASSERT_GL_NO_ERROR();

    // Test touching the first index is valid
    glDrawElements(GL_POINTS, 4, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(4));
    ASSERT_GL_NO_ERROR();

    // Test touching the first - 1 index is invalid
    // The error ha been specified to be INVALID_VALUE instead of INVALID_OPERATION because it was
    // the historic behavior of WebGL implementations
    glDrawElements(GL_POINTS, 4, GL_UNSIGNED_BYTE,
                   reinterpret_cast<const void *>(static_cast<ptrdiff_t>(0) - 1));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test the checks for OOB in vertex buffers caused by indices, non-instanced version
TEST_P(WebGLCompatibilityTest, DrawElementsBufferOutOfBoundsInVertexBuffer)
{
    constexpr char kVS[] =
        R"(attribute float a_pos;
void main()
{
    gl_Position = vec4(a_pos, a_pos, a_pos, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    GLint posLocation = glGetAttribLocation(program, "a_pos");
    ASSERT_NE(-1, posLocation);
    glUseProgram(program);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8, nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(posLocation);
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, nullptr);

    const uint8_t testIndices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 255};

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(testIndices), testIndices, GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // Test touching the end of the vertex buffer is valid
    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(7));
    ASSERT_GL_NO_ERROR();

    // Test touching just after the end of the vertex buffer is invalid
    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(8));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Test touching the whole vertex buffer is valid
    glDrawElements(GL_POINTS, 8, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    // Test an index that would be negative
    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(9));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test depth range with 'near' more or less than 'far.'
TEST_P(WebGLCompatibilityTest, DepthRange)
{
    glDepthRangef(0, 1);
    ASSERT_GL_NO_ERROR();

    glDepthRangef(.5, .5);
    ASSERT_GL_NO_ERROR();

    glDepthRangef(1, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test all blend function combinations.
// In WebGL it is invalid to combine constant color with constant alpha.
TEST_P(WebGLCompatibilityTest, BlendWithConstantColor)
{
    constexpr GLenum srcFunc[] = {
        GL_ZERO,
        GL_ONE,
        GL_SRC_COLOR,
        GL_ONE_MINUS_SRC_COLOR,
        GL_DST_COLOR,
        GL_ONE_MINUS_DST_COLOR,
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
        GL_DST_ALPHA,
        GL_ONE_MINUS_DST_ALPHA,
        GL_CONSTANT_COLOR,
        GL_ONE_MINUS_CONSTANT_COLOR,
        GL_CONSTANT_ALPHA,
        GL_ONE_MINUS_CONSTANT_ALPHA,
        GL_SRC_ALPHA_SATURATE,
    };

    constexpr GLenum dstFunc[] = {
        GL_ZERO,           GL_ONE,
        GL_SRC_COLOR,      GL_ONE_MINUS_SRC_COLOR,
        GL_DST_COLOR,      GL_ONE_MINUS_DST_COLOR,
        GL_SRC_ALPHA,      GL_ONE_MINUS_SRC_ALPHA,
        GL_DST_ALPHA,      GL_ONE_MINUS_DST_ALPHA,
        GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
        GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA,
    };

    for (GLenum src : srcFunc)
    {
        for (GLenum dst : dstFunc)
        {
            glBlendFunc(src, dst);
            CheckBlendFunctions(src, dst);
            glBlendFuncSeparate(src, dst, GL_ONE, GL_ONE);
            CheckBlendFunctions(src, dst);
        }
    }

    // Ensure the same semantics for indexed blendFunc
    if (IsGLExtensionRequestable("GL_OES_draw_buffers_indexed"))
    {
        glRequestExtensionANGLE("GL_OES_draw_buffers_indexed");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_OES_draw_buffers_indexed"));

        for (GLenum src : srcFunc)
        {
            for (GLenum dst : dstFunc)
            {
                glBlendFunciOES(0, src, dst);
                CheckBlendFunctions(src, dst);
                glBlendFuncSeparateiOES(0, src, dst, GL_ONE, GL_ONE);
                CheckBlendFunctions(src, dst);
            }
        }
    }
}

// Test draw state validation and invalidation wrt indexed blendFunc.
TEST_P(WebGLCompatibilityTest, IndexedBlendWithConstantColorInvalidation)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionRequestable("GL_OES_draw_buffers_indexed"));

    glRequestExtensionANGLE("GL_OES_draw_buffers_indexed");
    EXPECT_GL_NO_ERROR();
    EXPECT_TRUE(IsGLExtensionEnabled("GL_OES_draw_buffers_indexed"));

    constexpr char kVS[] =
        R"(#version 300 es
void main()
{
    gl_PointSize = 1.0;
    gl_Position = vec4(0, 0, 0, 1);
})";

    constexpr char kFS[] =
        R"(#version 300 es
precision lowp float;
layout(location = 0) out vec4 o_color0;
layout(location = 1) out vec4 o_color1;
void main()
{
    o_color0 = vec4(1, 0, 0, 1);
    o_color1 = vec4(0, 1, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    glDisable(GL_BLEND);
    glEnableiOES(GL_BLEND, 0);
    glEnableiOES(GL_BLEND, 1);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture2, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLenum drawbuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawbuffers);

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    // Force-invalidate draw call
    glBlendFuncSeparateiOES(0, GL_CONSTANT_COLOR, GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA,
                            GL_CONSTANT_ALPHA);
    EXPECT_GL_NO_ERROR();

    glBlendFuncSeparateiOES(1, GL_CONSTANT_ALPHA, GL_CONSTANT_ALPHA, GL_CONSTANT_COLOR,
                            GL_CONSTANT_COLOR);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test getIndexedParameter wrt GL_OES_draw_buffers_indexed.
TEST_P(WebGLCompatibilityTest, DrawBuffersIndexedGetIndexedParameter)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsGLExtensionRequestable("GL_OES_draw_buffers_indexed"));

    GLint value;
    GLboolean data[4];

    glGetIntegeri_v(GL_BLEND_EQUATION_RGB, 0, &value);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glGetIntegeri_v(GL_BLEND_EQUATION_ALPHA, 0, &value);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glGetIntegeri_v(GL_BLEND_SRC_RGB, 0, &value);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glGetIntegeri_v(GL_BLEND_SRC_ALPHA, 0, &value);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glGetIntegeri_v(GL_BLEND_DST_RGB, 0, &value);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glGetIntegeri_v(GL_BLEND_DST_ALPHA, 0, &value);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glGetBooleani_v(GL_COLOR_WRITEMASK, 0, data);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glRequestExtensionANGLE("GL_OES_draw_buffers_indexed");
    EXPECT_GL_NO_ERROR();
    EXPECT_TRUE(IsGLExtensionEnabled("GL_OES_draw_buffers_indexed"));

    glDisable(GL_BLEND);
    glEnableiOES(GL_BLEND, 0);
    glBlendEquationSeparateiOES(0, GL_FUNC_ADD, GL_FUNC_SUBTRACT);
    glBlendFuncSeparateiOES(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ZERO);
    glColorMaskiOES(0, true, false, true, false);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(true, glIsEnablediOES(GL_BLEND, 0));
    EXPECT_GL_NO_ERROR();
    glGetIntegeri_v(GL_BLEND_EQUATION_RGB, 0, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_FUNC_ADD, value);
    glGetIntegeri_v(GL_BLEND_EQUATION_ALPHA, 0, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_FUNC_SUBTRACT, value);
    glGetIntegeri_v(GL_BLEND_SRC_RGB, 0, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_SRC_ALPHA, value);
    glGetIntegeri_v(GL_BLEND_SRC_ALPHA, 0, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_ZERO, value);
    glGetIntegeri_v(GL_BLEND_DST_RGB, 0, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_ONE_MINUS_SRC_ALPHA, value);
    glGetIntegeri_v(GL_BLEND_DST_ALPHA, 0, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_ZERO, value);
    glGetBooleani_v(GL_COLOR_WRITEMASK, 0, data);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(true, data[0]);
    EXPECT_EQ(false, data[1]);
    EXPECT_EQ(true, data[2]);
    EXPECT_EQ(false, data[3]);
}

// Test that binding/querying uniforms and attributes with invalid names generates errors
TEST_P(WebGLCompatibilityTest, InvalidAttributeAndUniformNames)
{
    const std::string validAttribName =
        "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    const std::string validUniformName =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_1234567890";
    std::vector<char> invalidSet = {'"', '$', '`', '@', '\''};
    if (getClientMajorVersion() < 3)
    {
        invalidSet.push_back('\\');
    }

    std::string vert = "attribute float ";
    vert += validAttribName;
    vert +=
        R"(;
void main()
{
    gl_Position = vec4(1.0);
})";

    std::string frag =
        R"(precision highp float;
uniform vec4 )";
    frag += validUniformName;
    // Insert illegal characters into comments
    frag +=
        R"(;
    // $ \" @ /*
void main()
{/*
    ` @ $
    */gl_FragColor = vec4(1.0);
})";

    ANGLE_GL_PROGRAM(program, vert.c_str(), frag.c_str());
    EXPECT_GL_NO_ERROR();

    for (char invalidChar : invalidSet)
    {
        std::string invalidName = validAttribName + invalidChar;
        glGetAttribLocation(program, invalidName.c_str());
        EXPECT_GL_ERROR(GL_INVALID_VALUE)
            << "glGetAttribLocation unexpectedly succeeded for name \"" << invalidName << "\".";

        glBindAttribLocation(program, 0, invalidName.c_str());
        EXPECT_GL_ERROR(GL_INVALID_VALUE)
            << "glBindAttribLocation unexpectedly succeeded for name \"" << invalidName << "\".";
    }

    for (char invalidChar : invalidSet)
    {
        std::string invalidName = validUniformName + invalidChar;
        glGetUniformLocation(program, invalidName.c_str());
        EXPECT_GL_ERROR(GL_INVALID_VALUE)
            << "glGetUniformLocation unexpectedly succeeded for name \"" << invalidName << "\".";
    }

    for (char invalidChar : invalidSet)
    {
        std::string invalidAttribName = validAttribName + invalidChar;
        std::string invalidVert       = "attribute float ";
        invalidVert += invalidAttribName;
        invalidVert += R"(;,
void main(),
{,
    gl_Position = vec4(1.0);,
})";
        GLuint program_number = CompileProgram(invalidVert.c_str(), essl1_shaders::fs::Red());
        EXPECT_EQ(0u, program_number);
    }
}

// Test that line continuation is handled correctly when validating shader source
TEST_P(WebGLCompatibilityTest, ShaderSourceLineContinuation)
{
    // With recent changes to WebGL's shader source validation in
    // https://github.com/KhronosGroup/WebGL/pull/3206 and follow-ons,
    // the backslash character can be used in both WebGL 1.0 and 2.0
    // contexts.

    const char *validVert =
        R"(#define foo this \
    is a test
precision mediump float;
void main()
{
    gl_Position = vec4(1.0);
})";

    GLuint program = CompileProgram(validVert, essl1_shaders::fs::Red());
    EXPECT_NE(0u, program);
    glDeleteProgram(program);
}

// Test that line continuation is handled correctly when valdiating shader source
TEST_P(WebGL2CompatibilityTest, ShaderSourceLineContinuation)
{
    const char *validVert =
        R"(#version 300 es
precision mediump float;

void main ()
{
    float f\
oo = 1.0;
    gl_Position = vec4(foo);
})";

    const char *invalidVert =
        R"(#version 300 es
precision mediump float;

void main ()
{
    float f\$
oo = 1.0;
    gl_Position = vec4(foo);
})";

    GLuint program = CompileProgram(validVert, essl3_shaders::fs::Red());
    EXPECT_NE(0u, program);
    glDeleteProgram(program);

    program = CompileProgram(invalidVert, essl3_shaders::fs::Red());
    EXPECT_EQ(0u, program);
}

// Tests bindAttribLocations for reserved prefixes and length limits
TEST_P(WebGLCompatibilityTest, BindAttribLocationLimitation)
{
    constexpr int maxLocStringLength = 256;
    const std::string tooLongString(maxLocStringLength + 1, '_');

    glBindAttribLocation(0, 0, "_webgl_var");

    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBindAttribLocation(0, 0, static_cast<const GLchar *>(tooLongString.c_str()));

    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Tests getAttribLocation for reserved prefixes
TEST_P(WebGLCompatibilityTest, GetAttribLocationNameLimitation)
{
    GLint attrLocation;

    attrLocation = glGetAttribLocation(0, "gl_attr");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(-1, attrLocation);

    attrLocation = glGetAttribLocation(0, "webgl_attr");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(-1, attrLocation);

    attrLocation = glGetAttribLocation(0, "_webgl_attr");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(-1, attrLocation);
}

// Tests getAttribLocation for length limits
TEST_P(WebGLCompatibilityTest, GetAttribLocationLengthLimitation)
{
    constexpr int maxLocStringLength = 256;
    const std::string tooLongString(maxLocStringLength + 1, '_');

    glGetAttribLocation(0, static_cast<const GLchar *>(tooLongString.c_str()));

    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that having no attributes with a zero divisor is valid in WebGL2
TEST_P(WebGL2CompatibilityTest, InstancedDrawZeroDivisor)
{
    constexpr char kVS[] =
        R"(attribute float a_pos;
void main()
{
    gl_Position = vec4(a_pos, a_pos, a_pos, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());

    GLint posLocation = glGetAttribLocation(program, "a_pos");
    ASSERT_NE(-1, posLocation);

    glUseProgram(program);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 16, nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(posLocation);
    glVertexAttribDivisor(posLocation, 1);

    glVertexAttribPointer(0, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, nullptr);
    glDrawArraysInstanced(GL_POINTS, 0, 1, 4);
    ASSERT_GL_NO_ERROR();
}

// Tests that NPOT is not enabled by default in WebGL 1 and that it can be enabled
TEST_P(WebGLCompatibilityTest, NPOT)
{
    EXPECT_FALSE(IsGLExtensionEnabled("GL_OES_texture_npot"));

    // Create a texture and set an NPOT mip 0, should always be acceptable.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 10, 10, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    // Try setting an NPOT mip 1 and verify the error if WebGL 1
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 5, 5, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    if (getClientMajorVersion() < 3)
    {
        ASSERT_GL_ERROR(GL_INVALID_VALUE);
    }
    else
    {
        ASSERT_GL_NO_ERROR();
    }

    if (IsGLExtensionRequestable("GL_OES_texture_npot"))
    {
        glRequestExtensionANGLE("GL_OES_texture_npot");
        ASSERT_GL_NO_ERROR();

        // Try again to set NPOT mip 1
        glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 5, 5, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        ASSERT_GL_NO_ERROR();
    }
}

template <typename T>
void FillTexture2D(GLuint texture,
                   GLsizei width,
                   GLsizei height,
                   const T &onePixelData,
                   GLint level,
                   GLint internalFormat,
                   GLenum format,
                   GLenum type)
{
    std::vector<T> allPixelsData(width * height, onePixelData);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, format, type,
                 allPixelsData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// Test that unset gl_Position defaults to (0,0,0,0).
TEST_P(WebGLCompatibilityTest, DefaultPosition)
{
    // Draw a quad where each vertex is red if gl_Position is (0,0,0,0) before it is set,
    // and green otherwise.  The center of each quadrant will be red if and only if all
    // four corners are red.
    constexpr char kVS[] =
        R"(attribute vec3 pos;
varying vec4 color;
void main() {
    if (gl_Position == vec4(0,0,0,0)) {
        color = vec4(1,0,0,1);
    } else {
        color = vec4(0,1,0,1);
    }
    gl_Position = vec4(pos,1);
})";

    constexpr char kFS[] =
        R"(precision mediump float;
varying vec4 color;
void main() {
    gl_FragColor = color;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "pos", 0.0f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() * 1 / 4, getWindowHeight() * 1 / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() * 1 / 4, getWindowHeight() * 3 / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() * 3 / 4, getWindowHeight() * 1 / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() * 3 / 4, getWindowHeight() * 3 / 4, GLColor::red);
}

// Tests that a rendering feedback loop triggers a GL error under WebGL.
// Based on WebGL test conformance/renderbuffers/feedback-loop.html.
TEST_P(WebGLCompatibilityTest, RenderingFeedbackLoop)
{
    constexpr char kVS[] =
        R"(attribute vec4 a_position;
varying vec2 v_texCoord;
void main() {
    gl_Position = a_position;
    v_texCoord = (a_position.xy * 0.5) + 0.5;
})";

    constexpr char kFS[] =
        R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D u_texture;
void main() {
    // Shader swizzles color channels so we can tell if the draw succeeded.
    gl_FragColor = texture2D(u_texture, v_texCoord).gbra;
})";

    GLTexture texture;
    FillTexture2D(texture, 1, 1, GLColor::red, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint uniformLoc = glGetUniformLocation(program, "u_texture");
    ASSERT_NE(-1, uniformLoc);

    glUseProgram(program);
    glUniform1i(uniformLoc, 0);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    ASSERT_GL_NO_ERROR();

    // Drawing with a texture that is also bound to the current framebuffer should fail
    glBindTexture(GL_TEXTURE_2D, texture);
    drawQuad(program, "a_position", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Ensure that the texture contents did not change after the previous render
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    drawQuad(program, "a_position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Drawing when texture is bound to an inactive uniform should succeed
    GLTexture texture2;
    FillTexture2D(texture2, 1, 1, GLColor::green, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);
    drawQuad(program, "a_position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Multi-context uses of textures should not cause rendering feedback loops.
TEST_P(WebGLCompatibilityTest, MultiContextNoRenderingFeedbackLoops)
{
    constexpr char kUnusedTextureVS[] =
        R"(attribute vec4 a_position;
varying vec2 v_texCoord;
void main() {
    gl_Position = a_position;
    v_texCoord = (a_position.xy * 0.5) + 0.5;
})";

    constexpr char kUnusedTextureFS[] =
        R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D u_texture;
void main() {
    gl_FragColor = texture2D(u_texture, v_texCoord).rgba;
})";

    ANGLE_GL_PROGRAM(unusedProgram, kUnusedTextureVS, kUnusedTextureFS);

    glUseProgram(unusedProgram);
    GLint uniformLoc = glGetUniformLocation(unusedProgram, "u_texture");
    ASSERT_NE(-1, uniformLoc);
    glUniform1i(uniformLoc, 0);

    GLTexture texture;
    FillTexture2D(texture, 1, 1, GLColor::red, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Note that _texture_ is still bound to GL_TEXTURE_2D in this context at this point.

    EGLWindow *window          = getEGLWindow();
    EGLDisplay display         = window->getDisplay();
    EGLConfig config           = window->getConfig();
    EGLSurface surface         = window->getSurface();
    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR,
        GetParam().majorVersion,
        EGL_CONTEXT_MINOR_VERSION_KHR,
        GetParam().minorVersion,
        EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE,
        EGL_TRUE,
        EGL_NONE,
    };
    auto context1 = eglGetCurrentContext();
    // Create context2, sharing resources with context1.
    auto context2 = eglCreateContext(display, config, context1, contextAttributes);
    ASSERT_NE(context2, EGL_NO_CONTEXT);
    eglMakeCurrent(display, surface, surface, context2);

    constexpr char kVS[] =
        R"(attribute vec4 a_position;
void main() {
    gl_Position = a_position;
})";

    constexpr char kFS[] =
        R"(precision mediump float;
void main() {
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    // Render to the texture in context2.
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // Texture is still a valid name in context2.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    // There is no rendering feedback loop at this point.

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "a_position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    eglMakeCurrent(display, surface, surface, context1);
    eglDestroyContext(display, context2);
}

// Test for the max draw buffers and color attachments.
TEST_P(WebGLCompatibilityTest, MaxDrawBuffersAttachmentPoints)
{
    // This test only applies to ES2.
    if (getClientMajorVersion() != 2)
    {
        return;
    }

    GLFramebuffer fbo[2];
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);

    // Test that is valid when we bind with a single attachment point.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_NO_ERROR();

    // Test that enabling the draw buffers extension will allow us to bind with a non-zero
    // attachment point.
    if (IsGLExtensionRequestable("GL_EXT_draw_buffers"))
    {
        glRequestExtensionANGLE("GL_EXT_draw_buffers");
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(IsGLExtensionEnabled("GL_EXT_draw_buffers"));

        glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);

        GLTexture texture2;
        glBindTexture(GL_TEXTURE_2D, texture2);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture2, 0);
        ASSERT_GL_NO_ERROR();
    }
}

// Test that the offset in the index buffer is forced to be a multiple of the element size
TEST_P(WebGLCompatibilityTest, DrawElementsOffsetRestriction)
{
    constexpr char kVS[] =
        R"(attribute vec3 a_pos;
void main()
{
    gl_Position = vec4(a_pos, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());

    GLint posLocation = glGetAttribLocation(program, "a_pos");
    ASSERT_NE(-1, posLocation);
    glUseProgram(program);

    const auto &vertices = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);

    GLBuffer indexBuffer;
    const GLubyte indices[] = {0, 0, 0, 0, 0, 0, 0, 0};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    ASSERT_GL_NO_ERROR();

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    glDrawElements(GL_TRIANGLES, 4, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    glDrawElements(GL_TRIANGLES, 4, GL_UNSIGNED_SHORT, reinterpret_cast<const void *>(1));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that the offset and stride in the vertex buffer is forced to be a multiple of the element
// size
TEST_P(WebGLCompatibilityTest, VertexAttribPointerOffsetRestriction)
{
    // Base case, vector of two floats
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    ASSERT_GL_NO_ERROR();

    // Test setting a non-multiple offset
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void *>(1));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void *>(2));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void *>(3));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Test setting a non-multiple stride
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 1, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

void WebGLCompatibilityTest::drawBuffersEXTFeedbackLoop(GLuint program,
                                                        const std::array<GLenum, 2> &drawBuffers,
                                                        GLenum expectedError)
{
    glDrawBuffersEXT(2, drawBuffers.data());

    // Make sure framebuffer is complete before feedback loop detection
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    drawQuad(program, "aPosition", 0.5f, 1.0f, true);

    // "Rendering to a texture where it samples from should geneates INVALID_OPERATION. Otherwise,
    // it should be NO_ERROR"
    EXPECT_GL_ERROR(expectedError);
}

// This tests that rendering feedback loops works as expected with GL_EXT_draw_buffers.
// Based on WebGL test conformance/extensions/webgl-draw-buffers-feedback-loop.html
TEST_P(WebGLCompatibilityTest, RenderingFeedbackLoopWithDrawBuffersEXT)
{
    constexpr char kVS[] =
        R"(attribute vec4 aPosition;
varying vec2 texCoord;
void main() {
    gl_Position = aPosition;
    texCoord = (aPosition.xy * 0.5) + 0.5;
})";

    constexpr char kFS[] =
        R"(#extension GL_EXT_draw_buffers : require
precision mediump float;
uniform sampler2D tex;
varying vec2 texCoord;
void main() {
    gl_FragData[0] = texture2D(tex, texCoord);
    gl_FragData[1] = texture2D(tex, texCoord);
})";

    GLsizei width  = 8;
    GLsizei height = 8;

    // This shader cannot be run in ES3, because WebGL 2 does not expose the draw buffers
    // extension and gl_FragData semantics are changed to enforce indexing by zero always.
    // TODO(jmadill): This extension should be disabled in WebGL 2 contexts.
    if (/*!IsGLExtensionEnabled("GL_EXT_draw_buffers")*/ getClientMajorVersion() != 2)
    {
        // No WEBGL_draw_buffers support -- this is legal.
        return;
    }

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

    // Test skipped because MAX_DRAW_BUFFERS is too small.
    ANGLE_SKIP_TEST_IF(maxDrawBuffers < 2);

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glViewport(0, 0, width, height);

    GLTexture tex0;
    GLTexture tex1;
    GLFramebuffer fbo;
    FillTexture2D(tex0, width, height, GLColor::red, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    FillTexture2D(tex1, width, height, GLColor::green, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    ASSERT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, tex1);
    GLint texLoc = glGetUniformLocation(program, "tex");
    ASSERT_NE(-1, texLoc);
    glUniform1i(texLoc, 0);
    ASSERT_GL_NO_ERROR();

    // The sampling texture is bound to COLOR_ATTACHMENT1 during resource allocation
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex1, 0);

    drawBuffersEXTFeedbackLoop(program, {{GL_NONE, GL_COLOR_ATTACHMENT1}}, GL_INVALID_OPERATION);
    drawBuffersEXTFeedbackLoop(program, {{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1}},
                               GL_INVALID_OPERATION);
    // A feedback loop is formed regardless of drawBuffers settings.
    drawBuffersEXTFeedbackLoop(program, {{GL_COLOR_ATTACHMENT0, GL_NONE}}, GL_INVALID_OPERATION);
}

// Test tests that texture copying feedback loops are properly rejected in WebGL.
// Based on the WebGL test conformance/textures/misc/texture-copying-feedback-loops.html
TEST_P(WebGLCompatibilityTest, TextureCopyingFeedbackLoops)
{
    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // framebuffer should be FRAMEBUFFER_COMPLETE.
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    // testing copyTexImage2D

    // copyTexImage2D to same texture but different level
    glBindTexture(GL_TEXTURE_2D, texture);
    glCopyTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 0, 0, 2, 2, 0);
    EXPECT_GL_NO_ERROR();

    // copyTexImage2D to same texture same level, invalid feedback loop
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 2, 2, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // copyTexImage2D to different texture
    glBindTexture(GL_TEXTURE_2D, texture2);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 2, 2, 0);
    EXPECT_GL_NO_ERROR();

    // testing copyTexSubImage2D

    // copyTexSubImage2D to same texture but different level
    glBindTexture(GL_TEXTURE_2D, texture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 0, 0, 1, 1);
    EXPECT_GL_NO_ERROR();

    // copyTexSubImage2D to same texture same level, invalid feedback loop
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // copyTexSubImage2D to different texture
    glBindTexture(GL_TEXTURE_2D, texture2);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
    EXPECT_GL_NO_ERROR();
}

// Test that copying from mip 1 of a texture to mip 0 works.  When the framebuffer is attached to
// mip 1 of a mip-complete texture, an image with both mips are created.  When copying from the
// framebuffer to mip 0, it is being redefined.
TEST_P(WebGL2CompatibilityTest, CopyMip1ToMip0)
{
    // http://anglebug.com/42263391
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // http://anglebug.com/42263392
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && (IsWindows() || IsMac()));

    // TODO(anglebug.com/40096747): Failing on ARM64-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    const GLColor mip0[4] = {
        GLColor::red,
        GLColor::red,
        GLColor::red,
        GLColor::red,
    };
    const GLColor mip1[1] = {
        GLColor::green,
    };

    // Create a complete mip chain in mips 0 to 2
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1);

    // Framebuffer can bind to mip 1, as the texture is mip-complete.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 1);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Copy to mip 0.  This shouldn't crash.
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
    EXPECT_GL_NO_ERROR();

    // The framebuffer is now incomplete.
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // http://anglebug.com/42263389
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsNVIDIA());

    // http://anglebug.com/42263390
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsAMD() && IsMac());

    // Bind framebuffer to mip 0 and make sure the copy was done.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that copying from mip 0 of a texture to mip 1 works.  When the framebuffer is attached to
// mip 0 of a mip-complete texture, an image with both mips are created.  When copying from the
// framebuffer to mip 1, it is being redefined.
TEST_P(WebGL2CompatibilityTest, CopyMip0ToMip1)
{
    // http://anglebug.com/42263392
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsWindows());

    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsAMD() && IsWindows());

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    const GLColor mip0[4] = {
        GLColor::red,
        GLColor::red,
        GLColor::red,
        GLColor::red,
    };
    const GLColor mip1[1] = {
        GLColor::green,
    };

    // Create a complete mip chain in mips 0 to 2
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1);

    // Framebuffer can bind to mip 0, as the texture is mip-complete.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Copy to mip 1.  This shouldn't crash.
    glCopyTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 0, 0, 2, 2, 0);
    EXPECT_GL_NO_ERROR();

    // The framebuffer is still complete.
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    // Make sure mip 0 is untouched.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);

    // Bind framebuffer to mip 1 and make sure the copy was done.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 1);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
}

void WebGLCompatibilityTest::drawBuffersFeedbackLoop(GLuint program,
                                                     const std::array<GLenum, 2> &drawBuffers,
                                                     GLenum expectedError)
{
    glDrawBuffers(2, drawBuffers.data());

    // Make sure framebuffer is complete before feedback loop detection
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    drawQuad(program, "aPosition", 0.5f, 1.0f, true);

    // "Rendering to a texture where it samples from should geneates INVALID_OPERATION. Otherwise,
    // it should be NO_ERROR"
    EXPECT_GL_ERROR(expectedError);
}

// Tests invariance matching rules between built in varyings.
// Based on WebGL test conformance/glsl/misc/shaders-with-invariance.html.
TEST_P(WebGLCompatibilityTest, BuiltInInvariant)
{
    constexpr char kVS[] =
        R"(varying vec4 v_varying;
void main()
{
    gl_PointSize = 1.0;
    gl_Position = v_varying;
})";
    constexpr char kFSInvariantGlFragCoord[] =
        R"(invariant gl_FragCoord;
void main()
{
    gl_FragColor = gl_FragCoord;
})";
    constexpr char kFSInvariantGlPointCoord[] =
        R"(invariant gl_PointCoord;
void main()
{
    gl_FragColor = vec4(gl_PointCoord, 0.0, 0.0);
})";

    GLuint program = CompileProgram(kVS, kFSInvariantGlFragCoord);
    EXPECT_EQ(0u, program);

    program = CompileProgram(kVS, kFSInvariantGlPointCoord);
    EXPECT_EQ(0u, program);
}

// Tests global namespace conflicts between uniforms and attributes.
// Based on WebGL test conformance/glsl/misc/shaders-with-name-conflicts.html.
TEST_P(WebGLCompatibilityTest, GlobalNamesConflict)
{
    constexpr char kVS[] =
        R"(attribute vec4 foo;
void main()
{
    gl_Position = foo;
})";
    constexpr char kFS[] =
        R"(precision mediump float;
uniform vec4 foo;
void main()
{
    gl_FragColor = foo;
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

// Test dimension and image size validation of compressed textures
TEST_P(WebGLCompatibilityTest, CompressedTextureS3TC)
{
    if (IsGLExtensionRequestable("GL_EXT_texture_compression_dxt1"))
    {
        glRequestExtensionANGLE("GL_EXT_texture_compression_dxt1");
    }

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    constexpr uint8_t CompressedImageDXT1[] = {0x00, 0xf8, 0x00, 0xf8, 0xaa, 0xaa, 0xaa, 0xaa};

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    // Regular case, verify that it works
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_NO_ERROR();

    // Test various dimensions that are not valid
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 3, 4, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 3, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 2, 2, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 1, 1, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    // Test various image sizes that are not valid
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4, 0,
                           sizeof(CompressedImageDXT1) - 1, CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4, 0,
                           sizeof(CompressedImageDXT1) + 1, CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4, 0, 0,
                           CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 0, 0, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    // Fill a full mip chain and verify that it works
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 2, 2, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 1, 1, 0,
                           sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_NO_ERROR();

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                              sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_NO_ERROR();

    // Test that non-block size sub-uploads are not valid for the 0 mip
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 2, 2, 2, 2, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                              sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    // Test that non-block size sub-uploads are valid for if they fill the whole mip
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 2, 2, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                              sizeof(CompressedImageDXT1), CompressedImageDXT1);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, 1, 1, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                              sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_NO_ERROR();

    // Test that if the format miss-matches the texture, an error is generated
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 2, 2, 2, 2, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
                              sizeof(CompressedImageDXT1), CompressedImageDXT1);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test WebGL-specific constraints on sizes of S3TC textures' mipmap levels.
TEST_P(WebGLCompatibilityTest, CompressedTexImageS3TC)
{
    const char *extensions[] = {
        "GL_EXT_texture_compression_dxt1",
        "GL_ANGLE_texture_compression_dxt3",
        "GL_ANGLE_texture_compression_dxt5",
    };

    for (const char *extension : extensions)
    {
        if (IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
        }

        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(extension));
    }

    // Ported from WebGL conformance suite:
    // sdk/tests/conformance/extensions/s3tc-and-srgb.html
    constexpr GLenum formats[] = {
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
        GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
        GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
        GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
    };

    for (GLenum format : formats)
    {
        testCompressedTexImage(format);
    }
}

// Test WebGL-specific constraints on sizes of RGTC textures' mipmap levels.
TEST_P(WebGLCompatibilityTest, CompressedTexImageRGTC)
{
    if (IsGLExtensionRequestable("GL_EXT_texture_compression_rgtc"))
    {
        glRequestExtensionANGLE("GL_EXT_texture_compression_rgtc");
    }

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_rgtc"));

    // Ported from WebGL conformance suite:
    // sdk/tests/conformance/extensions/ext-texture-compression-rgtc.html
    constexpr GLenum formats[] = {GL_COMPRESSED_RED_RGTC1_EXT, GL_COMPRESSED_SIGNED_RED_RGTC1_EXT,
                                  GL_COMPRESSED_RED_GREEN_RGTC2_EXT,
                                  GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT};

    for (GLenum format : formats)
    {
        testCompressedTexImage(format);
    }
}

// Test WebGL-specific constraints on sizes of BPTC textures' mipmap levels.
TEST_P(WebGLCompatibilityTest, CompressedTexImageBPTC)
{
    if (IsGLExtensionRequestable("GL_EXT_texture_compression_bptc"))
    {
        glRequestExtensionANGLE("GL_EXT_texture_compression_bptc");
    }

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    // Ported from WebGL conformance suite:
    // sdk/tests/conformance/extensions/ext-texture-compression-bptc.html
    constexpr GLenum formats[] = {
        GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT,
        GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT};

    for (GLenum format : formats)
    {
        testCompressedTexImage(format);
    }
}

TEST_P(WebGLCompatibilityTest, L32FTextures)
{
    constexpr float textureData[]   = {15.1f, 0.0f, 0.0f, 0.0f};
    constexpr float readPixelData[] = {textureData[0], textureData[0], textureData[0], 1.0f};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized L 32F
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_float");
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render  = false;
            TestFloatTextureFormat(GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT, texture, filter, render,
                                   textureData, readPixelData);
        }

        if (getClientMajorVersion() >= 3 || IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized L 32F
            bool texture = IsGLExtensionEnabled("GL_OES_texture_float") &&
                           IsGLExtensionEnabled("GL_EXT_texture_storage");
            bool filter = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render = false;
            TestFloatTextureFormat(GL_LUMINANCE32F_EXT, GL_LUMINANCE, GL_FLOAT, texture, filter,
                                   render, textureData, readPixelData);
        }
    }
}

TEST_P(WebGLCompatibilityTest, A32FTextures)
{
    constexpr float textureData[]   = {33.33f, 0.0f, 0.0f, 0.0f};
    constexpr float readPixelData[] = {0.0f, 0.0f, 0.0f, textureData[0]};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized A 32F
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_float");
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render  = false;
            TestFloatTextureFormat(GL_ALPHA, GL_ALPHA, GL_FLOAT, texture, filter, render,
                                   textureData, readPixelData);
        }

        if (getClientMajorVersion() >= 3 || IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized A 32F
            bool texture = IsGLExtensionEnabled("GL_OES_texture_float") &&
                           IsGLExtensionEnabled("GL_EXT_texture_storage");
            bool filter = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render = false;
            TestFloatTextureFormat(GL_ALPHA32F_EXT, GL_ALPHA, GL_FLOAT, texture, filter, render,
                                   textureData, readPixelData);
        }
    }
}

TEST_P(WebGLCompatibilityTest, LA32FTextures)
{
    constexpr float textureData[]   = {-0.21f, 15.1f, 0.0f, 0.0f};
    constexpr float readPixelData[] = {textureData[0], textureData[0], textureData[0],
                                       textureData[1]};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized LA 32F
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_float");
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render  = false;
            TestFloatTextureFormat(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_FLOAT, texture,
                                   filter, render, textureData, readPixelData);
        }

        if (getClientMajorVersion() >= 3 || IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized LA 32F
            bool texture = IsGLExtensionEnabled("GL_OES_texture_float") &&
                           IsGLExtensionEnabled("GL_EXT_texture_storage");
            bool filter = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render = false;
            TestFloatTextureFormat(GL_LUMINANCE_ALPHA32F_EXT, GL_LUMINANCE_ALPHA, GL_FLOAT, texture,
                                   filter, render, textureData, readPixelData);
        }
    }
}

TEST_P(WebGLCompatibilityTest, R32FTextures)
{
    constexpr float data[] = {1000.0f, 0.0f, 0.0f, 1.0f};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized R 32F
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_float") &&
                           IsGLExtensionEnabled("GL_EXT_texture_rg");
            bool filter = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render = IsGLExtensionEnabled("GL_EXT_color_buffer_float");
            TestFloatTextureFormat(GL_RED, GL_RED, GL_FLOAT, texture, filter, render, data, data);
        }

        if (getClientMajorVersion() >= 3 || IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized R 32F
            bool texture =
                (getClientMajorVersion() >= 3) || (IsGLExtensionEnabled("GL_OES_texture_float") &&
                                                   IsGLExtensionEnabled("GL_EXT_texture_rg") &&
                                                   IsGLExtensionEnabled("GL_EXT_texture_storage"));
            bool filter = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render = IsGLExtensionEnabled("GL_EXT_color_buffer_float");
            TestFloatTextureFormat(GL_R32F, GL_RED, GL_FLOAT, texture, filter, render, data, data);
        }
    }
}

TEST_P(WebGLCompatibilityTest, RG32FTextures)
{
    constexpr float data[] = {1000.0f, -0.001f, 0.0f, 1.0f};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized RG 32F
        {
            bool texture = (IsGLExtensionEnabled("GL_OES_texture_float") &&
                            IsGLExtensionEnabled("GL_EXT_texture_rg"));
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render  = IsGLExtensionEnabled("GL_EXT_color_buffer_float");
            TestFloatTextureFormat(GL_RG, GL_RG, GL_FLOAT, texture, filter, render, data, data);
        }

        if (getClientMajorVersion() >= 3 || IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized RG 32F
            bool texture =
                (getClientMajorVersion() >= 3) || (IsGLExtensionEnabled("GL_OES_texture_float") &&
                                                   IsGLExtensionEnabled("GL_EXT_texture_rg") &&
                                                   IsGLExtensionEnabled("GL_EXT_texture_storage"));
            bool filter = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render = IsGLExtensionEnabled("GL_EXT_color_buffer_float");
            TestFloatTextureFormat(GL_RG32F, GL_RG, GL_FLOAT, texture, filter, render, data, data);
        }
    }
}

TEST_P(WebGLCompatibilityTest, RGB32FTextures)
{
    constexpr float data[] = {1000.0f, -500.0f, 10.0f, 1.0f};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized RGB 32F
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_float");
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render  = false;
            TestFloatTextureFormat(GL_RGB, GL_RGB, GL_FLOAT, texture, filter, render, data, data);
        }

        if (getClientMajorVersion() >= 3 || IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized RGB 32F
            bool texture =
                (getClientMajorVersion() >= 3) || (IsGLExtensionEnabled("GL_OES_texture_float") &&
                                                   IsGLExtensionEnabled("GL_EXT_texture_storage"));
            bool filter = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render = IsGLExtensionEnabled("GL_CHROMIUM_color_buffer_float_rgb");
            TestFloatTextureFormat(GL_RGB32F, GL_RGB, GL_FLOAT, texture, filter, render, data,
                                   data);
        }
    }
}

TEST_P(WebGLCompatibilityTest, RGBA32FTextures)
{
    // http://anglebug.com/42263897
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsMac());

    constexpr float data[] = {7000.0f, 100.0f, 33.0f, -1.0f};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized RGBA 32F
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_float");
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render  = false;
            TestFloatTextureFormat(GL_RGBA, GL_RGBA, GL_FLOAT, texture, filter, render, data, data);
        }

        if (getClientMajorVersion() >= 3 || IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized RGBA 32F
            bool texture =
                (getClientMajorVersion() >= 3) || (IsGLExtensionEnabled("GL_OES_texture_float") &&
                                                   IsGLExtensionEnabled("GL_EXT_texture_storage"));
            bool filter = IsGLExtensionEnabled("GL_OES_texture_float_linear");
            bool render = IsGLExtensionEnabled("GL_EXT_color_buffer_float") ||
                          IsGLExtensionEnabled("GL_CHROMIUM_color_buffer_float_rgba");
            TestFloatTextureFormat(GL_RGBA32F, GL_RGBA, GL_FLOAT, texture, filter, render, data,
                                   data);
        }
    }
}

// Test that has float color attachment caching works when color attachments change, by calling draw
// command when blending is enabled
TEST_P(WebGLCompatibilityTest, FramebufferFloatColorAttachment)
{
    if (getClientMajorVersion() >= 3)
    {
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_color_buffer_float"));
    }
    else
    {
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_texture_float"));
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_color_buffer_float_rgba"));
    }

    constexpr char kVS[] =
        R"(void main()
{
    gl_Position = vec4(0, 0, 0, 1);
})";

    constexpr char kFS[] =
        R"(void main()
{
    gl_FragColor = vec4(0, 1, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    glEnable(GL_BLEND);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo1;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDisable(GL_BLEND);
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();
    glEnable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glDrawArrays(GL_POINTS, 0, 1);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0,
                           0);  // test unbind
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDisable(GL_BLEND);
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();
    glEnable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();
}

// Test that has float color attachment caching works with multiple color attachments bound to a
// Framebuffer
TEST_P(WebGLCompatibilityTest, FramebufferFloatColorAttachmentMRT)
{
    bool isWebGL2 = getClientMajorVersion() >= 3;
    if (isWebGL2)
    {
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_color_buffer_float"));

        constexpr char kVS[] =
            R"(#version 300 es
void main()
{
    gl_Position = vec4(0, 0, 0, 1);
})";

        constexpr char kFS[] =
            R"(#version 300 es
precision lowp float;
layout(location = 0) out vec4 o_color0;
layout(location = 1) out vec4 o_color1;
void main()
{
    o_color0 = vec4(1, 0, 0, 1);
    o_color1 = vec4(0, 1, 0, 1);
})";

        ANGLE_GL_PROGRAM(program, kVS, kFS);
        glUseProgram(program);
    }
    else
    {
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_texture_float"));
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_color_buffer_float_rgba"));
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_draw_buffers"));

        constexpr char kVS[] =
            R"(void main()
{
    gl_Position = vec4(0, 0, 0, 1);
})";

        constexpr char kFS[] =
            R"(#extension GL_EXT_draw_buffers : require
precision lowp float;
void main()
{
    gl_FragData[0] = vec4(1, 0, 0, 1);
    gl_FragData[1] = vec4(0, 1, 0, 1);
})";

        ANGLE_GL_PROGRAM(program, kVS, kFS);
        glUseProgram(program);
    }

    glEnable(GL_BLEND);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    GLTexture textureF1;
    glBindTexture(GL_TEXTURE_2D, textureF1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    EXPECT_GL_NO_ERROR();

    GLTexture textureF2;
    glBindTexture(GL_TEXTURE_2D, textureF2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture2, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLenum drawbuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    if (isWebGL2)
    {
        glDrawBuffers(2, drawbuffers);
    }
    else
    {
        glDrawBuffersEXT(2, drawbuffers);
    }

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureF1, 0);
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureF2, 0);
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    if (isWebGL2)
    {
        // WebGL 1 will report a FRAMEBUFFER_UNSUPPORTED for one unsigned_byte and one float
        // attachment bound to one FBO at the same time
        glDrawBuffers(1, drawbuffers);
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        glDrawArrays(GL_POINTS, 0, 1);
        EXPECT_GL_NO_ERROR();
        glDrawBuffers(2, drawbuffers);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture2, 0);
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();
}

static void TestBlendColor(const bool shouldClamp)
{
    auto expected = GLColor32F(5, 0, 0, 0);
    glBlendColor(expected.R, expected.G, expected.B, expected.A);
    if (shouldClamp)
    {
        expected.R = 1;
    }

    float arr[4] = {};
    glGetFloatv(GL_BLEND_COLOR, arr);
    const auto actual = GLColor32F(arr[0], arr[1], arr[2], arr[3]);
    EXPECT_COLOR_NEAR(expected, actual, 0.001);
}

// Test if blending of float32 color attachment generates GL_INVALID_OPERATION when
// GL_EXT_float_blend is not enabled
TEST_P(WebGLCompatibilityTest, FloatBlend)
{
    if (getClientMajorVersion() >= 3)
    {
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_color_buffer_float"));
    }
    else
    {
        TestBlendColor(true);
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_texture_float"));
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_color_buffer_float_rgba"));
    }

    // -

    TestExtFloatBlend(GL_RGBA32F, GL_FLOAT, false);

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_float_blend"));
    ASSERT_GL_NO_ERROR();

    TestExtFloatBlend(GL_RGBA32F, GL_FLOAT, true);
}

// Test the blending of float16 color attachments
TEST_P(WebGLCompatibilityTest, HalfFloatBlend)
{
    GLenum internalFormat = GL_RGBA16F;
    GLenum type           = GL_FLOAT;
    if (getClientMajorVersion() >= 3)
    {
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_color_buffer_float"));
    }
    else
    {
        TestBlendColor(true);
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_texture_half_float"));
        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_color_buffer_half_float"));
        internalFormat = GL_RGBA;
        type           = GL_HALF_FLOAT_OES;
    }

    // -

    TestExtFloatBlend(internalFormat, type, true);
}

TEST_P(WebGLCompatibilityTest, R16FTextures)
{
    // http://anglebug.com/42263897
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsMac());

    constexpr float readPixelsData[] = {-5000.0f, 0.0f, 0.0f, 1.0f};
    const GLushort textureData[]     = {
        gl::float32ToFloat16(readPixelsData[0]), gl::float32ToFloat16(readPixelsData[1]),
        gl::float32ToFloat16(readPixelsData[2]), gl::float32ToFloat16(readPixelsData[3])};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized R 16F (OES)
        if (getClientMajorVersion() < 3)
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_half_float") &&
                           IsGLExtensionEnabled("GL_EXT_texture_rg");
            bool filter = IsGLExtensionEnabled("GL_OES_texture_half_float_linear");
            bool render = false;
            TestFloatTextureFormat(GL_RED, GL_RED, GL_HALF_FLOAT_OES, texture, filter, render,
                                   textureData, readPixelsData);
        }

        // Unsized R 16F
        {
            bool texture = false;
            bool filter  = false;
            bool render  = false;
            TestFloatTextureFormat(GL_RED, GL_RED, GL_HALF_FLOAT, texture, filter, render,
                                   textureData, readPixelsData);
        }

        if (getClientMajorVersion() >= 3)
        {
            // Sized R 16F
            bool texture = true;
            bool filter  = true;
            bool render  = IsGLExtensionEnabled("GL_EXT_color_buffer_float") ||
                          IsGLExtensionEnabled("GL_EXT_color_buffer_half_float");
            TestFloatTextureFormat(GL_R16F, GL_RED, GL_HALF_FLOAT, texture, filter, render,
                                   textureData, readPixelsData);
        }
        else if (IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized R 16F (OES)
            bool texture = IsGLExtensionEnabled("GL_OES_texture_half_float") &&
                           IsGLExtensionEnabled("GL_EXT_texture_rg");
            bool filter = IsGLExtensionEnabled("GL_OES_texture_half_float_linear");
            bool render = IsGLExtensionEnabled("GL_EXT_color_buffer_half_float");
            TestFloatTextureFormat(GL_R16F, GL_RED, GL_HALF_FLOAT_OES, texture, filter, render,
                                   textureData, readPixelsData);
        }
    }
}

TEST_P(WebGLCompatibilityTest, RG16FTextures)
{
    // http://anglebug.com/42263897
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsMac());

    constexpr float readPixelsData[] = {7108.0f, -10.0f, 0.0f, 1.0f};
    const GLushort textureData[]     = {
        gl::float32ToFloat16(readPixelsData[0]), gl::float32ToFloat16(readPixelsData[1]),
        gl::float32ToFloat16(readPixelsData[2]), gl::float32ToFloat16(readPixelsData[3])};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized RG 16F (OES)
        if (getClientMajorVersion() < 3)
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_half_float") &&
                           IsGLExtensionEnabled("GL_EXT_texture_rg");
            bool filter = IsGLExtensionEnabled("GL_OES_texture_half_float_linear");
            bool render = false;
            TestFloatTextureFormat(GL_RG, GL_RG, GL_HALF_FLOAT_OES, texture, filter, render,
                                   textureData, readPixelsData);
        }

        // Unsized RG 16F
        {
            bool texture = false;
            bool filter  = false;
            bool render  = false;
            TestFloatTextureFormat(GL_RG, GL_RG, GL_HALF_FLOAT, texture, filter, render,
                                   textureData, readPixelsData);
        }

        if (getClientMajorVersion() >= 3)
        {
            // Sized RG 16F
            bool texture = true;
            bool filter  = true;
            bool render  = IsGLExtensionEnabled("GL_EXT_color_buffer_float") ||
                          IsGLExtensionEnabled("GL_EXT_color_buffer_half_float");
            TestFloatTextureFormat(GL_RG16F, GL_RG, GL_HALF_FLOAT, texture, filter, render,
                                   textureData, readPixelsData);
        }
        else if (IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized RG 16F (OES)
            bool texture = IsGLExtensionEnabled("GL_OES_texture_half_float") &&
                           IsGLExtensionEnabled("GL_EXT_texture_rg");
            bool filter = IsGLExtensionEnabled("GL_OES_texture_half_float_linear");
            bool render = IsGLExtensionEnabled("GL_EXT_color_buffer_half_float");
            TestFloatTextureFormat(GL_RG16F, GL_RG, GL_HALF_FLOAT_OES, texture, filter, render,
                                   textureData, readPixelsData);
        }
    }
}

TEST_P(WebGLCompatibilityTest, RGB16FTextures)
{
    // http://anglebug.com/42263897
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsMac());

    ANGLE_SKIP_TEST_IF(IsOzone() && IsIntel());

    constexpr float readPixelsData[] = {7000.0f, 100.0f, 33.0f, 1.0f};
    const GLushort textureData[]     = {
        gl::float32ToFloat16(readPixelsData[0]), gl::float32ToFloat16(readPixelsData[1]),
        gl::float32ToFloat16(readPixelsData[2]), gl::float32ToFloat16(readPixelsData[3])};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized RGB 16F (OES)
        if (getClientMajorVersion() < 3)
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_half_float");
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_half_float_linear");
            // WebGL says that Unsized RGB 16F (OES) can be renderable with
            // GL_EXT_color_buffer_half_float.
            bool render = IsGLExtensionEnabled("GL_EXT_color_buffer_half_float");
            TestFloatTextureFormat(GL_RGB, GL_RGB, GL_HALF_FLOAT_OES, texture, filter, render,
                                   textureData, readPixelsData);
        }

        // Unsized RGB 16F
        {
            bool texture = false;
            bool filter  = false;
            bool render  = false;
            TestFloatTextureFormat(GL_RGB, GL_RGB, GL_HALF_FLOAT, texture, filter, render,
                                   textureData, readPixelsData);
        }

        if (getClientMajorVersion() >= 3)
        {
            // Sized RGB 16F
            bool texture = true;
            bool filter  = true;
            // Renderability of RGB is forbidden by GL_EXT_color_buffer_half_float in WebGL 2.
            bool render = false;
            TestFloatTextureFormat(GL_RGB16F, GL_RGB, GL_HALF_FLOAT, texture, filter, render,
                                   textureData, readPixelsData);
        }
        else if (IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized RGB 16F (OES)
            bool texture = IsGLExtensionEnabled("GL_OES_texture_half_float");
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_half_float_linear");
            bool render  = IsGLExtensionEnabled("GL_EXT_color_buffer_half_float");
            TestFloatTextureFormat(GL_RGB16F, GL_RGB, GL_HALF_FLOAT_OES, texture, filter, render,
                                   textureData, readPixelsData);
        }
    }
}

TEST_P(WebGLCompatibilityTest, RGBA16FTextures)
{
    // http://anglebug.com/42263897
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsMac());

    ANGLE_SKIP_TEST_IF(IsOzone() && IsIntel());

    constexpr float readPixelsData[] = {7000.0f, 100.0f, 33.0f, -1.0f};
    const GLushort textureData[]     = {
        gl::float32ToFloat16(readPixelsData[0]), gl::float32ToFloat16(readPixelsData[1]),
        gl::float32ToFloat16(readPixelsData[2]), gl::float32ToFloat16(readPixelsData[3])};

    for (auto extension : FloatingPointTextureExtensions)
    {
        if (strlen(extension) > 0 && IsGLExtensionRequestable(extension))
        {
            glRequestExtensionANGLE(extension);
            ASSERT_GL_NO_ERROR();
        }

        // Unsized RGBA 16F (OES)
        if (getClientMajorVersion() < 3)
        {
            bool texture = IsGLExtensionEnabled("GL_OES_texture_half_float");
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_half_float_linear");
            bool render  = IsGLExtensionEnabled("GL_EXT_color_buffer_half_float");
            TestFloatTextureFormat(GL_RGBA, GL_RGBA, GL_HALF_FLOAT_OES, texture, filter, render,
                                   textureData, readPixelsData);
        }

        // Unsized RGBA 16F
        {
            bool texture = false;
            bool filter  = false;
            bool render  = false;
            TestFloatTextureFormat(GL_RGBA, GL_RGBA, GL_HALF_FLOAT, texture, filter, render,
                                   textureData, readPixelsData);
        }

        if (getClientMajorVersion() >= 3)
        {
            // Sized RGBA 16F
            bool texture = true;
            bool filter  = true;
            bool render  = IsGLExtensionEnabled("GL_EXT_color_buffer_float") ||
                          IsGLExtensionEnabled("GL_EXT_color_buffer_half_float");
            TestFloatTextureFormat(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, texture, filter, render,
                                   textureData, readPixelsData);
        }
        else if (IsGLExtensionEnabled("GL_EXT_texture_storage"))
        {
            // Sized RGBA 16F (OES)
            bool texture = IsGLExtensionEnabled("GL_OES_texture_half_float");
            bool filter  = IsGLExtensionEnabled("GL_OES_texture_half_float_linear");
            bool render  = IsGLExtensionEnabled("GL_EXT_color_buffer_half_float");
            TestFloatTextureFormat(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT_OES, texture, filter, render,
                                   textureData, readPixelsData);
        }
    }
}

// Test that when GL_CHROMIUM_color_buffer_float_rgb[a] is enabled, sized GL_RGB[A]_32F formats are
// accepted by glTexImage2D
TEST_P(WebGLCompatibilityTest, SizedRGBA32FFormats)
{
    // Test skipped because it is only valid for WebGL1 contexts.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() != 2);

    ANGLE_SKIP_TEST_IF(!IsGLExtensionRequestable("GL_OES_texture_float"));

    glRequestExtensionANGLE("GL_OES_texture_float");
    ASSERT_GL_NO_ERROR();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    // dEQP implicitly defines error code ordering
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 1, 1, 0, GL_RGB, GL_FLOAT, nullptr);
    // dEQP implicitly defines error code ordering
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable("GL_CHROMIUM_color_buffer_float_rgba"))
    {
        glRequestExtensionANGLE("GL_CHROMIUM_color_buffer_float_rgba");
        ASSERT_GL_NO_ERROR();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
        EXPECT_GL_NO_ERROR();
    }

    if (IsGLExtensionRequestable("GL_CHROMIUM_color_buffer_float_rgb"))
    {
        glRequestExtensionANGLE("GL_CHROMIUM_color_buffer_float_rgb");
        ASSERT_GL_NO_ERROR();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 1, 1, 0, GL_RGB, GL_FLOAT, nullptr);
        EXPECT_GL_NO_ERROR();
    }
}

// Verify GL_DEPTH_STENCIL_ATTACHMENT is a valid attachment point.
TEST_P(WebGLCompatibilityTest, DepthStencilAttachment)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() > 2);

    // Test that attaching a bound texture succeeds.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

    GLint attachmentType = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachmentType);
    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_TEXTURE, attachmentType);

    // Test when if no attach object at the named attachment point and pname is not OBJECT_TYPE.
    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &attachmentType);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Verify framebuffer attachments return expected types when in an inconsistant state.
TEST_P(WebGLCompatibilityTest, FramebufferAttachmentConsistancy)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() > 2);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer rb1;
    glBindRenderbuffer(GL_RENDERBUFFER, rb1);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb1);

    GLint attachmentType = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachmentType);

    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_RENDERBUFFER, attachmentType);

    GLRenderbuffer rb2;
    glBindRenderbuffer(GL_RENDERBUFFER, rb2);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb2);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachmentType);

    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_RENDERBUFFER, attachmentType);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb2);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachmentType);

    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_RENDERBUFFER, attachmentType);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb2);

    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachmentType);

    EXPECT_GL_NO_ERROR();
    EXPECT_GLENUM_EQ(GL_RENDERBUFFER, attachmentType);
}

// This tests that rendering feedback loops works as expected with WebGL 2.
// Based on WebGL test conformance2/rendering/rendering-sampling-feedback-loop.html
TEST_P(WebGL2CompatibilityTest, RenderingFeedbackLoopWithDrawBuffers)
{
    constexpr char kVS[] =
        R"(#version 300 es
in vec4 aPosition;
out vec2 texCoord;
void main() {
    gl_Position = aPosition;
    texCoord = (aPosition.xy * 0.5) + 0.5;
})";

    constexpr char kFS[] =
        R"(#version 300 es
precision mediump float;
uniform sampler2D tex;
in vec2 texCoord;
out vec4 oColor;
void main() {
    oColor = texture(tex, texCoord);
})";

    GLsizei width  = 8;
    GLsizei height = 8;

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    // ES3 requires a minimum value of 4 for MAX_DRAW_BUFFERS.
    ASSERT_GE(maxDrawBuffers, 2);

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glViewport(0, 0, width, height);

    GLTexture tex0;
    GLTexture tex1;
    GLFramebuffer fbo;
    FillTexture2D(tex0, width, height, GLColor::red, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    FillTexture2D(tex1, width, height, GLColor::green, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    ASSERT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, tex1);
    GLint texLoc = glGetUniformLocation(program, "tex");
    ASSERT_NE(-1, texLoc);
    glUniform1i(texLoc, 0);

    // The sampling texture is bound to COLOR_ATTACHMENT1 during resource allocation
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex1, 0);
    ASSERT_GL_NO_ERROR();

    drawBuffersFeedbackLoop(program, {{GL_NONE, GL_COLOR_ATTACHMENT1}}, GL_INVALID_OPERATION);
    drawBuffersFeedbackLoop(program, {{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1}},
                            GL_INVALID_OPERATION);
    // A feedback loop is formed regardless of drawBuffers settings.
    drawBuffersFeedbackLoop(program, {{GL_COLOR_ATTACHMENT0, GL_NONE}}, GL_INVALID_OPERATION);
}

// This tests that texture base level for immutable textures is clamped to the valid range, unlike
// for non-immutable textures, for purposes of validation. Related to WebGL test
// conformance2/textures/misc/immutable-tex-render-feedback.html
TEST_P(WebGL2CompatibilityTest, RenderingFeedbackLoopWithImmutableTextureWithOutOfRangeBaseLevel)
{
    constexpr char kVS[] =
        R"(#version 300 es
in vec4 aPosition;
out vec2 texCoord;
void main() {
    gl_Position = aPosition;
    texCoord = (aPosition.xy * 0.5) + 0.5;
})";

    constexpr char kFS[] =
        R"(#version 300 es
precision mediump float;
uniform sampler2D tex;
in vec2 texCoord;
out vec4 oColor;
void main() {
    oColor = texture(tex, texCoord);
})";

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 4);
    std::vector<GLColor> texData(4 * 4, GLColor::green);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, texData.data());
    // Set a base level greater than the max level. It should be clamped to the actual max level.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint uniformLoc = glGetUniformLocation(program, "tex");
    ASSERT_NE(-1, uniformLoc);

    glUseProgram(program);
    glUniform1i(uniformLoc, 0);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    ASSERT_GL_NO_ERROR();

    // Ensure that the texture can be used for rendering.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, texture);
    drawQuad(program, "aPosition", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Ensure that the texture can't be used to create a feedback loop.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindTexture(GL_TEXTURE_2D, texture);
    drawQuad(program, "aPosition", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// This test covers detection of rendering feedback loops between the FBO and a depth Texture.
// Based on WebGL test conformance2/rendering/depth-stencil-feedback-loop.html
TEST_P(WebGL2CompatibilityTest, RenderingFeedbackLoopWithDepthStencil)
{
    constexpr char kVS[] =
        R"(#version 300 es
in vec4 aPosition;
out vec2 texCoord;
void main() {
    gl_Position = aPosition;
    texCoord = (aPosition.xy * 0.5) + 0.5;
})";

    constexpr char kFS[] =
        R"(#version 300 es
precision mediump float;
uniform sampler2D tex;
in vec2 texCoord;
out vec4 oColor;
void main() {
    oColor = texture(tex, texCoord);
})";

    GLsizei width  = 8;
    GLsizei height = 8;

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    glViewport(0, 0, width, height);

    GLint texLoc = glGetUniformLocation(program, "tex");
    glUniform1i(texLoc, 0);

    // Create textures and allocate storage
    GLTexture tex0;
    GLTexture tex1;
    GLTexture tex2;
    FillTexture2D(tex0, width, height, GLColor::black, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    FillTexture2D(tex1, width, height, 0x80, 0, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT,
                  GL_UNSIGNED_INT);
    FillTexture2D(tex2, width, height, 0x40, 0, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,
                  GL_UNSIGNED_INT_24_8);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex0, 0);

    // Test rendering and sampling feedback loop for depth buffer
    glBindTexture(GL_TEXTURE_2D, tex1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex1, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // The same image is used as depth buffer during rendering.
    glEnable(GL_DEPTH_TEST);
    drawQuad(program, "aPosition", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Same image as depth buffer should fail";

    // The same image is used as depth buffer. But depth mask is false.
    // This is now considered a feedback loop and should generate an error. http://crbug.com/763695
    glDepthMask(GL_FALSE);
    drawQuad(program, "aPosition", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Depth writes disabled should still fail";

    // The same image is used as depth buffer. But depth test is not enabled during rendering.
    // This is now considered a feedback loop and should generate an error. http://crbug.com/763695
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    drawQuad(program, "aPosition", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Depth read disabled should still fail";

    // Test rendering and sampling feedback loop for stencil buffer
    glBindTexture(GL_TEXTURE_2D, tex2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex2, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    constexpr GLint stencilClearValue = 0x40;
    glClearBufferiv(GL_STENCIL, 0, &stencilClearValue);

    // The same image is used as stencil buffer during rendering.
    glEnable(GL_STENCIL_TEST);
    drawQuad(program, "aPosition", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Same image as stencil buffer should fail";

    // The same image is used as stencil buffer. But stencil mask is zero.
    // This is now considered a feedback loop and should generate an error. http://crbug.com/763695
    glStencilMask(0x0);
    drawQuad(program, "aPosition", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Stencil mask zero should still fail";

    // The same image is used as stencil buffer. But stencil test is not enabled during rendering.
    // This is now considered a feedback loop and should generate an error. http://crbug.com/763695
    glStencilMask(0xffff);
    glDisable(GL_STENCIL_TEST);
    drawQuad(program, "aPosition", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION) << "Stencil test disabled should still fail";
}

// The source and the target for CopyTexSubImage3D are the same 3D texture.
// But the level of the 3D texture != the level of the read attachment.
TEST_P(WebGL2CompatibilityTest, NoTextureCopyingFeedbackLoopBetween3DLevels)
{
    GLTexture texture;
    GLFramebuffer framebuffer;

    glBindTexture(GL_TEXTURE_3D, texture);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3D(GL_TEXTURE_3D, 1, GL_RGBA8, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, 0);
    ASSERT_GL_NO_ERROR();

    glCopyTexSubImage3D(GL_TEXTURE_3D, 1, 0, 0, 0, 0, 0, 2, 2);
    EXPECT_GL_NO_ERROR();
}

// The source and the target for CopyTexSubImage3D are the same 3D texture.
// But the zoffset of the 3D texture != the layer of the read attachment.
TEST_P(WebGL2CompatibilityTest, NoTextureCopyingFeedbackLoopBetween3DLayers)
{
    GLTexture texture;
    GLFramebuffer framebuffer;

    glBindTexture(GL_TEXTURE_3D, texture);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, 1);
    ASSERT_GL_NO_ERROR();

    glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 0, 0, 2, 2);
    EXPECT_GL_NO_ERROR();
}

// The source and the target for CopyTexSubImage3D are the same 3D texture.
// And the level / zoffset of the 3D texture is equal to the level / layer of the read attachment.
TEST_P(WebGL2CompatibilityTest, TextureCopyingFeedbackLoop3D)
{
    GLTexture texture;
    GLFramebuffer framebuffer;

    glBindTexture(GL_TEXTURE_3D, texture);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 4, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3D(GL_TEXTURE_3D, 1, GL_RGBA8, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3D(GL_TEXTURE_3D, 2, GL_RGBA8, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 1, 0);
    ASSERT_GL_NO_ERROR();

    glCopyTexSubImage3D(GL_TEXTURE_3D, 1, 0, 0, 0, 0, 0, 2, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Verify that errors are generated when there isn't a defined conversion between the clear type and
// the buffer type.
TEST_P(WebGL2CompatibilityTest, ClearBufferTypeCompatibity)
{
    // Test skipped for D3D11 because it generates D3D11 runtime warnings.
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr float clearFloat[]       = {0.0f, 0.0f, 0.0f, 0.0f};
    constexpr int clearInt[]           = {0, 0, 0, 0};
    constexpr unsigned int clearUint[] = {0, 0, 0, 0};

    GLTexture texture;
    GLFramebuffer framebuffer;

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_NO_ERROR();

    // Unsigned integer buffer
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, 1, 1, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, nullptr);
    ASSERT_GL_NO_ERROR();

    glClearBufferfv(GL_COLOR, 0, clearFloat);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClearBufferiv(GL_COLOR, 0, clearInt);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClearBufferuiv(GL_COLOR, 0, clearUint);
    EXPECT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Integer buffer
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, 1, 1, 0, GL_RGBA_INTEGER, GL_INT, nullptr);
    ASSERT_GL_NO_ERROR();

    glClearBufferfv(GL_COLOR, 0, clearFloat);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClearBufferiv(GL_COLOR, 0, clearInt);
    EXPECT_GL_NO_ERROR();

    glClearBufferuiv(GL_COLOR, 0, clearUint);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Float buffer
    if (IsGLExtensionRequestable("GL_EXT_color_buffer_float"))
    {
        glRequestExtensionANGLE("GL_EXT_color_buffer_float");
    }

    if (IsGLExtensionEnabled("GL_EXT_color_buffer_float"))
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
        ASSERT_GL_NO_ERROR();

        glClearBufferfv(GL_COLOR, 0, clearFloat);
        EXPECT_GL_NO_ERROR();

        glClearBufferiv(GL_COLOR, 0, clearInt);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glClearBufferuiv(GL_COLOR, 0, clearUint);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();
    }

    // Normalized uint buffer
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    glClearBufferfv(GL_COLOR, 0, clearFloat);
    EXPECT_GL_NO_ERROR();

    glClearBufferiv(GL_COLOR, 0, clearInt);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClearBufferuiv(GL_COLOR, 0, clearUint);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();
}

// Test the interaction of WebGL compatibility clears with default framebuffers
TEST_P(WebGL2CompatibilityTest, ClearBufferDefaultFramebuffer)
{
    constexpr float clearFloat[]       = {0.0f, 0.0f, 0.0f, 0.0f};
    constexpr int clearInt[]           = {0, 0, 0, 0};
    constexpr unsigned int clearUint[] = {0, 0, 0, 0};

    // glClear works as usual, this is also a regression test for a bug where we
    // iterated on maxDrawBuffers for default framebuffers, triggering an assert
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Default framebuffers are normalized uints, so only glClearBufferfv works.
    glClearBufferfv(GL_COLOR, 0, clearFloat);
    EXPECT_GL_NO_ERROR();

    glClearBufferiv(GL_COLOR, 0, clearInt);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClearBufferuiv(GL_COLOR, 0, clearUint);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that clearing a non-existent drawbuffer of the default
// framebuffer does not cause an assertion in WebGL validation
TEST_P(WebGL2CompatibilityTest, ClearBuffer1OnDefaultFramebufferNoAssert)
{
    constexpr float clearFloat[]   = {0.0f, 0.0f, 0.0f, 0.0f};
    constexpr int32_t clearInt[]   = {0, 0, 0, 0};
    constexpr uint32_t clearUint[] = {0, 0, 0, 0};

    glClearBufferfv(GL_COLOR, 1, clearFloat);
    EXPECT_GL_NO_ERROR();

    glClearBufferiv(GL_COLOR, 1, clearInt);
    EXPECT_GL_NO_ERROR();

    glClearBufferuiv(GL_COLOR, 1, clearUint);
    EXPECT_GL_NO_ERROR();
}

// Verify that errors are generate when trying to blit from an image to itself
TEST_P(WebGL2CompatibilityTest, BlitFramebufferSameImage)
{
    GLTexture textures[2];
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 4, 4);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 4, 4);

    GLRenderbuffer renderbuffers[2];
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 4, 4);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 4, 4);

    GLFramebuffer framebuffers[2];
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[1]);

    ASSERT_GL_NO_ERROR();

    // Same texture
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0],
                           0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0],
                           0);
    ASSERT_GL_NO_ERROR();
    glBlitFramebuffer(0, 0, 4, 4, 0, 0, 4, 4, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    // Same textures but different renderbuffers
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffers[0]);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffers[1]);
    ASSERT_GL_NO_ERROR();
    glBlitFramebuffer(0, 0, 4, 4, 0, 0, 4, 4, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glBlitFramebuffer(0, 0, 4, 4, 0, 0, 4, 4, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glBlitFramebuffer(0, 0, 4, 4, 0, 0, 4, 4,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    // Same renderbuffers but different textures
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0],
                           0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1],
                           0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffers[0]);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffers[0]);
    ASSERT_GL_NO_ERROR();
    glBlitFramebuffer(0, 0, 4, 4, 0, 0, 4, 4, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glBlitFramebuffer(0, 0, 4, 4, 0, 0, 4, 4, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    glBlitFramebuffer(0, 0, 4, 4, 0, 0, 4, 4,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Verify that errors are generated when the fragment shader output doesn't match the bound color
// buffer types
TEST_P(WebGL2CompatibilityTest, FragmentShaderColorBufferTypeMissmatch)
{
    constexpr char kVS[] =
        R"(#version 300 es
void main() {
    gl_Position = vec4(0, 0, 0, 1);
})";

    constexpr char kFS[] =
        R"(#version 300 es
precision mediump float;
layout(location = 0) out vec4 floatOutput;
layout(location = 1) out uvec4 uintOutput;
layout(location = 2) out ivec4 intOutput;
void main() {
    floatOutput = vec4(0, 0, 0, 1);
    uintOutput = uvec4(0, 0, 0, 1);
    intOutput = ivec4(0, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLuint floatLocation = glGetFragDataLocation(program, "floatOutput");
    GLuint uintLocation  = glGetFragDataLocation(program, "uintOutput");
    GLuint intLocation   = glGetFragDataLocation(program, "intOutput");

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer floatRenderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, floatRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + floatLocation, GL_RENDERBUFFER,
                              floatRenderbuffer);

    GLRenderbuffer uintRenderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, uintRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8UI, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + uintLocation, GL_RENDERBUFFER,
                              uintRenderbuffer);

    GLRenderbuffer intRenderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, intRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8I, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + intLocation, GL_RENDERBUFFER,
                              intRenderbuffer);

    ASSERT_GL_NO_ERROR();

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    std::vector<GLenum> drawBuffers(static_cast<size_t>(maxDrawBuffers), GL_NONE);
    drawBuffers[floatLocation] = GL_COLOR_ATTACHMENT0 + floatLocation;
    drawBuffers[uintLocation]  = GL_COLOR_ATTACHMENT0 + uintLocation;
    drawBuffers[intLocation]   = GL_COLOR_ATTACHMENT0 + intLocation;

    glDrawBuffers(maxDrawBuffers, drawBuffers.data());

    // Check that the correct case generates no errors
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    // Unbind some buffers and verify that there are still no errors
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + uintLocation, GL_RENDERBUFFER,
                              0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + intLocation, GL_RENDERBUFFER,
                              0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    // Swap the int and uint buffers to and verify that an error is generated
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + uintLocation, GL_RENDERBUFFER,
                              intRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + intLocation, GL_RENDERBUFFER,
                              uintRenderbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Swap the float and uint buffers to and verify that an error is generated
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + uintLocation, GL_RENDERBUFFER,
                              floatRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + floatLocation, GL_RENDERBUFFER,
                              uintRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + intLocation, GL_RENDERBUFFER,
                              intRenderbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Verify that errors are generated when the vertex shader intput doesn't match the bound attribute
// types
TEST_P(WebGL2CompatibilityTest, VertexShaderAttributeTypeMismatch)
{
    constexpr char kVS[] =
        R"(#version 300 es
in vec4 floatInput;
in uvec4 uintInput;
in ivec4 intInput;
void main() {
    gl_Position = vec4(floatInput.x, uintInput.x, intInput.x, 1);
})";

    constexpr char kFS[] =
        R"(#version 300 es
precision mediump float;
out vec4 outputColor;
void main() {
    outputColor = vec4(0, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint floatLocation = glGetAttribLocation(program, "floatInput");
    GLint uintLocation  = glGetAttribLocation(program, "uintInput");
    GLint intLocation   = glGetAttribLocation(program, "intInput");

    // Default attributes are of float types
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Set the default attributes to the correct types, should succeed
    glVertexAttribI4ui(uintLocation, 0, 0, 0, 1);
    glVertexAttribI4i(intLocation, 0, 0, 0, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    // Change the default float attribute to an integer, should fail
    glVertexAttribI4ui(floatLocation, 0, 0, 0, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Use a buffer for some attributes
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 1024, nullptr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(floatLocation);
    glVertexAttribPointer(floatLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    // Use a float pointer attrib for a uint input
    glEnableVertexAttribArray(uintLocation);
    glVertexAttribPointer(uintLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Use a uint pointer for the uint input
    glVertexAttribIPointer(uintLocation, 4, GL_UNSIGNED_INT, 0, nullptr);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();
}

// Test that it's not possible to query the non-zero color attachments without the drawbuffers
// extension in WebGL1
TEST_P(WebGLCompatibilityTest, FramebufferAttachmentQuery)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() > 2);
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_EXT_draw_buffers"));

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    GLint result;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &result);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, renderbuffer);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Tests WebGL reports INVALID_OPERATION for mismatch of drawbuffers and fragment output
TEST_P(WebGLCompatibilityTest, DrawBuffers)
{
    // Make sure we can use at least 4 attachments for the tests.
    bool useEXT = false;
    if (getClientMajorVersion() < 3)
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionRequestable("GL_EXT_draw_buffers"));

        glRequestExtensionANGLE("GL_EXT_draw_buffers");
        useEXT = true;
        EXPECT_GL_NO_ERROR();
    }

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    // Test skipped because MAX_DRAW_BUFFERS is too small.
    ANGLE_SKIP_TEST_IF(maxDrawBuffers < 4);

    // Clears all the renderbuffers to red.
    auto ClearEverythingToRed = [](GLRenderbuffer *renderbuffers) {
        GLFramebuffer clearFBO;
        glBindFramebuffer(GL_FRAMEBUFFER, clearFBO);

        glClearColor(1, 0, 0, 1);
        for (int i = 0; i < 4; ++i)
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      renderbuffers[i]);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        ASSERT_GL_NO_ERROR();
    };

    // Checks that the renderbuffers specified by mask have the correct color
    auto CheckColors = [](GLRenderbuffer *renderbuffers, int mask, GLColor color) {
        GLFramebuffer readFBO;
        glBindFramebuffer(GL_FRAMEBUFFER, readFBO);

        for (int attachmentIndex = 0; attachmentIndex < 4; ++attachmentIndex)
        {
            if (mask & (1 << attachmentIndex))
            {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                          renderbuffers[attachmentIndex]);
                EXPECT_PIXEL_COLOR_EQ(0, 0, color) << "attachment " << attachmentIndex;
            }
        }
        ASSERT_GL_NO_ERROR();
    };

    // Depending on whether we are using the extension or ES3, a different entrypoint must be called
    auto DrawBuffers = [](bool useEXT, int numBuffers, GLenum *buffers) {
        if (useEXT)
        {
            glDrawBuffersEXT(numBuffers, buffers);
        }
        else
        {
            glDrawBuffers(numBuffers, buffers);
        }
    };

    // Initialized the test framebuffer
    GLFramebuffer drawFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);

    GLRenderbuffer renderbuffers[4];
    for (int i = 0; i < 4; ++i)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER,
                                  renderbuffers[i]);
    }

    ASSERT_GL_NO_ERROR();

    GLenum allDrawBuffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
    };

    GLenum halfDrawBuffers[] = {
        GL_NONE,
        GL_COLOR_ATTACHMENT1,
        GL_NONE,
        GL_COLOR_ATTACHMENT3,
    };

    // Test that when using gl_FragColor with no-array
    const char *fragESSL1 =
        R"(precision highp float;
void main()
{
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
})";
    ANGLE_GL_PROGRAM(programESSL1, essl1_shaders::vs::Simple(), fragESSL1);

    {
        glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
        DrawBuffers(useEXT, 4, allDrawBuffers);
        drawQuad(programESSL1, essl1_shaders::PositionAttrib(), 0.5, 1.0, true);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    // Test what happens when rendering to a subset of the outputs. There is a behavior difference
    // between the extension and ES3. In the extension gl_FragData is implicitly declared as an
    // array of size MAX_DRAW_BUFFERS, so the WebGL spec stipulates that elements not written to
    // should default to 0. On the contrary, in ES3 outputs are specified one by one, so
    // attachments not declared in the shader should not be written to.
    const char *positionAttrib;
    const char *writeOddOutputsVert;
    const char *writeOddOutputsFrag;
    if (useEXT)
    {
        positionAttrib      = essl1_shaders::PositionAttrib();
        writeOddOutputsVert = essl1_shaders::vs::Simple();
        writeOddOutputsFrag =
            R"(#extension GL_EXT_draw_buffers : require
precision highp float;
void main()
{
    gl_FragData[1] = vec4(0.0, 1.0, 0.0, 1.0);
    gl_FragData[3] = vec4(0.0, 1.0, 0.0, 1.0);
})";
    }
    else
    {
        positionAttrib      = essl3_shaders::PositionAttrib();
        writeOddOutputsVert = essl3_shaders::vs::Simple();
        writeOddOutputsFrag =
            R"(#version 300 es
precision highp float;
layout(location = 1) out vec4 output1;
layout(location = 3) out vec4 output2;
void main()
{
    output1 = vec4(0.0, 1.0, 0.0, 1.0);
    output2 = vec4(0.0, 1.0, 0.0, 1.0);
})";
    }
    ANGLE_GL_PROGRAM(writeOddOutputsProgram, writeOddOutputsVert, writeOddOutputsFrag);

    // Test that attachments not written to get the "unwritten" color (useEXT)
    // Or INVALID_OPERATION is generated if there's active draw buffer receive no output
    {
        ClearEverythingToRed(renderbuffers);

        glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
        DrawBuffers(useEXT, 4, allDrawBuffers);
        drawQuad(writeOddOutputsProgram, positionAttrib, 0.5, 1.0, true);

        if (useEXT)
        {
            ASSERT_GL_NO_ERROR();
            CheckColors(renderbuffers, 0b1010, GLColor::green);
            // In the extension, when an attachment isn't written to, it should get 0's
            CheckColors(renderbuffers, 0b0101, GLColor(0, 0, 0, 0));
        }
        else
        {
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        }
    }

    // Test that attachments written to get the correct color from shader output but that even when
    // the extension is used, disabled attachments are not written at all and stay red.
    {
        ClearEverythingToRed(renderbuffers);

        glBindFramebuffer(GL_FRAMEBUFFER, drawFBO);
        DrawBuffers(useEXT, 4, halfDrawBuffers);
        drawQuad(writeOddOutputsProgram, positionAttrib, 0.5, 1.0, true);
        ASSERT_GL_NO_ERROR();

        CheckColors(renderbuffers, 0b1010, GLColor::green);
        CheckColors(renderbuffers, 0b0101, GLColor::red);
    }
}

// Test that it's possible to generate mipmaps on unsized floating point textures once the
// extensions have been enabled
TEST_P(WebGLCompatibilityTest, GenerateMipmapUnsizedFloatingPointTexture)
{
    glRequestExtensionANGLE("GL_OES_texture_float");
    glRequestExtensionANGLE("GL_CHROMIUM_color_buffer_float_rgba");
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_float"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_color_buffer_float_rgba"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    constexpr GLColor32F data[4] = {
        kFloatRed,
        kFloatRed,
        kFloatGreen,
        kFloatBlue,
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_FLOAT, data);
    ASSERT_GL_NO_ERROR();

    glGenerateMipmap(GL_TEXTURE_2D);
    EXPECT_GL_NO_ERROR();
}
// Test that it's possible to generate mipmaps on unsized floating point textures once the
// extensions have been enabled
TEST_P(WebGLCompatibilityTest, GenerateMipmapSizedFloatingPointTexture)
{
    if (IsGLExtensionRequestable("GL_OES_texture_float"))
    {
        glRequestExtensionANGLE("GL_OES_texture_float");
    }
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_float"));

    if (IsGLExtensionRequestable("GL_EXT_texture_storage"))
    {
        glRequestExtensionANGLE("GL_EXT_texture_storage");
    }
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_storage"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    constexpr GLColor32F data[4] = {
        kFloatRed,
        kFloatRed,
        kFloatGreen,
        kFloatBlue,
    };
    glTexStorage2DEXT(GL_TEXTURE_2D, 2, GL_RGBA32F, 2, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_FLOAT, data);
    ASSERT_GL_NO_ERROR();

    glGenerateMipmap(GL_TEXTURE_2D);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    if (IsGLExtensionRequestable("GL_EXT_color_buffer_float"))
    {
        // Format is renderable but not filterable
        glRequestExtensionANGLE("GL_EXT_color_buffer_float");
        glGenerateMipmap(GL_TEXTURE_2D);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    if (IsGLExtensionRequestable("GL_EXT_color_buffer_float_linear"))
    {
        // Format is renderable but not filterable
        glRequestExtensionANGLE("GL_EXT_color_buffer_float_linear");

        if (IsGLExtensionEnabled("GL_EXT_color_buffer_float"))
        {
            // Format is filterable and renderable
            glGenerateMipmap(GL_TEXTURE_2D);
            EXPECT_GL_NO_ERROR();
        }
        else
        {
            // Format is filterable but not renderable
            glGenerateMipmap(GL_TEXTURE_2D);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        }
    }
}

// Verify that a texture format is only allowed with extension enabled.
void WebGLCompatibilityTest::validateTexImageExtensionFormat(GLenum format,
                                                             const std::string &extName)
{
    // Verify texture format fails by default.
    glTexImage2D(GL_TEXTURE_2D, 0, format, 1, 1, 0, format, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable(extName))
    {
        // Verify texture format is allowed once extension is enabled.
        glRequestExtensionANGLE(extName.c_str());
        EXPECT_TRUE(IsGLExtensionEnabled(extName));

        glTexImage2D(GL_TEXTURE_2D, 0, format, 1, 1, 0, format, GL_UNSIGNED_BYTE, nullptr);
        ASSERT_GL_NO_ERROR();
    }
}

// Test enabling various non-compressed texture format extensions
TEST_P(WebGLCompatibilityTest, EnableTextureFormatExtensions)
{
    ANGLE_SKIP_TEST_IF(IsOzone());
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() != 2);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    // Verify valid format is allowed.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    // Verify invalid format fails.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA32F, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Verify formats from enableable extensions.
    if (!IsOpenGLES())
    {
        validateTexImageExtensionFormat(GL_RED_EXT, "GL_EXT_texture_rg");
    }

    validateTexImageExtensionFormat(GL_SRGB_EXT, "GL_EXT_texture_sRGB");
    validateTexImageExtensionFormat(GL_BGRA_EXT, "GL_EXT_texture_format_BGRA8888");
}

void WebGLCompatibilityTest::validateCompressedTexImageExtensionFormat(GLenum format,
                                                                       GLsizei width,
                                                                       GLsizei height,
                                                                       GLsizei blockSize,
                                                                       const std::string &extName,
                                                                       bool subImageAllowed)
{
    std::vector<GLubyte> data(blockSize, 0u);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    // Verify texture format fails by default.
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, blockSize, data.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (IsGLExtensionRequestable(extName))
    {
        // Verify texture format is allowed once extension is enabled.
        glRequestExtensionANGLE(extName.c_str());
        EXPECT_TRUE(IsGLExtensionEnabled(extName));

        glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, blockSize, data.data());
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, blockSize,
                                  data.data());
        if (subImageAllowed)
        {
            EXPECT_GL_NO_ERROR();
        }
        else
        {
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        }
    }
}

GLint WebGLCompatibilityTest::expectedByteLength(GLenum format, GLsizei width, GLsizei height)
{
    switch (format)
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
            return ((width + 3) / 4) * ((height + 3) / 4) * 8;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_RGBA_BPTC_UNORM_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT:
            return ((width + 3) / 4) * ((height + 3) / 4) * 16;
    }

    UNREACHABLE();
    return 0;
}

void WebGLCompatibilityTest::testCompressedTexLevelDimension(GLenum format,
                                                             GLint level,
                                                             GLsizei width,
                                                             GLsizei height,
                                                             GLsizei expectedByteLength,
                                                             GLenum expectedError,
                                                             const char *explanation)
{
    std::vector<uint8_t> tempVector(expectedByteLength, 0);

    EXPECT_GL_NO_ERROR();

    GLTexture sourceTexture;
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, expectedByteLength,
                           tempVector.data());
    if (expectedError == 0)
    {
        EXPECT_GL_NO_ERROR() << explanation;
    }
    else
    {
        EXPECT_GL_ERROR(expectedError) << explanation;
    }

    if (level == 0 && width > 0)
    {
        GLTexture sourceTextureStorage;
        glBindTexture(GL_TEXTURE_2D, sourceTextureStorage);

        if (getClientMajorVersion() >= 3)
        {
            glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
            if (expectedError == 0)
            {
                EXPECT_GL_NO_ERROR() << explanation << " (texStorage2D)";
            }
            else
            {
                EXPECT_GL_ERROR(expectedError) << explanation << " (texStorage2D)";
            }
        }
        else
        {
            if (IsGLExtensionRequestable("GL_EXT_texture_storage"))
            {
                glRequestExtensionANGLE("GL_EXT_texture_storage");
                ASSERT_TRUE(IsGLExtensionEnabled("GL_EXT_texture_storage"));

                glTexStorage2DEXT(GL_TEXTURE_2D, 1, format, width, height);
                if (expectedError == 0)
                {
                    EXPECT_GL_NO_ERROR() << explanation << " (texStorage2DEXT)";
                }
                else
                {
                    EXPECT_GL_ERROR(expectedError) << explanation << " (texStorage2DEXT)";
                }
            }
        }
    }
}

void WebGLCompatibilityTest::testCompressedTexImage(GLenum format)
{
    struct TestCase
    {
        GLint level;
        GLsizei width;
        GLsizei height;
        GLenum expectedError;
        const char *explanation;
    };

    constexpr TestCase testCases[] = {
        {0, 4, 3, GL_INVALID_OPERATION, "level is 0, height is not a multiple of 4"},
        {0, 3, 4, GL_INVALID_OPERATION, "level is 0, width is not a multiple of 4"},
        {0, 2, 2, GL_INVALID_OPERATION, "level is 0, width is not a multiple of 4"},
        {0, 4, 4, GL_NO_ERROR, "is valid"},
        {1, 1, 1, GL_INVALID_OPERATION, "implied base mip 2x2 is invalid"},
        {1, 1, 2, GL_INVALID_OPERATION, "implied base mip 2x4 is invalid"},
        {1, 2, 1, GL_INVALID_OPERATION, "implied base mip 4x2 is invalid"},
        {1, 2, 2, GL_NO_ERROR, "implied base mip 4x4 is valid"},
    };

    constexpr TestCase webgl2TestCases[] = {
        {0, 0, 0, GL_NO_ERROR, "0: 0x0 is valid"},
        {0, 1, 1, GL_INVALID_OPERATION, "0: 1x1 is invalid"},
        {0, 2, 2, GL_INVALID_OPERATION, "0: 2x2 is invalid"},
        {0, 3, 3, GL_INVALID_OPERATION, "0: 3x3 is invalid"},
        {0, 10, 10, GL_INVALID_OPERATION, "0: 10x10 is invalid"},
        {0, 11, 11, GL_INVALID_OPERATION, "0: 11x11 is invalid"},
        {0, 11, 12, GL_INVALID_OPERATION, "0: 11x12 is invalid"},
        {0, 12, 11, GL_INVALID_OPERATION, "0: 12x11 is invalid"},
        {0, 12, 12, GL_NO_ERROR, "0: 12x12 is valid"},
        {1, 0, 0, GL_NO_ERROR, "1: 0x0 is valid"},
        {1, 3, 3, GL_INVALID_OPERATION, "1: 3x3 is invalid"},
        {1, 5, 5, GL_INVALID_OPERATION, "1: 5x5 is invalid"},
        {1, 5, 6, GL_INVALID_OPERATION, "1: 5x6 is invalid"},
        {1, 6, 5, GL_INVALID_OPERATION, "1: 6x5 is invalid"},
        {1, 6, 6, GL_NO_ERROR, "1: 6x6 is valid"},
        {2, 0, 0, GL_NO_ERROR, "2: 0x0 is valid"},
        {2, 3, 3, GL_NO_ERROR, "2: 3x3 is valid"},
        {3, 1, 3, GL_NO_ERROR, "3: 1x3 is valid"},
        {3, 1, 1, GL_NO_ERROR, "3: 1x1 is valid"},
        {2, 1, 3, GL_NO_ERROR, "implied base mip 4x12 is valid"},
    };

    for (const TestCase &test : testCases)
    {
        testCompressedTexLevelDimension(format, test.level, test.width, test.height,
                                        expectedByteLength(format, test.width, test.height),
                                        test.expectedError, test.explanation);
    }

    if (getClientMajorVersion() >= 3)
    {
        for (const TestCase &test : webgl2TestCases)
        {
            testCompressedTexLevelDimension(format, test.level, test.width, test.height,
                                            expectedByteLength(format, test.width, test.height),
                                            test.expectedError, test.explanation);
        }
    }
}

// Test enabling GL_EXT_texture_compression_dxt1 for GL_COMPRESSED_RGB_S3TC_DXT1_EXT
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionDXT1RGB)
{
    validateCompressedTexImageExtensionFormat(GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4, 8,
                                              "GL_EXT_texture_compression_dxt1", true);
}

// Test enabling GL_EXT_texture_compression_dxt1 for GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionDXT1RGBA)
{
    validateCompressedTexImageExtensionFormat(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 4, 4, 8,
                                              "GL_EXT_texture_compression_dxt1", true);
}

// Test enabling GL_ANGLE_texture_compression_dxt3
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionDXT3)
{
    validateCompressedTexImageExtensionFormat(GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, 4, 4, 16,
                                              "GL_ANGLE_texture_compression_dxt3", true);
}

// Test enabling GL_ANGLE_texture_compression_dxt5
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionDXT5)
{
    validateCompressedTexImageExtensionFormat(GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, 4, 4, 16,
                                              "GL_ANGLE_texture_compression_dxt5", true);
}

// Test enabling GL_EXT_texture_compression_s3tc_srgb for GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionDXT1SRGB)
{
    validateCompressedTexImageExtensionFormat(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, 4, 4, 8,
                                              "GL_EXT_texture_compression_s3tc_srgb", true);
}

// Test enabling GL_EXT_texture_compression_s3tc_srgb for GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionDXT1SRGBA)
{
    validateCompressedTexImageExtensionFormat(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 4, 4, 8,
                                              "GL_EXT_texture_compression_s3tc_srgb", true);
}

// Test enabling GL_EXT_texture_compression_s3tc_srgb for GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionDXT3SRGBA)
{
    validateCompressedTexImageExtensionFormat(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 4, 4, 16,
                                              "GL_EXT_texture_compression_s3tc_srgb", true);
}

// Test enabling GL_EXT_texture_compression_s3tc_srgb for GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionDXT5SRGBA)
{
    validateCompressedTexImageExtensionFormat(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 4, 4, 16,
                                              "GL_EXT_texture_compression_s3tc_srgb", true);
}

// Test enabling GL_OES_compressed_ETC1_RGB8_texture
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionETC1)
{
    validateCompressedTexImageExtensionFormat(
        GL_ETC1_RGB8_OES, 4, 4, 8, "GL_OES_compressed_ETC1_RGB8_texture",
        IsGLExtensionEnabled("GL_EXT_compressed_ETC1_RGB8_sub_texture"));
}

// Test enabling GL_ANGLE_lossy_etc_decode
TEST_P(WebGLCompatibilityTest, EnableCompressedTextureExtensionLossyDecode)
{
    validateCompressedTexImageExtensionFormat(GL_ETC1_RGB8_LOSSY_DECODE_ANGLE, 4, 4, 8,
                                              "GL_ANGLE_lossy_etc_decode", true);
}

// Reject attempts to allocate too-large arrays in shaders.
// This is an implementation-defined limit - crbug.com/1220237 .
TEST_P(WebGLCompatibilityTest, ValidateArraySizes)
{
    // Note: on macOS with ANGLE's OpenGL backend, getting anywhere
    // close to this limit causes pathologically slow shader
    // compilation in the driver. For the "ok" case, therefore, use a
    // fairly small array.
    constexpr char kVSArrayOK[] =
        R"(varying vec4 color;
const int array_size = 500;
void main()
{
    mat2 array[array_size];
    mat2 array2[array_size];
    if (array[0][0][0] + array2[0][0][0] == 2.0)
        color = vec4(0.0, 1.0, 0.0, 1.0);
    else
        color = vec4(1.0, 0.0, 0.0, 1.0);
})";

    constexpr char kVSArrayTooLarge[] =
        R"(varying vec4 color;
// 16 MB / 32 aligned bytes per mat2 = 524288
const int array_size = 524289;
void main()
{
    mat2 array[array_size];
    if (array[0][0][0] == 2.0)
        color = vec4(0.0, 1.0, 0.0, 1.0);
    else
        color = vec4(1.0, 0.0, 0.0, 1.0);
})";

    constexpr char kVSArrayMuchTooLarge[] =
        R"(varying vec4 color;
const int array_size = 757000;
void main()
{
    mat2 array[array_size];
    if (array[0][0][0] == 2.0)
        color = vec4(0.0, 1.0, 0.0, 1.0);
    else
        color = vec4(1.0, 0.0, 0.0, 1.0);
})";

    constexpr char kFS[] =
        R"(precision mediump float;
varying vec4 color;
void main()
{
    gl_FragColor = vec4(color.r - 0.5, 0.0, 0.0, 1.0);
})";

    GLuint program = CompileProgram(kVSArrayOK, kFS);
    EXPECT_NE(0u, program);

    program = CompileProgram(kVSArrayTooLarge, kFS);
    EXPECT_EQ(0u, program);

    program = CompileProgram(kVSArrayMuchTooLarge, kFS);
    EXPECT_EQ(0u, program);
}

// Reject attempts to allocate too-large structs in shaders.
// This is an implementation-defined limit - crbug.com/1220237 .
TEST_P(WebGLCompatibilityTest, ValidateStructSizes)
{
    // Note: on macOS with ANGLE's OpenGL backend, getting anywhere
    // close to this limit causes pathologically slow shader
    // compilation in the driver. For this reason, only perform a
    // negative test.
    constexpr char kFSStructTooLarge[] =
        R"(precision mediump float;
struct Light {
// 2 GB / 32 aligned bytes per mat2 = 67108864
mat2 array[67108865];
};

uniform Light light;

void main()
{
    if (light.array[0][0][0] == 2.0)
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    GLuint program = CompileProgram(essl1_shaders::vs::Simple(), kFSStructTooLarge);
    EXPECT_EQ(0u, program);

    // A second variation where the large array is on the variable itself not a member.
    constexpr char kFSStructTooLarge2[] =
        R"(precision mediump float;
struct Light {
mat2 array;
};

uniform Light light[67108865];

void main()
{
    if (light[0].array[0][0] == 2.0)
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    program = CompileProgram(essl1_shaders::vs::Simple(), kFSStructTooLarge2);
    EXPECT_EQ(0u, program);
}

// Reject attempts to allocate too much private memory.
// This is an implementation-defined limit - crbug.com/1431761.
TEST_P(WebGLCompatibilityTest, ValidateTotalPrivateSize)
{
    constexpr char kTooLargeGlobalMemory1[] =
        R"(precision mediump float;

// 16 MB / 16 bytes per vec4 = 1048576
vec4 array[524288];
vec4 array2[524289];

void main()
{
    if (array[0].x + array[1].x == 0.)
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    constexpr char kTooLargeGlobalMemory2[] =
        R"(precision mediump float;

// 16 MB / 16 bytes per vec4 = 1048576
vec4 array[524287];
vec4 array2[524287];
vec4 x, y, z;

void main()
{
    if (array[0].x + array[1].x == x.w + y.w + z.w)
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    constexpr char kTooLargeGlobalAndLocalMemory1[] =
        R"(precision mediump float;

// 16 MB / 16 bytes per vec4 = 1048576
vec4 array[524288];

void main()
{
    vec4 array2[524289];
    if (array[0].x + array[1].x == 2.0)
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    // Note: The call stack is not taken into account for the purposes of total memory calculation.
    constexpr char kTooLargeGlobalAndLocalMemory2[] =
        R"(precision mediump float;

// 16 MB / 16 bytes per vec4 = 1048576
vec4 array[524288];

float f()
{
    vec4 array2[524288];
    return array2[0].x;
}

float g()
{
    vec4 array3[524287];
    return array3[0].x;
}

float h()
{
    vec4 value;
    float value2;
    return value.x + value2;
}

void main()
{
    if (array[0].x + f() + g() + h() == 2.0)
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    constexpr char kTooLargeGlobalMemoryOverflow[] =
        R"(precision mediump float;

// 16 MB / 16 bytes per vec4 = 1048576
// Create 256 arrays so each is small, but the total overflows a 32-bit number
vec4 array[1048576], array2[1048576], array3[1048576], array4[1048576], array5[1048576];
vec4 array6[1048576], array7[1048576], array8[1048576], array9[1048576], array10[1048576];
vec4 array11[1048576], array12[1048576], array13[1048576], array14[1048576], array15[1048576];
vec4 array16[1048576], array17[1048576], array18[1048576], array19[1048576], array20[1048576];
vec4 array21[1048576], array22[1048576], array23[1048576], array24[1048576], array25[1048576];
vec4 array26[1048576], array27[1048576], array28[1048576], array29[1048576], array30[1048576];
vec4 array31[1048576], array32[1048576], array33[1048576], array34[1048576], array35[1048576];
vec4 array36[1048576], array37[1048576], array38[1048576], array39[1048576], array40[1048576];
vec4 array41[1048576], array42[1048576], array43[1048576], array44[1048576], array45[1048576];
vec4 array46[1048576], array47[1048576], array48[1048576], array49[1048576], array50[1048576];
vec4 array51[1048576], array52[1048576], array53[1048576], array54[1048576], array55[1048576];
vec4 array56[1048576], array57[1048576], array58[1048576], array59[1048576], array60[1048576];
vec4 array61[1048576], array62[1048576], array63[1048576], array64[1048576], array65[1048576];
vec4 array66[1048576], array67[1048576], array68[1048576], array69[1048576], array70[1048576];
vec4 array71[1048576], array72[1048576], array73[1048576], array74[1048576], array75[1048576];
vec4 array76[1048576], array77[1048576], array78[1048576], array79[1048576], array80[1048576];
vec4 array81[1048576], array82[1048576], array83[1048576], array84[1048576], array85[1048576];
vec4 array86[1048576], array87[1048576], array88[1048576], array89[1048576], array90[1048576];
vec4 array91[1048576], array92[1048576], array93[1048576], array94[1048576], array95[1048576];
vec4 array96[1048576], array97[1048576], array98[1048576], array99[1048576], array100[1048576];
vec4 array101[1048576], array102[1048576], array103[1048576], array104[1048576], array105[1048576];
vec4 array106[1048576], array107[1048576], array108[1048576], array109[1048576], array110[1048576];
vec4 array111[1048576], array112[1048576], array113[1048576], array114[1048576], array115[1048576];
vec4 array116[1048576], array117[1048576], array118[1048576], array119[1048576], array120[1048576];
vec4 array121[1048576], array122[1048576], array123[1048576], array124[1048576], array125[1048576];
vec4 array126[1048576], array127[1048576], array128[1048576], array129[1048576], array130[1048576];
vec4 array131[1048576], array132[1048576], array133[1048576], array134[1048576], array135[1048576];
vec4 array136[1048576], array137[1048576], array138[1048576], array139[1048576], array140[1048576];
vec4 array141[1048576], array142[1048576], array143[1048576], array144[1048576], array145[1048576];
vec4 array146[1048576], array147[1048576], array148[1048576], array149[1048576], array150[1048576];
vec4 array151[1048576], array152[1048576], array153[1048576], array154[1048576], array155[1048576];
vec4 array156[1048576], array157[1048576], array158[1048576], array159[1048576], array160[1048576];
vec4 array161[1048576], array162[1048576], array163[1048576], array164[1048576], array165[1048576];
vec4 array166[1048576], array167[1048576], array168[1048576], array169[1048576], array170[1048576];
vec4 array171[1048576], array172[1048576], array173[1048576], array174[1048576], array175[1048576];
vec4 array176[1048576], array177[1048576], array178[1048576], array179[1048576], array180[1048576];
vec4 array181[1048576], array182[1048576], array183[1048576], array184[1048576], array185[1048576];
vec4 array186[1048576], array187[1048576], array188[1048576], array189[1048576], array190[1048576];
vec4 array191[1048576], array192[1048576], array193[1048576], array194[1048576], array195[1048576];
vec4 array196[1048576], array197[1048576], array198[1048576], array199[1048576], array200[1048576];
vec4 array201[1048576], array202[1048576], array203[1048576], array204[1048576], array205[1048576];
vec4 array206[1048576], array207[1048576], array208[1048576], array209[1048576], array210[1048576];
vec4 array211[1048576], array212[1048576], array213[1048576], array214[1048576], array215[1048576];
vec4 array216[1048576], array217[1048576], array218[1048576], array219[1048576], array220[1048576];
vec4 array221[1048576], array222[1048576], array223[1048576], array224[1048576], array225[1048576];
vec4 array226[1048576], array227[1048576], array228[1048576], array229[1048576], array230[1048576];
vec4 array231[1048576], array232[1048576], array233[1048576], array234[1048576], array235[1048576];
vec4 array236[1048576], array237[1048576], array238[1048576], array239[1048576], array240[1048576];
vec4 array241[1048576], array242[1048576], array243[1048576], array244[1048576], array245[1048576];
vec4 array246[1048576], array247[1048576], array248[1048576], array249[1048576], array250[1048576];
vec4 array251[1048576], array252[1048576], array253[1048576], array254[1048576], array255[1048576];
vec4 array256[1048576];

void main()
{
    float f = array[0].x; f += array2[0].x; f += array3[0].x; f += array4[0].x; f += array5[0].x;
    f += array6[0].x; f += array7[0].x; f += array8[0].x; f += array9[0].x; f += array10[0].x;
    f += array11[0].x; f += array12[0].x; f += array13[0].x; f += array14[0].x; f += array15[0].x;
    f += array16[0].x; f += array17[0].x; f += array18[0].x; f += array19[0].x; f += array20[0].x;
    f += array21[0].x; f += array22[0].x; f += array23[0].x; f += array24[0].x; f += array25[0].x;
    f += array26[0].x; f += array27[0].x; f += array28[0].x; f += array29[0].x; f += array30[0].x;
    f += array31[0].x; f += array32[0].x; f += array33[0].x; f += array34[0].x; f += array35[0].x;
    f += array36[0].x; f += array37[0].x; f += array38[0].x; f += array39[0].x; f += array40[0].x;
    f += array41[0].x; f += array42[0].x; f += array43[0].x; f += array44[0].x; f += array45[0].x;
    f += array46[0].x; f += array47[0].x; f += array48[0].x; f += array49[0].x; f += array50[0].x;
    f += array51[0].x; f += array52[0].x; f += array53[0].x; f += array54[0].x; f += array55[0].x;
    f += array56[0].x; f += array57[0].x; f += array58[0].x; f += array59[0].x; f += array60[0].x;
    f += array61[0].x; f += array62[0].x; f += array63[0].x; f += array64[0].x; f += array65[0].x;
    f += array66[0].x; f += array67[0].x; f += array68[0].x; f += array69[0].x; f += array70[0].x;
    f += array71[0].x; f += array72[0].x; f += array73[0].x; f += array74[0].x; f += array75[0].x;
    f += array76[0].x; f += array77[0].x; f += array78[0].x; f += array79[0].x; f += array80[0].x;
    f += array81[0].x; f += array82[0].x; f += array83[0].x; f += array84[0].x; f += array85[0].x;
    f += array86[0].x; f += array87[0].x; f += array88[0].x; f += array89[0].x; f += array90[0].x;
    f += array91[0].x; f += array92[0].x; f += array93[0].x; f += array94[0].x; f += array95[0].x;
    f += array96[0].x; f += array97[0].x; f += array98[0].x; f += array99[0].x; f += array100[0].x;
    f += array101[0].x; f += array102[0].x; f += array103[0].x; f += array104[0].x;
    f += array105[0].x; f += array106[0].x; f += array107[0].x; f += array108[0].x;
    f += array109[0].x; f += array110[0].x; f += array111[0].x; f += array112[0].x;
    f += array113[0].x; f += array114[0].x; f += array115[0].x; f += array116[0].x;
    f += array117[0].x; f += array118[0].x; f += array119[0].x; f += array120[0].x;
    f += array121[0].x; f += array122[0].x; f += array123[0].x; f += array124[0].x;
    f += array125[0].x; f += array126[0].x; f += array127[0].x; f += array128[0].x;
    f += array129[0].x; f += array130[0].x; f += array131[0].x; f += array132[0].x;
    f += array133[0].x; f += array134[0].x; f += array135[0].x; f += array136[0].x;
    f += array137[0].x; f += array138[0].x; f += array139[0].x; f += array140[0].x;
    f += array141[0].x; f += array142[0].x; f += array143[0].x; f += array144[0].x;
    f += array145[0].x; f += array146[0].x; f += array147[0].x; f += array148[0].x;
    f += array149[0].x; f += array150[0].x; f += array151[0].x; f += array152[0].x;
    f += array153[0].x; f += array154[0].x; f += array155[0].x; f += array156[0].x;
    f += array157[0].x; f += array158[0].x; f += array159[0].x; f += array160[0].x;
    f += array161[0].x; f += array162[0].x; f += array163[0].x; f += array164[0].x;
    f += array165[0].x; f += array166[0].x; f += array167[0].x; f += array168[0].x;
    f += array169[0].x; f += array170[0].x; f += array171[0].x; f += array172[0].x;
    f += array173[0].x; f += array174[0].x; f += array175[0].x; f += array176[0].x;
    f += array177[0].x; f += array178[0].x; f += array179[0].x; f += array180[0].x;
    f += array181[0].x; f += array182[0].x; f += array183[0].x; f += array184[0].x;
    f += array185[0].x; f += array186[0].x; f += array187[0].x; f += array188[0].x;
    f += array189[0].x; f += array190[0].x; f += array191[0].x; f += array192[0].x;
    f += array193[0].x; f += array194[0].x; f += array195[0].x; f += array196[0].x;
    f += array197[0].x; f += array198[0].x; f += array199[0].x; f += array200[0].x;
    f += array201[0].x; f += array202[0].x; f += array203[0].x; f += array204[0].x;
    f += array205[0].x; f += array206[0].x; f += array207[0].x; f += array208[0].x;
    f += array209[0].x; f += array210[0].x; f += array211[0].x; f += array212[0].x;
    f += array213[0].x; f += array214[0].x; f += array215[0].x; f += array216[0].x;
    f += array217[0].x; f += array218[0].x; f += array219[0].x; f += array220[0].x;
    f += array221[0].x; f += array222[0].x; f += array223[0].x; f += array224[0].x;
    f += array225[0].x; f += array226[0].x; f += array227[0].x; f += array228[0].x;
    f += array229[0].x; f += array230[0].x; f += array231[0].x; f += array232[0].x;
    f += array233[0].x; f += array234[0].x; f += array235[0].x; f += array236[0].x;
    f += array237[0].x; f += array238[0].x; f += array239[0].x; f += array240[0].x;
    f += array241[0].x; f += array242[0].x; f += array243[0].x; f += array244[0].x;
    f += array245[0].x; f += array246[0].x; f += array247[0].x; f += array248[0].x;
    f += array249[0].x; f += array250[0].x; f += array251[0].x; f += array252[0].x;
    f += array253[0].x; f += array254[0].x; f += array255[0].x; f += array256[0].x;
    if (f == 2.0)
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    GLuint program = CompileProgram(essl1_shaders::vs::Simple(), kTooLargeGlobalMemory1);
    EXPECT_EQ(0u, program);

    program = CompileProgram(essl1_shaders::vs::Simple(), kTooLargeGlobalMemory2);
    EXPECT_EQ(0u, program);

    program = CompileProgram(essl1_shaders::vs::Simple(), kTooLargeGlobalAndLocalMemory1);
    EXPECT_EQ(0u, program);

    program = CompileProgram(essl1_shaders::vs::Simple(), kTooLargeGlobalAndLocalMemory2);
    EXPECT_EQ(0u, program);

    program = CompileProgram(essl1_shaders::vs::Simple(), kTooLargeGlobalMemoryOverflow);
    EXPECT_EQ(0u, program);
}

// Linking should fail when corresponding vertex/fragment uniform blocks have different precision
// qualifiers.
TEST_P(WebGL2CompatibilityTest, UniformBlockPrecisionMismatch)
{
    constexpr char kVS[] =
        R"(#version 300 es
uniform Block { mediump vec4 val; };
void main() { gl_Position = val; })";
    constexpr char kFS[] =
        R"(#version 300 es
uniform Block { highp vec4 val; };
out highp vec4 out_FragColor;
void main() { out_FragColor = val; })";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    ASSERT_NE(0u, vs);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    ASSERT_NE(0u, fs);

    GLuint program = glCreateProgram();

    glAttachShader(program, vs);
    glDeleteShader(vs);
    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    ASSERT_EQ(0, linkStatus);

    glDeleteProgram(program);
}

// Test no attribute vertex shaders
TEST_P(WebGL2CompatibilityTest, NoAttributeVertexShader)
{
    constexpr char kVS[] =
        R"(#version 300 es
void main()
{

    ivec2 xy = ivec2(gl_VertexID % 2, (gl_VertexID / 2 + gl_VertexID / 3) % 2);
    gl_Position = vec4(vec2(xy) * 2. - 1., 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl3_shaders::fs::Red());
    glUseProgram(program);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Tests bindAttribLocations for length limit
TEST_P(WebGL2CompatibilityTest, BindAttribLocationLimitation)
{
    constexpr int maxLocStringLength = 1024;
    const std::string tooLongString(maxLocStringLength + 1, '_');

    glBindAttribLocation(0, 0, static_cast<const GLchar *>(tooLongString.c_str()));

    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Tests getAttribLocation for length limit
TEST_P(WebGL2CompatibilityTest, GetAttribLocationLengthLimitation)
{
    constexpr int maxLocStringLength = 1024;
    const std::string tooLongString(maxLocStringLength + 1, '_');

    glGetAttribLocation(0, static_cast<const GLchar *>(tooLongString.c_str()));

    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Covers a bug in transform feedback loop detection.
TEST_P(WebGL2CompatibilityTest, TransformFeedbackCheckNullDeref)
{
    constexpr char kVS[] = R"(attribute vec4 color; void main() { color.r; })";
    constexpr char kFS[] = R"(void main(){})";
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    glEnableVertexAttribArray(0);
    glDrawArrays(GL_POINTS, 0, 1);

    // This should fail because it is trying to pull a vertex with no buffer.
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    // This should fail because it is trying to pull a vertex from an empty buffer.
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// We should forbid two transform feedback outputs going to the same buffer.
TEST_P(WebGL2CompatibilityTest, TransformFeedbackDoubleBinding)
{
    constexpr char kVS[] =
        R"(attribute float a; varying float b; varying float c; void main() { b = a; c = a; })";
    constexpr char kFS[] = R"(void main(){})";
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    static const char *varyings[] = {"b", "c"};
    glTransformFeedbackVaryings(program, 2, varyings, GL_SEPARATE_ATTRIBS);
    glLinkProgram(program);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    // Bind the transform feedback varyings to non-overlapping regions of the same buffer.
    GLBuffer buffer;
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer, 0, 4);
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 1, buffer, 4, 4);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 8, nullptr, GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();
    // Two varyings bound to the same buffer should be an error.
    glBeginTransformFeedback(GL_POINTS);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Check the return type of a given parameter upon getting the active uniforms.
TEST_P(WebGL2CompatibilityTest, UniformVariablesReturnTypes)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());

    std::vector<GLuint> validUniformIndices = {0};
    std::vector<GLint> uniformNameLengthBuf(validUniformIndices.size());

    // This should fail because GL_UNIFORM_NAME_LENGTH cannot be used in WebGL2.
    glGetActiveUniformsiv(program, static_cast<GLsizei>(validUniformIndices.size()),
                          &validUniformIndices[0], GL_UNIFORM_NAME_LENGTH,
                          &uniformNameLengthBuf[0]);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Tests an error case to ensure we don't crash.
TEST_P(WebGLCompatibilityTest, DrawWithNoProgram)
{
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Ensures that rendering to different texture levels of a sampled texture is supported.
TEST_P(WebGL2CompatibilityTest, RenderToLevelsOfSampledTexture)
{
    // TODO: Fix on Vulkan back-end. http://anglebug.com/40644733
    ANGLE_SKIP_TEST_IF(IsVulkan());

    constexpr GLsizei kTexSize   = 2;
    constexpr GLsizei kTexLevels = 2;

    std::vector<GLColor> texData(kTexSize * kTexSize, GLColor::green);

    GLTexture sourceTexture;
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, kTexLevels, GL_RGBA8, kTexSize, kTexSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize, GL_RGBA, GL_UNSIGNED_BYTE,
                    texData.data());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sourceTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    glViewport(0, 0, kTexSize / 2, kTexSize / 2);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    ASSERT_GL_NO_ERROR();

    // Should work - drawing from level 0 to level 1.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Should not work - drawing from levels [0,1] to level 1.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Should work - drawing with levels [0,1] to default FBO.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Reject attempts to allocate too-large variables in shaders.
// This is an implementation-defined limit - crbug.com/1220237 .
TEST_P(WebGL2CompatibilityTest, ValidateTypeSizes)
{
    constexpr char kFSArrayBlockTooLarge[] = R"(#version 300 es
precision mediump float;
// 1 + the maximum size this implementation allows.
uniform LargeArrayBlock {
    vec4 large_array[134217729];
};

out vec4 out_FragColor;

void main()
{
    if (large_array[1].x == 2.0)
        out_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else
        out_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

    GLuint program = CompileProgram(essl3_shaders::vs::Simple(), kFSArrayBlockTooLarge);
    EXPECT_EQ(0u, program);
}

// Ensure that new type size validation code added for
// crbug.com/1220237 does not crash.
TEST_P(WebGL2CompatibilityTest, ValidatingTypeSizesShouldNotCrash)
{
    constexpr char kFS1[] = R"(#version 300 es
precision mediump float;
out vec4 my_FragColor;

const vec4 constants[2] = vec4[] (
    vec4(0.6, 0.3, 0.0, 3.0),
    vec4(-0.6, 0.7, 0.0, -2.0)
);

void main()
{
    my_FragColor = constants[0] + constants[1];
    return;
})";

    constexpr char kFS2[] = R"(#version 300 es
precision mediump float;
out vec4 my_FragColor;

const vec4 constants[2] = vec4[] (
    vec4(0.6, 0.3, 0.0, 3.0),
    vec4(-0.6, 0.7, 0.0, -2.0)
);

const vec4 constants2[2] = vec4[] (
    constants[1],
    constants[0]
);

void main()
{
    my_FragColor = constants2[0] + constants2[1];
    return;
})";

    constexpr char kFS3[] = R"(#version 300 es
precision mediump float;
out vec4 my_FragColor;

const vec4 constants[2] = vec4[] (
    vec4(0.6, 0.3, 0.0, 3.0),
    vec4(-0.6, 0.7, 0.0, -2.0)
);

const vec4 constants2[2] = constants;

void main()
{
    my_FragColor = constants2[0] + constants2[1];
    return;
})";

    GLuint program = CompileProgram(essl3_shaders::vs::Simple(), kFS1);
    EXPECT_NE(0u, program);

    program = CompileProgram(essl3_shaders::vs::Simple(), kFS2);
    EXPECT_NE(0u, program);

    program = CompileProgram(essl3_shaders::vs::Simple(), kFS3);
    EXPECT_NE(0u, program);
}

// Verify glReadPixels will accept GL_RGBX8_ANGLE + GL_UNSIGNED_BYTE.
TEST_P(WebGL2CompatibilityTest, ReadPixelsRgbx8AngleUnsignedByte)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_rgbx_internal_format"));

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBX8_ANGLE, 1, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    GLColor pixel;
    glReadPixels(0, 0, 1, 1, GL_RGBX8_ANGLE, GL_UNSIGNED_BYTE, &pixel.R);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::red, pixel);
}

// Test that masked-out draw attachments do not require fragment outputs.
TEST_P(WebGL2CompatibilityTest, DrawWithMaskedOutAttachments)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_draw_buffers_indexed"));

    GLFramebuffer fbo;
    GLRenderbuffer rbo[2];
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[0]);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rbo[1]);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    constexpr char kFS[] = R"(#version 300 es
precision highp float;

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, bufs);

    // Error: no fragment output for attachment1
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // No error: attachment1 is masked-out
    glColorMaskiOES(1, false, false, false, false);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();
}

// Test that ETC2/EAC formats are rejected by unextended WebGL 2.0 contexts.
TEST_P(WebGL2CompatibilityTest, ETC2EACFormats)
{
    size_t byteLength          = 8;
    constexpr uint8_t data[16] = {};
    constexpr GLenum formats[] = {GL_COMPRESSED_R11_EAC,
                                  GL_COMPRESSED_SIGNED_R11_EAC,
                                  GL_COMPRESSED_RGB8_ETC2,
                                  GL_COMPRESSED_SRGB8_ETC2,
                                  GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
                                  GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
                                  GL_COMPRESSED_RG11_EAC,
                                  GL_COMPRESSED_SIGNED_RG11_EAC,
                                  GL_COMPRESSED_RGBA8_ETC2_EAC,
                                  GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC};

    for (const auto &fmt : formats)
    {
        if (fmt == GL_COMPRESSED_RG11_EAC)
            byteLength = 16;

        {
            GLTexture tex;
            glBindTexture(GL_TEXTURE_2D, tex);
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, fmt, 4, 4, 0, byteLength, data);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);
        }

        {
            GLTexture tex;
            glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
            glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 0, fmt, 4, 4, 1, 0, byteLength, data);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);
        }

        {
            GLTexture tex;
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexStorage2D(GL_TEXTURE_2D, 1, fmt, 4, 4);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);
        }

        {
            GLTexture tex;
            glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, fmt, 4, 4, 1);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);
        }
    }
}

// Test that GL_HALF_FLOAT_OES type is rejected by WebGL 2.0 contexts.
TEST_P(WebGL2CompatibilityTest, HalfFloatOesType)
{
    const std::array<std::pair<GLenum, GLenum>, 6> formats = {{{GL_R16F, GL_RED},
                                                               {GL_RG16F, GL_RG},
                                                               {GL_RGB16F, GL_RGB},
                                                               {GL_RGBA16F, GL_RGBA},
                                                               {GL_R11F_G11F_B10F, GL_RGB},
                                                               {GL_RGB9_E5, GL_RGB}}};
    for (const auto &fmt : formats)
    {
        {
            GLTexture tex;
            glBindTexture(GL_TEXTURE_2D, tex);
            EXPECT_GL_NO_ERROR();

            glTexImage2D(GL_TEXTURE_2D, 0, fmt.first, 1, 1, 0, fmt.second, GL_HALF_FLOAT_OES,
                         nullptr);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);

            glTexImage2D(GL_TEXTURE_2D, 0, fmt.first, 1, 1, 0, fmt.second, GL_HALF_FLOAT, nullptr);
            EXPECT_GL_NO_ERROR();
        }
        {
            GLTexture tex;
            glBindTexture(GL_TEXTURE_3D, tex);
            EXPECT_GL_NO_ERROR();

            glTexImage3D(GL_TEXTURE_3D, 0, fmt.first, 1, 1, 1, 0, fmt.second, GL_HALF_FLOAT_OES,
                         nullptr);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);

            glTexImage3D(GL_TEXTURE_3D, 0, fmt.first, 1, 1, 1, 0, fmt.second, GL_HALF_FLOAT,
                         nullptr);
            EXPECT_GL_NO_ERROR();
        }
    }
}

// Test that unsigned integer samplers work with stencil textures.
TEST_P(WebGL2CompatibilityTest, StencilTexturingStencil8)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_texture_stencil8"));

    const uint8_t stencilValue = 42;
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, 1, 1, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
                 &stencilValue);
    ASSERT_GL_NO_ERROR();

    constexpr char kFS[] = R"(#version 300 es
out mediump vec4 color;
uniform mediump usampler2D tex;
void main() {
    color = vec4(vec3(texture(tex, vec2(0.0, 0.0))) / 255.0, 1.0);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(42, 0, 0, 255), 1);
}

// Test that unsigned integer samplers work with combined depth/stencil textures.
TEST_P(WebGL2CompatibilityTest, StencilTexturingCombined)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_stencil_texturing"));

    const uint32_t stencilValue = 42;
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE_ANGLE, GL_STENCIL_INDEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 1, 1, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, &stencilValue);
    ASSERT_GL_NO_ERROR();

    constexpr char kFS[] = R"(#version 300 es
out mediump vec4 color;
uniform mediump usampler2D tex;
void main() {
    color = vec4(vec3(texture(tex, vec2(0.0, 0.0))) / 255.0, 1.0);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(42, 0, 0, 255), 1);
}

// Regression test for syncing internal state for TexImage calls while there is an incomplete
// framebuffer bound
TEST_P(WebGL2CompatibilityTest, TexImageSyncWithIncompleteFramebufferBug)
{
    glColorMask(false, true, false, false);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(100, 128, 65, 65537);

    GLFramebuffer fb1;
    glBindFramebuffer(GL_FRAMEBUFFER, fb1);

    GLRenderbuffer rb;
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RG8UI, 1304, 2041);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 8, 8, 0, GL_RED_EXT, GL_UNSIGNED_BYTE, nullptr);
}

// Test that "depth_unchanged" layout qualifier is rejected for WebGL contexts.
TEST_P(WebGL2CompatibilityTest, FragDepthLayoutUnchanged)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_conservative_depth"));

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_conservative_depth: enable
out highp vec4 color;
layout (depth_unchanged) out highp float gl_FragDepth;
void main() {
    color = vec4(0.0, 0.0, 0.0, 1.0);
    gl_FragDepth = 1.0;
})";

    GLProgram prg;
    prg.makeRaster(essl3_shaders::vs::Simple(), kFS);
    EXPECT_FALSE(prg.valid());
}

// Test that EXT_blend_func_extended does not allow omitting locations in WebGL 2.0 contexts.
TEST_P(WebGL2CompatibilityTest, EXTBlendFuncExtendedNoLocations)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_blend_func_extended"));

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
out highp vec4 color0;
out highp vec4 color1;
void main() {
    color0 = vec4(1.0, 0.0, 0.0, 1.0);
    color1 = vec4(0.0, 1.0, 0.0, 1.0);
})";

    GLProgram prg;
    prg.makeRaster(essl3_shaders::vs::Simple(), kFS);
    EXPECT_FALSE(prg.valid());
}

// Test that fragment outputs may be omitted when enabling
// SRC1 blend functions with all color channels masked out.
TEST_P(WebGLCompatibilityTest, EXTBlendFuncExtendedMissingOutputsWithAllChannelsMaskedOut)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_blend_func_extended"));

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC1_COLOR_EXT);
    glColorMask(false, false, false, false);

    // Secondary output missing
    {
        constexpr char kFragColor[] = R"(
            void main() {
                gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
            })";
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragColor);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0, true);
        EXPECT_GL_NO_ERROR();
    }

    // Primary output missing
    {
        constexpr char kSecondaryFragColor[] = R"(#extension GL_EXT_blend_func_extended : enable
            void main() {
                gl_SecondaryFragColorEXT = vec4(0.0, 1.0, 0.0, 1.0);
            })";
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kSecondaryFragColor);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0, true);
        EXPECT_GL_NO_ERROR();
    }

    // Both outputs missing
    {
        constexpr char kNone[] = "void main() {}";
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kNone);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0, true);
        EXPECT_GL_NO_ERROR();
    }
}

// Test that both fragment outputs must be statically used
// when enabling SRC1 blend functions in WebGL 1.0 contexts.
TEST_P(WebGLCompatibilityTest, EXTBlendFuncExtendedMissingOutputs)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_blend_func_extended"));

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC1_COLOR_EXT);
    ASSERT_GL_NO_ERROR();

    {
        constexpr char kFragColor[] = R"(
void main() {
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragColor);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    {
        constexpr char kSecondaryFragColor[] = R"(#extension GL_EXT_blend_func_extended : require
void main() {
    gl_SecondaryFragColorEXT = vec4(0.0, 1.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kSecondaryFragColor);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    {
        constexpr char kFragColorAndSecondaryFragColor[] =
            R"(#extension GL_EXT_blend_func_extended : require
void main() {
    gl_FragColor             = vec4(1.0, 0.0, 0.0, 1.0);
    gl_SecondaryFragColorEXT = vec4(0.0, 1.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragColorAndSecondaryFragColor);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();
    }
}

// Test that both fragment outputs must be statically used
// when enabling SRC1 blend functions in WebGL 1.0 contexts.
TEST_P(WebGLCompatibilityTest, EXTBlendFuncExtendedMissingOutputsArrays)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_blend_func_extended"));

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC1_COLOR_EXT);
    ASSERT_GL_NO_ERROR();

    {
        constexpr char kFragData[] = R"(
void main() {
    gl_FragData[0] = vec4(1.0, 0.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragData);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    {
        constexpr char kSecondaryFragData[] = R"(#extension GL_EXT_blend_func_extended : require
void main() {
    gl_SecondaryFragDataEXT[0] = vec4(0.0, 1.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kSecondaryFragData);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    {
        constexpr char kFragDataAndSecondaryFragData[] =
            R"(#extension GL_EXT_blend_func_extended : require
void main() {
    gl_FragData[0]             = vec4(1.0, 0.0, 0.0, 1.0);
    gl_SecondaryFragDataEXT[0] = vec4(0.0, 1.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragDataAndSecondaryFragData);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();
    }
}

// Test that both fragment outputs must be statically used
// when enabling SRC1 blend functions in WebGL 2.0 contexts.
TEST_P(WebGL2CompatibilityTest, EXTBlendFuncExtendedMissingOutputs)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_blend_func_extended"));

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC1_COLOR_EXT);
    ASSERT_GL_NO_ERROR();

    {
        constexpr char kColor0[] = R"(#version 300 es
out mediump vec4 color0;
void main() {
    color0 = vec4(1.0, 0.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kColor0);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    {
        constexpr char kColor1[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
layout(location = 0, index = 1) out mediump vec4 color1;
void main() {
    color1 = vec4(0.0, 1.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kColor1);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    {
        constexpr char kColor0AndColor1[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
layout(location = 0, index = 0) out mediump vec4 color0;
layout(location = 0, index = 1) out mediump vec4 color1;
void main() {
    color0 = vec4(1.0, 0.0, 0.0, 1.0);
    color1 = vec4(0.0, 1.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kColor0AndColor1);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();
    }
}

// Test that both fragment outputs must be statically used
// when enabling SRC1 blend functions in WebGL 2.0 contexts.
TEST_P(WebGL2CompatibilityTest, EXTBlendFuncExtendedMissingOutputsArrays)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_blend_func_extended"));

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC1_COLOR_EXT);
    ASSERT_GL_NO_ERROR();

    {
        constexpr char kArrayColor0[] = R"(#version 300 es
out mediump vec4 color0[1];
void main() {
    color0[0] = vec4(1.0, 0.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kArrayColor0);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    {
        constexpr char kArrayColor1[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
layout(location = 0, index = 1) out mediump vec4 color1[1];
void main() {
    color1[0] = vec4(0.0, 1.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kArrayColor1);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
    {
        constexpr char kArrayColor0AndColor0[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
layout(location = 0, index = 0) out mediump vec4 color0[1];
layout(location = 0, index = 1) out mediump vec4 color1[1];
void main() {
    color0[0] = vec4(1.0, 0.0, 0.0, 1.0);
    color1[0] = vec4(0.0, 1.0, 0.0, 1.0);
})";
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kArrayColor0AndColor0);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();
    }
}

// Test that vertex conversion correctly no-ops when the vertex format requires conversion but there
// are no vertices to convert.
TEST_P(WebGLCompatibilityTest, ConversionWithNoVertices)
{
    constexpr char kVS[] = R"(precision highp float;
attribute vec3 attr1;
void main(void) {
   gl_Position = vec4(attr1, 1.0);
})";

    constexpr char kFS[] = R"(precision highp float;
void main(void) {
   gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
})";

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    std::array<int8_t, 12> data = {
        1,
    };
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data[0]), data.data(), GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "attr1");
    glLinkProgram(program);
    ASSERT_TRUE(CheckLinkStatusAndReturnProgram(program, true));
    glUseProgram(program);

    // Set the offset of the attribute past the end of the buffer but use a format that requires
    // conversion in Vulkan
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_BYTE, true, 128, reinterpret_cast<void *>(256));

    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Either no error or invalid operation is okay.
}

// Tests that using an out of bounds draw offset with a dynamic array succeeds.
TEST_P(WebGLCompatibilityTest, DynamicVertexArrayOffsetOutOfBounds)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);

    glEnableVertexAttribArray(posLoc);
    GLBuffer buf;
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void *>(500));
    glBufferData(GL_ARRAY_BUFFER, 100, nullptr, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Either no error or invalid operation is okay.
}

// Covers situations where vertex conversion could read out of bounds.
TEST_P(WebGL2CompatibilityTest, OutOfBoundsByteAttribute)
{
    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(testProgram);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 2, nullptr, GL_STREAM_COPY);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_BYTE, false, 0xff, reinterpret_cast<const void *>(0xfe));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 1, 10, 1000);
}

// Test for a mishandling of instanced vertex attributes with zero-sized buffers bound on Apple
// OpenGL drivers.
TEST_P(WebGL2CompatibilityTest, DrawWithZeroSizedBuffer)
{
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glUseProgram(program);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    GLint posLocation = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    glEnableVertexAttribArray(posLocation);

    glVertexAttribDivisor(posLocation, 1);
    glVertexAttribPointer(posLocation, 1, GL_UNSIGNED_BYTE, GL_FALSE, 9,
                          reinterpret_cast<void *>(0x41424344));
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    // This should be caught as an invalid draw
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that draw calls exceeding the vertex attribute range are caught in the presence of both
// instanced and non-instanced attributes.
TEST_P(WebGL2CompatibilityTest, DrawWithInstancedAndNonInstancedAttributes)
{
    if (IsGLExtensionRequestable("GL_ANGLE_base_vertex_base_instance"))
    {
        glRequestExtensionANGLE("GL_ANGLE_base_vertex_base_instance");
    }

    const bool hasBaseInstance = IsGLExtensionEnabled("GL_ANGLE_base_vertex_base_instance");

    constexpr char kVS[] = R"(#version 300 es
in vec4 attr1;
in vec2 attr2;
in vec4 attr3;
in vec3 attr4;

out vec4 v1;
out vec2 v2;
out vec4 v3;
out vec3 v4;

void main()
{
    v1 = attr1;
    v2 = attr2;
    v3 = attr3;
    v4 = attr4;
    gl_Position = vec4(0, 0, 0, 0);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

in vec4 v1;
in vec2 v2;
in vec4 v3;
in vec3 v4;

out vec4 color;

void main()
{
    color = v1 + v2.xyxy + v3 + v4.xyxz;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    const GLint attrLocations[4] = {
        glGetAttribLocation(program, "attr1"),
        glGetAttribLocation(program, "attr2"),
        glGetAttribLocation(program, "attr3"),
        glGetAttribLocation(program, "attr4"),
    };

    GLBuffer buffers[4];

    // Set up all the buffers as such:
    //
    // Buffer 1: 64 bytes + (offset) 124
    // Buffer 2: 16 bytes + (offset) 212
    // Buffer 3: 128 bytes + (offset) 76
    // Buffer 4: 96 bytes + (offset) 52
    constexpr GLsizei kBufferSizes[4] = {
        64,
        16,
        128,
        96,
    };
    constexpr GLsizei kBufferOffsets[4] = {
        124,
        212,
        76,
        52,
    };
    // Attribute component count corresponding to the shader
    constexpr GLint kAttrComponents[4] = {
        4,
        2,
        4,
        3,
    };
    // Attribute types
    constexpr GLenum kAttrTypes[4] = {
        GL_SHORT,
        GL_BYTE,
        GL_FLOAT,
        GL_UNSIGNED_SHORT,
    };
    // Attribute strides.
    //
    // - Buffer 1 has 64 bytes, each attribute is 8 bytes.  With a stride of 12, 5 vertices can be
    //   drawn from this buffer.
    // - Buffer 2 has 16 bytes, each attribute is 2 bytes.  With a stride of 0, 8 vertices can be
    //   drawn from this buffer.
    // - Buffer 3 has 128 bytes, each attribute is 16 bytes.  With a stride of 20, 6 vertices can be
    //   drawn from this buffer.
    // - Buffer 4 has 96 bytes, each attribute is 6 bytes.  With a stride of 8, 12 vertices can be
    //   drawn from this buffer.
    constexpr GLsizei kAttrStrides[4] = {
        12,
        0,
        20,
        8,
    };

    for (int i = 0; i < 4; ++i)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
        glBufferData(GL_ARRAY_BUFFER, kBufferSizes[i] + kBufferOffsets[i], nullptr, GL_STATIC_DRAW);

        glEnableVertexAttribArray(attrLocations[i]);
        glVertexAttribPointer(attrLocations[i], kAttrComponents[i], kAttrTypes[i], GL_TRUE,
                              kAttrStrides[i], reinterpret_cast<void *>(kBufferOffsets[i]));
    }
    ASSERT_GL_NO_ERROR();

    // Without any attribute divisors, the maximum vertex attribute allowed is min(5, 8, 6, 12) with
    // non-instanced draws.
    glDrawArrays(GL_POINTS, 0, 4);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, 5);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArrays(GL_POINTS, 1, 5);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArrays(GL_POINTS, 1, 4);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 4, 1);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArrays(GL_POINTS, 5, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArrays(GL_POINTS, 200, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // Same with instanced draws.
    glDrawArraysInstanced(GL_POINTS, 0, 4, 10);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 0, 5, 1);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 0, 6, 5);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArraysInstanced(GL_POINTS, 1, 5, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArraysInstanced(GL_POINTS, 1, 4, 22);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 4, 1, 1240);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 4, 2, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArraysInstanced(GL_POINTS, 5, 1, 6);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArraysInstanced(GL_POINTS, 200, 1, 100);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // With a divisor on attribute 1, that attribute can reference up to vertex #5 (as first
    // attribute), while the rest are limited to min(8, 6, 12) as their maximum vertex attribute.
    glVertexAttribDivisor(attrLocations[0], 5);

    glDrawArrays(GL_POINTS, 0, 5);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, 6);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, 7);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // The following passes because attribute 1 only accesses index 0 regardless of first
    glDrawArrays(GL_POINTS, 4, 2);
    EXPECT_GL_NO_ERROR();
    // The following fails because attribute 3 accesses vertices [4, 7)
    glDrawArrays(GL_POINTS, 4, 3);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArrays(GL_POINTS, 5, 1);
    EXPECT_GL_NO_ERROR();

    // With instanced rendering, the same limits as above hold.  Additionally, attribute 1 does no
    // longer access only a single vertex, but it accesses instanceCount/5 (5 being the divisor)
    // elements.
    // The following passes because attribute 1 accesses vertices [0, 4)
    glDrawArraysInstanced(GL_POINTS, 0, 5, 20);
    EXPECT_GL_NO_ERROR();
    // The following passes because attribute 1 accesses vertices [0, 5)
    glDrawArraysInstanced(GL_POINTS, 0, 6, 25);
    EXPECT_GL_NO_ERROR();
    // The following fails because of the limit on non-instanced attributes
    glDrawArraysInstanced(GL_POINTS, 0, 7, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // The following fails because attribute 1 accesses vertices [0, 6)
    glDrawArraysInstanced(GL_POINTS, 0, 4, 26);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // The following passes because attribute 1 accesses vertices [0, 2).  Recall that first vertex
    // is ignored for instanced attributes.
    glDrawArraysInstanced(GL_POINTS, 3, 3, 9);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 3, 3, 10);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 3, 3, 11);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 5, 1, 1);
    EXPECT_GL_NO_ERROR();

    if (hasBaseInstance)
    {
        // The following passes because attribute 1 accesses vertices [0, 3)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 15, 0);
        EXPECT_GL_NO_ERROR();
        // The following passes because attribute 1 accesses vertices [1, 4)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 15, 5);
        EXPECT_GL_NO_ERROR();
        // The following passes because attribute 1 accesses vertices [0, 4)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 17, 3);
        EXPECT_GL_NO_ERROR();
        // The following passes because attribute 1 accesses vertices [3, 5)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 10, 15);
        EXPECT_GL_NO_ERROR();
        // The following fails because attribute 1 accesses vertices [3, 6)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 11, 15);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        // The following fails because attribute 1 accesses vertex 6
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 1, 25);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    // With a divisor on attribute 3, that attribute can reference up to vertex #6 (as first
    // attribute), while the rest are limited to min(8, 12) as their maximum vertex attribute.
    glVertexAttribDivisor(attrLocations[2], 3);

    glDrawArrays(GL_POINTS, 0, 7);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, 8);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, 9);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // The following passes because attribute 1 and 3 only access index 0 regardless of first and
    // count
    glDrawArrays(GL_POINTS, 4, 4);
    EXPECT_GL_NO_ERROR();
    // The following fails because attribute 2 accesses vertices [4, 9)
    glDrawArrays(GL_POINTS, 4, 5);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glDrawArrays(GL_POINTS, 5, 1);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 6, 1);
    EXPECT_GL_NO_ERROR();

    // With instanced rendering, the same limits as above hold.  Additionally, attribute 1 accesses
    // instanceCount/5 and attribute 3 accesses instanceCount/3 elements.
    // The following passes because attribute 1 accesses vertices [0, 4), and attribute 3 accesses
    // vertices [0, 6)
    glDrawArraysInstanced(GL_POINTS, 0, 5, 18);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 0, 8, 18);
    EXPECT_GL_NO_ERROR();
    // The following fails because attribute 3 accesses vertices [0, 7)
    glDrawArraysInstanced(GL_POINTS, 0, 5, 19);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // The following fails because of the limit on non-instanced attributes
    glDrawArraysInstanced(GL_POINTS, 0, 9, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // The following passes because attribute 1 accesses vertices [0, 3), and attribute 3 accesses
    // vertices [0, 4)
    glDrawArraysInstanced(GL_POINTS, 2, 4, 11);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 2, 4, 12);
    EXPECT_GL_NO_ERROR();
    // The following passes because attribute 3 accesses vertices [0, 5).  Attribute 1 still
    // accesses within limits of [0, 3)
    glDrawArraysInstanced(GL_POINTS, 2, 4, 13);
    EXPECT_GL_NO_ERROR();
    glDrawArraysInstanced(GL_POINTS, 5, 1, 1);
    EXPECT_GL_NO_ERROR();

    if (hasBaseInstance)
    {
        // The following passes because attribute 1 accesses vertices [0, 4), and attribute 3
        // accesses vertices [0, 6)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 18, 0);
        EXPECT_GL_NO_ERROR();
        // The following fails because attribute 3 accesses vertices [0, 7)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 19, 0);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        // The following fails because attribute 3 accesses vertices [1, 7)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 18, 1);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        // The following passes because attribute 3 accesses vertices [3, 6)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 7, 11);
        EXPECT_GL_NO_ERROR();
        // The following fails because attribute 3 accesses vertices [3, 7)
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 2, 4, 8, 11);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    // With a divisor on attribute 2, that attribute can reference up to vertex #8 (as first
    // attribute), and with a divisor on attribute 4, it can reference up to vertex #12.  There is
    // no particular limit on the maxmium vertex attribute when not instanced.
    glVertexAttribDivisor(attrLocations[1], 3);
    glVertexAttribDivisor(attrLocations[3], 1);

    // The following passes because all attributes only access index 0
    glDrawArrays(GL_POINTS, 0, 123);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 4, 500);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 5, 1);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 231, 1);
    EXPECT_GL_NO_ERROR();

    // With instanced rendering, the same limits as above hold.
    //
    // Attribute 1 accesses instanceCount/5 elements (note: buffer fits 5 vertices)
    // Attribute 2 accesses instanceCount/3 elements (note: buffer fits 8 vertices)
    // Attribute 3 accesses instanceCount/3 elements (note: buffer fits 6 vertices)
    // Attribute 4 accesses instanceCount/1 elements (note: buffer fits 12 vertices)
    //
    // Only instances [0, 12) are valid.
    glDrawArraysInstanced(GL_POINTS, 0, 123, 1);
    EXPECT_GL_NO_ERROR();
    // The following passes because attributes accessed are:
    // [0, 3), [0, 4), [0, 4), [0, 12)
    glDrawArraysInstanced(GL_POINTS, 0, 123, 12);
    EXPECT_GL_NO_ERROR();
    // The following fails because attributes accessed are:
    // [0, 3), [0, 5), [0, 5), [0, 13)
    //                              \-- overflow
    glDrawArraysInstanced(GL_POINTS, 0, 123, 13);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // The following passes because attributes accessed are:
    // [0, 2), [0, 3), [0, 3), [0, 9)
    glDrawArraysInstanced(GL_POINTS, 3, 359, 9);
    EXPECT_GL_NO_ERROR();
    // The following fails because attributes accessed are:
    // [0, 3), [0, 5), [0, 5), [0, 13)
    //                              \-- overflow
    glDrawArraysInstanced(GL_POINTS, 3, 359, 13);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // The following passes because attributes accessed are:
    // [0, 1), [0, 2), [0, 2), [0, 5)
    glDrawArraysInstanced(GL_POINTS, 120, 359, 5);
    EXPECT_GL_NO_ERROR();

    if (hasBaseInstance)
    {
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 120, 359, 12, 0);
        EXPECT_GL_NO_ERROR();
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 120, 359, 11, 1);
        EXPECT_GL_NO_ERROR();
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 120, 359, 1, 11);
        EXPECT_GL_NO_ERROR();
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 120, 359, 2, 11);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        glDrawArraysInstancedBaseInstanceANGLE(GL_POINTS, 120, 359, 1, 14);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(WebGLCompatibilityTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(WebGL2CompatibilityTest);
ANGLE_INSTANTIATE_TEST_ES3(WebGL2CompatibilityTest);
}  // namespace angle
