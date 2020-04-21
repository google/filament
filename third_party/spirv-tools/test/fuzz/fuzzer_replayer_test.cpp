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

#include "source/fuzz/fuzzer.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/replayer.h"
#include "source/fuzz/uniform_buffer_element_descriptor.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

const uint32_t kNumFuzzerRuns = 20;

// The SPIR-V came from this GLSL:
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

// The SPIR-V came from this GLSL, which was then optimized using spirv-opt
// with the -O argument:
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

// The SPIR-V came from this GLSL, which was then optimized using spirv-opt
// with the -O argument:
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

// The SPIR-V comes from the 'matrices_smart_loops' GLSL shader that ships
// with GraphicsFuzz.

const std::string kTestShader4 = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %327 %363 %65 %70 %80 %90 %99 %108 %117 %126 %135 %144 %333
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "matrix_number"
               OpName %12 "cols"
               OpName %23 "rows"
               OpName %31 "c"
               OpName %41 "r"
               OpName %65 "m22"
               OpName %68 "buf0"
               OpMemberName %68 0 "one"
               OpName %70 ""
               OpName %80 "m23"
               OpName %90 "m24"
               OpName %99 "m32"
               OpName %108 "m33"
               OpName %117 "m34"
               OpName %126 "m42"
               OpName %135 "m43"
               OpName %144 "m44"
               OpName %164 "sum_index"
               OpName %165 "cols"
               OpName %173 "rows"
               OpName %184 "sums"
               OpName %189 "c"
               OpName %198 "r"
               OpName %325 "region_x"
               OpName %327 "gl_FragCoord"
               OpName %331 "buf1"
               OpMemberName %331 0 "resolution"
               OpName %333 ""
               OpName %340 "region_y"
               OpName %348 "overall_region"
               OpName %363 "_GLF_color"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %12 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpMemberDecorate %68 0 Offset 0
               OpDecorate %68 Block
               OpDecorate %70 DescriptorSet 0
               OpDecorate %70 Binding 0
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %278 RelaxedPrecision
               OpDecorate %279 RelaxedPrecision
               OpDecorate %280 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
               OpDecorate %298 RelaxedPrecision
               OpDecorate %299 RelaxedPrecision
               OpDecorate %300 RelaxedPrecision
               OpDecorate %309 RelaxedPrecision
               OpDecorate %310 RelaxedPrecision
               OpDecorate %311 RelaxedPrecision
               OpDecorate %312 RelaxedPrecision
               OpDecorate %313 RelaxedPrecision
               OpDecorate %319 RelaxedPrecision
               OpDecorate %320 RelaxedPrecision
               OpDecorate %321 RelaxedPrecision
               OpDecorate %322 RelaxedPrecision
               OpDecorate %323 RelaxedPrecision
               OpDecorate %324 RelaxedPrecision
               OpDecorate %325 RelaxedPrecision
               OpDecorate %327 BuiltIn FragCoord
               OpMemberDecorate %331 0 Offset 0
               OpDecorate %331 Block
               OpDecorate %333 DescriptorSet 0
               OpDecorate %333 Binding 1
               OpDecorate %339 RelaxedPrecision
               OpDecorate %340 RelaxedPrecision
               OpDecorate %347 RelaxedPrecision
               OpDecorate %348 RelaxedPrecision
               OpDecorate %349 RelaxedPrecision
               OpDecorate %351 RelaxedPrecision
               OpDecorate %352 RelaxedPrecision
               OpDecorate %353 RelaxedPrecision
               OpDecorate %354 RelaxedPrecision
               OpDecorate %356 RelaxedPrecision
               OpDecorate %363 Location 0
               OpDecorate %364 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 0
          %7 = OpTypePointer Function %6
          %9 = OpConstant %6 0
         %10 = OpTypeInt 32 1
         %11 = OpTypePointer Function %10
         %13 = OpConstant %10 2
         %20 = OpConstant %10 4
         %21 = OpTypeBool
         %32 = OpConstant %10 0
         %61 = OpTypeFloat 32
         %62 = OpTypeVector %61 2
         %63 = OpTypeMatrix %62 2
         %64 = OpTypePointer Private %63
         %65 = OpVariable %64 Private
         %68 = OpTypeStruct %61
         %69 = OpTypePointer Uniform %68
         %70 = OpVariable %69 Uniform
         %71 = OpTypePointer Uniform %61
         %74 = OpTypePointer Private %61
         %77 = OpTypeVector %61 3
         %78 = OpTypeMatrix %77 2
         %79 = OpTypePointer Private %78
         %80 = OpVariable %79 Private
         %87 = OpTypeVector %61 4
         %88 = OpTypeMatrix %87 2
         %89 = OpTypePointer Private %88
         %90 = OpVariable %89 Private
         %97 = OpTypeMatrix %62 3
         %98 = OpTypePointer Private %97
         %99 = OpVariable %98 Private
        %106 = OpTypeMatrix %77 3
        %107 = OpTypePointer Private %106
        %108 = OpVariable %107 Private
        %115 = OpTypeMatrix %87 3
        %116 = OpTypePointer Private %115
        %117 = OpVariable %116 Private
        %124 = OpTypeMatrix %62 4
        %125 = OpTypePointer Private %124
        %126 = OpVariable %125 Private
        %133 = OpTypeMatrix %77 4
        %134 = OpTypePointer Private %133
        %135 = OpVariable %134 Private
        %142 = OpTypeMatrix %87 4
        %143 = OpTypePointer Private %142
        %144 = OpVariable %143 Private
        %153 = OpConstant %10 1
        %158 = OpConstant %6 1
        %181 = OpConstant %6 9
        %182 = OpTypeArray %61 %181
        %183 = OpTypePointer Function %182
        %186 = OpConstant %61 0
        %187 = OpTypePointer Function %61
        %314 = OpConstant %61 16
        %326 = OpTypePointer Input %87
        %327 = OpVariable %326 Input
        %328 = OpTypePointer Input %61
        %331 = OpTypeStruct %62
        %332 = OpTypePointer Uniform %331
        %333 = OpVariable %332 Uniform
        %336 = OpConstant %61 3
        %350 = OpConstant %10 3
        %357 = OpConstant %10 9
        %362 = OpTypePointer Output %87
        %363 = OpVariable %362 Output
        %368 = OpConstant %61 1
        %374 = OpConstantComposite %87 %186 %186 %186 %368
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %8 = OpVariable %7 Function
         %12 = OpVariable %11 Function
         %23 = OpVariable %11 Function
         %31 = OpVariable %11 Function
         %41 = OpVariable %11 Function
        %164 = OpVariable %11 Function
        %165 = OpVariable %11 Function
        %173 = OpVariable %11 Function
        %184 = OpVariable %183 Function
        %189 = OpVariable %11 Function
        %198 = OpVariable %11 Function
        %325 = OpVariable %11 Function
        %340 = OpVariable %11 Function
        %348 = OpVariable %11 Function
               OpStore %8 %9
               OpStore %12 %13
               OpBranch %14
         %14 = OpLabel
               OpLoopMerge %16 %17 None
               OpBranch %18
         %18 = OpLabel
         %19 = OpLoad %10 %12
         %22 = OpSLessThanEqual %21 %19 %20
               OpBranchConditional %22 %15 %16
         %15 = OpLabel
               OpStore %23 %13
               OpBranch %24
         %24 = OpLabel
               OpLoopMerge %26 %27 None
               OpBranch %28
         %28 = OpLabel
         %29 = OpLoad %10 %23
         %30 = OpSLessThanEqual %21 %29 %20
               OpBranchConditional %30 %25 %26
         %25 = OpLabel
               OpStore %31 %32
               OpBranch %33
         %33 = OpLabel
               OpLoopMerge %35 %36 None
               OpBranch %37
         %37 = OpLabel
         %38 = OpLoad %10 %31
         %39 = OpLoad %10 %12
         %40 = OpSLessThan %21 %38 %39
               OpBranchConditional %40 %34 %35
         %34 = OpLabel
               OpStore %41 %32
               OpBranch %42
         %42 = OpLabel
               OpLoopMerge %44 %45 None
               OpBranch %46
         %46 = OpLabel
         %47 = OpLoad %10 %41
         %48 = OpLoad %10 %23
         %49 = OpSLessThan %21 %47 %48
               OpBranchConditional %49 %43 %44
         %43 = OpLabel
         %50 = OpLoad %6 %8
               OpSelectionMerge %60 None
               OpSwitch %50 %60 0 %51 1 %52 2 %53 3 %54 4 %55 5 %56 6 %57 7 %58 8 %59
         %51 = OpLabel
         %66 = OpLoad %10 %31
         %67 = OpLoad %10 %41
         %72 = OpAccessChain %71 %70 %32
         %73 = OpLoad %61 %72
         %75 = OpAccessChain %74 %65 %66 %67
               OpStore %75 %73
               OpBranch %60
         %52 = OpLabel
         %81 = OpLoad %10 %31
         %82 = OpLoad %10 %41
         %83 = OpAccessChain %71 %70 %32
         %84 = OpLoad %61 %83
         %85 = OpAccessChain %74 %80 %81 %82
               OpStore %85 %84
               OpBranch %60
         %53 = OpLabel
         %91 = OpLoad %10 %31
         %92 = OpLoad %10 %41
         %93 = OpAccessChain %71 %70 %32
         %94 = OpLoad %61 %93
         %95 = OpAccessChain %74 %90 %91 %92
               OpStore %95 %94
               OpBranch %60
         %54 = OpLabel
        %100 = OpLoad %10 %31
        %101 = OpLoad %10 %41
        %102 = OpAccessChain %71 %70 %32
        %103 = OpLoad %61 %102
        %104 = OpAccessChain %74 %99 %100 %101
               OpStore %104 %103
               OpBranch %60
         %55 = OpLabel
        %109 = OpLoad %10 %31
        %110 = OpLoad %10 %41
        %111 = OpAccessChain %71 %70 %32
        %112 = OpLoad %61 %111
        %113 = OpAccessChain %74 %108 %109 %110
               OpStore %113 %112
               OpBranch %60
         %56 = OpLabel
        %118 = OpLoad %10 %31
        %119 = OpLoad %10 %41
        %120 = OpAccessChain %71 %70 %32
        %121 = OpLoad %61 %120
        %122 = OpAccessChain %74 %117 %118 %119
               OpStore %122 %121
               OpBranch %60
         %57 = OpLabel
        %127 = OpLoad %10 %31
        %128 = OpLoad %10 %41
        %129 = OpAccessChain %71 %70 %32
        %130 = OpLoad %61 %129
        %131 = OpAccessChain %74 %126 %127 %128
               OpStore %131 %130
               OpBranch %60
         %58 = OpLabel
        %136 = OpLoad %10 %31
        %137 = OpLoad %10 %41
        %138 = OpAccessChain %71 %70 %32
        %139 = OpLoad %61 %138
        %140 = OpAccessChain %74 %135 %136 %137
               OpStore %140 %139
               OpBranch %60
         %59 = OpLabel
        %145 = OpLoad %10 %31
        %146 = OpLoad %10 %41
        %147 = OpAccessChain %71 %70 %32
        %148 = OpLoad %61 %147
        %149 = OpAccessChain %74 %144 %145 %146
               OpStore %149 %148
               OpBranch %60
         %60 = OpLabel
               OpBranch %45
         %45 = OpLabel
        %152 = OpLoad %10 %41
        %154 = OpIAdd %10 %152 %153
               OpStore %41 %154
               OpBranch %42
         %44 = OpLabel
               OpBranch %36
         %36 = OpLabel
        %155 = OpLoad %10 %31
        %156 = OpIAdd %10 %155 %153
               OpStore %31 %156
               OpBranch %33
         %35 = OpLabel
        %157 = OpLoad %6 %8
        %159 = OpIAdd %6 %157 %158
               OpStore %8 %159
               OpBranch %27
         %27 = OpLabel
        %160 = OpLoad %10 %23
        %161 = OpIAdd %10 %160 %153
               OpStore %23 %161
               OpBranch %24
         %26 = OpLabel
               OpBranch %17
         %17 = OpLabel
        %162 = OpLoad %10 %12
        %163 = OpIAdd %10 %162 %153
               OpStore %12 %163
               OpBranch %14
         %16 = OpLabel
               OpStore %164 %32
               OpStore %165 %13
               OpBranch %166
        %166 = OpLabel
               OpLoopMerge %168 %169 None
               OpBranch %170
        %170 = OpLabel
        %171 = OpLoad %10 %165
        %172 = OpSLessThanEqual %21 %171 %20
               OpBranchConditional %172 %167 %168
        %167 = OpLabel
               OpStore %173 %13
               OpBranch %174
        %174 = OpLabel
               OpLoopMerge %176 %177 None
               OpBranch %178
        %178 = OpLabel
        %179 = OpLoad %10 %173
        %180 = OpSLessThanEqual %21 %179 %20
               OpBranchConditional %180 %175 %176
        %175 = OpLabel
        %185 = OpLoad %10 %164
        %188 = OpAccessChain %187 %184 %185
               OpStore %188 %186
               OpStore %189 %32
               OpBranch %190
        %190 = OpLabel
               OpLoopMerge %192 %193 None
               OpBranch %194
        %194 = OpLabel
        %195 = OpLoad %10 %189
        %196 = OpLoad %10 %165
        %197 = OpSLessThan %21 %195 %196
               OpBranchConditional %197 %191 %192
        %191 = OpLabel
               OpStore %198 %32
               OpBranch %199
        %199 = OpLabel
               OpLoopMerge %201 %202 None
               OpBranch %203
        %203 = OpLabel
        %204 = OpLoad %10 %198
        %205 = OpLoad %10 %173
        %206 = OpSLessThan %21 %204 %205
               OpBranchConditional %206 %200 %201
        %200 = OpLabel
        %207 = OpLoad %10 %164
               OpSelectionMerge %217 None
               OpSwitch %207 %217 0 %208 1 %209 2 %210 3 %211 4 %212 5 %213 6 %214 7 %215 8 %216
        %208 = OpLabel
        %218 = OpLoad %10 %164
        %219 = OpLoad %10 %189
        %220 = OpLoad %10 %198
        %221 = OpAccessChain %74 %65 %219 %220
        %222 = OpLoad %61 %221
        %223 = OpAccessChain %187 %184 %218
        %224 = OpLoad %61 %223
        %225 = OpFAdd %61 %224 %222
        %226 = OpAccessChain %187 %184 %218
               OpStore %226 %225
               OpBranch %217
        %209 = OpLabel
        %228 = OpLoad %10 %164
        %229 = OpLoad %10 %189
        %230 = OpLoad %10 %198
        %231 = OpAccessChain %74 %80 %229 %230
        %232 = OpLoad %61 %231
        %233 = OpAccessChain %187 %184 %228
        %234 = OpLoad %61 %233
        %235 = OpFAdd %61 %234 %232
        %236 = OpAccessChain %187 %184 %228
               OpStore %236 %235
               OpBranch %217
        %210 = OpLabel
        %238 = OpLoad %10 %164
        %239 = OpLoad %10 %189
        %240 = OpLoad %10 %198
        %241 = OpAccessChain %74 %90 %239 %240
        %242 = OpLoad %61 %241
        %243 = OpAccessChain %187 %184 %238
        %244 = OpLoad %61 %243
        %245 = OpFAdd %61 %244 %242
        %246 = OpAccessChain %187 %184 %238
               OpStore %246 %245
               OpBranch %217
        %211 = OpLabel
        %248 = OpLoad %10 %164
        %249 = OpLoad %10 %189
        %250 = OpLoad %10 %198
        %251 = OpAccessChain %74 %99 %249 %250
        %252 = OpLoad %61 %251
        %253 = OpAccessChain %187 %184 %248
        %254 = OpLoad %61 %253
        %255 = OpFAdd %61 %254 %252
        %256 = OpAccessChain %187 %184 %248
               OpStore %256 %255
               OpBranch %217
        %212 = OpLabel
        %258 = OpLoad %10 %164
        %259 = OpLoad %10 %189
        %260 = OpLoad %10 %198
        %261 = OpAccessChain %74 %108 %259 %260
        %262 = OpLoad %61 %261
        %263 = OpAccessChain %187 %184 %258
        %264 = OpLoad %61 %263
        %265 = OpFAdd %61 %264 %262
        %266 = OpAccessChain %187 %184 %258
               OpStore %266 %265
               OpBranch %217
        %213 = OpLabel
        %268 = OpLoad %10 %164
        %269 = OpLoad %10 %189
        %270 = OpLoad %10 %198
        %271 = OpAccessChain %74 %117 %269 %270
        %272 = OpLoad %61 %271
        %273 = OpAccessChain %187 %184 %268
        %274 = OpLoad %61 %273
        %275 = OpFAdd %61 %274 %272
        %276 = OpAccessChain %187 %184 %268
               OpStore %276 %275
               OpBranch %217
        %214 = OpLabel
        %278 = OpLoad %10 %164
        %279 = OpLoad %10 %189
        %280 = OpLoad %10 %198
        %281 = OpAccessChain %74 %126 %279 %280
        %282 = OpLoad %61 %281
        %283 = OpAccessChain %187 %184 %278
        %284 = OpLoad %61 %283
        %285 = OpFAdd %61 %284 %282
        %286 = OpAccessChain %187 %184 %278
               OpStore %286 %285
               OpBranch %217
        %215 = OpLabel
        %288 = OpLoad %10 %164
        %289 = OpLoad %10 %189
        %290 = OpLoad %10 %198
        %291 = OpAccessChain %74 %135 %289 %290
        %292 = OpLoad %61 %291
        %293 = OpAccessChain %187 %184 %288
        %294 = OpLoad %61 %293
        %295 = OpFAdd %61 %294 %292
        %296 = OpAccessChain %187 %184 %288
               OpStore %296 %295
               OpBranch %217
        %216 = OpLabel
        %298 = OpLoad %10 %164
        %299 = OpLoad %10 %189
        %300 = OpLoad %10 %198
        %301 = OpAccessChain %74 %144 %299 %300
        %302 = OpLoad %61 %301
        %303 = OpAccessChain %187 %184 %298
        %304 = OpLoad %61 %303
        %305 = OpFAdd %61 %304 %302
        %306 = OpAccessChain %187 %184 %298
               OpStore %306 %305
               OpBranch %217
        %217 = OpLabel
               OpBranch %202
        %202 = OpLabel
        %309 = OpLoad %10 %198
        %310 = OpIAdd %10 %309 %153
               OpStore %198 %310
               OpBranch %199
        %201 = OpLabel
               OpBranch %193
        %193 = OpLabel
        %311 = OpLoad %10 %189
        %312 = OpIAdd %10 %311 %153
               OpStore %189 %312
               OpBranch %190
        %192 = OpLabel
        %313 = OpLoad %10 %164
        %315 = OpAccessChain %187 %184 %313
        %316 = OpLoad %61 %315
        %317 = OpFDiv %61 %316 %314
        %318 = OpAccessChain %187 %184 %313
               OpStore %318 %317
        %319 = OpLoad %10 %164
        %320 = OpIAdd %10 %319 %153
               OpStore %164 %320
               OpBranch %177
        %177 = OpLabel
        %321 = OpLoad %10 %173
        %322 = OpIAdd %10 %321 %153
               OpStore %173 %322
               OpBranch %174
        %176 = OpLabel
               OpBranch %169
        %169 = OpLabel
        %323 = OpLoad %10 %165
        %324 = OpIAdd %10 %323 %153
               OpStore %165 %324
               OpBranch %166
        %168 = OpLabel
        %329 = OpAccessChain %328 %327 %9
        %330 = OpLoad %61 %329
        %334 = OpAccessChain %71 %333 %32 %9
        %335 = OpLoad %61 %334
        %337 = OpFDiv %61 %335 %336
        %338 = OpFDiv %61 %330 %337
        %339 = OpConvertFToS %10 %338
               OpStore %325 %339
        %341 = OpAccessChain %328 %327 %158
        %342 = OpLoad %61 %341
        %343 = OpAccessChain %71 %333 %32 %9
        %344 = OpLoad %61 %343
        %345 = OpFDiv %61 %344 %336
        %346 = OpFDiv %61 %342 %345
        %347 = OpConvertFToS %10 %346
               OpStore %340 %347
        %349 = OpLoad %10 %340
        %351 = OpIMul %10 %349 %350
        %352 = OpLoad %10 %325
        %353 = OpIAdd %10 %351 %352
               OpStore %348 %353
        %354 = OpLoad %10 %348
        %355 = OpSGreaterThan %21 %354 %32
        %356 = OpLoad %10 %348
        %358 = OpSLessThan %21 %356 %357
        %359 = OpLogicalAnd %21 %355 %358
               OpSelectionMerge %361 None
               OpBranchConditional %359 %360 %373
        %360 = OpLabel
        %364 = OpLoad %10 %348
        %365 = OpAccessChain %187 %184 %364
        %366 = OpLoad %61 %365
        %367 = OpCompositeConstruct %77 %366 %366 %366
        %369 = OpCompositeExtract %61 %367 0
        %370 = OpCompositeExtract %61 %367 1
        %371 = OpCompositeExtract %61 %367 2
        %372 = OpCompositeConstruct %87 %369 %370 %371 %368
               OpStore %363 %372
               OpBranch %361
        %373 = OpLabel
               OpStore %363 %374
               OpBranch %361
        %361 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

