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

static void print_usage(char* argv0) {
  printf(R"(%s - Compare two SPIR-V files

Usage: %s <src_filename> <dst_filename>

The SPIR-V binary is read from <src_filename> and <dst_filename>.  If either
file ends in .spvasm, the SPIR-V is read as text and disassembled.

The contents of the SPIR-V modules are analyzed and a diff is produced showing a
logical transformation from src to dst, in src's id-space.

  -h, --help      Print this help.
  --version       Display diff version information.

  --color         Force color output.  The default when printing to a terminal.
                  Overrides a previous --no-color option.
  --no-color      Don't print in color.  Overrides a previous --color option.
                  The default when output goes to something other than a
                  terminal (e.g. a pipe, or a shell redirection).

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

static const auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_6;

static bool is_assembly(const char* path) {
  const char* suffix = strrchr(path, '.');
  if (suffix == nullptr) {
    return false;
  }

  return strcmp(suffix, ".spvasm") == 0;
}

static std::unique_ptr<spvtools::opt::IRContext> load_module(const char* path) {
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

int main(int argc, char** argv) {
  const char* src_file = nullptr;
  const char* dst_file = nullptr;
  bool color_is_possible =
#if SPIRV_COLOR_TERMINAL
      true;
#else
      false;
#endif
  bool force_color = false;
  bool force_no_color = false;
  bool allow_indent = true;
  bool no_header = false;
  bool dump_id_map = false;
  bool ignore_set_binding = false;
  bool ignore_location = false;

  for (int argi = 1; argi < argc; ++argi) {
    if ('-' == argv[argi][0]) {
      switch (argv[argi][1]) {
        case 'h':
          print_usage(argv[0]);
          return 0;
        case '-': {
          // Long options
          if (strcmp(argv[argi], "--no-color") == 0) {
            force_no_color = true;
            force_color = false;
          } else if (strcmp(argv[argi], "--color") == 0) {
            force_no_color = false;
            force_color = true;
          } else if (strcmp(argv[argi], "--no-indent") == 0) {
            allow_indent = false;
          } else if (strcmp(argv[argi], "--no-header") == 0) {
            no_header = true;
          } else if (strcmp(argv[argi], "--with-id-map") == 0) {
            dump_id_map = true;
          } else if (strcmp(argv[argi], "--ignore-set-binding") == 0) {
            ignore_set_binding = true;
          } else if (strcmp(argv[argi], "--ignore-location") == 0) {
            ignore_location = true;
          } else if (strcmp(argv[argi], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
          } else if (strcmp(argv[argi], "--version") == 0) {
            printf("%s\n", spvSoftwareVersionDetailsString());
            printf("Target: %s\n",
                   spvTargetEnvDescription(kDefaultEnvironment));
            return 0;
          } else {
            print_usage(argv[0]);
            return 1;
          }
        } break;
        default:
          print_usage(argv[0]);
          return 1;
      }
    } else {
      if (src_file == nullptr) {
        src_file = argv[argi];
      } else if (dst_file == nullptr) {
        dst_file = argv[argi];
      } else {
        fprintf(stderr, "error: More than two input files specified\n");
        return 1;
      }
    }
  }

  if (src_file == nullptr || dst_file == nullptr) {
    print_usage(argv[0]);
    return 1;
  }

  spvtools::diff::Options options;

  if (allow_indent) options.indent = true;
  if (no_header) options.no_header = true;
  if (dump_id_map) options.dump_id_map = true;
  if (ignore_set_binding) options.ignore_set_binding = true;
  if (ignore_location) options.ignore_location = true;

  if (color_is_possible && !force_no_color) {
    bool output_is_tty = true;
#if defined(_POSIX_VERSION)
    output_is_tty = isatty(fileno(stdout));
#endif
    if (output_is_tty || force_color) {
      options.color_output = true;
    }
  }

  std::unique_ptr<spvtools::opt::IRContext> src = load_module(src_file);
  std::unique_ptr<spvtools::opt::IRContext> dst = load_module(dst_file);

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
