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
#ifndef DRACO_COMPRESSION_POINT_CLOUD_ALGORITHMS_DYNAMIC_INTEGER_POINTS_KD_TREE_ENCODER_H_
#define DRACO_COMPRESSION_POINT_CLOUD_ALGORITHMS_DYNAMIC_INTEGER_POINTS_KD_TREE_ENCODER_H_

#include <algorithm>
#include <array>
#include <memory>
#include <stack>
#include <vector>

#include "draco/compression/bit_coders/adaptive_rans_bit_encoder.h"
#include "draco/compression/bit_coders/direct_bit_encoder.h"
#include "draco/compression/bit_coders/folded_integer_bit_encoder.h"
#include "draco/compression/bit_coders/rans_bit_encoder.h"
#include "draco/compression/point_cloud/algorithms/point_cloud_types.h"
#include "draco/core/bit_utils.h"
#include "draco/core/encoder_buffer.h"
#include "draco/core/math_utils.h"

namespace draco {

// This policy class provides several configurations for the encoder that allow
// to trade speed vs compression rate. Level 0 is fastest while 6 is the best
// compression rate. The decoder must select the same level.
template <int compression_level_t>
struct DynamicIntegerPointsKdTreeEncoderCompressionPolicy
    : public DynamicIntegerPointsKdTreeEncoderCompressionPolicy<
          compression_level_t - 1> {};

template <>
struct DynamicIntegerPointsKdTreeEncoderCompressionPolicy<0> {
  typedef DirectBitEncoder NumbersEncoder;
  typedef DirectBitEncoder AxisEncoder;
  typedef DirectBitEncoder HalfEncoder;
  typedef DirectBitEncoder RemainingBitsEncoder;
  static constexpr bool select_axis = false;
};

template <>
struct DynamicIntegerPointsKdTreeEncoderCompressionPolicy<2>
    : public DynamicIntegerPointsKdTreeEncoderCompressionPolicy<1> {
  typedef RAnsBitEncoder NumbersEncoder;
};

template <>
struct DynamicIntegerPointsKdTreeEncoderCompressionPolicy<4>
    : public DynamicIntegerPointsKdTreeEncoderCompressionPolicy<3> {
  typedef FoldedBit32Encoder<RAnsBitEncoder> NumbersEncoder;
};

template <>
struct DynamicIntegerPointsKdTreeEncoderCompressionPolicy<6>
    : public DynamicIntegerPointsKdTreeEncoderCompressionPolicy<5> {
  static constexpr bool select_axis = true;
};

// This class encodes a given integer point cloud based on the point cloud
// compression algorithm in:
// Olivier Devillers and Pierre-Marie Gandoin
// "Geometric compression for interactive transmission"
//
// In principle the algorithm keeps on splitting the point cloud in the middle
// while alternating the axes. In 3D this results in an Octree like structure.
// In each step we encode the number of points in the first half.
// The algorithm does not preserve the order of points.
//
// However, the algorithm here differs from the original as follows:
// The algorithm keeps on splitting the point cloud in the middle of the axis
// that keeps the point cloud as clustered as possible, which gives a better
// compression rate.
// The number of points is encode by the deviation from the half of the points
// in the smaller half of the two. This results in a better compression rate as
// there are more leading zeros, which is then compressed better by the
// arithmetic encoding.
template <int compression_level_t>
class DynamicIntegerPointsKdTreeEncoder {
  static_assert(compression_level_t >= 0, "Compression level must in [0..6].");
  static_assert(compression_level_t <= 6, "Compression level must in [0..6].");
  typedef DynamicIntegerPointsKdTreeEncoderCompressionPolicy<
      compression_level_t>
      Policy;
  typedef typename Policy::NumbersEncoder NumbersEncoder;
  typedef typename Policy::AxisEncoder AxisEncoder;
  typedef typename Policy::HalfEncoder HalfEncoder;
  typedef typename Policy::RemainingBitsEncoder RemainingBitsEncoder;
  typedef std::vector<uint32_t> VectorUint32;

 public:
  explicit DynamicIntegerPointsKdTreeEncoder(uint32_t dimension)
      : bit_length_(0),
        dimension_(dimension),
        deviations_(dimension, 0),
        num_remaining_bits_(dimension, 0),
        axes_(dimension, 0),
        base_stack_(32 * dimension + 1, VectorUint32(dimension, 0)),
        levels_stack_(32 * dimension + 1, VectorUint32(dimension, 0)) {}

  // Encodes an integer point cloud given by [begin,end) into buffer.
  // |bit_length| gives the highest bit used for all coordinates.
  template <class RandomAccessIteratorT>
  bool EncodePoints(RandomAccessIteratorT begin, RandomAccessIteratorT end,
                    const uint32_t &bit_length, EncoderBuffer *buffer);

