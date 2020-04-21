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
#ifndef DRACO_MAYA_PLUGIN_H_
#define DRACO_MAYA_PLUGIN_H_

#include <fstream>

#include "draco/compression/decode.h"
#include "draco/compression/encode.h"

#ifdef BUILD_MAYA_PLUGIN

// If compiling with Visual Studio.
#if defined(_MSC_VER)
#define EXPORT_API __declspec(dllexport)
#else
// Other platforms don't need this.
#define EXPORT_API
#endif  // defined(_MSC_VER)

namespace draco {
namespace maya {

enum class EncodeResult {
  OK = 0,
  KO_WRONG_INPUT = -1,
  KO_MESH_ENCODING = -2,
  KO_FILE_CREATION = -3
};
enum class DecodeResult {
  OK = 0,
  KO_GEOMETRY_TYPE_INVALID = -1,
  KO_TRIANGULAR_MESH_NOT_FOUND = -2,
  KO_MESH_DECODING = -3
};

extern "C" {
struct EXPORT_API Drc2PyMesh {
  Drc2PyMesh()
      : faces_num(0),
        faces(nullptr),
        vertices_num(0),
        vertices(nullptr),
        normals_num(0),
        normals(nullptr),
        uvs_num(0),
        uvs_real_num(0),
        uvs(nullptr) {}
  int faces_num;
  int *faces;
  int vertices_num;
  float *vertices;
  int normals_num;
  float *normals;
  int uvs_num;
  int uvs_real_num;
  float *uvs;
};

EXPORT_API DecodeResult drc2py_decode(char *data, unsigned int length,
                                      Drc2PyMesh **res_mesh);
EXPORT_API void drc2py_free(Drc2PyMesh **res_mesh);
EXPORT_API EncodeResult drc2py_encode(Drc2PyMesh *in_mesh, char *file_path);
}  // extern "C"

}  // namespace maya
}  // namespace draco

#endif  // BUILD_MAYA_PLUGIN

#endif  // DRACO_MAYA_PLUGIN_H_
