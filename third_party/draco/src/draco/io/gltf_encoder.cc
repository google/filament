// Copyright 2018 The Draco Authors.
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
#include "draco/io/gltf_encoder.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <sys/types.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "draco/attributes/geometry_attribute.h"
#include "draco/attributes/point_attribute.h"
#include "draco/compression/draco_compression_options.h"
#include "draco/compression/expert_encode.h"
#include "draco/core/draco_types.h"
#include "draco/core/vector_d.h"
#include "draco/io/file_utils.h"
#include "draco/io/file_writer_utils.h"
#include "draco/io/gltf_utils.h"
#include "draco/io/texture_io.h"
#include "draco/mesh/mesh_features.h"
#include "draco/mesh/mesh_splitter.h"
#include "draco/mesh/mesh_utils.h"
#include "draco/metadata/property_attribute.h"
#include "draco/scene/instance_array.h"
#include "draco/scene/scene_indices.h"
#include "draco/scene/scene_utils.h"
#include "draco/texture/texture_utils.h"

namespace draco {

// Values are specfified from glTF 2.0 sampler spec. See here for more
// information:
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#sampler
int TextureFilterTypeToGltfValue(TextureMap::FilterType filter_type) {
  switch (filter_type) {
    case TextureMap::NEAREST:
      return 9728;
    case TextureMap::LINEAR:
      return 9729;
    case TextureMap::NEAREST_MIPMAP_NEAREST:
      return 9984;
    case TextureMap::LINEAR_MIPMAP_NEAREST:
      return 9985;
    case TextureMap::NEAREST_MIPMAP_LINEAR:
      return 9986;
    case TextureMap::LINEAR_MIPMAP_LINEAR:
      return 9987;
    default:
      return -1;
  }
}

// Values are specfified from glTF 2.0 sampler spec. See here for more
// information:
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#sampler
int TextureAxisWrappingModeToGltfValue(TextureMap::AxisWrappingMode mode) {
  switch (mode) {
    case TextureMap::CLAMP_TO_EDGE:
      return 33071;
    case TextureMap::MIRRORED_REPEAT:
      return 33648;
    case TextureMap::REPEAT:
      return 10497;
    default:
      return -1;
  }
}

// Returns a boolean indicating whether |mesh| attribute at |att_index| is a
// feature ID vertex attribute referred to by any of the feature ID sets stored
// in the |mesh|.
bool IsFeatureIdAttribute(int att_index, const Mesh &mesh) {
  for (MeshFeaturesIndex i(0); i < mesh.NumMeshFeatures(); ++i) {
    if (mesh.GetMeshFeatures(i).GetAttributeIndex() == att_index) {
      return true;
    }
  }
  return false;
}

// Returns a boolean indicating whether |mesh| attribute at |att_index| is a
// property attribute referred to by the |mesh| and its |structural_metadata|.
bool IsPropertyAttribute(int att_index, const Mesh &mesh,
                         const StructuralMetadata &structural_metadata) {
  // First check if structural metadata has any property attributes.
  if (structural_metadata.NumPropertyAttributes() == 0) {
    return false;
  }

  // Property attribute name must start with an underscore like _DIRECTION.
  const std::string attribute_name = mesh.attribute(att_index)->name();
  if (attribute_name.rfind('_', 0) != 0) {
    return false;
  }

  // Look for an |attribute_name| among all property attributes in the |mesh|.
  for (int i = 0; i < mesh.NumPropertyAttributesIndices(); ++i) {
    const int property_attribute_index = mesh.GetPropertyAttributesIndex(i);
    const PropertyAttribute &attribute =
        structural_metadata.GetPropertyAttribute(property_attribute_index);
    for (int i = 0; i < attribute.NumProperties(); ++i) {
      const PropertyAttribute::Property &property = attribute.GetProperty(i);
      if (property.GetAttributeName() == attribute_name) {
        return true;
      }
    }
  }
  return false;
}

// Struct to hold glTF Scene data.
struct GltfScene {
  std::vector<int> node_indices;
};

// Struct to hold glTF Node data.
struct GltfNode {
  GltfNode()
      : mesh_index(-1),
        skin_index(-1),
        light_index(-1),
        instance_array_index(-1),
        root_node(false) {}

  std::string name;
  std::vector<int> children_indices;
  int mesh_index;
  int skin_index;
  int light_index;
  int instance_array_index;
  bool root_node;
  TrsMatrix trs_matrix;
};

// Struct to hold image data.
struct GltfImage {
  std::string image_name;
  const Texture *texture;
  std::unique_ptr<Texture> owned_texture;
  int num_components = 0;
  int buffer_view = -1;
  std::string mime_type;
};

// Struct to hold texture filtering options. The members are based on glTF 2.0
// samplers. For more information see:
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#samplers
struct TextureSampler {
  TextureSampler(TextureMap::FilterType min, TextureMap::FilterType mag,
                 TextureMap::WrappingMode mode)
      : min_filter(min), mag_filter(mag), wrapping_mode(mode) {}

  bool operator==(const TextureSampler &other) const {
    if (min_filter != other.min_filter) {
      return false;
    }
    if (mag_filter != other.mag_filter) {
      return false;
    }
    return wrapping_mode.s == other.wrapping_mode.s &&
           wrapping_mode.t == other.wrapping_mode.t;
  }

  TextureMap::FilterType min_filter = TextureMap::UNSPECIFIED;
  TextureMap::FilterType mag_filter = TextureMap::UNSPECIFIED;
  TextureMap::WrappingMode wrapping_mode = {TextureMap::CLAMP_TO_EDGE,
                                            TextureMap::CLAMP_TO_EDGE};
};

// Struct to hold texture data. Multiple textures can reference the same image.
struct GltfTexture {
  GltfTexture(int image, int sampler)
      : image_index(image), sampler_index(sampler) {}
  bool operator==(const GltfTexture &other) const {
    return image_index == other.image_index &&
           sampler_index == other.sampler_index;
  }
  int image_index;
  int sampler_index;
};

// Struct to hold glTF Accessor data.
struct GltfAccessor {
  GltfAccessor()
      : buffer_view_index(-1),
        byte_stride(0),
        component_type(-1),
        normalized(false) {}

  int buffer_view_index;
  int byte_stride;
  int component_type;
  int64_t count;
  std::vector<GltfValue> max;
  std::vector<GltfValue> min;
  std::string type;
  bool normalized;
};

// Struct to hold glTF BufferView data. Currently there is only one Buffer, so
// there is no need to store a buffer index.
struct GltfBufferView {
  int64_t buffer_byte_offset = -1;
  int64_t byte_length = 0;
  int target = 0;
};

// Struct to hold information about a Draco compressed mesh.
struct GltfDracoCompressedMesh {
  int buffer_view_index = -1;
  std::map<std::string, int> attributes;
};

// Struct to hold glTF Primitive data.
struct GltfPrimitive {
  GltfPrimitive() : indices(-1), mode(4), material(0) {}

  int indices;
  int mode;
  int material;
  std::vector<MeshGroup::MaterialsVariantsMapping> material_variants_mappings;
  std::vector<const MeshFeatures *> mesh_features;
  std::vector<int> property_attributes;
  std::map<std::string, int> attributes;
  GltfDracoCompressedMesh compressed_mesh_info;

  // Map from the index of a feature ID vertex attribute in draco::Mesh to the
  // index in the feature ID vertex attribute name like _FEATURE_ID_5.
  std::unordered_map<int, int> feature_id_name_indices;
};

struct GltfMesh {
  std::string name;
  std::vector<GltfPrimitive> primitives;
};

// Class to hold and output glTF data.
class GltfAsset {
 public:
  // glTF value types and values.
  enum ComponentType {
    BYTE = 5120,
    UNSIGNED_BYTE = 5121,
    SHORT = 5122,
    UNSIGNED_SHORT = 5123,
    UNSIGNED_INT = 5125,
    FLOAT = 5126
  };
  // Return the size of the component based on |max_value|.
  static int UnsignedIntComponentSize(unsigned int max_value);

  // Return component type based on |max_value|.
  static ComponentType UnsignedIntComponentType(unsigned int max_value);

  GltfAsset();

  void set_copyright(const std::string &copyright) { copyright_ = copyright; }
  std::string copyright() const { return copyright_; }
  std::string generator() const { return generator_; }
  std::string version() const { return version_; }
  std::string buffer_name() const { return buffer_name_; }
  void buffer_name(const std::string &name) { buffer_name_ = name; }
  const EncoderBuffer *Buffer() const { return &buffer_; }

  // Convert a Draco Mesh to glTF data.
  bool AddDracoMesh(const Mesh &mesh);

  // Convert a Draco Scene to glTF data.
  Status AddScene(const Scene &scene);

  // Copy the glTF data to |buf_out|.
  Status Output(EncoderBuffer *buf_out);

  // Return the output image referenced by |index|.
  const GltfImage *GetImage(int index) const;

  // Return the number of images added to the GltfAsset.
  int NumImages() const { return images_.size(); }

  const std::string &image_name(int i) const { return images_[i].image_name; }

  void set_add_images_to_buffer(bool flag) { add_images_to_buffer_ = flag; }
  bool add_images_to_buffer() const { return add_images_to_buffer_; }
  void set_output_type(GltfEncoder::OutputType type) { output_type_ = type; }
  GltfEncoder::OutputType output_type() const { return output_type_; }
  void set_json_output_mode(JsonWriter::Mode mode) { gltf_json_.SetMode(mode); }

 private:
  // Pad |buffer_| to 4 byte boundary.
  bool PadBuffer();

  // Returns the index of the scene that was added. -1 on error.
  int AddScene();

  // Add a glTF attribute index to |draco_extension|.
  void AddAttributeToDracoExtension(
      const Mesh &mesh, GeometryAttribute::Type type, int index,
      const std::string &name, GltfDracoCompressedMesh *compressed_mesh_info);

  // Compresses |mesh| using Draco. On success returns the buffer_view in
  // |primitive| and number of encoded points and faces.
  Status CompressMeshWithDraco(const Mesh &mesh,
                               const Eigen::Matrix4d &transform,
                               GltfPrimitive *primitive,
                               int64_t *num_encoded_points,
                               int64_t *num_encoded_faces);

  // Adds a Draco mesh associated with a material id and material variants.
  bool AddDracoMesh(const Mesh &mesh, int material_id,
                    const std::vector<MeshGroup::MaterialsVariantsMapping>
                        &material_variants_mappings,
                    const Eigen::Matrix4d &transform);

  // Add the Draco mesh indices to the glTF data. |num_encoded_faces| is the
  // number of faces encoded in |mesh|, which can be different than
  // mesh.numfaces(). Returns the index of the accessor that was added. -1 on
  // error.
  int AddDracoIndices(const Mesh &mesh, int64_t num_encoded_faces);

  // Add the Draco mesh positions attribute to the glTF data.
  // |num_encoded_points| is the number of points encoded in |mesh|, which can
  // be different than mesh.num_points(). Returns the index of the accessor that
  // was added. -1 on error.
  int AddDracoPositions(const Mesh &mesh, int num_encoded_points);

  // Add the Draco mesh normals attribute to the glTF data. |num_encoded_points|
  // is the number of points encoded in |mesh|, which can be different than
  // mesh.num_points(). Returns the index of
  // the accessor that was added. -1 on error.
  int AddDracoNormals(const Mesh &mesh, int num_encoded_points);

  // Add the Draco mesh vertex color attribute to the glTF data.
  // |num_encoded_points| is the number of points encoded in |mesh|, which can
  // be different than mesh.num_points(). Returns the index of the accessor that
  // was added. -1 on error.
  int AddDracoColors(const Mesh &mesh, int num_encoded_points);

  // Add the Draco mesh texture attribute to the glTF data. |tex_coord_index| is
  // the index into the texture coordinates added to |mesh|.
  // |num_encoded_points| is the number of points encoded in |mesh|, which can
  // be different than mesh.num_points(). Returns the index of the accessor that
  // was added. -1 on error.
  int AddDracoTexture(const Mesh &mesh, int tex_coord_index,
                      int num_encoded_points);

  // Add the Draco mesh tangent attribute to the glTF data. The Draco mesh
  // tangents only contains the x, y, and z components and glTF needs the
  // x, y, z, and w components for glTF mesh tangents. Note this is not true
  // for tangents of glTF morph targets. This function will add the w component
  // to the glTF tangents. |num_encoded_points| is the
  // number of points encoded in |mesh|, which can be different than
  // mesh.num_points(). Returns the index of the accessor that was added.
  // -1 on error.
  // Note: Tangents are not added if the attribute contains "auto_generated"
  // metadata. See go/tangents_and_draco_simplifier for more details.
  int AddDracoTangents(const Mesh &mesh, int num_encoded_points);

  int AddDracoJoints(const Mesh &mesh, int num_encoded_points);
  int AddDracoWeights(const Mesh &mesh, int num_encoded_points);
  std::vector<std::pair<std::string, int>> AddDracoGenerics(
      const Mesh &mesh, int num_encoded_points,
      std::unordered_map<int, int> *feature_id_name_indices);

  // Iterate through the materials that are associated with |mesh| and add them
  // to the asset.
  void AddMaterials(const Mesh &mesh);

  // Checks whether a given Draco |attribute| has data of expected |data_type|
  // and whether the data has one of expected |num_components|. Returns true
  // when the |attribute| meets expectations, false otherwise.
  static bool CheckDracoAttribute(const PointAttribute *attribute,
                                  const std::set<DataType> &data_types,
                                  const std::set<int> &num_components);

  // Returns the name of |texture|. If |texture|'s name is empty then it will
  // generate a name using |texture_index| and |suffix|. If it cannot generate a
  // name then it will return an empty string.
  std::string GetTextureName(const Texture &texture, int texture_index,
                             const std::string &suffix) const;

  // Adds a new glTF image to the asset and returns its index. |owned_texture|
  // is an optional argument that can be used when the added image is not
  // contained in the encoded MaterialLibrary (e.g. for images that are locally
  // modified before they are encoded to disk). The image file name is generated
  // by combining |image_stem| and image mime type contained in the |texture|.
  StatusOr<int> AddImage(const std::string &image_stem, const Texture *texture,
                         int num_components);
  StatusOr<int> AddImage(const std::string &image_stem, const Texture *texture,
                         std::unique_ptr<Texture> owned_texture,
                         int num_components);

  // Saves an image with a given |image_index| into a buffer.
  Status SaveImageToBuffer(int image_index);

  // Adds |sampler| to vector of samplers and returns the index. If |sampler| is
  // equal to default values then |sampler| is not added to the vector and
  // returns -1.
  StatusOr<int> AddTextureSampler(const TextureSampler &sampler);

  // Adds a Draco SceneNode, referenced by |scene_node_index|, to the glTF data.
  Status AddSceneNode(const Scene &scene, SceneNodeIndex scene_node_index);

  // Iterate through the materials that are associated with |scene| and add them
  // to the asset.
  void AddMaterials(const Scene &scene);

  // Iterate through the animations that are associated with |scene| and add
  // them to the asset. Returns OkStatus() if |scene| does not contain any
  // animations.
  Status AddAnimations(const Scene &scene);

  // Converts the data associated with |node_animation_data| and adds that to
  // the encoder as an accessor.
  StatusOr<int> AddNodeAnimationData(
      const NodeAnimationData &node_animation_data);

  // Iterate through the skins that are associated with |scene| and add
  // them to the asset. Returns OkStatus() if |scene| does not contain any
  // skins.
  Status AddSkins(const Scene &scene);

  // Iterate through the lights that are associated with |scene| and add them to
  // the asset. Returns OkStatus() if |scene| does not contain any lights.
  Status AddLights(const Scene &scene);

  // Iterate through materials variants names that are associated with |scene|
  // and add them to the asset. Returns OkStatus() if |scene| does not contain
  // any materials variants.
  Status AddMaterialsVariantsNames(const Scene &scene);

  // Iterate through the mesh group instance arrays that are associated with
  // |scene| and add them to the asset. Returns OkStatus() if |scene| does not
  // contain any mesh group instance arrays.
  Status AddInstanceArrays(const Scene &scene);

  // Adds structural metadata from |geometry| to the asset, if any.
  template <typename GeometryT>
  void AddStructuralMetadata(const GeometryT &geometry);

  // Adds float |data| representing |num_components|-length vectors to the
  // encoder as accessor and return the new accessor index.
  StatusOr<int> AddData(const std::vector<float> &data, int num_components);

  // Adds property table |data| as buffer view and returns buffer view index.
  StatusOr<int> AddBufferView(const PropertyTable::Property::Data &data);

  bool EncodeAssetProperty(EncoderBuffer *buf_out);
  bool EncodeScenesProperty(EncoderBuffer *buf_out);
  bool EncodeInitialSceneProperty(EncoderBuffer *buf_out);
  bool EncodeNodesProperty(EncoderBuffer *buf_out);
  Status EncodeMeshesProperty(EncoderBuffer *buf_out);
  Status EncodePrimitiveExtensionsProperty(const GltfPrimitive &primitive,
                                           EncoderBuffer *buf_out);
  Status EncodeMaterials(EncoderBuffer *buf_out);

  // Encodes a color material. |red|, |green|, |blue|, |alpha|, and
  // |metallic_factor| are values in the range of 0.0 - 1.0.
  void EncodeColorMaterial(float red, float green, float blue, float alpha,
                           float metallic_factor);
  Status EncodeDefaultMaterial(EncoderBuffer *buf_out);

  // Encodes a texture map. |object_name| is the name of the texture map.
  // |image_index| is the index into the texture image array. |tex_coord_index|
  // is the index into the texture coordinates. |texture_map| is a reference to
  // the texture map that is going to be encoded.
  Status EncodeTextureMap(const std::string &object_name, int image_index,
                          int tex_coord_index, const Material &material,
                          const TextureMap &texture_map);

  // Encodes a texture map similar to the method above. When the |object_name|
  // is "texture" and |channels| is not empty, then the |channels| is encoded
  // into the "channels" property as required by the "texture" object of the
  // EXT_mesh_features extension.
  Status EncodeTextureMap(const std::string &object_name, int image_index,
                          int tex_coord_index, const Material &material,
                          const TextureMap &texture_map,
                          const std::vector<int> &channels);
  Status EncodeMaterialsProperty(EncoderBuffer *buf_out);

