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
#include "draco/compression/attributes/sequential_normal_attribute_encoder.h"

#include "draco/compression/attributes/normal_compression_utils.h"

namespace draco {

bool SequentialNormalAttributeEncoder::Init(PointCloudEncoder *encoder,
                                            int attribute_id) {
  if (!SequentialIntegerAttributeEncoder::Init(encoder, attribute_id))
    return false;
  // Currently this encoder works only for 3-component normal vectors.
  if (attribute()->num_components() != 3) {
    return false;
  }

  // Initialize AttributeOctahedronTransform.
  const int quantization_bits = encoder->options()->GetAttributeInt(
      attribute_id, "quantization_bits", -1);
  if (quantization_bits < 1) {
    return false;
  }
  attribute_octahedron_transform_.SetParameters(quantization_bits);
  return true;
}

bool SequentialNormalAttributeEncoder::EncodeDataNeededByPortableTransform(
    EncoderBuffer *out_buffer) {
  return attribute_octahedron_transform_.EncodeParameters(out_buffer);
}

bool SequentialNormalAttributeEncoder::PrepareValues(
    const std::vector<PointIndex> &point_ids, int num_points) {
  auto portable_att = attribute_octahedron_transform_.InitTransformedAttribute(
      *(attribute()), point_ids.size());
  if (!attribute_octahedron_transform_.TransformAttribute(
          *(attribute()), point_ids, portable_att.get())) {
    return false;
  }
  SetPortableAttribute(std::move(portable_att));
  return true;
}

}  // namespace draco
