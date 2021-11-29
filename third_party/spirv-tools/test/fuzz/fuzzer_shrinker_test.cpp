// Copyright (c) 2019 Google LLC
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

#include <functional>
#include <vector>

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "source/fuzz/shrinker.h"
#include "source/fuzz/uniform_buffer_element_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

// The following SPIR-V came from this GLSL:
//
// #version 310 es
//
// void foo() {
//   int x;
//   x = 2;
//   for (int i = 0; i < 100; i++) {
//     x += i;
//     x = x * 2;
//   }
//   return;
// }
//
// void main() {
//   foo();
//   for (int i = 0; i < 10; i++) {
//     int j = 20;
//     while(j > 0) {
//       foo();
//       j--;
//     }
//     do {
//       i++;
//     } while(i < 4);
//   }
// }

const std::string kTestShader1 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %6 "foo("
               OpName %10 "x"
               OpName %12 "i"
               OpName %33 "i"
               OpName %42 "j"
               OpDecorate %10 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %8 = OpTypeInt 32 1
          %9 = OpTypePointer Function %8
         %11 = OpConstant %8 2
         %13 = OpConstant %8 0
         %20 = OpConstant %8 100
         %21 = OpTypeBool
         %29 = OpConstant %8 1
         %40 = OpConstant %8 10
         %43 = OpConstant %8 20
         %61 = OpConstant %8 4
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %33 = OpVariable %9 Function
         %42 = OpVariable %9 Function
         %32 = OpFunctionCall %2 %6
               OpStore %33 %13
               OpBranch %34
         %34 = OpLabel
               OpLoopMerge %36 %37 None
               OpBranch %38
         %38 = OpLabel
         %39 = OpLoad %8 %33
         %41 = OpSLessThan %21 %39 %40
               OpBranchConditional %41 %35 %36
         %35 = OpLabel
               OpStore %42 %43
               OpBranch %44
         %44 = OpLabel
               OpLoopMerge %46 %47 None
               OpBranch %48
         %48 = OpLabel
         %49 = OpLoad %8 %42
         %50 = OpSGreaterThan %21 %49 %13
               OpBranchConditional %50 %45 %46
         %45 = OpLabel
         %51 = OpFunctionCall %2 %6
         %52 = OpLoad %8 %42
         %53 = OpISub %8 %52 %29
               OpStore %42 %53
               OpBranch %47
         %47 = OpLabel
               OpBranch %44
         %46 = OpLabel
               OpBranch %54
         %54 = OpLabel
               OpLoopMerge %56 %57 None
               OpBranch %55
         %55 = OpLabel
         %58 = OpLoad %8 %33
         %59 = OpIAdd %8 %58 %29
               OpStore %33 %59
               OpBranch %57
         %57 = OpLabel
         %60 = OpLoad %8 %33
         %62 = OpSLessThan %21 %60 %61
               OpBranchConditional %62 %54 %56
         %56 = OpLabel
               OpBranch %37
         %37 = OpLabel
         %63 = OpLoad %8 %33
         %64 = OpIAdd %8 %63 %29
               OpStore %33 %64
               OpBranch %34
         %36 = OpLabel
               OpReturn
               OpFunctionEnd
          %6 = OpFunction %2 None %3
          %7 = OpLabel
         %10 = OpVariable %9 Function
         %12 = OpVariable %9 Function
               OpStore %10 %11
               OpStore %12 %13
               OpBranch %14
         %14 = OpLabel
               OpLoopMerge %16 %17 None
               OpBranch %18
         %18 = OpLabel
         %19 = OpLoad %8 %12
         %22 = OpSLessThan %21 %19 %20
               OpBranchConditional %22 %15 %16
         %15 = OpLabel
         %23 = OpLoad %8 %12
         %24 = OpLoad %8 %10
         %25 = OpIAdd %8 %24 %23
               OpStore %10 %25
         %26 = OpLoad %8 %10
         %27 = OpIMul %8 %26 %11
               OpStore %10 %27
               OpBranch %17
         %17 = OpLabel
         %28 = OpLoad %8 %12
         %30 = OpIAdd %8 %28 %29
               OpStore %12 %30
               OpBranch %14
         %16 = OpLabel
               OpReturn
               OpFunctionEnd

  )";

