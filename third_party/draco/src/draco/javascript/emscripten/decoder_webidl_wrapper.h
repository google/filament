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
#ifndef DRACO_JAVASCRIPT_EMSCRIPTEN_DECODER_WEBIDL_WRAPPER_H_
#define DRACO_JAVASCRIPT_EMSCRIPTEN_DECODER_WEBIDL_WRAPPER_H_

#include <vector>

#include "draco/attributes/attribute_transform_type.h"
#include "draco/attributes/point_attribute.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/compression/decode.h"
#include "draco/core/decoder_buffer.h"
#include "draco/mesh/mesh.h"

typedef draco::AttributeTransformType draco_AttributeTransformType;
typedef draco::GeometryAttribute draco_GeometryAttribute;
typedef draco_GeometryAttribute::Type draco_GeometryAttribute_Type;
typedef draco::EncodedGeometryType draco_EncodedGeometryType;
typedef draco::Status draco_Status;
typedef draco::Status::Code draco_StatusCode;
typedef draco::DataType draco_DataType;

// To generate Draco JavaScript bindings you must have emscripten installed.
// Then run make -f Makefile.emcc jslib.
template <typename T>
class DracoArray {
 public:
  T GetValue(int index) const { return values_[index]; }

  void Resize(int size) { values_.resize(size); }
  void MoveData(std::vector<T> &&values) { values_ = std::move(values); }

  // Directly sets a value for a specific index. The array has to be already
  // allocated at this point (using Resize() method).
  void SetValue(int index, T val) { values_[index] = val; }

  int size() const { return values_.size(); }

 private:
  std::vector<T> values_;
};

using DracoFloat32Array = DracoArray<float>;
using DracoInt8Array = DracoArray<int8_t>;
using DracoUInt8Array = DracoArray<uint8_t>;
using DracoInt16Array = DracoArray<int16_t>;
using DracoUInt16Array = DracoArray<uint16_t>;
using DracoInt32Array = DracoArray<int32_t>;
using DracoUInt32Array = DracoArray<uint32_t>;

class MetadataQuerier {
 public:
  MetadataQuerier();

  bool HasEntry(const draco::Metadata &metadata, const char *entry_name) const;

  // This function does not guarantee that entry's type is long.
  long GetIntEntry(const draco::Metadata &metadata,
                   const char *entry_name) const;

  // This function does not guarantee that entry types are long.
  void GetIntEntryArray(const draco::Metadata &metadata, const char *entry_name,
                        DracoInt32Array *out_values) const;

  // This function does not guarantee that entry's type is double.
  double GetDoubleEntry(const draco::Metadata &metadata,
                        const char *entry_name) const;

  // This function does not guarantee that entry's type is char*.
  const char *GetStringEntry(const draco::Metadata &metadata,
                             const char *entry_name);

  long NumEntries(const draco::Metadata &metadata) const;
  const char *GetEntryName(const draco::Metadata &metadata, int entry_id);

 private:
  // Cached values for metadata entries.
  std::vector<std::string> entry_names_;
  const draco::Metadata *entry_names_metadata_;

  // Cached value for GetStringEntry() to avoid scoping issues.
  std::string last_string_returned_;
};

// Class used by emscripten WebIDL Binder [1] to wrap calls to decode Draco
// data.
// [1]http://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/WebIDL-Binder.html
class Decoder {
 public:
  Decoder();

  // Returns the geometry type stored in the |in_buffer|. Return values can be
  // INVALID_GEOMETRY_TYPE, POINT_CLOUD, or MESH.
  // Deprecated: Use decoder.GetEncodedGeometryType(array), where |array| is
  // an Int8Array containing the encoded data.
  static draco_EncodedGeometryType GetEncodedGeometryType_Deprecated(
      draco::DecoderBuffer *in_buffer);

  // Decodes a point cloud from the provided buffer.
  // Deprecated: Use DecodeArrayToPointCloud.
  const draco::Status *DecodeBufferToPointCloud(
      draco::DecoderBuffer *in_buffer, draco::PointCloud *out_point_cloud);

  // Decodes a point cloud from the provided array.
  const draco::Status *DecodeArrayToPointCloud(
      const char *data, size_t data_size, draco::PointCloud *out_point_cloud);

  // Decodes a triangular mesh from the provided buffer.
  // Deprecated: Use DecodeArrayToMesh.
  const draco::Status *DecodeBufferToMesh(draco::DecoderBuffer *in_buffer,
                                          draco::Mesh *out_mesh);

