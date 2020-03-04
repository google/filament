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
#ifndef DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_TRAVERSAL_PREDICTIVE_ENCODER_H_
#define DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_TRAVERSAL_PREDICTIVE_ENCODER_H_

#include "draco/compression/mesh/mesh_edgebreaker_traversal_encoder.h"

namespace draco {

// Encoder that tries to predict the edgebreaker traversal symbols based on the
// vertex valences of the unencoded portion of the mesh. The current prediction
// scheme assumes that each vertex has valence 6 which can be used to predict
// the symbol preceding the one that is currently encoded. Predictions are
// encoded using an arithmetic coding which can lead to less than 1 bit per
// triangle encoding for highly regular meshes.
class MeshEdgebreakerTraversalPredictiveEncoder
    : public MeshEdgebreakerTraversalEncoder {
 public:
  MeshEdgebreakerTraversalPredictiveEncoder()
      : corner_table_(nullptr),
        prev_symbol_(-1),
        num_split_symbols_(0),
        last_corner_(kInvalidCornerIndex),
        num_symbols_(0) {}

  bool Init(MeshEdgebreakerEncoderImplInterface *encoder) {
    if (!MeshEdgebreakerTraversalEncoder::Init(encoder)) {
      return false;
    }
    corner_table_ = encoder->GetCornerTable();
    // Initialize valences of all vertices.
    vertex_valences_.resize(corner_table_->num_vertices());
    for (uint32_t i = 0; i < vertex_valences_.size(); ++i) {
      vertex_valences_[i] = corner_table_->Valence(VertexIndex(i));
    }
    return true;
  }

  inline void NewCornerReached(CornerIndex corner) { last_corner_ = corner; }

  inline int32_t ComputePredictedSymbol(VertexIndex pivot) {
    const int valence = vertex_valences_[pivot.value()];
    if (valence < 0) {
      // This situation can happen only for split vertices. Returning
      // TOPOLOGY_INVALID always cases misprediction.
      return TOPOLOGY_INVALID;
    }
    if (valence < 6) {
      return TOPOLOGY_R;
    }
    return TOPOLOGY_C;
  }

  inline void EncodeSymbol(EdgebreakerTopologyBitPattern symbol) {
    ++num_symbols_;
    // Update valences on the mesh. And compute the predicted preceding symbol.
    // Note that the valences are computed for the so far unencoded part of the
    // mesh. Adding a new symbol either reduces valences on the vertices or
    // leaves the valence unchanged.
    int32_t predicted_symbol = -1;
    const CornerIndex next = corner_table_->Next(last_corner_);
    const CornerIndex prev = corner_table_->Previous(last_corner_);
    switch (symbol) {
      case TOPOLOGY_C:
        // Compute prediction.
        predicted_symbol = ComputePredictedSymbol(corner_table_->Vertex(next));
        FALLTHROUGH_INTENDED;
      case TOPOLOGY_S:
        // Update valences.
        vertex_valences_[corner_table_->Vertex(next).value()] -= 1;
        vertex_valences_[corner_table_->Vertex(prev).value()] -= 1;
        if (symbol == TOPOLOGY_S) {
          // Whenever we reach a split symbol, mark its tip vertex as invalid by
          // setting the valence to a negative value. Any prediction that will
          // use this vertex will then cause a misprediction. This is currently
          // necessary because the decoding works in the reverse direction and
          // the decoder doesn't know about these vertices until the split
          // symbol is decoded at which point two vertices are merged into one.
          // This can be most likely solved on the encoder side by splitting the
          // tip vertex into two, but since split symbols are relatively rare,
          // it's probably not worth doing it.
          vertex_valences_[corner_table_->Vertex(last_corner_).value()] = -1;
          ++num_split_symbols_;
        }
        break;
      case TOPOLOGY_R:
        // Compute prediction.
        predicted_symbol = ComputePredictedSymbol(corner_table_->Vertex(next));
        // Update valences.
        vertex_valences_[corner_table_->Vertex(last_corner_).value()] -= 1;
        vertex_valences_[corner_table_->Vertex(next).value()] -= 1;
        vertex_valences_[corner_table_->Vertex(prev).value()] -= 2;
        break;
      case TOPOLOGY_L:
        vertex_valences_[corner_table_->Vertex(last_corner_).value()] -= 1;
        vertex_valences_[corner_table_->Vertex(next).value()] -= 2;
        vertex_valences_[corner_table_->Vertex(prev).value()] -= 1;
        break;
      case TOPOLOGY_E:
        vertex_valences_[corner_table_->Vertex(last_corner_).value()] -= 2;
        vertex_valences_[corner_table_->Vertex(next).value()] -= 2;
        vertex_valences_[corner_table_->Vertex(prev).value()] -= 2;
        break;
      default:
        break;
    }
    // Flag used when it's necessary to explicitly store the previous symbol.
    bool store_prev_symbol = true;
    if (predicted_symbol != -1) {
      if (predicted_symbol == prev_symbol_) {
        predictions_.push_back(true);
        store_prev_symbol = false;
      } else if (prev_symbol_ != -1) {
        predictions_.push_back(false);
      }
    }
    if (store_prev_symbol && prev_symbol_ != -1) {
      MeshEdgebreakerTraversalEncoder::EncodeSymbol(
          static_cast<EdgebreakerTopologyBitPattern>(prev_symbol_));
    }
    prev_symbol_ = symbol;
  }

  void Done() {
    // We still need to store the last encoded symbol.
    if (prev_symbol_ != -1) {
      MeshEdgebreakerTraversalEncoder::EncodeSymbol(
          static_cast<EdgebreakerTopologyBitPattern>(prev_symbol_));
    }
    // Store the init face configurations and the explicitly encoded symbols.
    MeshEdgebreakerTraversalEncoder::Done();
    // Encode the number of split symbols.
    GetOutputBuffer()->Encode(num_split_symbols_);
    // Store the predictions.
    BinaryEncoder prediction_encoder;
    prediction_encoder.StartEncoding();
    for (int i = static_cast<int>(predictions_.size()) - 1; i >= 0; --i) {
      prediction_encoder.EncodeBit(predictions_[i]);
    }
    prediction_encoder.EndEncoding(GetOutputBuffer());
  }

  int NumEncodedSymbols() const { return num_symbols_; }

 private:
  const CornerTable *corner_table_;
  std::vector<int> vertex_valences_;
  std::vector<bool> predictions_;
  // Previously encoded symbol.
  int32_t prev_symbol_;
  // The total number of encoded split symbols.
  int32_t num_split_symbols_;
  CornerIndex last_corner_;
  // Explicitly count the number of encoded symbols.
  int num_symbols_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_TRAVERSAL_PREDICTIVE_ENCODER_H_