// The following SPIR-V came from this GLSL, which was then optimized using
// spirv-opt with the -O argument:
//
// #version 310 es
//
// precision highp float;
//
// layout(location = 0) out vec4 _GLF_color;
//
// layout(set = 0, binding = 0) uniform buf0 {
//  vec2 injectionSwitch;
// };
// layout(set = 0, binding = 1) uniform buf1 {
//  vec2 resolution;
// };
// bool checkSwap(float a, float b)
// {
//  return gl_FragCoord.y < resolution.y / 2.0 ? a > b : a < b;
// }
// void main()
// {
//  float data[10];
//  for(int i = 0; i < 10; i++)
//   {
//    data[i] = float(10 - i) * injectionSwitch.y;
//   }
//  for(int i = 0; i < 9; i++)
//   {
//    for(int j = 0; j < 10; j++)
//     {
//      if(j < i + 1)
//       {
//        continue;
//       }
//      bool doSwap = checkSwap(data[i], data[j]);
//      if(doSwap)
//       {
//        float temp = data[i];
//        data[i] = data[j];
//        data[j] = temp;
//       }
//     }
//   }
//  if(gl_FragCoord.x < resolution.x / 2.0)
//   {
//    _GLF_color = vec4(data[0] / 10.0, data[5] / 10.0, data[9] / 10.0, 1.0);
//   }
//  else
//   {
//    _GLF_color = vec4(data[5] / 10.0, data[9] / 10.0, data[0] / 10.0, 1.0);
//   }
// }

const std::string kTestShader2 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %16 %139 %25 %68
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %16 "gl_FragCoord"
               OpName %23 "buf1"
               OpMemberName %23 0 "resolution"
               OpName %25 ""
               OpName %61 "data"
               OpName %66 "buf0"
               OpMemberName %66 0 "injectionSwitch"
               OpName %68 ""
               OpName %139 "_GLF_color"
               OpDecorate %16 BuiltIn FragCoord
               OpMemberDecorate %23 0 Offset 0
               OpDecorate %23 Block
               OpDecorate %25 DescriptorSet 0
               OpDecorate %25 Binding 1
               OpDecorate %64 RelaxedPrecision
               OpMemberDecorate %66 0 Offset 0
               OpDecorate %66 Block
               OpDecorate %68 DescriptorSet 0
               OpDecorate %68 Binding 0
               OpDecorate %75 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %139 Location 0
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Function %6
          %8 = OpTypeBool
         %14 = OpTypeVector %6 4
         %15 = OpTypePointer Input %14
         %16 = OpVariable %15 Input
         %17 = OpTypeInt 32 0
         %18 = OpConstant %17 1
         %19 = OpTypePointer Input %6
         %22 = OpTypeVector %6 2
         %23 = OpTypeStruct %22
         %24 = OpTypePointer Uniform %23
         %25 = OpVariable %24 Uniform
         %26 = OpTypeInt 32 1
         %27 = OpConstant %26 0
         %28 = OpTypePointer Uniform %6
         %56 = OpConstant %26 10
         %58 = OpConstant %17 10
         %59 = OpTypeArray %6 %58
         %60 = OpTypePointer Function %59
         %66 = OpTypeStruct %22
         %67 = OpTypePointer Uniform %66
         %68 = OpVariable %67 Uniform
         %74 = OpConstant %26 1
         %83 = OpConstant %26 9
        %129 = OpConstant %17 0
        %138 = OpTypePointer Output %14
        %139 = OpVariable %138 Output
        %144 = OpConstant %26 5
        %151 = OpConstant %6 1
        %194 = OpConstant %6 0.5
        %195 = OpConstant %6 0.100000001
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %61 = OpVariable %60 Function
               OpBranch %50
         %50 = OpLabel
        %182 = OpPhi %26 %27 %5 %75 %51
         %57 = OpSLessThan %8 %182 %56
               OpLoopMerge %52 %51 None
               OpBranchConditional %57 %51 %52
         %51 = OpLabel
         %64 = OpISub %26 %56 %182
         %65 = OpConvertSToF %6 %64
         %69 = OpAccessChain %28 %68 %27 %18
         %70 = OpLoad %6 %69
         %71 = OpFMul %6 %65 %70
         %72 = OpAccessChain %7 %61 %182
               OpStore %72 %71
         %75 = OpIAdd %26 %182 %74
               OpBranch %50
         %52 = OpLabel
               OpBranch %77
         %77 = OpLabel
        %183 = OpPhi %26 %27 %52 %128 %88
         %84 = OpSLessThan %8 %183 %83
               OpLoopMerge %79 %88 None
               OpBranchConditional %84 %78 %79
         %78 = OpLabel
               OpBranch %86
         %86 = OpLabel
        %184 = OpPhi %26 %27 %78 %126 %89
         %92 = OpSLessThan %8 %184 %56
               OpLoopMerge %1000 %89 None
               OpBranchConditional %92 %87 %1000
         %87 = OpLabel
         %95 = OpIAdd %26 %183 %74
         %96 = OpSLessThan %8 %184 %95
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
               OpBranch %89
         %98 = OpLabel
        %104 = OpAccessChain %7 %61 %183
        %105 = OpLoad %6 %104
        %107 = OpAccessChain %7 %61 %184
        %108 = OpLoad %6 %107
        %166 = OpAccessChain %19 %16 %18
        %167 = OpLoad %6 %166
        %168 = OpAccessChain %28 %25 %27 %18
        %169 = OpLoad %6 %168
        %170 = OpFMul %6 %169 %194
        %171 = OpFOrdLessThan %8 %167 %170
               OpSelectionMerge %172 None
               OpBranchConditional %171 %173 %174
        %173 = OpLabel
        %177 = OpFOrdGreaterThan %8 %105 %108
               OpBranch %172
        %174 = OpLabel
        %180 = OpFOrdLessThan %8 %105 %108
               OpBranch %172
        %172 = OpLabel
        %186 = OpPhi %8 %177 %173 %180 %174
               OpSelectionMerge %112 None
               OpBranchConditional %186 %111 %112
        %111 = OpLabel
        %116 = OpLoad %6 %104
        %120 = OpLoad %6 %107
               OpStore %104 %120
               OpStore %107 %116
               OpBranch %112
        %112 = OpLabel
               OpBranch %89
         %89 = OpLabel
        %126 = OpIAdd %26 %184 %74
               OpBranch %86
       %1000 = OpLabel
               OpBranch %88
         %88 = OpLabel
        %128 = OpIAdd %26 %183 %74
               OpBranch %77
         %79 = OpLabel
        %130 = OpAccessChain %19 %16 %129
        %131 = OpLoad %6 %130
        %132 = OpAccessChain %28 %25 %27 %129
        %133 = OpLoad %6 %132
        %134 = OpFMul %6 %133 %194
        %135 = OpFOrdLessThan %8 %131 %134
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %153
        %136 = OpLabel
        %140 = OpAccessChain %7 %61 %27
        %141 = OpLoad %6 %140
        %143 = OpFMul %6 %141 %195
        %145 = OpAccessChain %7 %61 %144
        %146 = OpLoad %6 %145
        %147 = OpFMul %6 %146 %195
        %148 = OpAccessChain %7 %61 %83
        %149 = OpLoad %6 %148
        %150 = OpFMul %6 %149 %195
        %152 = OpCompositeConstruct %14 %143 %147 %150 %151
               OpStore %139 %152
               OpBranch %137
        %153 = OpLabel
        %154 = OpAccessChain %7 %61 %144
        %155 = OpLoad %6 %154
        %156 = OpFMul %6 %155 %195
        %157 = OpAccessChain %7 %61 %83
        %158 = OpLoad %6 %157
        %159 = OpFMul %6 %158 %195
        %160 = OpAccessChain %7 %61 %27
        %161 = OpLoad %6 %160
        %162 = OpFMul %6 %161 %195
        %163 = OpCompositeConstruct %14 %156 %159 %162 %151
               OpStore %139 %163
               OpBranch %137
        %137 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

