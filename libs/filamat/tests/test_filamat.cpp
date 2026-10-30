/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "generated/shaders.h"

#include "sca/ASTHelpers.h"
#include "sca/GLSLTools.h"

#include "shaders/ShaderGenerator.h"

#include <private/filament/Variant.h>

#include <filament/MaterialChunkType.h>
#include <filament/MaterialEnums.h>

#include <filamat/Enums.h>
#include <filamat/MaterialBuilder.h>

#include <filaflat/ChunkContainer.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/MaterialChunk.h>

#include <utils/JobSystem.h>
#include <utils/Panic.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

using namespace utils;
using namespace ASTHelpers;
using namespace filamat;
using namespace filament::backend;

static testing::AssertionResult PropertyListsMatch(const MaterialBuilder::PropertyList& expected,
        const MaterialBuilder::PropertyList& actual) {
    for (size_t i = 0; i < MaterialBuilder::MATERIAL_PROPERTIES_COUNT; i++) {
        if (expected[i] != actual[i]) {
            const auto& propString = Enums::toString<Property>(static_cast<Property>(i));
            return testing::AssertionFailure()
                    << "actual[" << propString << "] (" << actual[i]
                    << ") != expected[" << propString << "] (" << expected[i] << ")";
        }
    }
    return testing::AssertionSuccess();
}

static std::string shaderWithAllProperties(const ShaderStage type,
        const std::string& fragmentCode,
        const std::string& vertexCode = "",
        const MaterialBuilder::Shading shadingModel = MaterialBuilder::Shading::LIT,
        const MaterialBuilder::RefractionMode refractionMode = MaterialBuilder::RefractionMode::NONE,
        const MaterialBuilder::VertexDomain vertexDomain = MaterialBuilder::VertexDomain::OBJECT,
        const uint32_t targetApiLevel = filament::UNSTABLE_MATERIAL_API_LEVEL) {

    MaterialBuilder builder;
    builder.material(fragmentCode.c_str());
    builder.materialVertex(vertexCode.c_str());
    builder.platform(MaterialBuilder::Platform::MOBILE);
    builder.optimization(MaterialBuilder::Optimization::NONE);
    builder.shading(shadingModel);
    builder.refractionMode(refractionMode);
    builder.vertexDomain(vertexDomain);
    builder.setApiLevel(targetApiLevel);

    MaterialBuilder::PropertyList allProperties;
    std::fill_n(allProperties, MaterialBuilder::MATERIAL_PROPERTIES_COUNT, true);

    // Note: no need to call builder.build as we are only checking the properties.

    return builder.peek(type,
            {
                .shaderModel = ShaderModel::MOBILE,
                .targetApi = MaterialBuilder::TargetApi::OPENGL,
                .targetLanguage = MaterialBuilder::TargetLanguage::GLSL,
                .featureLevel = FeatureLevel::FEATURE_LEVEL_1,
            },
            allProperties);
}

TEST(StaticCodeAnalysisHelper, getFunctionName) {
    const auto name = getFunctionName("main(");
    EXPECT_EQ(name, "main");
}

TEST(StaticCodeAnalysisHelper, getFunctionNameNoParenthesis) {
    const auto name = getFunctionName("main");
    EXPECT_EQ(name, "main");
}

namespace {
class MaterialCompiler : public testing::Test {
protected:
    MaterialCompiler() = default;

    ~MaterialCompiler() override = default;

    void SetUp() override {
        jobSystem = std::make_unique<JobSystem>();
        jobSystem->adopt();
        MaterialBuilder::init();
    }

    void TearDown() override {
        jobSystem->emancipate();
        MaterialBuilder::shutdown();
    }

    std::unique_ptr<JobSystem> jobSystem;
};
} // namespace

TEST_F(MaterialCompiler, StaticCodeAnalyzerNothingDetected) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    constexpr MaterialBuilder::PropertyList expected{ false };
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerNothingDetectedVertex) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");
    const std::string vertexCode(R"(
        void materialVertex(inout MaterialVertexInputs material) {
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::VERTEX, fragmentCode, vertexCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::VERTEX, shaderCode, properties);
    constexpr MaterialBuilder::PropertyList expected{ false };
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}


