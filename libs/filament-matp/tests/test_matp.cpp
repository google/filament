/*
* Copyright (C) 2025 The Android Open Source Project
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

#include "TestMaterialParser.h"

#include <filament-matp/MaterialParser.h>
#include "../src/JsonishLexer.h"
#include "../src/JsonishParser.h"
#include "../src/MaterialLexer.h"

class MaterialLexer: public ::testing::Test {
protected:
    MaterialLexer() = default;
    ~MaterialLexer() override = default;
};

static std::string materialSource(R"(
    material {
        name : "Filament Default Material",
        shadingModel : unlit
    }

    fragment {
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor.rgb = vec3(0.8);
        }
    }
)");

static std::string materialSourceWithTool(R"(
    material {
        name : "Filament Default Material",
        shadingModel : unlit
    }

    fragment {
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor.rgb = vec3(0.8);
        }
    }

    tool {
        all of this should be ignored
        matching braces are fine
        {  } { { } } { }
    }
)");

static std::string materialSourceWithCommentedBraces(R"(
    material {
        name : "Filament Default Material",
        shadingModel : unlit
    }

    fragment {
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            // if (test) {
            material.baseColor.rgb = vec3(0.8); // if (test) {
        }
    }
)");

TEST_F(MaterialLexer, NormalMaterialLexing) {
    matp::MaterialLexer materialLexer;
    materialLexer.lex(materialSource.c_str(), materialSource.size(), 1);
    auto lexemes = materialLexer.getLexemes();
    // A material grammar is made of identifiers and blocks. The source provided has one identifier,
    // followed by block,identifier, and block. Total expected is four.
    EXPECT_EQ(lexemes.size(), 4);
}

TEST_F(MaterialLexer, MaterialParser) {
    matp::MaterialParser parser;
    TestMaterialParser testParser(parser);
    filamat::MaterialBuilder unused;
    bool result = testParser.parseMaterial(materialSource.c_str(), materialSource.size(), unused);
    EXPECT_EQ(result, true);
}

TEST_F(MaterialLexer, NoSpaceBetweenBlockAndIdentifier) {
    matp::MaterialLexer materialLexer;
    static std::string source(R"(
        material {}fragment {}
    )");
    materialLexer.lex(source.c_str(), source.size(), 1);
    auto lexemes = materialLexer.getLexemes();
    EXPECT_EQ(lexemes.size(), 4);
}

TEST_F(MaterialLexer, MaterialParserWithToolSection) {
    matp::MaterialParser parser;
    TestMaterialParser testParser(parser);
    filamat::MaterialBuilder unused;
    bool result = testParser.parseMaterial(materialSourceWithTool.c_str(), materialSourceWithTool.size(), unused);
    EXPECT_EQ(result, true);
}

TEST_F(MaterialLexer, MaterialParserWithCommentedBraces) {
    matp::MaterialParser parser;
    TestMaterialParser testParser(parser);
    filamat::MaterialBuilder unused;
    bool result = testParser.parseMaterial(materialSourceWithCommentedBraces.c_str(), materialSourceWithCommentedBraces.size(), unused);
    EXPECT_EQ(result, true);
}

TEST_F(MaterialLexer, MaterialParserErrorOnlyIdentifier) {
    std::string sourceMissingIdentifier(R"(
        singleIdentifier
    )");
    matp::MaterialParser parser;
    TestMaterialParser testParser(parser);
    filamat::MaterialBuilder unused;
    bool result = testParser.parseMaterial(
            sourceMissingIdentifier.c_str(), sourceMissingIdentifier.size(), unused);
    EXPECT_EQ(result, false);
}

TEST_F(MaterialLexer, MaterialParserErrorMissingBlock) {
    std::string sourceMissingBlock(R"(
        identifier1 {}
        identifier1
    )");
    matp::MaterialParser parser;
    TestMaterialParser testParser(parser);
    filamat::MaterialBuilder unused;
    bool result = testParser.parseMaterial(
            sourceMissingBlock.c_str(), sourceMissingBlock.size(), unused);
    EXPECT_EQ(result, false);
}

TEST_F(MaterialLexer, MaterialParserErrorTwoBlock) {
    std::string sourceTwoBlock(R"(
        identifier1 {} {}
    )");
    matp::MaterialParser parser;
    TestMaterialParser testParser(parser);
    filamat::MaterialBuilder unused;
    bool result = testParser.parseMaterial(sourceTwoBlock.c_str(), sourceTwoBlock.size(), unused);
    EXPECT_EQ(result, false);
}

TEST_F(MaterialLexer, MaterialParserSyntaxError) {
    std::string sourceSyntaxError(R"(
        identifier1 {} # identified2 {}
    )");
    matp::MaterialParser parser;
    TestMaterialParser testParser(parser);
    filamat::MaterialBuilder unused;
    bool result = testParser.parseMaterial(sourceSyntaxError.c_str(), sourceSyntaxError.size(), unused);
    EXPECT_EQ(result, false);
}

static std::string jsonMaterialSource(R"(
{
    "fragment": "void material(inout MaterialInputs material) {
        vec3 normal = vec3(0.0, 0.0, 1.0);
        material.normal = normal;
        prepareMaterial(material);
        material.baseColor = vec4(1.0, 1.0, 1.0, 1.0) * materialParams.baseColorFactor;
        material.metallic = materialParams.metallicFactor;
        material.roughness = materialParams.roughnessFactor;
        material.ambientOcclusion = 1.0;
        material.emissive = vec4(0.0);
        material.reflectance = 0.5;
        //  material.baseColor.xyz = normal;\n}",
    "material": {
        "blending": "opaque",
        "name": "Gltf 2 Metallic-Roughness Material",
        "parameters": [
            {
            "name": "baseColorFactor",
            "type": "float4"
            },
            {
            "name": "metallicFactor",
            "type": "float"
            },
            {
            "name": "roughnessFactor",
            "type": "float"
            },
            {
            "name": "emissiveFactor",
            "type": "float4"
            },
            {
            "name": "weights2d",
            "type": "float[9]"
            }
        ],
        "requires": [
           "position",
           "tangents",
           "uv0"
        ],
        "shadingModel": "lit"
    }
}
)");

static std::string jsonMaterialSourceNoQuotes(R"({

material: {
    blending: opaque,
    name: albertCamus,
    parameters: [
        {
            name: baseColorFactor,
            type: float4
        },
        {
            name: metallicFactor,
            type: float
        },
        {
            name: roughnessFactor,
            type: float
        },
        {
            name: emissiveFactor,
            type: float4
        },
        {
            name: weights2d,
            type: float[9]
        }
    ],
    requires: [
        position,
        tangents,
        uv0
    ],
    shadingModel: lit
}

})");

TEST_F(MaterialLexer, JsonMaterialLexingAndParsing) {
    matp::JsonishLexer jsonishLexer;
    jsonishLexer.lex(jsonMaterialSource.c_str(), jsonMaterialSource.size(), 1);
    auto lexemes = jsonishLexer.getLexemes();

    EXPECT_TRUE(!lexemes.empty());

    matp::JsonishParser jsonishParser(lexemes);
    std::unique_ptr<matp::JsonishObject> root = jsonishParser.parse();

    auto entries = root->getEntries();
    EXPECT_TRUE(entries.size() == 2);
    EXPECT_TRUE(entries.find("fragment") != entries.end());
    EXPECT_TRUE(entries.find("material") != entries.end());
}

TEST_F(MaterialLexer, JsonMaterialLexingAndParsingNoQuotes) {
    matp::JsonishLexer jsonishLexer;
    jsonishLexer.lex(jsonMaterialSourceNoQuotes.c_str(), jsonMaterialSourceNoQuotes.size(), 1);
    auto lexemes = jsonishLexer.getLexemes();

    EXPECT_TRUE(!lexemes.empty());

    matp::JsonishParser jsonishParser(lexemes);
    std::unique_ptr<matp::JsonishObject> root = jsonishParser.parse();

    auto entries = root->getEntries();
    EXPECT_TRUE(entries.size() == 1);
    EXPECT_TRUE(entries.find("material") != entries.end());
}

TEST_F(MaterialLexer, JsonMaterialParser) {
    matp::MaterialParser parser;
    TestMaterialParser testParser(parser);
    filamat::MaterialBuilder unused;
    bool result = testParser.parseMaterialAsJSON(jsonMaterialSource.c_str(), jsonMaterialSource.size(), unused);
    EXPECT_EQ(result, true);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
