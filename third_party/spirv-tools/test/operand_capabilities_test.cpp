// Copyright (c) 2015-2016 The Khronos Group Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Test capability dependencies for enums.

#include <tuple>
#include <vector>

#include "gmock/gmock.h"
#include "source/enum_set.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using spvtest::ElementsIn;
using ::testing::Combine;
using ::testing::Eq;
using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

// A test case for mapping an enum to a capability mask.
struct EnumCapabilityCase {
  spv_operand_type_t type;
  uint32_t value;
  CapabilitySet expected_capabilities;
};

// Test fixture for testing EnumCapabilityCases.
using EnumCapabilityTest =
    TestWithParam<std::tuple<spv_target_env, EnumCapabilityCase>>;

TEST_P(EnumCapabilityTest, Sample) {
  const auto env = std::get<0>(GetParam());
  const auto context = spvContextCreate(env);
  const AssemblyGrammar grammar(context);
  spv_operand_desc entry;

  ASSERT_EQ(SPV_SUCCESS,
            grammar.lookupOperand(std::get<1>(GetParam()).type,
                                  std::get<1>(GetParam()).value, &entry));
  const auto cap_set = grammar.filterCapsAgainstTargetEnv(
      entry->capabilities, entry->numCapabilities);

  EXPECT_THAT(ElementsIn(cap_set),
              Eq(ElementsIn(std::get<1>(GetParam()).expected_capabilities)))
      << " capability value " << std::get<1>(GetParam()).value;
  spvContextDestroy(context);
}

#define CASE0(TYPE, VALUE)                            \
  {                                                   \
    SPV_OPERAND_TYPE_##TYPE, uint32_t(spv::VALUE), {} \
  }
#define CASE1(TYPE, VALUE, CAP)                                    \
  {                                                                \
    SPV_OPERAND_TYPE_##TYPE, uint32_t(spv::VALUE), CapabilitySet { \
      spv::Capability::CAP                                         \
    }                                                              \
  }
#define CASE2(TYPE, VALUE, CAP1, CAP2)                             \
  {                                                                \
    SPV_OPERAND_TYPE_##TYPE, uint32_t(spv::VALUE), CapabilitySet { \
      spv::Capability::CAP1, spv::Capability::CAP2                 \
    }                                                              \
  }
#define CASE3(TYPE, VALUE, CAP1, CAP2, CAP3)                              \
  {                                                                       \
    SPV_OPERAND_TYPE_##TYPE, uint32_t(spv::VALUE), CapabilitySet {        \
      spv::Capability::CAP1, spv::Capability::CAP2, spv::Capability::CAP3 \
    }                                                                     \
  }
#define CASE4(TYPE, VALUE, CAP1, CAP2, CAP3, CAP4)                         \
  {                                                                        \
    SPV_OPERAND_TYPE_##TYPE, uint32_t(spv::VALUE), CapabilitySet {         \
      spv::Capability::CAP1, spv::Capability::CAP2, spv::Capability::CAP3, \
          spv::Capability::CAP4                                            \
    }                                                                      \
  }
#define CASE5(TYPE, VALUE, CAP1, CAP2, CAP3, CAP4, CAP5)                   \
  {                                                                        \
    SPV_OPERAND_TYPE_##TYPE, uint32_t(spv::VALUE), CapabilitySet {         \
      spv::Capability::CAP1, spv::Capability::CAP2, spv::Capability::CAP3, \
          spv::Capability::CAP4, spv::Capability::CAP5                     \
    }                                                                      \
  }

#define CASE6(TYPE, VALUE, CAP1, CAP2, CAP3, CAP4, CAP5, CAP6)                \
  {                                                                           \
    SPV_OPERAND_TYPE_##TYPE, uint32_t(spv::VALUE), CapabilitySet {            \
      spv::Capability::CAP1, spv::Capability::CAP2, spv::Capability::CAP3,    \
          spv::Capability::CAP4, spv::Capability::CAP5, spv::Capability::CAP6 \
    }                                                                         \
  }

// See SPIR-V Section 3.3 Execution Model
INSTANTIATE_TEST_SUITE_P(
    ExecutionModel, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(EXECUTION_MODEL, ExecutionModel::Vertex, Shader),
                CASE1(EXECUTION_MODEL, ExecutionModel::TessellationControl,
                      Tessellation),
                CASE1(EXECUTION_MODEL, ExecutionModel::TessellationEvaluation,
                      Tessellation),
                CASE1(EXECUTION_MODEL, ExecutionModel::Geometry, Geometry),
                CASE1(EXECUTION_MODEL, ExecutionModel::Fragment, Shader),
                CASE1(EXECUTION_MODEL, ExecutionModel::GLCompute, Shader),
                CASE1(EXECUTION_MODEL, ExecutionModel::Kernel, Kernel),
            })));

