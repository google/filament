// Copyright (c) 2018 Google LLC
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

#include "source/reduce/reducer.h"

#include "source/opt/build_module.h"
#include "source/reduce/operand_to_const_reduction_opportunity_finder.h"
#include "source/reduce/remove_unreferenced_instruction_reduction_opportunity_finder.h"
#include "test/reduce/reduce_test_util.h"

namespace spvtools {
namespace reduce {
namespace {

using opt::BasicBlock;
using opt::IRContext;

const spv_target_env kEnv = SPV_ENV_UNIVERSAL_1_3;
const MessageConsumer kMessageConsumer = CLIMessageConsumer;

// This changes its mind each time IsInteresting is invoked as to whether the
// binary is interesting, until some limit is reached after which the binary is
// always deemed interesting.  This is useful to test that reduction passes
// interleave in interesting ways for a while, and then always succeed after
// some point; the latter is important to end up with a predictable final
// reduced binary for tests.
class PingPongInteresting {
 public:
  explicit PingPongInteresting(uint32_t always_interesting_after)
      : is_interesting_(true),
        always_interesting_after_(always_interesting_after),
        count_(0) {}

  bool IsInteresting(const std::vector<uint32_t>&) {
    bool result;
    if (count_ > always_interesting_after_) {
      result = true;
    } else {
      result = is_interesting_;
      is_interesting_ = !is_interesting_;
    }
    count_++;
    return result;
  }

 private:
  bool is_interesting_;
  const uint32_t always_interesting_after_;
  uint32_t count_;
};

TEST(ReducerTest, ExprToConstantAndRemoveUnreferenced) {
  // Check that ExprToConstant and RemoveUnreferenced work together; once some
  // ID uses have been changed to constants, those IDs can be removed.
  std::string original = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %60
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %16 "buf2"
               OpMemberName %16 0 "i"
               OpName %18 ""
               OpName %25 "buf1"
               OpMemberName %25 0 "f"
               OpName %27 ""
               OpName %60 "_GLF_color"
               OpMemberDecorate %16 0 Offset 0
               OpDecorate %16 Block
               OpDecorate %18 DescriptorSet 0
               OpDecorate %18 Binding 2
               OpMemberDecorate %25 0 Offset 0
               OpDecorate %25 Block
               OpDecorate %27 DescriptorSet 0
               OpDecorate %27 Binding 1
               OpDecorate %60 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpConstant %6 0
         %16 = OpTypeStruct %6
         %17 = OpTypePointer Uniform %16
         %18 = OpVariable %17 Uniform
         %19 = OpTypePointer Uniform %6
         %22 = OpTypeBool
        %100 = OpConstantTrue %22
         %24 = OpTypeFloat 32
         %25 = OpTypeStruct %24
         %26 = OpTypePointer Uniform %25
         %27 = OpVariable %26 Uniform
         %28 = OpTypePointer Uniform %24
         %31 = OpConstant %24 2
         %56 = OpConstant %6 1
         %58 = OpTypeVector %24 4
         %59 = OpTypePointer Output %58
         %60 = OpVariable %59 Output
         %72 = OpUndef %24
         %74 = OpUndef %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
         %73 = OpPhi %6 %74 %5 %77 %34
         %71 = OpPhi %24 %72 %5 %76 %34
         %70 = OpPhi %6 %9 %5 %57 %34
         %20 = OpAccessChain %19 %18 %9
         %21 = OpLoad %6 %20
         %23 = OpSLessThan %22 %70 %21
               OpLoopMerge %12 %34 None
               OpBranchConditional %23 %11 %12
         %11 = OpLabel
         %29 = OpAccessChain %28 %27 %9
         %30 = OpLoad %24 %29
         %32 = OpFOrdGreaterThan %22 %30 %31
               OpSelectionMerge %90 None
               OpBranchConditional %32 %33 %46
         %33 = OpLabel
         %40 = OpFAdd %24 %71 %30
         %45 = OpISub %6 %73 %21
               OpBranch %90
         %46 = OpLabel
         %50 = OpFMul %24 %71 %30
         %54 = OpSDiv %6 %73 %21
               OpBranch %90
         %90 = OpLabel
         %77 = OpPhi %6 %45 %33 %54 %46
         %76 = OpPhi %24 %40 %33 %50 %46
               OpBranch %34
         %34 = OpLabel
         %57 = OpIAdd %6 %70 %56
               OpBranch %10
         %12 = OpLabel
         %61 = OpAccessChain %28 %27 %9
         %62 = OpLoad %24 %61
         %66 = OpConvertSToF %24 %21
         %68 = OpConvertSToF %24 %73
         %69 = OpCompositeConstruct %58 %62 %71 %66 %68
               OpStore %60 %69
               OpReturn
               OpFunctionEnd
  )";

