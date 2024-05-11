// Copyright 2016 The Draco Authors.
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
#include "draco/compression/mesh/mesh_edgebreaker_encoder.h"

#include "draco/compression/mesh/mesh_edgebreaker_encoder_impl.h"
#include "draco/compression/mesh/mesh_edgebreaker_traversal_predictive_encoder.h"
#include "draco/compression/mesh/mesh_edgebreaker_traversal_valence_encoder.h"

namespace draco {

MeshEdgebreakerEncoder::MeshEdgebreakerEncoder() {}

bool MeshEdgebreakerEncoder::InitializeEncoder() {
  const bool is_standard_edgebreaker_available =
      options()->IsFeatureSupported(features::kEdgebreaker);
  const bool is_predictive_edgebreaker_available =
      options()->IsFeatureSupported(features::kPredictiveEdgebreaker);

  impl_ = nullptr;
  // For tiny meshes it's usually better to use the basic edgebreaker as the
  // overhead of the predictive one may turn out to be too big.
  // TODO(b/111065939): Check if this can be improved.
  const bool is_tiny_mesh = mesh()->num_faces() < 1000;

  int selected_edgebreaker_method =
      options()->GetGlobalInt("edgebreaker_method", -1);
  if (selected_edgebreaker_method == -1) {
    if (is_standard_edgebreaker_available &&
        (options()->GetSpeed() >= 5 || !is_predictive_edgebreaker_available ||
         is_tiny_mesh)) {
      selected_edgebreaker_method = MESH_EDGEBREAKER_STANDARD_ENCODING;
    } else {
      selected_edgebreaker_method = MESH_EDGEBREAKER_VALENCE_ENCODING;
    }
  }

  if (selected_edgebreaker_method == MESH_EDGEBREAKER_STANDARD_ENCODING) {
    if (is_standard_edgebreaker_available) {
      buffer()->Encode(
          static_cast<uint8_t>(MESH_EDGEBREAKER_STANDARD_ENCODING));
      impl_ = std::unique_ptr<MeshEdgebreakerEncoderImplInterface>(
          new MeshEdgebreakerEncoderImpl<MeshEdgebreakerTraversalEncoder>());
    }
  } else if (selected_edgebreaker_method == MESH_EDGEBREAKER_VALENCE_ENCODING) {
    buffer()->Encode(static_cast<uint8_t>(MESH_EDGEBREAKER_VALENCE_ENCODING));
    impl_ = std::unique_ptr<MeshEdgebreakerEncoderImplInterface>(
        new MeshEdgebreakerEncoderImpl<
            MeshEdgebreakerTraversalValenceEncoder>());
  }
  if (!impl_) {
    return false;
  }
  if (!impl_->Init(this)) {
    return false;
  }
  return true;
}

bool MeshEdgebreakerEncoder::GenerateAttributesEncoder(int32_t att_id) {
  if (!impl_->GenerateAttributesEncoder(att_id)) {
    return false;
  }
  return true;
}

bool MeshEdgebreakerEncoder::EncodeAttributesEncoderIdentifier(
    int32_t att_encoder_id) {
  if (!impl_->EncodeAttributesEncoderIdentifier(att_encoder_id)) {
    return false;
  }
  return true;
}

Status MeshEdgebreakerEncoder::EncodeConnectivity() {
  return impl_->EncodeConnectivity();
}

void MeshEdgebreakerEncoder::ComputeNumberOfEncodedPoints() {
  if (!impl_) {
    return;
  }
  const CornerTable *const corner_table = impl_->GetCornerTable();
  if (!corner_table) {
    return;
  }
  size_t num_points =
      corner_table->num_vertices() - corner_table->NumIsolatedVertices();

  if (mesh()->num_attributes() > 1) {
    // Gather all corner tables for all non-position attributes.
    std::vector<const MeshAttributeCornerTable *> attribute_corner_tables;
    for (int i = 0; i < mesh()->num_attributes(); ++i) {
      if (mesh()->attribute(i)->attribute_type() ==
          GeometryAttribute::POSITION) {
        continue;
      }
      const MeshAttributeCornerTable *const att_corner_table =
          GetAttributeCornerTable(i);
      // Attribute corner table may not be used in some configurations. For
      // these cases we can assume the attribute connectivity to be the same as
      // the connectivity of the position data.
      if (att_corner_table) {
        attribute_corner_tables.push_back(att_corner_table);
      }
    }

    // Add a new point based on the configuration of interior attribute seams
    // (replicating what the decoder would do).
    for (VertexIndex vi(0); vi < corner_table->num_vertices(); ++vi) {
      if (corner_table->IsVertexIsolated(vi)) {
        continue;
      }
      // Go around all corners of the vertex and keep track of the observed
      // attribute seams.
      const CornerIndex first_corner_index = corner_table->LeftMostCorner(vi);
      const PointIndex first_point_index =
          mesh()->CornerToPointId(first_corner_index);

      PointIndex last_point_index = first_point_index;
      CornerIndex last_corner_index = first_corner_index;
      CornerIndex corner_index = corner_table->SwingRight(first_corner_index);
      size_t num_attribute_seams = 0;
      while (corner_index != kInvalidCornerIndex) {
        const PointIndex point_index = mesh()->CornerToPointId(corner_index);
        bool seam_found = false;
        if (point_index != last_point_index) {
          // Point index changed - new attribute seam detected.
          seam_found = true;
          last_point_index = point_index;
        } else {
          // Even though point indices matches, there still may be a seam caused
          // by non-manifold connectivity of non-position attribute data.
          for (int i = 0; i < attribute_corner_tables.size(); ++i) {
            if (attribute_corner_tables[i]->Vertex(corner_index) !=
                attribute_corner_tables[i]->Vertex(last_corner_index)) {
              seam_found = true;
              break;  // No need to process other attributes.
            }
          }
        }
        if (seam_found) {
          ++num_attribute_seams;
        }

        if (corner_index == first_corner_index) {
          break;
        }

        // Proceed to the next corner
        last_corner_index = corner_index;
        corner_index = corner_table->SwingRight(corner_index);
      }

      if (!corner_table->IsOnBoundary(vi) && num_attribute_seams > 0) {
        // If the last visited point index is the same as the first point index
        // we traveled all the way around the vertex. In this case the number of
        // new points should be num_attribute_seams - 1
        num_points += num_attribute_seams - 1;
      } else {
        // Else the vertex was either on a boundary (i.e. we couldn't travel all
        // around the vertex), or we ended up at a different point. In both of
        // these cases, the number of new points is equal to the number of
        // attribute seams.
        num_points += num_attribute_seams;
      }
    }
  }
  set_num_encoded_points(num_points);
}

void MeshEdgebreakerEncoder::ComputeNumberOfEncodedFaces() {
  if (!impl_) {
    return;
  }
  const CornerTable *const corner_table = impl_->GetCornerTable();
  if (!corner_table) {
    return;
  }
  set_num_encoded_faces(corner_table->num_faces() -
                        corner_table->NumDegeneratedFaces());
}

}  // namespace draco
