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

#include "sca/GLSLToolsLite.h"

#include <filamat/MaterialBuilder.h>
#include <filamat/Enums.h>

using namespace filamat;

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

class FilamatLite : public ::testing::Test {
protected:
    FilamatLite() {
    }

    virtual ~FilamatLite() {
    }

    virtual void SetUp() {
        MaterialBuilder::init();
    }
};

TEST_F(FilamatLite, StaticCodeAnalyzerNothingDetected) {
    utils::CString shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerNothingDetectedinVertex) {
    utils::CString shaderCode(R"(
        void materialVertex(inout MaterialVertexInputs material) {
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::VERTEX, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerDirectAssign) {
    utils::CString shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(0.8);
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerDirectAssignVertex) {
    utils::CString shaderCode(R"(
        void materialVertex(inout MaterialVertexInputs material) {
            material.clipSpaceTransform = mat4(2.0);
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::VERTEX, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(MaterialBuilder::Property::CLIP_SPACE_TRANSFORM)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerAssignMultiple) {
    utils::CString shaderCode(R"(
        void material(inout MaterialInputs material) {
            material.clearCoat = 1.0;
            prepareMaterial(material);
            material.baseColor = vec4(0.8);
            material.metallic = 1.0;
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(MaterialBuilder::Property::CLEAR_COAT)] = true;
    expected[size_t(MaterialBuilder::Property::BASE_COLOR)] = true;
    expected[size_t(MaterialBuilder::Property::METALLIC)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerDirectAssignWithSwizzling) {
    utils::CString shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.subsurfaceColor.rgb = vec3(1.0, 0.4, 0.8);
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(MaterialBuilder::Property::SUBSURFACE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerNoSpace) {
    utils::CString shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.ambientOcclusion=vec3(1.0);
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(MaterialBuilder::Property::AMBIENT_OCCLUSION)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerWhitespace) {
    utils::CString shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material  .subsurfaceColor = vec3(1.0);
            material    .   ambientOcclusion  = vec3(1.0);
            material
                .   baseColor  = vec3(1.0);
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(MaterialBuilder::Property::SUBSURFACE_COLOR)] = true;
    expected[size_t(MaterialBuilder::Property::AMBIENT_OCCLUSION)] = true;
    expected[size_t(MaterialBuilder::Property::BASE_COLOR)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerEndOfShader) {
    utils::CString shaderCode(R"(
        void material(inout MaterialInputs material) {
            material.)");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerSlashComments) {
    utils::CString shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.metallic = 1.0;
            // material.baseColor = vec4(1.0); // material.baseColor = vec4(1.0);
            // material.ambientOcclusion = vec3(1.0);
            material.clearCoat = 0.5;
            material.anisotropy = -1.0;
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(MaterialBuilder::Property::METALLIC)] = true;
    expected[size_t(MaterialBuilder::Property::CLEAR_COAT)] = true;
    expected[size_t(MaterialBuilder::Property::ANISOTROPY)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, StaticCodeAnalyzerMultilineComments) {
    utils::CString shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.metallic = 1.0;
            /*
            material.baseColor = vec4(1.0); // material.baseColor = vec4(1.0);
            material.ambientOcclusion = vec3(1.0);
            */
            material.clearCoat = 0.5;
        }
    )");

    GLSLToolsLite glslTools;
    MaterialBuilder::PropertyList properties {false};
    glslTools.findProperties(filament::backend::FRAGMENT, shaderCode, properties);
    MaterialBuilder::PropertyList expected {false};
    expected[size_t(MaterialBuilder::Property::METALLIC)] = true;
    expected[size_t(MaterialBuilder::Property::CLEAR_COAT)] = true;
    EXPECT_TRUE(PropertyListsMatch(expected, properties));
}

TEST_F(FilamatLite, RemoveLineDirectivesOneLine) {
    {
        std::string shaderCode("#line 10 \"foobar\"");
        GLSLToolsLite glslTools;
        glslTools.removeGoogleLineDirectives(shaderCode);
        EXPECT_STREQ("", shaderCode.c_str());
    }
    {
        // Ignore non-Google extension line directives
        std::string shaderCode("#line 100");
        GLSLToolsLite glslTools;
        glslTools.removeGoogleLineDirectives(shaderCode);
        EXPECT_STREQ("#line 100", shaderCode.c_str());
    }
}

TEST_F(FilamatLite, RemoveLineDirectives) {
    std::string shaderCode(R"(
aaa
#line 10 "foobar"
bbb
ccc
#line 100
    )");

    std::string expected(R"(
aaa
bbb
ccc
#line 100
    )");

    GLSLToolsLite glslTools;
    glslTools.removeGoogleLineDirectives(shaderCode);
    EXPECT_STREQ(expected.c_str(), shaderCode.c_str());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
