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


#include "ShaderMinifier.h"

#include <utils/Log.h>

namespace filamat {

static bool isIdCharNondigit(char c) {
    return c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isIdChar(char c) {
    return isIdCharNondigit(c) || (c >= '0' && c <= '9');
};

// Checks if a GLSL identifier lives at the given index in the given codeline.
// If so, returns the identifier and moves the given index to point
// to the first character after the identifier.
static bool getId(std::string_view codeline, size_t* pindex, std::string_view* id) {
    size_t index = *pindex;
    if (index >= codeline.size()) {
        return false;
    }
    if (!isIdCharNondigit(codeline[index])) {
        return false;
    }
    ++index;
    while (index < codeline.size() && isIdChar(codeline[index])) {
        ++index;
    }
    *id = codeline.substr(*pindex, index - *pindex);
    *pindex = index;
    return true;
}

// Checks if the given string lives anywhere after the given index in the given codeline.
// If so, moves the given index to point to the first character after the string.
static bool getString(std::string_view codeline, size_t* pindex, std::string_view s) {
    size_t index = codeline.find(s, *pindex);
    if (index == std::string_view::npos) {
        return false;
    }
    *pindex = index + s.size();
    return true;
}

// Checks if the given character is at the last position of the string.
static bool getLastChar(std::string_view codeline, size_t index, char c) {
    if (index != codeline.size() - 1) {
        return false;
    }
    return codeline[index] == c;
}

// Checks if whitespace lives at the given position of a codeline; if so, updates the given index.
static bool getWhitespace(std::string_view codeline, size_t* pindex) {
    size_t index = *pindex;
    while (index < codeline.size() && (codeline[index] == ' ' || codeline[index] == '\t')) {
        ++index;
    }
    if (index == *pindex) {
        return false;
    }
    *pindex = index;
    return true;
}

// Skips past an optional precision qualifier that is possibly surrounded by whitespace.
static void ignorePrecision(std::string_view codeline, size_t* pindex) {
    static std::string tokens[3] = {"lowp", "mediump", "highp"};
    const size_t i = *pindex;
    getWhitespace(codeline, pindex);
    for (const auto& token : tokens) {
        const size_t n = token.size();
        if (codeline.substr(i, i + n) == token) {
            *pindex += n;
            break;
        }
    }
    getWhitespace(codeline, pindex);
}

// Checks if the given string lives has an array size at the given codeline.
// If so, moves the given index to point to the first character after the array size.
// Always returns true for convenient daisy-chaining in the parser.
static bool ignoreArraySize(std::string_view codeline, size_t* pindex) {
    size_t index = *pindex;
    if (index >= codeline.size()) {
        return true;
    }
    if (codeline[index] != '[') {
        return true;
    }
    ++index;
    while (index < codeline.size() && codeline[index] != ']') {
        ++index;
    }
    *pindex = (index == codeline.size()) ? index : index + 1;
    return true;
}

static void replaceAll(std::string& result, std::string_view from, std::string to) {
    size_t n = 0;
    while ((n = result.find(from, n)) != std::string::npos) {
        result.replace(n, from.size(), to);
        n += to.size();
    }
}

namespace {
    enum ParserState {
        OUTSIDE,
        STRUCT_OPEN,
        STRUCT_DEFN,
    };
}

/**
 * Shrinks the specified string and returns a new string as the result.
 * To shrink the string, this method performs the following transforms:
 * - Remove leading white spaces at the beginning of each line
 * - Remove empty lines
 */
std::string ShaderMinifier::removeWhitespace(const std::string& s, bool mergeBraces) const {
    size_t cur = 0;

    std::string r;
    r.reserve(s.length());

    while (cur < s.length()) {
        size_t pos = cur;
        size_t len = 0;

        while (s[cur] != '\n') {
            cur++;
            len++;
        }

        size_t newPos = s.find_first_not_of(" \t", pos);
        if (newPos == std::string::npos) newPos = pos;

        // If we have a single { or } on a line, move it to the previous line instead
        size_t subLen = len - (newPos - pos);
        if (mergeBraces && subLen == 1 && (s[newPos] == '{' || s[newPos] == '}')) {
            r.replace(r.size() - 1, 1, 1, s[newPos]);
        } else {
            r.append(s, newPos, subLen);
        }
        r += '\n';

        while (s[cur] == '\n') {
            cur++;
        }
    }

    return r;
}

/**
 * Uniform block definitions can be quite big so this compresses them as follows.
 * First, the uniform struct definitions are found, new field names are generated, and a mapping
 * table is built. Second, all uses are replaced by applying the mapping table.
 *
 * The struct definition must be a sequence of tokens with the following pattern. This is fairly
 * constrained (e.g. no comments or nesting) but this is designed to operate on generated code.
 *
 *     "uniform" TypeIdentifier
 *     {
 *     OptionalPrecQual TypeIdentifier FieldIdentifier OptionalArraySize ;
 *     OptionalPrecQual TypeIdentifier FieldIdentifier OptionalArraySize ;
 *     OptionalPrecQual TypeIdentifier FieldIdentifier OptionalArraySize ;
 *     } StructIdentifier ;
 */
std::string ShaderMinifier::renameStructFields(const std::string& source) {
    std::string_view sv = source;
    size_t first = 0;
    mCodelines.clear();
    while (first < sv.size()) {
        const size_t second = sv.find_first_of('\n', first);
        if (first != second) {
            mCodelines.emplace_back(sv.substr(first, second - first));
        }
        if (second == std::string_view::npos) {
            break;
        }
        first = second + 1;
    }
    buildFieldMapping();
    return applyFieldMapping();
}

void ShaderMinifier::buildFieldMapping() {
    mStructFieldMap.clear();
    mStructDefnMap.clear();
    std::string currentStructPrefix;
    std::vector<std::string_view> currentStructFields;
    ParserState state = OUTSIDE;
    for (std::string_view codeline : mCodelines) {
        size_t cursor = 0;
        std::string_view typeId;
        std::string_view fieldName;
        switch (state) {
            case OUTSIDE:
                if (getString(codeline, &cursor, "uniform") &&
                        getWhitespace(codeline, &cursor) &&
                        getId(codeline, &cursor, &typeId) &&
                        cursor == codeline.size()) {
                    currentStructPrefix = std::string(typeId) + ".";
                    state = STRUCT_OPEN;
                }
                continue;
            case STRUCT_OPEN:
                state = getLastChar(codeline, 0, '{') ? STRUCT_DEFN : OUTSIDE;
                continue;
            case STRUCT_DEFN: {
                std::string_view structName;
                if (getString(codeline, &cursor, "}")) {
                    if (!getWhitespace(codeline, &cursor) ||
                            !getId(codeline, &cursor, &structName) ||
                            !getLastChar(codeline, cursor, ';')) {
                        break;
                    }
                    const std::string structNamePrefix = std::string(structName) + ".";
                    std::string generatedFieldName = "a";
                    for (auto field : currentStructFields) {
                        const std::string sField(field);
                        const std::string key = structNamePrefix + sField;
                        const std::string val = structNamePrefix + generatedFieldName;
                        mStructFieldMap.push_back({key, val});
                        mStructDefnMap[currentStructPrefix + sField] = generatedFieldName;
                        if (generatedFieldName[0] == 'z') {
                            generatedFieldName = "a" + generatedFieldName;
                        } else {
                            generatedFieldName[0]++;
                        }
                    }
                    currentStructFields.clear();
                    state = OUTSIDE;
                    break;
                }
                ignorePrecision(codeline, &cursor);
                if (!getId(codeline, &cursor, &typeId) ||
                        !getWhitespace(codeline, &cursor) ||
                        !getId(codeline, &cursor, &fieldName) ||
                        !ignoreArraySize(codeline, &cursor) ||
                        !getLastChar(codeline, cursor, ';')) {
                    break;
                }
                currentStructFields.push_back(fieldName);
                break;
            }
        }
    }

    // Sort keys from longest to shortest because we want to replace "fogColorFromIbl" before
    // replacing "fogColor".
    const auto& compare = [](const RenameEntry& a, const RenameEntry& b) {
        return a.first.length() > b.first.length();
    };
    std::sort(mStructFieldMap.begin(), mStructFieldMap.end(), compare);
}

std::string ShaderMinifier::applyFieldMapping() const {
    std::string result;
    ParserState state = OUTSIDE;
    std::string currentStructPrefix;
    for (std::string_view codeline : mCodelines) {
        std::string modified(codeline);
        std::string_view fieldName;
        std::string_view structName;
        std::string_view typeId;
        size_t cursor = 0;
        switch (state) {
            case OUTSIDE: {
                if (getString(codeline, &cursor, "uniform") &&
                        getWhitespace(codeline, &cursor) &&
                        getId(codeline, &cursor, &typeId) &&
                        cursor == codeline.size()) {
                    currentStructPrefix = std::string(typeId) + ".";
                    state = STRUCT_OPEN;
                    break;
                }
                for (const auto& key : mStructFieldMap) {
                    replaceAll(modified, key.first, key.second);
                }
                break;
            }
            case STRUCT_OPEN:
                state = getLastChar(codeline, 0, '{') ? STRUCT_DEFN : OUTSIDE;
                break;
            case STRUCT_DEFN: {
                if (getString(codeline, &cursor, "}") &&
                        getWhitespace(codeline, &cursor) &&
                        getId(codeline, &cursor, &structName) &&
                        getLastChar(codeline, cursor, ';')) {
                    state = OUTSIDE;
                    break;
                }
                ignorePrecision(codeline, &cursor);
                if (!getId(codeline, &cursor, &typeId) ||
                        !getWhitespace(codeline, &cursor) ||
                        !getId(codeline, &cursor, &fieldName) ||
                        !ignoreArraySize(codeline, &cursor) ||
                        !getLastChar(codeline, cursor, ';')) {
                    break;
                }
                std::string key = currentStructPrefix + std::string(fieldName);
                auto iter = mStructDefnMap.find(key);
                if (iter == mStructDefnMap.end()) {
                    utils::slog.e << "ShaderMinifier error: " << key << utils::io::endl;
                    break;
                }
                replaceAll(modified, fieldName, iter->second);
                break;
            }
        }
        result += modified + "\n";
    }
    return result;
}

} // namespace filamat
