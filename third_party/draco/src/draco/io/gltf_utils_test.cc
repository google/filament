// Copyright 2018 The Draco Authors.
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
#include "draco/io/gltf_utils.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include <limits>
#include <string>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace draco {

class GltfUtilsTest : public ::testing::Test {
 protected:
  void CompareGolden(JsonWriter *json_writer, const std::string &golden_str) {
    const std::string json = json_writer->MoveData();
    ASSERT_EQ(golden_str, json);
  }
};

TEST_F(GltfUtilsTest, TestNoData) {
  const std::string golden = "";
  JsonWriter json_writer;
  CompareGolden(&json_writer, golden);
}

TEST_F(GltfUtilsTest, TestValues) {
  JsonWriter json_writer;
  json_writer.OutputValue(0);
  CompareGolden(&json_writer, "0");

  json_writer.Reset();
  json_writer.OutputValue(1);
  CompareGolden(&json_writer, "1");

  json_writer.Reset();
  json_writer.OutputValue(-1);
  CompareGolden(&json_writer, "-1");

  json_writer.Reset();
  json_writer.OutputValue(0.0);
  CompareGolden(&json_writer, "0");

  json_writer.Reset();
  json_writer.OutputValue(1.0);
  CompareGolden(&json_writer, "1");

  json_writer.Reset();
  json_writer.OutputValue(0.25);
  CompareGolden(&json_writer, "0.25");

  json_writer.Reset();
  json_writer.OutputValue(-0.25);
  CompareGolden(&json_writer, "-0.25");

  json_writer.Reset();
  json_writer.OutputValue(false);
  CompareGolden(&json_writer, "false");

  json_writer.Reset();
  json_writer.OutputValue(true);
  CompareGolden(&json_writer, "true");

  json_writer.Reset();
  json_writer.OutputValue("test int", -1);
  CompareGolden(&json_writer, "\"test int\": -1");

  json_writer.Reset();
  json_writer.OutputValue("test float", -10.25);
  CompareGolden(&json_writer, "\"test float\": -10.25");

  json_writer.Reset();
  json_writer.OutputValue("test char*", "I am the string!");
  CompareGolden(&json_writer, "\"test char*\": \"I am the string!\"");

  json_writer.Reset();
  const std::string value = "I am the string!";
  json_writer.OutputValue("test string", value);
  CompareGolden(&json_writer, "\"test string\": \"I am the string!\"");

  json_writer.Reset();
  json_writer.OutputValue("test bool", false);
  CompareGolden(&json_writer, "\"test bool\": false");

  json_writer.Reset();
  json_writer.OutputValue("test bool", true);
  CompareGolden(&json_writer, "\"test bool\": true");
}

TEST_F(GltfUtilsTest, TestSpecialCharacters) {
  JsonWriter json_writer;
  const std::string test_double_quote = "I am double quote\"";
  json_writer.OutputValue("test double quote", test_double_quote);
  CompareGolden(&json_writer,
                "\"test double quote\": \"I am double quote\\\"\"");

  json_writer.Reset();
  const std::string test_backspace = "I am backspace\b";
  json_writer.OutputValue("test backspace", test_backspace);
  CompareGolden(&json_writer, "\"test backspace\": \"I am backspace\\\b\"");

  json_writer.Reset();
  const std::string test_form_feed = "I am form feed\f";
  json_writer.OutputValue("test form feed", test_form_feed);
  CompareGolden(&json_writer, "\"test form feed\": \"I am form feed\\\f\"");

  json_writer.Reset();
  const std::string test_newline = "I am newline\n";
  json_writer.OutputValue("test newline", test_newline);
  CompareGolden(&json_writer, "\"test newline\": \"I am newline\\\n\"");

  json_writer.Reset();
  const std::string test_tab = "I am tab\t";
  json_writer.OutputValue("test tab", test_tab);
  CompareGolden(&json_writer, "\"test tab\": \"I am tab\\\t\"");

  json_writer.Reset();
  const std::string test_backslash = "I am backslash\\";
  json_writer.OutputValue("test backslash", test_backslash);
  CompareGolden(&json_writer, "\"test backslash\": \"I am backslash\\\\\"");

  json_writer.Reset();
  const std::string test_multiple_special_characters = "\"break\"and\\more\"\\";
  json_writer.OutputValue("test multiple_special_characters",
                          test_multiple_special_characters);
  CompareGolden(&json_writer,
                "\"test multiple_special_characters\": "
                "\"\\\"break\\\"and\\\\more\\\"\\\\\"");
}