  void EncodeMaterialUnlitExtension(const Material &material);
  Status EncodeMaterialSheenExtension(const Material &material,
                                      const Material &defaults,
                                      int material_index);
  Status EncodeMaterialTransmissionExtension(const Material &material,
                                             const Material &defaults,
                                             int material_index);
  Status EncodeMaterialClearcoatExtension(const Material &material,
                                          const Material &defaults,
                                          int material_index);
  Status EncodeMaterialVolumeExtension(const Material &material,
                                       const Material &defaults,
                                       int material_index);
  Status EncodeMaterialIorExtension(const Material &material,
                                    const Material &defaults);
  Status EncodeMaterialSpecularExtension(const Material &material,
                                         const Material &defaults,
                                         int material_index);
  Status EncodeTexture(const std::string &name, const std::string &stem_suffix,
                       TextureMap::Type type, int num_components,
                       const Material &material, int material_index);
  Status EncodeAnimationsProperty(EncoderBuffer *buf_out);
  Status EncodeSkinsProperty(EncoderBuffer *buf_out);
  Status EncodeTopLevelExtensionsProperty(EncoderBuffer *buf_out);
  Status EncodeLightsProperty(EncoderBuffer *buf_out);
  Status EncodeMaterialsVariantsNamesProperty(EncoderBuffer *buf_out);
  Status EncodeStructuralMetadataProperty(EncoderBuffer *buf_out);
  bool EncodeAccessorsProperty(EncoderBuffer *buf_out);
  bool EncodeBufferViewsProperty(EncoderBuffer *buf_out);
  bool EncodeBuffersProperty(EncoderBuffer *buf_out);
  Status EncodeExtensionsProperties(EncoderBuffer *buf_out);

  // Encodes a draco::VectorNX as a glTF array.
  template <typename T>
  void EncodeVectorArray(const std::string &array_name, T vec) {
    gltf_json_.BeginArray(array_name);
    for (int i = 0; i < T::dimension; ++i) {
      gltf_json_.OutputValue(vec[i]);
    }
    gltf_json_.EndArray();
  }

  // Add a mesh Draco attribute |att| to the glTF data. Returns the index
  // accessor added to the glTF data. Returns -1 on error.
  int AddAttribute(const PointAttribute &att, int num_points,
                   int num_encoded_points, bool compress) {
    const int nep = num_encoded_points;
    switch (att.data_type()) {
      case DT_UINT8:
        return AddAttribute<uint8_t>(att, num_points, nep, compress);
      case DT_UINT16:
        return AddAttribute<uint16_t>(att, num_points, nep, compress);
      case DT_FLOAT32:
        return AddAttribute<float>(att, num_points, nep, compress);
      default:
        return -1;
    }
  }

  // Add a mesh Draco attribute |att| that is comprised of |att_data_t| values
  // to the glTF data. Returns the index accessor added to the glTF data.
  // Returns -1 on error.
  template <class att_data_t>
  int AddAttribute(const PointAttribute &att, int num_points,
                   int num_encoded_points, bool compress) {
    const int num_components = att.num_components();
    switch (num_components) {
      case 1:
        return AddAttribute<1, att_data_t>(att, num_points, num_encoded_points,
                                           "SCALAR", compress);
        break;
      case 2:
        return AddAttribute<2, att_data_t>(att, num_points, num_encoded_points,
                                           "VEC2", compress);
        break;
      case 3:
        return AddAttribute<3, att_data_t>(att, num_points, num_encoded_points,
                                           "VEC3", compress);
        break;
      case 4:
        return AddAttribute<4, att_data_t>(att, num_points, num_encoded_points,
                                           "VEC4", compress);
        break;
      default:
        break;
    }
    return -1;
  }

  // Template method only has specialized implementations for known glTF types.
  template <class att_data_t>
  ComponentType GetComponentType() const = delete;

  template <size_t att_components_t, class att_data_t>
  int AddAttribute(const PointAttribute &att, int num_points,
                   int num_encoded_points, const std::string &type,
                   bool compress);

  void SetCopyrightFromScene(const Scene &scene);
  void SetCopyrightFromMesh(const Mesh &mesh);

  std::string copyright_;
  std::string generator_;
  std::string version_;
  std::vector<GltfScene> scenes_;

  // Initial scene to load.
  int scene_index_;

  std::vector<GltfNode> nodes_;
  std::vector<GltfAccessor> accessors_;
  std::vector<GltfBufferView> buffer_views_;
  std::vector<GltfMesh> meshes_;

  // Data structure to copy the input meshes materials.
  MaterialLibrary material_library_;

  std::vector<GltfImage> images_;
  std::vector<GltfTexture> textures_;

  std::unordered_map<const Texture *, int> texture_to_image_index_map_;

  std::string buffer_name_;
  EncoderBuffer buffer_;
  JsonWriter gltf_json_;

  // Keeps track if the glTF mesh has been added.
  std::map<MeshGroupIndex, int> mesh_group_index_to_gltf_mesh_;
  std::map<MeshIndex, std::pair<int, int>> mesh_index_to_gltf_mesh_primitive_;
  IndexTypeVector<MeshIndex, Eigen::Matrix4d> base_mesh_transforms_;

  struct EncoderAnimation {
    std::string name;
    std::vector<std::unique_ptr<AnimationSampler>> samplers;
    std::vector<std::unique_ptr<AnimationChannel>> channels;
  };
  std::vector<std::unique_ptr<EncoderAnimation>> animations_;

  struct EncoderSkin {
    EncoderSkin() : inverse_bind_matrices_index(-1), skeleton_index(-1) {}
    int inverse_bind_matrices_index;
    std::vector<int> joints;
    int skeleton_index;
  };

  // Instance array is represented by its attribute accessors.
  struct EncoderInstanceArray {
    EncoderInstanceArray() : translation(-1), rotation(-1), scale(-1) {}
    int translation;
    int rotation;
    int scale;
  };

  std::vector<std::unique_ptr<EncoderSkin>> skins_;
  std::vector<std::unique_ptr<Light>> lights_;
  std::vector<std::string> materials_variants_names_;
  std::vector<EncoderInstanceArray> instance_arrays_;
  const StructuralMetadata *structural_metadata_;

  // Indicates whether Draco compression is used for any of the asset meshes.
  bool draco_compression_used_;

  // Indicates whether mesh features are used.
  bool mesh_features_used_;

  // Indicates whether structural metadata is used.
  bool structural_metadata_used_;

  // Counter for naming mesh feature textures.
  int mesh_features_texture_index_;

  // If set GltfAsset will add the images to |buffer_| instead of writing the
  // images to separate files.
  bool add_images_to_buffer_;

  // Used to hold the extensions used and required by the glTF asset.
  std::set<std::string> extensions_used_;
  std::set<std::string> extensions_required_;

  std::vector<TextureSampler> texture_samplers_;

  GltfEncoder::OutputType output_type_;

