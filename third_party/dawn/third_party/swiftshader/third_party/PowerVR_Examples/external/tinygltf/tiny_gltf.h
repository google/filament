//
// Header-only tiny glTF 2.0 loader and serializer.
//
//
// The MIT License (MIT)
//
// Copyright (c) 2015 - 2017 Syoyo Fujita, Aurélien Chatelain and many
// contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// Version:
//  - v2.0.0 glTF 2.0!.
//
// Tiny glTF loader is using following third party libraries:
//
//  - picojson: C++ JSON library.
//  - base64: base64 decode/encode library.
//  - stb_image: Image loading library.
//
// *** Modification By PowerVR Developer Technology Team, Imagination Technologies ***
// - Load function takes Android's AAssetManager object
//


#ifndef TINY_GLTF_H_
#define TINY_GLTF_H_

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace tinygltf {

#define TINYGLTF_MODE_POINTS (0)
#define TINYGLTF_MODE_LINE (1)
#define TINYGLTF_MODE_LINE_LOOP (2)
#define TINYGLTF_MODE_TRIANGLES (4)
#define TINYGLTF_MODE_TRIANGLE_STRIP (5)
#define TINYGLTF_MODE_TRIANGLE_FAN (6)

#define TINYGLTF_COMPONENT_TYPE_BYTE (5120)
#define TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE (5121)
#define TINYGLTF_COMPONENT_TYPE_SHORT (5122)
#define TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT (5123)
#define TINYGLTF_COMPONENT_TYPE_INT (5124)
#define TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT (5125)
#define TINYGLTF_COMPONENT_TYPE_FLOAT (5126)
#define TINYGLTF_COMPONENT_TYPE_DOUBLE (5127)

#define TINYGLTF_TEXTURE_FILTER_NEAREST (9728)
#define TINYGLTF_TEXTURE_FILTER_LINEAR (9729)
#define TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST (9984)
#define TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST (9985)
#define TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR (9986)
#define TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR (9987)

#define TINYGLTF_TEXTURE_WRAP_RPEAT (10497)
#define TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE (33071)
#define TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT (33648)

// Redeclarations of the above for technique.parameters.
#define TINYGLTF_PARAMETER_TYPE_BYTE (5120)
#define TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE (5121)
#define TINYGLTF_PARAMETER_TYPE_SHORT (5122)
#define TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT (5123)
#define TINYGLTF_PARAMETER_TYPE_INT (5124)
#define TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT (5125)
#define TINYGLTF_PARAMETER_TYPE_FLOAT (5126)

#define TINYGLTF_PARAMETER_TYPE_FLOAT_VEC2 (35664)
#define TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 (35665)
#define TINYGLTF_PARAMETER_TYPE_FLOAT_VEC4 (35666)

#define TINYGLTF_PARAMETER_TYPE_INT_VEC2 (35667)
#define TINYGLTF_PARAMETER_TYPE_INT_VEC3 (35668)
#define TINYGLTF_PARAMETER_TYPE_INT_VEC4 (35669)

#define TINYGLTF_PARAMETER_TYPE_BOOL (35670)
#define TINYGLTF_PARAMETER_TYPE_BOOL_VEC2 (35671)
#define TINYGLTF_PARAMETER_TYPE_BOOL_VEC3 (35672)
#define TINYGLTF_PARAMETER_TYPE_BOOL_VEC4 (35673)

#define TINYGLTF_PARAMETER_TYPE_FLOAT_MAT2 (35674)
#define TINYGLTF_PARAMETER_TYPE_FLOAT_MAT3 (35675)
#define TINYGLTF_PARAMETER_TYPE_FLOAT_MAT4 (35676)

#define TINYGLTF_PARAMETER_TYPE_SAMPLER_2D (35678)

// End parameter types

#define TINYGLTF_TYPE_VEC2 (2)
#define TINYGLTF_TYPE_VEC3 (3)
#define TINYGLTF_TYPE_VEC4 (4)
#define TINYGLTF_TYPE_MAT2 (32 + 2)
#define TINYGLTF_TYPE_MAT3 (32 + 3)
#define TINYGLTF_TYPE_MAT4 (32 + 4)
#define TINYGLTF_TYPE_SCALAR (64 + 1)
#define TINYGLTF_TYPE_VECTOR (64 + 4)
#define TINYGLTF_TYPE_MATRIX (64 + 16)

#define TINYGLTF_IMAGE_FORMAT_JPEG (0)
#define TINYGLTF_IMAGE_FORMAT_PNG (1)
#define TINYGLTF_IMAGE_FORMAT_BMP (2)
#define TINYGLTF_IMAGE_FORMAT_GIF (3)

#define TINYGLTF_TEXTURE_FORMAT_ALPHA (6406)
#define TINYGLTF_TEXTURE_FORMAT_RGB (6407)
#define TINYGLTF_TEXTURE_FORMAT_RGBA (6408)
#define TINYGLTF_TEXTURE_FORMAT_LUMINANCE (6409)
#define TINYGLTF_TEXTURE_FORMAT_LUMINANCE_ALPHA (6410)

#define TINYGLTF_TEXTURE_TARGET_TEXTURE2D (3553)
#define TINYGLTF_TEXTURE_TYPE_UNSIGNED_BYTE (5121)

#define TINYGLTF_TARGET_ARRAY_BUFFER (34962)
#define TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER (34963)

#define TINYGLTF_SHADER_TYPE_VERTEX_SHADER (35633)
#define TINYGLTF_SHADER_TYPE_FRAGMENT_SHADER (35632)

typedef enum {
  NULL_TYPE = 0,
  NUMBER_TYPE = 1,
  INT_TYPE = 2,
  BOOL_TYPE = 3,
  STRING_TYPE = 4,
  ARRAY_TYPE = 5,
  BINARY_TYPE = 6,
  OBJECT_TYPE = 7
} Type;

#ifdef __clang__
#pragma clang diagnostic push
// Suppress warning for : static Value null_value
// https://stackoverflow.com/questions/15708411/how-to-deal-with-global-constructor-warning-in-clang
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

// Simple class to represent JSON object
class Value {
 public:
  typedef std::vector<Value> Array;
  typedef std::map<std::string, Value> Object;

  Value() : type_(NULL_TYPE) {}

  explicit Value(bool b) : type_(BOOL_TYPE) { boolean_value_ = b; }
  explicit Value(int i) : type_(INT_TYPE) { int_value_ = i; }
  explicit Value(double n) : type_(NUMBER_TYPE) { number_value_ = n; }
  explicit Value(const std::string &s) : type_(STRING_TYPE) {
    string_value_ = s;
  }
  explicit Value(const unsigned char *p, size_t n) : type_(BINARY_TYPE) {
    binary_value_.resize(n);
    memcpy(binary_value_.data(), p, n);
  }
  explicit Value(const Array &a) : type_(ARRAY_TYPE) {
    array_value_ = Array(a);
  }
  explicit Value(const Object &o) : type_(OBJECT_TYPE) {
    object_value_ = Object(o);
  }

  char Type() const { return static_cast<const char>(type_); }

  bool IsBool() const { return (type_ == BOOL_TYPE); }

  bool IsInt() const { return (type_ == INT_TYPE); }

  bool IsNumber() const { return (type_ == NUMBER_TYPE); }

  bool IsString() const { return (type_ == STRING_TYPE); }

  bool IsBinary() const { return (type_ == BINARY_TYPE); }

  bool IsArray() const { return (type_ == ARRAY_TYPE); }

  bool IsObject() const { return (type_ == OBJECT_TYPE); }

  // Accessor
  template <typename T>
  const T &Get() const;
  template <typename T>
  T &Get();

  // Lookup value from an array
  const Value &Get(int idx) const {
    static Value null_value;
    assert(IsArray());
    assert(idx >= 0);
    return (static_cast<size_t>(idx) < array_value_.size())
               ? array_value_[static_cast<size_t>(idx)]
               : null_value;
  }

  // Lookup value from a key-value pair
  const Value &Get(const std::string &key) const {
    static Value null_value;
    assert(IsObject());
    Object::const_iterator it = object_value_.find(key);
    return (it != object_value_.end()) ? it->second : null_value;
  }

  size_t ArrayLen() const {
    if (!IsArray()) return 0;
    return array_value_.size();
  }

  // Valid only for object type.
  bool Has(const std::string &key) const {
    if (!IsObject()) return false;
    Object::const_iterator it = object_value_.find(key);
    return (it != object_value_.end()) ? true : false;
  }

  // List keys
  std::vector<std::string> Keys() const {
    std::vector<std::string> keys;
    if (!IsObject()) return keys;  // empty

    for (Object::const_iterator it = object_value_.begin();
         it != object_value_.end(); ++it) {
      keys.push_back(it->first);
    }

    return keys;
  }

  size_t Size() const { return (IsArray() ? ArrayLen() : Keys().size()); }

 protected:
  int type_;

  int int_value_;
  double number_value_;
  std::string string_value_;
  std::vector<unsigned char> binary_value_;
  Array array_value_;
  Object object_value_;
  bool boolean_value_;
  char pad[3];

  int pad0;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#define TINYGLTF_VALUE_GET(ctype, var)            \
  template <>                                     \
  inline const ctype &Value::Get<ctype>() const { \
    return var;                                   \
  }                                               \
  template <>                                     \
  inline ctype &Value::Get<ctype>() {             \
    return var;                                   \
  }
TINYGLTF_VALUE_GET(bool, boolean_value_)
TINYGLTF_VALUE_GET(double, number_value_)
TINYGLTF_VALUE_GET(int, int_value_)
TINYGLTF_VALUE_GET(std::string, string_value_)
TINYGLTF_VALUE_GET(std::vector<unsigned char>, binary_value_)
TINYGLTF_VALUE_GET(Value::Array, array_value_)
TINYGLTF_VALUE_GET(Value::Object, object_value_)
#undef TINYGLTF_VALUE_GET
/// Agregate object for representing a color
using ColorValue = std::array<double, 4>;
struct Parameter {
  bool bool_value;
  bool has_number_value = false;
  std::string string_value;
  std::vector<double> number_array;
  std::map<std::string, double> json_double_value;

  double number_value;
  // context sensitive methods. depending the type of the Parameter you are
  // accessing, these are either valid or not
  // If this parameter represent a texture map in a material, will return the
  // texture index

  /// Return the index of a texture if this Parameter is a texture map.
  /// Returned value is only valid if the parameter represent a texture from a
  /// material
  int TextureIndex() const {
    const auto it = json_double_value.find("index");
    if (it != std::end(json_double_value)) {
      return int(it->second);
    }
    return -1;
  }

  /// Material factor, like the roughness or metalness of a material
  /// Returned value is only valid if the parameter represent a texture from a
  /// material
  double Factor() const { return number_value; }

  /// Return the color of a material
  /// Returned value is only valid if the parameter represent a texture from a
  /// material
  ColorValue ColorFactor() const {
    return {
        {// this agregate intialize the std::array object, and uses C++11 RVO.
         number_array[0], number_array[1], number_array[2],
         (number_array.size() > 3 ? number_array[3] : 1.0)}};
  }
};

typedef std::map<std::string, Parameter> ParameterMap;

struct AnimationChannel {
  int sampler;              // required
  int target_node;          // required (index of the node to target)
  std::string target_path;  // required in ["translation", "rotation", "scale",
                            // "weights"]
  Value extras;

  AnimationChannel() : sampler(-1), target_node(-1) {}
};

struct AnimationSampler {
  int input;                  // required
  int output;                 // required
  std::string interpolation;  // in ["LINEAR", "STEP", "CATMULLROMSPLINE",
                              // "CUBICSPLINE"], default "LINEAR"

  AnimationSampler() : input(-1), output(-1), interpolation("LINEAR") {}
};

typedef struct {
  std::string name;
  std::vector<AnimationChannel> channels;
  std::vector<AnimationSampler> samplers;
  Value extras;
} Animation;

struct Skin {
  std::string name;
  int inverseBindMatrices;  // required here but not in the spec
  int skeleton;             // The index of the node used as a skeleton root
  std::vector<int> joints;  // Indices of skeleton nodes

  Skin() {
    inverseBindMatrices = -1;
    skeleton = -1;
  }
};

struct Sampler {
  std::string name;
  int minFilter;  // ["NEAREST", "LINEAR", "NEAREST_MIPMAP_LINEAR",
                  // "LINEAR_MIPMAP_NEAREST", "NEAREST_MIPMAP_LINEAR",
                  // "LINEAR_MIPMAP_LINEAR"]
  int magFilter;  // ["NEAREST", "LINEAR"]
  int wrapS;      // ["CLAMP_TO_EDGE", "MIRRORED_REPEAT", "REPEAT"], default
                  // "REPEAT"
  int wrapT;      // ["CLAMP_TO_EDGE", "MIRRORED_REPEAT", "REPEAT"], default
                  // "REPEAT"
  int wrapR;      // TinyGLTF extension
  int pad0;
  Value extras;

  Sampler()
      : wrapS(TINYGLTF_TEXTURE_WRAP_RPEAT),
        wrapT(TINYGLTF_TEXTURE_WRAP_RPEAT) {}
};

struct Image {
  std::string name;
  int width;
  int height;
  int component;
  int pad0;
  std::vector<unsigned char> image;
  int bufferView;        // (required if no uri)
  std::string mimeType;  // (required if no uri) ["image/jpeg", "image/png"]
  std::string uri;       // (reqiored if no mimeType)
  Value extras;

  Image() { bufferView = -1; }
};

struct Texture {
  int sampler;
  int source;  // Required (not specified in the spec ?)
  Value extras;

  Texture() : sampler(-1), source(-1) {}
};

// Each extension should be stored in a ParameterMap.
// members not in the values could be included in the ParameterMap
// to keep a single material model
struct Material {
  std::string name;

  ParameterMap values;            // PBR metal/roughness workflow
  ParameterMap additionalValues;  // normal/occlusion/emissive values
  ParameterMap extCommonValues;   // KHR_common_material extension
  ParameterMap extPBRValues;
  Value extras;
};

struct BufferView {
  std::string name;
  int buffer;         // Required
  size_t byteOffset;  // minimum 0, default 0
  size_t byteLength;  // required, minimum 1
  size_t byteStride;  // minimum 4, maximum 252 (multiple of 4), default 0 =
                      // understood to be tightly packed
  int target;         // ["ARRAY_BUFFER", "ELEMENT_ARRAY_BUFFER"]
  int pad0;
  Value extras;

  BufferView() : byteOffset(0), byteStride(0) {}
};

struct Accessor {
  int bufferView;  // optional in spec but required here since sparse accessor
                   // are not supported
  std::string name;
  size_t byteOffset;
  bool normalized;    // optinal.
  int componentType;  // (required) One of TINYGLTF_COMPONENT_TYPE_***
  size_t count;       // required
  int type;           // (required) One of TINYGLTF_TYPE_***   ..
  Value extras;

  std::vector<double> minValues;  // optional
  std::vector<double> maxValues;  // optional

  // TODO(syoyo): "sparse"

  Accessor() { bufferView = -1; }
};

struct PerspectiveCamera {
  float aspectRatio;  // min > 0
  float yfov;         // required. min > 0
  float zfar;         // min > 0
  float znear;        // required. min > 0

  PerspectiveCamera()
      : aspectRatio(0.0f),
        yfov(0.0f),
        zfar(0.0f)  // 0 = use infinite projecton matrix
        ,
        znear(0.0f) {}

  ParameterMap extensions;
  Value extras;
};

struct OrthographicCamera {
  float xmag;   // required. must not be zero.
  float ymag;   // required. must not be zero.
  float zfar;   // required. `zfar` must be greater than `znear`.
  float znear;  // required

  OrthographicCamera() : xmag(0.0f), ymag(0.0f), zfar(0.0f), znear(0.0f) {}

  ParameterMap extensions;
  Value extras;
};

struct Camera {
  std::string type;  // required. "perspective" or "orthographic"
  std::string name;

  PerspectiveCamera perspective;
  OrthographicCamera orthographic;

  Camera() {}

  ParameterMap extensions;
  Value extras;
};

struct Primitive {
  std::map<std::string, int> attributes;  // (required) A dictionary object of
                                          // integer, where each integer
                                          // is the index of the accessor
                                          // containing an attribute.
  int material;  // The index of the material to apply to this primitive
                 // when rendering.
  int indices;   // The index of the accessor that contains the indices.
  int mode;      // one of TINYGLTF_MODE_***
  std::vector<std::map<std::string, int> > targets;  // array of morph targets,
  // where each target is a dict with attribues in ["POSITION, "NORMAL",
  // "TANGENT"] pointing
  // to their corresponding accessors
  Value extras;

  Primitive() {
    material = -1;
    indices = -1;
  }
};

typedef struct {
  std::string name;
  std::vector<Primitive> primitives;
  std::vector<double> weights;  // weights to be applied to the Morph Targets
  std::vector<std::map<std::string, int> > targets;
  ParameterMap extensions;
  Value extras;
} Mesh;

class Node {
 public:
  Node() : camera(-1), skin(-1), mesh(-1) {}

  ~Node() {}

  int camera;  // the index of the camera referenced by this node

  std::string name;
  int skin;
  int mesh;
  std::vector<int> children;
  std::vector<double> rotation;     // length must be 0 or 4
  std::vector<double> scale;        // length must be 0 or 3
  std::vector<double> translation;  // length must be 0 or 3
  std::vector<double> matrix;       // length must be 0 or 16
  std::vector<double> weights;  // The weights of the instantiated Morph Target

  Value extras;
  ParameterMap extLightsValues;      // KHR_lights_cmn extension
};

typedef struct {
  std::string name;
  std::vector<unsigned char> data;
  std::string
      uri;  // considered as required here but not in the spec (need to clarify)
  Value extras;
} Buffer;

typedef struct {
  std::string version;  // required
  std::string generator;
  std::string minVersion;
  std::string copyright;
  ParameterMap extensions;
  Value extras;
} Asset;

struct Scene {
  std::string name;
  std::vector<int> nodes;

  ParameterMap extensions;
  ParameterMap extras;
};

struct Light {
  std::string name;
  std::vector<double> color;
  std::string type;
};

class Model {
 public:
  Model() {}
  ~Model() {}

  std::vector<Accessor> accessors;
  std::vector<Animation> animations;
  std::vector<Buffer> buffers;
  std::vector<BufferView> bufferViews;
  std::vector<Material> materials;
  std::vector<Mesh> meshes;
  std::vector<Node> nodes;
  std::vector<Texture> textures;
  std::vector<Image> images;
  std::vector<Skin> skins;
  std::vector<Sampler> samplers;
  std::vector<Camera> cameras;
  std::vector<Scene> scenes;
  std::vector<Light> lights;

  int defaultScene;
  std::vector<std::string> extensionsUsed;
  std::vector<std::string> extensionsRequired;

  Asset asset;

  Value extras;
};

enum SectionCheck {
  NO_REQUIRE = 0x00,
  REQUIRE_SCENE = 0x01,
  REQUIRE_SCENES = 0x02,
  REQUIRE_NODES = 0x04,
  REQUIRE_ACCESSORS = 0x08,
  REQUIRE_BUFFERS = 0x10,
  REQUIRE_BUFFER_VIEWS = 0x20,
  REQUIRE_ALL = 0x3f
};


class IFileLoader
{
  public:
    virtual ~IFileLoader(){}
    virtual bool loadExternalFile(std::vector<unsigned char> *out, std::string *err,
                                 const std::string &filename,
                                 const std::string &basedir, size_t reqBytes,
                                 bool checkSize) = 0;
};


class TinyGLTF {
 public:
  TinyGLTF() : bin_data_(NULL), bin_size_(0), is_binary_(false) {
    pad[0] = pad[1] = pad[2] = pad[3] = pad[4] = pad[5] = pad[6] = 0;
  }
  ~TinyGLTF() {}

  ///
  /// Loads glTF ASCII asset from a file.
  /// Returns false and set error string to `err` if there's an error.
  ///
  bool LoadASCIIFromFile(        
     IFileLoader& fileLoader,
     Model *model, std::string *err,
     const std::string &filename,
     unsigned int check_sections = REQUIRE_ALL);

  ///
  /// Loads glTF ASCII asset from string(memory).
  /// `length` = strlen(str);
  /// Returns false and set error string to `err` if there's an error.
  ///
  bool LoadASCIIFromString(
          IFileLoader& fileLoader,
          Model *model, std::string *err, const char *str,
          const unsigned int length,
          const std::string &base_dir,
          unsigned int check_sections = REQUIRE_ALL);

  ///
  /// Loads glTF binary asset from a file.
  /// Returns false and set error string to `err` if there's an error.
  ///
  bool LoadBinaryFromFile(        
          IFileLoader& fileLoader,
          Model *model, std::string *err,
          const std::string &filename,
          unsigned int check_sections = REQUIRE_ALL);

  ///
  /// Loads glTF binary asset from memory.
  /// `length` = strlen(str);
  /// Returns false and set error string to `err` if there's an error.
  ///
  bool LoadBinaryFromMemory(
          IFileLoader& fileLoader,
          Model *model, std::string *err,
                            const unsigned char *bytes,
                            const unsigned int length,
                            const std::string &base_dir = "",
                            unsigned int check_sections = REQUIRE_ALL);

  ///
  /// Write glTF to file.
  ///
  bool WriteGltfSceneToFile(
      Model *model,
      const std::string &
          filename /*, bool embedImages, bool embedBuffers, bool writeBinary*/);

 private:
  ///
  /// Loads glTF asset from string(memory).
  /// `length` = strlen(str);
  /// Returns false and set error string to `err` if there's an error.
  ///
  bool LoadFromString(
          IFileLoader& fileLoader,
          Model *model, std::string *err, const char *str,
                      const unsigned int length, const std::string &base_dir,
                      unsigned int check_sections);

  const unsigned char *bin_data_;
  size_t bin_size_;
  bool is_binary_;
  char pad[7];
};

}  // namespace tinygltf

#endif  // TINY_GLTF_H_

#ifdef TINYGLTF_IMPLEMENTATION
#include <algorithm>
//#include <cassert>
#include <fstream>
#include <sstream>

#ifdef __clang__
// Disable some warnings for external files.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wpadded"
#if __has_warning("-Wcomma")
#pragma clang diagnostic ignored "-Wcomma"
#endif
#endif

#define PICOJSON_USE_INT64
#include "./picojson.h"
#ifndef TINYGLTF_NO_STB_IMAGE
#include "./stb_image.h"
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _WIN32
#include <windows.h>
#elif !defined(__ANDROID__)
#include <wordexp.h>
#endif

#if defined(__sparcv9)
// Big endian
#else
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || MINIZ_X86_OR_X64_CPU
#define TINYGLTF_LITTLE_ENDIAN 1
#endif
#endif

#if __APPLE__
#include "TargetConditionals.h"
#endif

namespace tinygltf {

static void swap4(unsigned int *val) {
#ifdef TINYGLTF_LITTLE_ENDIAN
  (void)val;
#else
  unsigned int tmp = *val;
  unsigned char *dst = reinterpret_cast<unsigned char *>(val);
  unsigned char *src = reinterpret_cast<unsigned char *>(&tmp);

  dst[0] = src[3];
  dst[1] = src[2];
  dst[2] = src[1];
  dst[3] = src[0];
#endif
}

static bool FileExists(const std::string &abs_filename) {
  bool ret;
#ifdef _WIN32
  FILE *fp;
  errno_t err = fopen_s(&fp, abs_filename.c_str(), "rb");
  if (err != 0) {
    return false;
  }
#else
  FILE *fp = fopen(abs_filename.c_str(), "rb");
#endif
  if (fp) {
    ret = true;
    fclose(fp);
  } else {
    ret = false;
  }

  return ret;
}

static std::string ExpandFilePath(const std::string &filepath) {
#ifdef _WIN32
  DWORD len = ExpandEnvironmentStringsA(filepath.c_str(), NULL, 0);
  char *str = new char[len];
  ExpandEnvironmentStringsA(filepath.c_str(), str, len);

  std::string s(str);

  delete[] str;

  return s;
#else

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(__ANDROID__)
  // no expansion
  std::string s = filepath;
#else
  std::string s;
  wordexp_t p;

  if (filepath.empty()) {
    return "";
  }

  // char** w;
  int ret = wordexp(filepath.c_str(), &p, 0);
  if (ret) {
    // err
    s = filepath;
    return s;
  }

  // Use first element only.
  if (p.we_wordv) {
    s = std::string(p.we_wordv[0]);
    wordfree(&p);
  } else {
    s = filepath;
  }

#endif

  return s;
#endif
}

static std::string JoinPath(const std::string &path0,
                            const std::string &path1) {
  if (path0.empty()) {
    return path1;
  } else {
    // check '/'
    char lastChar = *path0.rbegin();
    if (lastChar != '/') {
      return path0 + std::string("/") + path1;
    } else {
      return path0 + path1;
    }
  }
}

static std::string FindFile(const std::vector<std::string> &paths,
                            const std::string &filepath) {
  for (size_t i = 0; i < paths.size(); i++) {
    std::string absPath = ExpandFilePath(JoinPath(paths[i], filepath));
    if (FileExists(absPath)) {
      return absPath;
    }
  }

  return std::string();
}

// std::string GetFilePathExtension(const std::string& FileName)
//{
//    if(FileName.find_last_of(".") != std::string::npos)
//        return FileName.substr(FileName.find_last_of(".")+1);
//    return "";
//}

static std::string GetBaseDir(const std::string &filepath) {
  if (filepath.find_last_of("/\\") != std::string::npos)
    return filepath.substr(0, filepath.find_last_of("/\\"));
  return "";
}

// std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const &s);

/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wconversion"
#endif
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(std::string const &encoded_string) {
  int in_len = static_cast<int>(encoded_string.size());
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && (encoded_string[in_] != '=') &&
         is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_];
    in_++;
    if (i == 4) {
      for (i = 0; i < 4; i++)
        char_array_4[i] =
            static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

      char_array_3[0] =
          (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] =
          ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++) ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 4; j++) char_array_4[j] = 0;

    for (j = 0; j < 4; j++)
      char_array_4[j] =
          static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] =
        ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifndef TINYGLTF_NO_STB_IMAGE
