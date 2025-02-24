//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLBlobCacheTest:
//   Unit tests for the EGL_ANDROID_blob_cache extension.

// Must be included first to prevent errors with "None".
#include "test_utils/ANGLETest.h"

#include <map>
#include <vector>

#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/MultiThreadSteps.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

using namespace angle;

constexpr char kEGLExtName[] = "EGL_ANDROID_blob_cache";

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

namespace
{
std::map<std::vector<uint8_t>, std::vector<uint8_t>> gApplicationCache;
CacheOpResult gLastCacheOpResult = CacheOpResult::ValueNotSet;

void SetBlob(const void *key, EGLsizeiANDROID keySize, const void *value, EGLsizeiANDROID valueSize)
{
    std::vector<uint8_t> keyVec(keySize);
    memcpy(keyVec.data(), key, keySize);

    std::vector<uint8_t> valueVec(valueSize);
    memcpy(valueVec.data(), value, valueSize);

    gApplicationCache[keyVec] = valueVec;

    gLastCacheOpResult = CacheOpResult::SetSuccess;
}

void SetCorruptedBlob(const void *key,
                      EGLsizeiANDROID keySize,
                      const void *value,
                      EGLsizeiANDROID valueSize)
{
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

    gApplicationCache[keyVec] = valueVec;

    gLastCacheOpResult = CacheOpResult::SetSuccess;
}

EGLsizeiANDROID GetBlob(const void *key,
                        EGLsizeiANDROID keySize,
                        void *value,
                        EGLsizeiANDROID valueSize)
{
    std::vector<uint8_t> keyVec(keySize);
    memcpy(keyVec.data(), key, keySize);

    auto entry = gApplicationCache.find(keyVec);
    if (entry == gApplicationCache.end())
    {
        // A compile+link operation can generate multiple queries to the cache; one per shader and
        // one for link.  For the purposes of the test, make sure that any of these hitting the
        // cache is considered a success, particularly because it's valid for the pipeline cache
        // entry not to exist in the cache.
        if (gLastCacheOpResult != CacheOpResult::GetSuccess)
        {
            gLastCacheOpResult = CacheOpResult::GetNotFound;
        }
        return 0;
    }

    if (entry->second.size() <= static_cast<size_t>(valueSize))
    {
        memcpy(value, entry->second.data(), entry->second.size());
        gLastCacheOpResult = CacheOpResult::GetSuccess;
    }
    else
    {
        gLastCacheOpResult = CacheOpResult::GetMemoryTooSmall;
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
}  // anonymous namespace

class EGLBlobCacheTest : public ANGLETest<>
{
  protected:
    EGLBlobCacheTest() : mHasBlobCache(false)
    {
        // Force disply caching off. Blob cache functions require it.
        forceNewDisplay();
    }

    void testSetUp() override
    {
        EGLDisplay display = getEGLWindow()->getDisplay();
        mHasBlobCache      = IsEGLDisplayExtensionEnabled(display, kEGLExtName);
    }

    void testTearDown() override { gApplicationCache.clear(); }

    bool programBinaryAvailable() { return IsGLExtensionEnabled("GL_OES_get_program_binary"); }

    bool mHasBlobCache;
};

// Makes sure the extension exists and works
TEST_P(EGLBlobCacheTest, Functional)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    EGLDisplay display = getEGLWindow()->getDisplay();

    EXPECT_TRUE(mHasBlobCache);
    eglSetBlobCacheFuncsANDROID(display, SetBlob, GetBlob);
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
        EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
        gLastCacheOpResult = CacheOpResult::ValueNotSet;

        // Compile the same shader again, so it would try to retrieve it from the cache
        program.makeRaster(kVertexShaderSrc, kFragmentShaderSrc);
        ASSERT_TRUE(program.valid());
        EXPECT_EQ(CacheOpResult::GetSuccess, gLastCacheOpResult);
        gLastCacheOpResult = CacheOpResult::ValueNotSet;

        // Compile another shader, which should create a new entry
        program.makeRaster(kVertexShaderSrc2, kFragmentShaderSrc2);
        ASSERT_TRUE(program.valid());
        WaitProgramBinaryReady(program);
        EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
        gLastCacheOpResult = CacheOpResult::ValueNotSet;

        // Compile the first shader again, which should still reside in the cache
        program.makeRaster(kVertexShaderSrc, kFragmentShaderSrc);
        ASSERT_TRUE(program.valid());
        EXPECT_EQ(CacheOpResult::GetSuccess, gLastCacheOpResult);
        gLastCacheOpResult = CacheOpResult::ValueNotSet;

        // Make sure deleting the program doesn't result in a binary save.  Regression test for a
        // bug where the binary was re-cached after being loaded.
        glUseProgram(0);
        program.reset();

        EXPECT_EQ(CacheOpResult::ValueNotSet, gLastCacheOpResult);
    }
}

