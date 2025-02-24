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

#include "dap/variant.h"
#include "dap/typeof.h"
#include "dap/types.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dap {

struct VariantTestObject {
  dap::integer i;
  dap::number n;
};

DAP_STRUCT_TYPEINFO(VariantTestObject,
                    "VariantTestObject",
                    DAP_FIELD(i, "i"),
                    DAP_FIELD(n, "n"));

}  // namespace dap

TEST(Variant, EmptyConstruct) {
  dap::variant<dap::integer, dap::boolean, dap::VariantTestObject> variant;
  ASSERT_TRUE(variant.is<dap::integer>());
  ASSERT_FALSE(variant.is<dap::boolean>());
  ASSERT_FALSE(variant.is<dap::VariantTestObject>());
}

TEST(Variant, Boolean) {
  dap::variant<dap::integer, dap::boolean, dap::VariantTestObject> variant(
      dap::boolean(true));
  ASSERT_TRUE(variant.is<dap::boolean>());
  ASSERT_EQ(variant.get<dap::boolean>(), dap::boolean(true));
}

TEST(Variant, Integer) {
  dap::variant<dap::integer, dap::boolean, dap::VariantTestObject> variant(
      dap::integer(10));
  ASSERT_TRUE(variant.is<dap::integer>());
  ASSERT_EQ(variant.get<dap::integer>(), dap::integer(10));
}

TEST(Variant, TestObject) {
  dap::variant<dap::integer, dap::boolean, dap::VariantTestObject> variant(
      dap::VariantTestObject{5, 3.0});
  ASSERT_TRUE(variant.is<dap::VariantTestObject>());
  ASSERT_EQ(variant.get<dap::VariantTestObject>().i, 5);
  ASSERT_EQ(variant.get<dap::VariantTestObject>().n, 3.0);
}

TEST(Variant, Assign) {
  dap::variant<dap::integer, dap::boolean, dap::VariantTestObject> variant(
      dap::integer(10));
  variant = dap::integer(10);
  ASSERT_TRUE(variant.is<dap::integer>());
  ASSERT_FALSE(variant.is<dap::boolean>());
  ASSERT_FALSE(variant.is<dap::VariantTestObject>());
  ASSERT_EQ(variant.get<dap::integer>(), dap::integer(10));
  variant = dap::boolean(true);
  ASSERT_FALSE(variant.is<dap::integer>());
  ASSERT_TRUE(variant.is<dap::boolean>());
  ASSERT_FALSE(variant.is<dap::VariantTestObject>());
  ASSERT_EQ(variant.get<dap::boolean>(), dap::boolean(true));
  variant = dap::VariantTestObject{5, 3.0};
  ASSERT_FALSE(variant.is<dap::integer>());
  ASSERT_FALSE(variant.is<dap::boolean>());
  ASSERT_TRUE(variant.is<dap::VariantTestObject>());
  ASSERT_EQ(variant.get<dap::VariantTestObject>().i, 5);
  ASSERT_EQ(variant.get<dap::VariantTestObject>().n, 3.0);
}

TEST(Variant, Accepts) {
  using variant =
      dap::variant<dap::integer, dap::boolean, dap::VariantTestObject>;
  ASSERT_TRUE(variant::accepts<dap::integer>());
  ASSERT_TRUE(variant::accepts<dap::boolean>());
  ASSERT_TRUE(variant::accepts<dap::VariantTestObject>());
  ASSERT_FALSE(variant::accepts<dap::number>());
  ASSERT_FALSE(variant::accepts<dap::string>());
}
