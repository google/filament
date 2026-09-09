//===- LibTestFixture.cpp ------------- Lib Test Fixture Implementation -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LibTestFixture.h"

#include <fstream>

#include "LibTestUtils.h"

namespace clang {
namespace spirv {

bool LibTest::parseCommand() {
  const char hlslStartLabel[] = "// RUN:";
  const auto runCmdStartPos = checkCommands.find(hlslStartLabel);
  if (runCmdStartPos != std::string::npos) {
    const auto runCmdEndPos = checkCommands.find('\n', runCmdStartPos);
    const auto runCommand = checkCommands.substr(runCmdStartPos, runCmdEndPos);
    if (!utils::processRunCommandArgs(runCommand, &targetProfile, &entryPoint,
                                      &targetEnv, &restArgs)) {
      // An error has occurred when parsing the Run command.
      return false;
    }
  } else {
    fprintf(stderr, "Error: Missing \"RUN:\" command.\n");
    return false;
  }

  // Everything was successful.
  return true;
}

std::string LibTest::compileCodeAndGetSpirvAsm(llvm::StringRef code,
                                               Expect expect,
                                               bool runValidation) {
  if (beforeHLSLLegalization)
    assert(runValidation);

  checkCommands = code;
  bool parseCommandSucceeded = parseCommand();
  EXPECT_TRUE(parseCommandSucceeded);
  if (!parseCommandSucceeded) {
    return "";
  }

  // Feed the HLSL source into the Compiler.
  std::string errorMessages;
  const bool compileOk = utils::compileCodeWithSpirvGeneration(
      code, entryPoint, targetProfile, restArgs, &generatedBinary,
      &errorMessages);

  checkTestResult(compileOk, expect, runValidation);
  return generatedSpirvAsm;
}

void LibTest::checkTestResult(const bool compileOk, Expect expect,
                              bool runValidation) {
  generatedSpirvAsm = "";
  if (expect == Expect::Failure) {
    EXPECT_FALSE(compileOk);
    return;
  }

  ASSERT_TRUE(compileOk);

  // Disassemble the generated SPIR-V binary.
  ASSERT_TRUE(utils::disassembleSpirvBinary(generatedBinary, &generatedSpirvAsm,
                                            true /* generateHeader */,
                                            targetEnv));

  if (runValidation) {
    EXPECT_TRUE(utils::validateSpirvBinary(targetEnv, generatedBinary,
                                           beforeHLSLLegalization, glLayout,
                                           dxLayout, scalarLayout));
  }
}

} // end namespace spirv
} // end namespace clang
