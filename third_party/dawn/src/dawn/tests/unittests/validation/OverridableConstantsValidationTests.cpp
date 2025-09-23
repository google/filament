// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <limits>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ComputePipelineOverridableConstantsValidationTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::ShaderF16};
    }

    void SetUpShadersWithDefaultValueConstants() {
        computeModule = utils::CreateShaderModule(device, R"(
enable f16;

override c0: bool = true;            // type: bool
override c1: bool = false;           // default override
override c2: f32 = 0.0;              // type: float32
override c3: f32 = 0.0;              // default override
override c4: f32 = 4.0;              // default
override c5: i32 = 0;                // type: int32
override c6: i32 = 0;                // default override
override c7: i32 = 7;                // default
override c8: u32 = 0u;               // type: uint32
override c9: u32 = 0u;               // default override
override c10: u32 = 10u;             // default
override c11: f16 = 0.0h;            // type: float16
override c12: f16 = 0.0h;            // default override
@id(1000) override c13: f16 = 4.0h;  // default

override u01: f16 = 0.0h;            // default override
@id(2000) override u02: f16 = 0.0h;  // default override

override u10: f32;
override u11: i32;
override u12: u32;
override u13: f16;

@compute @workgroup_size(1) fn main() {
    // make sure the overridable constants are not optimized out
    _ = u32(c0);
    _ = u32(c1);
    _ = u32(c2);
    _ = u32(c3);
    _ = u32(c4);
    _ = u32(c5);
    _ = u32(c6);
    _ = u32(c7);
    _ = u32(c8);
    _ = u32(c9);
    _ = u32(c10);
    _ = u32(c11);
    _ = u32(c12);
    _ = u32(c13);
})");
    }

    void SetUpShadersWithUninitializedConstants() {
        computeModule = utils::CreateShaderModule(device, R"(
enable f16;

override c0: bool;                   // type: bool
override c1: bool = false;           // default override
override c2: f32;                    // type: float32
override c3: f32 = 0.0;              // default override
override c4: f32 = 4.0;              // default
override c5: i32;                    // type: int32
override c6: i32 = 0;                // default override
override c7: i32 = 7;                // default
override c8: u32;                    // type: uint32
override c9: u32 = 0u;               // default override
override c10: u32 = 10u;             // default
override c11: f16;                   // type: float16
override c12: f16 = 0.0h;            // default override
@id(1000) override c13: f16 = 4.0h;  // default

@compute @workgroup_size(1) fn main() {
    // make sure the overridable constants are not optimized out
    _ = u32(c0);
    _ = u32(c1);
    _ = u32(c2);
    _ = u32(c3);
    _ = u32(c4);
    _ = u32(c5);
    _ = u32(c6);
    _ = u32(c7);
    _ = u32(c8);
    _ = u32(c9);
    _ = u32(c10);
    _ = u32(c11);
    _ = u32(c12);
    _ = u32(c13);
})");
    }

    void TestCreatePipeline(const std::vector<wgpu::ConstantEntry>& constants) {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = computeModule;
        csDesc.compute.constants = constants.data();
        csDesc.compute.constantCount = constants.size();
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);
    }

    wgpu::ShaderModule computeModule;
    wgpu::Buffer buffer;
};

// Basic constants lookup tests
TEST_F(ComputePipelineOverridableConstantsValidationTest, CreateShaderWithOverride) {
    SetUpShadersWithUninitializedConstants();
}

// Basic constants lookup tests
TEST_F(ComputePipelineOverridableConstantsValidationTest, ConstantsIdentifierLookUp) {
    SetUpShadersWithDefaultValueConstants();
    {
        // Valid: no constants specified
        std::vector<wgpu::ConstantEntry> constants;
        TestCreatePipeline(constants);
    }
    {
        // Valid: find by constant name
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c0", 0}};
        TestCreatePipeline(constants);
    }
    {
        // Valid: in module but unused by entry point.
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "u01", 0}};
        TestCreatePipeline(constants);
    }
    {
        // Valid: in module but unused by entry point.
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "2000", 0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: set the same constant twice
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", 0},
            {nullptr, "c0", 1},
        };
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: find by constant numeric id
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "1000", 0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: c13 already has a constant numeric id specified
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c13", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: constant numeric id not specified
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "9999", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: constant name doesn't exit
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c99", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}

// Test that it is invalid to leave any constants uninitialized
TEST_F(ComputePipelineOverridableConstantsValidationTest, UninitializedConstants) {
    SetUpShadersWithUninitializedConstants();
    {
        // Error: uninitialized constants exist
        std::vector<wgpu::ConstantEntry> constants;
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: uninitialized constants exist
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", 0.0},
            {nullptr, "c2", 1},
            // c5 is missing
            {nullptr, "c8", 1},
            {nullptr, "c11", 1},
        };
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: all constants initialized
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", 0.0}, {nullptr, "c2", 1},  {nullptr, "c5", 1},
            {nullptr, "c8", 1},   {nullptr, "c11", 1},
        };
        TestCreatePipeline(constants);
    }
    {
        // Error: duplicate initializations
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", 0.0}, {nullptr, "c2", 1},  {nullptr, "c5", 1},
            {nullptr, "c8", 1},   {nullptr, "c11", 1}, {nullptr, "c2", 2},
        };
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}

