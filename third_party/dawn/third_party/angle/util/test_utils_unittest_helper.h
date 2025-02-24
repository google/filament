//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// system_utils_unittest_helper.h: Constants used by the SystemUtils.RunApp unittest

#ifndef COMMON_SYSTEM_UTILS_UNITTEST_HELPER_H_
#define COMMON_SYSTEM_UTILS_UNITTEST_HELPER_H_

namespace
{
constexpr char kRunAppTestEnvVarName[]  = "RUN_APP_TEST_ENV";
constexpr char kRunAppTestEnvVarValue[] = "RunAppTest environment variable value\n";
constexpr char kRunAppTestStdout[]      = "RunAppTest stdout test\n";
constexpr char kRunAppTestStderr[] = "RunAppTest stderr test\n  .. that expands multiple lines\n";
constexpr char kRunAppTestArg1[]   = "--expected-arg1";
constexpr char kRunAppTestArg2[]   = "expected_arg2";
constexpr char kRunTestSuite[]     = "--run-test-suite";
}  // anonymous namespace

#endif  // COMMON_SYSTEM_UTILS_UNITTEST_HELPER_H_
