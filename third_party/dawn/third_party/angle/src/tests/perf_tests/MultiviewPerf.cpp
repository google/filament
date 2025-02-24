//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MultiviewPerfTest:
//   Performance tests for multiview rendering.
//   - MultiviewCPUBoundBenchmark issues many draw calls and state changes to stress the CPU.
//   - MultiviewGPUBoundBenchmark draws half a million quads with multiple attributes per vertex in
//   order to stress the GPU's memory system.
//

#include "ANGLEPerfTest.h"
#include "common/vector_utils.h"
#include "platform/autogen/FeaturesD3D_autogen.h"
#include "test_utils/MultiviewTest.h"
#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

#include <string.h>

using namespace angle;

namespace
{

std::string GetShaderExtensionHeader(bool usesMultiview,
                                     int numViews,
                                     GLenum shaderType,
                                     ExtensionName multiviewExtension)
{
    if (!usesMultiview)
    {
        return "";
    }

    std::string ext;
    switch (multiviewExtension)
    {
        case multiview:
            ext = "GL_OVR_multiview";
            break;
        case multiview2:
            ext = "GL_OVR_multiview2";
            break;
        default:
            ext = "extension_error";
    }

    if (shaderType == GL_VERTEX_SHADER)
    {
        return "#extension " + ext + " : require\nlayout(num_views = " + ToString(numViews) +
               ") in;\n";
    }
    return "#extension " + ext + " : require\n";
}

struct Vertex
{
    Vector4 position;
    Vector4 colorAttributeData[6];
};

enum class MultiviewOption
{
    NoAcceleration,
    InstancedMultiviewVertexShader,
    InstancedMultiviewGeometryShader,

    Unspecified
};

using MultiviewPerfWorkload = std::pair<int, int>;

struct MultiviewPerfParams final : public RenderTestParams
{
    MultiviewPerfParams(const EGLPlatformParameters &platformParametersIn,
                        const MultiviewPerfWorkload &workloadIn,
                        MultiviewOption multiviewOptionIn,
                        ExtensionName multiviewExtensionIn)
    {
        iterationsPerStep  = 1;
        majorVersion       = 3;
        minorVersion       = 0;
        eglParameters      = platformParametersIn;
        windowWidth        = workloadIn.first;
        windowHeight       = workloadIn.second;
        multiviewOption    = multiviewOptionIn;
        numViews           = 2;
        multiviewExtension = multiviewExtensionIn;

        if (multiviewOption == MultiviewOption::InstancedMultiviewGeometryShader)
        {
            eglParameters.enable(Feature::SelectViewInGeometryShader);
        }
    }

    std::string story() const override
    {
        std::string name = RenderTestParams::story();
        switch (multiviewOption)
        {
            case MultiviewOption::NoAcceleration:
                name += "_no_acc";
                break;
            case MultiviewOption::InstancedMultiviewVertexShader:
                name += "_instanced_multiview_vertex_shader";
                break;
            case MultiviewOption::InstancedMultiviewGeometryShader:
                name += "_instanced_multiview_geometry_shader";
                break;
            default:
                name += "_error";
                break;
        }
        std::string ext;
        switch (multiviewExtension)
        {
            case multiview:
                ext = "GL_OVR_multiview";
                break;
            case multiview2:
                ext = "GL_OVR_multiview2";
                break;
            default:
                ext = "extension_error";
                break;
        }
        name += "_" + ext;
        name += "_" + ToString(numViews) + "_views";
        return name;
    }