// See SPIR-V Section 3.4 Addressing Model
INSTANTIATE_TEST_SUITE_P(
    AddressingModel, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE0(ADDRESSING_MODEL, AddressingModel::Logical),
                CASE1(ADDRESSING_MODEL, AddressingModel::Physical32, Addresses),
                CASE1(ADDRESSING_MODEL, AddressingModel::Physical64, Addresses),
            })));

// See SPIR-V Section 3.5 Memory Model
INSTANTIATE_TEST_SUITE_P(
    MemoryModel, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(MEMORY_MODEL, MemoryModel::Simple, Shader),
                CASE1(MEMORY_MODEL, MemoryModel::GLSL450, Shader),
                CASE1(MEMORY_MODEL, MemoryModel::OpenCL, Kernel),
            })));

// See SPIR-V Section 3.6 Execution Mode
INSTANTIATE_TEST_SUITE_P(
    ExecutionMode, EnumCapabilityTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
        ValuesIn(std::vector<EnumCapabilityCase>{
            CASE1(EXECUTION_MODE, ExecutionMode::Invocations, Geometry),
            CASE1(EXECUTION_MODE, ExecutionMode::SpacingEqual, Tessellation),
            CASE1(EXECUTION_MODE, ExecutionMode::SpacingFractionalEven,
                  Tessellation),
            CASE1(EXECUTION_MODE, ExecutionMode::SpacingFractionalOdd,
                  Tessellation),
            CASE1(EXECUTION_MODE, ExecutionMode::VertexOrderCw, Tessellation),
            CASE1(EXECUTION_MODE, ExecutionMode::VertexOrderCcw, Tessellation),
            CASE1(EXECUTION_MODE, ExecutionMode::PixelCenterInteger, Shader),
            CASE1(EXECUTION_MODE, ExecutionMode::OriginUpperLeft, Shader),
            CASE1(EXECUTION_MODE, ExecutionMode::OriginLowerLeft, Shader),
            CASE1(EXECUTION_MODE, ExecutionMode::EarlyFragmentTests, Shader),
            CASE1(EXECUTION_MODE, ExecutionMode::PointMode, Tessellation),
            CASE1(EXECUTION_MODE, ExecutionMode::Xfb, TransformFeedback),
            CASE1(EXECUTION_MODE, ExecutionMode::DepthReplacing, Shader),
            CASE1(EXECUTION_MODE, ExecutionMode::DepthGreater, Shader),
            CASE1(EXECUTION_MODE, ExecutionMode::DepthLess, Shader),
            CASE1(EXECUTION_MODE, ExecutionMode::DepthUnchanged, Shader),
            CASE0(EXECUTION_MODE, ExecutionMode::LocalSize),
            CASE1(EXECUTION_MODE, ExecutionMode::LocalSizeHint, Kernel),
            CASE1(EXECUTION_MODE, ExecutionMode::InputPoints, Geometry),
            CASE1(EXECUTION_MODE, ExecutionMode::InputLines, Geometry),
            CASE1(EXECUTION_MODE, ExecutionMode::InputLinesAdjacency, Geometry),
            CASE2(EXECUTION_MODE, ExecutionMode::Triangles, Geometry,
                  Tessellation),
            CASE1(EXECUTION_MODE, ExecutionMode::InputTrianglesAdjacency,
                  Geometry),
            CASE1(EXECUTION_MODE, ExecutionMode::Quads, Tessellation),
            CASE1(EXECUTION_MODE, ExecutionMode::Isolines, Tessellation),
            CASE4(EXECUTION_MODE, ExecutionMode::OutputVertices, Geometry,
                  Tessellation, MeshShadingNV, MeshShadingEXT),
            CASE3(EXECUTION_MODE, ExecutionMode::OutputPoints, Geometry,
                  MeshShadingNV, MeshShadingEXT),
            CASE1(EXECUTION_MODE, ExecutionMode::OutputLineStrip, Geometry),
            CASE1(EXECUTION_MODE, ExecutionMode::OutputTriangleStrip, Geometry),
            CASE1(EXECUTION_MODE, ExecutionMode::VecTypeHint, Kernel),
            CASE1(EXECUTION_MODE, ExecutionMode::ContractionOff, Kernel),
        })));

INSTANTIATE_TEST_SUITE_P(
    ExecutionModeV11, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(EXECUTION_MODE, ExecutionMode::Initializer, Kernel),
                CASE1(EXECUTION_MODE, ExecutionMode::Finalizer, Kernel),
                CASE1(EXECUTION_MODE, ExecutionMode::SubgroupSize,
                      SubgroupDispatch),
                CASE1(EXECUTION_MODE, ExecutionMode::SubgroupsPerWorkgroup,
                      SubgroupDispatch)})));

