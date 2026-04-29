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

#include <uberz/ReadableArchive.h>

#include <utils/Logger.h>

#include <cstring>
#include <limits>

using namespace filament;
using namespace utils;

namespace filament::uberz {

static_assert(sizeof(ReadableArchive) == 4 + 4 + 8 + 8);
static_assert(sizeof(ArchiveSpec) == 1 + 1 + 2 + 4 + 8 + 8);
static_assert(sizeof(ArchiveFlag) == 8 + 8);

namespace {

constexpr uint32_t READABLE_ARCHIVE_MAGIC = 'UBER';
constexpr uint32_t READABLE_ARCHIVE_VERSION = 0;
constexpr size_t WORD_SIZE = sizeof(uint64_t);

bool checkedMultiply(uint64_t count, size_t itemSize, size_t& result) {
    if (count > std::numeric_limits<size_t>::max() / itemSize) {
        return false;
    }
    result = size_t(count) * itemSize;
    return true;
}

uint8_t* checkedOffset(ReadableArchive* archive, size_t archiveSize,
        uint64_t offset, size_t length) {
    if (offset > archiveSize || length > archiveSize - size_t(offset)) {
        return nullptr;
    }
    return reinterpret_cast<uint8_t*>(archive) + size_t(offset);
}

const char* checkedCString(ReadableArchive* archive, size_t archiveSize,
        uint64_t offset) {
    if (offset >= archiveSize) {
        return nullptr;
    }
    const char* string = reinterpret_cast<const char*>(archive) + size_t(offset);
    if (memchr(string, 0, archiveSize - size_t(offset)) == nullptr) {
        return nullptr;
    }
    return string;
}

} // namespace

bool convertOffsetsToPointers(ReadableArchive* archive, size_t archiveSize) {
    if (archiveSize < sizeof(ReadableArchive)) {
        LOG(ERROR) << "Uberz archive header is truncated";
        return false;
    }
    if (archive->magic != READABLE_ARCHIVE_MAGIC) {
        LOG(ERROR) << "Uberz archive has invalid magic";
        return false;
    }
    if (archive->version != READABLE_ARCHIVE_VERSION) {
        LOG(ERROR) << "Uberz archive has unsupported version";
        return false;
    }
    if (archive->specsOffset % WORD_SIZE != 0) {
        LOG(ERROR) << "Uberz specs offset is misaligned";
        return false;
    }

    size_t specsSize;
    if (!checkedMultiply(archive->specsCount, sizeof(ArchiveSpec), specsSize)) {
        LOG(ERROR) << "Uberz specs array size overflows";
        return false;
    }
    
    archive->specs = reinterpret_cast<ArchiveSpec*>(checkedOffset(archive, archiveSize,
            archive->specsOffset, specsSize));
    if (!archive->specs) {
        LOG(ERROR) << "Uberz specs array exceeds buffer";
        return false;
    }

    for (uint64_t i = 0; i < archive->specsCount; ++i) {
        ArchiveSpec& spec = archive->specs[i];
        if (spec.flagsOffset % WORD_SIZE != 0) {
            LOG(ERROR) << "Uberz flags offset is misaligned";
            return false;
        }
        
        size_t flagsSize;
        if (!checkedMultiply(spec.flagsCount, sizeof(ArchiveFlag), flagsSize)) {
            LOG(ERROR) << "Uberz flags array size overflows";
            return false;
        }
        
        spec.flags = reinterpret_cast<ArchiveFlag*>(checkedOffset(archive, archiveSize,
                spec.flagsOffset, flagsSize));
        if (!spec.flags) {
            LOG(ERROR) << "Uberz flags array exceeds buffer";
            return false;
        }
        
        spec.package = checkedOffset(archive, archiveSize, spec.packageOffset,
                spec.packageByteCount);
        if (!spec.package) {
            LOG(ERROR) << "Uberz package exceeds buffer";
            return false;
        }
        
        for (uint64_t j = 0; j < spec.flagsCount; ++j) {
            ArchiveFlag& flag = spec.flags[j];
            flag.name = checkedCString(archive, archiveSize, flag.nameOffset);
            if (!flag.name) {
                LOG(ERROR) << "Uberz flag name exceeds buffer or is not NUL-terminated";
                return false;
            }
        }
    }
    return true;
}

} // namespace filament::uberz
