//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Multiview draw tests:
// Test issuing multiview Draw* commands.
//

#include "platform/autogen/FeaturesD3D_autogen.h"
#include "test_utils/MultiviewTest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

std::vector<Vector2> ConvertPixelCoordinatesToClipSpace(const std::vector<Vector2I> &pixels,
                                                        int width,
                                                        int height)
{
    std::vector<Vector2> result(pixels.size());
    for (size_t i = 0; i < pixels.size(); ++i)
    {
        const auto &pixel          = pixels[i];
        float pixelCenterRelativeX = (static_cast<float>(pixel.x()) + .5f) / width;
        float pixelCenterRelativeY = (static_cast<float>(pixel.y()) + .5f) / height;
        float xInClipSpace         = 2.f * pixelCenterRelativeX - 1.f;
        float yInClipSpace         = 2.f * pixelCenterRelativeY - 1.f;
        result[i]                  = Vector2(xInClipSpace, yInClipSpace);
    }
    return result;
}
}  // namespace

struct MultiviewRenderTestParams final : public MultiviewImplementationParams
{
    MultiviewRenderTestParams(int samples,
                              const MultiviewImplementationParams &implementationParams)
        : MultiviewImplementationParams(implementationParams), mSamples(samples)
    {}
    int mSamples;
};

std::ostream &operator<<(std::ostream &os, const MultiviewRenderTestParams &params)
{
    const MultiviewImplementationParams &base =
        static_cast<const MultiviewImplementationParams &>(params);
    os << base;
    os << "_layered";

    if (params.mSamples > 0)
    {
        os << "_samples_" << params.mSamples;
    }
    return os;
}

class MultiviewFramebufferTestBase : public MultiviewTestBase,
                                     public ::testing::TestWithParam<MultiviewRenderTestParams>
{
  protected:
    MultiviewFramebufferTestBase(const PlatformParameters &params, int samples)
        : MultiviewTestBase(params),
          mViewWidth(0),
          mViewHeight(0),
          mNumViews(0),
          mColorTexture(0u),
          mDepthTexture(0u),
          mDrawFramebuffer(0u),
          mSamples(samples),
          mResolveTexture(0u)
    {}

    void FramebufferTestSetUp() { MultiviewTestBase::MultiviewTestBaseSetUp(); }

    void FramebufferTestTearDown()
    {
        freeFBOs();
        MultiviewTestBase::MultiviewTestBaseTearDown();
    }

    void updateFBOs(int viewWidth, int height, int numViews, int numLayers, int baseViewIndex)
    {
        ASSERT_TRUE(numViews + baseViewIndex <= numLayers);

        freeFBOs();

        mViewWidth  = viewWidth;
        mViewHeight = height;
        mNumViews   = numViews;

        glGenTextures(1, &mColorTexture);
        glGenTextures(1, &mDepthTexture);

        CreateMultiviewBackingTextures(mSamples, viewWidth, height, numLayers, mColorTexture,
                                       mDepthTexture, 0u);

        glGenFramebuffers(1, &mDrawFramebuffer);

        // Create draw framebuffer to be used for multiview rendering.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawFramebuffer);
        AttachMultiviewTextures(GL_DRAW_FRAMEBUFFER, viewWidth, numViews, baseViewIndex,
                                mColorTexture, mDepthTexture, 0u);

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

        // Create read framebuffer to be used to retrieve the pixel information for testing
        // purposes.
        mReadFramebuffer.resize(numLayers);
        glGenFramebuffers(static_cast<GLsizei>(mReadFramebuffer.size()), mReadFramebuffer.data());
        for (int i = 0; i < numLayers; ++i)
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mReadFramebuffer[i]);
            glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColorTexture, 0,
                                      i);
            ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE,
                             glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));
        }

        // Clear the buffers.
        glViewport(0, 0, viewWidth, height);
    }

    void updateFBOs(int viewWidth, int height, int numViews)
    {
        updateFBOs(viewWidth, height, numViews, numViews, 0);
    }

    void bindMemberDrawFramebuffer() { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawFramebuffer); }

    // In case we have a multisampled framebuffer, creates and binds a resolve framebuffer as the
    // draw framebuffer, and resolves the read framebuffer to it.
    void resolveMultisampledFBO()
    {
        if (mSamples == 0)
        {
            return;
        }
        int numLayers = mReadFramebuffer.size();
        if (mResolveFramebuffer.empty())
        {
            ASSERT_TRUE(mResolveTexture == 0u);
            glGenTextures(1, &mResolveTexture);
            CreateMultiviewBackingTextures(0, mViewWidth, mViewHeight, numLayers, mResolveTexture,
                                           0u, 0u);

            mResolveFramebuffer.resize(numLayers);
            glGenFramebuffers(static_cast<GLsizei>(mResolveFramebuffer.size()),
                              mResolveFramebuffer.data());
            for (int i = 0; i < numLayers; ++i)
            {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mResolveFramebuffer[i]);
                glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          mResolveTexture, 0, i);
                ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE,
                                 glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
            }
        }
        for (int i = 0; i < numLayers; ++i)
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mReadFramebuffer[i]);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mResolveFramebuffer[i]);
            glBlitFramebuffer(0, 0, mViewWidth, mViewHeight, 0, 0, mViewWidth, mViewHeight,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
    }

    GLColor GetViewColor(int x, int y, int view)
    {
        EXPECT_TRUE(static_cast<size_t>(view) < mReadFramebuffer.size());
        if (mSamples > 0)
        {
            EXPECT_TRUE(static_cast<size_t>(view) < mResolveFramebuffer.size());
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mResolveFramebuffer[view]);
        }
        else
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mReadFramebuffer[view]);
        }
        return ReadColor(x, y);
    }

    // Requests the OVR_multiview(2) extension and returns true if the operation succeeds.
    bool requestMultiviewExtension(bool requireMultiviewMultisample)
    {
        if (!EnsureGLExtensionEnabled(extensionName()))
        {
            std::cout << "Test skipped due to missing " << extensionName() << "." << std::endl;
            return false;
        }

        if (requireMultiviewMultisample)
        {
            if (!EnsureGLExtensionEnabled("GL_OES_texture_storage_multisample_2d_array"))
            {
                std::cout << "Test skipped due to missing GL_ANGLE_multiview_multisample."
                          << std::endl;
                return false;
            }

            if (!EnsureGLExtensionEnabled("GL_ANGLE_multiview_multisample"))
            {
                std::cout << "Test skipped due to missing GL_ANGLE_multiview_multisample."
                          << std::endl;
                return false;
            }
        }
        return true;
    }

    bool requestMultiviewExtension() { return requestMultiviewExtension(false); }
    std::string extensionName()
    {
        switch (GetParam().mMultiviewExtension)
        {
            case multiview:
                return "GL_OVR_multiview";
            case multiview2:
                return "GL_OVR_multiview2";
            default:
                // Ignore unknown.
                return "";
        }
    }

    bool isMultisampled() { return mSamples > 0; }

    int mViewWidth;
    int mViewHeight;
    int mNumViews;

    GLuint mColorTexture;
    GLuint mDepthTexture;

  private:
    GLuint mDrawFramebuffer;
    std::vector<GLuint> mReadFramebuffer;
    int mSamples;

    // For reading back multisampled framebuffer.
    std::vector<GLuint> mResolveFramebuffer;
    GLuint mResolveTexture;

    void freeFBOs()
    {
        if (mDrawFramebuffer)
        {
            glDeleteFramebuffers(1, &mDrawFramebuffer);
            mDrawFramebuffer = 0;
        }
        if (!mReadFramebuffer.empty())
        {
            GLsizei framebufferCount = static_cast<GLsizei>(mReadFramebuffer.size());
            glDeleteFramebuffers(framebufferCount, mReadFramebuffer.data());
            mReadFramebuffer.clear();
        }
        if (!mResolveFramebuffer.empty())
        {
            GLsizei framebufferCount = static_cast<GLsizei>(mResolveFramebuffer.size());
            glDeleteFramebuffers(framebufferCount, mResolveFramebuffer.data());
            mResolveFramebuffer.clear();
        }
        if (mDepthTexture)
        {
            glDeleteTextures(1, &mDepthTexture);
            mDepthTexture = 0;
        }
        if (mColorTexture)
        {
            glDeleteTextures(1, &mColorTexture);
            mColorTexture = 0;
        }
        if (mResolveTexture)
        {
            glDeleteTextures(1, &mResolveTexture);
            mResolveTexture = 0;
        }
    }
};

