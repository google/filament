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
#include "draco/compression/mesh/mesh_edgebreaker_encoder_impl.h"

#include <algorithm>

#include "draco/compression/attributes/sequential_attribute_encoders_controller.h"
#include "draco/compression/mesh/mesh_edgebreaker_encoder.h"
#include "draco/compression/mesh/mesh_edgebreaker_traversal_predictive_encoder.h"
#include "draco/compression/mesh/mesh_edgebreaker_traversal_valence_encoder.h"
#include "draco/compression/mesh/traverser/depth_first_traverser.h"
#include "draco/compression/mesh/traverser/max_prediction_degree_traverser.h"
#include "draco/compression/mesh/traverser/mesh_attribute_indices_encoding_observer.h"
#include "draco/compression/mesh/traverser/mesh_traversal_sequencer.h"
#include "draco/compression/mesh/traverser/traverser_base.h"
#include "draco/mesh/corner_table_iterators.h"
#include "draco/mesh/mesh_misc_functions.h"

namespace draco {
// TODO(draco-eng) consider converting 'typedef' to 'using' and deduplicate.
typedef CornerIndex CornerIndex;
typedef FaceIndex FaceIndex;
typedef VertexIndex VertexIndex;

template <class TraversalEncoder>
MeshEdgebreakerEncoderImpl<TraversalEncoder>::MeshEdgebreakerEncoderImpl()
    : encoder_(nullptr),
      mesh_(nullptr),
      last_encoded_symbol_id_(-1),
      num_split_symbols_(0),
      use_single_connectivity_(false) {}

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::Init(
    MeshEdgebreakerEncoder *encoder) {
  encoder_ = encoder;
  mesh_ = encoder->mesh();
  attribute_encoder_to_data_id_map_.clear();

  if (encoder_->options()->IsGlobalOptionSet("split_mesh_on_seams")) {
    use_single_connectivity_ =
        encoder_->options()->GetGlobalBool("split_mesh_on_seams", false);
  } else if (encoder_->options()->GetSpeed() >= 6) {
    // Else use default setting based on speed.
    use_single_connectivity_ = true;
  } else {
    use_single_connectivity_ = false;
  }
  return true;
}

template <class TraversalEncoder>
const MeshAttributeCornerTable *
MeshEdgebreakerEncoderImpl<TraversalEncoder>::GetAttributeCornerTable(
    int att_id) const {
  for (uint32_t i = 0; i < attribute_data_.size(); ++i) {
    if (attribute_data_[i].attribute_index == att_id) {
      if (attribute_data_[i].is_connectivity_used) {
        return &attribute_data_[i].connectivity_data;
      }
      return nullptr;
    }
  }
  return nullptr;
}

template <class TraversalEncoder>
const MeshAttributeIndicesEncodingData *
MeshEdgebreakerEncoderImpl<TraversalEncoder>::GetAttributeEncodingData(
    int att_id) const {
  for (uint32_t i = 0; i < attribute_data_.size(); ++i) {
    if (attribute_data_[i].attribute_index == att_id) {
      return &attribute_data_[i].encoding_data;
    }
  }
  return &pos_encoding_data_;
}

template <class TraversalEncoder>
template <class TraverserT>
std::unique_ptr<PointsSequencer>
MeshEdgebreakerEncoderImpl<TraversalEncoder>::CreateVertexTraversalSequencer(
    MeshAttributeIndicesEncodingData *encoding_data) {
  typedef typename TraverserT::TraversalObserver AttObserver;
  typedef typename TraverserT::CornerTable CornerTable;

  std::unique_ptr<MeshTraversalSequencer<TraverserT>> traversal_sequencer(
      new MeshTraversalSequencer<TraverserT>(mesh_, encoding_data));

  AttObserver att_observer(corner_table_.get(), mesh_,
                           traversal_sequencer.get(), encoding_data);

  TraverserT att_traverser;
  att_traverser.Init(corner_table_.get(), att_observer);

  // Set order of corners to simulate the corner order of the decoder.
  traversal_sequencer->SetCornerOrder(processed_connectivity_corners_);
  traversal_sequencer->SetTraverser(att_traverser);
  return std::move(traversal_sequencer);
}

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::GenerateAttributesEncoder(
    int32_t att_id) {
  // For now, either create one encoder for each attribute or use a single
  // encoder for all attributes. Ideally we can share the same encoder for
  // a sub-set of attributes with the same connectivity (this is especially true
  // for per-vertex attributes).
  if (use_single_connectivity_ && GetEncoder()->num_attributes_encoders() > 0) {
    // We are using single connectivity and we already have an attribute
    // encoder. Add the attribute to the encoder and return.
    GetEncoder()->attributes_encoder(0)->AddAttributeId(att_id);
    return true;
  }
  const int32_t element_type =
      GetEncoder()->mesh()->GetAttributeElementType(att_id);
  const PointAttribute *const att =
      GetEncoder()->point_cloud()->attribute(att_id);
  int32_t att_data_id = -1;
  for (uint32_t i = 0; i < attribute_data_.size(); ++i) {
    if (attribute_data_[i].attribute_index == att_id) {
      att_data_id = i;
      break;
    }
  }
  MeshTraversalMethod traversal_method = MESH_TRAVERSAL_DEPTH_FIRST;
  std::unique_ptr<PointsSequencer> sequencer;
  if (use_single_connectivity_ ||
      att->attribute_type() == GeometryAttribute::POSITION ||
      element_type == MESH_VERTEX_ATTRIBUTE ||
      (element_type == MESH_CORNER_ATTRIBUTE &&
       attribute_data_[att_data_id].connectivity_data.no_interior_seams())) {
    // Per-vertex attribute reached, use the basic corner table to traverse the
    // mesh.
    MeshAttributeIndicesEncodingData *encoding_data;
    if (use_single_connectivity_ ||
        att->attribute_type() == GeometryAttribute::POSITION) {
      encoding_data = &pos_encoding_data_;
    } else {
      encoding_data = &attribute_data_[att_data_id].encoding_data;

      // Ensure we use the correct number of vertices in the encoding data.
      encoding_data->vertex_to_encoded_attribute_value_index_map.assign(
          corner_table_->num_vertices(), -1);

      // Mark the attribute specific connectivity data as not used as we use the
      // position attribute connectivity data.
      attribute_data_[att_data_id].is_connectivity_used = false;
    }

    if (GetEncoder()->options()->GetSpeed() == 0 &&
        att->attribute_type() == GeometryAttribute::POSITION) {
      traversal_method = MESH_TRAVERSAL_PREDICTION_DEGREE;
      if (use_single_connectivity_ && mesh_->num_attributes() > 1) {
        // Make sure we don't use the prediction degree traversal when we encode
        // multiple attributes using the same connectivity.
        // TODO(ostava): We should investigate this and see if the prediction
        // degree can be actually used efficiently for non-position attributes.
        traversal_method = MESH_TRAVERSAL_DEPTH_FIRST;
      }
    }
    // Defining sequencer via a traversal scheme.
    if (traversal_method == MESH_TRAVERSAL_PREDICTION_DEGREE) {
      typedef MeshAttributeIndicesEncodingObserver<CornerTable> AttObserver;
      typedef MaxPredictionDegreeTraverser<CornerTable, AttObserver>
          AttTraverser;
      sequencer = CreateVertexTraversalSequencer<AttTraverser>(encoding_data);
    } else if (traversal_method == MESH_TRAVERSAL_DEPTH_FIRST) {
      typedef MeshAttributeIndicesEncodingObserver<CornerTable> AttObserver;
      typedef DepthFirstTraverser<CornerTable, AttObserver> AttTraverser;
      sequencer = CreateVertexTraversalSequencer<AttTraverser>(encoding_data);
    }
  } else {
    // Per-corner attribute encoder.
    typedef MeshAttributeIndicesEncodingObserver<MeshAttributeCornerTable>
        AttObserver;
    typedef DepthFirstTraverser<MeshAttributeCornerTable, AttObserver>
        AttTraverser;

    MeshAttributeIndicesEncodingData *const encoding_data =
        &attribute_data_[att_data_id].encoding_data;
    const MeshAttributeCornerTable *const corner_table =
        &attribute_data_[att_data_id].connectivity_data;

    // Ensure we use the correct number of vertices in the encoding data.
    attribute_data_[att_data_id]
        .encoding_data.vertex_to_encoded_attribute_value_index_map.assign(
            attribute_data_[att_data_id].connectivity_data.num_vertices(), -1);

    std::unique_ptr<MeshTraversalSequencer<AttTraverser>> traversal_sequencer(
        new MeshTraversalSequencer<AttTraverser>(mesh_, encoding_data));

    AttObserver att_observer(corner_table, mesh_, traversal_sequencer.get(),
                             encoding_data);

    AttTraverser att_traverser;
    att_traverser.Init(corner_table, att_observer);

    // Set order of corners to simulate the corner order of the decoder.
    traversal_sequencer->SetCornerOrder(processed_connectivity_corners_);
    traversal_sequencer->SetTraverser(att_traverser);
    sequencer = std::move(traversal_sequencer);
  }

  if (!sequencer) {
    return false;
  }

  if (att_data_id == -1) {
    pos_traversal_method_ = traversal_method;
  } else {
    attribute_data_[att_data_id].traversal_method = traversal_method;
  }

  std::unique_ptr<SequentialAttributeEncodersController> att_controller(
      new SequentialAttributeEncodersController(std::move(sequencer), att_id));

  // Update the mapping between the encoder id and the attribute data id.
  // This will be used by the decoder to select the appropriate attribute
  // decoder and the correct connectivity.
  attribute_encoder_to_data_id_map_.push_back(att_data_id);
  GetEncoder()->AddAttributesEncoder(std::move(att_controller));
  return true;
}  // namespace draco

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::
    EncodeAttributesEncoderIdentifier(int32_t att_encoder_id) {
  const int8_t att_data_id = attribute_encoder_to_data_id_map_[att_encoder_id];
  encoder_->buffer()->Encode(att_data_id);

  // Also encode the type of the encoder that we used.
  int32_t element_type = MESH_VERTEX_ATTRIBUTE;
  MeshTraversalMethod traversal_method;
  if (att_data_id >= 0) {
    const int32_t att_id = attribute_data_[att_data_id].attribute_index;
    element_type = GetEncoder()->mesh()->GetAttributeElementType(att_id);
    traversal_method = attribute_data_[att_data_id].traversal_method;
  } else {
    traversal_method = pos_traversal_method_;
  }
  if (element_type == MESH_VERTEX_ATTRIBUTE ||
      (element_type == MESH_CORNER_ATTRIBUTE &&
       attribute_data_[att_data_id].connectivity_data.no_interior_seams())) {
    // Per-vertex encoder.
    encoder_->buffer()->Encode(static_cast<uint8_t>(MESH_VERTEX_ATTRIBUTE));
  } else {
    // Per-corner encoder.
    encoder_->buffer()->Encode(static_cast<uint8_t>(MESH_CORNER_ATTRIBUTE));
  }
  // Encode the mesh traversal method.
  encoder_->buffer()->Encode(static_cast<uint8_t>(traversal_method));
  return true;
}

template <class TraversalEncoder>
Status MeshEdgebreakerEncoderImpl<TraversalEncoder>::EncodeConnectivity() {
  // To encode the mesh, we need face connectivity data stored in a corner
  // table. To compute the connectivity we must use indices associated with
  // POSITION attribute, because they define which edges can be connected
  // together, unless the option |use_single_connectivity_| is set in which case
  // we break the mesh along attribute seams and use the same connectivity for
  // all attributes.
  if (use_single_connectivity_) {
    corner_table_ = CreateCornerTableFromAllAttributes(mesh_);
  } else {
    corner_table_ = CreateCornerTableFromPositionAttribute(mesh_);
  }
  if (corner_table_ == nullptr ||
      corner_table_->num_faces() == corner_table_->NumDegeneratedFaces()) {
    // Failed to construct the corner table.
    // TODO(ostava): Add better error reporting.
    return Status(Status::DRACO_ERROR, "All triangles are degenerate.");
  }

  traversal_encoder_.Init(this);

  // Also encode the total number of vertices that is going to be encoded.
  // This can be different from the mesh_->num_points() + num_new_vertices,
  // because some of the vertices of the input mesh can be ignored (e.g.
  // vertices on degenerated faces or isolated vertices not attached to any
  // face).
  const uint32_t num_vertices_to_be_encoded =
      corner_table_->num_vertices() - corner_table_->NumIsolatedVertices();
  EncodeVarint(num_vertices_to_be_encoded, encoder_->buffer());

  const uint32_t num_faces =
      corner_table_->num_faces() - corner_table_->NumDegeneratedFaces();
  EncodeVarint(num_faces, encoder_->buffer());

  // Reset encoder data that may have been initialized in previous runs.
  visited_faces_.assign(mesh_->num_faces(), false);
  pos_encoding_data_.vertex_to_encoded_attribute_value_index_map.assign(
      corner_table_->num_vertices(), -1);
  pos_encoding_data_.encoded_attribute_value_index_to_corner_map.clear();
  pos_encoding_data_.encoded_attribute_value_index_to_corner_map.reserve(
      corner_table_->num_faces() * 3);
  visited_vertex_ids_.assign(corner_table_->num_vertices(), false);
  vertex_traversal_length_.clear();
  last_encoded_symbol_id_ = -1;
  num_split_symbols_ = 0;
  topology_split_event_data_.clear();
  face_to_split_symbol_map_.clear();
  visited_holes_.clear();
  vertex_hole_id_.assign(corner_table_->num_vertices(), -1);
  processed_connectivity_corners_.clear();
  processed_connectivity_corners_.reserve(corner_table_->num_faces());
  pos_encoding_data_.num_values = 0;

  if (!FindHoles()) {
    return Status(Status::DRACO_ERROR, "Failed to process mesh holes.");
  }

  if (!InitAttributeData()) {
    return Status(Status::DRACO_ERROR, "Failed to initialize attribute data.");
  }

  const uint8_t num_attribute_data =
      static_cast<uint8_t>(attribute_data_.size());
  encoder_->buffer()->Encode(num_attribute_data);
  traversal_encoder_.SetNumAttributeData(num_attribute_data);

  const int num_corners = corner_table_->num_corners();

  traversal_encoder_.Start();

  std::vector<CornerIndex> init_face_connectivity_corners;
  // Traverse the surface starting from each unvisited corner.
  for (int c_id = 0; c_id < num_corners; ++c_id) {
    CornerIndex corner_index(c_id);
    const FaceIndex face_id = corner_table_->Face(corner_index);
    if (visited_faces_[face_id.value()]) {
      continue;  // Face has been already processed.
    }
    if (corner_table_->IsDegenerated(face_id)) {
      continue;  // Ignore degenerated faces.
    }

    CornerIndex start_corner;
    const bool interior_config =
        FindInitFaceConfiguration(face_id, &start_corner);
    traversal_encoder_.EncodeStartFaceConfiguration(interior_config);

    if (interior_config) {
      // Select the correct vertex on the face as the root.
      corner_index = start_corner;
      const VertexIndex vert_id = corner_table_->Vertex(corner_index);
      // Mark all vertices of a given face as visited.
      const VertexIndex next_vert_id =
          corner_table_->Vertex(corner_table_->Next(corner_index));
      const VertexIndex prev_vert_id =
          corner_table_->Vertex(corner_table_->Previous(corner_index));

      visited_vertex_ids_[vert_id.value()] = true;
      visited_vertex_ids_[next_vert_id.value()] = true;
      visited_vertex_ids_[prev_vert_id.value()] = true;
      // New traversal started. Initiate it's length with the first vertex.
      vertex_traversal_length_.push_back(1);

      // Mark the face as visited.
      visited_faces_[face_id.value()] = true;
      // Start compressing from the opposite face of the "next" corner. This way
      // the first encoded corner corresponds to the tip corner of the regular
      // edgebreaker traversal (essentially the initial face can be then viewed
      // as a TOPOLOGY_C face).
      init_face_connectivity_corners.push_back(
          corner_table_->Next(corner_index));
      const CornerIndex opp_id =
          corner_table_->Opposite(corner_table_->Next(corner_index));
      const FaceIndex opp_face_id = corner_table_->Face(opp_id);
      if (opp_face_id != kInvalidFaceIndex &&
          !visited_faces_[opp_face_id.value()]) {
        if (!EncodeConnectivityFromCorner(opp_id)) {
          return Status(Status::DRACO_ERROR,
                        "Failed to encode mesh component.");
        }
      }
    } else {
      // Boundary configuration. We start on a boundary rather than on a face.
      // First encode the hole that's opposite to the start_corner.
      EncodeHole(corner_table_->Next(start_corner), true);
      // Start processing the face opposite to the boundary edge (the face
      // containing the start_corner).
      if (!EncodeConnectivityFromCorner(start_corner)) {
        return Status(Status::DRACO_ERROR, "Failed to encode mesh component.");
      }
    }
  }
  // Reverse the order of connectivity corners to match the order in which
  // they are going to be decoded.
  std::reverse(processed_connectivity_corners_.begin(),
               processed_connectivity_corners_.end());
  // Append the init face connectivity corners (which are processed in order by
  // the decoder after the regular corners.
  processed_connectivity_corners_.insert(processed_connectivity_corners_.end(),
                                         init_face_connectivity_corners.begin(),
                                         init_face_connectivity_corners.end());
  // Encode connectivity for all non-position attributes.
  if (!attribute_data_.empty()) {
    // Use the same order of corner that will be used by the decoder.
    visited_faces_.assign(mesh_->num_faces(), false);
    for (CornerIndex ci : processed_connectivity_corners_) {
      EncodeAttributeConnectivitiesOnFace(ci);
    }
  }
  traversal_encoder_.Done();

  // Encode the number of symbols.
  const uint32_t num_encoded_symbols =
      static_cast<uint32_t>(traversal_encoder_.NumEncodedSymbols());
  EncodeVarint(num_encoded_symbols, encoder_->buffer());

  // Encode the number of split symbols.
  EncodeVarint(num_split_symbols_, encoder_->buffer());

  // Append the traversal buffer.
  if (!EncodeSplitData()) {
    return Status(Status::DRACO_ERROR, "Failed to encode split data.");
  }
  encoder_->buffer()->Encode(traversal_encoder_.buffer().data(),
                             traversal_encoder_.buffer().size());

  return OkStatus();
}

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::EncodeSplitData() {
  uint32_t num_events =
      static_cast<uint32_t>(topology_split_event_data_.size());
  EncodeVarint(num_events, encoder_->buffer());
  if (num_events > 0) {
    // Encode split symbols using delta and varint coding. Split edges are
    // encoded using direct bit coding.
    int last_source_symbol_id = 0;  // Used for delta coding.
    for (uint32_t i = 0; i < num_events; ++i) {
      const TopologySplitEventData &event_data = topology_split_event_data_[i];
      // Encode source symbol id as delta from the previous source symbol id.
      // Source symbol ids are always stored in increasing order so the delta is
      // going to be positive.
      EncodeVarint<uint32_t>(
          event_data.source_symbol_id - last_source_symbol_id,
          encoder_->buffer());
      // Encode split symbol id as delta from the current source symbol id.
      // Split symbol id is always smaller than source symbol id so the below
      // delta is going to be positive.
      EncodeVarint<uint32_t>(
          event_data.source_symbol_id - event_data.split_symbol_id,
          encoder_->buffer());
      last_source_symbol_id = event_data.source_symbol_id;
    }
    encoder_->buffer()->StartBitEncoding(num_events, false);
    for (uint32_t i = 0; i < num_events; ++i) {
      const TopologySplitEventData &event_data = topology_split_event_data_[i];
      encoder_->buffer()->EncodeLeastSignificantBits32(1,
                                                       event_data.source_edge);
    }
    encoder_->buffer()->EndBitEncoding();
  }
  return true;
}

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::FindInitFaceConfiguration(
    FaceIndex face_id, CornerIndex *out_corner) const {
  CornerIndex corner_index = CornerIndex(3 * face_id.value());
  for (int i = 0; i < 3; ++i) {
    if (corner_table_->Opposite(corner_index) == kInvalidCornerIndex) {
      // If there is a boundary edge, the configuration is exterior and return
      // the CornerIndex opposite to the first boundary edge.
      *out_corner = corner_index;
      return false;
    }
    if (vertex_hole_id_[corner_table_->Vertex(corner_index).value()] != -1) {
      // Boundary vertex found. Find the first boundary edge attached to the
      // point and return the corner opposite to it.
      CornerIndex right_corner = corner_index;
      while (right_corner != kInvalidCornerIndex) {
        corner_index = right_corner;
        right_corner = corner_table_->SwingRight(right_corner);
      }
      // |corner_index| now lies on a boundary edge and its previous corner is
      // guaranteed to be the opposite corner of the boundary edge.
      *out_corner = corner_table_->Previous(corner_index);
      return false;
    }
    corner_index = corner_table_->Next(corner_index);
  }
  // Else we have an interior configuration. Return the first corner id.
  *out_corner = corner_index;
  return true;
}

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::EncodeConnectivityFromCorner(
    CornerIndex corner_id) {
  corner_traversal_stack_.clear();
  corner_traversal_stack_.push_back(corner_id);
  const int num_faces = mesh_->num_faces();
  while (!corner_traversal_stack_.empty()) {
    // Currently processed corner.
    corner_id = corner_traversal_stack_.back();
    // Make sure the face hasn't been visited yet.
    if (corner_id == kInvalidCornerIndex ||
        visited_faces_[corner_table_->Face(corner_id).value()]) {
      // This face has been already traversed.
      corner_traversal_stack_.pop_back();
      continue;
    }
    int num_visited_faces = 0;
    while (num_visited_faces < num_faces) {
      // Mark the current face as visited.
      ++num_visited_faces;
      ++last_encoded_symbol_id_;

      const FaceIndex face_id = corner_table_->Face(corner_id);
      visited_faces_[face_id.value()] = true;
      processed_connectivity_corners_.push_back(corner_id);
      traversal_encoder_.NewCornerReached(corner_id);
      const VertexIndex vert_id = corner_table_->Vertex(corner_id);
      const bool on_boundary = (vertex_hole_id_[vert_id.value()] != -1);
      if (!IsVertexVisited(vert_id)) {
        // A new unvisited vertex has been reached. We need to store its
        // position difference using next, prev, and opposite vertices.
        visited_vertex_ids_[vert_id.value()] = true;
        if (!on_boundary) {
          // If the vertex is on boundary it must correspond to an unvisited
          // hole and it will be encoded with TOPOLOGY_S symbol later).
          traversal_encoder_.EncodeSymbol(TOPOLOGY_C);
          // Move to the right triangle.
          corner_id = GetRightCorner(corner_id);
          continue;
        }
      }
      // The current vertex has been already visited or it was on a boundary.
      // We need to determine whether we can visit any of it's neighboring
      // faces.
      const CornerIndex right_corner_id = GetRightCorner(corner_id);
      const CornerIndex left_corner_id = GetLeftCorner(corner_id);
      const FaceIndex right_face_id = corner_table_->Face(right_corner_id);
      const FaceIndex left_face_id = corner_table_->Face(left_corner_id);
      if (IsRightFaceVisited(corner_id)) {
        // Right face has been already visited.
        // Check whether there is a topology split event.
        if (right_face_id != kInvalidFaceIndex) {
          CheckAndStoreTopologySplitEvent(last_encoded_symbol_id_,
                                          face_id.value(), RIGHT_FACE_EDGE,
                                          right_face_id.value());
        }
        if (IsLeftFaceVisited(corner_id)) {
          // Both neighboring faces are visited. End reached.
          // Check whether there is a topology split event on the left face.
          if (left_face_id != kInvalidFaceIndex) {
            CheckAndStoreTopologySplitEvent(last_encoded_symbol_id_,
                                            face_id.value(), LEFT_FACE_EDGE,
                                            left_face_id.value());
          }
          traversal_encoder_.EncodeSymbol(TOPOLOGY_E);
          corner_traversal_stack_.pop_back();
          break;  // Break from the while (num_visited_faces < num_faces) loop.
        } else {
          traversal_encoder_.EncodeSymbol(TOPOLOGY_R);
          // Go to the left face.
          corner_id = left_corner_id;
        }
      } else {
        // Right face was not visited.
        if (IsLeftFaceVisited(corner_id)) {
          // Check whether there is a topology split event on the left face.
          if (left_face_id != kInvalidFaceIndex) {
            CheckAndStoreTopologySplitEvent(last_encoded_symbol_id_,
                                            face_id.value(), LEFT_FACE_EDGE,
                                            left_face_id.value());
          }
          traversal_encoder_.EncodeSymbol(TOPOLOGY_L);
          // Left face visited, go to the right one.
          corner_id = right_corner_id;
        } else {
          traversal_encoder_.EncodeSymbol(TOPOLOGY_S);
          ++num_split_symbols_;
          // Both neighboring faces are unvisited, we need to visit both of
          // them.
          if (on_boundary) {
            // The tip vertex is on a hole boundary. If the hole hasn't been
            // visited yet we need to encode it.
            const int hole_id = vertex_hole_id_[vert_id.value()];
            if (!visited_holes_[hole_id]) {
              EncodeHole(corner_id, false);
            }
          }
          face_to_split_symbol_map_[face_id.value()] = last_encoded_symbol_id_;
          // Split the traversal.
          // First make the top of the current corner stack point to the left
          // face (this one will be processed second).
          corner_traversal_stack_.back() = left_corner_id;
          // Add a new corner to the top of the stack (right face needs to
          // be traversed first).
          corner_traversal_stack_.push_back(right_corner_id);
          // Break from the while (num_visited_faces < num_faces) loop.
          break;
        }
      }
    }
  }
  return true;  // All corners have been processed.
}

template <class TraversalEncoder>
int MeshEdgebreakerEncoderImpl<TraversalEncoder>::EncodeHole(
    CornerIndex start_corner_id, bool encode_first_vertex) {
  // We know that the start corner lies on a hole but we first need to find the
  // boundary edge going from that vertex. It is the first edge in CW
  // direction.
  CornerIndex corner_id = start_corner_id;
  corner_id = corner_table_->Previous(corner_id);
  while (corner_table_->Opposite(corner_id) != kInvalidCornerIndex) {
    corner_id = corner_table_->Opposite(corner_id);
    corner_id = corner_table_->Next(corner_id);
  }
  const VertexIndex start_vertex_id = corner_table_->Vertex(start_corner_id);

  int num_encoded_hole_verts = 0;
  if (encode_first_vertex) {
    visited_vertex_ids_[start_vertex_id.value()] = true;
    ++num_encoded_hole_verts;
  }

  // corner_id is now opposite to the boundary edge.
  // Mark the hole as visited.
  visited_holes_[vertex_hole_id_[start_vertex_id.value()]] = true;
  // Get the start vertex of the edge and use it as a reference.
  VertexIndex start_vert_id =
      corner_table_->Vertex(corner_table_->Next(corner_id));
  // Get the end vertex of the edge.
  VertexIndex act_vertex_id =
      corner_table_->Vertex(corner_table_->Previous(corner_id));
  while (act_vertex_id != start_vertex_id) {
    // Encode the end vertex of the boundary edge.

    start_vert_id = act_vertex_id;

    // Mark the vertex as visited.
    visited_vertex_ids_[act_vertex_id.value()] = true;
    ++num_encoded_hole_verts;
    corner_id = corner_table_->Next(corner_id);
    // Look for the next attached open boundary edge.
    while (corner_table_->Opposite(corner_id) != kInvalidCornerIndex) {
      corner_id = corner_table_->Opposite(corner_id);
      corner_id = corner_table_->Next(corner_id);
    }
    act_vertex_id = corner_table_->Vertex(corner_table_->Previous(corner_id));
  }
  return num_encoded_hole_verts;
}

template <class TraversalEncoder>
CornerIndex MeshEdgebreakerEncoderImpl<TraversalEncoder>::GetRightCorner(
    CornerIndex corner_id) const {
  const CornerIndex next_corner_id = corner_table_->Next(corner_id);
  return corner_table_->Opposite(next_corner_id);
}

template <class TraversalEncoder>
CornerIndex MeshEdgebreakerEncoderImpl<TraversalEncoder>::GetLeftCorner(
    CornerIndex corner_id) const {
  const CornerIndex prev_corner_id = corner_table_->Previous(corner_id);
  return corner_table_->Opposite(prev_corner_id);
}

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::IsRightFaceVisited(
    CornerIndex corner_id) const {
  const CornerIndex next_corner_id = corner_table_->Next(corner_id);
  const CornerIndex opp_corner_id = corner_table_->Opposite(next_corner_id);
  if (opp_corner_id != kInvalidCornerIndex) {
    return visited_faces_[corner_table_->Face(opp_corner_id).value()];
  }
  // Else we are on a boundary.
  return true;
}

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::IsLeftFaceVisited(
    CornerIndex corner_id) const {
  const CornerIndex prev_corner_id = corner_table_->Previous(corner_id);
  const CornerIndex opp_corner_id = corner_table_->Opposite(prev_corner_id);
  if (opp_corner_id != kInvalidCornerIndex) {
    return visited_faces_[corner_table_->Face(opp_corner_id).value()];
  }
  // Else we are on a boundary.
  return true;
}

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::FindHoles() {
  // TODO(ostava): Add more error checking for invalid geometry data.
  const int num_corners = corner_table_->num_corners();
  // Go over all corners and detect non-visited open boundaries
  for (CornerIndex i(0); i < num_corners; ++i) {
    if (corner_table_->IsDegenerated(corner_table_->Face(i))) {
      continue;  // Don't process corners assigned to degenerated faces.
    }
    if (corner_table_->Opposite(i) == kInvalidCornerIndex) {
      // No opposite corner means no opposite face, so the opposite edge
      // of the corner is an open boundary.
      // Check whether we have already traversed the boundary.
      VertexIndex boundary_vert_id =
          corner_table_->Vertex(corner_table_->Next(i));
      if (vertex_hole_id_[boundary_vert_id.value()] != -1) {
        // The start vertex of the boundary edge is already assigned to an
        // open boundary. No need to traverse it again.
        continue;
      }
      // Else we found a new open boundary and we are going to traverse along it
      // and mark all visited vertices.
      const int boundary_id = static_cast<int>(visited_holes_.size());
      visited_holes_.push_back(false);

      CornerIndex corner_id = i;
      while (vertex_hole_id_[boundary_vert_id.value()] == -1) {
        // Mark the first vertex on the open boundary.
        vertex_hole_id_[boundary_vert_id.value()] = boundary_id;
        corner_id = corner_table_->Next(corner_id);
        // Look for the next attached open boundary edge.
        while (corner_table_->Opposite(corner_id) != kInvalidCornerIndex) {
          corner_id = corner_table_->Opposite(corner_id);
          corner_id = corner_table_->Next(corner_id);
        }
        // Id of the next vertex in the vertex on the hole.
        boundary_vert_id =
            corner_table_->Vertex(corner_table_->Next(corner_id));
      }
    }
  }
  return true;
}

template <class TraversalEncoder>
int MeshEdgebreakerEncoderImpl<TraversalEncoder>::GetSplitSymbolIdOnFace(
    int face_id) const {
  auto it = face_to_split_symbol_map_.find(face_id);
  if (it == face_to_split_symbol_map_.end()) {
    return -1;
  }
  return it->second;
}

template <class TraversalEncoder>
void MeshEdgebreakerEncoderImpl<
    TraversalEncoder>::CheckAndStoreTopologySplitEvent(int src_symbol_id,
                                                       int /* src_face_id */,
                                                       EdgeFaceName src_edge,
                                                       int neighbor_face_id) {
  const int symbol_id = GetSplitSymbolIdOnFace(neighbor_face_id);
  if (symbol_id == -1) {
    return;  // Not a split symbol, no topology split event could happen.
  }
  TopologySplitEventData event_data;

  event_data.split_symbol_id = symbol_id;
  event_data.source_symbol_id = src_symbol_id;
  event_data.source_edge = src_edge;
  topology_split_event_data_.push_back(event_data);
}

template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<TraversalEncoder>::InitAttributeData() {
  if (use_single_connectivity_) {
    return true;  // All attributes use the same connectivity.
  }

  const int num_attributes = mesh_->num_attributes();
  // Ignore the position attribute. It's decoded separately.
  attribute_data_.resize(num_attributes - 1);
  if (num_attributes == 1) {
    return true;
  }
  int data_index = 0;
  for (int i = 0; i < num_attributes; ++i) {
    const int32_t att_index = i;
    if (mesh_->attribute(att_index)->attribute_type() ==
        GeometryAttribute::POSITION) {
      continue;
    }
    const PointAttribute *const att = mesh_->attribute(att_index);
    attribute_data_[data_index].attribute_index = att_index;
    attribute_data_[data_index]
        .encoding_data.encoded_attribute_value_index_to_corner_map.clear();
    attribute_data_[data_index]
        .encoding_data.encoded_attribute_value_index_to_corner_map.reserve(
            corner_table_->num_corners());
    attribute_data_[data_index].encoding_data.num_values = 0;
    attribute_data_[data_index].connectivity_data.InitFromAttribute(
        mesh_, corner_table_.get(), att);
    ++data_index;
  }
  return true;
}

// TODO(ostava): Note that if the input mesh used the same attribute index on
// multiple different vertices, such attribute will be duplicated using the
// encoding below. Eventually, we may consider either using a different encoding
// scheme for such cases, or at least deduplicating the attributes in the
// decoder.
template <class TraversalEncoder>
bool MeshEdgebreakerEncoderImpl<
    TraversalEncoder>::EncodeAttributeConnectivitiesOnFace(CornerIndex corner) {
  // Three corners of the face.
  const CornerIndex corners[3] = {corner, corner_table_->Next(corner),
                                  corner_table_->Previous(corner)};

  const FaceIndex src_face_id = corner_table_->Face(corner);
  visited_faces_[src_face_id.value()] = true;
  for (int c = 0; c < 3; ++c) {
    const CornerIndex opp_corner = corner_table_->Opposite(corners[c]);
    if (opp_corner == kInvalidCornerIndex) {
      continue;  // Don't encode attribute seams on boundary edges.
    }
    const FaceIndex opp_face_id = corner_table_->Face(opp_corner);
    // Don't encode edges when the opposite face has been already processed.
    if (visited_faces_[opp_face_id.value()]) {
      continue;
    }

    for (uint32_t i = 0; i < attribute_data_.size(); ++i) {
      if (attribute_data_[i].connectivity_data.IsCornerOppositeToSeamEdge(
              corners[c])) {
        traversal_encoder_.EncodeAttributeSeam(i, true);
      } else {
        traversal_encoder_.EncodeAttributeSeam(i, false);
      }
    }
  }
  return true;
}

template class MeshEdgebreakerEncoderImpl<MeshEdgebreakerTraversalEncoder>;
template class MeshEdgebreakerEncoderImpl<
    MeshEdgebreakerTraversalPredictiveEncoder>;
template class MeshEdgebreakerEncoderImpl<
    MeshEdgebreakerTraversalValenceEncoder>;

}  // namespace draco