// See SPIR-V Section 3.7 Storage Class
INSTANTIATE_TEST_SUITE_P(
    StorageClass, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE0(STORAGE_CLASS, StorageClass::UniformConstant),
                CASE1(STORAGE_CLASS, StorageClass::Uniform, Shader),
                CASE1(STORAGE_CLASS, StorageClass::Output, Shader),
                CASE0(STORAGE_CLASS, StorageClass::Workgroup),
                CASE0(STORAGE_CLASS, StorageClass::CrossWorkgroup),
                CASE2(STORAGE_CLASS, StorageClass::Private, Shader,
                      VectorComputeINTEL),
                CASE0(STORAGE_CLASS, StorageClass::Function),
                CASE1(STORAGE_CLASS, StorageClass::Generic,
                      GenericPointer),  // Bug 14287
                CASE1(STORAGE_CLASS, StorageClass::PushConstant, Shader),
                CASE1(STORAGE_CLASS, StorageClass::AtomicCounter,
                      AtomicStorage),
                CASE0(STORAGE_CLASS, StorageClass::Image),
            })));

// See SPIR-V Section 3.8 Dim
INSTANTIATE_TEST_SUITE_P(
    Dim, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE2(DIMENSIONALITY, Dim::Dim1D, Sampled1D, Image1D),
                CASE3(DIMENSIONALITY, Dim::Dim2D, Kernel, Shader, ImageMSArray),
                CASE0(DIMENSIONALITY, Dim::Dim3D),
                CASE2(DIMENSIONALITY, Dim::Cube, Shader, ImageCubeArray),
                CASE2(DIMENSIONALITY, Dim::Rect, SampledRect, ImageRect),
                CASE2(DIMENSIONALITY, Dim::Buffer, SampledBuffer, ImageBuffer),
                CASE1(DIMENSIONALITY, Dim::SubpassData, InputAttachment),
            })));

// See SPIR-V Section 3.9 Sampler Addressing Mode
INSTANTIATE_TEST_SUITE_P(
    SamplerAddressingMode, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(SAMPLER_ADDRESSING_MODE, SamplerAddressingMode::None,
                      Kernel),
                CASE1(SAMPLER_ADDRESSING_MODE,
                      SamplerAddressingMode::ClampToEdge, Kernel),
                CASE1(SAMPLER_ADDRESSING_MODE, SamplerAddressingMode::Clamp,
                      Kernel),
                CASE1(SAMPLER_ADDRESSING_MODE, SamplerAddressingMode::Repeat,
                      Kernel),
                CASE1(SAMPLER_ADDRESSING_MODE,
                      SamplerAddressingMode::RepeatMirrored, Kernel),
            })));

// See SPIR-V Section 3.10 Sampler Filter Mode
INSTANTIATE_TEST_SUITE_P(
    SamplerFilterMode, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(SAMPLER_FILTER_MODE, SamplerFilterMode::Nearest, Kernel),
                CASE1(SAMPLER_FILTER_MODE, SamplerFilterMode::Linear, Kernel),
            })));

// See SPIR-V Section 3.11 Image Format
INSTANTIATE_TEST_SUITE_P(
    ImageFormat, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                // clang-format off
        CASE0(SAMPLER_IMAGE_FORMAT, ImageFormat::Unknown),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba32f, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba16f, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R32f, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba8, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba8Snorm, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg32f, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg16f, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R11fG11fB10f, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R16f, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba16, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgb10A2, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg16, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg8, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R16, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R8, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba16Snorm, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg16Snorm, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg8Snorm, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R16Snorm, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R8Snorm, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba32i, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba16i, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba8i, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R32i, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg32i, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg16i, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg8i, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R16i, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R8i, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba32ui, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba16ui, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba8ui, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgba8ui, Shader),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rgb10a2ui, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg32ui, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg16ui, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::Rg8ui, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R16ui, StorageImageExtendedFormats),
        CASE1(SAMPLER_IMAGE_FORMAT, ImageFormat::R8ui, StorageImageExtendedFormats),
                // clang-format on
            })));

// See SPIR-V Section 3.12 Image Channel Order
INSTANTIATE_TEST_SUITE_P(
    ImageChannelOrder, EnumCapabilityTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
        ValuesIn(std::vector<EnumCapabilityCase>{
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::R, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::A, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::RG, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::RA, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::RGB, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::RGBA, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::BGRA, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::ARGB, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::Intensity, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::Luminance, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::Rx, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::RGx, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::RGBx, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::Depth, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::DepthStencil, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::sRGB, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::sRGBx, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::sRGBA, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::sBGRA, Kernel),
            CASE1(IMAGE_CHANNEL_ORDER, ImageChannelOrder::ABGR, Kernel),
        })));

