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

#include "spirv-tools/linker.hpp"

#include <cstring>
#include <iostream>
#include <vector>

#include "source/spirv_target_env.h"
#include "source/table.h"
#include "spirv-tools/libspirv.hpp"
#include "tools/io.h"

namespace {

const auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_5;

void print_usage(const char* program) {
  std::string target_env_list = spvTargetEnvList(16, 80);
  // NOTE: Please maintain flags in lexicographical order.
  printf(
      R"(%s - Link SPIR-V binary files together.

USAGE: %s [options] [-o <output>] <input>...

The SPIR-V binaries are read from the different <input>(s).
The SPIR-V resulting linked binary module is written to the file "out.spv"
unless the -o option is used; if <output> is "-", it is written to the standard
output.

NOTE: The linker is a work in progress.

Options (in lexicographical order):
  --allow-partial-linkage
               Allow partial linkage by accepting imported symbols to be
               unresolved.
  --create-library
               Link the binaries into a library, keeping all exported symbols.
  -h, --help
                  Print this help.
  --target-env <env>
               Set the target environment. Without this flag the target
               environment defaults to spv1.5. <env> must be one of
               {%s}
  --verify-ids
               Verify that IDs in the resulting modules are truly unique.
  --version
               Display linker version information
)",
      program, program, target_env_list.c_str());
}

}  // namespace

int main(int argc, char** argv) {
  std::vector<const char*> inFiles;
  const char* outFile = nullptr;
  spv_target_env target_env = kDefaultEnvironment;
  spvtools::LinkerOptions options;

  for (int argi = 1; argi < argc; ++argi) {
    const char* cur_arg = argv[argi];
    if ('-' == cur_arg[0]) {
      if (0 == strcmp(cur_arg, "-o")) {
        if (argi + 1 < argc) {
          if (!outFile) {
            outFile = argv[++argi];
          } else {
            fprintf(stderr, "error: More than one output file specified\n");
            return 1;
          }
        } else {
          fprintf(stderr, "error: Missing argument to %s\n", cur_arg);
          return 1;
        }
      } else if (0 == strcmp(cur_arg, "--allow-partial-linkage")) {
        options.SetAllowPartialLinkage(true);
      } else if (0 == strcmp(cur_arg, "--create-library")) {
        options.SetCreateLibrary(true);
      } else if (0 == strcmp(cur_arg, "--help") || 0 == strcmp(cur_arg, "-h")) {
        print_usage(argv[0]);
        return 0;
      } else if (0 == strcmp(cur_arg, "--target-env")) {
        if (argi + 1 < argc) {
          const auto env_str = argv[++argi];
          if (!spvParseTargetEnv(env_str, &target_env)) {
            fprintf(stderr, "error: Unrecognized target env: %s\n", env_str);
            return 1;
          }
        } else {
          fprintf(stderr, "error: Missing argument to --target-env\n");
          return 1;
        }
      } else if (0 == strcmp(cur_arg, "--verify-ids")) {
        options.SetVerifyIds(true);
      } else if (0 == strcmp(cur_arg, "--version")) {
        printf("%s\n", spvSoftwareVersionDetailsString());
        printf("Target: %s\n", spvTargetEnvDescription(target_env));
        return 0;
      } else {
        fprintf(stderr, "error: Unrecognized option: %s\n\n", argv[argi]);
        print_usage(argv[0]);
        return 1;
      }
    } else {
      inFiles.push_back(cur_arg);
    }
  }

  if (!outFile) {
    outFile = "out.spv";
  }

  if (inFiles.empty()) {
    fprintf(stderr, "error: No input file specified\n");
    return 1;
  }

  std::vector<std::vector<uint32_t>> contents(inFiles.size());
  for (size_t i = 0u; i < inFiles.size(); ++i) {
    if (!ReadBinaryFile<uint32_t>(inFiles[i], &contents[i])) return 1;
  }

  const spvtools::MessageConsumer consumer = [](spv_message_level_t level,
                                                const char*,
                                                const spv_position_t& position,
                                                const char* message) {
    switch (level) {
      case SPV_MSG_FATAL:
      case SPV_MSG_INTERNAL_ERROR:
      case SPV_MSG_ERROR:
        std::cerr << "error: " << position.index << ": " << message
                  << std::endl;
        break;
      case SPV_MSG_WARNING:
        std::cout << "warning: " << position.index << ": " << message
                  << std::endl;
        break;
      case SPV_MSG_INFO:
        std::cout << "info: " << position.index << ": " << message << std::endl;
        break;
      default:
        break;
    }
  };
  spvtools::Context context(target_env);
  context.SetMessageConsumer(consumer);

  std::vector<uint32_t> linkingResult;
  spv_result_t status = Link(context, contents, &linkingResult, options);

  if (!WriteFile<uint32_t>(outFile, "wb", linkingResult.data(),
                           linkingResult.size()))
    return 1;

  return status == SPV_SUCCESS ? 0 : 1;
}
