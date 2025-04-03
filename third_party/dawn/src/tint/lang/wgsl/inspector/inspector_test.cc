// Copyright 2020 The Dawn & Tint Authors
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

#include <algorithm>
#include <unordered_set>

#include "gmock/gmock.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/inspector/entry_point.h"
#include "src/tint/lang/wgsl/inspector/inspector.h"
#include "src/tint/lang/wgsl/reader/reader.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::inspector {
namespace {

/// Utility class for building programs in inspector tests
class TestHelper {
  public:
    /// Create a Program with Inspector from the provided WGSL shader.
    /// Should only be called once per test and cannot be used with Build.
    /// @param shader a WGSL shader
    /// @returns a reference to the Inspector for the built Program.
    Inspector& Initialize(std::string shader) {
        if (inspector_) {
            return *inspector_;
        }

        wgsl::reader::Options options;
        options.allowed_features = wgsl::AllowedFeatures::Everything();
        file_ = std::make_unique<Source::File>("test", shader);
        program_ = std::make_unique<Program>(wgsl::reader::Parse(file_.get(), options));
        if (!program_->IsValid()) {
            ADD_FAILURE() << program_->Diagnostics();
        }
        inspector_ = std::make_unique<Inspector>(*program_);
        return *inspector_;
    }

  protected:
    /// File created from input shader and used to create Program.
    std::unique_ptr<Source::File> file_;
    /// Program created by this runner.
    std::unique_ptr<Program> program_;
    /// Inspector for |program_|
    std::unique_ptr<Inspector> inspector_;
};

class InspectorTest : public TestHelper, public testing::Test {};

template <typename T>
class InspectorTestWithParam : public TestHelper, public testing::TestWithParam<T> {};

using InspectorGetEntryPointTest = InspectorTest;
using InspectorOverridesTest = InspectorTest;
using InspectorGetOverrideDefaultValuesTest = InspectorTest;
using InspectorGetConstantNameToIdMapTest = InspectorTest;
using InspectorGetResourceBindingsTest = InspectorTest;
using InspectorGetUsedExtensionNamesTest = InspectorTest;
using InspectorGetEnableDirectivesTest = InspectorTest;
using InspectorGetBlendSrcTest = InspectorTest;
using InspectorSubgroupMatrixTest = InspectorTest;
using InspectorTextureTest = InspectorTest;

// This is a catch all for shaders that have demonstrated regressions/crashes in the wild.
using InspectorRegressionTest = InspectorTest;

TEST_F(InspectorGetEntryPointTest, NoFunctions) {
    auto* src = R"(
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, NoEntryPoints) {
    auto* src = R"(
fn foo() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, OneEntryPoint) {
    auto* src = R"(
@fragment fn foo() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ("foo", result[0].name);
    EXPECT_EQ("foo", result[0].remapped_name);
    EXPECT_EQ(PipelineStage::kFragment, result[0].stage);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPoints) {
    auto* src = R"(
@fragment fn foo() {}
@compute @workgroup_size(1i) fn bar() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(2u, result.size());
    EXPECT_EQ("foo", result[0].name);
    EXPECT_EQ("foo", result[0].remapped_name);
    EXPECT_EQ(PipelineStage::kFragment, result[0].stage);
    EXPECT_EQ("bar", result[1].name);
    EXPECT_EQ("bar", result[1].remapped_name);
    EXPECT_EQ(PipelineStage::kCompute, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, MixFunctionsAndEntryPoints) {
    auto* src = R"(
fn func() {}

@compute @workgroup_size(1i)
fn foo() { func(); }

@fragment fn bar() { func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    EXPECT_FALSE(inspector.has_error());

    ASSERT_EQ(2u, result.size());
    EXPECT_EQ("foo", result[0].name);
    EXPECT_EQ("foo", result[0].remapped_name);
    EXPECT_EQ(PipelineStage::kCompute, result[0].stage);
    EXPECT_EQ("bar", result[1].name);
    EXPECT_EQ("bar", result[1].remapped_name);
    EXPECT_EQ(PipelineStage::kFragment, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, DefaultWorkgroupSize) {
    auto* src = R"(
@compute @workgroup_size(8i, 2i, 1i)  fn foo() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    auto workgroup_size = result[0].workgroup_size;
    ASSERT_TRUE(workgroup_size.has_value());
    EXPECT_EQ(8u, workgroup_size->x);
    EXPECT_EQ(2u, workgroup_size->y);
    EXPECT_EQ(1u, workgroup_size->z);
}

// Test that push_constant_size is zero if there are no push constants.
TEST_F(InspectorGetEntryPointTest, PushConstantSizeNone) {
    auto* src = R"(
@fragment fn foo() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].push_constant_size);
}

// Test that push_constant_size is 4 (bytes) if there is a single F32 push constant.
TEST_F(InspectorGetEntryPointTest, PushConstantSizeOneWord) {
    auto* src = R"(
enable chromium_experimental_push_constant;

var<push_constant> pc: f32;

@fragment fn foo() { _ = pc; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(4u, result[0].push_constant_size);
}

// Test that push_constant_size is 12 (bytes) if there is a struct containing one
// each of i32, f32 and u32.
TEST_F(InspectorGetEntryPointTest, PushConstantSizeThreeWords) {
    auto* src = R"(
enable chromium_experimental_push_constant;

struct S {
  a: i32,
  b: f32,
  c: u32,
}
var<push_constant> pc : S;

@fragment fn foo() { _ = pc; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(12u, result[0].push_constant_size);
}

// Test that push_constant_size is 4 (bytes) if there are two push constants,
// one used by the entry point containing an f32, and one unused by the entry
// point containing a struct of size 12 bytes.
TEST_F(InspectorGetEntryPointTest, PushConstantSizeTwoConstants) {
    auto* src = R"(
enable chromium_experimental_push_constant;

struct S {
  a: i32,
  b: f32,
  c: u32,
}
var<push_constant> unused : S;
var<push_constant> pc: f32;

@fragment fn foo() { _ = pc; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    // Check that the result only includes the single f32 push constant.
    EXPECT_EQ(4u, result[0].push_constant_size);
}

TEST_F(InspectorGetEntryPointTest, NonDefaultWorkgroupSize) {
    auto* src = R"(
@compute @workgroup_size(8i, 2i, 1i)
fn foo() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    auto workgroup_size = result[0].workgroup_size;
    ASSERT_TRUE(workgroup_size.has_value());
    EXPECT_EQ(8u, workgroup_size->x);
    EXPECT_EQ(2u, workgroup_size->y);
    EXPECT_EQ(1u, workgroup_size->z);
}

TEST_F(InspectorGetEntryPointTest, WorkgroupStorageSizeEmpty) {
    auto* src = R"(
@compute @workgroup_size(1i)
fn ep_func() {}
)";
    Inspector& inspector = Initialize(src);
    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].workgroup_storage_size);
}

TEST_F(InspectorGetEntryPointTest, WorkgroupStorageSizeSimple) {
    auto* src = R"(
var<workgroup> wg_f32: f32;
var<workgroup> wg_i32: i32;

fn f32_func() { _ = wg_f32; }
fn i32_func() { _ = wg_i32; }

@compute @workgroup_size(1i)
fn ep_func() {
  f32_func();
  i32_func();
}
)";
    Inspector& inspector = Initialize(src);
    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(32u, result[0].workgroup_storage_size);
}

TEST_F(InspectorGetEntryPointTest, WorkgroupStorageSizeCompoundTypes) {
    auto* src = R"(
// This struct should occupy 68 bytes.
struct WgStruct {
  a: i32,
  b: array<i32, 16>,
}
var<workgroup> wg_struct_var: WgStruct;

fn wg_struct_func() { _ = wg_struct_var.a; }

// Plus another 4 bytes from this other workgroup-class f32.
var<workgroup> wg_f32: f32;
fn f32_func() { _ = wg_f32; }

@compute @workgroup_size(1i)
fn ep_func() {
  wg_struct_func();
  f32_func();
}
)";
    Inspector& inspector = Initialize(src);
    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(96u, result[0].workgroup_storage_size);
}

TEST_F(InspectorGetEntryPointTest, WorkgroupStorageSizeAlignmentPadding) {
    auto* src = R"(
// vec3<f32> has an alignment of 16 but a size of 12. We leverage this to test
// that our padded size calculation for workgroup storage is accurate.
var<workgroup> wg_vec3: vec3f;

fn wg_func() { _ = wg_vec3; }

@compute @workgroup_size(1i)
fn ep_func() {
  wg_func();
}
)";
    Inspector& inspector = Initialize(src);
    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(16u, result[0].workgroup_storage_size);
}

TEST_F(InspectorGetEntryPointTest, WorkgroupStorageSizeStructAlignment) {
    auto* src = R"(
// Per WGSL spec, a struct's size is the offset its last member plus the size
// of its last member, rounded up to the alignment of its largest member. So
// here the struct is expected to occupy 1024 bytes of workgroup storage.
struct WgStruct {
  @align(1024i) a: f32,
}
var<workgroup> wg_struct_var: WgStruct;

fn wg_struct_func() { _ = wg_struct_var.a; }

@compute @workgroup_size(1i)
fn ep_func() {
  wg_struct_func();
}
)";
    Inspector& inspector = Initialize(src);
    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(1024u, result[0].workgroup_storage_size);
}

TEST_F(InspectorGetEntryPointTest, NoInOutVariables) {
    auto* src = R"(
fn func() {}
@fragment fn foo() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].input_variables.size());
    EXPECT_EQ(0u, result[0].output_variables.size());
}

std::string GetType(ComponentType component, CompositionType composition) {
    std::string comp;
    switch (component) {
        case ComponentType::kF32:
            comp = "f32";
            break;
        case ComponentType::kI32:
            comp = "i32";
            break;
        case ComponentType::kU32:
            comp = "u32";
            break;
        case ComponentType::kF16:
            comp = "f16";
            break;
        case ComponentType::kUnknown:
            TINT_UNREACHABLE();
    }

    uint32_t n;
    switch (composition) {
        case CompositionType::kScalar:
            return comp;
        case CompositionType::kVec2:
            n = 2;
            break;
        case CompositionType::kVec3:
            n = 3;
            break;
        case CompositionType::kVec4:
            n = 4;
            break;
        default:
            TINT_UNREACHABLE();
    }
    return std::string("vec") + std::to_string(n) + "<" + comp + ">";
}

typedef std::tuple<inspector::ComponentType, inspector::CompositionType>
    InspectorGetEntryPointComponentAndCompositionTestParams;
using InspectorGetEntryPointComponentAndCompositionTest =
    InspectorTestWithParam<InspectorGetEntryPointComponentAndCompositionTestParams>;

TEST_P(InspectorGetEntryPointComponentAndCompositionTest, Test) {
    ComponentType component;
    CompositionType composition;
    std::tie(component, composition) = GetParam();

    std::string src = "";
    if (component == ComponentType::kF16) {
        src += "enable f16;\n";
    }

    auto tint_type = GetType(component, composition);
    src += R"(
@fragment
fn foo(@location(0u) @interpolate(flat) in_var: )" +
           tint_type + ") -> @location(0) " + tint_type + R"( {
  return in_var;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ("in_var", result[0].input_variables[0].name);
    EXPECT_EQ("in_var", result[0].input_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].input_variables[0].attributes.location);
    EXPECT_EQ(component, result[0].input_variables[0].component_type);

    ASSERT_EQ(1u, result[0].output_variables.size());
    EXPECT_EQ("<retval>", result[0].output_variables[0].name);
    EXPECT_EQ("", result[0].output_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].output_variables[0].attributes.location);
    EXPECT_EQ(component, result[0].output_variables[0].component_type);
}
INSTANTIATE_TEST_SUITE_P(InspectorGetEntryPointTest,
                         InspectorGetEntryPointComponentAndCompositionTest,
                         testing::Combine(testing::Values(ComponentType::kF32,
                                                          ComponentType::kI32,
                                                          ComponentType::kU32,
                                                          ComponentType::kF16),
                                          testing::Values(CompositionType::kScalar,
                                                          CompositionType::kVec2,
                                                          CompositionType::kVec3,
                                                          CompositionType::kVec4)));