class MultiviewRenderTest : public MultiviewFramebufferTestBase
{
  protected:
    MultiviewRenderTest() : MultiviewFramebufferTestBase(GetParam(), GetParam().mSamples) {}

    virtual void testSetUp() {}
    virtual void testTearDown() {}

  private:
    void SetUp() override
    {
        MultiviewFramebufferTestBase::FramebufferTestSetUp();
        testSetUp();
    }
    void TearDown() override
    {
        testTearDown();
        MultiviewFramebufferTestBase::FramebufferTestTearDown();
    }
};

std::string DualViewVS(ExtensionName multiviewExtension)
{
    std::string ext;
    switch (multiviewExtension)
    {
        case multiview:
            ext = "GL_OVR_multiview";
            break;
        case multiview2:
            ext = "GL_OVR_multiview2";
            break;
    }

    std::string dualViewVSSource =
        "#version 300 es\n"
        "#extension " +
        ext +
        " : require\n"
        "layout(num_views = 2) in;\n"
        "in vec4 vPosition;\n"
        "void main()\n"
        "{\n"
        "   gl_Position.x = (gl_ViewID_OVR == 0u ? vPosition.x * 0.5 + 0.5 : vPosition.x * 0.5 - "
        "0.5);\n"
        "   gl_Position.yzw = vPosition.yzw;\n"
        "}\n";
    return dualViewVSSource;
}

std::string DualViewFS(ExtensionName multiviewExtension)
{
    std::string ext;
    switch (multiviewExtension)
    {
        case multiview:
            ext = "GL_OVR_multiview";
            break;
        case multiview2:
            ext = "GL_OVR_multiview2";
            break;
    }

    std::string dualViewFSSource =
        "#version 300 es\n"
        "#extension " +
        ext +
        " : require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "  col = vec4(0,1,0,1);\n"
        "}\n";
    return dualViewFSSource;
}

class MultiviewRenderDualViewTest : public MultiviewRenderTest
{
  protected:
    MultiviewRenderDualViewTest() : mProgram(0u) {}

    void testSetUp() override
    {
        if (!requestMultiviewExtension(isMultisampled()))
        {
            return;
        }

        updateFBOs(2, 1, 2);
        mProgram = CompileProgram(DualViewVS(GetParam().mMultiviewExtension).c_str(),
                                  DualViewFS(GetParam().mMultiviewExtension).c_str());
        ASSERT_NE(mProgram, 0u);
        glUseProgram(mProgram);
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        if (mProgram != 0u)
        {
            glDeleteProgram(mProgram);
            mProgram = 0u;
        }
    }

    void checkOutput()
    {
        resolveMultisampledFBO();
        EXPECT_EQ(GLColor::transparentBlack, GetViewColor(0, 0, 0));
        EXPECT_EQ(GLColor::green, GetViewColor(1, 0, 0));
        EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 1));
        EXPECT_EQ(GLColor::transparentBlack, GetViewColor(1, 0, 1));
    }

    GLuint mProgram;
};

class MultiviewRenderDualViewTestNoWebGL : public MultiviewRenderDualViewTest
{
  protected:
    MultiviewRenderDualViewTestNoWebGL() { setWebGLCompatibilityEnabled(false); }
};

// Base class for tests that care mostly about draw call validity and not rendering results.
class MultiviewDrawValidationTest : public MultiviewTest
{
  protected:
    MultiviewDrawValidationTest() : MultiviewTest() {}

    void initOnePixelColorTexture2DSingleLayered(GLuint texId)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texId);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
    }

    void initOnePixelColorTexture2DMultiLayered(GLuint texId)
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texId);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 1, 1, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
    }

    // This initializes a simple VAO with a valid vertex buffer and index buffer with three
    // vertices.
    void initVAO(GLuint vao, GLuint vertexBuffer, GLuint indexBuffer)
    {
        glBindVertexArray(vao);

        const float kVertexData[3] = {0.0f};
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3u, &kVertexData[0], GL_STATIC_DRAW);

        const unsigned int kIndices[3] = {0u, 1u, 2u};
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3, &kIndices[0],
                     GL_STATIC_DRAW);
        ASSERT_GL_NO_ERROR();
    }
};

class MultiviewOcclusionQueryTest : public MultiviewRenderTest
{
  protected:
    MultiviewOcclusionQueryTest() {}

    bool requestOcclusionQueryExtension()
    {
        if (!EnsureGLExtensionEnabled("GL_EXT_occlusion_query_boolean"))
        {
            std::cout << "Test skipped due to missing GL_EXT_occlusion_query_boolean." << std::endl;
            return false;
        }
        return true;
    }

    GLuint drawAndRetrieveOcclusionQueryResult(GLuint program)
    {
        GLQueryEXT query;
        glBeginQueryEXT(GL_ANY_SAMPLES_PASSED, query);
        drawQuad(program, "vPosition", 0.0f, 1.0f, true);
        glEndQueryEXT(GL_ANY_SAMPLES_PASSED);

        GLuint result = GL_TRUE;
        glGetQueryObjectuivEXT(query, GL_QUERY_RESULT, &result);
        return result;
    }
};

class MultiviewProgramGenerationTest : public MultiviewTest
{
  protected:
    MultiviewProgramGenerationTest() {}
};

class MultiviewRenderPrimitiveTest : public MultiviewRenderTest
{
  protected:
    MultiviewRenderPrimitiveTest() : mVBO(0u) {}

    void testSetUp() override { glGenBuffers(1, &mVBO); }

    void testTearDown() override
    {
        if (mVBO)
        {
            glDeleteBuffers(1, &mVBO);
            mVBO = 0u;
        }
    }

    void setupGeometry(const std::vector<Vector2> &vertexData)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector2), vertexData.data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    void checkGreenChannel(const GLubyte expectedGreenChannelData[])
    {
        for (int view = 0; view < mNumViews; ++view)
        {
            for (int w = 0; w < mViewWidth; ++w)
            {
                for (int h = 0; h < mViewHeight; ++h)
                {
                    size_t flatIndex =
                        static_cast<size_t>(view * mViewWidth * mViewHeight + mViewWidth * h + w);
                    EXPECT_EQ(GLColor(0, expectedGreenChannelData[flatIndex], 0,
                                      expectedGreenChannelData[flatIndex]),
                              GetViewColor(w, h, view))
                        << "view: " << view << ", w: " << w << ", h: " << h;
                }
            }
        }
    }
    GLuint mVBO;
};

class MultiviewLayeredRenderTest : public MultiviewFramebufferTestBase
{
  protected:
    MultiviewLayeredRenderTest() : MultiviewFramebufferTestBase(GetParam(), 0) {}
    void SetUp() final { MultiviewFramebufferTestBase::FramebufferTestSetUp(); }
    void TearDown() final { MultiviewFramebufferTestBase::FramebufferTestTearDown(); }
};

// The test verifies that glDraw*Indirect works for any number of views.
TEST_P(MultiviewDrawValidationTest, IndirectDraw)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension());

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{color = vec4(1);}\n";

    GLVertexArray vao;
    GLBuffer vertexBuffer;
    GLBuffer indexBuffer;
    initVAO(vao, vertexBuffer, indexBuffer);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLBuffer commandBuffer;
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, commandBuffer);
    const GLuint commandData[] = {1u, 1u, 0u, 0u, 0u};
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(GLuint) * 5u, &commandData[0], GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // Check that no errors are generated with the framebuffer having 2 views.
    {
        const std::string VS =
            "#version 300 es\n"
            "#extension " +
            extensionName() +
            ": require\n"
            "layout(num_views = 2) in;\n"
            "void main()\n"
            "{}\n";
        ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
        glUseProgram(program);

        GLTexture tex2DArray;
        initOnePixelColorTexture2DMultiLayered(tex2DArray);

        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArray, 0, 0, 2);

        glDrawArraysIndirect(GL_TRIANGLES, nullptr);
        EXPECT_GL_NO_ERROR();

        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
        EXPECT_GL_NO_ERROR();
    }

    // Check that no errors are generated if the number of views is 1.
    {
        const std::string VS =
            "#version 300 es\n"
            "#extension " +
            extensionName() +
            ": require\n"
            "layout(num_views = 1) in;\n"
            "void main()\n"
            "{}\n";
        ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
        glUseProgram(program);

        GLTexture tex2D;
        initOnePixelColorTexture2DSingleLayered(tex2D);

        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2D, 0, 0, 1);

        glDrawArraysIndirect(GL_TRIANGLES, nullptr);
        EXPECT_GL_NO_ERROR();

        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
        EXPECT_GL_NO_ERROR();
    }
}