static bool LoadImageData(Image *image, std::string *err, int req_width,
                          int req_height, const unsigned char *bytes,
                          int size) {
  std::cout << "size " << size << std::endl;

  int w, h, comp;
  // if image cannot be decoded, ignore parsing and keep it by its path
  // don't break in this case
  // FIXME we should only enter this function if the image is embedded. If
  // image->uri references
  // an image file, it should be left as it is. Image loading should not be
  // mandatory (to support other formats)
  unsigned char *data = stbi_load_from_memory(bytes, size, &w, &h, &comp, 0);
  if (!data) {
    if (err) {
      (*err) += "Unknown image format.\n";
    }
    return true;
  }

  if (w < 1 || h < 1) {
    free(data);
    if (err) {
      (*err) += "Invalid image data.\n";
    }
    return true;
  }

  if (req_width > 0) {
    if (req_width != w) {
      free(data);
      if (err) {
        (*err) += "Image width mismatch.\n";
      }
      return false;
    }
  }

  if (req_height > 0) {
    if (req_height != h) {
      free(data);
      if (err) {
        (*err) += "Image height mismatch.\n";
      }
      return false;
    }
  }

  image->width = w;
  image->height = h;
  image->component = comp;
  image->image.resize(static_cast<size_t>(w * h * comp));
  std::copy(data, data + w * h * comp, image->image.begin());

  free(data);

  return true;
}
#endif
static bool IsDataURI(const std::string &in) {
  std::string header = "data:application/octet-stream;base64,";
  if (in.find(header) == 0) {
    return true;
  }

  header = "data:image/png;base64,";
  if (in.find(header) == 0) {
    return true;
  }

  header = "data:image/jpeg;base64,";
  if (in.find(header) == 0) {
    return true;
  }

  header = "data:text/plain;base64,";
  if (in.find(header) == 0) {
    return true;
  }

  return false;
}