  std::string expected = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %60
               OpExecutionMode %4 OriginUpperLeft
               OpMemberDecorate %16 0 Offset 0
               OpDecorate %16 Block
               OpDecorate %18 DescriptorSet 0
               OpDecorate %18 Binding 2
               OpMemberDecorate %25 0 Offset 0
               OpDecorate %25 Block
               OpDecorate %27 DescriptorSet 0
               OpDecorate %27 Binding 1
               OpDecorate %60 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
          %9 = OpConstant %6 0
         %16 = OpTypeStruct %6
         %17 = OpTypePointer Uniform %16
         %18 = OpVariable %17 Uniform
         %22 = OpTypeBool
        %100 = OpConstantTrue %22
         %24 = OpTypeFloat 32
         %25 = OpTypeStruct %24
         %26 = OpTypePointer Uniform %25
         %27 = OpVariable %26 Uniform
         %31 = OpConstant %24 2
         %56 = OpConstant %6 1
         %58 = OpTypeVector %24 4
         %59 = OpTypePointer Output %58
         %60 = OpVariable %59 Output
         %72 = OpUndef %24
         %74 = OpUndef %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %10
         %10 = OpLabel
               OpLoopMerge %12 %34 None
               OpBranchConditional %100 %11 %12
         %11 = OpLabel
               OpSelectionMerge %90 None
               OpBranchConditional %100 %33 %46
         %33 = OpLabel
               OpBranch %90
         %46 = OpLabel
               OpBranch %90
         %90 = OpLabel
               OpBranch %34
         %34 = OpLabel
               OpBranch %10
         %12 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

  Reducer reducer(kEnv);
  PingPongInteresting ping_pong_interesting(10);
  reducer.SetMessageConsumer(NopDiagnostic);
  reducer.SetInterestingnessFunction(
      [&](const std::vector<uint32_t>& binary, uint32_t) -> bool {
        return ping_pong_interesting.IsInteresting(binary);
      });
  reducer.AddReductionPass(
      MakeUnique<RemoveUnreferencedInstructionReductionOpportunityFinder>(
          false));
  reducer.AddReductionPass(
      MakeUnique<OperandToConstReductionOpportunityFinder>());

  std::vector<uint32_t> binary_in;
  SpirvTools t(kEnv);

  ASSERT_TRUE(t.Assemble(original, &binary_in, kReduceAssembleOption));
  std::vector<uint32_t> binary_out;
  spvtools::ReducerOptions reducer_options;
  reducer_options.set_step_limit(500);
  reducer_options.set_fail_on_validation_error(true);
  spvtools::ValidatorOptions validator_options;

  Reducer::ReductionResultStatus status = reducer.Run(
      std::move(binary_in), &binary_out, reducer_options, validator_options);

  ASSERT_EQ(status, Reducer::ReductionResultStatus::kComplete);