// The test verifies that glDraw*:
// 1) generates an INVALID_OPERATION error if the number of views in the active draw framebuffer and
// program differs.
// 2) does not generate any error if the number of views is the same.
TEST_P(MultiviewDrawValidationTest, NumViewsMismatch)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension());

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{}\n";
    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{color = vec4(1);}\n";
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
    glUseProgram(program);

    GLVertexArray vao;
    GLBuffer vertexBuffer;
    GLBuffer indexBuffer;
    initVAO(vao, vertexBuffer, indexBuffer);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Check for a GL_INVALID_OPERATION error with the framebuffer and program having different
    // number of views.
    {
        GLTexture tex2D;
        initOnePixelColorTexture2DSingleLayered(tex2D);

        // The framebuffer has only 1 view.
        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2D, 0, 0, 1);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    // Check that no errors are generated if the number of views in both program and draw
    // framebuffer matches.
    {
        GLTexture tex2DArray;
        initOnePixelColorTexture2DMultiLayered(tex2DArray);

        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArray, 0, 0, 2);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_NO_ERROR();

        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
        EXPECT_GL_NO_ERROR();
    }
}

// The test verifies that glDraw* generates an INVALID_OPERATION error if the program does not use
// the multiview extension, but the active draw framebuffer has more than one view.
TEST_P(MultiviewDrawValidationTest, NumViewsMismatchForNonMultiviewProgram)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    constexpr char kVS[] =
        "#version 300 es\n"
        "void main()\n"
        "{}\n";
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "void main()\n"
        "{}\n";
    ANGLE_GL_PROGRAM(programNoMultiview, kVS, kFS);
    glUseProgram(programNoMultiview);

    GLVertexArray vao;
    GLBuffer vertexBuffer;
    GLBuffer indexBuffer;
    initVAO(vao, vertexBuffer, indexBuffer);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex2DArray;
    initOnePixelColorTexture2DMultiLayered(tex2DArray);

    glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArray, 0, 0, 2);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// The test verifies that glDraw*:
// 1) generates an INVALID_OPERATION error if the number of views in the active draw framebuffer is
// greater than 1 and there is an active not paused transform feedback object.
// 2) does not generate any error if the number of views in the draw framebuffer is 1.
TEST_P(MultiviewDrawValidationTest, ActiveTransformFeedback)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension());

    constexpr char kVS[] = R"(#version 300 es
out float tfVarying;
void main()
{
    tfVarying = 1.0;
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
void main()
{})";

    std::vector<std::string> tfVaryings;
    tfVaryings.emplace_back("tfVarying");
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(singleViewProgram, kVS, kFS, tfVaryings,
                                        GL_SEPARATE_ATTRIBS);

    std::vector<std::string> dualViewTFVaryings;
    dualViewTFVaryings.emplace_back("gl_Position");
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(dualViewProgram,
                                        DualViewVS(GetParam().mMultiviewExtension).c_str(),
                                        DualViewFS(GetParam().mMultiviewExtension).c_str(),
                                        dualViewTFVaryings, GL_SEPARATE_ATTRIBS);

    GLVertexArray vao;
    GLBuffer vertexBuffer;
    GLBuffer indexBuffer;
    initVAO(vao, vertexBuffer, indexBuffer);

    GLBuffer tbo;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbo);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float) * 16u, nullptr, GL_STATIC_DRAW);

    GLTransformFeedback transformFeedback;
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

    glUseProgram(dualViewProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex2DArray;
    initOnePixelColorTexture2DMultiLayered(tex2DArray);

    GLenum bufs[] = {GL_NONE};
    glDrawBuffers(1, bufs);

    // Check that drawArrays generates an error when there is an active transform feedback object
    // and the number of views in the draw framebuffer is greater than 1.
    {
        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArray, 0, 0, 2);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    glEndTransformFeedback();

    // Ending transform feedback should allow the draw to succeed.
    {
        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_NO_ERROR();
    }

    // A paused transform feedback should not trigger an error.
    glBeginTransformFeedback(GL_TRIANGLES);
    glPauseTransformFeedback();
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();

    // Unbind transform feedback - should succeed.
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();

    // Rebind paused transform feedback - should succeed.
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();

    glResumeTransformFeedback();
    glEndTransformFeedback();

    glUseProgram(singleViewProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    ASSERT_GL_NO_ERROR();

    GLTexture tex2D;
    initOnePixelColorTexture2DSingleLayered(tex2D);

    // Check that drawArrays does not generate an error when the number of views in the draw
    // framebuffer is 1.
    {
        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2D, 0, 0, 1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_NO_ERROR();
    }

    glEndTransformFeedback();
}

// The test verifies that glDraw*:
// 1) generates an INVALID_OPERATION error if the number of views in the active draw framebuffer is
// greater than 1 and there is an active query for target GL_TIME_ELAPSED_EXT.
// 2) does not generate any error if the number of views in the draw framebuffer is 1.
TEST_P(MultiviewDrawValidationTest, ActiveTimeElapsedQuery)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension());
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    ANGLE_GL_PROGRAM(dualViewProgram, DualViewVS(GetParam().mMultiviewExtension).c_str(),
                     DualViewFS(GetParam().mMultiviewExtension).c_str());

    constexpr char kVS[] =
        "#version 300 es\n"
        "void main()\n"
        "{}\n";
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "void main()\n"
        "{}\n";
    ANGLE_GL_PROGRAM(singleViewProgram, kVS, kFS);
    glUseProgram(singleViewProgram);

    GLVertexArray vao;
    GLBuffer vertexBuffer;
    GLBuffer indexBuffer;
    initVAO(vao, vertexBuffer, indexBuffer);

    GLuint query = 0u;
    glGenQueriesEXT(1, &query);
    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex2DArr;
    initOnePixelColorTexture2DMultiLayered(tex2DArr);

    GLenum bufs[] = {GL_NONE};
    glDrawBuffers(1, bufs);

    // Check first case.
    {
        glUseProgram(dualViewProgram);
        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArr, 0, 0, 2);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    GLTexture tex2D;
    initOnePixelColorTexture2DSingleLayered(tex2D);

    // Check second case.
    {
        glUseProgram(singleViewProgram);
        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2D, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_NO_ERROR();
    }

    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    glDeleteQueries(1, &query);

    // Check starting a query after a successful draw.
    {
        glUseProgram(dualViewProgram);
        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArr, 0, 0, 2);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_NO_ERROR();

        glGenQueriesEXT(1, &query);
        glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        glEndQueryEXT(GL_TIME_ELAPSED_EXT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        EXPECT_GL_NO_ERROR();

        glDeleteQueries(1, &query);
    }
}

// The test checks that glDrawArrays can be used to render into two views.
TEST_P(MultiviewRenderDualViewTest, DrawArrays)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension(isMultisampled()));
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    drawQuad(mProgram, "vPosition", 0.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    checkOutput();
}

// The test checks that glDrawArrays can be used to render into two views, after the program
// executable has been installed and the program relinked (with a failing link, and using a
// different number of views).
TEST_P(MultiviewRenderDualViewTestNoWebGL, DrawArraysAfterFailedRelink)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension(isMultisampled()));
    ANGLE_SKIP_TEST_IF(IsWindows() && IsD3D());

    std::string ext =
        GetParam().mMultiviewExtension == multiview ? "GL_OVR_multiview" : "GL_OVR_multiview2";

    const std::string kVS = R"(#version 300 es
#extension )" + ext + R"( : require
layout(num_views = 2) in;
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(1.0, -1.0); break;
        case 2: pos = vec2(-1.0, 1.0); break;
        case 3: pos = vec2(1.0, 1.0); break;
    };
    pos.x = gl_ViewID_OVR == 0u ? pos.x * 0.5 + 0.5 : pos.x * 0.5 - 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const std::string kFS = R"(#version 300 es
#extension )" + ext + R"( : require
precision mediump float;
out vec4 col;
void main()
{
    col = vec4(0, 1, 0, 1);
})";

    const std::string kBadVS = R"(#version 300 es
#extension )" + ext + R"( : require
layout(num_views = 4) in;
out vec4 linkError;
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(1.0, -1.0); break;
        case 2: pos = vec2(-1.0, 1.0); break;
        case 3: pos = vec2(1.0, 1.0); break;
    };
    pos.x = gl_ViewID_OVR == 0u ? pos.x * 0.5 + 0.5 : pos.x * 0.5 - 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
    linkError = vec4(0);
})";

    const std::string kBadFS = R"(#version 300 es