// See SPIR-V Section 3.13 Image Channel Data Type
INSTANTIATE_TEST_SUITE_P(
    ImageChannelDataType, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                // clang-format off
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::SnormInt8, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::SnormInt16, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnormInt8, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnormInt16, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnormShort565, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnormShort555, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnormInt101010, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::SignedInt8, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::SignedInt16, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::SignedInt32, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnsignedInt8, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnsignedInt16, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnsignedInt32, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::HalfFloat, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::Float, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnormInt24, Kernel),
                CASE1(IMAGE_CHANNEL_DATA_TYPE, ImageChannelDataType::UnormInt101010_2, Kernel),
                // clang-format on
            })));

// See SPIR-V Section 3.14 Image Operands
INSTANTIATE_TEST_SUITE_P(
    ImageOperands, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                // clang-format off
                CASE0(OPTIONAL_IMAGE, ImageOperandsMask::MaskNone),
                CASE1(OPTIONAL_IMAGE, ImageOperandsMask::Bias, Shader),
                CASE0(OPTIONAL_IMAGE, ImageOperandsMask::Lod),
                CASE0(OPTIONAL_IMAGE, ImageOperandsMask::Grad),
                CASE0(OPTIONAL_IMAGE, ImageOperandsMask::ConstOffset),
                CASE1(OPTIONAL_IMAGE, ImageOperandsMask::Offset, ImageGatherExtended),
                CASE1(OPTIONAL_IMAGE, ImageOperandsMask::ConstOffsets, ImageGatherExtended),
                CASE0(OPTIONAL_IMAGE, ImageOperandsMask::Sample),
                CASE1(OPTIONAL_IMAGE, ImageOperandsMask::MinLod, MinLod),
                // clang-format on
            })));

// See SPIR-V Section 3.17 Linkage Type
INSTANTIATE_TEST_SUITE_P(
    LinkageType, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(LINKAGE_TYPE, LinkageType::Export, Linkage),
                CASE1(LINKAGE_TYPE, LinkageType::Import, Linkage),
            })));

// See SPIR-V Section 3.18 Access Qualifier
INSTANTIATE_TEST_SUITE_P(
    AccessQualifier, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(ACCESS_QUALIFIER, AccessQualifier::ReadOnly, Kernel),
                CASE1(ACCESS_QUALIFIER, AccessQualifier::WriteOnly, Kernel),
                CASE1(ACCESS_QUALIFIER, AccessQualifier::ReadWrite, Kernel),
            })));

// See SPIR-V Section 3.19 Function Parameter Attribute
INSTANTIATE_TEST_SUITE_P(
    FunctionParameterAttribute, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                // clang-format off
                CASE1(FUNCTION_PARAMETER_ATTRIBUTE, FunctionParameterAttribute::Zext, Kernel),
                CASE1(FUNCTION_PARAMETER_ATTRIBUTE, FunctionParameterAttribute::Sext, Kernel),
                CASE1(FUNCTION_PARAMETER_ATTRIBUTE, FunctionParameterAttribute::ByVal, Kernel),
                CASE1(FUNCTION_PARAMETER_ATTRIBUTE, FunctionParameterAttribute::Sret, Kernel),
                CASE1(FUNCTION_PARAMETER_ATTRIBUTE, FunctionParameterAttribute::NoAlias, Kernel),
                CASE1(FUNCTION_PARAMETER_ATTRIBUTE, FunctionParameterAttribute::NoCapture, Kernel),
                CASE1(FUNCTION_PARAMETER_ATTRIBUTE, FunctionParameterAttribute::NoWrite, Kernel),
                CASE1(FUNCTION_PARAMETER_ATTRIBUTE, FunctionParameterAttribute::NoReadWrite, Kernel),
                // clang-format on
            })));

