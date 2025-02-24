//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BlobCacheTest:
//   Unit tests for the GL_ANGLE_blob_cache extension.

// Must be included first to prevent errors with "None".
#include "test_utils/ANGLETest.h"

#include <map>
#include <vector>

#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "test_utils/MultiThreadSteps.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

namespace angle
{
constexpr char kExtName[] = "GL_ANGLE_blob_cache";

enum class CacheOpResult
{
    SetSuccess,
    GetNotFound,
    GetMemoryTooSmall,
    GetSuccess,
    ValueNotSet,
    EnumCount
};

angle::PackedEnumMap<CacheOpResult, std::string> kCacheOpToString = {
    {CacheOpResult::SetSuccess, "SetSuccess"},
    {CacheOpResult::GetNotFound, "GetNotFound"},
    {CacheOpResult::GetMemoryTooSmall, "GetMemoryTooSmall"},
    {CacheOpResult::GetSuccess, "GetSuccess"},
    {CacheOpResult::ValueNotSet, "ValueNotSet"},
};

std::ostream &operator<<(std::ostream &os, CacheOpResult result)
{
    return os << kCacheOpToString[result];
}

struct TestUserData
{
    std::map<std::vector<uint8_t>, std::vector<uint8_t>> cache;
    CacheOpResult cacheOpResult = CacheOpResult::ValueNotSet;
};

void GL_APIENTRY SetBlob(const void *key,
                         GLsizeiptr keySize,
                         const void *value,
                         GLsizeiptr valueSize,
                         const void *userParam)
{
    TestUserData *data = reinterpret_cast<TestUserData *>(const_cast<void *>(userParam));

    std::vector<uint8_t> keyVec(keySize);
    memcpy(keyVec.data(), key, keySize);

    std::vector<uint8_t> valueVec(valueSize);
    memcpy(valueVec.data(), value, valueSize);

    data->cache[keyVec] = valueVec;

    data->cacheOpResult = CacheOpResult::SetSuccess;
}

void GL_APIENTRY SetCorruptedBlob(const void *key,
                                  GLsizeiptr keySize,
                                  const void *value,
                                  GLsizeiptr valueSize,
                                  const void *userParam)
{
    TestUserData *data = reinterpret_cast<TestUserData *>(const_cast<void *>(userParam));

    std::vector<uint8_t> keyVec(keySize);
    memcpy(keyVec.data(), key, keySize);

    std::vector<uint8_t> valueVec(valueSize);
    memcpy(valueVec.data(), value, valueSize);

    // Corrupt the data
    ++valueVec[valueVec.size() / 2];
    ++valueVec[valueVec.size() / 3];
    ++valueVec[valueVec.size() / 4];
    ++valueVec[2 * valueVec.size() / 3];
    ++valueVec[3 * valueVec.size() / 4];

    data->cache[keyVec] = valueVec;

    data->cacheOpResult = CacheOpResult::SetSuccess;
}

GLsizeiptr GL_APIENTRY GetBlob(const void *key,
                               GLsizeiptr keySize,
                               void *value,
                               GLsizeiptr valueSize,
                               const void *userParam)
{
    TestUserData *data = reinterpret_cast<TestUserData *>(const_cast<void *>(userParam));

    std::vector<uint8_t> keyVec(keySize);
    memcpy(keyVec.data(), key, keySize);

    auto entry = data->cache.find(keyVec);
    if (entry == data->cache.end())
    {
        // A compile+link operation can generate multiple queries to the cache; one per shader and
        // one for link.  For the purposes of the test, make sure that any of these hitting the
        // cache is considered a success, particularly because it's valid for the pipeline cache
        // entry not to exist in the cache.
        if (data->cacheOpResult != CacheOpResult::GetSuccess)
        {
            data->cacheOpResult = CacheOpResult::GetNotFound;
        }
        return 0;
    }

    if (entry->second.size() <= static_cast<size_t>(valueSize))
    {
        memcpy(value, entry->second.data(), entry->second.size());
        data->cacheOpResult = CacheOpResult::GetSuccess;
    }
    else
    {
        data->cacheOpResult = CacheOpResult::GetMemoryTooSmall;
    }

    return entry->second.size();
}

void WaitProgramBinaryReady(GLuint program)
{
    // Using GL_ANGLE_program_binary_readiness_query, wait for post-link tasks to finish.
    // Otherwise, the program binary may not yet be cached.  Only needed when a |set| operation is
    // expected.
    if (!IsGLExtensionEnabled("GL_ANGLE_program_binary_readiness_query"))
    {
        return;
    }

    GLint ready = false;
    while (!ready)
    {
        glGetProgramiv(program, GL_PROGRAM_BINARY_READY_ANGLE, &ready);
        angle::Sleep(0);
    }
}

class BlobCacheTest : public ANGLETest<>
{
  protected:
    BlobCacheTest() : mHasBlobCache(false)
    {
        // Force disply caching off. Blob cache functions require it.
        forceNewDisplay();
    }

