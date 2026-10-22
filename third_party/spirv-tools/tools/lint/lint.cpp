// Copyright (c) 2021 Google LLC.
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

#include <iostream>

#include "source/opt/log.h"
#include "spirv-tools/linter.hpp"
#include "tools/io.h"
#include "tools/util/cli_consumer.h"
#include "tools/util/flags.h"

namespace {

constexpr auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_6;
constexpr auto kHelpTextFmt =
    R"(%s - Lint a SPIR-V binary module.

Usage: %s [options] <filename>

Options:

  -h, --help      Print this help.
  --version       Display assembler version information.
)";

}  // namespace

// clang-format off
FLAG_SHORT_bool(  h,       /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool(   help,    /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool(   version, /* default_value= */ false, /* required= */ false);
// clang-format on

int main(int, const char** argv) {
  if (!flags::Parse(argv)) {
    return 1;
  }

  if (flags::h.value() || flags::help.value()) {
    printf(kHelpTextFmt, argv[0], argv[0]);
    return 0;
  }

  if (flags::version.value()) {
    printf("%s\n", spvSoftwareVersionDetailsString());
    return 0;
  }

  if (flags::positional_arguments.size() != 1) {
    spvtools::Error(spvtools::utils::CLIMessageConsumer, nullptr, {},
                    "expected exactly one input file.");
    return 1;
  }

  spvtools::Linter linter(kDefaultEnvironment);
  linter.SetMessageConsumer(spvtools::utils::CLIMessageConsumer);
  std::vector<uint32_t> binary;
  if (!ReadBinaryFile(flags::positional_arguments[0].c_str(), &binary)) {
    return 1;
  }

  return linter.Run(binary.data(), binary.size()) ? 0 : 1;
}
