//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/random_utils.h"
#include "util/test_utils.h"

using namespace angle;

class OcclusionQueriesTest : public ANGLETest<>
{
  protected:
    OcclusionQueriesTest() : mProgram(0), mRNG(1)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        ASSERT_NE(0u, mProgram);
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram;
    RNG mRNG;
};

class OcclusionQueriesTestES3 : public OcclusionQueriesTest
{};

TEST_P(OcclusionQueriesTest, IsOccluded)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // draw a quad at depth 0.3
    glEnable(GL_DEPTH_TEST);
    glUseProgram(mProgram);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.3f);
    glUseProgram(0);

    EXPECT_GL_NO_ERROR();

    GLQueryEXT query;
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(),
             0.8f);  // this quad should be occluded by first quad
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
    }

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);

    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(result);
}

TEST_P(OcclusionQueriesTest, IsNotOccluded)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    GLQueryEXT query;
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f);  // this quad should not be occluded
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);  // will block waiting for result

    EXPECT_GL_NO_ERROR();

    EXPECT_GL_TRUE(result);
}

// Test that glClear should not be counted by occlusion query.
TEST_P(OcclusionQueriesTest, ClearNotCounted)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    // http://anglebug.com/42263499
    ANGLE_SKIP_TEST_IF(IsD3D11());

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    GLQueryEXT query[2];

    // First query
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query[0]);
    // Full screen clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // View port clear
    glViewport(0, 0, getWindowWidth() / 2, getWindowHeight());
    glScissor(0, 0, getWindowWidth() / 2, getWindowHeight());
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    EXPECT_GL_NO_ERROR();

    // Second query
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query[1]);

    // View port clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // View port clear
    glViewport(0, 0, getWindowWidth() / 2, getWindowHeight());
    glScissor(0, 0, getWindowWidth() / 2, getWindowHeight());
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // this quad should not be occluded
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f, 0.5f);

    // Clear again
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // this quad should not be occluded
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f, 1.0);

    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint result[2] = {GL_TRUE, GL_TRUE};
    glGetQueryObjectuivEXT(query[0], GL_QUERY_RESULT_EXT,
                           &result[0]);  // will block waiting for result
    glGetQueryObjectuivEXT(query[1], GL_QUERY_RESULT_EXT,
                           &result[1]);  // will block waiting for result
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(result[0]);
    EXPECT_GL_TRUE(result[1]);
}

// Test that masked glClear should not be counted by occlusion query.
TEST_P(OcclusionQueriesTest, MaskedClearNotCounted)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    // http://anglebug.com/42263499
    ANGLE_SKIP_TEST_IF(IsD3D());

    GLQueryEXT query;

    // Masked clear
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT,
                           &result);  // will block waiting for result
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(result);
}

// Test that copies should not be counted by occlusion query.
TEST_P(OcclusionQueriesTest, CopyNotCounted)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    // http://anglebug.com/42263499
    ANGLE_SKIP_TEST_IF(IsD3D());

    GLQueryEXT query;

    // Unrelated draw before the query starts.
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f, 0.5f);

    // Copy to a texture with a different format from backbuffer
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, getWindowWidth(), getWindowHeight(), 0);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT,
                           &result);  // will block waiting for result
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(result);
}

// Test that blit should not be counted by occlusion query.
TEST_P(OcclusionQueriesTestES3, BlitNotCounted)
{
    // http://anglebug.com/42263499
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // http://anglebug.com/42263669
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsVulkan());

    constexpr GLuint kSize = 64;

    GLFramebuffer srcFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, srcFbo);

    GLTexture srcTex;
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex, 0);

    GLFramebuffer dstFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);

    GLTexture dstTex;
    glBindTexture(GL_TEXTURE_2D, dstTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, kSize, kSize, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTex, 0);

    GLQueryEXT query;

    // Unrelated draw before the query starts.
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f, 0.5f);

    // Blit flipped and with different formats.
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    glBlitFramebuffer(0, 0, 64, 64, 64, 64, 0, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT,
                           &result);  // will block waiting for result
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(result);
}