static bool DecodeDataURI(std::vector<unsigned char> *out,
                          const std::string &in, size_t reqBytes,
                          bool checkSize) {
  std::string header = "data:application/octet-stream;base64,";
  std::string data;
  if (in.find(header) == 0) {
    data = base64_decode(in.substr(header.size()));  // cut mime string.
  }

  if (data.empty()) {
    header = "data:image/jpeg;base64,";
    if (in.find(header) == 0) {
      data = base64_decode(in.substr(header.size()));  // cut mime string.
    }
  }

  if (data.empty()) {
    header = "data:image/png;base64,";
    if (in.find(header) == 0) {
      data = base64_decode(in.substr(header.size()));  // cut mime string.
    }
  }

  if (data.empty()) {
    header = "data:text/plain;base64,";
    if (in.find(header) == 0) {
      data = base64_decode(in.substr(header.size()));
    }
  }

  if (data.empty()) {
    return false;
  }

  if (checkSize) {
    if (data.size() != reqBytes) {
      return false;
    }
    out->resize(reqBytes);
  } else {
    out->resize(data.size());
  }
  std::copy(data.begin(), data.end(), out->begin());
  return true;
}

static void ParseObjectProperty(Value *ret, const picojson::object &o) {
  tinygltf::Value::Object vo;
  picojson::object::const_iterator it(o.begin());
  picojson::object::const_iterator itEnd(o.end());

  for (; it != itEnd; it++) {
    picojson::value v = it->second;

    if (v.is<bool>()) {
      vo[it->first] = tinygltf::Value(v.get<bool>());
    } else if (v.is<double>()) {
      vo[it->first] = tinygltf::Value(v.get<double>());
    } else if (v.is<int64_t>()) {
      vo[it->first] =
          tinygltf::Value(static_cast<int>(v.get<int64_t>()));  // truncate
    } else if (v.is<std::string>()) {
      vo[it->first] = tinygltf::Value(v.get<std::string>());
    } else if (v.is<picojson::object>()) {
      tinygltf::Value child_value;
      ParseObjectProperty(&child_value, v.get<picojson::object>());
      vo[it->first] = child_value;
      }
    // TODO(syoyo) binary, array
      }

  (*ret) = tinygltf::Value(vo);
  }

static bool ParseExtrasProperty(Value *ret, const picojson::object &o) {
  picojson::object::const_iterator it = o.find("extras");
  if (it == o.end()) {
    return false;
}

  // FIXME(syoyo) Currently we only support `object` type for extras property.
  if (!it->second.is<picojson::object>()) {
    return false;
  }

  ParseObjectProperty(ret, it->second.get<picojson::object>());

  return true;
}

static bool ParseBooleanProperty(bool *ret, std::string *err,
                                 const picojson::object &o,
                                 const std::string &property,
                                 const bool required,
                                 const std::string &parent_node = "") {
  picojson::object::const_iterator it = o.find(property);
  if (it == o.end()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is missing";
        if (!parent_node.empty()) {
          (*err) += " in " + parent_node;
        }
        (*err) += ".\n";
      }
    }
    return false;
  }

  if (!it->second.is<bool>()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is not a bool type.\n";
      }
    }
    return false;
  }

  if (ret) {
    (*ret) = it->second.get<bool>();
  }

  return true;
}

static bool ParseNumberProperty(double *ret, std::string *err,
                                const picojson::object &o,
                                const std::string &property,
                                const bool required,
                                const std::string &parent_node = "") {
  picojson::object::const_iterator it = o.find(property);
  if (it == o.end()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is missing";
        if (!parent_node.empty()) {
          (*err) += " in " + parent_node;
        }
        (*err) += ".\n";
      }
    }
    return false;
  }

  if (!it->second.is<double>()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is not a number type.\n";
      }
    }
    return false;
  }

  if (ret) {
    (*ret) = it->second.get<double>();
  }

  return true;
}

static bool ParseNumberArrayProperty(std::vector<double> *ret, std::string *err,
                                     const picojson::object &o,
                                     const std::string &property, bool required,
                                     const std::string &parent_node = "") {
  picojson::object::const_iterator it = o.find(property);
  if (it == o.end()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is missing";
        if (!parent_node.empty()) {
          (*err) += " in " + parent_node;
        }
        (*err) += ".\n";
      }
    }
    return false;
  }

  if (!it->second.is<picojson::array>()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is not an array";
        if (!parent_node.empty()) {
          (*err) += " in " + parent_node;
        }
        (*err) += ".\n";
      }
    }
    return false;
  }

  ret->clear();
  const picojson::array &arr = it->second.get<picojson::array>();
  for (size_t i = 0; i < arr.size(); i++) {
    if (!arr[i].is<double>()) {
      if (required) {
        if (err) {
          (*err) += "'" + property + "' property is not a number.\n";
          if (!parent_node.empty()) {
            (*err) += " in " + parent_node;
          }
          (*err) += ".\n";
        }
      }
      return false;
    }
    ret->push_back(arr[i].get<double>());
  }

  return true;
}

static bool ParseStringProperty(
    std::string *ret, std::string *err, const picojson::object &o,
    const std::string &property, bool required,
    const std::string &parent_node = std::string()) {
  picojson::object::const_iterator it = o.find(property);
  if (it == o.end()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is missing";
        if (parent_node.empty()) {
          (*err) += ".\n";
        } else {
          (*err) += " in `" + parent_node + "'.\n";
        }
      }
    }
    return false;
  }

  if (!it->second.is<std::string>()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is not a string type.\n";
      }
    }
    return false;
  }

  if (ret) {
    (*ret) = it->second.get<std::string>();
  }

  return true;
}

static bool ParseStringIntProperty(std::map<std::string, int> *ret,
                                   std::string *err, const picojson::object &o,
                                   const std::string &property, bool required, const std::string &parent = "") {
  picojson::object::const_iterator it = o.find(property);
  if (it == o.end()) {
    if (required) {
      if (err) {
        if (!parent.empty()) {
          (*err) += "'" + property + "' property is missing in " + parent + ".\n";
        } else {
          (*err) += "'" + property + "' property is missing.\n";
        }
      }
    }
    return false;
  }

  // Make sure we are dealing with an object / dictionary.
  if (!it->second.is<picojson::object>()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is not an object.\n";
      }
    }
    return false;
  }

  ret->clear();
  const picojson::object &dict = it->second.get<picojson::object>();

  picojson::object::const_iterator dictIt(dict.begin());
  picojson::object::const_iterator dictItEnd(dict.end());

  for (; dictIt != dictItEnd; ++dictIt) {
    if (!dictIt->second.is<double>()) {
      if (required) {
        if (err) {
          (*err) += "'" + property + "' value is not an int.\n";
        }
      }
      return false;
    }

    // Insert into the list.
    (*ret)[dictIt->first] = static_cast<int>(dictIt->second.get<double>());
  }
  return true;
}

static bool ParseJSONProperty(std::map<std::string, double> *ret,
                              std::string *err, const picojson::object &o,
                              const std::string &property, bool required) {
  picojson::object::const_iterator it = o.find(property);
  if (it == o.end()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is missing. \n'";
      }
    }
    return false;
  }

  if (!it->second.is<picojson::object>()) {
    if (required) {
      if (err) {
        (*err) += "'" + property + "' property is not a JSON object.\n";
      }
    }
    return false;
  }

  ret->clear();
  const picojson::object &obj = it->second.get<picojson::object>();
  picojson::object::const_iterator it2(obj.begin());
  picojson::object::const_iterator itEnd(obj.end());
  for (; it2 != itEnd; it2++) {
    if (it2->second.is<double>())
      ret->insert(std::pair<std::string, double>(it2->first,
                                                 it2->second.get<double>()));
  }

  return true;
}

static bool ParseAsset(Asset *asset, std::string *err,
                       const picojson::object &o) {
  ParseStringProperty(&asset->version, err, o, "version", true, "Asset");
  ParseStringProperty(&asset->generator, err, o, "generator", false, "Asset");
  ParseStringProperty(&asset->minVersion, err, o, "minVersion", false, "Asset");

  // Unity exporter version is added as extra here
  ParseExtrasProperty(&(asset->extras), o);

  return true;
}

static bool ParseImage(IFileLoader& fileLoader, Image *image, std::string *err,
                       const picojson::object &o, const std::string &basedir,
                       bool is_binary, const unsigned char *bin_data,
                       size_t bin_size) {
  // A glTF image must either reference a bufferView or an image uri
  double bufferView = -1;
  bool isEmbedded =
      ParseNumberProperty(&bufferView, err, o, "bufferView", false);

  std::string uri;
  std::string tmp_err;
  if (!ParseStringProperty(&uri, &tmp_err, o, "uri", false) && !isEmbedded) {
    if (err) {
      (*err) += "`bufferView` or `uri` required for Image.\n";
    }
    return false;
  }

  ParseStringProperty(&image->name, err, o, "name", false);

  std::vector<unsigned char> img;

  if (is_binary) {
    // Still binary glTF accepts external dataURI. First try external resources.
    bool loaded = false;
    if (IsDataURI(uri)) {
      loaded = DecodeDataURI(&img, uri, 0, false);
    } else {
      // Assume external .bin file.
      loaded = fileLoader.loadExternalFile(&img, err, uri, basedir, 0, false);
    }

    if (!loaded) {
      // load data from (embedded) binary data

      if ((bin_size == 0) || (bin_data == NULL)) {
    if (err) {
          (*err) += "Invalid binary data.\n";
    }
    return false;
  }

      double buffer_view = -1.0;
      if (!ParseNumberProperty(&buffer_view, err, o, "bufferView", true, "Image")) {
      return false;
    }

    std::string mime_type;
    ParseStringProperty(&mime_type, err, o, "mimeType", false);

    double width = 0.0;
    ParseNumberProperty(&width, err, o, "width", false);

    double height = 0.0;
    ParseNumberProperty(&height, err, o, "height", false);

    // Just only save some information here. Loading actual image data from
      // bufferView is done in other place.
      image->bufferView = static_cast<int>(buffer_view);
    image->mimeType = mime_type;
    image->width = static_cast<int>(width);
    image->height = static_cast<int>(height);

    return true;
  }
  } else {
  if (IsDataURI(uri)) {
      if (!DecodeDataURI(&img, uri, 0, false)) {
      if (err) {
        (*err) += "Failed to decode 'uri' for image parameter.\n";
      }
      return false;
    }
  } else {
    // Assume external file

    // Keep texture path (for textures that cannot be decoded)
    image->uri = uri;
#ifdef TINYGLTF_NO_EXTERNAL_IMAGE
    return true;
#endif
    if (!fileLoader.loadExternalFile(&img, err, uri, basedir, 0, false)) {
      if (err) {
        (*err) += "Failed to load external 'uri' for image parameter\n";
      }
      // If the image cannot be loaded, keep uri as image->uri.
      return true;
    }
    if (img.empty()) {
      if (err) {
        (*err) += "Image is empty.\n";
      }
      return false;
    }
  }
    }
#ifndef TINYGLTF_NO_STB_IMAGE
  return LoadImageData(image, err, 0, 0, &img.at(0),
                       static_cast<int>(img.size()));
#else
  return true;
#endif
  }

