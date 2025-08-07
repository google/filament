// Copyright (c) 2025 LunarG Inc.
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

#include "gmock/gmock.h"
#include "test/opt/pass_fixture.h"

namespace spvtools {
namespace opt {
namespace {

using CanonicalizeIdsTest = PassTest<::testing::Test>;

// ported from remap.basic.everything.frag
TEST_F(CanonicalizeIdsTest, remap_basic) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %9 %11
OpExecutionMode %4 OriginUpperLeft
OpSource GLSL 450
OpName %4 "main"
OpName %9 "outf4"
OpName %11 "inf"
OpDecorate %9 Location 0
OpDecorate %11 Location 0
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeVector %6 4
%8 = OpTypePointer Output %7
%9 = OpVariable %8 Output
%10 = OpTypePointer Input %6
%11 = OpVariable %10 Input
%4 = OpFunction %2 None %3
%5 = OpLabel
%12 = OpLoad %6 %11
%13 = OpCompositeConstruct %7 %12 %12 %12 %12
OpStore %9 %13
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %5663 "main" %4539 %3773
OpExecutionMode %5663 OriginUpperLeft
OpSource GLSL 450
OpName %5663 "main"
OpName %4539 "outf4"
OpName %3773 "inf"
OpDecorate %4539 Location 0
OpDecorate %3773 Location 0
%8 = OpTypeVoid
%1282 = OpTypeFunction %8
%13 = OpTypeFloat 32
%29 = OpTypeVector %13 4
%666 = OpTypePointer Output %29
%4539 = OpVariable %666 Output
%650 = OpTypePointer Input %13
%3773 = OpVariable %650 Input
%5663 = OpFunction %8 None %1282
%24968 = OpLabel
%17486 = OpLoad %13 %3773
%17691 = OpCompositeConstruct %29 %17486 %17486 %17486 %17486
OpStore %4539 %17691
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<CanonicalizeIdsPass>(before, after, false, false);
}

// ported from remap.hlsl.sample.basic.everything.frag
TEST_F(CanonicalizeIdsTest, remap_hlsl_sample_basic) {
  const std::string before =
      R"(OpCapability Shader
OpCapability Sampled1D
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %188 %192
OpExecutionMode %4 OriginUpperLeft
OpExecutionMode %4 DepthReplacing
OpSource HLSL 500
OpName %4 "main"
OpName %8 "PS_OUTPUT"
OpMemberName %8 0 "Color"
OpMemberName %8 1 "Depth"
OpName %10 "@main("
OpName %13 "MemberTest"
OpMemberName %13 0 "Sample"
OpMemberName %13 1 "CalculateLevelOfDetail"
OpMemberName %13 2 "CalculateLevelOfDetailUnclamped"
OpMemberName %13 3 "Gather"
OpMemberName %13 4 "GetDimensions"
OpMemberName %13 5 "GetSamplePosition"
OpMemberName %13 6 "Load"
OpMemberName %13 7 "SampleBias"
OpMemberName %13 8 "SampleCmp"
OpMemberName %13 9 "SampleCmpLevelZero"
OpMemberName %13 10 "SampleGrad"
OpMemberName %13 11 "SampleLevel"
OpName %15 "mtest"
OpName %54 "txval10"
OpName %45 "g_tTex1df4"
OpName %49 "g_sSamp"
OpName %66 "txval11"
OpName %60 "g_tTex1di4"
OpName %79 "txval12"
OpName %73 "g_tTex1du4"
OpName %90 "txval20"
OpName %83 "g_tTex2df4"
OpName %101 "txval21"
OpName %94 "g_tTex2di4"
OpName %113 "txval22"
OpName %105 "g_tTex2du4"
OpName %124 "txval30"
OpName %117 "g_tTex3df4"
OpName %134 "txval31"
OpName %128 "g_tTex3di4"
OpName %147 "txval32"
OpName %138 "g_tTex3du4"
OpName %156 "txval40"
OpName %151 "g_tTexcdf4"
OpName %165 "txval41"
OpName %160 "g_tTexcdi4"
OpName %174 "txval42"
OpName %169 "g_tTexcdu4"
OpName %176 "psout"
OpName %185 "flattenTemp"
OpName %188 "@entryPointOutput.Color"
OpName %192 "@entryPointOutput.Depth"
OpDecorate %45 Binding 0
OpDecorate %45 DescriptorSet 0
OpDecorate %49 Binding 0
OpDecorate %49 DescriptorSet 0
OpDecorate %60 Binding 2
OpDecorate %60 DescriptorSet 0
OpDecorate %73 Binding 3
OpDecorate %73 DescriptorSet 0
OpDecorate %83 Binding 4
OpDecorate %83 DescriptorSet 0
OpDecorate %94 Binding 5
OpDecorate %94 DescriptorSet 0
OpDecorate %105 Binding 6
OpDecorate %105 DescriptorSet 0
OpDecorate %117 Binding 7
OpDecorate %117 DescriptorSet 0
OpDecorate %128 Binding 8
OpDecorate %128 DescriptorSet 0
OpDecorate %138 Binding 9
OpDecorate %138 DescriptorSet 0
OpDecorate %151 Binding 10
OpDecorate %151 DescriptorSet 0
OpDecorate %160 Binding 11
OpDecorate %160 DescriptorSet 0
OpDecorate %169 Binding 12
OpDecorate %169 DescriptorSet 0
OpDecorate %188 Location 0
OpDecorate %192 BuiltIn FragDepth
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeVector %6 4
%8 = OpTypeStruct %7 %6
%9 = OpTypeFunction %8
%12 = OpTypeInt 32 1
%13 = OpTypeStruct %12 %12 %12 %12 %12 %12 %12 %12 %12 %12 %12 %12
%14 = OpTypePointer Function %13
%16 = OpConstant %12 1
%17 = OpTypePointer Function %12
%19 = OpConstant %12 2
%21 = OpConstant %12 3
%23 = OpConstant %12 4
%25 = OpConstant %12 5
%27 = OpConstant %12 6
%29 = OpConstant %12 0
%31 = OpConstant %12 7
%33 = OpConstant %12 8
%35 = OpConstant %12 9
%37 = OpConstant %12 10
%39 = OpConstant %12 11
%41 = OpTypePointer Function %7
%43 = OpTypeImage %6 1D 0 0 0 1 Unknown
%44 = OpTypePointer UniformConstant %43
%45 = OpVariable %44 UniformConstant
%47 = OpTypeSampler
%48 = OpTypePointer UniformConstant %47
%49 = OpVariable %48 UniformConstant
%51 = OpTypeSampledImage %43
%53 = OpConstant %6 0.100000001
%55 = OpTypeVector %12 4
%58 = OpTypeImage %12 1D 0 0 0 1 Unknown
%59 = OpTypePointer UniformConstant %58
%60 = OpVariable %59 UniformConstant
%63 = OpTypeSampledImage %58
%65 = OpConstant %6 0.200000003
%67 = OpTypeInt 32 0
%68 = OpTypeVector %67 4
%71 = OpTypeImage %67 1D 0 0 0 1 Unknown
%72 = OpTypePointer UniformConstant %71
%73 = OpVariable %72 UniformConstant
%76 = OpTypeSampledImage %71
%78 = OpConstant %6 0.300000012
%81 = OpTypeImage %6 2D 0 0 0 1 Unknown
%82 = OpTypePointer UniformConstant %81
%83 = OpVariable %82 UniformConstant
%86 = OpTypeSampledImage %81
%88 = OpTypeVector %6 2
%89 = OpConstantComposite %88 %53 %65
%92 = OpTypeImage %12 2D 0 0 0 1 Unknown
%93 = OpTypePointer UniformConstant %92
%94 = OpVariable %93 UniformConstant
%97 = OpTypeSampledImage %92
%99 = OpConstant %6 0.400000006
%100 = OpConstantComposite %88 %78 %99
%103 = OpTypeImage %67 2D 0 0 0 1 Unknown
%104 = OpTypePointer UniformConstant %103
%105 = OpVariable %104 UniformConstant
%108 = OpTypeSampledImage %103
%110 = OpConstant %6 0.5
%111 = OpConstant %6 0.600000024
%112 = OpConstantComposite %88 %110 %111
%115 = OpTypeImage %6 3D 0 0 0 1 Unknown
%116 = OpTypePointer UniformConstant %115
%117 = OpVariable %116 UniformConstant
%120 = OpTypeSampledImage %115
%122 = OpTypeVector %6 3
%123 = OpConstantComposite %122 %53 %65 %78
%126 = OpTypeImage %12 3D 0 0 0 1 Unknown
%127 = OpTypePointer UniformConstant %126
%128 = OpVariable %127 UniformConstant
%131 = OpTypeSampledImage %126
%133 = OpConstantComposite %122 %99 %110 %111
%136 = OpTypeImage %67 3D 0 0 0 1 Unknown
%137 = OpTypePointer UniformConstant %136
%138 = OpVariable %137 UniformConstant
%141 = OpTypeSampledImage %136
%143 = OpConstant %6 0.699999988
%144 = OpConstant %6 0.800000012
%145 = OpConstant %6 0.899999976
%146 = OpConstantComposite %122 %143 %144 %145
%149 = OpTypeImage %6 Cube 0 0 0 1 Unknown
%150 = OpTypePointer UniformConstant %149
%151 = OpVariable %150 UniformConstant
%154 = OpTypeSampledImage %149
%158 = OpTypeImage %12 Cube 0 0 0 1 Unknown
%159 = OpTypePointer UniformConstant %158
%160 = OpVariable %159 UniformConstant
%163 = OpTypeSampledImage %158
%167 = OpTypeImage %67 Cube 0 0 0 1 Unknown
%168 = OpTypePointer UniformConstant %167
%169 = OpVariable %168 UniformConstant
%172 = OpTypeSampledImage %167
%175 = OpTypePointer Function %8
%177 = OpConstant %6 1
%178 = OpConstantComposite %7 %177 %177 %177 %177
%180 = OpTypePointer Function %6
%187 = OpTypePointer Output %7
%188 = OpVariable %187 Output
%191 = OpTypePointer Output %6
%192 = OpVariable %191 Output
%4 = OpFunction %2 None %3
%5 = OpLabel
%185 = OpVariable %175 Function
%186 = OpFunctionCall %8 %10
OpStore %185 %186
%189 = OpAccessChain %41 %185 %29
%190 = OpLoad %7 %189
OpStore %188 %190
%193 = OpAccessChain %180 %185 %16
%194 = OpLoad %6 %193
OpStore %192 %194
OpReturn
OpFunctionEnd
%10 = OpFunction %8 None %9
%11 = OpLabel
%15 = OpVariable %14 Function
%176 = OpVariable %175 Function
%18 = OpAccessChain %17 %15 %16
OpStore %18 %16
%20 = OpAccessChain %17 %15 %19
OpStore %20 %16
%22 = OpAccessChain %17 %15 %21
OpStore %22 %16
%24 = OpAccessChain %17 %15 %23
OpStore %24 %16
%26 = OpAccessChain %17 %15 %25
OpStore %26 %16
%28 = OpAccessChain %17 %15 %27
OpStore %28 %16
%30 = OpAccessChain %17 %15 %29
OpStore %30 %16
%32 = OpAccessChain %17 %15 %31
OpStore %32 %16
%34 = OpAccessChain %17 %15 %33
OpStore %34 %16
%36 = OpAccessChain %17 %15 %35
OpStore %36 %16
%38 = OpAccessChain %17 %15 %37
OpStore %38 %16
%40 = OpAccessChain %17 %15 %39
OpStore %40 %16
%46 = OpLoad %43 %45
%50 = OpLoad %47 %49
%52 = OpSampledImage %51 %46 %50
%54 = OpImageSampleImplicitLod %7 %52 %53
%61 = OpLoad %58 %60
%62 = OpLoad %47 %49
%64 = OpSampledImage %63 %61 %62
%66 = OpImageSampleImplicitLod %55 %64 %65
%74 = OpLoad %71 %73
%75 = OpLoad %47 %49
%77 = OpSampledImage %76 %74 %75
%79 = OpImageSampleImplicitLod %68 %77 %78
%84 = OpLoad %81 %83
%85 = OpLoad %47 %49
%87 = OpSampledImage %86 %84 %85
%90 = OpImageSampleImplicitLod %7 %87 %89
%95 = OpLoad %92 %94
%96 = OpLoad %47 %49
%98 = OpSampledImage %97 %95 %96
%101 = OpImageSampleImplicitLod %55 %98 %100
%106 = OpLoad %103 %105
%107 = OpLoad %47 %49
%109 = OpSampledImage %108 %106 %107
%113 = OpImageSampleImplicitLod %68 %109 %112
%118 = OpLoad %115 %117
%119 = OpLoad %47 %49
%121 = OpSampledImage %120 %118 %119
%124 = OpImageSampleImplicitLod %7 %121 %123
%129 = OpLoad %126 %128
%130 = OpLoad %47 %49
%132 = OpSampledImage %131 %129 %130
%134 = OpImageSampleImplicitLod %55 %132 %133
%139 = OpLoad %136 %138
%140 = OpLoad %47 %49
%142 = OpSampledImage %141 %139 %140
%147 = OpImageSampleImplicitLod %68 %142 %146
%152 = OpLoad %149 %151
%153 = OpLoad %47 %49
%155 = OpSampledImage %154 %152 %153
%156 = OpImageSampleImplicitLod %7 %155 %123
%161 = OpLoad %158 %160
%162 = OpLoad %47 %49
%164 = OpSampledImage %163 %161 %162
%165 = OpImageSampleImplicitLod %55 %164 %133
%170 = OpLoad %167 %169
%171 = OpLoad %47 %49
%173 = OpSampledImage %172 %170 %171
%174 = OpImageSampleImplicitLod %68 %173 %146
%179 = OpAccessChain %41 %176 %29
OpStore %179 %178
%181 = OpAccessChain %180 %176 %16
OpStore %181 %177
%182 = OpLoad %8 %176
OpReturnValue %182
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
OpCapability Sampled1D
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %5663 "main" %4253 %3709
OpExecutionMode %5663 OriginUpperLeft
OpExecutionMode %5663 DepthReplacing
OpSource HLSL 500
OpName %5663 "main"
OpName %1032 "PS_OUTPUT"
OpMemberName %1032 0 "Color"
OpMemberName %1032 1 "Depth"
OpName %3317 "@main("
OpName %1335 "MemberTest"
OpMemberName %1335 0 "Sample"
OpMemberName %1335 1 "CalculateLevelOfDetail"
OpMemberName %1335 2 "CalculateLevelOfDetailUnclamped"
OpMemberName %1335 3 "Gather"
OpMemberName %1335 4 "GetDimensions"
OpMemberName %1335 5 "GetSamplePosition"
OpMemberName %1335 6 "Load"
OpMemberName %1335 7 "SampleBias"
OpMemberName %1335 8 "SampleCmp"
OpMemberName %1335 9 "SampleCmpLevelZero"
OpMemberName %1335 10 "SampleGrad"
OpMemberName %1335 11 "SampleLevel"
OpName %5830 "mtest"
OpName %3559 "txval10"
OpName %4727 "g_tTex1df4"
OpName %3305 "g_sSamp"
OpName %3560 "txval11"
OpName %4743 "g_tTex1di4"
OpName %3561 "txval12"
OpName %4807 "g_tTex1du4"
OpName %4568 "txval20"
OpName %5042 "g_tTex2df4"
OpName %4569 "txval21"
OpName %5058 "g_tTex2di4"
OpName %4570 "txval22"
OpName %5122 "g_tTex2du4"
OpName %5577 "txval30"
OpName %3967 "g_tTex3df4"
OpName %5578 "txval31"
OpName %3983 "g_tTex3di4"
OpName %5579 "txval32"
OpName %4047 "g_tTex3du4"
OpName %3575 "txval40"
OpName %3789 "g_tTexcdf4"
OpName %3576 "txval41"
OpName %3805 "g_tTexcdi4"
OpName %3577 "txval42"
OpName %3869 "g_tTexcdu4"
OpName %5072 "psout"
OpName %4104 "flattenTemp"
OpName %4253 "@entryPointOutput.Color"
OpName %3709 "@entryPointOutput.Depth"
OpDecorate %4727 Binding 0
OpDecorate %4727 DescriptorSet 0
OpDecorate %3305 Binding 0
OpDecorate %3305 DescriptorSet 0
OpDecorate %4743 Binding 2
OpDecorate %4743 DescriptorSet 0
OpDecorate %4807 Binding 3
OpDecorate %4807 DescriptorSet 0
OpDecorate %5042 Binding 4
OpDecorate %5042 DescriptorSet 0
OpDecorate %5058 Binding 5
OpDecorate %5058 DescriptorSet 0
OpDecorate %5122 Binding 6
OpDecorate %5122 DescriptorSet 0
OpDecorate %3967 Binding 7
OpDecorate %3967 DescriptorSet 0
OpDecorate %3983 Binding 8
OpDecorate %3983 DescriptorSet 0
OpDecorate %4047 Binding 9
OpDecorate %4047 DescriptorSet 0
OpDecorate %3789 Binding 10
OpDecorate %3789 DescriptorSet 0
OpDecorate %3805 Binding 11
OpDecorate %3805 DescriptorSet 0
OpDecorate %3869 Binding 12
OpDecorate %3869 DescriptorSet 0
OpDecorate %4253 Location 0
OpDecorate %3709 BuiltIn FragDepth
%8 = OpTypeVoid
%1282 = OpTypeFunction %8
%13 = OpTypeFloat 32
%29 = OpTypeVector %13 4
%1032 = OpTypeStruct %29 %13
%319 = OpTypeFunction %1032
%12 = OpTypeInt 32 1
%1335 = OpTypeStruct %12 %12 %12 %12 %12 %12 %12 %12 %12 %12 %12 %12
%1972 = OpTypePointer Function %1335
%2574 = OpConstant %12 1
%649 = OpTypePointer Function %12
%2577 = OpConstant %12 2
%2580 = OpConstant %12 3
%2583 = OpConstant %12 4
%2586 = OpConstant %12 5
%2589 = OpConstant %12 6
%2571 = OpConstant %12 0
%2592 = OpConstant %12 7
%2595 = OpConstant %12 8
%2598 = OpConstant %12 9
%2601 = OpConstant %12 10
%2604 = OpConstant %12 11
%666 = OpTypePointer Function %29
%149 = OpTypeImage %13 1D 0 0 0 1 Unknown
%786 = OpTypePointer UniformConstant %149
%4727 = OpVariable %786 UniformConstant
%508 = OpTypeSampler
%1145 = OpTypePointer UniformConstant %508
%3305 = OpVariable %1145 UniformConstant
%510 = OpTypeSampledImage %149
%2935 = OpConstant %13 0.100000001
%26 = OpTypeVector %12 4
%148 = OpTypeImage %12 1D 0 0 0 1 Unknown
%785 = OpTypePointer UniformConstant %148
%4743 = OpVariable %785 UniformConstant
%511 = OpTypeSampledImage %148
%2821 = OpConstant %13 0.200000003
%11 = OpTypeInt 32 0
%23 = OpTypeVector %11 4
%147 = OpTypeImage %11 1D 0 0 0 1 Unknown
%784 = OpTypePointer UniformConstant %147
%4807 = OpVariable %784 UniformConstant
%512 = OpTypeSampledImage %147
%2151 = OpConstant %13 0.300000012
%150 = OpTypeImage %13 2D 0 0 0 1 Unknown
%787 = OpTypePointer UniformConstant %150
%5042 = OpVariable %787 UniformConstant
%513 = OpTypeSampledImage %150
%19 = OpTypeVector %13 2
%1825 = OpConstantComposite %19 %2935 %2821
%151 = OpTypeImage %12 2D 0 0 0 1 Unknown
%788 = OpTypePointer UniformConstant %151
%5058 = OpVariable %788 UniformConstant
%514 = OpTypeSampledImage %151
%2707 = OpConstant %13 0.400000006
%2028 = OpConstantComposite %19 %2151 %2707
%152 = OpTypeImage %11 2D 0 0 0 1 Unknown
%789 = OpTypePointer UniformConstant %152
%5122 = OpVariable %789 UniformConstant
%515 = OpTypeSampledImage %152
%252 = OpConstant %13 0.5
%2037 = OpConstant %13 0.600000024
%2684 = OpConstantComposite %19 %252 %2037
%153 = OpTypeImage %13 3D 0 0 0 1 Unknown
%790 = OpTypePointer UniformConstant %153
%3967 = OpVariable %790 UniformConstant
%516 = OpTypeSampledImage %153
%24 = OpTypeVector %13 3
%1660 = OpConstantComposite %24 %2935 %2821 %2151
%154 = OpTypeImage %12 3D 0 0 0 1 Unknown
%791 = OpTypePointer UniformConstant %154
%3983 = OpVariable %791 UniformConstant
%517 = OpTypeSampledImage %154
%2174 = OpConstantComposite %24 %2707 %252 %2037
%155 = OpTypeImage %11 3D 0 0 0 1 Unknown
%792 = OpTypePointer UniformConstant %155
%4047 = OpVariable %792 UniformConstant
%518 = OpTypeSampledImage %155
%808 = OpConstant %13 0.699999988
%2593 = OpConstant %13 0.800000012
%1364 = OpConstant %13 0.899999976
%2476 = OpConstantComposite %24 %808 %2593 %1364
%156 = OpTypeImage %13 Cube 0 0 0 1 Unknown
%793 = OpTypePointer UniformConstant %156
%3789 = OpVariable %793 UniformConstant
%519 = OpTypeSampledImage %156
%157 = OpTypeImage %12 Cube 0 0 0 1 Unknown
%794 = OpTypePointer UniformConstant %157
%3805 = OpVariable %794 UniformConstant
%520 = OpTypeSampledImage %157
%158 = OpTypeImage %11 Cube 0 0 0 1 Unknown
%795 = OpTypePointer UniformConstant %158
%3869 = OpVariable %795 UniformConstant
%521 = OpTypeSampledImage %158
%1669 = OpTypePointer Function %1032
%138 = OpConstant %13 1
%1284 = OpConstantComposite %29 %138 %138 %138 %138
%650 = OpTypePointer Function %13
%667 = OpTypePointer Output %29
%4253 = OpVariable %667 Output
%651 = OpTypePointer Output %13
%3709 = OpVariable %651 Output
%5663 = OpFunction %8 None %1282
%24877 = OpLabel
%4104 = OpVariable %1669 Function
%18803 = OpFunctionCall %1032 %3317
OpStore %4104 %18803
%13396 = OpAccessChain %666 %4104 %2571
%7967 = OpLoad %29 %13396
OpStore %4253 %7967
%16622 = OpAccessChain %650 %4104 %2574
%11539 = OpLoad %13 %16622
OpStore %3709 %11539
OpReturn
OpFunctionEnd
%3317 = OpFunction %1032 None %319
%12442 = OpLabel
%5830 = OpVariable %1972 Function
%5072 = OpVariable %1669 Function
%22671 = OpAccessChain %649 %5830 %2574
OpStore %22671 %2574
%20306 = OpAccessChain %649 %5830 %2577
OpStore %20306 %2574
%20307 = OpAccessChain %649 %5830 %2580
OpStore %20307 %2574
%20308 = OpAccessChain %649 %5830 %2583
OpStore %20308 %2574
%20309 = OpAccessChain %649 %5830 %2586
OpStore %20309 %2574
%20310 = OpAccessChain %649 %5830 %2589
OpStore %20310 %2574
%20311 = OpAccessChain %649 %5830 %2571
OpStore %20311 %2574
%20312 = OpAccessChain %649 %5830 %2592
OpStore %20312 %2574
%20313 = OpAccessChain %649 %5830 %2595
OpStore %20313 %2574
%20314 = OpAccessChain %649 %5830 %2598
OpStore %20314 %2574
%20315 = OpAccessChain %649 %5830 %2601
OpStore %20315 %2574
%20230 = OpAccessChain %649 %5830 %2604
OpStore %20230 %2574
%15508 = OpLoad %149 %4727
%12260 = OpLoad %508 %3305
%12514 = OpSampledImage %510 %15508 %12260
%3559 = OpImageSampleImplicitLod %29 %12514 %2935
%9477 = OpLoad %148 %4743
%16280 = OpLoad %508 %3305
%12515 = OpSampledImage %511 %9477 %16280
%3560 = OpImageSampleImplicitLod %26 %12515 %2821
%9478 = OpLoad %147 %4807
%16281 = OpLoad %508 %3305
%12516 = OpSampledImage %512 %9478 %16281
%3561 = OpImageSampleImplicitLod %23 %12516 %2151
%9479 = OpLoad %150 %5042
%16282 = OpLoad %508 %3305
%12517 = OpSampledImage %513 %9479 %16282
%4568 = OpImageSampleImplicitLod %29 %12517 %1825
%9480 = OpLoad %151 %5058
%16283 = OpLoad %508 %3305
%12518 = OpSampledImage %514 %9480 %16283
%4569 = OpImageSampleImplicitLod %26 %12518 %2028
%9481 = OpLoad %152 %5122
%16284 = OpLoad %508 %3305
%12519 = OpSampledImage %515 %9481 %16284
%4570 = OpImageSampleImplicitLod %23 %12519 %2684
%9482 = OpLoad %153 %3967
%16285 = OpLoad %508 %3305
%12520 = OpSampledImage %516 %9482 %16285
%5577 = OpImageSampleImplicitLod %29 %12520 %1660
%9483 = OpLoad %154 %3983
%16286 = OpLoad %508 %3305
%12521 = OpSampledImage %517 %9483 %16286
%5578 = OpImageSampleImplicitLod %26 %12521 %2174
%9484 = OpLoad %155 %4047
%16287 = OpLoad %508 %3305
%12522 = OpSampledImage %518 %9484 %16287
%5579 = OpImageSampleImplicitLod %23 %12522 %2476
%9485 = OpLoad %156 %3789
%16288 = OpLoad %508 %3305
%12523 = OpSampledImage %519 %9485 %16288
%3575 = OpImageSampleImplicitLod %29 %12523 %1660
%9486 = OpLoad %157 %3805
%16289 = OpLoad %508 %3305
%12524 = OpSampledImage %520 %9486 %16289
%3576 = OpImageSampleImplicitLod %26 %12524 %2174
%9487 = OpLoad %158 %3869
%16290 = OpLoad %508 %3305
%12590 = OpSampledImage %521 %9487 %16290
%3577 = OpImageSampleImplicitLod %23 %12590 %2476
%14275 = OpAccessChain %666 %5072 %2571
OpStore %14275 %1284
%20231 = OpAccessChain %650 %5072 %2574
OpStore %20231 %138
%8692 = OpLoad %1032 %5072
OpReturnValue %8692
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<CanonicalizeIdsPass>(before, after, false, false);
}

// ported from remap.hlsl.templatetypes.everything.frag
TEST_F(CanonicalizeIdsTest, remap_hlsl_templatetypes) {
  const std::string before =
      R"(OpCapability Shader
OpCapability Float64
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %153 %156
OpExecutionMode %4 OriginUpperLeft
OpSource HLSL 500
OpName %4 "main"
OpName %11 "@main(vf4;"
OpName %10 "input"
OpName %18 "r00"
OpName %21 "r01"
OpName %25 "r12"
OpName %29 "r13"
OpName %14 "r14"
OpName %35 "r15"
OpName %39 "r16"
OpName %44 "r20"
OpName %49 "r21"
OpName %53 "r22"
OpName %58 "r23"
OpName %63 "r24"
OpName %67 "r30"
OpName %72 "r31"
OpName %76 "r32"
OpName %81 "r33"
OpName %86 "r34"
OpName %90 "r40"
OpName %95 "r41"
OpName %18 "r42"
OpName %101 "r43"
OpName %106 "r44"
OpName %125 "r50"
OpName %125 "r51"
OpName %131 "r61"
OpName %137 "r62"
OpName %142 "r65"
OpName %148 "r66"
OpName %154 "input"
OpName %153 "input"
OpName %156 "@entryPointOutput"
OpName %157 "param"
OpDecorate %153 Location 0
OpDecorate %156 Location 0
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeVector %6 4
%8 = OpTypePointer Function %7
%9 = OpTypeFunction %6 %8
%14 = OpConstant %6 1
%15 = OpConstant %6 2
%16 = OpConstant %6 3
%17 = OpConstant %6 4
%18 = OpConstantComposite %7 %14 %15 %16 %17
%20 = OpConstant %6 5
%21 = OpConstantComposite %7 %15 %16 %17 %20
%22 = OpTypeBool
%25 = OpConstantFalse %22
%26 = OpTypeInt 32 1
%29 = OpConstant %26 1
%32 = OpTypeFloat 64
%35 = OpConstant %32 1
%36 = OpTypeInt 32 0
%39 = OpConstant %36 1
%40 = OpTypeVector %22 2
%43 = OpConstantTrue %22
%44 = OpConstantComposite %40 %25 %43
%45 = OpTypeVector %26 2
%48 = OpConstant %26 2
%49 = OpConstantComposite %45 %29 %48
%50 = OpTypeVector %6 2
%53 = OpConstantComposite %50 %14 %15
%54 = OpTypeVector %32 2
%57 = OpConstant %32 2
%58 = OpConstantComposite %54 %35 %57
%59 = OpTypeVector %36 2
%62 = OpConstant %36 2
%63 = OpConstantComposite %59 %39 %62
%64 = OpTypeVector %22 3
%67 = OpConstantComposite %64 %25 %43 %43
%68 = OpTypeVector %26 3
%71 = OpConstant %26 3
%72 = OpConstantComposite %68 %29 %48 %71
%73 = OpTypeVector %6 3
%76 = OpConstantComposite %73 %14 %15 %16
%77 = OpTypeVector %32 3
%80 = OpConstant %32 3
%81 = OpConstantComposite %77 %35 %57 %80
%82 = OpTypeVector %36 3
%85 = OpConstant %36 3
%86 = OpConstantComposite %82 %39 %62 %85
%87 = OpTypeVector %22 4
%90 = OpConstantComposite %87 %25 %43 %43 %25
%91 = OpTypeVector %26 4
%94 = OpConstant %26 4
%95 = OpConstantComposite %91 %29 %48 %71 %94
%97 = OpTypeVector %32 4
%100 = OpConstant %32 4
%101 = OpConstantComposite %97 %35 %57 %80 %100
%102 = OpTypeVector %36 4
%105 = OpConstant %36 4
%106 = OpConstantComposite %102 %39 %62 %85 %105
%107 = OpTypeMatrix %7 4
%110 = OpConstant %6 0
%111 = OpConstantComposite %7 %110 %14 %15 %16
%112 = OpConstant %6 6
%113 = OpConstant %6 7
%114 = OpConstantComposite %7 %17 %20 %112 %113
%115 = OpConstant %6 8
%116 = OpConstant %6 9
%117 = OpConstant %6 10
%118 = OpConstant %6 11
%119 = OpConstantComposite %7 %115 %116 %117 %118
%120 = OpConstant %6 12
%121 = OpConstant %6 13
%122 = OpConstant %6 14
%123 = OpConstant %6 15
%124 = OpConstantComposite %7 %120 %121 %122 %123
%125 = OpConstantComposite %107 %111 %114 %119 %124
%127 = OpTypeMatrix %73 2
%130 = OpConstantComposite %73 %17 %20 %112
%131 = OpConstantComposite %127 %76 %130
%132 = OpTypeMatrix %50 3
%135 = OpConstantComposite %50 %16 %17
%136 = OpConstantComposite %50 %20 %112
%137 = OpConstantComposite %132 %53 %135 %136
%138 = OpTypeMatrix %50 4
%141 = OpConstantComposite %50 %113 %115
%142 = OpConstantComposite %138 %53 %135 %136 %141
%143 = OpTypeMatrix %73 4
%146 = OpConstantComposite %73 %113 %115 %116
%147 = OpConstantComposite %73 %117 %118 %120
%148 = OpConstantComposite %143 %76 %130 %146 %147
%152 = OpTypePointer Input %7
%153 = OpVariable %152 Input
%155 = OpTypePointer Output %6
%156 = OpVariable %155 Output
%4 = OpFunction %2 None %3
%5 = OpLabel
%157 = OpVariable %8 Function
%154 = OpLoad %7 %153
OpStore %157 %154
%159 = OpFunctionCall %6 %11 %157
OpStore %156 %159
OpReturn
OpFunctionEnd
%11 = OpFunction %6 None %9
%10 = OpFunctionParameter %8
%12 = OpLabel
OpReturnValue %110
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
OpCapability Float64
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %5663 "main" %4872 %4045
OpExecutionMode %5663 OriginUpperLeft
OpSource HLSL 500
OpName %5663 "main"
OpName %3917 "@main(vf4;"
OpName %10636 "input"
OpName %1616 "r00"
OpName %590 "r01"
OpName %1927 "r12"
OpName %2574 "r13"
OpName %138 "r14"
OpName %1201 "r15"
OpName %2573 "r16"
OpName %311 "r20"
OpName %1848 "r21"
OpName %312 "r22"
OpName %490 "r23"
OpName %1840 "r24"
OpName %869 "r30"
OpName %2668 "r31"
OpName %1271 "r32"
OpName %1108 "r33"
OpName %2654 "r34"
OpName %340 "r40"
OpName %56 "r41"
OpName %1616 "r42"
OpName %1328 "r43"
OpName %35 "r44"
OpName %1294 "r50"
OpName %1294 "r51"
OpName %1207 "r61"
OpName %162 "r62"
OpName %2695 "r65"
OpName %55 "r66"
OpName %24021 "input"
OpName %4872 "input"
OpName %4045 "@entryPointOutput"
OpName %5786 "param"
OpDecorate %4872 Location 0
OpDecorate %4045 Location 0
%8 = OpTypeVoid
%1282 = OpTypeFunction %8
%13 = OpTypeFloat 32
%29 = OpTypeVector %13 4
%666 = OpTypePointer Function %29
%255 = OpTypeFunction %13 %666
%138 = OpConstant %13 1
%24 = OpConstant %13 2
%2978 = OpConstant %13 3
%2921 = OpConstant %13 4
%1616 = OpConstantComposite %29 %138 %24 %2978 %2921
%1387 = OpConstant %13 5
%590 = OpConstantComposite %29 %24 %2978 %2921 %1387
%9 = OpTypeBool
%1927 = OpConstantFalse %9
%12 = OpTypeInt 32 1
%2574 = OpConstant %12 1
%14 = OpTypeFloat 64
%1201 = OpConstant %14 1
%11 = OpTypeInt 32 0
%2573 = OpConstant %11 1
%15 = OpTypeVector %9 2
%1926 = OpConstantTrue %9
%311 = OpConstantComposite %15 %1927 %1926
%18 = OpTypeVector %12 2
%2577 = OpConstant %12 2
%1848 = OpConstantComposite %18 %2574 %2577
%19 = OpTypeVector %13 2
%312 = OpConstantComposite %19 %138 %24
%20 = OpTypeVector %14 2
%2572 = OpConstant %14 2
%490 = OpConstantComposite %20 %1201 %2572
%17 = OpTypeVector %11 2
%2576 = OpConstant %11 2
%1840 = OpConstantComposite %17 %2573 %2576
%16 = OpTypeVector %9 3
%869 = OpConstantComposite %16 %1927 %1926 %1926
%22 = OpTypeVector %12 3
%2580 = OpConstant %12 3
%2668 = OpConstantComposite %22 %2574 %2577 %2580
%25 = OpTypeVector %13 3
%1271 = OpConstantComposite %25 %138 %24 %2978
%26 = OpTypeVector %14 3
%1057 = OpConstant %14 3
%1108 = OpConstantComposite %26 %1201 %2572 %1057
%21 = OpTypeVector %11 3
%2579 = OpConstant %11 3
%2654 = OpConstantComposite %21 %2573 %2576 %2579
%23 = OpTypeVector %9 4
%340 = OpConstantComposite %23 %1927 %1926 %1926 %1927
%27 = OpTypeVector %12 4
%2583 = OpConstant %12 4
%56 = OpConstantComposite %27 %2574 %2577 %2580 %2583
%30 = OpTypeVector %14 4
%2553 = OpConstant %14 4
%1328 = OpConstantComposite %30 %1201 %2572 %1057 %2553
%28 = OpTypeVector %11 4
%2582 = OpConstant %11 4
%35 = OpConstantComposite %28 %2573 %2576 %2579 %2582
%101 = OpTypeMatrix %29 4
%2575 = OpConstant %13 0
%1199 = OpConstantComposite %29 %2575 %138 %24 %2978
%2864 = OpConstant %13 6
%1330 = OpConstant %13 7
%2290 = OpConstantComposite %29 %2921 %1387 %2864 %1330
%2807 = OpConstant %13 8
%2040 = OpConstant %13 9
%1273 = OpConstant %13 10
%506 = OpConstant %13 11
%694 = OpConstantComposite %29 %2807 %2040 %1273 %506
%2750 = OpConstant %13 12
%1983 = OpConstant %13 13
%1216 = OpConstant %13 14
%449 = OpConstant %13 15
%2679 = OpConstantComposite %29 %2750 %1983 %1216 %449
%1294 = OpConstantComposite %101 %1199 %2290 %694 %2679
%54 = OpTypeMatrix %25 2
%837 = OpConstantComposite %25 %2921 %1387 %2864
%1207 = OpConstantComposite %54 %1271 %837
%60 = OpTypeMatrix %19 3
%2354 = OpConstantComposite %19 %2978 %2921
%364 = OpConstantComposite %19 %1387 %2864
%162 = OpConstantComposite %60 %312 %2354 %364
%71 = OpTypeMatrix %19 4
%2976 = OpConstantComposite %19 %1330 %2807
%2695 = OpConstantComposite %71 %312 %2354 %364 %2976
%86 = OpTypeMatrix %25 4
%635 = OpConstantComposite %25 %1330 %2807 %2040
%832 = OpConstantComposite %25 %1273 %506 %2750
%55 = OpConstantComposite %86 %1271 %837 %635 %832
%667 = OpTypePointer Input %29
%4872 = OpVariable %667 Input
%650 = OpTypePointer Output %13
%4045 = OpVariable %650 Output
%5663 = OpFunction %8 None %1282
%24953 = OpLabel
%5786 = OpVariable %666 Function
%24021 = OpLoad %29 %4872
OpStore %5786 %24021
%9338 = OpFunctionCall %13 %3917 %5786
OpStore %4045 %9338
OpReturn
OpFunctionEnd
%3917 = OpFunction %13 None %255
%10636 = OpFunctionParameter %666
%10637 = OpLabel
OpReturnValue %2575
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<CanonicalizeIdsPass>(before, after, false, false);
}

// ported from remap.if.everything.frag
TEST_F(CanonicalizeIdsTest, remap_if) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %8 %17
OpExecutionMode %4 OriginUpperLeft
OpSource GLSL 450
OpName %4 "main"
OpName %8 "inf"
OpName %17 "outf4"
OpDecorate %8 Location 0
OpDecorate %17 Location 0
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypePointer Input %6
%8 = OpVariable %7 Input
%10 = OpConstant %6 2
%11 = OpTypeBool
%15 = OpTypeVector %6 4
%16 = OpTypePointer Output %15
%17 = OpVariable %16 Output
%22 = OpConstant %6 -0.5
%4 = OpFunction %2 None %3
%5 = OpLabel
%9 = OpLoad %6 %8
%12 = OpFOrdGreaterThan %11 %9 %10
OpSelectionMerge %14 None
OpBranchConditional %12 %13 %20
%13 = OpLabel
%18 = OpLoad %6 %8
%19 = OpCompositeConstruct %15 %18 %18 %18 %18
OpStore %17 %19
OpBranch %14
%20 = OpLabel
%21 = OpLoad %6 %8
%23 = OpFAdd %6 %21 %22
%24 = OpCompositeConstruct %15 %23 %23 %23 %23
OpStore %17 %24
OpBranch %14
%14 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %5663 "main" %3773 %4539
OpExecutionMode %5663 OriginUpperLeft
OpSource GLSL 450
OpName %5663 "main"
OpName %3773 "inf"
OpName %4539 "outf4"
OpDecorate %3773 Location 0
OpDecorate %4539 Location 0
%8 = OpTypeVoid
%1282 = OpTypeFunction %8
%13 = OpTypeFloat 32
%650 = OpTypePointer Input %13
%3773 = OpVariable %650 Input
%24 = OpConstant %13 2
%9 = OpTypeBool
%29 = OpTypeVector %13 4
%666 = OpTypePointer Output %29
%4539 = OpVariable %666 Output
%947 = OpConstant %13 -0.5
%5663 = OpFunction %8 None %1282
%7911 = OpLabel
%21734 = OpLoad %13 %3773
%13508 = OpFOrdGreaterThan %9 %21734 %24
OpSelectionMerge %19578 None
OpBranchConditional %13508 %13182 %10142
%13182 = OpLabel
%9496 = OpLoad %13 %3773
%17615 = OpCompositeConstruct %29 %9496 %9496 %9496 %9496
OpStore %4539 %17615
OpBranch %19578
%10142 = OpLabel
%22854 = OpLoad %13 %3773
%9982 = OpFAdd %13 %22854 %947
%12421 = OpCompositeConstruct %29 %9982 %9982 %9982 %9982
OpStore %4539 %12421
OpBranch %19578
%19578 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<CanonicalizeIdsPass>(before, after, false, false);
}

// ported from remap.similar_1a.everything.frag
TEST_F(CanonicalizeIdsTest, remap_similar_1a) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %53 %73 %75
OpExecutionMode %4 OriginUpperLeft
OpSource GLSL 450
OpName %4 "main"
OpName %11 "Test1(i1;"
OpName %10 "bound"
OpName %14 "Test2(i1;"
OpName %13 "bound"
OpName %17 "r"
OpName %19 "x"
OpName %44 "param"
OpName %53 "ini4"
OpName %73 "outf4"
OpName %75 "inf"
OpName %78 "param"
OpName %82 "param"
OpDecorate %53 Flat
OpDecorate %53 Location 1
OpDecorate %73 Location 0
OpDecorate %75 Location 0
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpTypePointer Function %6
%8 = OpTypeFloat 32
%9 = OpTypeFunction %8 %7
%16 = OpTypePointer Function %8
%18 = OpConstant %8 0
%20 = OpConstant %6 0
%28 = OpTypeBool
%30 = OpConstant %8 0.5
%34 = OpConstant %6 1
%40 = OpConstant %6 2
%51 = OpTypeVector %6 4
%52 = OpTypePointer Input %51
%53 = OpVariable %52 Input
%54 = OpTypeInt 32 0
%55 = OpConstant %54 1
%56 = OpTypePointer Input %6
%59 = OpConstant %54 2
%64 = OpConstant %54 0
%71 = OpTypeVector %8 4
%72 = OpTypePointer Output %71
%73 = OpVariable %72 Output
%74 = OpTypePointer Input %8
%75 = OpVariable %74 Input
%4 = OpFunction %2 None %3
%5 = OpLabel
%78 = OpVariable %7 Function
%82 = OpVariable %7 Function
%76 = OpLoad %8 %75
%77 = OpConvertFToS %6 %76
OpStore %78 %77
%79 = OpFunctionCall %8 %11 %78
%80 = OpLoad %8 %75
%81 = OpConvertFToS %6 %80
OpStore %82 %81
%83 = OpFunctionCall %8 %14 %82
%84 = OpFAdd %8 %79 %83
%85 = OpCompositeConstruct %71 %84 %84 %84 %84
OpStore %73 %85
OpReturn
OpFunctionEnd
%11 = OpFunction %8 None %9
%10 = OpFunctionParameter %7
%12 = OpLabel
%17 = OpVariable %16 Function
%19 = OpVariable %7 Function
OpStore %17 %18
OpStore %19 %20
OpBranch %21
%21 = OpLabel
OpLoopMerge %23 %24 None
OpBranch %25
%25 = OpLabel
%26 = OpLoad %6 %19
%27 = OpLoad %6 %10
%29 = OpSLessThan %28 %26 %27
OpBranchConditional %29 %22 %23
%22 = OpLabel
%31 = OpLoad %8 %17
%32 = OpFAdd %8 %31 %30
OpStore %17 %32
OpBranch %24
%24 = OpLabel
%33 = OpLoad %6 %19
%35 = OpIAdd %6 %33 %34
OpStore %19 %35
OpBranch %21
%23 = OpLabel
%36 = OpLoad %8 %17
OpReturnValue %36
OpFunctionEnd
%14 = OpFunction %8 None %9
%13 = OpFunctionParameter %7
%15 = OpLabel
%44 = OpVariable %7 Function
%39 = OpLoad %6 %13
%41 = OpSGreaterThan %28 %39 %40
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %48
%42 = OpLabel
%45 = OpLoad %6 %13
OpStore %44 %45
%46 = OpFunctionCall %8 %11 %44
OpReturnValue %46
%48 = OpLabel
%49 = OpLoad %6 %13
%50 = OpIMul %6 %49 %40
%57 = OpAccessChain %56 %53 %55
%58 = OpLoad %6 %57
%60 = OpAccessChain %56 %53 %59
%61 = OpLoad %6 %60
%62 = OpIMul %6 %58 %61
%63 = OpIAdd %6 %50 %62
%65 = OpAccessChain %56 %53 %64
%66 = OpLoad %6 %65
%67 = OpIAdd %6 %63 %66
%68 = OpConvertSToF %8 %67
OpReturnValue %68
%43 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %5663 "main" %4201 %4539 %3773
OpExecutionMode %5663 OriginUpperLeft
OpSource GLSL 450
OpName %5663 "main"
OpName %3782 "Test1(i1;"
OpName %6931 "bound"
OpName %3836 "Test2(i1;"
OpName %4408 "bound"
OpName %4292 "r"
OpName %4298 "x"
OpName %22102 "param"
OpName %4201 "ini4"
OpName %4539 "outf4"
OpName %3773 "inf"
OpName %18415 "param"
OpName %5786 "param"
OpDecorate %4201 Flat
OpDecorate %4201 Location 1
OpDecorate %4539 Location 0
OpDecorate %3773 Location 0
%8 = OpTypeVoid
%1282 = OpTypeFunction %8
%12 = OpTypeInt 32 1
%649 = OpTypePointer Function %12
%13 = OpTypeFloat 32
%204 = OpTypeFunction %13 %649
%650 = OpTypePointer Function %13
%2572 = OpConstant %13 0
%2571 = OpConstant %12 0
%9 = OpTypeBool
%252 = OpConstant %13 0.5
%2574 = OpConstant %12 1
%2577 = OpConstant %12 2
%26 = OpTypeVector %12 4
%663 = OpTypePointer Input %26
%4201 = OpVariable %663 Input
%11 = OpTypeInt 32 0
%2573 = OpConstant %11 1
%651 = OpTypePointer Input %12
%2576 = OpConstant %11 2
%2570 = OpConstant %11 0
%29 = OpTypeVector %13 4
%666 = OpTypePointer Output %29
%4539 = OpVariable %666 Output
%652 = OpTypePointer Input %13
%3773 = OpVariable %652 Input
%5663 = OpFunction %8 None %1282
%24915 = OpLabel
%18415 = OpVariable %649 Function
%5786 = OpVariable %649 Function
%8366 = OpLoad %13 %3773
%8654 = OpConvertFToS %12 %8366
OpStore %18415 %8654
%17256 = OpFunctionCall %13 %3782 %18415
%14512 = OpLoad %13 %3773
%7041 = OpConvertFToS %12 %14512
OpStore %5786 %7041
%23993 = OpFunctionCall %13 %3836 %5786
%9180 = OpFAdd %13 %17256 %23993
%15728 = OpCompositeConstruct %29 %9180 %9180 %9180 %9180
OpStore %4539 %15728
OpReturn
OpFunctionEnd
%3782 = OpFunction %13 None %204
%6931 = OpFunctionParameter %649
%12220 = OpLabel
%4292 = OpVariable %650 Function
%4298 = OpVariable %649 Function
OpStore %4292 %2572
OpStore %4298 %2571
OpBranch %14924
%14924 = OpLabel
OpLoopMerge %8882 %6488 None
OpBranch %11857
%11857 = OpLabel
%13755 = OpLoad %12 %4298
%22731 = OpLoad %12 %6931
%20007 = OpSLessThan %9 %13755 %22731
OpBranchConditional %20007 %24750 %8882
%24750 = OpLabel
%22912 = OpLoad %13 %4292
%19471 = OpFAdd %13 %22912 %252
OpStore %4292 %19471
OpBranch %6488
%6488 = OpLabel
%19050 = OpLoad %12 %4298
%8593 = OpIAdd %12 %19050 %2574
OpStore %4298 %8593
OpBranch %14924
%8882 = OpLabel
%11601 = OpLoad %13 %4292
OpReturnValue %11601
OpFunctionEnd
%3836 = OpFunction %13 None %204
%4408 = OpFunctionParameter %649
%12143 = OpLabel
%22102 = OpVariable %649 Function
%24151 = OpLoad %12 %4408
%13868 = OpSGreaterThan %9 %24151 %2577
OpSelectionMerge %14966 None
OpBranchConditional %13868 %9492 %17416
%9492 = OpLabel
%15624 = OpLoad %12 %4408
OpStore %22102 %15624
%17278 = OpFunctionCall %13 %3782 %22102
OpReturnValue %17278
%17416 = OpLabel
%19506 = OpLoad %12 %4408
%22773 = OpIMul %12 %19506 %2577
%13472 = OpAccessChain %651 %4201 %2573
%15280 = OpLoad %12 %13472
%18079 = OpAccessChain %651 %4201 %2576
%15199 = OpLoad %12 %18079
%9343 = OpIMul %12 %15280 %15199
%11462 = OpIAdd %12 %22773 %9343
%11885 = OpAccessChain %651 %4201 %2570
%21176 = OpLoad %12 %11885
%10505 = OpIAdd %12 %11462 %21176
%14626 = OpConvertSToF %13 %10505
OpReturnValue %14626
%14966 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<CanonicalizeIdsPass>(before, after, false, false);
}