// See SPIR-V Section 3.20 Decoration
INSTANTIATE_TEST_SUITE_P(
    Decoration, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(DECORATION, Decoration::RelaxedPrecision, Shader),
                // DecorationSpecId handled below.
                CASE1(DECORATION, Decoration::Block, Shader),
                CASE1(DECORATION, Decoration::BufferBlock, Shader),
                CASE1(DECORATION, Decoration::RowMajor, Matrix),
                CASE1(DECORATION, Decoration::ColMajor, Matrix),
                CASE1(DECORATION, Decoration::ArrayStride, Shader),
                CASE1(DECORATION, Decoration::MatrixStride,
                      Matrix),  // Bug 15234
                CASE1(DECORATION, Decoration::GLSLShared, Shader),
                CASE1(DECORATION, Decoration::GLSLPacked, Shader),
                CASE1(DECORATION, Decoration::CPacked, Kernel),
                CASE0(DECORATION, Decoration::BuiltIn),  // Bug 15248
                // Value 12 placeholder
                CASE1(DECORATION, Decoration::NoPerspective, Shader),
                CASE1(DECORATION, Decoration::Flat, Shader),
                CASE1(DECORATION, Decoration::Patch, Tessellation),
                CASE1(DECORATION, Decoration::Centroid, Shader),
                CASE1(DECORATION, Decoration::Sample,
                      SampleRateShading),  // Bug 15234
                CASE1(DECORATION, Decoration::Invariant, Shader),
                CASE0(DECORATION, Decoration::Restrict),
                CASE0(DECORATION, Decoration::Aliased),
                CASE0(DECORATION, Decoration::Volatile),
                CASE1(DECORATION, Decoration::Constant, Kernel),
                CASE0(DECORATION, Decoration::Coherent),
                CASE0(DECORATION, Decoration::NonWritable),
                CASE0(DECORATION, Decoration::NonReadable),
                CASE1(DECORATION, Decoration::Uniform, Shader),
                // Value 27 is an intentional gap in the spec numbering.
                CASE1(DECORATION, Decoration::SaturatedConversion, Kernel),
                CASE1(DECORATION, Decoration::Stream, GeometryStreams),
                CASE1(DECORATION, Decoration::Location, Shader),
                CASE1(DECORATION, Decoration::Component, Shader),
                CASE1(DECORATION, Decoration::Index, Shader),
                CASE1(DECORATION, Decoration::Binding, Shader),
                CASE1(DECORATION, Decoration::DescriptorSet, Shader),
                CASE1(DECORATION, Decoration::Offset, Shader),  // Bug 15268
                CASE1(DECORATION, Decoration::XfbBuffer, TransformFeedback),
                CASE1(DECORATION, Decoration::XfbStride, TransformFeedback),
                CASE1(DECORATION, Decoration::FuncParamAttr, Kernel),
                CASE1(DECORATION, Decoration::FPFastMathMode, Kernel),
                CASE1(DECORATION, Decoration::LinkageAttributes, Linkage),
                CASE1(DECORATION, Decoration::NoContraction, Shader),
                CASE1(DECORATION, Decoration::InputAttachmentIndex,
                      InputAttachment),
                CASE1(DECORATION, Decoration::Alignment, Kernel),
            })));

#if 0
// SpecId has different requirements in v1.0 and v1.1:
INSTANTIATE_TEST_SUITE_P(DecorationSpecIdV10, EnumCapabilityTest,
                        Combine(Values(SPV_ENV_UNIVERSAL_1_0),
                                ValuesIn(std::vector<EnumCapabilityCase>{CASE1(
                                    DECORATION, DecorationSpecId, Shader)})));
#endif

INSTANTIATE_TEST_SUITE_P(
    DecorationV11, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE2(DECORATION, Decoration::SpecId, Shader, Kernel),
                CASE1(DECORATION, Decoration::MaxByteOffset, Addresses)})));

