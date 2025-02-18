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

#include <string>

#include "gmock/gmock.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {
namespace {

// Pipeline stage

struct PipelineStageCase {
    spv::ExecutionModel model;
    bool expect_success;
    ast::PipelineStage expected;
};
inline std::ostream& operator<<(std::ostream& out, PipelineStageCase psc) {
    out << "PipelineStageCase{ spv::ExecutionModel:::" << int(psc.model)
        << " expect_success?:" << int(psc.expect_success) << " expected:" << int(psc.expected)
        << "}";
    return out;
}

class SpvPipelineStageTest : public testing::TestWithParam<PipelineStageCase> {
  public:
    SpvPipelineStageTest()
        : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    StringStream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvPipelineStageTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToPipelineStage(params.model);
    EXPECT_EQ(success_, params.expect_success);
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::StartsWith("unknown SPIR-V execution model:"));
    }
}

INSTANTIATE_TEST_SUITE_P(EnumConverterGood,
                         SpvPipelineStageTest,
                         testing::Values(PipelineStageCase{spv::ExecutionModel::Vertex, true,
                                                           ast::PipelineStage::kVertex},
                                         PipelineStageCase{spv::ExecutionModel::Fragment, true,
                                                           ast::PipelineStage::kFragment},
                                         PipelineStageCase{spv::ExecutionModel::GLCompute, true,
                                                           ast::PipelineStage::kCompute}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvPipelineStageTest,
                         testing::Values(PipelineStageCase{static_cast<spv::ExecutionModel>(9999),
                                                           false, ast::PipelineStage::kNone},
                                         PipelineStageCase{spv::ExecutionModel::TessellationControl,
                                                           false, ast::PipelineStage::kNone}));

// Storage class

struct StorageClassCase {
    spv::StorageClass sc;
    bool expect_success;
    core::AddressSpace expected;
};
inline std::ostream& operator<<(std::ostream& out, StorageClassCase scc) {
    out << "StorageClassCase{ spv::StorageClass:::" << int(scc.sc)
        << " expect_success?:" << int(scc.expect_success) << " expected:" << int(scc.expected)
        << "}";
    return out;
}

class SpvStorageClassTest : public testing::TestWithParam<StorageClassCase> {
  public:
    SpvStorageClassTest()
        : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    StringStream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvStorageClassTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToAddressSpace(params.sc);
    EXPECT_EQ(success_, params.expect_success);
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::StartsWith("unknown SPIR-V storage class: "));
    }
}

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood,
    SpvStorageClassTest,
    testing::Values(
        StorageClassCase{spv::StorageClass::Input, true, core::AddressSpace::kIn},
        StorageClassCase{spv::StorageClass::Output, true, core::AddressSpace::kOut},
        StorageClassCase{spv::StorageClass::Uniform, true, core::AddressSpace::kUniform},
        StorageClassCase{spv::StorageClass::Workgroup, true, core::AddressSpace::kWorkgroup},
        StorageClassCase{spv::StorageClass::UniformConstant, true, core::AddressSpace::kUndefined},
        StorageClassCase{spv::StorageClass::StorageBuffer, true, core::AddressSpace::kStorage},
        StorageClassCase{spv::StorageClass::Private, true, core::AddressSpace::kPrivate},
        StorageClassCase{spv::StorageClass::Function, true, core::AddressSpace::kFunction}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvStorageClassTest,
                         testing::Values(StorageClassCase{static_cast<spv::StorageClass>(9999),
                                                          false, core::AddressSpace::kUndefined}));

// Builtin

struct BuiltinCase {
    spv::BuiltIn builtin;
    bool expect_success;
    core::BuiltinValue expected;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinCase bc) {
    out << "BuiltinCase{ spv::BuiltIn::" << int(bc.builtin)
        << " expect_success?:" << int(bc.expect_success) << " expected:" << int(bc.expected) << "}";
    return out;
}