TEST_F(GltfUtilsTest, TestObjects) {
  JsonWriter json_writer;
  json_writer.BeginObject();
  json_writer.EndObject();
  CompareGolden(&json_writer, "{\n}");

  json_writer.Reset();
  json_writer.BeginObject("object");
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object\": {\n}");

  json_writer.Reset();
  json_writer.BeginObject("object");
  json_writer.OutputValue(0);
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object\": {\n  0\n}");

  json_writer.Reset();
  json_writer.BeginObject("object");
  json_writer.OutputValue(0);
  json_writer.OutputValue(1);
  json_writer.OutputValue(2);
  json_writer.OutputValue(3);
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object\": {\n  0,\n  1,\n  2,\n  3\n}");

  json_writer.Reset();
  json_writer.BeginObject("object1");
  json_writer.EndObject();
  json_writer.BeginObject("object2");
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object1\": {\n},\n\"object2\": {\n}");

  json_writer.Reset();
  json_writer.BeginObject("object1");
  json_writer.BeginObject("object2");
  json_writer.EndObject();
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object1\": {\n  \"object2\": {\n  }\n}");
}

TEST_F(GltfUtilsTest, TestArrays) {
  JsonWriter json_writer;
  json_writer.BeginArray("array");
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array\": [\n]");

  json_writer.Reset();
  json_writer.BeginArray("array");
  json_writer.OutputValue(0);
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array\": [\n  0\n]");

  json_writer.Reset();
  json_writer.BeginArray("array");
  json_writer.OutputValue(0);
  json_writer.OutputValue(1);
  json_writer.OutputValue(2);
  json_writer.OutputValue(3);
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array\": [\n  0,\n  1,\n  2,\n  3\n]");

  json_writer.Reset();
  json_writer.BeginArray("array1");
  json_writer.EndArray();
  json_writer.BeginArray("array2");
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array1\": [\n],\n\"array2\": [\n]");

  json_writer.Reset();
  json_writer.BeginArray("array1");
  json_writer.BeginArray("array2");
  json_writer.EndArray();
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array1\": [\n  \"array2\": [\n  ]\n]");

  json_writer.Reset();
  json_writer.BeginArray("array1");
  json_writer.BeginArray();
  json_writer.EndArray();
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array1\": [\n  [\n  ]\n]");
}