static bool ParseTexture(Texture *texture, std::string *err,
                         const picojson::object &o,
                         const std::string &basedir) {
  (void)basedir;
  double sampler = -1.0;
  double source = -1.0;
  ParseNumberProperty(&sampler, err, o, "sampler", false);

  ParseNumberProperty(&source, err, o, "source", false);

  texture->sampler = static_cast<int>(sampler);
  texture->source = static_cast<int>(source);

  return true;
}

static bool ParseBuffer(
IFileLoader& fileLoader,
Buffer *buffer, std::string *err,
                        const picojson::object &o, const std::string &basedir,
                        bool is_binary = false,
                        const unsigned char *bin_data = NULL,
                        size_t bin_size = 0) {
  double byteLength;
  if (!ParseNumberProperty(&byteLength, err, o, "byteLength", true, "Buffer")) {
    return false;
  }

  // In glTF 2.0, uri is not mandatory anymore
  std::string uri;
  ParseStringProperty(&uri, err, o, "uri", false, "Buffer");

  // having an empty uri for a non embedded image should not be valid
  if (!is_binary && uri.empty()) {
    if (err) {
      (*err) += "'uri' is missing from non binary glTF file buffer.\n";
    }
  }

  picojson::object::const_iterator type = o.find("type");
  if (type != o.end()) {
    if (type->second.is<std::string>()) {
      const std::string &ty = (type->second).get<std::string>();
      if (ty.compare("arraybuffer") == 0) {
        // buffer.type = "arraybuffer";
      }
    }
  }

  size_t bytes = static_cast<size_t>(byteLength);
  if (is_binary) {
    // Still binary glTF accepts external dataURI. First try external resources.

    if (!uri.empty()) {
      // External .bin file.
      fileLoader.loadExternalFile(&buffer->data, err, uri, basedir, bytes, true);
    } else {
      // load data from (embedded) binary data

      if ((bin_size == 0) || (bin_data == NULL)) {
        if (err) {
          (*err) += "Invalid binary data in `Buffer'.\n";
        }
        return false;
      }

      if (byteLength > bin_size) {
        if (err) {
          std::stringstream ss;
          ss << "Invalid `byteLength'. Must be equal or less than binary size: "
                "`byteLength' = "
             << byteLength << ", binary size = " << bin_size << std::endl;
          (*err) += ss.str();
        }
        return false;
      }

      // Read buffer data
      buffer->data.resize(static_cast<size_t>(byteLength));
      memcpy(&(buffer->data.at(0)), bin_data, static_cast<size_t>(byteLength));
    }

  } else {
    if (IsDataURI(uri)) {
      if (!DecodeDataURI(&buffer->data, uri, bytes, true)) {
        if (err) {
          (*err) += "Failed to decode 'uri' : " + uri + " in Buffer\n";
        }
        return false;
      }
    } else {
      // Assume external .bin file.
      if (!fileLoader.loadExternalFile(&buffer->data, err, uri, basedir, bytes, true)) {
        return false;
      }
    }
  }

  ParseStringProperty(&buffer->name, err, o, "name", false);

  return true;
}

static bool ParseBufferView(BufferView *bufferView, std::string *err,
                            const picojson::object &o) {
  double buffer = -1.0;
  if (!ParseNumberProperty(&buffer, err, o, "buffer", true, "BufferView")) {
    return false;
  }

  double byteOffset = 0.0;
  ParseNumberProperty(&byteOffset, err, o, "byteOffset", false);

  double byteLength = 1.0;
  if (!ParseNumberProperty(&byteLength, err, o, "byteLength", true,
                           "BufferView")) {
    return false;
  }

  size_t byteStride = 0;
  double byteStrideValue = 0.0;
  if (!ParseNumberProperty(&byteStrideValue, err, o, "byteStride", false)) {
    // Spec says: When byteStride of referenced bufferView is not defined, it
    // means that accessor elements are tightly packed, i.e., effective stride
    // equals the size of the element.
    // We cannot determine the actual byteStride until Accessor are parsed, thus
    // set 0(= tightly packed) here(as done in OpenGL's VertexAttribPoiner)
    byteStride = 0;
  } else {
    byteStride = static_cast<size_t>(byteStrideValue);
  }

  if ((byteStride > 252) || ((byteStride % 4) != 0)) {
    if (err) {
      std::stringstream ss;
      ss << "Invalid `byteStride' value. `byteStride' must be the multiple of "
            "4 : "
         << byteStride << std::endl;

      (*err) += ss.str();
    }
    return false;
  }

  double target = 0.0;
  ParseNumberProperty(&target, err, o, "target", false);
  int targetValue = static_cast<int>(target);
  if ((targetValue == TINYGLTF_TARGET_ARRAY_BUFFER) ||
      (targetValue == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER)) {
    // OK
  } else {
    targetValue = 0;
  }
  bufferView->target = targetValue;

  ParseStringProperty(&bufferView->name, err, o, "name", false);

  bufferView->buffer = static_cast<int>(buffer);
  bufferView->byteOffset = static_cast<size_t>(byteOffset);
  bufferView->byteLength = static_cast<size_t>(byteLength);
  bufferView->byteStride = static_cast<size_t>(byteStride);

  return true;
}

static bool ParseAccessor(Accessor *accessor, std::string *err,
                          const picojson::object &o) {
  double bufferView = -1.0;
  if (!ParseNumberProperty(&bufferView, err, o, "bufferView", true,
                           "Accessor")) {
    return false;
  }

  double byteOffset = 0.0;
  ParseNumberProperty(&byteOffset, err, o, "byteOffset", false, "Accessor");

  bool normalized = false;
  ParseBooleanProperty(&normalized, err, o, "normalized", false, "Accessor");

  double componentType = 0.0;
  if (!ParseNumberProperty(&componentType, err, o, "componentType", true,
                           "Accessor")) {
    return false;
  }

  double count = 0.0;
  if (!ParseNumberProperty(&count, err, o, "count", true, "Accessor")) {
    return false;
  }

  std::string type;
  if (!ParseStringProperty(&type, err, o, "type", true, "Accessor")) {
    return false;
  }

  if (type.compare("SCALAR") == 0) {
    accessor->type = TINYGLTF_TYPE_SCALAR;
  } else if (type.compare("VEC2") == 0) {
    accessor->type = TINYGLTF_TYPE_VEC2;
  } else if (type.compare("VEC3") == 0) {
    accessor->type = TINYGLTF_TYPE_VEC3;
  } else if (type.compare("VEC4") == 0) {
    accessor->type = TINYGLTF_TYPE_VEC4;
  } else if (type.compare("MAT2") == 0) {
    accessor->type = TINYGLTF_TYPE_MAT2;
  } else if (type.compare("MAT3") == 0) {
    accessor->type = TINYGLTF_TYPE_MAT3;
  } else if (type.compare("MAT4") == 0) {
    accessor->type = TINYGLTF_TYPE_MAT4;
  } else {
    std::stringstream ss;
    ss << "Unsupported `type` for accessor object. Got \"" << type << "\"\n";
    if (err) {
      (*err) += ss.str();
    }
    return false;
  }

  ParseStringProperty(&accessor->name, err, o, "name", false);

  accessor->minValues.clear();
  accessor->maxValues.clear();
  ParseNumberArrayProperty(&accessor->minValues, err, o, "min", false,
                           "Accessor");

  ParseNumberArrayProperty(&accessor->maxValues, err, o, "max", false,
                           "Accessor");

  accessor->count = static_cast<size_t>(count);
  accessor->bufferView = static_cast<int>(bufferView);
  accessor->byteOffset = static_cast<size_t>(byteOffset);
  accessor->normalized = normalized;
  {
    int comp = static_cast<int>(componentType);
    if (comp >= TINYGLTF_COMPONENT_TYPE_BYTE &&
        comp <= TINYGLTF_COMPONENT_TYPE_DOUBLE) {
      // OK
      accessor->componentType = comp;
    } else {
      std::stringstream ss;
      ss << "Invalid `componentType` in accessor. Got " << comp << "\n";
      if (err) {
        (*err) += ss.str();
      }
      return false;
    }
  }

  ParseExtrasProperty(&(accessor->extras), o);

  return true;
}

static bool ParsePrimitive(Primitive *primitive, std::string *err,
                           const picojson::object &o) {
  double material = -1.0;
  ParseNumberProperty(&material, err, o, "material", false);
  primitive->material = static_cast<int>(material);

  double mode = static_cast<double>(TINYGLTF_MODE_TRIANGLES);
  ParseNumberProperty(&mode, err, o, "mode", false);

  int primMode = static_cast<int>(mode);
  primitive->mode = primMode;  // Why only triangled were supported ?

  double indices = -1.0;
  ParseNumberProperty(&indices, err, o, "indices", false);
  primitive->indices = static_cast<int>(indices);
  if (!ParseStringIntProperty(&primitive->attributes, err, o, "attributes",
                              true, "Primitive")) {
    return false;
  }

  // Look for morph targets
  picojson::object::const_iterator targetsObject = o.find("targets");
  if ((targetsObject != o.end()) &&
      (targetsObject->second).is<picojson::array>()) {
    const picojson::array &targetArray =
        (targetsObject->second).get<picojson::array>();
    for (size_t i = 0; i < targetArray.size(); i++) {
      std::map<std::string, int> targetAttribues;

      const picojson::object &dict = targetArray[i].get<picojson::object>();
      picojson::object::const_iterator dictIt(dict.begin());
      picojson::object::const_iterator dictItEnd(dict.end());

      for (; dictIt != dictItEnd; ++dictIt) {
        targetAttribues[dictIt->first] =
            static_cast<int>(dictIt->second.get<double>());
      }
      primitive->targets.push_back(targetAttribues);
    }
  }

  ParseExtrasProperty(&(primitive->extras), o);

  return true;
}

static bool ParseMesh(Mesh *mesh, std::string *err, const picojson::object &o) {
  ParseStringProperty(&mesh->name, err, o, "name", false);

  mesh->primitives.clear();
  picojson::object::const_iterator primObject = o.find("primitives");
  if ((primObject != o.end()) && (primObject->second).is<picojson::array>()) {
    const picojson::array &primArray =
        (primObject->second).get<picojson::array>();
    for (size_t i = 0; i < primArray.size(); i++) {
      Primitive primitive;
      if (ParsePrimitive(&primitive, err,
                         primArray[i].get<picojson::object>())) {
        // Only add the primitive if the parsing succeeds.
        mesh->primitives.push_back(primitive);
      }
    }
  }

  // Look for morph targets
  picojson::object::const_iterator targetsObject = o.find("targets");
  if ((targetsObject != o.end()) &&
      (targetsObject->second).is<picojson::array>()) {
    const picojson::array &targetArray =
        (targetsObject->second).get<picojson::array>();
    for (size_t i = 0; i < targetArray.size(); i++) {
      std::map<std::string, int> targetAttribues;

      const picojson::object &dict = targetArray[i].get<picojson::object>();
      picojson::object::const_iterator dictIt(dict.begin());
      picojson::object::const_iterator dictItEnd(dict.end());

      for (; dictIt != dictItEnd; ++dictIt) {
        targetAttribues[dictIt->first] =
            static_cast<int>(dictIt->second.get<double>());
      }
      mesh->targets.push_back(targetAttribues);
    }
  }

  // Should probably check if has targets and if dimensions fit
  ParseNumberArrayProperty(&mesh->weights, err, o, "weights", false);

  ParseExtrasProperty(&(mesh->extras), o);

  return true;
}

static bool ParseParameterProperty(Parameter *param, std::string *err,
                                   const picojson::object &o,
                                   const std::string &prop, bool required) {
    // A parameter value can either be a string or an array of either a boolean or
     // a number. Booleans of any kind aren't supported here. Granted, it
     // complicates the Parameter structure and breaks it semantically in the sense
     // that the client probably works off the assumption that if the string is
     // empty the vector is used, etc. Would a tagged union work?
     if (ParseStringProperty(&param->string_value, err, o, prop, false)) {
       // Found string property.
       return true;
     } else if (ParseNumberArrayProperty(&param->number_array, err, o, prop,
                                         false)) {
       // Found a number array.
       return true;
     } else if (ParseNumberProperty(&param->number_value, err, o, prop, false)) {
       return param->has_number_value = true;
     } else if (ParseJSONProperty(&param->json_double_value, err, o, prop,
                                  false)) {
       return true;
     } else if (ParseBooleanProperty(&param->bool_value, err, o, prop, false)) {
       return true;
     } else {
       if (required) {
         if (err) {
           (*err) += "parameter must be a string or number / number array.\n";
         }
       }
       return false;
     }
}

