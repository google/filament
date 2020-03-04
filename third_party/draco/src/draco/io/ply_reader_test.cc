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
#include "draco/io/ply_reader.h"

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/io/file_utils.h"
#include "draco/io/ply_property_reader.h"

namespace draco {

class PlyReaderTest : public ::testing::Test {
 protected:
  std::vector<char> ReadPlyFile(const std::string &file_name) const {
    const std::string path = GetTestFileFullPath(file_name);

    std::vector<char> data;
    EXPECT_TRUE(ReadFileToBuffer(path, &data));
    return data;
  }
};

TEST_F(PlyReaderTest, TestReader) {
  const std::string file_name = "test_pos_color.ply";
  const std::vector<char> data = ReadPlyFile(file_name);
  DecoderBuffer buf;
  buf.Init(data.data(), data.size());
  PlyReader reader;
  Status status = reader.Read(&buf);
  ASSERT_TRUE(status.ok()) << status;
  ASSERT_EQ(reader.num_elements(), 2);
  ASSERT_EQ(reader.element(0).num_properties(), 7);
  ASSERT_EQ(reader.element(1).num_properties(), 1);
  ASSERT_TRUE(reader.element(1).property(0).is_list());

  ASSERT_TRUE(reader.element(0).GetPropertyByName("red") != nullptr);
  const PlyProperty *const prop = reader.element(0).GetPropertyByName("red");
  PlyPropertyReader<uint8_t> reader_uint8(prop);
  PlyPropertyReader<uint32_t> reader_uint32(prop);
  PlyPropertyReader<float> reader_float(prop);
  for (int i = 0; i < reader.element(0).num_entries(); ++i) {
    ASSERT_EQ(reader_uint8.ReadValue(i), reader_uint32.ReadValue(i));
    ASSERT_EQ(reader_uint8.ReadValue(i), reader_float.ReadValue(i));
  }
}

TEST_F(PlyReaderTest, TestReaderAscii) {
  const std::string file_name = "test_pos_color.ply";
  const std::vector<char> data = ReadPlyFile(file_name);
  ASSERT_NE(data.size(), 0u);
  DecoderBuffer buf;
  buf.Init(data.data(), data.size());
  PlyReader reader;
  Status status = reader.Read(&buf);
  ASSERT_TRUE(status.ok()) << status;

  const std::string file_name_ascii = "test_pos_color_ascii.ply";
  const std::vector<char> data_ascii = ReadPlyFile(file_name_ascii);
  buf.Init(data_ascii.data(), data_ascii.size());
  PlyReader reader_ascii;
  status = reader_ascii.Read(&buf);
  ASSERT_TRUE(status.ok()) << status;
  ASSERT_EQ(reader.num_elements(), reader_ascii.num_elements());
  ASSERT_EQ(reader.element(0).num_properties(),
            reader_ascii.element(0).num_properties());

  ASSERT_TRUE(reader.element(0).GetPropertyByName("x") != nullptr);
  const PlyProperty *const prop = reader.element(0).GetPropertyByName("x");
  const PlyProperty *const prop_ascii =
      reader_ascii.element(0).GetPropertyByName("x");
  PlyPropertyReader<float> reader_float(prop);
  PlyPropertyReader<float> reader_float_ascii(prop_ascii);
  for (int i = 0; i < reader.element(0).num_entries(); ++i) {
    ASSERT_NEAR(reader_float.ReadValue(i), reader_float_ascii.ReadValue(i),
                1e-4f);
  }
}

TEST_F(PlyReaderTest, TestReaderExtraWhitespace) {
  const std::string file_name = "test_extra_whitespace.ply";
  const std::vector<char> data = ReadPlyFile(file_name);
  ASSERT_NE(data.size(), 0u);
  DecoderBuffer buf;
  buf.Init(data.data(), data.size());
  PlyReader reader;
  Status status = reader.Read(&buf);
  ASSERT_TRUE(status.ok()) << status;

  ASSERT_EQ(reader.num_elements(), 2);
  ASSERT_EQ(reader.element(0).num_properties(), 7);
  ASSERT_EQ(reader.element(1).num_properties(), 1);
  ASSERT_TRUE(reader.element(1).property(0).is_list());

  ASSERT_TRUE(reader.element(0).GetPropertyByName("red") != nullptr);
  const PlyProperty *const prop = reader.element(0).GetPropertyByName("red");
  PlyPropertyReader<uint8_t> reader_uint8(prop);
  PlyPropertyReader<uint32_t> reader_uint32(prop);
  PlyPropertyReader<float> reader_float(prop);
  for (int i = 0; i < reader.element(0).num_entries(); ++i) {
    ASSERT_EQ(reader_uint8.ReadValue(i), reader_uint32.ReadValue(i));
    ASSERT_EQ(reader_uint8.ReadValue(i), reader_float.ReadValue(i));
  }
}

TEST_F(PlyReaderTest, TestReaderMoreDataTypes) {
  const std::string file_name = "test_more_datatypes.ply";
  const std::vector<char> data = ReadPlyFile(file_name);
  ASSERT_NE(data.size(), 0u);
  DecoderBuffer buf;
  buf.Init(data.data(), data.size());
  PlyReader reader;
  Status status = reader.Read(&buf);
  ASSERT_TRUE(status.ok()) << status;

  ASSERT_EQ(reader.num_elements(), 2);
  ASSERT_EQ(reader.element(0).num_properties(), 7);
  ASSERT_EQ(reader.element(1).num_properties(), 1);
  ASSERT_TRUE(reader.element(1).property(0).is_list());

  ASSERT_TRUE(reader.element(0).GetPropertyByName("red") != nullptr);
  const PlyProperty *const prop = reader.element(0).GetPropertyByName("red");
  PlyPropertyReader<uint8_t> reader_uint8(prop);
  PlyPropertyReader<uint32_t> reader_uint32(prop);
  PlyPropertyReader<float> reader_float(prop);
  for (int i = 0; i < reader.element(0).num_entries(); ++i) {
    ASSERT_EQ(reader_uint8.ReadValue(i), reader_uint32.ReadValue(i));
    ASSERT_EQ(reader_uint8.ReadValue(i), reader_float.ReadValue(i));
  }
}

}  // namespace draco