TEST_F(InspectorGetEntryPointTest, MultipleInOutVariables) {
    auto* src = R"(
enable chromium_experimental_framebuffer_fetch;

@fragment
fn foo(@location(0u) @interpolate(flat) in_var0: u32, @location(1u) @interpolate(flat) in_var1: u32, @color(2u) in_var4: u32) -> @location(0u) u32 {
  return in_var0;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(3u, result[0].input_variables.size());
    EXPECT_EQ("in_var0", result[0].input_variables[0].name);
    EXPECT_EQ("in_var0", result[0].input_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].input_variables[0].attributes.location);
    EXPECT_EQ(std::nullopt, result[0].input_variables[0].attributes.color);
    EXPECT_EQ(InterpolationType::kFlat, result[0].input_variables[0].interpolation_type);
    EXPECT_EQ(ComponentType::kU32, result[0].input_variables[0].component_type);
    EXPECT_EQ("in_var1", result[0].input_variables[1].name);
    EXPECT_EQ("in_var1", result[0].input_variables[1].variable_name);
    EXPECT_EQ(1u, result[0].input_variables[1].attributes.location);
    EXPECT_EQ(std::nullopt, result[0].input_variables[1].attributes.color);
    EXPECT_EQ(InterpolationType::kFlat, result[0].input_variables[1].interpolation_type);
    EXPECT_EQ(ComponentType::kU32, result[0].input_variables[1].component_type);
    EXPECT_EQ("in_var4", result[0].input_variables[2].name);
    EXPECT_EQ("in_var4", result[0].input_variables[2].variable_name);
    EXPECT_EQ(std::nullopt, result[0].input_variables[2].attributes.location);
    EXPECT_EQ(2u, result[0].input_variables[2].attributes.color);
    EXPECT_EQ(InterpolationType::kPerspective, result[0].input_variables[2].interpolation_type);
    EXPECT_EQ(ComponentType::kU32, result[0].input_variables[2].component_type);

    ASSERT_EQ(1u, result[0].output_variables.size());
    EXPECT_EQ("<retval>", result[0].output_variables[0].name);
    EXPECT_EQ("", result[0].output_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].output_variables[0].attributes.location);
    EXPECT_EQ(std::nullopt, result[0].output_variables[0].attributes.color);
    EXPECT_EQ(ComponentType::kU32, result[0].output_variables[0].component_type);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutVariables) {
    auto* src = R"(
@fragment
fn foo(@location(0u) @interpolate(flat) in_var_foo: u32) -> @location(0) u32 {
  return in_var_foo;
}

@fragment
fn bar(@location(0u) @interpolate(flat) in_var_bar: u32) -> @location(1u) u32 {
  return in_var_bar;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(2u, result.size());

    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ("in_var_foo", result[0].input_variables[0].name);
    EXPECT_EQ("in_var_foo", result[0].input_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].input_variables[0].attributes.location);
    EXPECT_EQ(InterpolationType::kFlat, result[0].input_variables[0].interpolation_type);
    EXPECT_EQ(ComponentType::kU32, result[0].input_variables[0].component_type);

    ASSERT_EQ(1u, result[0].output_variables.size());
    EXPECT_EQ("<retval>", result[0].output_variables[0].name);
    EXPECT_EQ("", result[0].output_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].output_variables[0].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].output_variables[0].component_type);

    ASSERT_EQ(1u, result[1].input_variables.size());
    EXPECT_EQ("in_var_bar", result[1].input_variables[0].name);
    EXPECT_EQ("in_var_bar", result[1].input_variables[0].variable_name);
    EXPECT_EQ(0u, result[1].input_variables[0].attributes.location);
    EXPECT_EQ(InterpolationType::kFlat, result[1].input_variables[0].interpolation_type);
    EXPECT_EQ(ComponentType::kU32, result[1].input_variables[0].component_type);

    ASSERT_EQ(1u, result[1].output_variables.size());
    EXPECT_EQ("<retval>", result[1].output_variables[0].name);
    EXPECT_EQ("", result[1].output_variables[0].variable_name);
    EXPECT_EQ(1u, result[1].output_variables[0].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[1].output_variables[0].component_type);
}

TEST_F(InspectorGetEntryPointTest, BuiltInsNotStageVariables) {
    auto* src = R"(
@fragment
fn foo(@builtin(sample_index) in_var0: u32, @location(0u) in_var1: f32) -> @builtin(frag_depth) f32 {
  return in_var1;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ("in_var1", result[0].input_variables[0].name);
    EXPECT_EQ("in_var1", result[0].input_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].input_variables[0].attributes.location);
    EXPECT_EQ(ComponentType::kF32, result[0].input_variables[0].component_type);

    ASSERT_EQ(0u, result[0].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, InOutStruct) {
    auto* src = R"(
struct Interface {
  @location(0u) @interpolate(flat) a: u32,
  @location(1u) @interpolate(flat) b: u32,
}
@fragment
fn foo(param: Interface) -> Interface {
  return param;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(2u, result[0].input_variables.size());
    EXPECT_EQ("param.a", result[0].input_variables[0].name);
    EXPECT_EQ("a", result[0].input_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].input_variables[0].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].input_variables[0].component_type);
    EXPECT_EQ("param.b", result[0].input_variables[1].name);
    EXPECT_EQ("b", result[0].input_variables[1].variable_name);
    EXPECT_EQ(1u, result[0].input_variables[1].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].input_variables[1].component_type);

    ASSERT_EQ(2u, result[0].output_variables.size());
    EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
    EXPECT_EQ("a", result[0].output_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].output_variables[0].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].output_variables[0].component_type);
    EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
    EXPECT_EQ("b", result[0].output_variables[1].variable_name);
    EXPECT_EQ(1u, result[0].output_variables[1].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].output_variables[1].component_type);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutSharedStruct) {
    auto* src = R"(
struct Interface {
  @location(0u) @interpolate(flat) a: u32,
  @location(1u) @interpolate(flat) b: u32,
}
@fragment
fn foo() -> Interface {
  return Interface();
}

@fragment
fn bar(param: Interface) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(2u, result.size());

    ASSERT_EQ(0u, result[0].input_variables.size());

    ASSERT_EQ(2u, result[0].output_variables.size());
    EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
    EXPECT_EQ("a", result[0].output_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].output_variables[0].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].output_variables[0].component_type);
    EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
    EXPECT_EQ("b", result[0].output_variables[1].variable_name);
    EXPECT_EQ(1u, result[0].output_variables[1].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].output_variables[1].component_type);

    ASSERT_EQ(2u, result[1].input_variables.size());
    EXPECT_EQ("param.a", result[1].input_variables[0].name);
    EXPECT_EQ("a", result[1].input_variables[0].variable_name);
    EXPECT_EQ(0u, result[1].input_variables[0].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[1].input_variables[0].component_type);
    EXPECT_EQ("param.b", result[1].input_variables[1].name);
    EXPECT_EQ("b", result[1].input_variables[1].variable_name);
    EXPECT_EQ(1u, result[1].input_variables[1].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[1].input_variables[1].component_type);

    ASSERT_EQ(0u, result[1].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, MixInOutVariablesAndStruct) {
    auto* src = R"(
struct struct_a {
  @location(0u) @interpolate(flat) a: u32,
  @location(1u) @interpolate(flat) b: u32,
}
struct struct_b {
  @location(2u) @interpolate(flat) a: u32,
}

@fragment
fn foo(param_a: struct_a, param_b: struct_b, @location(3u) param_c: f32, @location(4u) param_d: f32) -> struct_a {
  return param_a;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(5u, result[0].input_variables.size());
    EXPECT_EQ("param_a.a", result[0].input_variables[0].name);
    EXPECT_EQ("a", result[0].input_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].input_variables[0].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].input_variables[0].component_type);
    EXPECT_EQ("param_a.b", result[0].input_variables[1].name);
    EXPECT_EQ("b", result[0].input_variables[1].variable_name);
    EXPECT_EQ(1u, result[0].input_variables[1].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].input_variables[1].component_type);
    EXPECT_EQ("param_b.a", result[0].input_variables[2].name);
    EXPECT_EQ("a", result[0].input_variables[2].variable_name);
    EXPECT_EQ(2u, result[0].input_variables[2].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].input_variables[2].component_type);
    EXPECT_EQ("param_c", result[0].input_variables[3].name);
    EXPECT_EQ("param_c", result[0].input_variables[3].variable_name);
    EXPECT_EQ(3u, result[0].input_variables[3].attributes.location);
    EXPECT_EQ(ComponentType::kF32, result[0].input_variables[3].component_type);
    EXPECT_EQ("param_d", result[0].input_variables[4].name);
    EXPECT_EQ("param_d", result[0].input_variables[4].variable_name);
    EXPECT_EQ(4u, result[0].input_variables[4].attributes.location);
    EXPECT_EQ(ComponentType::kF32, result[0].input_variables[4].component_type);

    ASSERT_EQ(2u, result[0].output_variables.size());
    EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
    EXPECT_EQ("a", result[0].output_variables[0].variable_name);
    EXPECT_EQ(0u, result[0].output_variables[0].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].output_variables[0].component_type);
    EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
    EXPECT_EQ("b", result[0].output_variables[1].variable_name);
    EXPECT_EQ(1u, result[0].output_variables[1].attributes.location);
    EXPECT_EQ(ComponentType::kU32, result[0].output_variables[1].component_type);
}

TEST_F(InspectorGetEntryPointTest, OverrideUnreferenced) {
    auto* src = R"(
override foo: f32;

@compute @workgroup_size(1i)
fn ep_func() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].overrides.size());
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedByEntryPoint) {
    auto* src = R"(
override foo: f32;

@compute @workgroup_size(1i)
fn ep_func() { _ = foo; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overrides.size());
    EXPECT_EQ("foo", result[0].overrides[0].name);
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedByCallee) {
    auto* src = R"(
override foo: f32;

fn callee_func() { _ = foo; }

@compute @workgroup_size(1i)
fn ep_func() {
  callee_func();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overrides.size());
    EXPECT_EQ("foo", result[0].overrides[0].name);
}