// The following SPIR-V came from this GLSL, which was then optimized using
// spirv-opt with the -O argument:
//
// #version 310 es
//
// precision highp float;
//
// layout(location = 0) out vec4 _GLF_color;
//
// layout(set = 0, binding = 0) uniform buf0 {
//  vec2 resolution;
// };
// void main(void)
// {
//  float A[50];
//  for(
//      int i = 0;
//      i < 200;
//      i ++
//  )
//   {
//    if(i >= int(resolution.x))
//     {
//      break;
//     }
//    if((4 * (i / 4)) == i)
//     {
//      A[i / 4] = float(i);
//     }
//   }
//  for(
//      int i = 0;
//      i < 50;
//      i ++
//  )
//   {
//    if(i < int(gl_FragCoord.x))
//     {
//      break;
//     }
//    if(i > 0)
//     {
//      A[i] += A[i - 1];
//     }
//   }
//  if(int(gl_FragCoord.x) < 20)
//   {
//    _GLF_color = vec4(A[0] / resolution.x, A[4] / resolution.y, 1.0, 1.0);
//   }
//  else
//   if(int(gl_FragCoord.x) < 40)
//    {
//     _GLF_color = vec4(A[5] / resolution.x, A[9] / resolution.y, 1.0, 1.0);
//    }
//   else
//    if(int(gl_FragCoord.x) < 60)
//     {
//      _GLF_color = vec4(A[10] / resolution.x, A[14] / resolution.y,
//      1.0, 1.0);
//     }
//    else
//     if(int(gl_FragCoord.x) < 80)
//      {
//       _GLF_color = vec4(A[15] / resolution.x, A[19] / resolution.y,
//       1.0, 1.0);
//      }
//     else
//      if(int(gl_FragCoord.x) < 100)
//       {
//        _GLF_color = vec4(A[20] / resolution.x, A[24] / resolution.y,
//        1.0, 1.0);
//       }
//      else
//       if(int(gl_FragCoord.x) < 120)
//        {
//         _GLF_color = vec4(A[25] / resolution.x, A[29] / resolution.y,
//         1.0, 1.0);
//        }
//       else
//        if(int(gl_FragCoord.x) < 140)
//         {
//          _GLF_color = vec4(A[30] / resolution.x, A[34] / resolution.y,
//          1.0, 1.0);
//         }
//        else
//         if(int(gl_FragCoord.x) < 160)
//          {
//           _GLF_color = vec4(A[35] / resolution.x, A[39] /
//           resolution.y, 1.0, 1.0);
//          }
//         else
//          if(int(gl_FragCoord.x) < 180)
//           {
//            _GLF_color = vec4(A[40] / resolution.x, A[44] /
//            resolution.y, 1.0, 1.0);
//           }
//          else
//           if(int(gl_FragCoord.x) < 180)
//            {
//             _GLF_color = vec4(A[45] / resolution.x, A[49] /
//             resolution.y, 1.0, 1.0);
//            }
//           else
//            {
//             discard;
//            }
// }

