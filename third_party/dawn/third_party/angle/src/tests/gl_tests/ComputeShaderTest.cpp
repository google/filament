//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ComputeShaderTest:
//   Compute shader specific tests.

#include <vector>
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class ComputeShaderTest : public ANGLETest<>
{
  protected:
    ComputeShaderTest() {}

    void createMockOutputImage(GLuint texture, GLenum internalFormat, GLint width, GLint height)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
        EXPECT_GL_NO_ERROR();

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, internalFormat);
        EXPECT_GL_NO_ERROR();
    }

    template <class T, GLint kWidth, GLint kHeight>
    void runSharedMemoryTest(const char *kCS,
                             GLenum internalFormat,
                             GLenum format,
                             const std::array<T, kWidth * kHeight> &inputData,
                             const std::array<T, kWidth * kHeight> &expectedValues)
    {
        GLTexture texture[2];
        GLFramebuffer framebuffer;

        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, kWidth, kHeight);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, format,
                        inputData.data());
        EXPECT_GL_NO_ERROR();

        constexpr T initData[kWidth * kHeight] = {};
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, kWidth, kHeight);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, format, initData);
        EXPECT_GL_NO_ERROR();

        ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
        glUseProgram(program);

        glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_ONLY, internalFormat);
        EXPECT_GL_NO_ERROR();

        glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, internalFormat);
        EXPECT_GL_NO_ERROR();

        glDispatchCompute(1, 1, 1);
        EXPECT_GL_NO_ERROR();

        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

        T outputValues[kWidth * kHeight] = {};
        glUseProgram(0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[1],
                               0);
        EXPECT_GL_NO_ERROR();
        glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, format, outputValues);
        EXPECT_GL_NO_ERROR();

        for (int i = 0; i < kWidth * kHeight; i++)
        {
            EXPECT_EQ(expectedValues[i], outputValues[i]);
        }
    }
};

class ComputeShaderTestES3 : public ANGLETest<>
{
  protected:
    ComputeShaderTestES3() {}
};

class WebGL2ComputeTest : public ComputeShaderTest
{
  protected:
    WebGL2ComputeTest() { setWebGLCompatibilityEnabled(true); }
};

// link a simple compute program. It should be successful.
TEST_P(ComputeShaderTest, LinkComputeProgram)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    EXPECT_GL_NO_ERROR();
}

// Link a simple compute program. Then detach the shader and dispatch compute.
// It should be successful.
TEST_P(ComputeShaderTest, DetachShaderAfterLinkSuccess)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";

    GLuint program = glCreateProgram();

    GLuint cs = CompileShader(GL_COMPUTE_SHADER, kCS);
    EXPECT_NE(0u, cs);

    glAttachShader(program, cs);
    glDeleteShader(cs);

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    glDetachShader(program, cs);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();
}

// link a simple compute program. There is no local size and linking should fail.
TEST_P(ComputeShaderTest, LinkComputeProgramNoLocalSizeLinkError)
{
    constexpr char kCS[] = R"(#version 310 es
void main()
{
})";

    GLuint program = CompileComputeProgram(kCS, false);
    EXPECT_EQ(0u, program);

    glDeleteProgram(program);

    EXPECT_GL_NO_ERROR();
}

// link a simple compute program.
// make sure that uniforms and uniform samplers get recorded
TEST_P(ComputeShaderTest, LinkComputeProgramWithUniforms)
{
    constexpr char kCS[] = R"(#version 310 es
precision mediump sampler2D;
layout(local_size_x=1) in;
uniform int myUniformInt;
uniform sampler2D myUniformSampler;
layout(rgba32i) uniform highp writeonly iimage2D imageOut;
void main()
{
    int q = myUniformInt;
    vec4 v = textureLod(myUniformSampler, vec2(0.0), 0.0);
    imageStore(imageOut, ivec2(0), ivec4(v) * q);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    GLint uniformLoc = glGetUniformLocation(program, "myUniformInt");
    EXPECT_NE(-1, uniformLoc);

    uniformLoc = glGetUniformLocation(program, "myUniformSampler");
    EXPECT_NE(-1, uniformLoc);

    EXPECT_GL_NO_ERROR();
}

// Attach both compute and non-compute shaders. A link time error should occur.
// OpenGL ES 3.10, 7.3 Program Objects
TEST_P(ComputeShaderTest, AttachMultipleShaders)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";

    constexpr char kVS[] = R"(#version 310 es
void main()
{
})";

    constexpr char kFS[] = R"(#version 310 es
void main()
{
})";

    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    GLuint cs = CompileShader(GL_COMPUTE_SHADER, kCS);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);
    EXPECT_NE(0u, cs);

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glAttachShader(program, cs);
    glDeleteShader(cs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    EXPECT_GL_FALSE(linkStatus);

    EXPECT_GL_NO_ERROR();
}

// Attach a vertex, fragment and compute shader.
// Query for the number of attached shaders and check the count.
TEST_P(ComputeShaderTest, AttachmentCount)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";

    constexpr char kVS[] = R"(#version 310 es
void main()
{
})";

    constexpr char kFS[] = R"(#version 310 es
void main()
{
})";

    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    GLuint cs = CompileShader(GL_COMPUTE_SHADER, kCS);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);
    EXPECT_NE(0u, cs);

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glAttachShader(program, cs);
    glDeleteShader(cs);

    GLint numAttachedShaders;
    glGetProgramiv(program, GL_ATTACHED_SHADERS, &numAttachedShaders);

    EXPECT_EQ(3, numAttachedShaders);

    glDeleteProgram(program);

    EXPECT_GL_NO_ERROR();
}

// Attach a compute shader and link, but start rendering.
TEST_P(ComputeShaderTest, StartRenderingWithComputeProgram)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDrawArrays(GL_POINTS, 0, 2);
    EXPECT_GL_NO_ERROR();
}

// Attach a vertex and fragment shader and link, but dispatch compute.
TEST_P(ComputeShaderTest, DispatchComputeWithRenderingProgram)
{
    constexpr char kVS[] = R"(#version 310 es
void main() {})";

    constexpr char kFS[] = R"(#version 310 es
void main() {})";

    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Access all compute shader special variables.
TEST_P(ComputeShaderTest, AccessAllSpecialVariables)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=4, local_size_y=3, local_size_z=2) in;
layout(rgba32ui) uniform highp writeonly uimage2D imageOut;
void main()
{
    uvec3 temp1 = gl_NumWorkGroups;
    uvec3 temp2 = gl_WorkGroupSize;
    uvec3 temp3 = gl_WorkGroupID;
    uvec3 temp4 = gl_LocalInvocationID;
    uvec3 temp5 = gl_GlobalInvocationID;
    uint  temp6 = gl_LocalInvocationIndex;
    imageStore(imageOut, ivec2(gl_LocalInvocationIndex, 0), uvec4(temp1 + temp2 + temp3 + temp4 + temp5, temp6));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
}

// Access part compute shader special variables.
TEST_P(ComputeShaderTest, AccessPartSpecialVariables)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=4, local_size_y=3, local_size_z=2) in;
layout(rgba32ui) uniform highp writeonly uimage2D imageOut;
void main()
{
    uvec3 temp1 = gl_WorkGroupSize;
    uvec3 temp2 = gl_WorkGroupID;
    uint  temp3 = gl_LocalInvocationIndex;
    imageStore(imageOut, ivec2(gl_LocalInvocationIndex, 0), uvec4(temp1 + temp2, temp3));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
}

// Use glDispatchCompute to define work group count.
TEST_P(ComputeShaderTest, DispatchCompute)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=4, local_size_y=3, local_size_z=2) in;
layout(rgba32ui) uniform highp writeonly uimage2D imageOut;
void main()
{
    uvec3 temp = gl_NumWorkGroups;
    imageStore(imageOut, ivec2(gl_GlobalInvocationID.xy), uvec4(temp, 0u));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    GLTexture texture;
    createMockOutputImage(texture, GL_RGBA32UI, 4, 3);

    glUseProgram(program);
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();
}

// Binds a storage buffer to slot 0, then binds a storage image to slot 0, then buffer again.
TEST_P(ComputeShaderTest, BufferImageBuffer)
{
    constexpr char kCS0[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(binding = 0, offset = 4) uniform atomic_uint ac[2];
void main()
{
    atomicCounterIncrement(ac[0]);
    atomicCounterDecrement(ac[1]);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program0, kCS0);
    glUseProgram(program0);

    unsigned int bufferData[3] = {11u, 4u, 4u};
    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    void *mappedBuffer =
        glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 3, GL_MAP_READ_BIT);
    memcpy(bufferData, mappedBuffer, sizeof(bufferData));
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    EXPECT_EQ(11u, bufferData[0]);
    EXPECT_EQ(5u, bufferData[1]);
    EXPECT_EQ(3u, bufferData[2]);

    constexpr char kCS1[] = R"(#version 310 es
layout(local_size_x=4, local_size_y=3, local_size_z=2) in;
layout(rgba32ui) uniform highp writeonly uimage2D imageOut;
void main()
{
    uvec3 temp = gl_NumWorkGroups;
    imageStore(imageOut, ivec2(gl_GlobalInvocationID.xy), uvec4(temp, 0u));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program1, kCS1);

    GLTexture texture;
    createMockOutputImage(texture, GL_RGBA32UI, 4, 3);

    glUseProgram(program1);
    glDispatchCompute(8, 4, 2);

    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
    glUseProgram(program0);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    mappedBuffer =
        glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 3, GL_MAP_READ_BIT);
    memcpy(bufferData, mappedBuffer, sizeof(bufferData));
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    EXPECT_EQ(11u, bufferData[0]);
    EXPECT_EQ(6u, bufferData[1]);
    EXPECT_EQ(2u, bufferData[2]);

    EXPECT_GL_NO_ERROR();
}

// Test that the buffer written to by imageStore() in the CS does not race with writing to the
// buffer when it's mapped.
TEST_P(ComputeShaderTest, BufferImageBufferMapWrite)
{
    constexpr char kCS0[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(binding = 0, offset = 4) uniform atomic_uint ac[2];
void main()
{
    atomicCounterIncrement(ac[0]);
    atomicCounterDecrement(ac[1]);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program0, kCS0);
    glUseProgram(program0);

    unsigned int expectedBufferData[3] = {11u, 4u, 4u};
    unsigned int bufferData[3]         = {0};
    memcpy(bufferData, expectedBufferData, sizeof(bufferData));

    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    void *mappedBuffer =
        glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 3, GL_MAP_READ_BIT);
    memcpy(bufferData, mappedBuffer, sizeof(bufferData));
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    EXPECT_EQ(11u, bufferData[0]);
    EXPECT_EQ(5u, bufferData[1]);
    EXPECT_EQ(3u, bufferData[2]);

    constexpr char kCS1[] = R"(#version 310 es
layout(local_size_x=4, local_size_y=3, local_size_z=2) in;
layout(rgba32ui) uniform highp writeonly uimage2D imageOut;
void main()
{
    uvec3 temp = gl_NumWorkGroups;
    imageStore(imageOut, ivec2(gl_GlobalInvocationID.xy), uvec4(temp, 0u));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program1, kCS1);

    GLTexture texture;
    createMockOutputImage(texture, GL_RGBA32UI, 4, 3);

    glUseProgram(program1);
    glDispatchCompute(8, 4, 2);

    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
    glUseProgram(program0);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    mappedBuffer =
        glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 3, GL_MAP_WRITE_BIT);

    memcpy(mappedBuffer, expectedBufferData, sizeof(expectedBufferData));
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    // Force the CS imageStore() writes to the buffer to complete.
    glFinish();
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    mappedBuffer =
        glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 3, GL_MAP_READ_BIT);
    memcpy(bufferData, mappedBuffer, sizeof(bufferData));
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    EXPECT_EQ(expectedBufferData[0], bufferData[0]);
    EXPECT_EQ(expectedBufferData[1], bufferData[1]);
    EXPECT_EQ(expectedBufferData[2], bufferData[2]);

    EXPECT_GL_NO_ERROR();
}

// Test that binds UAV with type image to slot 0, then binds UAV with type buffer to slot 0.
TEST_P(ComputeShaderTest, ImageAtomicCounterBuffer)
{
    // Flaky hang. http://anglebug.com/40644695
    ANGLE_SKIP_TEST_IF(IsWindows() && IsNVIDIA() && IsDesktopOpenGL());

    constexpr char kCS0[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) writeonly uniform highp uimage2D uImage[2];
void main()
{
    imageStore(uImage[0], ivec2(gl_LocalInvocationIndex, gl_WorkGroupID.x), uvec4(100, 0,
0, 0));
    imageStore(uImage[1], ivec2(gl_LocalInvocationIndex, gl_WorkGroupID.x), uvec4(100, 0,
0, 0));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program0, kCS0);
    glUseProgram(program0);
    int width = 1, height = 1;
    GLuint inputValues[] = {200};
    GLTexture mTexture[2];
    glBindTexture(GL_TEXTURE_2D, mTexture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    inputValues);

    glBindTexture(GL_TEXTURE_2D, mTexture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    inputValues);

    glBindImageTexture(0, mTexture[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    glBindImageTexture(1, mTexture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    constexpr char kCS1[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(binding = 0, offset = 4) uniform atomic_uint ac[2];
void main()
{
    atomicCounterIncrement(ac[0]);
    atomicCounterDecrement(ac[1]);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program1, kCS1);

    unsigned int bufferData[3] = {11u, 4u, 4u};
    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);

    glUseProgram(program1);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    void *mappedBuffer =
        glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 3, GL_MAP_READ_BIT);
    memcpy(bufferData, mappedBuffer, sizeof(bufferData));
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    EXPECT_EQ(11u, bufferData[0]);
    EXPECT_EQ(5u, bufferData[1]);
    EXPECT_EQ(3u, bufferData[2]);

    EXPECT_GL_NO_ERROR();
}

// Test that binds UAV with type image to slot 0, then binds UAV with type buffer to slot 0.
TEST_P(ComputeShaderTest, ImageShaderStorageBuffer)
{
    constexpr char kCS0[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) writeonly uniform highp uimage2D uImage[2];
void main()
{
    imageStore(uImage[0], ivec2(gl_LocalInvocationIndex, gl_WorkGroupID.x), uvec4(100, 0,
0, 0));
    imageStore(uImage[1], ivec2(gl_LocalInvocationIndex, gl_WorkGroupID.x), uvec4(100, 0,
0, 0));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program0, kCS0);
    glUseProgram(program0);
    int width = 1, height = 1;
    GLuint inputValues[] = {200};
    GLTexture mTexture[2];
    glBindTexture(GL_TEXTURE_2D, mTexture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    inputValues);

    glBindTexture(GL_TEXTURE_2D, mTexture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    inputValues);

    glBindImageTexture(0, mTexture[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    glBindImageTexture(1, mTexture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    constexpr char kCS1[] =
        R"(#version 310 es
 layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
 layout(std140, binding = 0) buffer blockOut {
     uvec2 data;
 } instanceOut;
 layout(std140, binding = 1) buffer blockIn {
     uvec2 data;
 } instanceIn;
 void main()
 {
     instanceOut.data = instanceIn.data;
 }
 )";

    ANGLE_GL_COMPUTE_PROGRAM(program1, kCS1);

    constexpr unsigned int kBufferSize              = 2;
    constexpr unsigned int kBufferData[kBufferSize] = {10, 20};

    GLBuffer blockIn;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockIn);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferData), kBufferData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockIn);

    GLBuffer blockOut;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockOut);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferData), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, blockOut);

    glUseProgram(program1);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockOut);
    unsigned int bufferDataOut[kBufferSize] = {};
    const GLColor *ptr                      = reinterpret_cast<GLColor *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kBufferData), GL_MAP_READ_BIT));
    memcpy(bufferDataOut, ptr, sizeof(kBufferData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    for (unsigned int index = 0; index < kBufferSize; ++index)
    {
        EXPECT_EQ(bufferDataOut[index], kBufferData[index]) << " index " << index;
    }
}

// Basic test for DispatchComputeIndirect.
TEST_P(ComputeShaderTest, DispatchComputeIndirect)
{
    // Flaky crash on teardown, see http://anglebug.com/40096579
    ANGLE_SKIP_TEST_IF(IsD3D11() && IsIntel() && IsWindows());

    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) uniform highp uimage2D uImage;
void main()
{
    imageStore(uImage, ivec2(gl_WorkGroupID.x, gl_WorkGroupID.y), uvec4(100, 0, 0, 0));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    glUseProgram(program);
    const int kWidth = 4, kHeight = 6;
    GLuint inputValues[kWidth][kHeight] = {};

    GLBuffer buffer;
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer);
    GLuint params[] = {kWidth, kHeight, 1};
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(params), params, GL_STATIC_DRAW);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    inputValues);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchComputeIndirect(0);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    GLuint outputValues[kWidth][kHeight];

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();

    constexpr GLuint kExpectedValue = 100u;
    for (int x = 0; x < kWidth; x++)
    {
        for (int y = 0; y < kHeight; y++)
        {
            EXPECT_EQ(kExpectedValue, outputValues[x][y]);
        }
    }
}

