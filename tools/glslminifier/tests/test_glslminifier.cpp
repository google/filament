/*
 * Copyright 2018 The Android Open Source Project
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

#include "GlslMinify.h"

#include <string>

using std::string;

using namespace glslminifier;

class GlslminifierTest : public testing::Test {};

TEST_F(GlslminifierTest, EmptyString) {
    EXPECT_EQ(minifyGlsl(""), "");
}

TEST_F(GlslminifierTest, GlslNoChanges) {
    std::string glsl = R"glsl("void main() { gl_FragColor = vec4(1.0); })glsl";
    EXPECT_EQ(minifyGlsl(glsl, GlslMinifyOptions::ALL), glsl);
}

TEST_F(GlslminifierTest, RemoveSlashSlashComments) {
    std::string glsl = R"glsl(
        #version 330
        void main() {// This is a comment at the end of a line.
            // This is a comment, and this whole line should be removed.
            gl_FragColor = vec4(2.0 / 4.0, 0.0, 0.0, 1.0);
            // Another comment.
        }
        // This is a comment without a trailing newline.)glsl";
    std::string expected = R"glsl(
        #version 330
        void main() {

            gl_FragColor = vec4(2.0 / 4.0, 0.0, 0.0, 1.0);

        }
)glsl";
    EXPECT_EQ(minifyGlsl(glsl, GlslMinifyOptions::STRIP_COMMENTS), expected);
}

TEST_F(GlslminifierTest, RemoveStarComments) {
    std::string glsl = R"glsl(
        #version 330
        void main() {   /* this is a comment block at the end of a line */
            /* this is a comment block
               that spans
               multiple lines */
            gl_FragColor = vec4(2.0 / 4.0, 0.0, 0.0, 1.0);
            /* this is another comment block */
        }
        )glsl";
    std::string expected = R"glsl(
        #version 330
        void main() {

            gl_FragColor = vec4(2.0 / 4.0, 0.0, 0.0, 1.0);

        }
        )glsl";
    EXPECT_EQ(minifyGlsl(glsl, GlslMinifyOptions::STRIP_COMMENTS), expected);
}

TEST_F(GlslminifierTest, RemoveBlankLines) {
    std::string glsl = R"glsl(
        #version 330


        void main() {

            gl_FragColor = vec4(2.0 / 4.0, 0.0, 0.0, 1.0);

        }
        )glsl";
    std::string expected = R"glsl(
        #version 330
        void main() {
            gl_FragColor = vec4(2.0 / 4.0, 0.0, 0.0, 1.0);
        }
        )glsl";
    EXPECT_EQ(minifyGlsl(glsl, GlslMinifyOptions::STRIP_EMPTY_LINES), expected);
}

TEST_F(GlslminifierTest, RemoveBlankLinesWindows) {
    std::string glsl = "line one\r\n\r\n\r\nline two";
    std::string expected = "line one\r\nline two";
    EXPECT_EQ(minifyGlsl(glsl, GlslMinifyOptions::STRIP_EMPTY_LINES), expected);
}

TEST_F(GlslminifierTest, RemoveIndentation) {
    std::string glsl = R"glsl(     #version 330
        void main() {
            gl_FragColor = vec4(2.0 / 4.0, 0.0, 0.0, 1.0);
        }
        )glsl";
    std::string expected = R"glsl(#version 330
void main() {
gl_FragColor = vec4(2.0 / 4.0, 0.0, 0.0, 1.0);
}
)glsl";
    EXPECT_EQ(minifyGlsl(glsl, GlslMinifyOptions::STRIP_INDENTATION), expected);
}

TEST_F(GlslminifierTest, TrailingNewline) {
    // Ensure trailing newlines are acceptable when stripping identation.
    std::string glsl = R"glsl(trailing newline;
)glsl";
    EXPECT_EQ(minifyGlsl(glsl, GlslMinifyOptions::STRIP_INDENTATION), glsl);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