// Test that multisampled-render-to-texture unresolve should not be counted by occlusion query.
TEST_P(OcclusionQueriesTestES3, UnresolveNotCounted)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    constexpr GLuint kSize = 64;

    GLFramebuffer fboMS;
    glBindFramebuffer(GL_FRAMEBUFFER, fboMS);

    // Create multisampled framebuffer to draw into
    GLTexture textureMS;
    glBindTexture(GL_TEXTURE_2D, textureMS);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         textureMS, 0, 4);

    GLRenderbuffer depthMS;
    glBindRenderbuffer(GL_RENDERBUFFER, depthMS);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMS);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw red into the multisampled color buffer.
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Create a texture and copy into it, forcing a resolve of the color buffer.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, kSize, kSize, 0);

    GLQueryEXT query;

    // Make a draw call that will fail the depth test, and therefore shouldn't contribute to
    // occlusion query.
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_NEVER);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f, 0.5f);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT,
                           &result);  // will block waiting for result
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(result);
}

// Test that reusing a query should reset its value to zero if no draw calls are emitted in the
// second pass.
TEST_P(OcclusionQueriesTest, RewriteDrawNoDrawToZero)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    GLQueryEXT query;
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // draw a quad at depth 0.3
    glEnable(GL_DEPTH_TEST);
    glUseProgram(mProgram);
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.3f);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    glUseProgram(0);

    EXPECT_GL_NO_ERROR();

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
    }

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);

    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(result);
}

// Test that changing framebuffers work
TEST_P(OcclusionQueriesTest, FramebufferBindingChange)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    constexpr GLsizei kSize = 4;

    // Create two framebuffers, and make sure they are synced.
    GLFramebuffer fbo[2];
    GLTexture color[2];

    for (size_t index = 0; index < 2; ++index)
    {
        glBindTexture(GL_TEXTURE_2D, color[index]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo[index]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color[index],
                               0);

        glClearColor(0, index, 1 - index, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        EXPECT_PIXEL_COLOR_EQ(0, 0, index ? GLColor::green : GLColor::blue);
    }
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kSize, kSize);

    // Start an occlusion query and issue a draw call to each framebuffer.
    GLQueryEXT query;

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);

    for (size_t index = 0; index < 2; ++index)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[index]);
        drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    }

    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    EXPECT_GL_NO_ERROR();

    GLuint result = GL_FALSE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_TRUE(result);
}

// Test that switching framebuffers without actually drawing, then issuing a masked clear while a
// query is active works.
TEST_P(OcclusionQueriesTestES3, SwitchFramebuffersThenMaskedClear)
{
    constexpr GLint kSize = 10;

    GLFramebuffer fbo1, fbo2;
    GLRenderbuffer rbo1, rbo2;

    // Set up two framebuffers
    glBindRenderbuffer(GL_RENDERBUFFER, rbo1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, kSize, kSize);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo1);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, kSize, kSize);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo2);

    // Start render pass on fbo1
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    // Begin a query
    GLQuery query;
    glBeginQuery(GL_ANY_SAMPLES_PASSED, query);

    // Switch to another render pass and clear.  In the Vulkan backend, this clear is deferred, so
    // while the framebuffer binding is synced, the previous render pass is not necessarily closed.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glClear(GL_STENCIL_BUFFER_BIT);

    // Switch back to the original render pass and issue a masked stencil clear.  In the Vulkan
    // backend, this is done with a draw call.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glStencilMask(0xAA);
    glClearStencil(0xF4);
    glClear(GL_STENCIL_BUFFER_BIT);

    // Verify the clear worked.
    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0xA4, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test that an empty query after a positive query returns false