const std::string kTestShader3 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %68 %100 %24
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %22 "buf0"
               OpMemberName %22 0 "resolution"
               OpName %24 ""
               OpName %46 "A"
               OpName %68 "gl_FragCoord"
               OpName %100 "_GLF_color"
               OpMemberDecorate %22 0 Offset 0
               OpDecorate %22 Block
               OpDecorate %24 DescriptorSet 0
               OpDecorate %24 Binding 0
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %68 BuiltIn FragCoord
               OpDecorate %83 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %100 Location 0
               OpDecorate %302 RelaxedPrecision
               OpDecorate %304 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpConstant %6 0
         %16 = OpConstant %6 200
         %17 = OpTypeBool
         %20 = OpTypeFloat 32
         %21 = OpTypeVector %20 2
         %22 = OpTypeStruct %21
         %23 = OpTypePointer Uniform %22
         %24 = OpVariable %23 Uniform
         %25 = OpTypeInt 32 0
         %26 = OpConstant %25 0
         %27 = OpTypePointer Uniform %20
         %35 = OpConstant %6 4
         %43 = OpConstant %25 50
         %44 = OpTypeArray %20 %43
         %45 = OpTypePointer Function %44
         %51 = OpTypePointer Function %20
         %54 = OpConstant %6 1
         %63 = OpConstant %6 50
         %66 = OpTypeVector %20 4
         %67 = OpTypePointer Input %66
         %68 = OpVariable %67 Input
         %69 = OpTypePointer Input %20
         %95 = OpConstant %6 20
         %99 = OpTypePointer Output %66
        %100 = OpVariable %99 Output
        %108 = OpConstant %25 1
        %112 = OpConstant %20 1
        %118 = OpConstant %6 40
        %122 = OpConstant %6 5
        %128 = OpConstant %6 9
        %139 = OpConstant %6 60
        %143 = OpConstant %6 10
        %149 = OpConstant %6 14
        %160 = OpConstant %6 80
        %164 = OpConstant %6 15
        %170 = OpConstant %6 19
        %181 = OpConstant %6 100
        %190 = OpConstant %6 24
        %201 = OpConstant %6 120
        %205 = OpConstant %6 25
        %211 = OpConstant %6 29
        %222 = OpConstant %6 140
        %226 = OpConstant %6 30
        %232 = OpConstant %6 34
        %243 = OpConstant %6 160
        %247 = OpConstant %6 35
        %253 = OpConstant %6 39
        %264 = OpConstant %6 180
        %273 = OpConstant %6 44
        %287 = OpConstant %6 45
        %293 = OpConstant %6 49
          %4 = OpFunction %2 None %3
          %5 = OpLabel
         %46 = OpVariable %45 Function
               OpBranch %10
         %10 = OpLabel
        %302 = OpPhi %6 %9 %5 %55 %42
         %18 = OpSLessThan %17 %302 %16
               OpLoopMerge %12 %42 None
               OpBranchConditional %18 %11 %12
         %11 = OpLabel
         %28 = OpAccessChain %27 %24 %9 %26
         %29 = OpLoad %20 %28
         %30 = OpConvertFToS %6 %29
         %31 = OpSGreaterThanEqual %17 %302 %30
               OpSelectionMerge %33 None
               OpBranchConditional %31 %32 %33
         %32 = OpLabel
               OpBranch %12
         %33 = OpLabel
         %37 = OpSDiv %6 %302 %35
         %38 = OpIMul %6 %35 %37
         %40 = OpIEqual %17 %38 %302
               OpBranchConditional %40 %41 %42
         %41 = OpLabel
         %50 = OpConvertSToF %20 %302
         %52 = OpAccessChain %51 %46 %37
               OpStore %52 %50
               OpBranch %42
         %42 = OpLabel
         %55 = OpIAdd %6 %302 %54
               OpBranch %10
         %12 = OpLabel
               OpBranch %57
         %57 = OpLabel
        %304 = OpPhi %6 %9 %12 %91 %80
         %64 = OpSLessThan %17 %304 %63
               OpLoopMerge %59 %80 None
               OpBranchConditional %64 %58 %59
         %58 = OpLabel
         %70 = OpAccessChain %69 %68 %26
         %71 = OpLoad %20 %70
         %72 = OpConvertFToS %6 %71
         %73 = OpSLessThan %17 %304 %72
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
               OpBranch %59
         %75 = OpLabel
         %78 = OpSGreaterThan %17 %304 %9
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
         %83 = OpISub %6 %304 %54
         %84 = OpAccessChain %51 %46 %83
         %85 = OpLoad %20 %84
         %86 = OpAccessChain %51 %46 %304
         %87 = OpLoad %20 %86
         %88 = OpFAdd %20 %87 %85
               OpStore %86 %88
               OpBranch %80
         %80 = OpLabel
         %91 = OpIAdd %6 %304 %54
               OpBranch %57
         %59 = OpLabel
         %92 = OpAccessChain %69 %68 %26
         %93 = OpLoad %20 %92
         %94 = OpConvertFToS %6 %93
         %96 = OpSLessThan %17 %94 %95
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %114
         %97 = OpLabel
        %101 = OpAccessChain %51 %46 %9
        %102 = OpLoad %20 %101
        %103 = OpAccessChain %27 %24 %9 %26
        %104 = OpLoad %20 %103
        %105 = OpFDiv %20 %102 %104
        %106 = OpAccessChain %51 %46 %35
        %107 = OpLoad %20 %106
        %109 = OpAccessChain %27 %24 %9 %108
        %110 = OpLoad %20 %109
        %111 = OpFDiv %20 %107 %110
        %113 = OpCompositeConstruct %66 %105 %111 %112 %112
               OpStore %100 %113
               OpBranch %98
        %114 = OpLabel
        %119 = OpSLessThan %17 %94 %118
               OpSelectionMerge %121 None
               OpBranchConditional %119 %120 %135
        %120 = OpLabel
        %123 = OpAccessChain %51 %46 %122
        %124 = OpLoad %20 %123
        %125 = OpAccessChain %27 %24 %9 %26
        %126 = OpLoad %20 %125
        %127 = OpFDiv %20 %124 %126
        %129 = OpAccessChain %51 %46 %128
        %130 = OpLoad %20 %129
        %131 = OpAccessChain %27 %24 %9 %108
        %132 = OpLoad %20 %131
        %133 = OpFDiv %20 %130 %132
        %134 = OpCompositeConstruct %66 %127 %133 %112 %112
               OpStore %100 %134
               OpBranch %121
        %135 = OpLabel
        %140 = OpSLessThan %17 %94 %139
               OpSelectionMerge %142 None
               OpBranchConditional %140 %141 %156
        %141 = OpLabel
        %144 = OpAccessChain %51 %46 %143
        %145 = OpLoad %20 %144
        %146 = OpAccessChain %27 %24 %9 %26
        %147 = OpLoad %20 %146
        %148 = OpFDiv %20 %145 %147
        %150 = OpAccessChain %51 %46 %149
        %151 = OpLoad %20 %150
        %152 = OpAccessChain %27 %24 %9 %108
        %153 = OpLoad %20 %152
        %154 = OpFDiv %20 %151 %153
        %155 = OpCompositeConstruct %66 %148 %154 %112 %112
               OpStore %100 %155
               OpBranch %142
        %156 = OpLabel
        %161 = OpSLessThan %17 %94 %160
               OpSelectionMerge %163 None
               OpBranchConditional %161 %162 %177
        %162 = OpLabel
        %165 = OpAccessChain %51 %46 %164
        %166 = OpLoad %20 %165
        %167 = OpAccessChain %27 %24 %9 %26
        %168 = OpLoad %20 %167
        %169 = OpFDiv %20 %166 %168
        %171 = OpAccessChain %51 %46 %170
        %172 = OpLoad %20 %171
        %173 = OpAccessChain %27 %24 %9 %108
        %174 = OpLoad %20 %173
        %175 = OpFDiv %20 %172 %174
        %176 = OpCompositeConstruct %66 %169 %175 %112 %112
               OpStore %100 %176
               OpBranch %163
        %177 = OpLabel
        %182 = OpSLessThan %17 %94 %181
               OpSelectionMerge %184 None
               OpBranchConditional %182 %183 %197
        %183 = OpLabel
        %185 = OpAccessChain %51 %46 %95
        %186 = OpLoad %20 %185
        %187 = OpAccessChain %27 %24 %9 %26
        %188 = OpLoad %20 %187
        %189 = OpFDiv %20 %186 %188
        %191 = OpAccessChain %51 %46 %190
        %192 = OpLoad %20 %191
        %193 = OpAccessChain %27 %24 %9 %108
        %194 = OpLoad %20 %193
        %195 = OpFDiv %20 %192 %194
        %196 = OpCompositeConstruct %66 %189 %195 %112 %112
               OpStore %100 %196
               OpBranch %184
        %197 = OpLabel
        %202 = OpSLessThan %17 %94 %201
               OpSelectionMerge %204 None
               OpBranchConditional %202 %203 %218
        %203 = OpLabel
        %206 = OpAccessChain %51 %46 %205
        %207 = OpLoad %20 %206
        %208 = OpAccessChain %27 %24 %9 %26
        %209 = OpLoad %20 %208
        %210 = OpFDiv %20 %207 %209
        %212 = OpAccessChain %51 %46 %211
        %213 = OpLoad %20 %212
        %214 = OpAccessChain %27 %24 %9 %108
        %215 = OpLoad %20 %214
        %216 = OpFDiv %20 %213 %215
        %217 = OpCompositeConstruct %66 %210 %216 %112 %112
               OpStore %100 %217
               OpBranch %204
        %218 = OpLabel
        %223 = OpSLessThan %17 %94 %222
               OpSelectionMerge %225 None
               OpBranchConditional %223 %224 %239
        %224 = OpLabel
        %227 = OpAccessChain %51 %46 %226
        %228 = OpLoad %20 %227
        %229 = OpAccessChain %27 %24 %9 %26
        %230 = OpLoad %20 %229
        %231 = OpFDiv %20 %228 %230
        %233 = OpAccessChain %51 %46 %232
        %234 = OpLoad %20 %233
        %235 = OpAccessChain %27 %24 %9 %108
        %236 = OpLoad %20 %235
        %237 = OpFDiv %20 %234 %236
        %238 = OpCompositeConstruct %66 %231 %237 %112 %112
               OpStore %100 %238
               OpBranch %225
        %239 = OpLabel
        %244 = OpSLessThan %17 %94 %243
               OpSelectionMerge %246 None
               OpBranchConditional %244 %245 %260
        %245 = OpLabel
        %248 = OpAccessChain %51 %46 %247
        %249 = OpLoad %20 %248
        %250 = OpAccessChain %27 %24 %9 %26
        %251 = OpLoad %20 %250
        %252 = OpFDiv %20 %249 %251
        %254 = OpAccessChain %51 %46 %253
        %255 = OpLoad %20 %254
        %256 = OpAccessChain %27 %24 %9 %108
        %257 = OpLoad %20 %256
        %258 = OpFDiv %20 %255 %257
        %259 = OpCompositeConstruct %66 %252 %258 %112 %112
               OpStore %100 %259
               OpBranch %246
        %260 = OpLabel
        %265 = OpSLessThan %17 %94 %264
               OpSelectionMerge %267 None
               OpBranchConditional %265 %266 %280
        %266 = OpLabel
        %268 = OpAccessChain %51 %46 %118
        %269 = OpLoad %20 %268
        %270 = OpAccessChain %27 %24 %9 %26
        %271 = OpLoad %20 %270
        %272 = OpFDiv %20 %269 %271
        %274 = OpAccessChain %51 %46 %273
        %275 = OpLoad %20 %274
        %276 = OpAccessChain %27 %24 %9 %108
        %277 = OpLoad %20 %276
        %278 = OpFDiv %20 %275 %277
        %279 = OpCompositeConstruct %66 %272 %278 %112 %112
               OpStore %100 %279
               OpBranch %267
        %280 = OpLabel
               OpSelectionMerge %285 None
               OpBranchConditional %265 %285 %300
        %285 = OpLabel
        %288 = OpAccessChain %51 %46 %287
        %289 = OpLoad %20 %288
        %290 = OpAccessChain %27 %24 %9 %26
        %291 = OpLoad %20 %290
        %292 = OpFDiv %20 %289 %291
        %294 = OpAccessChain %51 %46 %293
        %295 = OpLoad %20 %294
        %296 = OpAccessChain %27 %24 %9 %108
        %297 = OpLoad %20 %296
        %298 = OpFDiv %20 %295 %297
        %299 = OpCompositeConstruct %66 %292 %298 %112 %112
               OpStore %100 %299
               OpBranch %267
        %300 = OpLabel
               OpKill
        %267 = OpLabel
               OpBranch %246
        %246 = OpLabel
               OpBranch %225
        %225 = OpLabel
               OpBranch %204
        %204 = OpLabel
               OpBranch %184
        %184 = OpLabel
               OpBranch %163
        %163 = OpLabel
               OpBranch %142
        %142 = OpLabel
               OpBranch %121
        %121 = OpLabel
               OpBranch %98
         %98 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

