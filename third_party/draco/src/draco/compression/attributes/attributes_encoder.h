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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_ATTRIBUTES_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_ATTRIBUTES_ENCODER_H_

#include "draco/attributes/point_attribute.h"
#include "draco/core/encoder_buffer.h"
#include "draco/point_cloud/point_cloud.h"

namespace draco {

class PointCloudEncoder;

// Base class for encoding one or more attributes of a PointCloud (or other
// geometry). This base class provides only the basic interface that is used
// by the PointCloudEncoder.
class AttributesEncoder {
 public:
  AttributesEncoder();
  // Constructs an attribute encoder associated with a given point attribute.
  explicit AttributesEncoder(int point_attrib_id);
  virtual ~AttributesEncoder() = default;

  // Called after all attribute encoders are created. It can be used to perform
  // any custom initialization, including setting up attribute dependencies.
  // Note: no data should be encoded in this function, because the decoder may
  // process encoders in a different order from the decoder.
  virtual bool Init(PointCloudEncoder *encoder, const PointCloud *pc);

  // Encodes data needed by the target attribute decoder.
  virtual bool EncodeAttributesEncoderData(EncoderBuffer *out_buffer);

  // Returns a unique identifier of the given encoder type, that is used during
  // decoding to construct the corresponding attribute decoder.
  virtual uint8_t GetUniqueId() const = 0;

  // Encode attribute data to the target buffer.
  virtual bool EncodeAttributes(EncoderBuffer *out_buffer) {
    if (!TransformAttributesToPortableFormat()) {
      return false;
    }
    if (!EncodePortableAttributes(out_buffer)) {
      return false;
    }
    // Encode data needed by portable transforms after the attribute is encoded.
    // This corresponds to the order in which the data is going to be decoded by
    // the decoder.
    if (!EncodeDataNeededByPortableTransforms(out_buffer)) {
      return false;
    }
    return true;
  }

  // Returns the number of attributes that need to be encoded before the
  // specified attribute is encoded.
  // Note that the attribute is specified by its point attribute id.
  virtual int NumParentAttributes(int32_t /* point_attribute_id */) const {
    return 0;
  }

  virtual int GetParentAttributeId(int32_t /* point_attribute_id */,
                                   int32_t /* parent_i */) const {
    return -1;
  }

  // Marks a given attribute as a parent of another attribute.
  virtual bool MarkParentAttribute(int32_t /* point_attribute_id */) {
    return false;
  }

  // Returns an attribute containing data processed by the attribute transform.
  // (see TransformToPortableFormat() method). This data is guaranteed to be
  // encoded losslessly and it can be safely used for predictors.
  virtual const PointAttribute *GetPortableAttribute(
      int32_t /* point_attribute_id */) {
    return nullptr;
  }

  void AddAttributeId(int32_t id) {
    point_attribute_ids_.push_back(id);
    if (id >= static_cast<int32_t>(point_attribute_to_local_id_map_.size())) {
      point_attribute_to_local_id_map_.resize(id + 1, -1);
    }
    point_attribute_to_local_id_map_[id] =
        static_cast<int32_t>(point_attribute_ids_.size()) - 1;
  }

  // Sets new attribute point ids (replacing the existing ones).
  void SetAttributeIds(const std::vector<int32_t> &point_attribute_ids) {
    point_attribute_ids_.clear();
    point_attribute_to_local_id_map_.clear();
    for (int32_t att_id : point_attribute_ids) {
      AddAttributeId(att_id);
    }
  }

  int32_t GetAttributeId(int i) const { return point_attribute_ids_[i]; }
  uint32_t num_attributes() const {
    return static_cast<uint32_t>(point_attribute_ids_.size());
  }
  PointCloudEncoder *encoder() const { return point_cloud_encoder_; }

 protected:
  // Transforms the input attribute data into a form that should be losslessly
  // encoded (transform itself can be lossy).
  virtual bool TransformAttributesToPortableFormat() { return true; }

  // Losslessly encodes data of all portable attributes.
  // Precondition: All attributes must have been transformed into portable
  // format at this point (see TransformAttributesToPortableFormat() method).
  virtual bool EncodePortableAttributes(EncoderBuffer *out_buffer) = 0;

  // Encodes any data needed to revert the transform to portable format for each
  // attribute (e.g. data needed for dequantization of quantized values).
  virtual bool EncodeDataNeededByPortableTransforms(EncoderBuffer *out_buffer) {
    return true;
  }

  int32_t GetLocalIdForPointAttribute(int32_t point_attribute_id) const {
    const int id_map_size =
        static_cast<int>(point_attribute_to_local_id_map_.size());
    if (point_attribute_id >= id_map_size) {
      return -1;
    }
    return point_attribute_to_local_id_map_[point_attribute_id];
  }

 private:
  // List of attribute ids that need to be encoded with this encoder.
  std::vector<int32_t> point_attribute_ids_;

  // Map between point attribute id and the local id (i.e., the inverse of the
  // |point_attribute_ids_|.
  std::vector<int32_t> point_attribute_to_local_id_map_;

  PointCloudEncoder *point_cloud_encoder_;
  const PointCloud *point_cloud_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_ATTRIBUTES_ENCODER_H_