// Test that uploading data to buffer that's in use then using it as indirect buffer works.
TEST_P(ComputeShaderTest, UseAsUBOThenUpdateThenDispatchComputeIndirect)
{
    // Flaky crash on teardown, see http://anglebug.com/40096579
    ANGLE_SKIP_TEST_IF(IsD3D11() && IsIntel() && IsWindows());

    constexpr GLsizei kWidth = 4, kHeight = 6;

    const std::array<uint32_t, 4> kInitialData = {1, 2, 3, 4};
    const std::array<uint32_t, 4> kUpdateData  = {kWidth, kHeight, 1, 0};

    GLBuffer buffer;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kInitialData), kInitialData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();

    constexpr char kVerifyUBO[] = R"(#version 310 es
precision mediump float;
layout(binding = 0) uniform block {
    uvec4 data;
} ubo;
out vec4 colorOut;
void main()
{
    if (all(equal(ubo.data, uvec4(1, 2, 3, 4))))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    ANGLE_GL_PROGRAM(verifyUbo, essl31_shaders::vs::Simple(), kVerifyUBO);
    drawQuad(verifyUbo, essl31_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    // Update buffer data
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(kInitialData), kUpdateData.data());
    EXPECT_GL_NO_ERROR();

    const char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) uniform highp uimage2D uImage;
void main()
{
    imageStore(uImage, ivec2(gl_WorkGroupID.x, gl_WorkGroupID.y), uvec4(100, 0, 0, 0));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer);

    const std::vector<GLuint> inputValues(kWidth * kHeight, 0);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    inputValues.data());
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchComputeIndirect(0);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    GLuint outputValues[kWidth][kHeight];

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();

    constexpr GLuint kExpectedValue = 100u;
    for (int x = 0; x < kWidth; x++)
    {
        for (int y = 0; y < kHeight; y++)
        {
            EXPECT_EQ(kExpectedValue, outputValues[x][y]);
        }
    }
}

// Use image uniform to write texture in compute shader, and verify the content is expected.
TEST_P(ComputeShaderTest, BindImageTexture)
{
    GLTexture mTexture[2];
    GLFramebuffer mFramebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) writeonly uniform highp uimage2D uImage[2];
void main()
{
    imageStore(uImage[0], ivec2(gl_LocalInvocationIndex, gl_WorkGroupID.x), uvec4(100, 0,
0, 0));
    imageStore(uImage[1], ivec2(gl_LocalInvocationIndex, gl_WorkGroupID.x), uvec4(100, 0,
0, 0));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);
    int width = 1, height = 1;
    GLuint inputValues[] = {200};

    glBindTexture(GL_TEXTURE_2D, mTexture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    inputValues);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, mTexture[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, mTexture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    inputValues);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, mTexture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glUseProgram(0);
    GLuint outputValues[2][1];
    GLuint expectedValue = 100;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture[0],
                           0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues[0]);
    EXPECT_GL_NO_ERROR();

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture[1],
                           0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues[1]);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < width * height; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[0][i]);
        EXPECT_EQ(expectedValue, outputValues[1][i]);
    }
}

// Test that binding a 2D slice of a 3D texture works with compute shader
TEST_P(ComputeShaderTest, BindImageTexture3D)
{
    GLTexture mTexture[2];
    GLFramebuffer mFramebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) writeonly uniform highp uimage2D uImage[2];
void main()
{
    imageStore(uImage[0], ivec2(gl_LocalInvocationIndex, gl_WorkGroupID.x), uvec4(100, 0,
0, 0));
    imageStore(uImage[1], ivec2(gl_LocalInvocationIndex, gl_WorkGroupID.x), uvec4(100, 0,
0, 0));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);
    int width = 1, height = 1, depth = 1;
    GLuint inputValues[] = {200};

    glBindTexture(GL_TEXTURE_3D, mTexture[0]);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, width, height, depth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, inputValues);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, mTexture[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_3D, mTexture[1]);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, width, height, depth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, inputValues);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, mTexture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glUseProgram(0);
    GLuint outputValues[2][1];
    GLuint expectedValue = 100;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);

    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture[0], 0, 0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues[0]);
    EXPECT_GL_NO_ERROR();

    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture[1], 0, 0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues[1]);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < width * height; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[0][i]);
        EXPECT_EQ(expectedValue, outputValues[1][i]);
    }
}

// When declare a image array without a binding qualifier, all elements are bound to unit zero.
TEST_P(ComputeShaderTest, ImageArrayWithoutBindingQualifier)
{
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // TODO(xinghua.cao@intel.com): On AMD desktop OpenGL, bind two image variables to unit 0,
    // only one variable is valid.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsDesktopOpenGL());

    GLTexture mTexture;
    GLFramebuffer mFramebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui) writeonly uniform highp uimage2D uImage[2];
void main()
{
    imageStore(uImage[0], ivec2(gl_LocalInvocationIndex, 0), uvec4(100, 0, 0, 0));
    imageStore(uImage[1], ivec2(gl_LocalInvocationIndex, 1), uvec4(100, 0, 0, 0));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);
    constexpr int kTextureWidth = 1, kTextureHeight = 2;
    GLuint inputValues[] = {200, 200};

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kTextureWidth, kTextureHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureWidth, kTextureHeight, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, inputValues);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, mTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
    GLuint outputValues[kTextureWidth * kTextureHeight];
    glReadPixels(0, 0, kTextureWidth, kTextureHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                 outputValues);
    EXPECT_GL_NO_ERROR();

    GLuint expectedValue = 100;
    for (int i = 0; i < kTextureWidth * kTextureHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
}

// When an image array is declared without a binding qualifier, all elements are bound to unit zero.
// Check that the unused uniform image array element does not cause any corruption. Checks for a bug
// where unused element could make the whole array seem as unused.
TEST_P(ComputeShaderTest, ImageArrayUnusedElement)
{
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // TODO(xinghua.cao@intel.com): On AMD desktop OpenGL, bind two image variables to unit 0,
    // only one variable is valid.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsDesktopOpenGL());

    // Vulkan is currently unable to handle unbound image units in compute shaders.
    // http://anglebug.com/42263596
    ANGLE_SKIP_TEST_IF(IsVulkan());

    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding=0) writeonly uniform highp uimage2D uOut;
layout(r32ui, binding=1) readonly uniform highp uimage2D uIn[2];

void main()
{
    uvec4 inValue = imageLoad(uIn[0], ivec2(gl_LocalInvocationID.xy));
    imageStore(uOut, ivec2(gl_LocalInvocationIndex, 0), inValue);
    imageStore(uOut, ivec2(gl_LocalInvocationIndex, 1), inValue);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);
    constexpr int kTextureWidth = 1, kTextureHeight = 2;
    GLuint inputValues[] = {100, 100};
    GLTexture in;
    glBindTexture(GL_TEXTURE_2D, in);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kTextureWidth, kTextureHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureWidth, kTextureHeight, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, inputValues);
    EXPECT_GL_NO_ERROR();
    glBindImageTexture(1, in, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);

    GLuint initValues[] = {111, 111};
    GLTexture out;
    glBindTexture(GL_TEXTURE_2D, out);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kTextureWidth, kTextureHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureWidth, kTextureHeight, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, initValues);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, out, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, out, 0);
    GLuint outputValues[kTextureWidth * kTextureHeight];
    glReadPixels(0, 0, kTextureWidth, kTextureHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                 outputValues);
    EXPECT_GL_NO_ERROR();

    GLuint expectedValue = 100;
    for (int i = 0; i < kTextureWidth * kTextureHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
}
// imageLoad functions
TEST_P(ComputeShaderTest, ImageLoad)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=8) in;
layout(rgba8) uniform highp readonly image2D mImage2DInput;
layout(rgba16i) uniform highp readonly iimageCube mImageCubeInput;
layout(rgba32ui) uniform highp readonly uimage3D mImage3DInput;
layout(r32i) uniform highp writeonly iimage2D imageOut;
void main()
{
    vec4 result2d = imageLoad(mImage2DInput, ivec2(gl_LocalInvocationID.xy));
    ivec4 resultCube = imageLoad(mImageCubeInput, ivec3(gl_LocalInvocationID.xyz));
    uvec4 result3d = imageLoad(mImage3DInput, ivec3(gl_LocalInvocationID.xyz));
    imageStore(imageOut, ivec2(gl_LocalInvocationIndex, 0), ivec4(result2d) + resultCube + ivec4(result3d));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();
}

// imageStore functions
TEST_P(ComputeShaderTest, ImageStore)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=8) in;
layout(rgba16f) uniform highp writeonly imageCube mImageCubeOutput;
layout(r32f) uniform highp writeonly image3D mImage3DOutput;
layout(rgba8ui) uniform highp writeonly uimage2DArray mImage2DArrayOutput;
void main()
{
    imageStore(mImageCubeOutput, ivec3(gl_LocalInvocationID.xyz), vec4(0.0));
    imageStore(mImage3DOutput, ivec3(gl_LocalInvocationID.xyz), vec4(0.0));
    imageStore(mImage2DArrayOutput, ivec3(gl_LocalInvocationID.xyz), uvec4(0));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();
}

// imageSize functions
TEST_P(ComputeShaderTest, ImageSize)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=8) in;
layout(rgba8) uniform highp readonly imageCube mImageCubeInput;
layout(r32i) uniform highp readonly iimage2D mImage2DInput;
layout(rgba16ui) uniform highp readonly uimage2DArray mImage2DArrayInput;
layout(r32i) uniform highp writeonly iimage2D imageOut;
void main()
{
    ivec2 sizeCube = imageSize(mImageCubeInput);
    ivec2 size2D = imageSize(mImage2DInput);
    ivec3 size2DArray = imageSize(mImage2DArrayInput);
    imageStore(imageOut, ivec2(gl_LocalInvocationIndex, 0), ivec4(sizeCube, size2D.x, size2DArray.x));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();
}

// Test that texelFetch works well in compute shader.
TEST_P(ComputeShaderTest, TexelFetchFunction)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=16, local_size_y=12) in;
precision highp usampler2D;
uniform usampler2D tex;
layout(std140, binding = 0) buffer buf {
    uint outData[12][16];
};

void main()
{
    uint x = gl_LocalInvocationID.x;
    uint y = gl_LocalInvocationID.y;
    outData[y][x] = texelFetch(tex, ivec2(x, y), 0).x;
})";

    constexpr unsigned int kWidth  = 16;
    constexpr unsigned int kHeight = 12;
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    GLuint texels[kHeight][kWidth] = {{0}};
    for (unsigned int y = 0; y < kHeight; ++y)
    {
        for (unsigned int x = 0; x < kWidth; ++x)
        {
            texels[y][x] = x + y * kWidth;
        }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    texels);
    glBindTexture(GL_TEXTURE_2D, 0);

    // The array stride are rounded up to the base alignment of a vec4 for std140 layout.
    constexpr unsigned int kArrayStride = 16;
    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kWidth * kHeight * kArrayStride, nullptr,
                 GL_STREAM_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, kWidth * kHeight * kArrayStride, GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();
    for (unsigned int idx = 0; idx < kWidth * kHeight; idx++)
    {
        EXPECT_EQ(idx, *(ptr + idx * kArrayStride / 4));
    }
}

// Test that texture function works well in compute shader.
TEST_P(ComputeShaderTest, TextureFunction)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=16, local_size_y=16) in;
precision highp usampler2D;
uniform usampler2D tex;
layout(std140, binding = 0) buffer buf {
    uint outData[16][16];
};

void main()
{
    uint x = gl_LocalInvocationID.x;
    uint y = gl_LocalInvocationID.y;
    float xCoord = float(x) / float(16);
    float yCoord = float(y) / float(16);
    outData[y][x] = texture(tex, vec2(xCoord, yCoord)).x;
})";

    constexpr unsigned int kWidth  = 16;
    constexpr unsigned int kHeight = 16;
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    GLuint texels[kHeight][kWidth] = {{0}};
    for (unsigned int y = 0; y < kHeight; ++y)
    {
        for (unsigned int x = 0; x < kWidth; ++x)
        {
            texels[y][x] = x + y * kWidth;
        }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    texels);
    glBindTexture(GL_TEXTURE_2D, 0);

    // The array stride are rounded up to the base alignment of a vec4 for std140 layout.
    constexpr unsigned int kArrayStride = 16;
    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kWidth * kHeight * kArrayStride, nullptr,
                 GL_STREAM_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, kWidth * kHeight * kArrayStride, GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();
    for (unsigned int idx = 0; idx < kWidth * kHeight; idx++)
    {
        EXPECT_EQ(idx, *(ptr + idx * kArrayStride / 4));
    }
}

// Test mixed use of sampler and image.
TEST_P(ComputeShaderTest, SamplingAndImageReadWrite)
{
    GLTexture texture[3];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
precision highp usampler2D;
uniform usampler2D tex;
void main()
{
    uvec4 value_1 = texelFetch(tex, ivec2(gl_LocalInvocationID.xy), 0);
    uvec4 value_2 = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value_1 + value_2);
})";

    constexpr int kWidth = 1, kHeight = 1;
    constexpr GLuint kInputValues[3][1] = {{50}, {100}, {20}};

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues[0]);
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues[2]);
    EXPECT_GL_NO_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues[1]);

    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    glBindImageTexture(1, texture[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight];
    constexpr GLuint expectedValue = 150;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[2], 0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
}

// Use image uniform to read and write Texture2D in compute shader, and verify the contents.
TEST_P(ComputeShaderTest, BindImageTextureWithTexture2D)
{
    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr int kWidth = 1, kHeight = 1;
    constexpr GLuint kInputValues[2][1] = {{200}, {100}};

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues[0]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues[1]);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight];
    constexpr GLuint expectedValue = 200;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[1], 0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
}

// Use image uniform to read and write Texture2D with non-zero base in compute shader, and verify
// the contents.
TEST_P(ComputeShaderTest, BindImageTextureWithNonZeroBaseTexture2D)
{
    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr int kWidth = 1, kHeight = 1;
    constexpr GLuint kInputValues[2][1] = {{200}, {100}};

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_R32UI, kWidth * 2, kHeight * 2);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_R32UI, kWidth * 2, kHeight * 2);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 1, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, texture[1], 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight];
    constexpr GLuint expectedValue = 200;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[1], 1);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]) << "  at index: " << i;
    }
}

// Use image uniform to read and write Texture2DArray in compute shader, and verify the contents.
TEST_P(ComputeShaderTest, BindImageTextureWithTexture2DArray)
{
    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=2, local_size_y=2, local_size_z=2) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2DArray uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2DArray uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec3(gl_LocalInvocationID.xyz));
    imageStore(uImage_2, ivec3(gl_LocalInvocationID.xyz), value);
})";

    constexpr int kWidth = 1, kHeight = 1, kDepth = 2;
    constexpr GLuint kInputValues[2][2] = {{200, 200}, {100, 100}};

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture[0]);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[0]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture[1]);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[1]);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, texture[1], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight];
    constexpr GLuint expectedValue = 200;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture[1], 0, 0);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texture[1], 0, 1);
    EXPECT_GL_NO_ERROR();
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
}

