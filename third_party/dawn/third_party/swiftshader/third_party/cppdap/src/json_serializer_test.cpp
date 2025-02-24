// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "json_serializer.h"

#include "dap/typeinfo.h"
#include "dap/typeof.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dap {

struct JSONInnerTestObject {
  integer i;
};

DAP_STRUCT_TYPEINFO(JSONInnerTestObject,
                    "json-inner-test-object",
                    DAP_FIELD(i, "i"));

struct JSONTestObject {
  boolean b;
  integer i;
  number n;
  array<integer> a;
  object o;
  string s;
  optional<integer> o1;
  optional<integer> o2;
  JSONInnerTestObject inner;
};

DAP_STRUCT_TYPEINFO(JSONTestObject,
                    "json-test-object",
                    DAP_FIELD(b, "b"),
                    DAP_FIELD(i, "i"),
                    DAP_FIELD(n, "n"),
                    DAP_FIELD(a, "a"),
                    DAP_FIELD(o, "o"),
                    DAP_FIELD(s, "s"),
                    DAP_FIELD(o1, "o1"),
                    DAP_FIELD(o2, "o2"),
                    DAP_FIELD(inner, "inner"));

struct JSONObjectNoFields {};

DAP_STRUCT_TYPEINFO(JSONObjectNoFields, "json-object-no-fields");

}  // namespace dap

TEST(JSONSerializer, SerializeDeserialize) {
  dap::JSONTestObject encoded;
  encoded.b = true;
  encoded.i = 32;
  encoded.n = 123.456;
  encoded.a = {2, 4, 6, 8};
  encoded.o["one"] = dap::integer(1);
  encoded.o["two"] = dap::number(2);
  encoded.s = "hello world";
  encoded.o2 = 42;
  encoded.inner.i = 70;

  dap::json::Serializer s;
  ASSERT_TRUE(s.serialize(encoded));

  dap::JSONTestObject decoded;
  dap::json::Deserializer d(s.dump());
  ASSERT_TRUE(d.deserialize(&decoded));

  ASSERT_EQ(encoded.b, decoded.b);
  ASSERT_EQ(encoded.i, decoded.i);
  ASSERT_EQ(encoded.n, decoded.n);
  ASSERT_EQ(encoded.a, decoded.a);
  ASSERT_EQ(encoded.o["one"].get<dap::integer>(),
            decoded.o["one"].get<dap::integer>());
  ASSERT_EQ(encoded.o["two"].get<dap::number>(),
            decoded.o["two"].get<dap::number>());
  ASSERT_EQ(encoded.s, decoded.s);
  ASSERT_EQ(encoded.o2, decoded.o2);
  ASSERT_EQ(encoded.inner.i, decoded.inner.i);
}

TEST(JSONSerializer, SerializeObjectNoFields) {
  dap::JSONObjectNoFields obj;
  dap::json::Serializer s;
  ASSERT_TRUE(s.serialize(obj));
  ASSERT_EQ(s.dump(), "{}");
}
