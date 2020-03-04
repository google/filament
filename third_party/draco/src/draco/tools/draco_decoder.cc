// Copyright 2016 The Draco Authors.
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

#include "draco/compression/decode.h"
#include "draco/core/cycle_timer.h"
#include "draco/io/file_utils.h"
#include "draco/io/obj_encoder.h"
#include "draco/io/parser_utils.h"
#include "draco/io/ply_encoder.h"

namespace {

struct Options {
  Options();

  std::string input;
  std::string output;
};

Options::Options() {}

void Usage() {
  printf("Usage: draco_decoder [options] -i input\n");
  printf("\n");
  printf("Main options:\n");
  printf("  -h | -?               show help.\n");
  printf("  -o <output>           output file name.\n");
}

int ReturnError(const draco::Status &status) {
  printf("Failed to decode the input file %s\n", status.error_msg());
  return -1;
}

}  // namespace

int main(int argc, char **argv) {
  Options options;
  const int argc_check = argc - 1;

  for (int i = 1; i < argc; ++i) {
    if (!strcmp("-h", argv[i]) || !strcmp("-?", argv[i])) {
      Usage();
      return 0;
    } else if (!strcmp("-i", argv[i]) && i < argc_check) {
      options.input = argv[++i];
    } else if (!strcmp("-o", argv[i]) && i < argc_check) {
      options.output = argv[++i];
    }
  }
  if (argc < 3 || options.input.empty()) {
    Usage();
    return -1;
  }

  std::vector<char> data;
  if (!draco::ReadFileToBuffer(options.input, &data)) {
    printf("Failed opening the input file.\n");
    return -1;
  }

  if (data.empty()) {
    printf("Empty input file.\n");
    return -1;
  }

  // Create a draco decoding buffer. Note that no data is copied in this step.
  draco::DecoderBuffer buffer;
  buffer.Init(data.data(), data.size());

  draco::CycleTimer timer;
  // Decode the input data into a geometry.
  std::unique_ptr<draco::PointCloud> pc;
  draco::Mesh *mesh = nullptr;
  auto type_statusor = draco::Decoder::GetEncodedGeometryType(&buffer);
  if (!type_statusor.ok()) {
    return ReturnError(type_statusor.status());
  }
  const draco::EncodedGeometryType geom_type = type_statusor.value();
  if (geom_type == draco::TRIANGULAR_MESH) {
    timer.Start();
    draco::Decoder decoder;
    auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
    if (!statusor.ok()) {
      return ReturnError(statusor.status());
    }
    std::unique_ptr<draco::Mesh> in_mesh = std::move(statusor).value();
    timer.Stop();
    if (in_mesh) {
      mesh = in_mesh.get();
      pc = std::move(in_mesh);
    }
  } else if (geom_type == draco::POINT_CLOUD) {
    // Failed to decode it as mesh, so let's try to decode it as a point cloud.
    timer.Start();
    draco::Decoder decoder;
    auto statusor = decoder.DecodePointCloudFromBuffer(&buffer);
    if (!statusor.ok()) {
      return ReturnError(statusor.status());
    }
    pc = std::move(statusor).value();
    timer.Stop();
  }

  if (pc == nullptr) {
    printf("Failed to decode the input file.\n");
    return -1;
  }

  if (options.output.empty()) {
    // Save the output model into a ply file.
    options.output = options.input + ".ply";
  }

  // Save the decoded geometry into a file.
  // TODO(fgalligan): Change extension code to look for '.'.
  const std::string extension = draco::parser::ToLower(
      options.output.size() >= 4
          ? options.output.substr(options.output.size() - 4)
          : options.output);

  if (extension == ".obj") {
    draco::ObjEncoder obj_encoder;
    if (mesh) {
      if (!obj_encoder.EncodeToFile(*mesh, options.output)) {
        printf("Failed to store the decoded mesh as OBJ.\n");
        return -1;
      }
    } else {
      if (!obj_encoder.EncodeToFile(*pc.get(), options.output)) {
        printf("Failed to store the decoded point cloud as OBJ.\n");
        return -1;
      }
    }
  } else if (extension == ".ply") {
    draco::PlyEncoder ply_encoder;
    if (mesh) {
      if (!ply_encoder.EncodeToFile(*mesh, options.output)) {
        printf("Failed to store the decoded mesh as PLY.\n");
        return -1;
      }
    } else {
      if (!ply_encoder.EncodeToFile(*pc.get(), options.output)) {
        printf("Failed to store the decoded point cloud as PLY.\n");
        return -1;
      }
    }
  } else {
    printf("Invalid extension of the output file. Use either .ply or .obj.\n");
    return -1;
  }
  printf("Decoded geometry saved to %s (%" PRId64 " ms to decode)\n",
         options.output.c_str(), timer.GetInMs());
  return 0;
}
