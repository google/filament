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
#ifndef DRACO_COMPRESSION_POINT_CLOUD_POINT_CLOUD_ENCODER_H_
#define DRACO_COMPRESSION_POINT_CLOUD_POINT_CLOUD_ENCODER_H_

#include "draco/compression/attributes/attributes_encoder.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/compression/config/encoder_options.h"
#include "draco/core/encoder_buffer.h"
#include "draco/core/status.h"
#include "draco/point_cloud/point_cloud.h"

namespace draco {

// Abstract base class for all point cloud and mesh encoders. It provides a
// basic functionality that's shared between different encoders.
class PointCloudEncoder {
 public:
  PointCloudEncoder();
  virtual ~PointCloudEncoder() = default;

  // Sets the point cloud that is going be encoded. Must be called before the
  // Encode() method.
  void SetPointCloud(const PointCloud &pc);

  // The main entry point that encodes provided point cloud.
  Status Encode(const EncoderOptions &options, EncoderBuffer *out_buffer);

  virtual EncodedGeometryType GetGeometryType() const { return POINT_CLOUD; }

  // Returns the unique identifier of the encoding method (such as Edgebreaker
  // for mesh compression).
  virtual uint8_t GetEncodingMethod() const = 0;

  // Returns the number of points that were encoded during the last Encode()
  // function call. Valid only if "store_number_of_encoded_points" flag was set
  // in the provided EncoderOptions.
  size_t num_encoded_points() const { return num_encoded_points_; }

  int num_attributes_encoders() const {
    return static_cast<int>(attributes_encoders_.size());
  }
  AttributesEncoder *attributes_encoder(int i) {
    return attributes_encoders_[i].get();
  }

  // Adds a new attribute encoder, returning its id.
  int AddAttributesEncoder(std::unique_ptr<AttributesEncoder> att_enc) {
    attributes_encoders_.push_back(std::move(att_enc));
    return static_cast<int>(attributes_encoders_.size() - 1);
  }

  // Marks one attribute as a parent of another attribute. Must be called after
  // all attribute encoders are created (usually in the
  // AttributeEncoder::Init() method).
  bool MarkParentAttribute(int32_t parent_att_id);

  // Returns an attribute containing portable version of the attribute data that
  // is guaranteed to be encoded losslessly. This attribute can be used safely
  // as predictor for other attributes.
  const PointAttribute *GetPortableAttribute(int32_t point_attribute_id);

  EncoderBuffer *buffer() { return buffer_; }
  const EncoderOptions *options() const { return options_; }
  const PointCloud *point_cloud() const { return point_cloud_; }

 protected:
  // Can be implemented by derived classes to perform any custom initialization
  // of the encoder. Called in the Encode() method.
  virtual bool InitializeEncoder() { return true; }

  // Should be used to encode any encoder-specific data.
  virtual bool EncodeEncoderData() { return true; }

  // Encodes any global geometry data (such as the number of points).
  virtual Status EncodeGeometryData() { return OkStatus(); }

  // encode all attribute values. The attribute encoders are sorted to resolve
  // any attribute dependencies and all the encoded data is stored into the
  // |buffer_|.
  // Returns false if the encoding failed.
  virtual bool EncodePointAttributes();

  // Generate attribute encoders that are going to be used for encoding
  // point attribute data. Calls GenerateAttributesEncoder() for every attribute
  // of the encoded PointCloud.
  virtual bool GenerateAttributesEncoders();

  // Creates attribute encoder for a specific point attribute. This function
  // needs to be implemented by the derived classes. The derived classes need
  // to either 1. Create a new attribute encoder and add it using the
  // AddAttributeEncoder method, or 2. add the attribute to an existing
  // attribute encoder (using AttributesEncoder::AddAttributeId() method).
  virtual bool GenerateAttributesEncoder(int32_t att_id) = 0;

  // Encodes any data that is necessary to recreate a given attribute encoder.
  // Note: this is called in order in which the attribute encoders are going to
  // be encoded.
  virtual bool EncodeAttributesEncoderIdentifier(int32_t /* att_encoder_id */) {
    return true;
  }

  // Encodes all the attribute data using the created attribute encoders.
  virtual bool EncodeAllAttributes();

  // Computes and sets the num_encoded_points_ for the encoder.
  virtual void ComputeNumberOfEncodedPoints() = 0;

  void set_num_encoded_points(size_t num_points) {
    num_encoded_points_ = num_points;
  }

 private:
  // Encodes Draco header that is the same for all encoders.
  Status EncodeHeader();

  // Encode metadata.
  Status EncodeMetadata();

  // Rearranges attribute encoders and their attributes to reflect the
  // underlying attribute dependencies. This ensures that the attributes are
  // encoded in the correct order (parent attributes before their children).
  bool RearrangeAttributesEncoders();

  const PointCloud *point_cloud_;
  std::vector<std::unique_ptr<AttributesEncoder>> attributes_encoders_;

  // Map between attribute id and encoder id.
  std::vector<int32_t> attribute_to_encoder_map_;

  // Encoding order of individual attribute encoders (i.e., the order in which
  // they are processed during encoding that may be different from the order
  // in which they were created because of attribute dependencies.
  std::vector<int32_t> attributes_encoder_ids_order_;

  // This buffer holds the final encoded data.
  EncoderBuffer *buffer_;

  const EncoderOptions *options_;

  size_t num_encoded_points_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_POINT_CLOUD_POINT_CLOUD_ENCODER_H_
