// Copyright 2020 Google LLC
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

#include "dap/typeof.h"
#include "dap/types.h"
#include "json_serializer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dap {

struct BaseStruct {
  dap::integer i;
  dap::number n;
};

DAP_STRUCT_TYPEINFO(BaseStruct,
                    "BaseStruct",
                    DAP_FIELD(i, "i"),
                    DAP_FIELD(n, "n"));

struct DerivedStruct : public BaseStruct {
  dap::string s;
  dap::boolean b;
};

DAP_STRUCT_TYPEINFO_EXT(DerivedStruct,
                        BaseStruct,
                        "DerivedStruct",
                        DAP_FIELD(s, "s"),
                        DAP_FIELD(b, "b"));

}  // namespace dap

TEST(TypeInfo, Derived) {
  dap::DerivedStruct in;
  in.s = "hello world";
  in.b = true;
  in.i = 42;
  in.n = 3.14;

  dap::json::Serializer s;
  ASSERT_TRUE(s.serialize(in));

  dap::DerivedStruct out;
  dap::json::Deserializer d(s.dump());
  ASSERT_TRUE(d.deserialize(&out)) << "Failed to deserialize\n" << s.dump();

  ASSERT_EQ(out.s, "hello world");
  ASSERT_EQ(out.b, true);
  ASSERT_EQ(out.i, 42);
  ASSERT_EQ(out.n, 3.14);
}
