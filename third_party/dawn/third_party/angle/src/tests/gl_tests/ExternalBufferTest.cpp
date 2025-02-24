//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ExternalBufferTest:
//   Tests the correctness of external buffer ext extension.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

#include "common/android_util.h"

#if defined(ANGLE_PLATFORM_ANDROID) && __ANDROID_API__ >= 26
#    define ANGLE_AHARDWARE_BUFFER_SUPPORT
// NDK header file for access to Android Hardware Buffers
#    include <android/hardware_buffer.h>
#endif

namespace angle
{

class ExternalBufferTestES31 : public ANGLETest<>
{
  protected:
    ExternalBufferTestES31()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    AHardwareBuffer *createAndroidHardwareBuffer(size_t size, const GLubyte *data)
    {
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        // The height and width are number of pixels of size format
        AHardwareBuffer_Desc aHardwareBufferDescription = {};
        aHardwareBufferDescription.width                = size;
        aHardwareBufferDescription.height               = 1;
        aHardwareBufferDescription.layers               = 1;
        aHardwareBufferDescription.format               = AHARDWAREBUFFER_FORMAT_BLOB;
        aHardwareBufferDescription.usage =
            AHARDWAREBUFFER_USAGE_CPU_READ_RARELY | AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
        aHardwareBufferDescription.stride = 0;

        // Allocate memory from Android Hardware Buffer
        AHardwareBuffer *aHardwareBuffer = nullptr;
        EXPECT_EQ(0, AHardwareBuffer_allocate(&aHardwareBufferDescription, &aHardwareBuffer));

        void *mappedMemory = nullptr;
        EXPECT_EQ(0, AHardwareBuffer_lock(aHardwareBuffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY,
                                          -1, nullptr, &mappedMemory));

        // Need to grab the stride the implementation might have enforced
        AHardwareBuffer_describe(aHardwareBuffer, &aHardwareBufferDescription);

        if (data)
        {
            memcpy(mappedMemory, data, size);
        }

        EXPECT_EQ(0, AHardwareBuffer_unlock(aHardwareBuffer, nullptr));
        return aHardwareBuffer;
#else
        return nullptr;
#endif  // ANGLE_PLATFORM_ANDROID
    }

    void destroyAndroidHardwareBuffer(AHardwareBuffer *aHardwareBuffer)
    {
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        AHardwareBuffer_release(aHardwareBuffer);
#endif
    }

    void *lockAndroidHardwareBuffer(AHardwareBuffer *aHardwareBuffer)
    {
        void *data = nullptr;
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        EXPECT_EQ(0, AHardwareBuffer_lock(aHardwareBuffer, AHARDWAREBUFFER_USAGE_CPU_READ_RARELY,
                                          -1, nullptr, &data));
#endif
        return data;
    }

    void unlockAndroidHardwareBuffer(AHardwareBuffer *aHardwareBuffer)
    {
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
        AHardwareBuffer_unlock(aHardwareBuffer, nullptr);
#endif
    }
};

// Testing subdata update with external buffer from AHB
TEST_P(ExternalBufferTestES31, BufferSubData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_external_buffer") ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));
    constexpr uint8_t kBufferSize = 16;
    std::vector<GLubyte> initData(kBufferSize, 0xA);

    // Create the Image
    AHardwareBuffer *aHardwareBuffer;
    constexpr GLbitfield kFlags = GL_DYNAMIC_STORAGE_BIT_EXT;
    aHardwareBuffer             = createAndroidHardwareBuffer(kBufferSize, initData.data());

    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferStorageExternalEXT(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize,
                               eglGetNativeClientBufferANDROID(aHardwareBuffer), kFlags);
    ASSERT_GL_NO_ERROR();

    std::vector<GLubyte> expectedData(kBufferSize, 0xFF);

    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, expectedData.data());
    glFinish();

    ASSERT_GL_NO_ERROR();

    // Inspect the data written into the buffer using CPU access.
    uint8_t *data = static_cast<uint8_t *>(lockAndroidHardwareBuffer(aHardwareBuffer));

    for (uint32_t i = 0; i < kBufferSize; ++i)
    {
        EXPECT_EQ(data[i], 0xFF);
    }

    unlockAndroidHardwareBuffer(aHardwareBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // Delete the source AHB
    destroyAndroidHardwareBuffer(aHardwareBuffer);
}

// Verify that subdata updates to an external buffer backed by an AHB doesn't orphan the AHB
TEST_P(ExternalBufferTestES31, SubDataDoesNotCauseOrphaning)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_external_buffer") ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));
    constexpr uint8_t kBufferSize = 16;
    std::vector<GLubyte> initData(kBufferSize, 0xA);

