//===- unittests/HLSL/HLSLTestOptions.h ----- Command Line Options ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the command line options that can be passed to HLSL
// gtests. This file should be included in any test file that intends to use any
// options.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_UNITTESTS_HLSL_TEST_OPTIONS_H
#define LLVM_CLANG_UNITTESTS_HLSL_TEST_OPTIONS_H

#include <string>

namespace clang {
namespace hlsl {

/// \brief Includes any command line options that may be passed to gtest for
/// running the SPIR-V tests. New options should be added in this namespace.
namespace testOptions {

/// \brief Command line option that specifies the path to the directory that
/// contains files that have the HLSL source code (used for the CodeGen test
/// flow).
#define ARG_DECLARE(argname) extern std::string argname;

#define ARG_LIST(ARGOP)                                                        \
  ARGOP(HlslDataDir)                                                           \
  ARGOP(TestName)                                                              \
  ARGOP(DXBC)                                                                  \
  ARGOP(SaveImages)                                                            \
  ARGOP(ExperimentalShaders)                                                   \
  ARGOP(DebugLayer)                                                            \
  ARGOP(SuitePath)                                                             \
  ARGOP(InputPath)

ARG_LIST(ARG_DECLARE)

} // namespace testOptions
} // namespace hlsl
} // namespace clang

#endif
