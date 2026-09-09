//===- unittests/SPIRV/SpirvContextTest.cpp ----- SpirvContext tests ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SpirvTestBase.h"

using namespace clang::spirv;

namespace {

class SpirvContextTest : public SpirvTestBase {};

TEST_F(SpirvContextTest, VoidTypeUnique) {
  SpirvContext &spvContext = getSpirvContext();
  auto *void1 = spvContext.getVoidType();
  auto *void2 = spvContext.getVoidType();
  EXPECT_EQ(void1, void2);
}

TEST_F(SpirvContextTest, BoolTypeUnique) {
  SpirvContext &spvContext = getSpirvContext();
  auto *bool1 = spvContext.getBoolType();
  auto *bool2 = spvContext.getBoolType();
  EXPECT_EQ(bool1, bool2);
}

TEST_F(SpirvContextTest, SIntTypeUnique) {
  SpirvContext &spvContext = getSpirvContext();
  EXPECT_EQ(spvContext.getSIntType(16), spvContext.getSIntType(16));
  EXPECT_EQ(spvContext.getSIntType(32), spvContext.getSIntType(32));
  EXPECT_EQ(spvContext.getSIntType(64), spvContext.getSIntType(64));
}

TEST_F(SpirvContextTest, UIntTypeUnique) {
  SpirvContext &spvContext = getSpirvContext();
  EXPECT_EQ(spvContext.getUIntType(16), spvContext.getUIntType(16));
  EXPECT_EQ(spvContext.getUIntType(32), spvContext.getUIntType(32));
  EXPECT_EQ(spvContext.getUIntType(64), spvContext.getUIntType(64));
}

TEST_F(SpirvContextTest, FloatTypeUnique) {
  SpirvContext &spvContext = getSpirvContext();
  EXPECT_EQ(spvContext.getFloatType(16), spvContext.getFloatType(16));
  EXPECT_EQ(spvContext.getFloatType(32), spvContext.getFloatType(32));
  EXPECT_EQ(spvContext.getFloatType(64), spvContext.getFloatType(64));
}

TEST_F(SpirvContextTest, VectorTypeUnique) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  EXPECT_EQ(spvContext.getVectorType(int32, 2),
            spvContext.getVectorType(int32, 2));
  EXPECT_EQ(spvContext.getVectorType(int32, 3),
            spvContext.getVectorType(int32, 3));
  EXPECT_EQ(spvContext.getVectorType(int32, 4),
            spvContext.getVectorType(int32, 4));

  EXPECT_NE(spvContext.getVectorType(int32, 4),
            spvContext.getVectorType(int32, 2));
  EXPECT_NE(spvContext.getVectorType(int32, 4),
            spvContext.getVectorType(int32, 3));
  EXPECT_NE(spvContext.getVectorType(int32, 3),
            spvContext.getVectorType(int32, 2));
}

TEST_F(SpirvContextTest, MatrixTypeUnique) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *v2i32 = spvContext.getVectorType(int32, 2);
  const auto *v3i32 = spvContext.getVectorType(int32, 3);
  const auto *v4i32 = spvContext.getVectorType(int32, 4);

  EXPECT_EQ(spvContext.getMatrixType(v2i32, 2),
            spvContext.getMatrixType(v2i32, 2));
  EXPECT_EQ(spvContext.getMatrixType(v2i32, 3),
            spvContext.getMatrixType(v2i32, 3));
  EXPECT_EQ(spvContext.getMatrixType(v2i32, 4),
            spvContext.getMatrixType(v2i32, 4));

  EXPECT_NE(spvContext.getMatrixType(v2i32, 2),
            spvContext.getMatrixType(v2i32, 3));
  EXPECT_NE(spvContext.getMatrixType(v2i32, 2),
            spvContext.getMatrixType(v2i32, 4));

  // 2x2 is not the same as 3x2 or 3x3 or 3x4
  EXPECT_NE(spvContext.getMatrixType(v2i32, 2),
            spvContext.getMatrixType(v3i32, 2));
  EXPECT_NE(spvContext.getMatrixType(v2i32, 2),
            spvContext.getMatrixType(v3i32, 3));
  EXPECT_NE(spvContext.getMatrixType(v2i32, 2),
            spvContext.getMatrixType(v3i32, 4));

  // 2x2 is not the same as 4x2 or 4x3 or 4x4
  EXPECT_NE(spvContext.getMatrixType(v2i32, 2),
            spvContext.getMatrixType(v4i32, 2));
  EXPECT_NE(spvContext.getMatrixType(v2i32, 2),
            spvContext.getMatrixType(v4i32, 3));
  EXPECT_NE(spvContext.getMatrixType(v2i32, 2),
            spvContext.getMatrixType(v4i32, 4));

  // 3x2 != 2x3.  3x4 != 4x3. 2x4 != 4x2
  EXPECT_NE(spvContext.getMatrixType(v2i32, 3),
            spvContext.getMatrixType(v3i32, 2));
  EXPECT_NE(spvContext.getMatrixType(v4i32, 3),
            spvContext.getMatrixType(v3i32, 4));
  EXPECT_NE(spvContext.getMatrixType(v2i32, 4),
            spvContext.getMatrixType(v4i32, 2));
}