    void testSetUp() override { mHasBlobCache = EnsureGLExtensionEnabled(kExtName); }

    void testTearDown() override {}

    bool programBinaryAvailable() { return IsGLExtensionEnabled("GL_OES_get_program_binary"); }

    bool mHasBlobCache;
};

// Makes sure the extension exists and works
TEST_P(BlobCacheTest, Functional)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    EXPECT_TRUE(mHasBlobCache);

    TestUserData data;
    glBlobCacheCallbacksANGLE(SetBlob, GetBlob, &data);
    ASSERT_EGL_SUCCESS();

    constexpr char kVertexShaderSrc[] = R"(attribute vec4 aTest;
attribute vec2 aPosition;
varying vec4 vTest;
void main()
{
    vTest        = aTest;
    gl_Position  = vec4(aPosition, 0.0, 1.0);
    gl_PointSize = 1.0;
})";

    constexpr char kFragmentShaderSrc[] = R"(precision mediump float;
varying vec4 vTest;
void main()
{
    gl_FragColor = vTest;
})";

    constexpr char kVertexShaderSrc2[] = R"(attribute vec4 aTest;
attribute vec2 aPosition;
varying vec4 vTest;
void main()
{
    vTest        = aTest;
    gl_Position  = vec4(aPosition, 1.0, 1.0);
    gl_PointSize = 1.0;
})";

    constexpr char kFragmentShaderSrc2[] = R"(precision mediump float;
varying vec4 vTest;
void main()
{
    gl_FragColor = vTest - vec4(0.0, 1.0, 0.0, 0.0);
})";

    // Compile a shader so it puts something in the cache.  Note that with Vulkan, some optional
    // link subtasks may run beyond link, and so the caching is delayed.  An explicit wait on these
    // tasks is done for this reason.
    if (programBinaryAvailable())
    {
        ANGLE_GL_PROGRAM(program, kVertexShaderSrc, kFragmentShaderSrc);
        WaitProgramBinaryReady(program);
        EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
        data.cacheOpResult = CacheOpResult::ValueNotSet;

        // Compile the same shader again, so it would try to retrieve it from the cache
        program.makeRaster(kVertexShaderSrc, kFragmentShaderSrc);
        ASSERT_TRUE(program.valid());
        EXPECT_EQ(CacheOpResult::GetSuccess, data.cacheOpResult);
        data.cacheOpResult = CacheOpResult::ValueNotSet;

        // Compile another shader, which should create a new entry
        program.makeRaster(kVertexShaderSrc2, kFragmentShaderSrc2);
        ASSERT_TRUE(program.valid());
        WaitProgramBinaryReady(program);
        EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
        data.cacheOpResult = CacheOpResult::ValueNotSet;

        // Compile the first shader again, which should still reside in the cache
        program.makeRaster(kVertexShaderSrc, kFragmentShaderSrc);
        ASSERT_TRUE(program.valid());
        EXPECT_EQ(CacheOpResult::GetSuccess, data.cacheOpResult);
        data.cacheOpResult = CacheOpResult::ValueNotSet;

        // Make sure deleting the program doesn't result in a binary save.  Regression test for a
        // bug where the binary was re-cached after being loaded.
        glUseProgram(0);
        program.reset();

        EXPECT_EQ(CacheOpResult::ValueNotSet, data.cacheOpResult);
    }
}

// Makes sure the caching is always done without an explicit wait for post-link events (if any)
TEST_P(BlobCacheTest, FunctionalWithoutWait)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    EXPECT_TRUE(mHasBlobCache);

    TestUserData data;
    glBlobCacheCallbacksANGLE(SetBlob, GetBlob, &data);
    ASSERT_GL_NO_ERROR();

    constexpr char kVertexShaderSrc[] = R"(attribute vec4 aTest;
attribute vec2 aPosition;
varying vec4 vTest;
varying vec4 vTest2;
void main()
{
    vTest        = aTest;
    vTest2       = aTest;
    gl_Position  = vec4(aPosition, 1.0, 1.0);
    gl_PointSize = 1.0;
})";

    constexpr char kFragmentShaderSrc[] = R"(precision mediump float;