// See SPIR-V Section 3.21 BuiltIn
INSTANTIATE_TEST_SUITE_P(
    BuiltIn, EnumCapabilityTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
        ValuesIn(std::vector<EnumCapabilityCase>{
            // clang-format off
            CASE1(BUILT_IN, BuiltIn::Position, Shader),
            CASE1(BUILT_IN, BuiltIn::PointSize, Shader),
            // 2 is an intentional gap in the spec numbering.
            CASE1(BUILT_IN, BuiltIn::ClipDistance, ClipDistance),  // Bug 1407, 15234
            CASE1(BUILT_IN, BuiltIn::CullDistance, CullDistance),  // Bug 1407, 15234
            CASE1(BUILT_IN, BuiltIn::VertexId, Shader),
            CASE1(BUILT_IN, BuiltIn::InstanceId, Shader),
            CASE6(BUILT_IN, BuiltIn::PrimitiveId, Geometry, Tessellation,
                  RayTracingNV, RayTracingKHR, MeshShadingNV, MeshShadingEXT),
            CASE2(BUILT_IN, BuiltIn::InvocationId, Geometry, Tessellation),
            CASE4(BUILT_IN, BuiltIn::Layer, Geometry, ShaderViewportIndexLayerEXT, MeshShadingNV, MeshShadingEXT),
            CASE4(BUILT_IN, BuiltIn::ViewportIndex, MultiViewport, ShaderViewportIndexLayerEXT, MeshShadingNV, MeshShadingEXT),  // Bug 15234
            CASE1(BUILT_IN, BuiltIn::TessLevelOuter, Tessellation),
            CASE1(BUILT_IN, BuiltIn::TessLevelInner, Tessellation),
            CASE1(BUILT_IN, BuiltIn::TessCoord, Tessellation),
            CASE1(BUILT_IN, BuiltIn::PatchVertices, Tessellation),
            CASE1(BUILT_IN, BuiltIn::FragCoord, Shader),
            CASE1(BUILT_IN, BuiltIn::PointCoord, Shader),
            CASE1(BUILT_IN, BuiltIn::FrontFacing, Shader),
            CASE1(BUILT_IN, BuiltIn::SampleId, SampleRateShading),  // Bug 15234
            CASE1(BUILT_IN, BuiltIn::SamplePosition, SampleRateShading), // Bug 15234
            CASE1(BUILT_IN, BuiltIn::SampleMask, Shader),  // Bug 15234, Issue 182
            // Value 21 intentionally missing
            CASE1(BUILT_IN, BuiltIn::FragDepth, Shader),
            CASE1(BUILT_IN, BuiltIn::HelperInvocation, Shader),
            CASE0(BUILT_IN, BuiltIn::NumWorkgroups),
            CASE0(BUILT_IN, BuiltIn::WorkgroupSize),
            CASE0(BUILT_IN, BuiltIn::WorkgroupId),
            CASE0(BUILT_IN, BuiltIn::LocalInvocationId),
            CASE0(BUILT_IN, BuiltIn::GlobalInvocationId),
            CASE0(BUILT_IN, BuiltIn::LocalInvocationIndex),
            CASE1(BUILT_IN, BuiltIn::WorkDim, Kernel),
            CASE1(BUILT_IN, BuiltIn::GlobalSize, Kernel),
            CASE1(BUILT_IN, BuiltIn::EnqueuedWorkgroupSize, Kernel),
            CASE1(BUILT_IN, BuiltIn::GlobalOffset, Kernel),
            CASE1(BUILT_IN, BuiltIn::GlobalLinearId, Kernel),
            // Value 35 intentionally missing
            CASE2(BUILT_IN, BuiltIn::SubgroupSize, Kernel, SubgroupBallotKHR),
            CASE1(BUILT_IN, BuiltIn::SubgroupMaxSize, Kernel),
            CASE1(BUILT_IN, BuiltIn::NumSubgroups, Kernel),
            CASE1(BUILT_IN, BuiltIn::NumEnqueuedSubgroups, Kernel),
            CASE1(BUILT_IN, BuiltIn::SubgroupId, Kernel),
            CASE2(BUILT_IN, BuiltIn::SubgroupLocalInvocationId, Kernel, SubgroupBallotKHR),
            CASE1(BUILT_IN, BuiltIn::VertexIndex, Shader),
            CASE1(BUILT_IN, BuiltIn::InstanceIndex, Shader),
            // clang-format on
        })));

INSTANTIATE_TEST_SUITE_P(
    BuiltInV1_5, EnumCapabilityTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_5),
        ValuesIn(std::vector<EnumCapabilityCase>{
            // SPIR-V 1.5 adds new capabilities to enable these two builtins.
            CASE5(BUILT_IN, BuiltIn::Layer, Geometry, ShaderLayer,
                  ShaderViewportIndexLayerEXT, MeshShadingNV, MeshShadingEXT),
            CASE5(BUILT_IN, BuiltIn::ViewportIndex, MultiViewport,
                  ShaderViewportIndex, ShaderViewportIndexLayerEXT,
                  MeshShadingNV, MeshShadingEXT),
        })));

// See SPIR-V Section 3.22 Selection Control
INSTANTIATE_TEST_SUITE_P(
    SelectionControl, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE0(SELECTION_CONTROL, SelectionControlMask::MaskNone),
                CASE0(SELECTION_CONTROL, SelectionControlMask::Flatten),
                CASE0(SELECTION_CONTROL, SelectionControlMask::DontFlatten),
            })));

// See SPIR-V Section 3.23 Loop Control
INSTANTIATE_TEST_SUITE_P(
    LoopControl, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE0(LOOP_CONTROL, LoopControlMask::MaskNone),
                CASE0(LOOP_CONTROL, LoopControlMask::Unroll),
                CASE0(LOOP_CONTROL, LoopControlMask::DontUnroll),
            })));

INSTANTIATE_TEST_SUITE_P(
    LoopControlV11, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE0(LOOP_CONTROL, LoopControlMask::DependencyInfinite),
                CASE0(LOOP_CONTROL, LoopControlMask::DependencyLength),
            })));

// See SPIR-V Section 3.24 Function Control
INSTANTIATE_TEST_SUITE_P(
    FunctionControl, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE0(FUNCTION_CONTROL, FunctionControlMask::MaskNone),
                CASE0(FUNCTION_CONTROL, FunctionControlMask::Inline),
                CASE0(FUNCTION_CONTROL, FunctionControlMask::DontInline),
                CASE0(FUNCTION_CONTROL, FunctionControlMask::Pure),
                CASE0(FUNCTION_CONTROL, FunctionControlMask::Const),
            })));

