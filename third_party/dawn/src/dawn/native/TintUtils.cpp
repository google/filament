// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/native/TintUtils.h"

#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/Device.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/RenderPipeline.h"

#include "tint/tint.h"

namespace dawn::native {

namespace {

thread_local DeviceBase* tlDevice = nullptr;

void TintICEReporter(const tint::InternalCompilerError& err) {
    if (tlDevice) {
        tlDevice->HandleError(DAWN_INTERNAL_ERROR(err.Error()));
#if DAWN_ENABLE_ASSERTS
        HandleAssertionFailure(err.File(), "", err.Line(), err.Message().c_str());
#endif
    }
}

bool InitializeTintErrorReporter() {
    tint::SetInternalCompilerErrorReporter(&TintICEReporter);
    return true;
}

tint::VertexFormat ToTintVertexFormat(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8:
            return tint::VertexFormat::kUint8;
        case wgpu::VertexFormat::Uint8x2:
            return tint::VertexFormat::kUint8x2;
        case wgpu::VertexFormat::Uint8x4:
            return tint::VertexFormat::kUint8x4;
        case wgpu::VertexFormat::Sint8:
            return tint::VertexFormat::kSint8;
        case wgpu::VertexFormat::Sint8x2:
            return tint::VertexFormat::kSint8x2;
        case wgpu::VertexFormat::Sint8x4:
            return tint::VertexFormat::kSint8x4;
        case wgpu::VertexFormat::Unorm8:
            return tint::VertexFormat::kUnorm8;
        case wgpu::VertexFormat::Unorm8x2:
            return tint::VertexFormat::kUnorm8x2;
        case wgpu::VertexFormat::Unorm8x4:
            return tint::VertexFormat::kUnorm8x4;
        case wgpu::VertexFormat::Snorm8:
            return tint::VertexFormat::kSnorm8;
        case wgpu::VertexFormat::Snorm8x2:
            return tint::VertexFormat::kSnorm8x2;
        case wgpu::VertexFormat::Snorm8x4:
            return tint::VertexFormat::kSnorm8x4;
        case wgpu::VertexFormat::Uint16:
            return tint::VertexFormat::kUint16;
        case wgpu::VertexFormat::Uint16x2:
            return tint::VertexFormat::kUint16x2;
        case wgpu::VertexFormat::Uint16x4:
            return tint::VertexFormat::kUint16x4;
        case wgpu::VertexFormat::Sint16:
            return tint::VertexFormat::kSint16;
        case wgpu::VertexFormat::Sint16x2:
            return tint::VertexFormat::kSint16x2;
        case wgpu::VertexFormat::Sint16x4:
            return tint::VertexFormat::kSint16x4;
        case wgpu::VertexFormat::Unorm16:
            return tint::VertexFormat::kUnorm16;
        case wgpu::VertexFormat::Unorm16x2:
            return tint::VertexFormat::kUnorm16x2;
        case wgpu::VertexFormat::Unorm16x4:
            return tint::VertexFormat::kUnorm16x4;
        case wgpu::VertexFormat::Snorm16:
            return tint::VertexFormat::kSnorm16;
        case wgpu::VertexFormat::Snorm16x2:
            return tint::VertexFormat::kSnorm16x2;
        case wgpu::VertexFormat::Snorm16x4:
            return tint::VertexFormat::kSnorm16x4;
        case wgpu::VertexFormat::Float16:
            return tint::VertexFormat::kFloat16;
        case wgpu::VertexFormat::Float16x2:
            return tint::VertexFormat::kFloat16x2;
        case wgpu::VertexFormat::Float16x4:
            return tint::VertexFormat::kFloat16x4;
        case wgpu::VertexFormat::Float32:
            return tint::VertexFormat::kFloat32;
        case wgpu::VertexFormat::Float32x2:
            return tint::VertexFormat::kFloat32x2;
        case wgpu::VertexFormat::Float32x3:
            return tint::VertexFormat::kFloat32x3;
        case wgpu::VertexFormat::Float32x4:
            return tint::VertexFormat::kFloat32x4;
        case wgpu::VertexFormat::Uint32:
            return tint::VertexFormat::kUint32;
        case wgpu::VertexFormat::Uint32x2:
            return tint::VertexFormat::kUint32x2;
        case wgpu::VertexFormat::Uint32x3:
            return tint::VertexFormat::kUint32x3;
        case wgpu::VertexFormat::Uint32x4:
            return tint::VertexFormat::kUint32x4;
        case wgpu::VertexFormat::Sint32:
            return tint::VertexFormat::kSint32;
        case wgpu::VertexFormat::Sint32x2:
            return tint::VertexFormat::kSint32x2;
        case wgpu::VertexFormat::Sint32x3:
            return tint::VertexFormat::kSint32x3;
        case wgpu::VertexFormat::Sint32x4:
            return tint::VertexFormat::kSint32x4;
        case wgpu::VertexFormat::Unorm10_10_10_2:
            return tint::VertexFormat::kUnorm10_10_10_2;
        case wgpu::VertexFormat::Unorm8x4BGRA:
            return tint::VertexFormat::kUnorm8x4BGRA;
    }
    DAWN_UNREACHABLE();
}

tint::VertexStepMode ToTintVertexStepMode(wgpu::VertexStepMode mode) {
    switch (mode) {
        case wgpu::VertexStepMode::Vertex:
            return tint::VertexStepMode::kVertex;
        case wgpu::VertexStepMode::Instance:
            return tint::VertexStepMode::kInstance;
        case wgpu::VertexStepMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

}  // namespace

ScopedTintICEHandler::ScopedTintICEHandler(DeviceBase* device) {
    // Call tint::SetInternalCompilerErrorReporter() the first time
    // this constructor is called. Static initialization is
    // guaranteed to be thread-safe, and only occur once.
    static bool init_once_tint_error_reporter = InitializeTintErrorReporter();
    (void)init_once_tint_error_reporter;

    // Shouldn't have overlapping instances of this handler.
    DAWN_ASSERT(tlDevice == nullptr);
    tlDevice = device;
}

ScopedTintICEHandler::~ScopedTintICEHandler() {
    tlDevice = nullptr;
}

tint::VertexPullingConfig BuildVertexPullingTransformConfig(
    const RenderPipelineBase& renderPipeline,
    BindGroupIndex pullingBufferBindingSet) {
    tint::VertexPullingConfig cfg;
    cfg.pulling_group = static_cast<uint32_t>(pullingBufferBindingSet);

    cfg.vertex_state.resize(renderPipeline.GetVertexBufferCount());
    for (VertexBufferSlot slot : IterateBitSet(renderPipeline.GetVertexBuffersUsed())) {
        const VertexBufferInfo& dawnInfo = renderPipeline.GetVertexBuffer(slot);
        tint::VertexBufferLayoutDescriptor* tintInfo =
            &cfg.vertex_state[static_cast<uint8_t>(slot)];

        tintInfo->array_stride = dawnInfo.arrayStride;
        tintInfo->step_mode = ToTintVertexStepMode(dawnInfo.stepMode);
    }

    for (VertexAttributeLocation location :
         IterateBitSet(renderPipeline.GetAttributeLocationsUsed())) {
        const VertexAttributeInfo& dawnInfo = renderPipeline.GetAttribute(location);
        tint::VertexAttributeDescriptor tintInfo;
        tintInfo.format = ToTintVertexFormat(dawnInfo.format);
        tintInfo.offset = dawnInfo.offset;
        tintInfo.shader_location = static_cast<uint32_t>(static_cast<uint8_t>(location));

        uint8_t vertexBufferSlot = static_cast<uint8_t>(dawnInfo.vertexBufferSlot);
        cfg.vertex_state[vertexBufferSlot].attributes.push_back(tintInfo);
    }
    return cfg;
}

tint::ast::transform::SubstituteOverride::Config BuildSubstituteOverridesTransformConfig(
    const ProgrammableStage& stage) {
    const EntryPointMetadata& metadata = *stage.metadata;
    const auto& constants = stage.constants;

    tint::ast::transform::SubstituteOverride::Config cfg;

    for (const auto& [key, value] : constants) {
        const auto& o = metadata.overrides.at(key);
        cfg.map.insert({o.id, value});
    }

    return cfg;
}

// static
template <>
void stream::Stream<tint::Program>::Write(stream::Sink* sink, const tint::Program& p) {
#if TINT_BUILD_WGSL_WRITER
    tint::wgsl::writer::Options options{};
    StreamIn(sink, tint::wgsl::writer::Generate(p, options)->wgsl);
#else
    // TODO(crbug.com/dawn/1481): We shouldn't need to write back to WGSL if we have a CacheKey
    // built from the initial shader module input. Then, we would never need to parse the program
    // and write back out to WGSL.
    DAWN_UNREACHABLE();
#endif
}

}  // namespace dawn::native
