// Copyright (c) 2022 The Khronos Group Inc.
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

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#endif

#include "source/diff/diff.h"
#include "source/opt/build_module.h"
#include "source/opt/ir_context.h"
#include "spirv-tools/libspirv.hpp"
#include "tools/io.h"
#include "tools/util/cli_consumer.h"
#include "tools/util/flags.h"

namespace {

constexpr auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_6;

constexpr bool kColorIsPossible =
#if SPIRV_COLOR_TERMINAL
    true;
#else
    false;
#endif

void print_usage(const char* argv0) {
  printf(R"(%s - Compare two SPIR-V files

Usage: %s <src_filename> <dst_filename>

The SPIR-V binary is read from <src_filename> and <dst_filename>.  If either
file ends in .spvasm, the SPIR-V is read as text and disassembled.

The contents of the SPIR-V modules are analyzed and a diff is produced showing a
logical transformation from src to dst, in src's id-space.

  -h, --help      Print this help.
  --version       Display diff version information.

  --color         Force color output. The default when printing to a terminal.
                  If both --color and --no-color is present, --no-color prevails.
  --no-color      Don't print in color. The default when output goes to
                  something other than a terminal (e.g. a pipe, or a shell
                  redirection).
                  If both --color and --no-color is present, --no-color prevails.

  --no-indent     Don't indent instructions.

  --no-header     Don't output the header as leading comments.

  --with-id-map   Also output the mapping between src and dst outputs.

  --ignore-set-binding
                  Don't use set/binding decorations for variable matching.
  --ignore-location
                  Don't use location decorations for variable matching.
)",
         argv0, argv0);
}

bool is_assembly(const char* path) {
  const char* suffix = strrchr(path, '.');
  if (suffix == nullptr) {
    return false;
  }

  return strcmp(suffix, ".spvasm") == 0;
}

std::unique_ptr<spvtools::opt::IRContext> load_module(const char* path) {
  if (is_assembly(path)) {
    std::vector<char> contents;
    if (!ReadTextFile<char>(path, &contents)) return {};

    return spvtools::BuildModule(
        kDefaultEnvironment, spvtools::utils::CLIMessageConsumer,
        std::string(contents.begin(), contents.end()),
        spvtools::SpirvTools::kDefaultAssembleOption |
            SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  }

  std::vector<uint32_t> contents;
  if (!ReadBinaryFile<uint32_t>(path, &contents)) return {};

  return spvtools::BuildModule(kDefaultEnvironment,
                               spvtools::utils::CLIMessageConsumer,
                               contents.data(), contents.size());
}

}  // namespace

// clang-format off
FLAG_SHORT_bool(h,                  /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool( help,               /* default_value= */ false, /* required= */false);
FLAG_LONG_bool( version,            /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool( color,              /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool( no_color,           /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool( no_indent,          /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool( no_header,          /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool( with_id_map,        /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool( ignore_set_binding, /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool( ignore_location,    /* default_value= */ false, /* required= */ false);
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
    printf("%s\n", spvSoftwareVersionDetailsString());
    printf("Target: %s\n", spvTargetEnvDescription(kDefaultEnvironment));
    return 0;
  }

  if (flags::positional_arguments.size() != 2) {
    fprintf(stderr, "error: two input files required.\n");
    return 1;
  }

#if defined(_POSIX_VERSION)
  const bool output_is_tty = isatty(fileno(stdout));
#else
  const bool output_is_tty = true;
#endif

  const std::string& src_file = flags::positional_arguments[0];
  const std::string& dst_file = flags::positional_arguments[1];

  spvtools::diff::Options options;
  options.color_output = (output_is_tty || flags::color.value()) &&
                         !flags::no_color.value() && kColorIsPossible;
  options.indent = !flags::no_indent.value();
  options.no_header = flags::no_header.value();
  options.dump_id_map = flags::with_id_map.value();
  options.ignore_set_binding = flags::ignore_set_binding.value();
  options.ignore_location = flags::ignore_location.value();

  std::unique_ptr<spvtools::opt::IRContext> src = load_module(src_file.c_str());
  std::unique_ptr<spvtools::opt::IRContext> dst = load_module(dst_file.c_str());

  if (!src) {
    fprintf(stderr, "error: Loading src file\n");
  }
  if (!dst) {
    fprintf(stderr, "error: Loading dst file\n");
  }
  if (!src || !dst) {
    return 1;
  }

  spvtools::diff::Diff(src.get(), dst.get(), std::cout, options);

  return 0;
}