TEST_P(OcclusionQueriesTest, EmptyQueryAfterCompletedQuery)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    GLQueryEXT query;

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    ASSERT_GL_NO_ERROR();

    GLuint result = GL_FALSE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);
    ASSERT_GL_NO_ERROR();
    EXPECT_TRUE(result);

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    ASSERT_GL_NO_ERROR();

    result = GL_FALSE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);
    ASSERT_GL_NO_ERROR();
    EXPECT_FALSE(result);
}

// Some Metal drivers do not automatically clear visibility buffer
// at the beginning of a render pass. This test makes two queries
// that would use the same internal visibility buffer at the same
// offset and checks the query results.
TEST_P(OcclusionQueriesTest, EmptyQueryAfterCompletedQueryInterleaved)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    GLQueryEXT query;

    // Make a draw call to start a new render pass
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.0f);

    // Begin a query and make another draw call
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.0f);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    // Check the query result to end command encoding
    GLuint result = GL_FALSE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);
    EXPECT_TRUE(result);
    ASSERT_GL_NO_ERROR();

    // Make a draw call to start a new render pass
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.0f);

    // Begin and immediately resolve a new query; it must return false
    result = GL_FALSE;
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);
    EXPECT_FALSE(result);
    ASSERT_GL_NO_ERROR();
}

// Test multiple occlusion queries.
TEST_P(OcclusionQueriesTest, MultiQueries)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    // http://anglebug.com/42263499
    ANGLE_SKIP_TEST_IF(IsOpenGL() || IsD3D11());

    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    GLQueryEXT query[5];

    // First query
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query[0]);

    EXPECT_GL_NO_ERROR();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f);  // this quad should not be occluded

    EXPECT_GL_NO_ERROR();

    // Due to implementation might skip in-renderpass flush, we are using glFinish here to force a
    // flush. A flush shound't clear the query result.
    glFinish();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), -2, 0.25f);  // this quad should be occluded
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    // First query ends

    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f,
             0.25f);  // this quad should not be occluded

    // Second query
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query[1]);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.9f,
             0.25f);  // this quad should be occluded
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    // Third query
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query[2]);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.9f,
             0.5f);  // this quad should not be occluded
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    // ------------
    glFinish();

    glViewport(0, 0, getWindowWidth() / 2, getWindowHeight());
    glScissor(0, 0, getWindowWidth() / 2, getWindowHeight());
    glEnable(GL_SCISSOR_TEST);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.9f,
             0.5f);  // this quad should not be occluded

    // Fourth query: begin query then end then begin again
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query[3]);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.9f,
             1);  // this quad should not be occluded
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query[3]);
    EXPECT_GL_NO_ERROR();
    // glClear should not be counted toward query);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    // Fifth query spans across frames
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query[4]);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f,
             0.25f);  // this quad should not be occluded

    swapBuffers();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.9f,
             0.5f);  // this quad should not be occluded
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query[0], GL_QUERY_RESULT_EXT,
                           &result);  // will block waiting for result
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_TRUE(result);

    glGetQueryObjectuivEXT(query[1], GL_QUERY_RESULT_EXT,
                           &result);  // will block waiting for result
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(result);

    glGetQueryObjectuivEXT(query[2], GL_QUERY_RESULT_EXT,
                           &result);  // will block waiting for result
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_TRUE(result);

    glGetQueryObjectuivEXT(query[3], GL_QUERY_RESULT_EXT,
                           &result);  // will block waiting for result
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(result);

    glGetQueryObjectuivEXT(query[4], GL_QUERY_RESULT_EXT,
                           &result);  // will block waiting for result
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_TRUE(result);
}

