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
#include "draco/maya/draco_maya_plugin.h"

#ifdef DRACO_MAYA_PLUGIN

namespace draco {
namespace maya {

static void decode_faces(std::unique_ptr<draco::Mesh> &drc_mesh,
                         Drc2PyMesh *out_mesh) {
  int num_faces = drc_mesh->num_faces();
  out_mesh->faces = new int[num_faces * 3];
  out_mesh->faces_num = num_faces;
  for (int i = 0; i < num_faces; i++) {
    const draco::Mesh::Face &face = drc_mesh->face(draco::FaceIndex(i));
    out_mesh->faces[i * 3 + 0] = face[0].value();
    out_mesh->faces[i * 3 + 1] = face[1].value();
    out_mesh->faces[i * 3 + 2] = face[2].value();
  }
}
static void decode_vertices(std::unique_ptr<draco::Mesh> &drc_mesh,
                            Drc2PyMesh *out_mesh) {
  const auto pos_att =
      drc_mesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
  if (pos_att == nullptr) {
    out_mesh->vertices = new float[0];
    out_mesh->vertices_num = 0;
    return;
  }

  int num_vertices = drc_mesh->num_points();
  out_mesh->vertices = new float[num_vertices * 3];
  out_mesh->vertices_num = num_vertices;
  for (int i = 0; i < num_vertices; i++) {
    draco::PointIndex pi(i);
    const draco::AttributeValueIndex val_index = pos_att->mapped_index(pi);
    float out_vertex[3];
    bool is_ok = pos_att->ConvertValue<float, 3>(val_index, out_vertex);
    if (!is_ok) return;
    out_mesh->vertices[i * 3 + 0] = out_vertex[0];
    out_mesh->vertices[i * 3 + 1] = out_vertex[1];
    out_mesh->vertices[i * 3 + 2] = out_vertex[2];
  }
}
static void decode_normals(std::unique_ptr<draco::Mesh> &drc_mesh,
                           Drc2PyMesh *out_mesh) {
  const auto normal_att =
      drc_mesh->GetNamedAttribute(draco::GeometryAttribute::NORMAL);
  if (normal_att == nullptr) {
    out_mesh->normals = new float[0];
    out_mesh->normals_num = 0;
    return;
  }
  int num_normals = drc_mesh->num_points();
  out_mesh->normals = new float[num_normals * 3];
  out_mesh->normals_num = num_normals;

  for (int i = 0; i < num_normals; i++) {
    draco::PointIndex pi(i);
    const draco::AttributeValueIndex val_index = normal_att->mapped_index(pi);
    float out_normal[3];
    bool is_ok = normal_att->ConvertValue<float, 3>(val_index, out_normal);
    if (!is_ok) return;
    out_mesh->normals[i * 3 + 0] = out_normal[0];
    out_mesh->normals[i * 3 + 1] = out_normal[1];
    out_mesh->normals[i * 3 + 2] = out_normal[2];
  }
}
static void decode_uvs(std::unique_ptr<draco::Mesh> &drc_mesh,
                       Drc2PyMesh *out_mesh) {
  const auto uv_att =
      drc_mesh->GetNamedAttribute(draco::GeometryAttribute::TEX_COORD);
  if (uv_att == nullptr) {
    out_mesh->uvs = new float[0];
    out_mesh->uvs_num = 0;
    out_mesh->uvs_real_num = 0;
    return;
  }
  int num_uvs = drc_mesh->num_points();
  out_mesh->uvs = new float[num_uvs * 2];
  out_mesh->uvs_num = num_uvs;
  out_mesh->uvs_real_num = uv_att->size();

  for (int i = 0; i < num_uvs; i++) {
    draco::PointIndex pi(i);
    const draco::AttributeValueIndex val_index = uv_att->mapped_index(pi);
    float out_uv[2];
    bool is_ok = uv_att->ConvertValue<float, 2>(val_index, out_uv);
    if (!is_ok) return;
    out_mesh->uvs[i * 2 + 0] = out_uv[0];
    out_mesh->uvs[i * 2 + 1] = out_uv[1];
  }
}

void drc2py_free(Drc2PyMesh **mesh_ptr) {
  Drc2PyMesh *mesh = *mesh_ptr;
  if (!mesh) return;
  if (mesh->faces) {
    delete[] mesh->faces;
    mesh->faces = nullptr;
    mesh->faces_num = 0;
  }
  if (mesh->vertices) {
    delete[] mesh->vertices;
    mesh->vertices = nullptr;
    mesh->vertices_num = 0;
  }
  if (mesh->normals) {
    delete[] mesh->normals;
    mesh->normals = nullptr;
    mesh->normals_num = 0;
  }
  if (mesh->uvs) {
    delete[] mesh->uvs;
    mesh->uvs = nullptr;
    mesh->uvs_num = 0;
  }
  delete mesh;
  *mesh_ptr = nullptr;
}

DecodeResult drc2py_decode(char *data, unsigned int length,
                           Drc2PyMesh **res_mesh) {
  draco::DecoderBuffer buffer;
  buffer.Init(data, length);
  auto type_statusor = draco::Decoder::GetEncodedGeometryType(&buffer);
  if (!type_statusor.ok()) {
    return DecodeResult::KO_GEOMETRY_TYPE_INVALID;
  }
  const draco::EncodedGeometryType geom_type = type_statusor.value();
  if (geom_type != draco::TRIANGULAR_MESH) {
    return DecodeResult::KO_TRIANGULAR_MESH_NOT_FOUND;
  }

  draco::Decoder decoder;
  auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
  if (!statusor.ok()) {
    return DecodeResult::KO_MESH_DECODING;
  }
  std::unique_ptr<draco::Mesh> drc_mesh = std::move(statusor).value();

  *res_mesh = new Drc2PyMesh();
  decode_faces(drc_mesh, *res_mesh);
  decode_vertices(drc_mesh, *res_mesh);
  decode_normals(drc_mesh, *res_mesh);
  decode_uvs(drc_mesh, *res_mesh);
  return DecodeResult::OK;
}

// As encode references see https://github.com/google/draco/issues/116
EncodeResult drc2py_encode(Drc2PyMesh *in_mesh, char *file_path) {
  if (in_mesh->faces_num == 0) return EncodeResult::KO_WRONG_INPUT;
  if (in_mesh->vertices_num == 0) return EncodeResult::KO_WRONG_INPUT;
  // TODO: Add check to protect against quad faces. At the moment only
  // Triangular faces are supported

  std::unique_ptr<draco::Mesh> drc_mesh(new draco::Mesh());

  // Marshall Faces
  int num_faces = in_mesh->faces_num;
  drc_mesh->SetNumFaces(num_faces);
  for (int i = 0; i < num_faces; ++i) {
    Mesh::Face face;
    face[0] = in_mesh->faces[i * 3 + 0];
    face[1] = in_mesh->faces[i * 3 + 1];
    face[2] = in_mesh->faces[i * 3 + 2];
    drc_mesh->SetFace(FaceIndex(i), face);
  }

  // Marshall Vertices
  int num_points = in_mesh->vertices_num;
  drc_mesh->set_num_points(num_points);
  GeometryAttribute va;
  va.Init(GeometryAttribute::POSITION, nullptr, 3, DT_FLOAT32, false,
          sizeof(float) * 3, 0);
  int pos_att_id = drc_mesh->AddAttribute(va, true, num_points);
  float point[3];
  for (int i = 0; i < num_points; ++i) {
    point[0] = in_mesh->vertices[i * 3 + 0];
    point[1] = in_mesh->vertices[i * 3 + 1];
    point[2] = in_mesh->vertices[i * 3 + 2];
    drc_mesh->attribute(pos_att_id)
        ->SetAttributeValue(AttributeValueIndex(i), point);
  }

  // Marshall Normals
  int num_normals = in_mesh->normals_num;
  int norm_att_id;
  if (num_normals > 0) {
    GeometryAttribute va;
    va.Init(GeometryAttribute::NORMAL, nullptr, 3, DT_FLOAT32, false,
            sizeof(float) * 3, 0);
    norm_att_id = drc_mesh->AddAttribute(va, true, num_normals);

    float norm[3];
    for (int i = 0; i < num_normals; ++i) {
      norm[0] = in_mesh->normals[i * 3 + 0];
      norm[1] = in_mesh->normals[i * 3 + 1];
      norm[2] = in_mesh->normals[i * 3 + 2];
      drc_mesh->attribute(norm_att_id)
          ->SetAttributeValue(AttributeValueIndex(i), norm);
    }
  }

  // Marshall Uvs
  int num_uvs = in_mesh->uvs_num;
  int uv_att_id;
  if (num_uvs > 0) {
    GeometryAttribute va;
    va.Init(GeometryAttribute::TEX_COORD, nullptr, 2, DT_FLOAT32, false,
            sizeof(float) * 2, 0);
    uv_att_id = drc_mesh->AddAttribute(va, true, num_uvs);
    float uv[3];
    for (int i = 0; i < num_uvs; ++i) {
      uv[0] = in_mesh->uvs[i * 2 + 0];
      uv[1] = in_mesh->uvs[i * 2 + 1];
      drc_mesh->attribute(uv_att_id)->SetAttributeValue(AttributeValueIndex(i),
                                                        uv);
    }
  }

// Deduplicate Attributes and Points
#ifdef DRACO_ATTRIBUTE_VALUES_DEDUPLICATION_SUPPORTED
  drc_mesh->DeduplicateAttributeValues();
#endif
#ifdef DRACO_ATTRIBUTE_INDICES_DEDUPLICATION_SUPPORTED
  drc_mesh->DeduplicatePointIds();
#endif

  // Encode Mesh
  draco::Encoder encoder;  // Use default encode settings (See draco_encoder.cc
                           // Options struct)
  draco::EncoderBuffer buffer;
  const draco::Status status = encoder.EncodeMeshToBuffer(*drc_mesh, &buffer);
  if (!status.ok()) {
    // Use status.error_msg() to check the error
    return EncodeResult::KO_MESH_ENCODING;
  }

  // Save to file
  std::string file = file_path;
  std::ofstream out_file(file, std::ios::binary);
  if (!out_file) {
    return EncodeResult::KO_FILE_CREATION;
  }
  out_file.write(buffer.data(), buffer.size());
  return EncodeResult::OK;
}

}  // namespace maya

}  // namespace draco

#endif  // DRACO_MAYA_PLUGIN
