//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryBarrierTest:
//   Ensure that implementation of glMemoryBarrier is correct both in terms of memory barriers
//   issued and potential reordering of commands.
//
// The barrier bits accepted by glMemoryBarrier are used for synchronization as such:
//
//     VERTEX_ATTRIB_ARRAY_BARRIER_BIT: shader write -> vertex read
//     ELEMENT_ARRAY_BARRIER_BIT:       shader write -> index read
//     UNIFORM_BARRIER_BIT:             shader write -> uniform read
//     TEXTURE_FETCH_BARRIER_BIT:       shader write -> texture sample
//     SHADER_IMAGE_ACCESS_BARRIER_BIT: shader write -> image access
//                                      any access -> image write
//     COMMAND_BARRIER_BIT:             shader write -> indirect buffer read
//     PIXEL_BUFFER_BARRIER_BIT:        shader write -> pbo access
//     TEXTURE_UPDATE_BARRIER_BIT:      shader write -> texture data upload
//     BUFFER_UPDATE_BARRIER_BIT:       shader write -> buffer data upload/map
//     FRAMEBUFFER_BARRIER_BIT:         shader write -> access through framebuffer
//     TRANSFORM_FEEDBACK_BARRIER_BIT:  shader write -> transform feedback write
//     ATOMIC_COUNTER_BARRIER_BIT:      shader write -> atomic counter access
//     SHADER_STORAGE_BARRIER_BIT:      shader write -> buffer access
//                                      any access -> buffer write
//
// In summary, every bit defines a memory barrier for some access after a shader write.
// Additionally, SHADER_IMAGE_ACCESS_BARRIER_BIT and SHADER_STORAGE_BARRIER_BIT bits are used to
// define a memory barrier for shader writes after other accesses.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/random_utils.h"

using namespace angle;

namespace
{
enum class ShaderWritePipeline
{
    Graphics,
    Compute,
};

enum class WriteResource
{
    Image,
    ImageBuffer,
    Buffer,
};

enum class NoopOp
{
    None,
    Draw,
    Dispatch,
};

// Variations corresponding to enums above.
using MemoryBarrierVariationsTestParams =
    std::tuple<angle::PlatformParameters, ShaderWritePipeline, WriteResource, NoopOp, NoopOp>;

std::ostream &operator<<(std::ostream &out, WriteResource writeResource)
{
    switch (writeResource)
    {
        case WriteResource::Image:
            out << "image";
            break;
        case WriteResource::Buffer:
            out << "buffer";
            break;
        case WriteResource::ImageBuffer:
            out << "imagebuffer";
            break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, ShaderWritePipeline writePipeline)
{
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        out << "graphics";
    }
    else
    {
        out << "compute";
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, NoopOp noop)
{
    switch (noop)
    {
        case NoopOp::Draw:
            out << "NoopDraw";
            break;
        case NoopOp::Dispatch:
            out << "NoopDispatch";
            break;
        case NoopOp::None:
            out << "NoopNone";
            break;
    }

    return out;
}

void ParseMemoryBarrierVariationsTestParams(const MemoryBarrierVariationsTestParams &params,
                                            ShaderWritePipeline *writePipelineOut,
                                            WriteResource *writeResourceOut,
                                            NoopOp *preBarrierOpOut,
                                            NoopOp *postBarrierOpOut)
{
    *writePipelineOut = std::get<1>(params);
    *writeResourceOut = std::get<2>(params);
    *preBarrierOpOut  = std::get<3>(params);
    *postBarrierOpOut = std::get<4>(params);
}

std::string MemoryBarrierVariationsTestPrint(
    const ::testing::TestParamInfo<MemoryBarrierVariationsTestParams> &paramsInfo)
{
    const MemoryBarrierVariationsTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params);

    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;

    ParseMemoryBarrierVariationsTestParams(params, &writePipeline, &writeResource, &preBarrierOp,
                                           &postBarrierOp);

    out << "__" << writePipeline << "_" << writeResource;

    if (preBarrierOp != NoopOp::None)
    {
        out << "_prebarrier" << preBarrierOp;
    }
    if (postBarrierOp != NoopOp::None)
    {
        out << "_postbarrier" << postBarrierOp;
    }
    return out.str();
}

class MemoryBarrierTestBase
{
  protected:
    bool hasExtensions(WriteResource writeResource);

    // Helper functions
    void createFramebuffer(GLuint color, GLuint fbo, GLColor initialColor);
    void createStorageBuffer(WriteResource writeResource,
                             GLuint buffer,
                             GLuint textureBuffer,
                             size_t size,
                             const void *initialData);
    void createStorageImage(WriteResource writeResource,
                            GLuint bufferStorage,
                            GLuint texture,
                            const std::array<float, 4> &initialData);
    void createProgram(ShaderWritePipeline writePipeline,
                       WriteResource writeResource,
                       GLProgram *programOut);
    void createNoopGraphicsProgram(GLProgram *programOut);
    void createNoopComputeProgram(GLProgram *programOut);
    void createQuadVertexArray(GLuint positionBuffer);
    void setupVertexArray(ShaderWritePipeline writePipeline, GLuint program);
    void setUniformData(GLuint program, const std::array<float, 4> &data);
    void noopOp(NoopOp op);

    template <typename T>
    void verifyBufferContents(const std::array<T, 4> &expected);

    void verifyImageContents(GLuint texture, const std::array<float, 4> &expected);

    template <typename T>
    void verifyFramebufferAndBufferContents(ShaderWritePipeline writePipeline,
                                            const std::array<T, 4> &expected);

    void verifyFramebufferAndImageContents(ShaderWritePipeline writePipeline,
                                           WriteResource writeResource,
                                           GLuint texture,
                                           const std::array<float, 4> &expected);

    // Barrier bits affecting only buffers and imageBuffers
    void createVertexVerifyProgram(GLuint vertexBuffer, GLProgram *programOut);
    void vertexAttribArrayBitBufferWriteThenVertexRead(ShaderWritePipeline writePipeline,
                                                       WriteResource writeResource,
                                                       NoopOp preBarrierOp,
                                                       NoopOp postBarrierOp);
    void vertexAttribArrayBitVertexReadThenBufferWrite(ShaderWritePipeline writePipeline,
                                                       WriteResource writeResource,
                                                       NoopOp preBarrierOp,
                                                       NoopOp postBarrierOp,
                                                       GLbitfield barrierBit);

    void createIndexVerifyProgram(GLuint indexBuffer, GLProgram *programOut);
    void elementArrayBitBufferWriteThenIndexRead(ShaderWritePipeline writePipeline,
                                                 WriteResource writeResource,
                                                 NoopOp preBarrierOp,
                                                 NoopOp postBarrierOp);
    void elementArrayBitIndexReadThenBufferWrite(ShaderWritePipeline writePipeline,
                                                 WriteResource writeResource,
                                                 NoopOp preBarrierOp,
                                                 NoopOp postBarrierOp,
                                                 GLbitfield barrierBit);

    void createUBOVerifyProgram(GLuint buffer, GLProgram *programOut);
    void uniformBitBufferWriteThenUBORead(ShaderWritePipeline writePipeline,
                                          WriteResource writeResource,
                                          NoopOp preBarrierOp,
                                          NoopOp postBarrierOp);
    void uniformBitUBOReadThenBufferWrite(ShaderWritePipeline writePipeline,
                                          WriteResource writeResource,
                                          NoopOp preBarrierOp,
                                          NoopOp postBarrierOp,
                                          GLbitfield barrierBit);

    void createIndirectVerifyProgram(GLuint buffer, GLProgram *programOut);
    void commandBitBufferWriteThenIndirectRead(ShaderWritePipeline writePipeline,
                                               WriteResource writeResource,
                                               NoopOp preBarrierOp,
                                               NoopOp postBarrierOp);
    void commandBitIndirectReadThenBufferWrite(ShaderWritePipeline writePipeline,
                                               WriteResource writeResource,
                                               NoopOp preBarrierOp,
                                               NoopOp postBarrierOp,
                                               GLbitfield barrierBit);

    void pixelBufferBitBufferWriteThenPack(ShaderWritePipeline writePipeline,
                                           WriteResource writeResource,
                                           NoopOp preBarrierOp,
                                           NoopOp postBarrierOp);
    void pixelBufferBitBufferWriteThenUnpack(ShaderWritePipeline writePipeline,
                                             WriteResource writeResource,
                                             NoopOp preBarrierOp,
                                             NoopOp postBarrierOp);
    void pixelBufferBitPackThenBufferWrite(ShaderWritePipeline writePipeline,
                                           WriteResource writeResource,
                                           NoopOp preBarrierOp,
                                           NoopOp postBarrierOp,
                                           GLbitfield barrierBit);
    void pixelBufferBitUnpackThenBufferWrite(ShaderWritePipeline writePipeline,
                                             WriteResource writeResource,
                                             NoopOp preBarrierOp,
                                             NoopOp postBarrierOp,
                                             GLbitfield barrierBit);

    void bufferUpdateBitBufferWriteThenCopy(ShaderWritePipeline writePipeline,
                                            WriteResource writeResource,
                                            NoopOp preBarrierOp,
                                            NoopOp postBarrierOp);
    void bufferUpdateBitCopyThenBufferWrite(ShaderWritePipeline writePipeline,
                                            WriteResource writeResource,
                                            NoopOp preBarrierOp,
                                            NoopOp postBarrierOp,
                                            GLbitfield barrierBit);