#extension )" + ext + R"( : require
precision mediump float;
flat in uvec4 linkError;
out vec4 col;
void main()
{
    col = vec4(linkError);
})";

    // First, create a good program
    GLuint program = glCreateProgram();
    GLuint vs      = CompileShader(GL_VERTEX_SHADER, kVS.c_str());
    GLuint fs      = CompileShader(GL_FRAGMENT_SHADER, kFS.c_str());

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    CheckLinkStatusAndReturnProgram(program, true);

    // Detach the shaders for the sake of DrawArraysAfterFailedRelink
    glDetachShader(program, vs);
    glDetachShader(program, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    // Install the executable
    glUseProgram(program);

    // Relink the program but in an erroneous way
    GLuint badVs = CompileShader(GL_VERTEX_SHADER, kBadVS.c_str());
    GLuint badFs = CompileShader(GL_FRAGMENT_SHADER, kBadFS.c_str());

    glAttachShader(program, badVs);
    glAttachShader(program, badFs);

    glLinkProgram(program);

    glDeleteShader(badVs);
    glDeleteShader(badFs);
    ASSERT_GL_NO_ERROR();

    // Issue a draw and make sure everything works.
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    // drawQuad(mProgram, "vPosition", 0.0f, 1.0f, true);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL_NO_ERROR();

    checkOutput();

    glDeleteProgram(program);
}

// The test checks that glDrawElements can be used to render into two views.
TEST_P(MultiviewRenderDualViewTest, DrawElements)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension(isMultisampled()));
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    drawIndexedQuad(mProgram, "vPosition", 0.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    checkOutput();
}

// The test checks that glDrawRangeElements can be used to render into two views.
TEST_P(MultiviewRenderDualViewTest, DrawRangeElements)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension(isMultisampled()));
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    drawIndexedQuad(mProgram, "vPosition", 0.0f, 1.0f, true, true);
    ASSERT_GL_NO_ERROR();

    checkOutput();
}

// The test checks that glDrawArrays can be used to render into four views.
TEST_P(MultiviewRenderTest, DrawArraysFourViews)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension(isMultisampled()));
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        " : require\n"
        "layout(num_views = 4) in;\n"
        "in vec4 vPosition;\n"
        "void main()\n"
        "{\n"
        "   if (gl_ViewID_OVR == 0u) {\n"
        "       gl_Position.x = vPosition.x*0.25 - 0.75;\n"
        "   } else if (gl_ViewID_OVR == 1u) {\n"
        "       gl_Position.x = vPosition.x*0.25 - 0.25;\n"
        "   } else if (gl_ViewID_OVR == 2u) {\n"
        "       gl_Position.x = vPosition.x*0.25 + 0.25;\n"
        "   } else {\n"
        "       gl_Position.x = vPosition.x*0.25 + 0.75;\n"
        "   }"
        "   gl_Position.yzw = vPosition.yzw;\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        " : require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(0,1,0,1);\n"
        "}\n";

    updateFBOs(4, 1, 4);
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());

    drawQuad(program, "vPosition", 0.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    resolveMultisampledFBO();
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (i == j)
            {
                EXPECT_EQ(GLColor::green, GetViewColor(j, 0, i));
            }
            else
            {
                EXPECT_EQ(GLColor::transparentBlack, GetViewColor(j, 0, i));
            }
        }
    }
    EXPECT_GL_NO_ERROR();
}

// The test checks that glDrawArraysInstanced can be used to render into two views.
TEST_P(MultiviewRenderTest, DrawArraysInstanced)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension(isMultisampled()));
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "in vec4 vPosition;\n"
        "void main()\n"
        "{\n"
        "       vec4 p = vPosition;\n"
        "       if (gl_InstanceID == 1){\n"
        "               p.y = p.y * 0.5 + 0.5;\n"
        "       } else {\n"
        "               p.y = p.y * 0.5 - 0.5;\n"
        "       }\n"
        "       gl_Position.x = (gl_ViewID_OVR == 0u ? p.x * 0.5 + 0.5 : p.x * 0.5 - 0.5);\n"
        "       gl_Position.yzw = p.yzw;\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(0,1,0,1);\n"
        "}\n";

    const int kViewWidth  = 2;
    const int kViewHeight = 2;
    const int kNumViews   = 2;
    updateFBOs(kViewWidth, kViewHeight, kNumViews);
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());

    drawQuadInstanced(program, "vPosition", 0.0f, 1.0f, true, 2u);
    ASSERT_GL_NO_ERROR();

    resolveMultisampledFBO();

    const GLubyte expectedGreenChannel[kNumViews][kViewHeight][kViewWidth] = {{{0, 255}, {0, 255}},
                                                                              {{255, 0}, {255, 0}}};

    for (int view = 0; view < 2; ++view)
    {
        for (int y = 0; y < 2; ++y)
        {
            for (int x = 0; x < 2; ++x)
            {
                EXPECT_EQ(GLColor(0, expectedGreenChannel[view][y][x], 0,
                                  expectedGreenChannel[view][y][x]),
                          GetViewColor(x, y, view));
            }
        }
    }
}

// The test verifies that the attribute divisor is correctly adjusted when drawing with a multi-view
// program. The test draws 4 instances of a quad each of which covers a single pixel. The x and y
// offset of each quad are passed as separate attributes which are indexed based on the
// corresponding attribute divisors. A divisor of 1 is used for the y offset to have all quads
// drawn vertically next to each other. A divisor of 3 is used for the x offset to have the last
// quad offsetted by one pixel to the right. Note that the number of views is divisible by 1, but
// not by 3.
TEST_P(MultiviewRenderTest, AttribDivisor)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension(isMultisampled()));
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    // Looks like an incorrect D3D debug layer message is generated on Windows AMD and NVIDIA.
    // May be specific to Windows 7 / Windows Server 2008. http://anglebug.com/42261480
    if (IsWindows() && IsD3D11())
    {
        ignoreD3D11SDKLayersWarnings();
    }

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        " : require\n"
        "layout(num_views = 2) in;\n"
        "in vec3 vPosition;\n"
        "in float offsetX;\n"
        "in float offsetY;\n"
        "void main()\n"
        "{\n"
        "       vec4 p = vec4(vPosition, 1.);\n"
        "       p.xy = p.xy * 0.25 - vec2(0.75) + vec2(offsetX, offsetY);\n"
        "       gl_Position.x = (gl_ViewID_OVR == 0u ? p.x : p.x + 1.0);\n"
        "       gl_Position.yzw = p.yzw;\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(0,1,0,1);\n"
        "}\n";

    const int kViewWidth  = 4;
    const int kViewHeight = 4;
    const int kNumViews   = 2;
    updateFBOs(kViewWidth, kViewHeight, kNumViews);
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());

    GLBuffer xOffsetVBO;
    glBindBuffer(GL_ARRAY_BUFFER, xOffsetVBO);
    const GLfloat xOffsetData[4] = {0.0f, 0.5f, 1.0f, 1.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4, xOffsetData, GL_STATIC_DRAW);
    GLint xOffsetLoc = glGetAttribLocation(program, "offsetX");
    glVertexAttribPointer(xOffsetLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(xOffsetLoc, 3);
    glEnableVertexAttribArray(xOffsetLoc);

    GLBuffer yOffsetVBO;
    glBindBuffer(GL_ARRAY_BUFFER, yOffsetVBO);
    const GLfloat yOffsetData[4] = {0.0f, 0.5f, 1.0f, 1.5f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4, yOffsetData, GL_STATIC_DRAW);
    GLint yOffsetLoc = glGetAttribLocation(program, "offsetY");
    glVertexAttribDivisor(yOffsetLoc, 1);
    glVertexAttribPointer(yOffsetLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(yOffsetLoc);

    drawQuadInstanced(program, "vPosition", 0.0f, 1.0f, true, 4u);
    ASSERT_GL_NO_ERROR();

    resolveMultisampledFBO();

    const GLubyte expectedGreenChannel[kNumViews][kViewHeight][kViewWidth] = {
        {{255, 0, 0, 0}, {255, 0, 0, 0}, {255, 0, 0, 0}, {0, 255, 0, 0}},
        {{0, 0, 255, 0}, {0, 0, 255, 0}, {0, 0, 255, 0}, {0, 0, 0, 255}}};
    for (int view = 0; view < 2; ++view)
    {
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                EXPECT_EQ(GLColor(0, expectedGreenChannel[view][row][col], 0,
                                  expectedGreenChannel[view][row][col]),
                          GetViewColor(col, row, view));
            }
        }
    }
}

