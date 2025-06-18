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

#include <gtest/gtest.h>

#include "sca/ASTHelpers.h"
#include "sca/GLSLTools.h"
#include "shaders/ShaderGenerator.h"

#include "MockIncluder.h"

#include <filamat/Enums.h>
#include <filamat/MaterialBuilder.h>

#include <utils/JobSystem.h>
#include <utils/Panic.h>

#include <memory>

using namespace utils;
using namespace ASTHelpers;
using namespace filamat;
using namespace filament::backend;

static ::testing::AssertionResult PropertyListsMatch(const MaterialBuilder::PropertyList& expected,
        const MaterialBuilder::PropertyList& actual) {
    for (size_t i = 0; i < MaterialBuilder::MATERIAL_PROPERTIES_COUNT; i++) {
        if (expected[i] != actual[i]) {
            const auto& propString = Enums::toString<Property>(Property(i));
            return ::testing::AssertionFailure()
                    << "actual[" << propString << "] (" << actual[i]
                    << ") != expected[" << propString << "] (" << expected[i] << ")";
        }
    }
    return ::testing::AssertionSuccess();
}

std::string shaderWithAllProperties(JobSystem& jobSystem, ShaderStage type,
        const std::string& fragmentCode, const std::string& vertexCode = "",
        filamat::MaterialBuilder::Shading shadingModel = filamat::MaterialBuilder::Shading::LIT,
        filamat::MaterialBuilder::RefractionMode refractionMode = filamat::MaterialBuilder::RefractionMode::NONE,
        filamat::MaterialBuilder::VertexDomain vertexDomain = filamat::MaterialBuilder::VertexDomain::OBJECT) {
    MockIncluder includer;
    includer
            .sourceForInclude("modify_normal.h", "material.normal = vec3(0.8);");

    filamat::MaterialBuilder builder;
    builder.material(fragmentCode.c_str());
    builder.materialVertex(vertexCode.c_str());
    builder.platform(filamat::MaterialBuilder::Platform::MOBILE);
    builder.optimization(filamat::MaterialBuilder::Optimization::NONE);
    builder.shading(shadingModel);
    builder.includeCallback(includer);
    builder.refractionMode(refractionMode);
    builder.vertexDomain(vertexDomain);

    MaterialBuilder::PropertyList allProperties;
    std::fill_n(allProperties, MaterialBuilder::MATERIAL_PROPERTIES_COUNT, true);

    // We need to "build" the material to resolve any includes in user code.
    builder.build(jobSystem);

    return builder.peek(type, {
                    ShaderModel::MOBILE,
                    MaterialBuilder::TargetApi::OPENGL,
                    MaterialBuilder::TargetLanguage::GLSL,
                    FeatureLevel::FEATURE_LEVEL_1,
            },
            allProperties);
}

TEST(StaticCodeAnalysisHelper, getFunctionName) {
    auto name = getFunctionName("main(");
    EXPECT_EQ(name, "main");
}

TEST(StaticCodeAnalysisHelper, getFunctionNameNoParenthesis) {
    auto name = getFunctionName("main");
    EXPECT_EQ(name, "main");
}