TEST_P(OcclusionQueriesTest, Errors)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    GLuint query  = 0;
    GLuint query2 = 0;
    glGenQueriesEXT(1, &query);

    EXPECT_GL_FALSE(glIsQueryEXT(query));
    EXPECT_GL_FALSE(glIsQueryEXT(query2));

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, 0);  // can't pass 0 as query id
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT,
                    query2);  // can't initiate a query while one's already active
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    EXPECT_GL_TRUE(glIsQueryEXT(query));
    EXPECT_GL_FALSE(glIsQueryEXT(query2));  // have not called begin

    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f);  // this quad should not be occluded
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT);      // no active query for this target
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT,
                    query);  // can't begin a query as a different type than previously used
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT,
                    query2);  // have to call genqueries first
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glGenQueriesEXT(1, &query2);
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT, query2);  // should be ok now
    EXPECT_GL_TRUE(glIsQueryEXT(query2));

    drawQuad(mProgram, essl1_shaders::PositionAttrib(),
             0.3f);                  // this should draw in front of other quad
    glDeleteQueriesEXT(1, &query2);  // should delete when query becomes inactive
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT);  // should not incur error; should delete
                                                            // query + 1 at end of execution.
    EXPECT_GL_NO_ERROR();

    swapBuffers();

    EXPECT_GL_NO_ERROR();

    GLuint ready = GL_FALSE;
    glGetQueryObjectuivEXT(query2, GL_QUERY_RESULT_AVAILABLE_EXT,
                           &ready);  // this query is now deleted
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    EXPECT_GL_NO_ERROR();
}

// Test that running multiple simultaneous queries from multiple EGL contexts returns the correct
// result for each query.  Helps expose bugs in ANGLE's virtual contexts.
TEST_P(OcclusionQueriesTest, MultiContext)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    // TODO(cwallez@chromium.org): Suppression for http://anglebug.com/42261759
    ANGLE_SKIP_TEST_IF(IsWindows() && IsNVIDIA() && IsVulkan());

    // Test skipped because the D3D backends cannot support simultaneous queries on multiple
    // contexts yet.
    ANGLE_SKIP_TEST_IF(GetParam() == ES2_D3D9() || GetParam() == ES2_D3D11() ||
                       GetParam() == ES3_D3D11());

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // draw a quad at depth 0.5
    glEnable(GL_DEPTH_TEST);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EGLWindow *window = getEGLWindow();

    EGLDisplay display = window->getDisplay();
    EGLConfig config   = window->getConfig();
    EGLSurface surface = window->getSurface();

    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR,
        GetParam().majorVersion,
        EGL_CONTEXT_MINOR_VERSION_KHR,
        GetParam().minorVersion,
        EGL_NONE,
    };

    const size_t passCount = 5;
    struct ContextInfo
    {
        EGLContext context;
        GLuint program;
        GLuint query;
        bool visiblePasses[passCount];
        bool shouldPass;
    };

    ContextInfo contexts[] = {
        {
            EGL_NO_CONTEXT,
            0,
            0,
            {false, false, false, false, false},
            false,
        },
        {
            EGL_NO_CONTEXT,
            0,
            0,
            {false, true, false, true, false},
            true,
        },
        {
            EGL_NO_CONTEXT,
            0,
            0,
            {false, false, false, false, false},
            false,
        },
        {
            EGL_NO_CONTEXT,
            0,
            0,
            {true, true, false, true, true},
            true,
        },
        {
            EGL_NO_CONTEXT,
            0,
            0,
            {false, true, true, true, true},
            true,
        },
        {
            EGL_NO_CONTEXT,
            0,
            0,
            {true, false, false, true, false},
            true,
        },
        {
            EGL_NO_CONTEXT,
            0,
            0,
            {false, false, false, false, false},
            false,
        },
        {
            EGL_NO_CONTEXT,
            0,
            0,
            {false, false, false, false, false},
            false,
        },
    };

    for (auto &context : contexts)
    {
        context.context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
        ASSERT_NE(context.context, EGL_NO_CONTEXT);

        eglMakeCurrent(display, surface, surface, context.context);

        context.program = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        ASSERT_NE(context.program, 0u);

        glDepthMask(GL_FALSE);
        glEnable(GL_DEPTH_TEST);

        glGenQueriesEXT(1, &context.query);
        glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, context.query);

        ASSERT_GL_NO_ERROR();
    }

    for (size_t pass = 0; pass < passCount; pass++)
    {
        for (const auto &context : contexts)
        {
            eglMakeCurrent(display, surface, surface, context.context);

            float depth = context.visiblePasses[pass] ? mRNG.randomFloatBetween(0.0f, 0.4f)
                                                      : mRNG.randomFloatBetween(0.6f, 1.0f);
            drawQuad(context.program, essl1_shaders::PositionAttrib(), depth);

            EXPECT_GL_NO_ERROR();
        }
    }

    for (const auto &context : contexts)
    {
        eglMakeCurrent(display, surface, surface, context.context);
        glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

        GLuint result = GL_TRUE;
        glGetQueryObjectuivEXT(context.query, GL_QUERY_RESULT_EXT, &result);

        EXPECT_GL_NO_ERROR();

        GLuint expectation = context.shouldPass ? GL_TRUE : GL_FALSE;
        EXPECT_EQ(expectation, result);
    }

    eglMakeCurrent(display, surface, surface, window->getContext());

    for (auto &context : contexts)
    {
        eglDestroyContext(display, context.context);
        context.context = EGL_NO_CONTEXT;
    }
}

