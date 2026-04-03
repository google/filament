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

#include <utils/Panic.h>

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

size_t checkedMultiply(uint64_t count, size_t itemSize, const char* reason) {
    FILAMENT_CHECK_PRECONDITION(count <= std::numeric_limits<size_t>::max() / itemSize)
            << reason;
    return size_t(count) * itemSize;
}

uint8_t* checkedOffset(ReadableArchive* archive, size_t archiveSize,
        uint64_t offset, size_t length, const char* reason) {
    FILAMENT_CHECK_PRECONDITION(offset <= archiveSize && length <= archiveSize - size_t(offset))
            << reason;
    return reinterpret_cast<uint8_t*>(archive) + size_t(offset);
}

const char* checkedCString(ReadableArchive* archive, size_t archiveSize,
        uint64_t offset, const char* reason) {
    FILAMENT_CHECK_PRECONDITION(offset < archiveSize) << reason;
    const char* string = reinterpret_cast<const char*>(archive) + size_t(offset);
    FILAMENT_CHECK_PRECONDITION(memchr(string, 0, archiveSize - size_t(offset)) != nullptr)
            << reason;
    return string;
}

} // namespace

void convertOffsetsToPointers(ReadableArchive* archive, size_t archiveSize) {
    FILAMENT_CHECK_PRECONDITION(archiveSize >= sizeof(ReadableArchive))
            << "Uberz archive header is truncated";
    FILAMENT_CHECK_PRECONDITION(archive->magic == READABLE_ARCHIVE_MAGIC)
            << "Uberz archive has invalid magic";
    FILAMENT_CHECK_PRECONDITION(archive->version == READABLE_ARCHIVE_VERSION)
            << "Uberz archive has unsupported version";
    FILAMENT_CHECK_PRECONDITION(archive->specsOffset % WORD_SIZE == 0)
            << "Uberz specs offset is misaligned";

    const size_t specsSize = checkedMultiply(archive->specsCount, sizeof(ArchiveSpec),
            "Uberz specs array size overflows");
    archive->specs = reinterpret_cast<ArchiveSpec*>(checkedOffset(archive, archiveSize,
            archive->specsOffset, specsSize, "Uberz specs array exceeds buffer"));

    for (uint64_t i = 0; i < archive->specsCount; ++i) {
        ArchiveSpec& spec = archive->specs[i];
        FILAMENT_CHECK_PRECONDITION(spec.flagsOffset % WORD_SIZE == 0)
                << "Uberz flags offset is misaligned";
        const size_t flagsSize = checkedMultiply(spec.flagsCount, sizeof(ArchiveFlag),
                "Uberz flags array size overflows");
        spec.flags = reinterpret_cast<ArchiveFlag*>(checkedOffset(archive, archiveSize,
                spec.flagsOffset, flagsSize, "Uberz flags array exceeds buffer"));
        spec.package = checkedOffset(archive, archiveSize, spec.packageOffset,
                spec.packageByteCount, "Uberz package exceeds buffer");
        for (uint64_t j = 0; j < spec.flagsCount; ++j) {
            ArchiveFlag& flag = spec.flags[j];
            flag.name = checkedCString(archive, archiveSize, flag.nameOffset,
                    "Uberz flag name exceeds buffer");
        }
    }
}

} // namespace filament::uberz