TEST_F(InspectorGetEntryPointTest, OverrideSomeReferenced) {
    auto* src = R"(
@id(1) override foo: f32;
@id(2) override bar: f32;

fn callee_fn() { _ = foo; }

@compute @workgroup_size(1i)
fn ep_func() {
  callee_fn();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overrides.size());
    EXPECT_EQ("foo", result[0].overrides[0].name);
    EXPECT_EQ(1, result[0].overrides[0].id.value);
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedIndirectly) {
    auto* src = R"(
override foo: f32;
override bar: f32 = 2 * foo;

@compute @workgroup_size(1i)
fn ep_func() { _ = bar; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(2u, result[0].overrides.size());
    EXPECT_EQ("bar", result[0].overrides[0].name);
    EXPECT_TRUE(result[0].overrides[0].is_initialized);
    EXPECT_EQ("foo", result[0].overrides[1].name);
    EXPECT_FALSE(result[0].overrides[1].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedIndirectly_ViaPrivateInitializer) {
    auto* src = R"(
override foo: f32;
var<private> bar: f32 = 2 * foo;

@compute @workgroup_size(1i)
fn ep_func() { _ = bar; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overrides.size());
    EXPECT_EQ("foo", result[0].overrides[0].name);
    EXPECT_FALSE(result[0].overrides[0].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedIndirectly_MultipleEntryPoints) {
    auto* src = R"(
override foo1: f32;
override bar1: f32 = 2 * foo1;
@compute @workgroup_size(1i)
fn ep_func1() { _ = bar1; }

override foo2: f32;
override bar2: f32 = 2 * foo2;
@compute @workgroup_size(1i)
fn ep_func2() { _ = bar2; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(2u, result.size());

    ASSERT_EQ(2u, result[0].overrides.size());
    EXPECT_EQ("bar1", result[0].overrides[0].name);
    EXPECT_TRUE(result[0].overrides[0].is_initialized);
    EXPECT_EQ("foo1", result[0].overrides[1].name);
    EXPECT_FALSE(result[0].overrides[1].is_initialized);

    ASSERT_EQ(2u, result[1].overrides.size());
    EXPECT_EQ("bar2", result[1].overrides[0].name);
    EXPECT_TRUE(result[1].overrides[0].is_initialized);
    EXPECT_EQ("foo2", result[1].overrides[1].name);
    EXPECT_FALSE(result[1].overrides[1].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedByAttribute) {
    auto* src = R"(
override wgsize: u32;
@compute @workgroup_size(wgsize)
fn ep_func() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overrides.size());
    EXPECT_EQ("wgsize", result[0].overrides[0].name);
    EXPECT_FALSE(result[0].overrides[0].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedByAttributeIndirectly) {
    auto* src = R"(
override foo: u32;
override bar: u32 = 2 * foo;
@compute @workgroup_size(2 * bar)
fn ep_func() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(2u, result[0].overrides.size());
    EXPECT_EQ("bar", result[0].overrides[0].name);
    EXPECT_TRUE(result[0].overrides[0].is_initialized);
    EXPECT_EQ("foo", result[0].overrides[1].name);
    EXPECT_FALSE(result[0].overrides[1].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedByArraySize) {
    auto* src = R"(
override size: u32;
var<workgroup> v: array<f32, size>;

@compute @workgroup_size(1i)
fn ep() { _ = v[0]; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overrides.size());
    EXPECT_EQ("size", result[0].overrides[0].name);
    EXPECT_FALSE(result[0].overrides[0].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedByArraySizeIndirectly) {
    auto* src = R"(
override foo: u32;
override bar: u32 = 2 * foo;
var<workgroup> v: array<f32, 2 * bar>;

@compute @workgroup_size(1i)
fn ep() { _ = v[0]; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(2u, result[0].overrides.size());
    EXPECT_EQ("bar", result[0].overrides[0].name);
    EXPECT_TRUE(result[0].overrides[0].is_initialized);
    EXPECT_EQ("foo", result[0].overrides[1].name);
    EXPECT_FALSE(result[0].overrides[1].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideReferencedByArraySizeViaAlias) {
    auto* src = R"(
override foo: u32;
override bar: u32 = foo;
alias MyArray = array<f32, 2 * bar>;

override zoo: u32;
alias MyArrayUnused = array<f32, 2 * zoo>;

var<workgroup> v: MyArray;

@compute @workgroup_size(1i)
fn ep() { _ = v[0]; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(2u, result[0].overrides.size());
    EXPECT_EQ("bar", result[0].overrides[0].name);
    EXPECT_TRUE(result[0].overrides[0].is_initialized);
    EXPECT_EQ("foo", result[0].overrides[1].name);
    EXPECT_FALSE(result[0].overrides[1].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideTypes) {
    auto* src = R"(
enable f16;

override bool_var: bool;
override float_var: f32;
override u32_var: u32;
override i32_var: i32;
override f16_var: f16;

fn bool_func() { _ = bool_var; }
fn float_func() { _ = float_var; }
fn u32_func() { _ = u32_var; }
fn i32_func() { _ = i32_var; }
fn f16_func() { _ = f16_var; }

@compute @workgroup_size(1)
fn ep_func() {
  bool_func();
  float_func();
  u32_func();
  i32_func();
  f16_func();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(5u, result[0].overrides.size());
    EXPECT_EQ("bool_var", result[0].overrides[0].name);
    EXPECT_EQ(inspector::Override::Type::kBool, result[0].overrides[0].type);
    EXPECT_EQ("float_var", result[0].overrides[1].name);
    EXPECT_EQ(inspector::Override::Type::kFloat32, result[0].overrides[1].type);
    EXPECT_EQ("u32_var", result[0].overrides[2].name);
    EXPECT_EQ(inspector::Override::Type::kUint32, result[0].overrides[2].type);
    EXPECT_EQ("i32_var", result[0].overrides[3].name);
    EXPECT_EQ(inspector::Override::Type::kInt32, result[0].overrides[3].type);
    EXPECT_EQ("f16_var", result[0].overrides[4].name);
    EXPECT_EQ(inspector::Override::Type::kFloat16, result[0].overrides[4].type);
}

TEST_F(InspectorGetEntryPointTest, OverrideInitialized) {
    auto* src = R"(
override foo: f32 = 0f;
@compute @workgroup_size(1i)
fn ep_func() { _ = foo; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overrides.size());
    EXPECT_EQ("foo", result[0].overrides[0].name);
    EXPECT_TRUE(result[0].overrides[0].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideUninitialized) {
    auto* src = R"(
override foo: f32;
@compute @workgroup_size(1i)
fn ep_func() { _ = foo; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overrides.size());
    EXPECT_EQ("foo", result[0].overrides[0].name);

    EXPECT_FALSE(result[0].overrides[0].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverrideNumericIDSpecified) {
    auto* src = R"(
override foo_no_id: f32;
@id(1234) override foo_id: f32;

fn no_id_func() { _ = foo_no_id; }
fn id_func() { _ = foo_id; }

@compute @workgroup_size(1i)
fn ep_func() {
  no_id_func();
  id_func();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(2u, result[0].overrides.size());
    EXPECT_EQ("foo_no_id", result[0].overrides[0].name);
    EXPECT_EQ("foo_id", result[0].overrides[1].name);
    EXPECT_EQ(1234, result[0].overrides[1].id.value);

    EXPECT_FALSE(result[0].overrides[0].is_id_specified);
    EXPECT_TRUE(result[0].overrides[1].is_id_specified);
}

TEST_F(InspectorGetEntryPointTest, NonOverrideSkipped) {
    auto* src = R"(
struct foo_type {
  a: i32,
}
@binding(0) @group(0) var<uniform> foo_ub: foo_type;
fn ub_func() { _ = foo_ub.a; }

@fragment
fn ep_func() {
  ub_func();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].overrides.size());
}

TEST_F(InspectorGetEntryPointTest, BuiltinNotReferenced) {
    auto* src = R"(
@fragment
fn ep_func() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_FALSE(result[0].input_sample_mask_used);
    EXPECT_FALSE(result[0].output_sample_mask_used);
    EXPECT_FALSE(result[0].input_position_used);
    EXPECT_FALSE(result[0].front_facing_used);
    EXPECT_FALSE(result[0].sample_index_used);
    EXPECT_FALSE(result[0].num_workgroups_used);
    EXPECT_FALSE(result[0].frag_depth_used);
}

TEST_F(InspectorGetEntryPointTest, InputSampleMaskSimpleReferenced) {
    auto* src = R"(
@fragment
fn ep_func(@builtin(sample_mask) in_var: u32) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].input_sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, InputSampleMaskStructReferenced) {
    auto* src = R"(
struct in_struct {
  @builtin(sample_mask) inner_position: u32,
}

@fragment
fn ep_func(in_var: in_struct) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].input_sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, OutputSampleMaskSimpleReferenced) {
    auto* src = R"(
@fragment
fn ep_func(@builtin(sample_mask) in_var: u32) -> @builtin(sample_mask) u32 {
  return in_var;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].output_sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, OutputSampleMaskStructReferenced) {
    auto* src = R"(
struct out_struct {
  @builtin(sample_mask) inner_sample_mask: u32,
}

@fragment
fn ep_func() -> out_struct {
  var out_var: out_struct;
  return out_var;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].output_sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, InputPositionSimpleReferenced) {
    auto* src = R"(
@fragment
fn ep_func(@builtin(position) in_var: vec4f) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].input_position_used);
}

TEST_F(InspectorGetEntryPointTest, InputPositionStructReferenced) {
    auto* src = R"(
struct in_struct {
  @builtin(position) inner_position: vec4f,
}
@fragment
fn ep_func(in_var: in_struct) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].input_position_used);
}

TEST_F(InspectorGetEntryPointTest, FrontFacingSimpleReferenced) {
    auto* src = R"(
@fragment
fn ep_func(@builtin(front_facing) in_var: bool) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].front_facing_used);
}

TEST_F(InspectorGetEntryPointTest, FrontFacingStructReferenced) {
    auto* src = R"(
struct in_struct {
  @builtin(front_facing) inner_position: bool,
}
@fragment
fn ep_func(in_var: in_struct) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].front_facing_used);
}

TEST_F(InspectorGetEntryPointTest, SampleIndexSimpleReferenced) {
    auto* src = R"(
@fragment
fn ep_func(@builtin(sample_index) in_var: u32) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].sample_index_used);
}

TEST_F(InspectorGetEntryPointTest, SampleIndexStructReferenced) {
    auto* src = R"(
struct in_struct {
  @builtin(sample_index) inner_position: u32,
}

@fragment
fn ep_func(in_var: in_struct) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].sample_index_used);
}

TEST_F(InspectorGetEntryPointTest, NumWorkgroupsSimpleReferenced) {
    auto* src = R"(
@compute @workgroup_size(1i)
fn ep_func(@builtin(num_workgroups) in_var: vec3u) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].num_workgroups_used);
}

TEST_F(InspectorGetEntryPointTest, NumWorkgroupsStructReferenced) {
    auto* src = R"(
struct in_struct {
  @builtin(num_workgroups) inner_position: vec3u,
}

@compute @workgroup_size(1i)
fn ep_func(in_var: in_struct) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].num_workgroups_used);
}

TEST_F(InspectorGetEntryPointTest, FragDepthSimpleReferenced) {
    auto* src = R"(
@fragment
fn ep_func() -> @builtin(frag_depth) f32 {
  return 0f;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].frag_depth_used);
}

TEST_F(InspectorGetEntryPointTest, FragDepthStructReferenced) {
    auto* src = R"(
struct out_struct {
  @builtin(frag_depth) inner_frag_depth: f32,
}
@fragment
fn ep_func() -> out_struct {
  var out_var: out_struct;
  return out_var;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].frag_depth_used);
}

TEST_F(InspectorGetEntryPointTest, ClipDistancesReferenced) {
    auto* src = R"(
enable clip_distances;

struct out_struct {
  @builtin(clip_distances) inner_clip_distances: array<f32, 8>,
  @builtin(position) inner_position: vec4f,
}

@vertex
fn ep_func() -> out_struct {
  var out_var: out_struct;
  return out_var;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].clip_distances_size.has_value());
    EXPECT_EQ(8u, *result[0].clip_distances_size);
}

TEST_F(InspectorGetEntryPointTest, ClipDistancesNotReferenced) {
    auto* src = R"(
struct out_struct {
  @builtin(position) inner_position: vec4f,
}
@vertex
fn ep_func() -> out_struct {
  var out_var : out_struct;
  return out_var;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_FALSE(result[0].clip_distances_size.has_value());
}

TEST_F(InspectorGetEntryPointTest, ImplicitInterpolate) {
    auto* src = R"(
struct in_struct {
  @location(0) struct_inner: f32,
}
@fragment
fn ep_func(in_var: in_struct) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ(InterpolationType::kPerspective, result[0].input_variables[0].interpolation_type);
    EXPECT_EQ(InterpolationSampling::kCenter, result[0].input_variables[0].interpolation_sampling);
}

TEST_F(InspectorGetEntryPointTest, PixelLocalMemberDefault) {
    auto* src = R"(
@fragment
fn foo() {}
)";
    Inspector& inspector = Initialize(src);
    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].pixel_local_members.size());
}

TEST_F(InspectorGetEntryPointTest, PixelLocalMemberTypes) {
    auto* src = R"(
enable chromium_experimental_pixel_local;

struct Ure {
  toto: u32,
  titi: f32,
  tata: i32,
  tonton: u32, // Check having the same type multiple times
}

var<pixel_local> pls: Ure;
@fragment fn foo() {  _ = pls; }
)";
    Inspector& inspector = Initialize(src);
    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(4u, result[0].pixel_local_members.size());
    ASSERT_EQ(PixelLocalMemberType::kU32, result[0].pixel_local_members[0]);
    ASSERT_EQ(PixelLocalMemberType::kF32, result[0].pixel_local_members[1]);
    ASSERT_EQ(PixelLocalMemberType::kI32, result[0].pixel_local_members[2]);
    ASSERT_EQ(PixelLocalMemberType::kU32, result[0].pixel_local_members[3]);
}

struct InspectorGetEntryPointInterpolateTestParams {
    std::string in;
    inspector::InterpolationType out_type;
    inspector::InterpolationSampling out_sampling;
};
using InspectorGetEntryPointInterpolateTest =
    InspectorTestWithParam<InspectorGetEntryPointInterpolateTestParams>;