// Test multiple occlusion queries in flight. This test provoked a bug in the Metal backend that
// resulted in an infinite loop when trying to flush the command buffer when the maximum number of
// inflight render passes was reached.
TEST_P(OcclusionQueriesTest, ManyQueriesInFlight)
{
    constexpr int kManyQueryCount = 100;

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    // http://anglebug.com/42263499
    ANGLE_SKIP_TEST_IF(IsOpenGL() || IsD3D11());

    GLQueryEXT query;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLRenderbuffer rbo[2];
    glBindRenderbuffer(GL_RENDERBUFFER, rbo[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 32, 32);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 32, 32);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo[1]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (int i = 0; i < kManyQueryCount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query);
        drawQuad(mProgram, essl1_shaders::PositionAttrib(), 1.0f - 2.0f * i / kManyQueryCount);
        glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.8f);
    }

    glFinish();

    EXPECT_GL_NO_ERROR();
}

// Test two occlusion queries in sequence and there are some glBindFramebuffer in between.
// This test provoked a bug that the second query been skipped.
TEST_P(OcclusionQueriesTest, WrongSkippedQuery)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_occlusion_query_boolean"));

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 32, 32);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GL_NO_ERROR();

    GLQueryEXT query1;
    // Draw square in 1st FBO, clear main framebuffer - main framebuffer is active after
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query1);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    EXPECT_GL_NO_ERROR();

    GLQueryEXT query2;
    // Draw square in FBO, clear main framebuffer - FBO is active after
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query2);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    EXPECT_GL_NO_ERROR();

    GLuint results[2]  = {0};
    GLuint expectation = GL_TRUE;
    glGetQueryObjectuivEXT(query1, GL_QUERY_RESULT_EXT, &results[0]);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectation, results[0]);

    glGetQueryObjectuivEXT(query2, GL_QUERY_RESULT_EXT, &results[1]);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectation, results[1]);
}

class OcclusionQueriesNoSurfaceTestES3 : public ANGLETestBase,
                                         public ::testing::TestWithParam<angle::PlatformParameters>
{
  protected:
    OcclusionQueriesNoSurfaceTestES3()
        : ANGLETestBase(GetParam()), mUnusedConfig(0), mUnusedDisplay(nullptr)
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setDeferContextInit(true);
    }

    static constexpr int kWidth  = 300;
    static constexpr int kHeight = 300;

    void SetUp() override { ANGLETestBase::ANGLETestSetUp(); }
    void TearDown() override { ANGLETestBase::ANGLETestTearDown(); }

    void swapBuffers() override {}

    EGLConfig mUnusedConfig;
    EGLDisplay mUnusedDisplay;
};

