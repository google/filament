//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_unittests_utils.h:
//   Helpers for mocking and unit testing.

#ifndef TESTS_ANGLE_UNITTESTS_UTILS_H_
#define TESTS_ANGLE_UNITTESTS_UTILS_H_

#include "libANGLE/Surface.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/EGLImplFactory.h"
#include "libANGLE/renderer/GLImplFactory.h"

namespace rx
{

// Useful when mocking a part of the GLImplFactory class
class NullFactory : public GLImplFactory
{
  public:
    NullFactory() {}

    // Shader creation
    CompilerImpl *createCompiler() override { return nullptr; }
    ShaderImpl *createShader(const gl::ShaderState &data) override { return nullptr; }
    ProgramImpl *createProgram(const gl::ProgramState &data) override { return nullptr; }
    ProgramExecutableImpl *createProgramExecutable(const gl::ProgramExecutable *executable) override
    {
        return nullptr;
    }

    // Framebuffer creation
    FramebufferImpl *createFramebuffer(const gl::FramebufferState &data) override
    {
        return nullptr;
    }

    // Texture creation
    TextureImpl *createTexture(const gl::TextureState &data) override { return nullptr; }

    // Renderbuffer creation
    RenderbufferImpl *createRenderbuffer(const gl::RenderbufferState &state) override
    {
        return nullptr;
    }

    // Buffer creation
    BufferImpl *createBuffer(const gl::BufferState &state) override { return nullptr; }

    // Vertex Array creation
    VertexArrayImpl *createVertexArray(const gl::VertexArrayState &data) override
    {
        return nullptr;
    }

    // Query and Fence creation
    QueryImpl *createQuery(gl::QueryType type) override { return nullptr; }
    FenceNVImpl *createFenceNV() override { return nullptr; }
    SyncImpl *createSync() override { return nullptr; }

    // Transform Feedback creation
    TransformFeedbackImpl *createTransformFeedback(const gl::TransformFeedbackState &state) override
    {
        return nullptr;
    }

    // Sampler object creation
    SamplerImpl *createSampler(const gl::SamplerState &state) override { return nullptr; }

    // Program Pipeline creation
    ProgramPipelineImpl *createProgramPipeline(const gl::ProgramPipelineState &data) override
    {
        return nullptr;
    }

    SemaphoreImpl *createSemaphore() override { return nullptr; }

    OverlayImpl *createOverlay(const gl::OverlayState &state) override { return nullptr; }
};

// A class with all the factory methods mocked.
class MockGLFactory : public GLImplFactory
{
  public:
    MOCK_METHOD1(createContext, ContextImpl *(const gl::State &));
    MOCK_METHOD0(createCompiler, CompilerImpl *());
    MOCK_METHOD1(createShader, ShaderImpl *(const gl::ShaderState &));
    MOCK_METHOD1(createProgram, ProgramImpl *(const gl::ProgramState &));
    MOCK_METHOD1(createProgramExecutable, ProgramExecutableImpl *(const gl::ProgramExecutable *));
    MOCK_METHOD1(createProgramPipeline, ProgramPipelineImpl *(const gl::ProgramPipelineState &));
    MOCK_METHOD1(createFramebuffer, FramebufferImpl *(const gl::FramebufferState &));
    MOCK_METHOD0(createMemoryObject, MemoryObjectImpl *());
    MOCK_METHOD1(createTexture, TextureImpl *(const gl::TextureState &));
    MOCK_METHOD1(createRenderbuffer, RenderbufferImpl *(const gl::RenderbufferState &));
    MOCK_METHOD1(createBuffer, BufferImpl *(const gl::BufferState &));
    MOCK_METHOD1(createVertexArray, VertexArrayImpl *(const gl::VertexArrayState &));
    MOCK_METHOD1(createQuery, QueryImpl *(gl::QueryType type));
    MOCK_METHOD0(createFenceNV, FenceNVImpl *());
    MOCK_METHOD0(createSync, SyncImpl *());
    MOCK_METHOD1(createTransformFeedback,
                 TransformFeedbackImpl *(const gl::TransformFeedbackState &));
    MOCK_METHOD1(createSampler, SamplerImpl *(const gl::SamplerState &));
    MOCK_METHOD0(createSemaphore, SemaphoreImpl *());
    MOCK_METHOD1(createOverlay, OverlayImpl *(const gl::OverlayState &));
};

class MockEGLFactory : public EGLImplFactory
{
  public:
    MOCK_METHOD3(createWindowSurface,
                 SurfaceImpl *(const egl::SurfaceState &,
                               EGLNativeWindowType,
                               const egl::AttributeMap &));
    MOCK_METHOD2(createPbufferSurface,
                 SurfaceImpl *(const egl::SurfaceState &, const egl::AttributeMap &));
    MOCK_METHOD4(createPbufferFromClientBuffer,
                 SurfaceImpl *(const egl::SurfaceState &,
                               EGLenum,
                               EGLClientBuffer,
                               const egl::AttributeMap &));
    MOCK_METHOD3(createPixmapSurface,
                 SurfaceImpl *(const egl::SurfaceState &,
                               NativePixmapType,
                               const egl::AttributeMap &));
    MOCK_METHOD4(createImage,
                 ImageImpl *(const egl::ImageState &,
                             const gl::Context *,
                             EGLenum,
                             const egl::AttributeMap &));
    MOCK_METHOD5(createContext,
                 ContextImpl *(const gl::State &,
                               gl::ErrorSet *,
                               const egl::Config *,
                               const gl::Context *,
                               const egl::AttributeMap &));
    MOCK_METHOD2(createStreamProducerD3DTexture,
                 StreamProducerImpl *(egl::Stream::ConsumerType, const egl::AttributeMap &));
    MOCK_METHOD1(createShareGroup, ShareGroupImpl *(const egl::ShareGroupState &));
};

}  // namespace rx

#endif  // TESTS_ANGLE_UNITTESTS_UTILS_H_
