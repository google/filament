/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "filamat/MaterialBuilder.h"

#include "Includes.h"

#include "MockIncluder.h"

#include <utils/CString.h>

// -------------------------------------------------------------------------------------------------

TEST(IncludeParser, NoIncludes) {
    utils::CString code("// no includes");
    std::vector<filamat::FoundInclude> result = filamat::parseForIncludes(code);
    EXPECT_TRUE(result.size() == 0);
}

TEST(IncludeParser, SingleInclude) {
    utils::CString code(R"(#include "foobar.h")");
    auto result = filamat::parseForIncludes(code);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].name, "foobar.h");
    EXPECT_EQ(result[0].startPosition, 0);
    EXPECT_EQ(result[0].length, 19);
}

TEST(IncludeParser, MultipleIncludes) {
    utils::CString code("#include \"foobar.h\"\n#include \"bazbarfoo.h\"");
    auto result = filamat::parseForIncludes(code);

    EXPECT_EQ(2, result.size());
    EXPECT_STREQ("foobar.h", result[0].name.c_str());
    EXPECT_EQ(0, result[0].startPosition);
    EXPECT_EQ(19, result[0].length);

    EXPECT_STREQ("bazbarfoo.h", result[1].name.c_str());
    EXPECT_EQ(20, result[1].startPosition);
    EXPECT_EQ(22, result[1].length);
}

TEST(IncludeParser, EmptyInclude) {
    utils::CString code("#include \"\"");
    auto result = filamat::parseForIncludes(code);

    EXPECT_EQ(1, result.size());
    EXPECT_STREQ("", result[0].name.c_str_safe());
    EXPECT_EQ(0, result[0].startPosition);
    EXPECT_EQ(11, result[0].length);
}

TEST(IncludeParser, Whitepsace) {
    utils::CString code("  #include      \"foobarbaz.h\"");
    auto result = filamat::parseForIncludes(code);

    EXPECT_EQ(1, result.size());
    EXPECT_STREQ("foobarbaz.h", result[0].name.c_str());
    EXPECT_EQ(2, result[0].startPosition);
    EXPECT_EQ(27, result[0].length);
}

TEST(IncludeParser, InvalidIncludes) {
    utils::CString code("#include");
    auto result = filamat::parseForIncludes(code);
    EXPECT_EQ(0, result.size());
}

TEST(IncludeParser, InvalidWithValidInclude) {
    {
        utils::CString code("#include #include \"foobar.h\"");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(1, result.size());
        EXPECT_STREQ("foobar.h", result[0].name.c_str());
        EXPECT_EQ(9, result[0].startPosition);
        EXPECT_EQ(19, result[0].length);
    }

    {
        utils::CString code("abcdefghi #include#include#include\"foo.h\"");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(1, result.size());
        EXPECT_STREQ("foo.h", result[0].name.c_str());
        EXPECT_EQ(26, result[0].startPosition);
        EXPECT_EQ(15, result[0].length);
    }
}

// -------------------------------------------------------------------------------------------------

TEST(IncludeResolver, NoIncludes) {
    utils::CString code("no includes");
    MockIncluder includer;
    bool result = filamat::resolveIncludes(utils::CString(""), code, includer);
    EXPECT_TRUE(result);
    EXPECT_STREQ("no includes", code.c_str());
}

TEST(IncludeResolver, SingleInclude) {
    utils::CString code("#include \"test.h\"");
    MockIncluder includer;
    includer
        .sourceForInclude("test.h", "include");
    bool result = filamat::resolveIncludes(utils::CString(""), code, includer);
    EXPECT_TRUE(result);
    EXPECT_STREQ("include", code.c_str());
}

