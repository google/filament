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

// TODO: these tests are disabled as they fail on CI, which needs investigation.

TEST(DirIncluder, DISABLED_IncludeNonexistent) {
    matc::DirIncluder includer;
    {
        IncludeResult i {
            .includeName = CString("nonexistent.h")
        };
        bool success = includer(CString(""), i);
        EXPECT_FALSE(success);
    }
    {
        utils::CString includerFile((root + "Foo.h").getPath().c_str());
        IncludeResult i {
            .includeName = CString("nonexistent.h")
        };
        bool success = includer(includerFile, i);
        EXPECT_FALSE(success);
    }
}

TEST(DirIncluder, DISABLED_IncludeFile) {
    matc::DirIncluder includer;
    includer.setIncludeDirectory(root);

    IncludeResult result {
        .includeName = CString("Foo.h")
    };
    bool success = includer(CString(""), result);

    EXPECT_TRUE(success);

    // The result's source should be set to the contents of the includer file.
    EXPECT_STREQ("// test include file", result.text.c_str());

    // The result's name should be set to the full path to the header file.
    EXPECT_STREQ((root + "Foo.h").c_str(), result.name.c_str());
}

TEST(DirIncluder, DISABLED_IncludeFileFromIncluder) {
    matc::DirIncluder includer;
    includer.setIncludeDirectory(root);

    utils::CString includerFile((root + "Dir/Baz.h").c_str());

    IncludeResult result {
        .includeName = CString("Bar.h")
    };
    bool success = includer(includerFile, result);

    EXPECT_STREQ("// Bar.h", result.text.c_str());

    EXPECT_STREQ((root + "Dir/Bar.h").c_str(), result.name.c_str());
}
