///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// VerifierTest.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Test/CompilationResult.h"
#include "dxc/Test/HLSLTestData.h"
#include <memory>
#include <string>
#include <vector>

#include <fstream>

#ifdef _WIN32
#define TEST_CLASS_DERIVATION
#else
#define TEST_CLASS_DERIVATION : public ::testing::Test
#endif
#include "dxc/Test/HlslTestUtils.h"

using namespace std;

// The test fixture.
class VerifierTest TEST_CLASS_DERIVATION {
public:
  BEGIN_TEST_CLASS(VerifierTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_METHOD(RunCppErrors)
  TEST_METHOD(RunCppErrorsHV2015)

  void CheckVerifies(const wchar_t *path) {
    WEX::TestExecution::SetVerifyOutput verifySettings(
        WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
    const char startMarker[] = "%clang_cc1";
    const char endMarker[] = "%s";

    // I bumped this up to 1024, if we have a run line that large in our tests
    // bad will happen, and we probably deserve the pain...
    char firstLine[1024];
    memset(firstLine, 0, sizeof(firstLine));

    char *commandLine;

    //
    // Very simple processing for now.
    // See utils\lit\lit\TestRunner.py for the more thorough implementation.
    //
    // The first line for HLSL tests will always look like this:
    // // RUN: %clang_cc1 -fsyntax-only -Wno-unused-value -ffreestanding -verify
    // %s
    //
    // We turn this into ' -fsyntax-only -Wno-unused-value -ffreestanding
    // -verify ' by offseting after %clang_cc1 and chopping off everything after
    // '%s'.
    //

    ifstream infile(CW2A(path).m_psz);
    ASSERT_EQ(false, infile.bad());

    bool FoundRun = false;

    // This loop is super hacky, and not at all how we should do this. We should
    // have _one_ implementation of reading and interpreting RUN but instead we
    // currently have many, and this one only supports one RUN directive, which
    // is confusing and has resulted in test cases that aren't being run at all,
    // and are hiding bugs.

    // The goal of this loop is to process RUN lines at the top of the file,
    // until the first non-run line. This also isn't ideal, but is better.
    while (!infile.eof()) {
      infile.getline(firstLine, _countof(firstLine));
      char *found = strstr(firstLine, startMarker);
      if (found)
        FoundRun = true;
      else
        break;

      commandLine = found + strlen(startMarker);

      char *fileArgument = strstr(commandLine, endMarker);
      ASSERT_NE(nullptr, fileArgument);
      *fileArgument = '\0';

      CW2A asciiPath(path);
      CompilationResult result =
          CompilationResult::CreateForCommandLine(commandLine, asciiPath);
      if (!result.ParseSucceeded()) {
        std::stringstream ss;
        ss << "for program " << asciiPath << " with errors:\n"
           << result.GetTextForErrors();
        CA2W pszW(ss.str().c_str());
        ::WEX::Logging::Log::Comment(pszW);
      }
      VERIFY_IS_TRUE(result.ParseSucceeded());
    }
    ASSERT_EQ(true, FoundRun);
  }

  void CheckVerifiesHLSL(LPCWSTR name) {
    // Having a test per file makes it very easy to filter from the command
    // line.
    CheckVerifies(hlsl_test::GetPathToHlslDataFile(name).c_str());
  }
};

TEST_F(VerifierTest, RunCppErrors) { CheckVerifiesHLSL(L"cpp-errors.hlsl"); }

TEST_F(VerifierTest, RunCppErrorsHV2015) {
  CheckVerifiesHLSL(L"cpp-errors-hv2015.hlsl");
}
