#include "draco/compression/bit_coders/adaptive_rans_bit_decoder.h"
#include "draco/compression/bit_coders/adaptive_rans_bit_encoder.h"
#include "draco/compression/bit_coders/rans_bit_decoder.h"
#include "draco/compression/bit_coders/rans_bit_encoder.h"
#include "draco/core/draco_test_base.h"

// Just including rans_coding.h and adaptive_rans_coding.h gets an asan error
// when compiling (blaze test :rans_coding_test --config=asan)
TEST(RansCodingTest, LinkerTest) {}
