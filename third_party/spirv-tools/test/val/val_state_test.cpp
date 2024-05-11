// Copyright (c) 2015-2016 The Khronos Group Inc.
// Copyright (c) 2016 Google Inc.
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

// Unit tests for ValidationState_t.

#include <vector>

#include "gtest/gtest.h"
#include "source/latest_version_spirv_header.h"

#include "source/enum_set.h"
#include "source/extensions.h"
#include "source/spirv_validator_options.h"
#include "source/val/construct.h"
#include "source/val/function.h"
#include "source/val/validate.h"
#include "source/val/validation_state.h"

namespace spvtools {
namespace val {
namespace {

// This is all we need for these tests.
static uint32_t kFakeBinary[] = {0};

// A test with a ValidationState_t member transparently.
class ValidationStateTest : public testing::Test {
 public:
  ValidationStateTest()
      : context_(spvContextCreate(SPV_ENV_UNIVERSAL_1_0)),
        options_(spvValidatorOptionsCreate()),
        state_(context_, options_, kFakeBinary, 0, 1) {}

  ~ValidationStateTest() override {
    spvContextDestroy(context_);
    spvValidatorOptionsDestroy(options_);
  }

 protected:
  spv_context context_;
  spv_validator_options options_;
  ValidationState_t state_;
};

// A test of ValidationState_t::HasAnyOfCapabilities().
using ValidationState_HasAnyOfCapabilities = ValidationStateTest;

TEST_F(ValidationState_HasAnyOfCapabilities, EmptyMask) {
  EXPECT_TRUE(state_.HasAnyOfCapabilities({}));
  state_.RegisterCapability(spv::Capability::Matrix);
  EXPECT_TRUE(state_.HasAnyOfCapabilities({}));
  state_.RegisterCapability(spv::Capability::ImageMipmap);
  EXPECT_TRUE(state_.HasAnyOfCapabilities({}));
  state_.RegisterCapability(spv::Capability::Pipes);
  EXPECT_TRUE(state_.HasAnyOfCapabilities({}));
  state_.RegisterCapability(spv::Capability::StorageImageArrayDynamicIndexing);
  EXPECT_TRUE(state_.HasAnyOfCapabilities({}));
  state_.RegisterCapability(spv::Capability::ClipDistance);
  EXPECT_TRUE(state_.HasAnyOfCapabilities({}));
  state_.RegisterCapability(spv::Capability::StorageImageWriteWithoutFormat);
  EXPECT_TRUE(state_.HasAnyOfCapabilities({}));
}

TEST_F(ValidationState_HasAnyOfCapabilities, SingleCapMask) {
  EXPECT_FALSE(state_.HasAnyOfCapabilities({spv::Capability::Matrix}));
  EXPECT_FALSE(state_.HasAnyOfCapabilities({spv::Capability::ImageMipmap}));
  state_.RegisterCapability(spv::Capability::Matrix);
  EXPECT_TRUE(state_.HasAnyOfCapabilities({spv::Capability::Matrix}));
  EXPECT_FALSE(state_.HasAnyOfCapabilities({spv::Capability::ImageMipmap}));
  state_.RegisterCapability(spv::Capability::ImageMipmap);
  EXPECT_TRUE(state_.HasAnyOfCapabilities({spv::Capability::Matrix}));
  EXPECT_TRUE(state_.HasAnyOfCapabilities({spv::Capability::ImageMipmap}));
}

TEST_F(ValidationState_HasAnyOfCapabilities, MultiCapMask) {
  const auto set1 =
      CapabilitySet{spv::Capability::SampledRect, spv::Capability::ImageBuffer};
  const auto set2 =
      CapabilitySet{spv::Capability::StorageImageWriteWithoutFormat,
                    spv::Capability::StorageImageReadWithoutFormat,
                    spv::Capability::GeometryStreams};
  EXPECT_FALSE(state_.HasAnyOfCapabilities(set1));
  EXPECT_FALSE(state_.HasAnyOfCapabilities(set2));
  state_.RegisterCapability(spv::Capability::ImageBuffer);
  EXPECT_TRUE(state_.HasAnyOfCapabilities(set1));
  EXPECT_FALSE(state_.HasAnyOfCapabilities(set2));
}

// A test of ValidationState_t::HasAnyOfExtensions().
using ValidationState_HasAnyOfExtensions = ValidationStateTest;

TEST_F(ValidationState_HasAnyOfExtensions, EmptyMask) {
  EXPECT_TRUE(state_.HasAnyOfExtensions({}));
  state_.RegisterExtension(Extension::kSPV_KHR_shader_ballot);
  EXPECT_TRUE(state_.HasAnyOfExtensions({}));
  state_.RegisterExtension(Extension::kSPV_KHR_16bit_storage);
  EXPECT_TRUE(state_.HasAnyOfExtensions({}));
  state_.RegisterExtension(Extension::kSPV_NV_viewport_array2);
  EXPECT_TRUE(state_.HasAnyOfExtensions({}));
}

TEST_F(ValidationState_HasAnyOfExtensions, SingleCapMask) {
  EXPECT_FALSE(state_.HasAnyOfExtensions({Extension::kSPV_KHR_shader_ballot}));
  EXPECT_FALSE(state_.HasAnyOfExtensions({Extension::kSPV_KHR_16bit_storage}));
  state_.RegisterExtension(Extension::kSPV_KHR_shader_ballot);
  EXPECT_TRUE(state_.HasAnyOfExtensions({Extension::kSPV_KHR_shader_ballot}));
  EXPECT_FALSE(state_.HasAnyOfExtensions({Extension::kSPV_KHR_16bit_storage}));
  state_.RegisterExtension(Extension::kSPV_KHR_16bit_storage);
  EXPECT_TRUE(state_.HasAnyOfExtensions({Extension::kSPV_KHR_shader_ballot}));
  EXPECT_TRUE(state_.HasAnyOfExtensions({Extension::kSPV_KHR_16bit_storage}));
}

TEST_F(ValidationState_HasAnyOfExtensions, MultiCapMask) {
  const auto set1 = ExtensionSet{Extension::kSPV_KHR_multiview,
                                 Extension::kSPV_KHR_16bit_storage};
  const auto set2 = ExtensionSet{Extension::kSPV_KHR_shader_draw_parameters,
                                 Extension::kSPV_NV_stereo_view_rendering,
                                 Extension::kSPV_KHR_shader_ballot};
  EXPECT_FALSE(state_.HasAnyOfExtensions(set1));
  EXPECT_FALSE(state_.HasAnyOfExtensions(set2));
  state_.RegisterExtension(Extension::kSPV_KHR_multiview);
  EXPECT_TRUE(state_.HasAnyOfExtensions(set1));
  EXPECT_FALSE(state_.HasAnyOfExtensions(set2));
}

// A test of ValidationState_t::IsOpcodeInCurrentLayoutSection().
using ValidationState_InLayoutState = ValidationStateTest;

TEST_F(ValidationState_InLayoutState, Variable) {
  state_.SetCurrentLayoutSectionForTesting(kLayoutTypes);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpVariable));

