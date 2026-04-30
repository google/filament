#include "draco/core/math_utils.h"

#include <cmath>
#include <random>

#include "draco/core/draco_test_base.h"

namespace draco {

TEST(MathUtils, Mod) { EXPECT_EQ(DRACO_INCREMENT_MOD(1, 1 << 1), 0); }

TEST(MathUtils, IntSqrt) {
  ASSERT_EQ(IntSqrt(0), 0);
  // 64-bit pseudo random number generator seeded with a predefined number.
  std::mt19937_64 generator(109);
  std::uniform_int_distribution<uint64_t> distribution(0, 1ull << 60);

  for (int i = 0; i < 10000; ++i) {
    const uint64_t number = distribution(generator);
    ASSERT_EQ(IntSqrt(number), static_cast<uint64_t>(floor(std::sqrt(number))));
  }
}

}  // namespace draco