// ported from remap.similar_1b.everything.frag
TEST_F(CanonicalizeIdsTest, remap_similar_1b) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %58 %78 %80
OpExecutionMode %4 OriginUpperLeft
OpSource GLSL 450
OpName %4 "main"
OpName %11 "Test1(i1;"
OpName %10 "bound"
OpName %14 "Test2(i1;"
OpName %13 "bound"
OpName %17 "r"
OpName %19 "x"
OpName %49 "param"
OpName %58 "ini4"
OpName %78 "outf4"
OpName %80 "inf"
OpName %83 "param"
OpName %87 "param"
OpDecorate %58 Flat
OpDecorate %58 Location 0
OpDecorate %78 Location 0
OpDecorate %80 Location 1
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpTypePointer Function %6
%8 = OpTypeFloat 32
%9 = OpTypeFunction %8 %7
%16 = OpTypePointer Function %8
%18 = OpConstant %8 0
%20 = OpConstant %6 0
%28 = OpTypeBool
%30 = OpConstant %8 0.5
%34 = OpConstant %6 1
%36 = OpConstant %8 0.200000003
%43 = OpConstant %6 2
%54 = OpConstant %6 4
%56 = OpTypeVector %6 4
%57 = OpTypePointer Input %56
%58 = OpVariable %57 Input
%59 = OpTypeInt 32 0
%60 = OpConstant %59 1
%61 = OpTypePointer Input %6
%64 = OpConstant %59 2
%69 = OpConstant %59 0
%76 = OpTypeVector %8 4
%77 = OpTypePointer Output %76
%78 = OpVariable %77 Output
%79 = OpTypePointer Input %8
%80 = OpVariable %79 Input
%4 = OpFunction %2 None %3
%5 = OpLabel
%83 = OpVariable %7 Function
%87 = OpVariable %7 Function
%81 = OpLoad %8 %80
%82 = OpConvertFToS %6 %81
OpStore %83 %82
%84 = OpFunctionCall %8 %11 %83
%85 = OpLoad %8 %80
%86 = OpConvertFToS %6 %85
OpStore %87 %86
%88 = OpFunctionCall %8 %14 %87
%89 = OpFAdd %8 %84 %88
%90 = OpCompositeConstruct %76 %89 %89 %89 %89
OpStore %78 %90
OpReturn
OpFunctionEnd
%11 = OpFunction %8 None %9
%10 = OpFunctionParameter %7
%12 = OpLabel
%17 = OpVariable %16 Function
%19 = OpVariable %7 Function
OpStore %17 %18
OpStore %19 %20
OpBranch %21
%21 = OpLabel
OpLoopMerge %23 %24 None
OpBranch %25
%25 = OpLabel
%26 = OpLoad %6 %19
%27 = OpLoad %6 %10
%29 = OpSLessThan %28 %26 %27
OpBranchConditional %29 %22 %23
%22 = OpLabel
%31 = OpLoad %8 %17
%32 = OpFAdd %8 %31 %30
OpStore %17 %32
OpBranch %24
%24 = OpLabel
%33 = OpLoad %6 %19
%35 = OpIAdd %6 %33 %34
OpStore %19 %35
OpBranch %21
%23 = OpLabel
%37 = OpLoad %8 %17
%38 = OpFAdd %8 %37 %36
OpStore %17 %38
%39 = OpLoad %8 %17
OpReturnValue %39
OpFunctionEnd
%14 = OpFunction %8 None %9
%13 = OpFunctionParameter %7
%15 = OpLabel
%49 = OpVariable %7 Function
%42 = OpLoad %6 %13
%44 = OpSGreaterThan %28 %42 %43
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %52
%45 = OpLabel
%47 = OpLoad %6 %13
%48 = OpIMul %6 %47 %43
OpStore %49 %48
%50 = OpFunctionCall %8 %11 %49
OpReturnValue %50
%52 = OpLabel
%53 = OpLoad %6 %13
%55 = OpIMul %6 %53 %54
%62 = OpAccessChain %61 %58 %60
%63 = OpLoad %6 %62
%65 = OpAccessChain %61 %58 %64
%66 = OpLoad %6 %65
%67 = OpIMul %6 %63 %66
%68 = OpIAdd %6 %55 %67
%70 = OpAccessChain %61 %58 %69
%71 = OpLoad %6 %70
%72 = OpIAdd %6 %68 %71
%73 = OpConvertSToF %8 %72
OpReturnValue %73
%46 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %5663 "main" %4201 %4539 %3773
OpExecutionMode %5663 OriginUpperLeft
OpSource GLSL 450
OpName %5663 "main"
OpName %3782 "Test1(i1;"
OpName %6931 "bound"
OpName %3836 "Test2(i1;"
OpName %4408 "bound"
OpName %4292 "r"
OpName %4298 "x"
OpName %22102 "param"
OpName %4201 "ini4"
OpName %4539 "outf4"
OpName %3773 "inf"
OpName %18415 "param"
OpName %5786 "param"
OpDecorate %4201 Flat
OpDecorate %4201 Location 0
OpDecorate %4539 Location 0
OpDecorate %3773 Location 1
%8 = OpTypeVoid
%1282 = OpTypeFunction %8
%12 = OpTypeInt 32 1
%649 = OpTypePointer Function %12
%13 = OpTypeFloat 32
%204 = OpTypeFunction %13 %649
%650 = OpTypePointer Function %13
%2572 = OpConstant %13 0
%2571 = OpConstant %12 0
%9 = OpTypeBool
%252 = OpConstant %13 0.5
%2574 = OpConstant %12 1
%2821 = OpConstant %13 0.200000003
%2577 = OpConstant %12 2
%2583 = OpConstant %12 4
%26 = OpTypeVector %12 4
%663 = OpTypePointer Input %26
%4201 = OpVariable %663 Input
%11 = OpTypeInt 32 0
%2573 = OpConstant %11 1
%651 = OpTypePointer Input %12
%2576 = OpConstant %11 2
%2570 = OpConstant %11 0
%29 = OpTypeVector %13 4
%666 = OpTypePointer Output %29
%4539 = OpVariable %666 Output
%652 = OpTypePointer Input %13
%3773 = OpVariable %652 Input
%5663 = OpFunction %8 None %1282
%24915 = OpLabel
%18415 = OpVariable %649 Function
%5786 = OpVariable %649 Function
%8366 = OpLoad %13 %3773
%8654 = OpConvertFToS %12 %8366
OpStore %18415 %8654
%17256 = OpFunctionCall %13 %3782 %18415
%14512 = OpLoad %13 %3773
%7041 = OpConvertFToS %12 %14512
OpStore %5786 %7041
%23993 = OpFunctionCall %13 %3836 %5786
%9180 = OpFAdd %13 %17256 %23993
%15728 = OpCompositeConstruct %29 %9180 %9180 %9180 %9180
OpStore %4539 %15728
OpReturn
OpFunctionEnd
%3782 = OpFunction %13 None %204
%6931 = OpFunctionParameter %649
%12220 = OpLabel
%4292 = OpVariable %650 Function
%4298 = OpVariable %649 Function
OpStore %4292 %2572
OpStore %4298 %2571
OpBranch %14924
%14924 = OpLabel
OpLoopMerge %6507 %6488 None
OpBranch %11857
%11857 = OpLabel
%13755 = OpLoad %12 %4298
%22731 = OpLoad %12 %6931
%20007 = OpSLessThan %9 %13755 %22731
OpBranchConditional %20007 %24750 %6507
%24750 = OpLabel
%22912 = OpLoad %13 %4292
%19471 = OpFAdd %13 %22912 %252
OpStore %4292 %19471
OpBranch %6488
%6488 = OpLabel
%19050 = OpLoad %12 %4298
%8593 = OpIAdd %12 %19050 %2574
OpStore %4298 %8593
OpBranch %14924
%6507 = OpLabel
%18877 = OpLoad %13 %4292
%15899 = OpFAdd %13 %18877 %2821
OpStore %4292 %15899
%20342 = OpLoad %13 %4292
OpReturnValue %20342
OpFunctionEnd
%3836 = OpFunction %13 None %204
%4408 = OpFunctionParameter %649
%12143 = OpLabel
%22102 = OpVariable %649 Function
%24151 = OpLoad %12 %4408
%13868 = OpSGreaterThan %9 %24151 %2577
OpSelectionMerge %14966 None
OpBranchConditional %13868 %10822 %17416
%10822 = OpLabel
%22680 = OpLoad %12 %4408
%23216 = OpIMul %12 %22680 %2577
OpStore %22102 %23216
%7042 = OpFunctionCall %13 %3782 %22102
OpReturnValue %7042
%17416 = OpLabel
%19506 = OpLoad %12 %4408
%22773 = OpIMul %12 %19506 %2583
%13472 = OpAccessChain %651 %4201 %2573
%15280 = OpLoad %12 %13472
%18079 = OpAccessChain %651 %4201 %2576
%15199 = OpLoad %12 %18079
%9343 = OpIMul %12 %15280 %15199
%11462 = OpIAdd %12 %22773 %9343
%11885 = OpAccessChain %651 %4201 %2570
%21176 = OpLoad %12 %11885
%10505 = OpIAdd %12 %11462 %21176
%14626 = OpConvertSToF %13 %10505
OpReturnValue %14626
%14966 = OpLabel
OpUnreachable
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<CanonicalizeIdsPass>(before, after, false, false);
}

