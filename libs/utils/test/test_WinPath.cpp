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

TEST(WinPathTest, Sanitization) {
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
    r = Path::getCanonicalPath("C:\\");
    EXPECT_EQ("C:\\", r);

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

TEST(WinPathTest, AbsolutePath) {
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

TEST(WinPathTest, Split) {
    std::vector<std::string> segments;

    segments = Path("").split();
    EXPECT_EQ(0, segments.size());

    segments = Path("\\").split();
    EXPECT_EQ(1, segments.size());
    EXPECT_EQ("\\", segments[0]);

    segments = Path("C:\\").split();
    EXPECT_EQ(1, segments.size());
    EXPECT_EQ("C:\\", segments[0]);

    segments = Path("\\out\\blue\\bin").split();
    EXPECT_EQ(4, segments.size());
    EXPECT_EQ("\\", segments[0]);
    EXPECT_EQ("out", segments[1]);
    EXPECT_EQ("blue", segments[2]);
    EXPECT_EQ("bin", segments[3]);

    segments = Path("/out\\foo/blue\\bin/").split();
    EXPECT_EQ(5, segments.size());
    EXPECT_EQ("\\", segments[0]);
    EXPECT_EQ("out", segments[1]);
    EXPECT_EQ("foo", segments[2]);
    EXPECT_EQ("blue", segments[3]);
    EXPECT_EQ("bin", segments[4]);

    segments = Path("C:\\out\\foo/blue\\bin/").split();
    EXPECT_EQ(5, segments.size());
    EXPECT_EQ("C:\\", segments[0]);
    EXPECT_EQ("out", segments[1]);
    EXPECT_EQ("foo", segments[2]);
    EXPECT_EQ("blue", segments[3]);
    EXPECT_EQ("bin", segments[4]);
}

TEST(WinPathTest, Concatenate) {
    Path root("C:\\Volumes\\Replicant\\blue");

    Path r;
    r = root.concat("");
    EXPECT_EQ("C:\\Volumes\\Replicant\\blue", r.getPath());

    r = root.concat("C:\\out\\bin");
    EXPECT_EQ("C:\\out\\bin", r.getPath());

    r = root.concat("out\\bin");
    EXPECT_EQ("C:\\Volumes\\Replicant\\blue\\out\\bin", r.getPath());

    r = root.concat(".");
    EXPECT_EQ("C:\\Volumes\\Replicant\\blue", r.getPath());

    r = root.concat("..");
    EXPECT_EQ("C:\\Volumes\\Replicant", r.getPath());

    r = root.concat("C:\\");
    EXPECT_EQ("C:\\", r.getPath());

    r = root.concat("..\\remote-blue");
    EXPECT_EQ("C:\\Volumes\\Replicant\\remote-blue", r.getPath());

    r = root.concat("..\\remote-blue");
    EXPECT_EQ(r, root + Path("../remote-blue"));
    EXPECT_EQ(r, root + "../remote-blue");

    r = "C:\\out\\bin";
    r.concatToSelf("../bin");
    EXPECT_EQ("C:\\out\\bin", r.getPath());

    r += "./resources";
    EXPECT_EQ("C:\\out\\bin\\resources", r.getPath());

    // Unix-style separators work too
    r = root.concat("out/bin/foo/bar");
    EXPECT_EQ("C:\\Volumes\\Replicant\\blue\\out\\bin\\foo\\bar", r.getPath());

    r = "";
    r = r.concat("foo\\bar");
    EXPECT_EQ("foo\\bar", r.getPath());

    r = "";
    r.concatToSelf("foo\\bar");
    EXPECT_EQ("foo\\bar", r.getPath());
}

TEST(PathTest, GetParent) {
    std::string r;
    Path p("C:\\out\\bin");
    r = p.getParent();
    EXPECT_EQ("C:\\out\\", r);

    p = "C:\\out\\bin\\";
    r = p.getParent();
    EXPECT_EQ("C:\\out\\", r);

    p = "out\\bin";
    r = p.getParent();
    EXPECT_EQ("out\\", r);

    p = "out\\bin\\";
    r = p.getParent();
    EXPECT_EQ("out\\", r);

    p = "out";
    r = p.getParent();
    EXPECT_EQ("", r);

    p = "C:\\out";
    r = p.getParent();
    EXPECT_EQ("C:\\", r);

    p = "";
    r = p.getParent();
    EXPECT_EQ("", r);

    p = "C:\\";
    r = p.getParent();
    EXPECT_EQ("C:\\", r);
}