// Test that different sequences of vertexAttribDivisor, useProgram and bindVertexArray in a
// multi-view context propagate the correct divisor to the driver.
TEST_P(MultiviewRenderTest, DivisorOrderOfOperation)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension(isMultisampled()));
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    updateFBOs(1, 1, 2);

    // Create multiview program.
    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "layout(location = 0) in vec2 vPosition;\n"
        "layout(location = 1) in float offsetX;\n"
        "void main()\n"
        "{\n"
        "       vec4 p = vec4(vPosition, 0.0, 1.0);\n"
        "       p.x += offsetX;\n"
        "       gl_Position = p;\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        " : require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(0,1,0,1);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());

    constexpr char kStubVS[] =
        "#version 300 es\n"
        "layout(location = 0) in vec2 vPosition;\n"
        "layout(location = 1) in float offsetX;\n"
        "void main()\n"
        "{\n"
        "       gl_Position = vec4(vPosition, 0.0, 1.0);\n"
        "}\n";

    constexpr char kStubFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(0,0,0,1);\n"
        "}\n";

    ANGLE_GL_PROGRAM(stubProgram, kStubVS, kStubFS);

    GLBuffer xOffsetVBO;
    glBindBuffer(GL_ARRAY_BUFFER, xOffsetVBO);
    const GLfloat xOffsetData[12] = {0.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f,
                                     4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, xOffsetData, GL_STATIC_DRAW);

    GLBuffer vertexVBO;
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    Vector2 kQuadVertices[6] = {Vector2(-1.f, -1.f), Vector2(1.f, -1.f), Vector2(1.f, 1.f),
                                Vector2(-1.f, -1.f), Vector2(1.f, 1.f),  Vector2(-1.f, 1.f)};
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVertices), kQuadVertices, GL_STATIC_DRAW);

    GLVertexArray vao[2];
    for (size_t i = 0u; i < 2u; ++i)
    {
        glBindVertexArray(vao[i]);

        glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, xOffsetVBO);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
    }
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, 1, 1);
    glScissor(0, 0, 1, 1);
    glEnable(GL_SCISSOR_TEST);
    glClearColor(0, 0, 0, 1);

    // Clear the buffers, propagate divisor to the driver, bind the vao and keep it active.
    // It is necessary to call draw, so that the divisor is propagated and to guarantee that dirty
    // bits are cleared.
    glUseProgram(stubProgram);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vao[0]);
    glVertexAttribDivisor(1, 0);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();

    // Check that vertexAttribDivisor uses the number of views to update the divisor.
    bindMemberDrawFramebuffer();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    glVertexAttribDivisor(1, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

    resolveMultisampledFBO();
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 0));
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 1));

    // Clear the buffers and propagate divisor to the driver.
    // We keep the vao active and propagate the divisor to guarantee that there are no unresolved
    // dirty bits when useProgram is called.
    glUseProgram(stubProgram);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glVertexAttribDivisor(1, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();

    // Check that useProgram uses the number of views to update the divisor.
    bindMemberDrawFramebuffer();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

    resolveMultisampledFBO();
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 0));
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 1));

    // We go through similar steps as before.
    glUseProgram(stubProgram);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glVertexAttribDivisor(1, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();

    // Check that bindVertexArray uses the number of views to update the divisor.
    {
        // Call useProgram with vao[1] being active to guarantee that useProgram will adjust the
        // divisor for vao[1] only.
        bindMemberDrawFramebuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vao[1]);
        glUseProgram(program);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
        glBindVertexArray(0);
        ASSERT_GL_NO_ERROR();
    }
    // Bind vao[0] after useProgram is called to ensure that bindVertexArray is the call which
    // adjusts the divisor.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vao[0]);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

    resolveMultisampledFBO();
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 0));
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 1));
}

// Test that no fragments pass the occlusion query for a multi-view vertex shader which always
// transforms geometry to be outside of the clip region.
TEST_P(MultiviewOcclusionQueryTest, OcclusionQueryNothingVisible)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension());
    ANGLE_SKIP_TEST_IF(!requestOcclusionQueryExtension());
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "in vec3 vPosition;\n"
        "void main()\n"
        "{\n"
        "       gl_Position.x = 2.0;\n"
        "       gl_Position.yzw = vec3(vPosition.yz, 1.);\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        " : require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(1,0,0,0);\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
    updateFBOs(1, 1, 2);

    GLuint result = drawAndRetrieveOcclusionQueryResult(program);
    ASSERT_GL_NO_ERROR();
    EXPECT_GL_FALSE(result);
}

// Test that there are fragments passing the occlusion query if only view 0 can produce
// output.
TEST_P(MultiviewOcclusionQueryTest, OcclusionQueryOnlyLeftVisible)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension());
    ANGLE_SKIP_TEST_IF(!requestOcclusionQueryExtension());

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "in vec3 vPosition;\n"
        "void main()\n"
        "{\n"
        "       gl_Position.x = gl_ViewID_OVR == 0u ? vPosition.x : 2.0;\n"
        "       gl_Position.yzw = vec3(vPosition.yz, 1.);\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(1,0,0,0);\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
    updateFBOs(1, 1, 2);

    GLuint result = drawAndRetrieveOcclusionQueryResult(program);
    ASSERT_GL_NO_ERROR();
    EXPECT_GL_TRUE(result);
}

// Test that there are fragments passing the occlusion query if only view 1 can produce
// output.
TEST_P(MultiviewOcclusionQueryTest, OcclusionQueryOnlyRightVisible)
{
    ANGLE_SKIP_TEST_IF(!requestMultiviewExtension());
    ANGLE_SKIP_TEST_IF(!requestOcclusionQueryExtension());

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "in vec3 vPosition;\n"
        "void main()\n"
        "{\n"
        "       gl_Position.x = gl_ViewID_OVR == 1u ? vPosition.x : 2.0;\n"
        "       gl_Position.yzw = vec3(vPosition.yz, 1.);\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(1,0,0,0);\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
    updateFBOs(1, 1, 2);

    GLuint result = drawAndRetrieveOcclusionQueryResult(program);
    ASSERT_GL_NO_ERROR();
    EXPECT_GL_TRUE(result);
}

// Test that a simple multi-view program which doesn't use gl_ViewID_OVR in neither VS nor FS
// compiles and links without an error.
TEST_P(MultiviewProgramGenerationTest, SimpleProgram)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
    glUseProgram(program);

    EXPECT_GL_NO_ERROR();
}

// Test that a simple multi-view program which uses gl_ViewID_OVR only in VS compiles and links
// without an error.
TEST_P(MultiviewProgramGenerationTest, UseViewIDInVertexShader)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "   if (gl_ViewID_OVR == 0u) {\n"
        "       gl_Position = vec4(1,0,0,1);\n"
        "   } else {\n"
        "       gl_Position = vec4(-1,0,0,1);\n"
        "   }\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
    glUseProgram(program);

    EXPECT_GL_NO_ERROR();
}

// Test that a simple multi-view program which uses gl_ViewID_OVR only in FS compiles and links
// without an error.
TEST_P(MultiviewProgramGenerationTest, UseViewIDInFragmentShader)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "   if (gl_ViewID_OVR == 0u) {\n"
        "       col = vec4(1,0,0,1);\n"
        "   } else {\n"
        "       col = vec4(-1,0,0,1);\n"
        "   }\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
    glUseProgram(program);

    EXPECT_GL_NO_ERROR();
}

// The test checks that GL_POINTS is correctly rendered.
TEST_P(MultiviewRenderPrimitiveTest, Points)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "layout(location=0) in vec2 vPosition;\n"
        "void main()\n"
        "{\n"
        "   gl_PointSize = 1.0;\n"
        "   gl_Position = vec4(vPosition.xy, 0.0, 1.0);\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "   col = vec4(0,1,0,1);\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());
    glUseProgram(program);

    const int kViewWidth  = 4;
    const int kViewHeight = 2;
    const int kNumViews   = 2;
    updateFBOs(kViewWidth, kViewHeight, kNumViews);

    std::vector<Vector2I> windowCoordinates = {Vector2I(0, 0), Vector2I(3, 1)};
    std::vector<Vector2> vertexDataInClipSpace =
        ConvertPixelCoordinatesToClipSpace(windowCoordinates, 4, 2);
    setupGeometry(vertexDataInClipSpace);

    glDrawArrays(GL_POINTS, 0, 2);

    const GLubyte expectedGreenChannelData[kNumViews][kViewHeight][kViewWidth] = {
        {{255, 0, 0, 0}, {0, 0, 0, 255}}, {{255, 0, 0, 0}, {0, 0, 0, 255}}};
    checkGreenChannel(expectedGreenChannelData[0][0]);
}