    MultiviewOption multiviewOption;
    int numViews;
    angle::ExtensionName multiviewExtension;
};

std::ostream &operator<<(std::ostream &os, const MultiviewPerfParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class MultiviewBenchmark : public ANGLERenderTest,
                           public ::testing::WithParamInterface<MultiviewPerfParams>
{
  public:
    MultiviewBenchmark(const std::string &testName)
        : ANGLERenderTest(testName, GetParam()), mProgram(0)
    {
        switch (GetParam().multiviewExtension)
        {
            case multiview:
                addExtensionPrerequisite("GL_OVR_multiview");
                break;
            case multiview2:
                addExtensionPrerequisite("GL_OVR_multiview2");
                break;
            default:
                // Unknown extension.
                break;
        }
    }

    virtual ~MultiviewBenchmark()
    {
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
    }

    void initializeBenchmark() override;
    void drawBenchmark() final;

  protected:
    virtual void renderScene() = 0;

    void createProgram(const char *vs, const char *fs)
    {
        mProgram = CompileProgram(vs, fs);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }
        glUseProgram(mProgram);
        ASSERT_GL_NO_ERROR();
    }

    GLuint mProgram;
    GLVertexArray mVAO;
    GLBuffer mVBO;

  private:
    GLFramebuffer mFramebuffer;
    GLTexture mColorTexture;
    GLTexture mDepthTexture;
};

class MultiviewCPUBoundBenchmark : public MultiviewBenchmark
{
  public:
    MultiviewCPUBoundBenchmark() : MultiviewBenchmark("MultiviewCPUBoundBenchmark") {}

    void initializeBenchmark() override;

  protected:
    void renderScene() override;
};

class MultiviewGPUBoundBenchmark : public MultiviewBenchmark
{
  public:
    MultiviewGPUBoundBenchmark() : MultiviewBenchmark("MultiviewGPUBoundBenchmark") {}

    void initializeBenchmark() override;

  protected:
    void renderScene() override;
};

void MultiviewBenchmark::initializeBenchmark()
{
    const MultiviewPerfParams *params = static_cast<const MultiviewPerfParams *>(&mTestParams);

    switch (params->multiviewOption)
    {
        case MultiviewOption::NoAcceleration:
            // No acceleration texture arrays
            glBindTexture(GL_TEXTURE_2D, mColorTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, params->windowWidth, params->windowHeight, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glBindTexture(GL_TEXTURE_2D, mDepthTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, params->windowWidth,
                         params->windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

            glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   mColorTexture, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                                   mDepthTexture, 0);
            break;
        case MultiviewOption::InstancedMultiviewVertexShader:
        case MultiviewOption::InstancedMultiviewGeometryShader:
        {
            // Multiview texture arrays
            glBindTexture(GL_TEXTURE_2D_ARRAY, mColorTexture);
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, params->windowWidth,
                         params->windowHeight, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glBindTexture(GL_TEXTURE_2D_ARRAY, mDepthTexture);
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, params->windowWidth,
                         params->windowHeight, 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

            glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
            glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColorTexture, 0,
                                             0, params->numViews);
            glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture, 0,
                                             0, params->numViews);
            break;
        }
        case MultiviewOption::Unspecified:
            // implementation error.
            break;
    }

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);

    ASSERT_GL_NO_ERROR();
}

void MultiviewBenchmark::drawBenchmark()
{
    const MultiviewPerfParams *params = static_cast<const MultiviewPerfParams *>(&mTestParams);
    const int viewWidth               = params->windowWidth / params->numViews;
    const int viewHeight              = params->windowHeight;

    switch (params->multiviewOption)
    {
        case MultiviewOption::NoAcceleration:
            glEnable(GL_SCISSOR_TEST);
            // Iterate over each view and render the scene.
            for (int i = 0; i < params->numViews; ++i)
            {
                glViewport(viewWidth * i, 0, viewWidth, viewHeight);
                glScissor(viewWidth * i, 0, viewWidth, viewHeight);
                renderScene();
            }
            break;
        case MultiviewOption::InstancedMultiviewVertexShader:
        case MultiviewOption::InstancedMultiviewGeometryShader:
            glViewport(0, 0, viewWidth, viewHeight);
            glScissor(0, 0, viewWidth, viewHeight);
            renderScene();
            break;
        case MultiviewOption::Unspecified:
            // implementation error.
            break;
    }

    ASSERT_GL_NO_ERROR();
}

