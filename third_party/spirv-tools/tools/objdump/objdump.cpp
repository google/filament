// Copyright (c) 2023 Google LLC.
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

#include <filesystem>
#include <iostream>

#include "extract_source.h"
#include "source/opt/log.h"
#include "tools/io.h"
#include "tools/util/cli_consumer.h"
#include "tools/util/flags.h"

namespace {

constexpr auto kHelpTextFmt =
    R"(%s - Dumps information from a SPIR-V binary.

Usage: %s [options] <filename>

one of the following switches must be given:
  --source        Extract source files obtained from debug symbols, output to stdout.
  --entrypoint    Extracts the entrypoint name of the module, output to stdout.
  --compiler-cmd  Extracts the command line used to compile this module, output to stdout.


General options:
  -h, --help      Print this help.
  --version       Display assembler version information.
  -f,--force      Allow output file overwrite.

Source dump options:
  --list          Do not extract source code, only print filenames to stdout.
  --outdir        Where shall the exrtacted HLSL/HLSL files be written to?
                  File written to stdout if '-' is given. Default is `-`.
)";

// Removes trailing '/' from `input`.
// A behavior difference has been observed between libc++ implementations.
// Fixing path to prevent this edge case to be reached.
// (https://github.com/llvm/llvm-project/issues/60634)
std::string fixPathForLLVM(std::string input) {
  while (!input.empty() && input.back() == '/') input.resize(input.size() - 1);
  return input;
}

// Write each HLSL file described in `sources` in a file in `outdirPath`.
// Doesn't ovewrite existing files, unless `overwrite` is set to true. The
// created HLSL file's filename is the path's filename obtained from `sources`.
// Returns true if all files could be written. False otherwise.
bool OutputSourceFiles(
    const std::unordered_map<std::string, std::string>& sources,
    const std::string& outdirPath, bool overwrite) {
  std::filesystem::path outdir(fixPathForLLVM(outdirPath));
  if (!std::filesystem::is_directory(outdir)) {
    if (!std::filesystem::create_directories(outdir)) {
      std::cerr << "error: could not create output directory " << outdir
                << std::endl;
      return false;
    }
  }

  for (const auto & [ filepath, code ] : sources) {
    if (code.empty()) {
      std::cout << "Ignoring source for " << filepath
                << ": no code source in debug infos." << std::endl;
      continue;
    }

    std::filesystem::path old_path(filepath);
    std::filesystem::path new_path = outdir / old_path.filename();

    if (!overwrite && std::filesystem::exists(new_path)) {
      std::cerr << "file " << filepath
                << " already exists, aborting (use --overwrite to allow it)."
                << std::endl;
      return false;
    }

    std::cout << "Exporting " << new_path << std::endl;
    if (!WriteFile<char>(new_path.string().c_str(), "w", code.c_str(),
                         code.size())) {
      return false;
    }
  }
  return true;
}

}  // namespace

// clang-format off
FLAG_SHORT_bool(  h,            /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool(   help,         /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool(   version,      /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool(   source,       /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool(   entrypoint,   /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool(   compiler_cmd, /* default_value= */ false, /* required= */ false);
FLAG_SHORT_bool(  f,            /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool(   force,        /* default_value= */ false, /* required= */ false);
FLAG_LONG_string( outdir,       /* default_value= */ "-",   /* required= */ false);
FLAG_LONG_bool(   list,         /* default_value= */ false, /* required= */ false);
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
    std::cerr << "Expected exactly one input file." << std::endl;
    return 1;
  }
  if (flags::entrypoint.value() || flags::compiler_cmd.value()) {
    std::cerr << "Unimplemented flags." << std::endl;
    return 1;
  }

  std::vector<uint32_t> binary;
  if (!ReadBinaryFile(flags::positional_arguments[0].c_str(), &binary)) {
    return 1;
  }

  if (flags::source.value()) {
    std::unordered_map<std::string, std::string> sourceCode;
    if (!ExtractSourceFromModule(binary, &sourceCode)) {
      return 1;
    }

    if (flags::list.value()) {
      for (const auto & [ filename, source ] : sourceCode) {
        printf("%s\n", filename.c_str());
      }
      return 0;
    }

    const bool outputToConsole = flags::outdir.value() == "-";

    if (outputToConsole) {
      for (const auto & [ filename, source ] : sourceCode) {
        std::cout << filename << ":" << std::endl
                  << source << std::endl
                  << std::endl;
      }
      return 0;
    }

    const std::filesystem::path outdirPath(flags::outdir.value());
    if (!OutputSourceFiles(sourceCode, outdirPath.string(),
                           flags::force.value())) {
      return 1;
    }
  }

  // FIXME: implement logic.
  return 0;
}