// Use image uniform to read and write Texture2DArray with non-zero base in compute shader, and
// verify the contents.
TEST_P(ComputeShaderTest, BindImageTextureWithNonZeroBaseTexture2DArray)
{
    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=2, local_size_y=2, local_size_z=2) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2DArray uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2DArray uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec3(gl_LocalInvocationID.xyz));
    imageStore(uImage_2, ivec3(gl_LocalInvocationID.xyz), value);
})";

    constexpr int kWidth = 1, kHeight = 1, kDepth = 2;
    constexpr GLuint kInputValues[2][2] = {{200, 200}, {100, 100}};

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture[0]);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 2, GL_R32UI, kWidth * 2, kHeight * 2, kDepth);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[0]);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 1);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture[1]);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 2, GL_R32UI, kWidth * 2, kHeight * 2, kDepth);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[1]);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 1);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 1, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, texture[1], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight];
    constexpr GLuint expectedValue = 200;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture[1], 1, 0);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texture[1], 1, 1);
    EXPECT_GL_NO_ERROR();
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
}

// Use image uniform to read and write Texture3D in compute shader, and verify the contents.
TEST_P(ComputeShaderTest, BindImageTextureWithTexture3D)
{
    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=2) in;
layout(r32ui, binding = 0) readonly uniform highp uimage3D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage3D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec3(gl_LocalInvocationID.xyz));
    imageStore(uImage_2, ivec3(gl_LocalInvocationID.xyz), value);
})";

    constexpr int kWidth = 1, kHeight = 1, kDepth = 2;
    constexpr GLuint kInputValues[2][2] = {{200, 200}, {100, 100}};

    glBindTexture(GL_TEXTURE_3D, texture[0]);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[0]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_3D, texture[1]);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[1]);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, texture[1], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight];
    constexpr GLuint expectedValue = 200;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture[1], 0, 0);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texture[1], 0, 1);
    EXPECT_GL_NO_ERROR();
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]);
    }
}

// Use image uniform to read and write TextureCube in compute shader, and verify the contents.
TEST_P(ComputeShaderTest, BindImageTextureWithTextureCube)
{
    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimageCube uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimageCube uImage_2;
void main()
{
    for (int i = 0; i < 6; i++)
    {
        uvec4 value = imageLoad(uImage_1, ivec3(gl_LocalInvocationID.xy, i));
        imageStore(uImage_2, ivec3(gl_LocalInvocationID.xy, i), value);
    }
})";

    constexpr int kWidth = 1, kHeight = 1;
    constexpr GLuint kInputValues[2][1] = {{200}, {100}};

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture[0]);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_R32UI, kWidth, kHeight);
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         face++)
    {
        glTexSubImage2D(face, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                        kInputValues[0]);
    }
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture[1]);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_R32UI, kWidth, kHeight);
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         face++)
    {
        glTexSubImage2D(face, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                        kInputValues[1]);
    }
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, texture[1], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight];
    constexpr GLuint expectedValue = 200;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    for (GLenum face = 0; face < 6; face++)
    {
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture[1], 0);
        EXPECT_GL_NO_ERROR();
        glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
        EXPECT_GL_NO_ERROR();

        for (int i = 0; i < kWidth * kHeight; i++)
        {
            EXPECT_EQ(expectedValue, outputValues[i]);
        }
    }
}

// Use image uniform to read and write one layer of Texture2DArray in compute shader, and verify the
// contents.
TEST_P(ComputeShaderTest, BindImageTextureWithOneLayerTexture2DArray)
{
    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr int kWidth = 1, kHeight = 1, kDepth = 2;
    constexpr int kResultSize           = kWidth * kHeight;
    constexpr GLuint kInputValues[2][2] = {{200, 150}, {100, 50}};
    constexpr GLuint expectedValue_1    = 200;
    constexpr GLuint expectedValue_2    = 100;
    GLuint outputValues[kResultSize];

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture[0]);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[0]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture[1]);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[1]);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glBindImageTexture(1, texture[1], 0, GL_FALSE, 1, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture[1], 0, 0);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texture[1], 0, 1);
    EXPECT_GL_NO_ERROR();
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kResultSize; i++)
    {
        EXPECT_EQ(expectedValue_2, outputValues[i]);
    }
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kResultSize; i++)
    {
        EXPECT_EQ(expectedValue_1, outputValues[i]);
    }
}

// Use image uniform to read and write one layer of Texture3D in compute shader, and verify the
// contents.
TEST_P(ComputeShaderTest, BindImageTextureWithOneLayerTexture3D)
{
    // Vulkan validation error creating a 2D image view of a 3D image layer.
    // http://anglebug.com/42262531
    ANGLE_SKIP_TEST_IF(IsVulkan());

    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr int kWidth = 1, kHeight = 1, kDepth = 2;
    constexpr int kResultSize           = kWidth * kHeight;
    constexpr GLuint kInputValues[2][2] = {{200, 150}, {100, 50}};
    constexpr GLuint expectedValue_1    = 150;
    constexpr GLuint expectedValue_2    = 50;
    GLuint outputValues[kResultSize];

    glBindTexture(GL_TEXTURE_3D, texture[0]);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[0]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_3D, texture[1]);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, kInputValues[1]);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_FALSE, 1, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture[1], 0, 0);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texture[1], 0, 1);
    EXPECT_GL_NO_ERROR();
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kResultSize; i++)
    {
        EXPECT_EQ(expectedValue_1, outputValues[i]);
    }
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kResultSize; i++)
    {
        EXPECT_EQ(expectedValue_2, outputValues[i]);
    }
}

// Use image uniform to read and write one layer of TextureCube in compute shader, and verify the
// contents.
TEST_P(ComputeShaderTest, BindImageTextureWithOneLayerTextureCube)
{
    // GL_FRAMEBUFFER_BARRIER_BIT is invalid on Nvidia Linux platform.
    // http://anglebug.com/42262394
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGL() && IsLinux());

    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr int kWidth = 1, kHeight = 1;
    constexpr int kResultSize           = kWidth * kHeight;
    constexpr GLuint kInputValues[2][1] = {{200}, {100}};
    constexpr GLuint expectedValue_1    = 200;
    constexpr GLuint expectedValue_2    = 100;
    GLuint outputValues[kResultSize];

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture[0]);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_R32UI, kWidth, kHeight);
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         face++)
    {
        glTexSubImage2D(face, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                        kInputValues[0]);
    }
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture[1]);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_R32UI, kWidth, kHeight);
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         face++)
    {
        glTexSubImage2D(face, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                        kInputValues[1]);
    }
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_FALSE, 3, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glBindImageTexture(1, texture[1], 0, GL_FALSE, 4, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    for (GLenum face = 0; face < 6; face++)
    {
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture[1], 0);
        EXPECT_GL_NO_ERROR();
        glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
        EXPECT_GL_NO_ERROR();

        if (face == 4)
        {
            for (int i = 0; i < kResultSize; i++)
            {
                EXPECT_EQ(expectedValue_1, outputValues[i]);
            }
        }
        else
        {
            for (int i = 0; i < kResultSize; i++)
            {
                EXPECT_EQ(expectedValue_2, outputValues[i]);
            }
        }
    }
}

// Test to bind kinds of texture types, bind either the entire texture
// level or a single layer or face of the face level.
TEST_P(ComputeShaderTest, BindImageTextureWithMixTextureTypes)
{
    // GL_FRAMEBUFFER_BARRIER_BIT is invalid on Nvidia Linux platform.
    // http://anglebug.com/42262394
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGL() && IsLinux());

    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    GLTexture texture[4];
    GLFramebuffer framebuffer;
    const char csSource[] =
        R"(#version 310 es
        layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
        layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
        layout(r32ui, binding = 1) readonly uniform highp uimage2D uImage_2;
        layout(r32ui, binding = 2) readonly uniform highp uimage3D uImage_3;
        layout(r32ui, binding = 3) writeonly uniform highp uimage2D uImage_4;
        void main()
        {
            uvec4 value_1 = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
            uvec4 value_2 = imageLoad(uImage_2, ivec2(gl_LocalInvocationID.xy));
            uvec4 value_3 = imageLoad(uImage_3, ivec3(gl_LocalInvocationID.xyz));
            imageStore(uImage_4, ivec2(gl_LocalInvocationID.xy), value_1 + value_2 + value_3);
        })";

    constexpr int kWidth = 1, kHeight = 1, kDepth = 2;
    constexpr int kResultSize               = kWidth * kHeight;
    constexpr GLuint kInputValues2D[1]      = {11};
    constexpr GLuint KInputValues2DArray[2] = {23, 35};
    constexpr GLuint KInputValues3D[2]      = {102, 67};
    constexpr GLuint KInputValuesCube[1]    = {232};

    constexpr GLuint expectedValue_1 = 148;
    constexpr GLuint expectedValue_2 = 232;
    GLuint outputValues[kResultSize];

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues2D);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture[1]);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, KInputValues2DArray);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_3D, texture[2]);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RED_INTEGER,
                    GL_UNSIGNED_INT, KInputValues3D);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture[3]);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_R32UI, kWidth, kHeight);
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         face++)
    {
        glTexSubImage2D(face, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                        KInputValuesCube);
    }
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, csSource);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glBindImageTexture(1, texture[1], 0, GL_FALSE, 1, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glBindImageTexture(2, texture[2], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glBindImageTexture(3, texture[3], 0, GL_FALSE, 4, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    for (GLenum face = 0; face < 6; face++)
    {
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture[3], 0);
        EXPECT_GL_NO_ERROR();
        glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
        EXPECT_GL_NO_ERROR();

        if (face == 4)
        {
            for (int i = 0; i < kResultSize; i++)
            {
                EXPECT_EQ(expectedValue_1, outputValues[i]);
            }
        }
        else
        {
            for (int i = 0; i < kResultSize; i++)
            {
                EXPECT_EQ(expectedValue_2, outputValues[i]);
            }
        }
    }
}

// Verify an INVALID_OPERATION error is reported when querying GL_COMPUTE_WORK_GROUP_SIZE for a
// program which has not been linked successfully or which does not contain objects to form a
// compute shader.
TEST_P(ComputeShaderTest, QueryComputeWorkGroupSize)
{
    constexpr char kVS[] = R"(#version 310 es
void main()
{
})";

    constexpr char kFS[] = R"(#version 310 es
void main()
{
})";

    GLint workGroupSize[3];

    ANGLE_GL_PROGRAM(graphicsProgram, kVS, kFS);
    glGetProgramiv(graphicsProgram, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLuint computeProgram = glCreateProgram();
    GLShader computeShader(GL_COMPUTE_SHADER);
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    glDetachShader(computeProgram, computeShader);

    GLint linkStatus;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &linkStatus);
    ASSERT_GL_FALSE(linkStatus);

    glGetProgramiv(computeProgram, GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDeleteProgram(computeProgram);

    ASSERT_GL_NO_ERROR();
}

// Use groupMemoryBarrier and barrier to sync reads/writes order and the execution
// order of multiple shader invocations in compute shader.
TEST_P(ComputeShaderTest, GroupMemoryBarrierAndBarrierTest)
{
    // TODO(xinghua.cao@intel.com): Figure out why we get this error message
    // that shader uses features not recognized by this D3D version.
    ANGLE_SKIP_TEST_IF((IsAMD() || IsNVIDIA()) && IsD3D11());
    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    GLTexture texture;
    GLFramebuffer framebuffer;

    // Each invocation first stores a single value in an image, then each invocation sums up
    // all the values in the image and stores the sum in the image. groupMemoryBarrier is
    // used to order reads/writes to variables stored in memory accessible to other shader
    // invocations, and barrier is used to control the relative execution order of multiple
    // shader invocations used to process a local work group.
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=2, local_size_y=2, local_size_z=1) in;
layout(r32i, binding = 0) uniform highp iimage2D image;
void main()
{
    uint x = gl_LocalInvocationID.x;
    uint y = gl_LocalInvocationID.y;
    imageStore(image, ivec2(gl_LocalInvocationID.xy), ivec4(x + y));
    groupMemoryBarrier();
    barrier();
    int sum = 0;
    for (int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 2; j++)
        {
            sum += imageLoad(image, ivec2(i, j)).x;
        }
    }
    groupMemoryBarrier();
    barrier();
    imageStore(image, ivec2(gl_LocalInvocationID.xy), ivec4(sum));
})";

    constexpr int kWidth = 2, kHeight = 2;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, kWidth, kHeight);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight];
    constexpr GLuint kExpectedValue = 4;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_INT, outputValues);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(kExpectedValue, outputValues[i]);
    }
}

// Verify that a link error is generated when the sum of the number of active image uniforms and
// active shader storage blocks in a compute shader exceeds GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES.
TEST_P(ComputeShaderTest, ExceedCombinedShaderOutputResourcesInCS)
{
    GLint maxCombinedShaderOutputResources;
    GLint maxComputeShaderStorageBlocks;
    GLint maxComputeImageUniforms;

    glGetIntegerv(GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES, &maxCombinedShaderOutputResources);
    glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &maxComputeShaderStorageBlocks);
    glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &maxComputeImageUniforms);

    ANGLE_SKIP_TEST_IF(maxCombinedShaderOutputResources >=
                       maxComputeShaderStorageBlocks + maxComputeImageUniforms);

    std::ostringstream computeShaderStream;
    computeShaderStream << "#version 310 es\n"
                           "layout(local_size_x = 3, local_size_y = 1, local_size_z = 1) in;\n"
                           "layout(shared, binding = 0) buffer blockName"
                           "{\n"
                           "    uint data;\n"
                           "} instance["
                        << maxComputeShaderStorageBlocks << "];\n";

    ASSERT_GE(maxComputeImageUniforms, 4);
    int numImagesInArray  = maxComputeImageUniforms / 2;
    int numImagesNonArray = maxComputeImageUniforms - numImagesInArray;
    for (int i = 0; i < numImagesNonArray; ++i)
    {
        computeShaderStream << "layout(r32f, binding = " << i << ") uniform highp image2D image"
                            << i << ";\n";
    }

    computeShaderStream << "layout(r32f, binding = " << numImagesNonArray
                        << ") uniform highp image2D imageArray[" << numImagesInArray << "];\n";

    computeShaderStream << "void main()\n"
                           "{\n"
                           "    uint val = 0u;\n"
                           "    vec4 val2 = vec4(0.0);\n";

    for (int i = 0; i < maxComputeShaderStorageBlocks; ++i)
    {
        computeShaderStream << "    val += instance[" << i << "].data; \n";
    }

    for (int i = 0; i < numImagesNonArray; ++i)
    {
        computeShaderStream << "    val2 += imageLoad(image" << i
                            << ", ivec2(gl_LocalInvocationID.xy)); \n";
    }

    for (int i = 0; i < numImagesInArray; ++i)
    {
        computeShaderStream << "    val2 += imageLoad(imageArray[" << i << "]"
                            << ", ivec2(gl_LocalInvocationID.xy)); \n";
    }

    computeShaderStream << "    instance[0].data = val + uint(val2.x);\n"
                           "}\n";

    GLuint computeProgram = CompileComputeProgram(computeShaderStream.str().c_str());
    EXPECT_EQ(0u, computeProgram);
}

// Test that uniform block with struct member in compute shader is supported.
TEST_P(ComputeShaderTest, UniformBlockWithStructMember)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=8) in;
layout(rgba8) uniform highp readonly image2D mImage2DInput;
layout(rgba8) uniform highp writeonly image2D mImage2DOutput;
struct S {
    ivec3 a;
    ivec2 b;
};

layout(std140, binding=0) uniform blockName {
    S bd;
} instanceName;
void main()
{
    ivec2 t1 = instanceName.bd.b;
    vec4 result2d = imageLoad(mImage2DInput, t1);
    imageStore(mImage2DOutput, ivec2(gl_LocalInvocationID.xy), result2d);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();
}