  // Encodes an integer point cloud given by [begin,end) into buffer.
  template <class RandomAccessIteratorT>
  bool EncodePoints(RandomAccessIteratorT begin, RandomAccessIteratorT end,
                    EncoderBuffer *buffer) {
    return EncodePoints(begin, end, 32, buffer);
  }

  const uint32_t dimension() const { return dimension_; }

 private:
  template <class RandomAccessIteratorT>
  uint32_t GetAndEncodeAxis(RandomAccessIteratorT begin,
                            RandomAccessIteratorT end,
                            const VectorUint32 &old_base,
                            const VectorUint32 &levels, uint32_t last_axis);
  template <class RandomAccessIteratorT>
  void EncodeInternal(RandomAccessIteratorT begin, RandomAccessIteratorT end);

  class Splitter {
   public:
    Splitter(uint32_t axis, uint32_t value) : axis_(axis), value_(value) {}
    template <class PointT>
    bool operator()(const PointT &a) const {
      return a[axis_] < value_;
    }

   private:
    const uint32_t axis_;
    const uint32_t value_;
  };

  void EncodeNumber(int nbits, uint32_t value) {
    numbers_encoder_.EncodeLeastSignificantBits32(nbits, value);
  }

  template <class RandomAccessIteratorT>
  struct EncodingStatus {
    EncodingStatus(RandomAccessIteratorT begin_, RandomAccessIteratorT end_,
                   uint32_t last_axis_, uint32_t stack_pos_)
        : begin(begin_),
          end(end_),
          last_axis(last_axis_),
          stack_pos(stack_pos_) {
      num_remaining_points = static_cast<uint32_t>(end - begin);
    }

    RandomAccessIteratorT begin;
    RandomAccessIteratorT end;
    uint32_t last_axis;
    uint32_t num_remaining_points;
    uint32_t stack_pos;  // used to get base and levels
  };