// ported from remap.specconst.comp
TEST_F(CanonicalizeIdsTest, remap_specconst) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %4 "main"
OpExecutionMode %4 LocalSize 1 1 1
OpSource GLSL 450
OpName %4 "main"
OpDecorate %7 SpecId 0
OpDecorate %8 SpecId 1
OpDecorate %9 SpecId 2
OpDecorate %11 BuiltIn WorkgroupSize
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpSpecConstant %6 1
%8 = OpSpecConstant %6 1
%9 = OpSpecConstant %6 1
%10 = OpTypeVector %6 3
%11 = OpSpecConstantComposite %10 %7 %8 %9
%14 = OpSpecConstantOp %6 CompositeExtract %11 0
%16 = OpSpecConstantOp %6 CompositeExtract %11 1
%18 = OpSpecConstantOp %6 CompositeExtract %11 2
%19 = OpSpecConstantOp %6 IMul %16 %18
%20 = OpSpecConstantOp %6 IAdd %14 %19
%4 = OpFunction %2 None %3
%5 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5663 "main"
OpExecutionMode %5663 LocalSize 1 1 1
OpSource GLSL 450
OpName %5663 "main"
OpDecorate %2 SpecId 0
OpDecorate %3 SpecId 1
OpDecorate %4 SpecId 2
OpDecorate %5 BuiltIn WorkgroupSize
%8 = OpTypeVoid
%1282 = OpTypeFunction %8
%11 = OpTypeInt 32 0
%2 = OpSpecConstant %11 1
%3 = OpSpecConstant %11 1
%4 = OpSpecConstant %11 1
%20 = OpTypeVector %11 3
%5 = OpSpecConstantComposite %20 %2 %3 %4
%6 = OpSpecConstantOp %11 CompositeExtract %5 0
%7 = OpSpecConstantOp %11 CompositeExtract %5 1
%9 = OpSpecConstantOp %11 CompositeExtract %5 2
%10 = OpSpecConstantOp %11 IMul %7 %9
%12 = OpSpecConstantOp %11 IAdd %6 %10
%5663 = OpFunction %8 None %1282
%16103 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<CanonicalizeIdsPass>(before, after, false, false);
}