    void createXfbVerifyProgram(GLuint buffer, GLProgram *programOut);
    void transformFeedbackBitBufferWriteThenCapture(ShaderWritePipeline writePipeline,
                                                    WriteResource writeResource,
                                                    NoopOp preBarrierOp,
                                                    NoopOp postBarrierOp);
    void transformFeedbackBitCaptureThenBufferWrite(ShaderWritePipeline writePipeline,
                                                    WriteResource writeResource,
                                                    NoopOp preBarrierOp,
                                                    NoopOp postBarrierOp,
                                                    GLbitfield barrierBit);

    void createAtomicCounterVerifyProgram(GLuint buffer, GLProgram *programOut);
    void atomicCounterBitBufferWriteThenAtomic(ShaderWritePipeline writePipeline,
                                               WriteResource writeResource,
                                               NoopOp preBarrierOp,
                                               NoopOp postBarrierOp);
    void atomicCounterBitAtomicThenBufferWrite(ShaderWritePipeline writePipeline,
                                               WriteResource writeResource,
                                               NoopOp preBarrierOp,
                                               NoopOp postBarrierOp,
                                               GLbitfield barrierBit);

    void createSsboVerifyProgram(WriteResource writeResourcee, GLProgram *programOut);
    void shaderStorageBitBufferWriteThenBufferRead(ShaderWritePipeline writePipeline,
                                                   WriteResource writeResource,
                                                   NoopOp preBarrierOp,
                                                   NoopOp postBarrierOp);
    void shaderStorageBitBufferReadThenBufferWrite(ShaderWritePipeline writePipeline,
                                                   WriteResource writeResource,
                                                   NoopOp preBarrierOp,
                                                   NoopOp postBarrierOp,
                                                   GLbitfield barrierBit);

    // Barrier bits affecting only images and imageBuffers
    void createTextureVerifyProgram(WriteResource writeResource,
                                    GLuint texture,
                                    GLProgram *programOut);
    void textureFetchBitImageWriteThenSamplerRead(ShaderWritePipeline writePipeline,
                                                  WriteResource writeResource,
                                                  NoopOp preBarrierOp,
                                                  NoopOp postBarrierOp);
    void textureFetchBitSamplerReadThenImageWrite(ShaderWritePipeline writePipeline,
                                                  WriteResource writeResource,
                                                  NoopOp preBarrierOp,
                                                  NoopOp postBarrierOp,
                                                  GLbitfield barrierBit);

    void createImageVerifyProgram(WriteResource writeResource,
                                  GLuint texture,
                                  GLProgram *programOut);
    void shaderImageAccessBitImageWriteThenImageRead(ShaderWritePipeline writePipeline,
                                                     WriteResource writeResource,
                                                     NoopOp preBarrierOp,
                                                     NoopOp postBarrierOp);
    void shaderImageAccessBitImageReadThenImageWrite(ShaderWritePipeline writePipeline,
                                                     WriteResource writeResource,
                                                     NoopOp preBarrierOp,
                                                     NoopOp postBarrierOp);

    // Barrier bits affecting only images
    void textureUpdateBitImageWriteThenCopy(ShaderWritePipeline writePipeline,
                                            WriteResource writeResource,
                                            NoopOp preBarrierOp,
                                            NoopOp postBarrierOp);
    void textureUpdateBitCopyThenImageWrite(ShaderWritePipeline writePipeline,
                                            WriteResource writeResource,
                                            NoopOp preBarrierOp,
                                            NoopOp postBarrierOp,
                                            GLbitfield barrierBit);

    void framebufferBitImageWriteThenDraw(ShaderWritePipeline writePipeline,
                                          WriteResource writeResource,
                                          NoopOp preBarrierOp,
                                          NoopOp postBarrierOp);
    void framebufferBitImageWriteThenReadPixels(ShaderWritePipeline writePipeline,
                                                WriteResource writeResource,
                                                NoopOp preBarrierOp,
                                                NoopOp postBarrierOp);
    void framebufferBitImageWriteThenCopy(ShaderWritePipeline writePipeline,
                                          WriteResource writeResource,
                                          NoopOp preBarrierOp,
                                          NoopOp postBarrierOp);
    void framebufferBitImageWriteThenBlit(ShaderWritePipeline writePipeline,
                                          WriteResource writeResource,
                                          NoopOp preBarrierOp,
                                          NoopOp postBarrierOp);
    void framebufferBitDrawThenImageWrite(ShaderWritePipeline writePipeline,
                                          WriteResource writeResource,
                                          NoopOp preBarrierOp,
                                          NoopOp postBarrierOp,
                                          GLbitfield barrierBit);
    void framebufferBitReadPixelsThenImageWrite(ShaderWritePipeline writePipeline,
                                                WriteResource writeResource,
                                                NoopOp preBarrierOp,
                                                NoopOp postBarrierOp,
                                                GLbitfield barrierBit);
    void framebufferBitCopyThenImageWrite(ShaderWritePipeline writePipeline,
                                          WriteResource writeResource,
                                          NoopOp preBarrierOp,
                                          NoopOp postBarrierOp,
                                          GLbitfield barrierBit);
    void framebufferBitBlitThenImageWrite(ShaderWritePipeline writePipeline,
                                          WriteResource writeResource,
                                          NoopOp preBarrierOp,
                                          NoopOp postBarrierOp,
                                          GLbitfield barrierBit);

    static constexpr int kTextureSize    = 1;
    static constexpr char kUniformName[] = "uniformData";
};

bool MemoryBarrierTestBase::hasExtensions(WriteResource writeResource)
{
    return writeResource != WriteResource::ImageBuffer ||
           IsGLExtensionEnabled("GL_OES_texture_buffer");
}

void MemoryBarrierTestBase::createFramebuffer(GLuint color, GLuint fbo, GLColor initialColor)
{
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureSize, kTextureSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureSize, kTextureSize, GL_RGBA, GL_UNSIGNED_BYTE,
                    &initialColor);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    // Ensure all staged data is flushed.
    EXPECT_PIXEL_COLOR_EQ(0, 0, initialColor);
}

void MemoryBarrierTestBase::createStorageBuffer(WriteResource writeResource,
                                                GLuint buffer,
                                                GLuint textureBuffer,
                                                size_t size,
                                                const void *initialData)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, initialData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();

    if (writeResource == WriteResource::ImageBuffer)
    {
        glBindTexture(GL_TEXTURE_BUFFER, textureBuffer);
        glTexBufferEXT(GL_TEXTURE_BUFFER, GL_RGBA32F, buffer);
        glBindImageTexture(0, textureBuffer, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        EXPECT_GL_NO_ERROR();
    }
}

void MemoryBarrierTestBase::createStorageImage(WriteResource writeResource,
                                               GLuint bufferStorage,
                                               GLuint texture,
                                               const std::array<float, 4> &initialData)
{
    const std::array<uint8_t, 4> initialDataAsUnorm = {
        static_cast<uint8_t>(initialData[0] * 255),
        static_cast<uint8_t>(initialData[1] * 255),
        static_cast<uint8_t>(initialData[2] * 255),
        static_cast<uint8_t>(initialData[3] * 255),
    };

    if (writeResource == WriteResource::ImageBuffer)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferStorage);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(initialData), initialData.data(),
                     GL_STATIC_DRAW);
        EXPECT_GL_NO_ERROR();

        glBindTexture(GL_TEXTURE_BUFFER, texture);
        glTexBufferEXT(GL_TEXTURE_BUFFER, GL_RGBA32F, bufferStorage);
        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureSize, kTextureSize);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureSize, kTextureSize, GL_RGBA,
                        GL_UNSIGNED_BYTE, initialDataAsUnorm.data());
        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    }
}

void MemoryBarrierTestBase::createProgram(ShaderWritePipeline writePipeline,
                                          WriteResource writeResource,
                                          GLProgram *programOut)
{
    constexpr char kGraphicsImageFS[] = R"(#version 310 es
precision mediump float;
layout(rgba8, binding = 0) uniform highp writeonly image2D dst;
uniform vec4 uniformData;
out vec4 colorOut;
void main()
{
    colorOut = vec4(0, 0, 1.0, 1.0);
    imageStore(dst, ivec2(gl_FragCoord.xy), uniformData);
})";

    constexpr char kGraphicsBufferFS[] = R"(#version 310 es
precision mediump float;
uniform vec4 uniformData;
layout(std430, binding = 0) buffer block {
    vec4 data;
} outBlock;
out vec4 colorOut;
void main()
{
    colorOut = vec4(0, 0, 1.0, 1.0);
    outBlock.data = uniformData;
}
)";

    constexpr char kGraphicsImageBufferFS[] = R"(#version 310 es
#extension GL_OES_texture_buffer : require
precision mediump float;
layout(rgba32f, binding = 0) uniform highp writeonly imageBuffer dst;
uniform vec4 uniformData;
out vec4 colorOut;
void main()
{
    colorOut = vec4(0, 0, 1.0, 1.0);
    imageStore(dst, int(gl_FragCoord.x), uniformData);
})";

    constexpr char kComputeImage[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba8, binding = 0) uniform highp writeonly image2D dst;