// Abstract class exposing an interestingness function as a virtual method.
class InterestingnessTest {
 public:
  virtual ~InterestingnessTest() = default;

  // Abstract method that subclasses should implement for specific notions of
  // interestingness. Its signature matches Shrinker::InterestingnessFunction.
  // Argument |binary| is the SPIR-V binary to be checked; |counter| is used for
  // debugging purposes.
  virtual bool Interesting(const std::vector<uint32_t>& binary,
                           uint32_t counter) = 0;

  // Yields the Interesting instance method wrapped in a function object.
  Shrinker::InterestingnessFunction AsFunction() {
    return std::bind(&InterestingnessTest::Interesting, this,
                     std::placeholders::_1, std::placeholders::_2);
  }
};

// A test that says all binaries are interesting.
class AlwaysInteresting : public InterestingnessTest {
 public:
  bool Interesting(const std::vector<uint32_t>&, uint32_t) override {
    return true;
  }
};

// A test that says a binary is interesting first time round, and uninteresting
// thereafter.
class OnlyInterestingFirstTime : public InterestingnessTest {
 public:
  explicit OnlyInterestingFirstTime() : first_time_(true) {}

  bool Interesting(const std::vector<uint32_t>&, uint32_t) override {
    if (first_time_) {
      first_time_ = false;
      return true;
    }
    return false;
  }