// See SPIR-V Section 3.25 Memory Semantics <id>
INSTANTIATE_TEST_SUITE_P(
    MemorySemantics, EnumCapabilityTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
        ValuesIn(std::vector<EnumCapabilityCase>{
            CASE0(MEMORY_SEMANTICS_ID, MemorySemanticsMask::MaskNone),
            CASE0(MEMORY_SEMANTICS_ID, MemorySemanticsMask::Acquire),
            CASE0(MEMORY_SEMANTICS_ID, MemorySemanticsMask::Release),
            CASE0(MEMORY_SEMANTICS_ID, MemorySemanticsMask::AcquireRelease),
            CASE0(MEMORY_SEMANTICS_ID,
                  MemorySemanticsMask::SequentiallyConsistent),
            CASE1(MEMORY_SEMANTICS_ID, MemorySemanticsMask::UniformMemory,
                  Shader),
            CASE0(MEMORY_SEMANTICS_ID, MemorySemanticsMask::SubgroupMemory),
            CASE0(MEMORY_SEMANTICS_ID, MemorySemanticsMask::WorkgroupMemory),
            CASE0(MEMORY_SEMANTICS_ID,
                  MemorySemanticsMask::CrossWorkgroupMemory),
            CASE1(MEMORY_SEMANTICS_ID, MemorySemanticsMask::AtomicCounterMemory,
                  AtomicStorage),  // Bug 15234
            CASE0(MEMORY_SEMANTICS_ID, MemorySemanticsMask::ImageMemory),
        })));

// See SPIR-V Section 3.26 Memory Access
INSTANTIATE_TEST_SUITE_P(
    MemoryAccess, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE0(OPTIONAL_MEMORY_ACCESS, MemoryAccessMask::MaskNone),
                CASE0(OPTIONAL_MEMORY_ACCESS, MemoryAccessMask::Volatile),
                CASE0(OPTIONAL_MEMORY_ACCESS, MemoryAccessMask::Aligned),
                CASE0(OPTIONAL_MEMORY_ACCESS, MemoryAccessMask::Nontemporal),
            })));

// See SPIR-V Section 3.27 Scope <id>
INSTANTIATE_TEST_SUITE_P(
    Scope, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                   SPV_ENV_UNIVERSAL_1_2, SPV_ENV_UNIVERSAL_1_3),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE0(SCOPE_ID, Scope::CrossDevice),
                CASE0(SCOPE_ID, Scope::Device),
                CASE0(SCOPE_ID, Scope::Workgroup),
                CASE0(SCOPE_ID, Scope::Subgroup),
                CASE0(SCOPE_ID, Scope::Invocation),
                CASE1(SCOPE_ID, Scope::QueueFamilyKHR, VulkanMemoryModelKHR),
            })));

// See SPIR-V Section 3.28 Group Operation
INSTANTIATE_TEST_SUITE_P(
    GroupOperation, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE3(GROUP_OPERATION, GroupOperation::Reduce, Kernel,
                      GroupNonUniformArithmetic, GroupNonUniformBallot),
                CASE3(GROUP_OPERATION, GroupOperation::InclusiveScan, Kernel,
                      GroupNonUniformArithmetic, GroupNonUniformBallot),
                CASE3(GROUP_OPERATION, GroupOperation::ExclusiveScan, Kernel,
                      GroupNonUniformArithmetic, GroupNonUniformBallot),
            })));

// See SPIR-V Section 3.29 Kernel Enqueue Flags
INSTANTIATE_TEST_SUITE_P(
    KernelEnqueueFlags, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(KERNEL_ENQ_FLAGS, KernelEnqueueFlags::NoWait, Kernel),
                CASE1(KERNEL_ENQ_FLAGS, KernelEnqueueFlags::WaitKernel, Kernel),
                CASE1(KERNEL_ENQ_FLAGS, KernelEnqueueFlags::WaitWorkGroup,
                      Kernel),
            })));

// See SPIR-V Section 3.30 Kernel Profiling Info
INSTANTIATE_TEST_SUITE_P(
    KernelProfilingInfo, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE0(KERNEL_PROFILING_INFO, KernelProfilingInfoMask::MaskNone),
                CASE1(KERNEL_PROFILING_INFO,
                      KernelProfilingInfoMask::CmdExecTime, Kernel),
            })));