uniform vec4 uniformData;
void main()
{
    imageStore(dst, ivec2(gl_GlobalInvocationID.xy), uniformData);
})";

    constexpr char kComputeBuffer[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
uniform vec4 uniformData;
layout(std430, binding = 0) buffer block {
    vec4 data;
} outBlock;
void main()
{
    outBlock.data = uniformData;
}
)";

    constexpr char kComputeImageBuffer[] = R"(#version 310 es
#extension GL_OES_texture_buffer : require
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32f, binding = 0) uniform highp writeonly imageBuffer dst;
uniform vec4 uniformData;
void main()
{
    imageStore(dst, int(gl_GlobalInvocationID.x), uniformData);
})";

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        const char *fs = "";
        switch (writeResource)
        {
            case WriteResource::Image:
                fs = kGraphicsImageFS;
                break;
            case WriteResource::Buffer:
                fs = kGraphicsBufferFS;
                break;
            case WriteResource::ImageBuffer:
                fs = kGraphicsImageBufferFS;
                break;
        }

        programOut->makeRaster(essl31_shaders::vs::Simple(), fs);
    }
    else
    {
        const char *cs = "";
        switch (writeResource)
        {
            case WriteResource::Image:
                cs = kComputeImage;
                break;
            case WriteResource::Buffer:
                cs = kComputeBuffer;
                break;
            case WriteResource::ImageBuffer:
                cs = kComputeImageBuffer;
                break;
        }

        programOut->makeCompute(cs);
    }

    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);
}

void MemoryBarrierTestBase::createNoopGraphicsProgram(GLProgram *programOut)
{
    programOut->makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ASSERT_TRUE(programOut->valid());
}

void MemoryBarrierTestBase::createNoopComputeProgram(GLProgram *programOut)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
void main()
{
})";

    programOut->makeCompute(kCS);
    ASSERT_TRUE(programOut->valid());
}

