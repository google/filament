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
// TODO(b/199760123): Make this a wrapper using
// DynamicIntegerPointsKdTreeEncoder.
#ifndef DRACO_COMPRESSION_POINT_CLOUD_ALGORITHMS_INTEGER_POINTS_KD_TREE_ENCODER_H_
#define DRACO_COMPRESSION_POINT_CLOUD_ALGORITHMS_INTEGER_POINTS_KD_TREE_ENCODER_H_

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "draco/compression/bit_coders/adaptive_rans_bit_encoder.h"
#include "draco/compression/bit_coders/direct_bit_encoder.h"
#include "draco/compression/bit_coders/folded_integer_bit_encoder.h"
#include "draco/compression/bit_coders/rans_bit_encoder.h"
#include "draco/compression/point_cloud/algorithms/point_cloud_types.h"
#include "draco/compression/point_cloud/algorithms/queuing_policy.h"
#include "draco/core/bit_utils.h"
#include "draco/core/encoder_buffer.h"
#include "draco/core/math_utils.h"

namespace draco {

// This policy class provides several configurations for the encoder that allow
// to trade speed vs compression rate. Level 0 is fastest while 10 is the best
// compression rate. The decoder must select the same level.
template <int compression_level_t>
struct IntegerPointsKdTreeEncoderCompressionPolicy
    : public IntegerPointsKdTreeEncoderCompressionPolicy<compression_level_t -
                                                         1> {};

template <>
struct IntegerPointsKdTreeEncoderCompressionPolicy<0> {
  typedef DirectBitEncoder NumbersEncoder;
  typedef DirectBitEncoder AxisEncoder;
  typedef DirectBitEncoder HalfEncoder;
  typedef DirectBitEncoder RemainingBitsEncoder;
  static constexpr bool select_axis = false;

