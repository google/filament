// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/ast_parser/enum_converter.h"

#include "src/tint/lang/core/type/texture_dimension.h"

namespace tint::spirv::reader::ast_parser {

EnumConverter::EnumConverter(const FailStream& fs) : fail_stream_(fs) {}

EnumConverter::~EnumConverter() = default;

ast::PipelineStage EnumConverter::ToPipelineStage(spv::ExecutionModel model) {
    switch (model) {
        case spv::ExecutionModel::Vertex:
            return ast::PipelineStage::kVertex;
        case spv::ExecutionModel::Fragment:
            return ast::PipelineStage::kFragment;
        case spv::ExecutionModel::GLCompute:
            return ast::PipelineStage::kCompute;
        default:
            break;
    }

    Fail() << "unknown SPIR-V execution model: " << uint32_t(model);
    return ast::PipelineStage::kNone;
}

core::AddressSpace EnumConverter::ToAddressSpace(const spv::StorageClass sc) {
    switch (sc) {
        case spv::StorageClass::Input:
            return core::AddressSpace::kIn;
        case spv::StorageClass::Output:
            return core::AddressSpace::kOut;
        case spv::StorageClass::Uniform:
            return core::AddressSpace::kUniform;
        case spv::StorageClass::Workgroup:
            return core::AddressSpace::kWorkgroup;
        case spv::StorageClass::UniformConstant:
            return core::AddressSpace::kUndefined;
        case spv::StorageClass::StorageBuffer:
            return core::AddressSpace::kStorage;
        case spv::StorageClass::Private:
            return core::AddressSpace::kPrivate;
        case spv::StorageClass::Function:
            return core::AddressSpace::kFunction;
        default:
            break;
    }

    Fail() << "unknown SPIR-V storage class: " << uint32_t(sc);
    return core::AddressSpace::kUndefined;
}

core::BuiltinValue EnumConverter::ToBuiltin(spv::BuiltIn b) {
    switch (b) {
        case spv::BuiltIn::Position:
            return core::BuiltinValue::kPosition;
        case spv::BuiltIn::VertexIndex:
            return core::BuiltinValue::kVertexIndex;
        case spv::BuiltIn::InstanceIndex:
            return core::BuiltinValue::kInstanceIndex;
        case spv::BuiltIn::FrontFacing:
            return core::BuiltinValue::kFrontFacing;
        case spv::BuiltIn::FragCoord:
            return core::BuiltinValue::kPosition;
        case spv::BuiltIn::FragDepth:
            return core::BuiltinValue::kFragDepth;
        case spv::BuiltIn::LocalInvocationId:
            return core::BuiltinValue::kLocalInvocationId;
        case spv::BuiltIn::LocalInvocationIndex:
            return core::BuiltinValue::kLocalInvocationIndex;
        case spv::BuiltIn::GlobalInvocationId:
            return core::BuiltinValue::kGlobalInvocationId;
        case spv::BuiltIn::NumWorkgroups:
            return core::BuiltinValue::kNumWorkgroups;
        case spv::BuiltIn::WorkgroupId:
            return core::BuiltinValue::kWorkgroupId;
        case spv::BuiltIn::SampleId:
            return core::BuiltinValue::kSampleIndex;
        case spv::BuiltIn::SampleMask:
            return core::BuiltinValue::kSampleMask;
        case spv::BuiltIn::ClipDistance:
            return core::BuiltinValue::kClipDistances;
        default:
            break;
    }

    Fail() << "unknown SPIR-V builtin: " << uint32_t(b);
    return core::BuiltinValue::kUndefined;
}

core::type::TextureDimension EnumConverter::ToDim(spv::Dim dim, bool arrayed) {
    if (arrayed) {
        switch (dim) {
            case spv::Dim::Dim2D:
                return core::type::TextureDimension::k2dArray;
            case spv::Dim::Cube:
                return core::type::TextureDimension::kCubeArray;
            default:
                break;
        }
        Fail() << "arrayed dimension must be 2D or Cube. Got " << int(dim);
        return core::type::TextureDimension::kNone;
    }
    // Assume non-arrayed
    switch (dim) {
        case spv::Dim::Dim1D:
            return core::type::TextureDimension::k1d;
        case spv::Dim::Dim2D:
            return core::type::TextureDimension::k2d;
        case spv::Dim::Dim3D:
            return core::type::TextureDimension::k3d;
        case spv::Dim::Cube:
            return core::type::TextureDimension::kCube;
        default:
            break;
    }
    Fail() << "invalid dimension: " << int(dim);
    return core::type::TextureDimension::kNone;
}

core::TexelFormat EnumConverter::ToTexelFormat(spv::ImageFormat fmt) {
    switch (fmt) {
        case spv::ImageFormat::Unknown:
            return core::TexelFormat::kUndefined;

        // 8 bit channels
        case spv::ImageFormat::Rgba8:
            return core::TexelFormat::kRgba8Unorm;
        case spv::ImageFormat::Rgba8Snorm:
            return core::TexelFormat::kRgba8Snorm;
        case spv::ImageFormat::Rgba8ui:
            return core::TexelFormat::kRgba8Uint;
        case spv::ImageFormat::Rgba8i:
            return core::TexelFormat::kRgba8Sint;

        // 16 bit channels
        case spv::ImageFormat::Rgba16ui:
            return core::TexelFormat::kRgba16Uint;
        case spv::ImageFormat::Rgba16i:
            return core::TexelFormat::kRgba16Sint;
        case spv::ImageFormat::Rgba16f:
            return core::TexelFormat::kRgba16Float;

        // 32 bit channels
        case spv::ImageFormat::R32ui:
            return core::TexelFormat::kR32Uint;
        case spv::ImageFormat::R32i:
            return core::TexelFormat::kR32Sint;
        case spv::ImageFormat::R32f:
            return core::TexelFormat::kR32Float;
        case spv::ImageFormat::Rg32ui:
            return core::TexelFormat::kRg32Uint;
        case spv::ImageFormat::Rg32i:
            return core::TexelFormat::kRg32Sint;
        case spv::ImageFormat::Rg32f:
            return core::TexelFormat::kRg32Float;
        case spv::ImageFormat::Rgba32ui:
            return core::TexelFormat::kRgba32Uint;
        case spv::ImageFormat::Rgba32i:
            return core::TexelFormat::kRgba32Sint;
        case spv::ImageFormat::Rgba32f:
            return core::TexelFormat::kRgba32Float;
        default:
            break;
    }
    Fail() << "invalid image format: " << int(fmt);
    return core::TexelFormat::kUndefined;
}

}  // namespace tint::spirv::reader::ast_parser
