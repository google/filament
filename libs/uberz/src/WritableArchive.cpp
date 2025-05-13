/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <uberz/WritableArchive.h>
#include <uberz/ReadableArchive.h>

#include <zstd.h>

#include <string_view>

#include <utils/Log.h>
#include <utils/Panic.h>

using namespace utils;
using namespace std::literals;

using std::string_view;

namespace filament::uberz {

static bool isAlphaNumeric(char c) {
    return (c >= '0' && c <= '9') || c == '_'
        || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isWhitespace(char c) {
    return c == ' ' || c == '\t';
}

// Returns 0 if stringToScan does not start with prefix, otherwise returns length of prefix.
static size_t readPrefix(string_view prefix, string_view stringToScan) {
    const size_t prefixSize = prefix.size();
    if (UTILS_UNLIKELY(stringToScan.size() < prefixSize)) {
        return 0;
    }
    return prefix == string_view { stringToScan.data(), prefixSize } ? prefixSize : 0;
}

static string_view readArchiveFeature(string_view cursor, ArchiveFeature* feature) {
    size_t sz;
    if (sz = readPrefix("unsupported"sv, cursor); sz > 0) {
        *feature = ArchiveFeature::UNSUPPORTED;
    } else if (sz = readPrefix("required"sv, cursor); sz > 0) {
        *feature = ArchiveFeature::REQUIRED;
    } else if (sz = readPrefix("optional"sv, cursor); sz > 0) {
        *feature = ArchiveFeature::OPTIONAL;
    }
    return { cursor.data(), sz };
}

static string_view readBlendingMode(string_view cursor, BlendingMode* blending) {
    size_t sz;
    if (sz = readPrefix("opaque"sv, cursor); sz > 0) {
        *blending = BlendingMode::OPAQUE;
    } else if (sz = readPrefix("transparent"sv, cursor); sz > 0) {
        *blending = BlendingMode::TRANSPARENT;
    } else if (sz = readPrefix("add"sv, cursor); sz > 0) {
        *blending = BlendingMode::ADD;
    } else if (sz = readPrefix("masked"sv, cursor); sz > 0) {
        *blending = BlendingMode::MASKED;
    } else if (sz = readPrefix("fade"sv, cursor); sz > 0) {
        *blending = BlendingMode::FADE;
    } else if (sz = readPrefix("multiply"sv, cursor); sz > 0) {
        *blending = BlendingMode::MULTIPLY;
    } else if (sz = readPrefix("screen"sv, cursor); sz > 0) {
        *blending = BlendingMode::SCREEN;
    } else if (sz = readPrefix("custom"sv, cursor); sz > 0) {
        *blending = BlendingMode::CUSTOM;
    }
    return { cursor.data(), sz };
}

static string_view readShadingModel(string_view cursor, Shading* shading) {
    size_t sz;
    if (sz = readPrefix("unlit"sv, cursor); sz > 0) {
        *shading = Shading::UNLIT;
    } else if (sz = readPrefix("lit"sv, cursor); sz > 0) {
        *shading = Shading::LIT;
    } else if (sz = readPrefix("subsurface"sv, cursor); sz > 0) {
        *shading = Shading::SUBSURFACE;
    } else if (sz = readPrefix("cloth"sv, cursor); sz > 0) {
        *shading = Shading::CLOTH;
    } else if (sz = readPrefix("specularGlossiness"sv, cursor); sz > 0) {
        *shading = Shading::SPECULAR_GLOSSINESS;
    }
    return { cursor.data(), sz };
}

static string_view readSymbol(string_view cursor, char symbol)  {
    return { cursor.data(), (!cursor.empty() && cursor[0] == symbol) ? 1ul : 0ul };
}

static string_view readIdentifier(string_view cursor)  {
    size_t i = 0;
    while (i < cursor.size() && isAlphaNumeric(cursor[i])) i++;
    return { cursor.data(), i };
}

static string_view readWhitespace(string_view cursor) {
    size_t i = 0;
    while (i < cursor.size() && isWhitespace(cursor[i])) i++;
    return { cursor.data(), i };
}

void WritableArchive::addMaterial(const char* name, const uint8_t* package, size_t packageSize) {
    Material& material = mMaterials[++mMaterialIndex];

    material = {
        CString(name),
        FixedCapacityVector<uint8_t>(packageSize)
    };
    memcpy(material.package.data(), package, packageSize);

    // We use invalid values to denote "not set". e.g. if the spec file
    // does not set the blend mode, then it can be used for any blend mode.
    material.blendingMode = INVALID_BLENDING;
    material.shadingModel = INVALID_SHADING_MODEL;

    mLineNumber = 1;
}

void WritableArchive::addSpecLine(string_view line) {
    assert_invariant(mMaterialIndex > -1);
    Material& material = mMaterials[mMaterialIndex];

    string_view cursor = line;
    string_view token;

    // Don't try to recover, just emit the error message and quit.
    auto emitSyntaxError = [&](const char* msg) {
        const int column = 1 + line.size() - cursor.size();
        PANIC_POSTCONDITION("%s.spec(%d,%d): %s", material.name.c_str(), mLineNumber, column, msg);
    };

    // Advance the cursor forward each time a token is consumed.
    auto advanceCursorByToken = [&]() {
        cursor = { cursor.data() + token.size(), cursor.size() - token.size() };
    };

    // Sometimes we need to advance the cursor forward by a fixed amount.
    auto advanceCursorByCount = [&](size_t count) {
        cursor = { cursor.data() + count, cursor.size() - count };
    };

    auto parseFeatureFlag = [&]() {
        if (token = readIdentifier(cursor); token.empty()) {
            emitSyntaxError("expected identifier");
        }
        advanceCursorByToken();
        CString id(token.data(), token.size());
        token = readWhitespace(cursor);
        advanceCursorByToken();
        if (token = readSymbol(cursor, '='); token.empty()) {
            emitSyntaxError("expected equal sign");
        }
        advanceCursorByToken();
        token = readWhitespace(cursor);
        advanceCursorByToken();
        if (token = readArchiveFeature(cursor, &material.flags[id]); token.empty()) {
            emitSyntaxError("expected unsupported / optional / required");
        }
        advanceCursorByToken();
    };

    auto parseBlendingMode = [&]() {
        token = readWhitespace(cursor);
        advanceCursorByToken();
        if (token = readSymbol(cursor, '='); token.empty()) {
            emitSyntaxError("expected equal sign");
        }
        advanceCursorByToken();
        token = readWhitespace(cursor);
        advanceCursorByToken();
        if (token = readBlendingMode(cursor, &material.blendingMode); token.empty()) {
            emitSyntaxError("expected lowercase blending mode enum");
        }
        advanceCursorByToken();
    };

    auto parseShadingModel = [&]() {
        token = readWhitespace(cursor);
        advanceCursorByToken();
        if (token = readSymbol(cursor, '='); token.empty()) {
            emitSyntaxError("expected equal sign");
        }
        advanceCursorByToken();
        token = readWhitespace(cursor);
        advanceCursorByToken();
        if (token = readShadingModel(cursor, &material.shadingModel); token.empty()) {
            emitSyntaxError("expected lowercase shading enum");
        }
        advanceCursorByToken();
    };

    if (cursor.empty() || cursor[0] == '#') {
        ++mLineNumber;
        return;
    }

    if (size_t sz = readPrefix("BlendingMode"sv, cursor); sz > 0) {
        advanceCursorByCount(sz);
        parseBlendingMode();
    } else if (size_t sz = readPrefix("ShadingModel"sv, cursor); sz > 0) {
        advanceCursorByCount(sz);
        parseShadingModel();
    } else {
        parseFeatureFlag();
    }

    if (!cursor.empty()) {
        emitSyntaxError("unexpected trailing character");
    }

    ++mLineNumber;
}

FixedCapacityVector<uint8_t> WritableArchive::serialize() const {
    size_t byteCount = sizeof(ReadableArchive);
    for (const auto& mat : mMaterials) {
        byteCount += sizeof(ArchiveSpec);
    }
    size_t flaglistOffset = byteCount;
    for (const auto& mat : mMaterials) {
        for (const auto& pair : mat.flags) {
            byteCount += sizeof(ArchiveFlag);
        }
    }
    size_t nameOffset = byteCount;
    for (const auto& mat : mMaterials) {
        for (const auto& pair : mat.flags) {
            byteCount += pair.first.size() + 1;
        }
    }
    size_t filamatOffset = byteCount;
    for (const auto& mat : mMaterials) {
        byteCount += mat.package.size();
    }

    ReadableArchive archive;
    archive.magic = 'UBER';
    archive.version = 0;
    archive.specsCount = mMaterials.size();
    archive.specsOffset = sizeof(ReadableArchive);

    auto specs = FixedCapacityVector<ArchiveSpec>::with_capacity(mMaterials.size());
    size_t flagCount = 0;
    for (const auto& mat : mMaterials) {
        ArchiveSpec spec = {};
        spec.shadingModel = mat.shadingModel;
        spec.blendingMode = mat.blendingMode;
        spec.flagsCount = mat.flags.size();
        spec.flagsOffset = flaglistOffset + flagCount * sizeof(ArchiveFlag);
        spec.packageByteCount = mat.package.size();
        spec.packageOffset = filamatOffset;
        specs.push_back(spec);
        filamatOffset += mat.package.size();
        flagCount += mat.flags.size();
    }

    auto flags = FixedCapacityVector<ArchiveFlag>::with_capacity(flagCount);
    size_t charCount = 0;
    for (const auto& mat : mMaterials) {
        for (const auto& pair : mat.flags) {
            ArchiveFlag flag;
            flag.nameOffset = nameOffset + charCount;
            flag.value = pair.second;
            charCount += pair.first.size() + 1;
            flags.push_back(flag);
        }
    }

    std::string flagNames(charCount, ' ');
    char* flagNamesPtr = flagNames.data();
    for (const auto& mat : mMaterials) {
        for (const auto& pair : mat.flags) {
            memcpy(flagNamesPtr, pair.first.data(), pair.first.size() + 1);
            flagNamesPtr += pair.first.size() + 1;
        }
    }
    assert(flagNamesPtr - flagNames.data() == flagNames.size());

    FixedCapacityVector<uint8_t> outputBuf(byteCount);
    uint8_t* writeCursor = outputBuf.data();
    memcpy(writeCursor, &archive, sizeof(archive));
    writeCursor += sizeof(archive);
    memcpy(writeCursor, specs.data(), sizeof(ArchiveSpec) * specs.size());
    writeCursor += sizeof(ArchiveSpec) * specs.size();
    memcpy(writeCursor, flags.data(), sizeof(ArchiveFlag) * flags.size());
    writeCursor += sizeof(ArchiveFlag) * flags.size();
    memcpy(writeCursor, flagNames.data(), charCount);
    writeCursor += charCount;
    for (const auto& mat : mMaterials) {
        memcpy(writeCursor, mat.package.data(), mat.package.size());
        writeCursor += mat.package.size();
    }
    assert_invariant(writeCursor - outputBuf.data() == outputBuf.size());

    FixedCapacityVector<uint8_t> compressedBuf(ZSTD_compressBound(outputBuf.size()));

    // Maximum zstd compression is slow, but that's okay since uberz is invoked during the build,
    // not at run time.  However in debug builds it is debilitatingly slow, and we're fine with
    // larger archives, so we use minimum compression.
#ifdef NDEBUG
    const int compressionLevel = ZSTD_maxCLevel();
#else
    const int compressionLevel = ZSTD_minCLevel();
#endif

    size_t zstdResult = ZSTD_compress(compressedBuf.data(), compressedBuf.size(), outputBuf.data(),
            outputBuf.size(), compressionLevel);
    if (ZSTD_isError(zstdResult)) {
        PANIC_POSTCONDITION("Error during archive compression: %s", ZSTD_getErrorName(zstdResult));
    }

    compressedBuf.resize(zstdResult);
    return compressedBuf;
}

void WritableArchive::setShadingModel(Shading sm) {
    assert_invariant(mMaterialIndex > -1);
    mMaterials[mMaterialIndex].shadingModel = sm;
}

void WritableArchive::setBlendingModel(BlendingMode bm) {
    assert_invariant(mMaterialIndex > -1);
    mMaterials[mMaterialIndex].blendingMode = bm;
}

void WritableArchive::setFeatureFlag(const char* key, ArchiveFeature value) {
    assert_invariant(mMaterialIndex > -1);
    mMaterials[mMaterialIndex].flags[CString(key)] = value;
}

} // namespace filament::uberz