  // Temporary storage for meshes created during the runtime of the GltfEncoder.
  // We need to store them here to ensure their content doesn't get deleted
  // before it is used by the encoder.
  std::vector<std::unique_ptr<Mesh>> local_meshes_;
};

int GltfAsset::UnsignedIntComponentSize(unsigned int max_value) {
  // According to GLTF 2.0 spec, 0xff (and 0xffff respectively) are reserved for
  // the primitive restart symbol.
  if (max_value < 0xff) {
    return 1;
  } else if (max_value < 0xffff) {
    return 2;
  }
  return 4;
}

GltfAsset::ComponentType GltfAsset::UnsignedIntComponentType(
    unsigned int max_value) {
  // According to GLTF 2.0 spec, 0xff (and 0xffff respectively) are reserved for
  // the primitive restart symbol.
  if (max_value < 0xff) {
    return UNSIGNED_BYTE;
  } else if (max_value < 0xffff) {
    return UNSIGNED_SHORT;
  }
  return UNSIGNED_INT;
}

GltfAsset::GltfAsset()
    : generator_("draco_decoder"),
      version_("2.0"),
      scene_index_(-1),
      buffer_name_("buffer0.bin"),
      structural_metadata_(nullptr),
      draco_compression_used_(false),
      mesh_features_used_(false),
      structural_metadata_used_(false),
      mesh_features_texture_index_(0),
      add_images_to_buffer_(false),
      output_type_(GltfEncoder::COMPACT) {}

bool GltfAsset::AddDracoMesh(const Mesh &mesh) {
  const int scene_index = AddScene();
  if (scene_index < 0) {
    return false;
  }
  AddMaterials(mesh);

  GltfMesh gltf_mesh;
  meshes_.push_back(gltf_mesh);

  AddStructuralMetadata(mesh);
  if (copyright_.empty()) {
    SetCopyrightFromMesh(mesh);
  }

  const int32_t material_att_id =
      mesh.GetNamedAttributeId(GeometryAttribute::MATERIAL);
  if (material_att_id == -1) {
    if (!AddDracoMesh(mesh, 0, {}, Eigen::Matrix4d::Identity())) {
      return false;
    }
  } else {
    const auto mat_att = mesh.GetNamedAttribute(GeometryAttribute::MATERIAL);

    // Split mesh using the material attribute.
    MeshSplitter splitter;
    auto split_maybe = splitter.SplitMesh(mesh, material_att_id);
    if (!split_maybe.ok()) {
      return false;
    }
    auto split_meshes = std::move(split_maybe).value();
    for (int i = 0; i < split_meshes.size(); ++i) {
      if (split_meshes[i] == nullptr) {
        continue;  // Empty mesh. Ignore.
      }
      uint32_t mat_index = 0;
      mat_att->GetValue(AttributeValueIndex(i), &mat_index);

      // Copy over mesh features for a given material index.
      Mesh::CopyMeshFeaturesForMaterial(mesh, split_meshes[i].get(), mat_index);

      // Copy over property attributes indices for a given material index.
      Mesh::CopyPropertyAttributesIndicesForMaterial(
          mesh, split_meshes[i].get(), mat_index);

      // Move the split mesh to a temporary storage of the GltfAsset. This will
      // ensure the mesh will stay alive as long the asset needs it. We have to
      // do this because the split mesh may contain mesh features data that are
      // used later in the encoding process.
      local_meshes_.push_back(std::move(split_meshes[i]));

      // The material index in the glTF file corresponds to the index of the
      // split mesh.
      if (!AddDracoMesh(*(local_meshes_.back().get()), mat_index, {},
                        Eigen::Matrix4d::Identity())) {
        return false;
      }
    }
  }

  // Currently output only one mesh.
  GltfNode mesh_node;
  mesh_node.mesh_index = 0;
  nodes_.push_back(mesh_node);
  nodes_.back().root_node = true;
  return true;
}

int GltfAsset::AddScene() {
  GltfScene scene;
  scenes_.push_back(scene);
  const int scene_index = static_cast<int>(scenes_.size()) - 1;

  if (scene_index_ == -1) {
    scene_index_ = scene_index;
  }
  return scene_index;
}

Status GltfAsset::Output(EncoderBuffer *buf_out) {
  gltf_json_.BeginObject();
  if (!EncodeAssetProperty(buf_out)) {
    return Status(Status::DRACO_ERROR, "Failed encoding asset.");
  }
  if (!EncodeScenesProperty(buf_out)) {
    return Status(Status::DRACO_ERROR, "Failed encoding scenes.");
  }
  if (!EncodeInitialSceneProperty(buf_out)) {
    return Status(Status::DRACO_ERROR, "Failed encoding initial scene.");
  }
  if (!EncodeNodesProperty(buf_out)) {
    return Status(Status::DRACO_ERROR, "Failed encoding nodes.");
  }
  DRACO_RETURN_IF_ERROR(EncodeMeshesProperty(buf_out));
  DRACO_RETURN_IF_ERROR(EncodeMaterials(buf_out));
  if (!EncodeAccessorsProperty(buf_out)) {
    return Status(Status::DRACO_ERROR, "Failed encoding accessors.");
  }
  DRACO_RETURN_IF_ERROR(EncodeAnimationsProperty(buf_out));
  DRACO_RETURN_IF_ERROR(EncodeSkinsProperty(buf_out));
  DRACO_RETURN_IF_ERROR(EncodeTopLevelExtensionsProperty(buf_out));
  if (!EncodeBufferViewsProperty(buf_out)) {
    return Status(Status::DRACO_ERROR, "Failed encoding buffer views.");
  }
  if (!EncodeBuffersProperty(buf_out)) {
    return Status(Status::DRACO_ERROR, "Failed encoding buffers.");
  }
  DRACO_RETURN_IF_ERROR(EncodeExtensionsProperties(buf_out));
  gltf_json_.EndObject();

  const std::string asset_str = gltf_json_.MoveData();
  if (!buf_out->Encode(asset_str.data(), asset_str.length())) {
    return Status(Status::DRACO_ERROR, "Failed encoding json data.");
  }
  if (!buf_out->Encode("\n", 1)) {
    return Status(Status::DRACO_ERROR, "Failed encoding json data.");
  }
  return OkStatus();
}

const GltfImage *GltfAsset::GetImage(int index) const {
  if (index < 0 || index >= images_.size()) {
    return nullptr;
  }
  return &images_[index];
}

bool GltfAsset::PadBuffer() {
  if (buffer_.size() % 4 != 0) {
    const int pad_bytes = 4 - buffer_.size() % 4;
    const int pad_data = 0;
    if (!buffer_.Encode(&pad_data, pad_bytes)) {
      return false;
    }
  }
  return true;
}

void GltfAsset::AddAttributeToDracoExtension(
    const Mesh &mesh, GeometryAttribute::Type type, int index,
    const std::string &name, GltfDracoCompressedMesh *compressed_mesh_info) {
  if (mesh.IsCompressionEnabled()) {
    const PointAttribute *const att = mesh.GetNamedAttribute(type, index);
    if (att) {
      compressed_mesh_info->attributes.insert(
          std::pair<std::string, int>(name, att->unique_id()));
    }
  }
}

Status GltfAsset::CompressMeshWithDraco(const Mesh &mesh,
                                        const Eigen::Matrix4d &transform,
                                        GltfPrimitive *primitive,
                                        int64_t *num_encoded_points,
                                        int64_t *num_encoded_faces) {
  // Check that geometry comression options are valid.
  DracoCompressionOptions compression_options = mesh.GetCompressionOptions();
  DRACO_RETURN_IF_ERROR(compression_options.Check());

  // Make a copy of the mesh. It will be modified and compressed.
  std::unique_ptr<Mesh> mesh_copy(new Mesh());
  mesh_copy->Copy(mesh);

  // Delete auto-generated tangents.
  if (MeshUtils::HasAutoGeneratedTangents(*mesh_copy)) {
    for (int i = 0; i < mesh_copy->num_attributes(); ++i) {
      PointAttribute *const att = mesh_copy->attribute(i);
      if (att->attribute_type() == GeometryAttribute::TANGENT) {
        while (mesh_copy->GetNamedAttribute(GeometryAttribute::TANGENT)) {
          mesh_copy->DeleteAttribute(
              mesh_copy->GetNamedAttributeId(GeometryAttribute::TANGENT));
        }
        break;
      }
    }
  }

  // Create Draco encoder.
  EncoderBuffer buffer;
  std::unique_ptr<ExpertEncoder> encoder;
  if (mesh_copy->num_faces() > 0) {
    // Encode mesh.
    encoder.reset(new ExpertEncoder(*mesh_copy));
  } else {
    return Status(Status::DRACO_ERROR,
                  "Draco compression is not supported for glTF point clouds.");
  }
  encoder->SetTrackEncodedProperties(true);

  // Convert compression level to speed (that 0 = slowest, 10 = fastest).
  const int speed = 10 - compression_options.compression_level;
  encoder->SetSpeedOptions(speed, speed);

  // Configure attribute quantization.
  for (int i = 0; i < mesh_copy->num_attributes(); ++i) {
    const PointAttribute *const att = mesh_copy->attribute(i);
    if (att->attribute_type() == GeometryAttribute::POSITION &&
        !compression_options.quantization_position
             .AreQuantizationBitsDefined()) {
      // Desired spacing in the "global" coordinate system.
      const float global_spacing =
          compression_options.quantization_position.spacing();

      // Note: Ideally we would transform the whole mesh before encoding and
      // apply the original global spacing on the transformed mesh. But neither
      // KHR_draco_mesh_compression, nor Draco bitstream support post-decoding
      // transformations so we have to modify the grid settings here.

      // Transform this spacing to the local coordinate system of the base mesh.
      // We will get the largest scale factor from the transformation matrix and
      // use it to adjust the grid spacing.
      const Vector3f scale_vec(transform.col(0).norm(), transform.col(1).norm(),
                               transform.col(2).norm());

      const float max_scale = scale_vec.MaxCoeff();

      // Spacing is inverse to the scale. The larger the scale, the smaller the
      // spacing must be.
      const float local_spacing = global_spacing / max_scale;

      // Update the compression options of the processed mesh.
      compression_options.quantization_position.SetGrid(local_spacing);
    } else {
      int num_quantization_bits = -1;
      switch (att->attribute_type()) {
        case GeometryAttribute::POSITION:
          num_quantization_bits =
              compression_options.quantization_position.quantization_bits();
          break;
        case GeometryAttribute::NORMAL:
          num_quantization_bits = compression_options.quantization_bits_normal;
          break;
        case GeometryAttribute::TEX_COORD:
          num_quantization_bits =
              compression_options.quantization_bits_tex_coord;
          break;
        case GeometryAttribute::TANGENT:
          num_quantization_bits = compression_options.quantization_bits_tangent;
          break;
        case GeometryAttribute::WEIGHTS:
          num_quantization_bits = compression_options.quantization_bits_weight;
          break;
        case GeometryAttribute::GENERIC:
          if (!IsFeatureIdAttribute(i, *mesh_copy)) {
            num_quantization_bits =
                compression_options.quantization_bits_generic;
          } else {
            // Quantization is explicitly disabled for feature ID attributes.
            encoder->SetAttributeQuantization(i, -1);
          }
          break;
        default:
          break;
      }
      if (num_quantization_bits > 0) {
        encoder->SetAttributeQuantization(i, num_quantization_bits);
      }
    }
  }

  // Flip UV values as required by glTF Draco and non-Draco files.
  for (int i = 0; i < mesh_copy->num_attributes(); ++i) {
    PointAttribute *const att = mesh_copy->attribute(i);
    if (att->attribute_type() == GeometryAttribute::TEX_COORD) {
      if (!MeshUtils::FlipTextureUvValues(false, true, att)) {
        return Status(Status::DRACO_ERROR, "Could not flip texture UV values.");
      }
    }
  }

  // Change tangents, joints, and weights attribute types to generic. The
  // original mesh's attribute type is unchanged and the mapping of the glTF
  // attribute type to Draco compressed attribute id is written to the output
  // glTF file.
  for (int i = 0; i < mesh_copy->num_attributes(); ++i) {
    PointAttribute *const att = mesh_copy->attribute(i);
    if (att->attribute_type() == GeometryAttribute::TANGENT ||
        att->attribute_type() == GeometryAttribute::JOINTS ||
        att->attribute_type() == GeometryAttribute::WEIGHTS) {
      att->set_attribute_type(GeometryAttribute::GENERIC);
    }
  }

  // |compression_options| may have been modified and we need to update them
  // before we start the encoding.
  mesh_copy->SetCompressionOptions(compression_options);
  DRACO_RETURN_IF_ERROR(encoder->EncodeToBuffer(&buffer));
  *num_encoded_points = encoder->num_encoded_points();
  if (mesh_copy->num_faces() > 0) {
    *num_encoded_faces = encoder->num_encoded_faces();
  } else {
    *num_encoded_faces = 0;
  }
  const size_t buffer_start_offset = buffer_.size();
  if (!buffer_.Encode(buffer.data(), buffer.size())) {
    return Status(Status::DRACO_ERROR, "Could not copy Draco compressed data.");
  }
  if (!PadBuffer()) {
    return Status(Status::DRACO_ERROR, "Could not pad glTF buffer.");
  }

  GltfBufferView buffer_view;
  buffer_view.buffer_byte_offset = buffer_start_offset;
  buffer_view.byte_length = buffer_.size() - buffer_start_offset;
  buffer_views_.push_back(buffer_view);
  primitive->compressed_mesh_info.buffer_view_index =
      static_cast<int>(buffer_views_.size() - 1);
  return OkStatus();
}

bool CheckAndGetTexCoordAttributeOrder(const Mesh &mesh,
                                       std::vector<int> *tex_coord_order) {
  // We will only consider at most two texture coordinate attributes.
  *tex_coord_order = {0, 1};
  const int num_attributes =
      std::min(mesh.NumNamedAttributes(GeometryAttribute::TEX_COORD), 2);

  // Collect texture coordinate attribute names from metadata.
  std::vector<std::string> names(num_attributes, "");
  for (int i = 0; i < num_attributes; i++) {
    const auto metadata = mesh.GetAttributeMetadataByAttributeId(
        mesh.GetNamedAttributeId(GeometryAttribute::TEX_COORD, i));
    std::string attribute_name;
    if (metadata != nullptr) {
      metadata->GetEntryString("attribute_name", &attribute_name);
      names[i] = attribute_name;
    }
  }

  // Attribute names may be absent.
  if (num_attributes == 0 ||
      std::all_of(names.begin(), names.end(),
                  [](const std::string &name) { return name.empty(); })) {
    return true;
  }

  // Attribute names must be unique.
  const std::unordered_set<std::string> unique_names(names.begin(),
                                                     names.end());
  if (unique_names.size() != num_attributes) {
    return false;
  }

  // Attribute names must be valid.
  if (std::any_of(names.begin(), names.end(), [](const std::string &name) {
        return name != "TEXCOORD_0" && name != "TEXCOORD_1";
      })) {
    return false;
  }

  // Populate texture coordinate order index based on attribute names.
  if (names[0] == "TEXCOORD_1") {
    *tex_coord_order = {1, 0};
  }
  return true;
}

bool GltfAsset::AddDracoMesh(
    const Mesh &mesh, int material_id,
    const std::vector<MeshGroup::MaterialsVariantsMapping>
        &material_variants_mappings,
    const Eigen::Matrix4d &transform) {
  GltfPrimitive primitive;
  int64_t num_encoded_points = mesh.num_points();
  int64_t num_encoded_faces = mesh.num_faces();
  if (num_encoded_faces > 0 && mesh.IsCompressionEnabled()) {
    const Status status = CompressMeshWithDraco(
        mesh, transform, &primitive, &num_encoded_points, &num_encoded_faces);
    if (!status.ok()) {
      return false;
    }
    draco_compression_used_ = true;
  }
  int indices_index = -1;
  if (num_encoded_faces > 0) {
    indices_index = AddDracoIndices(mesh, num_encoded_faces);
    if (indices_index < 0) {
      return false;
    }
  }
  const int position_index = AddDracoPositions(mesh, num_encoded_points);
  if (position_index < 0) {
    return false;
  }
  // Check texture coordinate attributes and get the desired encoding order.
  std::vector<int> tex_coord_order;
  if (!CheckAndGetTexCoordAttributeOrder(mesh, &tex_coord_order)) {
    return false;
  }
  const int normals_accessor_index = AddDracoNormals(mesh, num_encoded_points);
  const int colors_accessor_index = AddDracoColors(mesh, num_encoded_points);
  const int texture0_accessor_index =
      AddDracoTexture(mesh, tex_coord_order[0], num_encoded_points);
  const int texture1_accessor_index =
      AddDracoTexture(mesh, tex_coord_order[1], num_encoded_points);
  const int tangent_accessor_index = AddDracoTangents(mesh, num_encoded_points);
  const int joints_accessor_index = AddDracoJoints(mesh, num_encoded_points);
  const int weights_accessor_index = AddDracoWeights(mesh, num_encoded_points);
  const std::vector<std::pair<std::string, int>> generics_accessors =
      AddDracoGenerics(mesh, num_encoded_points,
                       &primitive.feature_id_name_indices);

  if (num_encoded_faces == 0) {
    primitive.mode = 0;  // POINTS mode.
  }
  primitive.material = material_id;
  primitive.material_variants_mappings = material_variants_mappings;
  primitive.mesh_features.reserve(mesh.NumMeshFeatures());
  for (MeshFeaturesIndex i(0); i < mesh.NumMeshFeatures(); ++i) {
    primitive.mesh_features.push_back(&mesh.GetMeshFeatures(i));
  }
  primitive.property_attributes.reserve(mesh.NumPropertyAttributesIndices());
  for (int i = 0; i < mesh.NumPropertyAttributesIndices(); ++i) {
    primitive.property_attributes.push_back(mesh.GetPropertyAttributesIndex(i));
  }
  primitive.indices = indices_index;
  primitive.attributes.insert(
      std::pair<std::string, int>("POSITION", position_index));
  AddAttributeToDracoExtension(mesh, GeometryAttribute::POSITION, 0, "POSITION",
                               &primitive.compressed_mesh_info);
  if (normals_accessor_index > 0) {
    primitive.attributes.insert(
        std::pair<std::string, int>("NORMAL", normals_accessor_index));
    AddAttributeToDracoExtension(mesh, GeometryAttribute::NORMAL, 0, "NORMAL",
                                 &primitive.compressed_mesh_info);
  }
  if (colors_accessor_index > 0) {
    primitive.attributes.insert(
        std::pair<std::string, int>("COLOR_0", colors_accessor_index));
    AddAttributeToDracoExtension(mesh, GeometryAttribute::COLOR, 0, "COLOR_0",
                                 &primitive.compressed_mesh_info);
  }
  if (texture0_accessor_index > 0) {
    primitive.attributes.insert(
        std::pair<std::string, int>("TEXCOORD_0", texture0_accessor_index));
    AddAttributeToDracoExtension(mesh, GeometryAttribute::TEX_COORD, 0,
                                 "TEXCOORD_0", &primitive.compressed_mesh_info);
  }
  if (texture1_accessor_index > 0) {
    primitive.attributes.insert(
        std::pair<std::string, int>("TEXCOORD_1", texture1_accessor_index));
    AddAttributeToDracoExtension(mesh, GeometryAttribute::TEX_COORD, 1,
                                 "TEXCOORD_1", &primitive.compressed_mesh_info);
  }
  if (tangent_accessor_index > 0) {
    primitive.attributes.insert(
        std::pair<std::string, int>("TANGENT", tangent_accessor_index));
    AddAttributeToDracoExtension(mesh, GeometryAttribute::TANGENT, 0, "TANGENT",
                                 &primitive.compressed_mesh_info);
  }
  if (joints_accessor_index > 0) {
    primitive.attributes.insert(
        std::pair<std::string, int>("JOINTS_0", joints_accessor_index));
    AddAttributeToDracoExtension(mesh, GeometryAttribute::JOINTS, 0, "JOINTS_0",
                                 &primitive.compressed_mesh_info);
  }
  if (weights_accessor_index > 0) {
    primitive.attributes.insert(
        std::pair<std::string, int>("WEIGHTS_0", weights_accessor_index));
    AddAttributeToDracoExtension(mesh, GeometryAttribute::WEIGHTS, 0,
                                 "WEIGHTS_0", &primitive.compressed_mesh_info);
  }
  for (int att_index = 0; att_index < generics_accessors.size(); ++att_index) {
    const std::string &attribute_name = generics_accessors[att_index].first;
    if (!attribute_name.empty()) {
      primitive.attributes.insert(generics_accessors[att_index]);
      AddAttributeToDracoExtension(mesh, GeometryAttribute::GENERIC, att_index,
                                   attribute_name,
                                   &primitive.compressed_mesh_info);
    }
  }

  meshes_.back().primitives.push_back(primitive);
  return true;
}

int GltfAsset::AddDracoIndices(const Mesh &mesh, int64_t num_encoded_faces) {
  // Get the min and max value for the indices.
  uint32_t min_index = 0xffffffff;
  uint32_t max_index = 0;
  for (FaceIndex i(0); i < mesh.num_faces(); ++i) {
    const auto &f = mesh.face(i);

    for (int j = 0; j < 3; ++j) {
      if (f[j] < min_index) {
        min_index = f[j].value();
      }
      if (f[j] > max_index) {
        max_index = f[j].value();
      }
    }
  }

  const int component_size = GltfAsset::UnsignedIntComponentSize(max_index);

  GltfAccessor accessor;
  if (!mesh.IsCompressionEnabled()) {
    const size_t buffer_start_offset = buffer_.size();
    for (FaceIndex i(0); i < mesh.num_faces(); ++i) {
      const auto &f = mesh.face(i);
      for (int j = 0; j < 3; ++j) {
        int index = f[j].value();
        if (!buffer_.Encode(&index, component_size)) {
          return -1;
        }
      }
    }

    if (!PadBuffer()) {
      return -1;
    }

    GltfBufferView buffer_view;
    buffer_view.buffer_byte_offset = buffer_start_offset;
    buffer_view.byte_length = buffer_.size() - buffer_start_offset;
    buffer_views_.push_back(buffer_view);
    accessor.buffer_view_index = static_cast<int>(buffer_views_.size() - 1);
  }

  accessor.component_type = UnsignedIntComponentType(max_index);
  accessor.count = num_encoded_faces * 3;
  if (output_type_ == GltfEncoder::VERBOSE) {
    accessor.max.push_back(GltfValue(max_index));
    accessor.min.push_back(GltfValue(min_index));
  }
  accessor.type = "SCALAR";
  accessors_.push_back(accessor);
  return static_cast<int>(accessors_.size() - 1);
}

int GltfAsset::AddDracoPositions(const Mesh &mesh, int num_encoded_points) {
  const PointAttribute *const att =
      mesh.GetNamedAttribute(GeometryAttribute::POSITION);
  if (!CheckDracoAttribute(att, {DT_FLOAT32}, {3})) {
    return -1;
  }
  return AddAttribute<float>(*att, mesh.num_points(), num_encoded_points,
                             mesh.IsCompressionEnabled());
}

int GltfAsset::AddDracoNormals(const Mesh &mesh, int num_encoded_points) {
  const PointAttribute *const att =
      mesh.GetNamedAttribute(GeometryAttribute::NORMAL);
  if (!CheckDracoAttribute(att, {DT_FLOAT32}, {3})) {
    return -1;
  }
  return AddAttribute<float>(*att, mesh.num_points(), num_encoded_points,
                             mesh.IsCompressionEnabled());
}

int GltfAsset::AddDracoColors(const Mesh &mesh, int num_encoded_points) {
  const PointAttribute *const att =
      mesh.GetNamedAttribute(GeometryAttribute::COLOR);
  if (!CheckDracoAttribute(att, {DT_UINT8, DT_UINT16, DT_FLOAT32}, {3, 4})) {
    return -1;
  }
  if (att->data_type() == DT_UINT16) {
    return AddAttribute<uint16_t>(*att, mesh.num_points(), num_encoded_points,
                                  mesh.IsCompressionEnabled());
  }
  if (att->data_type() == DT_FLOAT32) {
    return AddAttribute<float>(*att, mesh.num_points(), num_encoded_points,
                               mesh.IsCompressionEnabled());
  }
  return AddAttribute<uint8_t>(*att, mesh.num_points(), num_encoded_points,
                               mesh.IsCompressionEnabled());
}

int GltfAsset::AddDracoTexture(const Mesh &mesh, int tex_coord_index,
                               int num_encoded_points) {
  const PointAttribute *const att =
      mesh.GetNamedAttribute(GeometryAttribute::TEX_COORD, tex_coord_index);
  // TODO(b/200303080): Add support for DT_UINT8 and DT_UINT16 with TEX_COORD.
  if (!CheckDracoAttribute(att, {DT_FLOAT32}, {2})) {
    return -1;
  }

  // glTF stores texture coordinates flipped on the horizontal axis compared to
  // how Draco stores texture coordinates.
  GeometryAttribute ga;
  ga.Init(GeometryAttribute::TEX_COORD, nullptr, 2, att->data_type(), false,
          DataTypeLength(att->data_type()) * 2, 0);
  PointAttribute ta(ga);
  ta.SetIdentityMapping();
  ta.Reset(mesh.num_points());

  std::array<float, 2> value;
  for (PointIndex v(0); v < mesh.num_points(); ++v) {
    if (!att->GetValue<float, 2>(att->mapped_index(v), &value)) {
      return -1;
    }

    // Draco texture v component needs to be flipped.
    Vector2f texture_coord(value[0], 1.0 - value[1]);
    ta.SetAttributeValue(AttributeValueIndex(v.value()), texture_coord.data());
  }
  return AddAttribute<float>(ta, mesh.num_points(), num_encoded_points,
                             mesh.IsCompressionEnabled());
}

int GltfAsset::AddDracoTangents(const Mesh &mesh, int num_encoded_points) {
  const PointAttribute *const att =
      mesh.GetNamedAttribute(GeometryAttribute::TANGENT);
  if (!CheckDracoAttribute(att, {DT_FLOAT32}, {3, 4})) {
    return -1;
  }
  if (MeshUtils::HasAutoGeneratedTangents(mesh)) {
    // Ignore auto-generated tangents. See go/tangents_and_draco_simplifier.
    return -1;
  }

  if (att->num_components() == 4) {
    return AddAttribute<float>(*att, mesh.num_points(), num_encoded_points,
                               mesh.IsCompressionEnabled());
  }

  // glTF mesh needs the w component.
  GeometryAttribute ga;
  ga.Init(GeometryAttribute::TANGENT, nullptr, 4, DT_FLOAT32, false,
          DataTypeLength(DT_FLOAT32) * 4, 0);
  PointAttribute ta(ga);
  ta.SetIdentityMapping();
  ta.Reset(mesh.num_points());

  std::array<float, 3> value;
  for (PointIndex v(0); v < mesh.num_points(); ++v) {
    if (!att->GetValue<float, 3>(att->mapped_index(v), &value)) {
      return -1;
    }

    // Draco tangent w component is always 1.0.
    Vector4f tangent(value[0], value[1], value[2], 1.0);
    ta.SetAttributeValue(AttributeValueIndex(v.value()), tangent.data());
  }
  return AddAttribute<float>(ta, mesh.num_points(), num_encoded_points,
                             mesh.IsCompressionEnabled());
}

int GltfAsset::AddDracoJoints(const Mesh &mesh, int num_encoded_points) {
  const PointAttribute *const att =
      mesh.GetNamedAttribute(GeometryAttribute::JOINTS);
  if (!CheckDracoAttribute(att, {DT_UINT8, DT_UINT16}, {4})) {
    return -1;
  }
  if (att->data_type() == DT_UINT16) {
    return AddAttribute<uint16_t>(*att, mesh.num_points(), num_encoded_points,
                                  mesh.IsCompressionEnabled());
  }
  return AddAttribute<uint8_t>(*att, mesh.num_points(), num_encoded_points,
                               mesh.IsCompressionEnabled());
}

int GltfAsset::AddDracoWeights(const Mesh &mesh, int num_encoded_points) {
  const PointAttribute *const att =
      mesh.GetNamedAttribute(GeometryAttribute::WEIGHTS);
  // TODO(b/200303026): Add support for DT_UINT8 and DT_UINT16 with WEIGHTS.
  if (!CheckDracoAttribute(att, {DT_FLOAT32}, {4})) {
    return -1;
  }
  return AddAttribute<float>(*att, mesh.num_points(), num_encoded_points,
                             mesh.IsCompressionEnabled());
}

// Adds generic attributes that have metadata describing the attribute name,
// attributes referred to by one of the mesh feature ID sets or in the |mesh|,
// and attributes referred to by one of the property attributes in the |mesh|.
// This allows for export of application-specific attributes, feature ID
// attributes defined in glTF extension EXT_mesh_features, and property
// attributes defined in glTF extension EXT_structural_metadata. Returns a
// vector of attribute-name, accessor pairs for each valid attribute. Populates
// map from |mesh| attribute index in the feature ID attribute name like
// _FEATURE_ID_5 or _DIRECTION for each feature ID and property attribute in the
// |mesh|.
std::vector<std::pair<std::string, int>> GltfAsset::AddDracoGenerics(
    const Mesh &mesh, int num_encoded_points,
    std::unordered_map<int, int> *feature_id_name_indices) {
  const int num_generic_attributes =
      mesh.NumNamedAttributes(GeometryAttribute::GENERIC);
  std::vector<std::pair<std::string, int>> attrs;
  int feature_id_count = 0;
  for (int i = 0; i < num_generic_attributes; ++i) {
    const int att_index =
        mesh.GetNamedAttributeId(GeometryAttribute::GENERIC, i);
    const PointAttribute *const att = mesh.attribute(att_index);
    std::string attr_name;
    int accessor = -1;

    auto const *metadata = mesh.GetAttributeMetadataByAttributeId(att_index);
    if (metadata) {
      if (metadata->GetEntryString(GltfEncoder::kDracoMetadataGltfAttributeName,
                                   &attr_name)) {
        if (att->data_type() == DT_FLOAT32) {
          accessor =
              AddAttribute<float>(*att, mesh.num_points(), num_encoded_points,
                                  mesh.IsCompressionEnabled());
        }
      }
    } else {
      if (IsFeatureIdAttribute(att_index, mesh) && att->num_components() == 1) {
        // This is an attribute referred to by one of the mesh feature ID sets
        // as defined by the EXT_mesh_features glTF extension.
        // TODO(vytyaz): Report an error if the number of components is not one.
        accessor = AddAttribute(*att, mesh.num_points(), num_encoded_points,
                                mesh.IsCompressionEnabled());

        // Generate attribute name like _FEATURE_ID_N where N starts at 0 for
        // the first feature ID vertex attribute and continues with consecutive
        // positive integers as dictated by the EXT_mesh_features extension.
        attr_name =
            std::string("_FEATURE_ID_") + std::to_string(feature_id_count);

        // Populate map from attribute index in the |mesh| to the index in a
        // feature ID vertex attribute name like _FEATURE_ID_5.
        (*feature_id_name_indices)[att_index] = feature_id_count;
        feature_id_count++;
      } else if (IsPropertyAttribute(att_index, mesh, *structural_metadata_)) {
        // This is a property attribute as defined by the
        // EXT_structural_metadata glTF extension.
        accessor = AddAttribute(*att, mesh.num_points(), num_encoded_points,
                                mesh.IsCompressionEnabled());
        attr_name = att->name();
      }
    }
    if (accessor != -1 && !attr_name.empty()) {
      attrs.emplace_back(attr_name, accessor);
    }
  }
  return attrs;
}

void GltfAsset::AddMaterials(const Mesh &mesh) {
  if (mesh.GetMaterialLibrary().NumMaterials()) {
    material_library_.Copy(mesh.GetMaterialLibrary());
  }
}

bool GltfAsset::CheckDracoAttribute(const PointAttribute *attribute,
                                    const std::set<DataType> &data_types,
                                    const std::set<int> &num_components) {
  // Attribute must be valid.
  if (attribute == nullptr || attribute->size() == 0) {
    return false;
  }

  // Attribute must have an expected data type.
  if (data_types.find(attribute->data_type()) == data_types.end()) {
    return false;
  }

  // Attribute must have an expected number of components.
  if (num_components.find(attribute->num_components()) ==
      num_components.end()) {
    return false;
  }

  return true;
}

StatusOr<int> GltfAsset::AddImage(const std::string &image_stem,
                                  const Texture *texture, int num_components) {
  return AddImage(image_stem, texture, nullptr, num_components);
}

StatusOr<int> GltfAsset::AddImage(const std::string &image_stem,
                                  const Texture *texture,
                                  std::unique_ptr<Texture> owned_texture,
                                  int num_components) {
  const auto it = texture_to_image_index_map_.find(texture);
  if (it != texture_to_image_index_map_.end()) {
    // We already have an image for the given |texture|. Update its number of
    // components if needed.
    GltfImage &image = images_[it->second];
    if (image.num_components < num_components) {
      image.num_components = num_components;
    }
    return it->second;
  }
  std::string extension = TextureUtils::GetTargetExtension(*texture);
  if (extension.empty()) {
    // Try to get extension from the source file name.
    extension = LowercaseFileExtension(texture->source_image().filename());
  }
  GltfImage image;
  image.image_name = image_stem + "." + extension;
  image.texture = texture;
  image.owned_texture = std::move(owned_texture);
  image.num_components = num_components;
  image.mime_type = TextureUtils::GetTargetMimeType(*texture);

  // For KTX2 with Basis compression, state that its extension is required.
  if (extension == "ktx2") {
    extensions_used_.insert("KHR_texture_basisu");
    extensions_required_.insert("KHR_texture_basisu");
  }

  // If this is webp, state that its extension is required.
  if (extension == "webp") {
    extensions_used_.insert("EXT_texture_webp");
    extensions_required_.insert("EXT_texture_webp");
  }

  images_.push_back(std::move(image));
  texture_to_image_index_map_[texture] = images_.size() - 1;
  return images_.size() - 1;
}

Status GltfAsset::SaveImageToBuffer(int image_index) {
  GltfImage &image = images_[image_index];
  const Texture *const texture = image.texture;
  const int num_components = image.num_components;
  std::vector<uint8_t> buffer;
  DRACO_RETURN_IF_ERROR(WriteTextureToBuffer(*texture, &buffer));

  // Add the image data to the buffer.
  const size_t buffer_start_offset = buffer_.size();
  buffer_.Encode(buffer.data(), buffer.size());
  if (!PadBuffer()) {
    return Status(Status::DRACO_ERROR,
                  "Could not pad buffer in SaveImageToBuffer.");
  }

  // Add a buffer view pointing to the image data in the buffer.
  GltfBufferView buffer_view;
  buffer_view.buffer_byte_offset = buffer_start_offset;
  buffer_view.byte_length = buffer_.size() - buffer_start_offset;
  buffer_views_.push_back(buffer_view);

  image.buffer_view = buffer_views_.size() - 1;
  return OkStatus();
}

// TODO(vytyaz): The return type could be int.
StatusOr<int> GltfAsset::AddTextureSampler(const TextureSampler &sampler) {
  // If sampler is equal to defaults do not add to vector and return -1.
  if (sampler.min_filter == TextureMap::UNSPECIFIED &&
      sampler.mag_filter == TextureMap::UNSPECIFIED &&
      sampler.wrapping_mode.s == TextureMap::REPEAT &&
      sampler.wrapping_mode.t == TextureMap::REPEAT) {
    return -1;
  }

  const auto &it =
      std::find(texture_samplers_.begin(), texture_samplers_.end(), sampler);
  if (it != texture_samplers_.end()) {
    const int index = std::distance(texture_samplers_.begin(), it);
    return index;
  }

  texture_samplers_.push_back(sampler);
  return texture_samplers_.size() - 1;
}

Status GltfAsset::AddScene(const Scene &scene) {
  const int scene_index = AddScene();
  if (scene_index < 0) {
    return Status(Status::DRACO_ERROR, "Error creating a new scene.");
  }
  AddMaterials(scene);
  AddStructuralMetadata(scene);

  // Initialize base mesh transforms that may be needed when the base meshes are
  // compressed with Draco.
  base_mesh_transforms_ = SceneUtils::FindLargestBaseMeshTransforms(scene);
  for (SceneNodeIndex i(0); i < scene.NumNodes(); ++i) {
    DRACO_RETURN_IF_ERROR(AddSceneNode(scene, i));
  }
  // There is 1:1 mapping between draco::Scene node indices and |nodes_|.
  for (int i = 0; i < scene.NumRootNodes(); ++i) {
    nodes_[scene.GetRootNodeIndex(i).value()].root_node = true;
  }
  DRACO_RETURN_IF_ERROR(AddAnimations(scene));
  DRACO_RETURN_IF_ERROR(AddSkins(scene));
  DRACO_RETURN_IF_ERROR(AddLights(scene));
  DRACO_RETURN_IF_ERROR(AddMaterialsVariantsNames(scene));
  DRACO_RETURN_IF_ERROR(AddInstanceArrays(scene));
  if (copyright_.empty()) {
    SetCopyrightFromScene(scene);
  }
  return OkStatus();
}

Status GltfAsset::AddSceneNode(const Scene &scene,
                               SceneNodeIndex scene_node_index) {
  const SceneNode *const scene_node = scene.GetNode(scene_node_index);
  if (scene_node == nullptr) {
    return Status(Status::DRACO_ERROR, "Could not find node in scene.");
  }

  GltfNode node;
  node.name = scene_node->GetName();
  node.trs_matrix.Copy(scene_node->GetTrsMatrix());

  for (int i = 0; i < scene_node->NumChildren(); ++i) {
    node.children_indices.push_back(scene_node->Child(i).value());
  }

  const MeshGroupIndex mesh_group_index = scene_node->GetMeshGroupIndex();
  if (mesh_group_index != kInvalidMeshGroupIndex) {
    const auto it = mesh_group_index_to_gltf_mesh_.find(mesh_group_index);
    if (it == mesh_group_index_to_gltf_mesh_.end()) {
      GltfMesh gltf_mesh;
      const MeshGroup *const mesh_group = scene.GetMeshGroup(mesh_group_index);
      if (!mesh_group->GetName().empty()) {
        gltf_mesh.name = mesh_group->GetName();
      }
      meshes_.push_back(gltf_mesh);

      for (int i = 0; i < mesh_group->NumMeshInstances(); ++i) {
        const MeshGroup::MeshInstance &instance =
            mesh_group->GetMeshInstance(i);
        const auto mi_it =
            mesh_index_to_gltf_mesh_primitive_.find(instance.mesh_index);
        if (mi_it == mesh_index_to_gltf_mesh_primitive_.end()) {
          // We have not added the mesh to the scene yet.
          const Mesh &mesh = scene.GetMesh(instance.mesh_index);
          if (!AddDracoMesh(mesh, instance.material_index,
                            instance.materials_variants_mappings,
                            base_mesh_transforms_[instance.mesh_index])) {
            return Status(Status::DRACO_ERROR, "Adding a Draco mesh failed.");
          }
          const int gltf_mesh_index = meshes_.size() - 1;
          const int gltf_primitive_index = meshes_.back().primitives.size() - 1;
          mesh_index_to_gltf_mesh_primitive_[instance.mesh_index] =
              std::make_pair(gltf_mesh_index, gltf_primitive_index);
        } else {
          // The mesh was already added to the scene. This is a copy instance
          // that may have a different material.
          const int gltf_mesh_index = mi_it->second.first;
          const int gltf_primitive_index = mi_it->second.second;
          GltfPrimitive primitive =
              meshes_[gltf_mesh_index].primitives[gltf_primitive_index];
          primitive.material = instance.material_index;
          primitive.material_variants_mappings =
              instance.materials_variants_mappings;
          const Mesh &mesh = scene.GetMesh(instance.mesh_index);
          primitive.mesh_features.clear();
          primitive.mesh_features.reserve(mesh.NumMeshFeatures());
          for (MeshFeaturesIndex j(0); j < mesh.NumMeshFeatures(); ++j) {
            primitive.mesh_features.push_back(&mesh.GetMeshFeatures(j));
          }
          primitive.property_attributes.reserve(
              mesh.NumPropertyAttributesIndices());
          for (int i = 0; i < mesh.NumPropertyAttributesIndices(); ++i) {
            primitive.property_attributes.push_back(
                mesh.GetPropertyAttributesIndex(i));
          }
          meshes_.back().primitives.push_back(primitive);
        }
      }
      mesh_group_index_to_gltf_mesh_[mesh_group_index] = meshes_.size() - 1;
    }
    node.mesh_index = mesh_group_index_to_gltf_mesh_[mesh_group_index];
  }
  node.skin_index = scene_node->GetSkinIndex().value();
  node.light_index = scene_node->GetLightIndex().value();
  node.instance_array_index = scene_node->GetInstanceArrayIndex().value();

  nodes_.push_back(node);
  return OkStatus();
}

void GltfAsset::AddMaterials(const Scene &scene) {
  if (scene.GetMaterialLibrary().NumMaterials()) {
    material_library_.Copy(scene.GetMaterialLibrary());
  }
}

Status GltfAsset::AddAnimations(const Scene &scene) {
  if (scene.NumAnimations() == 0) {
    return OkStatus();
  }
  // Mapping of the node animation data to the output accessors. The first part
  // of the key is the animation index and the second part of the key is the
  // node animation data index.
  std::map<std::pair<int, int>, int> node_animation_data_to_accessor;

  // Mapping of the node animation data to the output accessors.
  std::unordered_map<NodeAnimationDataHash, int, NodeAnimationDataHash::Hash>
      data_to_index_map;

  // First add all the accessors and create a mapping from animation accessors
  // to accessors owned by the encoder.
  for (AnimationIndex i(0); i < scene.NumAnimations(); ++i) {
    const Animation *const animation = scene.GetAnimation(i);

    for (int j = 0; j < animation->NumNodeAnimationData(); ++j) {
      const NodeAnimationData *const node_animation_data =
          animation->GetNodeAnimationData(j);

      int index = -1;

      NodeAnimationDataHash nadh(node_animation_data);
      if (data_to_index_map.find(nadh) == data_to_index_map.end()) {
        // The current data is new, add it to the encoder.
        DRACO_ASSIGN_OR_RETURN(
            index, AddNodeAnimationData(*nadh.GetNodeAnimationData()));
        data_to_index_map[nadh] = index;
      } else {
        index = data_to_index_map[nadh];
      }

      const auto key = std::make_pair(i.value(), j);
      node_animation_data_to_accessor[key] = index;
    }
  }

  // Add all the samplers and channels.
  for (AnimationIndex i(0); i < scene.NumAnimations(); ++i) {
    const Animation *const animation = scene.GetAnimation(i);
    std::unique_ptr<EncoderAnimation> new_animation(new EncoderAnimation);
    new_animation->name = animation->GetName();

    for (int j = 0; j < animation->NumSamplers(); ++j) {
      const AnimationSampler *const sampler = animation->GetSampler(j);
      const auto input_key = std::make_pair(i.value(), sampler->input_index);
      const auto input_it = node_animation_data_to_accessor.find(input_key);
      if (input_it == node_animation_data_to_accessor.end()) {
        return Status(Status::DRACO_ERROR,
                      "Could not find animation accessor input index.");
      }
      const auto output_key = std::make_pair(i.value(), sampler->output_index);
      const auto output_it = node_animation_data_to_accessor.find(output_key);
      if (output_it == node_animation_data_to_accessor.end()) {
        return Status(Status::DRACO_ERROR,
                      "Could not find animation accessor output index.");
      }

      std::unique_ptr<AnimationSampler> new_sampler(new AnimationSampler());
      new_sampler->input_index = input_it->second;
      new_sampler->output_index = output_it->second;

      if (output_type_ == GltfEncoder::COMPACT) {
        // Remove min/max from output accessor.
        accessors_[new_sampler->output_index].min.clear();
        accessors_[new_sampler->output_index].max.clear();
      }

      new_sampler->interpolation_type = sampler->interpolation_type;

      new_animation->samplers.push_back(std::move(new_sampler));
    }

    for (int j = 0; j < animation->NumChannels(); ++j) {
      const AnimationChannel *const channel = animation->GetChannel(j);
      std::unique_ptr<AnimationChannel> new_channel(new AnimationChannel());
      new_channel->Copy(*channel);
      new_animation->channels.push_back(std::move(new_channel));
    }

    animations_.push_back(std::move(new_animation));
  }
  return OkStatus();
}

StatusOr<int> GltfAsset::AddNodeAnimationData(
    const NodeAnimationData &node_animation_data) {
  const size_t buffer_start_offset = buffer_.size();

  const int component_size = node_animation_data.ComponentSize();
  const int num_components = node_animation_data.NumComponents();
  const std::vector<float> *data = node_animation_data.GetData();

  std::vector<float> min_values;
  min_values.resize(num_components);
  for (int j = 0; j < num_components; ++j) {
    min_values[j] = (*data)[j];
  }
  std::vector<float> max_values = min_values;

  for (int i = 0; i < node_animation_data.count(); ++i) {
    for (int j = 0; j < num_components; ++j) {
      const float value = (*data)[(i * num_components) + j];
      if (value < min_values[j]) {
        min_values[j] = value;
      }
      if (value > max_values[j]) {
        max_values[j] = value;
      }

      buffer_.Encode(&value, component_size);
    }
  }

  if (!PadBuffer()) {
    return Status(Status::DRACO_ERROR,
                  "AddNodeAnimationData: PadBuffer returned DRACO_ERROR.");
  }

  GltfBufferView buffer_view;
  buffer_view.buffer_byte_offset = buffer_start_offset;
  buffer_view.byte_length = buffer_.size() - buffer_start_offset;
  buffer_views_.push_back(buffer_view);

  GltfAccessor accessor;
  accessor.buffer_view_index = static_cast<int>(buffer_views_.size() - 1);
  accessor.component_type = ComponentType::FLOAT;
  accessor.count = node_animation_data.count();
  for (int j = 0; j < num_components; ++j) {
    accessor.max.push_back(GltfValue(max_values[j]));
    accessor.min.push_back(GltfValue(min_values[j]));
  }
  accessor.type = node_animation_data.TypeAsString();
  accessor.normalized = node_animation_data.normalized();
  accessors_.push_back(accessor);
  return static_cast<int>(accessors_.size() - 1);
}

Status GltfAsset::AddSkins(const Scene &scene) {
  if (scene.NumSkins() == 0) {
    return OkStatus();
  }

  for (SkinIndex i(0); i < scene.NumSkins(); ++i) {
    const Skin *const skin = scene.GetSkin(i);
    DRACO_ASSIGN_OR_RETURN(
        const int output_accessor_index,
        AddNodeAnimationData(skin->GetInverseBindMatrices()));

    std::unique_ptr<EncoderSkin> encoder_skin(new EncoderSkin);
    encoder_skin->inverse_bind_matrices_index = output_accessor_index;
    encoder_skin->joints.reserve(skin->NumJoints());
    for (int j = 0; j < skin->NumJoints(); j++) {
      encoder_skin->joints.push_back(skin->GetJoint(j).value());
    }
    encoder_skin->skeleton_index = skin->GetJointRoot().value();
    skins_.push_back(std::move(encoder_skin));
  }
  return OkStatus();
}

Status GltfAsset::AddLights(const Scene &scene) {
  if (scene.NumLights() == 0) {
    return OkStatus();
  }

  for (LightIndex i(0); i < scene.NumLights(); ++i) {
    std::unique_ptr<Light> light = std::unique_ptr<Light>(new Light());
    light->Copy(*scene.GetLight(i));
    lights_.push_back(std::move(light));
  }
  return OkStatus();
}

Status GltfAsset::AddMaterialsVariantsNames(const Scene &scene) {
  const MaterialLibrary &library = scene.GetMaterialLibrary();
  for (int i = 0; i < library.NumMaterialsVariants(); ++i) {
    materials_variants_names_.push_back(library.GetMaterialsVariantName(i));
  }
  return OkStatus();
}

Status GltfAsset::AddInstanceArrays(const Scene &scene) {
  if (scene.NumInstanceArrays() == 0) {
    return OkStatus();
  }

  // Add each of the instance arrays.
  std::vector<float> t_data;
  std::vector<float> r_data;
  std::vector<float> s_data;
  for (InstanceArrayIndex i(0); i < scene.NumInstanceArrays(); ++i) {
    // Find which of the optional TRS components are set.
    // TODO(vytyaz): Treat default TRS component vectors as absent.
    const InstanceArray &array = *scene.GetInstanceArray(i);
    bool is_t_set = false;
    bool is_r_set = false;
    bool is_s_set = false;
    for (int i = 0; i < array.NumInstances(); i++) {
      const InstanceArray::Instance &instance = array.GetInstance(i);
      if (instance.trs.TranslationSet()) {
        is_t_set = true;
      }
      if (instance.trs.RotationSet()) {
        is_r_set = true;
      }
      if (instance.trs.ScaleSet()) {
        is_s_set = true;
      }
    }

    // Create contiguous data vectors for individual TRS components.
    t_data.clear();
    r_data.clear();
    s_data.clear();
    if (is_t_set) {
      t_data.reserve(array.NumInstances() * 3);
    }
    if (is_r_set) {
      r_data.reserve(array.NumInstances() * 4);
    }
    if (is_s_set) {
      s_data.reserve(array.NumInstances() * 3);
    }

    // Add TRS vectors of each instance to corresponding data vectors.
    for (int i = 0; i < array.NumInstances(); i++) {
      const InstanceArray::Instance &instance = array.GetInstance(i);
      if (is_t_set) {
        DRACO_ASSIGN_OR_RETURN(const auto &t_vector,
                               instance.trs.Translation());
        t_data.push_back(t_vector.x());
        t_data.push_back(t_vector.y());
        t_data.push_back(t_vector.z());
      }
      if (is_r_set) {
        DRACO_ASSIGN_OR_RETURN(const auto &r_vector, instance.trs.Rotation());
        r_data.push_back(r_vector.x());
        r_data.push_back(r_vector.y());
        r_data.push_back(r_vector.z());
        r_data.push_back(r_vector.w());
      }
      if (is_s_set) {
        DRACO_ASSIGN_OR_RETURN(const auto &s_vector, instance.trs.Scale());
        s_data.push_back(s_vector.x());
        s_data.push_back(s_vector.y());
        s_data.push_back(s_vector.z());
      }
    }

    // Add TRS vectors to attribute buffers and collect their accessor indices.
    EncoderInstanceArray accessors;
    if (is_t_set) {
      DRACO_ASSIGN_OR_RETURN(accessors.translation, AddData(t_data, 3));
    }
    if (is_r_set) {
      DRACO_ASSIGN_OR_RETURN(accessors.rotation, AddData(r_data, 4));
    }
    if (is_s_set) {
      DRACO_ASSIGN_OR_RETURN(accessors.scale, AddData(s_data, 3));
    }

    // Store accessors for later to encode as EXT_mesh_gpu_instancing extension.
    instance_arrays_.push_back(accessors);
  }
  return OkStatus();
}

template <typename GeometryT>
void GltfAsset::AddStructuralMetadata(const GeometryT &geometry) {
  structural_metadata_ = &geometry.GetStructuralMetadata();
}

StatusOr<int> GltfAsset::AddData(const std::vector<float> &data,
                                 int num_components) {
  std::string type;
  switch (num_components) {
    case 3:
      type = "VEC3";
      break;
    case 4:
      type = "VEC4";
      break;
    default:
      return ErrorStatus("Unsupported number of components.");
  }

  const size_t buffer_start_offset = buffer_.size();

  std::vector<float> min_values(num_components);
  for (int j = 0; j < num_components; ++j) {
    min_values[j] = data[j];
  }
  std::vector<float> max_values = min_values;

  const int count = data.size() / num_components;
  for (int i = 0; i < count; ++i) {
    for (int j = 0; j < num_components; ++j) {
      const float value = data[(i * num_components) + j];
      if (value < min_values[j]) {
        min_values[j] = value;
      }
      if (value > max_values[j]) {
        max_values[j] = value;
      }
      buffer_.Encode(&value, sizeof(float));
    }
  }

  if (!PadBuffer()) {
    return ErrorStatus("AddArray: PadBuffer returned DRACO_ERROR.");
  }

  GltfBufferView buffer_view;
  buffer_view.buffer_byte_offset = buffer_start_offset;
  buffer_view.byte_length = buffer_.size() - buffer_start_offset;
  buffer_views_.push_back(buffer_view);

  GltfAccessor accessor;
  accessor.buffer_view_index = static_cast<int>(buffer_views_.size() - 1);
  accessor.component_type = ComponentType::FLOAT;
  accessor.count = count;
  for (int j = 0; j < num_components; ++j) {
    accessor.max.push_back(GltfValue(max_values[j]));
    accessor.min.push_back(GltfValue(min_values[j]));
  }
  accessor.type = type;
  accessor.normalized = false;
  accessors_.push_back(accessor);
  return static_cast<int>(accessors_.size() - 1);
}

StatusOr<int> GltfAsset::AddBufferView(
    const PropertyTable::Property::Data &data) {
  const size_t buffer_start_offset = buffer_.size();
  buffer_.Encode(data.data.data(), data.data.size());
  if (!PadBuffer()) {
    return ErrorStatus("AddBufferView: PadBuffer returned DRACO_ERROR.");
  }
  GltfBufferView buffer_view;
  buffer_view.buffer_byte_offset = buffer_start_offset;
  buffer_view.byte_length = buffer_.size() - buffer_start_offset;
  buffer_view.target = data.target;
  buffer_views_.push_back(buffer_view);
  return static_cast<int>(buffer_views_.size() - 1);
}

bool GltfAsset::EncodeAssetProperty(EncoderBuffer *buf_out) {
  gltf_json_.BeginObject("asset");
  gltf_json_.OutputValue("version", version_);
  gltf_json_.OutputValue("generator", generator_);
  if (!copyright_.empty()) {
    gltf_json_.OutputValue("copyright", copyright_);
  }
  gltf_json_.EndObject();

  const std::string asset_str = gltf_json_.MoveData();
  return buf_out->Encode(asset_str.data(), asset_str.length());
}

bool GltfAsset::EncodeScenesProperty(EncoderBuffer *buf_out) {
  // We currently only support one scene.
  gltf_json_.BeginArray("scenes");
  gltf_json_.BeginObject();
  gltf_json_.BeginArray("nodes");

  for (int i = 0; i < nodes_.size(); ++i) {
    if (nodes_[i].root_node) {
      gltf_json_.OutputValue(i);
    }
  }
  gltf_json_.EndArray();
  gltf_json_.EndObject();
  gltf_json_.EndArray();

  const std::string asset_str = gltf_json_.MoveData();
  return buf_out->Encode(asset_str.data(), asset_str.length());
}

bool GltfAsset::EncodeInitialSceneProperty(EncoderBuffer *buf_out) {
  gltf_json_.OutputValue("scene", scene_index_);
  const std::string asset_str = gltf_json_.MoveData();
  return buf_out->Encode(asset_str.data(), asset_str.length());
}

bool GltfAsset::EncodeNodesProperty(EncoderBuffer *buf_out) {
  gltf_json_.BeginArray("nodes");

  for (int i = 0; i < nodes_.size(); ++i) {
    gltf_json_.BeginObject();
    if (!nodes_[i].name.empty()) {
      gltf_json_.OutputValue("name", nodes_[i].name);
    }
    if (nodes_[i].mesh_index >= 0) {
      gltf_json_.OutputValue("mesh", nodes_[i].mesh_index);
    }
    if (nodes_[i].skin_index >= 0) {
      gltf_json_.OutputValue("skin", nodes_[i].skin_index);
    }
    if (nodes_[i].instance_array_index >= 0 || nodes_[i].light_index >= 0) {
      gltf_json_.BeginObject("extensions");
      if (nodes_[i].instance_array_index >= 0) {
        gltf_json_.BeginObject("EXT_mesh_gpu_instancing");
        gltf_json_.BeginObject("attributes");
        const int index = nodes_[i].instance_array_index;
        const EncoderInstanceArray &accessors = instance_arrays_[index];
        if (accessors.translation != -1) {
          gltf_json_.OutputValue("TRANSLATION", accessors.translation);
        }
        if (accessors.rotation != -1) {
          gltf_json_.OutputValue("ROTATION", accessors.rotation);
        }
        if (accessors.scale != -1) {
          gltf_json_.OutputValue("SCALE", accessors.scale);
        }
        gltf_json_.EndObject();
        gltf_json_.EndObject();
      }
      if (nodes_[i].light_index >= 0) {
        gltf_json_.BeginObject("KHR_lights_punctual");
        gltf_json_.OutputValue("light", nodes_[i].light_index);
        gltf_json_.EndObject();
      }
      gltf_json_.EndObject();
    }

    if (!nodes_[i].children_indices.empty()) {
      gltf_json_.BeginArray("children");
      for (int j = 0; j < nodes_[i].children_indices.size(); ++j) {
        gltf_json_.OutputValue(nodes_[i].children_indices[j]);
      }
      gltf_json_.EndArray();
    }

    if (!nodes_[i].trs_matrix.IsMatrixIdentity()) {
      const auto maybe_transformation = nodes_[i].trs_matrix.Matrix();
      const auto transformation = maybe_transformation.ValueOrDie();

      if (nodes_[i].trs_matrix.IsMatrixTranslationOnly()) {
        gltf_json_.BeginArray("translation");
        for (int j = 0; j < 3; ++j) {
          gltf_json_.OutputValue(transformation(j, 3));
        }
        gltf_json_.EndArray();
      } else {
        gltf_json_.BeginArray("matrix");
        for (int j = 0; j < 4; ++j) {
          for (int k = 0; k < 4; ++k) {
            gltf_json_.OutputValue(transformation(k, j));
          }
        }
        gltf_json_.EndArray();
      }
    } else {
      if (nodes_[i].trs_matrix.TranslationSet()) {
        const auto maybe_translation = nodes_[i].trs_matrix.Translation();
        const auto translation = maybe_translation.ValueOrDie();
        gltf_json_.BeginArray("translation");
        for (int j = 0; j < 3; ++j) {
          gltf_json_.OutputValue(translation[j]);
        }
        gltf_json_.EndArray();
      }
      if (nodes_[i].trs_matrix.RotationSet()) {
        const auto maybe_rotation = nodes_[i].trs_matrix.Rotation();
        const auto rotation = maybe_rotation.ValueOrDie();
        gltf_json_.BeginArray("rotation");
        for (int j = 0; j < 4; ++j) {
          // Note: coeffs() returns quaternion values as (x, y, z, w) which is
          // the expected format of glTF.
          gltf_json_.OutputValue(rotation.coeffs()[j]);
        }
        gltf_json_.EndArray();
      }
      if (nodes_[i].trs_matrix.ScaleSet()) {
        const auto maybe_scale = nodes_[i].trs_matrix.Scale();
        const auto scale = maybe_scale.ValueOrDie();
        gltf_json_.BeginArray("scale");
        for (int j = 0; j < 3; ++j) {
          gltf_json_.OutputValue(scale[j]);
        }
        gltf_json_.EndArray();
      }
    }

    gltf_json_.EndObject();
  }

  gltf_json_.EndArray();

  const std::string asset_str = gltf_json_.MoveData();
  return buf_out->Encode(asset_str.data(), asset_str.length());
}

Status GltfAsset::EncodeMeshesProperty(EncoderBuffer *buf_out) {
  mesh_features_texture_index_ = 0;
  gltf_json_.BeginArray("meshes");

  for (int i = 0; i < meshes_.size(); ++i) {
    gltf_json_.BeginObject();

    if (!meshes_[i].name.empty()) {
      gltf_json_.OutputValue("name", meshes_[i].name);
    }

    if (!meshes_[i].primitives.empty()) {
      gltf_json_.BeginArray("primitives");

      for (int j = 0; j < meshes_[i].primitives.size(); ++j) {
        const GltfPrimitive &primitive = meshes_[i].primitives[j];
        gltf_json_.BeginObject();

        gltf_json_.BeginObject("attributes");
        for (auto const &it : primitive.attributes) {
          gltf_json_.OutputValue(it.first, it.second);
        }
        gltf_json_.EndObject();

        if (primitive.indices >= 0) {
          gltf_json_.OutputValue("indices", primitive.indices);
        }
        gltf_json_.OutputValue("mode", primitive.mode);
        if (primitive.material >= 0) {
          gltf_json_.OutputValue("material", primitive.material);
        }
        DRACO_RETURN_IF_ERROR(
            EncodePrimitiveExtensionsProperty(primitive, buf_out));
        gltf_json_.EndObject();
      }

      gltf_json_.EndArray();
    }

    gltf_json_.EndObject();
  }

  gltf_json_.EndArray();

  const std::string asset_str = gltf_json_.MoveData();
  if (!buf_out->Encode(asset_str.data(), asset_str.length())) {
    return ErrorStatus("Failed encoding meshes.");
  }
  return OkStatus();
}

Status GltfAsset::EncodePrimitiveExtensionsProperty(
    const GltfPrimitive &primitive, EncoderBuffer *buf_out) {
  // Return if the primitive has no extensions to encode.
  const bool has_draco_mesh_compression =
      primitive.compressed_mesh_info.buffer_view_index >= 0;
  const bool has_materials_variants =
      !primitive.material_variants_mappings.empty();
  const bool has_structural_metadata = !primitive.property_attributes.empty();
  const bool has_mesh_features = !primitive.mesh_features.empty();
  if (!has_draco_mesh_compression && !has_materials_variants &&
      !has_mesh_features && !has_structural_metadata) {
    return OkStatus();
  }

  // Encode primitive extensions.
  gltf_json_.BeginObject("extensions");
  if (has_draco_mesh_compression) {
    gltf_json_.BeginObject("KHR_draco_mesh_compression");
    gltf_json_.OutputValue("bufferView",
                           primitive.compressed_mesh_info.buffer_view_index);
    gltf_json_.BeginObject("attributes");
    for (auto const &it : primitive.compressed_mesh_info.attributes) {
      gltf_json_.OutputValue(it.first, it.second);
    }
    gltf_json_.EndObject();  // attributes entry.
    gltf_json_.EndObject();  // KHR_draco_mesh_compression entry.
  }
  if (has_materials_variants) {
    gltf_json_.BeginObject("KHR_materials_variants");
    gltf_json_.BeginArray("mappings");
    for (const auto &mapping : primitive.material_variants_mappings) {
      gltf_json_.BeginObject();
      gltf_json_.OutputValue("material", mapping.material);
      gltf_json_.BeginArray("variants");
      for (const int variant : mapping.variants) {
        gltf_json_.OutputValue(variant);
      }
      gltf_json_.EndArray();  // variants array.
      gltf_json_.EndObject();
    }
    gltf_json_.EndArray();   // mappings array.
    gltf_json_.EndObject();  // KHR_materials_variants entry.
  }
  if (has_mesh_features) {
    gltf_json_.BeginObject("EXT_mesh_features");
    gltf_json_.BeginArray("featureIds");
    for (int i = 0; i < primitive.mesh_features.size(); i++) {
      const auto &features = primitive.mesh_features[i];
      gltf_json_.BeginObject();
      if (!features->GetLabel().empty()) {
        gltf_json_.OutputValue("label", features->GetLabel());
      }
      gltf_json_.OutputValue("featureCount", features->GetFeatureCount());
      if (features->GetAttributeIndex() != -1) {
        // Index referring to mesh feature ID attribute name like _FEATURE_ID_5.
        const int index =
            primitive.feature_id_name_indices.at(features->GetAttributeIndex());
        gltf_json_.OutputValue("attribute", index);
      }
      if (features->GetPropertyTableIndex() != -1) {
        gltf_json_.OutputValue("propertyTable",
                               features->GetPropertyTableIndex());
      }
      if (features->GetTextureMap().tex_coord_index() != -1) {
        const TextureMap &texture_map = features->GetTextureMap();
        const std::string texture_stem = TextureUtils::GetOrGenerateTargetStem(
            *texture_map.texture(), mesh_features_texture_index_++,
            "_MeshFeatures");

        // Save image as RGBA if the A channel is used to store feature ID.
        const auto &channels = features->GetTextureChannels();
        const int num_channels =
            std::count(channels.begin(), channels.end(), 3) == 1 ? 4 : 3;
        DRACO_ASSIGN_OR_RETURN(
            const int image_index,
            AddImage(texture_stem, texture_map.texture(), num_channels));
        const int tex_coord_index = texture_map.tex_coord_index();
        Material dummy_material;
        DRACO_RETURN_IF_ERROR(EncodeTextureMap("texture", image_index,
                                               tex_coord_index, dummy_material,
                                               texture_map, channels));
      }
      if (features->GetNullFeatureId() != -1) {
        gltf_json_.OutputValue("nullFeatureId", features->GetNullFeatureId());
      }
      gltf_json_.EndObject();
      mesh_features_used_ = true;
    }
    gltf_json_.EndArray();   // featureIds array.
    gltf_json_.EndObject();  // EXT_mesh_features entry.
  }
  if (has_structural_metadata) {
    structural_metadata_used_ = true;
    gltf_json_.BeginObject("EXT_structural_metadata");
    gltf_json_.BeginArray("propertyAttributes");
    for (const int property_attribute_index : primitive.property_attributes) {
      gltf_json_.OutputValue(property_attribute_index);
    }
    gltf_json_.EndArray();   // propertyAttributes array.
    gltf_json_.EndObject();  // EXT_structural_metadata entry.
  }
  gltf_json_.EndObject();  // extensions entry.
  return OkStatus();
}

Status GltfAsset::EncodeMaterials(EncoderBuffer *buf_out) {
  // Check if we have textures to write.
  if (material_library_.NumMaterials() == 0) {
    return EncodeDefaultMaterial(buf_out);
  }
  return EncodeMaterialsProperty(buf_out);
}

void GltfAsset::EncodeColorMaterial(float red, float green, float blue,
                                    float alpha, float metallic_factor) {
  gltf_json_.BeginObject("pbrMetallicRoughness");

  gltf_json_.BeginArray("baseColorFactor");
  gltf_json_.OutputValue(red);
  gltf_json_.OutputValue(green);
  gltf_json_.OutputValue(blue);
  gltf_json_.OutputValue(alpha);
  gltf_json_.EndArray();
  gltf_json_.OutputValue("metallicFactor", metallic_factor);

  gltf_json_.EndObject();  // pbrMetallicRoughness
}

Status GltfAsset::EncodeDefaultMaterial(EncoderBuffer *buf_out) {
  gltf_json_.BeginArray("materials");
  gltf_json_.BeginObject();
  EncodeColorMaterial(0.75, 0.75, 0.75, 1.0, 0.0);
  gltf_json_.EndObject();
  gltf_json_.EndArray();  // materials

  const std::string asset_str = gltf_json_.MoveData();
  if (!buf_out->Encode(asset_str.data(), asset_str.length())) {
    return Status(Status::DRACO_ERROR, "Error encoding default material.");
  }
  return OkStatus();
}

Status GltfAsset::EncodeTextureMap(const std::string &object_name,
                                   int image_index, int tex_coord_index,
                                   const Material &material,
                                   const TextureMap &texture_map) {
  return EncodeTextureMap(object_name, image_index, tex_coord_index, material,
                          texture_map, {});
}

Status GltfAsset::EncodeTextureMap(const std::string &object_name,
                                   int image_index, int tex_coord_index,
                                   const Material &material,
                                   const TextureMap &texture_map,
                                   const std::vector<int> &channels) {
  // Create a new texture sampler (or reuse an existing one if possible).
  const TextureSampler sampler(texture_map.min_filter(),
                               texture_map.mag_filter(),
                               texture_map.wrapping_mode());
  DRACO_ASSIGN_OR_RETURN(const int sampler_index, AddTextureSampler(sampler));

  // Check if we can reuse an existing texture object.
  const GltfTexture texture(image_index, sampler_index);
  const auto texture_it =
      std::find(textures_.begin(), textures_.end(), texture);
  int texture_index;
  if (texture_it == textures_.end()) {
    // Create a new texture object for this texture map.
    texture_index = textures_.size();
    textures_.push_back(GltfTexture(image_index, sampler_index));
  } else {
    // Reuse an existing texture object.
    texture_index = std::distance(textures_.begin(), texture_it);
  }

  gltf_json_.BeginObject(object_name);
  gltf_json_.OutputValue("index", texture_index);
  gltf_json_.OutputValue("texCoord", tex_coord_index);
  if (object_name == "normalTexture") {
    const float scale = material.GetNormalTextureScale();
    if (scale != 1.0f) {
      gltf_json_.OutputValue("scale", scale);
    }
  }

  // The "texture" object of the EXT_mesh_features extension has a custom
  // property "channels" that is encoded here.
  if (object_name == "texture" && !channels.empty()) {
    gltf_json_.BeginArray("channels");
    for (const int channel : channels) {
      gltf_json_.OutputValue(channel);
    }
    gltf_json_.EndArray();  // channels array.
  }

  // Check if |texture_map| is using the KHR_texture_transform extension.
  if (!TextureTransform::IsDefault(texture_map.texture_transform())) {
    gltf_json_.BeginObject("extensions");
    gltf_json_.BeginObject("KHR_texture_transform");
    if (texture_map.texture_transform().IsOffsetSet()) {
      const std::array<double, 2> &offset =
          texture_map.texture_transform().offset();
      gltf_json_.BeginArray("offset");
      gltf_json_.OutputValue(offset[0]);
      gltf_json_.OutputValue(offset[1]);
      gltf_json_.EndArray();
    }
    if (texture_map.texture_transform().IsRotationSet()) {
      gltf_json_.OutputValue("rotation",
                             texture_map.texture_transform().rotation());
    }
    if (texture_map.texture_transform().IsScaleSet()) {
      const std::array<double, 2> &scale =
          texture_map.texture_transform().scale();
      gltf_json_.BeginArray("scale");
      gltf_json_.OutputValue(scale[0]);
      gltf_json_.OutputValue(scale[1]);
      gltf_json_.EndArray();
    }
    // TODO(fgalligan): The spec says the extension is not required if the
    // pre-transform and the post-transform tex coords are the same. But I'm not
    // sure why. I have filed a bug asking for clarification.
    // https://github.com/KhronosGroup/glTF/issues/1724
    if (texture_map.texture_transform().IsTexCoordSet()) {
      gltf_json_.OutputValue("texCoord",
                             texture_map.texture_transform().tex_coord());
    } else {
      extensions_required_.insert("KHR_texture_transform");
    }
    gltf_json_.EndObject();
    gltf_json_.EndObject();

    extensions_used_.insert("KHR_texture_transform");
  }
  gltf_json_.EndObject();
  return OkStatus();
}

Status GltfAsset::EncodeMaterialsProperty(EncoderBuffer *buf_out) {
  gltf_json_.BeginArray("materials");
  for (int i = 0; i < material_library_.NumMaterials(); ++i) {
    const Material *const material = material_library_.GetMaterial(i);
    if (!material) {
      return Status(Status::DRACO_ERROR, "Error getting material.");
    }

    const TextureMap *const color =
        material->GetTextureMapByType(TextureMap::COLOR);
    const TextureMap *const metallic =
        material->GetTextureMapByType(TextureMap::METALLIC_ROUGHNESS);
    const TextureMap *const normal =
        material->GetTextureMapByType(TextureMap::NORMAL_TANGENT_SPACE);
    const TextureMap *const occlusion =
        material->GetTextureMapByType(TextureMap::AMBIENT_OCCLUSION);
    const TextureMap *const emissive =
        material->GetTextureMapByType(TextureMap::EMISSIVE);

    // Check if material is unlit and does not have a fallback.
    if (material->GetUnlit() &&
        (!color || metallic || normal || occlusion || emissive ||
         material->GetMetallicFactor() != 0.0 ||
         material->GetRoughnessFactor() <= 0.5 ||
         material->GetEmissiveFactor() != Vector3f(0.0, 0.0, 0.0))) {
      // If we find one material that is unlit and does not contain a fallback
      // we must set "KHR_materials_unlit" in extensions reqruied for the entire
      // glTF file.
      extensions_required_.insert("KHR_materials_unlit");
    }

    int occlusion_metallic_roughness_image_index = -1;

    gltf_json_.BeginObject();  // material object.

    gltf_json_.BeginObject("pbrMetallicRoughness");
    if (color) {
      const bool rgba = true;  // Unused for now.
      const std::string texture_stem = TextureUtils::GetOrGenerateTargetStem(
          *color->texture(), i, "_BaseColor");
      DRACO_ASSIGN_OR_RETURN(
          const int color_image_index,
          AddImage(texture_stem, color->texture(), rgba ? 4 : 3));
      DRACO_RETURN_IF_ERROR(
          EncodeTextureMap("baseColorTexture", color_image_index,
                           color->tex_coord_index(), *material, *color));
    }
    // Try to combine metallic and occlusion only if they have the same tex
    // coord index.
    // TODO(b/145991271): Check out if we need to check texture indices.
    if (metallic && occlusion &&
        metallic->tex_coord_index() == occlusion->tex_coord_index()) {
      if (metallic->texture() == occlusion->texture()) {
        const std::string texture_stem = TextureUtils::GetOrGenerateTargetStem(
            *metallic->texture(), i, "_OcclusionMetallicRoughness");
        // Metallic and occlusion textures are already combined.
        DRACO_ASSIGN_OR_RETURN(occlusion_metallic_roughness_image_index,
                               AddImage(texture_stem, metallic->texture(), 3));
      }
      if (occlusion_metallic_roughness_image_index != -1)
        DRACO_RETURN_IF_ERROR(EncodeTextureMap(
            "metallicRoughnessTexture",
            occlusion_metallic_roughness_image_index,
            metallic->tex_coord_index(), *material, *metallic));
    }

    if (metallic && occlusion_metallic_roughness_image_index == -1) {
      const std::string texture_stem = TextureUtils::GetOrGenerateTargetStem(
          *metallic->texture(), i, "_MetallicRoughness");
      DRACO_ASSIGN_OR_RETURN(const int metallic_roughness_image_index,
                             AddImage(texture_stem, metallic->texture(), 3));
      DRACO_RETURN_IF_ERROR(EncodeTextureMap(
          "metallicRoughnessTexture", metallic_roughness_image_index,
          metallic->tex_coord_index(), *material, *metallic));
    }

    EncodeVectorArray<Vector4f>("baseColorFactor", material->GetColorFactor());
    gltf_json_.OutputValue("metallicFactor", material->GetMetallicFactor());
    gltf_json_.OutputValue("roughnessFactor", material->GetRoughnessFactor());
    gltf_json_.EndObject();  // pbrMetallicRoughness

    if (normal) {
      const std::string texture_stem = TextureUtils::GetOrGenerateTargetStem(
          *normal->texture(), i, "_Normal");
      DRACO_ASSIGN_OR_RETURN(const int normal_image_index,
                             AddImage(texture_stem, normal->texture(), 3));
      DRACO_RETURN_IF_ERROR(
          EncodeTextureMap("normalTexture", normal_image_index,
                           normal->tex_coord_index(), *material, *normal));
    }

    if (occlusion_metallic_roughness_image_index != -1) {
      DRACO_RETURN_IF_ERROR(EncodeTextureMap(
          "occlusionTexture", occlusion_metallic_roughness_image_index,
          metallic->tex_coord_index(), *material, *metallic));
    } else if (occlusion) {
      // Store occlusion texture in a grayscale format, unless it is used by
      // metallic-roughness map of some other matierial. It is possible that
      // this material uses occlusion (R channel) and some other material uses
      // metallic-roughness (GB channels) from this texture.
      const int num_components = TextureUtils::ComputeRequiredNumChannels(
          *occlusion->texture(), material_library_);
      const std::string suffix =
          (num_components == 1) ? "_Occlusion" : "_OcclusionMetallicRoughness";
      const std::string texture_stem = TextureUtils::GetOrGenerateTargetStem(
          *occlusion->texture(), i, suffix);
      DRACO_ASSIGN_OR_RETURN(
          const int occlusion_image_index,
          AddImage(texture_stem, occlusion->texture(), num_components));
      DRACO_RETURN_IF_ERROR(EncodeTextureMap(
          "occlusionTexture", occlusion_image_index,
          occlusion->tex_coord_index(), *material, *occlusion));
    }

    if (emissive) {
      const std::string texture_stem = TextureUtils::GetOrGenerateTargetStem(
          *emissive->texture(), i, "_Emissive");
      DRACO_ASSIGN_OR_RETURN(const int emissive_image_index,
                             AddImage(texture_stem, emissive->texture(), 3));
      DRACO_RETURN_IF_ERROR(
          EncodeTextureMap("emissiveTexture", emissive_image_index,
                           emissive->tex_coord_index(), *material, *emissive));
    }

    EncodeVectorArray<Vector3f>("emissiveFactor",
                                material->GetEmissiveFactor());

    switch (material->GetTransparencyMode()) {
      case Material::TransparencyMode::TRANSPARENCY_MASK:
        gltf_json_.OutputValue("alphaMode", "MASK");
        gltf_json_.OutputValue("alphaCutoff", material->GetAlphaCutoff());
        break;
      case Material::TransparencyMode::TRANSPARENCY_BLEND:
        gltf_json_.OutputValue("alphaMode", "BLEND");
        break;
      default:
        gltf_json_.OutputValue("alphaMode", "OPAQUE");
        break;
    }
    if (!material->GetName().empty()) {
      gltf_json_.OutputValue("name", material->GetName());
    }

    // Output doubleSided if different than the default.
    if (material->GetDoubleSided()) {
      gltf_json_.OutputValue("doubleSided", material->GetDoubleSided());
    }

    // Encode material extensions if any.
    if (material->GetUnlit() || material->HasSheen() ||
        material->HasTransmission() || material->HasClearcoat() ||
        material->HasVolume() || material->HasIor() ||
        material->HasSpecular()) {
      gltf_json_.BeginObject("extensions");

      // Encode individual material extensions.
      if (material->GetUnlit()) {
        EncodeMaterialUnlitExtension(*material);
      } else {
        // PBR extensions can only be added to non-unlit materials.
        Material defaults;
        if (material->HasSheen()) {
          DRACO_RETURN_IF_ERROR(
              EncodeMaterialSheenExtension(*material, defaults, i));
        }
        if (material->HasTransmission()) {
          DRACO_RETURN_IF_ERROR(
              EncodeMaterialTransmissionExtension(*material, defaults, i));
        }
        if (material->HasClearcoat()) {
          DRACO_RETURN_IF_ERROR(
              EncodeMaterialClearcoatExtension(*material, defaults, i));
        }
        if (material->HasVolume()) {
          DRACO_RETURN_IF_ERROR(
              EncodeMaterialVolumeExtension(*material, defaults, i));
        }
        if (material->HasIor()) {
          DRACO_RETURN_IF_ERROR(
              EncodeMaterialIorExtension(*material, defaults));
        }
        if (material->HasSpecular()) {
          DRACO_RETURN_IF_ERROR(
              EncodeMaterialSpecularExtension(*material, defaults, i));
        }
      }

      gltf_json_.EndObject();  // extensions object.
    }

    gltf_json_.EndObject();  // material object.
  }

  gltf_json_.EndArray();  // materials array.

  if (!textures_.empty()) {
    gltf_json_.BeginArray("textures");
    for (int i = 0; i < textures_.size(); ++i) {
      const int image_index = textures_[i].image_index;
      gltf_json_.BeginObject();
      if (images_[image_index].mime_type == "image/webp") {
        gltf_json_.BeginObject("extensions");
        gltf_json_.BeginObject("EXT_texture_webp");
        gltf_json_.OutputValue("source", image_index);
        gltf_json_.EndObject();
        gltf_json_.EndObject();
      } else if (images_[image_index].mime_type == "image/ktx2") {
        gltf_json_.BeginObject("extensions");
        gltf_json_.BeginObject("KHR_texture_basisu");
        gltf_json_.OutputValue("source", image_index);
        gltf_json_.EndObject();
        gltf_json_.EndObject();
      } else {
        gltf_json_.OutputValue("source", image_index);
      }
      if (textures_[i].sampler_index >= 0) {
        gltf_json_.OutputValue("sampler", textures_[i].sampler_index);
      }
      gltf_json_.EndObject();
    }
    gltf_json_.EndArray();
  }

  if (!texture_samplers_.empty()) {
    gltf_json_.BeginArray("samplers");
    for (int i = 0; i < texture_samplers_.size(); ++i) {
      gltf_json_.BeginObject();

      const int mode_s = TextureAxisWrappingModeToGltfValue(
          texture_samplers_[i].wrapping_mode.s);
      const int mode_t = TextureAxisWrappingModeToGltfValue(
          texture_samplers_[i].wrapping_mode.t);
      gltf_json_.OutputValue("wrapS", mode_s);
      gltf_json_.OutputValue("wrapT", mode_t);

      if (texture_samplers_[i].min_filter != TextureMap::UNSPECIFIED) {
        gltf_json_.OutputValue(
            "minFilter",
            TextureFilterTypeToGltfValue(texture_samplers_[i].min_filter));
      }
      if (texture_samplers_[i].mag_filter != TextureMap::UNSPECIFIED) {
        gltf_json_.OutputValue(
            "magFilter",
            TextureFilterTypeToGltfValue(texture_samplers_[i].mag_filter));
      }

      gltf_json_.EndObject();
    }
    gltf_json_.EndArray();
  }

  if (!images_.empty()) {
    gltf_json_.BeginArray("images");
    for (int i = 0; i < images_.size(); ++i) {
      if (add_images_to_buffer_) {
        DRACO_RETURN_IF_ERROR(SaveImageToBuffer(i));
      }
      gltf_json_.BeginObject();
      if (images_[i].buffer_view >= 0) {
        gltf_json_.OutputValue("bufferView", images_[i].buffer_view);
        gltf_json_.OutputValue("mimeType", images_[i].mime_type);
      } else {
        gltf_json_.OutputValue("uri", images_[i].image_name);
      }
      gltf_json_.EndObject();
    }
    gltf_json_.EndArray();
  }

  const std::string asset_str = gltf_json_.MoveData();
  if (!buf_out->Encode(asset_str.data(), asset_str.length())) {
    return Status(Status::DRACO_ERROR, "Error encoding materials.");
  }
  return OkStatus();
}

void GltfAsset::EncodeMaterialUnlitExtension(const Material &material) {
  extensions_used_.insert("KHR_materials_unlit");
  gltf_json_.BeginObject("KHR_materials_unlit");
  gltf_json_.EndObject();
}

Status GltfAsset::EncodeMaterialSheenExtension(const Material &material,
                                               const Material &defaults,
                                               int material_index) {
  extensions_used_.insert("KHR_materials_sheen");
  gltf_json_.BeginObject("KHR_materials_sheen");

  // Add sheen color factor, unless it is the default.
  if (material.GetSheenColorFactor() != defaults.GetSheenColorFactor()) {
    EncodeVectorArray<Vector3f>("sheenColorFactor",
                                material.GetSheenColorFactor());
  }

  // Add sheen roughness factor, unless it is the default.
  if (material.GetSheenRoughnessFactor() !=
      defaults.GetSheenRoughnessFactor()) {
    gltf_json_.OutputValue("sheenRoughnessFactor",
                           material.GetSheenRoughnessFactor());
  }

  // Add sheen color texture (RGB channels) if present.
  // TODO(vytyaz): Combine sheen color and roughness images if possible.
  DRACO_RETURN_IF_ERROR(EncodeTexture("sheenColorTexture", "_SheenColor",
                                      TextureMap::SHEEN_COLOR, -1, material,
                                      material_index));

  // Add sheen roughness texture (A channel) if present.
  DRACO_RETURN_IF_ERROR(
      EncodeTexture("sheenRoughnessTexture", "_SheenRoughness",
                    TextureMap::SHEEN_ROUGHNESS, 4, material, material_index));

  gltf_json_.EndObject();  // KHR_materials_sheen object.

  return OkStatus();
}

Status GltfAsset::EncodeMaterialTransmissionExtension(const Material &material,
                                                      const Material &defaults,
                                                      int material_index) {
  extensions_used_.insert("KHR_materials_transmission");
  gltf_json_.BeginObject("KHR_materials_transmission");

  // Add transmission factor, unless it is the default.
  if (material.GetTransmissionFactor() != defaults.GetTransmissionFactor()) {
    gltf_json_.OutputValue("transmissionFactor",
                           material.GetTransmissionFactor());
  }

  // Add transmission texture (R channel) if present.
  // TODO(vytyaz): Store texture in a grayscale format if possible.
  DRACO_RETURN_IF_ERROR(EncodeTexture("transmissionTexture", "_Transmission",
                                      TextureMap::TRANSMISSION, 3, material,
                                      material_index));

  gltf_json_.EndObject();  // KHR_materials_transmission object.

  return OkStatus();
}

Status GltfAsset::EncodeMaterialClearcoatExtension(const Material &material,
                                                   const Material &defaults,
                                                   int material_index) {
  extensions_used_.insert("KHR_materials_clearcoat");
  gltf_json_.BeginObject("KHR_materials_clearcoat");

  // Add clearcoat factor, unless it is the default.
  if (material.GetClearcoatFactor() != defaults.GetClearcoatFactor()) {
    gltf_json_.OutputValue("clearcoatFactor", material.GetClearcoatFactor());
  }

  // Add clearcoat roughness factor, unless it is the default.
  if (material.GetClearcoatRoughnessFactor() !=
      defaults.GetClearcoatRoughnessFactor()) {
    gltf_json_.OutputValue("clearcoatRoughnessFactor",
                           material.GetClearcoatRoughnessFactor());
  }

  // Add clearcoat texture (R channel) if present.
  // TODO(vytyaz): Combine clearcoat and clearcoat roughness images if possible.
  // TODO(vytyaz): Store texture in a grayscale format if possible.
  DRACO_RETURN_IF_ERROR(EncodeTexture("clearcoatTexture", "_Clearcoat",
                                      TextureMap::CLEARCOAT, 3, material,
                                      material_index));

  // Add clearcoat roughness texture (G channel) if present.
  DRACO_RETURN_IF_ERROR(EncodeTexture(
      "clearcoatRoughnessTexture", "_ClearcoatRoughness",
      TextureMap::CLEARCOAT_ROUGHNESS, 3, material, material_index));

  // Add clearcoat normal texture (RGB channels) if present.
  DRACO_RETURN_IF_ERROR(
      EncodeTexture("clearcoatNormalTexture", "_ClearcoatNormal",
                    TextureMap::CLEARCOAT_NORMAL, 3, material, material_index));

  gltf_json_.EndObject();  // KHR_materials_clearcoat object.

  return OkStatus();
}

Status GltfAsset::EncodeMaterialVolumeExtension(const Material &material,
                                                const Material &defaults,
                                                int material_index) {
  extensions_used_.insert("KHR_materials_volume");
  gltf_json_.BeginObject("KHR_materials_volume");

  // Add thickness factor, unless it is the default.
  if (material.GetThicknessFactor() != defaults.GetThicknessFactor()) {
    gltf_json_.OutputValue("thicknessFactor", material.GetThicknessFactor());
  }

  // Add attenuation distance, unless it is the default.
  if (material.GetAttenuationDistance() != defaults.GetAttenuationDistance()) {
    gltf_json_.OutputValue("attenuationDistance",
                           material.GetAttenuationDistance());
  }

  // Add attenuation color, unless it is the default.
  if (material.GetAttenuationColor() != defaults.GetAttenuationColor()) {
    EncodeVectorArray<Vector3f>("attenuationColor",
                                material.GetAttenuationColor());
  }

  // Add thickness texture (G channel) if present.
  DRACO_RETURN_IF_ERROR(EncodeTexture("thicknessTexture", "_Thickness",
                                      TextureMap::THICKNESS, 3, material,
                                      material_index));

  gltf_json_.EndObject();  // KHR_materials_volume object.

  return OkStatus();
}

Status GltfAsset::EncodeMaterialIorExtension(const Material &material,
                                             const Material &defaults) {
  extensions_used_.insert("KHR_materials_ior");
  gltf_json_.BeginObject("KHR_materials_ior");

  // Add ior, unless it is the default.
  if (material.GetIor() != defaults.GetIor()) {
    gltf_json_.OutputValue("ior", material.GetIor());
  }

  gltf_json_.EndObject();  // KHR_materials_ior object.

  return OkStatus();
}

Status GltfAsset::EncodeMaterialSpecularExtension(const Material &material,
                                                  const Material &defaults,
                                                  int material_index) {
  extensions_used_.insert("KHR_materials_specular");
  gltf_json_.BeginObject("KHR_materials_specular");

  // Add specular factor, unless it is the default.
  if (material.GetSpecularFactor() != defaults.GetSpecularFactor()) {
    gltf_json_.OutputValue("specularFactor", material.GetSpecularFactor());
  }

  // Add specular color factor, unless it is the default.
  if (material.GetSpecularColorFactor() != defaults.GetSpecularColorFactor()) {
    EncodeVectorArray<Vector3f>("specularColorFactor",
                                material.GetSpecularColorFactor());
  }

  // Add specular texture (A channel) if present.
  // TODO(vytyaz): Combine specular and specular color images if possible.
  DRACO_RETURN_IF_ERROR(EncodeTexture("specularTexture", "_Specular",
                                      TextureMap::SPECULAR, 4, material,
                                      material_index));

  // Add specular color texture (RGB channels) if present.
  DRACO_RETURN_IF_ERROR(EncodeTexture("specularColorTexture", "_SpecularColor",
                                      TextureMap::SPECULAR_COLOR, -1, material,
                                      material_index));

  gltf_json_.EndObject();  // KHR_materials_specular object.

  return OkStatus();
}

Status GltfAsset::EncodeTexture(const std::string &name,
                                const std::string &stem_suffix,
                                TextureMap::Type type, int num_components,
                                const Material &material, int material_index) {
  const TextureMap *const texture_map = material.GetTextureMapByType(type);
  if (texture_map) {
    if (num_components == -1) {
      const bool rgba = true;  // Unused for now.
      num_components = rgba ? 4 : 3;
    }
    const std::string texture_stem = TextureUtils::GetOrGenerateTargetStem(
        *texture_map->texture(), material_index, stem_suffix);
    DRACO_ASSIGN_OR_RETURN(
        const int image_index,
        AddImage(texture_stem, texture_map->texture(), num_components));
    DRACO_RETURN_IF_ERROR(EncodeTextureMap(name, image_index,
                                           texture_map->tex_coord_index(),
                                           material, *texture_map));
  }
  return OkStatus();
}

Status GltfAsset::EncodeAnimationsProperty(EncoderBuffer *buf_out) {
  if (animations_.empty()) {
    return OkStatus();
  }

  gltf_json_.BeginArray("animations");
  for (int i = 0; i < animations_.size(); ++i) {
    gltf_json_.BeginObject();

    if (!animations_[i]->name.empty()) {
      gltf_json_.OutputValue("name", animations_[i]->name);
    }

    gltf_json_.BeginArray("samplers");
    for (int j = 0; j < animations_[i]->samplers.size(); ++j) {
      gltf_json_.BeginObject();
      gltf_json_.OutputValue("input", animations_[i]->samplers[j]->input_index);
      gltf_json_.OutputValue(
          "interpolation",
          AnimationSampler::InterpolationToString(
              animations_[i]->samplers[j]->interpolation_type));
      gltf_json_.OutputValue("output",
                             animations_[i]->samplers[j]->output_index);
      gltf_json_.EndObject();
    }
    gltf_json_.EndArray();

    gltf_json_.BeginArray("channels");
    for (int j = 0; j < animations_[i]->channels.size(); ++j) {
      gltf_json_.BeginObject();
      gltf_json_.OutputValue("sampler",
                             animations_[i]->channels[j]->sampler_index);

      gltf_json_.BeginObject("target");
      gltf_json_.OutputValue("node", animations_[i]->channels[j]->target_index);
      gltf_json_.OutputValue(
          "path", AnimationChannel::TransformationToString(
                      animations_[i]->channels[j]->transformation_type));
      gltf_json_.EndObject();

      gltf_json_.EndObject();  // Channel entry.
    }
    gltf_json_.EndArray();

    gltf_json_.EndObject();  // Animmation entry.
  }
  gltf_json_.EndArray();

  const std::string asset_str = gltf_json_.MoveData();
  if (!buf_out->Encode(asset_str.data(), asset_str.length())) {
    return Status(Status::DRACO_ERROR, "Could not encode animations.");
  }
  return OkStatus();
}

Status GltfAsset::EncodeSkinsProperty(EncoderBuffer *buf_out) {
  if (skins_.empty()) {
    return OkStatus();
  }

  gltf_json_.BeginArray("skins");
  for (int i = 0; i < skins_.size(); ++i) {
    gltf_json_.BeginObject();

    if (skins_[i]->inverse_bind_matrices_index >= 0) {
      gltf_json_.OutputValue("inverseBindMatrices",
                             skins_[i]->inverse_bind_matrices_index);
    }
    if (skins_[i]->skeleton_index >= 0) {
      gltf_json_.OutputValue("skeleton", skins_[i]->skeleton_index);
    }

    if (!skins_[i]->joints.empty()) {
      gltf_json_.BeginArray("joints");
      for (int j = 0; j < skins_[i]->joints.size(); ++j) {
        gltf_json_.OutputValue(skins_[i]->joints[j]);
      }
      gltf_json_.EndArray();
    }
    gltf_json_.EndObject();  // Skin entry.
  }
  gltf_json_.EndArray();

  const std::string asset_str = gltf_json_.MoveData();
  if (!buf_out->Encode(asset_str.data(), asset_str.length())) {
    return Status(Status::DRACO_ERROR, "Could not encode animations.");
  }
  return OkStatus();
}

Status GltfAsset::EncodeTopLevelExtensionsProperty(EncoderBuffer *buf_out) {
  // Return if there are no top-level asset extensions to encode.
  if (lights_.empty() && materials_variants_names_.empty() &&
      structural_metadata_->NumPropertyTables() == 0 &&
      structural_metadata_->NumPropertyAttributes() == 0) {
    return OkStatus();
  }

  // Encode top-level extensions.
  gltf_json_.BeginObject("extensions");
  DRACO_RETURN_IF_ERROR(EncodeLightsProperty(buf_out));
  DRACO_RETURN_IF_ERROR(EncodeMaterialsVariantsNamesProperty(buf_out));
  DRACO_RETURN_IF_ERROR(EncodeStructuralMetadataProperty(buf_out));
  gltf_json_.EndObject();  // extensions entry.
  return OkStatus();
}

Status GltfAsset::EncodeLightsProperty(EncoderBuffer *buf_out) {
  if (lights_.empty()) {
    return OkStatus();
  }

  gltf_json_.BeginObject("KHR_lights_punctual");
  gltf_json_.BeginArray("lights");
  const Light defaults;
  for (const auto &light : lights_) {
    gltf_json_.BeginObject();
    if (light->GetName() != defaults.GetName()) {
      gltf_json_.OutputValue("name", light->GetName());
    }
    if (light->GetColor() != defaults.GetColor()) {
      gltf_json_.BeginArray("color");
      gltf_json_.OutputValue(light->GetColor()[0]);
      gltf_json_.OutputValue(light->GetColor()[1]);
      gltf_json_.OutputValue(light->GetColor()[2]);
      gltf_json_.EndArray();
    }
    if (light->GetIntensity() != defaults.GetIntensity()) {
      gltf_json_.OutputValue("intensity", light->GetIntensity());
    }
    switch (light->GetType()) {
      case Light::DIRECTIONAL:
        gltf_json_.OutputValue("type", "directional");
        break;
      case Light::POINT:
        gltf_json_.OutputValue("type", "point");
        break;
      case Light::SPOT:
        gltf_json_.OutputValue("type", "spot");
        break;
    }
    if (light->GetRange() != defaults.GetRange()) {
      gltf_json_.OutputValue("range", light->GetRange());
    }
    if (light->GetType() == Light::SPOT) {
      gltf_json_.BeginObject("spot");
      if (light->GetInnerConeAngle() != defaults.GetInnerConeAngle()) {
        gltf_json_.OutputValue("innerConeAngle", light->GetInnerConeAngle());
      }
      if (light->GetOuterConeAngle() != defaults.GetOuterConeAngle()) {
        gltf_json_.OutputValue("outerConeAngle", light->GetOuterConeAngle());
      }
      gltf_json_.EndObject();
    }
    gltf_json_.EndObject();
  }
  gltf_json_.EndArray();
  gltf_json_.EndObject();  // KHR_lights_punctual entry.
  return OkStatus();
}

Status GltfAsset::EncodeMaterialsVariantsNamesProperty(EncoderBuffer *buf_out) {
  if (materials_variants_names_.empty()) {
    return OkStatus();
  }

  gltf_json_.BeginObject("KHR_materials_variants");
  gltf_json_.BeginArray("variants");
  for (const std::string &name : materials_variants_names_) {
    gltf_json_.BeginObject();
    gltf_json_.OutputValue("name", name);
    gltf_json_.EndObject();
  }
  gltf_json_.EndArray();
  gltf_json_.EndObject();  // KHR_materials_variants entry.
  return OkStatus();
}

Status GltfAsset::EncodeStructuralMetadataProperty(EncoderBuffer *buf_out) {
  if (structural_metadata_->GetSchema().Empty()) {
    return OkStatus();
  }

  structural_metadata_used_ = true;
  gltf_json_.BeginObject("EXT_structural_metadata");

  // Encodes structural metadata schema.
  struct SchemaWriter {
    typedef StructuralMetadataSchema::Object Object;
    static void Write(const Object &object, JsonWriter *json_writer) {
      switch (object.GetType()) {
        case Object::OBJECT:
          json_writer->BeginObject(object.GetName());
          for (const Object &obj : object.GetObjects()) {
            Write(obj, json_writer);
          }
          json_writer->EndObject();
          break;
        case Object::ARRAY:
          json_writer->BeginArray(object.GetName());
          for (const Object &obj : object.GetArray()) {
            Write(obj, json_writer);
          }
          json_writer->EndArray();
          break;
        case Object::STRING:
          json_writer->OutputValue(object.GetName(), object.GetString());
          break;
        case Object::INTEGER:
          json_writer->OutputValue(object.GetName(), object.GetInteger());
          break;
        case Object::BOOLEAN:
          json_writer->OutputValue(object.GetName(), object.GetBoolean());
          break;
      }
    }
  };

  // Encode property table schema.
  SchemaWriter::Write(structural_metadata_->GetSchema().json, &gltf_json_);

  // Encode all property tables.
  gltf_json_.BeginArray("propertyTables");
  for (int i = 0; i < structural_metadata_->NumPropertyTables(); i++) {
    const PropertyTable *const table =
        &structural_metadata_->GetPropertyTable(i);
    gltf_json_.BeginObject();
    if (!table->GetName().empty()) {
      gltf_json_.OutputValue("name", table->GetName());
    }
    if (!table->GetClass().empty()) {
      gltf_json_.OutputValue("class", table->GetClass());
    }
    gltf_json_.OutputValue("count", table->GetCount());

    // Encoder all property table properties.
    gltf_json_.BeginObject("properties");
    for (int i = 0; i < table->NumProperties(); ++i) {
      const PropertyTable::Property &property = table->GetProperty(i);
      gltf_json_.BeginObject(property.GetName());

      // Encode property values.
      DRACO_ASSIGN_OR_RETURN(const int buffer_view_index,
                             AddBufferView(property.GetData()));
      gltf_json_.OutputValue("values", buffer_view_index);

      // Encode offsets for variable-length arrays.
      if (!property.GetArrayOffsets().data.data.empty()) {
        if (!property.GetArrayOffsets().type.empty()) {
          gltf_json_.OutputValue("arrayOffsetType",
                                 property.GetArrayOffsets().type);
        }
        DRACO_ASSIGN_OR_RETURN(const int buffer_view_index,
                               AddBufferView(property.GetArrayOffsets().data));
        gltf_json_.OutputValue("arrayOffsets", buffer_view_index);
      }

      // Encode offsets for strings.
      if (!property.GetStringOffsets().data.data.empty()) {
        if (!property.GetStringOffsets().type.empty()) {
          gltf_json_.OutputValue("stringOffsetType",
                                 property.GetStringOffsets().type);
        }
        DRACO_ASSIGN_OR_RETURN(const int buffer_view_index,
                               AddBufferView(property.GetStringOffsets().data));
        gltf_json_.OutputValue("stringOffsets", buffer_view_index);
      }
      gltf_json_.EndObject();  // Named property entry.
    }
    gltf_json_.EndObject();  // properties entry.
    gltf_json_.EndObject();
  }
  gltf_json_.EndArray();  // propertyTables entry.

  // Encode all property attributes.
  gltf_json_.BeginArray("propertyAttributes");
  for (int i = 0; i < structural_metadata_->NumPropertyAttributes(); i++) {
    const PropertyAttribute *const attribute =
        &structural_metadata_->GetPropertyAttribute(i);
    gltf_json_.BeginObject();
    if (!attribute->GetName().empty()) {
      gltf_json_.OutputValue("name", attribute->GetName());
    }
    if (!attribute->GetClass().empty()) {
      gltf_json_.OutputValue("class", attribute->GetClass());
    }

    // Encoder all property attribute properties.
    gltf_json_.BeginObject("properties");
    for (int i = 0; i < attribute->NumProperties(); ++i) {
      const PropertyAttribute::Property &property = attribute->GetProperty(i);
      gltf_json_.BeginObject(property.GetName());
      gltf_json_.OutputValue("attribute", property.GetAttributeName());
      gltf_json_.EndObject();  // Named property entry.
    }
    gltf_json_.EndObject();  // properties entry.
    gltf_json_.EndObject();
  }
  gltf_json_.EndArray();   // propertyAttributes entry.
  gltf_json_.EndObject();  // EXT_structural_metadata entry.
  return OkStatus();
}

bool GltfAsset::EncodeAccessorsProperty(EncoderBuffer *buf_out) {
  gltf_json_.BeginArray("accessors");

  for (int i = 0; i < accessors_.size(); ++i) {
    gltf_json_.BeginObject();

    if (accessors_[i].buffer_view_index >= 0) {
      gltf_json_.OutputValue("bufferView", accessors_[i].buffer_view_index);
      if (output_type_ == GltfEncoder::VERBOSE) {
        gltf_json_.OutputValue("byteOffset", 0);
      }
    }
    gltf_json_.OutputValue("componentType", accessors_[i].component_type);
    gltf_json_.OutputValue("count", accessors_[i].count);
    if (accessors_[i].normalized) {
      gltf_json_.OutputValue("normalized", accessors_[i].normalized);
    }

    if (!accessors_[i].max.empty()) {
      gltf_json_.BeginArray("max");
      for (int j = 0; j < accessors_[i].max.size(); ++j) {
        gltf_json_.OutputValue(accessors_[i].max[j]);
      }
      gltf_json_.EndArray();
    }

    if (!accessors_[i].min.empty()) {
      gltf_json_.BeginArray("min");
      for (int j = 0; j < accessors_[i].min.size(); ++j) {
        gltf_json_.OutputValue(accessors_[i].min[j]);
      }
      gltf_json_.EndArray();
    }

    gltf_json_.OutputValue("type", accessors_[i].type);

    gltf_json_.EndObject();
  }

  gltf_json_.EndArray();

  const std::string asset_str = gltf_json_.MoveData();
  return buf_out->Encode(asset_str.data(), asset_str.length());
}

bool GltfAsset::EncodeBufferViewsProperty(EncoderBuffer *buf_out) {
  // We currently only support one buffer.
  gltf_json_.BeginArray("bufferViews");

  for (int i = 0; i < buffer_views_.size(); ++i) {
    gltf_json_.BeginObject();
    gltf_json_.OutputValue("buffer", 0);
    gltf_json_.OutputValue("byteOffset", buffer_views_[i].buffer_byte_offset);
    gltf_json_.OutputValue("byteLength", buffer_views_[i].byte_length);
    if (buffer_views_[i].target != 0) {
      gltf_json_.OutputValue("target", buffer_views_[i].target);
    }
    gltf_json_.EndObject();
  }

  gltf_json_.EndArray();

  const std::string asset_str = gltf_json_.MoveData();
  return buf_out->Encode(asset_str.data(), asset_str.length());
}

bool GltfAsset::EncodeBuffersProperty(EncoderBuffer *buf_out) {
  if (buffer_.size() == 0) {
    return true;
  }
  // We currently only support one buffer.
  gltf_json_.BeginArray("buffers");
  gltf_json_.BeginObject();
  gltf_json_.OutputValue("byteLength", buffer_.size());
  if (!buffer_name_.empty()) {
    gltf_json_.OutputValue("uri", buffer_name_);
  }
  gltf_json_.EndObject();
  gltf_json_.EndArray();

  const std::string asset_str = gltf_json_.MoveData();
  return buf_out->Encode(asset_str.data(), asset_str.length());
}

Status GltfAsset::EncodeExtensionsProperties(EncoderBuffer *buf_out) {
  if (draco_compression_used_) {
    const std::string draco_tag = "KHR_draco_mesh_compression";
    extensions_used_.insert(draco_tag);
    extensions_required_.insert(draco_tag);
  }
  if (!lights_.empty()) {
    extensions_used_.insert("KHR_lights_punctual");
  }
  if (!materials_variants_names_.empty()) {
    extensions_used_.insert("KHR_materials_variants");
  }
  if (!instance_arrays_.empty()) {
    extensions_used_.insert("EXT_mesh_gpu_instancing");
    extensions_required_.insert("EXT_mesh_gpu_instancing");
  }
  if (mesh_features_used_) {
    extensions_used_.insert("EXT_mesh_features");
  }
  if (structural_metadata_used_) {
    extensions_used_.insert("EXT_structural_metadata");
  }

  if (!extensions_required_.empty()) {
    gltf_json_.BeginArray("extensionsRequired");
    for (const auto &extension : extensions_required_) {
      gltf_json_.OutputValue(extension);
    }
    gltf_json_.EndArray();
  }
  if (!extensions_used_.empty()) {
    gltf_json_.BeginArray("extensionsUsed");
    for (const auto &extension : extensions_used_) {
      gltf_json_.OutputValue(extension);
    }
    gltf_json_.EndArray();
  }

  const std::string asset_str = gltf_json_.MoveData();
  if (!asset_str.empty()) {
    if (!buf_out->Encode(asset_str.data(), asset_str.length())) {
      return Status(Status::DRACO_ERROR, "Could not encode extensions.");
    }
  }
  return OkStatus();
}

template <>
GltfAsset::ComponentType GltfAsset::GetComponentType<int8_t>() const {
  return BYTE;
}

template <>
GltfAsset::ComponentType GltfAsset::GetComponentType<uint8_t>() const {
  return UNSIGNED_BYTE;
}

template <>
GltfAsset::ComponentType GltfAsset::GetComponentType<int16_t>() const {
  return SHORT;
}

template <>
GltfAsset::ComponentType GltfAsset::GetComponentType<uint16_t>() const {
  return UNSIGNED_SHORT;
}

template <>
GltfAsset::ComponentType GltfAsset::GetComponentType<uint32_t>() const {
  return UNSIGNED_INT;
}

template <>
GltfAsset::ComponentType GltfAsset::GetComponentType<float>() const {
  return FLOAT;
}

template <size_t att_components_t, class att_data_t>
int GltfAsset::AddAttribute(const PointAttribute &att, int num_points,
                            int num_encoded_points, const std::string &type,
                            bool compress) {
  if (att.size() == 0) {
    return -1;  // Attribute size must be greater than 0.
  }

  std::array<att_data_t, att_components_t> value;
  std::array<att_data_t, att_components_t> min_values;
  std::array<att_data_t, att_components_t> max_values;

  // Set min and max values.
  if (!att.ConvertValue<att_data_t, att_components_t>(AttributeValueIndex(0),
                                                      &min_values[0])) {
    return -1;
  }
  max_values = min_values;

  if (output_type_ == GltfEncoder::VERBOSE ||
      att.attribute_type() == GeometryAttribute::POSITION) {
    for (AttributeValueIndex i(1); i < static_cast<uint32_t>(att.size()); ++i) {
      if (!att.ConvertValue<att_data_t, att_components_t>(i, &value[0])) {
        return -1;
      }
      for (int j = 0; j < att_components_t; ++j) {
        if (value[j] < min_values[j]) {
          min_values[j] = value[j];
        }
        if (value[j] > max_values[j]) {
          max_values[j] = value[j];
        }
      }
    }
  }

  const int kComponentSize = sizeof(att_data_t);

  GltfAccessor accessor;
  if (!compress) {
    const size_t buffer_start_offset = buffer_.size();
    for (PointIndex v(0); v < num_points; ++v) {
      if (!att.ConvertValue<att_data_t, att_components_t>(att.mapped_index(v),
                                                          &value[0])) {
        return -1;
      }

      for (int j = 0; j < att_components_t; ++j) {
        buffer_.Encode(&value[j], kComponentSize);
      }
    }

    if (!PadBuffer()) {
      return -1;
    }

    GltfBufferView buffer_view;
    buffer_view.buffer_byte_offset = buffer_start_offset;
    buffer_view.byte_length = buffer_.size() - buffer_start_offset;
    buffer_views_.push_back(buffer_view);
    accessor.buffer_view_index = static_cast<int>(buffer_views_.size() - 1);
  }

  accessor.component_type = GetComponentType<att_data_t>();
  accessor.count = num_encoded_points;
  if (output_type_ == GltfEncoder::VERBOSE ||
      att.attribute_type() == GeometryAttribute::POSITION) {
    for (int j = 0; j < att_components_t; ++j) {
      accessor.max.push_back(GltfValue(max_values[j]));
      accessor.min.push_back(GltfValue(min_values[j]));
    }
  }
  accessor.type = type;
  accessor.normalized = att.attribute_type() == GeometryAttribute::COLOR &&
                        att.data_type() != DT_FLOAT32;
  accessors_.push_back(accessor);
  return static_cast<int>(accessors_.size() - 1);
}

void GltfAsset::SetCopyrightFromScene(const Scene &scene) {
  std::string copyright;
  scene.GetMetadata().GetEntryString("copyright", &copyright);
  set_copyright(copyright);
}

void GltfAsset::SetCopyrightFromMesh(const Mesh &mesh) {
  if (mesh.GetMetadata() != nullptr) {
    std::string copyright;
    mesh.GetMetadata()->GetEntryString("copyright", &copyright);
    set_copyright(copyright);
  }
}

const char GltfEncoder::kDracoMetadataGltfAttributeName[] =
    "//GLTF/ApplicationSpecificAttributeName";

GltfEncoder::GltfEncoder() : out_buffer_(nullptr), output_type_(COMPACT) {}

template <typename T>
bool GltfEncoder::EncodeToFile(const T &geometry, const std::string &file_name,
                               const std::string &base_dir) {
  const std::string buffer_name = base_dir + "/buffer0.bin";
  return EncodeFile(geometry, file_name, buffer_name, base_dir).ok();
}

template <typename T>
Status GltfEncoder::EncodeFile(const T &geometry, const std::string &filename) {
  if (filename.empty()) {
    return Status(Status::DRACO_ERROR, "Output parameter is empty.");
  }

  std::string dir_path;
  std::string basename;
  draco::SplitPath(filename, &dir_path, &basename);
  const std::string bin_basename = ReplaceFileExtension(basename, "bin");
  const std::string bin_filename = dir_path + "/" + bin_basename;
  return EncodeFile(geometry, filename, bin_filename, dir_path);
}

template <typename T>
Status GltfEncoder::EncodeFile(const T &geometry, const std::string &filename,
                               const std::string &bin_filename) {
  if (filename.empty()) {
    return Status(Status::DRACO_ERROR, "Output parameter is empty.");
  }

  std::string dir_path;
  std::string basename;
  draco::SplitPath(filename, &dir_path, &basename);
  return EncodeFile(geometry, filename, bin_filename, dir_path);
}

template <typename T>
Status GltfEncoder::EncodeFile(const T &geometry, const std::string &filename,
                               const std::string &bin_filename,
                               const std::string &resource_dir) {
  if (filename.empty() || bin_filename.empty() || resource_dir.empty()) {
    return Status(Status::DRACO_ERROR, "Output parameter is empty.");
  }
  const std::string extension = LowercaseFileExtension(filename);
  if (extension != "gltf" && extension != "glb") {
    return Status(Status::DRACO_ERROR,
                  "gltf_encoder only supports .gltf or .glb output.");
  }

  GltfAsset gltf_asset;
  gltf_asset.set_copyright(copyright_);
  gltf_asset.set_output_type(output_type_);

  if (extension == "gltf") {
    std::string bin_path;
    std::string bin_basename;
    draco::SplitPath(bin_filename, &bin_path, &bin_basename);
    gltf_asset.buffer_name(bin_basename);
  } else {
    gltf_asset.buffer_name("");
    gltf_asset.set_add_images_to_buffer(true);
  }

  // Encode the geometry into a buffer.
  EncoderBuffer buffer;
  DRACO_RETURN_IF_ERROR(EncodeToBuffer(geometry, &gltf_asset, &buffer));
  if (extension == "glb") {
    return WriteGlbFile(gltf_asset, buffer, filename);
  }
  return WriteGltfFiles(gltf_asset, buffer, filename, bin_filename,
                        resource_dir);
}

template <typename T>
Status GltfEncoder::EncodeToBuffer(const T &geometry,
                                   EncoderBuffer *out_buffer) {
  GltfAsset gltf_asset;
  gltf_asset.set_output_type(output_type_);
  gltf_asset.buffer_name("");
  gltf_asset.set_add_images_to_buffer(true);
  gltf_asset.set_copyright(copyright_);

  // Encode the geometry into a buffer.
  EncoderBuffer buffer;
  DRACO_RETURN_IF_ERROR(EncodeToBuffer(geometry, &gltf_asset, &buffer));

  // Define a function for concatenating GLB file chunks into a single buffer.
  const auto encode_chunk_to_buffer =
      [&out_buffer](const EncoderBuffer &chunk) -> Status {
    if (!out_buffer->Encode(chunk.data(), chunk.size())) {
      return Status(Status::DRACO_ERROR, "Error writing to buffer.");
    }
    return OkStatus();
  };

  // Create GLB file chunks and concatenate them to a single buffer.
  return ProcessGlbFileChunks(gltf_asset, buffer, encode_chunk_to_buffer);
}

// Explicit instantiation for Mesh and Scene.
template bool GltfEncoder::EncodeToFile<Mesh>(const Mesh &geometry,
                                              const std::string &file_name,
                                              const std::string &base_dir);
template bool GltfEncoder::EncodeToFile<Scene>(const Scene &geometry,
                                               const std::string &file_name,
                                               const std::string &base_dir);
template Status GltfEncoder::EncodeFile<Mesh>(const Mesh &geometry,
                                              const std::string &filename);
template Status GltfEncoder::EncodeFile<Scene>(const Scene &geometry,
                                               const std::string &filename);
template Status GltfEncoder::EncodeFile<Mesh>(const Mesh &geometry,
                                              const std::string &filename,
                                              const std::string &bin_filename);
template Status GltfEncoder::EncodeFile<Scene>(const Scene &geometry,
                                               const std::string &filename,
                                               const std::string &bin_filename);
template Status GltfEncoder::EncodeFile<Mesh>(const Mesh &geometry,
                                              const std::string &filename,
                                              const std::string &bin_filename,
                                              const std::string &resource_dir);
template Status GltfEncoder::EncodeFile<Scene>(const Scene &geometry,
                                               const std::string &filename,
                                               const std::string &bin_filename,
                                               const std::string &resource_dir);
template Status GltfEncoder::EncodeToBuffer<Mesh>(const Mesh &geometry,
                                                  EncoderBuffer *out_buffer);
template Status GltfEncoder::EncodeToBuffer<Scene>(const Scene &geometry,
                                                   EncoderBuffer *out_buffer);

Status GltfEncoder::EncodeToBuffer(const Mesh &mesh, GltfAsset *gltf_asset,
                                   EncoderBuffer *out_buffer) {
  out_buffer_ = out_buffer;
  SetJsonWriterMode(gltf_asset);
  if (!gltf_asset->AddDracoMesh(mesh)) {
    return Status(Status::DRACO_ERROR, "Error adding Draco mesh.");
  }
  return gltf_asset->Output(out_buffer);
}

Status GltfEncoder::EncodeToBuffer(const Scene &scene, GltfAsset *gltf_asset,
                                   EncoderBuffer *out_buffer) {
  out_buffer_ = out_buffer;
  SetJsonWriterMode(gltf_asset);
  DRACO_RETURN_IF_ERROR(gltf_asset->AddScene(scene));
  return gltf_asset->Output(out_buffer);
}

void GltfEncoder::SetJsonWriterMode(class GltfAsset *gltf_asset) {
  if (gltf_asset->output_type() == COMPACT &&
      gltf_asset->add_images_to_buffer()) {
    gltf_asset->set_json_output_mode(JsonWriter::COMPACT);
  } else {
    gltf_asset->set_json_output_mode(JsonWriter::READABLE);
  }
}

Status GltfEncoder::WriteGltfFiles(const GltfAsset &gltf_asset,
                                   const EncoderBuffer &buffer,
                                   const std::string &filename,
                                   const std::string &bin_filename,
                                   const std::string &resource_dir) {
  std::unique_ptr<FileWriterInterface> file =
      FileWriterFactory::OpenWriter(filename);
  if (!file) {
    return Status(Status::DRACO_ERROR, "Output glTF file could not be opened.");
  }
  std::unique_ptr<FileWriterInterface> bin_file =
      FileWriterFactory::OpenWriter(bin_filename);
  if (!bin_file) {
    return Status(Status::DRACO_ERROR,
                  "Output glTF bin file could not be opened.");
  }

  // Write the glTF data into the file.
  if (!file->Write(buffer.data(), buffer.size())) {
    return Status(Status::DRACO_ERROR, "Error writing to glTF file.");
  }

  // Write the glTF buffer into the file.
  if (!bin_file->Write(gltf_asset.Buffer()->data(),
                       gltf_asset.Buffer()->size())) {
    return Status(Status::DRACO_ERROR, "Error writing to glTF bin file.");
  }

  for (int i = 0; i < gltf_asset.NumImages(); ++i) {
    const std::string name = resource_dir + "/" + gltf_asset.image_name(i);
    const GltfImage *const image = gltf_asset.GetImage(i);
    if (!image) {
      return Status(Status::DRACO_ERROR, "Error getting glTF image.");
    }
    DRACO_RETURN_IF_ERROR(WriteTextureToFile(name, *image->texture));
  }
  return OkStatus();
}

Status GltfEncoder::WriteGlbFile(const GltfAsset &gltf_asset,
                                 const EncoderBuffer &json_data,
                                 const std::string &filename) {
  std::unique_ptr<FileWriterInterface> file =
      FileWriterFactory::OpenWriter(filename);
  if (!file) {
    return Status(Status::DRACO_ERROR, "Output glb file could not be opened.");
  }

  // Define a function for writing GLB file chunks to |file|.
  const auto write_chunk_to_file =
      [&file](const EncoderBuffer &chunk) -> Status {
    if (!file->Write(chunk.data(), chunk.size())) {
      return Status(Status::DRACO_ERROR, "Error writing to glb file.");
    }
    return OkStatus();
  };

  // Create GLB file chunks and write them to file.
  return ProcessGlbFileChunks(gltf_asset, json_data, write_chunk_to_file);
}

Status GltfEncoder::ProcessGlbFileChunks(
    const class GltfAsset &gltf_asset, const EncoderBuffer &json_data,
    const std::function<Status(const EncoderBuffer &)> &process_chunk) const {
  // The json data must be padded so the next chunk starts on a 4-byte boundary.
  const uint32_t json_pad_length =
      (json_data.size() % 4) ? 4 - json_data.size() % 4 : 0;
  const uint32_t json_length = json_data.size() + json_pad_length;
  const uint32_t total_length =
      12 + 8 + json_length + 8 + gltf_asset.Buffer()->size();

  EncoderBuffer header;
  // Write the glb file header.
  const uint32_t gltf_version = 2;
  if (!header.Encode("glTF", 4)) {
    return Status(Status::DRACO_ERROR, "Error writing to glb file.");
  }
  if (!header.Encode(gltf_version)) {
    return Status(Status::DRACO_ERROR, "Error writing to glb file.");
  }
  if (!header.Encode(total_length)) {
    return Status(Status::DRACO_ERROR, "Error writing to glb file.");
  }

  // Write the JSON chunk.
  const uint32_t json_chunk_type = 0x4E4F534A;
  if (!header.Encode(json_length)) {
    return Status(Status::DRACO_ERROR, "Error writing to glb file.");
  }
  if (!header.Encode(json_chunk_type)) {
    return Status(Status::DRACO_ERROR, "Error writing to glb file.");
  }
  DRACO_RETURN_IF_ERROR(process_chunk(header));
  DRACO_RETURN_IF_ERROR(process_chunk(json_data));

  // Pad the data if needed.
  header.Clear();
  if (json_pad_length > 0) {
    if (!header.Encode("   ", json_pad_length)) {
      return Status(Status::DRACO_ERROR, "Error writing to glb file.");
    }
  }

  // Write the binary buffer chunk.
  const uint32_t bin_chunk_type = 0x004E4942;
  const uint32_t gltf_bin_size = gltf_asset.Buffer()->size();
  if (!header.Encode(gltf_bin_size)) {
    return Status(Status::DRACO_ERROR, "Error writing to glb file.");
  }
  if (!header.Encode(bin_chunk_type)) {
    return Status(Status::DRACO_ERROR, "Error writing to glb file.");
  }
  DRACO_RETURN_IF_ERROR(process_chunk(header));
  DRACO_RETURN_IF_ERROR(process_chunk(*gltf_asset.Buffer()));
  return OkStatus();
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