// Test that only explicitly specified numeric ID can be referenced
TEST_F(ComputePipelineOverridableConstantsValidationTest, ConstantsIdentifierExplicitNumericID) {
    SetUpShadersWithDefaultValueConstants();
    {
        // Error: constant numeric id not explicitly specified
        // But could be impliciltly assigned to one of the constants
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "0", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: constant numeric id not explicitly specified
        // But could be impliciltly assigned to one of the constants
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "1", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: constant numeric id not explicitly specified
        // But could be impliciltly assigned to one of the constants
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "2", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: constant numeric id not explicitly specified
        // But could be impliciltly assigned to one of the constants
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "3", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}

// Test that identifiers are unique
TEST_F(ComputePipelineOverridableConstantsValidationTest, ConstantsIdentifierUnique) {
    SetUpShadersWithDefaultValueConstants();
    {
        // Valid: constant without numeric id can be referenced with variable name
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c0", 0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: constant with numeric id cannot be referenced with variable name
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c13", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}

// Test that values like NaN and Inf are treated as invalid.
TEST_F(ComputePipelineOverridableConstantsValidationTest, InvalidValue) {
    SetUpShadersWithDefaultValueConstants();
    {
        // Error: NaN
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c3", std::nan("")}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: -NaN
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c3", -std::nan("")}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: Inf
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c3", std::numeric_limits<double>::infinity()}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: -Inf
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c3", -std::numeric_limits<double>::infinity()}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: Max
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c3", std::numeric_limits<float>::max()}};
        TestCreatePipeline(constants);
    }
    {
        // Valid: Lowest
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c3", std::numeric_limits<float>::lowest()}};
        TestCreatePipeline(constants);
    }
}

// Test that values that are not representable by WGSL type i32/u32/f16/f32
TEST_F(ComputePipelineOverridableConstantsValidationTest, OutofRangeValue) {
    SetUpShadersWithDefaultValueConstants();
    {
        // Error: 1.79769e+308 cannot be represented by f32
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c3", std::numeric_limits<double>::max()}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: max f32 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c3", std::numeric_limits<float>::max()}};
        TestCreatePipeline(constants);
    }
    {
        // Error: one ULP higher than max f32 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c3",
             std::nextafter<double>(std::numeric_limits<float>::max(),
                                    std::numeric_limits<double>::max())}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: lowest f32 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c3", std::numeric_limits<float>::lowest()}};
        TestCreatePipeline(constants);
    }
    {
        // Error: one ULP lower than lowest f32 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c3",
             std::nextafter<double>(std::numeric_limits<float>::lowest(),
                                    std::numeric_limits<double>::lowest())}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: i32 out of range
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c5", static_cast<double>(std::numeric_limits<int32_t>::max()) + 1.0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: i32 out of range (negative)
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c5", static_cast<double>(std::numeric_limits<int32_t>::lowest()) - 1.0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: u32 out of range
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c8", static_cast<double>(std::numeric_limits<uint32_t>::max()) + 1.0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: conversion to boolean can't fail
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", static_cast<double>(std::numeric_limits<int32_t>::max()) + 1.0}};
        TestCreatePipeline(constants);
    }
    {
        // Valid: max f16 representable value
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c11", 65504.0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: one ULP higher than max f16 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c11", std::nextafter<double>(65504.0, std::numeric_limits<double>::max())}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: lowest f16 representable value
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c11", -65504.0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: one ULP lower than lowest f16 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c11",
             std::nextafter<double>(-65504.0, std::numeric_limits<double>::lowest())}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}

// Test that values that are not representable by WGSL type i32/u32/f16/f32 for unused overrides are
// errors
TEST_F(ComputePipelineOverridableConstantsValidationTest, UnusedOutofRangeValue) {
    SetUpShadersWithDefaultValueConstants();
    {
        // Error: 1.79769e+308 cannot be represented by f32
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u10", std::numeric_limits<double>::max()}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: max f32 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u10", std::numeric_limits<float>::max()}};
        TestCreatePipeline(constants);
    }
    {
        // Error: one ULP higher than max f32 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u10",
             std::nextafter<double>(std::numeric_limits<float>::max(),
                                    std::numeric_limits<double>::max())}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: lowest f32 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u10", std::numeric_limits<float>::lowest()}};
        TestCreatePipeline(constants);
    }
    {
        // Error: one ULP lower than lowest f32 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u10",
             std::nextafter<double>(std::numeric_limits<float>::lowest(),
                                    std::numeric_limits<double>::lowest())}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: i32 out of range
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u11", static_cast<double>(std::numeric_limits<int32_t>::max()) + 1.0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: i32 out of range (negative)
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u11", static_cast<double>(std::numeric_limits<int32_t>::lowest()) - 1.0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: u32 out of range
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u12", static_cast<double>(std::numeric_limits<uint32_t>::max()) + 1.0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: max f16 representable value
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c11", 65504.0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: one ULP higher than max f16 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u13", std::nextafter<double>(65504.0, std::numeric_limits<double>::max())}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: lowest f16 representable value
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "u13", -65504.0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: one ULP lower than lowest f16 representable value
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "u13",
             std::nextafter<double>(-65504.0, std::numeric_limits<double>::lowest())}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}

}  // anonymous namespace
}  // namespace dawn