TEST_F(SpirvContextTest, SamplerTypeUnique) {
  SpirvContext &spvContext = getSpirvContext();
  EXPECT_EQ(spvContext.getSamplerType(), spvContext.getSamplerType());
}

TEST_F(SpirvContextTest, ImageTypeUnique1) {
  // The only difference is the underlying type.

  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::No,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  auto *img2 = spvContext.getImageType(
      spvContext.getSIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::No,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  EXPECT_NE(img1, img2);
}

TEST_F(SpirvContextTest, ImageTypeUnique2) {
  // The only difference is the Dimension

  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim2D, ImageType::WithDepth::No,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  auto *img2 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::No,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  EXPECT_NE(img1, img2);
}

TEST_F(SpirvContextTest, ImageTypeUnique3) {
  // The only difference is the Depth

  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::No,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  auto *img2 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  EXPECT_NE(img1, img2);
}

TEST_F(SpirvContextTest, ImageTypeUnique4) {
  // The only difference is the whether it is arrayed.

  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      true, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  auto *img2 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  EXPECT_NE(img1, img2);
}

TEST_F(SpirvContextTest, ImageTypeUnique5) {
  // The only difference is the whether it is MS.

  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  auto *img2 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, true, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  EXPECT_NE(img1, img2);
}

TEST_F(SpirvContextTest, ImageTypeUnique6) {
  // The only difference is the whether it is with sampler.

  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  auto *img2 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::No, spv::ImageFormat::Unknown);

  EXPECT_NE(img1, img2);
}

TEST_F(SpirvContextTest, ImageTypeUnique7) {
  // The only difference is the image format.

  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  auto *img2 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::R32ui);

  EXPECT_NE(img1, img2);
}

TEST_F(SpirvContextTest, ImageTypeUnique8) {
  // The two image parameters are the same

  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  auto *img2 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  EXPECT_EQ(img1, img2);
}

TEST_F(SpirvContextTest, SampledImageTypeUnique1) {
  // The two images are the same. The sampled images should also match.
  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);
  auto *img2 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  EXPECT_EQ(spvContext.getSampledImageType(img1),
            spvContext.getSampledImageType(img2));
}

TEST_F(SpirvContextTest, SampledImageTypeUnique2) {
  // The two images are the same. The sampled images should also match.
  SpirvContext &spvContext = getSpirvContext();
  auto *img1 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      false, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);
  auto *img2 = spvContext.getImageType(
      spvContext.getUIntType(32), spv::Dim::Dim1D, ImageType::WithDepth::Yes,
      true, false, ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);

  EXPECT_NE(spvContext.getSampledImageType(img1),
            spvContext.getSampledImageType(img2));
}

TEST_F(SpirvContextTest, ArrayTypeUnique1) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  EXPECT_EQ(spvContext.getArrayType(int32, 5, 4),
            spvContext.getArrayType(int32, 5, 4));
  EXPECT_EQ(spvContext.getArrayType(int32, 5, llvm::None),
            spvContext.getArrayType(int32, 5, llvm::None));
}

