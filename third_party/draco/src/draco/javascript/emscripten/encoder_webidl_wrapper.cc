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
#include "draco/javascript/emscripten/encoder_webidl_wrapper.h"

#include "draco/compression/encode.h"
#include "draco/mesh/mesh.h"

DracoInt8Array::DracoInt8Array() {}

int8_t DracoInt8Array::GetValue(int index) const { return values_[index]; }

bool DracoInt8Array::SetValues(const char *values, int count) {
  values_.assign(values, values + count);
  return true;
}

using draco::Mesh;
using draco::Metadata;
using draco::PointCloud;

MetadataBuilder::MetadataBuilder() {}

bool MetadataBuilder::AddStringEntry(Metadata *metadata, const char *entry_name,
                                     const char *entry_value) {
  if (!metadata) {
    return false;
  }
  const std::string name{entry_name};
  const std::string value{entry_value};
  metadata->AddEntryString(entry_name, entry_value);
  return true;
}

bool MetadataBuilder::AddIntEntry(Metadata *metadata, const char *entry_name,
                                  long entry_value) {
  if (!metadata) {
    return false;
  }
  const std::string name{entry_name};
  metadata->AddEntryInt(name, entry_value);
  return true;
}

bool MetadataBuilder::AddIntEntryArray(draco::Metadata *metadata,
                                       const char *entry_name,
                                       const int32_t *entry_values,
                                       int32_t num_values) {
  if (!metadata) {
    return false;
  }
  const std::string name{entry_name};
  metadata->AddEntryIntArray(name, {entry_values, entry_values + num_values});
  return true;
}

bool MetadataBuilder::AddDoubleEntry(Metadata *metadata, const char *entry_name,
                                     double entry_value) {
  if (!metadata) {
    return false;
  }
  const std::string name{entry_name};
  metadata->AddEntryDouble(name, entry_value);
  return true;
}

int PointCloudBuilder::AddFloatAttribute(PointCloud *pc,
                                         draco_GeometryAttribute_Type type,
                                         long num_vertices, long num_components,
                                         const float *att_values) {
  return AddAttribute(pc, type, num_vertices, num_components, att_values,
                      draco::DT_FLOAT32);
}

int PointCloudBuilder::AddInt8Attribute(PointCloud *pc,
                                        draco_GeometryAttribute_Type type,
                                        long num_vertices, long num_components,
                                        const char *att_values) {
  return AddAttribute(pc, type, num_vertices, num_components, att_values,
                      draco::DT_INT8);
}

int PointCloudBuilder::AddUInt8Attribute(PointCloud *pc,
                                         draco_GeometryAttribute_Type type,
                                         long num_vertices, long num_components,
                                         const uint8_t *att_values) {
  return AddAttribute(pc, type, num_vertices, num_components, att_values,
                      draco::DT_UINT8);
}

int PointCloudBuilder::AddInt16Attribute(PointCloud *pc,
                                         draco_GeometryAttribute_Type type,
                                         long num_vertices, long num_components,
                                         const int16_t *att_values) {
  return AddAttribute(pc, type, num_vertices, num_components, att_values,
                      draco::DT_INT16);
}

int PointCloudBuilder::AddUInt16Attribute(PointCloud *pc,
                                          draco_GeometryAttribute_Type type,
                                          long num_vertices,
                                          long num_components,
                                          const uint16_t *att_values) {
  return AddAttribute(pc, type, num_vertices, num_components, att_values,
                      draco::DT_UINT16);
}

int PointCloudBuilder::AddInt32Attribute(PointCloud *pc,
                                         draco_GeometryAttribute_Type type,
                                         long num_vertices, long num_components,
                                         const int32_t *att_values) {
  return AddAttribute(pc, type, num_vertices, num_components, att_values,
                      draco::DT_INT32);
}

int PointCloudBuilder::AddUInt32Attribute(PointCloud *pc,
                                          draco_GeometryAttribute_Type type,
                                          long num_vertices,
                                          long num_components,
                                          const uint32_t *att_values) {
  return AddAttribute(pc, type, num_vertices, num_components, att_values,
                      draco::DT_UINT32);
}