// This test provked a bug in the Metal backend that only happened
// when there was no surfaces on the EGLContext and a query had
// just ended after a draw and then switching to a different
// context.
TEST_P(OcclusionQueriesNoSurfaceTestES3, SwitchingContextsWithQuery)
{
    EGLWindow *window = getEGLWindow();

    EGLDisplay display = window->getDisplay();
    EGLConfig config   = window->getConfig();

    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR,
        GetParam().majorVersion,
        EGL_CONTEXT_MINOR_VERSION_KHR,
        GetParam().minorVersion,
        EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE,
        EGL_TRUE,
        EGL_NONE,
    };

    // The following GL objects are implicitly deleted in
    // ContextInfo's destructor before the EGLContext is manually destroyed
    struct ContextInfo
    {
        EGLContext context;
        GLBuffer buf;
        GLProgram program;
        GLFramebuffer fb;
        GLTexture tex;
        GLQuery query;
    };

    // ContextInfo contains objects that clean themselves on destruction.
    // We want these objects to stick around until the test ends.
    std::vector<ContextInfo *> pairs;

    for (size_t i = 0; i < 2; ++i)
    {
        ContextInfo *infos[] = {
            new ContextInfo(),
            new ContextInfo(),
        };

        for (ContextInfo *pinfo : infos)
        {
            pairs.push_back(pinfo);
            ContextInfo &info = *pinfo;

            info.context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
            ASSERT_NE(info.context, EGL_NO_CONTEXT);

            // Make context current context with no draw and read surface.
            ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, info.context));

            // Create something to draw to.
            glBindFramebuffer(GL_FRAMEBUFFER, info.fb);
            glBindTexture(GL_TEXTURE_2D, info.tex);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, info.tex,
                                   0);
            EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            EXPECT_GL_NO_ERROR();
            glFlush();
        }

        // Setup an shader and quad buffer
        for (ContextInfo *pinfo : infos)
        {
            ContextInfo &info = *pinfo;
            ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, info.context));

            constexpr char kVS[] = R"(
            attribute vec4 position;
            void main() {
              gl_Position = position;
            }
          )";

            constexpr char kFS[] = R"(
          precision mediump float;
          void main() {
            gl_FragColor = vec4(1, 0, 0, 1);
          }
          )";

            info.program.makeRaster(kVS, kFS);
            glUseProgram(info.program);

            constexpr float vertices[] = {
                -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
            };
            glBindBuffer(GL_ARRAY_BUFFER, info.buf);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
            EXPECT_GL_NO_ERROR();
        }

        ContextInfo &info1 = *infos[0];
        ContextInfo &info2 = *infos[1];

        ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, info1.context));

        glBeginQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, info1.query);
        glFlush();

        ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, info2.context));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, info1.context));

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glEndQuery(GL_ANY_SAMPLES_PASSED_CONSERVATIVE);
        EXPECT_GL_NO_ERROR();

        ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, info2.context));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, info1.context));
        ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, info2.context));
        ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, info1.context));
    }

    // destroy GL objects on the correct context.
    for (ContextInfo *pinfo : pairs)
    {
        EGLContext context = pinfo->context;
        ASSERT_EGL_TRUE(eglMakeCurrent(display, nullptr, nullptr, context));
        EXPECT_GL_NO_ERROR();
        delete pinfo;
        ASSERT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        ASSERT_EGL_TRUE(eglDestroyContext(display, context));
        EXPECT_EGL_SUCCESS();
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(OcclusionQueriesTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(OcclusionQueriesTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(
    OcclusionQueriesTestES3,
    ES3_VULKAN().enable(Feature::PreferSubmitOnAnySamplesPassedQueryEnd));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(OcclusionQueriesNoSurfaceTestES3);
ANGLE_INSTANTIATE_TEST_ES3(OcclusionQueriesNoSurfaceTestES3);