    // Create the AHB
    AHardwareBuffer *aHardwareBuffer;
    constexpr GLbitfield kFlags = GL_DYNAMIC_STORAGE_BIT_EXT;
    aHardwareBuffer             = createAndroidHardwareBuffer(kBufferSize, initData.data());

    // Create externalBuffer
    GLBuffer externalBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, externalBuffer);
    glBufferStorageExternalEXT(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize,
                               eglGetNativeClientBufferANDROID(aHardwareBuffer), kFlags);
    ASSERT_GL_NO_ERROR();

    // Create a copy read buffer
    std::vector<GLubyte> copyReadBufferData(kBufferSize, 0xB);
    GLBuffer copyReadBuffer;
    glBindBuffer(GL_COPY_READ_BUFFER, copyReadBuffer);
    glBufferData(GL_COPY_READ_BUFFER, kBufferSize, copyReadBufferData.data(), GL_STATIC_READ);
    ASSERT_GL_NO_ERROR();

    // Copy from copyReadBuffer to externalBuffer
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_SHADER_STORAGE_BUFFER, 0, 0, kBufferSize);
    ASSERT_GL_NO_ERROR();

    // Update externalBuffer
    std::vector<GLubyte> expectedData(kBufferSize, 0xFF);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, expectedData.data());
    glFinish();
    ASSERT_GL_NO_ERROR();

    // Inspect the data written into the AHB, through externalBuffer, using CPU access.
    uint8_t *data = static_cast<uint8_t *>(lockAndroidHardwareBuffer(aHardwareBuffer));

    for (uint32_t i = 0; i < kBufferSize; ++i)
    {
        EXPECT_EQ(data[i], 0xFF);
    }

    unlockAndroidHardwareBuffer(aHardwareBuffer);

    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // Delete the source AHB
    destroyAndroidHardwareBuffer(aHardwareBuffer);
}

// Testing dispatch compute shader external from source AHB
TEST_P(ExternalBufferTestES31, DispatchCompute)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_external_buffer") ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));
    constexpr char kCS[] = R"(#version 310 es
    layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    layout(std430, binding=0) buffer Output {
        uint data[];
    } bOutput;
    void main() {
        bOutput.data[gl_GlobalInvocationID.x] =
            gl_GlobalInvocationID.x * 3u;
    }
)";

    constexpr uint8_t kBufferSize = 16 * 4;
    std::vector<GLubyte> initData(kBufferSize, 0xA);

    // Create the Image
    AHardwareBuffer *aHardwareBuffer;
    constexpr GLbitfield kFlags = GL_MAP_READ_BIT;
    aHardwareBuffer             = createAndroidHardwareBuffer(kBufferSize, initData.data());

    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferStorageExternalEXT(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize,
                               eglGetNativeClientBufferANDROID(aHardwareBuffer), kFlags);

    ASSERT_GL_NO_ERROR();

    GLProgram program;
    program.makeCompute(kCS);
    ASSERT_NE(program, 0U);
    ASSERT_GL_NO_ERROR();

    glUseProgram(program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    glDispatchCompute(kBufferSize, 1, 1);
    glFinish();
    ASSERT_GL_NO_ERROR();

    // Inspect the data written into the buffer using CPU access.
    uint32_t *data = static_cast<uint32_t *>(lockAndroidHardwareBuffer(aHardwareBuffer));

    for (uint32_t i = 0; i < (kBufferSize / sizeof(uint32_t)); ++i)
    {
        EXPECT_EQ(data[i], static_cast<uint32_t>(i * 3));
    }

    unlockAndroidHardwareBuffer(aHardwareBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // Delete the source AHB
    destroyAndroidHardwareBuffer(aHardwareBuffer);
}

// Test interaction between GL_OES_mapbuffer and GL_EXT_external_buffer extensions.
TEST_P(ExternalBufferTestES31, MapBuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_external_buffer") ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage") ||
                       !IsGLExtensionEnabled("GL_EXT_map_buffer_range"));
    constexpr uint8_t kBufferSize = 16;
    std::vector<GLubyte> initData(kBufferSize, 0xFF);

    // Create the AHB
    AHardwareBuffer *aHardwareBuffer;
    constexpr GLbitfield kFlags = (GL_MAP_READ_BIT_EXT | GL_MAP_WRITE_BIT_EXT);
    aHardwareBuffer             = createAndroidHardwareBuffer(kBufferSize, initData.data());

    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferStorageExternalEXT(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize,
                               eglGetNativeClientBufferANDROID(aHardwareBuffer), kFlags);

    ASSERT_GL_NO_ERROR();

    // Inspect the data written into the buffer using CPU access.
    uint8_t *data = static_cast<uint8_t *>(
        glMapBufferRangeEXT(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT_EXT));
    ASSERT_GL_NO_ERROR();

    for (uint32_t i = 0; i < kBufferSize; ++i)
    {
        EXPECT_EQ(data[i], 0xFF);
    }

    glUnmapBufferOES(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // Delete the source AHB
    destroyAndroidHardwareBuffer(aHardwareBuffer);
}

