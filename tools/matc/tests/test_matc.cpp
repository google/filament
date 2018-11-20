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

#include "MockConfig.h"

#include <filamat/sca/ASTHelpers.h>
#include <matc/MaterialLexer.h>

using namespace ASTUtils;

filamat::MaterialBuilder makeBuilder(const std::string shaderCode) {
    filamat::MaterialBuilder builder;
    builder.material(shaderCode.c_str());
    builder.platform(filamat::MaterialBuilder::Platform::MOBILE);
    return std::move(builder);
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
        GLSLTools::init();
    }

    virtual void TearDown() {
        GLSLTools::terminate();
    }
};

TEST_F(MaterialCompiler, StaticCodeAnalyzerNothingDetected) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    EXPECT_EQ(expected, properties);
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerNotFollowingINParameters) {
    std::string shaderCode(R"(
        void notAffectingInput(in MaterialInputs material) {
            material.baseColor = vec4(0.8);
        }
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            notAffectingInput(material);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    EXPECT_EQ(expected, properties);
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerDirectAssign) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(0.8);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::BASE_COLOR);
    EXPECT_EQ(expected, properties);
}


TEST_F(MaterialCompiler, StaticCodeAnalyzerDirectAssignWithSwizzling) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor.rgb = vec3(0.8);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::BASE_COLOR);
    EXPECT_EQ(expected, properties);
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolAsOutParameterWithAliasing) {
    std::string shaderCode(R"(

        void setBaseColor(inout vec4 aliasBaseColor) {
            aliasBaseColor = vec4(0.8,0.1,0.2,1.0);
        }

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            setBaseColor(material.baseColor);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::BASE_COLOR);
    EXPECT_EQ(expected, properties);
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolAsOutParameterWithAliasingAndSwizzling) {
    std::string shaderCode(R"(

        void setBaseColor(inout float aliasedRed) {
            aliasedRed = 0.8;
        }

        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            setBaseColor(material.baseColor.r);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::BASE_COLOR);
    EXPECT_EQ(expected, properties);
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolInOutInChainWithDirectIndexIntoStruct) {
    std::string shaderCode(R"(

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

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::BASE_COLOR);
    EXPECT_EQ(expected, properties);
}

TEST_F(MaterialCompiler, StaticCodeAnalyzerSymbolInOutInChain) {
    std::string shaderCode(R"(

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

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::BASE_COLOR);
    EXPECT_EQ(expected, properties);
}

// Tests all attributes in Property type.
TEST_F(MaterialCompiler, StaticCodeAnalyzerBaseColor) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(0.8);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::BASE_COLOR);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerRoughness) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.roughness = 0.8;
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::ROUGHNESS);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerMetallic) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.metallic = 0.8;
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::METALLIC);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerReflectance) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.reflectance= 0.8;
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::REFLECTANCE);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerAmbientOcclusion) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.ambientOcclusion = 0.8;
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::AMBIENT_OCCLUSION);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerClearCoat) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.clearCoat = 0.8;
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    builder.shading(filamat::MaterialBuilder::Shading::LIT);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::CLEAR_COAT);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerClearCoatRoughness) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.clearCoatRoughness = 0.8;
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    builder.shading(filamat::MaterialBuilder::Shading::LIT);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::CLEAR_COAT_ROUGHNESS);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerClearCoatNormal) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.clearCoatNormal = vec3(1.0, 1.0, 1.0);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    builder.shading(filamat::MaterialBuilder::Shading::LIT);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::CLEAR_COAT_NORMAL);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerThickness) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.thickness= 0.8;
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    builder.shading(filamat::MaterialBuilder::Shading::SUBSURFACE);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::THICKNESS);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerSubsurfacePower) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.subsurfacePower = 0.8;
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    builder.shading(filamat::MaterialBuilder::Shading::SUBSURFACE);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::SUBSURFACE_POWER);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerSubsurfaceColor) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.subsurfaceColor= vec3(0.8);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    builder.shading(filamat::MaterialBuilder::Shading::SUBSURFACE);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::SUBSURFACE_COLOR);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerAnisotropicDirection) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.anisotropyDirection = vec3(0.8);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    builder.shading(filamat::MaterialBuilder::Shading::LIT);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::ANISOTROPY_DIRECTION);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerAnisotropic) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.anisotropy = 0.8;
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    builder.shading(filamat::MaterialBuilder::Shading::LIT);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::ANISOTROPY);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerSheenColor) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.sheenColor = vec3(0.8);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    builder.shading(filamat::MaterialBuilder::Shading::CLOTH);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::SHEEN_COLOR);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, StaticCodeAnalyzerNormal) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.normal= vec3(0.8);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    GLSLTools glslTools;
    GLSLTools::PropertySet properties;
    glslTools.findProperties(builder, properties);
    GLSLTools::PropertySet expected;
    expected.insert(filamat::MaterialBuilder::Property::NORMAL);
    EXPECT_EQ(expected, properties);
}
TEST_F(MaterialCompiler, EmptyName) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    filamat::MaterialBuilder builder = makeBuilder(shaderCode);
    // The material should compile successfully with an empty name
    builder.name("");
    filamat::Package result = builder.build();
}
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
