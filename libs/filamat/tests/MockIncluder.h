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

#include "Includes.h"

#include <unordered_map>
#include <string>

// Functor used for test cases.
class MockIncluder {

public:

    MockIncluder& sourceForInclude(const std::string& include, const std::string& source) {
        mIncludeMap[include].source = source;
        return *this;
    }

    // Assert that the expected includer is responsible for including a given include file.
    MockIncluder& expectIncludeIncludedBy(const std::string& include, const std::string& includer) {
        mIncludeMap[include].expectedIncluder = includer;
        return *this;
    }

    bool operator()(const utils::CString& includedBy, filamat::IncludeResult& result) {
        auto key = result.includeName.c_str();
        auto found = mIncludeMap.find(key);

        if (found == mIncludeMap.end()) {
            return false;
        }

        auto include = found->second;

        if (!include.expectedIncluder.empty()) {
            EXPECT_STREQ(includedBy.c_str_safe(), include.expectedIncluder.c_str());
        }

        if (!include.source.empty()) {
            result.text = utils::CString(include.source.c_str());
            result.name = result.includeName;
            return true;
        }

        return false;
    }

private:

    struct Include {
        std::string source;
        std::string expectedIncluder;
    };

    std::unordered_map<std::string, Include> mIncludeMap;

};
