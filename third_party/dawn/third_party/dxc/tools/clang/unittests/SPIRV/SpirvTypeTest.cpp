//===- unittests/SPIRV/SpirvTypeTest.cpp - Tests For SPIR-V Type classes --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/SPIRV/SpirvType.h"
#include "SpirvTestBase.h"

using namespace clang::spirv;

namespace {

class SpirvTypeTest : public SpirvTestBase {};

TEST_F(SpirvTypeTest, VoidType) {
  VoidType t;
  EXPECT_TRUE(llvm::isa<VoidType>(t));
}

TEST_F(SpirvTypeTest, BoolType) {
  BoolType t;
  EXPECT_TRUE(llvm::isa<BoolType>(t));
}

TEST_F(SpirvTypeTest, IntType) {
  IntegerType sint16(16, true);
  IntegerType uint32(32, false);
  EXPECT_TRUE(llvm::isa<IntegerType>(sint16));
  EXPECT_TRUE(llvm::isa<IntegerType>(uint32));
  EXPECT_TRUE(llvm::isa<NumericalType>(sint16));
  EXPECT_TRUE(llvm::isa<NumericalType>(uint32));
  EXPECT_EQ(16u, sint16.getBitwidth());
  EXPECT_EQ(32u, uint32.getBitwidth());
  EXPECT_EQ(true, sint16.isSignedInt());
  EXPECT_EQ(false, uint32.isSignedInt());
}

TEST_F(SpirvTypeTest, FloatType) {
  FloatType f16(16);
  EXPECT_TRUE(llvm::isa<FloatType>(f16));
  EXPECT_TRUE(llvm::isa<NumericalType>(f16));
  EXPECT_EQ(16u, f16.getBitwidth());
}

TEST_F(SpirvTypeTest, VectorType) {
  FloatType f16(16);
  VectorType float3(&f16, 3);
  EXPECT_TRUE(llvm::isa<VectorType>(float3));
  EXPECT_EQ(&f16, float3.getElementType());
  EXPECT_EQ(3u, float3.getElementCount());
}

TEST_F(SpirvTypeTest, MatrixType) {
  FloatType f16(16);
  VectorType float3(&f16, 3);
  MatrixType mat2x3(&float3, 2);

  EXPECT_TRUE(llvm::isa<MatrixType>(mat2x3));
  EXPECT_EQ(&f16, float3.getElementType());
  EXPECT_EQ(2u, mat2x3.getVecCount());
  EXPECT_EQ(2u, mat2x3.numCols());
  EXPECT_EQ(3u, mat2x3.numRows());
}

TEST_F(SpirvTypeTest, ImageType) {
  FloatType f16(16);
  ImageType img(&f16, spv::Dim::Dim2D, ImageType::WithDepth::Yes,
                /*isArrayed*/ false, /*isMultiSampled*/ true,
                ImageType::WithSampler::No, spv::ImageFormat::R16f);

  EXPECT_TRUE(llvm::isa<ImageType>(img));
  EXPECT_EQ(&f16, img.getSampledType());
  EXPECT_EQ(spv::Dim::Dim2D, img.getDimension());
  EXPECT_EQ(ImageType::WithDepth::Yes, img.getDepth());
  EXPECT_EQ(false, img.isArrayedImage());
  EXPECT_EQ(true, img.isMSImage());
  EXPECT_EQ(ImageType::WithSampler::No, img.withSampler());
  EXPECT_EQ(spv::ImageFormat::R16f, img.getImageFormat());
  EXPECT_EQ(img.getName(), "type.2d.image");
  EXPECT_FALSE(SpirvType::isTexture(&img));
  EXPECT_TRUE(SpirvType::isRWTexture(&img));
}

TEST_F(SpirvTypeTest, SamplerType) {
  SamplerType t;
  EXPECT_TRUE(llvm::isa<SamplerType>(t));
  EXPECT_EQ(t.getName(), "type.sampler");
}

TEST_F(SpirvTypeTest, SampledImageType) {
  FloatType f16(16);
  ImageType img(&f16, spv::Dim::Dim2D, ImageType::WithDepth::Yes,
                /*isArrayed*/ false, /*isMultiSampled*/ true,
                ImageType::WithSampler::No, spv::ImageFormat::R16f);
  SampledImageType s(&img);

  EXPECT_TRUE(llvm::isa<SampledImageType>(s));
  EXPECT_EQ(s.getName(), "type.sampled.image");
  EXPECT_EQ(s.getImageType(), &img);
}

TEST_F(SpirvTypeTest, ArrayType) {
  FloatType f16(16);
  ArrayType arr5(&f16, 5, 2);
  EXPECT_TRUE(llvm::isa<ArrayType>(arr5));
  EXPECT_EQ(arr5.getElementType(), &f16);
  EXPECT_EQ(arr5.getElementCount(), 5u);
  EXPECT_TRUE(arr5.getStride().hasValue());
  EXPECT_EQ(arr5.getStride().getValue(), 2u);
}

TEST_F(SpirvTypeTest, RuntimeArrayType) {
  FloatType f16(16);
  RuntimeArrayType ra(&f16, 2);
  EXPECT_TRUE(llvm::isa<RuntimeArrayType>(ra));
  EXPECT_EQ(ra.getElementType(), &f16);
  EXPECT_TRUE(ra.getStride().hasValue());
  EXPECT_EQ(ra.getStride().getValue(), 2u);
}

TEST_F(SpirvTypeTest, StructType) {
  IntegerType int32(32, true);
  IntegerType uint32(32, false);

  StructType::FieldInfo field0(&int32, /* fieldIndex */ 0, "field1");
  StructType::FieldInfo field1(&uint32, /* fieldIndex */ 1, "field2",
                               /*offset*/ 4,
                               /*matrixStride*/ 16, /*isRowMajor*/ false);

  StructType s({field0, field1}, "some_struct", /*isReadOnly*/ true,
               StructInterfaceType::InternalStorage);

  EXPECT_TRUE(llvm::isa<StructType>(s));
  EXPECT_EQ(s.getName(), "some_struct");
  EXPECT_EQ(s.getStructName(), "some_struct");

  const auto &fields = s.getFields();
  EXPECT_EQ(2u, fields.size());
  EXPECT_EQ(fields[0], field0);
  EXPECT_EQ(fields[1], field1);
  EXPECT_TRUE(s.isReadOnly());
  EXPECT_EQ(s.getInterfaceType(), StructInterfaceType::InternalStorage);
}

TEST_F(SpirvTypeTest, SpirvPointerType) {
  FloatType f16(16);
  SpirvPointerType ptr(&f16, spv::StorageClass::UniformConstant);
  EXPECT_TRUE(llvm::isa<SpirvPointerType>(ptr));
  EXPECT_EQ(ptr.getStorageClass(), spv::StorageClass::UniformConstant);
  EXPECT_EQ(ptr.getPointeeType(), &f16);
}

TEST_F(SpirvTypeTest, FunctionType) {
  FloatType f16(16);
  IntegerType uint32(32, false);
  BoolType retType;
  FunctionType fnType(&retType, {&f16, &uint32});
  EXPECT_TRUE(llvm::isa<FunctionType>(fnType));
  EXPECT_EQ(fnType.getReturnType(), &retType);
  EXPECT_EQ(fnType.getParamTypes().size(), 2u);
  EXPECT_EQ(fnType.getParamTypes()[0], &f16);
  EXPECT_EQ(fnType.getParamTypes()[1], &uint32);
}

// TODO: Add tests for HybridTypes.

} // anonymous namespace
