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
#include "draco/core/decoder_buffer.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/encoder_buffer.h"

namespace draco {

class BufferBitCodingTest : public ::testing::Test {
 public:
  typedef DecoderBuffer::BitDecoder BitDecoder;
  typedef EncoderBuffer::BitEncoder BitEncoder;
};

TEST_F(BufferBitCodingTest, TestBitCodersByteAligned) {
  constexpr int buffer_size = 32;
  char buffer[buffer_size];
  BitEncoder encoder(buffer);
  const uint8_t data[] = {0x76, 0x54, 0x32, 0x10, 0x76, 0x54, 0x32, 0x10};
  const int bytes_to_encode = sizeof(data);

  for (int i = 0; i < bytes_to_encode; ++i) {
    encoder.PutBits(data[i], sizeof(data[i]) * 8);
    ASSERT_EQ((i + 1) * sizeof(data[i]) * 8, encoder.Bits());
  }

  BitDecoder decoder;
  decoder.reset(static_cast<const void *>(buffer), bytes_to_encode);
  for (int i = 0; i < bytes_to_encode; ++i) {
    uint32_t x = 0;
    ASSERT_TRUE(decoder.GetBits(8, &x));
    ASSERT_EQ(x, data[i]);
  }

  ASSERT_EQ(bytes_to_encode * 8u, decoder.BitsDecoded());
}

TEST_F(BufferBitCodingTest, TestBitCodersNonByte) {
  constexpr int buffer_size = 32;
  char buffer[buffer_size];
  BitEncoder encoder(buffer);
  const uint8_t data[] = {0x76, 0x54, 0x32, 0x10, 0x76, 0x54, 0x32, 0x10};
  const uint32_t bits_to_encode = 51;
  const int bytes_to_encode = (bits_to_encode / 8) + 1;

  for (int i = 0; i < bytes_to_encode; ++i) {
    const int num_bits = (encoder.Bits() + 8 <= bits_to_encode)
                             ? 8
                             : bits_to_encode - encoder.Bits();
    encoder.PutBits(data[i], num_bits);
  }

  BitDecoder decoder;
  decoder.reset(static_cast<const void *>(buffer), bytes_to_encode);
  int64_t bits_to_decode = encoder.Bits();
  for (int i = 0; i < bytes_to_encode; ++i) {
    uint32_t x = 0;
    const int num_bits = (bits_to_decode > 8) ? 8 : bits_to_decode;
    ASSERT_TRUE(decoder.GetBits(num_bits, &x));
    const int bits_to_shift = 8 - num_bits;
    const uint8_t test_byte =
        ((data[i] << bits_to_shift) & 0xff) >> bits_to_shift;
    ASSERT_EQ(x, test_byte);
    bits_to_decode -= 8;
  }

  ASSERT_EQ(bits_to_encode, decoder.BitsDecoded());
}

TEST_F(BufferBitCodingTest, TestSingleBits) {
  const int data = 0xaaaa;

  BitDecoder decoder;
  decoder.reset(static_cast<const void *>(&data), sizeof(data));

  for (uint32_t i = 0; i < 16; ++i) {
    uint32_t x = 0;
    ASSERT_TRUE(decoder.GetBits(1, &x));
    ASSERT_EQ(x, (i % 2));
  }

  ASSERT_EQ(16u, decoder.BitsDecoded());
}

TEST_F(BufferBitCodingTest, TestMultipleBits) {
  const uint8_t data[] = {0x76, 0x54, 0x32, 0x10, 0x76, 0x54, 0x32, 0x10};

  BitDecoder decoder;
  decoder.reset(static_cast<const void *>(data), sizeof(data));

  uint32_t x = 0;
  for (uint32_t i = 0; i < 2; ++i) {
    ASSERT_TRUE(decoder.GetBits(16, &x));
    ASSERT_EQ(x, 0x5476u);
    ASSERT_EQ(16 + (i * 32), decoder.BitsDecoded());

    ASSERT_TRUE(decoder.GetBits(16, &x));
    ASSERT_EQ(x, 0x1032u);
    ASSERT_EQ(32 + (i * 32), decoder.BitsDecoded());
  }
}

}  // namespace draco
