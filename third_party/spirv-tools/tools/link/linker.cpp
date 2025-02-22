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

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "source/spirv_target_env.h"
#include "source/table.h"
#include "spirv-tools/libspirv.hpp"
#include "tools/io.h"
#include "tools/util/flags.h"

namespace {

constexpr auto kDefaultEnvironment = "spv1.6";

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
  --allow-pointer-mismatch
               Allow pointer function parameters to mismatch the target link
               target. This is useful to workaround lost correct parameter type
               information due to LLVM's opaque pointers.
  --create-library
               Link the binaries into a library, keeping all exported symbols.
  -h, --help
               Print this help.
  --target-env <env>
               Set the environment used for interpreting the inputs. Without
               this option the environment defaults to spv1.6. <env> must be
               one of {%s}.
               NOTE: The SPIR-V version used by the linked binary module
               depends only on the version of the inputs, and is not affected
               by this option.
  --use-highest-version
               Upgrade the output SPIR-V version to the highest of the input
               files, instead of requiring all of them to have the same
               version.
               NOTE: If one of the older input files uses an instruction that
               is deprecated in the highest SPIR-V version, the output will
               be invalid.
  --verify-ids
               Verify that IDs in the resulting modules are truly unique.
  --version
               Display linker version information.
)",
      program, program, target_env_list.c_str());
}

}  // namespace

// clang-format off
FLAG_SHORT_bool(  h,                      /* default_value= */ false,               /* required= */ false);
FLAG_LONG_bool(   help,                   /* default_value= */ false,               /* required= */ false);
FLAG_LONG_bool(   version,                /* default_value= */ false,               /* required= */ false);
FLAG_LONG_bool(   verify_ids,             /* default_value= */ false,               /* required= */ false);
FLAG_LONG_bool(   create_library,         /* default_value= */ false,               /* required= */ false);
FLAG_LONG_bool(   allow_partial_linkage,  /* default_value= */ false,               /* required= */ false);
FLAG_LONG_bool(   allow_pointer_mismatch, /* default_value= */ false,               /* required= */ false);
FLAG_SHORT_string(o,                      /* default_value= */ "",                  /* required= */ false);
FLAG_LONG_string( target_env,             /* default_value= */ kDefaultEnvironment, /* required= */ false);
FLAG_LONG_bool(   use_highest_version,    /* default_value= */ false,               /* required= */ false);
// clang-format on

int main(int, const char* argv[]) {
  if (!flags::Parse(argv)) {
    return 1;
  }

  if (flags::h.value() || flags::help.value()) {
    print_usage(argv[0]);
    return 0;
  }

  if (flags::version.value()) {
    spv_target_env target_env;
    bool success = spvParseTargetEnv(kDefaultEnvironment, &target_env);
    assert(success && "Default environment should always parse.");
    if (!success) {
      fprintf(stderr,
              "error: invalid default target environment. Please report this "
              "issue.");
      return 1;
    }
    printf("%s\n", spvSoftwareVersionDetailsString());
    printf("Target: %s\n", spvTargetEnvDescription(target_env));
    return 0;
  }

  spv_target_env target_env;
  if (!spvParseTargetEnv(flags::target_env.value().c_str(), &target_env)) {
    fprintf(stderr, "error: Unrecognized target env: %s\n",
            flags::target_env.value().c_str());
    return 1;
  }

  const std::string outFile =
      flags::o.value().empty() ? "out.spv" : flags::o.value();
  const std::vector<std::string>& inFiles = flags::positional_arguments;

  spvtools::LinkerOptions options;
  options.SetAllowPartialLinkage(flags::allow_partial_linkage.value());
  options.SetAllowPtrTypeMismatch(flags::allow_pointer_mismatch.value());
  options.SetCreateLibrary(flags::create_library.value());
  options.SetVerifyIds(flags::verify_ids.value());
  options.SetUseHighestVersion(flags::use_highest_version.value());

  if (inFiles.empty()) {
    fprintf(stderr, "error: No input file specified\n");
    return 1;
  }

  std::vector<std::vector<uint32_t>> contents(inFiles.size());
  for (size_t i = 0u; i < inFiles.size(); ++i) {
    if (!ReadBinaryFile(inFiles[i].c_str(), &contents[i])) return 1;
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
  if (status != SPV_SUCCESS && status != SPV_WARNING) return 1;

  if (!WriteFile<uint32_t>(outFile.c_str(), "wb", linkingResult.data(),
                           linkingResult.size()))
    return 1;

  return 0;
}
