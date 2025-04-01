// Copyright (c) 2025 Google LLC
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

#include <iostream>
#include <ostream>

#include "spirv-tools/optimizer.hpp"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

struct ResolveBindingConflictsTest : public PassTest<::testing::Test> {
  virtual void SetUp() override {
    SetTargetEnv(SPV_ENV_VULKAN_1_1);  // allow storage buffer storage class
                                       // without extension.
    SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
    SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
                          SPV_BINARY_TO_TEXT_OPTION_INDENT |
                          SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
  }
};

using StringList = std::vector<std::string>;
std::string EntryPointDecls(const StringList& names) {
  std::ostringstream os;
  for (auto& name : names) {
    os << "               OpEntryPoint GLCompute %" + name + " \"" + name +
              "\"\n";
  }
  for (auto& name : names) {
    os << "               OpExecutionMode %" + name + " LocalSize 1 1 1\n";
  }
  for (auto& name : names) {
    os << "               OpName %" + name + " \"" + name + "\"\n";
  }
  return os.str();
}

std::string Preamble(const StringList& names = {"main"}) {
  return R"(               OpCapability Shader
               OpMemoryModel Logical GLSL450
)" + EntryPointDecls(names) +
         R"(               OpName %voidfn "voidfn"
               OpName %s_ty "s_ty"
               OpName %i_ty "i_ty"
               OpName %si_ty "si_ty"
               OpName %p_s_ty "p_s_ty"
               OpName %p_i_ty "p_i_ty"
               OpName %p_si_ty "p_si_ty"
               OpName %st_ty "st_ty"
               OpName %pu_st_ty "pu_st_ty"
               OpName %pb_st_ty "pb_st_ty"
)";
}

std::string BasicTypes() {
  return R"(               OpDecorate %st_ty Block
               OpMemberDecorate %st_ty 0 Offset 0
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_3 = OpConstant %uint 3
       %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
       %s_ty = OpTypeSampler
       %i_ty = OpTypeImage %float 2D 0 0 0 1 Unknown
      %si_ty = OpTypeSampledImage %i_ty
     %p_i_ty = OpTypePointer UniformConstant %i_ty
     %p_s_ty = OpTypePointer UniformConstant %s_ty
    %p_si_ty = OpTypePointer UniformConstant %si_ty
      %st_ty = OpTypeStruct %uint
   %pu_st_ty = OpTypePointer Uniform %st_ty
   %pb_st_ty = OpTypePointer StorageBuffer %st_ty
)";
}
std::string NoCheck() { return "; CHECK-NOT: nothing to see"; }

TEST_F(ResolveBindingConflictsTest, NoBindings_NoChange) {
  const std::string kTest = Preamble() + BasicTypes() +
                            R"(       %main = OpFunction %void None %voidfn
        %100 = OpLabel
               OpReturn
               OpFunctionEnd
)";
  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest + NoCheck(), /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithoutChange);
  EXPECT_EQ(kTest, disasm);
}

