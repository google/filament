/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by mIcable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define JSMN_HEADER

#include <viewer/Automation.h>

#include "jsonParseUtils.h"

#include <assert.h>

#include <sstream>
#include <string>
#include <vector>

#include <utils/Log.h>

using namespace utils;

using std::vector;

static const bool VERBOSE = false;

namespace filament {
namespace viewer {

struct SpecImpl {
    std::string name;
    std::vector<Settings> cases;
};

struct AutomationList::Impl {
    std::vector<SpecImpl> specs;
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
        readJson(json.c_str(), json.size(), out);
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
        SpecImpl* out) {
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
    while (true) {

        // Append a copy of the current Settings object to the case list.
        if (VERBOSE) {
            slog.i << "  Appending case " << caseIndex << io::endl;
        }
        out->cases[caseIndex++] = base;

        // Leave early if there are no permutations.
        if (iters.empty()) {
            return i;
        }

        // Use a basic counting algorithm to generate the next test case.
        // Bump the first digit, if it rolls back to 0 then bump the next digit, etc.
        // In this case, the "digit" is an iterator into a vector of JSON strings.
        propIndex = 0;
        for (auto& iter : iters) {
            const auto& prop = permute[propIndex++];
            if (++iter != prop.end()) {
                break;
            }
            iter = prop.begin();

            // Check if all permutations have been generated.
            if (propIndex == permute.size()) {
                assert(caseIndex == out->cases.size());
                return i;
            }
        }

        // Apply changes to the settings object.
        for (const auto& iter : iters) {
            const std::string& jsonString = *iter;
            if (VERBOSE) {
                slog.i << "  Applying " << jsonString.c_str() << io::endl;
            }
            if (!readJson(jsonString.c_str(), jsonString.size(), &base)) {
                return -1;
            }
        }
    }

    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, vector<SpecImpl>* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
    int size = tokens[i++].size;
    out->resize(size);
    for (int j = 0; j < size && i >= 0; ++j) {
        i = parseAutomationSpec(tokens, i, jsonChunk, &out->at(j));
    }
    return i;
}

AutomationList* AutomationList::generate(const char* jsonChunk, size_t size) {
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

    AutomationList::Impl* impl = new AutomationList::Impl();
    int i = parse(tokens, 0, jsonChunk, &impl->specs);
    free(tokens);

    if (i < 0) {
        delete impl;
        return nullptr;
    }

    return new AutomationList(impl);
}

AutomationSpec AutomationList::get(size_t index) const {
    const SpecImpl& spec = mImpl->specs[index];
    return {
        .name = spec.name.c_str(),
        .count = spec.cases.size(),
        .settings = spec.cases.data()
    };
}

size_t AutomationList::size() const { return mImpl->specs.size(); }
AutomationList::AutomationList(Impl* impl) : mImpl(impl) {}
AutomationList::~AutomationList() { delete mImpl; }

} // namespace viewer
} // namespace filament