// ported from remap.switch.everything.frag
TEST_F(CanonicalizeIdsTest, remap_switch) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %9 %23
OpExecutionMode %4 OriginUpperLeft
OpSource GLSL 450
OpName %4 "main"
OpName %9 "in0"
OpName %23 "FragColor"
OpDecorate %9 Location 0
OpDecorate %23 RelaxedPrecision
OpDecorate %23 Location 0
OpDecorate %29 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeVector %6 4
%8 = OpTypePointer Input %7
%9 = OpVariable %8 Input
%10 = OpTypeInt 32 0
%11 = OpConstant %10 3
%12 = OpTypePointer Input %6
%15 = OpTypeInt 32 1
%22 = OpTypePointer Output %7
%23 = OpVariable %22 Output
%24 = OpConstant %10 0
%27 = OpConstant %6 0
%31 = OpConstant %10 1
%34 = OpConstant %6 1
%38 = OpConstant %10 2
%41 = OpConstant %6 2
%45 = OpConstant %6 -1
%46 = OpConstantComposite %7 %45 %45 %45 %45
%4 = OpFunction %2 None %3
%5 = OpLabel
%13 = OpAccessChain %12 %9 %11
%14 = OpLoad %6 %13
%16 = OpConvertFToS %15 %14
OpSelectionMerge %21 None
OpSwitch %16 %20 0 %17 1 %18 2 %19
%20 = OpLabel
OpStore %23 %46
OpBranch %21
%17 = OpLabel
%25 = OpAccessChain %12 %9 %24
%26 = OpLoad %6 %25
%28 = OpFAdd %6 %26 %27
%29 = OpCompositeConstruct %7 %28 %28 %28 %28
OpStore %23 %29
OpBranch %21
%18 = OpLabel
%32 = OpAccessChain %12 %9 %31
%33 = OpLoad %6 %32
%35 = OpFAdd %6 %33 %34
%36 = OpCompositeConstruct %7 %35 %35 %35 %35
OpStore %23 %36
OpBranch %21
%19 = OpLabel
%39 = OpAccessChain %12 %9 %38
%40 = OpLoad %6 %39
%42 = OpFAdd %6 %40 %41
%43 = OpCompositeConstruct %7 %42 %42 %42 %42
OpStore %23 %43
OpBranch %21
%21 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %5663 "main" %3719 %3994
OpExecutionMode %5663 OriginUpperLeft
OpSource GLSL 450
OpName %5663 "main"
OpName %3719 "in0"
OpName %3994 "FragColor"
OpDecorate %3719 Location 0
OpDecorate %3994 RelaxedPrecision
OpDecorate %3994 Location 0
OpDecorate %12421 RelaxedPrecision
OpDecorate %12422 RelaxedPrecision
OpDecorate %12423 RelaxedPrecision
%8 = OpTypeVoid
%1282 = OpTypeFunction %8
%13 = OpTypeFloat 32
%29 = OpTypeVector %13 4
%666 = OpTypePointer Input %29
%3719 = OpVariable %666 Input
%11 = OpTypeInt 32 0
%2579 = OpConstant %11 3
%650 = OpTypePointer Input %13
%12 = OpTypeInt 32 1
%667 = OpTypePointer Output %29
%3994 = OpVariable %667 Output
%2570 = OpConstant %11 0
%2572 = OpConstant %13 0
%2573 = OpConstant %11 1
%138 = OpConstant %13 1
%2576 = OpConstant %11 2
%24 = OpConstant %13 2
%833 = OpConstant %13 -1
%1284 = OpConstantComposite %29 %833 %833 %833 %833
%5663 = OpFunction %8 None %1282
%23915 = OpLabel
%7984 = OpAccessChain %650 %3719 %2579
%11376 = OpLoad %13 %7984
%16859 = OpConvertFToS %12 %11376
OpSelectionMerge %19578 None
OpSwitch %16859 %15971 0 %8158 1 %8159 2 %8160
%15971 = OpLabel
OpStore %3994 %1284
OpBranch %19578
%8158 = OpLabel
%21848 = OpAccessChain %650 %3719 %2570
%23987 = OpLoad %13 %21848
%19989 = OpFAdd %13 %23987 %2572
%12421 = OpCompositeConstruct %29 %19989 %19989 %19989 %19989
OpStore %3994 %12421
OpBranch %19578
%8159 = OpLabel
%21849 = OpAccessChain %650 %3719 %2573
%23988 = OpLoad %13 %21849
%19990 = OpFAdd %13 %23988 %138
%12422 = OpCompositeConstruct %29 %19990 %19990 %19990 %19990
OpStore %3994 %12422
OpBranch %19578
%8160 = OpLabel
%21850 = OpAccessChain %650 %3719 %2576
%23989 = OpLoad %13 %21850
%19991 = OpFAdd %13 %23989 %24
%12423 = OpCompositeConstruct %29 %19991 %19991 %19991 %19991
OpStore %3994 %12423
OpBranch %19578
%19578 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<CanonicalizeIdsPass>(before, after, false, false);
}