class MaterialCompiler : public ::testing::Test {
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

TEST_F(MaterialCompiler, StaticCodeAnalyzerNothingDetected) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerNothingDetectedVertex) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");
    std::string vertexCode(R"(
        void materialVertex(inout MaterialVertexInputs material) {
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::VERTEX,
            fragmentCode, vertexCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::VERTEX, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}


TEST_F(MaterialCompiler, StaticCodeAnalyzerNotFollowingINParameters) {
    std::string fragmentCode(R"(
        void notAffectingInput(in MaterialInputs material) {
            material.baseColor = vec4(0.8);
        }
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            notAffectingInput(material);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerDirectAssign) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(0.8);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerDirectAssignVertex) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");
    std::string vertexCode(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            material.clipSpaceTransform = mat4(2.0);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::VERTEX,
            fragmentCode, vertexCode,
            MaterialBuilder::Shading::LIT,
            MaterialBuilder::RefractionMode::NONE,
            MaterialBuilder::VertexDomain::DEVICE);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::VERTEX, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::CLIP_SPACE_TRANSFORM)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerDirectAssignWithSwizzling) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor.rgb = vec3(0.8);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolAsOutParameterWithAliasing) {
    std::string fragmentCode(R"(

        void setBaseColor(inout vec4 aliasBaseColor) {
            aliasBaseColor = vec4(0.8,0.1,0.2,1.0);
        }

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            setBaseColor(material.baseColor);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolAsOutParameterWithAliasingAndSwizzling) {
    std::string fragmentCode(R"(

        void setBaseColor(inout float aliasedRed) {
            aliasedRed = 0.8;
        }

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            setBaseColor(material.baseColor.r);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolInOutInChainWithDirectIndexIntoStruct) {
    std::string fragmentCode(R"(

        float setBaseColorOtherFunction(inout vec4 myBaseColor) {
            myBaseColor = vec4(0.8,0.1,0.2,1.0);
            return 1.0;
        }

        void setBaseColor(inout vec4 aliaseBaseColor) {
            setBaseColorOtherFunction(aliaseBaseColor);
        }

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            setBaseColor(material.baseColor);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolInOutInChain) {
    std::string fragmentCode(R"(

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

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

// Tests all attributes in Property type.
TEST_F(MaterialCompiler, StaticCodeAnalyzerBaseColor) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(0.8);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerRoughness) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.roughness = 0.8;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::ROUGHNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerMetallic) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.metallic = 0.8;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::METALLIC)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerReflectance) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.reflectance= 0.8;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::REFLECTANCE)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerAmbientOcclusion) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.ambientOcclusion = 0.8;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::AMBIENT_OCCLUSION)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerClearCoat) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.clearCoat = 0.8;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::CLEAR_COAT)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerTransmission) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.absorption = vec3(0.0);
            material.transmission = 0.96;
            material.ior = 1.33;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT, fragmentCode,
            "",
            filamat::MaterialBuilder::Shading::LIT,
            filamat::MaterialBuilder::RefractionMode::CUBEMAP);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::ABSORPTION)] = true;
    expected[size_t(filamat::MaterialBuilder::Property::TRANSMISSION)] = true;
    expected[size_t(filamat::MaterialBuilder::Property::IOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerClearCoatRoughness) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.clearCoatRoughness = 0.8;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT, fragmentCode,
            "");

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::CLEAR_COAT_ROUGHNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerClearCoatNormal) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.clearCoat = 0.8;
            material.clearCoatNormal = vec3(1.0, 1.0, 1.0);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT, fragmentCode,
            "");

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::CLEAR_COAT)] = true;
    expected[size_t(filamat::MaterialBuilder::Property::CLEAR_COAT_NORMAL)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerThickness) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.thickness= 0.8;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT, fragmentCode,
            "",
            filamat::MaterialBuilder::Shading::SUBSURFACE);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::THICKNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSubsurfacePower) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.subsurfacePower = 0.8;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT, fragmentCode,
            "",
            filamat::MaterialBuilder::Shading::SUBSURFACE);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::SUBSURFACE_POWER)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSubsurfaceColor) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.subsurfaceColor= vec3(0.8);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT, fragmentCode,
            "",
            filamat::MaterialBuilder::Shading::SUBSURFACE);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::SUBSURFACE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerAnisotropicDirection) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.anisotropyDirection = vec3(0.8);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::ANISOTROPY_DIRECTION)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerAnisotropic) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.anisotropy = 0.8;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::ANISOTROPY)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSheenColor) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.sheenColor = vec3(0.8);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT, fragmentCode,
            "",
            filamat::MaterialBuilder::Shading::CLOTH);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::SHEEN_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSheenRoughness) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.sheenRoughness = 1.0;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::SHEEN_ROUGHNESS)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerNormal) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.normal= vec3(0.8);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::NORMAL)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerBentNormal) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.bentNormal = vec3(0.8);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::BENT_NORMAL)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerShadowStrength) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.shadowStrength = 0.1;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::SHADOW_STRENGTH)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerOutputFactor) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.postLightingColor = vec4(1.0);
            material.postLightingMixFactor = 0.5;
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::POST_LIGHTING_COLOR)] = true;
    expected[size_t(filamat::MaterialBuilder::Property::POST_LIGHTING_MIX_FACTOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerWithinInclude) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            #include "modify_normal.h"
        }
    )");

    std::string shaderCode = shaderWithAllProperties(*jobSystem, ShaderStage::FRAGMENT,
            fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties{ false };
    glslTools.findProperties(ShaderStage::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected{ false };
    expected[size_t(filamat::MaterialBuilder::Property::NORMAL)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, EmptyName) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    filamat::MaterialBuilder builder;
    builder.material(shaderCode.c_str());
    // The material should compile successfully with an empty name
    builder.name("");
    filamat::Package result = builder.build(*jobSystem);
}

TEST_F(MaterialCompiler, Uv0AndUv1) {
    filamat::MaterialBuilder builder;
    // Requiring both sets of UV coordinates should not fail.
    builder.require(filament::VertexAttribute::UV0);
    builder.require(filament::VertexAttribute::UV1);
    filamat::Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, FiveCustomVariables) {
    filamat::MaterialBuilder builder;
    builder.variable(MaterialBuilder::Variable::CUSTOM0, "custom0");
    builder.variable(MaterialBuilder::Variable::CUSTOM1, "custom1");
    builder.variable(MaterialBuilder::Variable::CUSTOM2, "custom2");
    builder.variable(MaterialBuilder::Variable::CUSTOM3, "custom3");
    builder.variable(MaterialBuilder::Variable::CUSTOM4, "custom4");
    filamat::Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, FourCustomVariablesAndColorAttribute) {
    filamat::MaterialBuilder builder;
    builder.require(filament::VertexAttribute::COLOR);
    builder.variable(MaterialBuilder::Variable::CUSTOM0, "custom0");
    builder.variable(MaterialBuilder::Variable::CUSTOM1, "custom1");
    builder.variable(MaterialBuilder::Variable::CUSTOM2, "custom2");
    builder.variable(MaterialBuilder::Variable::CUSTOM3, "custom3");
    filamat::Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, FiveCustomVariablesAndColorAttributeFails) {
    filamat::MaterialBuilder builder;
    builder.require(filament::VertexAttribute::COLOR);
    builder.variable(MaterialBuilder::Variable::CUSTOM0, "custom0");
    builder.variable(MaterialBuilder::Variable::CUSTOM1, "custom1");
    builder.variable(MaterialBuilder::Variable::CUSTOM2, "custom2");
    builder.variable(MaterialBuilder::Variable::CUSTOM3, "custom3");
    builder.variable(MaterialBuilder::Variable::CUSTOM4, "custom4");
    filamat::Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomVariable4AndColorAttributeFails) {
    filamat::MaterialBuilder builder;
    builder.require(filament::VertexAttribute::COLOR);
    builder.variable(MaterialBuilder::Variable::CUSTOM4, "custom4");
    filamat::Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, Arrays) {
    filamat::MaterialBuilder builder;

    builder.parameter("f4", 1, UniformType::FLOAT4);
    builder.parameter("f1", 1, UniformType::FLOAT);

    filamat::Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, CustomSurfaceShadingRequiresLit) {
    filamat::MaterialBuilder builder;
    builder.customSurfaceShading(true);
    builder.shading(filament::Shading::UNLIT);
    filamat::Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomSurfaceShadingRequiresFunction) {
    filamat::MaterialBuilder builder;
    builder.customSurfaceShading(true);
    builder.shading(filament::Shading::LIT);
    filamat::Package result = builder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialCompiler, CustomSurfaceShadingHasFunction) {
    std::string shaderCode(R"(
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

    filamat::MaterialBuilder builder;
    builder.customSurfaceShading(true);
    builder.shading(filament::Shading::LIT);
    builder.material(shaderCode.c_str());
    filamat::Package result = builder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, ConstantParameter) {
  std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            if (materialConstants_myBoolConstant) {
              material.baseColor.rgb = float3(materialConstants_myFloatConstant);
              int anInt = materialConstants_myIntConstant;
            }
        }
    )");
    std::string vertexCode(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            int anInt = materialConstants_myIntConstant;
            bool aBool = materialConstants_myBoolConstant;
            float aFloat = materialConstants_myFloatConstant;
        }
    )");
  filamat::MaterialBuilder builder;
  builder.constant("myFloatConstant", ConstantType::FLOAT, 1.0f);
  builder.constant("myIntConstant", ConstantType::INT, 123);
  builder.constant("myBoolConstant", ConstantType::BOOL, true);
  builder.constant<bool>("myOtherBoolConstant", ConstantType::BOOL);

  builder.shading(filament::Shading::LIT);
  builder.material(shaderCode.c_str());
  builder.materialVertex(vertexCode.c_str());
  filamat::Package result = builder.build(*jobSystem);
  EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, ConstantParameterSameName) {
#ifdef __EXCEPTIONS
    EXPECT_THROW({
        filamat::MaterialBuilder builder;
        builder.constant("myFloatConstant", ConstantType::FLOAT, 1.0f);
        builder.constant("myFloatConstant", ConstantType::FLOAT, 1.0f);
    }, utils::PostconditionPanic);
#endif
}