 private:
  bool first_time_;
};

// A test that says a binary is interesting first time round, after which
// interestingness ping pongs between false and true.
class PingPong : public InterestingnessTest {
 public:
  explicit PingPong() : interesting_(false) {}

  bool Interesting(const std::vector<uint32_t>&, uint32_t) override {
    interesting_ = !interesting_;
    return interesting_;
  }

 private:
  bool interesting_;
};

// A test that says a binary is interesting first time round, thereafter
// decides at random whether it is interesting.  This allows the logic of the
// shrinker to be exercised quite a bit.
class InterestingThenRandom : public InterestingnessTest {
 public:
  InterestingThenRandom(const PseudoRandomGenerator& random_generator)
      : first_time_(true), random_generator_(random_generator) {}

  bool Interesting(const std::vector<uint32_t>&, uint32_t) override {
    if (first_time_) {
      first_time_ = false;
      return true;
    }
    return random_generator_.RandomBool();
  }

 private:
  bool first_time_;
  PseudoRandomGenerator random_generator_;
};

// |binary_in| and |initial_facts| are a SPIR-V binary and sequence of facts to
// which |transformation_sequence_in| can be applied.  Shrinking of
// |transformation_sequence_in| gets performed with respect to
// |interestingness_function|.  If |expected_binary_out| is non-empty, it must
// match the binary obtained by applying the final shrunk set of
// transformations, in which case the number of such transformations should
// equal |expected_transformations_out_size|.
//
// The |step_limit| parameter restricts the number of steps that the shrinker
// will try; it can be set to something small for a faster (but less thorough)
// test.
//
// The |validator_options| parameter provides validator options that should be
// used during shrinking.
void RunAndCheckShrinker(
    const spv_target_env& target_env, const std::vector<uint32_t>& binary_in,
    const protobufs::FactSequence& initial_facts,
    const protobufs::TransformationSequence& transformation_sequence_in,
    const Shrinker::InterestingnessFunction& interestingness_function,
    const std::vector<uint32_t>& expected_binary_out,
    uint32_t expected_transformations_out_size, uint32_t step_limit,
    spv_validator_options validator_options) {
  // Run the shrinker.
  auto shrinker_result =
      Shrinker(target_env, kConsoleMessageConsumer, binary_in, initial_facts,
               transformation_sequence_in, interestingness_function, step_limit,
               false, validator_options)
          .Run();

  ASSERT_TRUE(Shrinker::ShrinkerResultStatus::kComplete ==
                  shrinker_result.status ||
              Shrinker::ShrinkerResultStatus::kStepLimitReached ==
                  shrinker_result.status);

  // If a non-empty expected binary was provided, check that it matches the
  // result of shrinking and that the expected number of transformations remain.
  if (!expected_binary_out.empty()) {
    ASSERT_EQ(expected_binary_out, shrinker_result.transformed_binary);
    ASSERT_EQ(
        expected_transformations_out_size,
        static_cast<uint32_t>(
            shrinker_result.applied_transformations.transformation_size()));
  }
}