bool PointCloudBuilder::AddMetadata(PointCloud *pc, const Metadata *metadata) {
  if (!pc) {
    return false;
  }
  // Not allow write over metadata.
  if (pc->metadata()) {
    return false;
  }
  std::unique_ptr<draco::GeometryMetadata> new_metadata =
      std::unique_ptr<draco::GeometryMetadata>(
          new draco::GeometryMetadata(*metadata));
  pc->AddMetadata(std::move(new_metadata));
  return true;
}

bool PointCloudBuilder::SetMetadataForAttribute(PointCloud *pc,
                                                long attribute_id,
                                                const Metadata *metadata) {
  if (!pc) {
    return false;
  }
  // If empty metadata, just ignore.
  if (!metadata) {
    return false;
  }
  if (attribute_id < 0 || attribute_id >= pc->num_attributes()) {
    return false;
  }

  if (!pc->metadata()) {
    std::unique_ptr<draco::GeometryMetadata> geometry_metadata =
        std::unique_ptr<draco::GeometryMetadata>(new draco::GeometryMetadata());
    pc->AddMetadata(std::move(geometry_metadata));
  }

  // Get unique attribute id for the attribute.
  const long unique_id = pc->attribute(attribute_id)->unique_id();

  std::unique_ptr<draco::AttributeMetadata> att_metadata =
      std::unique_ptr<draco::AttributeMetadata>(
          new draco::AttributeMetadata(*metadata));
  att_metadata->set_att_unique_id(unique_id);
  pc->metadata()->AddAttributeMetadata(std::move(att_metadata));
  return true;
}

bool PointCloudBuilder::SetNormalizedFlagForAttribute(draco::PointCloud *pc,
                                                      long attribute_id,
                                                      bool normalized) {
  if (!pc) {
    return false;
  }
  if (attribute_id < 0 || attribute_id >= pc->num_attributes()) {
    return false;
  }
  pc->attribute(attribute_id)->set_normalized(normalized);
  return true;
}

MeshBuilder::MeshBuilder() {}

bool MeshBuilder::AddFacesToMesh(Mesh *mesh, long num_faces, const int *faces) {
  if (!mesh) {
    return false;
  }
  mesh->SetNumFaces(num_faces);
  for (draco::FaceIndex i(0); i < num_faces; ++i) {
    draco::Mesh::Face face;
    face[0] = faces[i.value() * 3];
    face[1] = faces[i.value() * 3 + 1];
    face[2] = faces[i.value() * 3 + 2];
    mesh->SetFace(i, face);
  }
  return true;
}

int MeshBuilder::AddFloatAttributeToMesh(Mesh *mesh,
                                         draco_GeometryAttribute_Type type,
                                         long num_vertices, long num_components,
                                         const float *att_values) {
  return AddFloatAttribute(mesh, type, num_vertices, num_components,
                           att_values);
}

int MeshBuilder::AddInt32AttributeToMesh(draco::Mesh *mesh,
                                         draco_GeometryAttribute_Type type,
                                         long num_vertices, long num_components,
                                         const int32_t *att_values) {
  return AddInt32Attribute(mesh, type, num_vertices, num_components,
                           att_values);
}

bool MeshBuilder::AddMetadataToMesh(Mesh *mesh, const Metadata *metadata) {
  return AddMetadata(mesh, metadata);
}

Encoder::Encoder() {}

void Encoder::SetEncodingMethod(long method) {
  encoder_.SetEncodingMethod(method);
}

void Encoder::SetAttributeQuantization(draco_GeometryAttribute_Type type,
                                       long quantization_bits) {
  encoder_.SetAttributeQuantization(type, quantization_bits);
}

void Encoder::SetAttributeExplicitQuantization(
    draco_GeometryAttribute_Type type, long quantization_bits,
    long num_components, const float *origin, float range) {
  encoder_.SetAttributeExplicitQuantization(type, quantization_bits,
                                            num_components, origin, range);
}

void Encoder::SetSpeedOptions(long encoding_speed, long decoding_speed) {
  encoder_.SetSpeedOptions(encoding_speed, decoding_speed);
}