// Verify shared non-array variables can work correctly.
TEST_P(ComputeShaderTest, NonArraySharedVariable)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSShader[] = R"(#version 310 es
layout (local_size_x = 2, local_size_y = 2, local_size_z = 1) in;
layout (r32ui, binding = 0) readonly uniform highp uimage2D srcImage;
layout (r32ui, binding = 1) writeonly uniform highp uimage2D dstImage;
shared uint temp;
void main()
{
    if (gl_LocalInvocationID == uvec3(0, 0, 0))
    {
        temp = imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)).x;
    }
    groupMemoryBarrier();
    barrier();
    if (gl_LocalInvocationID == uvec3(1, 1, 0))
    {
        imageStore(dstImage, ivec2(gl_LocalInvocationID.xy), uvec4(temp));
    }
    else
    {
        uint inputValue = imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)).x;
        imageStore(dstImage, ivec2(gl_LocalInvocationID.xy), uvec4(inputValue));
    }
})";

    const std::array<GLuint, 4> inputData      = {{250, 200, 150, 100}};
    const std::array<GLuint, 4> expectedValues = {{250, 200, 150, 250}};
    runSharedMemoryTest<GLuint, 2, 2>(kCSShader, GL_R32UI, GL_UNSIGNED_INT, inputData,
                                      expectedValues);
}

// Verify shared non-struct array variables can work correctly.
TEST_P(ComputeShaderTest, NonStructArrayAsSharedVariable)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSShader[] = R"(#version 310 es
layout (local_size_x = 2, local_size_y = 2, local_size_z = 1) in;
layout (r32ui, binding = 0) readonly uniform highp uimage2D srcImage;
layout (r32ui, binding = 1) writeonly uniform highp uimage2D dstImage;
shared uint sharedData[2][2];
void main()
{
    uint inputData = imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)).x;
    sharedData[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = inputData;
    groupMemoryBarrier();
    barrier();
    imageStore(dstImage, ivec2(gl_LocalInvocationID.xy),
                uvec4(sharedData[gl_LocalInvocationID.y][gl_LocalInvocationID.x]));
})";

    const std::array<GLuint, 4> inputData      = {{250, 200, 150, 100}};
    const std::array<GLuint, 4> expectedValues = {{250, 150, 200, 100}};
    runSharedMemoryTest<GLuint, 2, 2>(kCSShader, GL_R32UI, GL_UNSIGNED_INT, inputData,
                                      expectedValues);
}

// Verify shared struct array variables work correctly.
TEST_P(ComputeShaderTest, StructArrayAsSharedVariable)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSShader[] = R"(#version 310 es
layout (local_size_x = 2, local_size_y = 2, local_size_z = 1) in;
layout (r32ui, binding = 0) readonly uniform highp uimage2D srcImage;
layout (r32ui, binding = 1) writeonly uniform highp uimage2D dstImage;
struct SharedStruct
{
    uint data;
};
shared SharedStruct sharedData[2][2];
void main()
{
    uint inputData = imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)).x;
    sharedData[gl_LocalInvocationID.x][gl_LocalInvocationID.y].data = inputData;
    groupMemoryBarrier();
    barrier();
    imageStore(dstImage, ivec2(gl_LocalInvocationID.xy),
                uvec4(sharedData[gl_LocalInvocationID.y][gl_LocalInvocationID.x].data));
})";

    const std::array<GLuint, 4> inputData      = {{250, 200, 150, 100}};
    const std::array<GLuint, 4> expectedValues = {{250, 150, 200, 100}};
    runSharedMemoryTest<GLuint, 2, 2>(kCSShader, GL_R32UI, GL_UNSIGNED_INT, inputData,
                                      expectedValues);
}

// Verify using atomic functions without return value can work correctly.
TEST_P(ComputeShaderTest, AtomicFunctionsNoReturnValue)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    // Fails to link on Android.  http://anglebug.com/42262519
    ANGLE_SKIP_TEST_IF(IsAndroid());

    ANGLE_SKIP_TEST_IF(IsARM64() && IsWindows() && IsD3D());

    const char kCSShader[] = R"(#version 310 es
layout (local_size_x = 8, local_size_y = 1, local_size_z = 1) in;
layout (r32ui, binding = 0) readonly uniform highp uimage2D srcImage;
layout (r32ui, binding = 1) writeonly uniform highp uimage2D dstImage;

const uint kSumIndex = 0u;
const uint kMinIndex = 1u;
const uint kMaxIndex = 2u;
const uint kOrIndex = 3u;
const uint kAndIndex = 4u;
const uint kXorIndex = 5u;
const uint kExchangeIndex = 6u;
const uint kCompSwapIndex = 7u;

shared highp uint results[8];

void main()
{
    if (gl_LocalInvocationID.x == kMinIndex || gl_LocalInvocationID.x == kAndIndex)
    {
        results[gl_LocalInvocationID.x] = 0xFFFFu;
    }
    else if (gl_LocalInvocationID.x == kCompSwapIndex)
    {
        results[gl_LocalInvocationID.x] = 1u;
    }
    else
    {
        results[gl_LocalInvocationID.x] = 0u;
    }
    memoryBarrierShared();
    barrier();

    uint value = imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)).x;
    atomicAdd(results[kSumIndex], value);
    atomicMin(results[kMinIndex], value);
    atomicMax(results[kMaxIndex], value);
    atomicOr(results[kOrIndex], value);
    atomicAnd(results[kAndIndex], value);
    atomicXor(results[kXorIndex], value);
    atomicExchange(results[kExchangeIndex], value);
    atomicCompSwap(results[kCompSwapIndex], value, 256u);
    memoryBarrierShared();
    barrier();

    imageStore(dstImage, ivec2(gl_LocalInvocationID.xy),
                uvec4(results[gl_LocalInvocationID.x]));
})";

    const std::array<GLuint, 8> inputData      = {{1, 2, 4, 8, 16, 32, 64, 128}};
    const std::array<GLuint, 8> expectedValues = {{255, 1, 128, 255, 0, 255, 128, 256}};
    runSharedMemoryTest<GLuint, 8, 1>(kCSShader, GL_R32UI, GL_UNSIGNED_INT, inputData,
                                      expectedValues);
}

// Verify using atomic functions in a non-initializer single assignment can work correctly.
TEST_P(ComputeShaderTest, AtomicFunctionsInNonInitializerSingleAssignment)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSShader[] = R"(#version 310 es
layout (local_size_x = 9, local_size_y = 1, local_size_z = 1) in;
layout (r32i, binding = 0) readonly uniform highp iimage2D srcImage;
layout (r32i, binding = 1) writeonly uniform highp iimage2D dstImage;

shared highp int sharedVariable;

shared highp int inputData[9];
shared highp int outputData[9];

void main()
{
    int inputValue = imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)).x;
    inputData[gl_LocalInvocationID.x] = inputValue;
    memoryBarrierShared();
    barrier();

    if (gl_LocalInvocationID.x == 0u)
    {
        sharedVariable = 0;

        outputData[0] = atomicAdd(sharedVariable, inputData[0]);
        outputData[1] = atomicMin(sharedVariable, inputData[1]);
        outputData[2] = atomicMax(sharedVariable, inputData[2]);
        outputData[3] = atomicAnd(sharedVariable, inputData[3]);
        outputData[4] = atomicOr(sharedVariable, inputData[4]);
        outputData[5] = atomicXor(sharedVariable, inputData[5]);
        outputData[6] = atomicExchange(sharedVariable, inputData[6]);
        outputData[7] = atomicCompSwap(sharedVariable, 64, inputData[7]);
        outputData[8] = atomicAdd(sharedVariable, inputData[8]);
    }
    memoryBarrierShared();
    barrier();

    imageStore(dstImage, ivec2(gl_LocalInvocationID.xy),
                ivec4(outputData[gl_LocalInvocationID.x]));
})";

    const std::array<GLint, 9> inputData      = {{1, 2, 4, 8, 16, 32, 64, 128, 1}};
    const std::array<GLint, 9> expectedValues = {{0, 1, 1, 4, 0, 16, 48, 64, 128}};
    runSharedMemoryTest<GLint, 9, 1>(kCSShader, GL_R32I, GL_INT, inputData, expectedValues);
}

// Verify using atomic functions in an initializers and using unsigned int works correctly.
TEST_P(ComputeShaderTest, AtomicFunctionsInitializerWithUnsigned)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    constexpr char kCShader[] = R"(#version 310 es
layout (local_size_x = 9, local_size_y = 1, local_size_z = 1) in;
layout (r32ui, binding = 0) readonly uniform highp uimage2D srcImage;
layout (r32ui, binding = 1) writeonly uniform highp uimage2D dstImage;

shared highp uint sharedVariable;

shared highp uint inputData[9];
shared highp uint outputData[9];

void main()
{
    uint inputValue = imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)).x;
    inputData[gl_LocalInvocationID.x] = inputValue;
    memoryBarrierShared();
    barrier();

    if (gl_LocalInvocationID.x == 0u)
    {
        sharedVariable = 0u;

        uint addValue = atomicAdd(sharedVariable, inputData[0]);
        outputData[0] = addValue;
        uint minValue = atomicMin(sharedVariable, inputData[1]);
        outputData[1] = minValue;
        uint maxValue = atomicMax(sharedVariable, inputData[2]);
        outputData[2] = maxValue;
        uint andValue = atomicAnd(sharedVariable, inputData[3]);
        outputData[3] = andValue;
        uint orValue = atomicOr(sharedVariable, inputData[4]);
        outputData[4] = orValue;
        uint xorValue = atomicXor(sharedVariable, inputData[5]);
        outputData[5] = xorValue;
        uint exchangeValue = atomicExchange(sharedVariable, inputData[6]);
        outputData[6] = exchangeValue;
        uint compSwapValue = atomicCompSwap(sharedVariable, 64u, inputData[7]);
        outputData[7] = compSwapValue;
        uint sharedVariable = atomicAdd(sharedVariable, inputData[8]);
        outputData[8] = sharedVariable;

    }
    memoryBarrierShared();
    barrier();

    imageStore(dstImage, ivec2(gl_LocalInvocationID.xy),
                uvec4(outputData[gl_LocalInvocationID.x]));
})";

    constexpr std::array<GLuint, 9> kInputData      = {{1, 2, 4, 8, 16, 32, 64, 128, 1}};
    constexpr std::array<GLuint, 9> kExpectedValues = {{0, 1, 1, 4, 0, 16, 48, 64, 128}};
    runSharedMemoryTest<GLuint, 9, 1>(kCShader, GL_R32UI, GL_UNSIGNED_INT, kInputData,
                                      kExpectedValues);
}

// Verify using atomic functions inside expressions as unsigned int.
TEST_P(ComputeShaderTest, AtomicFunctionsReturnWithUnsigned)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    constexpr char kCShader[] = R"(#version 310 es
layout (local_size_x = 9, local_size_y = 1, local_size_z = 1) in;
layout (r32ui, binding = 0) readonly uniform highp uimage2D srcImage;
layout (r32ui, binding = 1) writeonly uniform highp uimage2D dstImage;

shared highp uint sharedVariable;

shared highp uint inputData[9];
shared highp uint outputData[9];

void main()
{
    uint inputValue = imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)).x;
    inputData[gl_LocalInvocationID.x] = inputValue;
    memoryBarrierShared();
    barrier();

    if (gl_LocalInvocationID.x == 0u)
    {
        sharedVariable = 0u;

        outputData[0] = 1u + atomicAdd(sharedVariable, inputData[0]);
        outputData[1] = 1u + atomicMin(sharedVariable, inputData[1]);
        outputData[2] = 1u + atomicMax(sharedVariable, inputData[2]);
        outputData[3] = 1u + atomicAnd(sharedVariable, inputData[3]);
        outputData[4] = 1u + atomicOr(sharedVariable, inputData[4]);
        outputData[5] = 1u + atomicXor(sharedVariable, inputData[5]);
        outputData[6] = 1u + atomicExchange(sharedVariable, inputData[6]);
        outputData[7] = 1u + atomicCompSwap(sharedVariable, 64u, inputData[7]);
        outputData[8] = 1u + atomicAdd(sharedVariable, inputData[8]);
    }
    memoryBarrierShared();
    barrier();

    imageStore(dstImage, ivec2(gl_LocalInvocationID.xy),
                uvec4(outputData[gl_LocalInvocationID.x]));
})";

    constexpr std::array<GLuint, 9> kInputData      = {{1, 2, 4, 8, 16, 32, 64, 128, 1}};
    constexpr std::array<GLuint, 9> kExpectedValues = {{1, 2, 2, 5, 1, 17, 49, 65, 129}};
    runSharedMemoryTest<GLuint, 9, 1>(kCShader, GL_R32UI, GL_UNSIGNED_INT, kInputData,
                                      kExpectedValues);
}

// Verify using nested atomic functions in expressions.
TEST_P(ComputeShaderTest, AtomicFunctionsReturnWithMultipleTypes)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    constexpr char kCShader[] = R"(#version 310 es
layout (local_size_x = 4, local_size_y = 1, local_size_z = 1) in;
layout (r32ui, binding = 0) readonly uniform highp uimage2D srcImage;
layout (r32ui, binding = 1) writeonly uniform highp uimage2D dstImage;

shared highp uint sharedVariable;
shared highp int  indexVariable;

shared highp uint inputData[4];
shared highp uint outputData[4];

void main()
{
    uint inputValue = imageLoad(srcImage, ivec2(gl_LocalInvocationID.xy)).x;
    inputData[gl_LocalInvocationID.x] = inputValue;
    memoryBarrierShared();
    barrier();

    if (gl_LocalInvocationID.x == 0u)
    {
        sharedVariable = 0u;
        indexVariable = 2;

        outputData[0] = 1u + atomicAdd(sharedVariable, inputData[atomicAdd(indexVariable, -1)]);
        outputData[1] = 1u + atomicAdd(sharedVariable, inputData[atomicAdd(indexVariable, -1)]);
        outputData[2] = 1u + atomicAdd(sharedVariable, inputData[atomicAdd(indexVariable, -1)]);
        outputData[3] = atomicAdd(sharedVariable, 0u);

    }
    memoryBarrierShared();
    barrier();

    imageStore(dstImage, ivec2(gl_LocalInvocationID.xy),
                uvec4(outputData[gl_LocalInvocationID.x]));
})";

    constexpr std::array<GLuint, 4> kInputData      = {{1, 2, 3, 0}};
    constexpr std::array<GLuint, 4> kExpectedValues = {{1, 4, 6, 6}};
    runSharedMemoryTest<GLuint, 4, 1>(kCShader, GL_R32UI, GL_UNSIGNED_INT, kInputData,
                                      kExpectedValues);
}

// Basic uniform buffer functionality.
TEST_P(ComputeShaderTest, UniformBuffer)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    GLTexture texture;
    GLBuffer buffer;
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
uniform uni
{
    uvec4 value;
};
layout(rgba32ui, binding = 0) writeonly uniform highp uimage2D uImage;
void main()
{
    imageStore(uImage, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr int kWidth = 1, kHeight = 1;
    constexpr GLuint kInputValues[4] = {56, 57, 58, 59};

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGBA_INTEGER, GL_UNSIGNED_INT,
                    kInputValues);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "uni");
    EXPECT_NE(uniformBufferIndex, -1);
    GLuint data[4] = {201, 202, 203, 204};
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLuint) * 4, data, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight * 4];
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, kWidth, kHeight, GL_RGBA_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < kWidth * kHeight * 4; i++)
    {
        EXPECT_EQ(data[i], outputValues[i]);
    }
}

