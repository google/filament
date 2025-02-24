//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanUniformUpdatesTest:
//   Tests to validate our Vulkan dynamic uniform updates are working as expected.
//

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/random_utils.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{

class VulkanUniformUpdatesTest : public ANGLETest<>
{
  protected:
    VulkanUniformUpdatesTest() : mLastContext(nullptr) {}

    virtual void testSetUp() override
    {
        // Some of the tests bellow forces uniform buffer size to 128 bytes which may affect other
        // tests. This is to ensure that the assumption that each TEST_P will recreate context.
        ASSERT(mLastContext != getEGLWindow()->getContext());
        mLastContext = getEGLWindow()->getContext();

        mMaxSetsPerPool = rx::vk::DynamicDescriptorPool::GetMaxSetsPerPoolForTesting();
        mMaxSetsPerPoolMultiplier =
            rx::vk::DynamicDescriptorPool::GetMaxSetsPerPoolMultiplierForTesting();
    }

    void testTearDown() override
    {
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(mMaxSetsPerPool);
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(
            mMaxSetsPerPoolMultiplier);
    }

    gl::Context *hackContext() const
    {
        egl::Display *display   = static_cast<egl::Display *>(getEGLWindow()->getDisplay());
        gl::ContextID contextID = {
            static_cast<GLuint>(reinterpret_cast<uintptr_t>(getEGLWindow()->getContext()))};
        return display->getContext(contextID);
    }

    rx::ContextVk *hackANGLE() const
    {
        // Hack the angle!
        return rx::GetImplAs<rx::ContextVk>(hackContext());
    }

    rx::TextureVk *hackTexture(GLuint handle) const
    {
        // Hack the angle!
        const gl::Context *context = hackContext();
        const gl::Texture *texture = context->getTexture({handle});
        return rx::vk::GetImpl(texture);
    }

    static constexpr uint32_t kMaxSetsForTesting           = 1;
    static constexpr uint32_t kMaxSetsMultiplierForTesting = 1;

    void limitMaxSets()
    {
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(kMaxSetsForTesting);
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(
            kMaxSetsMultiplierForTesting);
    }

    void setExplicitMaxSetsLimit(uint32_t limit)
    {
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(limit);
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(
            kMaxSetsMultiplierForTesting);
    }

  private:
    EGLContext mLastContext;
    uint32_t mMaxSetsPerPool;
    uint32_t mMaxSetsPerPoolMultiplier;
};

// This test updates a uniform until a new buffer is allocated and then make sure the uniform
// updates still work.
TEST_P(VulkanUniformUpdatesTest, UpdateUntilNewBufferIsAllocated)
{
    ASSERT_TRUE(IsVulkan());

    constexpr char kPositionUniformVertexShader[] = R"(attribute vec2 position;
uniform vec2 uniPosModifier;
void main()
{
    gl_Position = vec4(position + uniPosModifier, 0, 1);
})";

    constexpr char kColorUniformFragmentShader[] = R"(precision mediump float;
uniform vec4 uniColor;
void main()
{
    gl_FragColor = uniColor;
})";

    ANGLE_GL_PROGRAM(program, kPositionUniformVertexShader, kColorUniformFragmentShader);
    glUseProgram(program);

    limitMaxSets();

    // Set a really small min size so that uniform updates often allocates a new buffer.
    rx::ContextVk *contextVk = hackANGLE();
    contextVk->setDefaultUniformBlocksMinSizeForTesting(128);

    GLint posUniformLocation = glGetUniformLocation(program, "uniPosModifier");
    ASSERT_NE(posUniformLocation, -1);
    GLint colorUniformLocation = glGetUniformLocation(program, "uniColor");
    ASSERT_NE(colorUniformLocation, -1);

    // Sets both uniforms 10 times, it should certainly trigger new buffers creations by the
    // underlying StreamingBuffer.
    for (int i = 0; i < 100; i++)
    {
        glUniform2f(posUniformLocation, -0.5, 0.0);
        glUniform4f(colorUniformLocation, 1.0, 0.0, 0.0, 1.0);
        drawQuad(program, "position", 0.5f, 1.0f);
        swapBuffers();
        ASSERT_GL_NO_ERROR();
    }
}