TEST_F(ResolveBindingConflictsTest, NoConflict_UnusedVars_NoChange) {
  const std::string kTest = Preamble() +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 0
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 0
               OpDecorate %104 DescriptorSet 0
               OpDecorate %104 Binding 0

      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 0
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 0
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 0
      ; CHECK: OpDecorate %104 DescriptorSet 0
      ; CHECK: OpDecorate %104 Binding 0

)" + BasicTypes() + R"(

 ; Unused variables

        %100 = OpVariable %p_i_ty UniformConstant  ; image
        %101 = OpVariable %p_s_ty UniformConstant  ; sampler
        %102 = OpVariable %pu_st_ty Uniform        ; UBO
        %103 = OpVariable %pb_st_ty StorageBuffer  ; SSBO
        %104 = OpVariable %p_si_ty UniformConstant ; combined sampled image

       %main = OpFunction %void None %voidfn
         %10 = OpLabel
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithoutChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest, NoConflict_UsedVars_NoChange) {
  const std::string kTest = Preamble() +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 1
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 2
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 3
               OpDecorate %110 DescriptorSet 1
               OpDecorate %110 Binding 0

      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 0
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 1
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 2
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 3
      ; CHECK: OpDecorate %110 DescriptorSet 1
      ; CHECK: OpDecorate %110 Binding 0

)" + BasicTypes() + R"(

        %100 = OpVariable %p_i_ty UniformConstant  ; image
        %101 = OpVariable %p_s_ty UniformConstant  ; sampler
        %102 = OpVariable %pu_st_ty Uniform        ; UBO
        %103 = OpVariable %pb_st_ty StorageBuffer  ; SSBO
        %110 = OpVariable %p_si_ty UniformConstant ; combined sampled image

       %main = OpFunction %void None %voidfn
         %10 = OpLabel
         %11 = OpCopyObject %p_i_ty %100
         %12 = OpCopyObject %p_s_ty %101
         %13 = OpCopyObject %pu_st_ty %102
         %14 = OpCopyObject %pb_st_ty %103
         %15 = OpCopyObject %p_si_ty %110
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithoutChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest,
       OneEntryPoint_SamplerFirstConflict_Resolves) {
  const std::string kTest = Preamble() +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0

      ; The sampler's binding number is incremented, even when listed first.
      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 1
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0

)" + BasicTypes() + R"(

        %100 = OpVariable %p_s_ty UniformConstant ; sampler listed first
        %101 = OpVariable %p_i_ty UniformConstant

       %main = OpFunction %void None %voidfn
         %10 = OpLabel
         %11 = OpCopyObject %p_s_ty %100
         %12 = OpCopyObject %p_i_ty %101
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest,
       OneEntryPoint_SamplerSecondConflict_Resolves) {
  const std::string kTest = Preamble() +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0

      ; The sampler's binding number is incremented, even when listed second.
      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 0
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 1

)" + BasicTypes() + R"(

        %100 = OpVariable %p_i_ty UniformConstant
        %101 = OpVariable %p_s_ty UniformConstant ; sampler listed second

       %main = OpFunction %void None %voidfn
         %10 = OpLabel
         %11 = OpCopyObject %p_i_ty %100
         %12 = OpCopyObject %p_s_ty %101
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest, OneEntryPoint_Conflict_Ripples) {
  const std::string kTest = Preamble() +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 1
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 2
               OpDecorate %104 DescriptorSet 0
               OpDecorate %104 Binding 3

      ; The sampler's binding number is incremented, and later
      ; bindings move out of the way.
      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 1
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 2
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 3
      ; CHECK: OpDecorate %104 DescriptorSet 0
      ; CHECK: OpDecorate %104 Binding 4

)" + BasicTypes() + R"(

        %100 = OpVariable %p_s_ty UniformConstant ; sampler comes first
        %101 = OpVariable %p_i_ty UniformConstant
        %102 = OpVariable %pu_st_ty Uniform
        %103 = OpVariable %pb_st_ty StorageBuffer
        %104 = OpVariable %p_si_ty UniformConstant

       %main = OpFunction %void None %voidfn
         %10 = OpLabel
         %11 = OpCopyObject %p_s_ty %100
         %12 = OpCopyObject %p_i_ty %101
         %13 = OpCopyObject %pu_st_ty %102
         %14 = OpCopyObject %pb_st_ty %103
         %15 = OpCopyObject %p_si_ty %104
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest,
       OneEntryPoint_Conflict_RippleStopsAtFirstHole) {
  const std::string kTest = Preamble() +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 1
               ; Leave a hole at (0, 2)
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 3
               OpDecorate %104 DescriptorSet 0
               OpDecorate %104 Binding 4

      ; There was a hole at binding 2.  The ripple stops there.
      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 1
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 2
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 3
      ; CHECK: OpDecorate %104 DescriptorSet 0
      ; CHECK: OpDecorate %104 Binding 4

)" + BasicTypes() + R"(

        %100 = OpVariable %p_s_ty UniformConstant ; sampler comes first
        %101 = OpVariable %p_i_ty UniformConstant
        %102 = OpVariable %pu_st_ty Uniform
        %103 = OpVariable %pb_st_ty StorageBuffer
        %104 = OpVariable %p_si_ty UniformConstant

       %main = OpFunction %void None %voidfn
         %10 = OpLabel
         %11 = OpCopyObject %p_s_ty %100
         %12 = OpCopyObject %p_i_ty %101
         %13 = OpCopyObject %pu_st_ty %102
         %14 = OpCopyObject %pb_st_ty %103
         %15 = OpCopyObject %p_si_ty %104
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest, OneEntryPoint_MultiConflict_Resolves) {
  const std::string kTest = Preamble() +
                            R"(
      ; Two conflicts: at Bindings 0, and 1
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 1
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 1
               OpDecorate %104 DescriptorSet 0
               OpDecorate %104 Binding 2

      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 1
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 2
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 3
      ; CHECK: OpDecorate %104 DescriptorSet 0
      ; CHECK: OpDecorate %104 Binding 4

)" + BasicTypes() + R"(

        %100 = OpVariable %p_s_ty UniformConstant ; sampler first
        %101 = OpVariable %p_i_ty UniformConstant
        %102 = OpVariable %p_i_ty UniformConstant
        %103 = OpVariable %p_s_ty UniformConstant ; sampler second
        %104 = OpVariable %pu_st_ty Uniform

       %main = OpFunction %void None %voidfn
         %10 = OpLabel
         %11 = OpCopyObject %p_s_ty %100
         %12 = OpCopyObject %p_i_ty %101
         %13 = OpCopyObject %p_i_ty %102
         %14 = OpCopyObject %p_s_ty %103
         %15 = OpCopyObject %pu_st_ty %104
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest,
       OneEntryPoint_MultiConflict_ComplexCallGraph_Resolves) {
  // Check that uses are seen even when used at various points in a complex call
  // graph.
  const std::string kTest = Preamble() +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 1
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 1
               OpDecorate %104 DescriptorSet 0
               OpDecorate %104 Binding 2

      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 1
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 2
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 3
      ; CHECK: OpDecorate %104 DescriptorSet 0
      ; CHECK: OpDecorate %104 Binding 4

)" + BasicTypes() + R"(

        %100 = OpVariable %p_s_ty UniformConstant ; used in %200
        %101 = OpVariable %p_i_ty UniformConstant ; used in %300, %400
        %102 = OpVariable %p_i_ty UniformConstant ; used in %500
        %103 = OpVariable %p_s_ty UniformConstant ; used in %400 twice
        %104 = OpVariable %pu_st_ty Uniform       ; used in %600

        %200 = OpFunction %void None %voidfn
        %201 = OpLabel
        %202 = OpCopyObject %p_s_ty %100
               OpReturn
               OpFunctionEnd

        %300 = OpFunction %void None %voidfn
        %301 = OpLabel
        %302 = OpCopyObject %p_i_ty %101
               OpReturn
               OpFunctionEnd

        %400 = OpFunction %void None %voidfn
        %401 = OpLabel
        %402 = OpFunctionCall %void %200
        %403 = OpCopyObject %p_s_ty %103
        %404 = OpCopyObject %p_i_ty %101
        %405 = OpCopyObject %p_s_ty %103
        %406 = OpFunctionCall %void %300
               OpReturn
               OpFunctionEnd

        %500 = OpFunction %void None %voidfn
        %501 = OpLabel
        %502 = OpFunctionCall %void %400
        %503 = OpCopyObject %p_i_ty %102
        %504 = OpFunctionCall %void %300
               OpReturn
               OpFunctionEnd

        %600 = OpFunction %void None %voidfn
        %601 = OpLabel
        %602 = OpFunctionCall %void %300
        %603 = OpFunctionCall %void %500
        %604 = OpCopyObject %pu_st_ty %104
               OpReturn
               OpFunctionEnd

       %main = OpFunction %void None %voidfn
       %1000 = OpLabel
       %1001 = OpFunctionCall %void %600
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest,
       MultiEntryPoint_DuplicatConflicts_ResolvesOnlyOnce) {
  // Before:
  //
  //   Binding:   0          1
  //   Alpha:    %100,%101
  //   Beta:     %100,%101
  //
  // After:
  //
  //   Binding:   0          1
  //   Alpha:    %101       %100
  //   Beta:     %101       %100
  const std::string kTest = Preamble({"alpha", "beta"}) +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0

      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 1
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0

)" + BasicTypes() + R"(

        %100 = OpVariable %p_s_ty UniformConstant
        %101 = OpVariable %p_i_ty UniformConstant

      %alpha = OpFunction %void None %voidfn
       %1000 = OpLabel
       %1001 = OpCopyObject %p_s_ty %100
       %1002 = OpCopyObject %p_i_ty %101
               OpReturn
               OpFunctionEnd

       %beta = OpFunction %void None %voidfn
       %2000 = OpLabel
       %2001 = OpCopyObject %p_s_ty %100
       %2002 = OpCopyObject %p_i_ty %101
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest,
       MultiEntryPoint_IndependentConflicts_Resolves) {
  // Before:
  //
  //   Binding:   0          1
  //   Alpha:    %100,%101
  //   Beta:     %102,%103
  //
  // After:
  //
  //   Binding:   0          1
  //   Alpha:    %101       %100
  //   Beta:     %102       %103
  const std::string kTest = Preamble({"alpha", "beta"}) +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 0
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 0

      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 1
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 0
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 1

)" + BasicTypes() + R"(

        %100 = OpVariable %p_s_ty UniformConstant
        %101 = OpVariable %p_i_ty UniformConstant
        %102 = OpVariable %p_i_ty UniformConstant
        %103 = OpVariable %p_s_ty UniformConstant

      %alpha = OpFunction %void None %voidfn
       %1000 = OpLabel
       %1001 = OpCopyObject %p_s_ty %100
       %1002 = OpCopyObject %p_i_ty %101
               OpReturn
               OpFunctionEnd

       %beta = OpFunction %void None %voidfn
       %2000 = OpLabel
       %2001 = OpCopyObject %p_i_ty %102
       %2002 = OpCopyObject %p_s_ty %103
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest,
       MultiEntryPoint_SameVarConflictsAcrossMultiEntryPoints_Resolves) {
  // A sampler variable is bumped, causing potential conflicts in other shaders.
  //
  // Before:
  //
  //   Binding:   0          1        2
  //   Alpha:    %100,%101
  //   Beta:     %100       %102
  //   Gamma:    %100                %103
  //
  // After:
  //
  //   Binding:   0          1        2
  //   Alpha:    %101       %100
  //   Beta:                %100     %102
  //   Gamma:    %100                %103
  //
  const std::string kTest = Preamble({"alpha", "beta", "gamma"}) +
                            R"(
               OpDecorate %100 DescriptorSet 0  ; The sampler
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 1
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 2

      ; bumped once
      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 1

      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0

      ; pushed back from bump of %100
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 2

      ; does not need to be bumped
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 2

)" + BasicTypes() + R"(

        %100 = OpVariable %p_s_ty UniformConstant ; used in alpha, beta, gamma
        %101 = OpVariable %p_i_ty UniformConstant ; used in alpha
        %102 = OpVariable %pu_st_ty Uniform       ; used in beta
        %103 = OpVariable %pb_st_ty StorageBuffer ; used in gamma

      %alpha = OpFunction %void None %voidfn
       %1000 = OpLabel
       %1001 = OpCopyObject %p_s_ty %100
       %1002 = OpCopyObject %p_i_ty %101
               OpReturn
               OpFunctionEnd

       %beta = OpFunction %void None %voidfn
       %2000 = OpLabel
       %2001 = OpCopyObject %p_s_ty %100
       %2002 = OpCopyObject %pu_st_ty %102
               OpReturn
               OpFunctionEnd

      %gamma = OpFunction %void None %voidfn
       %3000 = OpLabel
       %3001 = OpCopyObject %p_s_ty %100
       %3002 = OpCopyObject %pb_st_ty %103
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest, MultiEntryPoint_ConflictCascade_Resolves) {
  // Before:
  //
  //   Binding:   0          1        2        3
  //   Alpha:    %100,%101
  //   Beta:     %100       %102
  //   Gamma:               %102     %103
  //   Delta:                        %103     %104
  //
  // After:
  //
  //   Binding:   0          1        2        3        4
  //   Alpha:    %101       %100
  //   Beta:                %100     %102
  //   Gamma:                        %102     %103
  //   Delta:                                 %103     %104
  //
  const std::string kTest = Preamble({"alpha", "beta", "gamma", "delta"}) +
                            R"(
               OpDecorate %100 DescriptorSet 0  ; The sampler
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0
               OpDecorate %101 Binding 0
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 1
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 2
               OpDecorate %104 DescriptorSet 0
               OpDecorate %104 Binding 3

      ; %100 is bumped once:
      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 1

      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 0

      ; pushed back from bump of %100
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 2

      ; pushed back from bump of %102
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 3

      ; pushed back from bump of %103
      ; CHECK: OpDecorate %104 DescriptorSet 0
      ; CHECK: OpDecorate %104 Binding 4

)" + BasicTypes() + R"(

        %100 = OpVariable %p_s_ty UniformConstant  ; used in alpha, beta
        %101 = OpVariable %p_i_ty UniformConstant  ; used in alpha
        %102 = OpVariable %pu_st_ty Uniform        ; used in beta, gamma
        %103 = OpVariable %pb_st_ty StorageBuffer  ; used in gamma, delta
        %104 = OpVariable %p_si_ty UniformConstant ; used delta

      %alpha = OpFunction %void None %voidfn
       %1000 = OpLabel
       %1001 = OpCopyObject %p_s_ty %100
       %1002 = OpCopyObject %p_i_ty %101
               OpReturn
               OpFunctionEnd

       %beta = OpFunction %void None %voidfn
       %2000 = OpLabel
       %2001 = OpCopyObject %p_s_ty %100
       %2002 = OpCopyObject %pu_st_ty %102
               OpReturn
               OpFunctionEnd

      %gamma = OpFunction %void None %voidfn
       %3000 = OpLabel
       %3001 = OpCopyObject %pu_st_ty %102
       %3002 = OpCopyObject %pb_st_ty %103
               OpReturn
               OpFunctionEnd

      %delta = OpFunction %void None %voidfn
       %4000 = OpLabel
       %4001 = OpCopyObject %pb_st_ty %103
       %4002 = OpCopyObject %p_si_ty %104
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

TEST_F(ResolveBindingConflictsTest,
       MultiEntryPoint_ConflictCascade_RevisitEntryPoint) {
  // Prove that the settling algorithm knows to revisit entry points that
  // already had all their own conflicts resolved.
  //
  // Before:
  //
  //   Binding:   0          1       2        3       4
  //   Alpha:    %100,%101          %103     %104    %105
  //   Beta:          %101  %102    %103             %105
  //
  // After:
  //
  //   Binding:    0         1       2        3       4       5
  //   Alpha:    %100       %101             %103    %104    %105
  //   Beta:                %101    %102     %103            %105
  const std::string kTest = Preamble({"alpha", "beta"}) +
                            R"(
               OpDecorate %100 DescriptorSet 0
               OpDecorate %100 Binding 0
               OpDecorate %101 DescriptorSet 0 ; the sampler
               OpDecorate %101 Binding 0
               OpDecorate %102 DescriptorSet 0
               OpDecorate %102 Binding 1
               OpDecorate %103 DescriptorSet 0
               OpDecorate %103 Binding 2
               OpDecorate %104 DescriptorSet 0
               OpDecorate %104 Binding 3
               OpDecorate %105 DescriptorSet 0
               OpDecorate %105 Binding 4

      ; CHECK: OpDecorate %100 DescriptorSet 0
      ; CHECK: OpDecorate %100 Binding 0
      ; CHECK: OpDecorate %101 DescriptorSet 0
      ; CHECK: OpDecorate %101 Binding 1
      ; CHECK: OpDecorate %102 DescriptorSet 0
      ; CHECK: OpDecorate %102 Binding 2
      ; CHECK: OpDecorate %103 DescriptorSet 0
      ; CHECK: OpDecorate %103 Binding 3
      ; CHECK: OpDecorate %104 DescriptorSet 0
      ; CHECK: OpDecorate %104 Binding 4
      ; CHECK: OpDecorate %105 DescriptorSet 0
      ; CHECK: OpDecorate %105 Binding 5

)" + BasicTypes() + R"(

        %100 = OpVariable %p_i_ty UniformConstant
        %101 = OpVariable %p_s_ty UniformConstant
        %102 = OpVariable %pu_st_ty Uniform
        %103 = OpVariable %pb_st_ty StorageBuffer
        %104 = OpVariable %p_si_ty UniformConstant
        %105 = OpVariable %p_s_ty UniformConstant

      %alpha = OpFunction %void None %voidfn
       %1000 = OpLabel
       %1001 = OpCopyObject %p_i_ty %100
       %1002 = OpCopyObject %p_s_ty %101
       %1003 = OpCopyObject %pb_st_ty %103
       %1004 = OpCopyObject %p_si_ty %104
       %1005 = OpCopyObject %p_s_ty %105
               OpReturn
               OpFunctionEnd

       %beta = OpFunction %void None %voidfn
       %2000 = OpLabel
       %2001 = OpCopyObject %p_s_ty %101
       %2002 = OpCopyObject %pu_st_ty %102
       %2003 = OpCopyObject %pb_st_ty %103
       %2004 = OpCopyObject %p_s_ty %105
               OpReturn
               OpFunctionEnd
)";

  auto [disasm, status] = SinglePassRunAndMatch<ResolveBindingConflictsPass>(
      kTest, /* do_validation= */ true);
  EXPECT_EQ(status, Pass::Status::SuccessWithChange) << disasm;
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