// ported from remap.uniformarray.everything.frag
TEST_F(CanonicalizeIdsTest, remap_uniformarray) {
  const std::string before =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %14 %25 %43 %54
OpExecutionMode %4 OriginUpperLeft
OpSource GLSL 140
OpName %4 "main"
OpName %9 "texColor"
OpName %14 "color"
OpName %25 "inColor"
OpName %43 "alpha"
OpName %54 "gl_FragColor"
OpDecorate %14 Location 1
OpDecorate %25 Location 0
OpDecorate %43 Location 7
OpDecorate %54 Location 0
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeVector %6 4
%8 = OpTypePointer Function %7
%10 = OpTypeInt 32 0
%11 = OpConstant %10 6
%12 = OpTypeArray %7 %11
%13 = OpTypePointer Input %12
%14 = OpVariable %13 Input
%15 = OpTypeInt 32 1
%16 = OpConstant %15 1
%17 = OpTypePointer Input %7
%23 = OpTypeVector %6 3
%24 = OpTypePointer Input %23
%25 = OpVariable %24 Input
%30 = OpConstant %10 0
%31 = OpTypePointer Function %6
%34 = OpConstant %10 1
%37 = OpConstant %10 2
%40 = OpConstant %10 16
%41 = OpTypeArray %6 %40
%42 = OpTypePointer Input %41
%43 = OpVariable %42 Input
%44 = OpConstant %15 12
%45 = OpTypePointer Input %6
%48 = OpConstant %10 3
%53 = OpTypePointer Output %7
%54 = OpVariable %53 Output
%4 = OpFunction %2 None %3
%5 = OpLabel
%9 = OpVariable %8 Function
%18 = OpAccessChain %17 %14 %16
%19 = OpLoad %7 %18
%20 = OpAccessChain %17 %14 %16
%21 = OpLoad %7 %20
%22 = OpFAdd %7 %19 %21
OpStore %9 %22
%26 = OpLoad %23 %25
%27 = OpLoad %7 %9
%28 = OpVectorShuffle %23 %27 %27 0 1 2
%29 = OpFAdd %23 %28 %26
%32 = OpAccessChain %31 %9 %30
%33 = OpCompositeExtract %6 %29 0
OpStore %32 %33
%35 = OpAccessChain %31 %9 %34
%36 = OpCompositeExtract %6 %29 1
OpStore %35 %36
%38 = OpAccessChain %31 %9 %37
%39 = OpCompositeExtract %6 %29 2
OpStore %38 %39
%46 = OpAccessChain %45 %43 %44
%47 = OpLoad %6 %46
%49 = OpAccessChain %31 %9 %48
%50 = OpLoad %6 %49
%51 = OpFAdd %6 %50 %47
%52 = OpAccessChain %31 %9 %48
OpStore %52 %51
%55 = OpLoad %7 %9
OpStore %54 %55
OpReturn
OpFunctionEnd
)";

  const std::string after =
      R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %5663 "main" %3608 %4957 %4339 %5139
