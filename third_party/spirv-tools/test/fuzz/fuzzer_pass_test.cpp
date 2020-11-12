// Copyright (c) 2020 Vasyl Teliman
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

#include "gtest/gtest.h"
#include "source/fuzz/fuzzer_pass_add_opphi_synonyms.h"
#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/pseudo_random_generator.h"
#include "test/fuzz/fuzz_test_util.h"

namespace spvtools {
namespace fuzz {
namespace {

class FuzzerPassMock : public FuzzerPass {
 public:
  FuzzerPassMock(opt::IRContext* ir_context,
                 TransformationContext* transformation_context,
                 FuzzerContext* fuzzer_context,
                 protobufs::TransformationSequence* transformations)
      : FuzzerPass(ir_context, transformation_context, fuzzer_context,
                   transformations) {}

  ~FuzzerPassMock() override = default;

  const std::unordered_set<uint32_t>& GetReachedInstructions() const {
    return reached_ids_;
  }

  void Apply() override {
    ForEachInstructionWithInstructionDescriptor(
        [this](opt::Function* /*unused*/, opt::BasicBlock* /*unused*/,
               opt::BasicBlock::iterator inst_it,
               const protobufs::InstructionDescriptor& /*unused*/) {
          if (inst_it->result_id()) {
            reached_ids_.insert(inst_it->result_id());
          }
        });
  }

 private:
  std::unordered_set<uint32_t> reached_ids_;
};

TEST(FuzzerPassTest, ForEachInstructionWithInstructionDescriptor) {
  std::string shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main"
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %7 = OpUndef %6
               OpReturn
          %8 = OpLabel
          %9 = OpUndef %6
               OpReturn
               OpFunctionEnd
  )";

  const auto env = SPV_ENV_UNIVERSAL_1_3;
  const auto consumer = nullptr;
  const auto context = BuildModule(env, consumer, shader, kFuzzAssembleOption);
  spvtools::ValidatorOptions validator_options;
  ASSERT_TRUE(fuzzerutil::IsValidAndWellFormed(context.get(), validator_options,
                                               kConsoleMessageConsumer));
  TransformationContext transformation_context(
      MakeUnique<FactManager>(context.get()), validator_options);
  // Check that %5 is reachable and %8 is unreachable as expected.
  const auto* dominator_analysis =
      context->GetDominatorAnalysis(context->GetFunction(4));
  ASSERT_TRUE(dominator_analysis->IsReachable(5));
  ASSERT_FALSE(dominator_analysis->IsReachable(8));

  PseudoRandomGenerator prng(0);
  FuzzerContext fuzzer_context(&prng, 100);
  protobufs::TransformationSequence transformations;
  FuzzerPassMock fuzzer_pass_mock(context.get(), &transformation_context,
                                  &fuzzer_context, &transformations);
  fuzzer_pass_mock.Apply();

  ASSERT_TRUE(fuzzer_pass_mock.GetReachedInstructions().count(7));
  ASSERT_FALSE(fuzzer_pass_mock.GetReachedInstructions().count(9));
}

}  // namespace
}  // namespace fuzz
}  // namespace spvtools