// Assembles the given |shader| text, and then:
// - Runs the fuzzer with |seed| to yield a set of transformations
// - Shrinks the transformation with various interestingness functions,
//   asserting some properties about the result each time
void RunFuzzerAndShrinker(const std::string& shader,
                          const protobufs::FactSequence& initial_facts,
                          uint32_t seed) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;

  std::vector<uint32_t> binary_in;
  SpirvTools t(env);
  t.SetMessageConsumer(kConsoleMessageConsumer);
  ASSERT_TRUE(t.Assemble(shader, &binary_in, kFuzzAssembleOption));
  ASSERT_TRUE(t.Validate(binary_in));

  std::vector<fuzzerutil::ModuleSupplier> donor_suppliers;
  for (auto donor : {&kTestShader1, &kTestShader2, &kTestShader3}) {
    donor_suppliers.emplace_back([donor]() {
      return BuildModule(env, kConsoleMessageConsumer, *donor,
                         kFuzzAssembleOption);
    });
  }

  // Run the fuzzer and check that it successfully yields a valid binary.
  spvtools::ValidatorOptions validator_options;

  // Depending on the seed, decide whether to enable all passes and which
  // repeated pass manager to use.
  bool enable_all_passes = (seed % 4) == 0;
  RepeatedPassStrategy repeated_pass_strategy;
  if ((seed % 3) == 0) {
    repeated_pass_strategy = RepeatedPassStrategy::kSimple;
  } else if ((seed % 3) == 1) {
    repeated_pass_strategy = RepeatedPassStrategy::kLoopedWithRecommendations;
  } else {
    repeated_pass_strategy = RepeatedPassStrategy::kRandomWithRecommendations;
  }

  std::unique_ptr<opt::IRContext> ir_context;
  ASSERT_TRUE(fuzzerutil::BuildIRContext(
      env, kConsoleMessageConsumer, binary_in, validator_options, &ir_context));

  auto fuzzer_context = MakeUnique<FuzzerContext>(
      MakeUnique<PseudoRandomGenerator>(seed),
      FuzzerContext::GetMinFreshId(ir_context.get()), false);

  auto transformation_context = MakeUnique<TransformationContext>(
      MakeUnique<FactManager>(ir_context.get()), validator_options);
  transformation_context->GetFactManager()->AddInitialFacts(
      kConsoleMessageConsumer, initial_facts);

  Fuzzer fuzzer(std::move(ir_context), std::move(transformation_context),
                std::move(fuzzer_context), kConsoleMessageConsumer,
                donor_suppliers, enable_all_passes, repeated_pass_strategy,
                true, validator_options, false);
  auto fuzzer_result = fuzzer.Run(0);
  ASSERT_NE(Fuzzer::Status::kFuzzerPassLedToInvalidModule,
            fuzzer_result.status);
  std::vector<uint32_t> transformed_binary;
  fuzzer.GetIRContext()->module()->ToBinary(&transformed_binary, true);
  ASSERT_TRUE(t.Validate(transformed_binary));

  const uint32_t kReasonableStepLimit = 50;
  const uint32_t kSmallStepLimit = 20;

  // With the AlwaysInteresting test, we should quickly shrink to the original
  // binary with no transformations remaining.
  RunAndCheckShrinker(env, binary_in, initial_facts,
                      fuzzer.GetTransformationSequence(),
                      AlwaysInteresting().AsFunction(), binary_in, 0,
                      kReasonableStepLimit, validator_options);

  // With the OnlyInterestingFirstTime test, no shrinking should be achieved.
  RunAndCheckShrinker(
      env, binary_in, initial_facts, fuzzer.GetTransformationSequence(),
      OnlyInterestingFirstTime().AsFunction(), transformed_binary,
      static_cast<uint32_t>(
          fuzzer.GetTransformationSequence().transformation_size()),
      kReasonableStepLimit, validator_options);

  // The PingPong test is unpredictable; passing an empty expected binary
  // means that we don't check anything beyond that shrinking completes
  // successfully.
  RunAndCheckShrinker(
      env, binary_in, initial_facts, fuzzer.GetTransformationSequence(),
      PingPong().AsFunction(), {}, 0, kSmallStepLimit, validator_options);

  // The InterestingThenRandom test is unpredictable; passing an empty
  // expected binary means that we do not check anything about shrinking
  // results.
  RunAndCheckShrinker(
      env, binary_in, initial_facts, fuzzer.GetTransformationSequence(),
      InterestingThenRandom(PseudoRandomGenerator(seed)).AsFunction(), {}, 0,
      kSmallStepLimit, validator_options);
}

