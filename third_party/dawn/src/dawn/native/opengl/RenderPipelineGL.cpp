// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/opengl/RenderPipelineGL.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/Forward.h"
#include "dawn/native/opengl/PersistentPipelineStateGL.h"
#include "dawn/native/opengl/UtilsGL.h"

namespace dawn::native::opengl {

namespace {

GLenum GLPrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        case wgpu::PrimitiveTopology::PointList:
            return GL_POINTS;
        case wgpu::PrimitiveTopology::LineList:
            return GL_LINES;
        case wgpu::PrimitiveTopology::LineStrip:
            return GL_LINE_STRIP;
        case wgpu::PrimitiveTopology::TriangleList:
            return GL_TRIANGLES;
        case wgpu::PrimitiveTopology::TriangleStrip:
            return GL_TRIANGLE_STRIP;
        case wgpu::PrimitiveTopology::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MaybeError ApplyFrontFaceAndCulling(const OpenGLFunctions& gl,
                                    wgpu::FrontFace face,
                                    wgpu::CullMode mode) {
    // Note that we invert winding direction in OpenGL. Because Y axis is up in OpenGL,
    // which is different from WebGPU and other backends (Y axis is down).
    GLenum direction = (face == wgpu::FrontFace::CCW) ? GL_CW : GL_CCW;
    DAWN_GL_TRY(gl, FrontFace(direction));

    if (mode == wgpu::CullMode::None) {
        DAWN_GL_TRY(gl, Disable(GL_CULL_FACE));
    } else {
        DAWN_GL_TRY(gl, Enable(GL_CULL_FACE));

        GLenum cullMode = (mode == wgpu::CullMode::Front) ? GL_FRONT : GL_BACK;
        DAWN_GL_TRY(gl, CullFace(cullMode));
    }

    return {};
}

GLenum GLBlendFactor(wgpu::BlendFactor factor, bool alpha) {
    switch (factor) {
        case wgpu::BlendFactor::Zero:
            return GL_ZERO;
        case wgpu::BlendFactor::One:
            return GL_ONE;
        case wgpu::BlendFactor::Src:
            return GL_SRC_COLOR;
        case wgpu::BlendFactor::OneMinusSrc:
            return GL_ONE_MINUS_SRC_COLOR;
        case wgpu::BlendFactor::SrcAlpha:
            return GL_SRC_ALPHA;
        case wgpu::BlendFactor::OneMinusSrcAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        case wgpu::BlendFactor::Dst:
            return GL_DST_COLOR;
        case wgpu::BlendFactor::OneMinusDst:
            return GL_ONE_MINUS_DST_COLOR;
        case wgpu::BlendFactor::DstAlpha:
            return GL_DST_ALPHA;
        case wgpu::BlendFactor::OneMinusDstAlpha:
            return GL_ONE_MINUS_DST_ALPHA;
        case wgpu::BlendFactor::SrcAlphaSaturated:
            return GL_SRC_ALPHA_SATURATE;
        case wgpu::BlendFactor::Constant:
            return alpha ? GL_CONSTANT_ALPHA : GL_CONSTANT_COLOR;
        case wgpu::BlendFactor::OneMinusConstant:
            return alpha ? GL_ONE_MINUS_CONSTANT_ALPHA : GL_ONE_MINUS_CONSTANT_COLOR;
        case wgpu::BlendFactor::Src1:
            return GL_SRC1_COLOR;
        case wgpu::BlendFactor::OneMinusSrc1:
            return GL_ONE_MINUS_SRC1_COLOR;
        case wgpu::BlendFactor::Src1Alpha:
            return GL_SRC1_ALPHA;
        case wgpu::BlendFactor::OneMinusSrc1Alpha:
            return GL_ONE_MINUS_SRC1_ALPHA;
        case wgpu::BlendFactor::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

GLenum GLBlendMode(wgpu::BlendOperation operation) {
    switch (operation) {
        case wgpu::BlendOperation::Add:
            return GL_FUNC_ADD;
        case wgpu::BlendOperation::Subtract:
            return GL_FUNC_SUBTRACT;
        case wgpu::BlendOperation::ReverseSubtract:
            return GL_FUNC_REVERSE_SUBTRACT;
        case wgpu::BlendOperation::Min:
            return GL_MIN;
        case wgpu::BlendOperation::Max:
            return GL_MAX;
        case wgpu::BlendOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MaybeError ApplyColorState(const OpenGLFunctions& gl,
                           ColorAttachmentIndex attachment,
                           const ColorTargetState* state) {
    GLuint colorBuffer = static_cast<GLuint>(static_cast<uint8_t>(attachment));
    if (state->blend != nullptr) {
        DAWN_GL_TRY(gl, Enablei(GL_BLEND, colorBuffer));
        DAWN_GL_TRY(gl,
                    BlendEquationSeparatei(colorBuffer, GLBlendMode(state->blend->color.operation),
                                           GLBlendMode(state->blend->alpha.operation)));
        DAWN_GL_TRY(
            gl, BlendFuncSeparatei(colorBuffer, GLBlendFactor(state->blend->color.srcFactor, false),
                                   GLBlendFactor(state->blend->color.dstFactor, false),
                                   GLBlendFactor(state->blend->alpha.srcFactor, true),
                                   GLBlendFactor(state->blend->alpha.dstFactor, true)));
    } else {
        DAWN_GL_TRY(gl, Disablei(GL_BLEND, colorBuffer));
    }
    DAWN_GL_TRY(gl, ColorMaski(colorBuffer, state->writeMask & wgpu::ColorWriteMask::Red,
                               state->writeMask & wgpu::ColorWriteMask::Green,
                               state->writeMask & wgpu::ColorWriteMask::Blue,
                               state->writeMask & wgpu::ColorWriteMask::Alpha));
    return {};
}

MaybeError ApplyColorState(const OpenGLFunctions& gl, const ColorTargetState* state) {
    if (state->blend != nullptr) {
        DAWN_GL_TRY(gl, Enable(GL_BLEND));
        DAWN_GL_TRY(gl, BlendEquationSeparate(GLBlendMode(state->blend->color.operation),
                                              GLBlendMode(state->blend->alpha.operation)));
        DAWN_GL_TRY(gl, BlendFuncSeparate(GLBlendFactor(state->blend->color.srcFactor, false),
                                          GLBlendFactor(state->blend->color.dstFactor, false),
                                          GLBlendFactor(state->blend->alpha.srcFactor, true),
                                          GLBlendFactor(state->blend->alpha.dstFactor, true)));
    } else {
        DAWN_GL_TRY(gl, Disable(GL_BLEND));
    }
    DAWN_GL_TRY(gl, ColorMask(state->writeMask & wgpu::ColorWriteMask::Red,
                              state->writeMask & wgpu::ColorWriteMask::Green,
                              state->writeMask & wgpu::ColorWriteMask::Blue,
                              state->writeMask & wgpu::ColorWriteMask::Alpha));
    return {};
}

bool Equal(const BlendComponent& lhs, const BlendComponent& rhs) {
    return lhs.operation == rhs.operation && lhs.srcFactor == rhs.srcFactor &&
           lhs.dstFactor == rhs.dstFactor;
}

GLuint OpenGLStencilOperation(wgpu::StencilOperation stencilOperation) {
    switch (stencilOperation) {
        case wgpu::StencilOperation::Keep:
            return GL_KEEP;
        case wgpu::StencilOperation::Zero:
            return GL_ZERO;
        case wgpu::StencilOperation::Replace:
            return GL_REPLACE;
        case wgpu::StencilOperation::Invert:
            return GL_INVERT;
        case wgpu::StencilOperation::IncrementClamp:
            return GL_INCR;
        case wgpu::StencilOperation::DecrementClamp:
            return GL_DECR;
        case wgpu::StencilOperation::IncrementWrap:
            return GL_INCR_WRAP;
        case wgpu::StencilOperation::DecrementWrap:
            return GL_DECR_WRAP;
        case wgpu::StencilOperation::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

}  // anonymous namespace

// static
Ref<RenderPipeline> RenderPipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

RenderPipeline::RenderPipeline(Device* device,
                               const UnpackedPtr<RenderPipelineDescriptor>& descriptor)
    : RenderPipelineBase(device, descriptor),
      mVertexArrayObject(0),
      mGlPrimitiveTopology(GLPrimitiveTopology(GetPrimitiveTopology())) {}

MaybeError RenderPipeline::InitializeImpl() {
    VertexAttributeMask bgraSwizzleAttributes = {};
    for (VertexAttributeLocation i : IterateBitSet(GetAttributeLocationsUsed())) {
        bgraSwizzleAttributes.set(i, GetAttribute(i).format == wgpu::VertexFormat::Unorm8x4BGRA);
    }

    DAWN_TRY(InitializeBase(ToBackend(GetDevice())->GetGL(), ToBackend(GetLayout()), GetAllStages(),
                            UsesVertexIndex(), UsesInstanceIndex(), UsesFragDepth(),
                            bgraSwizzleAttributes));
    DAWN_TRY(CreateVAOForVertexState());
    return {};
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::DestroyImpl() {
    RenderPipelineBase::DestroyImpl();
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    DAWN_GL_TRY_IGNORE_ERRORS(gl, DeleteVertexArrays(1, &mVertexArrayObject));
    DAWN_GL_TRY_IGNORE_ERRORS(gl, BindVertexArray(0));
    DeleteProgram(gl);
}

GLenum RenderPipeline::GetGLPrimitiveTopology() const {
    return mGlPrimitiveTopology;
}

VertexAttributeMask RenderPipeline::GetAttributesUsingVertexBuffer(VertexBufferSlot slot) const {
    DAWN_ASSERT(!IsError());
    return mAttributesUsingVertexBuffer[slot];
}

MaybeError RenderPipeline::CreateVAOForVertexState() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    DAWN_GL_TRY(gl, GenVertexArrays(1, &mVertexArrayObject));
    DAWN_GL_TRY(gl, BindVertexArray(mVertexArrayObject));

    for (VertexAttributeLocation location : IterateBitSet(GetAttributeLocationsUsed())) {
        const auto& attribute = GetAttribute(location);
        GLuint glAttrib = static_cast<GLuint>(static_cast<uint8_t>(location));
        DAWN_GL_TRY(gl, EnableVertexAttribArray(glAttrib));

        mAttributesUsingVertexBuffer[attribute.vertexBufferSlot][location] = true;
        const VertexBufferInfo& vertexBuffer = GetVertexBuffer(attribute.vertexBufferSlot);

        if (vertexBuffer.arrayStride == 0) {
            // Emulate a stride of zero (constant vertex attribute) by
            // setting the attribute instance divisor to a huge number.
            DAWN_GL_TRY(gl, VertexAttribDivisor(glAttrib, 0xffffffff));
        } else {
            switch (vertexBuffer.stepMode) {
                case wgpu::VertexStepMode::Vertex:
                    break;
                case wgpu::VertexStepMode::Instance:
                    DAWN_GL_TRY(gl, VertexAttribDivisor(glAttrib, 1));
                    break;
                case wgpu::VertexStepMode::Undefined:
                    DAWN_UNREACHABLE();
            }
        }
    }

    return {};
}

MaybeError RenderPipeline::ApplyNow(PersistentPipelineState& persistentPipelineState) {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    DAWN_TRY(PipelineGL::ApplyNow(gl));

    DAWN_ASSERT(mVertexArrayObject);
    DAWN_GL_TRY(gl, BindVertexArray(mVertexArrayObject));

    DAWN_TRY(ApplyFrontFaceAndCulling(gl, GetFrontFace(), GetCullMode()));

    DAWN_TRY(ApplyDepthStencilState(gl, &persistentPipelineState));

    DAWN_GL_TRY(gl, SampleMaski(0, GetSampleMask()));
    if (IsAlphaToCoverageEnabled()) {
        DAWN_GL_TRY(gl, Enable(GL_SAMPLE_ALPHA_TO_COVERAGE));
    } else {
        DAWN_GL_TRY(gl, Disable(GL_SAMPLE_ALPHA_TO_COVERAGE));
    }

    if (IsDepthBiasEnabled()) {
        DAWN_GL_TRY(gl, Enable(GL_POLYGON_OFFSET_FILL));
        float depthBias = GetDepthBias();
        if (GetDevice()->IsToggleEnabled(Toggle::GLDepthBiasModifier)) {
            // There is an ambiguity in the GL and Vulkan specs with respect to
            // depthBias: If a depth value lies between 2^n and 2^(n+1), is the
            // "exponent of the depth value" n or n+1? Empirically, desktop GL drivers use
            // n+1, while the WebGPU CTS is expecting n. Scaling the depth
            // bias value by 0.5 gives results in line with other backends.
            // See: https://gitlab.khronos.org/Tracker/vk-gl-cts/-/issues/4169
            // See also the GL ES 3.1 spec, section "13.5.2 Depth Offset".
            depthBias *= 0.5f;
        }
        float slopeScale = GetDepthBiasSlopeScale();
        if (gl.PolygonOffsetClamp != nullptr) {
            DAWN_GL_TRY(gl, PolygonOffsetClamp(slopeScale, depthBias, GetDepthBiasClamp()));
        } else {
            DAWN_GL_TRY(gl, PolygonOffset(slopeScale, depthBias));
        }
    } else {
        DAWN_GL_TRY(gl, Disable(GL_POLYGON_OFFSET_FILL));
    }

    if (!GetDevice()->IsToggleEnabled(Toggle::DisableIndexedDrawBuffers)) {
        for (auto attachmentSlot : IterateBitSet(GetColorAttachmentsMask())) {
            DAWN_TRY(ApplyColorState(gl, attachmentSlot, GetColorTargetState(attachmentSlot)));
        }
    } else {
        const ColorTargetState* prevDescriptor = nullptr;
        for (auto attachmentSlot : IterateBitSet(GetColorAttachmentsMask())) {
            const ColorTargetState* descriptor = GetColorTargetState(attachmentSlot);
            if (!prevDescriptor) {
                DAWN_TRY(ApplyColorState(gl, descriptor));
                prevDescriptor = descriptor;
            } else if ((descriptor->blend == nullptr) != (prevDescriptor->blend == nullptr)) {
                // TODO(crbug.com/dawn/582): GLES < 3.2 does not support different blend states
                // per color target. Add validation to prevent this as it is not.
                DAWN_ASSERT(false);
            } else if (descriptor->blend != nullptr) {
                if (!Equal(descriptor->blend->alpha, prevDescriptor->blend->alpha) ||
                    !Equal(descriptor->blend->color, prevDescriptor->blend->color) ||
                    descriptor->writeMask != prevDescriptor->writeMask) {
                    // TODO(crbug.com/dawn/582)
                    DAWN_ASSERT(false);
                }
            }
        }
    }

    return {};
}

MaybeError RenderPipeline::ApplyDepthStencilState(
    const OpenGLFunctions& gl,
    PersistentPipelineState* persistentPipelineState) {
    const DepthStencilState* descriptor = GetDepthStencilState();

    // Depth writes only occur if depth is enabled
    if (descriptor->depthCompare == wgpu::CompareFunction::Always &&
        descriptor->depthWriteEnabled != wgpu::OptionalBool::True) {
        DAWN_GL_TRY(gl, Disable(GL_DEPTH_TEST));
    } else {
        DAWN_GL_TRY(gl, Enable(GL_DEPTH_TEST));
    }

    if (descriptor->depthWriteEnabled == wgpu::OptionalBool::True) {
        DAWN_GL_TRY(gl, DepthMask(GL_TRUE));
    } else {
        DAWN_GL_TRY(gl, DepthMask(GL_FALSE));
    }

    DAWN_GL_TRY(gl, DepthFunc(ToOpenGLCompareFunction(descriptor->depthCompare)));

    if (UsesStencil()) {
        DAWN_GL_TRY(gl, Enable(GL_STENCIL_TEST));
    } else {
        DAWN_GL_TRY(gl, Disable(GL_STENCIL_TEST));
    }

    GLenum backCompareFunction = ToOpenGLCompareFunction(descriptor->stencilBack.compare);
    GLenum frontCompareFunction = ToOpenGLCompareFunction(descriptor->stencilFront.compare);
    DAWN_TRY(persistentPipelineState->SetStencilFuncsAndMask(
        gl, backCompareFunction, frontCompareFunction, descriptor->stencilReadMask));

    DAWN_GL_TRY(gl,
                StencilOpSeparate(GL_BACK, OpenGLStencilOperation(descriptor->stencilBack.failOp),
                                  OpenGLStencilOperation(descriptor->stencilBack.depthFailOp),
                                  OpenGLStencilOperation(descriptor->stencilBack.passOp)));
    DAWN_GL_TRY(gl,
                StencilOpSeparate(GL_FRONT, OpenGLStencilOperation(descriptor->stencilFront.failOp),
                                  OpenGLStencilOperation(descriptor->stencilFront.depthFailOp),
                                  OpenGLStencilOperation(descriptor->stencilFront.passOp)));

    DAWN_GL_TRY(gl, StencilMask(descriptor->stencilWriteMask));

    return {};
}

}  // namespace dawn::native::opengl
