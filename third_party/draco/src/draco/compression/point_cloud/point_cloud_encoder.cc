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
#include "draco/compression/point_cloud/point_cloud_encoder.h"

#include "draco/metadata/metadata_encoder.h"

namespace draco {

PointCloudEncoder::PointCloudEncoder()
    : point_cloud_(nullptr), buffer_(nullptr), num_encoded_points_(0) {}

void PointCloudEncoder::SetPointCloud(const PointCloud &pc) {
  point_cloud_ = &pc;
}

Status PointCloudEncoder::Encode(const EncoderOptions &options,
                                 EncoderBuffer *out_buffer) {
  options_ = &options;
  buffer_ = out_buffer;

  // Cleanup from previous runs.
  attributes_encoders_.clear();
  attribute_to_encoder_map_.clear();
  attributes_encoder_ids_order_.clear();

  if (!point_cloud_) {
    return Status(Status::DRACO_ERROR, "Invalid input geometry.");
  }
  DRACO_RETURN_IF_ERROR(EncodeHeader())
  DRACO_RETURN_IF_ERROR(EncodeMetadata())
  if (!InitializeEncoder()) {
    return Status(Status::DRACO_ERROR, "Failed to initialize encoder.");
  }
  if (!EncodeEncoderData()) {
    return Status(Status::DRACO_ERROR, "Failed to encode internal data.");
  }
  DRACO_RETURN_IF_ERROR(EncodeGeometryData());
  if (!EncodePointAttributes()) {
    return Status(Status::DRACO_ERROR, "Failed to encode point attributes.");
  }
  if (options.GetGlobalBool("store_number_of_encoded_points", false)) {
    ComputeNumberOfEncodedPoints();
  }
  return OkStatus();
}

Status PointCloudEncoder::EncodeHeader() {
  // Encode the header according to our v1 specification.
  // Five bytes for Draco format.
  buffer_->Encode("DRACO", 5);
  // Version (major, minor).
  const uint8_t encoder_type = GetGeometryType();
  uint8_t version_major, version_minor;
  version_major = encoder_type == POINT_CLOUD
                      ? kDracoPointCloudBitstreamVersionMajor
                      : kDracoMeshBitstreamVersionMajor;
  version_minor = encoder_type == POINT_CLOUD
                      ? kDracoPointCloudBitstreamVersionMinor
                      : kDracoMeshBitstreamVersionMinor;

  buffer_->Encode(version_major);
  buffer_->Encode(version_minor);
  // Type of the encoder (point cloud, mesh, ...).
  buffer_->Encode(encoder_type);
  // Unique identifier for the selected encoding method (edgebreaker, etc...).
  buffer_->Encode(GetEncodingMethod());
  // Reserved for flags.
  uint16_t flags = 0;
  // First bit of |flags| is reserved for metadata.
  if (point_cloud_->GetMetadata()) {
    flags |= METADATA_FLAG_MASK;
  }
  buffer_->Encode(flags);
  return OkStatus();
}

Status PointCloudEncoder::EncodeMetadata() {
  if (!point_cloud_->GetMetadata()) {
    return OkStatus();
  }
  MetadataEncoder metadata_encoder;
  if (!metadata_encoder.EncodeGeometryMetadata(buffer_,
                                               point_cloud_->GetMetadata())) {
    return Status(Status::DRACO_ERROR, "Failed to encode metadata.");
  }
  return OkStatus();
}

bool PointCloudEncoder::EncodePointAttributes() {
  if (!GenerateAttributesEncoders()) {
    return false;
  }

  // Encode the number of attribute encoders.
  buffer_->Encode(static_cast<uint8_t>(attributes_encoders_.size()));

  // Initialize all the encoders (this is used for example to init attribute
  // dependencies, no data is encoded in this step).
  for (auto &att_enc : attributes_encoders_) {
    if (!att_enc->Init(this, point_cloud_)) {
      return false;
    }
  }

  // Rearrange attributes to respect dependencies between individual attributes.
  if (!RearrangeAttributesEncoders()) {
    return false;
  }

  // Encode any data that is necessary to create the corresponding attribute
  // decoder.
  for (int att_encoder_id : attributes_encoder_ids_order_) {
    if (!EncodeAttributesEncoderIdentifier(att_encoder_id)) {
      return false;
    }
  }

  // Also encode any attribute encoder data (such as the info about encoded
  // attributes).
  for (int att_encoder_id : attributes_encoder_ids_order_) {
    if (!attributes_encoders_[att_encoder_id]->EncodeAttributesEncoderData(
            buffer_)) {
      return false;
    }
  }

  // Lastly encode all the attributes using the provided attribute encoders.
  if (!EncodeAllAttributes()) {
    return false;
  }
  return true;
}

bool PointCloudEncoder::GenerateAttributesEncoders() {
  for (int i = 0; i < point_cloud_->num_attributes(); ++i) {
    if (!GenerateAttributesEncoder(i)) {
      return false;
    }
  }
  attribute_to_encoder_map_.resize(point_cloud_->num_attributes());
  for (uint32_t i = 0; i < attributes_encoders_.size(); ++i) {
    for (uint32_t j = 0; j < attributes_encoders_[i]->num_attributes(); ++j) {
      attribute_to_encoder_map_[attributes_encoders_[i]->GetAttributeId(j)] = i;
    }
  }
  return true;
}

bool PointCloudEncoder::EncodeAllAttributes() {
  for (int att_encoder_id : attributes_encoder_ids_order_) {
    if (!attributes_encoders_[att_encoder_id]->EncodeAttributes(buffer_)) {
      return false;
    }
  }
  return true;
}

bool PointCloudEncoder::MarkParentAttribute(int32_t parent_att_id) {
  if (parent_att_id < 0 || parent_att_id >= point_cloud_->num_attributes()) {
    return false;
  }
  const int32_t parent_att_encoder_id =
      attribute_to_encoder_map_[parent_att_id];
  if (!attributes_encoders_[parent_att_encoder_id]->MarkParentAttribute(
          parent_att_id)) {
    return false;
  }
  return true;
}

const PointAttribute *PointCloudEncoder::GetPortableAttribute(
    int32_t parent_att_id) {
  if (parent_att_id < 0 || parent_att_id >= point_cloud_->num_attributes()) {
    return nullptr;
  }
  const int32_t parent_att_encoder_id =
      attribute_to_encoder_map_[parent_att_id];
  return attributes_encoders_[parent_att_encoder_id]->GetPortableAttribute(
      parent_att_id);
}

bool PointCloudEncoder::RearrangeAttributesEncoders() {
  // Find the encoding order of the attribute encoders that is determined by
  // the parent dependencies between individual encoders. Instead of traversing
  // a graph we encode the attributes in multiple iterations where encoding of
  // attributes that depend on other attributes may get postponed until the
  // parent attributes are processed.
  // This is simpler to implement than graph traversal and it automatically
  // detects any cycles in the dependency graph.
  // TODO(ostava): Current implementation needs to encode all attributes of a
  // single encoder to be encoded in a single "chunk", therefore we need to sort
  // attribute encoders before we sort individual attributes. This requirement
  // can be lifted for encoders that can encode individual attributes separately
  // but it will require changes in the current API.
  attributes_encoder_ids_order_.resize(attributes_encoders_.size());
  std::vector<bool> is_encoder_processed(attributes_encoders_.size(), false);
  uint32_t num_processed_encoders = 0;
  while (num_processed_encoders < attributes_encoders_.size()) {
    // Flagged when any of the encoder get processed.
    bool encoder_processed = false;
    for (uint32_t i = 0; i < attributes_encoders_.size(); ++i) {
      if (is_encoder_processed[i]) {
        continue;  // Encoder already processed.
      }
      // Check if all parent encoders are already processed.
      bool can_be_processed = true;
      for (uint32_t p = 0; p < attributes_encoders_[i]->num_attributes(); ++p) {
        const int32_t att_id = attributes_encoders_[i]->GetAttributeId(p);
        for (int ap = 0;
             ap < attributes_encoders_[i]->NumParentAttributes(att_id); ++ap) {
          const uint32_t parent_att_id =
              attributes_encoders_[i]->GetParentAttributeId(att_id, ap);
          const int32_t parent_encoder_id =
              attribute_to_encoder_map_[parent_att_id];
          if (parent_att_id != i && !is_encoder_processed[parent_encoder_id]) {
            can_be_processed = false;
            break;
          }
        }
      }
      if (!can_be_processed) {
        continue;  // Try to process the encoder in the next iteration.
      }
      // Encoder can be processed. Update the encoding order.
      attributes_encoder_ids_order_[num_processed_encoders++] = i;
      is_encoder_processed[i] = true;
      encoder_processed = true;
    }
    if (!encoder_processed &&
        num_processed_encoders < attributes_encoders_.size()) {
      // No encoder was processed but there are still some remaining unprocessed
      // encoders.
      return false;
    }
  }

  // Now for every encoder, reorder the attributes to satisfy their
  // dependencies (an attribute may still depend on other attributes within an
  // encoder).
  std::vector<int32_t> attribute_encoding_order;
  std::vector<bool> is_attribute_processed(point_cloud_->num_attributes(),
                                           false);
  int num_processed_attributes;
  for (uint32_t ae_order = 0; ae_order < attributes_encoders_.size();
       ++ae_order) {
    const int ae = attributes_encoder_ids_order_[ae_order];
    const int32_t num_encoder_attributes =
        attributes_encoders_[ae]->num_attributes();
    if (num_encoder_attributes < 2) {
      continue;  // No need to resolve dependencies for a single attribute.
    }
    num_processed_attributes = 0;
    attribute_encoding_order.resize(num_encoder_attributes);
    while (num_processed_attributes < num_encoder_attributes) {
      // Flagged when any of the attributes get processed.
      bool attribute_processed = false;
      for (int i = 0; i < num_encoder_attributes; ++i) {
        const int32_t att_id = attributes_encoders_[ae]->GetAttributeId(i);
        if (is_attribute_processed[i]) {
          continue;  // Attribute already processed.
        }
        // Check if all parent attributes are already processed.
        bool can_be_processed = true;
        for (int p = 0;
             p < attributes_encoders_[ae]->NumParentAttributes(att_id); ++p) {
          const int32_t parent_att_id =
              attributes_encoders_[ae]->GetParentAttributeId(att_id, p);
          if (!is_attribute_processed[parent_att_id]) {
            can_be_processed = false;
            break;
          }
        }
        if (!can_be_processed) {
          continue;  // Try to process the attribute in the next iteration.
        }
        // Attribute can be processed. Update the encoding order.
        attribute_encoding_order[num_processed_attributes++] = i;
        is_attribute_processed[i] = true;
        attribute_processed = true;
      }
      if (!attribute_processed &&
          num_processed_attributes < num_encoder_attributes) {
        // No attribute was processed but there are still some remaining
        // unprocessed attributes.
        return false;
      }
    }
    // Update the order of the attributes within the encoder.
    attributes_encoders_[ae]->SetAttributeIds(attribute_encoding_order);
  }
  return true;
}

}  // namespace draco
