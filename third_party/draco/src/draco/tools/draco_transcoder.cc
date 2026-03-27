// Copyright 2019 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <cinttypes>
#include <cstdlib>

#include "draco/core/cycle_timer.h"
#include "draco/core/status.h"
#include "draco/draco_features.h"
#include "draco/texture/texture_utils.h"
#include "draco/tools/draco_transcoder_lib.h"

namespace {

// TODO(fgalligan): Add support for no compression to the transcoder lib.
void Usage() {
  // TODO(b/204212351): Revisit using a raw string literal here for readability.
  printf("Usage: draco_transcoder [options] -i input -o output\n\n");
  printf("Main options:\n");
  printf("  -h | -?         show help.\n");
  printf("  -i <input>      input file name.\n");
  printf("  -o <output>     output file name.\n");
  printf("  -qp <value>     quantization bits for the position attribute, ");
  printf("default=11.\n");
  printf("  -qt <value>     quantization bits for the texture coordinate ");
  printf("attribute, default=10.\n");
  printf("  -qn <value>     quantization bits for the normal vector attribute");
  printf(", default=8.\n");
  printf("  -qc <value>     quantization bits for the color attribute, ");
  printf("default=8.\n");
  printf("  -qtg <value>    quantization bits for the tangent attribute, ");
  printf("default=8.\n");
  printf("  -qw <value>     quantization bits for the weight attribute, ");
  printf("default=8.\n");
  printf("  -qg <value>     quantization bits for any generic attribute, ");
  printf("default=8.\n");

  printf("\nBoolean options may be negated by prefixing 'no'.\n");
}

int StringToInt(const std::string &s) {
  char *end;
  return strtol(s.c_str(), &end, 10);  // NOLINT
}

bool MatchesBooleanOption(const std::string &option, const std::string &value) {
  const std::string opt = "-" + option;
  const std::string noopt = "-no" + option;
  return value == opt || value == noopt;
}

draco::Status TranscodeFile(
    const draco::DracoTranscoder::FileOptions &file_options,
    const draco::DracoTranscodingOptions &transcode_options) {
  draco::CycleTimer timer;
  timer.Start();
  DRACO_ASSIGN_OR_RETURN(std::unique_ptr<draco::DracoTranscoder> dt,
                         draco::DracoTranscoder::Create(transcode_options));

  DRACO_RETURN_IF_ERROR(dt->Transcode(file_options));
  timer.Stop();
  printf("Transcode\t%s\t%" PRId64 "\n", file_options.input_filename.c_str(),
         timer.GetInMs());

  return draco::OkStatus();
}

}  // anonymous namespace

int main(int argc, char **argv) {
  draco::DracoTranscoder::FileOptions file_options;
  draco::DracoTranscodingOptions transcode_options;
  const int argc_check = argc - 1;

  for (int i = 1; i < argc; ++i) {
    if (!strcmp("-h", argv[i]) || !strcmp("-?", argv[i])) {
      Usage();
      return 0;
    } else if (!strcmp("-i", argv[i]) && i < argc_check) {
      file_options.input_filename = argv[++i];
    } else if (!strcmp("-o", argv[i]) && i < argc_check) {
      file_options.output_filename = argv[++i];
    } else if (!strcmp("-qp", argv[i]) && i < argc_check) {
      transcode_options.geometry.quantization_position.SetQuantizationBits(
          StringToInt(argv[++i]));
    } else if (!strcmp("-qt", argv[i]) && i < argc_check) {
      transcode_options.geometry.quantization_bits_tex_coord =
          StringToInt(argv[++i]);
    } else if (!strcmp("-qn", argv[i]) && i < argc_check) {
      transcode_options.geometry.quantization_bits_normal =
          StringToInt(argv[++i]);
    } else if (!strcmp("-qc", argv[i]) && i < argc_check) {
      transcode_options.geometry.quantization_bits_color =
          StringToInt(argv[++i]);
    } else if (!strcmp("-qtg", argv[i]) && i < argc_check) {
      transcode_options.geometry.quantization_bits_tangent =
          StringToInt(argv[++i]);
    } else if (!strcmp("-qw", argv[i]) && i < argc_check) {
      transcode_options.geometry.quantization_bits_weight =
          StringToInt(argv[++i]);
    } else if (!strcmp("-qg", argv[i]) && i < argc_check) {
      transcode_options.geometry.quantization_bits_generic =
          StringToInt(argv[++i]);
    }
  }
  if (argc < 3 || file_options.input_filename.empty() ||
      file_options.output_filename.empty()) {
    Usage();
    return -1;
  }

  const draco::Status status = TranscodeFile(file_options, transcode_options);
  if (!status.ok()) {
    printf("Failed\t%s\t%s\n", file_options.input_filename.c_str(),
           status.error_msg());
    return -1;
  }
  return 0;
}