// The test checks that GL_LINES is correctly rendered.
// The behavior of this test is not guaranteed by the spec:
// OpenGL ES 3.0.5 (November 3, 2016), Section 3.5.1 Basic Line Segment Rasterization:
// "The coordinates of a fragment produced by the algorithm may not deviate by more than one unit in
// either x or y window coordinates from a corresponding fragment produced by the diamond-exit
// rule."
TEST_P(MultiviewRenderPrimitiveTest, Lines)
{
    if (!requestMultiviewExtension())
    {
        return;
    }
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    GLuint program = CreateSimplePassthroughProgram(2, GetParam().mMultiviewExtension);
    ASSERT_NE(program, 0u);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    const int kViewWidth  = 4;
    const int kViewHeight = 2;
    const int kNumViews   = 2;
    updateFBOs(kViewWidth, kViewHeight, kNumViews);

    std::vector<Vector2I> windowCoordinates = {Vector2I(0, 0), Vector2I(4, 0)};
    std::vector<Vector2> vertexDataInClipSpace =
        ConvertPixelCoordinatesToClipSpace(windowCoordinates, 4, 2);
    setupGeometry(vertexDataInClipSpace);

    glDrawArrays(GL_LINES, 0, 2);

    const GLubyte expectedGreenChannelData[kNumViews][kViewHeight][kViewWidth] = {
        {{255, 255, 255, 255}, {0, 0, 0, 0}}, {{255, 255, 255, 255}, {0, 0, 0, 0}}};
    checkGreenChannel(expectedGreenChannelData[0][0]);

    glDeleteProgram(program);
}

// The test checks that GL_LINE_STRIP is correctly rendered.
// The behavior of this test is not guaranteed by the spec:
// OpenGL ES 3.0.5 (November 3, 2016), Section 3.5.1 Basic Line Segment Rasterization:
// "The coordinates of a fragment produced by the algorithm may not deviate by more than one unit in
// either x or y window coordinates from a corresponding fragment produced by the diamond-exit
// rule."
TEST_P(MultiviewRenderPrimitiveTest, LineStrip)
{
    if (!requestMultiviewExtension())
    {
        return;
    }
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    GLuint program = CreateSimplePassthroughProgram(2, GetParam().mMultiviewExtension);
    ASSERT_NE(program, 0u);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    const int kViewWidth  = 4;
    const int kViewHeight = 2;
    const int kNumViews   = 2;
    updateFBOs(kViewWidth, kViewHeight, kNumViews);

    std::vector<Vector2I> windowCoordinates = {Vector2I(0, 0), Vector2I(3, 0), Vector2I(3, 2)};
    std::vector<Vector2> vertexDataInClipSpace =
        ConvertPixelCoordinatesToClipSpace(windowCoordinates, 4, 2);
    setupGeometry(vertexDataInClipSpace);

    glDrawArrays(GL_LINE_STRIP, 0, 3);

    const GLubyte expectedGreenChannelData[kNumViews][kViewHeight][kViewWidth] = {
        {{255, 255, 255, 255}, {0, 0, 0, 255}}, {{255, 255, 255, 255}, {0, 0, 0, 255}}};
    checkGreenChannel(expectedGreenChannelData[0][0]);

    glDeleteProgram(program);
}

// The test checks that GL_LINE_LOOP is correctly rendered.
// The behavior of this test is not guaranteed by the spec:
// OpenGL ES 3.0.5 (November 3, 2016), Section 3.5.1 Basic Line Segment Rasterization:
// "The coordinates of a fragment produced by the algorithm may not deviate by more than one unit in
// either x or y window coordinates from a corresponding fragment produced by the diamond-exit
// rule."
TEST_P(MultiviewRenderPrimitiveTest, LineLoop)
{
    if (!requestMultiviewExtension())
    {
        return;
    }
    // Only this subtest fails on intel-hd-630-ubuntu-stable. Driver bug?
    // https://anglebug.com/42262137
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    GLuint program = CreateSimplePassthroughProgram(2, GetParam().mMultiviewExtension);
    ASSERT_NE(program, 0u);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    const int kViewWidth  = 4;
    const int kViewHeight = 4;
    const int kNumViews   = 2;
    updateFBOs(kViewWidth, kViewHeight, kNumViews);

    std::vector<Vector2I> windowCoordinates = {Vector2I(0, 0), Vector2I(3, 0), Vector2I(3, 3),
                                               Vector2I(0, 3)};
    std::vector<Vector2> vertexDataInClipSpace =
        ConvertPixelCoordinatesToClipSpace(windowCoordinates, 4, 4);
    setupGeometry(vertexDataInClipSpace);

    glDrawArrays(GL_LINE_LOOP, 0, 4);
    EXPECT_GL_NO_ERROR();

    const GLubyte expectedGreenChannelData[kNumViews][kViewHeight][kViewWidth] = {
        {{255, 255, 255, 255}, {255, 0, 0, 255}, {255, 0, 0, 255}, {255, 255, 255, 255}},
        {{255, 255, 255, 255}, {255, 0, 0, 255}, {255, 0, 0, 255}, {255, 255, 255, 255}}};
    checkGreenChannel(expectedGreenChannelData[0][0]);

    glDeleteProgram(program);
}

// The test checks that GL_TRIANGLE_STRIP is correctly rendered.
TEST_P(MultiviewRenderPrimitiveTest, TriangleStrip)
{
    if (!requestMultiviewExtension())
    {
        return;
    }
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    GLuint program = CreateSimplePassthroughProgram(2, GetParam().mMultiviewExtension);
    ASSERT_NE(program, 0u);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    std::vector<Vector2> vertexDataInClipSpace = {Vector2(1.0f, 0.0f), Vector2(0.0f, 0.0f),
                                                  Vector2(1.0f, 1.0f), Vector2(0.0f, 1.0f)};
    setupGeometry(vertexDataInClipSpace);

    const int kViewWidth  = 2;
    const int kViewHeight = 2;
    const int kNumViews   = 2;
    updateFBOs(kViewWidth, kViewHeight, kNumViews);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    const GLubyte expectedGreenChannelData[kNumViews][kViewHeight][kViewWidth] = {
        {{0, 0}, {0, 255}}, {{0, 0}, {0, 255}}};
    checkGreenChannel(expectedGreenChannelData[0][0]);

    glDeleteProgram(program);
}

// The test checks that GL_TRIANGLE_FAN is correctly rendered.
TEST_P(MultiviewRenderPrimitiveTest, TriangleFan)
{
    if (!requestMultiviewExtension())
    {
        return;
    }
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    GLuint program = CreateSimplePassthroughProgram(2, GetParam().mMultiviewExtension);
    ASSERT_NE(program, 0u);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    std::vector<Vector2> vertexDataInClipSpace = {Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f),
                                                  Vector2(1.0f, 1.0f), Vector2(1.0f, 0.0f)};
    setupGeometry(vertexDataInClipSpace);

    const int kViewWidth  = 2;
    const int kViewHeight = 2;
    const int kNumViews   = 2;
    updateFBOs(kViewWidth, kViewHeight, kNumViews);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    const GLubyte expectedGreenChannelData[kNumViews][kViewHeight][kViewWidth] = {
        {{0, 0}, {0, 255}}, {{0, 0}, {0, 255}}};
    checkGreenChannel(expectedGreenChannelData[0][0]);

    glDeleteProgram(program);
}

