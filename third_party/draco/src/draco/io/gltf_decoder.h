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
#ifndef DRACO_IO_GLTF_DECODER_H_
#define DRACO_IO_GLTF_DECODER_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "draco/core/decoder_buffer.h"
#include "draco/core/status.h"
#include "draco/core/status_or.h"
#include "draco/io/tiny_gltf_utils.h"
#include "draco/mesh/mesh.h"
#include "draco/mesh/triangle_soup_mesh_builder.h"
#include "draco/point_cloud/point_cloud_builder.h"
#include "draco/scene/scene.h"

namespace draco {

// Decodes a glTF file and returns a draco::Mesh. All of the |mesh|'s attributes
// will be merged into one draco::Mesh
class GltfDecoder {
 public:
  GltfDecoder();

  // Decodes a glTF file stored in the input |file_name| or |buffer| to a Mesh.
  // The second form returns a vector of files used as input to the mesh during
  // the decoding process. Returns nullptr when decode fails.
  StatusOr<std::unique_ptr<Mesh>> DecodeFromFile(const std::string &file_name);
  StatusOr<std::unique_ptr<Mesh>> DecodeFromFile(
      const std::string &file_name, std::vector<std::string> *mesh_files);
  StatusOr<std::unique_ptr<Mesh>> DecodeFromBuffer(DecoderBuffer *buffer);

  // Decodes a glTF file stored in the input |file_name| or |buffer| to a Scene.
  // The second form returns a vector of files used as input to the scene during
  // the decoding process. Returns nullptr if the decode fails.
  StatusOr<std::unique_ptr<Scene>> DecodeFromFileToScene(
      const std::string &file_name);
  StatusOr<std::unique_ptr<Scene>> DecodeFromFileToScene(
      const std::string &file_name, std::vector<std::string> *scene_files);
  StatusOr<std::unique_ptr<Scene>> DecodeFromBufferToScene(
      DecoderBuffer *buffer);

  // Scene graph can be loaded either as a tree or a general directed acyclic
  // graph (DAG) that allows multiple parent nodes. By default. we decode the
  // scene graph as a tree. If the tree mode is selected and the input contains
  // nodes with multiple parents, these nodes are duplicated to form a tree.
  // TODO(ostava): Add support for DAG mode to other parts of the Draco
  // library.
  enum class GltfSceneGraphMode { TREE, DAG };
  void SetSceneGraphMode(GltfSceneGraphMode mode) {
    gltf_scene_graph_mode_ = mode;
  }

  // By default, the decoder will attempt to deduplicate vertices after decoding
  // the mesh. This means lower memory usage and smaller output glTFs after
  // reencoding. However, for very large meshes, this may become an expensive
  // operation. If that becomes an issue, you might want to consider disabling
  // deduplication with |SetDeduplicateVertices(false)|.
  //
  // Note that at this moment, disabling deduplication works ONLY for point
  // clouds.
  void SetDeduplicateVertices(bool deduplicate_vertices) {
    deduplicate_vertices_ = deduplicate_vertices;
  }

 private:
  // Loads |file_name| into |gltf_model_|. Fills |input_files| with paths to all
  // input files when non-null.
  Status LoadFile(const std::string &file_name,
                  std::vector<std::string> *input_files);

  // Loads |gltf_model_| from |buffer| in GLB format.
  Status LoadBuffer(const DecoderBuffer &buffer);

  // Builds mesh from |gltf_model_|.
  StatusOr<std::unique_ptr<Mesh>> BuildMesh();

  // Checks |gltf_model_| for unsupported features. If |gltf_model_| contains
  // unsupported features then the function will return with a status code of
  // UNSUPPORTED_FEATURE.
  Status CheckUnsupportedFeatures();

  // Decodes a glTF Node as well as any child Nodes. If |node| contains a mesh
  // it will process all of the mesh's primitives.
  Status DecodeNode(int node_index, const Eigen::Matrix4d &parent_matrix);

  // Decodes the number of entries in the first attribute of a given glTF
  // |primitive|. Note that all attributes have the same entry count according
  // to glTF 2.0 spec.
  StatusOr<int> DecodePrimitiveAttributeCount(
      const tinygltf::Primitive &primitive) const;