  // Decodes a mesh from the provided array.
  const draco::Status *DecodeArrayToMesh(const char *data, size_t data_size,
                                         draco::Mesh *out_mesh);

  // Returns an attribute id for the first attribute of a given type.
  long GetAttributeId(const draco::PointCloud &pc,
                      draco_GeometryAttribute_Type type) const;

  // Returns an attribute id of an attribute that contains a valid metadata
  // entry "name" with value |attribute_name|.
  static long GetAttributeIdByName(const draco::PointCloud &pc,
                                   const char *attribute_name);

  // Returns an attribute id of an attribute with a specified metadata pair
  // <|metadata_name|, |metadata_value|>.
  static long GetAttributeIdByMetadataEntry(const draco::PointCloud &pc,
                                            const char *metadata_name,
                                            const char *metadata_value);

  // Returns an attribute id of an attribute that has the unique id.
  static const draco::PointAttribute *GetAttributeByUniqueId(
      const draco::PointCloud &pc, long unique_id);

  // Returns a PointAttribute pointer from |att_id| index.
  static const draco::PointAttribute *GetAttribute(const draco::PointCloud &pc,
                                                   long att_id);

  // Returns Mesh::Face values in |out_values| from |face_id| index.
  static bool GetFaceFromMesh(const draco::Mesh &m,
                              draco::FaceIndex::ValueType face_id,
                              DracoInt32Array *out_values);

  // Returns triangle strips for mesh |m|. If there's multiple strips,
  // the strips will be separated by degenerate faces.
  static long GetTriangleStripsFromMesh(const draco::Mesh &m,
                                        DracoInt32Array *strip_values);

  // Returns all faces as triangles. Fails if indices exceed the data range (in
  // particular for uint16), or the output array size does not match.
  // |out_size| is the size in bytes of |out_values|. |out_values| must be
  // allocated before calling this function.
  static bool GetTrianglesUInt16Array(const draco::Mesh &m, int out_size,
                                      void *out_values);
  static bool GetTrianglesUInt32Array(const draco::Mesh &m, int out_size,
                                      void *out_values);

  // Returns float attribute values in |out_values| from |entry_index| index.
  static bool GetAttributeFloat(
      const draco::PointAttribute &pa,
      draco::AttributeValueIndex::ValueType entry_index,
      DracoFloat32Array *out_values);

  // Returns float attribute values for all point ids of the point cloud.
  // I.e., the |out_values| is going to contain m.num_points() entries.
  static bool GetAttributeFloatForAllPoints(const draco::PointCloud &pc,
                                            const draco::PointAttribute &pa,
                                            DracoFloat32Array *out_values);

  // Returns float attribute values for all point ids of the point cloud.
  // I.e., the |out_values| is going to contain m.num_points() entries.
  static bool GetAttributeFloatArrayForAllPoints(
      const draco::PointCloud &pc, const draco::PointAttribute &pa,
      int out_size, void *out_values);

  // Returns int8_t attribute values for all point ids of the point cloud.
  // I.e., the |out_values| is going to contain m.num_points() entries.
  static bool GetAttributeInt8ForAllPoints(const draco::PointCloud &pc,
                                           const draco::PointAttribute &pa,
                                           DracoInt8Array *out_values);

  // Returns uint8_t attribute values for all point ids of the point cloud.
  // I.e., the |out_values| is going to contain m.num_points() entries.
  static bool GetAttributeUInt8ForAllPoints(const draco::PointCloud &pc,
                                            const draco::PointAttribute &pa,
                                            DracoUInt8Array *out_values);

  // Returns int16_t attribute values for all point ids of the point cloud.
  // I.e., the |out_values| is going to contain m.num_points() entries.
  static bool GetAttributeInt16ForAllPoints(const draco::PointCloud &pc,
                                            const draco::PointAttribute &pa,
                                            DracoInt16Array *out_values);

  // Returns uint16_t attribute values for all point ids of the point cloud.
  // I.e., the |out_values| is going to contain m.num_points() entries.
  static bool GetAttributeUInt16ForAllPoints(const draco::PointCloud &pc,
                                             const draco::PointAttribute &pa,
                                             DracoUInt16Array *out_values);

  // Returns int32_t attribute values for all point ids of the point cloud.
  // I.e., the |out_values| is going to contain m.num_points() entries.
  static bool GetAttributeInt32ForAllPoints(const draco::PointCloud &pc,
                                            const draco::PointAttribute &pa,
                                            DracoInt32Array *out_values);