// Verify that mapping an external buffer backed by an AHB doesn't orphan the AHB
TEST_P(ExternalBufferTestES31, MapBufferDoesNotCauseOrphaning)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_external_buffer") ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage") ||
                       !IsGLExtensionEnabled("GL_EXT_map_buffer_range"));
    constexpr uint8_t kBufferSize = 16;
    std::vector<GLubyte> initData(kBufferSize, 0xA);

    // Create the AHB
    AHardwareBuffer *aHardwareBuffer;
    constexpr GLbitfield kFlags =
        (GL_MAP_READ_BIT_EXT | GL_MAP_WRITE_BIT_EXT | GL_DYNAMIC_STORAGE_BIT_EXT);
    aHardwareBuffer = createAndroidHardwareBuffer(kBufferSize, initData.data());

    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferStorageExternalEXT(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize,
                               eglGetNativeClientBufferANDROID(aHardwareBuffer), kFlags);
    ASSERT_GL_NO_ERROR();

    // Create a copy read buffer
    std::vector<GLubyte> copyReadBufferData(kBufferSize, 0xB);
    GLBuffer copyReadBuffer;
    glBindBuffer(GL_COPY_READ_BUFFER, copyReadBuffer);
    glBufferData(GL_COPY_READ_BUFFER, kBufferSize, copyReadBufferData.data(), GL_STATIC_READ);
    ASSERT_GL_NO_ERROR();

    // Copy from copyReadBuffer to externalBuffer
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_SHADER_STORAGE_BUFFER, 0, 0, kBufferSize);
    ASSERT_GL_NO_ERROR();

    // Inspect the data written into the buffer using map buffer API.
    constexpr GLbitfield kMapFlags = (GL_MAP_WRITE_BIT_EXT | GL_MAP_INVALIDATE_BUFFER_BIT);
    uint8_t *mapData               = static_cast<uint8_t *>(
        glMapBufferRangeEXT(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, kMapFlags));
    ASSERT_GL_NO_ERROR();
    EXPECT_NE(mapData, nullptr);
    glUnmapBufferOES(GL_SHADER_STORAGE_BUFFER);

    // Update externalBuffer
    std::vector<GLubyte> expectedData(kBufferSize, 0xFF);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, expectedData.data());
    glFinish();
    ASSERT_GL_NO_ERROR();

    // Inspect the data written into the AHB, through externalBuffer, using CPU access.
    uint8_t *data = static_cast<uint8_t *>(lockAndroidHardwareBuffer(aHardwareBuffer));

    for (uint32_t i = 0; i < kBufferSize; ++i)
    {
        EXPECT_EQ(data[i], 0xFF);
    }

    unlockAndroidHardwareBuffer(aHardwareBuffer);

    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // Delete the source AHB
    destroyAndroidHardwareBuffer(aHardwareBuffer);
}

// Verify that create and destroy external buffer backed by an AHB doesn't leak AHB
TEST_P(ExternalBufferTestES31, BufferDoesNotLeakAHB)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_external_buffer") ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    // Create and destroy 128M AHB backed buffer in a loop. If we leak AHB, it will fail due to AHB
    // allocation failure before loop ends.
    constexpr size_t kBufferSize = 128 * 1024 * 1024;
    for (int loop = 0; loop < 1000; loop++)
    {
        // Create the AHB
        AHardwareBuffer *aHardwareBuffer;
        constexpr GLbitfield kFlags = GL_DYNAMIC_STORAGE_BIT_EXT;
        aHardwareBuffer             = createAndroidHardwareBuffer(kBufferSize, nullptr);
        GLBuffer buffer;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        glBufferStorageExternalEXT(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize,
                                   eglGetNativeClientBufferANDROID(aHardwareBuffer), kFlags);
        ASSERT_GL_NO_ERROR();
        // Delete the source AHB
        destroyAndroidHardwareBuffer(aHardwareBuffer);
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ExternalBufferTestES31);
ANGLE_INSTANTIATE_TEST_ES31(ExternalBufferTestES31);
}  // namespace angle