TEST_F(MaterialCompiler, StaticCodeAnalyzerNotFollowingINParameters) {
    const std::string fragmentCode(R"(
        void notAffectingInput(in MaterialInputs material) {
            material.baseColor = vec4(0.8);
        }
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            notAffectingInput(material);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    constexpr MaterialBuilder::PropertyList expected{ false };
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerDirectAssign) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(0.8);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerDirectAssignVertex) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");
    const std::string vertexCode(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            material.clipSpaceTransform = mat4(2.0);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::VERTEX, fragmentCode, vertexCode,
            MaterialBuilder::Shading::LIT, MaterialBuilder::RefractionMode::NONE,
            MaterialBuilder::VertexDomain::DEVICE);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::VERTEX, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::CLIP_SPACE_TRANSFORM)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerCompoundAssign) {
    const char* const ops[] = { "*=", "+=", "-=", "/=" };
    for (const char* const op : ops) {
        const std::string fragmentCode = std::string(R"(
            void material(inout MaterialInputs material) {
                prepareMaterial(material);
                material.baseColor )") + op + R"( vec4(0.8);
                material.emissive )" + op + R"( 0.5;
                material.roughness )" + op + R"( 0.5;
            }
        )";

        const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

        constexpr GLSLTools glslTools;
        MaterialBuilder::PropertyList properties{ false };
        glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
        MaterialBuilder::PropertyList expected{ false };
        expected[static_cast<size_t>(MaterialBuilder::Property::BASE_COLOR)] = true;
        expected[static_cast<size_t>(MaterialBuilder::Property::EMISSIVE)] = true;
        expected[static_cast<size_t>(MaterialBuilder::Property::ROUGHNESS)] = true;
        EXPECT_TRUE(PropertyListsMatch(expected, properties));
    }

    // Also test vector *= matrix compound assignment
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor *= mat4(1.0);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerCompoundAssignVertex) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    const char* const ops[] = { "*=", "+=", "-=", "/=" };
    const char* const rhsValues[] = { "0.3", "mat4(2.0)" };
    for (const char* const op : ops) {
        for (const char* const rhs : rhsValues) {
            const std::string vertexCode = std::string(R"(
                void materialVertex(inout MaterialVertexInputs material) {
                    material.clipSpaceTransform )") + op + " " + rhs + R"(;
                }
            )";

            const std::string shaderCode = shaderWithAllProperties(ShaderStage::VERTEX,
                    fragmentCode, vertexCode, MaterialBuilder::Shading::LIT,
                    MaterialBuilder::RefractionMode::NONE, MaterialBuilder::VertexDomain::DEVICE);

            constexpr GLSLTools glslTools;
            MaterialBuilder::PropertyList properties{ false };
            glslTools.findProperties(ShaderStage::VERTEX, shaderCode, properties);
            MaterialBuilder::PropertyList expected{ false };
            expected[static_cast<size_t>(MaterialBuilder::Property::CLIP_SPACE_TRANSFORM)] = true;
            EXPECT_TRUE(PropertyListsMatch(expected, properties));
        }
    }
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerClipSpacePosition) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");
    const std::string vertexCode(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            material.clipSpacePosition = vec4(2.0);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::VERTEX, fragmentCode,
            vertexCode, MaterialBuilder::Shading::LIT, MaterialBuilder::RefractionMode::NONE,
            MaterialBuilder::VertexDomain::DEVICE, filament::UNSTABLE_MATERIAL_API_LEVEL);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::VERTEX, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::CLIP_SPACE_POSITION)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSecondRoughness) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.secondRoughness = 0.5;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "",
            MaterialBuilder::Shading::LIT);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::SECOND_ROUGHNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSecondRoughnessWeight) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.secondRoughnessWeight = 0.5;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "",
            MaterialBuilder::Shading::LIT);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::SECOND_ROUGHNESS_WEIGHT)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerDirectAssignWithSwizzling) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor.rgb = vec3(0.8);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolAsOutParameterWithAliasing) {
    const std::string fragmentCode(R"(

        void setBaseColor(inout vec4 aliasBaseColor) {
            aliasBaseColor = vec4(0.8,0.1,0.2,1.0);
        }

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            setBaseColor(material.baseColor);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolAsOutParameterWithAliasingAndSwizzling) {
    const std::string fragmentCode(R"(

        void setBaseColor(inout float aliasedRed) {
            aliasedRed = 0.8;
        }

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            setBaseColor(material.baseColor.r);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolInOutInChainWithDirectIndexIntoStruct) {
    const std::string fragmentCode(R"(

        float setBaseColorOtherFunction(inout vec4 myBaseColor) {
            myBaseColor = vec4(0.8,0.1,0.2,1.0);
            return 1.0;
        }

        void setBaseColor(inout vec4 aliasBaseColor) {
            setBaseColorOtherFunction(aliasBaseColor);
        }

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            setBaseColor(material.baseColor);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolInOutInChain) {
    const std::string fragmentCode(R"(

        float setBaseColorOtherFunction(inout MaterialInputs foo) {
            foo.baseColor = vec4(0.8,0.1,0.2,1.0);
            return 1.0;
        }

        void setBaseColor(inout MaterialInputs bar) {
            setBaseColorOtherFunction(bar);
        }

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            setBaseColor(material);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

// Tests all attributes in Property type.
TEST_F(MaterialCompiler, StaticCodeAnalyzerBaseColor) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(0.8);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerRoughness) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.roughness = 0.8;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::ROUGHNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerMetallic) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.metallic = 0.8;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::METALLIC)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerReflectance) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.reflectance= 0.8;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::REFLECTANCE)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