void AddConstantUniformFact(protobufs::FactSequence* facts,
                            uint32_t descriptor_set, uint32_t binding,
                            std::vector<uint32_t>&& indices, uint32_t value) {
  protobufs::FactConstantUniform fact;
  *fact.mutable_uniform_buffer_element_descriptor() =
      MakeUniformBufferElementDescriptor(descriptor_set, binding,
                                         std::move(indices));
  *fact.mutable_constant_word()->Add() = value;
  protobufs::Fact temp;
  *temp.mutable_constant_uniform_fact() = fact;
  *facts->mutable_fact()->Add() = temp;
}

// Reinterpret the bits of |value| as a 32-bit unsigned int
uint32_t FloatBitsAsUint(float value) {
  uint32_t result;
  memcpy(&result, &value, sizeof(float));
  return result;
}

// Assembles the given |shader| text, and then runs the fuzzer |num_runs|
// times, using successive seeds starting from |initial_seed|.  Checks that
// the binary produced after each fuzzer run is valid, and that replaying
// the transformations that were applied during fuzzing leads to an
// identical binary.
void RunFuzzerAndReplayer(const std::string& shader,
                          const protobufs::FactSequence& initial_facts,
                          uint32_t initial_seed, uint32_t num_runs) {
  const auto env = SPV_ENV_UNIVERSAL_1_5;

  std::vector<uint32_t> binary_in;
  SpirvTools t(env);
  t.SetMessageConsumer(kConsoleMessageConsumer);
  ASSERT_TRUE(t.Assemble(shader, &binary_in, kFuzzAssembleOption));
  ASSERT_TRUE(t.Validate(binary_in));

  std::vector<fuzzerutil::ModuleSupplier> donor_suppliers;
  for (auto donor :
       {&kTestShader1, &kTestShader2, &kTestShader3, &kTestShader4}) {
    donor_suppliers.emplace_back([donor]() {
      return BuildModule(env, kConsoleMessageConsumer, *donor,
                         kFuzzAssembleOption);
    });
  }

  for (uint32_t seed = initial_seed; seed < initial_seed + num_runs; seed++) {
    std::vector<uint32_t> fuzzer_binary_out;
    protobufs::TransformationSequence fuzzer_transformation_sequence_out;

    Fuzzer fuzzer(env, seed, true);
    fuzzer.SetMessageConsumer(kSilentConsumer);
    auto fuzzer_result_status =
        fuzzer.Run(binary_in, initial_facts, donor_suppliers,
                   &fuzzer_binary_out, &fuzzer_transformation_sequence_out);
    ASSERT_EQ(Fuzzer::FuzzerResultStatus::kComplete, fuzzer_result_status);
    ASSERT_TRUE(t.Validate(fuzzer_binary_out));

    std::vector<uint32_t> replayer_binary_out;
    protobufs::TransformationSequence replayer_transformation_sequence_out;

    Replayer replayer(env, false);
    replayer.SetMessageConsumer(kSilentConsumer);
    auto replayer_result_status = replayer.Run(
        binary_in, initial_facts, fuzzer_transformation_sequence_out,
        &replayer_binary_out, &replayer_transformation_sequence_out);
    ASSERT_EQ(Replayer::ReplayerResultStatus::kComplete,
              replayer_result_status);

    // After replaying the transformations applied by the fuzzer, exactly those
    // transformations should have been applied, and the binary resulting from
    // replay should be identical to that which resulted from fuzzing.
    std::string fuzzer_transformations_string;
    std::string replayer_transformations_string;
    fuzzer_transformation_sequence_out.SerializeToString(
        &fuzzer_transformations_string);
    replayer_transformation_sequence_out.SerializeToString(
        &replayer_transformations_string);
    ASSERT_EQ(fuzzer_transformations_string, replayer_transformations_string);
    ASSERT_EQ(fuzzer_binary_out, replayer_binary_out);
  }
}

