/*
 * Copyright (C) 2015 The Android Open Source Project
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

void mk_file(Path &p, const char *str) {
    FILE *f = ::fopen(p.c_str(), "w");
    ::fprintf(f, "placeholder %s", str);
    ::fclose(f);
};

TEST(PathTest, Constructor) {
    // Ensure the path is canonical
    Path path("/out/blue/../bin/./test_path");
    EXPECT_EQ(Path::getCanonicalPath("/out/blue/../bin/./test_path"), path.getPath());
}

TEST(PathTest, Conversion) {
    Path r("/out/blue/bin");
    std::string s(r);
    EXPECT_EQ(r, s);
}

TEST(PathTest, Assignment) {
    Path r("/out/blue/bin");

    r = "/out/blue";
    EXPECT_EQ("/out/blue", r.getPath());

    std::string s("/etc");
    r = s;
    EXPECT_EQ("/etc", r.getPath());

    r = Path("/bin");
    EXPECT_EQ("/bin", r.getPath());

    r.setPath("/blue/bin");
    EXPECT_EQ("/blue/bin", r.getPath());
}

TEST(PathTest, Concatenate) {
    Path root("/Volumes/Replicant/blue");

    Path r;
    r = root.concat("");
    EXPECT_EQ("/Volumes/Replicant/blue", r.getPath());

    r = root.concat("/out/bin");
    EXPECT_EQ("/out/bin", r.getPath());

    r = root.concat("out/bin");
    EXPECT_EQ("/Volumes/Replicant/blue/out/bin", r.getPath());

    r = root.concat(".");
    EXPECT_EQ("/Volumes/Replicant/blue", r.getPath());

    r = root.concat("..");
    EXPECT_EQ("/Volumes/Replicant", r.getPath());

    r = root.concat("/");
    EXPECT_EQ("/", r.getPath());

    r = root.concat("../remote-blue");
    EXPECT_EQ("/Volumes/Replicant/remote-blue", r.getPath());

    r = root.concat("../remote-blue");
    EXPECT_EQ(r, root + Path("../remote-blue"));
    EXPECT_EQ(r, root + "../remote-blue");

    r = "/out/bin";
    r.concatToSelf("../bin");
    EXPECT_EQ("/out/bin", r.getPath());

    r += "./resources";
    EXPECT_EQ("/out/bin/resources", r.getPath());
}

TEST(PathTest, Sanitization) {
    std::string r;

    // An empty path remains empty
    r = Path::getCanonicalPath("");
    EXPECT_EQ("", r);

    // A single / is preserved
    r = Path::getCanonicalPath("/");
    EXPECT_EQ("/", r);

    // A single / is preserved
    r = Path::getCanonicalPath("//");
    EXPECT_EQ("/", r);

    // A leading . is preserved
    r = Path::getCanonicalPath("./out");
    EXPECT_EQ("./out", r);

    // A leading . is preserved
    r = Path::getCanonicalPath(".");
    EXPECT_EQ(".", r);

    // A leading .. is preserved
    r = Path::getCanonicalPath("/.");
    EXPECT_EQ("/", r);

    // A leading .. is preserved
    r = Path::getCanonicalPath("../out");
    EXPECT_EQ("../out", r);

    // A leading .. is preserved
    r = Path::getCanonicalPath("/..");
    EXPECT_EQ("/", r);

    // A leading / is preserved
    r = Path::getCanonicalPath("/out");
    EXPECT_EQ("/out", r);

    // A middle . is removed
    r = Path::getCanonicalPath("out/./bin");
    EXPECT_EQ("out/bin", r);

    // two middle . are removed
    r = Path::getCanonicalPath("out/././bin");
    EXPECT_EQ("out/bin", r);

    // three middle . are removed
    r = Path::getCanonicalPath("out/./././bin");
    EXPECT_EQ("out/bin", r);

    // a starting . is kept
    r = Path::getCanonicalPath("./bin");
    EXPECT_EQ("./bin", r);

    // several starting . are collapsed to one
    r = Path::getCanonicalPath("././bin");
    EXPECT_EQ("./bin", r);

    // several starting . are collapsed to one
    r = Path::getCanonicalPath("./././bin");
    EXPECT_EQ("./bin", r);

    // A middle .. is removed and the previous segment is removed
    r = Path::getCanonicalPath("out/blue/../bin");
    EXPECT_EQ("out/bin", r);

    // Special case of the previous test
    // A .. in second spot pops to an empty stack
    r = Path::getCanonicalPath("out/../bin");
    EXPECT_EQ("bin", r);

    // Special case of the previous test
    // A .. in second spot pops to an empty stack
    r = Path::getCanonicalPath("out/../../bin");
    EXPECT_EQ("../bin", r);

    // make sure it works with several ../
    r = Path::getCanonicalPath("../../bin");
    EXPECT_EQ("../../bin", r);

    // make sure to test odd/even numbers of ../ and more than one
    r = Path::getCanonicalPath("../../../bin");
    EXPECT_EQ("../../../bin", r);

    // check odd and more than 1 or 2 ../ in the middle
    r = Path::getCanonicalPath("out/../../../bin");
    EXPECT_EQ("../../bin", r);

    // Two or more slashes is the same as one
    r = Path::getCanonicalPath("out/blue//bin");
    EXPECT_EQ("out/blue/bin", r);

    // A trailing / is preserved
    r = Path::getCanonicalPath("out/blue/bin/");
    EXPECT_EQ("out/blue/bin/", r);

    // Both leading and trailing / are preserved
    r = Path::getCanonicalPath("/out/blue/bin/");
    EXPECT_EQ("/out/blue/bin/", r);

    // preserve a segment starting with a .
    r = Path::getCanonicalPath("/out/.blue/bin/");
    EXPECT_EQ("/out/.blue/bin/", r);

    // remove a /./ following a ..
    r = Path::getCanonicalPath("/out/.././bin/");
    EXPECT_EQ("/bin/", r);

    // remove a /./ following a ..
    r = Path::getCanonicalPath(".././bin/");
    EXPECT_EQ("../bin/", r);

    // collapse multiple /
    r = Path::getCanonicalPath("////");
    EXPECT_EQ("/", r);

    // collapse multiple /
    r = Path::getCanonicalPath("/aaa///bbb/");
    EXPECT_EQ("/aaa/bbb/", r);

    // collapse multiple /
    r = Path::getCanonicalPath("///.///");
    EXPECT_EQ("/", r);

    // multiple ..
    r = Path::getCanonicalPath("../out/../in");
    EXPECT_EQ("../in", r);

    // /..
    r = Path::getCanonicalPath("/../out/../in");
    EXPECT_EQ("/in", r);

    // No sanitizing required
    r = Path::getCanonicalPath("out");
    EXPECT_EQ("out", r);
}

TEST(PathTest, GetParent) {
    std::string r;
    Path p("/out/bin");
    r = p.getParent();
    EXPECT_EQ("/out/", r);

    p = "/out/bin/";
    r = p.getParent();
    EXPECT_EQ("/out/", r);

    p = "F:\\out\\bin\\";
    r = p.getParent();
    EXPECT_EQ("F:/out/", r);

    p = "out/bin";
    r = p.getParent();
    EXPECT_EQ("out/", r);

    p = "out/bin/";
    r = p.getParent();
    EXPECT_EQ("out/", r);

    p = "out";
    r = p.getParent();
    EXPECT_EQ("", r);

    p = "/out";
    r = p.getParent();
    EXPECT_EQ("/", r);

    p = "";
    r = p.getParent();
    EXPECT_EQ("", r);

    p = "/";
    r = p.getParent();
    EXPECT_EQ("/", r);
}

TEST(PathTest, GetName) {
    Path p("/out/bin");
    EXPECT_EQ("bin", p.getName());

    p = "/out/bin/";
    EXPECT_EQ("bin", p.getName());

    p = "/";
    EXPECT_EQ("/", p.getName());

    p = "out";
    EXPECT_EQ("out", p.getName());
}

TEST(PathTest, Exists) {
    EXPECT_FALSE(Path("").exists());
    EXPECT_TRUE(Path("/").exists());
    EXPECT_FALSE(Path("this/better/not/be/a/path").exists());
}

TEST(PathTest, Split) {
    std::vector<std::string> segments;

    segments = Path("").split();
    EXPECT_EQ(0, segments.size());

    segments = Path("/").split();
    EXPECT_EQ(1, segments.size());
    EXPECT_EQ("/", segments[0]);

    segments = Path("d:\\").split();
    EXPECT_EQ(1, segments.size());
    EXPECT_EQ("d:", segments[0]);

    segments = Path("out/blue/bin").split();
    EXPECT_EQ(3, segments.size());
    EXPECT_EQ("out", segments[0]);
    EXPECT_EQ("blue", segments[1]);
    EXPECT_EQ("bin", segments[2]);

    segments = Path("/out/blue/bin").split();
    EXPECT_EQ(4, segments.size());
    EXPECT_EQ("/", segments[0]);
    EXPECT_EQ("out", segments[1]);
    EXPECT_EQ("blue", segments[2]);
    EXPECT_EQ("bin", segments[3]);

    segments = Path("d:\\out\\blue").split();
    EXPECT_EQ(3, segments.size());
    EXPECT_EQ("d:", segments[0]);
    EXPECT_EQ("out", segments[1]);
    EXPECT_EQ("blue", segments[2]);

    segments = Path("\\out\\blue").split();
    EXPECT_EQ(3, segments.size());
    EXPECT_EQ("/", segments[0]);
    EXPECT_EQ("out", segments[1]);
    EXPECT_EQ("blue", segments[2]);

    segments = Path("/out/blue/bin/").split();
    EXPECT_EQ(4, segments.size());
    EXPECT_EQ("/", segments[0]);
    EXPECT_EQ("out", segments[1]);
    EXPECT_EQ("blue", segments[2]);
    EXPECT_EQ("bin", segments[3]);
}

TEST(PathTest, CurrentDirectory) {
    Path p(Path::getCurrentDirectory());
    EXPECT_FALSE(p.isEmpty());
    EXPECT_TRUE (p.isAbsolute());
    EXPECT_TRUE (p.exists());
    EXPECT_TRUE (p.isDirectory());
    EXPECT_FALSE(p.isFile());
}

TEST(PathTest, CurrentExecutable) {
    Path p(Path::getCurrentExecutable());
    EXPECT_FALSE(p.isEmpty());
    EXPECT_TRUE(p.isAbsolute());
    EXPECT_TRUE(p.exists());
    EXPECT_FALSE(p.isDirectory());
    EXPECT_TRUE (p.isFile());
}

TEST(PathTest, AbsolutePath) {
    Path cwd = Path::getCurrentDirectory();

    Path p;
    p = Path("/out/blue/bin");
    EXPECT_TRUE(p.isAbsolute());

    p = p.getAbsolutePath();
    EXPECT_EQ("/out/blue/bin", p.getPath());

    p = Path("../bin").getAbsolutePath();
    EXPECT_NE(cwd, p);
    EXPECT_TRUE(p.isAbsolute());
}

TEST(PathTest, IsFile) {
    Path dir(Path::getCurrentDirectory());
    Path exe(Path::getCurrentExecutable());
    EXPECT_TRUE (exe.isFile());
    EXPECT_FALSE(dir.isFile());
}

TEST(PathTest, IsDirectory) {
    Path dir(Path::getCurrentDirectory());
    Path exe(Path::getCurrentExecutable());
    EXPECT_FALSE(exe.isDirectory());
    EXPECT_TRUE (dir.isDirectory());
}

TEST(PathTest, GetExtension) {
    Path p("/out/bin/somefile.txt");
    EXPECT_EQ(p.getExtension(), "txt");

    p = Path("/out/bin/somefilewithoutextension");
    EXPECT_EQ(p.getExtension(), "");

    p = Path("/out/bin/.tempdir/somefile.txt.bak");
    EXPECT_EQ(p.getExtension(), "bak");

    p = Path("/out/bin/.tempfile");
    EXPECT_EQ(p.getExtension(), "");

    p = Path("/out/bin/endsindot.");
    EXPECT_EQ(p.getExtension(), "");

    p = Path::getCurrentDirectory();
    EXPECT_EQ(p.getExtension(), "");

    p = Path();
    EXPECT_EQ(p.getExtension(), "");
}
