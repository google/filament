// Copyright (c) 2017 Pierre Moreau
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

#ifndef TEST_LINK_LINKER_FIXTURE_H_
#define TEST_LINK_LINKER_FIXTURE_H_

#include <iostream>
#include <string>
#include <vector>

#include "effcee/effcee.h"
#include "re2/re2.h"
#include "source/spirv_constant.h"
#include "spirv-tools/linker.hpp"
#include "test/unit_spirv.h"

namespace spvtest {

using Binary = std::vector<uint32_t>;
using Binaries = std::vector<Binary>;

class LinkerTest : public ::testing::Test {
 public:
  LinkerTest()
      : context_(SPV_ENV_UNIVERSAL_1_2),
        tools_(SPV_ENV_UNIVERSAL_1_2),
        assemble_options_(spvtools::SpirvTools::kDefaultAssembleOption),
        disassemble_options_(spvtools::SpirvTools::kDefaultDisassembleOption) {
    const auto consumer = [this](spv_message_level_t level, const char*,
                                 const spv_position_t& position,
                                 const char* message) {
      if (!error_message_.empty()) error_message_ += "\n";
      switch (level) {
        case SPV_MSG_FATAL:
        case SPV_MSG_INTERNAL_ERROR:
        case SPV_MSG_ERROR:
          error_message_ += "ERROR";
          break;
        case SPV_MSG_WARNING:
          error_message_ += "WARNING";
          break;
        case SPV_MSG_INFO:
          error_message_ += "INFO";
          break;
        case SPV_MSG_DEBUG:
          error_message_ += "DEBUG";
          break;
      }
      error_message_ += ": " + std::to_string(position.index) + ": " + message;
    };
    context_.SetMessageConsumer(consumer);
    tools_.SetMessageConsumer(consumer);
  }

  void TearDown() override { error_message_.clear(); }

  // Assembles each of the given strings into SPIR-V binaries before linking
  // them together. SPV_ERROR_INVALID_TEXT is returned if the assembling failed
  // for any of the input strings, and SPV_ERROR_INVALID_POINTER if
  // |linked_binary| is a null pointer.
  spv_result_t AssembleAndLink(
      const std::vector<std::string>& bodies, spvtest::Binary* linked_binary,
      spvtools::LinkerOptions options = spvtools::LinkerOptions()) {
    if (!linked_binary) return SPV_ERROR_INVALID_POINTER;

    spvtest::Binaries binaries(bodies.size());
    for (size_t i = 0u; i < bodies.size(); ++i)
      if (!tools_.Assemble(bodies[i], binaries.data() + i, assemble_options_))
        return SPV_ERROR_INVALID_TEXT;

    return spvtools::Link(context_, binaries, linked_binary, options);
  }

  // Assembles and links a vector of SPIR-V bodies based on the |templateBody|.
  // Template arguments to be replaced are written as {a,b,...}.
  // SPV_ERROR_INVALID_TEXT is returned if the assembling failed for any of the
  // resulting bodies (or errors in the template), and SPV_ERROR_INVALID_POINTER
  // if |linked_binary| is a null pointer.
  spv_result_t ExpandAndLink(
      const std::string& templateBody, spvtest::Binary* linked_binary,
      spvtools::LinkerOptions options = spvtools::LinkerOptions()) {
    if (!linked_binary) return SPV_ERROR_INVALID_POINTER;

    // Find out how many template arguments there are, we assume they all have
    // the same number. We'll error later if they don't.
    re2::StringPiece temp(templateBody);
    re2::StringPiece x;
    int cnt = 0;
    if (!RE2::FindAndConsume(&temp, "{")) return SPV_ERROR_INVALID_TEXT;
    while (RE2::FindAndConsume(&temp, "([,}])", &x) && x[0] == ',') cnt++;
    cnt++;
    if (cnt <= 1) return SPV_ERROR_INVALID_TEXT;

    // Construct a regex for a single common strip and template expansion.
    std::string regex("([^{]*){");
    for (int i = 0; i < cnt; i++) regex += (i > 0) ? ",([^,]*)" : "([^,]*)";
    regex += "}";
    RE2 pattern(regex);

    // Prepare the RE2::Args for processing.
    re2::StringPiece common;
    std::vector<re2::StringPiece> variants(cnt);
    std::vector<RE2::Arg> args(cnt + 1);
    args[0] = RE2::Arg(&common);
    std::vector<RE2::Arg*> pargs(cnt + 1);
    pargs[0] = &args[0];
    for (int i = 0; i < cnt; i++) {
      args[i + 1] = RE2::Arg(&variants[i]);
      pargs[i + 1] = &args[i + 1];
    }

    // Reset and construct the bodies bit by bit.
    std::vector<std::string> bodies(cnt);
    re2::StringPiece temp2(templateBody);
    while (RE2::ConsumeN(&temp2, pattern, pargs.data(), cnt + 1)) {
      for (int i = 0; i < cnt; i++) {
        bodies[i].append(common.begin(), common.end());
        bodies[i].append(variants[i].begin(), variants[i].end());
      }
    }
    RE2::Consume(&temp2, "([^{]*)", &common);
    for (int i = 0; i < cnt; i++)
      bodies[i].append(common.begin(), common.end());

    // Run through the assemble and link stages of the process.
    return AssembleAndLink(bodies, linked_binary, options);
  }