  // Decodes the number of indices in a given glTF |primitive|. If primitive's
  // indices property is not defined, the index count is implied from the entry
  // count of a primitive attribute.
  StatusOr<int> DecodePrimitiveIndicesCount(
      const tinygltf::Primitive &primitive) const;

  // Decodes indices property of a given glTF |primitive|. If primitive's
  // indices property is not defined, the indices are generated based on entry
  // count of a primitive attribute.
  StatusOr<std::vector<uint32_t>> DecodePrimitiveIndices(
      const tinygltf::Primitive &primitive) const;

  // Decodes a glTF Primitive. All of the |primitive|'s attributes will be
  // merged into the draco::Mesh output if they are of the same type that
  // already has been decoded.
  Status DecodePrimitive(const tinygltf::Primitive &primitive,
                         const Eigen::Matrix4d &transform_matrix);

  // Sums the number of elements per attribute for |node|'s mesh and any of
  // |node|'s children. Fills out the material index map.
  Status NodeGatherAttributeAndMaterialStats(const tinygltf::Node &node);

  // Sums the number of elements per attribute for all of the meshes and
  // primitives.
  Status GatherAttributeAndMaterialStats();

  // Sums the attribute counts into total_attribute_counts_.
  void SumAttributeStats(const std::string &attribute_name, int count);

  // Checks that all the same glTF attribute types in different meshes and
  // primitives contain the same characteristics.
  Status CheckTypes(const std::string &attribute_name, int component_type,
                    int type, bool normalized);

  // Accumulates the number of elements per attribute for |primitive|.
  Status AccumulatePrimitiveStats(const tinygltf::Primitive &primitive);

  // Adds all of the attributes from the glTF file to a Draco mesh.
  // GatherAttributeAndMaterialStats() must be called before this function. The
  // GeometryAttribute::MATERIAL attribute will be created only if the glTF file
  // contains more than one material.
  template <typename BuilderT>
  Status AddAttributesToDracoMesh(BuilderT *builder);

  // Copies attribute data from |accessor| and adds it to a Draco mesh using the
  // geometry builder |builder|.
  template <typename BuilderT>
  Status AddAttributeValuesToBuilder(const std::string &attribute_name,
                                     const tinygltf::Accessor &accessor,
                                     const std::vector<uint32_t> &indices_data,
                                     int att_id, int number_of_elements,
                                     const Eigen::Matrix4d &transform_matrix,
                                     BuilderT *builder);

  // Copies the tangent attribute data from |accessor| and adds it to a Draco
  // mesh. This function will transform all of the data by |transform_matrix|
  // and then normalize before adding the data to the Draco mesh.
  // |indices_data| is the indices data from the glTF file. |att_id| is the
  // attribute id of the tangent attribute in the Draco mesh.
  // |number_of_elements| is the number of faces or points this function will
  // process. |reverse_winding| if set will change the orientation of the data.
  template <typename BuilderT>
  Status AddTangentToBuilder(const tinygltf::Accessor &accessor,
                             const std::vector<uint32_t> &indices_data,
                             int att_id, int number_of_elements,
                             const Eigen::Matrix4d &transform_matrix,
                             bool reverse_winding, BuilderT *builder);

  // Copies the texture coordinate attribute data from |accessor| and adds it to
  // a Draco mesh. This function will flip the data on the horizontal axis as
  // Draco meshes store the texture coordinates differently than glTF.
  // |indices_data| is the indices data from the glTF file. |att_id| is the
  // attribute id of the texture coordinate attribute in the Draco mesh.
  // |number_of_elements| is the number of faces or points this function will
  // process. |reverse_winding| if set will change the orientation of the data.
  template <typename BuilderT>
  Status AddTexCoordToBuilder(const tinygltf::Accessor &accessor,
                              const std::vector<uint32_t> &indices_data,
                              int att_id, int number_of_elements,
                              bool reverse_winding, BuilderT *builder);

  // Copies the mesh feature ID attribute data from |accessor| and adds it to a
  // Draco mesh. |indices_data| is the indices data from the glTF file. |att_id|
  // is the attribute ID of the mesh feature ID attribute in the Draco mesh.
  // |number_of_elements| is the number of faces or points this function will
  // process. |reverse_winding| if set will change the orientation of the data.
  template <typename BuilderT>
  Status AddFeatureIdToBuilder(const tinygltf::Accessor &accessor,
                               const std::vector<uint32_t> &indices_data,
                               int att_id, int number_of_elements,
                               bool reverse_winding,
                               const std::string &attribute_name,
                               BuilderT *builder);