TEST_F(GltfUtilsTest, TestGltfValues) {
  JsonWriter json_writer;
  const int8_t int8_value_min = std::numeric_limits<int8_t>::min();
  const int8_t int8_value_max = std::numeric_limits<int8_t>::max();
  const GltfValue int8_value_low(int8_value_min);
  const GltfValue int8_value_high(int8_value_max);
  json_writer.OutputValue(int8_value_low);
  json_writer.OutputValue(int8_value_high);
  CompareGolden(&json_writer, "-128,\n127");

  json_writer.Reset();
  const uint8_t uint8_value_min = std::numeric_limits<uint8_t>::min();
  const uint8_t uint8_value_max = std::numeric_limits<uint8_t>::max();
  const GltfValue uint8_value_low(uint8_value_min);
  const GltfValue uint8_value_high(uint8_value_max);
  json_writer.OutputValue(uint8_value_low);
  json_writer.OutputValue(uint8_value_high);
  CompareGolden(&json_writer, "0,\n255");

  json_writer.Reset();
  const int16_t int16_value_min = std::numeric_limits<int16_t>::min();
  const int16_t int16_value_max = std::numeric_limits<int16_t>::max();
  const GltfValue int16_value_low(int16_value_min);
  const GltfValue int16_value_high(int16_value_max);
  json_writer.OutputValue(int16_value_low);
  json_writer.OutputValue(int16_value_high);
  CompareGolden(&json_writer, "-32768,\n32767");

  json_writer.Reset();
  const uint16_t uint16_value_min = std::numeric_limits<uint16_t>::min();
  const uint16_t uint16_value_max = std::numeric_limits<uint16_t>::max();
  const GltfValue uint16_value_low(uint16_value_min);
  const GltfValue uint16_value_high(uint16_value_max);
  json_writer.OutputValue(uint16_value_low);
  json_writer.OutputValue(uint16_value_high);
  CompareGolden(&json_writer, "0,\n65535");

  json_writer.Reset();
  const uint32_t uint32_value_min = std::numeric_limits<uint32_t>::min();
  const uint32_t uint32_value_max = std::numeric_limits<uint32_t>::max();
  const GltfValue uint32_value_low(uint32_value_min);
  const GltfValue uint32_value_high(uint32_value_max);
  json_writer.OutputValue(uint32_value_low);
  json_writer.OutputValue(uint32_value_high);
  CompareGolden(&json_writer, "0,\n4294967295");

  json_writer.Reset();
  const float float_value_min = std::numeric_limits<float>::min();
  const float float_value_max = std::numeric_limits<float>::max();
  const GltfValue float_value_low(float_value_min);
  const GltfValue float_value_high(float_value_max);
  json_writer.OutputValue(float_value_low);
  json_writer.OutputValue(float_value_high);
  CompareGolden(&json_writer,
                "1.1754943508222875e-38,\n3.4028234663852886e+38");

  json_writer.Reset();
  const GltfValue float_value_0(0.1f);
  const GltfValue float_value_1(1.f);
  json_writer.OutputValue(float_value_0);
  json_writer.OutputValue(float_value_1);
  CompareGolden(&json_writer, "0.10000000149011612,\n1");
}

TEST_F(GltfUtilsTest, TestObjectsCompact) {
  JsonWriter json_writer;
  json_writer.SetMode(JsonWriter::COMPACT);
  json_writer.BeginObject();
  json_writer.EndObject();
  CompareGolden(&json_writer, "{}");

  json_writer.Reset();
  json_writer.BeginObject("object");
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object\":{}");

  json_writer.Reset();
  json_writer.BeginObject("object");
  json_writer.OutputValue(0);
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object\":{0}");

  json_writer.Reset();
  json_writer.BeginObject("object");
  json_writer.OutputValue(0);
  json_writer.OutputValue(1);
  json_writer.OutputValue(2);
  json_writer.OutputValue(3);
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object\":{0,1,2,3}");

  json_writer.Reset();
  json_writer.BeginObject("object1");
  json_writer.EndObject();
  json_writer.BeginObject("object2");
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object1\":{},\"object2\":{}");

  json_writer.Reset();
  json_writer.BeginObject("object1");
  json_writer.BeginObject("object2");
  json_writer.EndObject();
  json_writer.EndObject();
  CompareGolden(&json_writer, "\"object1\":{\"object2\":{}}");
}

TEST_F(GltfUtilsTest, TestArraysCompact) {
  JsonWriter json_writer;
  json_writer.SetMode(JsonWriter::COMPACT);
  json_writer.BeginArray("array");
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array\":[]");

  json_writer.Reset();
  json_writer.BeginArray("array");
  json_writer.OutputValue(0);
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array\":[0]");

  json_writer.Reset();
  json_writer.BeginArray("array");
  json_writer.OutputValue(0);
  json_writer.OutputValue(1);
  json_writer.OutputValue(2);
  json_writer.OutputValue(3);
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array\":[0,1,2,3]");

  json_writer.Reset();
  json_writer.BeginArray("array1");
  json_writer.EndArray();
  json_writer.BeginArray("array2");
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array1\":[],\"array2\":[]");

  json_writer.Reset();
  json_writer.BeginArray("array1");
  json_writer.BeginArray("array2");
  json_writer.EndArray();
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array1\":[\"array2\":[]]");

  json_writer.Reset();
  json_writer.BeginArray("array1");
  json_writer.BeginArray();
  json_writer.EndArray();
  json_writer.EndArray();
  CompareGolden(&json_writer, "\"array1\":[[]]");
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
