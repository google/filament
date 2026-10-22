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
#include <cstdlib>

#include "draco/compression/config/compression_shared.h"
#include "draco/compression/encode.h"
#include "draco/compression/expert_encode.h"
#include "draco/core/cycle_timer.h"
#include "draco/io/file_utils.h"
#include "draco/io/mesh_io.h"
#include "draco/io/point_cloud_io.h"

namespace {

struct Options {
  Options();

  bool is_point_cloud;
  int pos_quantization_bits;
  int tex_coords_quantization_bits;
  bool tex_coords_deleted;
  int normals_quantization_bits;
  bool normals_deleted;
  int generic_quantization_bits;
  bool generic_deleted;
  int compression_level;
  bool preserve_polygons;
  bool use_metadata;
  std::string input;
  std::string output;
};

Options::Options()
    : is_point_cloud(false),
      pos_quantization_bits(11),
      tex_coords_quantization_bits(10),
      tex_coords_deleted(false),
      normals_quantization_bits(8),
      normals_deleted(false),
      generic_quantization_bits(8),
      generic_deleted(false),
      compression_level(7),
      preserve_polygons(false),
      use_metadata(false) {}

void Usage() {
  printf("Usage: draco_encoder [options] -i input\n");
  printf("\n");
  printf("Main options:\n");
  printf("  -h | -?               show help.\n");
  printf("  -i <input>            input file name.\n");
  printf("  -o <output>           output file name.\n");
  printf(
      "  -point_cloud          forces the input to be encoded as a point "
      "cloud.\n");
  printf(
      "  -qp <value>           quantization bits for the position "
      "attribute, default=11.\n");
  printf(
      "  -qt <value>           quantization bits for the texture coordinate "
      "attribute, default=10.\n");
  printf(
      "  -qn <value>           quantization bits for the normal vector "
      "attribute, default=8.\n");
  printf(
      "  -qg <value>           quantization bits for any generic attribute, "
      "default=8.\n");
  printf(
      "  -cl <value>           compression level [0-10], most=10, least=0, "
      "default=7.\n");
  printf(
      "  --skip ATTRIBUTE_NAME skip a given attribute (NORMAL, TEX_COORD, "
      "GENERIC)\n");
  printf(
      "  --metadata            use metadata to encode extra information in "
      "mesh files.\n");
  // Mesh with polygonal faces loaded from OBJ format is converted to triangular
  // mesh and polygon reconstruction information is encoded into a new generic
  // attribute.
  printf("  -preserve_polygons    encode polygon info as an attribute.\n");

  printf(
      "\nUse negative quantization values to skip the specified attribute\n");
}

int StringToInt(const std::string &s) {
  char *end;
  return strtol(s.c_str(), &end, 10);  // NOLINT
}

void PrintOptions(const draco::PointCloud &pc, const Options &options) {
  printf("Encoder options:\n");
  printf("  Compression level = %d\n", options.compression_level);
  if (options.pos_quantization_bits == 0) {
    printf("  Positions: No quantization\n");
  } else {
    printf("  Positions: Quantization = %d bits\n",
           options.pos_quantization_bits);
  }

  if (pc.GetNamedAttributeId(draco::GeometryAttribute::TEX_COORD) >= 0) {
    if (options.tex_coords_quantization_bits == 0) {
      printf("  Texture coordinates: No quantization\n");
    } else {
      printf("  Texture coordinates: Quantization = %d bits\n",
             options.tex_coords_quantization_bits);
    }
  } else if (options.tex_coords_deleted) {
    printf("  Texture coordinates: Skipped\n");
  }

  if (pc.GetNamedAttributeId(draco::GeometryAttribute::NORMAL) >= 0) {
    if (options.normals_quantization_bits == 0) {
      printf("  Normals: No quantization\n");
    } else {
      printf("  Normals: Quantization = %d bits\n",
             options.normals_quantization_bits);
    }
  } else if (options.normals_deleted) {
    printf("  Normals: Skipped\n");
  }

  if (pc.GetNamedAttributeId(draco::GeometryAttribute::GENERIC) >= 0) {
    if (options.generic_quantization_bits == 0) {
      printf("  Generic: No quantization\n");
    } else {
      printf("  Generic: Quantization = %d bits\n",
             options.generic_quantization_bits);
    }
  } else if (options.generic_deleted) {
    printf("  Generic: Skipped\n");
  }
  printf("\n");
}

int EncodePointCloudToFile(const draco::PointCloud &pc, const std::string &file,
                           draco::ExpertEncoder *encoder) {
  draco::CycleTimer timer;
  // Encode the geometry.
  draco::EncoderBuffer buffer;
  timer.Start();
  const draco::Status status = encoder->EncodeToBuffer(&buffer);
  if (!status.ok()) {
    printf("Failed to encode the point cloud.\n");
    printf("%s\n", status.error_msg());
    return -1;
  }
  timer.Stop();
  // Save the encoded geometry into a file.
  if (!draco::WriteBufferToFile(buffer.data(), buffer.size(), file)) {
    printf("Failed to write the output file.\n");
    return -1;
  }
  printf("Encoded point cloud saved to %s (%" PRId64 " ms to encode).\n",
         file.c_str(), timer.GetInMs());
  printf("\nEncoded size = %zu bytes\n\n", buffer.size());
  return 0;
}

int EncodeMeshToFile(const draco::Mesh &mesh, const std::string &file,
                     draco::ExpertEncoder *encoder) {
  draco::CycleTimer timer;
  // Encode the geometry.
  draco::EncoderBuffer buffer;
  timer.Start();
  const draco::Status status = encoder->EncodeToBuffer(&buffer);
  if (!status.ok()) {
    printf("Failed to encode the mesh.\n");
    printf("%s\n", status.error_msg());
    return -1;
  }
  timer.Stop();
  // Save the encoded geometry into a file.
  if (!draco::WriteBufferToFile(buffer.data(), buffer.size(), file)) {
    printf("Failed to create the output file.\n");
    return -1;
  }
  printf("Encoded mesh saved to %s (%" PRId64 " ms to encode).\n", file.c_str(),
         timer.GetInMs());
  printf("\nEncoded size = %zu bytes\n\n", buffer.size());
  return 0;
}

}  // anonymous namespace

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
    } else if (!strcmp("-point_cloud", argv[i])) {
      options.is_point_cloud = true;
    } else if (!strcmp("-qp", argv[i]) && i < argc_check) {
      options.pos_quantization_bits = StringToInt(argv[++i]);
      if (options.pos_quantization_bits > 30) {
        printf(
            "Error: The maximum number of quantization bits for the position "
            "attribute is 30.\n");
        return -1;
      }
    } else if (!strcmp("-qt", argv[i]) && i < argc_check) {
      options.tex_coords_quantization_bits = StringToInt(argv[++i]);
      if (options.tex_coords_quantization_bits > 30) {
        printf(
            "Error: The maximum number of quantization bits for the texture "
            "coordinate attribute is 30.\n");
        return -1;
      }
    } else if (!strcmp("-qn", argv[i]) && i < argc_check) {
      options.normals_quantization_bits = StringToInt(argv[++i]);
      if (options.normals_quantization_bits > 30) {
        printf(
            "Error: The maximum number of quantization bits for the normal "
            "attribute is 30.\n");
        return -1;
      }
    } else if (!strcmp("-qg", argv[i]) && i < argc_check) {
      options.generic_quantization_bits = StringToInt(argv[++i]);
      if (options.generic_quantization_bits > 30) {
        printf(
            "Error: The maximum number of quantization bits for generic "
            "attributes is 30.\n");
        return -1;
      }
    } else if (!strcmp("-cl", argv[i]) && i < argc_check) {
      options.compression_level = StringToInt(argv[++i]);
    } else if (!strcmp("--skip", argv[i]) && i < argc_check) {
      if (!strcmp("NORMAL", argv[i + 1])) {
        options.normals_quantization_bits = -1;
      } else if (!strcmp("TEX_COORD", argv[i + 1])) {
        options.tex_coords_quantization_bits = -1;
      } else if (!strcmp("GENERIC", argv[i + 1])) {
        options.generic_quantization_bits = -1;
      } else {
        printf("Error: Invalid attribute name after --skip\n");
        return -1;
      }
      ++i;
    } else if (!strcmp("--metadata", argv[i])) {
      options.use_metadata = true;
    } else if (!strcmp("-preserve_polygons", argv[i])) {
      options.preserve_polygons = true;
    }
  }
  if (argc < 3 || options.input.empty()) {
    Usage();
    return -1;
  }

  std::unique_ptr<draco::PointCloud> pc;
  draco::Mesh *mesh = nullptr;
  if (!options.is_point_cloud) {
    draco::Options load_options;
    load_options.SetBool("use_metadata", options.use_metadata);
    load_options.SetBool("preserve_polygons", options.preserve_polygons);
    auto maybe_mesh = draco::ReadMeshFromFile(options.input, load_options);
    if (!maybe_mesh.ok()) {
      printf("Failed loading the input mesh: %s.\n",
             maybe_mesh.status().error_msg());
      return -1;
    }
    mesh = maybe_mesh.value().get();
    pc = std::move(maybe_mesh).value();
  } else {
    auto maybe_pc = draco::ReadPointCloudFromFile(options.input);
    if (!maybe_pc.ok()) {
      printf("Failed loading the input point cloud: %s.\n",
             maybe_pc.status().error_msg());
      return -1;
    }
    pc = std::move(maybe_pc).value();
  }

  if (options.pos_quantization_bits < 0) {
    printf("Error: Position attribute cannot be skipped.\n");
    return -1;
  }

  // Delete attributes if needed. This needs to happen before we set any
  // quantization settings.
  if (options.tex_coords_quantization_bits < 0) {
    if (pc->NumNamedAttributes(draco::GeometryAttribute::TEX_COORD) > 0) {
      options.tex_coords_deleted = true;
    }
    while (pc->NumNamedAttributes(draco::GeometryAttribute::TEX_COORD) > 0) {
      pc->DeleteAttribute(
          pc->GetNamedAttributeId(draco::GeometryAttribute::TEX_COORD, 0));
    }
  }
  if (options.normals_quantization_bits < 0) {
    if (pc->NumNamedAttributes(draco::GeometryAttribute::NORMAL) > 0) {
      options.normals_deleted = true;
    }
    while (pc->NumNamedAttributes(draco::GeometryAttribute::NORMAL) > 0) {
      pc->DeleteAttribute(
          pc->GetNamedAttributeId(draco::GeometryAttribute::NORMAL, 0));
    }
  }
  if (options.generic_quantization_bits < 0) {
    if (pc->NumNamedAttributes(draco::GeometryAttribute::GENERIC) > 0) {
      options.generic_deleted = true;
    }
    while (pc->NumNamedAttributes(draco::GeometryAttribute::GENERIC) > 0) {
      pc->DeleteAttribute(
          pc->GetNamedAttributeId(draco::GeometryAttribute::GENERIC, 0));
    }
  }