  // Copies the property attribute data from |accessor| and adds it to a
  // Draco mesh. |indices_data| is the indices data from the glTF file. |att_id|
  // is the attribute ID of the mesh feature ID attribute in the Draco mesh.
  // |number_of_elements| is the number of faces or points this function will
  // process. |reverse_winding| if set will change the orientation of the data.
  template <typename BuilderT>
  Status AddPropertyAttributeToBuilder(
      const tinygltf::Accessor &accessor,
      const std::vector<uint32_t> &indices_data, int att_id,
      int number_of_elements, bool reverse_winding,
      const std::string &attribute_name, BuilderT *builder);

  // Copies the attribute data from |accessor| and adds it to a Draco mesh.
  // This function will transform all of the data by |transform_matrix| before
  // adding the data to the Draco mesh. |indices_data| is the indices data
  // from the glTF file. |att_id| is the attribute id of the attribute in the
  // Draco mesh. |number_of_elements| is the number of faces or points this
  // function will process. |normalize| if set will normalize all of the vector
  // data after transformation. |reverse_winding| if set will change the
  // orientation of the data.
  template <typename BuilderT>
  Status AddTransformedDataToBuilder(const tinygltf::Accessor &accessor,
                                     const std::vector<uint32_t> &indices_data,
                                     int att_id, int number_of_elements,
                                     const Eigen::Matrix4d &transform_matrix,
                                     bool normalize, bool reverse_winding,
                                     BuilderT *builder);

  // Sets values in |data| into the builder |builder| for |att_id|.
  template <typename T>
  void SetValuesForBuilder(const std::vector<uint32_t> &indices_data,
                           int att_id, int number_of_elements,
                           const std::vector<T> &data, bool reverse_winding,
                           TriangleSoupMeshBuilder *builder);
  template <typename T>
  void SetValuesForBuilder(const std::vector<uint32_t> &indices_data,
                           int att_id, int number_of_elements,
                           const std::vector<T> &data, bool reverse_winding,
                           PointCloudBuilder *builder);

  // Sets colors of for all vertices to white.
  template <typename BuilderT>
  void SetWhiteVertexColor(int color_att_id, draco::DataType type,
                           BuilderT *builder);
  template <typename ComponentT>
  void SetWhiteVertexColorOfType(int color_att_id,
                                 TriangleSoupMeshBuilder *builder);
  template <typename ComponentT>
  void SetWhiteVertexColorOfType(int color_att_id, PointCloudBuilder *builder);

  // Sets values in |data| into the mesh builder |mb| for |att_id|.
  // |reverse_winding| if set will change the orientation of the data.
  template <typename T>
  void SetValuesPerFace(const std::vector<uint32_t> &indices_data, int att_id,
                        int number_of_faces, const std::vector<T> &data,
                        bool reverse_winding, TriangleSoupMeshBuilder *mb);

  // Returns an address pointing to the content stored in |data|. This is used
  // when passing values to mesh / point cloud builder when the input type can
  // be either a VectorD or an arithmetic type.
  template <typename T>
  const void *GetDataContentAddress(const T &data) const;

  // Adds the attribute data in |accessor| to |mb| for unique attribute
  // |att_id|. |indices_data| is the mesh's indices data. |reverse_winding| if
  // set will change the orientation of the data.
  template <typename BuilderT>
  Status AddAttributeDataByTypes(const tinygltf::Accessor &accessor,
                                 const std::vector<uint32_t> &indices_data,
                                 int att_id, int number_of_elements,
                                 bool reverse_winding, BuilderT *builder);

  // Adds the textures to |owner|.
  template <typename T>
  Status CopyTextures(T *owner);

  // Sets extra attribute properties on a constructed draco mesh.
  void SetAttributePropertiesOnDracoMesh(Mesh *mesh);

  // Adds the materials to |mesh|.
  Status AddMaterialsToDracoMesh(Mesh *mesh);

