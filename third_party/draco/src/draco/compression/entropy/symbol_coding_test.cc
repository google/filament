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
#include "draco/compression/config/compression_shared.h"
#include "draco/compression/entropy/symbol_decoding.h"
#include "draco/compression/entropy/symbol_encoding.h"
#include "draco/core/bit_utils.h"
#include "draco/core/decoder_buffer.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/encoder_buffer.h"

namespace draco {

class SymbolCodingTest : public ::testing::Test {
 protected:
  SymbolCodingTest() : bitstream_version_(kDracoMeshBitstreamVersion) {}

  template <class SignedIntTypeT>
  void TestConvertToSymbolAndBack(SignedIntTypeT x) {
    typedef typename std::make_unsigned<SignedIntTypeT>::type Symbol;
    Symbol symbol = ConvertSignedIntToSymbol(x);
    SignedIntTypeT y = ConvertSymbolToSignedInt(symbol);
    ASSERT_EQ(x, y);
  }

  uint16_t bitstream_version_;
};

TEST_F(SymbolCodingTest, TestLargeNumbers) {
  // This test verifies that SymbolCoding successfully encodes an array of large
  // numbers.
  const uint32_t in[] = {12345678, 1223333, 111, 5};
  const int num_values = sizeof(in) / sizeof(uint32_t);
  EncoderBuffer eb;
  ASSERT_TRUE(EncodeSymbols(in, num_values, 1, nullptr, &eb));

  std::vector<uint32_t> out;
  out.resize(num_values);
  DecoderBuffer db;
  db.Init(eb.data(), eb.size());
  db.set_bitstream_version(bitstream_version_);
  ASSERT_TRUE(DecodeSymbols(num_values, 1, &db, &out[0]));
  for (int i = 0; i < num_values; ++i) {
    EXPECT_EQ(in[i], out[i]);
  }
}

TEST_F(SymbolCodingTest, TestManyNumbers) {
  // This test verifies that SymbolCoding successfully encodes an array of
  // several numbers that repeat many times.

  // Value/frequency pairs.
  const std::pair<uint32_t, uint32_t> in[] = {
      {12, 1500}, {1025, 31000}, {7, 1}, {9, 5}, {0, 6432}};

  const int num_pairs = sizeof(in) / sizeof(std::pair<uint32_t, uint32_t>);

  std::vector<uint32_t> in_values;
  for (int i = 0; i < num_pairs; ++i) {
    in_values.insert(in_values.end(), in[i].second, in[i].first);
  }
  for (int method = 0; method < NUM_SYMBOL_CODING_METHODS; ++method) {
    // Test the encoding using all available symbol coding methods.
    Options options;
    SetSymbolEncodingMethod(&options, static_cast<SymbolCodingMethod>(method));

    EncoderBuffer eb;
    ASSERT_TRUE(
        EncodeSymbols(in_values.data(), in_values.size(), 1, &options, &eb));
    std::vector<uint32_t> out_values;
    out_values.resize(in_values.size());
    DecoderBuffer db;
    db.Init(eb.data(), eb.size());
    db.set_bitstream_version(bitstream_version_);
    ASSERT_TRUE(DecodeSymbols(in_values.size(), 1, &db, &out_values[0]));
    for (uint32_t i = 0; i < in_values.size(); ++i) {
      ASSERT_EQ(in_values[i], out_values[i]);
    }
  }
}

TEST_F(SymbolCodingTest, TestEmpty) {
  // This test verifies that SymbolCoding successfully encodes an empty array.
  EncoderBuffer eb;
  ASSERT_TRUE(EncodeSymbols(nullptr, 0, 1, nullptr, &eb));
  DecoderBuffer db;
  db.Init(eb.data(), eb.size());
  db.set_bitstream_version(bitstream_version_);
  ASSERT_TRUE(DecodeSymbols(0, 1, &db, nullptr));
}

TEST_F(SymbolCodingTest, TestOneSymbol) {
  // This test verifies that SymbolCoding successfully encodes an a single
  // symbol.
  EncoderBuffer eb;
  const std::vector<uint32_t> in(1200, 0);
  ASSERT_TRUE(EncodeSymbols(in.data(), in.size(), 1, nullptr, &eb));

  std::vector<uint32_t> out(in.size());
  DecoderBuffer db;
  db.Init(eb.data(), eb.size());
  db.set_bitstream_version(bitstream_version_);
  ASSERT_TRUE(DecodeSymbols(in.size(), 1, &db, &out[0]));
  for (uint32_t i = 0; i < in.size(); ++i) {
    ASSERT_EQ(in[i], out[i]);
  }
}

TEST_F(SymbolCodingTest, TestBitLengths) {
  // This test verifies that SymbolCoding successfully encodes symbols of
  // various bit lengths
  EncoderBuffer eb;
  std::vector<uint32_t> in;
  constexpr int bit_lengths = 18;
  for (int i = 0; i < bit_lengths; ++i) {
    in.push_back(1 << i);
  }
  std::vector<uint32_t> out(in.size());
  for (int i = 0; i < bit_lengths; ++i) {
    eb.Clear();
    ASSERT_TRUE(EncodeSymbols(in.data(), i + 1, 1, nullptr, &eb));
    DecoderBuffer db;
    db.Init(eb.data(), eb.size());
    db.set_bitstream_version(bitstream_version_);
    ASSERT_TRUE(DecodeSymbols(i + 1, 1, &db, &out[0]));
    for (int j = 0; j < i + 1; ++j) {
      ASSERT_EQ(in[j], out[j]);
    }
  }
}

TEST_F(SymbolCodingTest, TestLargeNumberCondition) {
  // This test verifies that SymbolCoding successfully encodes large symbols
  // that are on the boundary between raw scheme and tagged scheme (18 bits).
  EncoderBuffer eb;
  constexpr int num_symbols = 1000000;
  const std::vector<uint32_t> in(num_symbols, 1 << 18);
  ASSERT_TRUE(EncodeSymbols(in.data(), in.size(), 1, nullptr, &eb));

  std::vector<uint32_t> out(in.size());
  DecoderBuffer db;
  db.Init(eb.data(), eb.size());
  db.set_bitstream_version(bitstream_version_);
  ASSERT_TRUE(DecodeSymbols(in.size(), 1, &db, &out[0]));
  for (uint32_t i = 0; i < in.size(); ++i) {
    ASSERT_EQ(in[i], out[i]);
  }
}

TEST_F(SymbolCodingTest, TestConversionFullRange) {
  TestConvertToSymbolAndBack(static_cast<int8_t>(-128));
  TestConvertToSymbolAndBack(static_cast<int8_t>(-127));
  TestConvertToSymbolAndBack(static_cast<int8_t>(-1));
  TestConvertToSymbolAndBack(static_cast<int8_t>(0));
  TestConvertToSymbolAndBack(static_cast<int8_t>(1));
  TestConvertToSymbolAndBack(static_cast<int8_t>(127));
}

}  // namespace draco