TEST_F(SpirvContextTest, ArrayTypeUnique2) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);
  EXPECT_NE(spvContext.getArrayType(int32, 5, 4),
            spvContext.getArrayType(uint32, 5, 4));
}

TEST_F(SpirvContextTest, ArrayTypeUnique3) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  EXPECT_NE(spvContext.getArrayType(int32, 5, 4),
            spvContext.getArrayType(int32, 7, 4));
}

TEST_F(SpirvContextTest, ArrayTypeUnique4) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  EXPECT_NE(spvContext.getArrayType(int32, 5, 4),
            spvContext.getArrayType(int32, 5, 8));

  EXPECT_NE(spvContext.getArrayType(int32, 5, 4),
            spvContext.getArrayType(int32, 5, llvm::None));
}

TEST_F(SpirvContextTest, RuntimeArrayTypeUnique1) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  EXPECT_EQ(spvContext.getRuntimeArrayType(int32, 4),
            spvContext.getRuntimeArrayType(int32, 4));
  EXPECT_EQ(spvContext.getRuntimeArrayType(int32, llvm::None),
            spvContext.getRuntimeArrayType(int32, llvm::None));
}

TEST_F(SpirvContextTest, RuntimeArrayTypeUnique2) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);
  EXPECT_NE(spvContext.getRuntimeArrayType(int32, 4),
            spvContext.getRuntimeArrayType(uint32, 4));
}

TEST_F(SpirvContextTest, RuntimeArrayTypeUnique3) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  EXPECT_NE(spvContext.getRuntimeArrayType(int32, 4),
            spvContext.getRuntimeArrayType(int32, 8));

  EXPECT_NE(spvContext.getRuntimeArrayType(int32, 4),
            spvContext.getRuntimeArrayType(int32, llvm::None));
}

TEST_F(SpirvContextTest, PointerTypeUnique1) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  EXPECT_EQ(spvContext.getPointerType(int32, spv::StorageClass::Function),
            spvContext.getPointerType(int32, spv::StorageClass::Function));
}

TEST_F(SpirvContextTest, PointerTypeUnique2) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);
  EXPECT_NE(spvContext.getPointerType(int32, spv::StorageClass::Function),
            spvContext.getPointerType(uint32, spv::StorageClass::Function));
}

TEST_F(SpirvContextTest, PointerTypeUnique3) {
  SpirvContext &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  EXPECT_NE(spvContext.getPointerType(int32, spv::StorageClass::Function),
            spvContext.getPointerType(int32, spv::StorageClass::Uniform));
}

TEST_F(SpirvContextTest, FunctionTypeUnique1) {
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);
  const auto *float32 = spvContext.getFloatType(32);
  auto *fnType1 = spvContext.getFunctionType(int32, {uint32, float32});
  auto *fnType2 = spvContext.getFunctionType(int32, {uint32, float32});
  EXPECT_EQ(fnType1, fnType2);
}

TEST_F(SpirvContextTest, FunctionTypeUnique2) {
  // The number of params is different.
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);
  const auto *float32 = spvContext.getFloatType(32);
  auto *fnType1 = spvContext.getFunctionType(int32, {uint32, float32});
  auto *fnType2 = spvContext.getFunctionType(int32, {uint32});
  EXPECT_NE(fnType1, fnType2);
}

TEST_F(SpirvContextTest, FunctionTypeUnique3) {
  // Return type is different.
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);
  const auto *float32 = spvContext.getFloatType(32);
  auto *fnType1 = spvContext.getFunctionType(int32, {uint32, float32});
  auto *fnType2 = spvContext.getFunctionType(uint32, {uint32, float32});
  EXPECT_NE(fnType1, fnType2);
}

TEST_F(SpirvContextTest, FunctionTypeUnique4) {
  // Parameter kinds are different.
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);
  const auto *float32 = spvContext.getFloatType(32);
  auto *fnType1 = spvContext.getFunctionType(int32, {uint32, float32});
  auto *fnType2 = spvContext.getFunctionType(int32, {int32, float32});
  EXPECT_NE(fnType1, fnType2);
}

