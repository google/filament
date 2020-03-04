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
#include "draco/compression/attributes/sequential_quantization_attribute_decoder.h"

#include "draco/attributes/attribute_quantization_transform.h"
#include "draco/core/quantization_utils.h"

namespace draco {

SequentialQuantizationAttributeDecoder::SequentialQuantizationAttributeDecoder()
    : quantization_bits_(-1), max_value_dif_(0.f) {}

bool SequentialQuantizationAttributeDecoder::Init(PointCloudDecoder *decoder,
                                                  int attribute_id) {
  if (!SequentialIntegerAttributeDecoder::Init(decoder, attribute_id)) {
    return false;
  }
  const PointAttribute *const attribute =
      decoder->point_cloud()->attribute(attribute_id);
  // Currently we can quantize only floating point arguments.
  if (attribute->data_type() != DT_FLOAT32) {
    return false;
  }
  return true;
}

bool SequentialQuantizationAttributeDecoder::DecodeIntegerValues(
    const std::vector<PointIndex> &point_ids, DecoderBuffer *in_buffer) {
#ifdef DRACO_BACKWARDS_COMPATIBILITY_SUPPORTED
  if (decoder()->bitstream_version() < DRACO_BITSTREAM_VERSION(2, 0) &&
      !DecodeQuantizedDataInfo()) {
    return false;
  }
#endif
  return SequentialIntegerAttributeDecoder::DecodeIntegerValues(point_ids,
                                                                in_buffer);
}

bool SequentialQuantizationAttributeDecoder::
    DecodeDataNeededByPortableTransform(
        const std::vector<PointIndex> &point_ids, DecoderBuffer *in_buffer) {
  if (decoder()->bitstream_version() >= DRACO_BITSTREAM_VERSION(2, 0)) {
    // Decode quantization data here only for files with bitstream version 2.0+
    if (!DecodeQuantizedDataInfo()) {
      return false;
    }
  }

  // Store the decoded transform data in portable attribute;
  AttributeQuantizationTransform transform;
  transform.SetParameters(quantization_bits_, min_value_.get(),
                          attribute()->num_components(), max_value_dif_);
  return transform.TransferToAttribute(portable_attribute());
}

bool SequentialQuantizationAttributeDecoder::StoreValues(uint32_t num_values) {
  return DequantizeValues(num_values);
}

bool SequentialQuantizationAttributeDecoder::DecodeQuantizedDataInfo() {
  const int num_components = attribute()->num_components();
  min_value_ = std::unique_ptr<float[]>(new float[num_components]);
  if (!decoder()->buffer()->Decode(min_value_.get(),
                                   sizeof(float) * num_components)) {
    return false;
  }
  if (!decoder()->buffer()->Decode(&max_value_dif_)) {
    return false;
  }
  uint8_t quantization_bits;
  if (!decoder()->buffer()->Decode(&quantization_bits) ||
      quantization_bits > 31) {
    return false;
  }
  quantization_bits_ = quantization_bits;
  return true;
}

bool SequentialQuantizationAttributeDecoder::DequantizeValues(
    uint32_t num_values) {
  // Convert all quantized values back to floats.
  const int32_t max_quantized_value =
      (1u << static_cast<uint32_t>(quantization_bits_)) - 1;
  const int num_components = attribute()->num_components();
  const int entry_size = sizeof(float) * num_components;
  const std::unique_ptr<float[]> att_val(new float[num_components]);
  int quant_val_id = 0;
  int out_byte_pos = 0;
  Dequantizer dequantizer;
  if (!dequantizer.Init(max_value_dif_, max_quantized_value)) {
    return false;
  }
  const int32_t *const portable_attribute_data = GetPortableAttributeData();
  for (uint32_t i = 0; i < num_values; ++i) {
    for (int c = 0; c < num_components; ++c) {
      float value =
          dequantizer.DequantizeFloat(portable_attribute_data[quant_val_id++]);
      value = value + min_value_[c];
      att_val[c] = value;
    }
    // Store the floating point value into the attribute buffer.
    attribute()->buffer()->Write(out_byte_pos, att_val.get(), entry_size);
    out_byte_pos += entry_size;
  }
  return true;
}

}  // namespace draco