  // Deprecated: Use GetAttributeInt32ForAllPoints() instead.
  static bool GetAttributeIntForAllPoints(const draco::PointCloud &pc,
                                          const draco::PointAttribute &pa,
                                          DracoInt32Array *out_values);

  // Returns uint32_t attribute values for all point ids of the point cloud.
  // I.e., the |out_values| is going to contain m.num_points() entries.
  static bool GetAttributeUInt32ForAllPoints(const draco::PointCloud &pc,
                                             const draco::PointAttribute &pa,
                                             DracoUInt32Array *out_values);

  // Returns |data_type| attribute values for all point ids of the point cloud.
  // I.e., the |out_values| is going to contain m.num_points() entries.
  // |out_size| is the size in bytes of |out_values|. |out_values| must be
  // allocated before calling this function.
  static bool GetAttributeDataArrayForAllPoints(const draco::PointCloud &pc,
                                                const draco::PointAttribute &pa,
                                                draco_DataType data_type,
                                                int out_size, void *out_values);

  // Tells the decoder to skip an attribute transform (e.g. dequantization) for
  // an attribute of a given type.
  void SkipAttributeTransform(draco_GeometryAttribute_Type att_type);

  const draco::Metadata *GetMetadata(const draco::PointCloud &pc) const;
  const draco::Metadata *GetAttributeMetadata(const draco::PointCloud &pc,
                                              long att_id) const;

 private:
  template <class DracoArrayT, class ValueTypeT>
  static bool GetAttributeDataForAllPoints(const draco::PointCloud &pc,
                                           const draco::PointAttribute &pa,
                                           draco::DataType draco_signed_type,
                                           draco::DataType draco_unsigned_type,
                                           DracoArrayT *out_values) {
    const int components = pa.num_components();
    const int num_points = pc.num_points();
    const int num_entries = num_points * components;

    if ((pa.data_type() == draco_signed_type ||
         pa.data_type() == draco_unsigned_type) &&
        pa.is_mapping_identity()) {
      // Copy values directly to the output vector.
      const ValueTypeT *ptr = reinterpret_cast<const ValueTypeT *>(
          pa.GetAddress(draco::AttributeValueIndex(0)));
      out_values->MoveData({ptr, ptr + num_entries});
      return true;
    }

    // Copy values one by one.
    std::vector<ValueTypeT> values(components);
    int entry_id = 0;

    out_values->Resize(num_entries);
    for (draco::PointIndex i(0); i < num_points; ++i) {
      const draco::AttributeValueIndex val_index = pa.mapped_index(i);
      if (!pa.ConvertValue<ValueTypeT>(val_index, &values[0])) {
        return false;
      }
      for (int j = 0; j < components; ++j) {
        out_values->SetValue(entry_id++, values[j]);
      }
    }
    return true;
  }

  template <class T>
  static bool GetAttributeDataArrayForAllPoints(const draco::PointCloud &pc,
                                                const draco::PointAttribute &pa,
                                                const draco::DataType type,
                                                int out_size,
                                                void *out_values) {
    const int components = pa.num_components();
    const int num_points = pc.num_points();
    const int data_size = num_points * components * sizeof(T);
    if (data_size != out_size) {
      return false;
    }
    const bool requested_type_matches = pa.data_type() == type;
    if (requested_type_matches && pa.is_mapping_identity()) {
      // Copy values directly to the output vector.
      const auto ptr = pa.GetAddress(draco::AttributeValueIndex(0));
      ::memcpy(out_values, ptr, data_size);
      return true;
    }

    // Copy values one by one.
    std::vector<T> values(components);
    int entry_id = 0;

    T *const typed_output = reinterpret_cast<T *>(out_values);
    for (draco::PointIndex i(0); i < num_points; ++i) {
      const draco::AttributeValueIndex val_index = pa.mapped_index(i);
      if (requested_type_matches) {
        pa.GetValue(val_index, values.data());
      } else {
        if (!pa.ConvertValue<T>(val_index, values.data())) {
          return false;
        }
      }
      for (int j = 0; j < components; ++j) {
        typed_output[entry_id++] = values[j];
      }
    }
    return true;
  }

  draco::Decoder decoder_;
  draco::Status last_status_;
};

#endif  // DRACO_JAVASCRIPT_EMSCRIPTEN_DECODER_WEBIDL_WRAPPER_H_