#ifdef DRACO_ATTRIBUTE_INDICES_DEDUPLICATION_SUPPORTED
  // If any attribute has been deleted, run deduplication of point indices again
  // as some points can be possibly combined.
  if (options.tex_coords_deleted || options.normals_deleted ||
      options.generic_deleted) {
    pc->DeduplicatePointIds();
  }
#endif

  // Convert compression level to speed (that 0 = slowest, 10 = fastest).
  const int speed = 10 - options.compression_level;

  draco::Encoder encoder;

  // Setup encoder options.
  if (options.pos_quantization_bits > 0) {
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION,
                                     options.pos_quantization_bits);
  }
  if (options.tex_coords_quantization_bits > 0) {
    encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD,
                                     options.tex_coords_quantization_bits);
  }
  if (options.normals_quantization_bits > 0) {
    encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL,
                                     options.normals_quantization_bits);
  }
  if (options.generic_quantization_bits > 0) {
    encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC,
                                     options.generic_quantization_bits);
  }
  encoder.SetSpeedOptions(speed, speed);

  if (options.output.empty()) {
    // Create a default output file by attaching .drc to the input file name.
    options.output = options.input + ".drc";
  }

  PrintOptions(*pc, options);

  const bool input_is_mesh = mesh && mesh->num_faces() > 0;

  // Convert to ExpertEncoder that allows us to set per-attribute options.
  std::unique_ptr<draco::ExpertEncoder> expert_encoder;
  if (input_is_mesh) {
    expert_encoder.reset(new draco::ExpertEncoder(*mesh));
  } else {
    expert_encoder.reset(new draco::ExpertEncoder(*pc));
  }
  expert_encoder->Reset(encoder.CreateExpertEncoderOptions(*pc));

  // Check if there is an attribute that stores polygon edges. If so, we disable
  // the default prediction scheme for the attribute as it actually makes the
  // compression worse.
  const int poly_att_id =
      pc->GetAttributeIdByMetadataEntry("name", "added_edges");
  if (poly_att_id != -1) {
    expert_encoder->SetAttributePredictionScheme(
        poly_att_id, draco::PredictionSchemeMethod::PREDICTION_NONE);
  }

  int ret = -1;

  if (input_is_mesh) {
    ret = EncodeMeshToFile(*mesh, options.output, expert_encoder.get());
  } else {
    ret = EncodePointCloudToFile(*pc, options.output, expert_encoder.get());
  }

  if (ret != -1 && options.compression_level < 10) {
    printf(
        "For better compression, increase the compression level up to '-cl 10' "
        ".\n\n");
  }

  return ret;
}
