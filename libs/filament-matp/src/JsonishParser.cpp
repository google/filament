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

#include "JsonishParser.h"

#include <utils/sstream.h>
#include <utils/string.h>
#include <utils/Status.h>

#include <string.h>

namespace matp {

static std::pair<utils::Status, std::string> resolveEscapes(const std::string& s) {
    std::string out;
    out.reserve(s.length());

    bool inEscape = false;
    for (char c : s) {
        if (inEscape) {
            switch (c) {
                case '\\':
                    out += '\\';
                    break;
                case '/':
                    out += '\u002F';
                    break;
                case 't':
                    out += '\t';
                    break;
                case 'r':
                    out += '\r';
                    break;
                case 'n':
                    out += '\n';
                    break;
                case 'f':
                    out += '\f';
                    break;
                case 'b':
                    out += '\b';
                    break;
                case '"':
                    out += '"';
                    break;
                case 'u':
                    // TODO: implement unicode escape sequences
                    return { utils::Status::unsupported(
                            "Unicode escape sequences currently not supported."),
                        out };
                    break;
                default:
                    utils::io::sstream errorMessage;
                    errorMessage << "Invalid escape sequence \\" << c << ".";
                    return { utils::Status::invalidArgument(
                                     errorMessage.c_str()), out };
            }
            inEscape = false;
        } else {
            if (c == '\\') {
                inEscape = true;
                continue;
            } else {
                out += c;
            }
        }
    }

    if (inEscape) {
        return { utils::Status::invalidArgument("Incomplete escape sequence."), out };
    }

    return { utils::Status::ok(),  out };
}

std::unique_ptr<JsonishObject> JsonishParser::parse() noexcept {
    std::unique_ptr<JsonishObject> p(parseObject());
    return p;
}

utils::Status JsonishParser::getParseStatus() const noexcept {
    return mStatus;
}

JsonishObject* JsonishParser::parseObject() noexcept {
    // Read the {
    if (!consumeLexeme(JsonType::BLOCK_START)) {
        setInvalidArgumentError("expected token '{'");
        return nullptr;
    }

    // Empty blocks are allowed.
    if (consumeLexeme(BLOCK_END)) {
        return new JsonishObject();
    }

    // Read members.
    JsonishObject* object = parseMembers();

    // Read the }
    if (!consumeLexeme(JsonType::BLOCK_END)) {
        setInvalidArgumentError("expected token '}'");
        delete object;
        return nullptr;
    }

    return object;
}

JsonishArray* JsonishParser::parseArray() noexcept {
    // Read the [
    if (!consumeLexeme(JsonType::ARRAY_START)) {
        setInvalidArgumentError("expected token '['");
        return nullptr;
    }

    // Empty arrays are allowed.
    if (consumeLexeme(ARRAY_END))  {
        return new JsonishArray();
    }

    // Read elements.
    JsonishArray* array = parseElements();

    // Read the ]
    if (!consumeLexeme(JsonType::ARRAY_END)) {
        setInvalidArgumentError("expected token ']'");
        delete array;
        return nullptr;
    }

    return array;
}

JsonishPair JsonishParser::parsePair() noexcept {
    JsonishPair pair {"" , nullptr};
    // Read key string.
    const JsonLexeme* keyLexeme = consumeLexeme(JsonType::STRING);
    if (!keyLexeme) {
        return pair;
    }

    // Read the :
    if (!consumeLexeme(JsonType::COLUMN)) {
        return pair;
    }

    // Read the value.
    JsonishValue* value = parseValue();
    if (!value) {
        return pair;
    }

    pair = {keyLexeme->getStringValue(), value};
    return pair;
}

const JsonLexeme* JsonishParser::consumeLexeme(JsonType type) noexcept {
    if (mCursor >= mLexemes.size()) {
        return nullptr;
    }
    const JsonLexeme* lexeme = &mLexemes[mCursor];

    if (lexeme->getType() == type) {
        mCursor++;
        return lexeme;
    } else {
        return nullptr;
    }
}

const JsonLexeme* JsonishParser::peekNextLexemeType() const noexcept {
    if (mCursor >= mLexemes.size()) {
        return nullptr;
    }
    return &mLexemes[mCursor];
}

const JsonLexeme* JsonishParser::peekCurrentLexemeType() const noexcept {
    if (mCursor <= 0) {
        return nullptr;
    }
    return &mLexemes[mCursor - 1];
}

JsonishObject* JsonishParser::parseMembers() noexcept {
    JsonishObject* object = new JsonishObject();
    while (true) {
        JsonishPair pair = parsePair();
        if (!pair.value) {
            // Accept a comma after the last element
            const JsonLexeme* lexeme = peekNextLexemeType();
            if (lexeme && lexeme->getType() == BLOCK_END) {
                break;
            }

            setInvalidArgumentError("unable to parse pair");

            delete object;
            return nullptr;
        }

        object->addPair(pair);

        // Is there a comma and more pairs?
        if (!consumeLexeme(COMMA)) {
            break;
        }
    }
    return object;
}

JsonishArray* JsonishParser::parseElements() noexcept {
    JsonishArray* array = new JsonishArray();

    while (true) {
        JsonishValue* value = parseValue();
        if (!value) {
            setInvalidArgumentError("unable to read value");
            delete array;
            return nullptr;
        }
        array->addValue(value);

        // Is there a comma and more pairs?
        if (!consumeLexeme(COMMA)) {
            break;
        }
    }

    return array;
}

// Strings optionally have an array suffix, as in: "float[9]". To handle this we parse the array
// value (which might be multidimensional), discard the parsed value, and append it to the string.
JsonishValue* JsonishParser::parseString() noexcept {
    std::string tmp;
    const JsonLexeme* strLexeme = consumeLexeme(STRING);
    const JsonLexeme* arrLexeme = peekNextLexemeType();
    JsonishValue* arrValue;
    if (arrLexeme && arrLexeme->getType() == ARRAY_START && (arrValue = parseArray())) {
        delete arrValue;

        const JsonLexeme* next = peekNextLexemeType();
        if (!next) {
            return nullptr;
        }

        size_t length = next->getStart() - strLexeme->getStart();
        tmp = std::string(strLexeme->getStart(), length);
    } else {
        tmp = std::string(strLexeme->getStart(), strLexeme->getSize());
    }
    std::pair<utils::Status, std::string> resolved = resolveEscapes(tmp);
    if (!resolved.first.isOk()) {
        mStatus = resolved.first;
        return nullptr;
    }
    return new JsonishString(resolved.second);
}

JsonishValue* JsonishParser::parseValue() noexcept {
    const JsonLexeme* next = peekNextLexemeType();
    if (next == nullptr) {
        return nullptr;
    }

    switch (next->getType()) {
        case STRING:
            return parseString();
        case NUMBER:
            consumeLexeme(NUMBER);
            return new JsonishNumber(utils::strtof_c(next->getStart(), nullptr));
        case BLOCK_START:
            return parseObject();
        case ARRAY_START:
            return parseArray();
        case BOOLEAN:
            consumeLexeme(BOOLEAN);
            return new JsonishBool(!strncmp(next->getStart(), "true", next->getSize()));
        case NUll:
            consumeLexeme(NUll);
            return new JsonishNull();
        default:
            setInvalidArgumentError("unexpected token");
            return nullptr;
    }
}

void JsonishParser::setInvalidArgumentError(const char* message) noexcept {
    // Already encountered an error.
    if (!mStatus.isOk()) {
        return;
    }

    utils::io::sstream errorMessage;
    errorMessage << "JsonishParser error" << utils::io::endl;
    errorMessage << "Syntax error, " << message;

    const JsonLexeme* lexeme = peekNextLexemeType();
    if (lexeme == nullptr) {
        lexeme = peekCurrentLexemeType();
    }

    if (lexeme != nullptr) {
        errorMessage << " at line:" << lexeme->getLine()
          << " position:" << lexeme->getLinePosition() << utils::io::endl
          << "  got lexeme type: "
          << JsonLexeme::getTypeString(lexeme->getType()) << utils::io::endl
          << "  got lexeme value: " << lexeme->getStringValue() << utils::io::endl;
    } else {
        errorMessage << " but reached end of file instead." << utils::io::endl;
    }

    mStatus = utils::Status::invalidArgument(errorMessage.c_str());
}

} // namespace matp
