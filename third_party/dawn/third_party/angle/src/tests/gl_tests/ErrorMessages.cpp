//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ErrorMessages.cpp : Tests functionality of internal error error messages

#include "test_utils/ANGLETest.h"

#include "../src/libANGLE/ErrorStrings.h"
#include "test_utils/gl_raii.h"

namespace
{

struct Message
{
    GLenum source;
    GLenum type;
    GLenum id;
    GLenum severity;
    std::string message;
    const void *userParam;

    inline bool operator==(Message a)
    {
        if (a.source == source && a.type == type && a.id == id && a.message == message)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

static void GL_APIENTRY Callback(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar *message,
                                 const void *userParam)
{
    Message m{source, type, id, severity, std::string(message, length), userParam};
    std::vector<Message> *messages =
        static_cast<std::vector<Message> *>(const_cast<void *>(userParam));
    messages->push_back(m);
}

}  // namespace

namespace angle
{

class ErrorMessagesTest : public ANGLETest<>
{
  protected:
    ErrorMessagesTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setWebGLCompatibilityEnabled(true);
    }
};

// Verify functionality of WebGL specific errors using KHR_debug
TEST_P(ErrorMessagesTest, ErrorMessages)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_KHR_debug"));

    glEnable(GL_DEBUG_OUTPUT);

    std::vector<Message> messages;
    glDebugMessageCallbackKHR(Callback, &messages);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    constexpr GLenum source    = GL_DEBUG_SOURCE_API;
    constexpr GLenum type      = GL_DEBUG_TYPE_ERROR;
    constexpr GLenum severity  = GL_DEBUG_SEVERITY_HIGH;
    constexpr GLuint id1       = 1282;
    const std::string message1 = gl::err::kWebglBindAttribLocationReservedPrefix;
    Message expectedMessage;

    GLint numMessages = 0;
    glGetIntegerv(GL_DEBUG_LOGGED_MESSAGES, &numMessages);
    EXPECT_EQ(0, numMessages);

    glBindAttribLocation(0, 0, "_webgl_var");

    ASSERT_EQ(1u, messages.size());

    expectedMessage.source   = source;
    expectedMessage.id       = id1;
    expectedMessage.type     = type;
    expectedMessage.severity = severity;
    expectedMessage.message  = message1;

    Message &m = messages.front();
    ASSERT_TRUE(m == expectedMessage);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(ErrorMessagesTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_METAL(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES(),
                       ES2_VULKAN());
}  // namespace angle
