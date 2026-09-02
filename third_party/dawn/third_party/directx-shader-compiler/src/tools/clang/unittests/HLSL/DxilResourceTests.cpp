//===- unittests/DXIL/DxilResourceTests.cpp ----- Tests for DXIL Resource -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilResource.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/Test/HlslTestUtils.h"

using namespace hlsl;
using namespace hlsl::DXIL;

#ifdef _WIN32
class DxilResourceTest {
#else
class DxilResourceTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(DxilResourceTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_METHOD(KindChecks)
};

struct KindTestCase {
  ResourceKind Kind;
  bool IsTexture;
  bool IsTextureArray;
  bool IsTextureCube;
  bool IsFeedbackTexture;
  bool IsArray;
};

KindTestCase Kinds[] = {
    {ResourceKind::Invalid, false, false, false, false, false},
    {ResourceKind::Texture1D, true, false, false, false, false},
    {ResourceKind::Texture2D, true, false, false, false, false},
    {ResourceKind::Texture2DMS, true, false, false, false, false},
    {ResourceKind::Texture3D, true, false, false, false, false},
    {ResourceKind::TextureCube, true, false, true, false, false},
    {ResourceKind::Texture1DArray, true, true, false, false, true},
    {ResourceKind::Texture2DArray, true, true, false, false, true},
    {ResourceKind::Texture2DMSArray, true, true, false, false, true},
    {ResourceKind::TextureCubeArray, true, true, true, false, true},
    {ResourceKind::TypedBuffer, false, false, false, false, false},
    {ResourceKind::RawBuffer, false, false, false, false, false},
    {ResourceKind::StructuredBuffer, false, false, false, false, false},
    {ResourceKind::CBuffer, false, false, false, false, false},
    {ResourceKind::Sampler, false, false, false, false, false},
    {ResourceKind::TBuffer, false, false, false, false, false},
    {ResourceKind::RTAccelerationStructure, false, false, false, false, false},
    {ResourceKind::FeedbackTexture2D, false, false, false, true, false},
    {ResourceKind::FeedbackTexture2DArray, false, false, false, true, true}};

static_assert(sizeof(Kinds) / sizeof(KindTestCase) ==
                  (int)ResourceKind::NumEntries,
              "There is a missing resoure type in the test cases!");

TEST_F(DxilResourceTest, KindChecks) {
  for (int Idx = 0; Idx < (int)ResourceKind::NumEntries; ++Idx) {
    EXPECT_EQ(IsAnyTexture(Kinds[Idx].Kind), Kinds[Idx].IsTexture);
    EXPECT_EQ(IsAnyArrayTexture(Kinds[Idx].Kind), Kinds[Idx].IsTextureArray);
    EXPECT_EQ(IsAnyTextureCube(Kinds[Idx].Kind), Kinds[Idx].IsTextureCube);
    EXPECT_EQ(IsFeedbackTexture(Kinds[Idx].Kind), Kinds[Idx].IsFeedbackTexture);
    EXPECT_EQ(IsArrayKind(Kinds[Idx].Kind), Kinds[Idx].IsArray);

    // Also test the entries through the DxilResource class, these tests should
    // be redundant, but historically DxilResource had its own implementations.
    EXPECT_EQ(DxilResource::IsAnyTexture(Kinds[Idx].Kind),
              Kinds[Idx].IsTexture);
    EXPECT_EQ(DxilResource::IsAnyArrayTexture(Kinds[Idx].Kind),
              Kinds[Idx].IsTextureArray);
    EXPECT_EQ(DxilResource::IsAnyTextureCube(Kinds[Idx].Kind),
              Kinds[Idx].IsTextureCube);
    EXPECT_EQ(DxilResource::IsFeedbackTexture(Kinds[Idx].Kind),
              Kinds[Idx].IsFeedbackTexture);
    EXPECT_EQ(DxilResource::IsArrayKind(Kinds[Idx].Kind), Kinds[Idx].IsArray);
  }
}
