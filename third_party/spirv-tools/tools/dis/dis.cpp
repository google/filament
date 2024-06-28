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

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <stdio.h>  // Need fileno
#include <unistd.h>
#endif

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "spirv-tools/libspirv.h"
#include "tools/io.h"
#include "tools/util/flags.h"

static const std::string kHelpText = R"(%s - Disassemble a SPIR-V binary module

Usage: %s [options] [<filename>]

The SPIR-V binary is read from <filename>. If no file is specified,
or if the filename is "-", then the binary is read from standard input.

Options:

  -h, --help      Print this help.
  --version       Display disassembler version information.

  -o <filename>   Set the output filename.
                  Output goes to standard output if this option is
                  not specified, or if the filename is "-".

  --color         Force color output.  The default when printing to a terminal.
                  Overrides a previous --no-color option.
  --no-color      Don't print in color.  Overrides a previous --color option.
                  The default when output goes to something other than a
                  terminal (e.g. a file, a pipe, or a shell redirection).

  --no-indent     Don't indent instructions.

  --no-header     Don't output the header as leading comments.

  --raw-id        Show raw Id values instead of friendly names.

  --offsets       Show byte offsets for each instruction.

  --comment       Add comments to make reading easier
)";

// clang-format off
FLAG_SHORT_bool  (h,         /* default_value= */ false, /* required= */ false);
FLAG_SHORT_string(o,         /* default_value= */ "-",   /* required= */ false);
FLAG_LONG_bool   (help,      /* default_value= */ false, /* required= */false);
FLAG_LONG_bool   (version,   /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool   (color,     /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool   (no_color,  /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool   (no_indent, /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool   (no_header, /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool   (raw_id,    /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool   (offsets,   /* default_value= */ false, /* required= */ false);
FLAG_LONG_bool   (comment,   /* default_value= */ false, /* required= */ false);
// clang-format on

static const auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_5;

int main(int, const char** argv) {
  if (!flags::Parse(argv)) {
    return 1;
  }

  if (flags::h.value() || flags::help.value()) {
    printf(kHelpText.c_str(), argv[0], argv[0]);
    return 0;
  }

  if (flags::version.value()) {
    printf("%s\n", spvSoftwareVersionDetailsString());
    printf("Target: %s\n", spvTargetEnvDescription(kDefaultEnvironment));
    return 0;
  }

  if (flags::positional_arguments.size() > 1) {
    fprintf(stderr, "error: more than one input file specified.\n");
    return 1;
  }

  const std::string inFile = flags::positional_arguments.size() == 0
                                 ? "-"
                                 : flags::positional_arguments[0];
  const std::string outFile = flags::o.value();

  bool color_is_possible =
#if SPIRV_COLOR_TERMINAL
      true;
#else
      false;
#endif

  uint32_t options = SPV_BINARY_TO_TEXT_OPTION_NONE;

  if (!flags::no_indent.value()) options |= SPV_BINARY_TO_TEXT_OPTION_INDENT;

  if (flags::offsets.value())
    options |= SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET;

  if (flags::no_header.value()) options |= SPV_BINARY_TO_TEXT_OPTION_NO_HEADER;

  if (!flags::raw_id.value())
    options |= SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;

  if (flags::comment.value()) options |= SPV_BINARY_TO_TEXT_OPTION_COMMENT;

  if (flags::o.value() == "-") {
    // Print to standard output.
    options |= SPV_BINARY_TO_TEXT_OPTION_PRINT;
    if (color_is_possible && !flags::no_color.value()) {
      bool output_is_tty = true;
#if defined(_POSIX_VERSION)
      output_is_tty = isatty(fileno(stdout));
#endif
      if (output_is_tty || flags::color.value()) {
        options |= SPV_BINARY_TO_TEXT_OPTION_COLOR;
      }
    }
  }

  // Read the input binary.
  std::vector<uint32_t> contents;
  if (!ReadBinaryFile<uint32_t>(inFile.c_str(), &contents)) return 1;

  // If printing to standard output, then spvBinaryToText should
  // do the printing.  In particular, colour printing on Windows is
  // controlled by modifying console objects synchronously while
  // outputting to the stream rather than by injecting escape codes
  // into the output stream.
  // If the printing option is off, then save the text in memory, so
  // it can be emitted later in this function.
  const bool print_to_stdout = SPV_BINARY_TO_TEXT_OPTION_PRINT & options;
  spv_text text = nullptr;
  spv_text* textOrNull = print_to_stdout ? nullptr : &text;
  spv_diagnostic diagnostic = nullptr;
  spv_context context = spvContextCreate(kDefaultEnvironment);
  spv_result_t error =
      spvBinaryToText(context, contents.data(), contents.size(), options,
                      textOrNull, &diagnostic);
  spvContextDestroy(context);
  if (error) {
    spvDiagnosticPrint(diagnostic);
    spvDiagnosticDestroy(diagnostic);
    return error;
  }

  if (!print_to_stdout) {
    if (!WriteFile<char>(outFile.c_str(), "w", text->str, text->length)) {
      spvTextDestroy(text);
      return 1;
    }
  }
  spvTextDestroy(text);

  return 0;
}