TEST(FuzzerShrinkerTest, Miscellaneous1) {
  RunFuzzerAndShrinker(kTestShader1, protobufs::FactSequence(), 2);
}

TEST(FuzzerShrinkerTest, Miscellaneous2) {
  RunFuzzerAndShrinker(kTestShader2, protobufs::FactSequence(), 19);
}

TEST(FuzzerShrinkerTest, Miscellaneous3) {
  // Add the facts "resolution.x == 250" and "resolution.y == 100".
  protobufs::FactSequence facts;
  {
    protobufs::FactConstantUniform resolution_x_eq_250;
    *resolution_x_eq_250.mutable_uniform_buffer_element_descriptor() =
        MakeUniformBufferElementDescriptor(0, 0, {0, 0});
    *resolution_x_eq_250.mutable_constant_word()->Add() = 250;
    protobufs::Fact temp;
    *temp.mutable_constant_uniform_fact() = resolution_x_eq_250;
    *facts.mutable_fact()->Add() = temp;
  }
  {
    protobufs::FactConstantUniform resolution_y_eq_100;
    *resolution_y_eq_100.mutable_uniform_buffer_element_descriptor() =
        MakeUniformBufferElementDescriptor(0, 0, {0, 1});
    *resolution_y_eq_100.mutable_constant_word()->Add() = 100;
    protobufs::Fact temp;
    *temp.mutable_constant_uniform_fact() = resolution_y_eq_100;
    *facts.mutable_fact()->Add() = temp;
  }
  // Also add an invalid fact, which should be ignored.
  {
    protobufs::FactConstantUniform bad_fact;
    // The descriptor set, binding and indices used here deliberately make no
    // sense.
    *bad_fact.mutable_uniform_buffer_element_descriptor() =
        MakeUniformBufferElementDescriptor(22, 33, {44, 55});
    *bad_fact.mutable_constant_word()->Add() = 100;
    protobufs::Fact temp;
    *temp.mutable_constant_uniform_fact() = bad_fact;
    *facts.mutable_fact()->Add() = temp;
  }

  // Do 2 fuzzer runs, starting from an initial seed of 194 (seed value chosen
  // arbitrarily).
  RunFuzzerAndShrinker(kTestShader3, facts, 194);
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
