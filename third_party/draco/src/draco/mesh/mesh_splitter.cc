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
#include "draco/mesh/mesh_splitter.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "draco/attributes/geometry_attribute.h"
#include "draco/mesh/mesh_utils.h"
#include "draco/mesh/triangle_soup_mesh_builder.h"
#include "draco/point_cloud/point_cloud_builder.h"
#include "draco/texture/texture_map.h"

namespace draco {

// Helper class that handles splitting of meshes with faces / without faces,
// i.e. point clouds.
template <typename BuilderT>
class MeshSplitterInternal {
 public:
  struct WorkData : public MeshSplitter::WorkData {
    // TriangleSoupMeshBuilder or PointCloudBuilder.
    std::vector<BuilderT> builders;
    std::vector<int> *att_id_map;
  };

  // Computes number of elements (faces or points) for each sub-mesh.
  Status InitializeWorkDataNumElements(const Mesh &mesh, int split_attribute_id,
                                       WorkData *work_data) const;
  // Initializes a builder for a given sub-mesh.
  void InitializeBuilder(int b_index, int num_elements, const Mesh &mesh,
                         int ignored_attribute_id, WorkData *work_data) const;
  // Add all faces or points to the builders.
  void AddElementsToBuilder(const Mesh &mesh,
                            const PointAttribute *split_attribute,
                            WorkData *work_data) const;
  // Builds the meshes from the data accumulated in the builders.
  StatusOr<MeshSplitter::MeshVector> BuildMeshes(
      const Mesh &mesh, WorkData *work_data, bool deduplicate_vertices) const;
};

namespace {

// Helper functions for copying single element from source |mesh| to a target
// builder |b_index| stored in |work_data|.
void AddElementToBuilder(
    int b_index, FaceIndex source_i, FaceIndex target_i, const Mesh &mesh,
    MeshSplitterInternal<TriangleSoupMeshBuilder>::WorkData *work_data);
void AddElementToBuilder(
    int b_index, PointIndex source_i, PointIndex target_i, const Mesh &mesh,
    MeshSplitterInternal<PointCloudBuilder>::WorkData *work_data);
}  // namespace

MeshSplitter::MeshSplitter()
    : preserve_materials_(false),
      remove_unused_material_indices_(true),
      preserve_mesh_features_(false),
      preserve_structural_metadata_(false),
      deduplicate_vertices_(true) {}

StatusOr<MeshSplitter::MeshVector> MeshSplitter::SplitMesh(
    const Mesh &mesh, uint32_t split_attribute_id) {
  if (mesh.num_attributes() <= split_attribute_id) {
    return Status(Status::DRACO_ERROR, "Invalid attribute id.");
  }
  if (mesh.num_faces() == 0) {
    return SplitMeshInternal<PointCloudBuilder>(mesh, split_attribute_id);
  } else {
    return SplitMeshInternal<TriangleSoupMeshBuilder>(mesh, split_attribute_id);
  }
}

template <typename BuilderT>
StatusOr<MeshSplitter::MeshVector> MeshSplitter::SplitMeshInternal(
    const Mesh &mesh, int split_attribute_id) {
  const PointAttribute *const split_attribute =
      mesh.attribute(split_attribute_id);

  // Preserve the split attribute only if it is the material attribute and the
  // |preserve_materials_| flag is set. Otherwise, the split attribute will get
  // discarded.
  // TODO(ostava): We may revisit this later and add an option to always
  // preserve the split attribute.
  const bool preserve_split_attribute =
      preserve_materials_ &&
      split_attribute->attribute_type() == GeometryAttribute::MATERIAL;

  const int num_out_meshes = split_attribute->size();
  MeshSplitterInternal<BuilderT> splitter_internal;
  typename MeshSplitterInternal<BuilderT>::WorkData work_data;
  work_data.num_sub_mesh_elements.resize(num_out_meshes, 0);
  work_data.split_by_materials =
      (split_attribute->attribute_type() == GeometryAttribute::MATERIAL);

  DRACO_RETURN_IF_ERROR(splitter_internal.InitializeWorkDataNumElements(
      mesh, split_attribute_id, &work_data));

  // Create the sub-meshes.
  work_data.builders.resize(num_out_meshes);
  // Map between attribute ids of the input and output meshes.
  att_id_map_.resize(mesh.num_attributes(), -1);
  work_data.att_id_map = &att_id_map_;
  const int ignored_att_id =
      (!preserve_split_attribute ? split_attribute_id : -1);
  for (int mi = 0; mi < num_out_meshes; ++mi) {
    if (work_data.num_sub_mesh_elements[mi] == 0) {
      continue;  // Empty mesh, don't initialize it.
    }

    const int num_elements = work_data.num_sub_mesh_elements[mi];
    splitter_internal.InitializeBuilder(mi, num_elements, mesh, ignored_att_id,
                                        &work_data);

    // Reset the element counter for the sub-mesh. It will be used to keep track
    // of number of elements added to the sub-mesh.
    work_data.num_sub_mesh_elements[mi] = 0;
  }

  splitter_internal.AddElementsToBuilder(mesh, split_attribute, &work_data);

  DRACO_ASSIGN_OR_RETURN(
      MeshVector out_meshes,
      splitter_internal.BuildMeshes(mesh, &work_data, deduplicate_vertices_));
  return FinalizeMeshes(mesh, work_data, std::move(out_meshes));
}

template <>
Status
MeshSplitterInternal<TriangleSoupMeshBuilder>::InitializeWorkDataNumElements(
    const Mesh &mesh, int split_attribute_id, WorkData *work_data) const {
  const PointAttribute *const split_attribute =
      mesh.attribute(split_attribute_id);
  // Verify that the attribute values are defined "per-face", i.e., all points
  // on a face are always mapped to the same attribute value.
  for (FaceIndex fi(0); fi < mesh.num_faces(); ++fi) {
    const auto face = mesh.face(fi);
    const AttributeValueIndex avi = split_attribute->mapped_index(face[0]);
    for (int c = 1; c < 3; ++c) {
      if (split_attribute->mapped_index(face[c]) != avi) {
        return Status(Status::DRACO_ERROR,
                      "Attribute values not consistent on a face.");
      }
    }
    work_data->num_sub_mesh_elements[avi.value()] += 1;
  }
  return OkStatus();
}

template <>
Status MeshSplitterInternal<PointCloudBuilder>::InitializeWorkDataNumElements(
    const Mesh &mesh, int split_attribute_id, WorkData *work_data) const {
  const PointAttribute *const split_attribute =
      mesh.attribute(split_attribute_id);
  // Each point can have a different value. Just accumulate the number of points
  // with the same attribute value index.
  for (PointIndex pi(0); pi < mesh.num_points(); ++pi) {
    const AttributeValueIndex avi = split_attribute->mapped_index(pi);
    work_data->num_sub_mesh_elements[avi.value()] += 1;
  }
  return OkStatus();
}

template <typename BuilderT>
void MeshSplitterInternal<BuilderT>::InitializeBuilder(
    int b_index, int num_elements, const Mesh &mesh, int ignored_attribute_id,
    WorkData *work_data) const {
  work_data->builders[b_index].Start(num_elements);

  // Add all attributes.
  for (int ai = 0; ai < mesh.num_attributes(); ++ai) {
    if (ai == ignored_attribute_id) {
      continue;
    }
    const GeometryAttribute *const src_att = mesh.attribute(ai);
    (*work_data->att_id_map)[ai] = work_data->builders[b_index].AddAttribute(
        src_att->attribute_type(), src_att->num_components(),
        src_att->data_type(), src_att->normalized());
    work_data->builders[b_index].SetAttributeName(work_data->att_id_map->at(ai),
                                                  src_att->name());
  }
}

template <>
void MeshSplitterInternal<TriangleSoupMeshBuilder>::AddElementsToBuilder(
    const Mesh &mesh, const PointAttribute *split_attribute,
    WorkData *work_data) const {
  // Go over all faces of the input mesh and add them to the appropriate
  // sub-mesh.
  for (FaceIndex fi(0); fi < mesh.num_faces(); ++fi) {
    const auto face = mesh.face(fi);
    const int sub_mesh_id = split_attribute->mapped_index(face[0]).value();
    const FaceIndex target_fi(work_data->num_sub_mesh_elements[sub_mesh_id]++);
    AddElementToBuilder(sub_mesh_id, fi, target_fi, mesh, work_data);
  }
}

template <>
void MeshSplitterInternal<PointCloudBuilder>::AddElementsToBuilder(
    const Mesh &mesh, const PointAttribute *split_attribute,
    WorkData *work_data) const {
  // Go over all points of the input mesh and add them to the appropriate
  // sub-mesh.
  for (PointIndex pi(0); pi < mesh.num_points(); ++pi) {
    const int sub_mesh_id = split_attribute->mapped_index(pi).value();
    const PointIndex target_pi(work_data->num_sub_mesh_elements[sub_mesh_id]++);
    AddElementToBuilder(sub_mesh_id, pi, target_pi, mesh, work_data);
  }
}

namespace {

void AddElementToBuilder(
    int b_index, FaceIndex source_i, FaceIndex target_i, const Mesh &mesh,
    MeshSplitterInternal<TriangleSoupMeshBuilder>::WorkData *work_data) {
  const auto &face = mesh.face(source_i);
  for (int ai = 0; ai < mesh.num_attributes(); ++ai) {
    const PointAttribute *const src_att = mesh.attribute(ai);
    const int target_att_id = work_data->att_id_map->at(ai);
    if (target_att_id == -1) {
      continue;
    }
    // Add value for each corner of the face.
    work_data->builders[b_index].SetAttributeValuesForFace(
        target_att_id, target_i, src_att->GetAddressOfMappedIndex(face[0]),
        src_att->GetAddressOfMappedIndex(face[1]),
        src_att->GetAddressOfMappedIndex(face[2]));
  }
}

void AddElementToBuilder(
    int b_index, PointIndex source_i, PointIndex target_i, const Mesh &mesh,
    MeshSplitterInternal<PointCloudBuilder>::WorkData *work_data) {
  for (int ai = 0; ai < mesh.num_attributes(); ++ai) {
    const PointAttribute *const src_att = mesh.attribute(ai);
    const int target_att_id = work_data->att_id_map->at(ai);
    if (target_att_id == -1) {
      continue;
    }
    // Add value for the point |target_i|.
    work_data->builders[b_index].SetAttributeValueForPoint(
        target_att_id, target_i, src_att->GetAddressOfMappedIndex(source_i));
  }
}

}  // namespace

template <>
StatusOr<MeshSplitter::MeshVector>
MeshSplitterInternal<TriangleSoupMeshBuilder>::BuildMeshes(
    const Mesh &mesh, WorkData *work_data, bool deduplicate_vertices) const {
  const int num_out_meshes = work_data->builders.size();
  MeshSplitter::MeshVector out_meshes(num_out_meshes);
  for (int mi = 0; mi < num_out_meshes; ++mi) {
    if (work_data->num_sub_mesh_elements[mi] == 0) {
      continue;
    }
    out_meshes[mi] = work_data->builders[mi].Finalize();
    if (out_meshes[mi] == nullptr) {
      continue;
    }
  }
  return out_meshes;
}

template <>
StatusOr<MeshSplitter::MeshVector>
MeshSplitterInternal<PointCloudBuilder>::BuildMeshes(
    const Mesh &mesh, WorkData *work_data, bool deduplicate_vertices) const {
  const int num_out_meshes = work_data->builders.size();
  MeshSplitter::MeshVector out_meshes(num_out_meshes);
  for (int mi = 0; mi < num_out_meshes; ++mi) {
    if (work_data->num_sub_mesh_elements[mi] == 0) {
      continue;
    }
    // For point clouds, we first build a point cloud and copy it over into
    // a draco::Mesh.
    std::unique_ptr<PointCloud> pc =
        work_data->builders[mi].Finalize(deduplicate_vertices);
    if (pc == nullptr) {
      continue;
    }
    std::unique_ptr<Mesh> mesh(new Mesh());
    PointCloud *mesh_pc = mesh.get();
    mesh_pc->Copy(*pc);
    out_meshes[mi] = std::move(mesh);
  }
  return out_meshes;
}

StatusOr<MeshSplitter::MeshVector> MeshSplitter::FinalizeMeshes(
    const Mesh &mesh, const WorkData &work_data, MeshVector out_meshes) const {
  // Finalize meshes.
  const int num_out_meshes = out_meshes.size();

  // If we are going to preserve mesh features, we will need to update texture
  // pointers for all mesh feature textures. Here we store the mapping between
  // the old texture pointers and their indices.
  std::unordered_map<const Texture *, int> features_texture_to_index_map;
  if (preserve_mesh_features_) {
    features_texture_to_index_map =
        mesh.GetNonMaterialTextureLibrary().ComputeTextureToIndexMap();
  }

  for (int mi = 0; mi < num_out_meshes; ++mi) {
    if (out_meshes[mi] == nullptr) {
      continue;
    }
    out_meshes[mi]->SetName(mesh.GetName());
    if (preserve_materials_) {
      if (work_data.split_by_materials) {
        // When splitting by material, only copy the material in use.
        if (out_meshes[mi]->num_points() != 0 &&
            mesh.GetMaterialLibrary().NumMaterials() != 0) {
          uint64_t material_index = 0;
          out_meshes[mi]
              ->GetNamedAttribute(GeometryAttribute::MATERIAL)
              ->GetMappedValue(PointIndex(0), &material_index);

          // Populate empty materials and textures. Unused materials and
          // textures will be cleared later.
          out_meshes[mi]->GetMaterialLibrary().MutableMaterial(
              mesh.GetMaterialLibrary().NumMaterials() - 1);
          for (int i = 0;
               i < mesh.GetMaterialLibrary().GetTextureLibrary().NumTextures();
               ++i) {
            out_meshes[mi]
                ->GetMaterialLibrary()
                .MutableTextureLibrary()
                .PushTexture(std::make_unique<Texture>());
          }

          // Copy the material that we're actually going to use.
          out_meshes[mi]
              ->GetMaterialLibrary()
              .MutableMaterial(material_index)
              ->Copy(*mesh.GetMaterialLibrary().GetMaterial(material_index));
          std::unordered_map<const Texture *, int> texture_to_index =
              mesh.GetMaterialLibrary()
                  .GetTextureLibrary()
                  .ComputeTextureToIndexMap();
          for (int tmi = 0; tmi < mesh.GetMaterialLibrary()
                                      .GetMaterial(material_index)
                                      ->NumTextureMaps();
               ++tmi) {
            const TextureMap *const source_texture_map =
                mesh.GetMaterialLibrary()
                    .GetMaterial(material_index)
                    ->GetTextureMapByIndex(tmi);

            // Get the texture map index
            const int texture_index =
                texture_to_index[source_texture_map->texture()];

            // Use the index to assign texture to the corresponding texture map
            // on the split mesh.
            TextureMap *new_texture_map = out_meshes[mi]
                                              ->GetMaterialLibrary()
                                              .MutableMaterial(material_index)
                                              ->GetTextureMapByIndex(tmi);
            new_texture_map->SetTexture(out_meshes[mi]
                                            ->GetMaterialLibrary()
                                            .MutableTextureLibrary()
                                            .GetTexture(texture_index));
            new_texture_map->texture()->Copy(*source_texture_map->texture());
          }
        }
      } else {
        out_meshes[mi]->GetMaterialLibrary().Copy(mesh.GetMaterialLibrary());
      }
    }

    // Copy metadata of the original mesh to the output meshes.
    if (mesh.GetMetadata() != nullptr) {
      const GeometryMetadata &metadata = *mesh.GetMetadata();
      out_meshes[mi]->AddMetadata(
          std::unique_ptr<GeometryMetadata>(new GeometryMetadata(metadata)));
    }

    // Copy over attribute unique ids.
    for (int att_id = 0; att_id < mesh.num_attributes(); ++att_id) {
      const int mapped_att_id = att_id_map_[att_id];
      if (mapped_att_id == -1) {
        continue;
      }
      const PointAttribute *const src_att = mesh.attribute(att_id);
      PointAttribute *const dst_att = out_meshes[mi]->attribute(mapped_att_id);
      dst_att->set_unique_id(src_att->unique_id());
    }

    // Copy compression settings of the original mesh to the output meshes.
    out_meshes[mi]->SetCompressionEnabled(mesh.IsCompressionEnabled());
    out_meshes[mi]->SetCompressionOptions(mesh.GetCompressionOptions());

    if (preserve_mesh_features_) {
      // Copy mesh features from the source |mesh| to the |out_meshes[mi]|.
      for (MeshFeaturesIndex mfi(0); mfi < mesh.NumMeshFeatures(); ++mfi) {
        if (work_data.split_by_materials) {
          // Copy over only those mesh features that were masked to the material
          // corresponding to |mi|.
          bool is_used = false;
          if (mesh.NumMeshFeaturesMaterialMasks(mfi) == 0) {
            is_used = true;
          } else {
            for (int mask_index = 0;
                 mask_index < mesh.NumMeshFeaturesMaterialMasks(mfi);
                 ++mask_index) {
              if (mesh.GetMeshFeaturesMaterialMask(mfi, mask_index) == mi) {
                is_used = true;
                break;
              }
            }
          }
          if (!is_used) {
            // Ignore this mesh features.
            continue;
          }
        }
        // Create a copy of source mesh features.
        std::unique_ptr<MeshFeatures> mf(new MeshFeatures());
        mf->Copy(mesh.GetMeshFeatures(mfi));

        // Update mesh features attribute index if used.
        if (mf->GetAttributeIndex() != -1) {
          const int new_mf_attribute_index =
              att_id_map_[mf->GetAttributeIndex()];
          mf->SetAttributeIndex(new_mf_attribute_index);
        }

        const MeshFeaturesIndex new_mfi =
            out_meshes[mi]->AddMeshFeatures(std::move(mf));
        if (work_data.split_by_materials && !preserve_materials_) {
          // If the input |mesh| was split by materials and we didn't preserve
          // the materials, all mesh features must be masked to material 0.
          out_meshes[mi]->AddMeshFeaturesMaterialMask(new_mfi, 0);
        } else {
          // Otherwise mesh features use same masking as the source mesh because
          // the material attribute is still present in the split meshes.
          // Note that this masking can be later changed in
          // RemoveUnusedMaterials() call below.
          for (int mask_index = 0;
               mask_index < mesh.NumMeshFeaturesMaterialMasks(mfi);
               ++mask_index) {
            out_meshes[mi]->AddMeshFeaturesMaterialMask(
                new_mfi, mesh.GetMeshFeaturesMaterialMask(mfi, mask_index));
          }
        }
      }

      // Copy over all features textures to the split mesh.
      out_meshes[mi]->GetNonMaterialTextureLibrary().Copy(
          mesh.GetNonMaterialTextureLibrary());

      // Update mesh features texture pointers to the new library.
      for (MeshFeaturesIndex mfi(0); mfi < out_meshes[mi]->NumMeshFeatures();
           ++mfi) {
        Mesh::UpdateMeshFeaturesTexturePointer(
            features_texture_to_index_map,
            &out_meshes[mi]->GetNonMaterialTextureLibrary(),
            &out_meshes[mi]->GetMeshFeatures(mfi));
      }

      // This will remove any mesh features that may not be be actually used
      // by this |out_meshes[mi]| (e.g. because corresponding material indices
      // were not present in this split mesh). This also removes any unused
      // features textures from the non-material texture library.
      DRACO_RETURN_IF_ERROR(
          MeshUtils::RemoveUnusedMeshFeatures(out_meshes[mi].get()));
    }

    if (preserve_structural_metadata_) {
      // Copy proeprty attributes indices from the source |mesh| to the
      // |out_meshes[mi]|.
      for (int i = 0; i < mesh.NumPropertyAttributesIndices(); ++i) {
        if (work_data.split_by_materials) {
          // Copy over only those property attribute indices that were masked to
          // the material corresponding to |mi|.
          bool is_used = false;
          if (mesh.NumPropertyAttributesIndexMaterialMasks(i) == 0) {
            is_used = true;
          } else {
            for (int mask_index = 0;
                 mask_index < mesh.NumPropertyAttributesIndexMaterialMasks(i);
                 ++mask_index) {
              if (mesh.GetPropertyAttributesIndexMaterialMask(i, mask_index) ==
                  mi) {
                is_used = true;
                break;
              }
            }
          }
          if (!is_used) {
            // Ignore this property attributes index.
            continue;
          }
        }
        // Create a copy of source property attributes index.
        const int new_i = out_meshes[mi]->AddPropertyAttributesIndex(
            mesh.GetPropertyAttributesIndex(i));
        if (work_data.split_by_materials && !preserve_materials_) {
          // If the input |mesh| was split by materials and we didn't preserve
          // the materials, all property attributes indices must be masked to
          // material 0.
          out_meshes[mi]->AddPropertyAttributesIndexMaterialMask(new_i, 0);
        } else {
          // Otherwise property attributes index uses same masking as the source
          // mesh because the material attribute is still present in the split
          // meshes. Note that this masking can be later changed in
          // RemoveUnusedMaterials() call below.
          for (int mask_index = 0;
               mask_index < mesh.NumPropertyAttributesIndexMaterialMasks(i);
               ++mask_index) {
            out_meshes[mi]->AddPropertyAttributesIndexMaterialMask(
                new_i,
                mesh.GetPropertyAttributesIndexMaterialMask(i, mask_index));
          }
        }
      }

      // This will remove any property attributes indices that may not be be
      // actually used by this |out_meshes[mi]| (e.g. because corresponding
      // material indices were not present in this split mesh).
      DRACO_RETURN_IF_ERROR(MeshUtils::RemoveUnusedPropertyAttributesIndices(
          out_meshes[mi].get()));
    }

    // Remove unused materials after we remove mesh features because some of
    // the mesh features may have referenced old material indices.
    if (preserve_materials_) {
      out_meshes[mi]->RemoveUnusedMaterials(remove_unused_material_indices_);
    }

    // Copy structural metadata from input mesh to each of the output meshes.
    out_meshes[mi]->GetStructuralMetadata().Copy(mesh.GetStructuralMetadata());
  }
  return std::move(out_meshes);
}

StatusOr<MeshSplitter::MeshVector> MeshSplitter::SplitMeshToComponents(
    const Mesh &mesh, const MeshConnectedComponents &connected_components) {
  // Create the sub-meshes.
  const int num_out_meshes = connected_components.NumConnectedComponents();
  MeshSplitterInternal<TriangleSoupMeshBuilder> splitter_internal;
  typename MeshSplitterInternal<TriangleSoupMeshBuilder>::WorkData work_data;
  work_data.builders.resize(num_out_meshes);
  work_data.num_sub_mesh_elements.resize(num_out_meshes, 0);
  att_id_map_.resize(mesh.num_attributes(), -1);
  work_data.att_id_map = &att_id_map_;
  for (int mi = 0; mi < num_out_meshes; ++mi) {
    const int num_faces = connected_components.NumConnectedComponentFaces(mi);
    work_data.num_sub_mesh_elements[mi] = num_faces;
    splitter_internal.InitializeBuilder(mi, num_faces, mesh, -1, &work_data);
  }

  // Go over all faces of the input mesh and add them to the appropriate
  // sub-mesh.
  for (int mi = 0; mi < num_out_meshes; ++mi) {
    for (int cfi = 0; cfi < connected_components.NumConnectedComponentFaces(mi);
         ++cfi) {
      const FaceIndex fi(
          connected_components.GetConnectedComponent(mi).faces[cfi]);
      const FaceIndex target_fi(cfi);
      AddElementToBuilder(mi, fi, target_fi, mesh, &work_data);
    }
  }
  DRACO_ASSIGN_OR_RETURN(
      auto out_meshes,
      splitter_internal.BuildMeshes(mesh, &work_data, deduplicate_vertices_));
  return FinalizeMeshes(mesh, work_data, std::move(out_meshes));
}

int MeshSplitter::GetSplitMeshAttributeIndex(int source_mesh_att_index) const {
  return att_id_map_[source_mesh_att_index];
}

}  // namespace draco
#endif  // DRACO_TRANSCODER_SUPPORTED
