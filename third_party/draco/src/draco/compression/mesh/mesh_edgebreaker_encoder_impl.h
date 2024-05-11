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
#ifndef DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_ENCODER_IMPL_H_
#define DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_ENCODER_IMPL_H_

#include <unordered_map>

#include "draco/compression/attributes/mesh_attribute_indices_encoding_data.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/compression/mesh/mesh_edgebreaker_encoder_impl_interface.h"
#include "draco/compression/mesh/mesh_edgebreaker_shared.h"
#include "draco/compression/mesh/traverser/mesh_traversal_sequencer.h"
#include "draco/core/encoder_buffer.h"
#include "draco/mesh/mesh_attribute_corner_table.h"

namespace draco {

// Class implementing the edgebreaker encoding as described in "3D Compression
// Made Simple: Edgebreaker on a Corner-Table" by Rossignac at al.'01.
// http://www.cc.gatech.edu/~jarek/papers/CornerTableSMI.pdf
template <class TraversalEncoderT>
class MeshEdgebreakerEncoderImpl : public MeshEdgebreakerEncoderImplInterface {
 public:
  MeshEdgebreakerEncoderImpl();
  explicit MeshEdgebreakerEncoderImpl(
      const TraversalEncoderT &traversal_encoder);
  bool Init(MeshEdgebreakerEncoder *encoder) override;

  const MeshAttributeCornerTable *GetAttributeCornerTable(
      int att_id) const override;
  const MeshAttributeIndicesEncodingData *GetAttributeEncodingData(
      int att_id) const override;

  bool GenerateAttributesEncoder(int32_t att_id) override;
  bool EncodeAttributesEncoderIdentifier(int32_t att_encoder_id) override;
  Status EncodeConnectivity() override;

  const CornerTable *GetCornerTable() const override {
    return corner_table_.get();
  }
  bool IsFaceEncoded(FaceIndex fi) const override {
    return visited_faces_[fi.value()];
  }
  MeshEdgebreakerEncoder *GetEncoder() const override { return encoder_; }

 private:
  // Initializes data needed for encoding non-position attributes.
  // Returns false on error.
  bool InitAttributeData();

  // Creates a vertex traversal sequencer for the specified |TraverserT| type.
  template <class TraverserT>
  std::unique_ptr<PointsSequencer> CreateVertexTraversalSequencer(
      MeshAttributeIndicesEncodingData *encoding_data);

  // Finds the configuration of the initial face that starts the traversal.
  // Configurations are determined by location of holes around the init face
  // and they are described in mesh_edgebreaker_shared.h.
  // Returns true if the face configuration is interior and false if it is
  // exterior.
  bool FindInitFaceConfiguration(FaceIndex face_id,
                                 CornerIndex *out_corner_id) const;

  // Encodes the connectivity between vertices.
  bool EncodeConnectivityFromCorner(CornerIndex corner_id);

  // Encodes all vertices of a hole starting at start_corner_id.
  // The vertex associated with the first corner is encoded only if
  // |encode_first_vertex| is true.
  // Returns the number of encoded hole vertices.
  int EncodeHole(CornerIndex start_corner_id, bool encode_first_vertex);

  // Encodes topology split data.
  // Returns nullptr on error.
  bool EncodeSplitData();

  CornerIndex GetRightCorner(CornerIndex corner_id) const;
  CornerIndex GetLeftCorner(CornerIndex corner_id) const;

  bool IsRightFaceVisited(CornerIndex corner_id) const;
  bool IsLeftFaceVisited(CornerIndex corner_id) const;
  bool IsVertexVisited(VertexIndex vert_id) const {
    return visited_vertex_ids_[vert_id.value()];
  }

  // Finds and stores data about all holes in the input mesh.
  bool FindHoles();

  // For faces encoded with symbol TOPOLOGY_S (split), this method returns
  // the encoded symbol id or -1 if the face wasn't encoded by a split symbol.
  int GetSplitSymbolIdOnFace(int face_id) const;