static bool ParseLight(Light *light, std::string *err, const picojson::object &o) {
  ParseStringProperty(&light->name, err, o, "name", false);
  ParseNumberArrayProperty(&light->color, err, o, "color", false);
  ParseStringProperty(&light->type, err, o, "type", false);
  return true;
}

static bool ParseNode(Node *node, std::string *err, const picojson::object &o) {
  ParseStringProperty(&node->name, err, o, "name", false);

  double skin = -1.0;
  ParseNumberProperty(&skin, err, o, "skin", false);
  node->skin = static_cast<int>(skin);

  // Matrix and T/R/S are exclusive
  if (!ParseNumberArrayProperty(&node->matrix, err, o, "matrix", false)) {
    ParseNumberArrayProperty(&node->rotation, err, o, "rotation", false);
    ParseNumberArrayProperty(&node->scale, err, o, "scale", false);
    ParseNumberArrayProperty(&node->translation, err, o, "translation", false);
  }

  double camera = -1.0;
  ParseNumberProperty(&camera, err, o, "camera", false);
  node->camera = static_cast<int>(camera);

  double mesh = -1.0;
  ParseNumberProperty(&mesh, err, o, "mesh", false);
  node->mesh = int(mesh);

  node->children.clear();
  picojson::object::const_iterator childrenObject = o.find("children");
  if ((childrenObject != o.end()) &&
      (childrenObject->second).is<picojson::array>()) {
    const picojson::array &childrenArray =
        (childrenObject->second).get<picojson::array>();
    for (size_t i = 0; i < childrenArray.size(); i++) {
      if (!childrenArray[i].is<double>()) {
        if (err) {
          (*err) += "Invalid `children` array.\n";
        }
        return false;
      }
      const int &childrenNode =
          static_cast<int>(childrenArray[i].get<double>());
      node->children.push_back(childrenNode);
    }
  }

  ParseExtrasProperty(&(node->extras), o);

  picojson::object::const_iterator extensions_object = o.find("extensions");
  if ((extensions_object != o.end()) &&
      (extensions_object->second).is<picojson::object>()) {
    const picojson::object &values_object =
      (extensions_object->second).get<picojson::object>();

    picojson::object::const_iterator it(values_object.begin());
    picojson::object::const_iterator itEnd(values_object.end());

    for (; it != itEnd; it++) {
      if (it->first == "KHR_lights_cmn" &&
         (it->second).is<picojson::object>()) {
        const picojson::object &values_object =
          (it->second).get<picojson::object>();

        picojson::object::const_iterator itVal(values_object.begin());
        picojson::object::const_iterator itValEnd(values_object.end());

        for (; itVal != itValEnd; itVal++) {
          Parameter param;
          if (ParseParameterProperty(&param, err, values_object, itVal->first,
                false)) {
            node->extLightsValues[itVal->first] = param;
          }
        }
      }
    }
  }

  return true;
}

static bool ParseMaterial(Material *material, std::string *err,
                          const picojson::object &o) {
  material->values.clear();
  material->extPBRValues.clear();
  material->additionalValues.clear();

  picojson::object::const_iterator it(o.begin());
  picojson::object::const_iterator itEnd(o.end());

  for (; it != itEnd; it++) {
    if (it->first == "pbrMetallicRoughness") {
      if ((it->second).is<picojson::object>()) {
        const picojson::object &values_object =
            (it->second).get<picojson::object>();

        picojson::object::const_iterator itVal(values_object.begin());
        picojson::object::const_iterator itValEnd(values_object.end());

        for (; itVal != itValEnd; itVal++) {
          Parameter param;
          if (ParseParameterProperty(&param, err, values_object, itVal->first,
                                     false)) {
            material->values[itVal->first] = param;
          }
        }
      }
    } else if (it->first == "extensions") {
      if ((it->second).is<picojson::object>()) {
        const picojson::object &extension =
            (it->second).get<picojson::object>();

        picojson::object::const_iterator extIt = extension.begin();
        if (!extIt->second.is<picojson::object>()) continue;

        const picojson::object &values_object =
            (extIt->second).get<picojson::object>();

        picojson::object::const_iterator itVal(values_object.begin());
        picojson::object::const_iterator itValEnd(values_object.end());

        for (; itVal != itValEnd; itVal++) {
          Parameter param;
          if (ParseParameterProperty(&param, err, values_object, itVal->first,
                                     false)) {
            material->extPBRValues[itVal->first] = param;
          }
        }
      }
    } else {
      Parameter param;
      if (ParseParameterProperty(&param, err, o, it->first, false)) {
        material->additionalValues[it->first] = param;
      }
    }
  }

  ParseExtrasProperty(&(material->extras), o);

  return true;
}

static bool ParseAnimationChannel(AnimationChannel *channel, std::string *err,
                                  const picojson::object &o) {
  double samplerIndex = -1.0;
  double targetIndex = -1.0;
  if (!ParseNumberProperty(&samplerIndex, err, o, "sampler", true, "AnimationChannel")) {
    if (err) {
      (*err) += "`sampler` field is missing in animation channels\n";
    }
    return false;
  }

  picojson::object::const_iterator targetIt = o.find("target");
  if ((targetIt != o.end()) && (targetIt->second).is<picojson::object>()) {
    const picojson::object &target_object =
        (targetIt->second).get<picojson::object>();

    if (!ParseNumberProperty(&targetIndex, err, target_object, "node", true)) {
      if (err) {
        (*err) += "`node` field is missing in animation.channels.target\n";
      }
      return false;
    }

    if (!ParseStringProperty(&channel->target_path, err, target_object, "path",
                             true)) {
      if (err) {
        (*err) += "`path` field is missing in animation.channels.target\n";
      }
      return false;
    }
  }

  channel->sampler = static_cast<int>(samplerIndex);
  channel->target_node = static_cast<int>(targetIndex);

  ParseExtrasProperty(&(channel->extras), o);

  return true;
}

static bool ParseAnimation(Animation *animation, std::string *err,
                           const picojson::object &o) {
  {
    picojson::object::const_iterator channelsIt = o.find("channels");
    if ((channelsIt != o.end()) && (channelsIt->second).is<picojson::array>()) {
      const picojson::array &channelArray =
          (channelsIt->second).get<picojson::array>();
      for (size_t i = 0; i < channelArray.size(); i++) {
        AnimationChannel channel;
        if (ParseAnimationChannel(&channel, err,
                                  channelArray[i].get<picojson::object>())) {
          // Only add the channel if the parsing succeeds.
          animation->channels.push_back(channel);
        }
      }
    }
  }

  {
    picojson::object::const_iterator samplerIt = o.find("samplers");
    if ((samplerIt != o.end()) && (samplerIt->second).is<picojson::array>()) {
      const picojson::array &sampler_array =
          (samplerIt->second).get<picojson::array>();

      picojson::array::const_iterator it = sampler_array.begin();
      picojson::array::const_iterator itEnd = sampler_array.end();

      for (; it != itEnd; it++) {
        const picojson::object &s = it->get<picojson::object>();

        AnimationSampler sampler;
        double inputIndex = -1.0;
        double outputIndex = -1.0;
        if (!ParseNumberProperty(&inputIndex, err, s, "input", true)) {
          if (err) {
            (*err) += "`input` field is missing in animation.sampler\n";
          }
          return false;
        }
        if (!ParseStringProperty(&sampler.interpolation, err, s,
                                 "interpolation", true)) {
          if (err) {
            (*err) += "`interpolation` field is missing in animation.sampler\n";
          }
          return false;
        }
        if (!ParseNumberProperty(&outputIndex, err, s, "output", true)) {
          if (err) {
            (*err) += "`output` field is missing in animation.sampler\n";
          }
          return false;
        }
        sampler.input = static_cast<int>(inputIndex);
        sampler.output = static_cast<int>(outputIndex);
        animation->samplers.push_back(sampler);
      }
    }
  }

  ParseStringProperty(&animation->name, err, o, "name", false);

  ParseExtrasProperty(&(animation->extras), o);

  return true;
}

static bool ParseSampler(Sampler *sampler, std::string *err,
                         const picojson::object &o) {
  ParseStringProperty(&sampler->name, err, o, "name", false);

  double minFilter =
      static_cast<double>(TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR);
  double magFilter = static_cast<double>(TINYGLTF_TEXTURE_FILTER_LINEAR);
  double wrapS = static_cast<double>(TINYGLTF_TEXTURE_WRAP_RPEAT);
  double wrapT = static_cast<double>(TINYGLTF_TEXTURE_WRAP_RPEAT);
  ParseNumberProperty(&minFilter, err, o, "minFilter", false);
  ParseNumberProperty(&magFilter, err, o, "magFilter", false);
  ParseNumberProperty(&wrapS, err, o, "wrapS", false);
  ParseNumberProperty(&wrapT, err, o, "wrapT", false);

  sampler->minFilter = static_cast<int>(minFilter);
  sampler->magFilter = static_cast<int>(magFilter);
  sampler->wrapS = static_cast<int>(wrapS);
  sampler->wrapT = static_cast<int>(wrapT);

  ParseExtrasProperty(&(sampler->extras), o);

  return true;
}

static bool ParseSkin(Skin *skin, std::string *err, const picojson::object &o) {
  ParseStringProperty(&skin->name, err, o, "name", false, "Skin");

  std::vector<double> joints;
  if (!ParseNumberArrayProperty(&joints, err, o, "joints", false, "Skin")) {
    return false;
  }

  double skeleton = -1.0;
  ParseNumberProperty(&skeleton, err, o, "skeleton", false, "Skin");
  skin->skeleton = static_cast<int>(skeleton);

  skin->joints.resize(joints.size());
  for (size_t i = 0; i < joints.size(); i++) {
    skin->joints[i] = static_cast<int>(joints[i]);
  }

  double invBind = -1.0;
  ParseNumberProperty(&invBind, err, o, "inverseBindMatrices", true, "Skin");
  skin->inverseBindMatrices = static_cast<int>(invBind);

  return true;
}

static bool ParsePerspectiveCamera(PerspectiveCamera *camera, std::string *err,
                                   const picojson::object &o) {
  double yfov = 0.0;
  if (!ParseNumberProperty(&yfov, err, o, "yfov", true, "OrthographicCamera")) {
    return false;
  }

  double znear = 0.0;
  if (!ParseNumberProperty(&znear, err, o, "znear", true,
                           "PerspectiveCamera")) {
    return false;
  }

  double aspectRatio = 0.0;  // = invalid
  ParseNumberProperty(&aspectRatio, err, o, "aspectRatio", false,
                      "PerspectiveCamera");

  double zfar = 0.0;  // = invalid
  ParseNumberProperty(&zfar, err, o, "zfar", false, "PerspectiveCamera");

  camera->aspectRatio = float(aspectRatio);
  camera->zfar = float(zfar);
  camera->yfov = float(yfov);
  camera->znear = float(znear);

  ParseExtrasProperty(&(camera->extras), o);

  // TODO(syoyo): Validate parameter values.

  return true;
}

static bool ParseOrthographicCamera(OrthographicCamera *camera,
                                    std::string *err,
                                    const picojson::object &o) {
  double xmag = 0.0;
  if (!ParseNumberProperty(&xmag, err, o, "xmag", true, "OrthographicCamera")) {
    return false;
  }

  double ymag = 0.0;
  if (!ParseNumberProperty(&ymag, err, o, "ymag", true, "OrthographicCamera")) {
    return false;
  }

  double zfar = 0.0;
  if (!ParseNumberProperty(&zfar, err, o, "zfar", true, "OrthographicCamera")) {
    return false;
  }

  double znear = 0.0;
  if (!ParseNumberProperty(&znear, err, o, "znear", true,
                           "OrthographicCamera")) {
    return false;
  }

  ParseExtrasProperty(&(camera->extras), o);

  camera->xmag = float(xmag);
  camera->ymag = float(ymag);
  camera->zfar = float(zfar);
  camera->znear = float(znear);

  // TODO(syoyo): Validate parameter values.

  return true;
}

