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
#include <utils/JobSystem.h>

#include <memory>

using namespace utils;

// -------------------------------------------------------------------------------------------------

TEST(IncludeParser, NoIncludes) {
    utils::CString code("// no includes");
    std::vector<filamat::FoundInclude> result = filamat::parseForIncludes(code);
    EXPECT_TRUE(result.size() == 0);
}

TEST(IncludeParser, SingleInclude) {
    utils::CString code(R"(#include "foobar.h")");
    auto result = filamat::parseForIncludes(code);
    EXPECT_EQ(1, result.size());
    EXPECT_STREQ("foobar.h", result[0].name.c_str());
    EXPECT_EQ(0, result[0].startPosition);
    EXPECT_EQ(19, result[0].length);
    EXPECT_EQ(1, result[0].line);
}

TEST(IncludeParser, MultipleIncludes) {
    utils::CString code("#include \"foobar.h\"\n#include \"bazbarfoo.h\"");
    auto result = filamat::parseForIncludes(code);

    EXPECT_EQ(2, result.size());
    EXPECT_STREQ("foobar.h", result[0].name.c_str());
    EXPECT_EQ(0, result[0].startPosition);
    EXPECT_EQ(19, result[0].length);
    EXPECT_EQ(1, result[0].line);

    EXPECT_STREQ("bazbarfoo.h", result[1].name.c_str());
    EXPECT_EQ(20, result[1].startPosition);
    EXPECT_EQ(22, result[1].length);
    EXPECT_EQ(2, result[1].line);
}

TEST(IncludeParser, EmptyInclude) {
    utils::CString code("#include \"\"");
    auto result = filamat::parseForIncludes(code);

    EXPECT_EQ(1, result.size());
    EXPECT_STREQ("", result[0].name.c_str_safe());
    EXPECT_EQ(0, result[0].startPosition);
    EXPECT_EQ(11, result[0].length);
    EXPECT_EQ(1, result[0].line);
}

TEST(IncludeParser, Whitepsace) {
    utils::CString code("  #include      \"foobarbaz.h\"");
    auto result = filamat::parseForIncludes(code);

    EXPECT_EQ(1, result.size());
    EXPECT_STREQ("foobarbaz.h", result[0].name.c_str());
    EXPECT_EQ(2, result[0].startPosition);
    EXPECT_EQ(27, result[0].length);
    EXPECT_EQ(1, result[0].line);
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
        EXPECT_EQ(1, result[0].line);
    }

    {
        utils::CString code("abcdefghi #include#include#include\"foo.h\"");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(1, result.size());
        EXPECT_STREQ("foo.h", result[0].name.c_str());
        EXPECT_EQ(26, result[0].startPosition);
        EXPECT_EQ(15, result[0].length);
        EXPECT_EQ(1, result[0].line);
    }
}

TEST(IncludeParser, LineNumbers) {
        utils::CString code("#include \"one.h\"\n#include \"two.h\"\n\n#include \"four.h\"");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(3, result.size());
        EXPECT_EQ(1, result[0].line);
        EXPECT_EQ(2, result[1].line);
        EXPECT_EQ(4, result[2].line);
}

TEST(IncludeParser, IncludeWithinStarComment) {
    {
        utils::CString code(R"(/* #include "foobar.h" */)");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(0, result.size());
    }
    {
        utils::CString code(R"(/* */ /*#include "foobar.h"*/)");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(0, result.size());
    }
    {
        utils::CString code(R"(/**/ /* /* #include "foobar.h" */ */)");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(0, result.size());
    }
    {
        utils::CString code(R"(/*#include "foobar.h")");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(0, result.size());
    }
    {
        utils::CString code(R"(/*   */ #include "foobar.h")");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(1, result.size());
        EXPECT_STREQ("foobar.h", result[0].name.c_str());
        EXPECT_EQ(8, result[0].startPosition);
        EXPECT_EQ(19, result[0].length);
        EXPECT_EQ(1, result[0].line);
    }
}