  CheckEqual(kEnv, expected, binary_out);
}

bool InterestingWhileOpcodeExists(const std::vector<uint32_t>& binary,
                                  uint32_t opcode, uint32_t count, bool dump) {
  if (dump) {
    std::stringstream ss;
    ss << "temp_" << count << ".spv";
    DumpShader(binary, ss.str().c_str());
  }

  std::unique_ptr<IRContext> context =
      BuildModule(kEnv, kMessageConsumer, binary.data(), binary.size());
  assert(context);
  bool interesting = false;
  for (auto& function : *context->module()) {
    context->cfg()->ForEachBlockInPostOrder(
        &*function.begin(), [opcode, &interesting](BasicBlock* block) -> void {
          for (auto& inst : *block) {
            if (inst.opcode() == opcode) {
              interesting = true;
              break;
            }
          }
        });
    if (interesting) {
      break;
    }
  }
  return interesting;
}

bool InterestingWhileIMulReachable(const std::vector<uint32_t>& binary,
                                   uint32_t count) {
  return InterestingWhileOpcodeExists(binary, SpvOpIMul, count, false);
}

bool InterestingWhileSDivReachable(const std::vector<uint32_t>& binary,
                                   uint32_t count) {
  return InterestingWhileOpcodeExists(binary, SpvOpSDiv, count, false);
}

// The shader below was derived from the following GLSL, and optimized.
// #version 310 es
// precision highp float;
// layout(location = 0) out vec4 _GLF_color;
// int foo() {
//    int x = 1;
//    int y;
//    x = y / x;   // SDiv
//    return x;
// }
// void main() {
//    int c;
//    while (bool(c)) {
//        do {
//            if (bool(c)) {
//                if (bool(c)) {
//                    ++c;
//                } else {
//                    _GLF_color.x = float(c*c);  // IMul
//                }
//                return;
//            }
//        } while(bool(foo()));
//        return;
//    }
// }
const std::string kShaderWithLoopsDivAndMul = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %49
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %49 "_GLF_color"
               OpDecorate %49 Location 0
               OpDecorate %52 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeInt 32 1
         %12 = OpConstant %6 1
         %27 = OpTypeBool
         %28 = OpTypeInt 32 0
         %29 = OpConstant %28 0
         %46 = OpTypeFloat 32
         %47 = OpTypeVector %46 4
         %48 = OpTypePointer Output %47
         %49 = OpVariable %48 Output
         %54 = OpTypePointer Output %46
         %64 = OpConstantFalse %27
         %67 = OpConstantTrue %27
         %81 = OpUndef %6
          %4 = OpFunction %2 None %3
          %5 = OpLabel
               OpBranch %61
         %61 = OpLabel
               OpLoopMerge %60 %63 None
               OpBranch %20
         %20 = OpLabel
         %30 = OpINotEqual %27 %81 %29
               OpLoopMerge %22 %23 None
               OpBranchConditional %30 %21 %22
         %21 = OpLabel
               OpBranch %31
         %31 = OpLabel
               OpLoopMerge %33 %38 None
               OpBranch %32
         %32 = OpLabel
               OpBranchConditional %30 %37 %38
         %37 = OpLabel
               OpSelectionMerge %42 None
               OpBranchConditional %30 %41 %45
         %41 = OpLabel
               OpBranch %42
         %45 = OpLabel
         %52 = OpIMul %6 %81 %81
         %53 = OpConvertSToF %46 %52
         %55 = OpAccessChain %54 %49 %29
               OpStore %55 %53
               OpBranch %42
         %42 = OpLabel
               OpBranch %33
         %38 = OpLabel
         %77 = OpSDiv %6 %81 %12
         %58 = OpINotEqual %27 %77 %29
               OpBranchConditional %58 %31 %33
         %33 = OpLabel
         %86 = OpPhi %27 %67 %42 %64 %38
               OpSelectionMerge %68 None
               OpBranchConditional %86 %22 %68
         %68 = OpLabel
               OpBranch %22
         %23 = OpLabel
               OpBranch %20
         %22 = OpLabel
         %90 = OpPhi %27 %64 %20 %86 %33 %67 %68
               OpSelectionMerge %70 None
               OpBranchConditional %90 %60 %70
         %70 = OpLabel
               OpBranch %60
         %63 = OpLabel
               OpBranch %61
         %60 = OpLabel
               OpReturn
               OpFunctionEnd
  )";

TEST(ReducerTest, ShaderReduceWhileMulReachable) {
  Reducer reducer(kEnv);

  reducer.SetInterestingnessFunction(InterestingWhileIMulReachable);
  reducer.AddDefaultReductionPasses();
  reducer.SetMessageConsumer(kMessageConsumer);

  std::vector<uint32_t> binary_in;
  SpirvTools t(kEnv);

  ASSERT_TRUE(
      t.Assemble(kShaderWithLoopsDivAndMul, &binary_in, kReduceAssembleOption));
  std::vector<uint32_t> binary_out;
  spvtools::ReducerOptions reducer_options;
  reducer_options.set_step_limit(500);
  reducer_options.set_fail_on_validation_error(true);
  spvtools::ValidatorOptions validator_options;

  Reducer::ReductionResultStatus status = reducer.Run(
      std::move(binary_in), &binary_out, reducer_options, validator_options);

  ASSERT_EQ(status, Reducer::ReductionResultStatus::kComplete);
}

TEST(ReducerTest, ShaderReduceWhileDivReachable) {
  Reducer reducer(kEnv);

  reducer.SetInterestingnessFunction(InterestingWhileSDivReachable);
  reducer.AddDefaultReductionPasses();
  reducer.SetMessageConsumer(kMessageConsumer);

  std::vector<uint32_t> binary_in;
  SpirvTools t(kEnv);

  ASSERT_TRUE(
      t.Assemble(kShaderWithLoopsDivAndMul, &binary_in, kReduceAssembleOption));
  std::vector<uint32_t> binary_out;
  spvtools::ReducerOptions reducer_options;
  reducer_options.set_step_limit(500);
  reducer_options.set_fail_on_validation_error(true);
  spvtools::ValidatorOptions validator_options;

  Reducer::ReductionResultStatus status = reducer.Run(
      std::move(binary_in), &binary_out, reducer_options, validator_options);

  ASSERT_EQ(status, Reducer::ReductionResultStatus::kComplete);
}

}  // namespace
}  // namespace reduce
}  // namespace spvtools
