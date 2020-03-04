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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_ATTRIBUTE_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_ATTRIBUTE_ENCODER_H_

#include "draco/compression/attributes/prediction_schemes/prediction_scheme_interface.h"
#include "draco/compression/point_cloud/point_cloud_encoder.h"

namespace draco {

// A base class for encoding attribute values of a single attribute using a
// given sequence of point ids. The default implementation encodes all attribute
// values directly to the buffer but derived classes can perform any custom
// encoding (such as quantization) by overriding the EncodeValues() method.
class SequentialAttributeEncoder {
 public:
  SequentialAttributeEncoder();
  virtual ~SequentialAttributeEncoder() = default;

  // Method that can be used for custom initialization of an attribute encoder,
  // such as creation of prediction schemes and initialization of attribute
  // encoder dependencies.
  // |encoder| is the parent PointCloudEncoder,
  // |attribute_id| is the id of the attribute that is being encoded by this
  // encoder.
  // This method is automatically called by the PointCloudEncoder after all
  // attribute encoders are created and it should not be called explicitly from
  // other places.
  virtual bool Init(PointCloudEncoder *encoder, int attribute_id);

  // Initialization for a specific attribute. This can be used mostly for
  // standalone encoding of an attribute without an PointCloudEncoder.
  virtual bool InitializeStandalone(PointAttribute *attribute);

  // Transforms attribute data into format that is going to be encoded
  // losslessly. The transform itself can be lossy.
  virtual bool TransformAttributeToPortableFormat(
      const std::vector<PointIndex> &point_ids);

  // Performs lossless encoding of the transformed attribute data.
  virtual bool EncodePortableAttribute(const std::vector<PointIndex> &point_ids,
                                       EncoderBuffer *out_buffer);

  // Encodes any data related to the portable attribute transform.
  virtual bool EncodeDataNeededByPortableTransform(EncoderBuffer *out_buffer);

  virtual bool IsLossyEncoder() const { return false; }

  int NumParentAttributes() const {
    return static_cast<int>(parent_attributes_.size());
  }
  int GetParentAttributeId(int i) const { return parent_attributes_[i]; }

  const PointAttribute *GetPortableAttribute() const {
    if (portable_attribute_ != nullptr) {
      return portable_attribute_.get();
    }
    return attribute();
  }

  // Called when this attribute encoder becomes a parent encoder of another
  // encoder.
  void MarkParentAttribute();

  virtual uint8_t GetUniqueId() const {
    return SEQUENTIAL_ATTRIBUTE_ENCODER_GENERIC;
  }

  const PointAttribute *attribute() const { return attribute_; }
  int attribute_id() const { return attribute_id_; }
  PointCloudEncoder *encoder() const { return encoder_; }

 protected:
  // Should be used to initialize newly created prediction scheme.
  // Returns false when the initialization failed (in which case the scheme
  // cannot be used).
  virtual bool InitPredictionScheme(PredictionSchemeInterface *ps);

  // Sets parent attributes for a given prediction scheme. Must be called
  // after all prediction schemes are initialized, but before the prediction
  // scheme is used.
  virtual bool SetPredictionSchemeParentAttributes(
      PredictionSchemeInterface *ps);

  // Encodes all attribute values in the specified order. Should be overridden
  // for specialized  encoders.
  virtual bool EncodeValues(const std::vector<PointIndex> &point_ids,
                            EncoderBuffer *out_buffer);

  bool is_parent_encoder() const { return is_parent_encoder_; }

  void SetPortableAttribute(std::unique_ptr<PointAttribute> att) {
    portable_attribute_ = std::move(att);
  }

  // Returns a mutable attribute that should be filled by derived encoders with
  // the transformed version of the attribute data. To get a public const
  // version, use the GetPortableAttribute() method.
  PointAttribute *portable_attribute() { return portable_attribute_.get(); }

 private:
  PointCloudEncoder *encoder_;
  const PointAttribute *attribute_;
  int attribute_id_;

  // List of attribute encoders that need to be encoded before this attribute.
  // E.g. The parent attributes may be used to predict values used by this
  // attribute encoder.
  std::vector<int32_t> parent_attributes_;

  bool is_parent_encoder_;

  // Attribute that stores transformed data from the source attribute after it
  // is processed through the ApplyTransform() method. Attribute data stored
  // within this attribute is guaranteed to be encoded losslessly and it can be
  // safely used for prediction of other attributes.
  std::unique_ptr<PointAttribute> portable_attribute_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_ATTRIBUTE_ENCODER_H_