// Verify that re-linking a program adjusts the attribute divisor.
// The test uses instacing to draw for each view a strips of two red quads and two blue quads next
// to each other. The quads' position and color depend on the corresponding attribute divisors.
TEST_P(MultiviewRenderTest, ProgramRelinkUpdatesAttribDivisor)
{
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());
    if (!requestMultiviewExtension(isMultisampled()))
    {
        return;
    }

    // Looks like an incorrect D3D debug layer message is generated on Windows AMD and NVIDIA.
    // May be specific to Windows 7 / Windows Server 2008. http://anglebug.com/42261480
    if (IsWindows() && IsD3D11())
    {
        ignoreD3D11SDKLayersWarnings();
    }

    const int kViewWidth  = 4;
    const int kViewHeight = 1;
    const int kNumViews   = 2;

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "in vec4 oColor;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = oColor;\n"
        "}\n";

    auto generateVertexShaderSource = [](int numViews, std::string extensionName) -> std::string {
        std::string source =
            "#version 300 es\n"
            "#extension " +
            extensionName +
            ": require\n"
            "layout(num_views = " +
            ToString(numViews) +
            ") in;\n"
            "in vec3 vPosition;\n"
            "in float vOffsetX;\n"
            "in vec4 vColor;\n"
            "out vec4 oColor;\n"
            "void main()\n"
            "{\n"
            "       vec4 p = vec4(vPosition, 1.);\n"
            "       p.x = p.x * 0.25 - 0.75 + vOffsetX;\n"
            "       oColor = vColor;\n"
            "       gl_Position = p;\n"
            "}\n";
        return source;
    };

    std::string vsSource = generateVertexShaderSource(kNumViews, extensionName());
    ANGLE_GL_PROGRAM(program, vsSource.c_str(), FS.c_str());
    glUseProgram(program);

    GLint positionLoc;
    GLBuffer xOffsetVBO;
    GLint xOffsetLoc;
    GLBuffer colorVBO;
    GLint colorLoc;

    {
        // Initialize buffers and setup attributes.
        glBindBuffer(GL_ARRAY_BUFFER, xOffsetVBO);
        const GLfloat kXOffsetData[4] = {0.0f, 0.5f, 1.0f, 1.5f};
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4, kXOffsetData, GL_STATIC_DRAW);
        xOffsetLoc = glGetAttribLocation(program, "vOffsetX");
        glVertexAttribPointer(xOffsetLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribDivisor(xOffsetLoc, 1);
        glEnableVertexAttribArray(xOffsetLoc);

        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        const GLColor kColors[2] = {GLColor::red, GLColor::blue};
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLColor) * 2, kColors, GL_STATIC_DRAW);
        colorLoc = glGetAttribLocation(program, "vColor");
        glVertexAttribDivisor(colorLoc, 2);
        glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(colorLoc);

        positionLoc = glGetAttribLocation(program, "vPosition");
    }

    {
        updateFBOs(kViewWidth, kViewHeight, kNumViews);

        drawQuadInstanced(program, "vPosition", 0.0f, 1.0f, true, 4u);
        ASSERT_GL_NO_ERROR();

        resolveMultisampledFBO();
        EXPECT_EQ(GLColor::red, GetViewColor(0, 0, 0));
        EXPECT_EQ(GLColor::red, GetViewColor(1, 0, 0));
        EXPECT_EQ(GLColor::blue, GetViewColor(2, 0, 0));
        EXPECT_EQ(GLColor::blue, GetViewColor(3, 0, 0));
    }

    {
        const int kNewNumViews = 3;
        vsSource               = generateVertexShaderSource(kNewNumViews, extensionName());
        updateFBOs(kViewWidth, kViewHeight, kNewNumViews);

        GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource.c_str());
        ASSERT_NE(0u, vs);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, FS.c_str());
        ASSERT_NE(0u, fs);

        GLint numAttachedShaders = 0;
        glGetProgramiv(program, GL_ATTACHED_SHADERS, &numAttachedShaders);

        GLuint attachedShaders[2] = {0u};
        glGetAttachedShaders(program, numAttachedShaders, nullptr, attachedShaders);
        for (int i = 0; i < 2; ++i)
        {
            glDetachShader(program, attachedShaders[i]);
        }

        glAttachShader(program, vs);
        glDeleteShader(vs);

        glAttachShader(program, fs);
        glDeleteShader(fs);

        glBindAttribLocation(program, positionLoc, "vPosition");
        glBindAttribLocation(program, xOffsetLoc, "vOffsetX");
        glBindAttribLocation(program, colorLoc, "vColor");

        glLinkProgram(program);

        drawQuadInstanced(program, "vPosition", 0.0f, 1.0f, true, 4u);
        ASSERT_GL_NO_ERROR();

        resolveMultisampledFBO();
        for (int i = 0; i < kNewNumViews; ++i)
        {
            EXPECT_EQ(GLColor::red, GetViewColor(0, 0, i));
            EXPECT_EQ(GLColor::red, GetViewColor(1, 0, i));
            EXPECT_EQ(GLColor::blue, GetViewColor(2, 0, i));
            EXPECT_EQ(GLColor::blue, GetViewColor(3, 0, i));
        }
    }
}

// Test that useProgram applies the number of views in computing the final value of the attribute
// divisor.
TEST_P(MultiviewRenderTest, DivisorUpdatedOnProgramChange)
{
    if (!requestMultiviewExtension(isMultisampled()))
    {
        return;
    }

    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    // Looks like an incorrect D3D debug layer message is generated on Windows / AMD.
    // May be specific to Windows 7 / Windows Server 2008. http://anglebug.com/42261480
    if (IsWindows() && IsD3D11())
    {
        ignoreD3D11SDKLayersWarnings();
    }

    GLVertexArray vao;
    glBindVertexArray(vao);
    GLBuffer vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    std::vector<Vector2I> windowCoordinates = {Vector2I(0, 0), Vector2I(1, 0), Vector2I(2, 0),
                                               Vector2I(3, 0)};
    std::vector<Vector2> vertexDataInClipSpace =
        ConvertPixelCoordinatesToClipSpace(windowCoordinates, 4, 1);
    // Fill with x positions so that the resulting clip space coordinate fails the clip test.
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2) * vertexDataInClipSpace.size(),
                 vertexDataInClipSpace.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, nullptr);
    glVertexAttribDivisor(0, 1);
    ASSERT_GL_NO_ERROR();

    // Create a program and fbo with N views and draw N instances of a point horizontally.
    for (int numViews = 2; numViews <= 4; ++numViews)
    {
        updateFBOs(4, 1, numViews);
        ASSERT_GL_NO_ERROR();

        GLuint program = CreateSimplePassthroughProgram(numViews, GetParam().mMultiviewExtension);
        ASSERT_NE(program, 0u);
        glUseProgram(program);
        ASSERT_GL_NO_ERROR();

        glDrawArraysInstanced(GL_POINTS, 0, 1, numViews);

        resolveMultisampledFBO();
        for (int view = 0; view < numViews; ++view)
        {
            for (int j = 0; j < numViews; ++j)
            {
                EXPECT_EQ(GLColor::green, GetViewColor(j, 0, view));
            }
            for (int j = numViews; j < 4; ++j)
            {
                EXPECT_EQ(GLColor::transparentBlack, GetViewColor(j, 0, view));
            }
        }

        glDeleteProgram(program);
    }
}

// The test checks that gl_ViewID_OVR is correctly propagated to the fragment shader.
TEST_P(MultiviewRenderTest, SelectColorBasedOnViewIDOVR)
{
    if (!requestMultiviewExtension(isMultisampled()))
    {
        return;
    }

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 3) in;\n"
        "in vec3 vPosition;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(vPosition, 1.);\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0u) {\n"
        "       col = vec4(1,0,0,1);\n"
        "    } else if (gl_ViewID_OVR == 1u) {\n"
        "       col = vec4(0,1,0,1);\n"
        "    } else if (gl_ViewID_OVR == 2u) {\n"
        "       col = vec4(0,0,1,1);\n"
        "    } else {\n"
        "       col = vec4(0,0,0,0);\n"
        "    }\n"
        "}\n";

    updateFBOs(1, 1, 3);
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());

    drawQuad(program, "vPosition", 0.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    resolveMultisampledFBO();
    EXPECT_EQ(GLColor::red, GetViewColor(0, 0, 0));
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 1));
    EXPECT_EQ(GLColor::blue, GetViewColor(0, 0, 2));
}

// The test checks that the inactive layers of a 2D texture array are not written to by a
// multi-view program.
TEST_P(MultiviewLayeredRenderTest, RenderToSubrangeOfLayers)
{
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());
    if (!requestMultiviewExtension())
    {
        return;
    }

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "in vec3 vPosition;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(vPosition, 1.);\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "     col = vec4(0,1,0,1);\n"
        "}\n";

    updateFBOs(1, 1, 2, 4, 1);
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());

    drawQuad(program, "vPosition", 0.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    resolveMultisampledFBO();
    EXPECT_EQ(GLColor::transparentBlack, GetViewColor(0, 0, 0));
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 1));
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 2));
    EXPECT_EQ(GLColor::transparentBlack, GetViewColor(0, 0, 3));
}