  template <class T>
  using QueuingStrategy = Stack<T>;
};

template <>
struct IntegerPointsKdTreeEncoderCompressionPolicy<2>
    : public IntegerPointsKdTreeEncoderCompressionPolicy<1> {
  typedef RAnsBitEncoder NumbersEncoder;
};

template <>
struct IntegerPointsKdTreeEncoderCompressionPolicy<4>
    : public IntegerPointsKdTreeEncoderCompressionPolicy<3> {
  typedef FoldedBit32Encoder<RAnsBitEncoder> NumbersEncoder;
};

template <>
struct IntegerPointsKdTreeEncoderCompressionPolicy<6>
    : public IntegerPointsKdTreeEncoderCompressionPolicy<5> {
  static constexpr bool select_axis = true;
};

template <>
struct IntegerPointsKdTreeEncoderCompressionPolicy<8>
    : public IntegerPointsKdTreeEncoderCompressionPolicy<7> {
  typedef FoldedBit32Encoder<AdaptiveRAnsBitEncoder> NumbersEncoder;
  template <class T>
  using QueuingStrategy = Queue<T>;
};

template <>
struct IntegerPointsKdTreeEncoderCompressionPolicy<10>
    : public IntegerPointsKdTreeEncoderCompressionPolicy<9> {
  template <class T>
  using QueuingStrategy = PriorityQueue<T>;
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
//
// |PointDiT| is a type representing a point with uint32_t coordinates.
// must provide construction from three uint32_t and operator[].
template <class PointDiT, int compression_level_t>
class IntegerPointsKdTreeEncoder {
  typedef IntegerPointsKdTreeEncoderCompressionPolicy<compression_level_t>
      Policy;
  typedef typename Policy::NumbersEncoder NumbersEncoder;
  typedef typename Policy::AxisEncoder AxisEncoder;
  typedef typename Policy::HalfEncoder HalfEncoder;
  typedef typename Policy::RemainingBitsEncoder RemainingBitsEncoder;

 public:
  IntegerPointsKdTreeEncoder() : bit_length_(0) {}

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

 private:
  // For the sack of readability of code, we decided to make this exception
  // from the naming scheme.
  static constexpr int D = PointTraits<PointDiT>::Dimension();
  template <class RandomAccessIteratorT>
  uint32_t GetAxis(RandomAccessIteratorT begin, RandomAccessIteratorT end,
                   const PointDiT &old_base, std::array<uint32_t, D> levels,
                   uint32_t last_axis);

  template <class RandomAccessIteratorT>
  void EncodeInternal(RandomAccessIteratorT begin, RandomAccessIteratorT end,
                      PointDiT old_base, std::array<uint32_t, D> levels,
                      uint32_t last_axis);

  class Splitter {
   public:
    Splitter(int axis, uint32_t value) : axis_(axis), value_(value) {}
    bool operator()(const PointDiT &a) { return a[axis_] < value_; }

   private:
    int axis_;
    uint32_t value_;
  };

  void EncodeNumber(int nbits, uint32_t value) {
    numbers_encoder_.EncodeLeastSignificantBits32(nbits, value);
  }

  template <class RandomAccessIteratorT>
  struct EncodingStatus {
    EncodingStatus(
        RandomAccessIteratorT begin_, RandomAccessIteratorT end_,
        const PointDiT &old_base_,
        std::array<uint32_t, PointTraits<PointDiT>::Dimension()> levels_,
        uint32_t last_axis_)
        : begin(begin_),
          end(end_),
          old_base(old_base_),
          levels(levels_),
          last_axis(last_axis_) {
      num_remaining_points = end - begin;
    }

    RandomAccessIteratorT begin;
    RandomAccessIteratorT end;
    PointDiT old_base;
    std::array<uint32_t, D> levels;
    uint32_t last_axis;
    uint32_t num_remaining_points;
    friend bool operator<(const EncodingStatus &l, const EncodingStatus &r) {
      return l.num_remaining_points < r.num_remaining_points;
    }
  };

  uint32_t bit_length_;
  uint32_t num_points_;
  NumbersEncoder numbers_encoder_;
  RemainingBitsEncoder remaining_bits_encoder_;
  AxisEncoder axis_encoder_;
  HalfEncoder half_encoder_;
};

template <class PointDiT, int compression_level_t>
template <class RandomAccessIteratorT>
bool IntegerPointsKdTreeEncoder<PointDiT, compression_level_t>::EncodePoints(
    RandomAccessIteratorT begin, RandomAccessIteratorT end,
    const uint32_t &bit_length, EncoderBuffer *buffer) {
  bit_length_ = bit_length;
  num_points_ = end - begin;

  buffer->Encode(bit_length_);
  buffer->Encode(num_points_);
  if (num_points_ == 0) {
    return true;
  }

  numbers_encoder_.StartEncoding();
  remaining_bits_encoder_.StartEncoding();
  axis_encoder_.StartEncoding();
  half_encoder_.StartEncoding();

  EncodeInternal(begin, end, PointTraits<PointDiT>::Origin(),
                 PointTraits<PointDiT>::ZeroArray(), 0);

  numbers_encoder_.EndEncoding(buffer);
  remaining_bits_encoder_.EndEncoding(buffer);
  axis_encoder_.EndEncoding(buffer);
  half_encoder_.EndEncoding(buffer);

  return true;
}
template <class PointDiT, int compression_level_t>
template <class RandomAccessIteratorT>
uint32_t IntegerPointsKdTreeEncoder<PointDiT, compression_level_t>::GetAxis(
    RandomAccessIteratorT begin, RandomAccessIteratorT end,
    const PointDiT &old_base, std::array<uint32_t, D> levels,
    uint32_t last_axis) {
  if (!Policy::select_axis) {
    return DRACO_INCREMENT_MOD(last_axis, D);
  }

  // For many points this function selects the axis that should be used
  // for the split by keeping as many points as possible bundled.
  // In the best case we do not split the point cloud at all.
  // For lower number of points, we simply choose the axis that is refined the
  // least so far.

  DRACO_DCHECK_EQ(true, end - begin != 0);

  uint32_t best_axis = 0;
  if (end - begin < 64) {
    for (uint32_t axis = 1; axis < D; ++axis) {
      if (levels[best_axis] > levels[axis]) {
        best_axis = axis;
      }
    }
  } else {
    const uint32_t size = (end - begin);
    std::array<uint32_t, D> num_remaining_bits =
        PointTraits<PointDiT>::ZeroArray();
    for (int i = 0; i < D; i++) {
      num_remaining_bits[i] = bit_length_ - levels[i];
    }
    PointDiT split(old_base);

    for (int i = 0; i < D; i++) {
      if (num_remaining_bits[i]) {
        split[i] += 1 << (num_remaining_bits[i] - 1);
      }
    }

    std::array<uint32_t, D> deviations = PointTraits<PointDiT>::ZeroArray();
    for (auto it = begin; it != end; ++it) {
      for (int i = 0; i < D; i++) {
        deviations[i] += ((*it)[i] < split[i]);
      }
    }
    for (int i = 0; i < D; i++) {
      deviations[i] = std::max(size - deviations[i], deviations[i]);
    }

    uint32_t max_value = 0;
    best_axis = 0;
    for (int i = 0; i < D; i++) {
      // If axis can be subdivided.
      if (num_remaining_bits[i]) {
        // Check if this is the better axis.
        if (max_value < deviations[i]) {
          max_value = deviations[i];
          best_axis = i;
        }
      }
    }
    axis_encoder_.EncodeLeastSignificantBits32(4, best_axis);
  }

  return best_axis;
}

template <class PointDiT, int compression_level_t>
template <class RandomAccessIteratorT>
void IntegerPointsKdTreeEncoder<PointDiT, compression_level_t>::EncodeInternal(
    RandomAccessIteratorT begin, RandomAccessIteratorT end, PointDiT old_base,
    std::array<uint32_t, D> levels, uint32_t last_axis) {
  EncodingStatus<RandomAccessIteratorT> init_status(begin, end, old_base,
                                                    levels, last_axis);
  typename Policy::template QueuingStrategy<
      EncodingStatus<RandomAccessIteratorT>>
      status_q;

  status_q.push(init_status);

  while (!status_q.empty()) {
    EncodingStatus<RandomAccessIteratorT> status = status_q.front();
    status_q.pop();

    begin = status.begin;
    end = status.end;
    old_base = status.old_base;
    levels = status.levels;
    last_axis = status.last_axis;

    const uint32_t axis = GetAxis(begin, end, old_base, levels, last_axis);
    const uint32_t level = levels[axis];
    const uint32_t num_remaining_points = end - begin;

    // If this happens all axis are subdivided to the end.
    if ((bit_length_ - level) == 0) {
      continue;
    }

    // Fast encoding of remaining bits if number of points is 1.
    // Doing this also for 2 gives a slight additional speed up.
    if (num_remaining_points <= 2) {
      std::array<uint32_t, D> axes;
      axes[0] = axis;
      for (int i = 1; i < D; i++) {
        axes[i] = DRACO_INCREMENT_MOD(axes[i - 1], D);
      }

      std::array<uint32_t, D> num_remaining_bits;
      for (int i = 0; i < D; i++) {
        num_remaining_bits[i] = bit_length_ - levels[axes[i]];
      }

      for (uint32_t i = 0; i < num_remaining_points; ++i) {
        const PointDiT &p = *(begin + i);
        for (int j = 0; j < D; j++) {
          if (num_remaining_bits[j]) {
            remaining_bits_encoder_.EncodeLeastSignificantBits32(
                num_remaining_bits[j], p[axes[j]]);
          }
        }
      }
      continue;
    }

    const uint32_t num_remaining_bits = bit_length_ - level;
    const uint32_t modifier = 1 << (num_remaining_bits - 1);
    PointDiT new_base(old_base);
    new_base[axis] += modifier;
    const RandomAccessIteratorT split =
        std::partition(begin, end, Splitter(axis, new_base[axis]));

    DRACO_DCHECK_EQ(true, (end - begin) > 0);

    // Encode number of points in first and second half.
    const int required_bits = MostSignificantBit(num_remaining_points);

    const uint32_t first_half = split - begin;
    const uint32_t second_half = end - split;
    const bool left = first_half < second_half;

    if (first_half != second_half) {
      half_encoder_.EncodeBit(left);
    }

    if (left) {
      EncodeNumber(required_bits, num_remaining_points / 2 - first_half);
    } else {
      EncodeNumber(required_bits, num_remaining_points / 2 - second_half);
    }

    levels[axis] += 1;
    if (split != begin) {
      status_q.push(EncodingStatus<RandomAccessIteratorT>(
          begin, split, old_base, levels, axis));
    }
    if (split != end) {
      status_q.push(EncodingStatus<RandomAccessIteratorT>(split, end, new_base,
                                                          levels, axis));
    }
  }
}

extern template class IntegerPointsKdTreeEncoder<Point3ui, 0>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 1>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 2>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 3>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 4>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 5>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 6>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 7>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 8>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 9>;
extern template class IntegerPointsKdTreeEncoder<Point3ui, 10>;

}  // namespace draco

#endif  // DRACO_COMPRESSION_POINT_CLOUD_ALGORITHMS_INTEGER_POINTS_KD_TREE_ENCODER_H_
