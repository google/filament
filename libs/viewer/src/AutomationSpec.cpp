/*
 * Copyright (C) 2020 The Android Open Source Project
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

#define JSMN_HEADER

#include <viewer/AutomationSpec.h>

#include "jsonParseUtils.h"

#include <utils/Log.h>

#include <sstream>
#include <string>
#include <vector>

#include <assert.h>
#include <string.h>

using namespace utils;

using std::vector;

static const bool VERBOSE = false;

namespace filament::viewer {

static const char* DEFAULT_AUTOMATION = R"TXT([
    {
        "name": "ppoff",
        "base": {
            "view.postProcessingEnabled": false
        }
    },
    {
        "name": "vieweropts",
        "base": {
            "viewer.cameraFocusDistance": 0.1
        }
    },
    {
        "name": "viewopts",
        "base": {
        },
        "permute": {
            "view.msaa.enabled": [false, true],
            "view.taa.enabled": [false, true],
            "view.antiAliasing": ["NONE", "FXAA"],
            "view.ssao.enabled": [false, true],
            "view.screenSpaceReflections.enabled": [false, true]
            "view.bloom.enabled": [false, true],
            "view.dof.enabled": [false, true],
            "view.guardBand.enabled": [false, true]
        }
    }
]
)TXT";

struct Case {
    Settings settings;
    char const* name;
};

struct CaseGroup {
    std::string name;
    std::vector<Settings> cases;
};

struct AutomationSpec::Impl {
    std::vector<Case> cases;
    std::vector<std::string> names;
};

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, std::string* val) {
    CHECK_TOKTYPE(tokens[i], JSMN_STRING);
    *val = STR(tokens[i], jsonChunk);
    return i + 1;
}

static int parseBaseSettings(jsmntok_t const* tokens, int i, const char* jsonChunk, Settings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j, i += 2) {
        std::stringstream dk(STR(tokens[i], jsonChunk));
        std::string token;
        std::string prefix;
        int depth = 0;

        // Expand "foo.bar.baz" into "foo: { bar: { baz: "
        while (getline(dk, token, '.')) {
            prefix += "{ \"" + token + "\": ";
            depth++;
        }
        std::string json = prefix + STR(tokens[i + 1], jsonChunk);
        for (int d = 0; d < depth; d++) {  json += " } "; }
        if (VERBOSE) {
            slog.i << "  Base: " << json.c_str() << io::endl;
        }

        // Now that we have a complete JSON string, apply this property change.
        JsonSerializer serializer;
        serializer.readJson(json.c_str(), json.size(), out);
    }
    return i;
}

static int parsePermutationsSpec(jsmntok_t const* tokens, int i, const char* jsonChunk,
        vector<vector<std::string>>* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    out->resize(size);
    for (int j = 0; j < size; ++j) {
        std::stringstream dk(STR(tokens[i], jsonChunk));
        std::string token;
        std::string prefix;
        int depth = 0;

        // Expand "foo.bar.baz" into "foo: { bar: { baz: "
        while (getline(dk, token, '.')) {
            prefix += "{ \"" + token + "\": ";
            depth++;
        }
        ++i;

        // Build a complete JSON string for each requested property value.
        const jsmntok_t valueArray = tokens[i++];
        CHECK_TOKTYPE(valueArray, JSMN_ARRAY);
        vector<std::string>& spec = (*out)[j];
        spec.resize(valueArray.size);
        for (int k = 0; k < valueArray.size; k++, i++) {
            std::string json = prefix + STR(tokens[i], jsonChunk);
            for (int d = 0; d < depth; d++) {  json += " } "; }
            spec[k] = json;
        }
    }
    return i;
}

static int parseAutomationSpec(jsmntok_t const* tokens, int i, const char* jsonChunk,
        CaseGroup* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    Settings base;
    vector<vector<std::string>> permute;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (0 == compare(tok, jsonChunk, "name")) {
            i = parse(tokens, i + 1, jsonChunk, &out->name);
            if (VERBOSE) {
                slog.i << "Building spec [" << out->name << "]" << io::endl;
            }
        } else if (0 == compare(tok, jsonChunk, "base")) {
            i = parseBaseSettings(tokens, i + 1, jsonChunk, &base);
        } else if (0 == compare(tok, jsonChunk, "permute")) {
            i = parsePermutationsSpec(tokens, i + 1, jsonChunk, &permute);
        } else {
            slog.w << "Invalid automation key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid automation value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }

    // Leave early if there are no permutations.
    if (permute.empty()) {
        out->cases.resize(1);
        out->cases[0] = base;
        return i;
    }

    // Determine the number of permutations.
    size_t caseCount = 1;
    size_t propIndex = 0;
    vector<vector<std::string>::const_iterator> iters(permute.size());
    for (const auto& prop : permute) {
        caseCount *= prop.size();
        if (VERBOSE) {
            for (const auto& s : prop) {
                slog.i << "  Perm: " << s.c_str() << io::endl;
            }
        }
        iters[propIndex++] = prop.begin();
    }
    out->cases.resize(caseCount);
    if (VERBOSE) {
        slog.i << "  Case count: " << caseCount << io::endl;
    }

    size_t caseIndex = 0;
    JsonSerializer serializer;
    while (true) {
        if (VERBOSE) {
            slog.i << "  Generating test case " << caseIndex << io::endl;
        }

        // Use a basic counting algorithm to select the next combination of property values.
        // Bump the first digit, if it rolls back to 0 then bump the next digit, etc.
        // In this case, each "digit" is an iterator into a vector of JSON strings.
        if (caseIndex > 0) {
            propIndex = 0;
            for (auto& iter : iters) {
                const auto& prop = permute[propIndex++];
                if (++iter != prop.end()) {
                    break;
                }
                iter = prop.begin();

                // Check if all permutations have been generated.
                if (propIndex == permute.size()) {
                    assert(caseIndex == caseCount);
                    return i;
                }
            }
        }

        // Copy the base settings object, then apply changes.
        Settings& testCase = out->cases[caseIndex++];
        testCase = base;
        for (const auto& iter : iters) {
            const std::string& jsonString = *iter;
            if (VERBOSE) {
                slog.i << "    Applying " << jsonString.c_str() << io::endl;
            }
            if (!serializer.readJson(jsonString.c_str(), jsonString.size(), &testCase)) {
                return -1;
            }
        }
    }

    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, vector<CaseGroup>* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
    int size = tokens[i++].size;
    out->resize(size);
    for (int j = 0; j < size && i >= 0; ++j) {
        i = parseAutomationSpec(tokens, i, jsonChunk, &out->at(j));
    }
    return i;
}

AutomationSpec* AutomationSpec::generate(const char* jsonChunk, size_t size) {
    jsmn_parser parser = { 0, 0, 0 };

    int tokenCount = jsmn_parse(&parser, jsonChunk, size, nullptr, 0);
    if (tokenCount <= 0) {
        return nullptr;
    }

    jsmntok_t* tokens = (jsmntok_t*) malloc(sizeof(jsmntok_t) * tokenCount);
    assert(tokens);

    jsmn_init(&parser);
    tokenCount = jsmn_parse(&parser, jsonChunk, size, tokens, tokenCount);

    if (tokenCount <= 0) {
        free(tokens);
        return nullptr;
    }

    vector<CaseGroup> groups;
    int i = parse(tokens, 0, jsonChunk, &groups);
    free(tokens);

    if (i < 0) {
        return nullptr;
    }

    AutomationSpec::Impl* impl = new AutomationSpec::Impl();

    // Compute the flattened number of Settings objects.
    size_t total = 0;
    for (const auto& group : groups) {
        total += group.cases.size();
    }

    impl->names.resize(groups.size());
    impl->cases.resize(total);

    // Flatten the groups.
    size_t caseIndex = 0, groupIndex = 0;
    for (const auto& group : groups) {
        impl->names[groupIndex] = group.name;
        for (const auto& settings : group.cases) {
            impl->cases[caseIndex].name = impl->names[groupIndex].c_str();
            impl->cases[caseIndex].settings = settings;
            ++caseIndex;
        }
        ++groupIndex;
    }

    return new AutomationSpec(impl);
}

AutomationSpec* AutomationSpec::generateDefaultTestCases() {
    return generate(DEFAULT_AUTOMATION, strlen(DEFAULT_AUTOMATION));
}

bool AutomationSpec::get(size_t index, Settings* out) const {
    if (index >= mImpl->cases.size()) {
        return false;
    }
    if (out == nullptr) {
        return true;
    }
    *out = mImpl->cases.at(index).settings;
    return true;
}

char const* AutomationSpec::getName(size_t index) const {
    if (index >= mImpl->cases.size()) {
        return nullptr;
    }
    return mImpl->cases.at(index).name;
}

size_t AutomationSpec::size() const { return mImpl->cases.size(); }
AutomationSpec::AutomationSpec(Impl* impl) : mImpl(impl) {}
AutomationSpec::~AutomationSpec() { delete mImpl; }

} // namespace filament::viewer
