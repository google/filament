// Copyright (c) 2017 Google Inc.
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
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "source/spirv_stats.h"
#include "source/table.h"
#include "spirv-tools/libspirv.h"
#include "stats_analyzer.h"
#include "tools/io.h"

using spvtools::SpirvStats;

namespace {

struct ScopedContext {
  ScopedContext(spv_target_env env) : context(spvContextCreate(env)) {}
  ~ScopedContext() { spvContextDestroy(context); }
  spv_context context;
};

void PrintUsage(char* argv0) {
  printf(
      R"(%s - Collect statistics from one or more SPIR-V binary file(s).

USAGE: %s [options] [<filepaths>]

TIP: In order to collect statistics from all .spv files under current dir use
find . -name "*.spv" -print0 | xargs -0 -s 2000000 %s

Options:
  -h, --help
                   Print this help.

  -v, --verbose
                   Print additional info to stderr.

  --codegen_opcode_hist
                   Output generated C++ code for opcode histogram.
                   This flag disables non-C++ output.

  --codegen_opcode_and_num_operands_hist
                   Output generated C++ code for opcode_and_num_operands
                   histogram.
                   This flag disables non-C++ output.

  --codegen_opcode_and_num_operands_markov_huffman_codecs
                   Output generated C++ code for Huffman codecs of
                   opcode_and_num_operands Markov chain.
                   This flag disables non-C++ output.

  --codegen_literal_string_huffman_codecs
                   Output generated C++ code for Huffman codecs for
                   literal strings.
                   This flag disables non-C++ output.

  --codegen_non_id_word_huffman_codecs
                   Output generated C++ code for Huffman codecs for
                   single-word non-id slots.
                   This flag disables non-C++ output.

  --codegen_id_descriptor_huffman_codecs
                   Output generated C++ code for Huffman codecs for
                   common id descriptors.
                   This flag disables non-C++ output.
)",
      argv0, argv0, argv0);
}

void DiagnosticsMessageHandler(spv_message_level_t level, const char*,
                               const spv_position_t& position,
                               const char* message) {
  switch (level) {
    case SPV_MSG_FATAL:
    case SPV_MSG_INTERNAL_ERROR:
    case SPV_MSG_ERROR:
      std::cerr << "error: " << position.index << ": " << message << std::endl;
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
}

}  // namespace

