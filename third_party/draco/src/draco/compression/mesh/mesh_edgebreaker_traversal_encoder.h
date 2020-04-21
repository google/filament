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
#ifndef DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_TRAVERSAL_ENCODER_H_
#define DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_TRAVERSAL_ENCODER_H_

#include "draco/compression/bit_coders/rans_bit_encoder.h"
#include "draco/compression/mesh/mesh_edgebreaker_encoder.h"
#include "draco/compression/mesh/mesh_edgebreaker_encoder_impl_interface.h"
#include "draco/core/macros.h"

namespace draco {

typedef RAnsBitEncoder BinaryEncoder;

// Default implementation of the edgebreaker traversal encoder. Face
// configurations are stored directly into the output buffer and the symbols
// are first collected and then encoded in the reverse order to make the
// decoding faster.
class MeshEdgebreakerTraversalEncoder {
 public:
  MeshEdgebreakerTraversalEncoder()
      : encoder_impl_(nullptr),
        attribute_connectivity_encoders_(nullptr),
        num_attribute_data_(0) {}
  bool Init(MeshEdgebreakerEncoderImplInterface *encoder) {
    encoder_impl_ = encoder;
    return true;
  }

  // Set the number of non-position attribute data for which we need to encode
  // the connectivity.
  void SetNumAttributeData(int num_data) { num_attribute_data_ = num_data; }

  // Called before the traversal encoding is started.
  void Start() {
    start_face_encoder_.StartEncoding();
    if (num_attribute_data_ > 0) {
      // Init and start arithmetic encoders for storing configuration types
      // of non-position attributes.
      attribute_connectivity_encoders_ = std::unique_ptr<BinaryEncoder[]>(
          new BinaryEncoder[num_attribute_data_]);
      for (int i = 0; i < num_attribute_data_; ++i) {
        attribute_connectivity_encoders_[i].StartEncoding();
      }
    }
  }

  // Called when a traversal starts from a new initial face.
  inline void EncodeStartFaceConfiguration(bool interior) {
    start_face_encoder_.EncodeBit(interior);
  }

  // Called when a new corner is reached during the traversal. No-op for the
  // default encoder.
  inline void NewCornerReached(CornerIndex /* corner */) {}

  // Called whenever a new symbol is reached during the edgebreaker traversal.
  inline void EncodeSymbol(EdgebreakerTopologyBitPattern symbol) {
    // Store the symbol. It will be encoded after all symbols are processed.
    symbols_.push_back(symbol);
  }

  // Called for every pair of connected and visited faces. |is_seam| specifies
  // whether there is an attribute seam between the two faces.

  inline void EncodeAttributeSeam(int attribute, bool is_seam) {
    attribute_connectivity_encoders_[attribute].EncodeBit(is_seam ? 1 : 0);
  }

  // Called when the traversal is finished.
  void Done() {
    EncodeTraversalSymbols();
    EncodeStartFaces();
    EncodeAttributeSeams();
  }

  // Returns the number of encoded symbols.
  int NumEncodedSymbols() const { return static_cast<int>(symbols_.size()); }

  const EncoderBuffer &buffer() const { return traversal_buffer_; }

 protected:
  void EncodeTraversalSymbols() {
    // Bit encode the collected symbols.
    // Allocate enough storage for the bit encoder.
    // It's guaranteed that each face will need only up to 3 bits.
    traversal_buffer_.StartBitEncoding(
        encoder_impl_->GetEncoder()->mesh()->num_faces() * 3, true);
    for (int i = static_cast<int>(symbols_.size() - 1); i >= 0; --i) {
      traversal_buffer_.EncodeLeastSignificantBits32(
          edge_breaker_topology_bit_pattern_length[symbols_[i]], symbols_[i]);
    }
    traversal_buffer_.EndBitEncoding();
  }

  void EncodeStartFaces() {
    start_face_encoder_.EndEncoding(&traversal_buffer_);
  }

  void EncodeAttributeSeams() {
    if (attribute_connectivity_encoders_ != nullptr) {
      for (int i = 0; i < num_attribute_data_; ++i) {
        attribute_connectivity_encoders_[i].EndEncoding(&traversal_buffer_);
      }
    }
  }

  EncoderBuffer *GetOutputBuffer() { return &traversal_buffer_; }
  const MeshEdgebreakerEncoderImplInterface *encoder_impl() const {
    return encoder_impl_;
  }

 private:
  BinaryEncoder start_face_encoder_;
  EncoderBuffer traversal_buffer_;
  const MeshEdgebreakerEncoderImplInterface *encoder_impl_;
  // Symbols collected during the traversal.
  std::vector<EdgebreakerTopologyBitPattern> symbols_;
  // Arithmetic encoder for encoding attribute seams.
  // One context for each non-position attribute.
  std::unique_ptr<BinaryEncoder[]> attribute_connectivity_encoders_;
  int num_attribute_data_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_MESH_MESH_EDGEBREAKER_TRAVERSAL_ENCODER_H_
