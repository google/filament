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

#include "draco/compression/mesh/mesh_edgebreaker_encoder.h"
#include "draco/compression/mesh/mesh_sequential_encoder.h"
#ifdef DRACO_POINT_CLOUD_COMPRESSION_SUPPORTED
#include "draco/compression/point_cloud/point_cloud_kd_tree_encoder.h"
#include "draco/compression/point_cloud/point_cloud_sequential_encoder.h"
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

}  // namespace draco
