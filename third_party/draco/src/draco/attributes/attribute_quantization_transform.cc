
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
#include "draco/attributes/attribute_quantization_transform.h"

#include "draco/attributes/attribute_transform_type.h"
#include "draco/core/quantization_utils.h"

namespace draco {

bool AttributeQuantizationTransform::InitFromAttribute(
    const PointAttribute &attribute) {
  const AttributeTransformData *const transform_data =
      attribute.GetAttributeTransformData();
  if (!transform_data ||
      transform_data->transform_type() != ATTRIBUTE_QUANTIZATION_TRANSFORM) {
    return false;  // Wrong transform type.
  }
  int32_t byte_offset = 0;
  quantization_bits_ = transform_data->GetParameterValue<int32_t>(byte_offset);
  byte_offset += 4;
  min_values_.resize(attribute.num_components());
  for (int i = 0; i < attribute.num_components(); ++i) {
    min_values_[i] = transform_data->GetParameterValue<float>(byte_offset);
    byte_offset += 4;
  }
  range_ = transform_data->GetParameterValue<float>(byte_offset);
  return true;
}

// Copy parameter values into the provided AttributeTransformData instance.
void AttributeQuantizationTransform::CopyToAttributeTransformData(
    AttributeTransformData *out_data) const {
  out_data->set_transform_type(ATTRIBUTE_QUANTIZATION_TRANSFORM);
  out_data->AppendParameterValue(quantization_bits_);
  for (int i = 0; i < min_values_.size(); ++i) {
    out_data->AppendParameterValue(min_values_[i]);
  }
  out_data->AppendParameterValue(range_);
}

void AttributeQuantizationTransform::SetParameters(int quantization_bits,
                                                   const float *min_values,
                                                   int num_components,
                                                   float range) {
  quantization_bits_ = quantization_bits;
  min_values_.assign(min_values, min_values + num_components);
  range_ = range;
}

bool AttributeQuantizationTransform::ComputeParameters(
    const PointAttribute &attribute, const int quantization_bits) {
  if (quantization_bits_ != -1) {
    return false;  // already initialized.
  }
  quantization_bits_ = quantization_bits;

  const int num_components = attribute.num_components();
  range_ = 0.f;
  min_values_ = std::vector<float>(num_components, 0.f);
  const std::unique_ptr<float[]> max_values(new float[num_components]);
  const std::unique_ptr<float[]> att_val(new float[num_components]);
  // Compute minimum values and max value difference.
  attribute.GetValue(AttributeValueIndex(0), att_val.get());
  attribute.GetValue(AttributeValueIndex(0), min_values_.data());
  attribute.GetValue(AttributeValueIndex(0), max_values.get());

  for (AttributeValueIndex i(1); i < static_cast<uint32_t>(attribute.size());
       ++i) {
    attribute.GetValue(i, att_val.get());
    for (int c = 0; c < num_components; ++c) {
      if (min_values_[c] > att_val[c]) {
        min_values_[c] = att_val[c];
      }
      if (max_values[c] < att_val[c]) {
        max_values[c] = att_val[c];
      }
    }
  }
  for (int c = 0; c < num_components; ++c) {
    if (std::isnan(min_values_[c]) || std::isinf(min_values_[c]) ||
        std::isnan(max_values[c]) || std::isinf(max_values[c])) {
      return false;
    }
    const float dif = max_values[c] - min_values_[c];
    if (dif > range_) {
      range_ = dif;
    }
  }

  // In case all values are the same, initialize the range to unit length. This
  // will ensure that all values are quantized properly to the same value.
  if (range_ == 0.f) {
    range_ = 1.f;
  }

  return true;
}

bool AttributeQuantizationTransform::EncodeParameters(
    EncoderBuffer *encoder_buffer) const {
  if (is_initialized()) {
    encoder_buffer->Encode(min_values_.data(),
                           sizeof(float) * min_values_.size());
    encoder_buffer->Encode(range_);
    encoder_buffer->Encode(static_cast<uint8_t>(quantization_bits_));
    return true;
  }
  return false;
}

std::unique_ptr<PointAttribute>
AttributeQuantizationTransform::GeneratePortableAttribute(
    const PointAttribute &attribute, int num_points) const {
  DRACO_DCHECK(is_initialized());

  // Allocate portable attribute.
  const int num_entries = num_points;
  const int num_components = attribute.num_components();
  std::unique_ptr<PointAttribute> portable_attribute =
      InitPortableAttribute(num_entries, num_components, 0, attribute, true);

  // Quantize all values using the order given by point_ids.
  int32_t *const portable_attribute_data = reinterpret_cast<int32_t *>(
      portable_attribute->GetAddress(AttributeValueIndex(0)));
  const uint32_t max_quantized_value = (1 << (quantization_bits_)) - 1;
  Quantizer quantizer;
  quantizer.Init(range(), max_quantized_value);
  int32_t dst_index = 0;
  const std::unique_ptr<float[]> att_val(new float[num_components]);
  for (PointIndex i(0); i < num_points; ++i) {
    const AttributeValueIndex att_val_id = attribute.mapped_index(i);
    attribute.GetValue(att_val_id, att_val.get());
    for (int c = 0; c < num_components; ++c) {
      const float value = (att_val[c] - min_values()[c]);
      const int32_t q_val = quantizer.QuantizeFloat(value);
      portable_attribute_data[dst_index++] = q_val;
    }
  }
  return portable_attribute;
}

std::unique_ptr<PointAttribute>
AttributeQuantizationTransform::GeneratePortableAttribute(
    const PointAttribute &attribute, const std::vector<PointIndex> &point_ids,
    int num_points) const {
  DRACO_DCHECK(is_initialized());

  // Allocate portable attribute.
  const int num_entries = static_cast<int>(point_ids.size());
  const int num_components = attribute.num_components();
  std::unique_ptr<PointAttribute> portable_attribute = InitPortableAttribute(
      num_entries, num_components, num_points, attribute, true);

  // Quantize all values using the order given by point_ids.
  int32_t *const portable_attribute_data = reinterpret_cast<int32_t *>(
      portable_attribute->GetAddress(AttributeValueIndex(0)));
  const uint32_t max_quantized_value = (1 << (quantization_bits_)) - 1;
  Quantizer quantizer;
  quantizer.Init(range(), max_quantized_value);
  int32_t dst_index = 0;
  const std::unique_ptr<float[]> att_val(new float[num_components]);
  for (uint32_t i = 0; i < point_ids.size(); ++i) {
    const AttributeValueIndex att_val_id = attribute.mapped_index(point_ids[i]);
    attribute.GetValue(att_val_id, att_val.get());
    for (int c = 0; c < num_components; ++c) {
      const float value = (att_val[c] - min_values()[c]);
      const int32_t q_val = quantizer.QuantizeFloat(value);
      portable_attribute_data[dst_index++] = q_val;
    }
  }
  return portable_attribute;
}

}  // namespace draco