TEST_F(MaterialCompiler, ConstantParameterWrongType) {
#ifdef __EXCEPTIONS
    EXPECT_THROW({
        filamat::MaterialBuilder builder;
        builder.constant("myFloatConstant", ConstantType::FLOAT, 10);
    }, utils::PostconditionPanic);
#endif
}

TEST_F(MaterialCompiler, FeatureLevel0Sampler2D) {
  std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = texture2D(materialParams_sampler, vec2(0.0, 0.0));
        }
    )");
  filamat::MaterialBuilder builder;
  builder.parameter("sampler", SamplerType::SAMPLER_2D);

  builder.featureLevel(FeatureLevel::FEATURE_LEVEL_0);
  builder.shading(filament::Shading::UNLIT);
  builder.material(shaderCode.c_str());
  filamat::Package result = builder.build(*jobSystem);
  EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, FeatureLevel0Ess3CallFails) {
  std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = texture(materialParams_sampler, vec3(0.0, 0.0));
        }
    )");
  filamat::MaterialBuilder builder;
  builder.parameter("sampler", SamplerType::SAMPLER_2D);

  builder.featureLevel(FeatureLevel::FEATURE_LEVEL_0);
  builder.shading(filament::Shading::UNLIT);
  builder.material(shaderCode.c_str());
  filamat::Package result = builder.build(*jobSystem);
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
    filamat::MaterialBuilder builder;
    builder.targetApi(filamat::MaterialBuilderBase::TargetApi::WEBGPU);
    builder.material(bakedColorCodeFrag.c_str());
    builder.shading(filamat::MaterialBuilder::Shading::UNLIT);
    builder.name("BakedColor");
    builder.culling(CullingMode::NONE);
    builder.featureLevel(FeatureLevel::FEATURE_LEVEL_0);
    builder.require(filament::VertexAttribute::COLOR);
    builder.variantFilter(static_cast<filament::UserVariantFilterMask>(
            filament::UserVariantFilterBit::SKINNING | filament::UserVariantFilterBit::STE));
    filamat::Package result = builder.build(*jobSystem);
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
    filamat::MaterialBuilder builder;
    builder.targetApi(filamat::MaterialBuilderBase::TargetApi::WEBGPU);
    builder.material(litTransparentCodeFrag.c_str());
    builder.shading(filamat::MaterialBuilder::Shading::LIT);
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
    filamat::Package result = builder.build(*jobSystem);
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