void InitTexture(GLColor color, GLTexture *texture)
{
    const std::vector<GLColor> colors(4, color);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

// Force uniform updates until the dynamic descriptor pool wraps into a new pool allocation.
TEST_P(VulkanUniformUpdatesTest, DescriptorPoolUpdates)
{
    ASSERT_TRUE(IsVulkan());

    // Initialize texture program.
    GLuint program = get2DTexturedQuadProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    // Force a small limit on the max sets per pool to more easily trigger a new allocation.
    limitMaxSets();

    GLint texLoc = glGetUniformLocation(program, "tex");
    ASSERT_NE(-1, texLoc);

    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, redColors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw multiple times, each iteration will create a new descriptor set.
    for (uint32_t iteration = 0; iteration < kMaxSetsForTesting * 8; ++iteration)
    {
        glUniform1i(texLoc, 0);
        drawQuad(program, "position", 0.5f, 1.0f, true);
        swapBuffers();
        ASSERT_GL_NO_ERROR();
    }
}

// Uniform updates along with Texture updates.
TEST_P(VulkanUniformUpdatesTest, DescriptorPoolUniformAndTextureUpdates)
{
    ASSERT_TRUE(IsVulkan());

    // Initialize texture program.
    constexpr char kFS[] = R"(varying mediump vec2 v_texCoord;
uniform sampler2D tex;
uniform mediump vec4 colorMask;
void main()
{
    gl_FragColor = texture2D(tex, v_texCoord) * colorMask;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);
    glUseProgram(program);

    limitMaxSets();

    // Get uniform locations.
    GLint texLoc = glGetUniformLocation(program, "tex");
    ASSERT_NE(-1, texLoc);

    GLint colorMaskLoc = glGetUniformLocation(program, "colorMask");
    ASSERT_NE(-1, colorMaskLoc);

    // Initialize white texture.
    GLTexture whiteTexture;
    InitTexture(GLColor::white, &whiteTexture);
    ASSERT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);

    // Initialize magenta texture.
    GLTexture magentaTexture;
    InitTexture(GLColor::magenta, &magentaTexture);
    ASSERT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, magentaTexture);

    // Draw multiple times, each iteration will create a new descriptor set.
    for (uint32_t iteration = 0; iteration < kMaxSetsForTesting * 2; ++iteration)
    {
        // Draw with white.
        glUniform1i(texLoc, 0);
        glUniform4f(colorMaskLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

        // Draw with white masking out red.
        glUniform4f(colorMaskLoc, 0.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

        // Draw with magenta.
        glUniform1i(texLoc, 1);
        glUniform4f(colorMaskLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

        // Draw with magenta masking out red.
        glUniform4f(colorMaskLoc, 0.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

        swapBuffers();
        ASSERT_GL_NO_ERROR();
    }
}

// Uniform updates along with Texture regeneration.
TEST_P(VulkanUniformUpdatesTest, DescriptorPoolUniformAndTextureRegeneration)
{
    ASSERT_TRUE(IsVulkan());

    // Initialize texture program.
    constexpr char kFS[] = R"(varying mediump vec2 v_texCoord;
uniform sampler2D tex;
uniform mediump vec4 colorMask;
void main()
{
    gl_FragColor = texture2D(tex, v_texCoord) * colorMask;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);
    glUseProgram(program);

    limitMaxSets();

    // Initialize large arrays of textures.
    std::vector<GLTexture> whiteTextures;
    std::vector<GLTexture> magentaTextures;

    for (uint32_t iteration = 0; iteration < kMaxSetsForTesting * 2; ++iteration)
    {
        // Initialize white texture.
        GLTexture whiteTexture;
        InitTexture(GLColor::white, &whiteTexture);
        ASSERT_GL_NO_ERROR();
        whiteTextures.emplace_back(std::move(whiteTexture));

        // Initialize magenta texture.
        GLTexture magentaTexture;
        InitTexture(GLColor::magenta, &magentaTexture);
        ASSERT_GL_NO_ERROR();
        magentaTextures.emplace_back(std::move(magentaTexture));
    }

    // Get uniform locations.
    GLint texLoc = glGetUniformLocation(program, "tex");
    ASSERT_NE(-1, texLoc);

    GLint colorMaskLoc = glGetUniformLocation(program, "colorMask");
    ASSERT_NE(-1, colorMaskLoc);

    // Draw multiple times, each iteration will create a new descriptor set.
    for (int outerIteration = 0; outerIteration < 2; ++outerIteration)
    {
        for (uint32_t iteration = 0; iteration < kMaxSetsForTesting * 2; ++iteration)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, whiteTextures[iteration]);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, magentaTextures[iteration]);

            // Draw with white.
            glUniform1i(texLoc, 0);
            glUniform4f(colorMaskLoc, 1.0f, 1.0f, 1.0f, 1.0f);
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

            // Draw with white masking out red.
            glUniform4f(colorMaskLoc, 0.0f, 1.0f, 1.0f, 1.0f);
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

            // Draw with magenta.
            glUniform1i(texLoc, 1);
            glUniform4f(colorMaskLoc, 1.0f, 1.0f, 1.0f, 1.0f);
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

            // Draw with magenta masking out red.
            glUniform4f(colorMaskLoc, 0.0f, 1.0f, 1.0f, 1.0f);
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);

            swapBuffers();
            ASSERT_GL_NO_ERROR();
        }
    }
}