static bool ParseCamera(Camera *camera, std::string *err,
                        const picojson::object &o) {
  if (!ParseStringProperty(&camera->type, err, o, "type", true, "Camera")) {
    return false;
  }

  if (camera->type.compare("orthographic") == 0) {
    if (o.find("orthographic") == o.end()) {
      if (err) {
        std::stringstream ss;
        ss << "Orhographic camera description not found." << std::endl;
        (*err) += ss.str();
      }
      return false;
    }

    const picojson::value &v = o.find("orthographic")->second;
    if (!v.is<picojson::object>()) {
      if (err) {
        std::stringstream ss;
        ss << "\"orthographic\" is not a JSON object." << std::endl;
        (*err) += ss.str();
      }
      return false;
    }

    if (!ParseOrthographicCamera(&camera->orthographic, err,
                                 v.get<picojson::object>())) {
      return false;
    }
  } else if (camera->type.compare("perspective") == 0) {
    if (o.find("perspective") == o.end()) {
      if (err) {
        std::stringstream ss;
        ss << "Perspective camera description not found." << std::endl;
        (*err) += ss.str();
      }
      return false;
    }

    const picojson::value &v = o.find("perspective")->second;
    if (!v.is<picojson::object>()) {
      if (err) {
        std::stringstream ss;
        ss << "\"perspective\" is not a JSON object." << std::endl;
        (*err) += ss.str();
      }
      return false;
    }

    if (!ParsePerspectiveCamera(&camera->perspective, err,
                                v.get<picojson::object>())) {
      return false;
    }
  } else {
    if (err) {
      std::stringstream ss;
      ss << "Invalid camera type: \"" << camera->type
         << "\". Must be \"perspective\" or \"orthographic\"" << std::endl;
      (*err) += ss.str();
    }
    return false;
  }

  ParseStringProperty(&camera->name, err, o, "name", false);

  ParseExtrasProperty(&(camera->extras), o);

  return true;
}

bool TinyGLTF::LoadFromString(
        IFileLoader& fileLoader,
        Model *model, std::string *err, const char *str,
                              unsigned int length, const std::string &base_dir,
                              unsigned int check_sections) {
  if (length < 4) {
    if (err) {
      (*err) = "JSON string too short.\n";
    }
    return false;
  }

  // TODO(syoyo): Add feature not using exception handling.
  picojson::value v;
  try {
    std::string perr = picojson::parse(v, str, str + length);

    if (!perr.empty()) {
    if (err) {
        (*err) = "JSON parsing error: " + perr;
    }
    return false;
  }
  } catch (std::exception e) {
      if (err) {
      (*err) = e.what();
      }
      return false;
    }

  if (!v.is<picojson::object>()) {
    // root is not an object.
    if (err) {
      (*err) = "Root element is not a JSON object\n";
    }
    return false;
  }

  // scene is not mandatory.
  // FIXME Maybe a better way to handle it than removing the code

  if (v.contains("scenes") && v.get("scenes").is<picojson::array>()) {
      // OK
    } else if (check_sections & REQUIRE_SCENES) {
      if (err) {
      (*err) += "\"scenes\" object not found in .gltf\n";
      }
      return false;
    }

  if (v.contains("nodes") && v.get("nodes").is<picojson::array>()) {
      // OK
    } else if (check_sections & REQUIRE_NODES) {
      if (err) {
        (*err) += "\"nodes\" object not found in .gltf\n";
      }
      return false;
    }

  if (v.contains("accessors") && v.get("accessors").is<picojson::array>()) {
      // OK
    } else if (check_sections & REQUIRE_ACCESSORS) {
      if (err) {
        (*err) += "\"accessors\" object not found in .gltf\n";
      }
      return false;
    }

  if (v.contains("buffers") && v.get("buffers").is<picojson::array>()) {
      // OK
    } else if (check_sections & REQUIRE_BUFFERS) {
      if (err) {
        (*err) += "\"buffers\" object not found in .gltf\n";
      }
      return false;
    }

  if (v.contains("bufferViews") && v.get("bufferViews").is<picojson::array>()) {
      // OK
    } else if (check_sections & REQUIRE_BUFFER_VIEWS) {
      if (err) {
        (*err) += "\"bufferViews\" object not found in .gltf\n";
      }
      return false;
    }

  model->buffers.clear();
  model->bufferViews.clear();
  model->accessors.clear();
  model->meshes.clear();
  model->cameras.clear();
  model->nodes.clear();
  model->extensionsUsed.clear();
  model->extensionsRequired.clear();
  model->defaultScene = -1;

  // 0. Parse Asset
  if (v.contains("asset") && v.get("asset").is<picojson::object>()) {
    const picojson::object &root = v.get("asset").get<picojson::object>();

      ParseAsset(&model->asset, err, root);
    }

  // 0. Parse extensionUsed
  if (v.contains("extensionsUsed") &&
      v.get("extensionsUsed").is<picojson::array>()) {
    const picojson::array &root =
        v.get("extensionsUsed").get<picojson::array>();
      for (unsigned int i = 0; i < root.size(); ++i) {
        model->extensionsUsed.push_back(root[i].get<std::string>());
      }
    }
  if (v.contains("extensionsRequired") &&
      v.get("extensionsRequired").is<picojson::array>()) {
    const picojson::array &root =
        v.get("extensionsRequired").get<picojson::array>();
      for (unsigned int i = 0; i < root.size(); ++i) {
        model->extensionsRequired.push_back(root[i].get<std::string>());
      }
    }

  // 1. Parse Buffer
  if (v.contains("buffers") && v.get("buffers").is<picojson::array>()) {
    const picojson::array &root = v.get("buffers").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; it++) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`buffers' does not contain an JSON object.";
          }
          return false;
        }
        Buffer buffer;
        if (!ParseBuffer(
fileLoader, &buffer, err, it->get<picojson::object>(), base_dir,
                       is_binary_, bin_data_, bin_size_)) {
          return false;
        }

        model->buffers.push_back(buffer);
      }
    }

  // 2. Parse BufferView
  if (v.contains("bufferViews") && v.get("bufferViews").is<picojson::array>()) {
    const picojson::array &root = v.get("bufferViews").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; it++) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`bufferViews' does not contain an JSON object.";
          }
          return false;
        }
        BufferView bufferView;
      if (!ParseBufferView(&bufferView, err, it->get<picojson::object>())) {
          return false;
        }

        model->bufferViews.push_back(bufferView);
      }
    }

  // 3. Parse Accessor
  if (v.contains("accessors") && v.get("accessors").is<picojson::array>()) {
    const picojson::array &root = v.get("accessors").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; it++) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`accessors' does not contain an JSON object.";
          }
          return false;
        }
        Accessor accessor;
      if (!ParseAccessor(&accessor, err, it->get<picojson::object>())) {
          return false;
        }

        model->accessors.push_back(accessor);
      }
    }

  // 4. Parse Mesh
  if (v.contains("meshes") && v.get("meshes").is<picojson::array>()) {
    const picojson::array &root = v.get("meshes").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; it++) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`meshes' does not contain an JSON object.";
          }
          return false;
        }
        Mesh mesh;
      if (!ParseMesh(&mesh, err, it->get<picojson::object>())) {
          return false;
        }

        model->meshes.push_back(mesh);
      }
    }

  // 5. Parse Node
  if (v.contains("nodes") && v.get("nodes").is<picojson::array>()) {
    const picojson::array &root = v.get("nodes").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; it++) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`nodes' does not contain an JSON object.";
          }
          return false;
        }
        Node node;
      if (!ParseNode(&node, err, it->get<picojson::object>())) {
          return false;
        }

        model->nodes.push_back(node);
      }
    }

  // 6. Parse scenes.
  if (v.contains("scenes") && v.get("scenes").is<picojson::array>()) {
    const picojson::array &root = v.get("scenes").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; it++) {
      if (!(it->is<picojson::object>())) {
          if (err) {
            (*err) += "`scenes' does not contain an JSON object.";
          }
          return false;
        }
      const picojson::object &o = it->get<picojson::object>();
        std::vector<double> nodes;
        if (!ParseNumberArrayProperty(&nodes, err, o, "nodes", false)) {
          return false;
        }

        Scene scene;
        ParseStringProperty(&scene.name, err, o, "name", false);
        std::vector<int> nodesIds;
        for (size_t i = 0; i < nodes.size(); i++) {
          nodesIds.push_back(static_cast<int>(nodes[i]));
        }
        scene.nodes = nodesIds;

        model->scenes.push_back(scene);
      }
    }

  // 7. Parse default scenes.
  if (v.contains("scene") && v.get("scene").is<double>()) {
    const int defaultScene = int(v.get("scene").get<double>());

      model->defaultScene = static_cast<int>(defaultScene);
    }

  // 8. Parse Material
  if (v.contains("materials") && v.get("materials").is<picojson::array>()) {
    const picojson::array &root = v.get("materials").get<picojson::array>();
    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; it++) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`materials' does not contain an JSON object.";
          }
          return false;
        }
      picojson::object jsonMaterial = it->get<picojson::object>();

        Material material;
        ParseStringProperty(&material.name, err, jsonMaterial, "name", false);

        if (!ParseMaterial(&material, err, jsonMaterial)) {
          return false;
        }

        model->materials.push_back(material);
      }
    }

  // 9. Parse Image
  if (v.contains("images") && v.get("images").is<picojson::array>()) {
    const picojson::array &root = v.get("images").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; it++) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`images' does not contain an JSON object.";
          }
          return false;
        }
        Image image;
      if (!ParseImage(fileLoader, &image, err, it->get<picojson::object>(), base_dir,
                      is_binary_, bin_data_, bin_size_)) {
          return false;
        }

        if (image.bufferView != -1) {
          // Load image from the buffer view.
          if (size_t(image.bufferView) >= model->bufferViews.size()) {
            if (err) {
              std::stringstream ss;
              ss << "bufferView \"" << image.bufferView
                 << "\" not found in the scene." << std::endl;
              (*err) += ss.str();
            }
            return false;
          }

          const BufferView &bufferView =
              model->bufferViews[size_t(image.bufferView)];
          const Buffer &buffer = model->buffers[size_t(bufferView.buffer)];

#ifndef TINYGLTF_NO_STB_IMAGE
          bool ret = LoadImageData(&image, err, image.width, image.height,
                                   &buffer.data[bufferView.byteOffset],
                                 static_cast<int>(bufferView.byteLength));
          if (!ret) {
            return false;
          }
#endif
        }

        model->images.push_back(image);
      }
    }

  // 10. Parse Texture
  if (v.contains("textures") && v.get("textures").is<picojson::array>()) {
    const picojson::array &root = v.get("textures").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; it++) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`textures' does not contain an JSON object.";
          }
          return false;
        }
        Texture texture;
      if (!ParseTexture(&texture, err, it->get<picojson::object>(), base_dir)) {
          return false;
        }

        model->textures.push_back(texture);
      }
    }

  // 11. Parse Animation
  if (v.contains("animations") && v.get("animations").is<picojson::array>()) {
    const picojson::array &root = v.get("animations").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; ++it) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`animations' does not contain an JSON object.";
          }
          return false;
        }
        Animation animation;
      if (!ParseAnimation(&animation, err, it->get<picojson::object>())) {
          return false;
        }

        model->animations.push_back(animation);
      }
    }

  // 12. Parse Skin
  if (v.contains("skins") && v.get("skins").is<picojson::array>()) {
    const picojson::array &root = v.get("skins").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; ++it) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`skins' does not contain an JSON object.";
          }
          return false;
        }
        Skin skin;
      if (!ParseSkin(&skin, err, it->get<picojson::object>())) {
          return false;
        }

        model->skins.push_back(skin);
      }
    }

  // 13. Parse Sampler
  if (v.contains("samplers") && v.get("samplers").is<picojson::array>()) {
    const picojson::array &root = v.get("samplers").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; ++it) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`samplers' does not contain an JSON object.";
          }
          return false;
        }
        Sampler sampler;
      if (!ParseSampler(&sampler, err, it->get<picojson::object>())) {
          return false;
        }

        model->samplers.push_back(sampler);
      }
    }

  // 14. Parse Camera
  if (v.contains("cameras") && v.get("cameras").is<picojson::array>()) {
    const picojson::array &root = v.get("cameras").get<picojson::array>();

    picojson::array::const_iterator it(root.begin());
    picojson::array::const_iterator itEnd(root.end());
      for (; it != itEnd; ++it) {
      if (!it->is<picojson::object>()) {
          if (err) {
            (*err) += "`cameras' does not contain an JSON object.";
          }
          return false;
        }
        Camera camera;
      if (!ParseCamera(&camera, err, it->get<picojson::object>())) {
          return false;
        }

        model->cameras.push_back(camera);
      }
    }

  // 15. Parse Extensions
  if (v.contains("extensions") && v.get("extensions").is<picojson::object>()) {
    const picojson::object &root = v.get("extensions").get<picojson::object>();
    picojson::object::const_iterator it(root.begin());
    picojson::object::const_iterator itEnd(root.end());
      for (; it != itEnd; ++it) {
        // parse KHR_lights_cmn extension
      if (it->first == "KHR_lights_cmn" && it->second.is<picojson::object>()) {
        const picojson::object &object = it->second.get<picojson::object>();
        picojson::object::const_iterator it(object.find("lights"));
        picojson::object::const_iterator itEnd(object.end());
        if (it == itEnd)
            continue;

        const picojson::array &lights = it->second.get<picojson::array>();
        picojson::array::const_iterator arrayIt(lights.begin());
        picojson::array::const_iterator arrayItEnd(lights.end());
          for (; arrayIt != arrayItEnd; ++arrayIt) {
            Light light;
          if (!ParseLight(&light, err, arrayIt->get<picojson::object>())) {
              return false;
            }
            model->lights.push_back(light);
          }
        }
      }

  }
  return true;
}