  uint32_t bit_length_;
  uint32_t num_points_;
  uint32_t dimension_;
  NumbersEncoder numbers_encoder_;
  RemainingBitsEncoder remaining_bits_encoder_;
  AxisEncoder axis_encoder_;
  HalfEncoder half_encoder_;
  VectorUint32 deviations_;
  VectorUint32 num_remaining_bits_;
  VectorUint32 axes_;
  std::vector<VectorUint32> base_stack_;
  std::vector<VectorUint32> levels_stack_;
};

template <int compression_level_t>
template <class RandomAccessIteratorT>
bool DynamicIntegerPointsKdTreeEncoder<compression_level_t>::EncodePoints(
    RandomAccessIteratorT begin, RandomAccessIteratorT end,
    const uint32_t &bit_length, EncoderBuffer *buffer) {
  bit_length_ = bit_length;
  num_points_ = static_cast<uint32_t>(end - begin);

  buffer->Encode(bit_length_);
  buffer->Encode(num_points_);
  if (num_points_ == 0) {
    return true;
  }

  numbers_encoder_.StartEncoding();
  remaining_bits_encoder_.StartEncoding();
  axis_encoder_.StartEncoding();
  half_encoder_.StartEncoding();

  EncodeInternal(begin, end);

  numbers_encoder_.EndEncoding(buffer);
  remaining_bits_encoder_.EndEncoding(buffer);
  axis_encoder_.EndEncoding(buffer);
  half_encoder_.EndEncoding(buffer);

  return true;
}
template <int compression_level_t>
template <class RandomAccessIteratorT>
uint32_t
DynamicIntegerPointsKdTreeEncoder<compression_level_t>::GetAndEncodeAxis(
    RandomAccessIteratorT begin, RandomAccessIteratorT end,
    const VectorUint32 &old_base, const VectorUint32 &levels,
    uint32_t last_axis) {
  if (!Policy::select_axis) {
    return DRACO_INCREMENT_MOD(last_axis, dimension_);
  }

  // For many points this function selects the axis that should be used
  // for the split by keeping as many points as possible bundled.
  // In the best case we do not split the point cloud at all.
  // For lower number of points, we simply choose the axis that is refined the
  // least so far.

  DRACO_DCHECK_EQ(true, end - begin != 0);

  uint32_t best_axis = 0;
  if (end - begin < 64) {
    for (uint32_t axis = 1; axis < dimension_; ++axis) {
      if (levels[best_axis] > levels[axis]) {
        best_axis = axis;
      }
    }
  } else {
    const uint32_t size = static_cast<uint32_t>(end - begin);
    for (uint32_t i = 0; i < dimension_; i++) {
      deviations_[i] = 0;
      num_remaining_bits_[i] = bit_length_ - levels[i];
      if (num_remaining_bits_[i] > 0) {
        const uint32_t split =
            old_base[i] + (1 << (num_remaining_bits_[i] - 1));
        for (auto it = begin; it != end; ++it) {
          deviations_[i] += ((*it)[i] < split);
        }
        deviations_[i] = std::max(size - deviations_[i], deviations_[i]);
      }
    }

    uint32_t max_value = 0;
    best_axis = 0;
    for (uint32_t i = 0; i < dimension_; i++) {
      // If axis can be subdivided.
      if (num_remaining_bits_[i]) {
        // Check if this is the better axis.
        if (max_value < deviations_[i]) {
          max_value = deviations_[i];
          best_axis = i;
        }
      }
    }
    axis_encoder_.EncodeLeastSignificantBits32(4, best_axis);
  }

  return best_axis;
}

template <int compression_level_t>
template <class RandomAccessIteratorT>
void DynamicIntegerPointsKdTreeEncoder<compression_level_t>::EncodeInternal(
    RandomAccessIteratorT begin, RandomAccessIteratorT end) {
  typedef EncodingStatus<RandomAccessIteratorT> Status;

  base_stack_[0] = VectorUint32(dimension_, 0);
  levels_stack_[0] = VectorUint32(dimension_, 0);
  Status init_status(begin, end, 0, 0);
  std::stack<Status> status_stack;
  status_stack.push(init_status);

  // TODO(b/199760123): Use preallocated vector instead of stack.
  while (!status_stack.empty()) {
    Status status = status_stack.top();
    status_stack.pop();

    begin = status.begin;
    end = status.end;
    const uint32_t last_axis = status.last_axis;
    const uint32_t stack_pos = status.stack_pos;
    const VectorUint32 &old_base = base_stack_[stack_pos];
    const VectorUint32 &levels = levels_stack_[stack_pos];

    const uint32_t axis =
        GetAndEncodeAxis(begin, end, old_base, levels, last_axis);
    const uint32_t level = levels[axis];
    const uint32_t num_remaining_points = static_cast<uint32_t>(end - begin);

    // If this happens all axis are subdivided to the end.
    if ((bit_length_ - level) == 0) {
      continue;
    }

    // Fast encoding of remaining bits if number of points is 1 or 2.
    // Doing this also for 2 gives a slight additional speed up.
    if (num_remaining_points <= 2) {
      // TODO(b/199760123): |axes_| not necessary, remove would change
      // bitstream!
      axes_[0] = axis;
      for (uint32_t i = 1; i < dimension_; i++) {
        axes_[i] = DRACO_INCREMENT_MOD(axes_[i - 1], dimension_);
      }
      for (uint32_t i = 0; i < num_remaining_points; ++i) {
        const auto &p = *(begin + i);
        for (uint32_t j = 0; j < dimension_; j++) {
          const uint32_t num_remaining_bits = bit_length_ - levels[axes_[j]];
          if (num_remaining_bits) {
            remaining_bits_encoder_.EncodeLeastSignificantBits32(
                num_remaining_bits, p[axes_[j]]);
          }
        }
      }
      continue;
    }

    const uint32_t num_remaining_bits = bit_length_ - level;
    const uint32_t modifier = 1 << (num_remaining_bits - 1);
    base_stack_[stack_pos + 1] = old_base;  // copy
    base_stack_[stack_pos + 1][axis] += modifier;
    const VectorUint32 &new_base = base_stack_[stack_pos + 1];

    const RandomAccessIteratorT split =
        std::partition(begin, end, Splitter(axis, new_base[axis]));

    DRACO_DCHECK_EQ(true, (end - begin) > 0);

    // Encode number of points in first and second half.
    const int required_bits = MostSignificantBit(num_remaining_points);

    const uint32_t first_half = static_cast<uint32_t>(split - begin);
    const uint32_t second_half = static_cast<uint32_t>(end - split);
    const bool left = first_half < second_half;

    if (first_half != second_half) {
      half_encoder_.EncodeBit(left);
    }

    if (left) {
      EncodeNumber(required_bits, num_remaining_points / 2 - first_half);
    } else {
      EncodeNumber(required_bits, num_remaining_points / 2 - second_half);
    }

    levels_stack_[stack_pos][axis] += 1;
    levels_stack_[stack_pos + 1] = levels_stack_[stack_pos];  // copy
    if (split != begin) {
      status_stack.push(Status(begin, split, axis, stack_pos));
    }
    if (split != end) {
      status_stack.push(Status(split, end, axis, stack_pos + 1));
    }
  }
}
extern template class DynamicIntegerPointsKdTreeEncoder<0>;
extern template class DynamicIntegerPointsKdTreeEncoder<2>;
extern template class DynamicIntegerPointsKdTreeEncoder<4>;
extern template class DynamicIntegerPointsKdTreeEncoder<6>;

}  // namespace draco

#endif  // DRACO_COMPRESSION_POINT_CLOUD_ALGORITHMS_DYNAMIC_INTEGER_POINTS_KD_TREE_ENCODER_H_