  // Adds the material data for the GeometryAttribute::MATERIAL attribute to the
  // Draco mesh.
  template <typename BuilderT>
  Status AddMaterialDataToBuilder(int material_value, int number_of_elements,
                                  BuilderT *builder);
  template <typename T>
  Status AddMaterialDataToBuilderInternal(T material_value, int number_of_faces,
                                          TriangleSoupMeshBuilder *builder);
  template <typename T>
  Status AddMaterialDataToBuilderInternal(T material_value,
                                          int number_of_points,
                                          PointCloudBuilder *builder);

  // Checks if the glTF file contains a texture. If there is a texture, this
  // function will read the texture data and add it to the Draco |material|. If
  // there is no texture, this function will return OkStatus(). |texture_info|
  // is the data structure containing information about the texture in the glTF
  // file. |type| is the type of texture defined by Draco. This is not the same
  // as the texture coordinate attribute id.
  Status CheckAndAddTextureToDracoMaterial(
      int texture_index, int tex_coord_attribute_index,
      const tinygltf::ExtensionMap &tex_info_ext, Material *material,
      TextureMap::Type type);

  // Decode glTF file to scene.
  Status DecodeGltfToScene();

  // Decode glTF lights into a scene.
  Status AddLightsToScene();

  // Decodes glTF materials variants names into a scene.
  Status AddMaterialsVariantsNamesToScene();

  // Decode glTF animations into a scene. All of the glTF nodes must be decoded
  // to the scene before this function is called.
  Status AddAnimationsToScene();

  // Decode glTF node into a Draco scene. |parent_index| is the index of the
  // parent node. If |node| is a root node set |parent_index| to
  // |kInvalidSceneNodeIndex|. All glTF lights must be decoded to the scene
  // before this function is called.
  Status DecodeNodeForScene(int node_index, SceneNodeIndex parent_index);

  // Decode glTF primitive into a Draco scene.
  Status DecodePrimitiveForScene(const tinygltf::Primitive &primitive,
                                 MeshGroup *mesh_group);

  // Decodes glTF materials variants from |extension| and adds it into materials
  // variants |mappings|. Before calling this function, all materials variants
  // names must be decoded by calling AddMaterialsVariantsNamesToScene().
  Status DecodeMaterialsVariantsMappings(
      const tinygltf::Value::Object &extension,
      std::vector<MeshGroup::MaterialsVariantsMapping> *mappings);

  // Decode extensions on all primitives of all scenes, such as mesh features
  // and structural metadata extensions, and add their contents to |mesh|.
  Status AddPrimitiveExtensionsToDracoMesh(Mesh *mesh);
  Status AddPrimitiveExtensionsToDracoMesh(int node_index, Mesh *mesh);
  Status AddPrimitiveExtensionsToDracoMesh(const tinygltf::Primitive &primitive,
                                           TextureLibrary *texture_library,
                                           Mesh *mesh);

  // Decodes glTF structural metadata from glTF model and adds it to |geometry|.
  template <typename GeometryT>
  Status AddStructuralMetadataToGeometry(GeometryT *geometry);

  // Decodes glTF structural metadata schema from |extension| and adds it to
  // |geometry|.
  template <typename GeometryT>
  Status AddStructuralMetadataSchemaToGeometry(
      const tinygltf::Value::Object &extension, GeometryT *geometry);

  // Decodes glTF structural metadata property tables from |extension| and adds
  // them to |geometry|.
  template <typename GeometryT>
  Status AddPropertyTablesToGeometry(const tinygltf::Value::Object &extension,
                                     GeometryT *geometry);

  // Decodes glTF structural metadata property attributes from |extension| and
  // adds them to |geometry|.
  template <typename GeometryT>
  Status AddPropertyAttributesToGeometry(
      const tinygltf::Value::Object &extension, GeometryT *geometry);

  // Decodes glTF mesh feature ID sets from |primitive| and adds them to |mesh|.
  Status DecodeMeshFeatures(const tinygltf::Primitive &primitive,
                            TextureLibrary *texture_library, Mesh *mesh);

  // Decodes glTF structural metadata from |primitive| and adds it to |mesh|.
  Status DecodeStructuralMetadata(const tinygltf::Primitive &primitive,
                                  Mesh *mesh);

