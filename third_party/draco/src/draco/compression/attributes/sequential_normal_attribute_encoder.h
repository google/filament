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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_NORMAL_ATTRIBUTE_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_NORMAL_ATTRIBUTE_ENCODER_H_

#include "draco/attributes/attribute_octahedron_transform.h"
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_encoder_factory.h"
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_canonicalized_encoding_transform.h"
#include "draco/compression/attributes/sequential_integer_attribute_encoder.h"
#include "draco/compression/config/compression_shared.h"

namespace draco {

// Class for encoding normal vectors using an octahedral encoding, see Cigolle
// et al.'14 “A Survey of Efficient Representations for Independent Unit
// Vectors”. Compared to the basic quantization encoder, this encoder results
// in a better compression rate under the same accuracy settings. Note that this
// encoder doesn't preserve the lengths of input vectors, therefore it will not
// work correctly when the input values are not normalized.
class SequentialNormalAttributeEncoder
    : public SequentialIntegerAttributeEncoder {
 public:
  uint8_t GetUniqueId() const override {
    return SEQUENTIAL_ATTRIBUTE_ENCODER_NORMALS;
  }
  bool IsLossyEncoder() const override { return true; }

  bool EncodeDataNeededByPortableTransform(EncoderBuffer *out_buffer) override;

 protected:
  bool Init(PointCloudEncoder *encoder, int attribute_id) override;

  // Put quantized values in portable attribute for sequential encoding.
  bool PrepareValues(const std::vector<PointIndex> &point_ids,
                     int num_points) override;

  std::unique_ptr<PredictionSchemeTypedEncoderInterface<int32_t>>
  CreateIntPredictionScheme(PredictionSchemeMethod /* method */) override {
    typedef PredictionSchemeNormalOctahedronCanonicalizedEncodingTransform<
        int32_t>
        Transform;
    const int32_t quantization_bits = encoder()->options()->GetAttributeInt(
        attribute_id(), "quantization_bits", -1);
    const int32_t max_value = (1 << quantization_bits) - 1;
    const Transform transform(max_value);
    const PredictionSchemeMethod default_prediction_method =
        SelectPredictionMethod(attribute_id(), encoder());
    const int32_t prediction_method = encoder()->options()->GetAttributeInt(
        attribute_id(), "prediction_scheme", default_prediction_method);

    if (prediction_method == MESH_PREDICTION_GEOMETRIC_NORMAL) {
      return CreatePredictionSchemeForEncoder<int32_t, Transform>(
          MESH_PREDICTION_GEOMETRIC_NORMAL, attribute_id(), encoder(),
          transform);
    }
    if (prediction_method == PREDICTION_DIFFERENCE) {
      return CreatePredictionSchemeForEncoder<int32_t, Transform>(
          PREDICTION_DIFFERENCE, attribute_id(), encoder(), transform);
    }
    DRACO_DCHECK(false);  // Should never be reached.
    return nullptr;
  }

  // Used for the conversion to quantized normals in octahedral format.
  AttributeOctahedronTransform attribute_octahedron_transform_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_NORMAL_ATTRIBUTE_ENCODER_H_