// Test that storing data to image and then loading the same image data works correctly.
TEST_P(ComputeShaderTest, StoreImageThenLoad)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr GLuint kInputValues[3][1] = {{300}, {200}, {100}};
    GLTexture texture[3];
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, kInputValues[0]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, kInputValues[1]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, kInputValues[2]);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, texture[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    glBindImageTexture(1, texture[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    GLuint outputValue;
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[2], 0);
    glReadPixels(0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &outputValue);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(300u, outputValue);
}

// Test that loading image data and then storing data to the same image works correctly.
TEST_P(ComputeShaderTest, LoadImageThenStore)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr GLuint kInputValues[3][1] = {{300}, {200}, {100}};
    GLTexture texture[3];
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, kInputValues[0]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, kInputValues[1]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, kInputValues[2]);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(0, texture[2], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    glBindImageTexture(1, texture[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    GLuint outputValue;
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[0], 0);
    glReadPixels(0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &outputValue);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(100u, outputValue);
}

// Test that the length of a struct buffer variable is supported.
TEST_P(ComputeShaderTest, ShaderStorageBlocksStructLength)
{
    const char kCSSource[] = R"(#version 310 es
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Particle
{
    int len;
};

layout(binding = 0, std430) readonly buffer Buf1
{
    Particle particlesRead[];
};

layout(binding = 1, std430) buffer Buf2
{
    Particle particlesWrite[];
};

void main()
{
    int index = int(gl_GlobalInvocationID.x);
    particlesWrite[index].len = particlesRead.length();
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    EXPECT_GL_NO_ERROR();
}

// Test that scalar buffer variables are supported.
TEST_P(ComputeShaderTest, ShaderStorageBlocksScalar)
{
    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1) in;
layout(std140, binding = 0) buffer blockA {
    uvec3 uv;
    float f;
} instanceA;
layout(std140, binding = 1) buffer blockB {
    vec2 v;
    uint u[3];
    float f;
};
void main()
{
    f = instanceA.f;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    EXPECT_GL_NO_ERROR();
}

// Test that vector buffer variables are supported.
TEST_P(ComputeShaderTest, ShaderStorageBlocksVector)
{
    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1) in;
layout(std140, binding = 0) buffer blockA {
    vec2 f;
} instanceA;
layout(std140, binding = 1) buffer blockB {
    vec3 f;
};
void main()
{
    f[1] = instanceA.f[0];
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    EXPECT_GL_NO_ERROR();
}

// Test that matrix buffer variables are supported.
TEST_P(ComputeShaderTest, ShaderStorageBlocksMatrix)
{
    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1) in;
layout(std140, binding = 0) buffer blockA {
    mat3x4 m;
} instanceA;
layout(std140, binding = 1) buffer blockB {
    mat3x4 m;
};
void main()
{
    m[0][1] = instanceA.m[0][1];
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    EXPECT_GL_NO_ERROR();
}

// Test that scalar array buffer variables are supported.
TEST_P(ComputeShaderTest, ShaderStorageBlocksScalarArray)
{
    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=8) in;
layout(std140, binding = 0) buffer blockA {
    float f[8];
} instanceA;
layout(std140, binding = 1) buffer blockB {
    float f[8];
};
void main()
{
    f[gl_LocalInvocationIndex] = instanceA.f[gl_LocalInvocationIndex];
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    EXPECT_GL_NO_ERROR();
}

// Test that vector array buffer variables are supported.
TEST_P(ComputeShaderTest, ShaderStorageBlocksVectorArray)
{
    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=4) in;
layout(std140, binding = 0) buffer blockA {
    vec2 v[4];
} instanceA;
layout(std140, binding = 1) buffer blockB {
    vec4 v[4];
};
void main()
{
    v[0][gl_LocalInvocationIndex] = instanceA.v[gl_LocalInvocationIndex][1];
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    EXPECT_GL_NO_ERROR();
}

// Test that matrix array buffer variables are supported.
TEST_P(ComputeShaderTest, ShaderStorageBlocksMatrixArray)
{
    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=8) in;
layout(std140, binding = 0) buffer blockA {
    float v1[5];
    mat4 m[8];
} instanceA;
layout(std140, binding = 1) buffer blockB {
    vec2 v1[3];
    mat4 m[8];
};
void main()
{
    float data = instanceA.m[gl_LocalInvocationIndex][0][0];
    m[gl_LocalInvocationIndex][gl_LocalInvocationIndex][gl_LocalInvocationIndex] = data;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    EXPECT_GL_NO_ERROR();
}

// Test that shader storage blocks only in assignment right is supported.
TEST_P(ComputeShaderTest, ShaderStorageBlocksInAssignmentRight)
{
    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=8) in;
layout(std140, binding = 0) buffer blockA {
    float data[8];
} instanceA;
layout(r32f, binding = 0) writeonly uniform highp image2D imageOut;

void main()
{
    float data = 1.0;
    data = instanceA.data[gl_LocalInvocationIndex];
    imageStore(imageOut, ivec2(gl_LocalInvocationID.xy), vec4(data));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    EXPECT_GL_NO_ERROR();
}

// Test that shader storage blocks with unsized array are supported.
TEST_P(ComputeShaderTest, ShaderStorageBlocksWithUnsizedArray)
{
    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=8) in;
layout(std140, binding = 0) buffer blockA {
    float v[];
} instanceA;
layout(std140, binding = 0) buffer blockB {
    float v[];
} instanceB[1];

void main()
{
    float data = instanceA.v[gl_LocalInvocationIndex];
    instanceB[0].v[gl_LocalInvocationIndex * 2u + 1u] = data;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCSSource);
    EXPECT_GL_NO_ERROR();
}

// Test that EOpIndexDirect/EOpIndexIndirect/EOpIndexDirectStruct nodes in ssbo EOpIndexInDirect
// don't need to calculate the offset and should be translated by OutputHLSL directly.
TEST_P(ComputeShaderTest, IndexAndDotOperatorsInSSBOIndexIndirectOperator)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout(local_size_x=1) in;
layout(std140, binding = 0) buffer blockA {
    float v[4];
};
layout(std140, binding = 1) buffer blockB {
    float v[4];
} instanceB[1];
struct S
{
    uvec4 index[2];
} s;
void main()
{
        s.index[0] = uvec4(0u, 1u, 2u, 3u);
    float data = v[s.index[0].y];
    instanceB[0].v[s.index[0].x] = data;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();
}

// Test that swizzle node in non-SSBO symbol works well.
TEST_P(ComputeShaderTest, ShaderStorageBlocksWithNonSSBOSwizzle)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout(local_size_x=8) in;
layout(std140, binding = 0) buffer blockA {
    float v[8];
};
layout(std140, binding = 1) buffer blockB {
    float v[8];
} instanceB[1];

void main()
{
    float data = v[gl_GlobalInvocationID.x];
    instanceB[0].v[gl_GlobalInvocationID.x] = data;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();
}

// Test that swizzle node in SSBO symbol works well.
TEST_P(ComputeShaderTest, ShaderStorageBlocksWithSSBOSwizzle)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout(local_size_x=1) in;
layout(std140, binding = 0) buffer blockA {
    vec2 v;
};
layout(std140, binding = 1) buffer blockB {
    float v;
} instanceB[1];

void main()
{
    instanceB[0].v = v.x;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();
}

// Test that a large struct array in std140 uniform block won't consume too much time.
TEST_P(ComputeShaderTest, LargeStructArraySize)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout(local_size_x=8) in;
precision mediump float;

struct InstancingData
{
    mat4 transformation;
};

#define MAX_INSTANCE_COUNT 800

layout(std140) uniform InstanceBlock
{
    InstancingData instances[MAX_INSTANCE_COUNT];
};

layout(std140, binding = 1) buffer blockB {
    mat4 v[];
} instanceB;

void main()
{
    instanceB.v[gl_GlobalInvocationID.x] = instances[gl_GlobalInvocationID.x].transformation;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();
}

// Check that it is not possible to create a compute shader when the context does not support ES
// 3.10
TEST_P(ComputeShaderTestES3, NotSupported)
{
    GLuint computeShaderHandle = glCreateShader(GL_COMPUTE_SHADER);
    EXPECT_EQ(0u, computeShaderHandle);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// The contents of shared variables should be cleared to zero at the beginning of shader execution.
TEST_P(WebGL2ComputeTest, sharedVariablesShouldBeZero)
{
    // http://anglebug.com/40644676
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // Fails on Android, AMD/windows and Intel/windows.  Probably works by chance on other
    // platforms, so suppressing on all platforms to avoid possible flakiness.
    // http://anglebug.com/42262513
    ANGLE_SKIP_TEST_IF(IsVulkan());

    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());
    ANGLE_SKIP_TEST_IF(IsOpenGL() &&
                       ((getClientMajorVersion() == 3) && (getClientMinorVersion() >= 1)));

    const char kCSShader[] = R"(#version 310 es
layout (local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
layout (r32ui, binding = 0) readonly uniform highp uimage2D srcImage;
layout (r32ui, binding = 1) writeonly uniform highp uimage2D dstImage;
struct S {
    float f;
    int i;
    uint u;
    bool b;
    vec4 v[64];
};

shared S vars[16];
void main()
{
    S zeroS;
    zeroS.f = 0.0f;
    zeroS.i = 0;
    zeroS.u = 0u;
    zeroS.b = false;
    for (int i = 0; i < 64; i++)
    {
        zeroS.v[i] = vec4(0.0f);
    }

    uint tid = gl_LocalInvocationID.x + gl_LocalInvocationID.y * 4u;
    uint value = (zeroS == vars[tid] ? 127u : 0u);
    imageStore(dstImage, ivec2(gl_LocalInvocationID.xy), uvec4(value));
})";

    const std::array<GLuint, 16> inputData = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    const std::array<GLuint, 16> expectedValues = {
        {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127}};
    runSharedMemoryTest<GLuint, 4, 4>(kCSShader, GL_R32UI, GL_UNSIGNED_INT, inputData,
                                      expectedValues);
}

// Test uniform dirty in compute shader, and verify the contents.
TEST_P(ComputeShaderTest, UniformDirty)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
uniform uint factor;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value * factor);
})";

    constexpr int kWidth = 1, kHeight = 1;
    constexpr GLuint kInputValues[2][1] = {{200}, {100}};

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues[0]);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues[1]);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glUniform1ui(glGetUniformLocation(program, "factor"), 2);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glUniform1ui(glGetUniformLocation(program, "factor"), 3);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth * kHeight];
    GLuint expectedValue = 600;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[1], 0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < kWidth * kHeight; i++)
    {
        EXPECT_EQ(expectedValue, outputValues[i]) << " index " << i;
    }
}

// Test storage buffer bound is unchanged, shader writes it, buffer content should be updated.
TEST_P(ComputeShaderTest, StorageBufferBoundUnchanged)
{
    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF(isSwiftshader());
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=16, local_size_y=16) in;
precision highp usampler2D;
uniform usampler2D tex;
uniform uint factor;
layout(std140, binding = 0) buffer buf {
    uint outData[16][16];
};

void main()
{
    uint x = gl_LocalInvocationID.x;
    uint y = gl_LocalInvocationID.y;
    float xCoord = float(x) / float(16);
    float yCoord = float(y) / float(16);
    outData[y][x] = texture(tex, vec2(xCoord, yCoord)).x + factor;
})";

    constexpr unsigned int kWidth  = 16;
    constexpr unsigned int kHeight = 16;
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
    GLuint texels[kHeight][kWidth] = {{0}};
    for (unsigned int y = 0; y < kHeight; ++y)
    {
        for (unsigned int x = 0; x < kWidth; ++x)
        {
            texels[y][x] = x + y * kWidth;
        }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    texels);
    glBindTexture(GL_TEXTURE_2D, 0);

    // The array stride are rounded up to the base alignment of a vec4 for std140 layout.
    constexpr unsigned int kArrayStride = 16;
    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kWidth * kHeight * kArrayStride, nullptr,
                 GL_STREAM_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glUniform1ui(glGetUniformLocation(program, "factor"), 2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    glDispatchCompute(1, 1, 1);

    const GLuint *ptr1 = reinterpret_cast<const GLuint *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, kWidth * kHeight * kArrayStride, GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();
    for (unsigned int idx = 0; idx < kWidth * kHeight; idx++)
    {
        EXPECT_EQ(idx + 2, *(ptr1 + idx * kArrayStride / 4));
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glUniform1ui(glGetUniformLocation(program, "factor"), 3);
    glDispatchCompute(1, 1, 1);

    const GLuint *ptr2 = reinterpret_cast<const GLuint *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, kWidth * kHeight * kArrayStride, GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();
    for (unsigned int idx = 0; idx < kWidth * kHeight; idx++)
    {
        EXPECT_EQ(idx + 3, *(ptr2 + idx * kArrayStride / 4));
    }
}

// Test imageSize to access mipmap slice.
TEST_P(ComputeShaderTest, ImageSizeMipmapSlice)
{
    // TODO(xinghua.cao@intel.com): http://anglebug.com/42261780
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux());

    // http://anglebug.com/42263018
    ANGLE_SKIP_TEST_IF(IsWindows() && IsNVIDIA() && IsD3D11());

    GLTexture texture[2];
    GLFramebuffer framebuffer;
    const char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(rgba32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    ivec2 size = imageSize(uImage_1);
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), uvec4(size, 0, 0));
})";

    constexpr int kWidth1 = 8, kHeight1 = 4, kWidth2 = 1, kHeight2 = 1;
    constexpr GLuint kInputValues[] = {0, 0, 0, 0};

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_R32UI, kWidth1, kHeight1);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, kWidth2, kHeight2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth2, kHeight2, GL_RGBA_INTEGER, GL_UNSIGNED_INT,
                    kInputValues);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 1, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues[kWidth2 * kHeight2 * 4];
    constexpr GLuint expectedValue[] = {4, 2};
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[1], 0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, kWidth2, kHeight2, GL_RGBA_INTEGER, GL_UNSIGNED_INT, outputValues);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < kWidth2 * kHeight2; i++)
    {
        EXPECT_EQ(expectedValue[i], outputValues[i]);
        EXPECT_EQ(expectedValue[i + 1], outputValues[i + 1]);
    }
}

// Test imageLoad to access mipmap slice.
TEST_P(ComputeShaderTest, ImageLoadMipmapSlice)
{
    // TODO(xinghua.cao@intel.com): http://anglebug.com/42261780
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux());

    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr int kWidth1 = 2, kHeight1 = 2, kWidth2 = 1, kHeight2 = 1;
    constexpr GLuint kInputValues11[] = {3, 3, 3, 3};
    constexpr GLuint kInputValues12[] = {2};
    constexpr GLuint kInputValues2[]  = {1};

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_R32UI, kWidth1, kHeight1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth1, kHeight1, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues11);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kWidth2, kHeight2, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues12);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth2, kHeight2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth2, kHeight2, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues2);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 1, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    glBindImageTexture(1, texture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues;
    constexpr GLuint expectedValue = 2;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[1], 0);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, kWidth2, kHeight2, GL_RED_INTEGER, GL_UNSIGNED_INT, &outputValues);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectedValue, outputValues);
}

// Test imageStore to access mipmap slice.
TEST_P(ComputeShaderTest, ImageStoreMipmapSlice)
{
    // TODO(xinghua.cao@intel.com): http://anglebug.com/42261780
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    GLTexture texture[2];
    GLFramebuffer framebuffer;
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";

    constexpr int kWidth1 = 1, kHeight1 = 1, kWidth2 = 2, kHeight2 = 2;
    constexpr GLuint kInputValues1[]  = {3};
    constexpr GLuint kInputValues21[] = {2, 2, 2, 2};
    constexpr GLuint kInputValues22[] = {1};

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth1, kHeight1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth1, kHeight1, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues1);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_R32UI, kWidth2, kHeight2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth2, kHeight2, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues21);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kWidth1, kHeight1, GL_RED_INTEGER, GL_UNSIGNED_INT,
                    kInputValues22);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(0, texture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    glBindImageTexture(1, texture[1], 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    GLuint outputValues;
    constexpr GLuint expectedValue = 3;
    glUseProgram(0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[1], 1);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, kWidth1, kHeight1, GL_RED_INTEGER, GL_UNSIGNED_INT, &outputValues);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectedValue, outputValues);
}

// Test that a resource is bound on render pipeline output, and then it's bound as the compute
// pipeline input. It works well. See http://anglebug.com/42262319
TEST_P(ComputeShaderTest, DrawTexture1DispatchTexture2)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_color_buffer_float"));

    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
precision highp sampler2D;
uniform sampler2D tex;
layout(rgba32f, binding = 0) writeonly uniform highp image2D image;
void main()
{
    vec4 value = texelFetch(tex, ivec2(gl_LocalInvocationID.xy), 0);
    imageStore(image, ivec2(gl_LocalInvocationID.xy), vec4(value.x - 1.0, 1.0, 0.0, value.w - 1.0));
})";

    const char kVSSource[] = R"(#version 310 es
