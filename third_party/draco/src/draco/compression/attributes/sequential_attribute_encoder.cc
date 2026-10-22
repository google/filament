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
#include "draco/compression/attributes/sequential_attribute_encoder.h"

namespace draco {

SequentialAttributeEncoder::SequentialAttributeEncoder()
    : encoder_(nullptr),
      attribute_(nullptr),
      attribute_id_(-1),
      is_parent_encoder_(false) {}

bool SequentialAttributeEncoder::Init(PointCloudEncoder *encoder,
                                      int attribute_id) {
  encoder_ = encoder;
  attribute_ = encoder_->point_cloud()->attribute(attribute_id);
  attribute_id_ = attribute_id;
  return true;
}

bool SequentialAttributeEncoder::InitializeStandalone(
    PointAttribute *attribute) {
  attribute_ = attribute;
  attribute_id_ = -1;
  return true;
}

bool SequentialAttributeEncoder::TransformAttributeToPortableFormat(
    const std::vector<PointIndex> &point_ids) {
  // Default implementation doesn't transform the input data.
  return true;
}

bool SequentialAttributeEncoder::EncodePortableAttribute(
    const std::vector<PointIndex> &point_ids, EncoderBuffer *out_buffer) {
  // Lossless encoding of the input values.
  if (!EncodeValues(point_ids, out_buffer)) {
    return false;
  }
  return true;
}

bool SequentialAttributeEncoder::EncodeDataNeededByPortableTransform(
    EncoderBuffer *out_buffer) {
  // Default implementation doesn't transform the input data.
  return true;
}

bool SequentialAttributeEncoder::EncodeValues(
    const std::vector<PointIndex> &point_ids, EncoderBuffer *out_buffer) {
  const int entry_size = static_cast<int>(attribute_->byte_stride());
  const std::unique_ptr<uint8_t[]> value_data_ptr(new uint8_t[entry_size]);
  uint8_t *const value_data = value_data_ptr.get();
  // Encode all attribute values in their native raw format.
  for (uint32_t i = 0; i < point_ids.size(); ++i) {
    const AttributeValueIndex entry_id = attribute_->mapped_index(point_ids[i]);
    attribute_->GetValue(entry_id, value_data);
    out_buffer->Encode(value_data, entry_size);
  }
  return true;
}

void SequentialAttributeEncoder::MarkParentAttribute() {
  is_parent_encoder_ = true;
}

bool SequentialAttributeEncoder::InitPredictionScheme(
    PredictionSchemeInterface *ps) {
  for (int i = 0; i < ps->GetNumParentAttributes(); ++i) {
    const int att_id = encoder_->point_cloud()->GetNamedAttributeId(
        ps->GetParentAttributeType(i));
    if (att_id == -1) {
      return false;  // Requested attribute does not exist.
    }
    parent_attributes_.push_back(att_id);
    encoder_->MarkParentAttribute(att_id);
  }
  return true;
}

bool SequentialAttributeEncoder::SetPredictionSchemeParentAttributes(
    PredictionSchemeInterface *ps) {
  for (int i = 0; i < ps->GetNumParentAttributes(); ++i) {
    const int att_id = encoder_->point_cloud()->GetNamedAttributeId(
        ps->GetParentAttributeType(i));
    if (att_id == -1) {
      return false;  // Requested attribute does not exist.
    }
    if (!ps->SetParentAttribute(encoder_->GetPortableAttribute(att_id))) {
      return false;
    }
  }
  return true;
}

}  // namespace draco
