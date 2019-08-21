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

#include <matc/DirIncluder.h>

using namespace utils;
using namespace filamat;

const utils::Path root = utils::Path(__FILE__).getParent();

TEST(DirIncluder, IncludeNonexistent) {
    matc::DirIncluder includer;
    {
        Includer::IncludeResult* result = includer.includeLocal(CString("nonexistent.h"), CString(""));
        EXPECT_EQ(nullptr, result);
        delete result;
    }
    {
        utils::CString includerFile((root + "Foo.h").getPath().c_str());
        Includer::IncludeResult* result = includer.includeLocal(CString("nonexistent.h"), includerFile);
        EXPECT_EQ(nullptr, result);
        delete result;
    }
}

TEST(DirIncluder, IncludeFile) {
    matc::DirIncluder includer;
    includer.setIncludeDirectory(root);

    Includer::IncludeResult* result = includer.includeLocal(CString("Foo.h"), CString(""));

    // The result's source should be set to the contents of the includer file.
    EXPECT_STREQ("// test include file", result->source.c_str());

    // The result's name should be set to the full path to the header file.
    EXPECT_STREQ((root + "Foo.h").c_str(), result->name.c_str());

    delete result;
}

TEST(DirIncluder, IncludeFileFromIncluder) {
    matc::DirIncluder includer;
    includer.setIncludeDirectory(root);

    utils::CString includerFile((root + "Dir/Baz.h").c_str());

    Includer::IncludeResult* result = includer.includeLocal(CString("Bar.h"), includerFile);

    EXPECT_STREQ("// Bar.h", result->source.c_str());

    EXPECT_STREQ((root + "Dir/Bar.h").c_str(), result->name.c_str());

    delete result;
}
