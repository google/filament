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
#ifndef DRACO_JAVASCRIPT_EMSCRITPEN_ENCODER_WEBIDL_WRAPPER_H_
#define DRACO_JAVASCRIPT_EMSCRITPEN_ENCODER_WEBIDL_WRAPPER_H_

#include <vector>

#include "draco/attributes/point_attribute.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/compression/config/encoder_options.h"
#include "draco/compression/encode.h"
#include "draco/compression/expert_encode.h"
#include "draco/mesh/mesh.h"

typedef draco::GeometryAttribute draco_GeometryAttribute;
typedef draco::GeometryAttribute::Type draco_GeometryAttribute_Type;
typedef draco::EncodedGeometryType draco_EncodedGeometryType;
typedef draco::MeshEncoderMethod draco_MeshEncoderMethod;

class DracoInt8Array {
 public:
  DracoInt8Array();
  int8_t GetValue(int index) const;
  bool SetValues(const char *values, int count);

  size_t size() { return values_.size(); }

 private:
  std::vector<int8_t> values_;
};

class MetadataBuilder {
 public:
  MetadataBuilder();
  bool AddStringEntry(draco::Metadata *metadata, const char *entry_name,
                      const char *entry_value);
  bool AddIntEntry(draco::Metadata *metadata, const char *entry_name,
                   long entry_value);
  bool AddDoubleEntry(draco::Metadata *metadata, const char *entry_name,
                      double entry_value);
};

class PointCloudBuilder {
 public:
  PointCloudBuilder() {}
  int AddFloatAttribute(draco::PointCloud *pc,
                        draco_GeometryAttribute_Type type, long num_vertices,
                        long num_components, const float *att_values);
  int AddInt8Attribute(draco::PointCloud *pc, draco_GeometryAttribute_Type type,
                       long num_vertices, long num_components,
                       const char *att_values);
  int AddUInt8Attribute(draco::PointCloud *pc,
                        draco_GeometryAttribute_Type type, long num_vertices,
                        long num_components, const uint8_t *att_values);
  int AddInt16Attribute(draco::PointCloud *pc,
                        draco_GeometryAttribute_Type type, long num_vertices,
                        long num_components, const int16_t *att_values);
  int AddUInt16Attribute(draco::PointCloud *pc,
                         draco_GeometryAttribute_Type type, long num_vertices,
                         long num_components, const uint16_t *att_values);
  int AddInt32Attribute(draco::PointCloud *pc,
                        draco_GeometryAttribute_Type type, long num_vertices,
                        long num_components, const int32_t *att_values);
  int AddUInt32Attribute(draco::PointCloud *pc,
                         draco_GeometryAttribute_Type type, long num_vertices,
                         long num_components, const uint32_t *att_values);
  bool SetMetadataForAttribute(draco::PointCloud *pc, long attribute_id,
                               const draco::Metadata *metadata);
  bool AddMetadata(draco::PointCloud *pc, const draco::Metadata *metadata);

 private:
  template <typename DataTypeT>
  int AddAttribute(draco::PointCloud *pc, draco_GeometryAttribute_Type type,
                   long num_vertices, long num_components,
                   const DataTypeT *att_values,
                   draco::DataType draco_data_type) {
    if (!pc) {
      return -1;
    }
    std::unique_ptr<draco::PointAttribute> att(new draco::PointAttribute());
    att->Init(type, num_components, draco_data_type,
              /* normalized */ false, num_vertices);
    const int att_id = pc->AddAttribute(std::move(att));
    draco::PointAttribute *const att_ptr = pc->attribute(att_id);

    for (draco::PointIndex i(0); i < num_vertices; ++i) {
      att_ptr->SetAttributeValue(att_ptr->mapped_index(i),
                                 &att_values[i.value() * num_components]);
    }
    if (pc->num_points() == 0) {
      pc->set_num_points(num_vertices);
    } else if (pc->num_points() != num_vertices) {
      return -1;
    }
    return att_id;
  }
};

// TODO(draco-eng): Regenerate wasm decoder.
// TODO(draco-eng): Add script to generate and test all Javascipt code.
class MeshBuilder : public PointCloudBuilder {
 public:
  MeshBuilder();

  bool AddFacesToMesh(draco::Mesh *mesh, long num_faces, const int *faces);

  // Deprecated: Use AddFloatAttribute() instead.
  int AddFloatAttributeToMesh(draco::Mesh *mesh,
                              draco_GeometryAttribute_Type type,
                              long num_vertices, long num_components,
                              const float *att_values);

  // Deprecated: Use AddInt32Attribute() instead.
  int AddInt32AttributeToMesh(draco::Mesh *mesh,
                              draco_GeometryAttribute_Type type,
                              long num_vertices, long num_components,
                              const int32_t *att_values);

  // Deprecated: Use AddMetadata() instead.
  bool AddMetadataToMesh(draco::Mesh *mesh, const draco::Metadata *metadata);
};

class Encoder {
 public:
  Encoder();

  void SetEncodingMethod(long method);
  void SetAttributeQuantization(draco_GeometryAttribute_Type type,
                                long quantization_bits);
  void SetAttributeExplicitQuantization(draco_GeometryAttribute_Type type,
                                        long quantization_bits,
                                        long num_components,
                                        const float *origin, float range);
  void SetSpeedOptions(long encoding_speed, long decoding_speed);
  void SetTrackEncodedProperties(bool flag);

  int EncodeMeshToDracoBuffer(draco::Mesh *mesh, DracoInt8Array *buffer);

  int EncodePointCloudToDracoBuffer(draco::PointCloud *pc,
                                    bool deduplicate_values,
                                    DracoInt8Array *buffer);
  int GetNumberOfEncodedPoints();
  int GetNumberOfEncodedFaces();

 private:
  draco::Encoder encoder_;
};

class ExpertEncoder {
 public:
  ExpertEncoder(draco::PointCloud *pc);

  void SetEncodingMethod(long method);
  void SetAttributeQuantization(long att_id, long quantization_bits);
  void SetAttributeExplicitQuantization(long att_id, long quantization_bits,
                                        long num_components,
                                        const float *origin, float range);
  void SetSpeedOptions(long encoding_speed, long decoding_speed);
  void SetTrackEncodedProperties(bool flag);

  int EncodeToDracoBuffer(bool deduplicate_values, DracoInt8Array *buffer);

  int GetNumberOfEncodedPoints();
  int GetNumberOfEncodedFaces();

 private:
  std::unique_ptr<draco::ExpertEncoder> encoder_;

  draco::PointCloud *pc_;
};

#endif  // DRACO_JAVASCRIPT_EMSCRITPEN_ENCODER_WEBIDL_WRAPPER_H_
