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
#include "draco/compression/attributes/kd_tree_attributes_encoder.h"

#include "draco/compression/attributes/kd_tree_attributes_shared.h"
#include "draco/compression/attributes/point_d_vector.h"
#include "draco/compression/point_cloud/algorithms/dynamic_integer_points_kd_tree_encoder.h"
#include "draco/compression/point_cloud/algorithms/float_points_tree_encoder.h"
#include "draco/compression/point_cloud/point_cloud_encoder.h"
#include "draco/core/varint_encoding.h"

namespace draco {

KdTreeAttributesEncoder::KdTreeAttributesEncoder() : num_components_(0) {}

KdTreeAttributesEncoder::KdTreeAttributesEncoder(int att_id)
    : AttributesEncoder(att_id), num_components_(0) {}

bool KdTreeAttributesEncoder::TransformAttributesToPortableFormat() {
  // Convert any of the input attributes into a format that can be processed by
  // the kd tree encoder (quantization of floating attributes for now).
  const size_t num_points = encoder()->point_cloud()->num_points();
  int num_components = 0;
  for (uint32_t i = 0; i < num_attributes(); ++i) {
    const int att_id = GetAttributeId(i);
    const PointAttribute *const att =
        encoder()->point_cloud()->attribute(att_id);
    num_components += att->num_components();
  }
  num_components_ = num_components;

  // Go over all attributes and quantize them if needed.
  for (uint32_t i = 0; i < num_attributes(); ++i) {
    const int att_id = GetAttributeId(i);
    const PointAttribute *const att =
        encoder()->point_cloud()->attribute(att_id);
    if (att->data_type() == DT_FLOAT32) {
      // Quantization path.
      AttributeQuantizationTransform attribute_quantization_transform;
      const int quantization_bits = encoder()->options()->GetAttributeInt(
          att_id, "quantization_bits", -1);
      if (quantization_bits < 1) {
        return false;
      }
      if (encoder()->options()->IsAttributeOptionSet(att_id,
                                                     "quantization_origin") &&
          encoder()->options()->IsAttributeOptionSet(att_id,
                                                     "quantization_range")) {
        // Quantization settings are explicitly specified in the provided
        // options.
        std::vector<float> quantization_origin(att->num_components());
        encoder()->options()->GetAttributeVector(att_id, "quantization_origin",
                                                 att->num_components(),
                                                 &quantization_origin[0]);
        const float range = encoder()->options()->GetAttributeFloat(
            att_id, "quantization_range", 1.f);
        attribute_quantization_transform.SetParameters(
            quantization_bits, quantization_origin.data(),
            att->num_components(), range);
      } else {
        // Compute quantization settings from the attribute values.
        if (!attribute_quantization_transform.ComputeParameters(
                *att, quantization_bits)) {
          return false;
        }
      }
      attribute_quantization_transforms_.push_back(
          attribute_quantization_transform);
      // Store the quantized attribute in an array that will be used when we do
      // the actual encoding of the data.
      auto portable_att =
          attribute_quantization_transform.InitTransformedAttribute(*att,
                                                                    num_points);
      attribute_quantization_transform.TransformAttribute(*att, {},
                                                          portable_att.get());
      quantized_portable_attributes_.push_back(std::move(portable_att));
    } else if (att->data_type() == DT_INT32 || att->data_type() == DT_INT16 ||
               att->data_type() == DT_INT8) {
      // For signed types, find the minimum value for each component. These
      // values are going to be used to transform the attribute values to
      // unsigned integers that can be processed by the core kd tree algorithm.
      std::vector<int32_t> min_value(att->num_components(),
                                     std::numeric_limits<int32_t>::max());
      std::vector<int32_t> act_value(att->num_components());
      for (AttributeValueIndex avi(0); avi < static_cast<uint32_t>(att->size());
           ++avi) {
        att->ConvertValue<int32_t>(avi, &act_value[0]);
        for (int c = 0; c < att->num_components(); ++c) {
          if (min_value[c] > act_value[c]) {
            min_value[c] = act_value[c];
          }
        }
      }
      for (int c = 0; c < att->num_components(); ++c) {
        min_signed_values_.push_back(min_value[c]);
      }
    }
  }
  return true;
}

bool KdTreeAttributesEncoder::EncodeDataNeededByPortableTransforms(
    EncoderBuffer *out_buffer) {
  // Store quantization settings for all attributes that need it.
  for (int i = 0; i < attribute_quantization_transforms_.size(); ++i) {
    attribute_quantization_transforms_[i].EncodeParameters(out_buffer);
  }

  // Encode data needed for transforming signed integers to unsigned ones.
  for (int i = 0; i < min_signed_values_.size(); ++i) {
    EncodeVarint<int32_t>(min_signed_values_[i], out_buffer);
  }
  return true;
}

bool KdTreeAttributesEncoder::EncodePortableAttributes(
    EncoderBuffer *out_buffer) {
  // Encode the data using the kd tree encoder algorithm. The data is first
  // copied to a PointDVector that provides all the API expected by the core
  // encoding algorithm.

  // We limit the maximum value of compression_level to 6 as we don't currently
  // have viable algorithms for higher compression levels.
  uint8_t compression_level =
      std::min(10 - encoder()->options()->GetSpeed(), 6);
  DRACO_DCHECK_LE(compression_level, 6);

  if (compression_level == 6 && num_components_ > 15) {
    // Don't use compression level for CL >= 6. Axis selection is currently
    // encoded using 4 bits.
    compression_level = 5;
  }

  out_buffer->Encode(compression_level);

  // Init PointDVector. The number of dimensions is equal to the total number
  // of dimensions across all attributes.
  const int num_points = encoder()->point_cloud()->num_points();
  PointDVector<uint32_t> point_vector(num_points, num_components_);

  int num_processed_components = 0;
  int num_processed_quantized_attributes = 0;
  int num_processed_signed_components = 0;
  // Copy data to the point vector.
  for (uint32_t i = 0; i < num_attributes(); ++i) {
    const int att_id = GetAttributeId(i);
    const PointAttribute *const att =
        encoder()->point_cloud()->attribute(att_id);
    const PointAttribute *source_att = nullptr;
    if (att->data_type() == DT_UINT32 || att->data_type() == DT_UINT16 ||
        att->data_type() == DT_UINT8 || att->data_type() == DT_INT32 ||
        att->data_type() == DT_INT16 || att->data_type() == DT_INT8) {
      // Use the original attribute.
      source_att = att;
    } else if (att->data_type() == DT_FLOAT32) {
      // Use the portable (quantized) attribute instead.
      source_att =
          quantized_portable_attributes_[num_processed_quantized_attributes]
              .get();
      num_processed_quantized_attributes++;
    } else {
      // Unsupported data type.
      return false;
    }

    if (source_att == nullptr) {
      return false;
    }

    // Copy source_att to the vector.
    if (source_att->data_type() == DT_UINT32) {
      // If the data type is the same as the one used by the point vector, we
      // can directly copy individual elements.
      for (PointIndex pi(0); pi < num_points; ++pi) {
        const AttributeValueIndex avi = source_att->mapped_index(pi);
        const uint8_t *const att_value_address = source_att->GetAddress(avi);
        point_vector.CopyAttribute(source_att->num_components(),
                                   num_processed_components, pi.value(),
                                   att_value_address);
      }
    } else if (source_att->data_type() == DT_INT32 ||
               source_att->data_type() == DT_INT16 ||
               source_att->data_type() == DT_INT8) {
      // Signed values need to be converted to unsigned before they are stored
      // in the point vector.
      std::vector<int32_t> signed_point(source_att->num_components());
      std::vector<uint32_t> unsigned_point(source_att->num_components());
      for (PointIndex pi(0); pi < num_points; ++pi) {
        const AttributeValueIndex avi = source_att->mapped_index(pi);
        source_att->ConvertValue<int32_t>(avi, &signed_point[0]);
        for (int c = 0; c < source_att->num_components(); ++c) {
          unsigned_point[c] =
              signed_point[c] -
              min_signed_values_[num_processed_signed_components + c];
        }

        point_vector.CopyAttribute(source_att->num_components(),
                                   num_processed_components, pi.value(),
                                   &unsigned_point[0]);
      }
      num_processed_signed_components += source_att->num_components();
    } else {
      // If the data type of the attribute is different, we have to convert the
      // value before we put it to the point vector.
      std::vector<uint32_t> point(source_att->num_components());
      for (PointIndex pi(0); pi < num_points; ++pi) {
        const AttributeValueIndex avi = source_att->mapped_index(pi);
        source_att->ConvertValue<uint32_t>(avi, &point[0]);
        point_vector.CopyAttribute(source_att->num_components(),
                                   num_processed_components, pi.value(),
                                   point.data());
      }
    }
    num_processed_components += source_att->num_components();
  }

  // Compute the maximum bit length needed for the kd tree encoding.
  int num_bits = 0;
  const uint32_t *data = point_vector[0];
  for (int i = 0; i < num_points * num_components_; ++i) {
    if (data[i] > 0) {
      const int msb = MostSignificantBit(data[i]) + 1;
      if (msb > num_bits) {
        num_bits = msb;
      }
    }
  }

  switch (compression_level) {
    case 6: {
      DynamicIntegerPointsKdTreeEncoder<6> points_encoder(num_components_);
      if (!points_encoder.EncodePoints(point_vector.begin(), point_vector.end(),
                                       num_bits, out_buffer)) {
        return false;
      }
      break;
    }
    case 5: {
      DynamicIntegerPointsKdTreeEncoder<5> points_encoder(num_components_);
      if (!points_encoder.EncodePoints(point_vector.begin(), point_vector.end(),
                                       num_bits, out_buffer)) {
        return false;
      }
      break;
    }
    case 4: {
      DynamicIntegerPointsKdTreeEncoder<4> points_encoder(num_components_);
      if (!points_encoder.EncodePoints(point_vector.begin(), point_vector.end(),
                                       num_bits, out_buffer)) {
        return false;
      }
      break;
    }
    case 3: {
      DynamicIntegerPointsKdTreeEncoder<3> points_encoder(num_components_);
      if (!points_encoder.EncodePoints(point_vector.begin(), point_vector.end(),
                                       num_bits, out_buffer)) {
        return false;
      }
      break;
    }
    case 2: {
      DynamicIntegerPointsKdTreeEncoder<2> points_encoder(num_components_);
      if (!points_encoder.EncodePoints(point_vector.begin(), point_vector.end(),
                                       num_bits, out_buffer)) {
        return false;
      }
      break;
    }
    case 1: {
      DynamicIntegerPointsKdTreeEncoder<1> points_encoder(num_components_);
      if (!points_encoder.EncodePoints(point_vector.begin(), point_vector.end(),
                                       num_bits, out_buffer)) {
        return false;
      }
      break;
    }
    case 0: {
      DynamicIntegerPointsKdTreeEncoder<0> points_encoder(num_components_);
      if (!points_encoder.EncodePoints(point_vector.begin(), point_vector.end(),
                                       num_bits, out_buffer)) {
        return false;
      }
      break;
    }
    // Compression level and/or encoding speed seem wrong.
    default:
      return false;
  }
  return true;
}

}  // namespace draco