// See SPIR-V Section 3.31 Capability
INSTANTIATE_TEST_SUITE_P(
    CapabilityDependsOn, EnumCapabilityTest,
    Combine(
        Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1),
        ValuesIn(std::vector<EnumCapabilityCase>{
            // clang-format off
            CASE0(CAPABILITY, Capability::Matrix),
            CASE1(CAPABILITY, Capability::Shader, Matrix),
            CASE1(CAPABILITY, Capability::Geometry, Shader),
            CASE1(CAPABILITY, Capability::Tessellation, Shader),
            CASE0(CAPABILITY, Capability::Addresses),
            CASE0(CAPABILITY, Capability::Linkage),
            CASE0(CAPABILITY, Capability::Kernel),
            CASE1(CAPABILITY, Capability::Vector16, Kernel),
            CASE1(CAPABILITY, Capability::Float16Buffer, Kernel),
            CASE0(CAPABILITY, Capability::Float16),  // Bug 15234
            CASE0(CAPABILITY, Capability::Float64),
            CASE0(CAPABILITY, Capability::Int64),
            CASE1(CAPABILITY, Capability::Int64Atomics, Int64),
            CASE1(CAPABILITY, Capability::ImageBasic, Kernel),
            CASE1(CAPABILITY, Capability::ImageReadWrite, ImageBasic),
            CASE1(CAPABILITY, Capability::ImageMipmap, ImageBasic),
            // Value 16 intentionally missing.
            CASE1(CAPABILITY, Capability::Pipes, Kernel),
            CASE0(CAPABILITY, Capability::Groups),
            CASE1(CAPABILITY, Capability::DeviceEnqueue, Kernel),
            CASE1(CAPABILITY, Capability::LiteralSampler, Kernel),
            CASE1(CAPABILITY, Capability::AtomicStorage, Shader),
            CASE0(CAPABILITY, Capability::Int16),
            CASE1(CAPABILITY, Capability::TessellationPointSize, Tessellation),
            CASE1(CAPABILITY, Capability::GeometryPointSize, Geometry),
            CASE1(CAPABILITY, Capability::ImageGatherExtended, Shader),
            // Value 26 intentionally missing.
            CASE1(CAPABILITY, Capability::StorageImageMultisample, Shader),
            CASE1(CAPABILITY, Capability::UniformBufferArrayDynamicIndexing, Shader),
            CASE1(CAPABILITY, Capability::SampledImageArrayDynamicIndexing, Shader),
            CASE1(CAPABILITY, Capability::StorageBufferArrayDynamicIndexing, Shader),
            CASE1(CAPABILITY, Capability::StorageImageArrayDynamicIndexing, Shader),
            CASE1(CAPABILITY, Capability::ClipDistance, Shader),
            CASE1(CAPABILITY, Capability::CullDistance, Shader),
            CASE1(CAPABILITY, Capability::ImageCubeArray, SampledCubeArray),
            CASE1(CAPABILITY, Capability::SampleRateShading, Shader),
            CASE1(CAPABILITY, Capability::ImageRect, SampledRect),
            CASE1(CAPABILITY, Capability::SampledRect, Shader),
            CASE1(CAPABILITY, Capability::GenericPointer, Addresses),
            CASE0(CAPABILITY, Capability::Int8),
            CASE1(CAPABILITY, Capability::InputAttachment, Shader),
            CASE1(CAPABILITY, Capability::SparseResidency, Shader),
            CASE1(CAPABILITY, Capability::MinLod, Shader),
            CASE1(CAPABILITY, Capability::Image1D, Sampled1D),
            CASE1(CAPABILITY, Capability::SampledCubeArray, Shader),
            CASE1(CAPABILITY, Capability::ImageBuffer, SampledBuffer),
            CASE1(CAPABILITY, Capability::ImageMSArray, Shader),
            CASE1(CAPABILITY, Capability::StorageImageExtendedFormats, Shader),
            CASE1(CAPABILITY, Capability::ImageQuery, Shader),
            CASE1(CAPABILITY, Capability::DerivativeControl, Shader),
            CASE1(CAPABILITY, Capability::InterpolationFunction, Shader),
            CASE1(CAPABILITY, Capability::TransformFeedback, Shader),
            CASE1(CAPABILITY, Capability::GeometryStreams, Geometry),
            CASE1(CAPABILITY, Capability::StorageImageReadWithoutFormat, Shader),
            CASE1(CAPABILITY, Capability::StorageImageWriteWithoutFormat, Shader),
            CASE1(CAPABILITY, Capability::MultiViewport, Geometry),
            // clang-format on
        })));

INSTANTIATE_TEST_SUITE_P(
    CapabilityDependsOnV11, EnumCapabilityTest,
    Combine(Values(SPV_ENV_UNIVERSAL_1_1),
            ValuesIn(std::vector<EnumCapabilityCase>{
                CASE1(CAPABILITY, Capability::SubgroupDispatch, DeviceEnqueue),
                CASE1(CAPABILITY, Capability::NamedBarrier, Kernel),
                CASE1(CAPABILITY, Capability::PipeStorage, Pipes),
            })));

#undef CASE0
#undef CASE1
#undef CASE2

}  // namespace
}  // namespace spvtools