layout (location = 0) in vec2 pos;
out vec2 texCoord;
void main(void) {
    texCoord = 0.5*pos + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFSSource[] = R"(#version 310 es
precision highp float;
uniform sampler2D tex;
in vec2 texCoord;
out vec4 fragColor;
void main(void) {
    fragColor = texture(tex, texCoord);
})";

    GLuint aPosLoc = 0;
    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glBindAttribLocation(program, aPosLoc, "pos");
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLfloat vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(aPosLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aPosLoc);

    constexpr GLfloat kInputValues[4] = {1.0, 0.0, 0.0, 1.0};
    constexpr GLfloat kZero[4]        = {0.0, 0.0, 0.0, 0.0};
    GLFramebuffer framebuffer;
    GLTexture texture[3];
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, kInputValues);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, kZero);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, kZero);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[1], 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    GLfloat actual[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, actual);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(1.0, actual[0]);
    EXPECT_EQ(0.0, actual[1]);
    EXPECT_EQ(0.0, actual[2]);
    EXPECT_EQ(1.0, actual[3]);

    glUseProgram(csProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glBindImageTexture(0, texture[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[2], 0);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, actual);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0.0, actual[0]);
    EXPECT_EQ(1.0, actual[1]);
    EXPECT_EQ(0.0, actual[2]);
    EXPECT_EQ(0.0, actual[3]);
}

// Test that render pipeline and compute pipeline access to the same texture.
// Steps:
//   1. DispatchCompute.
//   2. DrawArrays.
TEST_P(ComputeShaderTest, DispatchDraw)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32f, binding = 0) writeonly uniform highp image2D image;
void main()
{
    imageStore(image, ivec2(gl_LocalInvocationID.xy), vec4(0.0, 0.0, 1.0, 1.0));
})";

    const char kVSSource[] = R"(#version 310 es
layout (location = 0) in vec2 pos;
out vec2 texCoord;
void main(void) {
    texCoord = 0.5*pos + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFSSource[] = R"(#version 310 es
precision highp float;
uniform sampler2D tex;
in vec2 texCoord;
out vec4 fragColor;
void main(void) {
    fragColor = texture(tex, texCoord);
})";

    GLuint aPosLoc = 0;
    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    glBindAttribLocation(program, aPosLoc, "pos");
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLfloat vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(aPosLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aPosLoc);

    constexpr GLfloat kInputValues[4] = {1.0, 0.0, 0.0, 1.0};
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, kInputValues);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::blue);
}

// Test that render pipeline and compute pipeline access to the same texture.
// Steps:
//   1. DrawArrays.
//   2. DispatchCompute.
//   3. DispatchCompute.
//   4. DrawArrays.
TEST_P(ComputeShaderTest, DrawDispatchDispatchDraw)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32f, binding = 0) writeonly uniform highp image2D image;
uniform float factor;
void main()
{
    imageStore(image, ivec2(gl_LocalInvocationID.xy), vec4(factor, 0.0, 1.0, 1.0));
})";

    const char kVSSource[] = R"(#version 310 es
layout (location = 0) in vec2 pos;
out vec2 texCoord;
void main(void) {
    texCoord = 0.5*pos + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFSSource[] = R"(#version 310 es
precision highp float;
uniform sampler2D tex;
in vec2 texCoord;
out vec4 fragColor;
void main(void) {
    fragColor = texture(tex, texCoord);
})";

    GLuint aPosLoc = 0;
    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    glBindAttribLocation(program, aPosLoc, "pos");
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLfloat vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(aPosLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aPosLoc);

    constexpr GLfloat kInputValues[4] = {1.0, 0.0, 0.0, 1.0};
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, kInputValues);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);
    glUniform1f(glGetUniformLocation(csProgram, "factor"), 0.0);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    glUniform1f(glGetUniformLocation(csProgram, "factor"), 1.0);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::magenta);
}

// Test that render pipeline and compute pipeline access to the same texture.
// Steps:
//   1. DispatchCompute.
//   2. DrawArrays.
//   3. DrawArrays.
//   4. DispatchCompute.
TEST_P(ComputeShaderTest, DispatchDrawDrawDispatch)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32f, binding = 0) writeonly uniform highp image2D image;

void main()
{
    imageStore(image, ivec2(gl_LocalInvocationID.xy), vec4(0.0, 0.0, 1.0, 1.0));
})";

    const char kVSSource[] = R"(#version 310 es
layout (location = 0) in vec2 pos;
out vec2 texCoord;
void main(void) {
    texCoord = 0.5*pos + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFSSource[] = R"(#version 310 es
precision highp float;
uniform sampler2D tex;
in vec2 texCoord;
uniform float factor;
out vec4 fragColor;
void main(void) {
    fragColor = texture(tex, texCoord) + vec4(factor, 0.0, 0.0, 0.0);
})";

    GLuint aPosLoc = 0;
    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    glBindAttribLocation(program, aPosLoc, "pos");
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLfloat vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(aPosLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(aPosLoc);

    constexpr GLfloat kInputValues[4] = {1.0, 0.0, 0.0, 1.0};
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, kInputValues);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "factor"), 0.0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    glUniform1f(glGetUniformLocation(program, "factor"), 1.0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glUseProgram(csProgram);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "factor"), 0.0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::blue);
}

// Test color texture sample from fragment shader and then read access from compute
TEST_P(ComputeShaderTest, DrawReadDrawDispatch)
{
    const char kVS[] = R"(#version 310 es
layout (location = 0) in vec3 pos;
void main(void) {
    gl_Position = vec4(pos, 1.0);
})";

    const char kFS[] = R"(#version 310 es
precision highp float;
uniform sampler2D tex;
out vec4 fragColor;
void main(void) {
        fragColor = texture(tex,vec2(0,0));
})";

    // Create color texture
    GLTexture colorTexture;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
    const GLColor textureData = GLColor::green;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());

    // Render to surface with texture sample
    ANGLE_GL_PROGRAM(graphicsProgram, kVS, kFS);
    glUseProgram(graphicsProgram);
    const auto &quadVertices = GetQuadVertices();
    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vector3), quadVertices.data(),
                 GL_STATIC_DRAW);
    GLint positionAttributeLocation = 0;
    glBindAttribLocation(graphicsProgram, positionAttributeLocation, "pos");
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionAttributeLocation);

    GLint uTextureLocation = glGetUniformLocation(graphicsProgram, "tex");
    ASSERT_NE(-1, uTextureLocation);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glUniform1i(uTextureLocation, 0);
    // Sample the color texture from fragment shader and verify. This flushes out commands which
    // ensures there will be no layout transition in next renderPass.
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, textureData);

    // Sample the texture from fragment shader. No image layout transition expected here.
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Sample it from compute shader while the renderPass also sample from the same texture
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding=0) buffer buf {
    vec4 outData;
};
uniform sampler2D u_tex2D;
void main()
{
    outData = texture(u_tex2D, vec2(gl_LocalInvocationID.xy));
})";
    ANGLE_GL_COMPUTE_PROGRAM(computeProgram, kCS);
    glUseProgram(computeProgram);
    uTextureLocation = glGetUniformLocation(computeProgram, "u_tex2D");
    ASSERT_NE(-1, uTextureLocation);
    glUniform1i(uTextureLocation, 0);
    GLBuffer ssbo;
    const std::vector<GLfloat> initialData(4, 0.0f);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, initialData.size() * sizeof(GLfloat), initialData.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    const GLfloat *ptr      = reinterpret_cast<const GLfloat *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, initialData.size() * sizeof(GLfloat), GL_MAP_READ_BIT));
    angle::Vector4 expected = textureData.toNormalizedVector();
    EXPECT_NEAR(expected[0], ptr[0], 0.001);
    EXPECT_NEAR(expected[1], ptr[1], 0.001);
    EXPECT_NEAR(expected[2], ptr[2], 0.001);
    EXPECT_NEAR(expected[3], ptr[3], 0.001);
    EXPECT_GL_NO_ERROR();
}

// Test that invalid memory barrier will produce an error.
TEST_P(ComputeShaderTest, InvalidMemoryBarrier)
{
    GLbitfield barriers = 0;
    glMemoryBarrier(barriers);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// test atomic counter increment
// http://anglebug.com/42261924
TEST_P(ComputeShaderTest, AtomicCounterIncrement)
{
    constexpr char kComputeShader[] = R"(#version 310 es
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 1, std430) buffer Output {
  uint preGet[1];
  uint increment[1];
  uint postGet[1];
} sb_in;
layout(binding=0) uniform atomic_uint counter0;

void main(void)
{
  uint id = (gl_GlobalInvocationID.x);
  sb_in.preGet[0u]    = atomicCounter(counter0);
  sb_in.increment[0u] = atomicCounterIncrement(counter0);
  sb_in.postGet[0u]   = atomicCounter(counter0);
}
)";
    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent = sizeof(GLuint);

    GLBuffer shaderStorageBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * kBytesPerComponent, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer);
    EXPECT_GL_NO_ERROR();

    constexpr GLuint atomicBufferInitialData[] = {2u};
    GLuint atomicBuffer;
    glGenBuffers(1, &atomicBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(atomicBufferInitialData), atomicBufferInitialData,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicBuffer);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    glFinish();
    EXPECT_GL_NO_ERROR();

    // read back
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 3 * kBytesPerComponent, GL_MAP_READ_BIT));
    EXPECT_EQ(2u, ptr[0]);
    EXPECT_EQ(2u, ptr[1]);
    EXPECT_EQ(3u, ptr[2]);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Create a 'very large' array inside of a function in a compute shader.
TEST_P(ComputeShaderTest, VeryLargeArrayInsideFunction)
{
    constexpr char kComputeShader[] = R"(#version 310 es
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 0, std430) buffer Output {
  int value[1];
} output_data;

void main()
{
    int values[1000];
    for (int i = 0; i < values.length(); i++)
    {
        values[i] = 0;
    }

    int total = 0;
    for (int i = 0; i < values.length(); i++)
    {
        total += i;
        values[i] = total;
    }
    output_data.value[0u] = values[1000-1];
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent = sizeof(GLint);

    GLBuffer shaderStorageBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1 * kBytesPerComponent, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    glFinish();
    EXPECT_GL_NO_ERROR();

    // read back
    const GLint *ptr = reinterpret_cast<const GLint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 1 * kBytesPerComponent, GL_MAP_READ_BIT));
    EXPECT_EQ(499500, ptr[0]);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that render pipeline and compute pipeline access to the same texture.
// Steps:
//   1. Clear the texture and DrawArrays.
//   2. DispatchCompute to set the image's first pixel to a specific color.
//   3. DrawArrays and check data.
TEST_P(ComputeShaderTest, DrawDispatchDrawPreserve)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    const char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1) in;
layout(rgba8, binding = 0) writeonly uniform highp image2D image;
void main()
{
    imageStore(image, ivec2(0, 0), vec4(0.0, 0.0, 1.0, 1.0));
})";

    const char kVSSource[] = R"(#version 310 es
layout (location = 0) in vec2 pos;
in vec4 inTex;
out vec4 texCoord;
void main(void) {
    texCoord = inTex;
    gl_Position = vec4(pos, 0.0, 1.0);
})";

    const char kFSSource[] = R"(#version 310 es
precision highp float;
uniform sampler2D tex;
in vec4 texCoord;
out vec4 fragColor;
void main(void) {
    fragColor = texture(tex, texCoord.xy);
})";
    GLuint aPosLoc         = 0;
    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    glBindAttribLocation(program, aPosLoc, "pos");

    unsigned char *data = new unsigned char[4 * getWindowWidth() * getWindowHeight()];
    for (int i = 0; i < getWindowWidth() * getWindowHeight(); i++)
    {
        data[i * 4]     = 0xff;
        data[i * 4 + 1] = 0;
        data[i * 4 + 2] = 0;
        data[i * 4 + 3] = 0xff;
    }
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, getWindowWidth(), getWindowHeight());
    // Clear the texture level 0 to Red.
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA,
                    GL_UNSIGNED_BYTE, data);
    for (int i = 0; i < getWindowWidth() * getWindowHeight(); i++)
    {
        data[i * 4]     = 0;
        data[i * 4 + 1] = 0xff;
        data[i * 4 + 2] = 0;
        data[i * 4 + 3] = 0xff;
    }
    // Clear the texture level 1 to Green.
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, getWindowWidth() / 2, getWindowHeight() / 2, GL_RGBA,
                    GL_UNSIGNED_BYTE, data);
    delete[] data;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glUseProgram(program);
    GLfloat vertices[]  = {-1, -1, 1, -1, -1, 1, 1, 1};
    GLfloat texCoords[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    GLint pos           = glGetAttribLocation(program, "pos");
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    GLint posTex = glGetAttribLocation(program, "inTex");
    glEnableVertexAttribArray(posTex);
    glVertexAttribPointer(posTex, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

    // Draw with level 0, the whole framebuffer should be Red.
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);
    // Draw with level 1, a quarter of the framebuffer should be Green.
    glViewport(0, 0, getWindowWidth() / 2, getWindowHeight() / 2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2 - 1, getWindowHeight() / 2 - 1, GLColor::green);

    // Clear the texture level 0's (0, 0) position to Blue.
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();
    glFinish();

    glUseProgram(program);
    // Draw with level 0, the first position should be Blue.
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);
    // Draw with level 1, a quarter of the framebuffer should be Green.
    glViewport(0, 0, getWindowWidth() / 2, getWindowHeight() / 2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2 - 1, getWindowHeight() / 2 - 1, GLColor::green);
}

// Test that maxComputeWorkGroupCount is valid number.
TEST_P(ComputeShaderTest, ValidateMaxComputeWorkGroupCount)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";

    GLuint program = glCreateProgram();
    GLuint cs      = CompileShader(GL_COMPUTE_SHADER, kCS);
    EXPECT_NE(0u, cs);

    glAttachShader(program, cs);
    glDeleteShader(cs);

    GLint x, y, z;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &x);
    EXPECT_LE(65535, x);
    EXPECT_GE(std::numeric_limits<GLint>::max(), x);

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &y);
    EXPECT_LE(65535, y);
    EXPECT_GE(std::numeric_limits<GLint>::max(), y);

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &z);
    EXPECT_LE(65535, z);
    EXPECT_GE(std::numeric_limits<GLint>::max(), z);

    glDeleteProgram(program);
    EXPECT_GL_NO_ERROR();
}

// Validate that on Vulkan, compute pipeline driver uniforms descriptor set is updated after an
// internal compute-based UtilsVk function is used.  The latter is achieved through a draw with a
// vertex buffer whose format is not natively supported.  Atomic counters are used to make sure the
// compute pipeline uses the driver uniforms descriptor set.
TEST_P(ComputeShaderTest, DispatchConvertVertexDispatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_vertex_type_10_10_10_2"));

    constexpr uint32_t kVertexCount = 6;

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=6, local_size_y=1, local_size_z=1) in;

layout(binding = 0) uniform atomic_uint ac;

layout(binding=0, std140) buffer VertexData
{
    uint data[];
};

void main()
{
    atomicCounterIncrement(ac);
    data[gl_GlobalInvocationID.x] = gl_GlobalInvocationID.x;
})";

    constexpr char kVS[] = R"(#version 310 es
precision mediump float;

layout(location = 0) in vec4 position;
layout(location = 1) in uvec4 data;

out vec4 color;

void main() {
    color = data.x < 6u && data.y == 0u && data.z == 0u && data.w == 0u
        ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
in vec4 color;
out vec4 colorOut;
void main() {
    colorOut = color;
})";

    ANGLE_GL_COMPUTE_PROGRAM(programCS, kCS);
    ANGLE_GL_PROGRAM(programVSFS, kVS, kFS);
    EXPECT_GL_NO_ERROR();

    // Create atomic counter buffer
    GLBuffer atomicCounterBuffer;
    constexpr GLuint kInitialAcbData = 0;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(kInitialAcbData), &kInitialAcbData,
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
    EXPECT_GL_NO_ERROR();

    // Create vertex buffer
    constexpr unsigned kVertexBufferInitData[kVertexCount] = {};
    GLBuffer vertexBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kVertexBufferInitData), kVertexBufferInitData,
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexBuffer);
    EXPECT_GL_NO_ERROR();

    // Create position buffer
    constexpr GLfloat positions[kVertexCount * 2] = {1.0, 1.0, -1.0, 1.0,  -1.0, -1.0,
                                                     1.0, 1.0, -1.0, -1.0, 1.0,  -1.0};
    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Create vertex array
    GLVertexArray vao;
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_INT_10_10_10_2_OES, false, 0, 0);
    EXPECT_GL_NO_ERROR();

    // Fill the vertex buffer with a dispatch call
    glUseProgram(programCS);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    // Draw using the vertex buffer, causing vertex format conversion in compute (in the Vulkan
    // backend)
    glUseProgram(programVSFS);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, kVertexCount);
    EXPECT_GL_NO_ERROR();

    // Issue another dispatch call. The driver uniforms descriptor set must be rebound.
    glUseProgram(programCS);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // Verify that the atomic counter has the expected value.
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    GLuint *mappedBuffer = static_cast<GLuint *>(
        glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT));
    EXPECT_EQ(kVertexCount * 2, mappedBuffer[0]);
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}