varying vec4 vTest;
varying vec4 vTest2;
void main()
{
    gl_FragColor = vTest + vTest2 - vec4(0.0, 1.0, 0.0, 0.0);
})";

    if (programBinaryAvailable())
    {
        // Make the conditions ideal for Vulkan's warm up task to match the draw call.
        constexpr uint32_t kSize = 1;
        GLTexture color;
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(program, kVertexShaderSrc, kFragmentShaderSrc);

        // First, draw with the program.  In the Vulkan backend, this can lead to a wait on the warm
        // up task since the description matches the one needed for the draw.
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);

        // Delete the program to make sure caching the binary can no longer be delayed.
        glUseProgram(0);
        program.reset();

        EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
        data.cacheOpResult = CacheOpResult::ValueNotSet;
    }
}

// Tests error conditions of the APIs.
TEST_P(BlobCacheTest, NegativeAPI)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    EXPECT_TRUE(mHasBlobCache);

    // Test bad arguments
    glBlobCacheCallbacksANGLE(SetBlob, nullptr, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBlobCacheCallbacksANGLE(nullptr, GetBlob, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Regression test for including the fragment output locations in the program key.
// http://anglebug.com/42263144
TEST_P(BlobCacheTest, FragmentOutputLocationKey)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_blend_func_extended") ||
                       getClientMajorVersion() < 3);

    TestUserData data;
    glBlobCacheCallbacksANGLE(SetBlob, GetBlob, &data);
    ASSERT_GL_NO_ERROR();

    // Compile a shader so it puts something in the cache
    if (programBinaryAvailable())
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, 1, 1);

        constexpr char kFragmentShaderSrc[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src;
uniform vec4 src1;
out vec4 FragData;
out vec4 SecondaryFragData;
void main() {
    FragData = src;
    SecondaryFragData = src1;
})";

        constexpr char kVertexShaderSrc[] = R"(#version 300 es
in vec4 position;
void main() {
    gl_Position = position;
})";

        GLuint program = CompileProgram(kVertexShaderSrc, kFragmentShaderSrc, [](GLuint p) {
            glBindFragDataLocationEXT(p, 0, "FragData[0]");
            glBindFragDataLocationIndexedEXT(p, 0, 1, "SecondaryFragData[0]");
        });
        ASSERT_NE(0u, program);
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        WaitProgramBinaryReady(program);
        EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
        data.cacheOpResult = CacheOpResult::ValueNotSet;

        // Re-link the program with different fragment output bindings
        program = CompileProgram(kVertexShaderSrc, kFragmentShaderSrc, [](GLuint p) {
            glBindFragDataLocationEXT(p, 0, "FragData");
            glBindFragDataLocationIndexedEXT(p, 0, 1, "SecondaryFragData");
        });
        ASSERT_NE(0u, program);
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        WaitProgramBinaryReady(program);
        EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
        data.cacheOpResult = CacheOpResult::ValueNotSet;
    }
}

// Checks that the shader cache, which is used when this extension is available, is working
// properly.
TEST_P(BlobCacheTest, ShaderCacheFunctional)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    ANGLE_SKIP_TEST_IF(!IsVulkan());

    TestUserData data;
    glBlobCacheCallbacksANGLE(SetBlob, GetBlob, &data);
    ASSERT_GL_NO_ERROR();

    constexpr char kVertexShaderSrc[] = R"(attribute vec4 aTest;
attribute vec2 aPosition;
varying vec4 vTest;
void main()
{
    vTest        = aTest;
    gl_Position  = vec4(aPosition, 0.0, 1.0);
    gl_PointSize = 1.0;
})";

    constexpr char kFragmentShaderSrc[] = R"(precision mediump float;
varying vec4 vTest;
void main()
{
    gl_FragColor = vTest;
})";

    // Compile a shader so it puts something in the cache
    GLuint shaderID = CompileShader(GL_VERTEX_SHADER, kVertexShaderSrc);
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
    data.cacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile the same shader again, so it would try to retrieve it from the cache
    shaderID = CompileShader(GL_VERTEX_SHADER, kVertexShaderSrc);
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::GetSuccess, data.cacheOpResult);
    data.cacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile another shader, which should create a new entry
    shaderID = CompileShader(GL_FRAGMENT_SHADER, kFragmentShaderSrc);
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
    data.cacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile the first shader again, which should still reside in the cache
    shaderID = CompileShader(GL_VERTEX_SHADER, kVertexShaderSrc);
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::GetSuccess, data.cacheOpResult);
    data.cacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);
}

// Makes sure ANGLE recovers from corrupted cache.
TEST_P(BlobCacheTest, CacheCorruption)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    TestUserData data;
    glBlobCacheCallbacksANGLE(SetCorruptedBlob, GetBlob, &data);
    ASSERT_GL_NO_ERROR();

    ANGLE_SKIP_TEST_IF(!programBinaryAvailable());

    // Compile the program once and draw with it
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);

    const GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, 1, 0, 0, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    WaitProgramBinaryReady(program);
    EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
    data.cacheOpResult = CacheOpResult::ValueNotSet;

    // Compile/link the same program again, so it would try to retrieve it from the cache. GetBlob
    // should return success, but because the cache is corrupted by using SetCorruptedBlob, ANGLE
    // should redo the compile/link when Program::deserialize fails and set the blob again.
    program.makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    ASSERT_TRUE(program.valid());
    glUseProgram(program);

    glUniform4f(colorUniformLocation, 0, 1, 0, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    WaitProgramBinaryReady(program);
    EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
}