static bool readsSurfaceInterpolants(const std::string& fragmentCode,
        MaterialBuilder::Shading shading = MaterialBuilder::Shading::UNLIT) {
    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode,
            "", shading);
    auto const info = GLSLTools::analyzeFragmentShader(shaderCode, ShaderModel::MOBILE,
            MaterialBuilder::MaterialDomain::SURFACE, MaterialBuilder::TargetApi::OPENGL,
            MaterialBuilder::TargetLanguage::GLSL, false);
    EXPECT_TRUE(info.has_value());
    return info.has_value() && info->userMaterialReadsSurfaceInterpolants;
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSurfaceInterpolantsNotRead) {
    EXPECT_FALSE(readsSurfaceInterpolants(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(vec3(getObjectUserData()), 1.0);
        }
    )"));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSurfaceInterpolantsReadThroughGetter) {
    EXPECT_TRUE(readsSurfaceInterpolants(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(getWorldPosition(), 1.0);
        }
    )"));
    EXPECT_TRUE(readsSurfaceInterpolants(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(getNormalizedViewportCoord(), 1.0);
        }
    )"));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSurfaceInterpolantsReadThroughHelper) {
    EXPECT_TRUE(readsSurfaceInterpolants(R"(
        vec3 helper() {
            return getWorldViewVector();
        }
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(helper(), 1.0);
        }
    )"));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSurfaceInterpolantsReadDirectly) {
    EXPECT_TRUE(readsSurfaceInterpolants(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(shading_view, 1.0);
        }
    )"));
    EXPECT_TRUE(readsSurfaceInterpolants(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(shading_normalizedViewportCoord, 0.0, 1.0);
        }
    )"));
    EXPECT_TRUE(readsSurfaceInterpolants(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vertex_worldPosition;
        }
    )"));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSurfaceInterpolantsReadThroughEveryGetter) {
    // Each getter reads a different shading_* global, so renaming one that the symbol list
    // misses fails here. Lit, because the tangent getters only exist with tangents.
    for (const char* getter : { "getWorldTangentFrame()[0]", "getWorldPosition()",
                 "getUserWorldPosition()", "getWorldViewVector()", "getWorldNormalVector()",
                 "getWorldGeometricNormalVector()", "getWorldReflectedVector()",
                 "vec3(getNdotV())", "getNormalizedViewportCoord()" }) {
        SCOPED_TRACE(getter);
        EXPECT_TRUE(readsSurfaceInterpolants(std::string(R"(
            void material(inout MaterialInputs material) {
                prepareMaterial(material);
                material.baseColor = vec4()") + getter + R"(, 1.0);
            }
        )", MaterialBuilder::Shading::LIT));
    }
}

// The analysis only notices reads of the symbols it lists, so every shading_* global in
// common_shading.fs and every varying that MATERIAL_SKIPS_SURFACE_INTERPOLANTS removes in
// surface_varyings.glsl must be in GLSLTools::kSurfaceInterpolantSymbols.
TEST(StaticCodeAnalysis, SurfaceInterpolantSymbolsCoverTheShaders) {
    auto const expectListed = [](const std::string& source, const char* pattern) {
        const auto& symbols = GLSLTools::kSurfaceInterpolantSymbols;
        const std::regex re(pattern);
        int count = 0;
        for (auto it = std::sregex_iterator(source.begin(), source.end(), re);
                it != std::sregex_iterator(); ++it, ++count) {
            EXPECT_NE(std::find(std::begin(symbols), std::end(symbols), it->str()),
                    std::end(symbols)) << it->str() << " is not in kSurfaceInterpolantSymbols";
        }
        EXPECT_GT(count, 0) << pattern;
    };

    expectListed(std::string(reinterpret_cast<const char*>(SHADERS_COMMON_SHADING_FS_DATA),
            SHADERS_COMMON_SHADING_FS_SIZE), R"(\bshading_\w+)");

    const std::string varyings(reinterpret_cast<const char*>(SHADERS_SURFACE_VARYINGS_GLSL_DATA),
            SHADERS_SURFACE_VARYINGS_GLSL_SIZE);
    const size_t begin = varyings.find("#if !defined(FILAMENT_SKIPS_SURFACE_INTERPOLANTS)");
    ASSERT_NE(begin, std::string::npos);
    std::istringstream lines(varyings.substr(begin));
    std::string line;
    std::string block;
    int depth = 0;
    while (std::getline(lines, line)) {
        if (line.rfind("#if", 0) == 0) {
            depth++;
        } else if (line.rfind("#endif", 0) == 0 && --depth == 0) {
            break;
        }
        block += line + '\n';
    }
    expectListed(block, R"(\bvertex_\w+)");
}

// Builds an unlit material with the fog variant filtered out, as optimized mobile GLSL so
// that the preprocessor has run and the test sees the varyings the shaders actually use.
static Package buildUnlitMobileGlsl(JobSystem& jobSystem, const char* fragmentCode) {
    MaterialBuilder builder;
    builder.material(fragmentCode);
    builder.shading(MaterialBuilder::Shading::UNLIT);
    builder.platform(MaterialBuilder::Platform::MOBILE);
    builder.targetApi(MaterialBuilder::TargetApi::OPENGL);
    builder.optimization(MaterialBuilder::Optimization::PERFORMANCE);
    builder.variantFilter(filament::UserVariantFilterMask(
            filament::UserVariantFilterBit::FOG | filament::UserVariantFilterBit::SKINNING |
            filament::UserVariantFilterBit::SHADOW_RECEIVER | filament::UserVariantFilterBit::VSM |
            filament::UserVariantFilterBit::SSR));
    Package package = builder.build(jobSystem);
    EXPECT_TRUE(package.isValid());
    return package;
}

static bool shaderUsesSurfaceInterpolants(const Package& package, filament::Variant variant,
        ShaderStage stage) {
    filaflat::ChunkContainer container(package.getData(), package.getSize());
    EXPECT_TRUE(container.parse());
    filaflat::BlobDictionary dictionary;
    EXPECT_TRUE(filaflat::DictionaryReader::unflatten(container, ChunkType::DictionaryText,
            dictionary));
    filaflat::MaterialChunk chunk(container);
    EXPECT_TRUE(chunk.initialize(ChunkType::MaterialGlsl));
    filaflat::ShaderContent content;
    EXPECT_TRUE(chunk.getShader(content, dictionary, ShaderModel::MOBILE, variant, stage));
    const std::string glsl(reinterpret_cast<const char*>(content.data()), content.size());
    return glsl.find("vertex_worldPosition") != std::string::npos ||
           glsl.find("vertex_position") != std::string::npos;
}

TEST_F(MaterialCompiler, UnlitMaterialSkipsSurfaceInterpolants) {
    const filament::Variant color(0);
    const filament::Variant depth(filament::Variant::DEP);
    // The picking fragment shader pairs with the plain depth vertex shader.
    const filament::Variant picking(filament::Variant::DEP | filament::Variant::PCK);

    const Package skips = buildUnlitMobileGlsl(*jobSystem, R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(1.0);
        }
    )");
    EXPECT_FALSE(shaderUsesSurfaceInterpolants(skips, color, ShaderStage::VERTEX));
    EXPECT_FALSE(shaderUsesSurfaceInterpolants(skips, color, ShaderStage::FRAGMENT));
    // Picking reads vertex_position, so the depth variants keep the interpolants.
    EXPECT_TRUE(shaderUsesSurfaceInterpolants(skips, depth, ShaderStage::VERTEX));
    EXPECT_TRUE(shaderUsesSurfaceInterpolants(skips, picking, ShaderStage::FRAGMENT));

    const Package reads = buildUnlitMobileGlsl(*jobSystem, R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(getWorldPosition(), 1.0);
        }
    )");
    EXPECT_TRUE(shaderUsesSurfaceInterpolants(reads, color, ShaderStage::VERTEX));
    EXPECT_TRUE(shaderUsesSurfaceInterpolants(reads, color, ShaderStage::FRAGMENT));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerAmbientOcclusion) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.ambientOcclusion = 0.8;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::AMBIENT_OCCLUSION)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerClearCoat) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.clearCoat = 0.8;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::CLEAR_COAT)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerTransmission) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.absorption = vec3(0.0);
            material.transmission = 0.96;
            material.ior = 1.33;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "",
            MaterialBuilder::Shading::LIT, MaterialBuilder::RefractionMode::CUBEMAP);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::ABSORPTION)] = true;
    expected[static_cast<size_t>(MaterialBuilder::Property::TRANSMISSION)] = true;
    expected[static_cast<size_t>(MaterialBuilder::Property::IOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerDispersion) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.absorption = vec3(0.0);
            material.transmission = 0.96;
            material.ior = 1.33;
            material.dispersion = 0.33;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "",
            MaterialBuilder::Shading::LIT, MaterialBuilder::RefractionMode::CUBEMAP);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::ABSORPTION)] = true;
    expected[static_cast<size_t>(MaterialBuilder::Property::TRANSMISSION)] = true;
    expected[static_cast<size_t>(MaterialBuilder::Property::IOR)] = true;
    expected[static_cast<size_t>(MaterialBuilder::Property::DISPERSION)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerClearCoatRoughness) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.clearCoatRoughness = 0.8;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "");

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::CLEAR_COAT_ROUGHNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerClearCoatNormal) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.clearCoat = 0.8;
            material.clearCoatNormal = vec3(1.0, 1.0, 1.0);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "");

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::CLEAR_COAT)] = true;
    expected[static_cast<size_t>(MaterialBuilder::Property::CLEAR_COAT_NORMAL)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerThickness) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.thickness= 0.8;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "",
            MaterialBuilder::Shading::SUBSURFACE);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::THICKNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSubsurfacePower) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.subsurfacePower = 0.8;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "",
            MaterialBuilder::Shading::SUBSURFACE);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::SUBSURFACE_POWER)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSubsurfaceColor) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.subsurfaceColor= vec3(0.8);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "",
            MaterialBuilder::Shading::SUBSURFACE);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::SUBSURFACE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerAnisotropicDirection) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.anisotropyDirection = vec3(0.8);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::ANISOTROPY_DIRECTION)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerAnisotropic) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.anisotropy = 0.8;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::ANISOTROPY)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSheenColor) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.sheenColor = vec3(0.8);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode, "",
            MaterialBuilder::Shading::CLOTH);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::SHEEN_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSheenRoughness) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.sheenRoughness = 1.0;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::SHEEN_ROUGHNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerIridescence) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.iridescence = 1.0;
            material.iridescenceIor = 1.3;
            material.iridescenceThickness = 400.0;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::IRIDESCENCE)] = true;
    expected[static_cast<size_t>(MaterialBuilder::Property::IRIDESCENCE_IOR)] = true;
    expected[static_cast<size_t>(MaterialBuilder::Property::IRIDESCENCE_THICKNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerNormal) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.normal= vec3(0.8);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::NORMAL)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerBentNormal) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.bentNormal = vec3(0.8);
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::BENT_NORMAL)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerShadowStrength) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.shadowStrength = 0.1;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::SHADOW_STRENGTH)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerOutputFactor) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.postLightingColor = vec4(1.0);
            material.postLightingMixFactor = 0.5;
        }
    )");

    const std::string shaderCode = shaderWithAllProperties(ShaderStage::FRAGMENT, fragmentCode);

    constexpr GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[static_cast<size_t>(MaterialBuilder::Property::POST_LIGHTING_COLOR)] = true;
    expected[static_cast<size_t>(MaterialBuilder::Property::POST_LIGHTING_MIX_FACTOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerVertexCustomDepthDetection) {
    const std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    auto analyzeVertex = [&](const std::string& vertexCode,
            MaterialBuilder::VertexDomain domain = MaterialBuilder::VertexDomain::OBJECT) {
        MaterialBuilder builder;
        builder.material(fragmentCode.c_str());
        builder.materialVertex(vertexCode.c_str());
        builder.platform(MaterialBuilder::Platform::MOBILE);
        builder.optimization(MaterialBuilder::Optimization::NONE);
        builder.shading(MaterialBuilder::Shading::LIT);
        builder.vertexDomain(domain);
        builder.require(filament::VertexAttribute::UV0);
        builder.require(filament::VertexAttribute::COLOR);
        builder.setApiLevel(filament::UNSTABLE_MATERIAL_API_LEVEL);

        MaterialBuilder::PropertyList allProperties;
        std::fill_n(allProperties, MaterialBuilder::MATERIAL_PROPERTIES_COUNT, true);

        const std::string shaderCode = builder.peek(ShaderStage::VERTEX,
                {
                    .shaderModel = ShaderModel::MOBILE,
                    .targetApi = MaterialBuilder::TargetApi::OPENGL,
                    .targetLanguage = MaterialBuilder::TargetLanguage::GLSL,
                    .featureLevel = FeatureLevel::FEATURE_LEVEL_1,
                },
                allProperties);
        auto const result = GLSLTools::analyzeVertexShader(shaderCode, ShaderModel::MOBILE,
                MaterialBuilder::MaterialDomain::SURFACE, MaterialBuilder::TargetApi::OPENGL,
                MaterialBuilder::TargetLanguage::GLSL);
        EXPECT_TRUE(result.has_value());
        return result.has_value() && result->userMaterialVertexHasCustomDepth;
    };

    // Empty vertex shader does not modify position.
    EXPECT_FALSE(analyzeVertex(R"(
        void materialVertex(inout MaterialVertexInputs material) {
        }
    )"));

    // Modifying UVs, color, worldNormal, or reading worldPosition does not modify vertex position.
    EXPECT_FALSE(analyzeVertex(R"(
        float readPos(in vec4 p) {
            return p.x + p.y;
        }
        void materialVertex(inout MaterialVertexInputs material) {
            material.uv0 = vec2(readPos(material.worldPosition));
            material.color = vec4(1.0);
            material.worldNormal = normalize(material.worldNormal);
        }
    )"));

    // Direct or compound write to worldPosition modifies vertex position.
    EXPECT_TRUE(analyzeVertex(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            material.worldPosition.xyz += vec3(0.0, 1.0, 0.0);
        }
    )"));

    // Passing worldPosition as an inout parameter to a helper function modifies vertex position,
    // including when the call appears on the RHS of an assignment/initialization.
    EXPECT_TRUE(analyzeVertex(R"(
        void displace(inout vec3 pos) {
            pos.y += 1.0;
        }
        void materialVertex(inout MaterialVertexInputs material) {
            displace(material.worldPosition.xyz);
        }
    )"));

    EXPECT_TRUE(analyzeVertex(R"(
        float applyWind(inout vec4 pos) {
            pos.x += sin(getUserTime().x) * 0.1;
            return pos.x;
        }
        void materialVertex(inout MaterialVertexInputs material) {
            float sway = applyWind(material.worldPosition);
            material.uv0.x = sway;
        }
    )"));

    // Passing the MaterialVertexInputs struct to an inout helper that writes worldPosition.
    EXPECT_TRUE(analyzeVertex(R"(
        void helper(inout MaterialVertexInputs m) {
            m.worldPosition = vec4(1.0);
        }
        void materialVertex(inout MaterialVertexInputs material) {
            helper(material);
        }
    )"));

    // Passing the MaterialVertexInputs struct to an inout helper that only writes uv0.
    EXPECT_FALSE(analyzeVertex(R"(
        void helper(inout MaterialVertexInputs m) {
            m.uv0 = vec2(0.5);
        }
        void materialVertex(inout MaterialVertexInputs material) {
            helper(material);
        }
    )"));

    // Direct writes to clipSpaceTransform or clipSpacePosition modify vertex position.
    EXPECT_TRUE(analyzeVertex(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            material.clipSpaceTransform = mat4(1.0);
        }
    )", MaterialBuilder::VertexDomain::DEVICE));

    EXPECT_TRUE(analyzeVertex(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            material.clipSpacePosition = vec4(0.0, 0.0, 0.0, 1.0);
        }
    )", MaterialBuilder::VertexDomain::DEVICE));

    // Direct write to gl_Position or gl_PointSize modifies vertex position.
    EXPECT_TRUE(analyzeVertex(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
        }
    )"));

    EXPECT_TRUE(analyzeVertex(R"(
        float setPointSize() {
            gl_PointSize = 4.0;
            return 1.0;
        }
        void materialVertex(inout MaterialVertexInputs material) {
            material.uv0.x = setPointSize();
        }
    )"));
}

TEST_F(MaterialCompiler, EmptyName) {
    const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    MaterialBuilder builder;
    builder.material(shaderCode.c_str());
    // The material should compile successfully with an empty name
    builder.name("");
    const Package result = builder.build(*jobSystem);
}

TEST_F(MaterialCompiler, Uv0AndUv1) {
    MaterialBuilder builder;
    // Requiring both sets of UV coordinates should not fail.
    builder.require(filament::VertexAttribute::UV0);
    builder.require(filament::VertexAttribute::UV1);
    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, FiveCustomVariables) {
    MaterialBuilder builder;
    builder.variable(MaterialBuilder::Variable::CUSTOM0, "custom0");
    builder.variable(MaterialBuilder::Variable::CUSTOM1, "custom1");
    builder.variable(MaterialBuilder::Variable::CUSTOM2, "custom2");
    builder.variable(MaterialBuilder::Variable::CUSTOM3, "custom3");
    builder.variable(MaterialBuilder::Variable::CUSTOM4, "custom4");
    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, FourCustomVariablesAndColorAttribute) {
    MaterialBuilder builder;
    builder.require(filament::VertexAttribute::COLOR);
    builder.variable(MaterialBuilder::Variable::CUSTOM0, "custom0");
    builder.variable(MaterialBuilder::Variable::CUSTOM1, "custom1");
    builder.variable(MaterialBuilder::Variable::CUSTOM2, "custom2");
    builder.variable(MaterialBuilder::Variable::CUSTOM3, "custom3");
    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, FiveCustomVariablesAndColorAttributeFails) {
    MaterialBuilder builder;
    builder.require(filament::VertexAttribute::COLOR);
    builder.variable(MaterialBuilder::Variable::CUSTOM0, "custom0");
    builder.variable(MaterialBuilder::Variable::CUSTOM1, "custom1");
    builder.variable(MaterialBuilder::Variable::CUSTOM2, "custom2");
    builder.variable(MaterialBuilder::Variable::CUSTOM3, "custom3");
    builder.variable(MaterialBuilder::Variable::CUSTOM4, "custom4");
    const Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomVariable4AndColorAttributeFails) {
    MaterialBuilder builder;
    builder.require(filament::VertexAttribute::COLOR);
    builder.variable(MaterialBuilder::Variable::CUSTOM4, "custom4");
    const Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, Arrays) {
    MaterialBuilder builder;

    builder.parameter("f4", 1, UniformType::FLOAT4);
    builder.parameter("f1", 1, UniformType::FLOAT);

    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, CustomSurfaceShadingRequiresLit) {
    MaterialBuilder builder;
    builder.customSurfaceShading(true);
    builder.shading(filament::Shading::UNLIT);
    const Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomSurfaceShadingRequiresFunction) {
    MaterialBuilder builder;
    builder.customSurfaceShading(true);
    builder.shading(filament::Shading::LIT);
    const Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomSurfaceShadingHasFunction) {
    const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }

        vec3 surfaceShading(
                const MaterialInputs materialInputs,
                const ShadingData shadingData,
                const LightData lightData
        ) {
            return vec3(1.0);
        }
    )");

    MaterialBuilder builder;
    builder.customSurfaceShading(true);
    builder.shading(filament::Shading::LIT);
    builder.material(shaderCode.c_str());
    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, ConstantParameter) {
  const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            if (materialConstants_myBoolConstant) {
              material.baseColor.rgb = float3(materialConstants_myFloatConstant);
              int anInt = materialConstants_myIntConstant;
            }
        }
    )");
    const std::string vertexCode(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            int anInt = materialConstants_myIntConstant;
            bool aBool = materialConstants_myBoolConstant;
            float aFloat = materialConstants_myFloatConstant;
        }
    )");
  MaterialBuilder builder;
  builder.constant("myFloatConstant", ConstantType::FLOAT, 1.0f);
  builder.constant("myIntConstant", ConstantType::INT, 123);
  builder.constant("myBoolConstant", ConstantType::BOOL, true);
  builder.constant<bool>("myOtherBoolConstant", ConstantType::BOOL);

  builder.shading(filament::Shading::LIT);
  builder.material(shaderCode.c_str());
  builder.materialVertex(vertexCode.c_str());
  const Package result = builder.build(*jobSystem);
  EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, ConstantParameterSameName) {
#ifdef __EXCEPTIONS
    EXPECT_THROW({
        MaterialBuilder builder;
        builder.constant("myFloatConstant", ConstantType::FLOAT, 1.0f);
        builder.constant("myFloatConstant", ConstantType::FLOAT, 1.0f);
    }, utils::PostconditionPanic);
#endif
}

TEST_F(MaterialCompiler, ConstantParameterWrongType) {
#ifdef __EXCEPTIONS
    EXPECT_THROW({
        MaterialBuilder builder;
        builder.constant("myFloatConstant", ConstantType::FLOAT, 10);
    }, utils::PostconditionPanic);
#endif
}

TEST_F(MaterialCompiler, FeatureLevel0Sampler2D) {
  const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = texture(materialParams_sampler, vec2(0.0, 0.0));
        }
    )");
  MaterialBuilder builder;
  builder.parameter("sampler", SamplerType::SAMPLER_2D);

  builder.featureLevel(FeatureLevel::FEATURE_LEVEL_0);
  builder.shading(filament::Shading::UNLIT);
  builder.material(shaderCode.c_str());
  const Package result = builder.build(*jobSystem);
  EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, SamplerTransformName) {
  const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            vec3 uvw = materialParams.sampler_transform * vec3(0.0, 0.0, 0.0);
            material.baseColor = texture(materialParams_sampler, uvw.xy);
        }
    )");
  MaterialBuilder builder;
  builder.parameter("sampler", SamplerType::SAMPLER_2D, SamplerFormat::FLOAT,
          ParameterPrecision::DEFAULT, true, false, "sampler_transform");

  builder.featureLevel(FeatureLevel::FEATURE_LEVEL_0);
  builder.shading(filament::Shading::UNLIT);
  builder.material(shaderCode.c_str());
  const Package result = builder.build(*jobSystem);
  EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, SamplerMissingTransformName) {
  const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            vec3 uvw = materialParams.sampler_transform * vec3(0.0, 0.0, 0.0);
            material.baseColor = texture(materialParams_sampler, uvw.xy);
        }
    )");
  MaterialBuilder builder;
  builder.parameter("sampler", SamplerType::SAMPLER_2D);

  builder.featureLevel(FeatureLevel::FEATURE_LEVEL_0);
  builder.shading(filament::Shading::UNLIT);
  builder.material(shaderCode.c_str());
  const Package result = builder.build(*jobSystem);
  EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, FeatureLevel0Ess3CallFails) {
  const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = texture(materialParams_sampler, vec3(0.0, 0.0));
        }
    )");
  MaterialBuilder builder;
  builder.parameter("sampler", SamplerType::SAMPLER_2D);

  builder.featureLevel(FeatureLevel::FEATURE_LEVEL_0);
  builder.shading(filament::Shading::UNLIT);
  builder.material(shaderCode.c_str());
  const Package result = builder.build(*jobSystem);
  EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, EmbedMaterialSourceSucceeds) {
    const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(1.);
        }
    )");
    MaterialBuilder builder;
    builder.materialSource(shaderCode);

    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, ClientApiLevelUnstableNoUseOfUnstableApiSucceeds) {
    const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(1.);
        }
    )");
    MaterialBuilder builder;
    builder.material(shaderCode.c_str());
    builder.setApiLevel(filament::UNSTABLE_MATERIAL_API_LEVEL);

    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, ClientApiLevelUnstableUseOfUnstableApiSucceeds) {
    const std::string shaderCode(R"(
        // Emulate an unstable api added in the public shaders
        #if CLIENT_MATERIAL_API_LEVEL >= UNSTABLE_MATERIAL_API_LEVEL
        /* @unstable-api */
        float unstableApi() { return 1.; }
        #endif

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(unstableApi());
        }
    )");
    MaterialBuilder builder;
    builder.material(shaderCode.c_str());
    builder.setApiLevel(filament::UNSTABLE_MATERIAL_API_LEVEL);

    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, ClientApiLevelReleasedUseOfUnstableApiFails) {
    const std::string shaderCode(R"(
        // Emulate an unstable api added in the public shaders
        #if CLIENT_MATERIAL_API_LEVEL >= UNSTABLE_MATERIAL_API_LEVEL
        /* @unstable-api */
        float unstableApi() { return 1.; }
        #endif

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(unstableApi());
        }
    )");
    MaterialBuilder builder;
    builder.material(shaderCode.c_str());
    builder.setApiLevel(filament::RELEASED_MATERIAL_API_LEVEL);

    const Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomOutputSuccess) {
    const std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.customOutput = uvec4(1, 2, 3, 4);
        }
    )");

    MaterialBuilder builder;
    builder.material(shaderCode.c_str());
    builder.shading(filament::Shading::UNLIT);
    builder.blending(filament::BlendingMode::OPAQUE);
    builder.featureLevel(FeatureLevel::FEATURE_LEVEL_1);
    builder.output(MaterialBuilder::VariableQualifier::OUT,
                   MaterialBuilder::OutputTarget::COLOR,
                   MaterialBuilder::Precision::DEFAULT,
                   MaterialBuilder::OutputType::UINT4,
                   "customOutput");

    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, CustomOutputFeatureLevel0Fails) {
    MaterialBuilder builder;
    builder.featureLevel(FeatureLevel::FEATURE_LEVEL_0);
    builder.output(MaterialBuilder::VariableQualifier::OUT,
                   MaterialBuilder::OutputTarget::COLOR,
                   MaterialBuilder::Precision::DEFAULT,
                   MaterialBuilder::OutputType::UINT4,
                   "customOutput");
    const Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomOutputMoreThanOneFails) {
    MaterialBuilder builder;
    builder.featureLevel(FeatureLevel::FEATURE_LEVEL_1);
    builder.output(MaterialBuilder::VariableQualifier::OUT,
                   MaterialBuilder::OutputTarget::COLOR,
                   MaterialBuilder::Precision::DEFAULT,
                   MaterialBuilder::OutputType::UINT4,
                   "customOutput");
    builder.output(MaterialBuilder::VariableQualifier::OUT,
                   MaterialBuilder::OutputTarget::COLOR,
                   MaterialBuilder::Precision::DEFAULT,
                   MaterialBuilder::OutputType::UINT4,
                   "customOutput1");
    const Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomOutputLitFails) {
    MaterialBuilder builder;
    builder.featureLevel(FeatureLevel::FEATURE_LEVEL_1);
    builder.shading(filament::Shading::LIT);
    builder.output(MaterialBuilder::VariableQualifier::OUT,
                   MaterialBuilder::OutputTarget::COLOR,
                   MaterialBuilder::Precision::DEFAULT,
                   MaterialBuilder::OutputType::UINT4,
                   "customOutput");
    const Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomOutputTransparentFails) {
    MaterialBuilder builder;
    builder.featureLevel(FeatureLevel::FEATURE_LEVEL_1);
    builder.shading(filament::Shading::UNLIT);
    builder.blending(filament::BlendingMode::TRANSPARENT);
    builder.output(MaterialBuilder::VariableQualifier::OUT,
                   MaterialBuilder::OutputTarget::COLOR,
                   MaterialBuilder::Precision::DEFAULT,
                   MaterialBuilder::OutputType::UINT4,
                   "customOutput");
    const Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

#if FILAMENT_SUPPORTS_WEBGPU
TEST_F(MaterialCompiler, WgslConversionBakedColor) {
    std::string bakedColorCodeFrag(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = getColor();
        }
    )");
    MaterialBuilder builder;
    builder.targetApi(MaterialBuilderBase::TargetApi::WEBGPU);
    builder.material(bakedColorCodeFrag.c_str());
    builder.shading(MaterialBuilder::Shading::UNLIT);
    builder.name("BakedColor");
    builder.culling(CullingMode::NONE);
    builder.featureLevel(FeatureLevel::FEATURE_LEVEL_0);
    builder.require(filament::VertexAttribute::COLOR);
    builder.variantFilter(static_cast<filament::UserVariantFilterMask>(
            filament::UserVariantFilterBit::SKINNING | filament::UserVariantFilterBit::STE));
    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, WgslConversionSandboxLitTransparent) {
    std::string litTransparentCodeFrag(R"(
    void material(inout MaterialInputs material) {
        prepareMaterial(material);
        material.baseColor.rgb = materialParams.baseColor * materialParams.alpha;
        material.baseColor.a = materialParams.alpha;
        material.roughness = materialParams.roughness;
        material.metallic = materialParams.metallic;
        material.reflectance = materialParams.reflectance;
        material.sheenColor = materialParams.sheenColor;
        material.sheenRoughness = materialParams.sheenRoughness;
        material.clearCoat = materialParams.clearCoat;
        material.clearCoatRoughness = materialParams.clearCoatRoughness;
        material.anisotropy = materialParams.anisotropy;
        material.emissive = materialParams.emissive;
    }
    )");
    MaterialBuilder builder;
    builder.targetApi(MaterialBuilderBase::TargetApi::WEBGPU);
    builder.material(litTransparentCodeFrag.c_str());
    builder.shading(MaterialBuilder::Shading::LIT);
    builder.name("LitTransparent");
    builder.blending(filament::BlendingMode::TRANSPARENT);
    builder.specularAntiAliasing(true);

    builder.parameter("alpha", 1, UniformType::FLOAT);
    builder.parameter("baseColor", 1, UniformType::FLOAT3);
    builder.parameter("roughness", 1, UniformType::FLOAT);
    builder.parameter("metallic", 1, UniformType::FLOAT);
    builder.parameter("reflectance", 1, UniformType::FLOAT);
    builder.parameter("sheenColor", 1, UniformType::FLOAT3);
    builder.parameter("sheenRoughness", 1, UniformType::FLOAT);
    builder.parameter("clearCoat", 1, UniformType::FLOAT);
    builder.parameter("clearCoatRoughness", 1, UniformType::FLOAT);
    builder.parameter("anisotropy", 1, UniformType::FLOAT);
    builder.parameter("emissive", 1, UniformType::FLOAT4);

    builder.variantFilter(static_cast<filament::UserVariantFilterMask>(
            filament::UserVariantFilterBit::SKINNING | filament::UserVariantFilterBit::STE));
    const Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}
#endif

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    const int rv = RUN_ALL_TESTS();
    if (testing::UnitTest::GetInstance()->test_to_run_count() == 0) {
        //If you run a test filter that contains 0 tests that was likely not intentional. Fail in that scenario.
        return 1;
    }
    return rv;
}