void MultiviewCPUBoundBenchmark::initializeBenchmark()
{
    MultiviewBenchmark::initializeBenchmark();

    const MultiviewPerfParams *params = static_cast<const MultiviewPerfParams *>(&mTestParams);
    const bool usesMultiview = (params->multiviewOption != MultiviewOption::NoAcceleration);

    const std::string vs = "#version 300 es\n" +
                           GetShaderExtensionHeader(usesMultiview, params->numViews,
                                                    GL_VERTEX_SHADER, params->multiviewExtension) +
                           "layout(location=0) in vec4 vPosition;\n"
                           "uniform vec2 uOffset;\n"
                           "void main()\n"
                           "{\n"
                           "   vec4 v = vPosition;\n"
                           "   v.xy += uOffset;\n"
                           "    gl_Position = v;\n"
                           "}\n";

    const std::string fs =
        "#version 300 es\n" +
        GetShaderExtensionHeader(usesMultiview, params->numViews, GL_FRAGMENT_SHADER,
                                 params->multiviewExtension) +
        "precision mediump float;\n"
        "out vec4 col;\n"
        "uniform float uColor;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(1.);\n"
        "}\n";

    createProgram(vs.c_str(), fs.c_str());

    const float viewWidth  = static_cast<float>(params->windowWidth / params->numViews);
    const float viewHeight = static_cast<float>(params->windowHeight);
    const float quadWidth  = 2.f / viewWidth;
    const float quadHeight = 2.f / viewHeight;
    Vector4 vertices[6]    = {Vector4(.0f, .0f, .0f, 1.f),
                              Vector4(quadWidth, .0f, .0f, 1.f),
                              Vector4(quadWidth, quadHeight, 0.f, 1.f),
                              Vector4(.0f, .0f, 0.f, 1.f),
                              Vector4(quadWidth, quadHeight, .0f, 1.f),
                              Vector4(.0f, quadHeight, .0f, 1.f)};

    glBindVertexArray(mVAO);

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vector4), vertices, GL_STATIC_DRAW);

    const GLint posLoc = glGetAttribLocation(mProgram, "vPosition");
    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLoc);

    // Render once to guarantee that the program is compiled and linked.
    drawBenchmark();

    ASSERT_GL_NO_ERROR();
}