// The D3D11 renderer uses a GS whenever the varyings are flat interpolated which can cause
// potential bugs if the view is selected in the VS. The test contains a program in which the
// gl_InstanceID is passed as a flat varying to the fragment shader where it is used to discard the
// fragment if its value is negative. The gl_InstanceID should never be negative and that branch is
// never taken. One quad is drawn and the color is selected based on the ViewID - red for view 0 and
// green for view 1.
TEST_P(MultiviewRenderTest, FlatInterpolation)
{
    if (!requestMultiviewExtension(isMultisampled()))
    {
        return;
    }

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "in vec3 vPosition;\n"
        "flat out int oInstanceID;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(vPosition, 1.);\n"
        "   oInstanceID = gl_InstanceID;\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "flat in int oInstanceID;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    if (oInstanceID < 0) {\n"
        "       discard;\n"
        "    }\n"
        "    if (gl_ViewID_OVR == 0u) {\n"
        "       col = vec4(1,0,0,1);\n"
        "    } else {\n"
        "       col = vec4(0,1,0,1);\n"
        "    }\n"
        "}\n";

    updateFBOs(1, 1, 2);
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());

    drawQuad(program, "vPosition", 0.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    resolveMultisampledFBO();
    EXPECT_EQ(GLColor::red, GetViewColor(0, 0, 0));
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 1));
}

// This test assigns gl_ViewID_OVR to a flat int varying and then sets the color based on that
// varying in the fragment shader.
TEST_P(MultiviewRenderTest, FlatInterpolation2)
{
    if (!requestMultiviewExtension(isMultisampled()))
    {
        return;
    }

    const std::string VS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "layout(num_views = 2) in;\n"
        "in vec3 vPosition;\n"
        "flat out int flatVarying;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(vPosition, 1.);\n"
        "   flatVarying = int(gl_ViewID_OVR);\n"
        "}\n";

    const std::string FS =
        "#version 300 es\n"
        "#extension " +
        extensionName() +
        ": require\n"
        "precision mediump float;\n"
        "flat in int flatVarying;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    if (flatVarying == 0) {\n"
        "       col = vec4(1,0,0,1);\n"
        "    } else {\n"
        "       col = vec4(0,1,0,1);\n"
        "    }\n"
        "}\n";

    updateFBOs(1, 1, 2);
    ANGLE_GL_PROGRAM(program, VS.c_str(), FS.c_str());

    drawQuad(program, "vPosition", 0.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    resolveMultisampledFBO();
    EXPECT_EQ(GLColor::red, GetViewColor(0, 0, 0));
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 1));
}

// Test that shader caching maintains the num_views value used in GL_OVR_multiview between shader
// compilations.
TEST_P(MultiviewRenderTest, ShaderCacheVertexWithOVRMultiview)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    if (!requestMultiviewExtension(true))
    {
        return;
    }

    constexpr char kVS[] = R"(#version 300 es
#extension GL_OVR_multiview : enable

layout (num_views = 2) in;

precision mediump float;

layout (location = 0) in vec4 a_position;

out float redValue;
out float greenValue;

void main() {
    gl_Position = a_position;
    if (gl_ViewID_OVR == uint(0))
    {
        redValue = 1.;
        greenValue = 0.;
    }
    else
    {
        redValue = 0.;
        greenValue = 1.;
    }
})";

    constexpr char kFS[] = R"(#version 300 es

precision mediump float;

in float redValue;
in float greenValue;

out vec4 fragColor;

void main()
{
    fragColor = vec4(redValue, greenValue, 0., 1.);
})";

    // Only use a single 1x1 FBO
    updateFBOs(1, 1, 2);

    ANGLE_GL_PROGRAM(unusedProgram, kVS, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    resolveMultisampledFBO();
    EXPECT_EQ(GLColor::red, GetViewColor(0, 0, 0));
    EXPECT_EQ(GLColor::green, GetViewColor(0, 0, 1));
}

MultiviewRenderTestParams VertexShaderOpenGL(ExtensionName multiviewExtension)
{
    return MultiviewRenderTestParams(0, VertexShaderOpenGL(3, 0, multiviewExtension));
}

MultiviewRenderTestParams VertexShaderVulkan(ExtensionName multiviewExtension)
{
    return MultiviewRenderTestParams(0, VertexShaderVulkan(3, 0, multiviewExtension));
}

MultiviewRenderTestParams GeomShaderD3D11(ExtensionName multiviewExtension)
{
    return MultiviewRenderTestParams(0, GeomShaderD3D11(3, 0, multiviewExtension));
}

MultiviewRenderTestParams VertexShaderD3D11(ExtensionName multiviewExtension)
{
    return MultiviewRenderTestParams(0, VertexShaderD3D11(3, 0, multiviewExtension));
}

MultiviewRenderTestParams MultisampledVertexShaderOpenGL(ExtensionName multiviewExtension)
{
    return MultiviewRenderTestParams(2, VertexShaderOpenGL(3, 1, multiviewExtension));
}

MultiviewRenderTestParams MultisampledVertexShaderVulkan(ExtensionName multiviewExtension)
{
    return MultiviewRenderTestParams(2, VertexShaderVulkan(3, 1, multiviewExtension));
}

MultiviewRenderTestParams MultisampledVertexShaderD3D11(ExtensionName multiviewExtension)
{
    return MultiviewRenderTestParams(2, VertexShaderD3D11(3, 1, multiviewExtension));
}

#define ALL_VERTEX_SHADER_CONFIGS(minor)                         \
    VertexShaderOpenGL(3, minor, ExtensionName::multiview),      \
        VertexShaderVulkan(3, minor, ExtensionName::multiview),  \
        VertexShaderD3D11(3, minor, ExtensionName::multiview),   \
        VertexShaderOpenGL(3, minor, ExtensionName::multiview2), \
        VertexShaderVulkan(3, minor, ExtensionName::multiview2), \
        VertexShaderD3D11(3, minor, ExtensionName::multiview2)

#define ALL_SINGLESAMPLE_CONFIGS()                                                              \
    VertexShaderOpenGL(ExtensionName::multiview), VertexShaderVulkan(ExtensionName::multiview), \
        VertexShaderD3D11(ExtensionName::multiview), GeomShaderD3D11(ExtensionName::multiview), \
        VertexShaderOpenGL(ExtensionName::multiview2),                                          \
        VertexShaderVulkan(ExtensionName::multiview2),                                          \
        VertexShaderD3D11(ExtensionName::multiview2), GeomShaderD3D11(ExtensionName::multiview2)

#define ALL_MULTISAMPLE_CONFIGS()                                  \
    MultisampledVertexShaderOpenGL(ExtensionName::multiview),      \
        MultisampledVertexShaderVulkan(ExtensionName::multiview),  \
        MultisampledVertexShaderD3D11(ExtensionName::multiview),   \
        MultisampledVertexShaderOpenGL(ExtensionName::multiview2), \
        MultisampledVertexShaderVulkan(ExtensionName::multiview2), \
        MultisampledVertexShaderD3D11(ExtensionName::multiview2)

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultiviewDrawValidationTest);
ANGLE_INSTANTIATE_TEST(MultiviewDrawValidationTest, ALL_VERTEX_SHADER_CONFIGS(1));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultiviewRenderDualViewTest);
ANGLE_INSTANTIATE_TEST(MultiviewRenderDualViewTest,
                       ALL_SINGLESAMPLE_CONFIGS(),
                       ALL_MULTISAMPLE_CONFIGS());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultiviewRenderDualViewTestNoWebGL);
ANGLE_INSTANTIATE_TEST(MultiviewRenderDualViewTestNoWebGL,
                       ALL_SINGLESAMPLE_CONFIGS(),
                       ALL_MULTISAMPLE_CONFIGS());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultiviewRenderTest);
ANGLE_INSTANTIATE_TEST(MultiviewRenderTest, ALL_SINGLESAMPLE_CONFIGS(), ALL_MULTISAMPLE_CONFIGS());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultiviewOcclusionQueryTest);
ANGLE_INSTANTIATE_TEST(MultiviewOcclusionQueryTest, ALL_SINGLESAMPLE_CONFIGS());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultiviewProgramGenerationTest);
ANGLE_INSTANTIATE_TEST(MultiviewProgramGenerationTest,
                       ALL_VERTEX_SHADER_CONFIGS(0),
                       GeomShaderD3D11(3, 0, ExtensionName::multiview),
                       GeomShaderD3D11(3, 0, ExtensionName::multiview2));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultiviewRenderPrimitiveTest);
ANGLE_INSTANTIATE_TEST(MultiviewRenderPrimitiveTest, ALL_SINGLESAMPLE_CONFIGS());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultiviewLayeredRenderTest);
ANGLE_INSTANTIATE_TEST(MultiviewLayeredRenderTest, ALL_SINGLESAMPLE_CONFIGS());