  // Expand the |templateBody| and link the results as with ExpandAndLink,
  // then disassemble and test that the result matches the |expected|.
  void ExpandAndCheck(
      const std::string& templateBody, const std::string& expected,
      const spvtools::LinkerOptions options = spvtools::LinkerOptions()) {
    spvtest::Binary linked_binary;
    spv_result_t res = ExpandAndLink(templateBody, &linked_binary, options);
    EXPECT_EQ(SPV_SUCCESS, res) << GetErrorMessage() << "\nExpanded from:\n"
                                << templateBody;
    if (res == SPV_SUCCESS) {
      std::string result;
      EXPECT_TRUE(
          tools_.Disassemble(linked_binary, &result, disassemble_options_))
          << GetErrorMessage();
      EXPECT_EQ(expected, result);
    }
  }

  // An alternative to ExpandAndCheck, which uses the |templateBody| as the
  // match pattern for the disassembled linked result.
  void ExpandAndMatch(
      const std::string& templateBody,
      const spvtools::LinkerOptions options = spvtools::LinkerOptions()) {
    spvtest::Binary linked_binary;
    spv_result_t res = ExpandAndLink(templateBody, &linked_binary, options);
    EXPECT_EQ(SPV_SUCCESS, res) << GetErrorMessage() << "\nExpanded from:\n"
                                << templateBody;
    if (res == SPV_SUCCESS) {
      std::string result;
      EXPECT_TRUE(
          tools_.Disassemble(linked_binary, &result, disassemble_options_))
          << GetErrorMessage();
      auto match_res = effcee::Match(result, templateBody);
      EXPECT_EQ(effcee::Result::Status::Ok, match_res.status())
          << match_res.message() << "\nExpanded from:\n"
          << templateBody << "\nChecking result:\n"
          << result;
    }
  }

  // Links the given SPIR-V binaries together; SPV_ERROR_INVALID_POINTER is
  // returned if |linked_binary| is a null pointer.
  spv_result_t Link(
      const spvtest::Binaries& binaries, spvtest::Binary* linked_binary,
      spvtools::LinkerOptions options = spvtools::LinkerOptions()) {
    if (!linked_binary) return SPV_ERROR_INVALID_POINTER;
    return spvtools::Link(context_, binaries, linked_binary, options);
  }

  // Disassembles |binary| and outputs the result in |text|. If |text| is a
  // null pointer, SPV_ERROR_INVALID_POINTER is returned.
  spv_result_t Disassemble(const spvtest::Binary& binary, std::string* text) {
    if (!text) return SPV_ERROR_INVALID_POINTER;
    return tools_.Disassemble(binary, text, disassemble_options_)
               ? SPV_SUCCESS
               : SPV_ERROR_INVALID_BINARY;
  }

  // Sets the options for the assembler.
  void SetAssembleOptions(uint32_t assemble_options) {
    assemble_options_ = assemble_options;
  }

  // Sets the options used by the disassembler.
  void SetDisassembleOptions(uint32_t disassemble_options) {
    disassemble_options_ = disassemble_options;
  }

  // Returns the accumulated error messages for the test.
  std::string GetErrorMessage() const { return error_message_; }

  bool Validate(const spvtest::Binary& binary) {
    return tools_.Validate(binary);
  }

 private:
  spvtools::Context context_;
  spvtools::SpirvTools
      tools_;  // An instance for calling SPIRV-Tools functionalities.
  uint32_t assemble_options_;
  uint32_t disassemble_options_;
  std::string error_message_;
};

}  // namespace spvtest

#endif  // TEST_LINK_LINKER_FIXTURE_H_