void MultiviewCPUBoundBenchmark::renderScene()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(mProgram);

    glBindVertexArray(mVAO);

    const MultiviewPerfParams *params = static_cast<const MultiviewPerfParams *>(&mTestParams);
    const int viewWidth               = params->windowWidth / params->numViews;
    const int viewHeight              = params->windowHeight;

    for (int w = 0; w < viewWidth; ++w)
    {
        for (int h = 0; h < viewHeight; ++h)
        {
            const float wf = static_cast<float>(w) / viewWidth;
            const float wh = static_cast<float>(h) / viewHeight;
            glUniform2f(glGetUniformLocation(mProgram, "uOffset"), 2.f * wf - 1.f, 2.f * wh - 1.f);
            glUniform1f(glGetUniformLocation(mProgram, "uColor"), wf);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
}

void MultiviewGPUBoundBenchmark::initializeBenchmark()
{
    MultiviewBenchmark::initializeBenchmark();

    const MultiviewPerfParams *params = static_cast<const MultiviewPerfParams *>(&mTestParams);
    const bool usesMultiview = (params->multiviewOption != MultiviewOption::NoAcceleration);

    const std::string &vs = "#version 300 es\n" +
                            GetShaderExtensionHeader(usesMultiview, params->numViews,
                                                     GL_VERTEX_SHADER, params->multiviewExtension) +
                            "layout(location=0) in vec4 vPosition;\n"
                            "layout(location=1) in vec4 vert_Col0;\n"
                            "layout(location=2) in vec4 vert_Col1;\n"
                            "layout(location=3) in vec4 vert_Col2;\n"
                            "layout(location=4) in vec4 vert_Col3;\n"
                            "layout(location=5) in vec4 vert_Col4;\n"
                            "layout(location=6) in vec4 vert_Col5;\n"
                            "out vec4 frag_Col0;\n"
                            "out vec4 frag_Col1;\n"
                            "out vec4 frag_Col2;\n"
                            "out vec4 frag_Col3;\n"
                            "out vec4 frag_Col4;\n"
                            "out vec4 frag_Col5;\n"
                            "void main()\n"
                            "{\n"
                            "   frag_Col0 = vert_Col0;\n"
                            "   frag_Col1 = vert_Col1;\n"
                            "   frag_Col2 = vert_Col2;\n"
                            "   frag_Col3 = vert_Col3;\n"
                            "   frag_Col4 = vert_Col4;\n"
                            "   frag_Col5 = vert_Col5;\n"
                            "   gl_Position = vPosition;\n"
                            "}\n";

    const std::string &fs =
        "#version 300 es\n" +
        GetShaderExtensionHeader(usesMultiview, params->numViews, GL_FRAGMENT_SHADER,
                                 params->multiviewExtension) +
        "precision mediump float;\n"
        "in vec4 frag_Col0;\n"
        "in vec4 frag_Col1;\n"
        "in vec4 frag_Col2;\n"
        "in vec4 frag_Col3;\n"
        "in vec4 frag_Col4;\n"
        "in vec4 frag_Col5;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col += frag_Col0;\n"
        "    col += frag_Col1;\n"
        "    col += frag_Col2;\n"
        "    col += frag_Col3;\n"
        "    col += frag_Col4;\n"
        "    col += frag_Col5;\n"
        "}\n";

    createProgram(vs.c_str(), fs.c_str());
    ASSERT_GL_NO_ERROR();

    // Generate a vertex buffer of triangulated quads so that we have one quad per pixel.
    const int viewWidth           = params->windowWidth / params->numViews;
    const int viewHeight          = params->windowHeight;
    const float quadWidth         = 2.f / static_cast<float>(viewWidth);
    const float quadHeight        = 2.f / static_cast<float>(viewHeight);
    const int kNumQuads           = viewWidth * viewHeight;
    const int kNumVerticesPerQuad = 6;
    std::vector<Vertex> vertexData(kNumQuads * kNumVerticesPerQuad);
    for (int h = 0; h < viewHeight; ++h)
    {
        for (int w = 0; w < viewWidth; ++w)
        {
            float wf = static_cast<float>(w) / viewWidth;
            float hf = static_cast<float>(h) / viewHeight;

            size_t index = static_cast<size_t>(h * viewWidth + w) * 6u;

            auto &v0    = vertexData[index];
            v0.position = Vector4(2.f * wf - 1.f, 2.f * hf - 1.f, .0f, 1.f);
            memset(v0.colorAttributeData, 0, sizeof(v0.colorAttributeData));

            auto &v1    = vertexData[index + 1];
            v1.position = Vector4(v0.position.x() + quadWidth, v0.position.y(), .0f, 1.f);
            memset(v1.colorAttributeData, 0, sizeof(v1.colorAttributeData));

            auto &v2    = vertexData[index + 2];
            v2.position = Vector4(v1.position.x(), v1.position.y() + quadHeight, .0f, 1.f);
            memset(v2.colorAttributeData, 0, sizeof(v2.colorAttributeData));

            auto &v3    = vertexData[index + 3];
            v3.position = v0.position;
            memset(v3.colorAttributeData, 0, sizeof(v3.colorAttributeData));

            auto &v4    = vertexData[index + 4];
            v4.position = v2.position;
            memset(v4.colorAttributeData, 0, sizeof(v4.colorAttributeData));

            auto &v5    = vertexData[index + 5];
            v5.position = Vector4(v0.position.x(), v0.position.y() + quadHeight, .0f, 1.f);
            memset(v5.colorAttributeData, 0, sizeof(v5.colorAttributeData));
        }
    }

    glBindVertexArray(mVAO);

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(0);

    for (unsigned int i = 0u; i < 6u; ++i)
    {
        size_t offset = sizeof(Vector4) * (i + 1u);
        glVertexAttribPointer(i + 1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<const void *>(offset));
        glEnableVertexAttribArray(i + 1);
    }

    // Render once to guarantee that the program is compiled and linked.
    drawBenchmark();
}

void MultiviewGPUBoundBenchmark::renderScene()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(mProgram);

    glBindVertexArray(mVAO);

    const MultiviewPerfParams *params = static_cast<const MultiviewPerfParams *>(&mTestParams);
    const int viewWidth               = params->windowWidth / params->numViews;
    const int viewHeight              = params->windowHeight;
    glDrawArrays(GL_TRIANGLES, 0, viewWidth * viewHeight * 6);
}

