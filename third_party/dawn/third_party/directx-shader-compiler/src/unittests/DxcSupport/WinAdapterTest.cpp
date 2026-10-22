//===- unittests/Basic/WinAdapterTest.cpp -- Windows Adapter tests --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _WIN32

#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/WinIncludes.h"
#include "gtest/gtest.h"

#include <string.h>

namespace {

// Check that WArgV.
TEST(WArgVTest, suppressAndTrap) {
  int argc = 2;
  std::wstring data[] = {L"a", L"b"};
  const wchar_t *ref_argv[] = {data[0].c_str(), data[1].c_str()};
  {
    const char *argv[] = {"a", "b"};
    WArgV ArgV(argc, argv);
    const wchar_t **wargv = ArgV.argv();
    for (int i = 0; i < argc; ++i) {
      EXPECT_EQ(0, std::wcscmp(ref_argv[i], wargv[i]));
    }
  }
}

} // namespace
#endif
