// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <array>
#include <iostream>

#include "gmock/gmock.h"
#include "source/spirv_target_env.h"
#include "source/table2.h"
#include "test/unit_spirv.h"

using ::testing::ContainerEq;
using ::testing::ValuesIn;

namespace spvtools {
namespace {

struct ExtInstLookupCase {
  spv_ext_inst_type_t type;
  std::string name;
  uint32_t value;
  bool expect_pass = true;
};

std::ostream& operator<<(std::ostream& os, const ExtInstLookupCase& eilc) {
  os << "EILC(" << static_cast<int>(eilc.type) << ", '" << eilc.name << "', "
     << eilc.value << ", expect pass? " << eilc.expect_pass << ")";
  return os;
}

using ExtInstLookupTest = ::testing::TestWithParam<ExtInstLookupCase>;

TEST_P(ExtInstLookupTest, ExtInstLookup_ByName) {
  const ExtInstDesc* desc = nullptr;
  auto status = LookupExtInst(GetParam().type, GetParam().name.data(), &desc);
  if (GetParam().expect_pass) {
    EXPECT_EQ(status, SPV_SUCCESS);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(static_cast<uint32_t>(desc->value), GetParam().value);
  } else {
    EXPECT_NE(status, SPV_SUCCESS);
    EXPECT_EQ(desc, nullptr);
  }
}

TEST_P(ExtInstLookupTest, ExtInstLookup_ByValue_Success) {
  const ExtInstDesc* desc = nullptr;
  if (GetParam().expect_pass) {
    auto status = LookupExtInst(GetParam().type, GetParam().value, &desc);
    EXPECT_EQ(status, SPV_SUCCESS);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->value, GetParam().value);
    EXPECT_EQ(std::string(desc->name().data()), GetParam().name);
  }
}

INSTANTIATE_TEST_SUITE_P(Samples, ExtInstLookupTest,
                         ValuesIn(std::vector<ExtInstLookupCase>{
                             {SPV_EXT_INST_TYPE_GLSL_STD_450, "FMix", 46},
                             {SPV_EXT_INST_TYPE_OPENCL_STD, "s_mul24", 169},
                             {SPV_EXT_INST_TYPE_OPENCL_STD, "mix", 99},
                         }));

TEST(ExtInstLookupSingleTest, ExtInstLookup_Value_Fails) {
  // This list may need adjusting over time.
  std::array<uint32_t, 3> bad_values = {{99999, 37737, 110101}};
  for (auto bad_value : bad_values) {
    const ExtInstDesc* desc = nullptr;
    auto status = LookupExtInst(SPV_EXT_INST_TYPE_OPENCL_STD, bad_value, &desc);
    EXPECT_NE(status, SPV_SUCCESS);
    ASSERT_EQ(desc, nullptr);
  }
}

}  // namespace
}  // namespace spvtools
