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
#ifndef DRACO_UNITY_DRACO_UNITY_PLUGIN_H_
#define DRACO_UNITY_DRACO_UNITY_PLUGIN_H_

#include "draco/attributes/geometry_attribute.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/compression/decode.h"
#include "draco/core/draco_types.h"

#ifdef DRACO_UNITY_PLUGIN

// If compiling with Visual Studio.
#if defined(_MSC_VER)
#define EXPORT_API __declspec(dllexport)
#else
// Other platforms don't need this.
#define EXPORT_API
#endif  // defined(_MSC_VER)

namespace draco {

extern "C" {

// Struct representing Draco attribute data within Unity.
struct EXPORT_API DracoData {
  DracoData() : data_type(DT_INVALID), data(nullptr) {}

  DataType data_type;
  void *data;
};

// Struct representing a Draco attribute within Unity.
struct EXPORT_API DracoAttribute {
  DracoAttribute()
      : attribute_type(GeometryAttribute::INVALID),
        data_type(DT_INVALID),
        num_components(0),
        unique_id(0),
        private_attribute(nullptr) {}

  GeometryAttribute::Type attribute_type;
  DataType data_type;
  int num_components;
  int unique_id;
  const void *private_attribute;
};

// Struct representing a Draco mesh within Unity.
struct EXPORT_API DracoMesh {
  DracoMesh()
      : num_faces(0),
        num_vertices(0),
        num_attributes(0),
        private_mesh(nullptr) {}

  int num_faces;
  int num_vertices;
  int num_attributes;
  void *private_mesh;
};

// Release data associated with DracoMesh.
void EXPORT_API ReleaseDracoMesh(DracoMesh **mesh_ptr);
// Release data associated with DracoAttribute.
void EXPORT_API ReleaseDracoAttribute(DracoAttribute **attr_ptr);
// Release attribute data.
void EXPORT_API ReleaseDracoData(DracoData **data_ptr);

// Decodes compressed Draco mesh in |data| and returns |mesh|. On input, |mesh|
// must be null. The returned |mesh| must be released with ReleaseDracoMesh.
int EXPORT_API DecodeDracoMesh(char *data, unsigned int length,
                               DracoMesh **mesh);

// Returns |attribute| at |index| in |mesh|.  On input, |attribute| must be
// null. The returned |attribute| must be released with ReleaseDracoAttribute.
bool EXPORT_API GetAttribute(const DracoMesh *mesh, int index,
                             DracoAttribute **attribute);
// Returns |attribute| of |type| at |index| in |mesh|. E.g. If the mesh has
// two texture coordinates then GetAttributeByType(mesh,
// AttributeType.TEX_COORD, 1, &attr); will return the second TEX_COORD
// attribute. On input, |attribute| must be null. The returned |attribute| must
// be released with ReleaseDracoAttribute.
bool EXPORT_API GetAttributeByType(const DracoMesh *mesh,
                                   GeometryAttribute::Type type, int index,
                                   DracoAttribute **attribute);
// Returns |attribute| with |unique_id| in |mesh|. On input, |attribute| must be
// null. The returned |attribute| must be released with ReleaseDracoAttribute.
bool EXPORT_API GetAttributeByUniqueId(const DracoMesh *mesh, int unique_id,
                                       DracoAttribute **attribute);
// Returns the indices as well as the type of data in |indices|. On input,
// |indices| must be null. The returned |indices| must be released with
// ReleaseDracoData.
bool EXPORT_API GetMeshIndices(const DracoMesh *mesh, DracoData **indices);
// Returns the attribute data from attribute as well as the type of data in
// |data|. On input, |data| must be null. The returned |data| must be released
// with ReleaseDracoData.
bool EXPORT_API GetAttributeData(const DracoMesh *mesh,
                                 const DracoAttribute *attribute,
                                 DracoData **data);

// DracoToUnityMesh is deprecated.
struct EXPORT_API DracoToUnityMesh {
  DracoToUnityMesh()
      : num_faces(0),
        indices(nullptr),
        num_vertices(0),
        position(nullptr),
        has_normal(false),
        normal(nullptr),
        has_texcoord(false),
        texcoord(nullptr),
        has_color(false),
        color(nullptr) {}

  int num_faces;
  int *indices;
  int num_vertices;
  float *position;
  bool has_normal;
  float *normal;
  bool has_texcoord;
  float *texcoord;
  bool has_color;
  float *color;
};

// ReleaseUnityMesh is deprecated.
void EXPORT_API ReleaseUnityMesh(DracoToUnityMesh **mesh_ptr);

// To use this function, you do not allocate memory for |tmp_mesh|, just
// define and pass a null pointer. Otherwise there will be memory leak.
// DecodeMeshForUnity is deprecated.
int EXPORT_API DecodeMeshForUnity(char *data, unsigned int length,
                                  DracoToUnityMesh **tmp_mesh);
}  // extern "C"

}  // namespace draco

#endif  // DRACO_UNITY_PLUGIN

#endif  // DRACO_UNITY_DRACO_UNITY_PLUGIN_H_