TEST_F(SpirvContextTest, ByteAddressBufferTypeUnique1) {
  auto &spvContext = getSpirvContext();
  const auto *type1 = spvContext.getByteAddressBufferType(false);
  const auto *type2 = spvContext.getByteAddressBufferType(false);
  EXPECT_EQ(type1, type2);
}

TEST_F(SpirvContextTest, ByteAddressBufferTypeUnique2) {
  auto &spvContext = getSpirvContext();
  const auto *type1 = spvContext.getByteAddressBufferType(false);
  const auto *type2 = spvContext.getByteAddressBufferType(true);
  EXPECT_NE(type1, type2);
}

TEST_F(SpirvContextTest, ACSBufferCounterTypeUnique) {
  auto &spvContext = getSpirvContext();
  const auto *type1 = spvContext.getACSBufferCounterType();
  const auto *type2 = spvContext.getACSBufferCounterType();
  EXPECT_EQ(type1, type2);
}

TEST_F(SpirvContextTest, StructTypeUnique1) {
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);

  const auto *type1 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  const auto *type2 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  EXPECT_EQ(type1, type2);
}

TEST_F(SpirvContextTest, StructTypeUnique2) {
  // Struct names are different
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);

  const auto *type1 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  const auto *type2 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct2", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  EXPECT_NE(type1, type2);
}

TEST_F(SpirvContextTest, StructTypeUnique3) {
  // Read-only-ness is different
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);

  const auto *type1 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  const auto *type2 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct1", /*isReadOnly*/ true, StructInterfaceType::InternalStorage);

  EXPECT_NE(type1, type2);
}

TEST_F(SpirvContextTest, StructTypeUnique4) {
  // Interface type is different
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);

  const auto *type1 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  const auto *type2 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::StorageBuffer);

  EXPECT_NE(type1, type2);
}

TEST_F(SpirvContextTest, StructTypeUnique5) {
  // Field types are different
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);

  const auto *type1 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  const auto *type2 = spvContext.getStructType(
      {StructType::FieldInfo(uint32, /* fieldIndex */ 0, "field"),
       StructType::FieldInfo(int32, /* fieldIndex */ 1, "field")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  EXPECT_NE(type1, type2);
}

TEST_F(SpirvContextTest, StructTypeUnique6) {
  // Field names are different
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);

  const auto *type1 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "sine"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  const auto *type2 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "cosine"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2")},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  EXPECT_NE(type1, type2);
}

TEST_F(SpirvContextTest, StructTypeUnique7) {
  // Fields have different offsets.
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);

  const auto *type1 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2",
                             /*offset*/ 8)},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  const auto *type2 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2",
                             /*offset*/ 4)},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  EXPECT_NE(type1, type2);
}

TEST_F(SpirvContextTest, StructTypeUnique8) {
  // Fields have different matrix strides.
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);

  const auto *type1 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2", /*offset*/ 4,
                             /*matrixStride*/ 16)},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  const auto *type2 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2", /*offset*/ 4,
                             /*matrixStride*/ 32)},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  EXPECT_NE(type1, type2);
}

TEST_F(SpirvContextTest, StructTypeUnique9) {
  // Fields have different majorness.
  auto &spvContext = getSpirvContext();
  const auto *int32 = spvContext.getSIntType(32);
  const auto *uint32 = spvContext.getUIntType(32);

  const auto *type1 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2", /*offset*/ 4,
                             /*matrixStride*/ 16, /*isRowMajor*/ false)},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  const auto *type2 = spvContext.getStructType(
      {StructType::FieldInfo(int32, /* fieldIndex */ 0, "field1"),
       StructType::FieldInfo(uint32, /* fieldIndex */ 1, "field2", /*offset*/ 4,
                             /*matrixStride*/ 16, /*isRowMajor*/ true)},
      "struct1", /*isReadOnly*/ false, StructInterfaceType::InternalStorage);

  EXPECT_NE(type1, type2);
}

} // anonymous namespace
