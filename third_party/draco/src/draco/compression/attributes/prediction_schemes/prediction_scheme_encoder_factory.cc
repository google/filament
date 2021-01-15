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
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_encoder_factory.h"

namespace draco {

PredictionSchemeMethod SelectPredictionMethod(
    int att_id, const PointCloudEncoder *encoder) {
  if (encoder->options()->GetSpeed() >= 10) {
    // Selected fastest, though still doing some compression.
    return PREDICTION_DIFFERENCE;
  }
  if (encoder->GetGeometryType() == TRIANGULAR_MESH) {
    // Use speed setting to select the best encoding method.
    const PointAttribute *const att = encoder->point_cloud()->attribute(att_id);
    if (att->attribute_type() == GeometryAttribute::TEX_COORD) {
      if (encoder->options()->GetSpeed() < 4) {
        // Use texture coordinate prediction for speeds 0, 1, 2, 3.
        return MESH_PREDICTION_TEX_COORDS_PORTABLE;
      }
    }
    if (att->attribute_type() == GeometryAttribute::NORMAL) {
#ifdef DRACO_NORMAL_ENCODING_SUPPORTED
      if (encoder->options()->GetSpeed() < 4) {
        // Use geometric normal prediction for speeds 0, 1, 2, 3.
        // For this prediction, the position attribute needs to be either
        // integer or quantized as well.
        const int pos_att_id = encoder->point_cloud()->GetNamedAttributeId(
            GeometryAttribute::POSITION);
        const PointAttribute *const pos_att =
            encoder->point_cloud()->GetNamedAttribute(
                GeometryAttribute::POSITION);
        if (pos_att && (IsDataTypeIntegral(pos_att->data_type()) ||
                        encoder->options()->GetAttributeInt(
                            pos_att_id, "quantization_bits", -1) > 0)) {
          return MESH_PREDICTION_GEOMETRIC_NORMAL;
        }
      }
#endif
      return PREDICTION_DIFFERENCE;  // default
    }
    // Handle other attribute types.
    if (encoder->options()->GetSpeed() >= 8) {
      return PREDICTION_DIFFERENCE;
    }
    if (encoder->options()->GetSpeed() >= 2 ||
        encoder->point_cloud()->num_points() < 40) {
      // Parallelogram prediction is used for speeds 2 - 7 or when the overhead
      // of using constrained multi-parallelogram would be too high.
      return MESH_PREDICTION_PARALLELOGRAM;
    }
    // Multi-parallelogram is used for speeds 0, 1.
    return MESH_PREDICTION_CONSTRAINED_MULTI_PARALLELOGRAM;
  }
  // Default option is delta coding.
  return PREDICTION_DIFFERENCE;
}

// Returns the preferred prediction scheme based on the encoder options.
PredictionSchemeMethod GetPredictionMethodFromOptions(
    int att_id, const EncoderOptions &options) {
  const int pred_type =
      options.GetAttributeInt(att_id, "prediction_scheme", -1);
  if (pred_type == -1) {
    return PREDICTION_UNDEFINED;
  }
  if (pred_type < 0 || pred_type >= NUM_PREDICTION_SCHEMES) {
    return PREDICTION_NONE;
  }
  return static_cast<PredictionSchemeMethod>(pred_type);
}

}  // namespace draco