void Encoder::SetTrackEncodedProperties(bool flag) {
  encoder_.SetTrackEncodedProperties(flag);
}

int Encoder::EncodeMeshToDracoBuffer(Mesh *mesh, DracoInt8Array *draco_buffer) {
  if (!mesh) {
    return 0;
  }
  draco::EncoderBuffer buffer;
  if (mesh->GetNamedAttributeId(draco::GeometryAttribute::POSITION) == -1) {
    return 0;
  }
  if (!mesh->DeduplicateAttributeValues()) {
    return 0;
  }
  mesh->DeduplicatePointIds();
  if (!encoder_.EncodeMeshToBuffer(*mesh, &buffer).ok()) {
    return 0;
  }
  draco_buffer->SetValues(buffer.data(), buffer.size());
  return buffer.size();
}

int Encoder::EncodePointCloudToDracoBuffer(draco::PointCloud *pc,
                                           bool deduplicate_values,
                                           DracoInt8Array *draco_buffer) {
  // TODO(ostava): Refactor common functionality with EncodeMeshToDracoBuffer().
  if (!pc) {
    return 0;
  }
  draco::EncoderBuffer buffer;
  if (pc->GetNamedAttributeId(draco::GeometryAttribute::POSITION) == -1) {
    return 0;
  }
  if (deduplicate_values) {
    if (!pc->DeduplicateAttributeValues()) {
      return 0;
    }
    pc->DeduplicatePointIds();
  }
  if (!encoder_.EncodePointCloudToBuffer(*pc, &buffer).ok()) {
    return 0;
  }
  draco_buffer->SetValues(buffer.data(), buffer.size());
  return buffer.size();
}

int Encoder::GetNumberOfEncodedPoints() {
  return encoder_.num_encoded_points();
}

int Encoder::GetNumberOfEncodedFaces() { return encoder_.num_encoded_faces(); }

ExpertEncoder::ExpertEncoder(PointCloud *pc) : pc_(pc) {
  // Web-IDL interface does not support constructor overloading so instead we
  // use RTTI to determine whether the input is a mesh or a point cloud.
  Mesh *mesh = dynamic_cast<Mesh *>(pc);
  if (mesh) {
    encoder_ =
        std::unique_ptr<draco::ExpertEncoder>(new draco::ExpertEncoder(*mesh));
  } else {
    encoder_ =
        std::unique_ptr<draco::ExpertEncoder>(new draco::ExpertEncoder(*pc));
  }
}

void ExpertEncoder::SetEncodingMethod(long method) {
  encoder_->SetEncodingMethod(method);
}

void ExpertEncoder::SetAttributeQuantization(long att_id,
                                             long quantization_bits) {
  encoder_->SetAttributeQuantization(att_id, quantization_bits);
}

void ExpertEncoder::SetAttributeExplicitQuantization(long att_id,
                                                     long quantization_bits,
                                                     long num_components,
                                                     const float *origin,
                                                     float range) {
  encoder_->SetAttributeExplicitQuantization(att_id, quantization_bits,
                                             num_components, origin, range);
}

void ExpertEncoder::SetSpeedOptions(long encoding_speed, long decoding_speed) {
  encoder_->SetSpeedOptions(encoding_speed, decoding_speed);
}

void ExpertEncoder::SetTrackEncodedProperties(bool flag) {
  encoder_->SetTrackEncodedProperties(flag);
}

int ExpertEncoder::EncodeToDracoBuffer(bool deduplicate_values,
                                       DracoInt8Array *draco_buffer) {
  if (!pc_) {
    return 0;
  }
  if (deduplicate_values) {
    if (!pc_->DeduplicateAttributeValues()) {
      return 0;
    }
    pc_->DeduplicatePointIds();
  }

  draco::EncoderBuffer buffer;
  if (!encoder_->EncodeToBuffer(&buffer).ok()) {
    return 0;
  }
  draco_buffer->SetValues(buffer.data(), buffer.size());
  return buffer.size();
}

int ExpertEncoder::GetNumberOfEncodedPoints() {
  return encoder_->num_encoded_points();
}

int ExpertEncoder::GetNumberOfEncodedFaces() {
  return encoder_->num_encoded_faces();
}