// Uniform updates along with Texture updates.
TEST_P(VulkanUniformUpdatesTest, DescriptorPoolUniformAndTextureUpdatesTwoShaders)
{
    ASSERT_TRUE(IsVulkan());

    // Initialize program.
    constexpr char kVS[] = R"(attribute vec2 position;
varying mediump vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(varying mediump vec2 texCoord;
uniform mediump vec4 colorMask;
void main()
{
    gl_FragColor = colorMask;
})";

    ANGLE_GL_PROGRAM(program1, kVS, kFS);
    ANGLE_GL_PROGRAM(program2, kVS, kFS);
    glUseProgram(program1);

    // Force a small limit on the max sets per pool to more easily trigger a new allocation.
    limitMaxSets();
    limitMaxSets();

    // Set a really small min size so that uniform updates often allocates a new buffer.
    rx::ContextVk *contextVk = hackANGLE();
    contextVk->setDefaultUniformBlocksMinSizeForTesting(128);

    // Get uniform locations.
    GLint colorMaskLoc1 = glGetUniformLocation(program1, "colorMask");
    ASSERT_NE(-1, colorMaskLoc1);
    GLint colorMaskLoc2 = glGetUniformLocation(program2, "colorMask");
    ASSERT_NE(-1, colorMaskLoc2);

    // Draw with white using program1.
    glUniform4f(colorMaskLoc1, 1.0f, 1.0f, 1.0f, 1.0f);
    drawQuad(program1, "position", 0.5f, 1.0f, true);
    swapBuffers();
    ASSERT_GL_NO_ERROR();

    // Now switch to use program2
    glUseProgram(program2);
    // Draw multiple times w/ program2, each iteration will create a new descriptor set.
    // This will cause the first descriptor pool to be cleaned up
    for (uint32_t iteration = 0; iteration < kMaxSetsForTesting * 2; ++iteration)
    {
        // Draw with white.
        glUniform4f(colorMaskLoc2, 1.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(program2, "position", 0.5f, 1.0f, true);

        // Draw with white masking out red.
        glUniform4f(colorMaskLoc2, 0.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(program2, "position", 0.5f, 1.0f, true);

        // Draw with magenta.
        glUniform4f(colorMaskLoc2, 1.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(program2, "position", 0.5f, 1.0f, true);

        // Draw with magenta masking out red.
        glUniform4f(colorMaskLoc2, 0.0f, 1.0f, 1.0f, 1.0f);
        drawQuad(program2, "position", 0.5f, 1.0f, true);

        swapBuffers();
        ASSERT_GL_NO_ERROR();
    }
    // Finally, attempt to draw again with program1, with original uniform values.
    glUseProgram(program1);
    drawQuad(program1, "position", 0.5f, 1.0f, true);
    swapBuffers();
    ASSERT_GL_NO_ERROR();
}

// Verify that overflowing a Texture's staging buffer doesn't overwrite current data.
TEST_P(VulkanUniformUpdatesTest, TextureStagingBufferRecycling)
{
    ASSERT_TRUE(IsVulkan());

    // Fails on older MESA drivers.  http://crbug.com/979349
    ANGLE_SKIP_TEST_IF(IsAMD() && IsLinux());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const GLColor kColors[4] = {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow};

    // Repeatedly update the staging buffer to trigger multiple recyclings.
    const GLsizei kHalfX      = getWindowWidth() / 2;
    const GLsizei kHalfY      = getWindowHeight() / 2;
    constexpr int kIterations = 4;
    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            const int kColorIndex = x + y * 2;
            const GLColor kColor  = kColors[kColorIndex];

            for (int iteration = 0; iteration < kIterations; ++iteration)
            {
                for (int subX = 0; subX < kHalfX; ++subX)
                {
                    for (int subY = 0; subY < kHalfY; ++subY)
                    {
                        const GLsizei xoffset = x * kHalfX + subX;
                        const GLsizei yoffset = y * kHalfY + subY;

                        // Update a single pixel.
                        glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, 1, 1, GL_RGBA,
                                        GL_UNSIGNED_BYTE, kColor.data());
                    }
                }
            }
        }
    }

    draw2DTexturedQuad(0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Verify pixels.
    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            const GLsizei xoffset = x * kHalfX;
            const GLsizei yoffset = y * kHalfY;
            const int kColorIndex = x + y * 2;
            const GLColor kColor  = kColors[kColorIndex];
            EXPECT_PIXEL_RECT_EQ(xoffset, yoffset, kHalfX, kHalfY, kColor);
        }
    }
}

