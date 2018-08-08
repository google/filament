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

#include <limits.h>
#include <gtest/gtest.h>

#include <utils/Path.h>

#include <iosfwd>
#include <string>
#include <vector>

#ifndef PATH_MAX    // should be in <limits.h>
#define PATH_MAX    4096
#endif

using namespace utils;

TEST(PathTest, Sanitization) {
    std::string r;

    // An empty path remains empty
    r = Path::getCanonicalPath("");
    EXPECT_EQ("", r);

    // A single / is preserved
    r = Path::getCanonicalPath("\\");
    EXPECT_EQ("\\", r);

    // Unix style paths are converted to Windows style
    r = Path::getCanonicalPath("out/./././bin/foo/../../bar");
    EXPECT_EQ("out\\bar", r);

    // A mix of Unix style paths and Windows style
    r = Path::getCanonicalPath("out/.\\././bin/foo\\../..\\bar");
    EXPECT_EQ("out\\bar", r);

    // Disk designation
    r = Path::getCanonicalPath("C:\\out\\bin");
    EXPECT_EQ("C:\\out\\bin", r);

    // Collapse .. with disk designation
    r = Path::getCanonicalPath("C:\\out\\bin\\..\\foo");
    EXPECT_EQ("C:\\out\\foo", r);

    // Collapse multiple .. with disk designation
    r = Path::getCanonicalPath("C:\\out\\bin\\..\\..\\foo");
    EXPECT_EQ("C:\\foo", r);

    // Collapse . with disk designation
    r = Path::getCanonicalPath("C:\\out\\.\\foo");
    EXPECT_EQ("C:\\out\\foo", r);

    // Collapse multiple . with disk designation
    r = Path::getCanonicalPath("C:\\out\\.\\.\\foo");
    EXPECT_EQ("C:\\out\\foo", r);

    // Collapse multiple . and .. with disk designation
    r = Path::getCanonicalPath("C:\\out\\bin\\.\\..\\..\\foo");
    EXPECT_EQ("C:\\foo", r);

    // make sure it works with several ../
    r = Path::getCanonicalPath("..\\..\\bin");
    EXPECT_EQ("..\\..\\bin", r);
}

TEST(PathTest, AbsolutePath) {
    Path cwd = Path::getCurrentDirectory();

    Path p;
    p = Path("C:\\out\\blue\\bin");
    EXPECT_TRUE(p.isAbsolute());

    p = p.getAbsolutePath();
    EXPECT_EQ("C:\\out\\blue\\bin", p.getPath());

    p = Path("../bin").getAbsolutePath();
    EXPECT_NE(cwd, p);
    EXPECT_TRUE(p.isAbsolute());
}
