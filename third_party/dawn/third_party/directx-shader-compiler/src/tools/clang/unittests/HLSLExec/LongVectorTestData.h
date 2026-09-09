#ifndef LONGVECTORTESTDATA_H
#define LONGVECTORTESTDATA_H

#include <Verify.h>

#include <limits>
#include <map>
#include <string>
#include <vector>

#include "HlslTestDataTypes.h"

namespace LongVector {

// Import shared HLSL type wrappers into LongVector namespace.
using HLSLTestDataTypes::HLSLBool_t;
using HLSLTestDataTypes::HLSLHalf_t;
using HLSLTestDataTypes::HLSLMin16Float_t;
using HLSLTestDataTypes::HLSLMin16Int_t;
using HLSLTestDataTypes::HLSLMin16Uint_t;

enum class InputSet {
#define INPUT_SET(SYMBOL) SYMBOL,
#include "LongVectorOps.def"
};

template <typename T> const std::vector<T> &getInputSet(InputSet InputSet) {
  static_assert(false, "No InputSet for this type");
}

#define BEGIN_INPUT_SETS(TYPE)                                                 \
  template <> const std::vector<TYPE> &getInputSet<TYPE>(InputSet InputSet) {  \
    using T = TYPE;                                                            \
    switch (InputSet) {

#define INPUT_SET(SET, ...)                                                    \
  case SET: {                                                                  \
    static std::vector<T> Data = {__VA_ARGS__};                                \
    return Data;                                                               \
  }

#define END_INPUT_SETS()                                                       \
  default:                                                                     \
    break;                                                                     \
    }                                                                          \
    VERIFY_FAIL("Missing input set");                                          \
    std::abort();                                                              \
    }

BEGIN_INPUT_SETS(HLSLBool_t)
INPUT_SET(InputSet::Default1, false, true, false, false, false, false, true,
          true, true, true);

INPUT_SET(InputSet::Default2, true, false, false, false, false, true, true,
          true, false, false);
INPUT_SET(InputSet::Default3, true, false, true, false, true, true, true, true,
          false, true);
INPUT_SET(InputSet::Zero, false);
INPUT_SET(InputSet::NoZero, true);
INPUT_SET(InputSet::SelectCond, false, true);
END_INPUT_SETS()

BEGIN_INPUT_SETS(int16_t)
INPUT_SET(InputSet::Default1, -6, 1, 7, 3, 8, 4, -3, 8, 8, -2);
INPUT_SET(InputSet::Default2, 5, -6, -3, -2, 9, 3, 1, -3, -7, 2);
INPUT_SET(InputSet::Default3, -5, 6, 3, 2, -9, -3, -1, 3, 7, -2);
INPUT_SET(InputSet::BitShiftRhs, 1, 6, 3, 0, 9, 3, 12, 13, 14, 15);
INPUT_SET(InputSet::Zero, 0);
INPUT_SET(InputSet::NoZero, 1);
INPUT_SET(InputSet::Bitwise, std::numeric_limits<int16_t>::min(), -1, 0, 1, 3,
          6, 9, 0x5555, static_cast<int16_t>(0xAAAA),
          std::numeric_limits<int16_t>::max());
INPUT_SET(InputSet::SelectCond, 0, 1);
INPUT_SET(InputSet::AllOnes, 1);
INPUT_SET(InputSet::WaveMultiPrefixBitwise, 0x0, 0x1, 0x3, 0x4, 0x10, 0x12, 0xF,
          -1);
END_INPUT_SETS()

BEGIN_INPUT_SETS(int32_t)
INPUT_SET(InputSet::Default1, -6, 1, 7, 3, 8, 4, -3, 8, 8, -2);
INPUT_SET(InputSet::Default2, 5, -6, -3, -2, 9, 3, 1, -3, -7, 2);
INPUT_SET(InputSet::Default3, -5, 6, 3, 2, -9, -3, -1, 3, 7, -2);
INPUT_SET(InputSet::BitShiftRhs, 1, 6, 3, 0, 9, 3, 30, 31, 32);
INPUT_SET(InputSet::Zero, 0);
INPUT_SET(InputSet::NoZero, 1);
INPUT_SET(InputSet::Bitwise, std::numeric_limits<int32_t>::min(), -1, 0, 1, 3,
          6, 9, 0x55555555, static_cast<int32_t>(0xAAAAAAAA),
          std::numeric_limits<int32_t>::max());
INPUT_SET(InputSet::SelectCond, 0, 1);
INPUT_SET(InputSet::AllOnes, 1);
INPUT_SET(InputSet::WaveMultiPrefixBitwise, 0x0, 0x1, 0x3, 0x4, 0x10, 0x12, 0xF,
          -1);
END_INPUT_SETS()

BEGIN_INPUT_SETS(int64_t)
INPUT_SET(InputSet::Default1, -6, 11, 7, 3, 8, 4, -3, 8, 8, -2);
INPUT_SET(InputSet::Default2, 5, -1337, -3, -2, 9, 3, 1, -3, 501, 2);
INPUT_SET(InputSet::Default3, -5, 1337, 3, 2, -9, -3, -1, 3, -501, -2);
INPUT_SET(InputSet::BitShiftRhs, 1, 6, 3, 0, 9, 3, 62, 63, 64);
INPUT_SET(InputSet::Zero, 0);
INPUT_SET(InputSet::NoZero, 1);
INPUT_SET(InputSet::Bitwise, std::numeric_limits<int64_t>::min(), -1, 0, 1, 3,
          6, 9, 0x5555555555555555LL, 0xAAAAAAAAAAAAAAAALL,
          std::numeric_limits<int64_t>::max());
INPUT_SET(InputSet::SelectCond, 0, 1);
INPUT_SET(InputSet::AllOnes, 1);
INPUT_SET(InputSet::WaveMultiPrefixBitwise, 0x0, 0x1, 0x3, 0x4, 0x10, 0x12, 0xF,
          -1ll);
END_INPUT_SETS()

BEGIN_INPUT_SETS(uint16_t)
INPUT_SET(InputSet::Default1, 1, 699, 3, 1023, 5, 6, 0, 8, 9, 10);
INPUT_SET(InputSet::Default2, 2, 111, 3, 4, 5, 9, 21, 8, 9, 10);
INPUT_SET(InputSet::Default3, 4, 112, 4, 5, 3, 7, 21, 1, 11, 9);
INPUT_SET(InputSet::Zero, 0);
INPUT_SET(InputSet::BitShiftRhs, 1, 6, 3, 0, 9, 3, 12, 13, 14, 15);
INPUT_SET(InputSet::Bitwise, 0, 1, 3, 6, 9, 0x5555, 0xAAAA, 0x8000, 127,
          std::numeric_limits<uint16_t>::max());
INPUT_SET(InputSet::SelectCond, 0, 1);
INPUT_SET(InputSet::AllOnes, 1);
INPUT_SET(InputSet::WaveMultiPrefixBitwise, 0x0, 0x1, 0x3, 0x4, 0x10, 0x12, 0xF,
          std::numeric_limits<uint16_t>::max());
END_INPUT_SETS()

BEGIN_INPUT_SETS(uint32_t)
INPUT_SET(InputSet::Default1, 1, 2, 3, 4, 5, 0, 7, 8, 9, 10);
INPUT_SET(InputSet::Default2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
INPUT_SET(InputSet::Default3, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
INPUT_SET(InputSet::Zero, 0);
INPUT_SET(InputSet::BitShiftRhs, 1, 6, 3, 0, 9, 3, 30, 31, 32);
INPUT_SET(InputSet::Bitwise, 0, 1, 3, 6, 9, 0x55555555, 0xAAAAAAAA, 0x80000000,
          127, std::numeric_limits<uint32_t>::max());
INPUT_SET(InputSet::SelectCond, 0, 1);
INPUT_SET(InputSet::AllOnes, 1);
INPUT_SET(InputSet::WaveMultiPrefixBitwise, 0x0, 0x1, 0x3, 0x4, 0xA, 0xC, 0xF,
          std::numeric_limits<uint32_t>::max());
END_INPUT_SETS()

BEGIN_INPUT_SETS(uint64_t)
INPUT_SET(InputSet::Default1, 1, 2, 3, 4, 5, 0, 7, 1000, 9, 10);
INPUT_SET(InputSet::Default2, 1, 2, 1337, 4, 5, 6, 7, 8, 9, 10);
INPUT_SET(InputSet::Default3, 10, 20, 1338, 40, 50, 60, 70, 80, 90, 11);
INPUT_SET(InputSet::Zero, 0);
INPUT_SET(InputSet::BitShiftRhs, 1, 6, 3, 0, 9, 3, 62, 63, 64);
INPUT_SET(InputSet::Bitwise, 0, 1, 3, 6, 9, 0x5555555555555555,
          0xAAAAAAAAAAAAAAAA, 0x8000000000000000, 127,
          std::numeric_limits<uint64_t>::max());
INPUT_SET(InputSet::SelectCond, 0, 1);
INPUT_SET(InputSet::AllOnes, 1);
INPUT_SET(InputSet::WaveMultiPrefixBitwise, 0x0, 0x1, 0x3, 0x4, 0xA, 0xC, 0xF,
          std::numeric_limits<uint64_t>::max());
END_INPUT_SETS()

BEGIN_INPUT_SETS(HLSLHalf_t)
INPUT_SET(InputSet::Default1, -1.0, -1.0, 1.0, -0.01, 1.0, -0.01, 1.0, -0.01,
          1.0, -0.01);
INPUT_SET(InputSet::Default2, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
          -1.0);
INPUT_SET(InputSet::Default3, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0,
          1.0);
INPUT_SET(InputSet::Zero, 0.0);
INPUT_SET(InputSet::RangeHalfPi, -1.073, 0.044, -1.047, 0.313, 1.447, -0.865,
          1.364, -0.715, -0.800, 0.541);
INPUT_SET(InputSet::RangeOne, 0.331, 0.727, -0.957, 0.677, -0.025, 0.495, 0.855,
          -0.673, -0.678, -0.905);
INPUT_SET(InputSet::Positive, 1.0, 1.0, 342.0, 0.01, 5531.0, 0.01, 1.0, 0.01,
          331.2330, 3250.01);
INPUT_SET(InputSet::SelectCond, 0.0, 1.0);
// HLSLHalf_t has a constructor which accepts a float and converts it to half
// precision by clamping to the representable range via
// DirectX::PackedVector::XMConvertFloatToHalf.
INPUT_SET(InputSet::FloatSpecial, std::numeric_limits<float>::infinity(),
          -std::numeric_limits<float>::infinity(),
          std::numeric_limits<float>::signaling_NaN(),
          -std::numeric_limits<float>::signaling_NaN(),
          std::numeric_limits<float>::quiet_NaN(),
          -std::numeric_limits<float>::quiet_NaN(), 0.0, -0.0,
          std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
          -std::numeric_limits<float>::min(),
          -std::numeric_limits<float>::max(),
          std::numeric_limits<float>::denorm_min(),
          std::numeric_limits<float>::denorm_min() * 10.0, 1.0 / 3.0);
INPUT_SET(InputSet::AllOnes, 1.0);
END_INPUT_SETS()

BEGIN_INPUT_SETS(float)
INPUT_SET(InputSet::Default1, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
          -1.0);
INPUT_SET(InputSet::Default2, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
          -1.0);
INPUT_SET(InputSet::Default3, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0,
          1.0);
INPUT_SET(InputSet::Zero, 0.0);
INPUT_SET(InputSet::RangeHalfPi, 0.315f, -0.316f, 1.409f, -0.09f, -1.569f,
          1.302f, -0.326f, 0.781f, -1.235f, 0.623f);
INPUT_SET(InputSet::RangeOne, 0.727f, 0.331f, -0.957f, 0.677f, -0.025f, 0.495f,
          0.855f, -0.673f, -0.678f, -0.905f);
INPUT_SET(InputSet::Positive, 1.0f, 1.0f, 65535.0f, 0.01f, 5531.0f, 0.01f, 1.0f,
          0.01f, 331.2330f, 3250.01f);
INPUT_SET(InputSet::SelectCond, 0.0f, 1.0f);
INPUT_SET(InputSet::FloatSpecial, std::numeric_limits<float>::infinity(),
          -std::numeric_limits<float>::infinity(),
          std::numeric_limits<float>::signaling_NaN(),
          -std::numeric_limits<float>::signaling_NaN(),
          std::numeric_limits<float>::quiet_NaN(),
          -std::numeric_limits<float>::quiet_NaN(), 0.0f, -0.0f,
          std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
          -std::numeric_limits<float>::min(),
          -std::numeric_limits<float>::max(),
          std::numeric_limits<float>::denorm_min(),
          std::numeric_limits<float>::denorm_min() * 10.0f, 1.0f / 3.0f);
INPUT_SET(InputSet::AllOnes, 1.0f);
END_INPUT_SETS()

BEGIN_INPUT_SETS(double)
INPUT_SET(InputSet::Default1, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
          -1.0);
INPUT_SET(InputSet::Default2, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
          -1.0);
INPUT_SET(InputSet::Default3, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0,
          1.0);
INPUT_SET(InputSet::Zero, 0.0);
INPUT_SET(InputSet::RangeHalfPi, 0.807, 0.605, 1.317, 0.188, 1.566, -1.507,
          0.67, -1.553, 0.194, -0.883);
INPUT_SET(InputSet::RangeOne, 0.331, 0.277, -0.957, 0.677, -0.025, 0.495, 0.855,
          -0.673, -0.678, -0.905);
INPUT_SET(InputSet::SplitDouble, 0.0, -1.0, 1.0, -1.0, 12345678.87654321, -1.0,
          1.0, -1.0, 1.0, -1.0);
INPUT_SET(InputSet::Positive, 1.0, 1.0, 65535.0, 0.01, 5531.0, 0.01, 1.0, 0.01,
          331.2330, 3250.01);
INPUT_SET(InputSet::SelectCond, 0.0, 1.0);
INPUT_SET(InputSet::AllOnes, 1.0);
END_INPUT_SETS()

// Min precision input sets. All values are exactly representable in float16
// to avoid precision mismatch between CPU-side expected values and GPU-side
// min precision computation. No FP specials (INF/NaN/denorm) as min precision
// types do not support them.
BEGIN_INPUT_SETS(HLSLMin16Float_t)
INPUT_SET(InputSet::Default1, -1.0f, -1.0f, 1.0f, -0.03125f, 1.0f, -0.03125f,
          1.0f, -0.03125f, 1.0f, -0.03125f);
INPUT_SET(InputSet::Default2, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f,
          -1.0f, 1.0f, -1.0f);
INPUT_SET(InputSet::Default3, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
          1.0f, -1.0f, 1.0f);
INPUT_SET(InputSet::Zero, 0.0f);
INPUT_SET(InputSet::RangeHalfPi, -1.0625f, 0.046875f, -1.046875f, 0.3125f,
          1.4375f, -0.875f, 1.375f, -0.71875f, -0.8125f, 0.5625f);
INPUT_SET(InputSet::RangeOne, 0.328125f, 0.71875f, -0.953125f, 0.671875f,
          -0.03125f, 0.5f, 0.84375f, -0.671875f, -0.6875f, -0.90625f);
INPUT_SET(InputSet::Positive, 1.0f, 1.0f, 342.0f, 0.03125f, 5504.0f, 0.03125f,
          1.0f, 0.03125f, 331.25f, 3250.0f);
INPUT_SET(InputSet::SelectCond, 0.0f, 1.0f);
INPUT_SET(InputSet::AllOnes, 1.0f);
END_INPUT_SETS()

// Values constrained to int16 range. Kept small to avoid overflow ambiguity.
// Shift amounts limited so results fit in int16 (-32768..32767).
BEGIN_INPUT_SETS(HLSLMin16Int_t)
INPUT_SET(InputSet::Default1, -6, 1, 7, 3, 8, 4, -3, 8, 8, -2);
INPUT_SET(InputSet::Default2, 5, -6, -3, -2, 9, 3, 1, -3, -7, 2);
INPUT_SET(InputSet::Default3, -5, 6, 3, 2, -9, -3, -1, 3, 7, -2);
INPUT_SET(InputSet::BitShiftRhs, 1, 6, 3, 0, 9, 3, 12, 11, 11, 14);
INPUT_SET(InputSet::Zero, 0);
INPUT_SET(InputSet::NoZero, 1);
INPUT_SET(InputSet::SelectCond, 0, 1);
INPUT_SET(InputSet::AllOnes, 1);
INPUT_SET(InputSet::WaveMultiPrefixBitwise, 0x0, 0x1, 0x3, 0x4, 0x10, 0x12, 0xF,
          -1);
END_INPUT_SETS()

// Values constrained so results stay below 0x8000 (bit 15 clear). WARP may
// compute min precision at 16-bit and sign-extend bit 15 on 32-bit store.
BEGIN_INPUT_SETS(HLSLMin16Uint_t)
INPUT_SET(InputSet::Default1, 3, 7, 3, 5, 5, 10, 4, 8, 9, 10);
INPUT_SET(InputSet::Default2, 2, 6, 3, 4, 5, 9, 3, 8, 9, 10);
INPUT_SET(InputSet::Default3, 4, 5, 4, 5, 3, 7, 3, 1, 11, 9);
INPUT_SET(InputSet::Zero, 0);
INPUT_SET(InputSet::BitShiftRhs, 1, 6, 3, 0, 9, 3, 8, 8, 8, 8);
INPUT_SET(InputSet::SelectCond, 0, 1);
INPUT_SET(InputSet::AllOnes, 1);
INPUT_SET(InputSet::WaveMultiPrefixBitwise, 0x0, 0x1, 0x3, 0x4, 0x10, 0x12, 0xF,
          0x7FFF);
END_INPUT_SETS()

#undef BEGIN_INPUT_SETS
#undef INPUT_SET
#undef END_INPUT_SETS

}; // namespace LongVector

#endif // LONGVECTORTESTDATA_H