// Validate that on Vulkan, compute pipeline is correctly bound after an internal dispatch call is
// made.  Blit stencil may issue a dispatch call.
TEST_P(ComputeShaderTest, DispatchBlitStencilDispatch)
{
    // http://anglebug.com/42264069
    ANGLE_SKIP_TEST_IF(IsQualcomm() && IsOpenGLES());

    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    constexpr GLsizei kSize = 1;

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=6, local_size_y=1, local_size_z=1) in;

uniform vec4 data;

layout(rgba8, binding = 0) writeonly uniform highp image2D image;

void main()
{
    imageStore(image, ivec2(gl_LocalInvocationID.xy), data);
})";

    ANGLE_GL_COMPUTE_PROGRAM(programCS, kCS);
    EXPECT_GL_NO_ERROR();

    // Create a framebuffer with stencil buffer.  Use multisampled textures so the future blit
    // cannot use vkCmdBlitImage.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, true);

    GLTexture depthStencil;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthStencil);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH24_STENCIL8, kSize, kSize,
                              true);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, color,
                           0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE,
                           depthStencil, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Clear the stencil and make sure it's done.
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    GLTexture colorCopy;
    glBindTexture(GL_TEXTURE_2D, colorCopy);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);

    GLFramebuffer copyFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, copyFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorCopy, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, copyFbo);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Setup image for compute call
    GLTexture computeOut;
    glBindTexture(GL_TEXTURE_2D, computeOut);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glBindImageTexture(0, computeOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    ASSERT_GL_NO_ERROR();

    // Issue a dispatch call.
    glUseProgram(programCS);
    GLint uniformLoc = glGetUniformLocation(programCS, "data");
    ASSERT_NE(uniformLoc, -1);

    glUniform4f(uniformLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    ASSERT_GL_NO_ERROR();

    // Blit the stencil texture.  This may use a compute shader internally.
    GLTexture depthStencilCopy;
    glBindTexture(GL_TEXTURE_2D, depthStencilCopy);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, kSize, kSize);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, copyFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           depthStencilCopy, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Issue another dispatch call.
    glUniform4f(uniformLoc, 0.0f, 1.0f, 0.0f, 1.0f);
    glDispatchCompute(1, 1, 1);
    ASSERT_GL_NO_ERROR();

    // Verify the results.
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, copyFbo);
    glBindTexture(GL_TEXTURE_2D, computeOut);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, computeOut, 0);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Verify the blit copy results.
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Validate that on Vulkan, compute pipeline is correctly bound after an internal dispatch call is
// made.  Generate mipmap may issue a dispatch call.
TEST_P(ComputeShaderTest, DispatchGenerateMipmapDispatch)
{
    constexpr GLsizei kSize = 8;

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=6, local_size_y=1, local_size_z=1) in;

uniform vec4 data;

layout(rgba8, binding = 0) writeonly uniform highp image2D image;

void main()
{
    imageStore(image, ivec2(gl_LocalInvocationID.xy), data);
})";

    ANGLE_GL_COMPUTE_PROGRAM(programCS, kCS);
    EXPECT_GL_NO_ERROR();

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, kSize, kSize);

    const std::vector<GLColor> kInitialColor(kSize * kSize, GLColor::green);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE,
                    kInitialColor.data());

    // Setup image for compute call
    GLTexture computeOut;
    glBindTexture(GL_TEXTURE_2D, computeOut);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glBindImageTexture(0, computeOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    ASSERT_GL_NO_ERROR();

    // Issue a dispatch call.
    glUseProgram(programCS);
    GLint uniformLoc = glGetUniformLocation(programCS, "data");
    ASSERT_NE(uniformLoc, -1);

    glUniform4f(uniformLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    ASSERT_GL_NO_ERROR();

    // Generate mipmap on the texture.  This may use a compute shader internally.
    glBindTexture(GL_TEXTURE_2D, color);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Issue another dispatch call.
    glUniform4f(uniformLoc, 0.0f, 1.0f, 0.0f, 1.0f);
    glDispatchCompute(1, 1, 1);
    ASSERT_GL_NO_ERROR();

    // Verify the results.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, computeOut);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, computeOut, 0);
    ASSERT_GL_NO_ERROR();

    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Write to image array with an aliasing format.
TEST_P(ComputeShaderTest, AliasingFormatForImageArray)
{
    // http://anglebug.com/42263894
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=2) in;
layout(r32ui, binding = 0) writeonly uniform highp uimage2DArray image;
void main()
{
    uint yellow = 0xFF00FFFFu;
    imageStore(image, ivec3(gl_LocalInvocationID.xyz), uvec4(yellow, 0, 0, 0));
})";

    constexpr int kWidth = 1, kHeight = 1, kDepth = 2;

    const std::vector<GLColor> kInitData(kWidth * kHeight * kDepth, GLColor::black);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RGBA,
                    GL_UNSIGNED_BYTE, kInitData.data());
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    // Output yellow to both layers.
    glBindImageTexture(0, texture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Verify results.
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, 0);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texture, 0, 1);
    EXPECT_GL_NO_ERROR();

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
}

// Write to one layer of image array with an aliasing format.
TEST_P(ComputeShaderTest, AliasingFormatForOneLayerOfImageArray)
{
    // http://anglebug.com/42263894
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) writeonly uniform highp uimage2D image;
void main()
{
    uint yellow = 0xFF00FFFFu;
    imageStore(image, ivec2(gl_LocalInvocationID.xy), uvec4(yellow, 0, 0, 0));
})";

    constexpr int kWidth = 1, kHeight = 1, kDepth = 2;

    const std::vector<GLColor> kInitData(kWidth * kHeight * kDepth, GLColor::black);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, kWidth, kHeight, kDepth);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RGBA,
                    GL_UNSIGNED_BYTE, kInitData.data());
    EXPECT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, 0);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texture, 0, 1);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    // Output yellow to layer 0.
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Verify that only layer 0 was changed.
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Reset texture back to black.
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kWidth, kHeight, kDepth, GL_RGBA,
                    GL_UNSIGNED_BYTE, kInitData.data());

    // Output yellow to layer 1.
    glBindImageTexture(0, texture, 0, GL_FALSE, 1, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Verify that only layer 1 was changed.
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
}

// Test glMemoryBarrier(CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT) by writing to persistenly mapped
// buffer from a compute shader.
TEST_P(ComputeShaderTest, WriteToPersistentBuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding = 0) buffer block {
    uvec4 data;
} outBlock;
void main()
{
    outBlock.data += uvec4(1);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    constexpr std::array<uint32_t, 4> kInitData = {};

    GLBuffer coherentBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, coherentBuffer);
    glBufferStorageEXT(
        GL_SHADER_STORAGE_BUFFER, sizeof(kInitData), kInitData.data(),
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

    GLBuffer nonCoherentBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nonCoherentBuffer);
    glBufferStorageEXT(GL_SHADER_STORAGE_BUFFER, sizeof(kInitData), kInitData.data(),
                       GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT);

    // Map the buffers for read and write.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, coherentBuffer);
    uint32_t *coherentMapped = reinterpret_cast<uint32_t *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, sizeof(kInitData),
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT));
    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nonCoherentBuffer);
    uint32_t *nonCoherentMapped = reinterpret_cast<uint32_t *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kInitData),
                         GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT));
    ASSERT_GL_NO_ERROR();

    constexpr std::array<uint32_t, 4> kCoherentExpectedData = {
        0x12354678u,
        0x2468ACE0u,
        0x13579BDFu,
        0x76543210u,
    };

    constexpr std::array<uint32_t, 4> kNonCoherentExpectedData = {
        0x9ABCDEF0u,
        0xFDB97531u,
        0x1F2E3D4Bu,
        0x5A697887u,
    };

    coherentMapped[0] = kCoherentExpectedData[0] - 1;
    coherentMapped[1] = kCoherentExpectedData[1] - 1;
    coherentMapped[2] = kCoherentExpectedData[2] - 1;
    coherentMapped[3] = kCoherentExpectedData[3] - 1;

    nonCoherentMapped[0] = kNonCoherentExpectedData[0] - 1;
    nonCoherentMapped[1] = kNonCoherentExpectedData[1] - 1;
    nonCoherentMapped[2] = kNonCoherentExpectedData[2] - 1;
    nonCoherentMapped[3] = kNonCoherentExpectedData[3] - 1;

    // Test coherent write
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, coherentBuffer);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glFinish();
    EXPECT_EQ(coherentMapped[0], kCoherentExpectedData[0]);
    EXPECT_EQ(coherentMapped[1], kCoherentExpectedData[1]);
    EXPECT_EQ(coherentMapped[2], kCoherentExpectedData[2]);
    EXPECT_EQ(coherentMapped[3], kCoherentExpectedData[3]);

    // Test non-coherent write
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nonCoherentBuffer);
    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT);
    EXPECT_GL_NO_ERROR();

    glFinish();
    EXPECT_EQ(nonCoherentMapped[0], kNonCoherentExpectedData[0]);
    EXPECT_EQ(nonCoherentMapped[1], kNonCoherentExpectedData[1]);
    EXPECT_EQ(nonCoherentMapped[2], kNonCoherentExpectedData[2]);
    EXPECT_EQ(nonCoherentMapped[3], kNonCoherentExpectedData[3]);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, coherentBuffer);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nonCoherentBuffer);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Verify the CS doesn't overwrite the mapped buffer data.
TEST_P(ComputeShaderTest, ImageBufferMapWrite)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_buffer"));

    // Claims to support GL_OES_texture_buffer, but fails compilation of shader because "extension
    // 'GL_OES_texture_buffer' is not supported".  http://anglebug.com/42264369
    ANGLE_SKIP_TEST_IF(IsQualcomm() && IsOpenGLES());

    constexpr char kComputeImageBuffer[] = R"(#version 310 es
#extension GL_OES_texture_buffer : require
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32f, binding = 0) uniform highp writeonly imageBuffer dst;
uniform vec4 uniformData;
void main()
{
    imageStore(dst, int(gl_GlobalInvocationID.x), uniformData);
})";

    GLProgram program;
    program.makeCompute(kComputeImageBuffer);
    glUseProgram(program);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {1.0, 0.0, 0.0, 1.0};

    glBindBuffer(GL_TEXTURE_BUFFER, textureBufferStorage);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(kInitData), kInitData.data(), GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_BUFFER, texture);
    glTexBufferEXT(GL_TEXTURE_BUFFER, GL_RGBA32F, textureBufferStorage);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    EXPECT_GL_NO_ERROR();

    constexpr std::array<float, 4> kComputeShaderData = {0.0, 1.0, 0.0, 1.0};
    GLint uniformLocation = glGetUniformLocation(program, "uniformData");
    ASSERT_NE(uniformLocation, -1);
    glUniform4f(uniformLocation, kComputeShaderData[0], kComputeShaderData[1],
                kComputeShaderData[2], kComputeShaderData[3]);
    EXPECT_GL_NO_ERROR();

    // Write to the buffer with the CS.
    glDispatchCompute(1, 1, 1);

    // Issue the appropriate memory barrier before mapping the buffer.
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // Map the buffer and write to it.
    constexpr std::array<float, 4> kMapData = {0.0, 0.0, 1.0, 1.0};
    void *mappedBuffer =
        glMapBufferRange(GL_TEXTURE_BUFFER, 0, sizeof(GLuint) * 3, GL_MAP_WRITE_BIT);
    memcpy(mappedBuffer, kMapData.data(), sizeof(kMapData));
    glUnmapBuffer(GL_TEXTURE_BUFFER);

    glFinish();

    // Read back and verify buffer data.
    std::array<float, 4> bufferData = {0};
    mappedBuffer = glMapBufferRange(GL_TEXTURE_BUFFER, 0, sizeof(GLuint) * 3, GL_MAP_READ_BIT);
    memcpy(bufferData.data(), mappedBuffer, sizeof(bufferData));
    glUnmapBuffer(GL_TEXTURE_BUFFER);

    EXPECT_EQ(bufferData[0], kMapData[0]);
    EXPECT_EQ(bufferData[1], kMapData[1]);
    EXPECT_EQ(bufferData[2], kMapData[2]);
    EXPECT_EQ(bufferData[3], kMapData[3]);
}

// Test compute shader write to texture buffer followed by texSubData and followed by compute shader
// write to texture buffer again.
TEST_P(ComputeShaderTest, ImageBufferMapWriteAndBufferSubData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_buffer"));

    // Claims to support GL_OES_texture_buffer, but fails compilation of shader because "extension
    // 'GL_OES_texture_buffer' is not supported".  http://anglebug.com/42264369
    ANGLE_SKIP_TEST_IF(IsQualcomm() && IsOpenGLES());

    // angleporject:6545. Known bug.
    ANGLE_SKIP_TEST_IF(IsVulkan());

    constexpr char kComputeImageBuffer[] = R"(#version 310 es
#extension GL_OES_texture_buffer : require
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32f, binding = 0) uniform highp writeonly imageBuffer dst;
uniform vec4 uniformData;
uniform int uniformOffset;
void main()
{
    imageStore(dst, uniformOffset, uniformData);
})";

    GLProgram program;
    program.makeCompute(kComputeImageBuffer);
    glUseProgram(program);

    for (int loop = 0; loop < 2; loop++)
    {
        GLBuffer textureBufferStorage;
        GLTexture texture;
        constexpr unsigned int kShaderUsedSize    = sizeof(float) * 4;
        constexpr unsigned int kMiddlePaddingSize = 1024;
        constexpr unsigned int kBufferSize = kShaderUsedSize + kMiddlePaddingSize + kShaderUsedSize;
        constexpr unsigned int kOffset0    = 0;
        constexpr unsigned int kOffset1    = kShaderUsedSize;
        constexpr unsigned int kOffset2    = kShaderUsedSize + kMiddlePaddingSize;

        glBindBuffer(GL_TEXTURE_BUFFER, textureBufferStorage);
        glBufferData(GL_TEXTURE_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);

        glBindTexture(GL_TEXTURE_BUFFER, texture);
        glTexBufferEXT(GL_TEXTURE_BUFFER, GL_RGBA32F, textureBufferStorage);
        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        // Write to the buffer with the CS.
        constexpr std::array<float, 4> kComputeShaderData1 = {0.0, 1.0, 0.0, 1.0};
        GLint uniformDataLocation = glGetUniformLocation(program, "uniformData");
        ASSERT_NE(uniformDataLocation, -1);
        glUniform4f(uniformDataLocation, kComputeShaderData1[0], kComputeShaderData1[1],
                    kComputeShaderData1[2], kComputeShaderData1[3]);
        GLint uniformOffsetLocation = glGetUniformLocation(program, "uniformOffset");
        ASSERT_NE(uniformOffsetLocation, -1);
        glUniform1i(uniformOffsetLocation, kOffset0 / (sizeof(float) * 4));
        glDispatchCompute(1, 1, 1);
        // Issue the appropriate memory barrier before mapping the buffer.
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Write to the buffer with the CS.
        constexpr std::array<float, 4> kComputeShaderData2 = {1.0, 0.0, 1.0, 1.0};
        glUniform4f(uniformDataLocation, kComputeShaderData2[0], kComputeShaderData2[1],
                    kComputeShaderData2[2], kComputeShaderData2[3]);
        glUniform1i(uniformOffsetLocation, kOffset2 / (sizeof(float) * 4));
        glDispatchCompute(1, 1, 1);

        if (loop == 1)
        {
            // Make write operation finished but read operation pending. We don't care actual
            // rendering result but just to have a unflushed rendering using the buffer so that it
            // will appears as pending.
            glFinish();
            constexpr char kVS[] = R"(attribute vec4 in_attrib;
                                    varying vec4 v_attrib;
                                    void main()
                                    {
                                        v_attrib = in_attrib;
                                        gl_Position = vec4(0.0, 0.0, 0.5, 1.0);
                                        gl_PointSize = 100.0;
                                    })";
            constexpr char kFS[] = R"(precision mediump float;
                                    varying vec4 v_attrib;
                                    void main()
                                    {
                                        gl_FragColor = v_attrib;
                                    })";
            GLuint readProgram   = CompileProgram(kVS, kFS);
            ASSERT_NE(readProgram, 0U);
            GLint attribLocation = glGetAttribLocation(readProgram, "in_attrib");
            ASSERT_NE(attribLocation, -1);
            glUseProgram(readProgram);
            ASSERT_GL_NO_ERROR();
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);
            glBindBuffer(GL_ARRAY_BUFFER, textureBufferStorage);
            glVertexAttribPointer(attribLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4, nullptr);
            glEnableVertexAttribArray(attribLocation);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textureBufferStorage);
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, nullptr);
            ASSERT_GL_NO_ERROR();
        }

        // Use subData to update middle portion of data to trigger acquireAndUpdate code path in
        // ANGLE
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
        glBindBuffer(GL_TEXTURE_BUFFER, textureBufferStorage);
        constexpr unsigned int kMiddlePaddingValue = 0x55555555u;
        std::vector<unsigned int> kPaddingValues(kMiddlePaddingSize / sizeof(unsigned int),
                                                 kMiddlePaddingValue);
        glBufferSubData(GL_TEXTURE_BUFFER, kOffset1, kMiddlePaddingSize, kPaddingValues.data());

        // Read back and verify buffer data.
        const GLbyte *mappedBuffer = reinterpret_cast<const GLbyte *>(
            glMapBufferRange(GL_TEXTURE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT));

        const GLfloat *ptr0 = reinterpret_cast<const GLfloat *>(mappedBuffer);
        EXPECT_EQ(ptr0[0], kComputeShaderData1[0]);
        EXPECT_EQ(ptr0[1], kComputeShaderData1[1]);
        EXPECT_EQ(ptr0[2], kComputeShaderData1[2]);
        EXPECT_EQ(ptr0[3], kComputeShaderData1[3]);

        const GLuint *ptr1 = reinterpret_cast<const GLuint *>(mappedBuffer + kOffset1);
        for (unsigned int idx = 0; idx < kMiddlePaddingSize / sizeof(unsigned int); idx++)
        {
            EXPECT_EQ(ptr1[idx], kMiddlePaddingValue);
        }

        const GLfloat *ptr2 = reinterpret_cast<const GLfloat *>(mappedBuffer + kOffset2);
        EXPECT_EQ(ptr2[0], kComputeShaderData2[0]);
        EXPECT_EQ(ptr2[1], kComputeShaderData2[1]);
        EXPECT_EQ(ptr2[2], kComputeShaderData2[2]);
        EXPECT_EQ(ptr2[3], kComputeShaderData2[3]);

        glUnmapBuffer(GL_TEXTURE_BUFFER);
        EXPECT_GL_NO_ERROR();
    }
}

