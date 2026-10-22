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
#include "draco/core/quantization_utils.h"

#include "draco/core/draco_test_base.h"

namespace draco {

class QuantizationUtilsTest : public ::testing::Test {};

TEST_F(QuantizationUtilsTest, TestQuantizer) {
  Quantizer quantizer;
  quantizer.Init(10.f, 255);
  EXPECT_EQ(quantizer.QuantizeFloat(0.f), 0);
  EXPECT_EQ(quantizer.QuantizeFloat(10.f), 255);
  EXPECT_EQ(quantizer.QuantizeFloat(-10.f), -255);
  EXPECT_EQ(quantizer.QuantizeFloat(4.999f), 127);
  EXPECT_EQ(quantizer.QuantizeFloat(5.f), 128);
  EXPECT_EQ(quantizer.QuantizeFloat(-4.9999f), -127);
  // Note: Both -5.f and +5.f lie exactly on the boundary between two
  // quantized values (127.5f and -127.5f). Due to rounding, both values are
  // then converted to 128 and -127 respectively.
  EXPECT_EQ(quantizer.QuantizeFloat(-5.f), -127);
  EXPECT_EQ(quantizer.QuantizeFloat(-5.0001f), -128);

  // Out of range quantization.
  // The behavior is technically undefined, but both quantizer and dequantizer
  // should still work correctly unless the quantized values overflow.
  EXPECT_LT(quantizer.QuantizeFloat(-15.f), -255);
  EXPECT_GT(quantizer.QuantizeFloat(15.f), 255);
}

TEST_F(QuantizationUtilsTest, TestDequantizer) {
  Dequantizer dequantizer;
  ASSERT_TRUE(dequantizer.Init(10.f, 255));
  EXPECT_EQ(dequantizer.DequantizeFloat(0), 0.f);
  EXPECT_EQ(dequantizer.DequantizeFloat(255), 10.f);
  EXPECT_EQ(dequantizer.DequantizeFloat(-255), -10.f);
  EXPECT_EQ(dequantizer.DequantizeFloat(128), 10.f * (128.f / 255.f));

  // Test that the dequantizer fails to initialize with invalid input
  // parameters.
  ASSERT_FALSE(dequantizer.Init(1.f, 0));
  ASSERT_FALSE(dequantizer.Init(1.f, -4));
}

TEST_F(QuantizationUtilsTest, TestDeltaQuantization) {
  // Test verifies that the quantizer and dequantizer work correctly when
  // initialized with a delta value.
  Quantizer quantizer_delta;
  quantizer_delta.Init(0.5f);

  Quantizer quantizer_range;
  quantizer_range.Init(50.f, 100);

  EXPECT_EQ(quantizer_delta.QuantizeFloat(1.2f), 2);
  EXPECT_EQ(quantizer_delta.QuantizeFloat(10.f),
            quantizer_range.QuantizeFloat(10.f));
  EXPECT_EQ(quantizer_delta.QuantizeFloat(-3.3f),
            quantizer_range.QuantizeFloat(-3.3f));
  EXPECT_EQ(quantizer_delta.QuantizeFloat(0.25f),
            quantizer_range.QuantizeFloat(0.25f));

  Dequantizer dequantizer_delta;
  dequantizer_delta.Init(0.5f);

  Dequantizer dequantizer_range;
  dequantizer_range.Init(50.f, 100);

  EXPECT_EQ(dequantizer_delta.DequantizeFloat(2), 1.f);
  EXPECT_EQ(dequantizer_delta.DequantizeFloat(-4),
            dequantizer_range.DequantizeFloat(-4));
  EXPECT_EQ(dequantizer_delta.DequantizeFloat(9),
            dequantizer_range.DequantizeFloat(9));
  EXPECT_EQ(dequantizer_delta.DequantizeFloat(0),
            dequantizer_range.DequantizeFloat(0));
}

}  // namespace draco