TEST(IncludeResolver, MultipleIncludes) {
    utils::CString code(R"(
        #include "one.h"
        #include "two.h"
        #include "three.h"
    )");

    MockIncluder includer;
    includer
        .sourceForInclude("one.h", "1")
        .sourceForInclude("two.h", "2")
        .sourceForInclude("three.h", "3");

    bool result = filamat::resolveIncludes(utils::CString(""), code, includer);
    EXPECT_TRUE(result);
    EXPECT_STREQ(utils::CString(R"(
        1
        2
        3
    )").c_str(), code.c_str());
}

TEST(IncludeResolver, IncludeWithinInclude) {
    utils::CString code(R"(
        #include "one.h"
        #include "two.h"
    )");

    MockIncluder includer;
    includer
        .sourceForInclude("one.h", "1")
        .sourceForInclude("two.h", "#include \"three.h\"")
        .sourceForInclude("three.h", "3")
        .expectIncludeIncludedBy("three.h", "two.h");


    bool result = filamat::resolveIncludes(utils::CString(""), code, includer);
    EXPECT_TRUE(result);
    EXPECT_STREQ(utils::CString(R"(
        1
        3
    )").c_str(), code.c_str());
}

TEST(IncludeResolver, Includers) {
    utils::CString code(R"(
        #include "dir/one.h"
    )");

    MockIncluder includer;
    includer
        .sourceForInclude("dir/one.h", "#include \"two.h\"")
        .sourceForInclude("two.h", "#include \"three.h\"")
        .sourceForInclude("three.h", "3")
        .expectIncludeIncludedBy("two.h", "dir/one.h");

    bool result = filamat::resolveIncludes(utils::CString(""), code, includer);
    EXPECT_TRUE(result);
    EXPECT_STREQ(utils::CString(R"(
        3
    )").c_str(), code.c_str());
}

TEST(IncludeResolver, IncludeFailure) {
    {
        utils::CString code(R"(
            #include "one.h"
        )");

        MockIncluder includer;
        includer
            .sourceForInclude("one.h", "#include \"two.h\"");

        bool result = filamat::resolveIncludes(utils::CString(""), code, includer);
        EXPECT_FALSE(result);
    }
    {
        utils::CString code(R"(
            #include "one.h"
        )");

        MockIncluder includer;

        bool result = filamat::resolveIncludes(utils::CString(""), code, includer);
        EXPECT_FALSE(result);
    }
}

TEST(IncludeResolver, Cycle) {
    utils::CString code(R"(
        #include "foo.h"
    )");

    MockIncluder includer;
    includer
        .sourceForInclude("foo.h", "#include \"bar.h\"")
        .sourceForInclude("bar.h", "#include \"foo.h\"");

    bool result = filamat::resolveIncludes(utils::CString(""), code, includer);
    // Include cycles are disallowed. We should still terminate in finite time and report false.
    EXPECT_FALSE(result);
}

// -------------------------------------------------------------------------------------------------

#include <utils/Log.h>

class MaterialBuilder : public ::testing::Test {
protected:

    filamat::MaterialBuilder mBuilder;

    MaterialBuilder() {
        filamat::MaterialBuilder::init();
        mBuilder.optimization(filamat::MaterialBuilder::Optimization::NONE);
    }

    virtual ~MaterialBuilder() {
        filamat::MaterialBuilder::shutdown();
    }
};

TEST_F(MaterialBuilder, NoIncluder) {
    std::string shaderCode(R"(
        #include "test.h"
    )");
    mBuilder.material(shaderCode.c_str());

    filamat::Package result = mBuilder.build();

    // Shader code with an include should fail to compile if no includer is specified.
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialBuilder, Include) {
    std::string shaderCode(R"(
        #include "test.h"
    )");
    mBuilder.material(shaderCode.c_str());

    MockIncluder includer;
    includer.sourceForInclude("test.h", R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
        }
    )");
    mBuilder.includeCallback(includer);

    filamat::Package result = mBuilder.build();
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialBuilder, IncludeVertex) {
    std::string shaderCode(R"(
        #include "test.h"
        void materialVertex(inout MaterialVertexInputs material) {
            functionDefinedInTest();
        }
    )");
    mBuilder.materialVertex(shaderCode.c_str());

    MockIncluder includer;
    includer.sourceForInclude("test.h", R"(
        void functionDefinedInTest() {

        }
    )");
    mBuilder.includeCallback(includer);

    filamat::Package result = mBuilder.build();
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialBuilder, IncludeWithinFunction) {
    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            #include "prepare.h"
        }
    )");
    mBuilder.material(shaderCode.c_str());

    MockIncluder includer;
    includer
        .sourceForInclude("prepare.h", R"(
                prepareMaterial(material);
        )");

    mBuilder.includeCallback(includer);

    filamat::Package result = mBuilder.build();
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialBuilder, IncludeFailure) {
    std::string shaderCode(R"(
        #include "nonexistent.h"
    )");
    mBuilder.material(shaderCode.c_str());

    MockIncluder includer;
    mBuilder.includeCallback(includer);

    filamat::Package result = mBuilder.build();
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialBuilder, NoShaderCode) {
    filamat::Package result = mBuilder.build();
    EXPECT_TRUE(result.isValid());
}