// Makes sure the caching is always done without an explicit wait for post-link events (if any)
TEST_P(EGLBlobCacheTest, FunctionalWithoutWait)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    EGLDisplay display = getEGLWindow()->getDisplay();

    EXPECT_TRUE(mHasBlobCache);
    eglSetBlobCacheFuncsANDROID(display, SetBlob, GetBlob);
    ASSERT_EGL_SUCCESS();

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

        EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
        gLastCacheOpResult = CacheOpResult::ValueNotSet;
    }
}

// Tests error conditions of the APIs.
TEST_P(EGLBlobCacheTest, NegativeAPI)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    EXPECT_TRUE(mHasBlobCache);

    // Test bad display
    eglSetBlobCacheFuncsANDROID(EGL_NO_DISPLAY, nullptr, nullptr);
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);

    eglSetBlobCacheFuncsANDROID(EGL_NO_DISPLAY, SetBlob, GetBlob);
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);

    EGLDisplay display = getEGLWindow()->getDisplay();

    // Test bad arguments
    eglSetBlobCacheFuncsANDROID(display, nullptr, nullptr);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    eglSetBlobCacheFuncsANDROID(display, SetBlob, nullptr);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    eglSetBlobCacheFuncsANDROID(display, nullptr, GetBlob);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // Set the arguments once and test setting them again (which should fail)
    eglSetBlobCacheFuncsANDROID(display, SetBlob, GetBlob);
    ASSERT_EGL_SUCCESS();

    eglSetBlobCacheFuncsANDROID(display, SetBlob, GetBlob);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // Try again with bad parameters
    eglSetBlobCacheFuncsANDROID(EGL_NO_DISPLAY, nullptr, nullptr);
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);

    eglSetBlobCacheFuncsANDROID(display, nullptr, nullptr);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    eglSetBlobCacheFuncsANDROID(display, SetBlob, nullptr);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    eglSetBlobCacheFuncsANDROID(display, nullptr, GetBlob);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
}

// Regression test for including the fragment output locatins in the program key.
// http://anglebug.com/42263144
TEST_P(EGLBlobCacheTest, FragmentOutputLocationKey)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_blend_func_extended") ||
                       getClientMajorVersion() < 3);

    EGLDisplay display = getEGLWindow()->getDisplay();

    EXPECT_TRUE(mHasBlobCache);
    eglSetBlobCacheFuncsANDROID(display, SetBlob, GetBlob);
    ASSERT_EGL_SUCCESS();

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
        EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
        gLastCacheOpResult = CacheOpResult::ValueNotSet;

        // Re-link the program with different fragment output bindings
        program = CompileProgram(kVertexShaderSrc, kFragmentShaderSrc, [](GLuint p) {
            glBindFragDataLocationEXT(p, 0, "FragData");
            glBindFragDataLocationIndexedEXT(p, 0, 1, "SecondaryFragData");
        });
        ASSERT_NE(0u, program);
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        WaitProgramBinaryReady(program);
        EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
        gLastCacheOpResult = CacheOpResult::ValueNotSet;
    }
}

// Checks that the shader cache, which is used when this extension is available, is working
// properly.
TEST_P(EGLBlobCacheTest, ShaderCacheFunctional)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EGLDisplay display = getEGLWindow()->getDisplay();

    EXPECT_TRUE(mHasBlobCache);
    eglSetBlobCacheFuncsANDROID(display, SetBlob, GetBlob);
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

    // Compile a shader so it puts something in the cache
    GLuint shaderID = CompileShader(GL_VERTEX_SHADER, kVertexShaderSrc);
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
    gLastCacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile the same shader again, so it would try to retrieve it from the cache
    shaderID = CompileShader(GL_VERTEX_SHADER, kVertexShaderSrc);
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::GetSuccess, gLastCacheOpResult);
    gLastCacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile another shader, which should create a new entry
    shaderID = CompileShader(GL_FRAGMENT_SHADER, kFragmentShaderSrc);
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
    gLastCacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile the first shader again, which should still reside in the cache
    shaderID = CompileShader(GL_VERTEX_SHADER, kVertexShaderSrc);
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::GetSuccess, gLastCacheOpResult);
    gLastCacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);
}