// This test tries to create a situation that VS and FS's uniform data might get placed in
// different buffers and verify uniforms not getting stale data.
TEST_P(VulkanUniformUpdatesTest, UpdateAfterNewBufferIsAllocated)
{
    ASSERT_TRUE(IsVulkan());

    constexpr char kPositionUniformVertexShader[] = R"(attribute vec2 position;
uniform float uniformVS;
varying vec4 outVS;
void main()
{
    outVS = vec4(uniformVS, uniformVS, uniformVS, uniformVS);
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kColorUniformFragmentShader[] = R"(precision mediump float;
varying vec4 outVS;
uniform float uniformFS;
void main()
{
    if(outVS[0] > uniformFS)
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    else
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kPositionUniformVertexShader, kColorUniformFragmentShader);
    glUseProgram(program);

    limitMaxSets();

    // Set a really small min size so that every uniform update actually allocates a new buffer.
    rx::ContextVk *contextVk = hackANGLE();
    contextVk->setDefaultUniformBlocksMinSizeForTesting(128);

    GLint uniformVSLocation = glGetUniformLocation(program, "uniformVS");
    ASSERT_NE(uniformVSLocation, -1);
    GLint uniformFSLocation = glGetUniformLocation(program, "uniformFS");
    ASSERT_NE(uniformFSLocation, -1);

    glUniform1f(uniformVSLocation, 10.0);
    glUniform1f(uniformFSLocation, 11.0);
    drawQuad(program, "position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();

    const GLsizei kHalfX  = getWindowWidth() / 2;
    const GLsizei kHalfY  = getWindowHeight() / 2;
    const GLsizei xoffset = kHalfX;
    const GLsizei yoffset = kHalfY;
    // 10.0f < 11.0f, should see green
    EXPECT_PIXEL_RECT_EQ(xoffset, yoffset, kHalfX, kHalfY, GLColor::green);

    // Now only update FS's uniform
    for (int i = 0; i < 3; i++)
    {
        glUniform1f(uniformFSLocation, 1.0f + i / 10.0f);
        drawQuad(program, "position", 0.5f, 1.0f);
        ASSERT_GL_NO_ERROR();
    }
    // 10.0f > 9.0f, should see red
    EXPECT_PIXEL_RECT_EQ(xoffset, yoffset, kHalfX, kHalfY, GLColor::red);

    // 10.0f < 11.0f, should see green again
    glUniform1f(uniformFSLocation, 11.0f);
    drawQuad(program, "position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(xoffset, yoffset, kHalfX, kHalfY, GLColor::green);

    // Now only update VS's uniform and flush the draw and readback and verify for every iteration.
    // This will ensure the old buffers are finished and possibly recycled.
    for (int i = 0; i < 100; i++)
    {
        // Make VS uniform value ping pong across FS uniform value
        float vsUniformValue  = (i % 2) == 0 ? (11.0 + (i - 50)) : (11.0 - (i - 50));
        GLColor expectedColor = vsUniformValue > 11.0f ? GLColor::red : GLColor::green;
        glUniform1f(uniformVSLocation, vsUniformValue);
        drawQuad(program, "position", 0.5f, 1.0f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_RECT_EQ(xoffset, yoffset, kHalfX, kHalfY, expectedColor);
    }
}

// Covers a usage pattern where two programs share a descriptor pool.
TEST_P(VulkanUniformUpdatesTest, MultipleProgramsShareDescriptors)
{
    setExplicitMaxSetsLimit(2);

    // Set a min size so uniform updates allocate a new buffer every 2nd time.
    rx::ContextVk *contextVk = hackANGLE();
    contextVk->setDefaultUniformBlocksMinSizeForTesting(512);

    constexpr size_t kNumPrograms                       = 2;
    constexpr size_t kDrawIterations                    = 4;
    constexpr GLint kPosLoc                             = 0;
    const std::array<Vector3, kDrawIterations> uniforms = {
        Vector3(0.1f, 0.2f, 0.3f), Vector3(0.4f, 0.5f, 0.6f), Vector3(0.7f, 0.8f, 0.9f),
        Vector3(0.1f, 0.5f, 0.9f)};
    const std::array<GLColor, kDrawIterations> expectedColors = {
        GLColor(25, 51, 76, 255), GLColor(102, 127, 153, 255), GLColor(178, 204, 229, 255),
        GLColor(25, 127, 229, 255)};

    std::array<GLuint, kNumPrograms> programs = {};

    for (GLuint &program : programs)
    {
        auto preLinkCallback = [](GLuint program) {
            glBindAttribLocation(program, kPosLoc, essl1_shaders::PositionAttrib());
        };

        program = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor(),
                                 preLinkCallback);
        ASSERT_NE(program, 0u);
    }

    const std::array<Vector3, 6> &quadVerts = GetQuadVertices();

    GLBuffer vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(kPosLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(kPosLoc);

    ASSERT_GL_NO_ERROR();

    for (size_t drawIteration = 0; drawIteration < kDrawIterations; ++drawIteration)
    {
        for (GLuint program : programs)
        {
            glUseProgram(program);
            glUniform4f(0, uniforms[drawIteration].x(), uniforms[drawIteration].y(),
                        uniforms[drawIteration].z(), 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            EXPECT_PIXEL_COLOR_NEAR(0, 0, expectedColors[drawIteration], 5);
        }
    }

    ASSERT_GL_NO_ERROR();

    for (GLuint &program : programs)
    {
        glDeleteProgram(program);
    }
}

ANGLE_INSTANTIATE_TEST(VulkanUniformUpdatesTest, ES2_VULKAN(), ES3_VULKAN());

// This test tries to test uniform data update while switching between PPO and monolithic program.
// The uniform data update occurred on one should carry over to the other. Also buffers are hacked
// to smallest size to force updates occur in the new buffer so that any bug related to buffer
// recycling will be exposed.
class PipelineProgramUniformUpdatesTest : public VulkanUniformUpdatesTest
{};
TEST_P(PipelineProgramUniformUpdatesTest, ToggleBetweenPPOAndProgramVKWithUniformUpdate)
{
    ASSERT_TRUE(IsVulkan());

    const GLchar *kPositionUniformVertexShader = R"(attribute vec2 position;
uniform float uniformVS;
varying vec4 outVS;
void main()
{
    outVS = vec4(uniformVS, uniformVS, uniformVS, uniformVS);
    gl_Position = vec4(position, 0, 1);
})";

    const GLchar *kColorUniformFragmentShader = R"(precision mediump float;
varying vec4 outVS;
uniform float uniformFS;
void main()
{
    if(outVS[0] > uniformFS)
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    else
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
})";

    // Compile and link a separable vertex shader
    GLShader vertShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &kPositionUniformVertexShader, nullptr);
    glCompileShader(vertShader);
    GLShader fragShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &kColorUniformFragmentShader, nullptr);
    glCompileShader(fragShader);
    GLuint program = glCreateProgram();
    glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    limitMaxSets();
    // Set a really small min size so that every uniform update actually allocates a new buffer.
    rx::ContextVk *contextVk = hackANGLE();
    contextVk->setDefaultUniformBlocksMinSizeForTesting(128);

    // Setup vertices
    std::array<Vector3, 6> quadVertices = ANGLETestBase::GetQuadVertices();
    GLint positionLocation              = glGetAttribLocation(program, "position");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, quadVertices.data());
    glEnableVertexAttribArray(positionLocation);

    GLint uniformVSLocation = glGetUniformLocation(program, "uniformVS");
    ASSERT_NE(uniformVSLocation, -1);
    GLint uniformFSLocation = glGetUniformLocation(program, "uniformFS");
    ASSERT_NE(uniformFSLocation, -1);

    glUseProgram(0);

    // Generate a pipeline program out of the monolithic program
    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, program);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();

    // First use monolithic program and update uniforms
    glUseProgram(program);
    glUniform1f(uniformVSLocation, 10.0);
    glUniform1f(uniformFSLocation, 11.0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    const GLsizei kHalfX  = getWindowWidth() / 2;
    const GLsizei kHalfY  = getWindowHeight() / 2;
    const GLsizei xoffset = kHalfX;
    const GLsizei yoffset = kHalfY;
    // 10.0f < 11.0f, should see green
    EXPECT_PIXEL_RECT_EQ(xoffset, yoffset, kHalfX, kHalfY, GLColor::green);

    // Now use PPO and only update FS's uniform
    glUseProgram(0);
    for (int i = 0; i < 3; i++)
    {
        glActiveShaderProgram(pipeline, program);
        glUniform1f(uniformFSLocation, 1.0f + i / 10.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
    }
    // 10.0f > 9.0f, should see red
    EXPECT_PIXEL_RECT_EQ(xoffset, yoffset, kHalfX, kHalfY, GLColor::red);

    // Now switch back to monolithic program and only update FS's uniform.
    // 10.0f < 11.0f, should see green again
    glUseProgram(program);
    glUniform1f(uniformFSLocation, 11.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(xoffset, yoffset, kHalfX, kHalfY, GLColor::green);

    // Now only update VS's uniform and flush the draw and readback and verify for every iteration.
    // This will ensure the old buffers are finished and possibly recycled.
    for (int i = 0; i < 100; i++)
    {
        bool iteration_even = (i % 2) == 0 ? true : false;
        float vsUniformValue;

        // Make VS uniform value ping pong across FS uniform value and also pin pong between
        // monolithic program and PPO
        if (iteration_even)
        {
            vsUniformValue = 11.0 + (i - 50);
            glUseProgram(program);
            glUniform1f(uniformVSLocation, vsUniformValue);
        }
        else
        {
            vsUniformValue = 11.0 - (i - 50);
            glUseProgram(0);
            glActiveShaderProgram(pipeline, program);
            glUniform1f(uniformVSLocation, vsUniformValue);
        }

        GLColor expectedColor = vsUniformValue > 11.0f ? GLColor::red : GLColor::green;
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_RECT_EQ(xoffset, yoffset, kHalfX, kHalfY, expectedColor);
    }
}

ANGLE_INSTANTIATE_TEST(PipelineProgramUniformUpdatesTest, ES31_VULKAN());

}  // anonymous namespace