void MemoryBarrierTestBase::createQuadVertexArray(GLuint positionBuffer)
{
    const std::array<Vector3, 6> kQuadVertices = {{
        Vector3(-1.0f, 1.0f, 0.5f),
        Vector3(-1.0f, -1.0f, 0.5f),
        Vector3(1.0f, -1.0f, 0.5f),
        Vector3(-1.0f, 1.0f, 0.5f),
        Vector3(1.0f, -1.0f, 0.5f),
        Vector3(1.0f, 1.0f, 0.5f),
    }};

    const size_t bufferSize = kQuadVertices.size() * sizeof(Vector3);

    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, kQuadVertices.data(), GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::setupVertexArray(ShaderWritePipeline writePipeline, GLuint program)
{
    if (writePipeline == ShaderWritePipeline::Compute)
    {
        return;
    }

    GLint positionLoc = glGetAttribLocation(program, essl31_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLoc);

    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::setUniformData(GLuint program, const std::array<float, 4> &data)
{
    GLint uniformLocation = glGetUniformLocation(program, kUniformName);
    ASSERT_NE(uniformLocation, -1);

    glUniform4f(uniformLocation, data[0], data[1], data[2], data[3]);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::noopOp(NoopOp op)
{
    if (op == NoopOp::None)
    {
        return;
    }

    GLProgram noopProgram;
    if (op == NoopOp::Draw)
    {
        createNoopGraphicsProgram(&noopProgram);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ZERO, GL_ONE);

        glUseProgram(noopProgram);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glDisable(GL_BLEND);
    }
    else
    {
        createNoopComputeProgram(&noopProgram);
        glUseProgram(noopProgram);
        glDispatchCompute(1, 1, 1);
    }

    EXPECT_GL_NO_ERROR();
}

template <typename T>
void MemoryBarrierTestBase::verifyBufferContents(const std::array<T, 4> &expected)
{
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    T *bufferContents = static_cast<T *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(expected), GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(bufferContents[0], expected[0]);
    EXPECT_EQ(bufferContents[1], expected[1]);
    EXPECT_EQ(bufferContents[2], expected[2]);
    EXPECT_EQ(bufferContents[3], expected[3]);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void MemoryBarrierTestBase::verifyImageContents(GLuint texture,
                                                const std::array<float, 4> &expected)
{
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    glBindTexture(GL_TEXTURE_2D, texture);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    const GLColor kExpected(expected[0] * 255, expected[1] * 255, expected[2] * 255,
                            expected[3] * 255);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kExpected, 1);
}

template <typename T>
void MemoryBarrierTestBase::verifyFramebufferAndBufferContents(ShaderWritePipeline writePipeline,
                                                               const std::array<T, 4> &expected)
{
    // Verify the result of the verify shader
    const GLColor kExpected =
        writePipeline == ShaderWritePipeline::Graphics ? GLColor::cyan : GLColor::green;
    EXPECT_PIXEL_COLOR_EQ(0, 0, kExpected);

    // Verify the contents of the buffer
    verifyBufferContents(expected);
}

void MemoryBarrierTestBase::verifyFramebufferAndImageContents(ShaderWritePipeline writePipeline,
                                                              WriteResource writeResource,
                                                              GLuint texture,
                                                              const std::array<float, 4> &expected)
{
    // Verify the result of the verify shader
    const GLColor kExpected =
        writePipeline == ShaderWritePipeline::Graphics ? GLColor::cyan : GLColor::green;
    EXPECT_PIXEL_COLOR_EQ(0, 0, kExpected);

    if (writeResource == WriteResource::ImageBuffer)
    {
        // Verify the contents of the buffer
        verifyBufferContents(expected);
    }
    else
    {
        // Verify the contents of the image
        verifyImageContents(texture, expected);
    }
}

void MemoryBarrierTestBase::createVertexVerifyProgram(GLuint vertexBuffer, GLProgram *programOut)
{
    constexpr char kVS[] = R"(#version 310 es
in float attribIn;
out float v;

void main()
{
    v = attribIn;
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
in float v;
out vec4 colorOut;
void main()
{
    if (v == 2.0)
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    programOut->makeRaster(kVS, kFS);
    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);

    GLint attribLoc = glGetAttribLocation(*programOut, "attribIn");
    ASSERT_NE(-1, attribLoc);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(attribLoc, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(attribLoc);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::vertexAttribArrayBitBufferWriteThenVertexRead(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer vertexBuffer;
    GLTexture vertexTextureBuffer;
    constexpr std::array<float, 4> kInitData = {12.34, 5.6, 78.91, 123.456};
    createStorageBuffer(writeResource, vertexBuffer, vertexTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    constexpr std::array<float, 4> kWriteData = {2.0, 2.0, 2.0, 2.0};
    setUniformData(writeProgram, kWriteData);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the buffer
    GLProgram readProgram;
    createVertexVerifyProgram(vertexBuffer, &readProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    // Verify the vertex data was read correctly
    const GLColor kExpected =
        writePipeline == ShaderWritePipeline::Graphics ? GLColor::cyan : GLColor::green;
    EXPECT_PIXEL_COLOR_EQ(0, 0, kExpected);

    // Verify the contents of the buffer
    verifyBufferContents(kWriteData);
}

void MemoryBarrierTestBase::vertexAttribArrayBitVertexReadThenBufferWrite(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp,
    GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer vertexBuffer;
    GLTexture vertexTextureBuffer;
    constexpr std::array<float, 4> kInitData = {2.0, 2.0, 2.0, 2.0};
    createStorageBuffer(writeResource, vertexBuffer, vertexTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    // Use the buffer
    GLProgram readProgram;
    createVertexVerifyProgram(vertexBuffer, &readProgram);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    GLint attribLoc = glGetAttribLocation(readProgram, "attribIn");
    glDisableVertexAttribArray(attribLoc);

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the buffer
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {12.34, 5.6, 78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::createIndexVerifyProgram(GLuint indexBuffer, GLProgram *programOut)
{
    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
out vec4 colorOut;
void main()
{
    colorOut = vec4(0, 1.0, 0, 1.0);
})";

    programOut->makeRaster(kVS, kFS);
    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::elementArrayBitBufferWriteThenIndexRead(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer indexBuffer;
    GLTexture indexTextureBuffer;
    constexpr std::array<float, 4> kInitData = {12.34, 5.6, 78.91, 123.456};
    createStorageBuffer(writeResource, indexBuffer, indexTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    constexpr std::array<uint32_t, 4> kWriteData = {0, 1, 2, 3};
    const std::array<float, 4> kWriteDataAsFloat = {
        *reinterpret_cast<const float *>(&kWriteData[0]),
        *reinterpret_cast<const float *>(&kWriteData[1]),
        *reinterpret_cast<const float *>(&kWriteData[2]),
        *reinterpret_cast<const float *>(&kWriteData[3]),
    };
    setUniformData(writeProgram, kWriteDataAsFloat);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_ELEMENT_ARRAY_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the buffer
    GLProgram readProgram;
    createIndexVerifyProgram(indexBuffer, &readProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
    EXPECT_GL_NO_ERROR();

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::elementArrayBitIndexReadThenBufferWrite(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp,
    GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer indexBuffer;
    GLTexture indexTextureBuffer;
    constexpr std::array<uint32_t, 4> kInitData = {0, 1, 2, 3};
    createStorageBuffer(writeResource, indexBuffer, indexTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    // Use the buffer
    GLProgram readProgram;
    createIndexVerifyProgram(indexBuffer, &readProgram);

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the buffer
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {12.34, 5.6, 78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::createUBOVerifyProgram(GLuint buffer, GLProgram *programOut)
{
    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
layout(binding = 0) uniform block {
    vec4 data;
} ubo;
out vec4 colorOut;
void main()
{
    if (ubo.data == vec4(1.5, 3.75, 5.0, 12.125))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    programOut->makeRaster(kVS, kFS);
    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);

    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::uniformBitBufferWriteThenUBORead(ShaderWritePipeline writePipeline,
                                                             WriteResource writeResource,
                                                             NoopOp preBarrierOp,
                                                             NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer uniformBuffer;
    GLTexture uniformTextureBuffer;
    constexpr std::array<float, 4> kInitData = {12.34, 5.6, 78.91, 123.456};
    createStorageBuffer(writeResource, uniformBuffer, uniformTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    constexpr std::array<float, 4> kWriteData = {1.5, 3.75, 5.0, 12.125};
    setUniformData(writeProgram, kWriteData);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_UNIFORM_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the buffer
    GLProgram readProgram;
    createUBOVerifyProgram(uniformBuffer, &readProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::uniformBitUBOReadThenBufferWrite(ShaderWritePipeline writePipeline,
                                                             WriteResource writeResource,
                                                             NoopOp preBarrierOp,
                                                             NoopOp postBarrierOp,
                                                             GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer uniformBuffer;
    GLTexture uniformTextureBuffer;
    constexpr std::array<float, 4> kInitData = {1.5, 3.75, 5.0, 12.125};
    createStorageBuffer(writeResource, uniformBuffer, uniformTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    // Use the buffer
    GLProgram readProgram;
    createUBOVerifyProgram(uniformBuffer, &readProgram);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the buffer
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {12.34, 5.6, 78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::createIndirectVerifyProgram(GLuint buffer, GLProgram *programOut)
{
    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
out vec4 colorOut;
void main()
{
    colorOut = vec4(0, 1.0, 0, 1.0);
})";

    programOut->makeRaster(kVS, kFS);
    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::commandBitBufferWriteThenIndirectRead(ShaderWritePipeline writePipeline,
                                                                  WriteResource writeResource,
                                                                  NoopOp preBarrierOp,
                                                                  NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer indirectBuffer;
    GLTexture indirectTextureBuffer;
    constexpr std::array<float, 4> kInitData = {12.34, 5.6, 78.91, 123.456};
    createStorageBuffer(writeResource, indirectBuffer, indirectTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    constexpr std::array<uint32_t, 4> kWriteData = {4, 1, 0, 0};
    const std::array<float, 4> kWriteDataAsFloat = {
        *reinterpret_cast<const float *>(&kWriteData[0]),
        *reinterpret_cast<const float *>(&kWriteData[1]),
        *reinterpret_cast<const float *>(&kWriteData[2]),
        *reinterpret_cast<const float *>(&kWriteData[3]),
    };
    setUniformData(writeProgram, kWriteDataAsFloat);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the buffer
    GLProgram readProgram;
    createIndirectVerifyProgram(indirectBuffer, &readProgram);

    GLVertexArray vao;
    glBindVertexArray(vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArraysIndirect(GL_TRIANGLE_STRIP, nullptr);
    EXPECT_GL_NO_ERROR();

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::commandBitIndirectReadThenBufferWrite(ShaderWritePipeline writePipeline,
                                                                  WriteResource writeResource,
                                                                  NoopOp preBarrierOp,
                                                                  NoopOp postBarrierOp,
                                                                  GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer indirectBuffer;
    GLTexture indirectTextureBuffer;
    constexpr std::array<uint32_t, 4> kInitData = {4, 1, 0, 0};
    createStorageBuffer(writeResource, indirectBuffer, indirectTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    // Use the buffer
    GLProgram readProgram;
    createIndirectVerifyProgram(indirectBuffer, &readProgram);

    GLVertexArray vao;
    glBindVertexArray(vao);

    glDrawArraysIndirect(GL_TRIANGLE_STRIP, nullptr);
    EXPECT_GL_NO_ERROR();

    glBindVertexArray(0);

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the buffer
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {12.34, 5.6, 78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::pixelBufferBitBufferWriteThenPack(ShaderWritePipeline writePipeline,
                                                              WriteResource writeResource,
                                                              NoopOp preBarrierOp,
                                                              NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer packBuffer;
    GLTexture packTextureBuffer;
    constexpr std::array<float, 4> kInitData = {1.5, 3.75, 5.0, 12.125};
    createStorageBuffer(writeResource, packBuffer, packTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    constexpr std::array<float, 4> kWriteData = {12.34, 5.6, 78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_PIXEL_BUFFER_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the buffer
    glBindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
    glReadPixels(0, 0, kTextureSize, kTextureSize, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    const std::array<uint32_t, 4> kExpectedData = {
        writePipeline == ShaderWritePipeline::Graphics ? 0xFFFFFF00u : 0xFF00FF00u,
        *reinterpret_cast<const uint32_t *>(&kWriteData[1]),
        *reinterpret_cast<const uint32_t *>(&kWriteData[2]),
        *reinterpret_cast<const uint32_t *>(&kWriteData[3]),
    };
    verifyFramebufferAndBufferContents(writePipeline, kExpectedData);
}

void MemoryBarrierTestBase::pixelBufferBitBufferWriteThenUnpack(ShaderWritePipeline writePipeline,
                                                                WriteResource writeResource,
                                                                NoopOp preBarrierOp,
                                                                NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer unpackBuffer;
    GLTexture unpackTextureBuffer;
    constexpr std::array<float, 4> kInitData = {1.5, 3.75, 5.0, 12.125};
    createStorageBuffer(writeResource, unpackBuffer, unpackTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    const std::array<float, 4> kWriteData = {*reinterpret_cast<const float *>(&GLColor::green), 5.6,
                                             78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_PIXEL_BUFFER_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the buffer
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureSize, kTextureSize, GL_RGBA, GL_UNSIGNED_BYTE,
                    0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // Verify the result of the unpack operation
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Verify the contents of the buffer
    verifyBufferContents(kWriteData);
}

void MemoryBarrierTestBase::pixelBufferBitPackThenBufferWrite(ShaderWritePipeline writePipeline,
                                                              WriteResource writeResource,
                                                              NoopOp preBarrierOp,
                                                              NoopOp postBarrierOp,
                                                              GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);

    GLBuffer packBuffer;
    GLTexture packTextureBuffer;
    constexpr std::array<float, 4> kInitData = {1.5, 3.75, 5.0, 12.125};
    createStorageBuffer(writeResource, packBuffer, packTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    // Use the buffer
    glBindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
    glReadPixels(0, 0, kTextureSize, kTextureSize, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the buffer
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {12.34, 5.6, 78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::pixelBufferBitUnpackThenBufferWrite(ShaderWritePipeline writePipeline,
                                                                WriteResource writeResource,
                                                                NoopOp preBarrierOp,
                                                                NoopOp postBarrierOp,
                                                                GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer unpackBuffer;
    GLTexture unpackTextureBuffer;
    const std::array<float, 4> kInitData = {*reinterpret_cast<const float *>(&GLColor::green), 3.75,
                                            5.0, 12.125};
    createStorageBuffer(writeResource, unpackBuffer, unpackTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    // Use the buffer
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureSize, kTextureSize, GL_RGBA, GL_UNSIGNED_BYTE,
                    0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the buffer
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {12.34, 5.6, 78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::bufferUpdateBitBufferWriteThenCopy(ShaderWritePipeline writePipeline,
                                                               WriteResource writeResource,
                                                               NoopOp preBarrierOp,
                                                               NoopOp postBarrierOp)
{
    GLBuffer srcBuffer;
    GLTexture srcTextureBuffer;
    constexpr std::array<float, 4> kSrcInitData = {9.3, 3.7, 11.34, 0.65};
    createStorageBuffer(WriteResource::Buffer, srcBuffer, srcTextureBuffer, sizeof(kSrcInitData),
                        kSrcInitData.data());

    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer writeBuffer;
    GLTexture writeTextureBuffer;
    constexpr std::array<float, 4> kInitData = {12.34, 5.6, 78.91, 123.456};
    createStorageBuffer(writeResource, writeBuffer, writeTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    constexpr std::array<float, 4> kWriteData = {1.5, 3.75, 5.0, 12.125};
    setUniformData(writeProgram, kWriteData);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Copy from src buffer over the buffer
    glBindBuffer(GL_UNIFORM_BUFFER, srcBuffer);
    glCopyBufferSubData(GL_UNIFORM_BUFFER, GL_SHADER_STORAGE_BUFFER, 0, 0, sizeof(kInitData));

    verifyFramebufferAndBufferContents(writePipeline, kSrcInitData);

    // Verify the src buffer is unaffected
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, srcBuffer);
    verifyBufferContents(kSrcInitData);
}

void MemoryBarrierTestBase::bufferUpdateBitCopyThenBufferWrite(ShaderWritePipeline writePipeline,
                                                               WriteResource writeResource,
                                                               NoopOp preBarrierOp,
                                                               NoopOp postBarrierOp,
                                                               GLbitfield barrierBit)
{
    GLBuffer srcBuffer;
    GLTexture srcTextureBuffer;
    constexpr std::array<float, 4> kSrcInitData = {9.3, 3.7, 11.34, 0.65};
    createStorageBuffer(WriteResource::Buffer, srcBuffer, srcTextureBuffer, sizeof(kSrcInitData),
                        kSrcInitData.data());

    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);

    GLBuffer writeBuffer;
    GLTexture writeTextureBuffer;
    constexpr std::array<float, 4> kInitData = {12.34, 5.6, 78.91, 123.456};
    createStorageBuffer(writeResource, writeBuffer, writeTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    // Copy from src buffer over the buffer
    glBindBuffer(GL_UNIFORM_BUFFER, srcBuffer);
    glCopyBufferSubData(GL_UNIFORM_BUFFER, GL_SHADER_STORAGE_BUFFER, 0, 0, sizeof(kInitData));

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the buffer
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {1.5, 3.75, 5.0, 12.125};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);

    // Verify the src buffer is unaffected
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, srcBuffer);
    verifyBufferContents(kSrcInitData);
}

void MemoryBarrierTestBase::createXfbVerifyProgram(GLuint buffer, GLProgram *programOut)
{
    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //   0 (000)     -1   -1
    //   1 (001)      1   -1
    //   2 (010)     -1    1
    //   3 (011)      1    1
    //   4 (100)     -1    1
    //   5 (101)      1   -1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID < 4 ? gl_VertexID >> 1 & 1 : ~bit0 & 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
out vec4 colorOut;
void main()
{
    colorOut = vec4(0, 1.0, 0, 1.0);
})";

    const std::vector<std::string> &tfVaryings = {"gl_Position"};

    programOut->makeRasterWithTransformFeedback(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::transformFeedbackBitBufferWriteThenCapture(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer xfbBuffer;
    GLTexture xfbTextureBuffer;
    constexpr size_t kOneInstanceDataSize                                        = 4;
    constexpr size_t kInstanceCount                                              = 6;
    constexpr std::array<float, kOneInstanceDataSize * kInstanceCount> kInitData = {12.34, 5.6,
                                                                                    78.91, 123.456};
    createStorageBuffer(writeResource, xfbBuffer, xfbTextureBuffer,
                        sizeof(kInitData[0]) * kInitData.size(), kInitData.data());

    constexpr std::array<float, 4> kWriteData = {1.5, 3.75, 5.0, 12.125};
    setUniformData(writeProgram, kWriteData);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_TRANSFORM_FEEDBACK_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the buffer
    GLProgram xfbProgram;
    createXfbVerifyProgram(xfbBuffer, &xfbProgram);

    glBeginTransformFeedback(GL_TRIANGLES);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEndTransformFeedback();
    EXPECT_GL_NO_ERROR();

    const std::array<float, 4> kExpectedData = {-1.0, -1.0, 0.0, 1.0};
    verifyFramebufferAndBufferContents(writePipeline, kExpectedData);
}

void MemoryBarrierTestBase::transformFeedbackBitCaptureThenBufferWrite(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp,
    GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer xfbBuffer;
    GLTexture xfbTextureBuffer;
    constexpr size_t kOneInstanceDataSize                                        = 4;
    constexpr size_t kInstanceCount                                              = 6;
    constexpr std::array<float, kOneInstanceDataSize * kInstanceCount> kInitData = {12.34, 5.6,
                                                                                    78.91, 123.456};
    createStorageBuffer(writeResource, xfbBuffer, xfbTextureBuffer,
                        sizeof(kInitData[0]) * kInitData.size(), kInitData.data());

    // Use the buffer
    GLProgram xfbProgram;
    createXfbVerifyProgram(xfbBuffer, &xfbProgram);

    glBeginTransformFeedback(GL_TRIANGLES);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEndTransformFeedback();
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the buffer
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {1.5, 3.75, 5.0, 12.125};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::createAtomicCounterVerifyProgram(GLuint buffer, GLProgram *programOut)
{
    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
layout(binding = 0, offset = 0) uniform atomic_uint ac[4];
out vec4 colorOut;
void main()
{
    uvec4 acValue = uvec4(atomicCounterIncrement(ac[0]),
                          atomicCounterIncrement(ac[1]),
                          atomicCounterIncrement(ac[2]),
                          atomicCounterIncrement(ac[3]));

    if (all(equal(acValue, uvec4(10, 20, 30, 40))))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    programOut->makeRaster(kVS, kFS);
    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buffer);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::atomicCounterBitBufferWriteThenAtomic(ShaderWritePipeline writePipeline,
                                                                  WriteResource writeResource,
                                                                  NoopOp preBarrierOp,
                                                                  NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer atomicCounterBuffer;
    GLTexture atomicCounterTextureBuffer;
    constexpr std::array<uint32_t, 4> kInitData = {0x12345678u, 0x9ABCDEF0u, 0x13579BDFu,
                                                   0x2468ACE0u};
    createStorageBuffer(writeResource, atomicCounterBuffer, atomicCounterTextureBuffer,
                        sizeof(kInitData), kInitData.data());

    constexpr std::array<uint32_t, 4> kWriteData = {10, 20, 30, 40};
    const std::array<float, 4> kWriteDataAsFloat = {
        *reinterpret_cast<const float *>(&kWriteData[0]),
        *reinterpret_cast<const float *>(&kWriteData[1]),
        *reinterpret_cast<const float *>(&kWriteData[2]),
        *reinterpret_cast<const float *>(&kWriteData[3]),
    };
    setUniformData(writeProgram, kWriteDataAsFloat);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the buffer
    GLProgram readProgram;
    createAtomicCounterVerifyProgram(atomicCounterBuffer, &readProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    constexpr std::array<uint32_t, 4> kExpectedData = {11, 21, 31, 41};
    verifyFramebufferAndBufferContents(writePipeline, kExpectedData);
}

void MemoryBarrierTestBase::atomicCounterBitAtomicThenBufferWrite(ShaderWritePipeline writePipeline,
                                                                  WriteResource writeResource,
                                                                  NoopOp preBarrierOp,
                                                                  NoopOp postBarrierOp,
                                                                  GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer atomicCounterBuffer;
    GLTexture atomicCounterTextureBuffer;
    constexpr std::array<uint32_t, 4> kInitData = {10, 20, 30, 40};
    createStorageBuffer(writeResource, atomicCounterBuffer, atomicCounterTextureBuffer,
                        sizeof(kInitData), kInitData.data());

    // Use the buffer
    GLProgram readProgram;
    createAtomicCounterVerifyProgram(atomicCounterBuffer, &readProgram);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the buffer
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {12.34, 5.6, 78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::createSsboVerifyProgram(WriteResource writeResource,
                                                    GLProgram *programOut)
{
    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
layout(std430, binding = 0) buffer block {
    vec4 data;
} inBlock;
out vec4 colorOut;
void main()
{
    if (all(lessThan(abs(inBlock.data - vec4(1.5, 3.75, 5.0, 12.125)), vec4(0.01))))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    programOut->makeRaster(kVS, kFS);
    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);
}

void MemoryBarrierTestBase::shaderStorageBitBufferWriteThenBufferRead(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer writeBuffer;
    GLTexture writeTextureBuffer;
    constexpr std::array<float, 4> kInitData = {12.34, 5.6, 78.91, 123.456};
    createStorageBuffer(writeResource, writeBuffer, writeTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    constexpr std::array<float, 4> kWriteData = {1.5, 3.75, 5.0, 12.125};
    setUniformData(writeProgram, kWriteData);

    // Fill the buffer
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the buffer
    GLProgram readProgram;
    createSsboVerifyProgram(writeResource, &readProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::shaderStorageBitBufferReadThenBufferWrite(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp,
    GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer writeBuffer;
    GLTexture writeTextureBuffer;
    constexpr std::array<float, 4> kInitData = {1.5, 3.75, 5.0, 12.125};
    createStorageBuffer(writeResource, writeBuffer, writeTextureBuffer, sizeof(kInitData),
                        kInitData.data());

    // Use the buffer
    GLProgram readProgram;
    createSsboVerifyProgram(writeResource, &readProgram);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the image
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {12.34, 5.6, 78.91, 123.456};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndBufferContents(writePipeline, kWriteData);
}

void MemoryBarrierTestBase::createTextureVerifyProgram(WriteResource writeResource,
                                                       GLuint texture,
                                                       GLProgram *programOut)
{
    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kImageFS[] = R"(#version 310 es
precision mediump float;
uniform sampler2D s;
out vec4 colorOut;
void main()
{
    if (all(lessThan(abs(texelFetch(s, ivec2(0, 0), 0)- vec4(0.125, 0.25, 0.5, 0.75)), vec4(0.01))))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    constexpr char kImageBufferFS[] = R"(#version 310 es
#extension GL_OES_texture_buffer : require
precision mediump float;
uniform highp samplerBuffer s;
out vec4 colorOut;
void main()
{
    if (texelFetch(s, 0) == vec4(0.125, 0.25, 0.5, 0.75))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    programOut->makeRaster(kVS,
                           writeResource == WriteResource::ImageBuffer ? kImageBufferFS : kImageFS);
    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);

    glBindTexture(writeResource == WriteResource::ImageBuffer ? GL_TEXTURE_BUFFER : GL_TEXTURE_2D,
                  texture);
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::textureFetchBitImageWriteThenSamplerRead(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.65, 0.92, 0.11, 0.54};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    constexpr std::array<float, 4> kWriteData = {0.125, 0.25, 0.5, 0.75};
    setUniformData(writeProgram, kWriteData);

    // Fill the image
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the image
    GLProgram readProgram;
    createTextureVerifyProgram(writeResource, texture, &readProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);
}

void MemoryBarrierTestBase::textureFetchBitSamplerReadThenImageWrite(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp,
    GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.125, 0.25, 0.5, 0.75};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    // Use the image
    GLProgram readProgram;
    createTextureVerifyProgram(writeResource, texture, &readProgram);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the image
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {0.65, 0.20, 0.40, 0.95};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);
}

void MemoryBarrierTestBase::createImageVerifyProgram(WriteResource writeResource,
                                                     GLuint texture,
                                                     GLProgram *programOut)
{
    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kImageFS[] = R"(#version 310 es
precision mediump float;
layout(rgba8, binding = 0) uniform highp readonly image2D img;
out vec4 colorOut;
void main()
{
    if (all(lessThan(abs(imageLoad(img, ivec2(0, 0))- vec4(0.125, 0.25, 0.5, 0.75)), vec4(0.01))))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    constexpr char kImageBufferFS[] = R"(#version 310 es
#extension GL_OES_texture_buffer : require
precision mediump float;
layout(rgba32f, binding = 0) uniform highp readonly imageBuffer img;
out vec4 colorOut;
void main()
{
    if (imageLoad(img, 0) == vec4(0.125, 0.25, 0.5, 0.75))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    programOut->makeRaster(kVS,
                           writeResource == WriteResource::ImageBuffer ? kImageBufferFS : kImageFS);
    ASSERT_TRUE(programOut->valid());
    glUseProgram(*programOut);

    if (writeResource == WriteResource::ImageBuffer)
    {
        glBindTexture(GL_TEXTURE_BUFFER, texture);
        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    }
    EXPECT_GL_NO_ERROR();
}

void MemoryBarrierTestBase::shaderImageAccessBitImageWriteThenImageRead(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.65, 0.92, 0.11, 0.54};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    constexpr std::array<float, 4> kWriteData = {0.125, 0.25, 0.5, 0.75};
    setUniformData(writeProgram, kWriteData);

    // Fill the image
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Use the image
    GLProgram readProgram;
    createImageVerifyProgram(writeResource, texture, &readProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);
}

void MemoryBarrierTestBase::shaderImageAccessBitImageReadThenImageWrite(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::black);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.125, 0.25, 0.5, 0.75};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    // Use the image
    GLProgram readProgram;
    createImageVerifyProgram(writeResource, texture, &readProgram);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Fill the image
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {0.65, 0.20, 0.40, 0.95};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);
}

void MemoryBarrierTestBase::textureUpdateBitImageWriteThenCopy(ShaderWritePipeline writePipeline,
                                                               WriteResource writeResource,
                                                               NoopOp preBarrierOp,
                                                               NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.65, 0.92, 0.11, 0.54};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    constexpr std::array<float, 4> kWriteData = {0.125, 0.25, 0.5, 0.75};
    setUniformData(writeProgram, kWriteData);

    // Fill the image
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Copy from framebuffer over the texture
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kTextureSize, kTextureSize);

    const std::array<float, 4> kExpectedData = {
        0, 1.0f, writePipeline == ShaderWritePipeline::Graphics ? 1.0f : 0, 1.0f};
    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kExpectedData);
}

void MemoryBarrierTestBase::textureUpdateBitCopyThenImageWrite(ShaderWritePipeline writePipeline,
                                                               WriteResource writeResource,
                                                               NoopOp preBarrierOp,
                                                               NoopOp postBarrierOp,
                                                               GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.65, 0.92, 0.11, 0.54};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    // Copy from framebuffer over the texture
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kTextureSize, kTextureSize);

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the image
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {0.125, 0.25, 0.5, 0.75};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);
}

void MemoryBarrierTestBase::framebufferBitImageWriteThenDraw(ShaderWritePipeline writePipeline,
                                                             WriteResource writeResource,
                                                             NoopOp preBarrierOp,
                                                             NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.65, 0.92, 0.11, 0.54};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    constexpr std::array<float, 4> kWriteData = {0.125, 0.25, 0.5, 0.75};
    setUniformData(writeProgram, kWriteData);

    // Fill the image
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Draw to image via framebuffer
    ANGLE_GL_PROGRAM(verifyProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(verifyProgram);
    GLint colorUniformLocation =
        glGetUniformLocation(verifyProgram, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);
    setupVertexArray(ShaderWritePipeline::Graphics, verifyProgram);

    GLFramebuffer drawFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    constexpr std::array<float, 4> kBlendData = {0.125, 0.25, 0.5, 0.25};
    glUniform4fv(colorUniformLocation, 1, kBlendData.data());

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);
    EXPECT_GL_NO_ERROR();

    const std::array<float, 4> kExpectedData = {
        kWriteData[0] + kBlendData[0],
        kWriteData[1] + kBlendData[1],
        kWriteData[2] + kBlendData[2],
        kWriteData[3] + kBlendData[3],
    };
    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kExpectedData);
}

void MemoryBarrierTestBase::framebufferBitImageWriteThenReadPixels(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.65, 0.92, 0.11, 0.54};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    constexpr std::array<float, 4> kWriteData = {1.0, 1.0, 0.0, 1.0};
    setUniformData(writeProgram, kWriteData);

    // Fill the image
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Read from image via framebuffer
    GLBuffer packBuffer;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
    constexpr std::array<uint32_t, 4> kPBOInitData = {0x12345678u, 0x9ABCDEF0u, 0x13579BDFu,
                                                      0x2468ACE0u};
    glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(kInitData), kPBOInitData.data(), GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer readFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    glReadPixels(0, 0, kTextureSize, kTextureSize, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    EXPECT_GL_NO_ERROR();

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);

    // Verify the PBO for completeness
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, packBuffer);
    constexpr std::array<uint32_t, 4> kExpectedData = {
        0xFF00FFFFu,
        kPBOInitData[1],
        kPBOInitData[2],
        kPBOInitData[3],
    };
    verifyBufferContents(kExpectedData);
}

void MemoryBarrierTestBase::framebufferBitImageWriteThenCopy(ShaderWritePipeline writePipeline,
                                                             WriteResource writeResource,
                                                             NoopOp preBarrierOp,
                                                             NoopOp postBarrierOp)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.65, 0.92, 0.11, 0.54};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    constexpr std::array<float, 4> kWriteData = {1.0, 1.0, 0.0, 1.0};
    setUniformData(writeProgram, kWriteData);

    // Fill the image
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Copy from framebuffer to another texture
    GLFramebuffer readFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    GLTexture copyTexture;
    glBindTexture(GL_TEXTURE_2D, copyTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureSize, kTextureSize);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kTextureSize, kTextureSize);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);

    // Verify the copy texture for completeness
    verifyImageContents(copyTexture, kWriteData);
}

void MemoryBarrierTestBase::framebufferBitImageWriteThenBlit(ShaderWritePipeline writePipeline,
                                                             WriteResource writeResource,
                                                             NoopOp preBarrierOp,
                                                             NoopOp postBarrierOp)
{
    GLTexture blitColor;
    GLFramebuffer blitFbo;
    createFramebuffer(blitColor, blitFbo, GLColor::black);

    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.65, 0.92, 0.11, 0.54};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    constexpr std::array<float, 4> kWriteData = {1.0, 1.0, 0.0, 1.0};
    setUniformData(writeProgram, kWriteData);

    // Fill the image
    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    noopOp(postBarrierOp);

    // Blit from framebuffer to another framebuffer
    GLFramebuffer readFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blitFbo);
    glBlitFramebuffer(0, 0, kTextureSize, kTextureSize, 0, 0, kTextureSize, kTextureSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);

    // Verify the blit fbo for completeness
    glBindFramebuffer(GL_READ_FRAMEBUFFER, blitFbo);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
}

void MemoryBarrierTestBase::framebufferBitDrawThenImageWrite(ShaderWritePipeline writePipeline,
                                                             WriteResource writeResource,
                                                             NoopOp preBarrierOp,
                                                             NoopOp postBarrierOp,
                                                             GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {0.65, 0.92, 0.11, 0.54};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    // Draw to image via framebuffer
    ANGLE_GL_PROGRAM(verifyProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(verifyProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(ShaderWritePipeline::Graphics, verifyProgram);

    GLFramebuffer drawFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the image
    createProgram(writePipeline, writeResource, &writeProgram);

    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {0.125, 0.25, 0.5, 0.75};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);
}

void MemoryBarrierTestBase::framebufferBitReadPixelsThenImageWrite(
    ShaderWritePipeline writePipeline,
    WriteResource writeResource,
    NoopOp preBarrierOp,
    NoopOp postBarrierOp,
    GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {1.0, 1.0, 0.0, 1.0};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    // Read from image via framebuffer
    GLBuffer packBuffer;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
    constexpr std::array<uint32_t, 4> kPBOInitData = {0x12345678u, 0x9ABCDEF0u, 0x13579BDFu,
                                                      0x2468ACE0u};
    glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(kInitData), kPBOInitData.data(), GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer readFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    glReadPixels(0, 0, kTextureSize, kTextureSize, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the image
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {0.125, 0.25, 0.5, 0.75};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);

    // Verify the PBO for completeness
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, packBuffer);
    constexpr std::array<uint32_t, 4> kExpectedData = {
        0xFF00FFFFu,
        kPBOInitData[1],
        kPBOInitData[2],
        kPBOInitData[3],
    };
    verifyBufferContents(kExpectedData);
}

void MemoryBarrierTestBase::framebufferBitCopyThenImageWrite(ShaderWritePipeline writePipeline,
                                                             WriteResource writeResource,
                                                             NoopOp preBarrierOp,
                                                             NoopOp postBarrierOp,
                                                             GLbitfield barrierBit)
{
    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {1.0, 1.0, 0.0, 1.0};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    // Copy from framebuffer to another texture
    GLFramebuffer readFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    GLTexture copyTexture;
    glBindTexture(GL_TEXTURE_2D, copyTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureSize, kTextureSize);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kTextureSize, kTextureSize);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the image
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {0.125, 0.25, 0.5, 0.75};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);

    // Verify the copy texture for completeness
    verifyImageContents(copyTexture, kInitData);
}

void MemoryBarrierTestBase::framebufferBitBlitThenImageWrite(ShaderWritePipeline writePipeline,
                                                             WriteResource writeResource,
                                                             NoopOp preBarrierOp,
                                                             NoopOp postBarrierOp,
                                                             GLbitfield barrierBit)
{
    GLTexture blitColor;
    GLFramebuffer blitFbo;
    createFramebuffer(blitColor, blitFbo, GLColor::black);

    GLTexture color;
    GLFramebuffer fbo;
    GLProgram writeProgram;

    createFramebuffer(color, fbo, GLColor::green);

    GLBuffer textureBufferStorage;
    GLTexture texture;
    constexpr std::array<float, 4> kInitData = {1.0, 1.0, 0.0, 1.0};
    createStorageImage(writeResource, textureBufferStorage, texture, kInitData);

    // Blit from framebuffer to another framebuffer
    GLFramebuffer readFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blitFbo);
    glBlitFramebuffer(0, 0, kTextureSize, kTextureSize, 0, 0, kTextureSize, kTextureSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    noopOp(preBarrierOp);

    // Issue the appropriate memory barrier
    glMemoryBarrier(barrierBit);

    noopOp(postBarrierOp);

    // Fill the image
    createProgram(writePipeline, writeResource, &writeProgram);

    GLBuffer positionBuffer;
    createQuadVertexArray(positionBuffer);
    setupVertexArray(writePipeline, writeProgram);

    constexpr std::array<float, 4> kWriteData = {0.125, 0.25, 0.5, 0.75};
    setUniformData(writeProgram, kWriteData);

    if (writePipeline == ShaderWritePipeline::Graphics)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        glDispatchCompute(1, 1, 1);
    }

    verifyFramebufferAndImageContents(writePipeline, writeResource, texture, kWriteData);

    // Verify the blit fbo for completeness
    glBindFramebuffer(GL_READ_FRAMEBUFFER, blitFbo);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
}

class MemoryBarrierBufferTest : public MemoryBarrierTestBase,
                                public ANGLETest<MemoryBarrierVariationsTestParams>
{
  protected:
    MemoryBarrierBufferTest()
    {
        setWindowWidth(16);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class MemoryBarrierImageTest : public MemoryBarrierTestBase,
                               public ANGLETest<MemoryBarrierVariationsTestParams>
{
  protected:
    MemoryBarrierImageTest()
    {
        setWindowWidth(16);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class MemoryBarrierImageBufferOnlyTest : public MemoryBarrierTestBase,
                                         public ANGLETest<MemoryBarrierVariationsTestParams>
{
  protected:
    MemoryBarrierImageBufferOnlyTest()
    {
        setWindowWidth(16);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class MemoryBarrierImageOnlyTest : public MemoryBarrierTestBase,
                                   public ANGLETest<MemoryBarrierVariationsTestParams>
{
  protected:
    MemoryBarrierImageOnlyTest()
    {
        setWindowWidth(16);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class MemoryBarrierBufferOnlyTest : public MemoryBarrierTestBase,
                                    public ANGLETest<MemoryBarrierVariationsTestParams>
{
  protected:
    MemoryBarrierBufferOnlyTest()
    {
        setWindowWidth(16);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT; shader write -> vertex read
TEST_P(MemoryBarrierBufferTest, VertexAtrribArrayBitWriteThenVertexRead)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    vertexAttribArrayBitBufferWriteThenVertexRead(writePipeline, writeResource, preBarrierOp,
                                                  postBarrierOp);
}

// Test GL_ELEMENT_ARRAY_BARRIER_BIT; shader write -> index read
TEST_P(MemoryBarrierBufferTest, ElementArrayBitWriteThenIndexRead)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    elementArrayBitBufferWriteThenIndexRead(writePipeline, writeResource, preBarrierOp,
                                            postBarrierOp);
}

// Test GL_UNIFORM_BARRIER_BIT; shader write -> ubo read
TEST_P(MemoryBarrierBufferTest, UniformBitWriteThenUBORead)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    uniformBitBufferWriteThenUBORead(writePipeline, writeResource, preBarrierOp, postBarrierOp);
}

// Test GL_TEXTURE_FETCH_BARRIER_BIT; shader write -> sampler read
TEST_P(MemoryBarrierImageTest, TextureFetchBitWriteThenSamplerRead)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    textureFetchBitImageWriteThenSamplerRead(writePipeline, writeResource, preBarrierOp,
                                             postBarrierOp);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; shader write -> image read
TEST_P(MemoryBarrierImageTest, ShaderImageAccessBitWriteThenImageRead)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    shaderImageAccessBitImageWriteThenImageRead(writePipeline, writeResource, preBarrierOp,
                                                postBarrierOp);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; image read -> shader write
TEST_P(MemoryBarrierImageTest, ShaderImageAccessBitImageReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    // Looks like the implementation is missing this line from the spec:
    //
    // > Additionally, image stores issued after the barrier will not execute until all memory
    // > accesses (e.g., loads, stores, texture fetches, vertex fetches) initiated prior to the
    // > barrier complete.
    //
    // http://anglebug.com/42264187
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsNVIDIA() && writeResource == WriteResource::Image);

    shaderImageAccessBitImageReadThenImageWrite(writePipeline, writeResource, preBarrierOp,
                                                postBarrierOp);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; vertex read -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitVertexReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    vertexAttribArrayBitVertexReadThenBufferWrite(writePipeline, writeResource, preBarrierOp,
                                                  postBarrierOp,
                                                  GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; index read -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitIndexReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    elementArrayBitIndexReadThenBufferWrite(writePipeline, writeResource, preBarrierOp,
                                            postBarrierOp, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; ubo read -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitUBOReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    uniformBitUBOReadThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                     GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; sampler read -> shader write
TEST_P(MemoryBarrierImageTest, ShaderImageAccessBitSamplerReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    textureFetchBitSamplerReadThenImageWrite(writePipeline, writeResource, preBarrierOp,
                                             postBarrierOp, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; indirect read -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitIndirectReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    commandBitIndirectReadThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                          GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; pixel pack -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitPackThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    pixelBufferBitPackThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                      GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; pixel unpack -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitUnpackThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    pixelBufferBitUnpackThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                        GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; texture copy -> shader write
TEST_P(MemoryBarrierImageOnlyTest, ShaderImageAccessBitCopyThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    textureUpdateBitCopyThenImageWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                       GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; buffer copy -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitCopyThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    bufferUpdateBitCopyThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                       GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; draw -> shader write
TEST_P(MemoryBarrierImageOnlyTest, ShaderImageAccessBitDrawThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    framebufferBitDrawThenImageWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                     GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; read pixels -> shader write
TEST_P(MemoryBarrierImageOnlyTest, ShaderImageAccessBitReadPixelsThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    framebufferBitReadPixelsThenImageWrite(writePipeline, writeResource, preBarrierOp,
                                           postBarrierOp, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; fbo copy -> shader write
TEST_P(MemoryBarrierImageOnlyTest, ShaderImageAccessBitFBOCopyThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    framebufferBitCopyThenImageWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                     GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; blit -> shader write
TEST_P(MemoryBarrierImageOnlyTest, ShaderImageAccessBitBlitThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    framebufferBitBlitThenImageWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                     GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; xfb capture -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitCaptureThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    transformFeedbackBitCaptureThenBufferWrite(writePipeline, writeResource, preBarrierOp,
                                               postBarrierOp, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; atomic write -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitAtomicThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    atomicCounterBitAtomicThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                          GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_SHADER_IMAGE_ACCESS_BARRIER_BIT; buffer read -> shader write
TEST_P(MemoryBarrierImageBufferOnlyTest, ShaderImageAccessBitBufferReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    shaderStorageBitBufferReadThenBufferWrite(writePipeline, writeResource, preBarrierOp,
                                              postBarrierOp, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Test GL_COMMAND_BARRIER_BIT; shader write -> indirect read
TEST_P(MemoryBarrierBufferTest, CommandBitWriteThenIndirectRead)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    commandBitBufferWriteThenIndirectRead(writePipeline, writeResource, preBarrierOp,
                                          postBarrierOp);
}

// Test GL_PIXEL_BUFFER_BARRIER_BIT; shader write -> pixel pack
TEST_P(MemoryBarrierBufferTest, PixelBufferBitWriteThenPack)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    pixelBufferBitBufferWriteThenPack(writePipeline, writeResource, preBarrierOp, postBarrierOp);
}

// Test GL_PIXEL_BUFFER_BARRIER_BIT; shader write -> pixel unpack
TEST_P(MemoryBarrierBufferTest, PixelBufferBitWriteThenUnpack)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    pixelBufferBitBufferWriteThenUnpack(writePipeline, writeResource, preBarrierOp, postBarrierOp);
}

// Test GL_TEXTURE_UPDATE_BARRIER_BIT; shader write -> texture copy
TEST_P(MemoryBarrierImageOnlyTest, TextureUpdateBitWriteThenCopy)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    textureUpdateBitImageWriteThenCopy(writePipeline, writeResource, preBarrierOp, postBarrierOp);
}

// Test GL_BUFFER_UPDATE_BARRIER_BIT; shader write -> buffer copy
TEST_P(MemoryBarrierBufferTest, BufferUpdateBitWriteThenCopy)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    bufferUpdateBitBufferWriteThenCopy(writePipeline, writeResource, preBarrierOp, postBarrierOp);
}

// Test GL_FRAMEBUFFER_BARRIER_BIT; shader write -> draw
TEST_P(MemoryBarrierImageOnlyTest, FramebufferBitWriteThenDraw)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    framebufferBitImageWriteThenDraw(writePipeline, writeResource, preBarrierOp, postBarrierOp);
}

// Test GL_FRAMEBUFFER_BARRIER_BIT; shader write -> read pixels
TEST_P(MemoryBarrierImageOnlyTest, FramebufferBitWriteThenReadPixels)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    framebufferBitImageWriteThenReadPixels(writePipeline, writeResource, preBarrierOp,
                                           postBarrierOp);
}

// Test GL_FRAMEBUFFER_BARRIER_BIT; shader write -> copy
TEST_P(MemoryBarrierImageOnlyTest, FramebufferBitWriteThenCopy)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    framebufferBitImageWriteThenCopy(writePipeline, writeResource, preBarrierOp, postBarrierOp);
}

// Test GL_FRAMEBUFFER_BARRIER_BIT; shader write -> blit
TEST_P(MemoryBarrierImageOnlyTest, FramebufferBitWriteThenBlit)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    framebufferBitImageWriteThenBlit(writePipeline, writeResource, preBarrierOp, postBarrierOp);
}

// Test GL_TRANSFORM_FEEDBACK_BARRIER_BIT; shader write -> xfb capture
TEST_P(MemoryBarrierBufferTest, TransformFeedbackBitWriteThenCapture)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    transformFeedbackBitBufferWriteThenCapture(writePipeline, writeResource, preBarrierOp,
                                               postBarrierOp);
}

// Test GL_ATOMIC_COUNTER_BARRIER_BIT; shader write -> atomic write
TEST_P(MemoryBarrierBufferTest, AtomicCounterBitWriteThenAtomic)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    atomicCounterBitBufferWriteThenAtomic(writePipeline, writeResource, preBarrierOp,
                                          postBarrierOp);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; shader write -> shader read
TEST_P(MemoryBarrierBufferTest, ShaderStorageBitWriteThenRead)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    shaderStorageBitBufferWriteThenBufferRead(writePipeline, writeResource, preBarrierOp,
                                              postBarrierOp);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; shader read -> buffer write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    shaderStorageBitBufferReadThenBufferWrite(writePipeline, writeResource, preBarrierOp,
                                              postBarrierOp, GL_SHADER_STORAGE_BARRIER_BIT);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; vertex read -> shader write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitVertexReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    vertexAttribArrayBitVertexReadThenBufferWrite(writePipeline, writeResource, preBarrierOp,
                                                  postBarrierOp, GL_SHADER_STORAGE_BARRIER_BIT);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; index read -> shader write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitIndexReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    elementArrayBitIndexReadThenBufferWrite(writePipeline, writeResource, preBarrierOp,
                                            postBarrierOp, GL_SHADER_STORAGE_BARRIER_BIT);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; ubo read -> shader write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitUBOReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    uniformBitUBOReadThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                     GL_SHADER_STORAGE_BARRIER_BIT);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; indirect read -> shader write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitIndirectReadThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    commandBitIndirectReadThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                          GL_SHADER_STORAGE_BARRIER_BIT);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; pixel pack -> shader write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitPackThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    pixelBufferBitPackThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                      GL_SHADER_STORAGE_BARRIER_BIT);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; pixel unpack -> shader write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitUnpackThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    pixelBufferBitUnpackThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                        GL_SHADER_STORAGE_BARRIER_BIT);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; buffer copy -> shader write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitCopyThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    bufferUpdateBitCopyThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                       GL_SHADER_STORAGE_BARRIER_BIT);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; xfb capture -> shader write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitCaptureThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    transformFeedbackBitCaptureThenBufferWrite(writePipeline, writeResource, preBarrierOp,
                                               postBarrierOp, GL_SHADER_STORAGE_BARRIER_BIT);
}

// Test GL_SHADER_STORAGE_BARRIER_BIT; atomic write -> shader write
TEST_P(MemoryBarrierBufferOnlyTest, ShaderStorageBitAtomicThenWrite)
{
    ShaderWritePipeline writePipeline;
    WriteResource writeResource;
    NoopOp preBarrierOp;
    NoopOp postBarrierOp;
    ParseMemoryBarrierVariationsTestParams(GetParam(), &writePipeline, &writeResource,
                                           &preBarrierOp, &postBarrierOp);

    ANGLE_SKIP_TEST_IF(!hasExtensions(writeResource));

    atomicCounterBitAtomicThenBufferWrite(writePipeline, writeResource, preBarrierOp, postBarrierOp,
                                          GL_SHADER_STORAGE_BARRIER_BIT);
}

constexpr ShaderWritePipeline kWritePipelines[] = {
    ShaderWritePipeline::Graphics,
    ShaderWritePipeline::Compute,
};
constexpr WriteResource kBufferWriteResources[] = {
    WriteResource::Buffer,
    WriteResource::ImageBuffer,
};
constexpr WriteResource kImageWriteResources[] = {
    WriteResource::Image,
    WriteResource::ImageBuffer,
};
constexpr WriteResource kImageBufferOnlyWriteResources[] = {
    WriteResource::ImageBuffer,
};
constexpr WriteResource kImageOnlyWriteResources[] = {
    WriteResource::Image,
};
constexpr WriteResource kBufferOnlyWriteResources[] = {
    WriteResource::Buffer,
};
constexpr NoopOp kNoopOps[] = {
    NoopOp::None,
    NoopOp::Draw,
    NoopOp::Dispatch,
};

// Note: due to large number of tests, these are only run on Vulkan and a single configuration
// (swiftshader).

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MemoryBarrierBufferTest);
ANGLE_INSTANTIATE_TEST_COMBINE_4(MemoryBarrierBufferTest,
                                 MemoryBarrierVariationsTestPrint,
                                 testing::ValuesIn(kWritePipelines),
                                 testing::ValuesIn(kBufferWriteResources),
                                 testing::ValuesIn(kNoopOps),
                                 testing::ValuesIn(kNoopOps),
                                 ES31_VULKAN_SWIFTSHADER());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MemoryBarrierImageTest);
ANGLE_INSTANTIATE_TEST_COMBINE_4(MemoryBarrierImageTest,
                                 MemoryBarrierVariationsTestPrint,
                                 testing::ValuesIn(kWritePipelines),
                                 testing::ValuesIn(kImageWriteResources),
                                 testing::ValuesIn(kNoopOps),
                                 testing::ValuesIn(kNoopOps),
                                 ES31_VULKAN_SWIFTSHADER());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MemoryBarrierImageBufferOnlyTest);
ANGLE_INSTANTIATE_TEST_COMBINE_4(MemoryBarrierImageBufferOnlyTest,
                                 MemoryBarrierVariationsTestPrint,
                                 testing::ValuesIn(kWritePipelines),
                                 testing::ValuesIn(kImageBufferOnlyWriteResources),
                                 testing::ValuesIn(kNoopOps),
                                 testing::ValuesIn(kNoopOps),
                                 ES31_VULKAN_SWIFTSHADER());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MemoryBarrierImageOnlyTest);
ANGLE_INSTANTIATE_TEST_COMBINE_4(MemoryBarrierImageOnlyTest,
                                 MemoryBarrierVariationsTestPrint,
                                 testing::ValuesIn(kWritePipelines),
                                 testing::ValuesIn(kImageOnlyWriteResources),
                                 testing::ValuesIn(kNoopOps),
                                 testing::ValuesIn(kNoopOps),
                                 ES31_VULKAN_SWIFTSHADER());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MemoryBarrierBufferOnlyTest);
ANGLE_INSTANTIATE_TEST_COMBINE_4(MemoryBarrierBufferOnlyTest,
                                 MemoryBarrierVariationsTestPrint,
                                 testing::ValuesIn(kWritePipelines),
                                 testing::ValuesIn(kBufferOnlyWriteResources),
                                 testing::ValuesIn(kNoopOps),
                                 testing::ValuesIn(kNoopOps),
                                 ES31_VULKAN_SWIFTSHADER());

}  // anonymous namespace