  // Checks whether there is a topology split event on a neighboring face and
  // stores the event data if necessary. For more info about topology split
  // events, see description of TopologySplitEventData in
  // mesh_edgebreaker_shared.h.
  void CheckAndStoreTopologySplitEvent(int src_symbol_id, int src_face_id,
                                       EdgeFaceName src_edge,
                                       int neighbor_face_id);

  // Encodes connectivity of all attributes on a newly traversed face.
  bool EncodeAttributeConnectivitiesOnFace(CornerIndex corner);

  // This function is used to to assign correct encoding order of attributes
  // to unprocessed corners. The encoding order is equal to the order in which
  // the attributes are going to be processed by the decoder and it is necessary
  // for proper prediction of attribute values.
  bool AssignPositionEncodingOrderToAllCorners();

  // This function is used to generate encoding order for all non-position
  // attributes.
  // Returns false when one or more attributes failed to be processed.
  bool GenerateEncodingOrderForAttributes();

  // The main encoder that owns this class.
  MeshEdgebreakerEncoder *encoder_;
  // Mesh that's being encoded.
  const Mesh *mesh_;
  // Corner table stores the mesh face connectivity data.
  std::unique_ptr<CornerTable> corner_table_;
  // Stack used for storing corners that need to be traversed when encoding
  // the connectivity. New corner is added for each initial face and a split
  // symbol, and one corner is removed when the end symbol is reached.
  // Stored as member variable to prevent frequent memory reallocations when
  // handling meshes with lots of disjoint components. Originally, we used
  // recursive functions to handle this behavior, but that can cause stack
  // memory overflow when compressing huge meshes.
  std::vector<CornerIndex> corner_traversal_stack_;
  // Array for marking visited faces.
  std::vector<bool> visited_faces_;

  // Attribute data for position encoding.
  MeshAttributeIndicesEncodingData pos_encoding_data_;

  // Traversal method used for the position attribute.
  MeshTraversalMethod pos_traversal_method_;

  // Array storing corners in the order they were visited during the
  // connectivity encoding (always storing the tip corner of each newly visited
  // face).
  std::vector<CornerIndex> processed_connectivity_corners_;

  // Array for storing visited vertex ids of all input vertices.
  std::vector<bool> visited_vertex_ids_;

  // For each traversal, this array stores the number of visited vertices.
  std::vector<int> vertex_traversal_length_;
  // Array for storing all topology split events encountered during the mesh
  // traversal.
  std::vector<TopologySplitEventData> topology_split_event_data_;
  // Map between face_id and symbol_id. Contains entries only for faces that
  // were encoded with TOPOLOGY_S symbol.
  std::unordered_map<int, int> face_to_split_symbol_map_;

  // Array for marking holes that has been reached during the traversal.
  std::vector<bool> visited_holes_;
  // Array for mapping vertices to hole ids. If a vertex is not on a hole, the
  // stored value is -1.
  std::vector<int> vertex_hole_id_;

  // Id of the last encoded symbol.
  int last_encoded_symbol_id_;

  // The number of encoded split symbols.
  uint32_t num_split_symbols_;

  // Struct holding data used for encoding each non-position attribute.
  // TODO(ostava): This should be probably renamed to something better.
  struct AttributeData {
    AttributeData() : attribute_index(-1), is_connectivity_used(true) {}
    int attribute_index;
    MeshAttributeCornerTable connectivity_data;
    // Flag that can mark the connectivity_data invalid. In such case the base
    // corner table of the mesh should be used instead.
    bool is_connectivity_used;
    // Data about attribute encoding order.
    MeshAttributeIndicesEncodingData encoding_data;
    // Traversal method used to generate the encoding data for this attribute.
    MeshTraversalMethod traversal_method;
  };
  std::vector<AttributeData> attribute_data_;

  // Array storing mapping between attribute encoder id and attribute data id.
  std::vector<int32_t> attribute_encoder_to_data_id_map_;

  TraversalEncoderT traversal_encoder_;

  // If set, the encoder is going to use the same connectivity for all
  // attributes. This effectively breaks the mesh along all attribute seams.
  // In general, this approach should be much faster compared to encoding each
  // connectivity separately, but the decoded model may contain higher number of
  // duplicate attribute values which may decrease the compression ratio.
  bool use_single_connectivity_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_ENCODER_IMPL_H_