TEST(FuzzerReplayerTest, Miscellaneous1) {
  // Do some fuzzer runs, starting from an initial seed of 0 (seed value chosen
  // arbitrarily).
  RunFuzzerAndReplayer(kTestShader1, protobufs::FactSequence(), 0,
                       kNumFuzzerRuns);
}

TEST(FuzzerReplayerTest, Miscellaneous2) {
  // Do some fuzzer runs, starting from an initial seed of 10 (seed value chosen
  // arbitrarily).
  RunFuzzerAndReplayer(kTestShader2, protobufs::FactSequence(), 10,
                       kNumFuzzerRuns);
}

TEST(FuzzerReplayerTest, Miscellaneous3) {
  // Add the facts "resolution.x == 250" and "resolution.y == 100".
  protobufs::FactSequence facts;
  AddConstantUniformFact(&facts, 0, 0, {0, 0}, 250);
  AddConstantUniformFact(&facts, 0, 0, {0, 1}, 100);

  // Do some fuzzer runs, starting from an initial seed of 94 (seed value chosen
  // arbitrarily).
  RunFuzzerAndReplayer(kTestShader3, facts, 94, kNumFuzzerRuns);
}

TEST(FuzzerReplayerTest, Miscellaneous4) {
  // Add the facts:
  //  - "one == 1.0"
  //  - "resolution.y == 256.0",
  protobufs::FactSequence facts;
  AddConstantUniformFact(&facts, 0, 0, {0}, FloatBitsAsUint(1.0));
  AddConstantUniformFact(&facts, 0, 1, {0, 0}, FloatBitsAsUint(256.0));
  AddConstantUniformFact(&facts, 0, 1, {0, 1}, FloatBitsAsUint(256.0));

  // Do some fuzzer runs, starting from an initial seed of 14 (seed value chosen
  // arbitrarily).
  RunFuzzerAndReplayer(kTestShader4, facts, 14, kNumFuzzerRuns);
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
