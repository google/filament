//===- unittests/SPIRV/CodeGenSPIRVTest.cpp ---- Run CodeGenSPIRV tests ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LibTestFixture.h"

#include "gmock/gmock.h"

namespace {
using clang::spirv::LibTest;

using ::testing::ContainsRegex;

// This test is purely for demonstrating how to use `LibTest`, and does not
// test anything specific in DXC itself.
TEST_F(LibTest, SourceCodeWithoutFilePath) {
  const std::string command(R"(// RUN: %dxc -T ps_6_0 -E PSMain)");
  const std::string code = command + R"(
float4 PSMain(float4 color : COLOR) : SV_TARGET { return color; }
)";
  std::string spirv = compileCodeAndGetSpirvAsm(code);
  EXPECT_THAT(spirv, ContainsRegex("%PSMain = OpFunction"));
}

// This test demonstrates that in-memory source is transmitted
// to OpSource faithfully
TEST_F(LibTest, InlinedCodeWithDebugTest) {
  const std::string command(R"(// RUN: %dxc -T ps_6_0 -E PSMain -Zi)");
  const std::string code = command + R"(
float4 PSMain(float4 color : COLOR) : SV_TARGET { return color; }
)";
  std::string spirv = compileCodeAndGetSpirvAsm(code);
  EXPECT_THAT(
      spirv,
      ContainsRegex(
          "OpSource HLSL 600 %4 \"// RUN: %dxc -T ps_6_0 -E PSMain -Zi"));
}

// This test demonstrates that in-memory source is transmitted
// to OpDebugSource only once when there are multiple files referenced
TEST_F(LibTest, InlinedCodeWithDebugMultipleFilesTest) {
  const std::string command(
      R"(// RUN: %dxc -T vs_6_0 -E main -fspv-debug=vulkan-with-source)");
  const std::string code = command + R"(
#line 1 "otherfile.hlsl"
struct VertexOutput {
  [[vk::location(0)]] float3 Color : COLOR0;
};

float4 main(VertexOutput v) : SV_Position
{
  return float4(v.Color, 1.0);
}
)";
  std::string spirv = compileCodeAndGetSpirvAsm(code);
  EXPECT_THAT(spirv, ContainsRegex("%23 = OpString \"// RUN: %dxc -T vs_6_0 -E "
                                   "main -fspv-debug=vulkan-with-source"));
  EXPECT_THAT(spirv, ContainsRegex("DebugSource %5 %23\n"));
  EXPECT_THAT(spirv, ContainsRegex("DebugSource %28\n"));
}
} // namespace
