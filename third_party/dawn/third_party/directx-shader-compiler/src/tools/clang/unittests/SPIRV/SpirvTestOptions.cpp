//===- unittests/SPIRV/SpirvTestOptions.cpp ----- Test Options Init -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines and initializes command line options that can be passed to
// SPIR-V gtests.
//
//===----------------------------------------------------------------------===//

#include "SpirvTestOptions.h"

namespace clang {
namespace spirv {
namespace testOptions {

std::string inputDataDir = DEFAULT_TEST_DIR;

} // namespace testOptions
} // namespace spirv
} // namespace clang