TEST(IncludeParser, IncludeWithinSlashComment) {
    {
        utils::CString code(R"(// #include "foobar.h")");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(0, result.size());
    }
    {
        utils::CString code(R"(
        //
        // #include "foobar.h"
        )");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(0, result.size());
    }
    {
        utils::CString code(R"(
        //
        // // #include "foobar.h"

        )");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(0, result.size());
    }
    {
        utils::CString code("// comment\n#include \"foobar.h\"");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(1, result.size());
        EXPECT_STREQ("foobar.h", result[0].name.c_str());
        EXPECT_EQ(11, result[0].startPosition);
        EXPECT_EQ(19, result[0].length);
        EXPECT_EQ(2, result[0].line);
    }
}

TEST(IncludeParser, IncludeWithinBothSlashStarComments) {
    {
        utils::CString code(R"(
        // none of these are valid includes:
        // #include "foobar.h"
        /* #include "foobar.h" */
        /* // #include "foobar.h" */
        // /* #include "foobar.h" */
        /* #include "foobar.h"
        )");
        auto result = filamat::parseForIncludes(code);
        EXPECT_EQ(0, result.size());
    }
}

// -------------------------------------------------------------------------------------------------

TEST(IncludeResolver, NoIncludes) {
    utils::CString code("no includes");
    MockIncluder includer;
    filamat::IncludeResult source {
        .text = code
    };
    bool result = filamat::resolveIncludes(source, includer, {});
    EXPECT_TRUE(result);
    EXPECT_STREQ("no includes", source.text.c_str());
}

TEST(IncludeResolver, SingleInclude) {
    utils::CString code("#include \"test.h\"");
    MockIncluder includer;
    includer
        .sourceForInclude("test.h", "include");
    filamat::IncludeResult source {
        .text = code
    };
    bool result = filamat::resolveIncludes(source, includer, {});
    EXPECT_TRUE(result);
    EXPECT_STREQ("include", source.text.c_str());
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

    filamat::IncludeResult source {
        .text = code
    };
    bool result = filamat::resolveIncludes(source, includer, {});
    EXPECT_TRUE(result);
    EXPECT_STREQ(utils::CString(R"(
        1
        2
        3
    )").c_str(), source.text.c_str());
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

    filamat::IncludeResult source {
        .text = code
    };
    bool result = filamat::resolveIncludes(source, includer, {});
    EXPECT_TRUE(result);
    EXPECT_STREQ(utils::CString(R"(
        1
        3
    )").c_str(), source.text.c_str());
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

    filamat::IncludeResult source {
        .text = code
    };
    bool result = filamat::resolveIncludes(source, includer, {});
    EXPECT_TRUE(result);
    EXPECT_STREQ(utils::CString(R"(
        3
    )").c_str(), source.text.c_str());
}

TEST(IncludeResolver, IncludeFailure) {
    {
        utils::CString code(R"(
            #include "one.h"
        )");

        MockIncluder includer;
        includer
            .sourceForInclude("one.h", "#include \"two.h\"");

        filamat::IncludeResult source {
            .text = code
        };
        bool result = filamat::resolveIncludes(source, includer, {});
        EXPECT_FALSE(result);
    }
    {
        utils::CString code(R"(
            #include "one.h"
        )");

        MockIncluder includer;

        filamat::IncludeResult source {
            .text = code
        };
        bool result = filamat::resolveIncludes(source, includer, {});
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

    filamat::IncludeResult source {
        .text = code
    };
    bool result = filamat::resolveIncludes(source, includer, {});
    // Include cycles are disallowed. We should still terminate in finite time and report false.
    EXPECT_FALSE(result);
}

// Helper function that lets us write comparison cases with line breaks.
void EXPECT_STREQ_TRIMMARGIN(const char* expected, const char* actual) {
    size_t len = strlen(expected);
    char* trimmed = (char*) malloc(len);

    const char* end = expected + len;
    const char* e = expected;
    char* t = trimmed;

    while (e < end) {
        while (e < end) {
            if (*e == '|') {
                e += 2; // eat | and space
                break;
            }
            e++;
        }
        while (e < end) {
            if (*e == '\n') {
                *t++ = *e++;
                break;
            }
            *t++ = *e++;
        }
    }

    *t++ = 0;

    EXPECT_STREQ(trimmed, actual);

    free(trimmed);
}

TEST(IncludeResolver, SingleIncludeLineDirective) {
    utils::CString code("#include \"test.h\"");
    MockIncluder includer;
    includer
        .sourceForInclude("test.h", "include");
    filamat::ResolveOptions options = {
        .insertLineDirectives = true,
        .insertLineDirectiveCheck = false   // makes it simplier to test
    };
    filamat::IncludeResult source {
        .includeName = utils::CString("root.h"),
        .text = code
    };
    bool result = filamat::resolveIncludes(source, includer, options);
    EXPECT_TRUE(result);
    EXPECT_STREQ_TRIMMARGIN(R"(
        | #line 1 "root.h"
        | #line 1 "test.h"
        | include
        | #line 1 "root.h"
        | )", source.text.c_str());
}

TEST(IncludeResolver, MultipleIncludesLineDirective) {
    utils::CString code("#include \"one.h\"\n#include \"two.h\"\n");

    MockIncluder includer;
    includer
        .sourceForInclude("one.h", "1")
        .sourceForInclude("two.h", "2");

    filamat::ResolveOptions options = {
        .insertLineDirectives = true,
        .insertLineDirectiveCheck = false   // makes it simplier to test
    };
    filamat::IncludeResult source {
        .includeName = utils::CString("root.h"),
        .text = code,
    };
    bool result = filamat::resolveIncludes(source, includer, options);
    EXPECT_TRUE(result);

    EXPECT_STREQ_TRIMMARGIN(R"(
        | #line 1 "root.h"
        | #line 1 "one.h"
        | 1
        | #line 2 "root.h"
        | #line 1 "two.h"
        | 2
        | #line 3 "root.h"
        | )", source.text.c_str());
}

TEST(IncludeResolver, MultipleIncludesSameLineLineDirective) {
    // includes are on the same line
    utils::CString code("#include \"one.h\"#include \"two.h\"\n");

    MockIncluder includer;
    includer
        .sourceForInclude("one.h", "1")
        .sourceForInclude("two.h", "2")
        .expectIncludeIncludedBy("three.h", "two.h");

    filamat::ResolveOptions options = {
        .insertLineDirectives = true,
        .insertLineDirectiveCheck = false   // makes it simplier to test
    };
    filamat::IncludeResult source {
        .includeName = utils::CString("root.h"),
        .text = code
    };
    bool result = filamat::resolveIncludes(source, includer, options);
    EXPECT_TRUE(result);

    EXPECT_STREQ_TRIMMARGIN(R"(
        | #line 1 "root.h"
        | #line 1 "one.h"
        | 1
        | #line 1 "root.h"
        | #line 1 "two.h"
        | 2
        | #line 2 "root.h"
        | )", source.text.c_str());
}

// -------------------------------------------------------------------------------------------------

#include <utils/Log.h>

class MaterialBuilder : public ::testing::Test {
protected:
    std::unique_ptr<JobSystem> jobSystem;
    filamat::MaterialBuilder mBuilder;

    MaterialBuilder() {
        jobSystem = std::make_unique<JobSystem>();
        jobSystem->adopt();
        filamat::MaterialBuilder::init();
        mBuilder.optimization(filamat::MaterialBuilder::Optimization::NONE);
    }

    virtual ~MaterialBuilder() {
        jobSystem->emancipate();
        filamat::MaterialBuilder::shutdown();
    }
};

TEST_F(MaterialBuilder, NoIncluder) {
    std::string shaderCode(R"(
        #include "test.h"
    )");
    mBuilder.material(shaderCode.c_str());

    filamat::Package result = mBuilder.build(*jobSystem);

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

    filamat::Package result = mBuilder.build(*jobSystem);
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

    filamat::Package result = mBuilder.build(*jobSystem);
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

    filamat::Package result = mBuilder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}

TEST_F(MaterialBuilder, IncludeFailure) {
    std::string shaderCode(R"(
        #include "nonexistent.h"
    )");
    mBuilder.material(shaderCode.c_str());

    MockIncluder includer;
    mBuilder.includeCallback(includer);

    filamat::Package result = mBuilder.build(*jobSystem);
    EXPECT_FALSE(result.isValid());
}

TEST_F(MaterialBuilder, NoShaderCode) {
    filamat::Package result = mBuilder.build(*jobSystem);
    EXPECT_TRUE(result.isValid());
}