class BlobCacheInternalRejectionTest : public BlobCacheTest
{};

// Makes sure ANGLE recovers from internal (backend) rejection of the program blob, while everything
// seems fine to ANGLE.
TEST_P(BlobCacheInternalRejectionTest, Functional)
{
    TestUserData data;
    glBlobCacheCallbacksANGLE(SetBlob, GetBlob, &data);
    ASSERT_GL_NO_ERROR();

    ANGLE_SKIP_TEST_IF(!programBinaryAvailable());

    // Compile the program once and draw with it
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);

    const GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glUniform4f(colorUniformLocation, 1, 0, 0, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    WaitProgramBinaryReady(program);
    EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
    data.cacheOpResult = CacheOpResult::ValueNotSet;

    // Compile/link the same program again, so it would try to retrieve it from the cache. The blob
    // will be corrupted due to the CorruptProgramBinaryForTesting feature. GetBlob should return
    // success, and ANGLE would think the program is fine.  After ANGLE internal updates, the
    // backend should reject the program binary, at which point ANGLE should redo the compile/link
    // and set the blob again.
    program.makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    ASSERT_TRUE(program.valid());
    glUseProgram(program);

    glUniform4f(colorUniformLocation, 0, 1, 0, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    WaitProgramBinaryReady(program);
    EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
}

// Makes sure ANGLE recovers from internal (backend) rejection of the shader blob, while everything
// seems fine to ANGLE.
TEST_P(BlobCacheInternalRejectionTest, ShaderCacheFunctional)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    TestUserData data;
    glBlobCacheCallbacksANGLE(SetBlob, GetBlob, &data);
    ASSERT_GL_NO_ERROR();

    // Compile a shader so it puts something in the cache
    GLuint shaderID = CompileShader(GL_VERTEX_SHADER, essl1_shaders::vs::Simple());
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
    data.cacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile another shader, which should create a new entry
    shaderID = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::UniformColor());
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
    data.cacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile the first shader again, which should still reside in the cache, but is corrupted.
    // The cached entry should be discarded and compilation performed again (which sets another
    // entry in the cache).
    shaderID = CompileShader(GL_VERTEX_SHADER, essl1_shaders::vs::Simple());
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, data.cacheOpResult);
    data.cacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);
}

ANGLE_INSTANTIATE_TEST(BlobCacheTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES3_OPENGLES(),
                       ES2_OPENGLES(),
                       ES2_METAL(),
                       ES3_METAL(),
                       // Note: For the Vulkan backend, disable reads and writes for the global
                       // pipeline cache, so it does not interfere with the test's expectations of
                       // when the cache should and shouldn't be hit.
                       ES2_VULKAN()
                           .enable(Feature::DisablePipelineCacheLoadForTesting)
                           .disable(Feature::SyncMonolithicPipelinesToBlobCache),
                       ES3_VULKAN_SWIFTSHADER()
                           .enable(Feature::DisablePipelineCacheLoadForTesting)
                           .disable(Feature::SyncMonolithicPipelinesToBlobCache),
                       ES3_VULKAN()
                           .enable(Feature::DisablePipelineCacheLoadForTesting)
                           .disable(Feature::SyncMonolithicPipelinesToBlobCache),
                       ES2_VULKAN_SWIFTSHADER()
                           .enable(Feature::DisablePipelineCacheLoadForTesting)
                           .disable(Feature::SyncMonolithicPipelinesToBlobCache),
                       ES2_VULKAN_SWIFTSHADER()
                           .enable(Feature::EnableParallelCompileAndLink)
                           .enable(Feature::DisablePipelineCacheLoadForTesting)
                           .disable(Feature::SyncMonolithicPipelinesToBlobCache),
                       ES3_VULKAN()
                           .enable(Feature::EnableParallelCompileAndLink)
                           .enable(Feature::DisablePipelineCacheLoadForTesting)
                           .disable(Feature::SyncMonolithicPipelinesToBlobCache));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BlobCacheInternalRejectionTest);
ANGLE_INSTANTIATE_TEST(BlobCacheInternalRejectionTest,
                       ES2_OPENGL().enable(Feature::CorruptProgramBinaryForTesting),
                       ES2_OPENGLES().enable(Feature::CorruptProgramBinaryForTesting));

}  // namespace angle