TEST_P(InspectorGetEntryPointInterpolateTest, Test) {
    auto& params = GetParam();
    auto src = R"(
struct in_struct {
  )" + params.in +
               R"( @location(0) struct_inner: f32,
}
@fragment fn ep_func(in_var: in_struct) {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ(params.out_type, result[0].input_variables[0].interpolation_type);
    EXPECT_EQ(params.out_sampling, result[0].input_variables[0].interpolation_sampling);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetEntryPointTest,
    InspectorGetEntryPointInterpolateTest,
    testing::Values(
        InspectorGetEntryPointInterpolateTestParams{"@interpolate(perspective, center)",
                                                    InterpolationType::kPerspective,
                                                    InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{"@interpolate(perspective, centroid)",
                                                    InterpolationType::kPerspective,
                                                    InterpolationSampling::kCentroid},
        InspectorGetEntryPointInterpolateTestParams{"@interpolate(perspective, sample)",
                                                    InterpolationType::kPerspective,
                                                    InterpolationSampling::kSample},
        InspectorGetEntryPointInterpolateTestParams{"@interpolate(perspective)",
                                                    InterpolationType::kPerspective,
                                                    InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{"@interpolate(linear, center)",
                                                    InterpolationType::kLinear,
                                                    InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{"@interpolate(linear, centroid)",
                                                    InterpolationType::kLinear,
                                                    InterpolationSampling::kCentroid},
        InspectorGetEntryPointInterpolateTestParams{"@interpolate(linear, sample)",
                                                    InterpolationType::kLinear,
                                                    InterpolationSampling::kSample},
        InspectorGetEntryPointInterpolateTestParams{
            "@interpolate(linear)", InterpolationType::kLinear, InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{"@interpolate(flat)", InterpolationType::kFlat,
                                                    InterpolationSampling::kFirst},
        InspectorGetEntryPointInterpolateTestParams{
            "@interpolate(flat, first)", InterpolationType::kFlat, InterpolationSampling::kFirst},
        InspectorGetEntryPointInterpolateTestParams{"@interpolate(flat, either)",
                                                    InterpolationType::kFlat,
                                                    InterpolationSampling::kEither}));

TEST_F(InspectorOverridesTest, NoOverrides) {
    auto* src = R"(
@compute @workgroup_size(1i)
fn ep_func() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.Overrides();
    EXPECT_TRUE(result.empty());
}

TEST_F(InspectorOverridesTest, Multiple) {
    auto* src = R"(
@id(1) override foo: f32;
@id(2) override bar: f32;

fn callee_func() { _ = foo; }
@compute @workgroup_size(1i) fn ep_func() {
  callee_func();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.Overrides();
    ASSERT_EQ(2u, result.size());

    auto& ep = result[0];
    EXPECT_EQ(ep.name, "foo");
    EXPECT_EQ(ep.id.value, 1);
    EXPECT_FALSE(ep.is_initialized);
    EXPECT_TRUE(ep.is_id_specified);

    ep = result[1];
    EXPECT_EQ(ep.name, "bar");
    EXPECT_EQ(ep.id.value, 2);
    EXPECT_FALSE(ep.is_initialized);
    EXPECT_TRUE(ep.is_id_specified);
}

TEST_F(InspectorGetEntryPointTest, HasTextureLoadWithDepthTexture) {
    std::string shader = R"(
        @group(0) @binding(0) var td : texture_depth_2d;
        @group(0) @binding(1) var tdm : texture_depth_multisampled_2d;
        @group(0) @binding(2) var t : texture_2d<f32>;
        @group(0) @binding(3) var s : sampler;

        @compute @workgroup_size(1) fn load_texture_depth() {
            _ = textureLoad(td, vec2(0), 0);
        }
        @compute @workgroup_size(1) fn load_texture_depth_multisample() {
            _ = textureLoad(td, vec2(0), 0);
        }
        @compute @workgroup_size(1) fn load_texture_2d() {
            _ = textureLoad(t, vec2(0), 0);
        }
        @fragment fn sample_texture_depth() -> @location(0) u32 {
            _ = textureSample(td, s, vec2(0));
            return 0;
        }
        fn load_texture_depth_arg(tex : texture_depth_2d) {
            _ = textureLoad(tex, vec2(0), 0);
        }
        @compute @workgroup_size(1) fn load_texture_depth_in_function() {
            load_texture_depth_arg(td);
        }
    )";
    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_TRUE(inspector.GetEntryPoint("load_texture_depth").has_texture_load_with_depth_texture);
    EXPECT_TRUE(inspector.GetEntryPoint("load_texture_depth_multisample")
                    .has_texture_load_with_depth_texture);
    EXPECT_FALSE(inspector.GetEntryPoint("load_texture_2d").has_texture_load_with_depth_texture);
    EXPECT_FALSE(
        inspector.GetEntryPoint("sample_texture_depth").has_texture_load_with_depth_texture);
    EXPECT_TRUE(inspector.GetEntryPoint("load_texture_depth_in_function")
                    .has_texture_load_with_depth_texture);
}

TEST_F(InspectorGetEntryPointTest, HasDepthTextureWithNonComparisonSampler) {
    std::string shader = R"(
        @group(0) @binding(0) var td : texture_depth_2d;
        @group(0) @binding(1) var s : sampler;
        @group(0) @binding(2) var cs : sampler_comparison;

        @fragment fn sample_texture_depth() -> @location(0) u32 {
            _ = textureSample(td, s, vec2(0));
            return 0;
        }
        @fragment fn comparison_sample_texture_depth() -> @location(0) u32 {
            _ = textureSampleCompare(td, cs, vec2(0), 0.5);
            return 0;
        }
        @fragment fn gather_texture_depth() -> @location(0) u32 {
            _ = textureGather(td, s, vec2(0));
            return 0;
        }
        @fragment fn comparison_gather_texture_depth() -> @location(0) u32 {
            _ = textureGatherCompare(td, cs, vec2(0), 0.5);
            return 0;
        }
        @fragment fn sample_level_texture_depth() -> @location(0) u32 {
            _ = textureSampleLevel(td, s, vec2(0), 0);
            return 0;
        }
        @fragment fn comparison_sample_level_texture_depth() -> @location(0) u32 {
            _ = textureSampleCompareLevel(td, cs, vec2(0), 0.5);
            return 0;
        }

        fn sample_texture_depth_arg(tex : texture_depth_2d) {
            _ = textureSample(tex, s, vec2(0));
        }
        @fragment fn sample_texture_depth_in_function() -> @location(0) u32 {
            sample_texture_depth_arg(td);
            return 0;
        }
    )";
    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_TRUE(inspector.GetEntryPoint("sample_texture_depth")
                    .has_depth_texture_with_non_comparison_sampler);
    EXPECT_FALSE(inspector.GetEntryPoint("comparison_sample_texture_depth")
                     .has_depth_texture_with_non_comparison_sampler);
    EXPECT_TRUE(inspector.GetEntryPoint("gather_texture_depth")
                    .has_depth_texture_with_non_comparison_sampler);
    EXPECT_FALSE(inspector.GetEntryPoint("comparison_gather_texture_depth")
                     .has_depth_texture_with_non_comparison_sampler);
    EXPECT_TRUE(inspector.GetEntryPoint("sample_level_texture_depth")
                    .has_depth_texture_with_non_comparison_sampler);
    EXPECT_FALSE(inspector.GetEntryPoint("comparison_sample_level_texture_depth")
                     .has_depth_texture_with_non_comparison_sampler);
    EXPECT_TRUE(inspector.GetEntryPoint("sample_texture_depth_in_function")
                    .has_depth_texture_with_non_comparison_sampler);
}

TEST_F(InspectorGetOverrideDefaultValuesTest, Bool) {
    auto* src = R"(
const C = true;
@id(1) override a: bool;
@id(20) override b: bool = true;
@id(300) override c = false;
@id(400) override d = true || false;
@id(500) override e = C;
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetOverrideDefaultValues();
    ASSERT_EQ(5u, result.size());

    ASSERT_TRUE(result.find(OverrideId{1}) != result.end());
    EXPECT_TRUE(result[OverrideId{1}].IsNull());

    ASSERT_TRUE(result.find(OverrideId{20}) != result.end());
    EXPECT_TRUE(result[OverrideId{20}].IsBool());
    EXPECT_TRUE(result[OverrideId{20}].AsBool());

    ASSERT_TRUE(result.find(OverrideId{300}) != result.end());
    EXPECT_TRUE(result[OverrideId{300}].IsBool());
    EXPECT_FALSE(result[OverrideId{300}].AsBool());

    ASSERT_TRUE(result.find(OverrideId{400}) != result.end());
    EXPECT_TRUE(result[OverrideId{400}].IsBool());
    EXPECT_TRUE(result[OverrideId{400}].AsBool());

    ASSERT_TRUE(result.find(OverrideId{500}) != result.end());
    EXPECT_TRUE(result[OverrideId{500}].IsBool());
    EXPECT_TRUE(result[OverrideId{500}].AsBool());
}

TEST_F(InspectorGetOverrideDefaultValuesTest, U32) {
    auto* src = R"(
const C = 100u;
@id(1) override a: u32;
@id(20) override b: u32 = 42u;
@id(30) override c: u32 = 42;
@id(40) override d: u32 = 42 + 10;
@id(50) override e = 42 + 10u;
@id(60) override f = C;
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetOverrideDefaultValues();
    ASSERT_EQ(6u, result.size());

    ASSERT_TRUE(result.find(OverrideId{1}) != result.end());
    EXPECT_TRUE(result[OverrideId{1}].IsNull());

    ASSERT_TRUE(result.find(OverrideId{20}) != result.end());
    EXPECT_TRUE(result[OverrideId{20}].IsU32());
    EXPECT_EQ(42u, result[OverrideId{20}].AsU32());

    ASSERT_TRUE(result.find(OverrideId{30}) != result.end());
    EXPECT_TRUE(result[OverrideId{30}].IsU32());
    EXPECT_EQ(42u, result[OverrideId{30}].AsU32());

    ASSERT_TRUE(result.find(OverrideId{40}) != result.end());
    EXPECT_TRUE(result[OverrideId{40}].IsU32());
    EXPECT_EQ(52u, result[OverrideId{40}].AsU32());

    ASSERT_TRUE(result.find(OverrideId{50}) != result.end());
    EXPECT_TRUE(result[OverrideId{50}].IsU32());
    EXPECT_EQ(52u, result[OverrideId{50}].AsU32());

    ASSERT_TRUE(result.find(OverrideId{60}) != result.end());
    EXPECT_TRUE(result[OverrideId{60}].IsU32());
    EXPECT_EQ(100u, result[OverrideId{60}].AsU32());
}

TEST_F(InspectorGetOverrideDefaultValuesTest, I32) {
    auto* src = R"(
const C = 100;
@id(1) override a: i32;
@id(20) override b: i32 = -42i;
@id(300) override c: i32 = 42i;
@id(400) override d = 42;
@id(500) override e = 42 + 7;
@id(6000) override f = C;
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetOverrideDefaultValues();
    ASSERT_EQ(6u, result.size());

    ASSERT_TRUE(result.find(OverrideId{1}) != result.end());
    EXPECT_TRUE(result[OverrideId{1}].IsNull());

    ASSERT_TRUE(result.find(OverrideId{20}) != result.end());
    EXPECT_TRUE(result[OverrideId{20}].IsI32());
    EXPECT_EQ(-42, result[OverrideId{20}].AsI32());

    ASSERT_TRUE(result.find(OverrideId{300}) != result.end());
    EXPECT_TRUE(result[OverrideId{300}].IsI32());
    EXPECT_EQ(42, result[OverrideId{300}].AsI32());

    ASSERT_TRUE(result.find(OverrideId{400}) != result.end());
    EXPECT_TRUE(result[OverrideId{400}].IsI32());
    EXPECT_EQ(42, result[OverrideId{400}].AsI32());

    ASSERT_TRUE(result.find(OverrideId{500}) != result.end());
    EXPECT_TRUE(result[OverrideId{500}].IsI32());
    EXPECT_EQ(49, result[OverrideId{500}].AsI32());

    ASSERT_TRUE(result.find(OverrideId{6000}) != result.end());
    EXPECT_TRUE(result[OverrideId{6000}].IsI32());
    EXPECT_EQ(100, result[OverrideId{6000}].AsI32());
}

TEST_F(InspectorGetOverrideDefaultValuesTest, F32) {
    auto* src = R"(
@id(1) override a: f32;
@id(20) override b: f32 = 0f;
@id(300) override c: f32 = -10f;
@id(4000) override d = 15f;
@id(5000) override e = 42.0;
@id(6000) override f: f32 = 15f * 10;
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetOverrideDefaultValues();
    ASSERT_EQ(6u, result.size());

    ASSERT_TRUE(result.find(OverrideId{1}) != result.end());
    EXPECT_TRUE(result[OverrideId{1}].IsNull());

    ASSERT_TRUE(result.find(OverrideId{20}) != result.end());
    EXPECT_TRUE(result[OverrideId{20}].IsFloat());
    EXPECT_FLOAT_EQ(0.0f, result[OverrideId{20}].AsFloat());

    ASSERT_TRUE(result.find(OverrideId{300}) != result.end());
    EXPECT_TRUE(result[OverrideId{300}].IsFloat());
    EXPECT_FLOAT_EQ(-10.0f, result[OverrideId{300}].AsFloat());

    ASSERT_TRUE(result.find(OverrideId{4000}) != result.end());
    EXPECT_TRUE(result[OverrideId{4000}].IsFloat());
    EXPECT_FLOAT_EQ(15.0f, result[OverrideId{4000}].AsFloat());

    ASSERT_TRUE(result.find(OverrideId{5000}) != result.end());
    EXPECT_TRUE(result[OverrideId{5000}].IsFloat());
    EXPECT_FLOAT_EQ(42.0f, result[OverrideId{5000}].AsFloat());

    ASSERT_TRUE(result.find(OverrideId{6000}) != result.end());
    EXPECT_TRUE(result[OverrideId{6000}].IsFloat());
    EXPECT_FLOAT_EQ(150.0f, result[OverrideId{6000}].AsFloat());
}

TEST_F(InspectorGetOverrideDefaultValuesTest, F16) {
    auto* src = R"(
enable f16;

@id(1) override a: f16;
@id(20) override b: f16 = 0h;
@id(300) override c: f16 = -10h;
@id(4000) override d = 15h;
@id(5000) override e = 42.0h;
@id(6000) override f: f16 = 15h * 10;
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetOverrideDefaultValues();
    ASSERT_EQ(6u, result.size());

    ASSERT_TRUE(result.find(OverrideId{1}) != result.end());
    EXPECT_TRUE(result[OverrideId{1}].IsNull());

    ASSERT_TRUE(result.find(OverrideId{20}) != result.end());
    // Default value of f16 override is also stored as float scalar.
    EXPECT_TRUE(result[OverrideId{20}].IsFloat());
    EXPECT_FLOAT_EQ(0.0f, result[OverrideId{20}].AsFloat());

    ASSERT_TRUE(result.find(OverrideId{300}) != result.end());
    EXPECT_TRUE(result[OverrideId{300}].IsFloat());
    EXPECT_FLOAT_EQ(-10.0f, result[OverrideId{300}].AsFloat());

    ASSERT_TRUE(result.find(OverrideId{4000}) != result.end());
    EXPECT_TRUE(result[OverrideId{4000}].IsFloat());
    EXPECT_FLOAT_EQ(15.0f, result[OverrideId{4000}].AsFloat());

    ASSERT_TRUE(result.find(OverrideId{5000}) != result.end());
    EXPECT_TRUE(result[OverrideId{5000}].IsFloat());
    EXPECT_FLOAT_EQ(42.0f, result[OverrideId{5000}].AsFloat());

    ASSERT_TRUE(result.find(OverrideId{6000}) != result.end());
    EXPECT_TRUE(result[OverrideId{6000}].IsFloat());
    EXPECT_FLOAT_EQ(150.0f, result[OverrideId{6000}].AsFloat());
}

TEST_F(InspectorGetConstantNameToIdMapTest, WithAndWithoutIds) {
    auto* src = R"(
@id(1) override v1: f32;
@id(20) override v20: f32;
@id(300) override v300: f32;
override a: f32;
override b: f32;
override c: f32;
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetNamedOverrideIds();
    ASSERT_EQ(6u, result.size());

    ASSERT_TRUE(result.count("v1"));
    EXPECT_EQ(result["v1"].value, 1u);

    ASSERT_TRUE(result.count("v20"));
    EXPECT_EQ(result["v20"].value, 20u);

    ASSERT_TRUE(result.count("v300"));
    EXPECT_EQ(result["v300"].value, 300u);

    ASSERT_TRUE(result.count("a"));
    EXPECT_EQ(result["a"].value, 0);

    ASSERT_TRUE(result.count("b"));
    EXPECT_EQ(result["b"].value, 2);

    ASSERT_TRUE(result.count("c"));
    EXPECT_EQ(result["c"].value, 3);
}

TEST_F(InspectorGetResourceBindingsTest, Empty) {
    auto* src = R"(
@fragment fn ep_func() {}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetResourceBindingsTest, Simple) {
    auto* src = R"(
struct ub_type {
  a: i32,
}
@group(0) @binding(0) var<uniform> ub_var: ub_type;
fn ub_func() { _ = ub_var.a; }

struct sb_type {
  a: i32,
}
@group(1) @binding(0) var<storage, read_write> sb_var: sb_type;
fn sb_func() { _ = sb_var.a; }

struct rosb_type {
  a: i32,
}
@group(1) @binding(1) var<storage, read> rosb_var: rosb_type;
fn rosb_func() { _ = rosb_var.a; }

@group(2) @binding(0) var s_texture : texture_1d<f32>;
@group(3) @binding(0) var s_var: sampler;
var<private> s_coords: f32;
fn s_func() {
  let sampler_result = textureSample(s_texture, s_var, s_coords);
}

@group(3) @binding(1) var cs_texture : texture_depth_2d;
@group(3) @binding(2) var cs_var: sampler_comparison;
var<private> cs_coords: vec2f;
var<private> cs_depth: f32;
fn cs_func() {
  let sampler_result = textureSampleCompare(cs_texture, cs_var, cs_coords, cs_depth);
}

@group(3) @binding(3) var depth_ms_texture : texture_depth_multisampled_2d;
fn depth_ms_func() {
  _ = depth_ms_texture;
}

@group(4) @binding(0) var st_var: texture_storage_2d<r32uint, write>;
fn st_func() { let dim = textureDimensions(st_var); }

@fragment
fn ep_func() {
  ub_func();
  sb_func();
  rosb_func();
  s_func();
  cs_func();
  depth_ms_func();
  st_func();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(9u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[1].resource_type);
    EXPECT_EQ(1u, result[1].bind_group);
    EXPECT_EQ(0u, result[1].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[2].resource_type);
    EXPECT_EQ(1u, result[2].bind_group);
    EXPECT_EQ(1u, result[2].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture, result[3].resource_type);
    EXPECT_EQ(2u, result[3].bind_group);
    EXPECT_EQ(0u, result[3].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[4].resource_type);
    EXPECT_EQ(3u, result[4].bind_group);
    EXPECT_EQ(0u, result[4].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kDepthTexture, result[5].resource_type);
    EXPECT_EQ(3u, result[5].bind_group);
    EXPECT_EQ(1u, result[5].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler, result[6].resource_type);
    EXPECT_EQ(3u, result[6].bind_group);
    EXPECT_EQ(2u, result[6].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kDepthMultisampledTexture, result[7].resource_type);
    EXPECT_EQ(3u, result[7].bind_group);
    EXPECT_EQ(3u, result[7].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kWriteOnlyStorageTexture, result[8].resource_type);
    EXPECT_EQ(4u, result[8].bind_group);
    EXPECT_EQ(0u, result[8].binding);
}

TEST_F(InspectorGetResourceBindingsTest, InputAttachment) {
    auto* src = R"(
enable chromium_internal_input_attachments;

@group(0) @binding(1) @input_attachment_index(3) var input_tex1: input_attachment<f32>;
@group(4) @binding(3) @input_attachment_index(1) var input_tex2: input_attachment<i32>;

fn f1() -> vec4f { return inputAttachmentLoad(input_tex1); }
fn f2() -> vec4i { return inputAttachmentLoad(input_tex2); }

@fragment
fn main() {
  f1();
  f2();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("main");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(2u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kInputAttachment, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(1u, result[0].binding);
    EXPECT_EQ(3u, result[0].input_attachment_index);
    EXPECT_EQ(inspector::ResourceBinding::SampledKind::kFloat, result[0].sampled_kind);

    EXPECT_EQ(ResourceBinding::ResourceType::kInputAttachment, result[1].resource_type);
    EXPECT_EQ(4u, result[1].bind_group);
    EXPECT_EQ(3u, result[1].binding);
    EXPECT_EQ(1u, result[1].input_attachment_index);
    EXPECT_EQ(inspector::ResourceBinding::SampledKind::kSInt, result[1].sampled_kind);
}

TEST_F(InspectorGetResourceBindingsTest, MissingEntryPoint) {
    auto* src = R"()";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_TRUE(inspector.has_error());
    std::string error = inspector.error();
    EXPECT_TRUE(error.find("not found") != std::string::npos);
}

TEST_F(InspectorGetResourceBindingsTest, NonEntryPointFunc) {
    auto* src = R"(
struct foo_type {
  a: i32,
}
@group(0) @binding(0) var<uniform> foo_ub: foo_type;
fn ub_func() { _ = foo_ub.a; }

@fragment fn ep_func() { ub_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ub_func");
    std::string error = inspector.error();
    EXPECT_TRUE(error.find("not an entry point") != std::string::npos);
}

TEST_F(InspectorGetResourceBindingsTest, UniformBuffer_Simple_NonStruct) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> foo_ub: i32;
fn ub_func() { _ = foo_ub; }
@fragment fn ep_func() { ub_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, UniformBuffer_Simple_Struct) {
    auto* src = R"(
struct foo_type {
  a: i32,
}
@group(0) @binding(0) var<uniform> foo_ub: foo_type;
fn ub_func() { _ = foo_ub.a; }
@fragment fn ep_func() { ub_func(); }
)";
    Inspector& inspector = Initialize(src);
    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, UniformBuffer_MultipleMembers) {
    auto* src = R"(
struct foo_type {
  a: i32,
  b: u32,
  c: f32,
}
@group(0) @binding(0) var<uniform> foo_ub: foo_type;
fn ub_func() {
  _ = foo_ub.a;
  _ = foo_ub.b;
  _ = foo_ub.c;
}
@fragment fn ep_func() { ub_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, UniformBuffer_ContainingPadding) {
    auto* src = R"(
struct foo_type {
  a: vec3f,
}
@group(0) @binding(0) var<uniform> foo_ub: foo_type;
fn ub_func() { _ = foo_ub.a; }
@fragment fn ep_func() { ub_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(16u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, UniformBuffer_NonStructVec3) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> foo_ub: vec3f;
fn ub_func() { _ = foo_ub; }
@fragment fn ep_func() { ub_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, UniformBuffer_Multiple) {
    auto* src = R"(
struct ub_type {
  a: i32,
  b: u32,
  c: f32,
}
@group(0) @binding(0) var<uniform> ub_foo: ub_type;
@group(0) @binding(1) var<uniform> ub_bar: ub_type;
@group(2) @binding(0) var<uniform> ub_baz: ub_type;
fn ub_foo_func() { _ = ub_foo.a; _ = ub_foo.b; _ = ub_foo.c; }
fn ub_bar_func() { _ = ub_bar.a; _ = ub_bar.b; _ = ub_bar.c; }
fn ub_baz_func() { _ = ub_baz.a; _ = ub_baz.b; _ = ub_baz.c; }

@fragment fn ep_func() {
  ub_foo_func();
  ub_bar_func();
  ub_baz_func();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(3u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[1].resource_type);
    EXPECT_EQ(0u, result[1].bind_group);
    EXPECT_EQ(1u, result[1].binding);
    EXPECT_EQ(12u, result[1].size);
    EXPECT_EQ(12u, result[1].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[2].resource_type);
    EXPECT_EQ(2u, result[2].bind_group);
    EXPECT_EQ(0u, result[2].binding);
    EXPECT_EQ(12u, result[2].size);
    EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, UniformBuffer_ContainingArray) {
    // Manually create uniform buffer to make sure it had a valid layout (array
    // with elem stride of 16, and that is 16-byte aligned within the struct)
    auto* src = R"(
struct foo_type {
  a: i32,
  @align(16) b: array<vec4i, 4>,
}
@group(0) @binding(0) var<uniform> foo_ub: foo_type;
fn ub_func() { _ = foo_ub.a; }
@fragment fn ep_func() { ub_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(80u, result[0].size);
    EXPECT_EQ(80u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_Simple_NonStruct) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> foo_sb: i32;
fn sb_func() { _ = foo_sb; }
@fragment fn ep_func() { sb_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_Simple_Struct) {
    auto* src = R"(
struct foo_type {
  a: i32,
}
@group(0) @binding(0) var<storage, read_write> foo_sb: foo_type;
fn sb_func() { _ = foo_sb.a; }
@fragment fn ep_func() { sb_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_MultipleMembers) {
    auto* src = R"(
struct foo_type {
  a: i32,
  b: u32,
  c: f32
}
@group(0) @binding(0) var<storage, read_write> foo_sb: foo_type;
fn sb_func() { _ = foo_sb.a; _ = foo_sb.b; _ = foo_sb.c; }
@fragment fn ep_func() { sb_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_Multiple) {
    auto* src = R"(
struct sb_type {
  a: i32,
  b: u32,
  c: f32,
}
@group(0) @binding(0) var<storage, read_write> sb_foo: sb_type;
@group(0) @binding(1) var<storage, read_write> sb_bar: sb_type;
@group(2) @binding(0) var<storage, read_write> sb_baz: sb_type;
fn sb_foo_func() { _ = sb_foo.a; _ = sb_foo.b; _ = sb_foo.c; }
fn sb_bar_func() { _ = sb_bar.a; _ = sb_bar.b; _ = sb_bar.c; }
fn sb_baz_func() { _ = sb_baz.a; _ = sb_baz.b; _ = sb_baz.c; }

@fragment
fn ep_func() {
  sb_foo_func();
  sb_bar_func();
  sb_baz_func();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(3u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[1].resource_type);
    EXPECT_EQ(0u, result[1].bind_group);
    EXPECT_EQ(1u, result[1].binding);
    EXPECT_EQ(12u, result[1].size);
    EXPECT_EQ(12u, result[1].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[2].resource_type);
    EXPECT_EQ(2u, result[2].bind_group);
    EXPECT_EQ(0u, result[2].binding);
    EXPECT_EQ(12u, result[2].size);
    EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_ContainingArray) {
    auto* src = R"(
struct foo_type {
  a: i32,
  b: array<u32, 4>,
}
@group(0) @binding(0) var<storage, read_write> foo_sb: foo_type;
fn sb_func() { _ = foo_sb.a; }
@fragment fn ep_func() { sb_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(20u, result[0].size);
    EXPECT_EQ(20u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_ContainingRuntimeArray) {
    auto* src = R"(
struct foo_type {
  a: i32,
  b: array<u32>,
}
@group(0) @binding(0) var<storage, read_write> foo_sb: foo_type;
fn sb_func() { _ = foo_sb.a; }
@fragment fn ep_func() { sb_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(8u, result[0].size);
    EXPECT_EQ(8u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_ContainingPadding) {
    auto* src = R"(
struct foo_type {
  a: vec3f,
}
@group(0) @binding(0) var<storage, read_write> foo_sb: foo_type;
fn sb_func() { _ = foo_sb.a; }
@fragment fn ep_func() { sb_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(16u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_NonStructVec3) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> foo_ub: vec3f;
fn ub_func() { _ = foo_ub; }
@fragment fn ep_func() { ub_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_ReadOnlySimple) {
    auto* src = R"(
struct foo_type {
  a: i32,
}
@group(0) @binding(0) var<storage, read> foo_sb: foo_type;
fn sb_func() { _ = foo_sb.a; }
@fragment fn ep_func() { sb_func(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, StorageBuffer_MultipleROAndRW) {
    auto* src = R"(
struct sb_type {
  a: i32,
  b: u32,
  c: f32,
}
@group(0) @binding(0) var<storage, read> sb_foo: sb_type;
@group(0) @binding(1) var<storage, read_write> sb_bar: sb_type;
@group(2) @binding(0) var<storage, read> sb_baz: sb_type;
fn sb_foo_func() { _ = sb_foo.a; _ = sb_foo.b; _ = sb_foo.c; }
fn sb_bar_func() { _ = sb_bar.a; _ = sb_bar.b; _ = sb_bar.c; }
fn sb_baz_func() { _ = sb_baz.a; _ = sb_baz.b; _ = sb_baz.c; }
@fragment fn ep_func() {
  sb_foo_func();
  sb_bar_func();
  sb_baz_func();
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(3u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[1].resource_type);
    EXPECT_EQ(0u, result[1].bind_group);
    EXPECT_EQ(1u, result[1].binding);
    EXPECT_EQ(12u, result[1].size);
    EXPECT_EQ(12u, result[1].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[2].resource_type);
    EXPECT_EQ(2u, result[2].bind_group);
    EXPECT_EQ(0u, result[2].binding);
    EXPECT_EQ(12u, result[2].size);
    EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetResourceBindingsTest, Sampler_Simple) {
    auto* src = R"(
@group(0) @binding(0) var foo_sampler: sampler;
@group(0) @binding(1) var foo_texture: texture_1d<f32>;
var<private> foo_coords: f32;
@fragment fn ep() {
  _ = textureSample(foo_texture, foo_sampler, foo_coords);
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(2u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[1].resource_type);
    EXPECT_EQ(0u, result[1].bind_group);
    EXPECT_EQ(0u, result[1].binding);
}

TEST_F(InspectorGetResourceBindingsTest, Sampler_InFunction) {
    auto* src = R"(
@group(0) @binding(0) var foo_sampler: sampler;
@group(0) @binding(1) var foo_texture: texture_1d<f32>;
var<private> foo_coords: f32;
@fragment fn ep_func() {
  _ = textureSample(foo_texture, foo_sampler, foo_coords);
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(2u, result.size());
    EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[1].resource_type);
    EXPECT_EQ(0u, result[1].bind_group);
    EXPECT_EQ(0u, result[1].binding);
}

TEST_F(InspectorGetResourceBindingsTest, Sampler_Comparison) {
    auto* src = R"(
@group(0) @binding(0) var foo_sampler: sampler_comparison;
@group(0) @binding(1) var foo_texture: texture_depth_2d;
var<private> foo_coords: vec2f;
var<private> foo_depth: f32;
@fragment fn ep() {
  _ = textureSampleCompare(foo_texture, foo_sampler, foo_coords, foo_depth);
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(2u, result.size());
    EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler, result[1].resource_type);
    EXPECT_EQ(0u, result[1].bind_group);
    EXPECT_EQ(0u, result[1].binding);
}

std::string CoordsType(core::type::TextureDimension dim, std::string_view name) {
    switch (dim) {
        case core::type::TextureDimension::k1d:
            return std::string(name);
        case core::type::TextureDimension::k2d:
        case core::type::TextureDimension::k2dArray:
            return "vec2<" + std::string(name) + ">";
        case core::type::TextureDimension::k3d:
        case core::type::TextureDimension::kCube:
        case core::type::TextureDimension::kCubeArray:
            return "vec3<" + std::string(name) + ">";
        default:
            break;
    }
    TINT_UNREACHABLE();
}

struct SampledTextureTestParams {
    core::type::TextureDimension type_dim;
    inspector::ResourceBinding::TextureDimension inspector_dim;
    inspector::ResourceBinding::SampledKind sampled_kind;
};
using InspectorGetResourceBindingsTest_WithSampledTextureParams =
    InspectorTestWithParam<SampledTextureTestParams>;
TEST_P(InspectorGetResourceBindingsTest_WithSampledTextureParams, TextureSample) {
    auto& params = GetParam();

    auto src = R"(
@group(0) @binding(0) var foo_texture: texture_)" +
               std::string(ToString(params.type_dim)) +
               R"(<f32>;
@group(0) @binding(1) var foo_sampler: sampler;
var<private> foo_coords: )" +
               CoordsType(params.type_dim, "f32") + R"(;
@fragment fn ep() {
  _ = textureSample(foo_texture, foo_sampler, foo_coords);
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(2u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(params.inspector_dim, result[0].dim);
    EXPECT_EQ(params.sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetResourceBindingsTest,
    InspectorGetResourceBindingsTest_WithSampledTextureParams,
    testing::Values(SampledTextureTestParams{core::type::TextureDimension::k1d,
                                             inspector::ResourceBinding::TextureDimension::k1d,
                                             inspector::ResourceBinding::SampledKind::kFloat},
                    SampledTextureTestParams{core::type::TextureDimension::k2d,
                                             inspector::ResourceBinding::TextureDimension::k2d,
                                             inspector::ResourceBinding::SampledKind::kFloat},
                    SampledTextureTestParams{core::type::TextureDimension::k3d,
                                             inspector::ResourceBinding::TextureDimension::k3d,
                                             inspector::ResourceBinding::SampledKind::kFloat},
                    SampledTextureTestParams{core::type::TextureDimension::kCube,
                                             inspector::ResourceBinding::TextureDimension::kCube,
                                             inspector::ResourceBinding::SampledKind::kFloat}));

using ArraySampledTextureTestParams = SampledTextureTestParams;
using InspectorGetResourceBindingsTest_WithArraySampledTextureParams =
    InspectorTestWithParam<ArraySampledTextureTestParams>;
TEST_P(InspectorGetResourceBindingsTest_WithArraySampledTextureParams, TextureSample) {
    auto& params = GetParam();
    auto src = R"(
@group(0) @binding(0) var foo_texture: texture_)" +
               std::string(ToString(params.type_dim)) +
               R"(<f32>;
@group(0) @binding(1) var foo_sampler: sampler;
var<private> foo_coords: )" +
               CoordsType(params.type_dim, "f32") + R"(;
var<private> foo_array_index: i32;
@fragment fn ep() {
  _ = textureSample(foo_texture, foo_sampler, foo_coords, foo_array_index);
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(2u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
    EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

using MultisampledTextureTestParams = SampledTextureTestParams;
using InspectorGetResourceBindingsTest_WithMultisampledTextureParams =
    InspectorTestWithParam<MultisampledTextureTestParams>;
INSTANTIATE_TEST_SUITE_P(
    InspectorGetResourceBindingsTest,
    InspectorGetResourceBindingsTest_WithArraySampledTextureParams,
    testing::Values(
        ArraySampledTextureTestParams{core::type::TextureDimension::k2dArray,
                                      inspector::ResourceBinding::TextureDimension::k2dArray,
                                      inspector::ResourceBinding::SampledKind::kFloat},
        ArraySampledTextureTestParams{core::type::TextureDimension::kCubeArray,
                                      inspector::ResourceBinding::TextureDimension::kCubeArray,
                                      inspector::ResourceBinding::SampledKind::kFloat}));

std::string BaseType(ResourceBinding::SampledKind sampled_kind) {
    switch (sampled_kind) {
        case ResourceBinding::SampledKind::kFloat:
            return "f32";
        case ResourceBinding::SampledKind::kSInt:
            return "i32";
        case ResourceBinding::SampledKind::kUInt:
            return "u32";
        default:
            TINT_UNREACHABLE();
    }
}

TEST_P(InspectorGetResourceBindingsTest_WithMultisampledTextureParams, TextureLoad) {
    auto& params = GetParam();
    auto src = R"(
@group(0) @binding(0) var foo_texture: texture_multisampled_)" +
               std::string(ToString(params.type_dim)) + "<" + BaseType(params.sampled_kind) + R"(>;
var<private> foo_coords: )" +
               CoordsType(params.type_dim, "i32") + R"(;
var<private> foo_sample_index: i32;
@fragment fn ep() {
  _ = textureLoad(foo_texture, foo_coords, foo_sample_index);
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(ResourceBinding::ResourceType::kMultisampledTexture, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
    EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetResourceBindingsTest,
    InspectorGetResourceBindingsTest_WithMultisampledTextureParams,
    testing::Values(MultisampledTextureTestParams{core::type::TextureDimension::k2d,
                                                  inspector::ResourceBinding::TextureDimension::k2d,
                                                  inspector::ResourceBinding::SampledKind::kFloat},
                    MultisampledTextureTestParams{core::type::TextureDimension::k2d,
                                                  inspector::ResourceBinding::TextureDimension::k2d,
                                                  inspector::ResourceBinding::SampledKind::kSInt},
                    MultisampledTextureTestParams{core::type::TextureDimension::k2d,
                                                  inspector::ResourceBinding::TextureDimension::k2d,
                                                  inspector::ResourceBinding::SampledKind::kUInt}));

using DimensionParams = std::tuple<core::type::TextureDimension, ResourceBinding::TextureDimension>;
using TexelFormatParams =
    std::tuple<core::TexelFormat, ResourceBinding::TexelFormat, ResourceBinding::SampledKind>;
using StorageTextureTestParams = std::tuple<DimensionParams, TexelFormatParams, core::Access>;
using InspectorGetResourceBindingsTest_WithStorageTextureParams =
    InspectorTestWithParam<StorageTextureTestParams>;
TEST_P(InspectorGetResourceBindingsTest_WithStorageTextureParams, Simple) {
    DimensionParams dim_params;
    TexelFormatParams format_params;
    core::Access access;
    std::tie(dim_params, format_params, access) = GetParam();

    core::type::TextureDimension dim;
    ResourceBinding::TextureDimension expected_dim;
    std::tie(dim, expected_dim) = dim_params;

    core::TexelFormat format;
    ResourceBinding::TexelFormat expected_format;
    ResourceBinding::SampledKind expected_kind;
    std::tie(format, expected_format, expected_kind) = format_params;

    ResourceBinding::ResourceType expectedResourceType;
    switch (access) {
        case core::Access::kWrite:
            expectedResourceType = ResourceBinding::ResourceType::kWriteOnlyStorageTexture;
            break;
        case core::Access::kRead:
            expectedResourceType = ResourceBinding::ResourceType::kReadOnlyStorageTexture;
            break;
        case core::Access::kReadWrite:
            expectedResourceType = ResourceBinding::ResourceType::kReadWriteStorageTexture;
            break;
        case core::Access::kUndefined:
            ASSERT_TRUE(false);
            break;
    }

    auto src = R"(
@group(0) @binding(0) var st_var: texture_storage_)" +
               std::string(ToString(dim)) + "<" + std::string(ToString(format)) + ", " +
               std::string(ToString(access)) + R"(>;
@fragment fn ep() { _ = textureDimensions(st_var); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(expectedResourceType, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(expected_dim, result[0].dim);
    EXPECT_EQ(expected_format, result[0].image_format);
    EXPECT_EQ(expected_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetResourceBindingsTest,
    InspectorGetResourceBindingsTest_WithStorageTextureParams,
    testing::Combine(
        testing::Values(std::make_tuple(core::type::TextureDimension::k1d,
                                        ResourceBinding::TextureDimension::k1d),
                        std::make_tuple(core::type::TextureDimension::k2d,
                                        ResourceBinding::TextureDimension::k2d),
                        std::make_tuple(core::type::TextureDimension::k2dArray,
                                        ResourceBinding::TextureDimension::k2dArray),
                        std::make_tuple(core::type::TextureDimension::k3d,
                                        ResourceBinding::TextureDimension::k3d)),
        testing::Values(std::make_tuple(core::TexelFormat::kR32Float,
                                        ResourceBinding::TexelFormat::kR32Float,
                                        ResourceBinding::SampledKind::kFloat),
                        std::make_tuple(core::TexelFormat::kR32Sint,
                                        ResourceBinding::TexelFormat::kR32Sint,
                                        ResourceBinding::SampledKind::kSInt),
                        std::make_tuple(core::TexelFormat::kR32Uint,
                                        ResourceBinding::TexelFormat::kR32Uint,
                                        ResourceBinding::SampledKind::kUInt),
                        std::make_tuple(core::TexelFormat::kRg32Float,
                                        ResourceBinding::TexelFormat::kRg32Float,
                                        ResourceBinding::SampledKind::kFloat),
                        std::make_tuple(core::TexelFormat::kRg32Sint,
                                        ResourceBinding::TexelFormat::kRg32Sint,
                                        ResourceBinding::SampledKind::kSInt),
                        std::make_tuple(core::TexelFormat::kRg32Uint,
                                        ResourceBinding::TexelFormat::kRg32Uint,
                                        ResourceBinding::SampledKind::kUInt),
                        std::make_tuple(core::TexelFormat::kRgba16Float,
                                        ResourceBinding::TexelFormat::kRgba16Float,
                                        ResourceBinding::SampledKind::kFloat),
                        std::make_tuple(core::TexelFormat::kRgba16Sint,
                                        ResourceBinding::TexelFormat::kRgba16Sint,
                                        ResourceBinding::SampledKind::kSInt),
                        std::make_tuple(core::TexelFormat::kRgba16Uint,
                                        ResourceBinding::TexelFormat::kRgba16Uint,
                                        ResourceBinding::SampledKind::kUInt),
                        std::make_tuple(core::TexelFormat::kRgba32Float,
                                        ResourceBinding::TexelFormat::kRgba32Float,
                                        ResourceBinding::SampledKind::kFloat),
                        std::make_tuple(core::TexelFormat::kRgba32Sint,
                                        ResourceBinding::TexelFormat::kRgba32Sint,
                                        ResourceBinding::SampledKind::kSInt),
                        std::make_tuple(core::TexelFormat::kRgba32Uint,
                                        ResourceBinding::TexelFormat::kRgba32Uint,
                                        ResourceBinding::SampledKind::kUInt),
                        std::make_tuple(core::TexelFormat::kRgba8Sint,
                                        ResourceBinding::TexelFormat::kRgba8Sint,
                                        ResourceBinding::SampledKind::kSInt),
                        std::make_tuple(core::TexelFormat::kRgba8Snorm,
                                        ResourceBinding::TexelFormat::kRgba8Snorm,
                                        ResourceBinding::SampledKind::kFloat),
                        std::make_tuple(core::TexelFormat::kRgba8Uint,
                                        ResourceBinding::TexelFormat::kRgba8Uint,
                                        ResourceBinding::SampledKind::kUInt),
                        std::make_tuple(core::TexelFormat::kRgba8Unorm,
                                        ResourceBinding::TexelFormat::kRgba8Unorm,
                                        ResourceBinding::SampledKind::kFloat)),
        testing::Values(core::Access::kRead, core::Access::kWrite, core::Access::kReadWrite)));

struct DepthTextureTestParams {
    core::type::TextureDimension type_dim;
    inspector::ResourceBinding::TextureDimension inspector_dim;
};
using InspectorGetResourceBindingsTest_WithDepthTextureParams =
    InspectorTestWithParam<DepthTextureTestParams>;
TEST_P(InspectorGetResourceBindingsTest_WithDepthTextureParams, TextureDimensions) {
    auto& params = GetParam();
    auto src = R"(
@group(0) @binding(0) var dt: texture_depth_)" +
               std::string(ToString(params.type_dim)) + R"(;
@fragment fn ep() {
  _ = textureDimensions(dt);
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(ResourceBinding::ResourceType::kDepthTexture, result[0].resource_type);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(params.inspector_dim, result[0].dim);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetResourceBindingsTest,
    InspectorGetResourceBindingsTest_WithDepthTextureParams,
    testing::Values(DepthTextureTestParams{core::type::TextureDimension::k2d,
                                           inspector::ResourceBinding::TextureDimension::k2d},
                    DepthTextureTestParams{core::type::TextureDimension::k2dArray,
                                           inspector::ResourceBinding::TextureDimension::k2dArray},
                    DepthTextureTestParams{core::type::TextureDimension::kCube,
                                           inspector::ResourceBinding::TextureDimension::kCube},
                    DepthTextureTestParams{
                        core::type::TextureDimension::kCubeArray,
                        inspector::ResourceBinding::TextureDimension::kCubeArray}));

TEST_F(InspectorGetResourceBindingsTest, DepthMultisampledTexture_TextureDimensions) {
    auto* src = R"(
@group(0) @binding(0) var tex: texture_depth_multisampled_2d;
@fragment fn ep() {
  _ = textureDimensions(tex);
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(ResourceBinding::ResourceType::kDepthMultisampledTexture, result[0].resource_type);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(ResourceBinding::TextureDimension::k2d, result[0].dim);
}

TEST_F(InspectorGetResourceBindingsTest, ExternalTexture) {
    auto* src = R"(
@group(0) @binding(0) var et: texture_external;
@fragment fn ep() { _ = textureDimensions(et); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    EXPECT_EQ(ResourceBinding::ResourceType::kExternalTexture, result[0].resource_type);

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
}

class InspectorGetSamplerTextureUsesTest : public TestHelper, public testing::Test {
  public:
    using ResultExpectation = std::initializer_list<SamplerTexturePair>;

    size_t SizeOf(const ResultExpectation& expectation) { return expectation.size(); }
    size_t SizeOf(const std::vector<SamplerTexturePair>& result) { return result.size(); }

    // ValidateEqual checks that the expected and actual SamplerTexturePair list contain same pairs
    // and both are deduplicated.
    template <typename T, typename U>
    void ValidateEqual(const T& expected, const U& actual) {
        ASSERT_EQ(SizeOf(expected), SizeOf(actual));
        std::unordered_set<SamplerTexturePair> pairSet;
        // Insert all pairs in the expected into the set.
        for (const auto& pair : expected) {
            // Expectation should be deduplicated, so every insertion should take place.
            EXPECT_TRUE(pairSet.insert(pair).second)
                << "Duplicated SamplerTexturePair found: Sampler: ("
                << pair.sampler_binding_point.group << ", " << pair.sampler_binding_point.binding
                << "), " << "Texture: (" << pair.texture_binding_point.group << ", "
                << pair.texture_binding_point.binding << ")";
        }
        // Check that each SamplerTexturePair in the actual is in the set and occurs only once.
        for (const auto& pair : actual) {
            EXPECT_TRUE(pairSet.erase(pair) == 1)
                << "Unexpected SamplerTexturePair: Sampler: (" << pair.sampler_binding_point.group
                << ", " << pair.sampler_binding_point.binding << "), " << "Texture: ("
                << pair.texture_binding_point.group << ", " << pair.texture_binding_point.binding
                << ")";
        }
    }

    constexpr static BindingPoint non_sampler_placeholder{123u, 654u};
};

TEST_F(InspectorGetSamplerTextureUsesTest, None) {
    std::string shader = R"(
@fragment
fn main() {
})";

    ResultExpectation expected = {};

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("main");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("main", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
}

// Regression test for crbug.com/dawn/380433758.
TEST_F(InspectorGetSamplerTextureUsesTest, DiamondSampler) {
    std::string shader = R"(
fn sample(t: texture_2d<f32>, s: sampler) -> vec4f {
  return textureSampleLevel(t, s, vec2f(0), 0);
}

fn useCombos0() -> vec4f {
  return sample(tex0_0, smp0_0);
}

fn useCombos1() -> vec4f {
  return sample(tex1_15, smp0_0);
}

@group(0) @binding(0) var tex0_0: texture_2d<f32>;
@group(0) @binding(1) var tex1_15: texture_2d<f32>;
@group(0) @binding(2) var smp0_0: sampler;

@vertex fn vs() -> @builtin(position) vec4f {
  return useCombos0();
}

@fragment fn fs() -> @location(0) vec4f {
  return vec4f(useCombos1());
})";

    ResultExpectation expected_vs = {
        {/* Sampler */ BindingPoint{0, 2}, /* Texture */ BindingPoint{0, 0}},
    };
    ResultExpectation expected_fs = {
        {/* Sampler */ BindingPoint{0, 2}, /* Texture */ BindingPoint{0, 1}},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("vs");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_vs, result);
    }
    {
        auto result = inspector.GetSamplerTextureUses("fs");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_fs, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("vs", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_vs, result);
    }
    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("fs", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_fs, result);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, DiamondSampler2) {
    std::string shader = R"(
fn sample(t: texture_2d<f32>, s: sampler) -> vec4f {
  return textureSampleLevel(t, s, vec2f(0), 0);
}

fn useCombos0() -> vec4f {
  return sample(tex0_0, smp0_0);
}

fn useCombos1(t: texture_2d<f32>) -> vec4f {
  return sample(t, smp0_0);
}

fn useCombos2() -> vec4f {
  return useCombos1(tex1_15);
}

@group(0) @binding(0) var tex0_0: texture_2d<f32>;
@group(0) @binding(1) var tex1_15: texture_2d<f32>;
@group(0) @binding(2) var smp0_0: sampler;

@vertex fn vs() -> @builtin(position) vec4f {
  return useCombos0();
}

@fragment fn fs() -> @location(0) vec4f {
  return vec4f(useCombos2());
})";

    ResultExpectation expected_vs = {
        {/* Sampler */ BindingPoint{0, 2}, /* Texture */ BindingPoint{0, 0}},
    };
    ResultExpectation expected_fs = {
        {/* Sampler */ BindingPoint{0, 2}, /* Texture */ BindingPoint{0, 1}},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("vs");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_vs, result);
    }
    {
        auto result = inspector.GetSamplerTextureUses("fs");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_fs, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("vs", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_vs, result);
    }
    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("fs", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_fs, result);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, DiamondSampler3) {
    std::string shader = R"(
fn sample(t: texture_2d<f32>, s: sampler) -> vec4f {
  return textureSampleLevel(t, s, vec2f(0), 0);
}

fn useCombos0() -> vec4f {
  return sample(tex0_0, smp0_0);
}

fn useCombos1(t: texture_2d<f32>) -> vec4f {
  return sample(t, smp0_0);
}

fn useCombos2() -> vec4f {
  return useCombos1(tex1_15);
}

@group(0) @binding(0) var tex0_0: texture_2d<f32>;
@group(0) @binding(1) var tex1_15: texture_2d<f32>;
@group(0) @binding(2) var smp0_0: sampler;

@vertex fn vs() -> @builtin(position) vec4f {
  _ = useCombos0();
  return useCombos2();
}

@fragment fn fs() -> @location(0) vec4f {
  return vec4f(useCombos2());
})";

    ResultExpectation expected_vs = {
        {/* Sampler */ BindingPoint{0, 2}, /* Texture */ BindingPoint{0, 0}},
        {/* Sampler */ BindingPoint{0, 2}, /* Texture */ BindingPoint{0, 1}},
    };
    ResultExpectation expected_fs = {
        {/* Sampler */ BindingPoint{0, 2}, /* Texture */ BindingPoint{0, 1}},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("vs");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_vs, result);
    }
    {
        auto result = inspector.GetSamplerTextureUses("fs");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_fs, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("vs", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_vs, result);
    }
    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("fs", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_fs, result);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, Simple) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

@fragment
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    ResultExpectation expected = {
        {/* Sampler */ BindingPoint{0, 1}, /* Texture */ BindingPoint{0, 2}},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("main");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("main", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, UnknownEntryPoint) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

@fragment
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    {
        Inspector& inspector = Initialize(shader);
        inspector.GetSamplerTextureUses("foo");
        ASSERT_TRUE(inspector.has_error()) << inspector.error();
    }
    {
        Inspector& inspector = Initialize(shader);
        inspector.GetSamplerAndNonSamplerTextureUses("foo", non_sampler_placeholder);
        ASSERT_TRUE(inspector.has_error()) << inspector.error();
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, MultipleCalls) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

@fragment
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);

    {
        auto result_0 = inspector.GetSamplerTextureUses("main");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        auto result_1 = inspector.GetSamplerTextureUses("main");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(result_0, result_1);
    }

    {
        auto result_0 =
            inspector.GetSamplerAndNonSamplerTextureUses("main", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();
        auto result_1 =
            inspector.GetSamplerAndNonSamplerTextureUses("main", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();
        ValidateEqual(result_0, result_1);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, BothIndirect) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, uv);
}

@fragment
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    ResultExpectation expected = {
        {/* Sampler */ BindingPoint{0, 1}, /* Texture */ BindingPoint{0, 2}},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("main");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("main", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, SamplerIndirect) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(myTexture, s, uv);
}

@fragment
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(mySampler, fragUV) * fragPosition;
})";

    ResultExpectation expected = {
        {/* Sampler */ BindingPoint{0, 1}, /* Texture */ BindingPoint{0, 2}},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("main");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("main", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, TextureIndirect) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(t: texture_2d<f32>, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(t, mySampler, uv);
}

@fragment
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(myTexture, fragUV) * fragPosition;
})";

    ResultExpectation expected = {
        {/* Sampler */ BindingPoint{0, 1}, /* Texture */ BindingPoint{0, 2}},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("main");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("main", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, NeitherIndirect) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(uv: vec2<f32>) -> vec4<f32> {
  return textureSample(myTexture, mySampler, uv);
}

@fragment
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(fragUV) * fragPosition;
})";

    ResultExpectation expected = {
        {/* Sampler */ BindingPoint{0, 1}, /* Texture */ BindingPoint{0, 2}},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("main");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("main", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, Complex) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;


fn doSample(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, uv);
}

fn X(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return doSample(t, s, uv);
}

fn Y(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return doSample(t, s, uv);
}

fn Z(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return X(t, s, uv) + Y(t, s, uv);
}

@fragment
fn via_call(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return Z(myTexture, mySampler, fragUV) * fragPosition;
}

@fragment
fn via_ptr(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) + fragPosition;
}

@fragment
fn direct(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) + fragPosition;
})";

    ResultExpectation expected = {
        {/* Sampler */ BindingPoint{0, 1}, /* Texture */ BindingPoint{0, 2}},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("via_call");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
    {
        auto result = inspector.GetSamplerTextureUses("via_ptr");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
    {
        auto result = inspector.GetSamplerTextureUses("direct");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }

    {
        auto result =
            inspector.GetSamplerAndNonSamplerTextureUses("via_call", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
    {
        auto result =
            inspector.GetSamplerAndNonSamplerTextureUses("via_ptr", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
    {
        auto result =
            inspector.GetSamplerAndNonSamplerTextureUses("direct", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected, result);
    }
}

TEST_F(InspectorGetSamplerTextureUsesTest, SamplerAndNonSamplerTexture) {
    std::string shader = R"(
@group(0) @binding(1) var sampler0: sampler;
@group(0) @binding(3) var sampler1: sampler;
@group(0) @binding(2) var texture0: texture_2d<f32>;
@group(2) @binding(1) var texture1: texture_2d<f32>;
// Storage texture should not be included in the result.
@group(2) @binding(3) var texture2: texture_storage_2d<r32float, read_write>;
@group(2) @binding(4) var external0 : texture_external;
@group(2) @binding(5) var external1 : texture_external;

const loadStoreCoords = vec2<u32>(0u, 0u);

fn doSample(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  _ = textureLoad(t, loadStoreCoords, 0u);
  return textureSample(t, s, uv);
}

@fragment
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  // Usage with a sampler
  _ = textureSample(texture1, sampler0, fragUV);

  // Non-sampler texture usage.
  _ = textureLoad(texture1, loadStoreCoords, 0u);

  // Both sampler and non-sampler usage but inside a function.
  _ = doSample(texture0, sampler0, fragUV);

  // Using texture0 with sampler0 again, should be deduplicated in the result.
  _ = textureSample(texture0, sampler0, fragUV);

  // Storage texture should not be included in the result.
  _ = textureLoad(texture2, loadStoreCoords);
  textureStore(texture2, loadStoreCoords, fragPosition);

  // Usages of texture_external with and without samplers
  _ = textureSampleBaseClampToEdge(external0, sampler0, fragUV);
  _ = textureLoad(external1, vec2(0, 0));

  // Another usage with a sampler.
  return textureSample(texture0, sampler1, fragUV) + fragPosition;
}
)";

    constexpr BindingPoint sampler_0 = {0, 1};
    constexpr BindingPoint sampler_1 = {0, 3};
    constexpr BindingPoint texture_0 = {0, 2};
    constexpr BindingPoint texture_1 = {2, 1};
    constexpr BindingPoint external_0 = {2, 4};
    constexpr BindingPoint external_1 = {2, 5};
    // Storage texture texture2 should not be included in the result.

    ResultExpectation expected_sampler_only = {
        {/* Sampler */ sampler_0, /* Texture */ texture_1},
        {/* Sampler */ sampler_0, /* Texture */ texture_0},
        {/* Sampler */ sampler_1, /* Texture */ texture_0},
        {/* Sampler */ sampler_0, /* Texture */ external_0},
    };
    ResultExpectation expected_sampler_and_non_sampler = {
        {/* Sampler */ sampler_0, /* Texture */ texture_1},
        {/* Sampler */ non_sampler_placeholder, /* Texture */ texture_1},
        {/* Sampler */ non_sampler_placeholder, /* Texture */ texture_0},
        {/* Sampler */ sampler_0, /* Texture */ texture_0},
        {/* Sampler */ sampler_1, /* Texture */ texture_0},
        {/* Sampler */ sampler_0, /* Texture */ external_0},
        {/* Sampler */ non_sampler_placeholder, /* Texture */ external_1},
    };

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("main");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_sampler_only, result);
    }

    {
        auto result = inspector.GetSamplerAndNonSamplerTextureUses("main", non_sampler_placeholder);
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ValidateEqual(expected_sampler_and_non_sampler, result);
    }
}

// Test calling GetUsedExtensionNames on a empty shader.
TEST_F(InspectorGetUsedExtensionNamesTest, Empty) {
    std::string shader = "";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetUsedExtensionNames();
    EXPECT_EQ(result.size(), 0u);
}

// Test calling GetUsedExtensionNames on a shader with no extension.
TEST_F(InspectorGetUsedExtensionNamesTest, None) {
    std::string shader = R"(
@fragment
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetUsedExtensionNames();
    EXPECT_EQ(result.size(), 0u);
}

// Test calling GetUsedExtensionNames on a shader with valid extension.
TEST_F(InspectorGetUsedExtensionNamesTest, Simple) {
    std::string shader = R"(
enable f16;

@fragment
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetUsedExtensionNames();
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], "f16");
}

// Test calling GetUsedExtensionNames on a shader with a extension enabled for
// multiple times.
TEST_F(InspectorGetUsedExtensionNamesTest, Duplicated) {
    std::string shader = R"(
enable f16;
enable f16;

@fragment
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetUsedExtensionNames();
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], "f16");
}

// Test calling GetEnableDirectives on a empty shader.
TEST_F(InspectorGetEnableDirectivesTest, Empty) {
    std::string shader = "";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetEnableDirectives();
    EXPECT_EQ(result.size(), 0u);
}

// Test calling GetEnableDirectives on a shader with no extension.
TEST_F(InspectorGetEnableDirectivesTest, None) {
    std::string shader = R"(
@fragment
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetEnableDirectives();
    EXPECT_EQ(result.size(), 0u);
}

// Test calling GetEnableDirectives on a shader with valid extension.
TEST_F(InspectorGetEnableDirectivesTest, Simple) {
    std::string shader = R"(
enable f16;

@fragment
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetEnableDirectives();
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].first, "f16");
    EXPECT_EQ(result[0].second.range, (Source::Range{{2, 8}, {2, 11}}));
}

// Test calling GetEnableDirectives on a shader with a extension enabled for
// multiple times.
TEST_F(InspectorGetEnableDirectivesTest, Duplicated) {
    std::string shader = R"(
enable f16;

enable f16;
@fragment
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetEnableDirectives();
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0].first, "f16");
    EXPECT_EQ(result[0].second.range, (Source::Range{{2, 8}, {2, 11}}));
    EXPECT_EQ(result[1].first, "f16");
    EXPECT_EQ(result[1].second.range, (Source::Range{{4, 8}, {4, 11}}));
}

// Crash was occuring in ::GenerateSamplerTargets, when
// ::GetSamplerTextureUses was called.
TEST_F(InspectorRegressionTest, tint967) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, uv);
}

@fragment
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);
    inspector.GetSamplerTextureUses("main");
}

TEST_F(InspectorTextureTest, TextureLevelInEP) {
    std::string shader = R"(
@group(2) @binding(3) var myTexture: texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  let num = textureNumLevels(myTexture);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(1u, info.size());
    EXPECT_EQ(Inspector::TextureQueryType::kTextureNumLevels, info[0].type);
    EXPECT_EQ(2u, info[0].group);
    EXPECT_EQ(3u, info[0].binding);
}

TEST_F(InspectorTextureTest, TextureLevelInEPNoDups) {
    std::string shader = R"(
@group(0) @binding(0) var myTexture: texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  let num1 = textureNumLevels(myTexture);
  let num2 = textureNumLevels(myTexture);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(1u, info.size());
}

TEST_F(InspectorTextureTest, TextureLevelInEPMultiple) {
    std::string shader = R"(
@group(2) @binding(3) var tex1: texture_2d<f32>;
@group(1) @binding(2) var tex2: texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  let num1 = textureNumLevels(tex1);
  let num2 = textureNumLevels(tex2);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(2u, info.size());

    Inspector::LevelSampleInfo info1 = {
        /*type */ Inspector::TextureQueryType::kTextureNumLevels,
        /*group*/ 1,
        /*binding*/ 2,
    };
    Inspector::LevelSampleInfo info2 = {
        /*type */ Inspector::TextureQueryType::kTextureNumLevels,
        /*group*/ 2,
        /*binding*/ 3,
    };
    EXPECT_THAT(info, testing::UnorderedElementsAre(info1, info2));
}

TEST_F(InspectorTextureTest, TextureSamplesInEP) {
    std::string shader = R"(
@group(2) @binding(3) var myTexture: texture_multisampled_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  let num = textureNumSamples(myTexture);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(1u, info.size());
    EXPECT_EQ(Inspector::TextureQueryType::kTextureNumSamples, info[0].type);
    EXPECT_EQ(2u, info[0].group);
    EXPECT_EQ(3u, info[0].binding);
}

TEST_F(InspectorTextureTest, TextureSamplesInEPNoDups) {
    std::string shader = R"(
@group(0) @binding(0) var myTexture: texture_multisampled_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  let num1 = textureNumSamples(myTexture);
  let num2 = textureNumSamples(myTexture);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(1u, info.size());
}

TEST_F(InspectorTextureTest, TextureSamplesInEPMultiple) {
    std::string shader = R"(
@group(2) @binding(3) var tex1: texture_multisampled_2d<f32>;
@group(1) @binding(2) var tex2: texture_multisampled_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  let num1 = textureNumSamples(tex1);
  let num2 = textureNumSamples(tex2);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(2u, info.size());

    Inspector::LevelSampleInfo info1 = {
        /*type */ Inspector::TextureQueryType::kTextureNumSamples,
        /*group*/ 1,
        /*binding*/ 2,
    };
    Inspector::LevelSampleInfo info2 = {
        /*type */ Inspector::TextureQueryType::kTextureNumSamples,
        /*group*/ 2,
        /*binding*/ 3,
    };
    EXPECT_THAT(info, testing::UnorderedElementsAre(info1, info2));
}

TEST_F(InspectorTextureTest, TextureLoadInEP) {
    std::string shader = R"(
@group(2) @binding(3) var tex1: texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  let num1 = textureLoad(tex1, vec2(0, 0), 0);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(1u, info.size());

    EXPECT_EQ(Inspector::TextureQueryType::kTextureNumLevels, info[0].type);
    EXPECT_EQ(2u, info[0].group);
    EXPECT_EQ(3u, info[0].binding);
}

TEST_F(InspectorTextureTest, TextureLoadMultisampledInEP) {
    std::string shader = R"(
@group(2) @binding(3) var tex1: texture_multisampled_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  let num1 = textureLoad(tex1, vec2(0, 0), 0);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(0u, info.size());
}

TEST_F(InspectorTextureTest, TextureLoadMultipleInEP) {
    std::string shader = R"(
@group(2) @binding(3) var tex1: texture_2d<f32>;
@group(1) @binding(4) var tex2: texture_multisampled_2d<f32>;
@group(0) @binding(1) var tex3: texture_2d<f32>;

@compute @workgroup_size(1)
fn main() {
  let num1 = textureLoad(tex1, vec2(0, 0), 0);
  let num2 = textureLoad(tex2, vec2(0, 0), 0);
  let num3 = textureLoad(tex3, vec2(0, 0), 0);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(2u, info.size());

    Inspector::LevelSampleInfo info1 = {
        /*type */ Inspector::TextureQueryType::kTextureNumLevels,
        /*group*/ 0,
        /*binding*/ 1,
    };
    Inspector::LevelSampleInfo info2 = {
        /*type */ Inspector::TextureQueryType::kTextureNumLevels,
        /*group*/ 2,
        /*binding*/ 3,
    };
    EXPECT_THAT(info, testing::UnorderedElementsAre(info1, info2));
}

TEST_F(InspectorTextureTest, TextureInSubfunction) {
    std::string shader = R"(
@group(2) @binding(3) var tex1: texture_2d<f32>;
@group(1) @binding(4) var tex2: texture_multisampled_2d<f32>;
@group(1) @binding(3) var tex3: texture_2d<f32>;

fn b(tx1: texture_2d<f32>, tx2: texture_multisampled_2d<f32>, tx3: texture_2d<f32>, tx4: texture_2d<f32>) {
  let v1 = textureNumLevels(tx1);
  let v2 = textureNumSamples(tx2);
  let v3 = textureLoad(tx3, vec2(0, 0), 0);
  let v4 = textureNumLevels(tx4);
}

fn a(tx1: texture_2d<f32>, tx2: texture_multisampled_2d<f32>, tx3: texture_2d<f32>) {
  b(tx1, tx2, tx3, tx1);
}

@compute @workgroup_size(1)
fn main() {
  a(tex1, tex2, tex3);
})";

    Inspector& inspector = Initialize(shader);
    auto info = inspector.GetTextureQueries("main");

    ASSERT_EQ(3u, info.size());

    Inspector::LevelSampleInfo info1 = {
        /*type */ Inspector::TextureQueryType::kTextureNumLevels,
        /*group*/ 1,
        /*binding*/ 3,
    };
    Inspector::LevelSampleInfo info2 = {
        /*type */ Inspector::TextureQueryType::kTextureNumLevels,
        /*group*/ 2,
        /*binding*/ 3,
    };
    Inspector::LevelSampleInfo info3 = {
        /*type */ Inspector::TextureQueryType::kTextureNumSamples,
        /*group*/ 1,
        /*binding*/ 4,
    };
    EXPECT_THAT(info, testing::UnorderedElementsAre(info1, info2, info3));
}

TEST_F(InspectorTextureTest, TextureMultipleEPs) {
    std::string shader = R"(
@group(0) @binding(0) var<storage, read_write> dstBuf : array<u32>;
@group(0) @binding(1) var tex1 : texture_2d_array<f32>;
@group(0) @binding(4) var tex2 : texture_multisampled_2d<f32>;
@group(1) @binding(3) var tex3 : texture_2d_array<f32>;

@compute @workgroup_size(1, 1, 1) fn main1() {
    dstBuf[0] = textureNumLayers(tex1);
    dstBuf[1] = textureNumLevels(tex1);
    dstBuf[2] = textureNumSamples(tex2);
    dstBuf[3] = textureNumLevels(tex3);
}

@compute @workgroup_size(1, 1, 1) fn main2() {
    dstBuf[0] = textureNumLayers(tex1);
    dstBuf[1] = textureNumLevels(tex1);
    dstBuf[2] = textureNumSamples(tex2);
}
    )";
    Inspector& inspector = Initialize(shader);
    {
        auto info = inspector.GetTextureQueries("main1");
        ASSERT_EQ(3u, info.size());

        Inspector::LevelSampleInfo info1 = {
            /*type */ Inspector::TextureQueryType::kTextureNumLevels,
            /*group*/ 1,
            /*binding*/ 3,
        };
        Inspector::LevelSampleInfo info2 = {
            /*type */ Inspector::TextureQueryType::kTextureNumLevels,
            /*group*/ 0,
            /*binding*/ 1,
        };
        Inspector::LevelSampleInfo info3 = {
            /*type */ Inspector::TextureQueryType::kTextureNumSamples,
            /*group*/ 0,
            /*binding*/ 4,
        };
        EXPECT_THAT(info, testing::UnorderedElementsAre(info1, info2, info3));
    }
    {
        auto info = inspector.GetTextureQueries("main2");
        ASSERT_EQ(2u, info.size());

        Inspector::LevelSampleInfo info1 = {
            /*type */ Inspector::TextureQueryType::kTextureNumLevels,
            /*group*/ 0,
            /*binding*/ 1,
        };
        Inspector::LevelSampleInfo info2 = {
            /*type */ Inspector::TextureQueryType::kTextureNumSamples,
            /*group*/ 0,
            /*binding*/ 4,
        };
        EXPECT_THAT(info, testing::UnorderedElementsAre(info1, info2));
    }
}

TEST_F(InspectorGetBlendSrcTest, Basic) {
    auto* src = R"(
enable dual_source_blending;

struct out_struct {
  @location(0u) @blend_src(0u) output_color: vec4f,
  @location(0u) @blend_src(1u) output_blend: vec4f,
}

@fragment
fn ep_func() -> out_struct {
  var out_var: out_struct;
  return out_var;
}
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(2u, result[0].output_variables.size());
    EXPECT_EQ(0u, result[0].output_variables[0].attributes.blend_src);
    EXPECT_EQ(1u, result[0].output_variables[1].attributes.blend_src);
}

TEST_F(InspectorSubgroupMatrixTest, DirectUse) {
    auto* src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> sm: subgroup_matrix_result<f32, 8, 8>;
@compute @workgroup_size(1) fn foo() { _ = sm; }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].uses_subgroup_matrix);
}

TEST_F(InspectorSubgroupMatrixTest, IndirectUse) {
    auto* src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> sm: subgroup_matrix_result<f32, 8, 8>;
fn foo() { _ = sm; }
@compute @workgroup_size(1) fn main() { foo(); }
)";
    Inspector& inspector = Initialize(src);

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].uses_subgroup_matrix);
}

}  // namespace

static std::ostream& operator<<(std::ostream& out, const Inspector::TextureQueryType& ty) {
    switch (ty) {
        case Inspector::TextureQueryType::kTextureNumLevels:
            out << "textureNumLevels";
            break;
        case Inspector::TextureQueryType::kTextureNumSamples:
            out << "textureNumSamples";
            break;
    }
    return out;
}

}  // namespace tint::inspector