bool TinyGLTF::LoadASCIIFromString(
        IFileLoader& fileLoader,
        Model *model, std::string *err,
                                   const char *str, unsigned int length,
                                   const std::string &base_dir,
                                   unsigned int check_sections) {
  is_binary_ = false;
  bin_data_ = NULL;
  bin_size_ = 0;

  return LoadFromString(
fileLoader, model, err, str, length, base_dir, check_sections);
}

bool TinyGLTF::LoadASCIIFromFile(
        IFileLoader& fileLoader,
        Model *model, std::string *err,
        const std::string &filename,
        unsigned int check_sections) {
  std::stringstream ss;

  std::ifstream f(filename.c_str());
  if (!f) {
    ss << "Failed to open file: " << filename << std::endl;
    if (err) {
      (*err) = ss.str();
    }
    return false;
  }

  f.seekg(0, f.end);
  size_t sz = static_cast<size_t>(f.tellg());
  std::vector<char> buf(sz);

  if (sz == 0) {
    if (err) {
      (*err) = "Empty file.";
    }
    return false;
  }

  f.seekg(0, f.beg);
  f.read(&buf.at(0), static_cast<std::streamsize>(sz));
  f.close();

  std::string basedir = GetBaseDir(filename);

  bool ret = LoadASCIIFromString(
            fileLoader,
             model, err, &buf.at(0),
                                 static_cast<unsigned int>(buf.size()), basedir,
                                 check_sections);

  return ret;
}

bool TinyGLTF::LoadBinaryFromMemory(
        IFileLoader& fileLoader,
        Model *model, std::string *err,
                                    const unsigned char *bytes,
                                    unsigned int size,
                                    const std::string &base_dir,
                                    unsigned int check_sections) {
  if (size < 20) {
    if (err) {
      (*err) = "Too short data size for glTF Binary.";
    }
    return false;
  }

  if (bytes[0] == 'g' && bytes[1] == 'l' && bytes[2] == 'T' &&
      bytes[3] == 'F') {
    // ok
  } else {
    if (err) {
      (*err) = "Invalid magic.";
    }
    return false;
  }

  unsigned int version;       // 4 bytes
  unsigned int length;        // 4 bytes
  unsigned int model_length;  // 4 bytes
  unsigned int model_format;  // 4 bytes;

  // @todo { Endian swap for big endian machine. }
  memcpy(&version, bytes + 4, 4);
  swap4(&version);
  memcpy(&length, bytes + 8, 4);
  swap4(&length);
  memcpy(&model_length, bytes + 12, 4);
  swap4(&model_length);
  memcpy(&model_format, bytes + 16, 4);
  swap4(&model_format);

  if ((20 + model_length >= size) || (model_length < 1) ||
      (model_format != 0x4E4F534A)) {  // 0x4E4F534A = JSON format.
    if (err) {
      (*err) = "Invalid glTF binary.";
    }
    return false;
  }

  // Extract JSON string.
  std::string jsonString(reinterpret_cast<const char *>(&bytes[20]),
                         model_length);

  is_binary_ = true;
  bin_data_ = bytes + 20 + model_length +
              8;  // 4 bytes (buffer_length) + 4 bytes(buffer_format)
  bin_size_ =
      length - (20 + model_length);  // extract header + JSON scene data.

  bool ret =
      LoadFromString(
                fileLoader,
              model, err, reinterpret_cast<const char *>(&bytes[20]),
                     model_length, base_dir, check_sections);
  if (!ret) {
    return ret;
  }

  return true;
}

bool TinyGLTF::LoadBinaryFromFile(
        IFileLoader& fileLoader,
        Model *model, std::string *err,
                                  const std::string &filename,
                                  unsigned int check_sections) {
  std::stringstream ss;

  std::ifstream f(filename.c_str(), std::ios::binary);
  if (!f) {
    ss << "Failed to open file: " << filename << std::endl;
    if (err) {
      (*err) = ss.str();
    }
    return false;
  }

  f.seekg(0, f.end);
  size_t sz = static_cast<size_t>(f.tellg());
  std::vector<char> buf(sz);

  f.seekg(0, f.beg);
  f.read(&buf.at(0), static_cast<std::streamsize>(sz));
  f.close();

  std::string basedir = GetBaseDir(filename);

  bool ret = LoadBinaryFromMemory(
      fileLoader,
      model, err, reinterpret_cast<unsigned char *>(&buf.at(0)),
      static_cast<unsigned int>(buf.size()), basedir, check_sections);

  return ret;
}

///////////////////////
// GLTF Serialization
///////////////////////

typedef std::pair<std::string, picojson::value> json_object_pair;

template <typename T>
static void SerializeNumberProperty(const std::string &key, T number,
                                    picojson::object &obj) {
  obj.insert(
      json_object_pair(key, picojson::value(static_cast<double>(number))));
}

template <typename T>
static void SerializeNumberArrayProperty(const std::string &key,
                                         const std::vector<T> &value,
                                         picojson::object &obj) {
  picojson::object o;
  picojson::array vals;

  for (unsigned int i = 0; i < value.size(); ++i) {
    vals.push_back(picojson::value(static_cast<double>(value[i])));
  }

  obj.insert(json_object_pair(key, picojson::value(vals)));
}

static void SerializeStringProperty(const std::string &key,
                                    const std::string &value,
                                    picojson::object &obj) {
  picojson::value strVal(value);
  obj.insert(json_object_pair(key, strVal));
}

static void SerializeStringArrayProperty(const std::string &key,
                                         const std::vector<std::string> &value,
                                         picojson::object &obj) {
  picojson::object o;
  picojson::array vals;

  for (unsigned int i = 0; i < value.size(); ++i) {
    vals.push_back(picojson::value(value[i]));
  }

  obj.insert(json_object_pair(key, picojson::value(vals)));
}

static void SerializeValue(const std::string &key, const Value &value,
                           picojson::object &obj) {
  if (value.IsArray()) {
    picojson::array jsonValue;
      for (unsigned int i = 0; i < value.ArrayLen(); ++i) {
        Value elementValue = value.Get(int(i));
      if (elementValue.IsString())
        jsonValue.push_back(picojson::value(elementValue.Get<std::string>()));
      }
    obj.insert(json_object_pair(key, picojson::value(jsonValue)));
  } else {
    picojson::object jsonValue;
    std::vector<std::string> valueKeys;
    for (unsigned int i = 0; i < valueKeys.size(); ++i) {
      Value elementValue = value.Get(valueKeys[i]);
      if (elementValue.IsInt())
        jsonValue.insert(json_object_pair(
            valueKeys[i],
            picojson::value(static_cast<double>(elementValue.Get<int>()))));
    }

    obj.insert(json_object_pair(key, picojson::value(jsonValue)));
}
}

static void SerializeGltfBufferData(const std::vector<unsigned char> &data,
                                    const std::string &binFilePath) {
  std::ofstream output(binFilePath.c_str(), std::ofstream::binary);
  output.write(reinterpret_cast<const char *>(&data[0]),
               std::streamsize(data.size()));
  output.close();
}

static void SerializeParameterMap(ParameterMap &param, picojson::object &o) {
  for (ParameterMap::iterator paramIt = param.begin(); paramIt != param.end();
       ++paramIt) {
    if (paramIt->second.number_array.size()) {
      SerializeNumberArrayProperty<double>(paramIt->first,
                                           paramIt->second.number_array, o);
    } else if (paramIt->second.json_double_value.size()) {
      picojson::object json_double_value;

      for (std::map<std::string, double>::iterator it =
               paramIt->second.json_double_value.begin();
           it != paramIt->second.json_double_value.end(); ++it) {
        json_double_value.insert(
            json_object_pair(it->first, picojson::value(it->second)));
      }

      o.insert(
          json_object_pair(paramIt->first, picojson::value(json_double_value)));
    } else if (!paramIt->second.string_value.empty()) {
      SerializeStringProperty(paramIt->first, paramIt->second.string_value, o);
    } else {
      o.insert(json_object_pair(paramIt->first,
                                picojson::value(paramIt->second.bool_value)));
    }
  }
}

static void SerializeGltfAccessor(Accessor &accessor, picojson::object &o) {
  SerializeNumberProperty<int>("bufferView", accessor.bufferView, o);

  if (accessor.byteOffset != 0.0)
    SerializeNumberProperty<int>("byteOffset", int(accessor.byteOffset), o);

  SerializeNumberProperty<int>("componentType", accessor.componentType, o);
  SerializeNumberProperty<size_t>("count", accessor.count, o);
  SerializeNumberArrayProperty<double>("min", accessor.minValues, o);
  SerializeNumberArrayProperty<double>("max", accessor.maxValues, o);
  std::string type;
  switch (accessor.type) {
    case TINYGLTF_TYPE_SCALAR:
      type = "SCALAR";
      break;
    case TINYGLTF_TYPE_VEC2:
      type = "VEC2";
      break;
    case TINYGLTF_TYPE_VEC3:
      type = "VEC3";
      break;
    case TINYGLTF_TYPE_VEC4:
      type = "VEC4";
      break;
    case TINYGLTF_TYPE_MAT2:
      type = "MAT2";
      break;
    case TINYGLTF_TYPE_MAT3:
      type = "MAT3";
      break;
    case TINYGLTF_TYPE_MAT4:
      type = "MAT4";
      break;
  }

  SerializeStringProperty("type", type, o);
}

static void SerializeGltfAnimationChannel(AnimationChannel &channel,
                                          picojson::object &o) {
  SerializeNumberProperty("sampler", channel.sampler, o);
  picojson::object target;
  SerializeNumberProperty("node", channel.target_node, target);
  SerializeStringProperty("path", channel.target_path, target);

  o.insert(json_object_pair("target", picojson::value(target)));
}

static void SerializeGltfAnimationSampler(AnimationSampler &sampler,
                                          picojson::object &o) {
  SerializeNumberProperty("input", sampler.input, o);
  SerializeNumberProperty("output", sampler.output, o);
  SerializeStringProperty("interpolation", sampler.interpolation, o);
}

static void SerializeGltfAnimation(Animation &animation, picojson::object &o) {
  SerializeStringProperty("name", animation.name, o);
  picojson::array channels;
  for (unsigned int i = 0; i < animation.channels.size(); ++i) {
    picojson::object channel;
    AnimationChannel gltfChannel = animation.channels[i];
    SerializeGltfAnimationChannel(gltfChannel, channel);
    channels.push_back(picojson::value(channel));
  }
  o.insert(json_object_pair("channels", picojson::value(channels)));

  picojson::array samplers;
  for (unsigned int i = 0; i < animation.samplers.size(); ++i) {
    picojson::object sampler;
    AnimationSampler gltfSampler = animation.samplers[i];
    SerializeGltfAnimationSampler(gltfSampler, sampler);
    samplers.push_back(picojson::value(sampler));
  }

  o.insert(json_object_pair("samplers", picojson::value(samplers)));
}

static void SerializeGltfAsset(Asset &asset, picojson::object &o) {
  if (!asset.generator.empty()) {
    SerializeStringProperty("generator", asset.generator, o);
  }

  if (!asset.version.empty()) {
    SerializeStringProperty("version", asset.version, o);
  }

  if (asset.extras.Keys().size()) {
    SerializeValue("extras", asset.extras, o);
  }
}

static void SerializeGltfBuffer(Buffer &buffer, picojson::object &o,
                                const std::string &binFilePath) {
  SerializeGltfBufferData(buffer.data, binFilePath);
  SerializeNumberProperty("byteLength", buffer.data.size(), o);
  SerializeStringProperty("uri", binFilePath, o);

  if (buffer.name.size()) SerializeStringProperty("name", buffer.name, o);
}

static void SerializeGltfBufferView(BufferView &bufferView,
                                    picojson::object &o) {
  SerializeNumberProperty("buffer", bufferView.buffer, o);
  SerializeNumberProperty<size_t>("byteLength", bufferView.byteLength, o);
    SerializeNumberProperty<size_t>("byteStride", bufferView.byteStride, o);
    SerializeNumberProperty<size_t>("byteOffset", bufferView.byteOffset, o);
    SerializeNumberProperty("target", bufferView.target, o);

  if (bufferView.name.size()) {
    SerializeStringProperty("name", bufferView.name, o);
  }
}

// Only external textures are serialized for now
static void SerializeGltfImage(Image &image, picojson::object &o) {
  SerializeStringProperty("uri", image.uri, o);

  if (image.name.size()) {
    SerializeStringProperty("name", image.name, o);
  }
}

static void SerializeGltfMaterial(Material &material, picojson::object &o) {
  if (material.extPBRValues.size()) {
    // Serialize PBR specular/glossiness material
    picojson::object values;
    SerializeParameterMap(material.extPBRValues, values);

    picojson::object extension;
    o.insert(json_object_pair("extensions", picojson::value(extension)));
  }

  if (material.values.size()) {
    picojson::object pbrMetallicRoughness;
    SerializeParameterMap(material.values, pbrMetallicRoughness);
    o.insert(json_object_pair("pbrMetallicRoughness",
                              picojson::value(pbrMetallicRoughness)));
  }

  picojson::object additionalValues;
  SerializeParameterMap(material.additionalValues, o);

  if (material.name.size()) {
    SerializeStringProperty("name", material.name, o);
  }
}

