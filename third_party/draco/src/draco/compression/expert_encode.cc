// Copyright 2017 The Draco Authors.
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
#include "draco/compression/expert_encode.h"

#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "draco/compression/mesh/mesh_edgebreaker_encoder.h"
#include "draco/compression/mesh/mesh_sequential_encoder.h"
#ifdef DRACO_POINT_CLOUD_COMPRESSION_SUPPORTED
#include "draco/compression/point_cloud/point_cloud_kd_tree_encoder.h"
#include "draco/compression/point_cloud/point_cloud_sequential_encoder.h"
#endif

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/core/bit_utils.h"
#endif
namespace draco {

ExpertEncoder::ExpertEncoder(const PointCloud &point_cloud)
    : point_cloud_(&point_cloud), mesh_(nullptr) {}

ExpertEncoder::ExpertEncoder(const Mesh &mesh)
    : point_cloud_(&mesh), mesh_(&mesh) {}

Status ExpertEncoder::EncodeToBuffer(EncoderBuffer *out_buffer) {
  if (point_cloud_ == nullptr) {
    return Status(Status::DRACO_ERROR, "Invalid input geometry.");
  }
  if (mesh_ == nullptr) {
    return EncodePointCloudToBuffer(*point_cloud_, out_buffer);
  }
  return EncodeMeshToBuffer(*mesh_, out_buffer);
}

Status ExpertEncoder::EncodePointCloudToBuffer(const PointCloud &pc,
                                               EncoderBuffer *out_buffer) {
#ifdef DRACO_POINT_CLOUD_COMPRESSION_SUPPORTED
#ifdef DRACO_TRANSCODER_SUPPORTED
  // Apply DracoCompressionOptions associated with the point cloud.
  DRACO_RETURN_IF_ERROR(ApplyCompressionOptions(pc));
#endif  // DRACO_TRANSCODER_SUPPORTED

  std::unique_ptr<PointCloudEncoder> encoder;
  const int encoding_method = options().GetGlobalInt("encoding_method", -1);

  if (encoding_method == POINT_CLOUD_SEQUENTIAL_ENCODING) {
    // Use sequential encoding if requested.
    encoder.reset(new PointCloudSequentialEncoder());
  } else if (encoding_method == -1 && options().GetSpeed() == 10) {
    // Use sequential encoding if speed is at max.
    encoder.reset(new PointCloudSequentialEncoder());
  } else {
    // Speed < 10, use POINT_CLOUD_KD_TREE_ENCODING if possible.
    bool kd_tree_possible = true;
    // Kd-Tree encoder can be currently used only when the following conditions
    // are satisfied for all attributes:
    //     -data type is float32 and quantization is enabled, OR
    //     -data type is uint32, uint16, uint8 or int32, int16, int8
    for (int i = 0; i < pc.num_attributes(); ++i) {
      const PointAttribute *const att = pc.attribute(i);
      if (kd_tree_possible && att->data_type() != DT_FLOAT32 &&
          att->data_type() != DT_UINT32 && att->data_type() != DT_UINT16 &&
          att->data_type() != DT_UINT8 && att->data_type() != DT_INT32 &&
          att->data_type() != DT_INT16 && att->data_type() != DT_INT8) {
        kd_tree_possible = false;
      }
      if (kd_tree_possible && att->data_type() == DT_FLOAT32 &&
          options().GetAttributeInt(i, "quantization_bits", -1) <= 0) {
        kd_tree_possible = false;  // Quantization not enabled.
      }
      if (!kd_tree_possible) {
        break;
      }
    }

    if (kd_tree_possible) {
      // Create kD-tree encoder (all checks passed).
      encoder.reset(new PointCloudKdTreeEncoder());
    } else if (encoding_method == POINT_CLOUD_KD_TREE_ENCODING) {
      // Encoding method was explicitly specified but we cannot use it for
      // the given input (some of the checks above failed).
      return Status(Status::DRACO_ERROR, "Invalid encoding method.");
    }
  }
  if (!encoder) {
    // Default choice.
    encoder.reset(new PointCloudSequentialEncoder());
  }
  encoder->SetPointCloud(pc);
  DRACO_RETURN_IF_ERROR(encoder->Encode(options(), out_buffer));

  set_num_encoded_points(encoder->num_encoded_points());
  set_num_encoded_faces(0);
  return OkStatus();
#else
  return Status(Status::DRACO_ERROR, "Point cloud encoding is not enabled.");
#endif
}

Status ExpertEncoder::EncodeMeshToBuffer(const Mesh &m,
                                         EncoderBuffer *out_buffer) {
#ifdef DRACO_TRANSCODER_SUPPORTED
  // Apply DracoCompressionOptions associated with the mesh.
  DRACO_RETURN_IF_ERROR(ApplyCompressionOptions(m));
#endif  // DRACO_TRANSCODER_SUPPORTED

  std::unique_ptr<MeshEncoder> encoder;
  // Select the encoding method only based on the provided options.
  int encoding_method = options().GetGlobalInt("encoding_method", -1);
  if (encoding_method == -1) {
    // For now select the edgebreaker for all options expect of speed 10
    if (options().GetSpeed() == 10) {
      encoding_method = MESH_SEQUENTIAL_ENCODING;
    } else {
      encoding_method = MESH_EDGEBREAKER_ENCODING;
    }
  }
  if (encoding_method == MESH_EDGEBREAKER_ENCODING) {
    encoder = std::unique_ptr<MeshEncoder>(new MeshEdgebreakerEncoder());
  } else {
    encoder = std::unique_ptr<MeshEncoder>(new MeshSequentialEncoder());
  }
  encoder->SetMesh(m);

  DRACO_RETURN_IF_ERROR(encoder->Encode(options(), out_buffer));

  set_num_encoded_points(encoder->num_encoded_points());
  set_num_encoded_faces(encoder->num_encoded_faces());
  return OkStatus();
}

void ExpertEncoder::Reset(const EncoderOptions &options) {
  Base::Reset(options);
}

void ExpertEncoder::Reset() { Base::Reset(); }

void ExpertEncoder::SetSpeedOptions(int encoding_speed, int decoding_speed) {
  Base::SetSpeedOptions(encoding_speed, decoding_speed);
}

void ExpertEncoder::SetAttributeQuantization(int32_t attribute_id,
                                             int quantization_bits) {
  options().SetAttributeInt(attribute_id, "quantization_bits",
                            quantization_bits);
}

void ExpertEncoder::SetAttributeExplicitQuantization(int32_t attribute_id,
                                                     int quantization_bits,
                                                     int num_dims,
                                                     const float *origin,
                                                     float range) {
  options().SetAttributeInt(attribute_id, "quantization_bits",
                            quantization_bits);
  options().SetAttributeVector(attribute_id, "quantization_origin", num_dims,
                               origin);
  options().SetAttributeFloat(attribute_id, "quantization_range", range);
}

void ExpertEncoder::SetUseBuiltInAttributeCompression(bool enabled) {
  options().SetGlobalBool("use_built_in_attribute_compression", enabled);
}

void ExpertEncoder::SetEncodingMethod(int encoding_method) {
  Base::SetEncodingMethod(encoding_method);
}

void ExpertEncoder::SetEncodingSubmethod(int encoding_submethod) {
  Base::SetEncodingSubmethod(encoding_submethod);
}

Status ExpertEncoder::SetAttributePredictionScheme(
    int32_t attribute_id, int prediction_scheme_method) {
  auto att = point_cloud_->attribute(attribute_id);
  auto att_type = att->attribute_type();
  const Status status =
      CheckPredictionScheme(att_type, prediction_scheme_method);
  if (!status.ok()) {
    return status;
  }
  options().SetAttributeInt(attribute_id, "prediction_scheme",
                            prediction_scheme_method);
  return status;
}

#ifdef DRACO_TRANSCODER_SUPPORTED
Status ExpertEncoder::ApplyCompressionOptions(const PointCloud &pc) {
  if (!pc.IsCompressionEnabled()) {
    return OkStatus();
  }
  const auto &compression_options = pc.GetCompressionOptions();

  // Set any encoder options that haven't been explicitly set by users (don't
  // override existing options).
  if (!options().IsSpeedSet()) {
    options().SetSpeed(10 - compression_options.compression_level,
                       10 - compression_options.compression_level);
  }

  for (int ai = 0; ai < pc.num_attributes(); ++ai) {
    if (options().IsAttributeOptionSet(ai, "quantization_bits")) {
      continue;  // Don't override options that have been set.
    }
    int quantization_bits = 0;
    const auto type = pc.attribute(ai)->attribute_type();
    switch (type) {
      case GeometryAttribute::POSITION:
        if (compression_options.quantization_position
                .AreQuantizationBitsDefined()) {
          quantization_bits =
              compression_options.quantization_position.quantization_bits();
        } else {
          DRACO_RETURN_IF_ERROR(ApplyGridQuantization(pc, ai));
        }
        break;
      case GeometryAttribute::TEX_COORD:
        quantization_bits = compression_options.quantization_bits_tex_coord;
        break;
      case GeometryAttribute::NORMAL:
        quantization_bits = compression_options.quantization_bits_normal;
        break;
      case GeometryAttribute::COLOR:
        quantization_bits = compression_options.quantization_bits_color;
        break;
      case GeometryAttribute::TANGENT:
        quantization_bits = compression_options.quantization_bits_tangent;
        break;
      case GeometryAttribute::WEIGHTS:
        quantization_bits = compression_options.quantization_bits_weight;
        break;
      case GeometryAttribute::GENERIC:
        quantization_bits = compression_options.quantization_bits_generic;
        break;
      default:
        break;
    }
    if (quantization_bits > 0) {
      options().SetAttributeInt(ai, "quantization_bits", quantization_bits);
    }
  }
  return OkStatus();
}

Status ExpertEncoder::ApplyGridQuantization(const PointCloud &pc,
                                            int attribute_index) {
  const auto compression_options = pc.GetCompressionOptions();
  const float spacing = compression_options.quantization_position.spacing();
  return SetAttributeGridQuantization(pc, attribute_index, spacing);
}

Status ExpertEncoder::SetAttributeGridQuantization(const PointCloud &pc,
                                                   int attribute_index,
                                                   float spacing) {
  const auto *const att = pc.attribute(attribute_index);
  if (att->attribute_type() != GeometryAttribute::POSITION) {
    return ErrorStatus(
        "Invalid attribute type: Grid quantization is currently supported only "
        "for positions.");
  }
  if (att->num_components() != 3) {
    return ErrorStatus(
        "Invalid number of components: Grid quantization is currently "
        "supported only for 3D positions.");
  }
  // Compute quantization properties based on the grid spacing.
  const auto &bbox = pc.ComputeBoundingBox();
  // Snap min and max points of the |bbox| to the quantization grid vertices.
  Vector3f min_pos;
  int num_values = 0;  // Number of values that we need to encode.
  for (int c = 0; c < 3; ++c) {
    // Min / max position on grid vertices in grid coordinates.
    const float min_grid_pos = floor(bbox.GetMinPoint()[c] / spacing);
    const float max_grid_pos = ceil(bbox.GetMaxPoint()[c] / spacing);

    // Min pos on grid vertex in mesh coordinates.
    min_pos[c] = min_grid_pos * spacing;

    const float component_num_values =
        static_cast<int>(max_grid_pos) - static_cast<int>(min_grid_pos) + 1;
    if (component_num_values > num_values) {
      num_values = component_num_values;
    }
  }
  // Now compute the number of bits needed to encode |num_values|.
  int bits = MostSignificantBit(num_values);
  if ((1 << bits) < num_values) {
    // If the |num_values| is larger than number of values representable by
    // |bits|, we need to use one more bit. This will be almost always true
    // unless |num_values| was equal to 1 << |bits|.
    bits++;
  }
  // Compute the range in mesh coordinates that matches the quantization bits.
  // Note there are n-1 intervals between the |n| quantization values.
  const float range = ((1 << bits) - 1) * spacing;
  SetAttributeExplicitQuantization(attribute_index, bits, 3, min_pos.data(),
                                   range);
  return OkStatus();
}
#endif  // DRACO_TRANSCODER_SUPPORTED

}  // namespace draco