  state_.SetCurrentLayoutSectionForTesting(kLayoutFunctionDefinitions);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpVariable));
}

TEST_F(ValidationState_InLayoutState, ExtInst) {
  state_.SetCurrentLayoutSectionForTesting(kLayoutTypes);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpExtInst));

  state_.SetCurrentLayoutSectionForTesting(kLayoutFunctionDefinitions);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpExtInst));
}

TEST_F(ValidationState_InLayoutState, Undef) {
  state_.SetCurrentLayoutSectionForTesting(kLayoutTypes);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpUndef));

  state_.SetCurrentLayoutSectionForTesting(kLayoutFunctionDefinitions);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpUndef));
}

TEST_F(ValidationState_InLayoutState, Function) {
  state_.SetCurrentLayoutSectionForTesting(kLayoutFunctionDeclarations);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpFunction));

  state_.SetCurrentLayoutSectionForTesting(kLayoutFunctionDefinitions);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpFunction));
}

TEST_F(ValidationState_InLayoutState, FunctionParameter) {
  state_.SetCurrentLayoutSectionForTesting(kLayoutFunctionDeclarations);
  EXPECT_TRUE(
      state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpFunctionParameter));

  state_.SetCurrentLayoutSectionForTesting(kLayoutFunctionDefinitions);
  EXPECT_TRUE(
      state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpFunctionParameter));
}

TEST_F(ValidationState_InLayoutState, FunctionEnd) {
  state_.SetCurrentLayoutSectionForTesting(kLayoutFunctionDeclarations);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpFunctionEnd));

  state_.SetCurrentLayoutSectionForTesting(kLayoutFunctionDefinitions);
  EXPECT_TRUE(state_.IsOpcodeInCurrentLayoutSection(spv::Op::OpFunctionEnd));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