static void SerializeGltfMesh(Mesh &mesh, picojson::object &o) {
  picojson::array primitives;
  for (unsigned int i = 0; i < mesh.primitives.size(); ++i) {
    picojson::object primitive;
    picojson::object attributes;
    Primitive gltfPrimitive = mesh.primitives[i];
    for (std::map<std::string, int>::iterator attrIt =
             gltfPrimitive.attributes.begin();
         attrIt != gltfPrimitive.attributes.end(); ++attrIt) {
      SerializeNumberProperty<int>(attrIt->first, attrIt->second, attributes);
    }

    primitive.insert(
        json_object_pair("attributes", picojson::value(attributes)));
      SerializeNumberProperty<int>("indices", gltfPrimitive.indices, primitive);
    SerializeNumberProperty<int>("material", gltfPrimitive.material, primitive);
    SerializeNumberProperty<int>("mode", gltfPrimitive.mode, primitive);

    // Morph targets
    if (gltfPrimitive.targets.size()) {
      picojson::array targets;
      for (unsigned int k = 0; k < gltfPrimitive.targets.size(); ++k) {
        picojson::object targetAttributes;
        std::map<std::string, int> targetData = gltfPrimitive.targets[k];
        for (std::map<std::string, int>::iterator attrIt = targetData.begin();
             attrIt != targetData.end(); ++attrIt) {
          SerializeNumberProperty<int>(attrIt->first, attrIt->second,
                                       targetAttributes);
        }

        targets.push_back(picojson::value(targetAttributes));
      }
      primitive.insert(json_object_pair("targets", picojson::value(targets)));
    }

    primitives.push_back(picojson::value(primitive));
  }

  o.insert(json_object_pair("primitives", picojson::value(primitives)));
  if (mesh.weights.size()) {
    SerializeNumberArrayProperty<double>("weights", mesh.weights, o);
  }

  if (mesh.name.size()) {
    SerializeStringProperty("name", mesh.name, o);
  }
}

static void SerializeGltfLight(Light &light, picojson::object &o) {
  SerializeStringProperty("name", light.name, o);
  SerializeNumberArrayProperty("color", light.color, o);
  SerializeStringProperty("type", light.type, o);
}

static void SerializeGltfNode(Node &node, picojson::object &o) {
  if (node.translation.size() > 0) {
    SerializeNumberArrayProperty<double>("translation", node.translation, o);
  }
  if (node.rotation.size() > 0) {
    SerializeNumberArrayProperty<double>("rotation", node.rotation, o);
  }
  if (node.scale.size() > 0) {
    SerializeNumberArrayProperty<double>("scale", node.scale, o);
  }
  if (node.matrix.size() > 0) {
    SerializeNumberArrayProperty<double>("matrix", node.matrix, o);
  }
  if (node.mesh != -1) {
    SerializeNumberProperty<int>("mesh", node.mesh, o);
  }

  if (node.skin != -1) {
    SerializeNumberProperty<int>("skin", node.skin, o);
  }

  if (node.camera != -1) {
    SerializeNumberProperty<int>("camera", node.camera, o);
  }

  if (node.extLightsValues.size()) {
    picojson::object values;
    SerializeParameterMap(node.extLightsValues, values);
    picojson::object lightsExt;
    lightsExt.insert(json_object_pair("KHR_lights_cmn", picojson::value(values)));
    o.insert(json_object_pair("extensions", picojson::value(lightsExt)));
  }


  SerializeStringProperty("name", node.name, o);
  SerializeNumberArrayProperty<int>("children", node.children, o);
}

static void SerializeGltfSampler(Sampler &sampler, picojson::object &o) {
  SerializeNumberProperty("magFilter", sampler.magFilter, o);
  SerializeNumberProperty("minFilter", sampler.minFilter, o);
  SerializeNumberProperty("wrapS", sampler.wrapS, o);
  SerializeNumberProperty("wrapT", sampler.wrapT, o);
}

static void SerializeGltfOrthographicCamera(const OrthographicCamera &camera,
                                            picojson::object &o) {
  SerializeNumberProperty("zfar", camera.zfar, o);
  SerializeNumberProperty("znear", camera.znear, o);
  SerializeNumberProperty("xmag", camera.xmag, o);
  SerializeNumberProperty("ymag", camera.ymag, o);
}

static void SerializeGltfPerspectiveCamera(const PerspectiveCamera &camera,
                                           picojson::object &o) {
  SerializeNumberProperty("zfar", camera.zfar, o);
  SerializeNumberProperty("znear", camera.znear, o);
  if (camera.aspectRatio > 0) {
    SerializeNumberProperty("aspectRatio", camera.aspectRatio, o);
  }

  if (camera.yfov > 0) {
    SerializeNumberProperty("yfov", camera.yfov, o);
  }
}

static void SerializeGltfCamera(const Camera &camera, picojson::object &o) {
  SerializeStringProperty("type", camera.type, o);
  if (!camera.name.empty()) {
    SerializeStringProperty("name", camera.type, o);
  }

  if (camera.type.compare("orthographic") == 0) {
    picojson::object orthographic;
    SerializeGltfOrthographicCamera(camera.orthographic, orthographic);
    o.insert(json_object_pair("orthographic", picojson::value(orthographic)));
  } else if (camera.type.compare("perspective") == 0) {
    picojson::object perspective;
    SerializeGltfPerspectiveCamera(camera.perspective, perspective);
    o.insert(json_object_pair("perspective", picojson::value(perspective)));
  } else {
    // ???
  }
}

static void SerializeGltfScene(Scene &scene, picojson::object &o) {
  SerializeNumberArrayProperty<int>("nodes", scene.nodes, o);

  if (scene.name.size()) {
    SerializeStringProperty("name", scene.name, o);
  }
}

static void SerializeGltfSkin(Skin &skin, picojson::object &o) {
  if (skin.inverseBindMatrices != -1)
    SerializeNumberProperty("inverseBindMatrices", skin.inverseBindMatrices, o);

  SerializeNumberArrayProperty<int>("joints", skin.joints, o);
  SerializeNumberProperty("skeleton", skin.skeleton, o);
  if (skin.name.size()) {
    SerializeStringProperty("name", skin.name, o);
  }
}

static void SerializeGltfTexture(Texture &texture, picojson::object &o) {
    SerializeNumberProperty("sampler", texture.sampler, o);
  SerializeNumberProperty("source", texture.source, o);

  if (texture.extras.Size()) {
    picojson::object extras;
    SerializeValue("extras", texture.extras, o);
    o.insert(json_object_pair("extras", picojson::value(extras)));
  }
}

static void WriteGltfFile(const std::string &output,
                          const std::string &content) {
  std::ofstream gltfFile(output.c_str());
  gltfFile << content << std::endl;
}

bool TinyGLTF::WriteGltfSceneToFile(
    Model *model,
    const std::string
        &filename /*, bool embedImages, bool embedBuffers, bool writeBinary*/) {
  picojson::object output;

  // ACCESSORS
  picojson::array accessors;
  for (unsigned int i = 0; i < model->accessors.size(); ++i) {
    picojson::object accessor;
    SerializeGltfAccessor(model->accessors[i], accessor);
    accessors.push_back(picojson::value(accessor));
  }
  output.insert(json_object_pair("accessors", picojson::value(accessors)));

  // ANIMATIONS
  if (model->animations.size()) {
    picojson::array animations;
    for (unsigned int i = 0; i < model->animations.size(); ++i) {
      if (model->animations[i].channels.size()) {
        picojson::object animation;
        SerializeGltfAnimation(model->animations[i], animation);
        animations.push_back(picojson::value(animation));
      }
    }
    output.insert(json_object_pair("animations", picojson::value(animations)));
  }

  // ASSET
  picojson::object asset;
  SerializeGltfAsset(model->asset, asset);
  output.insert(json_object_pair("asset", picojson::value(asset)));

  std::string binFilePath = filename;
  std::string ext = ".bin";
  std::string::size_type pos = binFilePath.rfind('.', binFilePath.length());

  if (pos != std::string::npos) {
    binFilePath = binFilePath.substr(0, pos) + ext;
  } else {
    binFilePath = "./" + binFilePath + ".bin";
  }

  // BUFFERS (We expect only one buffer here)
  picojson::array buffers;
  for (unsigned int i = 0; i < model->buffers.size(); ++i) {
    picojson::object buffer;
    SerializeGltfBuffer(model->buffers[i], buffer, binFilePath);
    buffers.push_back(picojson::value(buffer));
    }
  output.insert(json_object_pair("buffers", picojson::value(buffers)));

  // BUFFERVIEWS
  picojson::array bufferViews;
  for (unsigned int i = 0; i < model->bufferViews.size(); ++i) {
    picojson::object bufferView;
    SerializeGltfBufferView(model->bufferViews[i], bufferView);
    bufferViews.push_back(picojson::value(bufferView));
  }
  output.insert(json_object_pair("bufferViews", picojson::value(bufferViews)));

  // Extensions used
  if (model->extensionsUsed.size()) {
    SerializeStringArrayProperty("extensionsUsed", model->extensionsUsed,
                                 output);
  }

  // Extensions required
  if (model->extensionsRequired.size()) {
    SerializeStringArrayProperty("extensionsRequired",
                                 model->extensionsRequired, output);
  }

  // IMAGES
  picojson::array images;
    for (unsigned int i = 0; i < model->images.size(); ++i) {
    picojson::object image;
      SerializeGltfImage(model->images[i], image);
    images.push_back(picojson::value(image));
    }
  output.insert(json_object_pair("images", picojson::value(images)));

  // MATERIALS
  picojson::array materials;
    for (unsigned int i = 0; i < model->materials.size(); ++i) {
    picojson::object material;
      SerializeGltfMaterial(model->materials[i], material);
    materials.push_back(picojson::value(material));
    }
  output.insert(json_object_pair("materials", picojson::value(materials)));

  // MESHES
  picojson::array meshes;
    for (unsigned int i = 0; i < model->meshes.size(); ++i) {
    picojson::object mesh;
      SerializeGltfMesh(model->meshes[i], mesh);
    meshes.push_back(picojson::value(mesh));
    }
  output.insert(json_object_pair("meshes", picojson::value(meshes)));

  // NODES
  picojson::array nodes;
    for (unsigned int i = 0; i < model->nodes.size(); ++i) {
    picojson::object node;
      SerializeGltfNode(model->nodes[i], node);
    nodes.push_back(picojson::value(node));
    }
  output.insert(json_object_pair("nodes", picojson::value(nodes)));

  // SCENE
    SerializeNumberProperty<int>("scene", model->defaultScene, output);

  // SCENES
  picojson::array scenes;
    for (unsigned int i = 0; i < model->scenes.size(); ++i) {
    picojson::object currentScene;
      SerializeGltfScene(model->scenes[i], currentScene);
    scenes.push_back(picojson::value(currentScene));
    }
  output.insert(json_object_pair("scenes", picojson::value(scenes)));

  // SKINS
  if (model->skins.size()) {
    picojson::array skins;
    for (unsigned int i = 0; i < model->skins.size(); ++i) {
      picojson::object skin;
      SerializeGltfSkin(model->skins[i], skin);
      skins.push_back(picojson::value(picojson::value(skin)));
    }
    output.insert(json_object_pair("skins", picojson::value(skins)));
  }

  // TEXTURES
  picojson::array textures;
    for (unsigned int i = 0; i < model->textures.size(); ++i) {
    picojson::object texture;
      SerializeGltfTexture(model->textures[i], texture);
    textures.push_back(picojson::value(texture));
    }
  output.insert(json_object_pair("textures", picojson::value(textures)));

  // SAMPLERS
  picojson::array samplers;
    for (unsigned int i = 0; i < model->samplers.size(); ++i) {
    picojson::object sampler;
      SerializeGltfSampler(model->samplers[i], sampler);
    samplers.push_back(picojson::value(sampler));
    }
  output.insert(json_object_pair("samplers", picojson::value(samplers)));

  // CAMERAS
  picojson::array cameras;
    for (unsigned int i = 0; i < model->cameras.size(); ++i) {
    picojson::object camera;
      SerializeGltfCamera(model->cameras[i], camera);
    cameras.push_back(picojson::value(camera));
    }
  output.insert(json_object_pair("cameras", picojson::value(cameras)));

  // LIGHTS
  picojson::array lights;
    for (unsigned int i = 0; i < model->lights.size(); ++i) {
    picojson::object light;
      SerializeGltfLight(model->lights[i], light);
    lights.push_back(picojson::value(light));
    }
  output.insert(json_object_pair("lights", picojson::value(lights)));

  WriteGltfFile(filename, picojson::value(output).serialize());
  return true;
}

}  // namespace tinygltf

#endif  // TINYGLTF_IMPLEMENTATION
