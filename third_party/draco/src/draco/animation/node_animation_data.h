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
#ifndef DRACO_ANIMATION_NODE_ANIMATION_DATA_H_
#define DRACO_ANIMATION_NODE_ANIMATION_DATA_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

#include "draco/core/hash_utils.h"
#include "draco/core/status.h"
#include "draco/core/status_or.h"

namespace draco {

// This class is used to store information and data for animations that only
// affect the nodes.
// TODO(fgalligan): Think about changing the name of this class now that Skin
// is using it.
class NodeAnimationData {
 public:
  enum class Type { SCALAR, VEC3, VEC4, MAT4 };

  NodeAnimationData() : type_(Type::SCALAR), count_(0), normalized_(false) {}

  void Copy(const NodeAnimationData &src) {
    type_ = src.type_;
    count_ = src.count_;
    normalized_ = src.normalized_;
    data_ = src.data_;
  }

  Type type() const { return type_; }
  int count() const { return count_; }
  bool normalized() const { return normalized_; }

  std::vector<float> *GetMutableData() { return &data_; }
  const std::vector<float> *GetData() const { return &data_; }

  void SetType(Type type) { type_ = type; }
  void SetCount(int count) { count_ = count; }
  void SetNormalized(bool normalized) { normalized_ = normalized; }

  int ComponentSize() const { return sizeof(float); }
  int NumComponents() const {
    switch (type_) {
      case Type::SCALAR:
        return 1;
      case Type::VEC3:
        return 3;
      case Type::MAT4:
        return 16;
      default:
        return 4;
    }
  }

  std::string TypeAsString() const {
    switch (type_) {
      case Type::SCALAR:
        return "SCALAR";
      case Type::VEC3:
        return "VEC3";
      case Type::MAT4:
        return "MAT4";
      default:
        return "VEC4";
    }
  }

  bool operator==(const NodeAnimationData &nad) const {
    return type_ == nad.type_ && count_ == nad.count_ &&
           normalized_ == nad.normalized_ && data_ == nad.data_;
  }

 private:
  Type type_;
  int count_;
  bool normalized_;
  std::vector<float> data_;
};

// Wrapper class for hashing NodeAnimationData. When using different containers,
// this class is preferable instead of copying the data in NodeAnimationData
// every time.
class NodeAnimationDataHash {
 public:
  NodeAnimationDataHash() = delete;
  NodeAnimationDataHash &operator=(const NodeAnimationDataHash &) = delete;
  NodeAnimationDataHash(NodeAnimationDataHash &&) = delete;
  NodeAnimationDataHash &operator=(NodeAnimationDataHash &&) = delete;

  explicit NodeAnimationDataHash(const NodeAnimationData *nad)
      : node_animation_data_(nad) {
    hash_ = NodeAnimationDataHash::HashNodeAnimationData(*node_animation_data_);
  }

  NodeAnimationDataHash(const NodeAnimationDataHash &nadh) {
    node_animation_data_ = nadh.node_animation_data_;
    hash_ = nadh.hash_;
  }

  bool operator==(const NodeAnimationDataHash &nadh) const {
    return *node_animation_data_ == *nadh.node_animation_data_;
  }

  struct Hash {
    size_t operator()(const NodeAnimationDataHash &nadh) const {
      return nadh.hash_;
    }
  };

  const NodeAnimationData *GetNodeAnimationData() {
    return node_animation_data_;
  }

 private:
  // Returns a hash of |nad|.
  static size_t HashNodeAnimationData(const NodeAnimationData &nad) {
    size_t hash = 79;  // Magic number.
    hash = HashCombine(static_cast<int>(nad.type()), hash);
    hash = HashCombine(nad.count(), hash);
    hash = HashCombine(nad.normalized(), hash);
    const uint64_t data_hash =
        FingerprintString(reinterpret_cast<const char *>(nad.GetData()->data()),
                          nad.GetData()->size() * sizeof(float));
    hash = HashCombine(data_hash, hash);
    return hash;
  }

  const NodeAnimationData *node_animation_data_;
  size_t hash_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_ANIMATION_NODE_ANIMATION_DATA_H_