  // Decodes glTF mesh feature ID sets from |extension| and adds them to the
  // |mesh_features| vector.
  Status DecodeMeshFeatures(
      const tinygltf::Value::Object &extension, TextureLibrary *texture_library,
      std::vector<std::unique_ptr<MeshFeatures>> *mesh_features);

  // Decodes glTF structural metadata from |extension| of a glTF primitive and
  // adds its property attribute indices to the |property_attributes| vector.
  Status DecodeStructuralMetadata(const tinygltf::Value::Object &extension,
                                  std::vector<int> *property_attributes);

  // Adds an attribute of type |attribute_name| to |builder|. Returns the
  // attribute id.
  template <typename BuilderT>
  StatusOr<int> AddAttribute(const std::string &attribute_name,
                             int component_type, int type, BuilderT *builder);

  // Adds an attribute of |attribute_type| to |builder|. Returns the attribute
  // id.
  template <typename BuilderT>
  StatusOr<int> AddAttribute(GeometryAttribute::Type attribute_type,
                             int component_type, int type, BuilderT *builder);

  // Returns true if the KHR_texture_transform extension is set in |extension|.
  // If the KHR_texture_transform extension is set then the values are returned
  // in |transform|.
  StatusOr<bool> CheckKhrTextureTransform(
      const tinygltf::ExtensionMap &extension, TextureTransform *transform);

  // Adds glTF material |input_material_index| to |output_material|.
  Status AddGltfMaterial(int input_material_index, Material *output_material);

  // Adds unlit property from glTF |input_material| to |output_material|.
  void DecodeMaterialUnlitExtension(const tinygltf::Material &input_material,
                                    Material *output_material);

  // Adds sheen properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialSheenExtension(const tinygltf::Material &input_material,
                                      Material *output_material);

  // Adds transmission from glTF |input_material| to |output_material|.
  Status DecodeMaterialTransmissionExtension(
      const tinygltf::Material &input_material, Material *output_material);

