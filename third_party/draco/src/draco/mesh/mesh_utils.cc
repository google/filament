// Copyright 2019 The Draco Authors.
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
#include "draco/mesh/mesh_utils.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/attributes/attribute_quantization_transform.h"
#include "draco/core/quantization_utils.h"

namespace draco {

void MeshUtils::TransformMesh(const Eigen::Matrix4d &transform, Mesh *mesh) {
  // Transform positions.
  PointAttribute *pos_att =
      mesh->attribute(mesh->GetNamedAttributeId(GeometryAttribute::POSITION));
  for (AttributeValueIndex avi(0); avi < pos_att->size(); ++avi) {
    Vector3f pos_val;
    pos_att->GetValue(avi, &pos_val[0]);
    Eigen::Vector4d transformed_val(pos_val[0], pos_val[1], pos_val[2], 1);
    transformed_val = transform * transformed_val;
    pos_val =
        Vector3f(transformed_val[0], transformed_val[1], transformed_val[2]);
    pos_att->SetAttributeValue(avi, &pos_val[0]);
  }

  // Transform normals and tangents.
  PointAttribute *normal_att = nullptr;
  PointAttribute *tangent_att = nullptr;
  if (mesh->NumNamedAttributes(GeometryAttribute::NORMAL) > 0) {
    normal_att =
        mesh->attribute(mesh->GetNamedAttributeId(GeometryAttribute::NORMAL));
  }
  if (mesh->NumNamedAttributes(GeometryAttribute::TANGENT) > 0) {
    tangent_att =
        mesh->attribute(mesh->GetNamedAttributeId(GeometryAttribute::TANGENT));
  }

  if (normal_att || tangent_att) {
    // Use inverse-transpose matrix to transform normals and tangents.
    Eigen::Matrix3d it_transform = transform.block<3, 3>(0, 0);

    it_transform = it_transform.inverse().transpose();

    if (normal_att) {
      TransformNormalizedAttribute(it_transform, normal_att);
    }
    if (tangent_att) {
      TransformNormalizedAttribute(it_transform, tangent_att);
    }
  }
}

namespace {

// Merges entries from |src_metadata| to |dst_metadata|. Any metadata entries
// with the same names are left unchanged.
void MergeMetadataInternal(const Metadata &src_metadata,
                           Metadata *dst_metadata) {
  const auto &src_entries = src_metadata.entries();
  const auto &dst_entries = dst_metadata->entries();
  for (const auto &it : src_entries) {
    if (dst_entries.find(it.first) != dst_entries.end()) {
      // Source entry already exists in the target metadata.
      continue;
    }
    // Copy over the entry (entries don't store the data type so binary copy
    // is ok).
    dst_metadata->AddEntryBinary(it.first, it.second.data());
  }

  // Merge any sub-metadata.
  const auto &src_sub_metadata = src_metadata.sub_metadatas();
  const auto &dst_sub_metadata = dst_metadata->sub_metadatas();
  for (const auto &it : src_sub_metadata) {
    if (dst_sub_metadata.find(it.first) == dst_sub_metadata.end()) {
      // Source sub-metadata doesn't exists in the target metadata, copy it
      // over.
      std::unique_ptr<Metadata> sub_metadata(new Metadata(*it.second));
      dst_metadata->AddSubMetadata(it.first, std::move(sub_metadata));
      continue;
    }
    // Merge entries on the sub-metadata.
    MergeMetadataInternal(*it.second, dst_metadata->sub_metadata(it.first));
  }
}

}  // namespace

void MeshUtils::MergeMetadata(const Mesh &src_mesh, Mesh *dst_mesh) {
  const auto *src_metadata = src_mesh.GetMetadata();
  if (src_metadata == nullptr) {
    return;  // Nothing to merge.
  }
  if (dst_mesh->GetMetadata() == nullptr) {
    // Create new metadata for the |dst_mesh|. We do not copy the metadata
    // directly because some of the underlying attribute metadata may need to
    // be remapped to the format used by |dst_mesh| (e.g. unique ids of the
    // attributes may have changed or some attributes may be missing on the
    // |dst_mesh|).
    std::unique_ptr<GeometryMetadata> new_metadata(new GeometryMetadata());
    dst_mesh->AddMetadata(std::move(new_metadata));
  }
  auto *dst_metadata = dst_mesh->metadata();

  // First go over all entries of the geometry part of |src_metadata|.
  MergeMetadataInternal(*src_metadata, dst_metadata);

  // Go over attribute metadata. Merges only metadata for attributes that exist
  // both on the source and target meshes. Attribute unique ids are remapped
  // if needed.
  for (int att_type_i = 0;
       att_type_i < GeometryAttribute::NAMED_ATTRIBUTES_COUNT; ++att_type_i) {
    const GeometryAttribute::Type att_type =
        static_cast<GeometryAttribute::Type>(att_type_i);
    // TODO(ostava): Handle case when the number of attributes of a given type
    // does not match.
    if (src_mesh.NumNamedAttributes(att_type) !=
        dst_mesh->NumNamedAttributes(att_type)) {
      continue;
    }
    for (int j = 0; j < src_mesh.NumNamedAttributes(att_type); ++j) {
      // First check if we have a metadata for this attribute.
      const PointAttribute *const src_att =
          src_mesh.GetNamedAttribute(att_type, j);
      const auto *src_metadata =
          src_mesh.GetMetadata()->GetAttributeMetadataByUniqueId(
              src_att->unique_id());
      if (src_metadata == nullptr) {
        // No metadata at the source, ignore the attribute.
        continue;
      }
      // Find target attribute corresponding to the source.
      const PointAttribute *const dst_att =
          dst_mesh->GetNamedAttribute(att_type, j);
      if (dst_att == nullptr) {
        // No corresponding attribute found, ignore the source metadata.
        continue;
      }
      auto *dst_metadata =
          dst_mesh->metadata()->attribute_metadata(dst_att->unique_id());
      if (dst_metadata == nullptr) {
        // Copy over the metadata (with remapped attribute unique id).
        std::unique_ptr<AttributeMetadata> new_metadata(
            new AttributeMetadata(*src_metadata));
        new_metadata->set_att_unique_id(dst_att->unique_id());
        dst_mesh->metadata()->AddAttributeMetadata(std::move(new_metadata));
        continue;
      }
      // Merge metadata entries.
      MergeMetadataInternal(*src_metadata, dst_metadata);
    }
  }
}

// Returns indices of all used materials on the |mesh|.
std::unordered_set<int> FindUsedMaterials(const Mesh &mesh) {
  const PointAttribute *const mat_att =
      mesh.GetNamedAttribute(GeometryAttribute::MATERIAL);
  std::unordered_set<int> used_materials;
  if (mat_att == nullptr) {
    // Only material with index 0 is assumed to be used.
    used_materials.insert(0);
  } else {
    for (AttributeValueIndex avi(0); avi < mat_att->size(); ++avi) {
      uint32_t mat_index = 0;
      mat_att->GetValue(avi, &mat_index);
      used_materials.insert(mat_index);
    }
  }
  return used_materials;
}

Status MeshUtils::RemoveUnusedMeshFeatures(Mesh *mesh) {
  // Unused mesh features are features that are not used by any face / vertex
  // of the |mesh|. Currently, each mesh feature can be "masked" for specific
  // materials, in which case we need to check whether the mask materials
  // are present in the |mesh|. If not, we can remove the mesh features from the
  // mesh.
  const std::unordered_set<int> used_materials = FindUsedMaterials(*mesh);
  std::vector<MeshFeaturesIndex> unused_mesh_features;
  for (MeshFeaturesIndex mfi(0); mfi < mesh->NumMeshFeatures(); ++mfi) {
    bool is_used = false;
    if (mesh->NumMeshFeaturesMaterialMasks(mfi) == 0) {
      is_used = true;
    } else {
      for (int mask_i = 0; mask_i < mesh->NumMeshFeaturesMaterialMasks(mfi);
           ++mask_i) {
        const int material_index =
            mesh->GetMeshFeaturesMaterialMask(mfi, mask_i);
        if (used_materials.count(material_index)) {
          is_used = true;
          break;
        }
      }
    }
    if (!is_used) {
      unused_mesh_features.push_back(mfi);
    }
  }

  // Remove the unused mesh features (from back).
  for (auto it = unused_mesh_features.rbegin();
       it != unused_mesh_features.rend(); ++it) {
    const MeshFeaturesIndex mfi = *it;
    mesh->RemoveMeshFeatures(mfi);
  }

  // Remove all features textures that are not used anymore.

  // First find which textures are referenced by the mesh features.
  std::unordered_set<const Texture *> used_textures;
  for (MeshFeaturesIndex mfi(0); mfi < mesh->NumMeshFeatures(); ++mfi) {
    const Texture *const texture =
        mesh->GetMeshFeatures(mfi).GetTextureMap().texture();
    if (texture) {
      used_textures.insert(texture);
    }
  }

  if (!used_textures.empty() &&
      mesh->GetNonMaterialTextureLibrary().NumTextures() == 0) {
    return ErrorStatus(
        "Trying to remove mesh features textures that are not owned by the "
        "mesh.");
  }

  // Remove all unreferenced textures from the non-material texture library.
  for (int ti = mesh->GetNonMaterialTextureLibrary().NumTextures() - 1; ti >= 0;
       --ti) {
    const Texture *const texture =
        mesh->GetNonMaterialTextureLibrary().GetTexture(ti);
    if (used_textures.count(texture) == 0) {
      mesh->GetNonMaterialTextureLibrary().RemoveTexture(ti);
    }
  }
  return OkStatus();
}

Status MeshUtils::RemoveUnusedPropertyAttributesIndices(Mesh *mesh) {
  // Unused property attributes indices are indices that are not used by any
  // face / vertex of the |mesh|. Currently, each property attributes index can
  // be "masked" for specific materials, in which case we need to check whether
  // the mask materials are present in the |mesh|. If not, we can remove the
  // property attributes from the mesh.
  const std::unordered_set<int> used_materials = FindUsedMaterials(*mesh);
  std::vector<int> unused_property_attributes_indices;
  for (int i = 0; i < mesh->NumPropertyAttributesIndices(); ++i) {
    bool is_used = false;
    if (mesh->NumPropertyAttributesIndexMaterialMasks(i) == 0) {
      is_used = true;
    } else {
      for (int mask_i = 0;
           mask_i < mesh->NumPropertyAttributesIndexMaterialMasks(i);
           ++mask_i) {
        const int material_index =
            mesh->GetPropertyAttributesIndexMaterialMask(i, mask_i);
        if (used_materials.count(material_index)) {
          is_used = true;
          break;
        }
      }
    }
    if (!is_used) {
      unused_property_attributes_indices.push_back(i);
    }
  }

  // Remove the unused property attributes indices (from back).
  for (auto it = unused_property_attributes_indices.rbegin();
       it != unused_property_attributes_indices.rend(); ++it) {
    const int i = *it;
    mesh->RemovePropertyAttributesIndex(i);
  }
  return OkStatus();
}

bool MeshUtils::FlipTextureUvValues(bool flip_u, bool flip_v,
                                    PointAttribute *att) {
  if (att->attribute_type() != GeometryAttribute::TEX_COORD) {
    return false;
  }
  if (att->data_type() != DataType::DT_FLOAT32) {
    return false;
  }
  if (att->num_components() != 2) {
    return false;
  }

  std::array<float, 2> value;
  for (AttributeValueIndex avi(0); avi < att->size(); ++avi) {
    if (!att->GetValue<float, 2>(avi, &value)) {
      return false;
    }
    if (flip_u) {
      value[0] = 1.0 - value[0];
    }
    if (flip_v) {
      value[1] = 1.0 - value[1];
    }
    att->SetAttributeValue(avi, value.data());
  }
  return true;
}

// TODO(fgalligan): Change att_id to be of type const PointAttribute &.
int MeshUtils::CountDegenerateFaces(const Mesh &mesh, int att_id) {
  const PointAttribute *const att = mesh.attribute(att_id);
  if (att == nullptr) {
    return -1;
  }
  const int num_components = att->num_components();
  switch (num_components) {
    case 2:
      return MeshUtils::CountDegenerateFaces<Vector2f>(mesh, *att);
    case 3:
      return MeshUtils::CountDegenerateFaces<Vector3f>(mesh, *att);
    case 4:
      return MeshUtils::CountDegenerateFaces<Vector4f>(mesh, *att);
    default:
      break;
  }
  return -1;
}

StatusOr<int> MeshUtils::FindLowestTextureQuantization(
    const Mesh &mesh, const PointAttribute &pos_att, int pos_quantization_bits,
    const PointAttribute &tex_att, int tex_target_quantization_bits) {
  if (tex_target_quantization_bits < 0 || tex_target_quantization_bits >= 30) {
    return Status(Status::DRACO_ERROR,
                  "Target texture quantization is out of range.");
  }
  // The target quantization is no quantization, so return 0.
  if (tex_target_quantization_bits == 0) {
    return 0;
  }
  const uint32_t pos_max_quantized_value = (1 << (pos_quantization_bits)) - 1;
  AttributeQuantizationTransform pos_transform;
  if (!pos_transform.ComputeParameters(pos_att, pos_quantization_bits)) {
    return Status(Status::DRACO_ERROR,
                  "Failed computing position quantization parameters.");
  }

  // Get all degenerate faces for positions. If the model already has
  // degenerate faces for positions, but valid faces for texture coordinates,
  // those will not count as new degenerate faces for texture coordinates,
  // because the faces would not have been rendered anyway.
  const std::vector<FaceIndex> pos_degenerate_faces_sorted =
      MeshUtils::ListDegenerateQuantizedFaces(
          mesh, pos_att, pos_transform.range(), pos_max_quantized_value, false);

  // Initialize return value to zero signifying that it could not find a
  // quantization that did not cause any new degenerate faces.
  int lowest_quantization_bits = 0;
  int min_quantization_bits = tex_target_quantization_bits;
  int max_quantization_bits = 29;
  while (true) {
    const int curr_quantization_bits =
        min_quantization_bits +
        (max_quantization_bits - min_quantization_bits) / 2;
    AttributeQuantizationTransform transform;
    if (!transform.ComputeParameters(tex_att, curr_quantization_bits)) {
      return Status(Status::DRACO_ERROR,
                    "Failed computing texture quantization parameters.");
    }

    const uint32_t max_quantized_value = (1 << (curr_quantization_bits)) - 1;

    // Get only new degenerate faces for texture coordinates. If the model
    // already has degenerate faces for texture coordinates, we don't want to
    // take into account those faces in the source, because those faces would
    // not have been rendered correctly anyway.
    const std::vector<FaceIndex> tex_degenerate_faces_sorted =
        MeshUtils::ListDegenerateQuantizedFaces(
            mesh, tex_att, transform.range(), max_quantized_value, true);

    if (tex_degenerate_faces_sorted.size() <=
        pos_degenerate_faces_sorted.size()) {
      if (std::includes(pos_degenerate_faces_sorted.begin(),
                        pos_degenerate_faces_sorted.end(),
                        tex_degenerate_faces_sorted.begin(),
                        tex_degenerate_faces_sorted.end())) {
        // Degenerate texture coordinate faces are a subset of position
        // degenerate faces.
        lowest_quantization_bits = curr_quantization_bits;
      }
    }

    if (lowest_quantization_bits == curr_quantization_bits) {
      // The lowest quantization is the current quantization, see if lower
      // quantization is possible.
      max_quantization_bits = curr_quantization_bits - 1;
    } else {
      min_quantization_bits = curr_quantization_bits + 1;
    }
    if (min_quantization_bits > max_quantization_bits) {
      break;
    }
  }
  return lowest_quantization_bits;
}

void MeshUtils::TransformNormalizedAttribute(const Eigen::Matrix3d &transform,
                                             PointAttribute *att) {
  for (AttributeValueIndex avi(0); avi < att->size(); ++avi) {
    // Store up to 4 component values.
    Vector4f val(0, 0, 0, 1);
    att->GetValue(avi, &val);
    // Ignore the last component during transformation.
    Eigen::Vector3d transformed_val(val[0], val[1], val[2]);
    transformed_val = transform * transformed_val;
    transformed_val = transformed_val.normalized();
    // Last component is passed to the transformed value.
    val = Vector4f(transformed_val[0], transformed_val[1], transformed_val[2],
                   val[3]);

    // Set the value to the attribute. Note that in case the attribute is using
    // fewer than 4 components, the 4th component is going to be ignored.
    att->SetAttributeValue(avi, &val[0]);
  }
}

template <typename att_components_t>
int MeshUtils::CountDegenerateFaces(const Mesh &mesh,
                                    const PointAttribute &att) {
  if (att.data_type() != DataType::DT_FLOAT32) {
    return -1;
  }
  std::array<att_components_t, 3> values;
  int degenerate_values = 0;
  for (FaceIndex fi(0); fi < mesh.num_faces(); ++fi) {
    const auto &face = mesh.face(fi);
    for (int c = 0; c < 3; ++c) {
      att.GetMappedValue(face[c], &values[c][0]);
    }
    if (values[0] == values[1] || values[0] == values[2] ||
        values[1] == values[2]) {
      degenerate_values++;
    }
  }
  return degenerate_values;
}

std::vector<FaceIndex> MeshUtils::ListDegenerateQuantizedFaces(
    const Mesh &mesh, const PointAttribute &att, float range,
    uint32_t max_quantized_value, bool quantized_degenerate_only) {
  const int num_components = att.num_components();
  switch (num_components) {
    case 2:
      return MeshUtils::ListDegenerateQuantizedFaces<Vector2f,
                                                     VectorD<int32_t, 2>>(
          mesh, att, range, max_quantized_value, quantized_degenerate_only);
    case 3:
      return MeshUtils::ListDegenerateQuantizedFaces<Vector3f,
                                                     VectorD<int32_t, 3>>(
          mesh, att, range, max_quantized_value, quantized_degenerate_only);
    case 4:
      return MeshUtils::ListDegenerateQuantizedFaces<Vector4f,
                                                     VectorD<int32_t, 4>>(
          mesh, att, range, max_quantized_value, quantized_degenerate_only);
    default:
      break;
  }
  return std::vector<FaceIndex>();
}

template <typename att_components_t, typename quantized_components_t>
std::vector<FaceIndex> MeshUtils::ListDegenerateQuantizedFaces(
    const Mesh &mesh, const PointAttribute &att, float range,
    uint32_t max_quantized_value, bool quantized_degenerate_only) {
  std::array<att_components_t, 3> values;
  std::array<quantized_components_t, 3> quantized_values;

  Quantizer quantizer;
  quantizer.Init(range, max_quantized_value);
  std::vector<FaceIndex> degenerate_faces;

  for (FaceIndex fi(0); fi < mesh.num_faces(); ++fi) {
    const auto &face = mesh.face(fi);
    for (int c = 0; c < 3; ++c) {
      att.GetMappedValue(face[c], &values[c][0]);
      for (int i = 0; i < att_components_t::dimension; ++i) {
        quantized_values[c][i] = quantizer.QuantizeFloat(values[c][i]);
      }
    }

    if (quantized_degenerate_only &&
        (values[0] == values[1] || values[0] == values[2] ||
         values[1] == values[2])) {
      continue;
    }
    if (quantized_values[0] == quantized_values[1] ||
        quantized_values[0] == quantized_values[2] ||
        quantized_values[1] == quantized_values[2]) {
      degenerate_faces.push_back(fi);
    }
  }
  return degenerate_faces;
}

bool MeshUtils::HasAutoGeneratedTangents(const Mesh &mesh) {
  const int tangent_att_id =
      mesh.GetNamedAttributeId(draco::GeometryAttribute::TANGENT);
  if (tangent_att_id == -1) {
    return false;
  }
  const auto metadata = mesh.GetAttributeMetadataByAttributeId(tangent_att_id);
  if (metadata) {
    int is_auto_generated = 0;
    if (metadata->GetEntryInt("auto_generated", &is_auto_generated) &&
        is_auto_generated == 1) {
      return true;
    }
  }
  return false;
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