// Tests compiling a program in multiple threads, then fetching the compiled program/shaders from
// the cache. We then perform a draw call and test the result to ensure nothing was corrupted.
TEST_P(EGLBlobCacheTest, ThreadSafety)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EGLDisplay display = getEGLWindow()->getDisplay();

    EXPECT_TRUE(mHasBlobCache);
    eglSetBlobCacheFuncsANDROID(display, SetBlob, GetBlob);
    ASSERT_EGL_SUCCESS();

    auto threadFunc = [&](int threadID, EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        ANGLE_GL_PROGRAM(unusedProgramTemp1, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

        // Insert a new entry into the cache unique to this thread.
        std::stringstream ss;
        ss << essl1_shaders::vs::Simple() << "//" << threadID;
        std::string newEntryVSSource = ss.str().c_str();
        ANGLE_GL_PROGRAM(unusedProgramTemp2, newEntryVSSource.c_str(), essl1_shaders::fs::Red());

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    constexpr int kNumThreads = 32;

    std::vector<LockStepThreadFunc> threadFuncs(kNumThreads);
    for (int i = 0; i < kNumThreads; ++i)
    {
        threadFuncs[i] = [=](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
            return threadFunc(i, dpy, surface, context);
        };
    }

    gLastCacheOpResult = CacheOpResult::ValueNotSet;

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    EXPECT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    EXPECT_EQ(CacheOpResult::GetSuccess, gLastCacheOpResult);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Makes sure ANGLE recovers from corrupted cache.
TEST_P(EGLBlobCacheTest, CacheCorruption)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::CacheCompiledShader));
    ANGLE_SKIP_TEST_IF(getEGLWindow()->isFeatureEnabled(Feature::DisableProgramCaching));

    EGLDisplay display = getEGLWindow()->getDisplay();

    EXPECT_TRUE(mHasBlobCache);
    eglSetBlobCacheFuncsANDROID(display, SetCorruptedBlob, GetBlob);
    ASSERT_EGL_SUCCESS();

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
    EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
    gLastCacheOpResult = CacheOpResult::ValueNotSet;

    // Compile/link the same program again, so it would try to retrieve it from the cache.  GetBlob
    // should return success, but because the cache is corrupt, ANGLE should redo the compile/link
    // and set the blob again.
    program.makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    ASSERT_TRUE(program.valid());
    glUseProgram(program);

    glUniform4f(colorUniformLocation, 0, 1, 0, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    WaitProgramBinaryReady(program);
    EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
}

class EGLBlobCacheInternalRejectionTest : public EGLBlobCacheTest
{};

// Makes sure ANGLE recovers from internal (backend) rejection of the program blob, while everything
// seems fine to ANGLE.
TEST_P(EGLBlobCacheInternalRejectionTest, Functional)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    EXPECT_TRUE(mHasBlobCache);
    eglSetBlobCacheFuncsANDROID(display, SetBlob, GetBlob);
    ASSERT_EGL_SUCCESS();

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
    EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
    gLastCacheOpResult = CacheOpResult::ValueNotSet;

    // Compile/link the same program again, so it would try to retrieve it from the cache.  GetBlob
    // should return success, and ANGLE would think the program is fine.  After ANGLE internal
    // updates, the backend should reject the program binary, at which point ANGLE should redo the
    // compile/link and set the blob again.
    program.makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    ASSERT_TRUE(program.valid());
    glUseProgram(program);

    glUniform4f(colorUniformLocation, 0, 1, 0, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    WaitProgramBinaryReady(program);
    EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
}

// Makes sure ANGLE recovers from internal (backend) rejection of the shader blob, while everything
// seems fine to ANGLE.
TEST_P(EGLBlobCacheInternalRejectionTest, ShaderCacheFunctional)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EGLDisplay display = getEGLWindow()->getDisplay();

    EXPECT_TRUE(mHasBlobCache);
    eglSetBlobCacheFuncsANDROID(display, SetBlob, GetBlob);
    ASSERT_EGL_SUCCESS();

    // Compile a shader so it puts something in the cache
    GLuint shaderID = CompileShader(GL_VERTEX_SHADER, essl1_shaders::vs::Simple());
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
    gLastCacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile another shader, which should create a new entry
    shaderID = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::UniformColor());
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
    gLastCacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);

    // Compile the first shader again, which should still reside in the cache, but is corrupted.
    // The cached entry should be discarded and compilation performed again (which sets another
    // entry in the cache).
    shaderID = CompileShader(GL_VERTEX_SHADER, essl1_shaders::vs::Simple());
    ASSERT_TRUE(shaderID != 0);
    EXPECT_EQ(CacheOpResult::SetSuccess, gLastCacheOpResult);
    gLastCacheOpResult = CacheOpResult::ValueNotSet;
    glDeleteShader(shaderID);
}

ANGLE_INSTANTIATE_TEST(EGLBlobCacheTest,
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

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLBlobCacheInternalRejectionTest);
ANGLE_INSTANTIATE_TEST(EGLBlobCacheInternalRejectionTest,
                       ES2_OPENGL().enable(Feature::CorruptProgramBinaryForTesting),
                       ES2_OPENGLES().enable(Feature::CorruptProgramBinaryForTesting));