  // Adds clearcoat properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialClearcoatExtension(
      const tinygltf::Material &input_material, Material *output_material);

  // Adds volume properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialVolumeExtension(const tinygltf::Material &input_material,
                                       int input_material_index,
                                       Material *output_material);

  // Adds ior properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialIorExtension(const tinygltf::Material &input_material,
                                    Material *output_material);

  // Adds specular properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialSpecularExtension(
      const tinygltf::Material &input_material, Material *output_material);

  // Decodes a float value with |name| from |object| to |value| and returns true
  // if a well-formed value with such |name| is present.
  static StatusOr<bool> DecodeFloat(const std::string &name,
                                    const tinygltf::Value::Object &object,
                                    float *value);

  // Decodes an integer value with |name| from |object| to |value| and returns
  // true if a well-formed value with such |name| is present.
  static StatusOr<bool> DecodeInt(const std::string &name,
                                  const tinygltf::Value::Object &object,
                                  int *value);

  // Decodes a string value with |name| from |object| to |value| and returns
  // true if a well-formed value with such |name| is present.
  static StatusOr<bool> DecodeString(const std::string &name,
                                     const tinygltf::Value::Object &object,
                                     std::string *value);

  // Decodes data and data target from buffer view index with |name| in |object|
  // to |data| and returns true if a well-formed data is present.
  StatusOr<bool> DecodePropertyTableData(const std::string &name,
                                         const tinygltf::Value::Object &object,
                                         PropertyTable::Property::Data *data);

  // Decodes a 3D vector with |name| from |object| to |value| and returns true
  // if a well-formed vector with such |name| is present.
  static StatusOr<bool> DecodeVector3f(const std::string &name,
                                       const tinygltf::Value::Object &object,
                                       Vector3f *value);

  // Decodes a texture with |name| from |object| and adds it to |material| as a
  // texture map of |type|.
  Status DecodeTexture(const std::string &name, TextureMap::Type type,
                       const tinygltf::Value::Object &object,
                       Material *material);

  // Reads texture with |texture_name| from |container_object| into
  // |texture_info|.
  static Status ParseTextureInfo(
      const std::string &texture_name,
      const tinygltf::Value::Object &container_object,
      tinygltf::TextureInfo *texture_info);

  // Adds the materials to the scene.
  Status AddMaterialsToScene();

  // Adds the skins to the scene.
  Status AddSkinsToScene();

  // Adds various asset metadata to the scene or mesh.
  Status AddAssetMetadata(Scene *scene);
  Status AddAssetMetadata(Mesh *mesh);
  Status AddAssetMetadata(Metadata *metadata);

  // All material and non-material textures (e.g., from EXT_mesh_features) are
  // initially loaded into a texture library inside the the material library.
  // These methods move |non_material_textures| from material texture library
  // |material_tl| to non-material texture library |non_material_tl|.
  static void MoveNonMaterialTextures(Mesh *mesh);
  static void MoveNonMaterialTextures(Scene *scene);
  static void MoveNonMaterialTextures(
      const std::unordered_set<Texture *> &non_material_textures,
      TextureLibrary *material_tl, TextureLibrary *non_material_tl);

  // Builds and returns a mesh constructed from either mesh builder |mb| or
  // point cloud builder |pb|. Mesh builder is used if |use_mesh_builder| is set
  // to true.
  static StatusOr<std::unique_ptr<Mesh>> BuildMeshFromBuilder(
      bool use_mesh_builder, TriangleSoupMeshBuilder *mb, PointCloudBuilder *pb,
      bool deduplicate_vertices);

  // Map of glTF Mesh to Draco scene mesh group.
  std::map<int, MeshGroupIndex> gltf_mesh_to_scene_mesh_group_;

  // Data structure that stores the glTF data.
  tinygltf::Model gltf_model_;

  // Path to the glTF file.
  std::string input_file_name_;

  // Class used to build the Draco mesh.
  TriangleSoupMeshBuilder mb_;
  PointCloudBuilder pb_;

  // Map from the index in a feature ID vertex attribute name like _FEATURE_ID_5
  // to the corresponding attribute index in the current geometry builder.
  std::unordered_map<int, int> feature_id_attribute_indices_;

  // Next face index used when adding attribute data to the Draco mesh.
  int next_face_id_;

  // Next point index used when adding attribute data to the point cloud.
  int next_point_id_;

  // Total number of indices from all the meshes and primitives.
  int total_face_indices_count_;
  int total_point_indices_count_;

  // This is the id of the GeometryAttribute::MATERIAL attribute added to the
  // Draco mesh.
  int material_att_id_;

  // Data used when decoding the entire glTF asset into a single draco::Mesh.
  // The struct tracks the total number of elements across all matching
  // attributes and it ensures all matching attributes are compatible.
  struct MeshAttributeData {
    int component_type = 0;
    int attribute_type = 0;
    bool normalized = false;
    int total_attribute_counts = 0;
  };

  // Map of glTF attribute name to attribute component type.
  std::map<std::string, MeshAttributeData> mesh_attribute_data_;

  // Map of glTF attribute name to Draco mesh attribute id.
  std::map<std::string, int> attribute_name_to_draco_mesh_attribute_id_;

  // Map of glTF material to Draco material index.
  std::map<int, int> gltf_primitive_material_to_draco_material_;

  // Map of glTF material index to transformation scales of primitives.
  std::map<int, std::vector<float>> gltf_primitive_material_to_scales_;

  // Map of glTF image to Draco textures.
  std::map<int, Texture *> gltf_image_to_draco_texture_;

  std::unique_ptr<Scene> scene_;

  // Map of glTF Node to local store order.
  std::map<int, SceneNodeIndex> gltf_node_to_scenenode_index_;

  // Selected mode of the decoded scene graph.
  GltfSceneGraphMode gltf_scene_graph_mode_ = GltfSceneGraphMode::TREE;

  // Whether vertices should be deduplicated after loading.
  bool deduplicate_vertices_ = true;

  // Functionality for deduping primitives on decode.
  struct PrimitiveSignature {
    const tinygltf::Primitive &primitive;
    explicit PrimitiveSignature(const tinygltf::Primitive &primitive)
        : primitive(primitive) {}
    bool operator==(const PrimitiveSignature &signature) const;
    struct Hash {
      size_t operator()(const PrimitiveSignature &signature) const;
    };
  };
  std::unordered_map<PrimitiveSignature, MeshIndex, PrimitiveSignature::Hash>
      gltf_primitive_to_draco_mesh_index_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_IO_GLTF_DECODER_H_
