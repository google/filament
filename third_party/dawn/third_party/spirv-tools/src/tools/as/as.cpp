// Copyright (c) 2015-2016 The Khronos Group Inc.
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

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

#include "source/spirv_target_env.h"
#include "spirv-tools/libspirv.h"
#include "tools/io.h"
#include "tools/util/flags.h"

constexpr auto kDefaultTarget = SPV_ENV_UNIVERSAL_1_6;
static const std::string kHelpText =
    R"(%s - Create a SPIR-V binary module from SPIR-V assembly text

Usage: %s [options] [<filename>]

The SPIR-V assembly text is read from <filename>.  If no file is specified,
or if the filename is "-", then the assembly text is read from standard input.
The SPIR-V binary module is written to file "out.spv", unless the -o option
is used.

Options:

  -h, --help      Print this help.

  -o <filename>   Set the output filename. Use '-' to mean stdout.
  --version       Display assembler version information.
  --preserve-numeric-ids
                  Numeric IDs in the binary will have the same values as in the
                  source. Non-numeric IDs are allocated by filling in the gaps,
                  starting with 1 and going up.
  --target-env    %s
                  Use specified environment.
)";

// clang-format off
//                flag name=            default_value=   required=
FLAG_SHORT_bool(  h,                    false,           false);
FLAG_LONG_bool(   help,                 false,           false);
FLAG_LONG_bool(   version,              false,           false);
FLAG_LONG_bool(   preserve_numeric_ids, false,           false);
FLAG_SHORT_string(o,                    "",              false);
FLAG_LONG_string( target_env,           "",              false);
// clang-format on

int main(int, const char** argv) {
  if (!flags::Parse(argv)) {
    return 1;
  }

  if (flags::h.value() || flags::help.value()) {
    const std::string target_env_list = spvTargetEnvList(19, 80);
    printf(kHelpText.c_str(), argv[0], argv[0], target_env_list.c_str());
    return 0;
  }

  if (flags::version.value()) {
    printf("%s\n", spvSoftwareVersionDetailsString());
    printf("Target: %s\n", spvTargetEnvDescription(kDefaultTarget));
    return 0;
  }

  std::string outFile = flags::o.value();
  if (outFile.empty()) {
    outFile = "out.spv";
  }

  uint32_t options = 0;
  if (flags::preserve_numeric_ids.value()) {
    options |= SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS;
  }

  if (flags::positional_arguments.size() != 1) {
    fprintf(stderr, "error: exactly one input file must be specified.\n");
    return 1;
  }
  std::string inFile = flags::positional_arguments[0];

  std::vector<char> contents;
  if (!ReadTextFile(inFile.c_str(), &contents)) return 1;

  // Can only deduce target after the file has been read
  spv_target_env target_env;
  if (flags::target_env.value().empty()) {
    if (!spvReadEnvironmentFromText(contents, &target_env)) {
      // Revert to default version since deduction failed
      target_env = kDefaultTarget;
    }
  } else if (!spvParseTargetEnv(flags::target_env.value().c_str(),
                                &target_env)) {
    fprintf(stderr, "error: Unrecognized target env: %s\n",
            flags::target_env.value().c_str());
    return 1;
  }

  spv_binary binary;
  spv_diagnostic diagnostic = nullptr;
  spv_context context = spvContextCreate(target_env);
  spv_result_t error = spvTextToBinaryWithOptions(
      context, contents.data(), contents.size(), options, &binary, &diagnostic);
  spvContextDestroy(context);
  if (error) {
    spvDiagnosticPrint(diagnostic);
    spvDiagnosticDestroy(diagnostic);
    return error;
  }

  if (!WriteFile<uint32_t>(outFile.c_str(), "wb", binary->code,
                           binary->wordCount)) {
    spvBinaryDestroy(binary);
    return 1;
  }

  spvBinaryDestroy(binary);

  return 0;
}
