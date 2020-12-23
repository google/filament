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
#include "shaders/ShaderGenerator.h"

#include "MockIncluder.h"

#include <filamat/Enums.h>

using namespace ASTUtils;
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

std::string shaderWithAllProperties(ShaderType type,
        const std::string fragmentCode, const std::string vertexCode = "",
        filamat::MaterialBuilder::Shading shadingModel = filamat::MaterialBuilder::Shading::LIT,
        filamat::MaterialBuilder::RefractionMode refractionMode = filamat::MaterialBuilder::RefractionMode::NONE) {
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

    MaterialBuilder::PropertyList allProperties;
    std::fill_n(allProperties, MaterialBuilder::MATERIAL_PROPERTIES_COUNT, true);

    // We need to "build" the material to resolve any includes in user code.
    builder.build();

    return builder.peek(type,
            {1, MaterialBuilder::TargetApi::OPENGL, MaterialBuilder::TargetLanguage::GLSL},
            allProperties);
}

TEST(StaticCodeAnalysisHelper, getFunctionName) {
    std::string name = getFunctionName("main(");
    EXPECT_EQ(name, "main");
}

TEST(StaticCodeAnalysisHelper, getFunctionNameNoParenthesis) {
    std::string name = getFunctionName("main");
    EXPECT_EQ(name, "main");
}

class MaterialCompiler : public ::testing::Test {
protected:
    MaterialCompiler() {
    }

    virtual ~MaterialCompiler() {
    }

    virtual void SetUp() {
        MaterialBuilder::init();
    }
};

TEST_F(MaterialCompiler, StaticCodeAnalyzerNothingDetected) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::VERTEX, fragmentCode, vertexCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::VERTEX, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerDirectAssign) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(0.8);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::VERTEX, fragmentCode, vertexCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::VERTEX, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode, "",
            filamat::MaterialBuilder::Shading::LIT,
            filamat::MaterialBuilder::RefractionMode::CUBEMAP);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode, "");

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode, "");

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode, "",
            filamat::MaterialBuilder::Shading::SUBSURFACE);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode, "",
            filamat::MaterialBuilder::Shading::SUBSURFACE);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode, "",
            filamat::MaterialBuilder::Shading::SUBSURFACE);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode, "",
            filamat::MaterialBuilder::Shading::CLOTH);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(filamat::MaterialBuilder::Property::BENT_NORMAL)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerOutputFactor) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.postLightingColor = vec4(1.0);
        }
    )");

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(filamat::MaterialBuilder::Property::POST_LIGHTING_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerWithinInclude) {
    std::string fragmentCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            #include "modify_normal.h"
        }
    )");

    std::string shaderCode = shaderWithAllProperties(ShaderType::FRAGMENT, fragmentCode);

    GLSLTools glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
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
    filamat::Package result = builder.build();
}

TEST_F(MaterialCompiler, Uv0AndUv1) {
    filamat::MaterialBuilder builder;
    // Requiring both sets of UV coordinates should not fail.
    builder.require(filament::VertexAttribute::UV0);
    builder.require(filament::VertexAttribute::UV1);
    filamat::Package result = builder.build();
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialCompiler, Arrays) {
    filamat::MaterialBuilder builder;

    builder.parameter(UniformType::FLOAT4, 1, "f4");
    builder.parameter(UniformType::FLOAT, 1, "f1");

    filamat::Package result = builder.build();
    EXPECT_TRUE(result.isValid());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
