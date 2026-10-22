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
#include <numeric>

#include "draco/compression/attributes/sequential_integer_attribute_decoder.h"
#include "draco/compression/attributes/sequential_integer_attribute_encoder.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/core/draco_test_base.h"

namespace draco {

class SequentialIntegerAttributeEncodingTest : public ::testing::Test {
 protected:
};

TEST_F(SequentialIntegerAttributeEncodingTest, DoesCompress) {
  // This test verifies that IntegerEncoding encodes and decodes the given data.
  const std::vector<int32_t> values{1,   8,  7,  5, 5,   5, 9,
                                    155, -6, -9, 9, 125, 1, 0};
  PointAttribute pa;
  pa.Init(GeometryAttribute::GENERIC, 1, DT_INT32, false, values.size());
  for (uint32_t i = 0; i < values.size(); ++i) {
    pa.SetAttributeValue(AttributeValueIndex(i), &values[i]);
  }
  // List of point ids from 0 to point_ids.size() - 1.
  std::vector<PointIndex> point_ids(values.size());
  std::iota(point_ids.begin(), point_ids.end(), 0);

  EncoderBuffer out_buf;
  SequentialIntegerAttributeEncoder ie;
  ASSERT_TRUE(ie.InitializeStandalone(&pa));
  ASSERT_TRUE(ie.TransformAttributeToPortableFormat(point_ids));
  ASSERT_TRUE(ie.EncodePortableAttribute(point_ids, &out_buf));
  ASSERT_TRUE(ie.EncodeDataNeededByPortableTransform(&out_buf));

  DecoderBuffer in_buf;
  in_buf.Init(out_buf.data(), out_buf.size());
  in_buf.set_bitstream_version(kDracoMeshBitstreamVersion);
  SequentialIntegerAttributeDecoder id;
  ASSERT_TRUE(id.InitializeStandalone(&pa));
  ASSERT_TRUE(id.DecodePortableAttribute(point_ids, &in_buf));
  ASSERT_TRUE(id.DecodeDataNeededByPortableTransform(point_ids, &in_buf));
  ASSERT_TRUE(id.TransformAttributeToOriginalFormat(point_ids));

  for (uint32_t i = 0; i < values.size(); ++i) {
    int32_t entry_val;
    pa.GetValue(AttributeValueIndex(i), &entry_val);
    ASSERT_EQ(entry_val, values[i]);
  }
}

}  // namespace draco
