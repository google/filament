// Copyright 2016 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_WEIGHTED_RESERVOIR_SAMPLER_H_
#define SRC_WEIGHTED_RESERVOIR_SAMPLER_H_

#include <cassert>
#include <random>

namespace protobuf_mutator {

// Algorithm pick one item from the sequence of weighted items.
// https://en.wikipedia.org/wiki/Reservoir_sampling#Algorithm_A-Chao
//
// Example:
//   WeightedReservoirSampler<int> sampler;
//   for(int i = 0; i < size; ++i)
//     sampler.Pick(weight[i], i);
//   return sampler.GetSelected();
template <class T, class RandomEngine = std::default_random_engine>
class WeightedReservoirSampler {
 public:
  explicit WeightedReservoirSampler(RandomEngine* random) : random_(random) {}

  void Try(uint64_t weight, const T& item) {
    if (Pick(weight)) selected_ = item;
  }

  const T& selected() const { return selected_; }

  bool IsEmpty() const { return total_weight_ == 0; }

 private:
  bool Pick(uint64_t weight) {
    if (weight == 0) return false;
    total_weight_ += weight;
    return weight == total_weight_ || std::uniform_int_distribution<uint64_t>(
                                          1, total_weight_)(*random_) <= weight;
  }

  T selected_ = {};
  uint64_t total_weight_ = 0;
  RandomEngine* random_;
};

}  // namespace protobuf_mutator

#endif  // SRC_WEIGHTED_RESERVOIR_SAMPLER_H_
