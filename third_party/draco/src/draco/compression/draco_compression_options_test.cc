#include "draco/compression/draco_compression_options.h"

#include "draco/core/draco_test_utils.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

namespace {

TEST(DracoCompressionOptionsTest, TestPositionQuantizationBits) {
  // Test verifies that we can define draco compression options using
  // quantization bits.
  draco::SpatialQuantizationOptions options(10);

  // Quantization bits should be used by default.
  ASSERT_TRUE(options.AreQuantizationBitsDefined());
  ASSERT_EQ(options.quantization_bits(), 10);

  // Change the quantization bits.
  options.SetQuantizationBits(9);
  ASSERT_TRUE(options.AreQuantizationBitsDefined());
  ASSERT_EQ(options.quantization_bits(), 9);

  // If we select the grid, quantization bits should not be used.
  options.SetGrid(0.5f);
  ASSERT_FALSE(options.AreQuantizationBitsDefined());
}

TEST(DracoCompressionOptionsTest, TestPositionQuantizationGrid) {
  // Test verifies that we can define draco compression options using
  // quantization grid.
  draco::SpatialQuantizationOptions options(10);

  // Quantization bits should be used by default.
  ASSERT_TRUE(options.AreQuantizationBitsDefined());

  // Set the grid parameters.
  options.SetGrid(0.25f);
  ASSERT_FALSE(options.AreQuantizationBitsDefined());

  ASSERT_EQ(options.spacing(), 0.25f);
}

}  // namespace

#endif  // DRACO_TRANSCODER_SUPPORTED