int main(int argc, char** argv) {
  bool continue_processing = true;
  int return_code = 0;

  bool expect_output_path = false;
  bool verbose = false;
  bool export_text = true;
  bool codegen_opcode_hist = false;
  bool codegen_opcode_and_num_operands_hist = false;
  bool codegen_opcode_and_num_operands_markov_huffman_codecs = false;
  bool codegen_literal_string_huffman_codecs = false;
  bool codegen_non_id_word_huffman_codecs = false;
  bool codegen_id_descriptor_huffman_codecs = false;

  std::vector<const char*> paths;
  const char* output_path = nullptr;

  for (int argi = 1; continue_processing && argi < argc; ++argi) {
    const char* cur_arg = argv[argi];
    if ('-' == cur_arg[0]) {
      if (0 == strcmp(cur_arg, "--help") || 0 == strcmp(cur_arg, "-h")) {
        PrintUsage(argv[0]);
        continue_processing = false;
        return_code = 0;
      } else if (0 == strcmp(cur_arg, "--codegen_opcode_hist")) {
        codegen_opcode_hist = true;
        export_text = false;
      } else if (0 ==
                 strcmp(cur_arg, "--codegen_opcode_and_num_operands_hist")) {
        codegen_opcode_and_num_operands_hist = true;
        export_text = false;
      } else if (strcmp(
                     "--codegen_opcode_and_num_operands_markov_huffman_codecs",
                     cur_arg) == 0) {
        codegen_opcode_and_num_operands_markov_huffman_codecs = true;
        export_text = false;
      } else if (0 ==
                 strcmp(cur_arg, "--codegen_literal_string_huffman_codecs")) {
        codegen_literal_string_huffman_codecs = true;
        export_text = false;
      } else if (0 == strcmp(cur_arg, "--codegen_non_id_word_huffman_codecs")) {
        codegen_non_id_word_huffman_codecs = true;
        export_text = false;
      } else if (0 ==
                 strcmp(cur_arg, "--codegen_id_descriptor_huffman_codecs")) {
        codegen_id_descriptor_huffman_codecs = true;
        export_text = false;
      } else if (0 == strcmp(cur_arg, "--verbose") ||
                 0 == strcmp(cur_arg, "-v")) {
        verbose = true;
      } else if (0 == strcmp(cur_arg, "--output") ||
                 0 == strcmp(cur_arg, "-o")) {
        expect_output_path = true;
      } else {
        PrintUsage(argv[0]);
        continue_processing = false;
        return_code = 1;
      }
    } else {
      if (expect_output_path) {
        output_path = cur_arg;
        expect_output_path = false;
      } else {
        paths.push_back(cur_arg);
      }
    }
  }

  // Exit if command line parsing was not successful.
  if (!continue_processing) {
    return return_code;
  }

  std::cerr << "Processing " << paths.size() << " files..." << std::endl;

  ScopedContext ctx(SPV_ENV_UNIVERSAL_1_1);
  spvtools::SetContextMessageConsumer(ctx.context, DiagnosticsMessageHandler);

  spvtools::SpirvStats stats;
  stats.opcode_markov_hist.resize(1);

  for (size_t index = 0; index < paths.size(); ++index) {
    const size_t kMilestonePeriod = 1000;
    if (verbose) {
      if (index % kMilestonePeriod == kMilestonePeriod - 1)
        std::cerr << "Processed " << index + 1 << " files..." << std::endl;
    }

    const char* path = paths[index];
    std::vector<uint32_t> contents;
    if (!ReadFile<uint32_t>(path, "rb", &contents)) return 1;

    if (SPV_SUCCESS != spvtools::AggregateStats(*ctx.context, contents.data(),
                                                contents.size(), nullptr,
                                                &stats)) {
      std::cerr << "error: Failed to aggregate stats for " << path << std::endl;
      return 1;
    }
  }

  StatsAnalyzer analyzer(stats);

  std::ofstream fout;
  if (output_path) {
    fout.open(output_path);
    if (!fout.is_open()) {
      std::cerr << "error: Failed to open " << output_path << std::endl;
      return 1;
    }
  }

  std::ostream& out = fout.is_open() ? fout : std::cout;

  if (export_text) {
    out << std::endl;
    analyzer.WriteVersion(out);
    analyzer.WriteGenerator(out);

    out << std::endl;
    analyzer.WriteCapability(out);

    out << std::endl;
    analyzer.WriteExtension(out);

    out << std::endl;
    analyzer.WriteOpcode(out);

    out << std::endl;
    analyzer.WriteOpcodeMarkov(out);

    out << std::endl;
    analyzer.WriteConstantLiterals(out);
  }

  if (codegen_opcode_hist) {
    out << std::endl;
    analyzer.WriteCodegenOpcodeHist(out);
  }

  if (codegen_opcode_and_num_operands_hist) {
    out << std::endl;
    analyzer.WriteCodegenOpcodeAndNumOperandsHist(out);
  }

  if (codegen_opcode_and_num_operands_markov_huffman_codecs) {
    out << std::endl;
    analyzer.WriteCodegenOpcodeAndNumOperandsMarkovHuffmanCodecs(out);
  }

  if (codegen_literal_string_huffman_codecs) {
    out << std::endl;
    analyzer.WriteCodegenLiteralStringHuffmanCodecs(out);
  }

  if (codegen_non_id_word_huffman_codecs) {
    out << std::endl;
    analyzer.WriteCodegenNonIdWordHuffmanCodecs(out);
  }

  if (codegen_id_descriptor_huffman_codecs) {
    out << std::endl;
    analyzer.WriteCodegenIdDescriptorHuffmanCodecs(out);
  }

  return 0;
}
