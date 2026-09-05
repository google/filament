//===--- utils/unittest/HLSL/TestMain.cpp - unittest driver --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "llvm/Support/Signals.h"

#include "HLSLTestOptions.h"
#include "dxc/Test/WEXAdapter.h"

#if defined(_WIN32)
#include <windows.h>
#if defined(_MSC_VER)
#include <crtdbg.h>
#endif
#endif

namespace {
using namespace ::testing;

/// A GoogleTest event printer that only prints test failures.
class FailurePrinter : public TestEventListener {
public:
  explicit FailurePrinter(TestEventListener *listener)
      : defaultListener(listener) {}

  ~FailurePrinter() override { delete defaultListener; }

  void OnTestProgramStart(const UnitTest &ut) override {
    defaultListener->OnTestProgramStart(ut);
  }

  void OnTestIterationStart(const UnitTest &ut, int iteration) override {
    defaultListener->OnTestIterationStart(ut, iteration);
  }

  void OnEnvironmentsSetUpStart(const UnitTest &ut) override {
    defaultListener->OnEnvironmentsSetUpStart(ut);
  }

  void OnEnvironmentsSetUpEnd(const UnitTest &ut) override {
    defaultListener->OnEnvironmentsSetUpEnd(ut);
  }

  void OnTestCaseStart(const TestCase &tc) override {
    defaultListener->OnTestCaseStart(tc);
  }

  void OnTestStart(const TestInfo &ti) override {
    // Do not output on test start
    // defaultListener->OnTestStart(ti);
  }

  void OnTestPartResult(const TestPartResult &result) override {
    defaultListener->OnTestPartResult(result);
  }

  void OnTestEnd(const TestInfo &ti) override {
    // Only output if failure on test end
    if (ti.result()->Failed())
      defaultListener->OnTestEnd(ti);
  }

  void OnTestCaseEnd(const TestCase &tc) override {
    defaultListener->OnTestCaseEnd(tc);
  }

  void OnEnvironmentsTearDownStart(const UnitTest &ut) override {
    defaultListener->OnEnvironmentsTearDownStart(ut);
  }

  void OnEnvironmentsTearDownEnd(const UnitTest &ut) override {
    defaultListener->OnEnvironmentsTearDownEnd(ut);
  }

  void OnTestIterationEnd(const UnitTest &ut, int iteration) override {
    defaultListener->OnTestIterationEnd(ut, iteration);
  }

  void OnTestProgramEnd(const UnitTest &ut) override {
    defaultListener->OnTestProgramEnd(ut);
  }

private:
  TestEventListener *defaultListener;
};
} // namespace

const char *TestMainArgv0;

#define SAVE_ARG(argname)                                                      \
  if (std::string("--" #argname) == argv[i]) {                                 \
    if (i + 1 < argc) {                                                        \
      clang::hlsl::testOptions::argname = argv[++i];                           \
    } else {                                                                   \
      fprintf(stderr, "Error: --" #argname " requires an argument\n");         \
      return 1;                                                                \
    }                                                                          \
  }

int main(int argc, char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal(true /* Disable crash reporting */);

  for (int i = 1; i < argc; ++i) {
    ARG_LIST(SAVE_ARG)
  }

  // Initialize both gmock and gtest.
  testing::InitGoogleMock(&argc, argv);

  // Switch event listener to one that only prints failures.
  testing::TestEventListeners &listeners =
      ::testing::UnitTest::GetInstance()->listeners();
  auto *defaultPrinter = listeners.Release(listeners.default_result_printer());
  // Google Test takes the ownership.
  listeners.Append(new FailurePrinter(defaultPrinter));

  // Make it easy for a test to re-execute itself by saving argv[0].
  TestMainArgv0 = argv[0];

#if defined(_WIN32)
  // Disable all of the possible ways Windows conspires to make automated
  // testing impossible.
  ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#if defined(_MSC_VER)
  ::_set_error_mode(_OUT_TO_STDERR);
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
  _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
  _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
#endif

  moduleSetup();
  int rv = RUN_ALL_TESTS();
  moduleTeardown();
  return rv;
}