namespace
{
MultiviewPerfWorkload SmallWorkload()
{
    return MultiviewPerfWorkload(64, 64);
}

MultiviewPerfWorkload BigWorkload()
{
    return MultiviewPerfWorkload(1024, 768);
}

MultiviewPerfParams NoAcceleration(const EGLPlatformParameters &eglParameters,
                                   const MultiviewPerfWorkload &workload,
                                   ExtensionName multiviewExtensionIn)
{
    return MultiviewPerfParams(eglParameters, workload, MultiviewOption::NoAcceleration,
                               multiviewExtensionIn);
}

MultiviewPerfParams SelectViewInGeometryShader(const MultiviewPerfWorkload &workload,
                                               ExtensionName multiviewExtensionIn)
{
    return MultiviewPerfParams(egl_platform::D3D11(), workload,
                               MultiviewOption::InstancedMultiviewGeometryShader,
                               multiviewExtensionIn);
}

MultiviewPerfParams SelectViewInVertexShader(const EGLPlatformParameters &eglParameters,
                                             const MultiviewPerfWorkload &workload,
                                             ExtensionName multiviewExtensionIn)
{
    return MultiviewPerfParams(eglParameters, workload,
                               MultiviewOption::InstancedMultiviewVertexShader,
                               multiviewExtensionIn);
}
}  // namespace

TEST_P(MultiviewCPUBoundBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(
    MultiviewCPUBoundBenchmark,
    NoAcceleration(egl_platform::OPENGL_OR_GLES(), SmallWorkload(), ExtensionName::multiview),
    NoAcceleration(egl_platform::D3D11(), SmallWorkload(), ExtensionName::multiview),
    SelectViewInGeometryShader(SmallWorkload(), ExtensionName::multiview),
    SelectViewInVertexShader(egl_platform::OPENGL_OR_GLES(),
                             SmallWorkload(),
                             ExtensionName::multiview),
    SelectViewInVertexShader(egl_platform::D3D11(), SmallWorkload(), ExtensionName::multiview),
    NoAcceleration(egl_platform::OPENGL_OR_GLES(), SmallWorkload(), ExtensionName::multiview2),
    NoAcceleration(egl_platform::D3D11(), SmallWorkload(), ExtensionName::multiview2),
    SelectViewInGeometryShader(SmallWorkload(), ExtensionName::multiview2),
    SelectViewInVertexShader(egl_platform::OPENGL_OR_GLES(),
                             SmallWorkload(),
                             ExtensionName::multiview2),
    SelectViewInVertexShader(egl_platform::D3D11(), SmallWorkload(), ExtensionName::multiview2));

TEST_P(MultiviewGPUBoundBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(
    MultiviewGPUBoundBenchmark,
    NoAcceleration(egl_platform::OPENGL_OR_GLES(), BigWorkload(), ExtensionName::multiview),
    NoAcceleration(egl_platform::D3D11(), BigWorkload(), ExtensionName::multiview),
    SelectViewInGeometryShader(BigWorkload(), ExtensionName::multiview),
    SelectViewInVertexShader(egl_platform::OPENGL_OR_GLES(),
                             BigWorkload(),
                             ExtensionName::multiview),
    SelectViewInVertexShader(egl_platform::D3D11(), BigWorkload(), ExtensionName::multiview),
    NoAcceleration(egl_platform::OPENGL_OR_GLES(), BigWorkload(), ExtensionName::multiview2),
    NoAcceleration(egl_platform::D3D11(), BigWorkload(), ExtensionName::multiview2),
    SelectViewInGeometryShader(BigWorkload(), ExtensionName::multiview2),
    SelectViewInVertexShader(egl_platform::OPENGL_OR_GLES(),
                             BigWorkload(),
                             ExtensionName::multiview2),
    SelectViewInVertexShader(egl_platform::D3D11(), BigWorkload(), ExtensionName::multiview2));

}  // anonymous namespace