class SpvBuiltinTest : public testing::TestWithParam<BuiltinCase> {
  public:
    SpvBuiltinTest()
        : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    StringStream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvBuiltinTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToBuiltin(params.builtin);
    EXPECT_EQ(success_, params.expect_success);
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::StartsWith("unknown SPIR-V builtin: "));
    }
}

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood_Input,
    SpvBuiltinTest,
    testing::Values(
        BuiltinCase{spv::BuiltIn::Position, true, core::BuiltinValue::kPosition},
        BuiltinCase{spv::BuiltIn::InstanceIndex, true, core::BuiltinValue::kInstanceIndex},
        BuiltinCase{spv::BuiltIn::FrontFacing, true, core::BuiltinValue::kFrontFacing},
        BuiltinCase{spv::BuiltIn::FragCoord, true, core::BuiltinValue::kPosition},
        BuiltinCase{spv::BuiltIn::LocalInvocationId, true, core::BuiltinValue::kLocalInvocationId},
        BuiltinCase{spv::BuiltIn::LocalInvocationIndex, true,
                    core::BuiltinValue::kLocalInvocationIndex},
        BuiltinCase{spv::BuiltIn::GlobalInvocationId, true,
                    core::BuiltinValue::kGlobalInvocationId},
        BuiltinCase{spv::BuiltIn::NumWorkgroups, true, core::BuiltinValue::kNumWorkgroups},
        BuiltinCase{spv::BuiltIn::WorkgroupId, true, core::BuiltinValue::kWorkgroupId},
        BuiltinCase{spv::BuiltIn::SampleId, true, core::BuiltinValue::kSampleIndex},
        BuiltinCase{spv::BuiltIn::SampleMask, true, core::BuiltinValue::kSampleMask}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood_Output,
    SpvBuiltinTest,
    testing::Values(BuiltinCase{spv::BuiltIn::Position, true, core::BuiltinValue::kPosition},
                    BuiltinCase{spv::BuiltIn::FragDepth, true, core::BuiltinValue::kFragDepth},
                    BuiltinCase{spv::BuiltIn::SampleMask, true, core::BuiltinValue::kSampleMask}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvBuiltinTest,
                         testing::Values(BuiltinCase{static_cast<spv::BuiltIn>(9999), false,
                                                     core::BuiltinValue::kUndefined},
                                         BuiltinCase{static_cast<spv::BuiltIn>(9999), false,
                                                     core::BuiltinValue::kUndefined}));

// Dim

struct DimCase {
    spv::Dim dim;
    bool arrayed;
    bool expect_success;
    core::type::TextureDimension expected;
};
inline std::ostream& operator<<(std::ostream& out, DimCase dc) {
    out << "DimCase{ spv::Dim:::" << int(dc.dim) << " arrayed?:" << int(dc.arrayed)
        << " expect_success?:" << int(dc.expect_success) << " expected:" << int(dc.expected) << "}";
    return out;
}

class SpvDimTest : public testing::TestWithParam<DimCase> {
  public:
    SpvDimTest() : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    StringStream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvDimTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToDim(params.dim, params.arrayed);
    EXPECT_EQ(success_, params.expect_success);
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::HasSubstr("dimension"));
    }
}

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood,
    SpvDimTest,
    testing::Values(
        // Non-arrayed
        DimCase{spv::Dim::Dim1D, false, true, core::type::TextureDimension::k1d},
        DimCase{spv::Dim::Dim2D, false, true, core::type::TextureDimension::k2d},
        DimCase{spv::Dim::Dim3D, false, true, core::type::TextureDimension::k3d},
        DimCase{spv::Dim::Cube, false, true, core::type::TextureDimension::kCube},
        // Arrayed
        DimCase{spv::Dim::Dim2D, true, true, core::type::TextureDimension::k2dArray},
        DimCase{spv::Dim::Cube, true, true, core::type::TextureDimension::kCubeArray}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvDimTest,
    testing::Values(
        // Invalid SPIR-V dimensionality.
        DimCase{spv::Dim::Max, false, false, core::type::TextureDimension::kNone},
        DimCase{spv::Dim::Max, true, false, core::type::TextureDimension::kNone},
        // Vulkan non-arrayed dimensionalities not supported by WGSL.
        DimCase{spv::Dim::Rect, false, false, core::type::TextureDimension::kNone},
        DimCase{spv::Dim::Buffer, false, false, core::type::TextureDimension::kNone},
        DimCase{spv::Dim::SubpassData, false, false, core::type::TextureDimension::kNone},
        // Arrayed dimensionalities not supported by WGSL
        DimCase{spv::Dim::Dim3D, true, false, core::type::TextureDimension::kNone},
        DimCase{spv::Dim::Rect, true, false, core::type::TextureDimension::kNone},
        DimCase{spv::Dim::Buffer, true, false, core::type::TextureDimension::kNone},
        DimCase{spv::Dim::SubpassData, true, false, core::type::TextureDimension::kNone}));

