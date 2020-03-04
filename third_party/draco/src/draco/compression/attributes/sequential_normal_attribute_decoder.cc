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
#include "draco/compression/attributes/sequential_normal_attribute_decoder.h"

#include "draco/attributes/attribute_octahedron_transform.h"
#include "draco/compression/attributes/normal_compression_utils.h"

namespace draco {

SequentialNormalAttributeDecoder::SequentialNormalAttributeDecoder()
    : quantization_bits_(-1) {}

bool SequentialNormalAttributeDecoder::Init(PointCloudDecoder *decoder,
                                            int attribute_id) {
  if (!SequentialIntegerAttributeDecoder::Init(decoder, attribute_id))
    return false;
  // Currently, this encoder works only for 3-component normal vectors.
  if (attribute()->num_components() != 3) {
    return false;
  }
  // Also the data type must be DT_FLOAT32.
  if (attribute()->data_type() != DT_FLOAT32) {
    return false;
  }
  return true;
}

bool SequentialNormalAttributeDecoder::DecodeIntegerValues(
    const std::vector<PointIndex> &point_ids, DecoderBuffer *in_buffer) {
#ifdef DRACO_BACKWARDS_COMPATIBILITY_SUPPORTED
  if (decoder()->bitstream_version() < DRACO_BITSTREAM_VERSION(2, 0)) {
    uint8_t quantization_bits;
    if (!in_buffer->Decode(&quantization_bits)) {
      return false;
    }
    quantization_bits_ = quantization_bits;
  }
#endif
  return SequentialIntegerAttributeDecoder::DecodeIntegerValues(point_ids,
                                                                in_buffer);
}

bool SequentialNormalAttributeDecoder::DecodeDataNeededByPortableTransform(
    const std::vector<PointIndex> &point_ids, DecoderBuffer *in_buffer) {
  if (decoder()->bitstream_version() >= DRACO_BITSTREAM_VERSION(2, 0)) {
    // For newer file version, decode attribute transform data here.
    uint8_t quantization_bits;
    if (!in_buffer->Decode(&quantization_bits)) {
      return false;
    }
    quantization_bits_ = quantization_bits;
  }

  // Store the decoded transform data in portable attribute.
  AttributeOctahedronTransform octahedral_transform;
  octahedral_transform.SetParameters(quantization_bits_);
  return octahedral_transform.TransferToAttribute(portable_attribute());
}

bool SequentialNormalAttributeDecoder::StoreValues(uint32_t num_points) {
  // Convert all quantized values back to floats.
  const int num_components = attribute()->num_components();
  const int entry_size = sizeof(float) * num_components;
  float att_val[3];
  int quant_val_id = 0;
  int out_byte_pos = 0;
  const int32_t *const portable_attribute_data = GetPortableAttributeData();
  OctahedronToolBox octahedron_tool_box;
  if (!octahedron_tool_box.SetQuantizationBits(quantization_bits_))
    return false;
  for (uint32_t i = 0; i < num_points; ++i) {
    const int32_t s = portable_attribute_data[quant_val_id++];
    const int32_t t = portable_attribute_data[quant_val_id++];
    octahedron_tool_box.QuantizedOctaherdalCoordsToUnitVector(s, t, att_val);
    // Store the decoded floating point value into the attribute buffer.
    attribute()->buffer()->Write(out_byte_pos, att_val, entry_size);
    out_byte_pos += entry_size;
  }
  return true;
}

}  // namespace draco