OpExecutionMode %5663 OriginUpperLeft
OpSource GLSL 140
OpName %5663 "main"
OpName %4902 "texColor"
OpName %3608 "color"
OpName %4957 "inColor"
OpName %4339 "alpha"
OpName %5139 "gl_FragColor"
OpDecorate %3608 Location 1
OpDecorate %4957 Location 0
OpDecorate %4339 Location 7
OpDecorate %5139 Location 0
%8 = OpTypeVoid
%1282 = OpTypeFunction %8
%13 = OpTypeFloat 32
%29 = OpTypeVector %13 4
%666 = OpTypePointer Function %29
%11 = OpTypeInt 32 0
%2588 = OpConstant %11 6
%740 = OpTypeArray %29 %2588
%1377 = OpTypePointer Input %740
%3608 = OpVariable %1377 Input
%12 = OpTypeInt 32 1
%2574 = OpConstant %12 1
%667 = OpTypePointer Input %29
%24 = OpTypeVector %13 3
%661 = OpTypePointer Input %24
%4957 = OpVariable %661 Input
%2570 = OpConstant %11 0
%650 = OpTypePointer Function %13
%2573 = OpConstant %11 1
%2576 = OpConstant %11 2
%2618 = OpConstant %11 16
%709 = OpTypeArray %13 %2618
%1346 = OpTypePointer Input %709
%4339 = OpVariable %1346 Input
%2607 = OpConstant %12 12
%651 = OpTypePointer Input %13
%2579 = OpConstant %11 3
%668 = OpTypePointer Output %29
%5139 = OpVariable %668 Output
%5663 = OpFunction %8 None %1282
%25029 = OpLabel
%4902 = OpVariable %666 Function
%10645 = OpAccessChain %667 %3608 %2574
%8181 = OpLoad %29 %10645
%21370 = OpAccessChain %667 %3608 %2574
%11355 = OpLoad %29 %21370
%23084 = OpFAdd %29 %8181 %11355
OpStore %4902 %23084
%21218 = OpLoad %24 %4957
%13695 = OpLoad %29 %4902
%23959 = OpVectorShuffle %24 %13695 %13695 0 1 2
%14937 = OpFAdd %24 %23959 %21218
%15653 = OpAccessChain %650 %4902 %2570
%21354 = OpCompositeExtract %13 %14937 0
OpStore %15653 %21354
%16378 = OpAccessChain %650 %4902 %2573
%15746 = OpCompositeExtract %13 %14937 1
OpStore %16378 %15746
%16379 = OpAccessChain %650 %4902 %2576
%15747 = OpCompositeExtract %13 %14937 2
OpStore %16379 %15747
%19895 = OpAccessChain %651 %4339 %2607
%7372 = OpLoad %13 %19895
%21371 = OpAccessChain %650 %4902 %2579
%11412 = OpLoad %13 %21371
%22584 = OpFAdd %13 %11412 %7372
%17318 = OpAccessChain %650 %4902 %2579
OpStore %17318 %22584
%17934 = OpLoad %29 %4902
OpStore %5139 %17934
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  SinglePassRunAndCheck<CanonicalizeIdsPass>(before, after, false, false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