// Test one texture sampled by fragment shader, then bind it to image, followed by compute
// shader load this image, and fragment shader read it again.
TEST_P(ComputeShaderTest, DrawDispatchImageReadDraw)
{

    constexpr char kVSSource[] = R"(#version 310 es
in vec4 a_position;
out vec2 v_texCoord;

void main()
{
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
    v_texCoord = a_position.xy * 0.5 + vec2(0.5);
})";

    constexpr char kFSSource[] = R"(#version 310 es
precision mediump float;
uniform sampler2D u_tex2D;
in vec2 v_texCoord;
out vec4 out_FragColor;
void main()
{
    out_FragColor = texture(u_tex2D, v_texCoord);
})";

    constexpr char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32f, binding=0) readonly  uniform highp image2D uIn;
layout(std140, binding=0) buffer buf {
    vec4 outData;
};

void main()
{
    outData = imageLoad(uIn, ivec2(gl_LocalInvocationID.xy));
})";

    GLfloat initValue[4] = {1.0, 1.0, 1.0, 1.0};

    // Step 1: Set up a simple 2D Texture rendering loop.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, initValue);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLBuffer vertexBuffer;
    GLfloat vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 16, nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, "a_position");
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Step 2: load this image through compute
    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    const GLfloat *ptr = reinterpret_cast<const GLfloat *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 16, GL_MAP_READ_BIT));

    EXPECT_GL_NO_ERROR();
    for (unsigned int idx = 0; idx < 4; idx++)
    {
        EXPECT_EQ(1.0, *(ptr + idx));
    }

    // Step3: use the first program sample texture again
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::white);
}

// Test fragment shader read a image, followed by compute shader sample it.
TEST_P(ComputeShaderTest, FSReadImageThenCSSample)
{
    GLint maxFragmentImageUniforms = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &maxFragmentImageUniforms);

    // MAX_FRAGMENT_IMAGE_UNIFORMS can be 0 according to OpenGL ES 3.1 SPEC.
    ANGLE_SKIP_TEST_IF(maxFragmentImageUniforms == 0);

    constexpr char kVSSource[] = R"(#version 310 es
in vec4 a_position;
out vec2 v_texCoord;
void main()
{
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
    v_texCoord = a_position.xy * 0.5 + vec2(0.5);;
})";

    constexpr char kFSSource[] = R"(#version 310 es
precision mediump float;
layout(rgba32f, binding=0) readonly  uniform highp image2D uIn;
in vec2 v_texCoord;
out vec4 out_FragColor;

void main()
{
    out_FragColor = imageLoad(uIn, ivec2(v_texCoord));
})";

    constexpr char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding=0) buffer buf {
    vec4 outData;
};
uniform sampler2D u_tex2D;
void main()
{
    outData = texture(u_tex2D, vec2(gl_LocalInvocationID.xy));
})";

    GLfloat initValue[4] = {1.0, 1.0, 1.0, 1.0};

    // Step 1: Set up a simple 2D Texture rendering loop.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, initValue);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLBuffer vertexBuffer;
    GLfloat vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 16, nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, "a_position");
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL_NO_ERROR();

    // Step 2: load this image through compute
    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    glDispatchCompute(1, 1, 1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    const GLfloat *ptr = reinterpret_cast<const GLfloat *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 16, GL_MAP_READ_BIT));

    EXPECT_GL_NO_ERROR();
    for (unsigned int idx = 0; idx < 4; idx++)
    {
        EXPECT_EQ(1.0, *(ptr + idx));
    }
}

// Replicate the dEQP test dEQP-GLES31.functional.synchronization.in_invocation.ssbo_alias_overwrite
TEST_P(ComputeShaderTest, SSBOAliasOverWrite)
{
    constexpr char kCSSource[] = R"(#version 310 es
    layout (local_size_x=16, local_size_y=8) in;
    layout(binding=0, std430) buffer Output {
        highp int values[];
    } sb_result;
    layout(binding=1, std430) coherent buffer Storage0
    {
        highp int values[];
    } sb_store0;
    layout(binding=2, std430) coherent buffer Storage1
    {
        highp int values[];
    } sb_store1;

    highp int getIndex(in highp uvec2 localID, in highp int element)
    {
        highp uint groupNdx = gl_NumWorkGroups.x * gl_WorkGroupID.y + gl_WorkGroupID.x;
        return int((localID.y * gl_NumWorkGroups.x * gl_NumWorkGroups.y * gl_WorkGroupSize.x) + (groupNdx * gl_WorkGroupSize.x) + localID.x) * 8 + element;
    }

    void main (void)
    {
        int resultNdx = int(gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x + gl_GlobalInvocationID.x);
        int groupNdx = int(gl_NumWorkGroups.x * gl_WorkGroupID.y + gl_WorkGroupID.x);
        bool allOk = true;

        sb_store0.values[getIndex(gl_LocalInvocationID.xy, 0)] = 456;
        sb_store0.values[getIndex(gl_LocalInvocationID.xy, 1)] = 456;
        sb_store0.values[getIndex(gl_LocalInvocationID.xy, 2)] = 456;
        sb_store0.values[getIndex(gl_LocalInvocationID.xy, 3)] = 456;
        sb_store0.values[getIndex(gl_LocalInvocationID.xy, 4)] = 456;
        sb_store0.values[getIndex(gl_LocalInvocationID.xy, 5)] = 456;
        sb_store0.values[getIndex(gl_LocalInvocationID.xy, 6)] = 456;
        sb_store0.values[getIndex(gl_LocalInvocationID.xy, 7)] = 456;

        sb_store1.values[getIndex(gl_LocalInvocationID.xy, 0)] = groupNdx;
        sb_store1.values[getIndex(gl_LocalInvocationID.xy, 1)] = groupNdx;
        sb_store1.values[getIndex(gl_LocalInvocationID.xy, 2)] = groupNdx;
        sb_store1.values[getIndex(gl_LocalInvocationID.xy, 3)] = groupNdx;
        sb_store1.values[getIndex(gl_LocalInvocationID.xy, 4)] = groupNdx;
        sb_store1.values[getIndex(gl_LocalInvocationID.xy, 5)] = groupNdx;
        sb_store1.values[getIndex(gl_LocalInvocationID.xy, 6)] = groupNdx;
        sb_store1.values[getIndex(gl_LocalInvocationID.xy, 7)] = groupNdx;

        allOk = allOk && (sb_store0.values[getIndex(gl_LocalInvocationID.xy, 0)] == groupNdx);
        allOk = allOk && (sb_store0.values[getIndex(gl_LocalInvocationID.xy, 1)] == groupNdx);
        allOk = allOk && (sb_store0.values[getIndex(gl_LocalInvocationID.xy, 2)] == groupNdx);
        allOk = allOk && (sb_store0.values[getIndex(gl_LocalInvocationID.xy, 3)] == groupNdx);
        allOk = allOk && (sb_store0.values[getIndex(gl_LocalInvocationID.xy, 4)] == groupNdx);
        allOk = allOk && (sb_store0.values[getIndex(gl_LocalInvocationID.xy, 5)] == groupNdx);
        allOk = allOk && (sb_store0.values[getIndex(gl_LocalInvocationID.xy, 6)] == groupNdx);
        allOk = allOk && (sb_store0.values[getIndex(gl_LocalInvocationID.xy, 7)] == groupNdx);

        sb_result.values[resultNdx] = allOk ? (1) : (2);

    })";

    const int totalWorkWidth        = 256;
    const int totalWorkHeight       = 256;
    const int elementsPerInvocation = 8;

    // define compute shader input storage buffer
    const int inputSSBOBufferSizeInBytes =
        totalWorkWidth * totalWorkHeight * elementsPerInvocation * sizeof(uint32_t);
    const int inputSSBOBufferElementsCount =
        totalWorkWidth * totalWorkHeight * elementsPerInvocation;
    std::vector<uint32_t> zeros(inputSSBOBufferElementsCount, 0);
    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, inputSSBOBufferSizeInBytes, zeros.data(),
                 GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // define compute shader output buffer
    const int outputBufferSizeInBytes   = totalWorkWidth * totalWorkHeight * sizeof(int32_t);
    const int outputBufferElementsCount = totalWorkWidth * totalWorkHeight;
    std::vector<int32_t> minusOnes(outputBufferElementsCount, -1);
    GLBuffer resultBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, resultBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, outputBufferSizeInBytes, &minusOnes[0], GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // dispatch compute shader
    const int localWidth  = 16;
    const int localHeight = 8;
    ASSERT(totalWorkWidth % localWidth == 0);
    ASSERT(totalWorkHeight % localHeight == 0);
    const int numGroupDimX = totalWorkWidth / localWidth;
    const int numGroupDimY = totalWorkHeight / localHeight;

    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);
    ASSERT_GL_NO_ERROR();

    // Bind storage buffer to compute shader binding locations
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, resultBuffer);

    glDispatchCompute(numGroupDimX, numGroupDimY, 1);
    ASSERT_GL_NO_ERROR();

    // verify the result
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, resultBuffer);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    void *mappedResults =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, outputBufferSizeInBytes, GL_MAP_READ_BIT);
    std::vector<int32_t> results(outputBufferElementsCount);
    memcpy(results.data(), mappedResults, outputBufferSizeInBytes);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ASSERT_GL_NO_ERROR();

    bool error = false;
    for (int index = 0; index < static_cast<int>(results.size()); ++index)
    {
        if (results[index] != 1)
        {
            error = true;
        }
    }
    EXPECT_EQ(false, error);
}

// Performs an atomic operation and assigns the previous value to an SSBO.
TEST_P(ComputeShaderTest, AtomicOpPreviousValueAssignedToSSBO)
{

    constexpr char kCSSource[] = R"(#version 310 es
    shared int wg;
    layout(binding = 0, std430) buffer Storage0 {
      int inner[16];
    } buf;

    layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    void main() {
      wg = 0;
      atomicExchange(wg, 0);
      barrier();
      buf.inner[gl_WorkGroupID.x] = atomicOr(wg, 1);
    })";

    const int dispatchSize = 16;

    // define compute shader output buffer
    const int outputBufferSizeInBytes   = dispatchSize * sizeof(int32_t);
    const int outputBufferElementsCount = dispatchSize;
    std::vector<int32_t> minusOnes(outputBufferElementsCount, -1);
    GLBuffer resultBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, resultBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, outputBufferSizeInBytes, &minusOnes[0], GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);
    ASSERT_GL_NO_ERROR();

    // Bind storage buffer to compute shader binding locations
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, resultBuffer);

    glDispatchCompute(dispatchSize, 1, 1);
    ASSERT_GL_NO_ERROR();

    // verify the result
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, resultBuffer);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    void *mappedResults =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, outputBufferSizeInBytes, GL_MAP_READ_BIT);
    std::vector<int32_t> results(outputBufferElementsCount);
    memcpy(results.data(), mappedResults, outputBufferSizeInBytes);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ASSERT_GL_NO_ERROR();

    for (int index = 0; index < static_cast<int>(results.size()); ++index)
    {
        EXPECT_EQ(results[index], 0);
    }
}

class StorageImageRenderProgramTest : public ANGLETest<>
{};

// Test creating a program with a vertex shader using storage image.
TEST_P(StorageImageRenderProgramTest, StorageImageInVertexShader)
{
    GLint maxVertexShaderImage = 0;
    glGetIntegerv(GL_MAX_VERTEX_IMAGE_UNIFORMS, &maxVertexShaderImage);

    // MAX_VERTEX_IMAGE_UNIFORMS can be 0 according to OpenGL ES 3.1 SPEC.
    ANGLE_SKIP_TEST_IF(maxVertexShaderImage == 0);

    constexpr char kVSSource_readonly[] = R"(#version 310 es
layout(rgba32f, binding = 0) readonly uniform highp image2D uIn;
void main()
{
    gl_Position = imageLoad(uIn, ivec2(0, 0));
})";

    constexpr char kVSSource_writeonly[] = R"(#version 310 es
layout(rgba32f, binding = 0) writeonly uniform highp image2D uOut;
void main()
{
    gl_Position = vec4(0, 0, 0, 1);
    imageStore(uOut, ivec2(0, 0), vec4(0, 0, 0, 1));
})";

    constexpr char kVSSource_readwrite[] = R"(#version 310 es
layout(r32f, binding = 0) uniform highp image2D uImage;
void main()
{
    gl_Position = imageLoad(uImage, ivec2(0, 0));
    imageStore(uImage, ivec2(0, 0), vec4(0, 0, 0, 1));
})";

    constexpr char kFSSource[] = R"(#version 310 es
precision mediump float;
out vec4 out_FragColor;
void main()
{
    out_FragColor = vec4(0, 1, 0, 1);
})";
    ANGLE_GL_PROGRAM(program1, kVSSource_readonly, kFSSource);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(program2, kVSSource_writeonly, kFSSource);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(program3, kVSSource_readwrite, kFSSource);
    EXPECT_GL_NO_ERROR();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ComputeShaderTest);
ANGLE_INSTANTIATE_TEST_ES31(ComputeShaderTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ComputeShaderTestES3);
ANGLE_INSTANTIATE_TEST_ES3(ComputeShaderTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(WebGL2ComputeTest);
ANGLE_INSTANTIATE_TEST_ES31(WebGL2ComputeTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(StorageImageRenderProgramTest);
ANGLE_INSTANTIATE_TEST_ES31(StorageImageRenderProgramTest);
}  // namespace
