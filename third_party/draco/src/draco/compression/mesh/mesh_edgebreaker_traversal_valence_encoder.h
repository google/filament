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
#ifndef DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_TRAVERSAL_VALENCE_ENCODER_H_
#define DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_TRAVERSAL_VALENCE_ENCODER_H_

#include "draco/compression/entropy/symbol_encoding.h"
#include "draco/compression/mesh/mesh_edgebreaker_traversal_encoder.h"
#include "draco/core/varint_encoding.h"

namespace draco {

// Predictive encoder for the Edgebreaker symbols based on valences of the
// previously encoded vertices, following the method described in: Szymczak'02,
// "Optimized Edgebreaker Encoding for Large and Regular Triangle Meshes". Each
// valence is used to specify a different entropy context for encoding of the
// symbols.
// Encoder can operate in various predefined modes that can be used to select
// the way in which the entropy contexts are computed (e.g. using different
// clamping for valences, or even using different inputs to compute the
// contexts), see EdgebreakerValenceCodingMode in mesh_edgebreaker_shared.h for
// a list of supported modes.
class MeshEdgebreakerTraversalValenceEncoder
    : public MeshEdgebreakerTraversalEncoder {
 public:
  MeshEdgebreakerTraversalValenceEncoder()
      : corner_table_(nullptr),
        prev_symbol_(-1),
        last_corner_(kInvalidCornerIndex),
        num_symbols_(0),
        min_valence_(2),
        max_valence_(7) {}

  bool Init(MeshEdgebreakerEncoderImplInterface *encoder) {
    if (!MeshEdgebreakerTraversalEncoder::Init(encoder)) {
      return false;
    }
    min_valence_ = 2;
    max_valence_ = 7;
    corner_table_ = encoder->GetCornerTable();

    // Initialize valences of all vertices.
    vertex_valences_.resize(corner_table_->num_vertices());
    for (VertexIndex i(0); i < static_cast<uint32_t>(vertex_valences_.size());
         ++i) {
      vertex_valences_[i] = corner_table_->Valence(VertexIndex(i));
    }

    // Replicate the corner to vertex map from the corner table. We need to do
    // this because the map may get updated during encoding because we add new
    // vertices when we encounter split symbols.
    corner_to_vertex_map_.resize(corner_table_->num_corners());
    for (CornerIndex i(0); i < corner_table_->num_corners(); ++i) {
      corner_to_vertex_map_[i] = corner_table_->Vertex(i);
    }
    const int32_t num_unique_valences = max_valence_ - min_valence_ + 1;

    context_symbols_.resize(num_unique_valences);
    return true;
  }

  inline void NewCornerReached(CornerIndex corner) { last_corner_ = corner; }

  inline void EncodeSymbol(EdgebreakerTopologyBitPattern symbol) {
    ++num_symbols_;
    // Update valences on the mesh and compute the context that is going to be
    // used to encode the processed symbol.
    // Note that the valences are computed for the so far unencoded part of the
    // mesh (i.e. the decoding is reverse). Adding a new symbol either reduces
    // valences on the vertices or leaves the valence unchanged.

    const CornerIndex next = corner_table_->Next(last_corner_);
    const CornerIndex prev = corner_table_->Previous(last_corner_);

    // Get valence on the tip corner of the active edge (outgoing edge that is
    // going to be used in reverse decoding of the connectivity to predict the
    // next symbol).
    const int active_valence = vertex_valences_[corner_to_vertex_map_[next]];
    switch (symbol) {
      case TOPOLOGY_C:
        // Compute prediction.
        FALLTHROUGH_INTENDED;
      case TOPOLOGY_S:
        // Update valences.
        vertex_valences_[corner_to_vertex_map_[next]] -= 1;
        vertex_valences_[corner_to_vertex_map_[prev]] -= 1;
        if (symbol == TOPOLOGY_S) {
          // Whenever we reach a split symbol, we need to split the vertex into
          // two and attach all corners on the left and right sides of the split
          // vertex to the respective vertices (see image below). This is
          // necessary since the decoder works in the reverse order and it
          // merges the two vertices only after the split symbol is processed.
          //
          //     * -----
          //    / \--------
          //   /   \--------
          //  /     \-------
          // *-------v-------*
          //  \     /c\     /
          //   \   /   \   /
          //    \ /n S p\ /
          //     *.......*
          //

          // Count the number of faces on the left side of the split vertex and
          // update the valence on the "left vertex".
          int num_left_faces = 0;
          CornerIndex act_c = corner_table_->Opposite(prev);
          while (act_c != kInvalidCornerIndex) {
            if (encoder_impl()->IsFaceEncoded(corner_table_->Face(act_c))) {
              break;  // Stop when we reach the first visited face.
            }
            ++num_left_faces;
            act_c = corner_table_->Opposite(corner_table_->Next(act_c));
          }
          vertex_valences_[corner_to_vertex_map_[last_corner_]] =
              num_left_faces + 1;

          // Create a new vertex for the right side and count the number of
          // faces that should be attached to this vertex.
          const int new_vert_id = static_cast<int>(vertex_valences_.size());
          int num_right_faces = 0;

          act_c = corner_table_->Opposite(next);
          while (act_c != kInvalidCornerIndex) {
            if (encoder_impl()->IsFaceEncoded(corner_table_->Face(act_c))) {
              break;  // Stop when we reach the first visited face.
            }
            ++num_right_faces;
            // Map corners on the right side to the newly created vertex.
            corner_to_vertex_map_[corner_table_->Next(act_c)] = new_vert_id;
            act_c = corner_table_->Opposite(corner_table_->Previous(act_c));
          }
          vertex_valences_.push_back(num_right_faces + 1);
        }
        break;
      case TOPOLOGY_R:
        // Update valences.
        vertex_valences_[corner_to_vertex_map_[last_corner_]] -= 1;
        vertex_valences_[corner_to_vertex_map_[next]] -= 1;
        vertex_valences_[corner_to_vertex_map_[prev]] -= 2;
        break;
      case TOPOLOGY_L:

        vertex_valences_[corner_to_vertex_map_[last_corner_]] -= 1;
        vertex_valences_[corner_to_vertex_map_[next]] -= 2;
        vertex_valences_[corner_to_vertex_map_[prev]] -= 1;
        break;
      case TOPOLOGY_E:
        vertex_valences_[corner_to_vertex_map_[last_corner_]] -= 2;
        vertex_valences_[corner_to_vertex_map_[next]] -= 2;
        vertex_valences_[corner_to_vertex_map_[prev]] -= 2;
        break;
      default:
        break;
    }

    if (prev_symbol_ != -1) {
      int clamped_valence;
      if (active_valence < min_valence_) {
        clamped_valence = min_valence_;
      } else if (active_valence > max_valence_) {
        clamped_valence = max_valence_;
      } else {
        clamped_valence = active_valence;
      }

      const int context = clamped_valence - min_valence_;
      context_symbols_[context].push_back(
          edge_breaker_topology_to_symbol_id[prev_symbol_]);
    }

    prev_symbol_ = symbol;
  }

  void Done() {
    // Store the init face configurations and attribute seam data
    MeshEdgebreakerTraversalEncoder::EncodeStartFaces();
    MeshEdgebreakerTraversalEncoder::EncodeAttributeSeams();

    // Store the contexts.
    for (int i = 0; i < context_symbols_.size(); ++i) {
      EncodeVarint<uint32_t>(static_cast<uint32_t>(context_symbols_[i].size()),
                             GetOutputBuffer());
      if (context_symbols_[i].size() > 0) {
        EncodeSymbols(context_symbols_[i].data(),
                      static_cast<int>(context_symbols_[i].size()), 1, nullptr,
                      GetOutputBuffer());
      }
    }
  }

  int NumEncodedSymbols() const { return num_symbols_; }

 private:
  const CornerTable *corner_table_;
  // Explicit map between corners and vertices. We cannot use the one stored
  // in the |corner_table_| because we may need to add additional vertices to
  // handle split symbols.
  IndexTypeVector<CornerIndex, VertexIndex> corner_to_vertex_map_;
  IndexTypeVector<VertexIndex, int> vertex_valences_;
  // Previously encoded symbol.
  int32_t prev_symbol_;
  CornerIndex last_corner_;
  // Explicitly count the number of encoded symbols.
  int num_symbols_;

  int min_valence_;
  int max_valence_;
  std::vector<std::vector<uint32_t>> context_symbols_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_TRAVERSAL_VALENCE_ENCODER_H_