// TexelFormat

struct TexelFormatCase {
    spv::ImageFormat format;
    bool expect_success;
    core::TexelFormat expected;
};
inline std::ostream& operator<<(std::ostream& out, TexelFormatCase ifc) {
    out << "TexelFormatCase{ spv::ImageFormat:::" << int(ifc.format)
        << " expect_success?:" << int(ifc.expect_success) << " expected:" << int(ifc.expected)
        << "}";
    return out;
}

class SpvImageFormatTest : public testing::TestWithParam<TexelFormatCase> {
  public:
    SpvImageFormatTest()
        : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    StringStream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvImageFormatTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToTexelFormat(params.format);
    EXPECT_EQ(success_, params.expect_success) << params;
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::StartsWith("invalid image format: "));
    }
}

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood,
    SpvImageFormatTest,
    testing::Values(
        // Unknown.  This is used for sampled images.
        TexelFormatCase{spv::ImageFormat::Unknown, true, core::TexelFormat::kUndefined},
        // 8 bit channels
        TexelFormatCase{spv::ImageFormat::Rgba8, true, core::TexelFormat::kRgba8Unorm},
        TexelFormatCase{spv::ImageFormat::Rgba8Snorm, true, core::TexelFormat::kRgba8Snorm},
        TexelFormatCase{spv::ImageFormat::Rgba8ui, true, core::TexelFormat::kRgba8Uint},
        TexelFormatCase{spv::ImageFormat::Rgba8i, true, core::TexelFormat::kRgba8Sint},
        // 16 bit channels
        TexelFormatCase{spv::ImageFormat::Rgba16ui, true, core::TexelFormat::kRgba16Uint},
        TexelFormatCase{spv::ImageFormat::Rgba16i, true, core::TexelFormat::kRgba16Sint},
        TexelFormatCase{spv::ImageFormat::Rgba16f, true, core::TexelFormat::kRgba16Float},
        // 32 bit channels
        // ... 1 channel
        TexelFormatCase{spv::ImageFormat::R32ui, true, core::TexelFormat::kR32Uint},
        TexelFormatCase{spv::ImageFormat::R32i, true, core::TexelFormat::kR32Sint},
        TexelFormatCase{spv::ImageFormat::R32f, true, core::TexelFormat::kR32Float},
        // ... 2 channels
        TexelFormatCase{spv::ImageFormat::Rg32ui, true, core::TexelFormat::kRg32Uint},
        TexelFormatCase{spv::ImageFormat::Rg32i, true, core::TexelFormat::kRg32Sint},
        TexelFormatCase{spv::ImageFormat::Rg32f, true, core::TexelFormat::kRg32Float},
        // ... 4 channels
        TexelFormatCase{spv::ImageFormat::Rgba32ui, true, core::TexelFormat::kRgba32Uint},
        TexelFormatCase{spv::ImageFormat::Rgba32i, true, core::TexelFormat::kRgba32Sint},
        TexelFormatCase{spv::ImageFormat::Rgba32f, true, core::TexelFormat::kRgba32Float}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvImageFormatTest,
    testing::Values(
        // Scanning in order from the SPIR-V spec.
        TexelFormatCase{spv::ImageFormat::Rg16f, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R11fG11fB10f, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R16f, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rgb10A2, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg16, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg8, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R16, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R8, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rgba16Snorm, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg16Snorm, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg8Snorm, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg16i, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg8i, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R8i, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rgb10a2ui, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg16ui, false, core::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg8ui, false, core::TexelFormat::kUndefined}));

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
