/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_JSONPARSER_H
#define TNT_JSONPARSER_H

#include "JsonishLexeme.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace matc {

// Follows the LL(1) grammar described at http://json.org:
//   with the following productions.
//
// object
//     {}
//     { members }
//  members
//     pair
//     pair , members
//  pair
//     string : value
//  array
//     []
//     [ elements ]
//  elements
//     value
//     value , elements
//  value
//     string
//     number
//     object
//     array
//     true
//     false
//     null
//
// The parser is a recursive top-descent parser with a method for each
// grammar production.

class JsonishNull;
class JsonishBool;
class JsonishNumber;
class JsonishString;
class JsonishArray;
class JsonishObject;

class JsonishValue {
public:
    enum Type {
        STRING,
        NUMBER,
        OBJECT,
        ARRAY,
        BOOL,
        NUll
    };

    static const char * typeToString(Type type) {
        switch (type) {
            case STRING: return "STRING";
            case NUMBER: return "NUMBER";
            case OBJECT: return "OBJECT";
            case ARRAY: return "ARRAY";
            case BOOL: return "BOOL";
            case NUll: return "NULL";
        }
        return "NULL";
    }

    const JsonishBool* toJsonBool() const {
        if (mType != BOOL) {
            return nullptr;
        } else {
            return reinterpret_cast<const JsonishBool*>(this);
        }
    }

    const JsonishNumber* toJsonNumber() const {
        if (mType != NUMBER) {
            return nullptr;
        } else {
            return reinterpret_cast<const JsonishNumber*>(this);
        }
    }

    const JsonishString* toJsonString() const {
        if (mType != STRING) {
            return nullptr;
        } else {
            return reinterpret_cast<const JsonishString*>(this);
        }
    }

    const JsonishObject* toJsonObject() const {
        if (mType != OBJECT) {
            return nullptr;
        } else {
            return reinterpret_cast<const JsonishObject*>(this);
        }
    }
    const JsonishArray* toJsonArray() const {
        if (mType != ARRAY) {
            return nullptr;
        } else {
            return reinterpret_cast<const JsonishArray*>(this);
        }
    }

    const JsonishNull* toJsonNull() const {
        if (mType != BOOL) {
            return nullptr;
        } else {
            return reinterpret_cast<const JsonishNull*>(this);
        }
    }

    explicit JsonishValue(Type type) : mType(type) { }

    virtual ~JsonishValue() = default;

    Type getType() const { return mType; }

protected:
    Type mType;
};

class JsonishNull final : public JsonishValue {
public:
    explicit JsonishNull() : JsonishValue(NUll) { }

    ~JsonishNull() override = default;
};

class JsonishBool final : public JsonishValue {
public:
    explicit JsonishBool(bool b) : JsonishValue(BOOL), mBool(b) {}

    ~JsonishBool() override = default;

    bool getBool() const {
        return mBool;
    }

private:
    bool mBool;
};

class JsonishNumber final : public JsonishValue {
public:
    explicit JsonishNumber(float f) : JsonishValue(NUMBER), mFloat(f) {}

    ~JsonishNumber() override = default;

    float getFloat() const {
        return mFloat;
    }

private:
    float mFloat;
};

class JsonishString final : public JsonishValue {
public:
    explicit JsonishString(const std::string& string);

    ~JsonishString() override = default;

    const std::string& getString() const {
        return mString;
    }

private:
    std::string mString;
};

struct JsonishPair {
    std::string key;
    JsonishValue* value;
};

class JsonishObject final : public JsonishValue {
public:
    JsonishObject() : JsonishValue(OBJECT) { }

    ~JsonishObject() override {
        for (const auto& member : mMembers) {
            delete member.second;
        }
    }

    void addPair(JsonishPair& pair) {
        mMembers[pair.key] = pair.value;
    }

    const std::unordered_map<std::string, const JsonishValue*>& getEntries() const noexcept {
        return mMembers;
    }

    bool hasKey(const std::string& key) const noexcept {
        return mMembers.find(key) != mMembers.end();
    }

    const JsonishValue* getValue(const std::string& key) const noexcept {
        if (!hasKey(key)) {
            return nullptr;
        } else {
            return mMembers.at(key);
        }
    }

private:
    std::unordered_map<std::string, const JsonishValue*> mMembers;
};

class JsonishArray final : public JsonishValue {
public:
    JsonishArray() : JsonishValue(ARRAY) {}

    ~JsonishArray() override {
        for (const JsonishValue* v : mElements) {
            delete v;
        }
    }

    void addValue(const JsonishValue* value) {
        mElements.push_back(value);
    }

    const std::vector<const JsonishValue*>& getElements() const noexcept {
        return mElements;
    }

private:
    std::vector<const JsonishValue*> mElements;
};


class JsonishParser {
public:
    explicit JsonishParser(const std::vector<JsonLexeme>& lexemes): mCursor(0), mLexemes(lexemes) {
    }

    std::unique_ptr<JsonishObject> parse() noexcept;

private:
    JsonishObject* parseObject() noexcept;
    JsonishArray* parseArray() noexcept;
    JsonishPair parsePair() noexcept;
    JsonishValue* parseValue() noexcept;
    JsonishObject* parseMembers() noexcept;
    JsonishArray* parseElements() noexcept;
    JsonishValue* parseString() noexcept;

    const JsonLexeme* consumeLexeme(JsonType type) noexcept;
    const JsonLexeme* peekNextLexemeType() const noexcept;
    const JsonLexeme* peekCurrentLexemeType() const noexcept;

    void reportError(const char* message) noexcept;

    size_t mCursor = 0;
    const std::vector<JsonLexeme>& mLexemes;
    bool mErrorReported = false;
};

} // namespace matc

#endif //TNT_JSONPARSER_H
