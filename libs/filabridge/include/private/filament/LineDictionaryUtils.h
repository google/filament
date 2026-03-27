/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file law. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_PRIVATE_LINEDICTIONARYUTILS_H
#define TNT_FILAMENT_PRIVATE_LINEDICTIONARYUTILS_H

#include <stddef.h>
#include <stdint.h>

namespace filament {

class LineDictionaryUtils {
public:
    // Base markers
    static constexpr uint8_t DICTIONARY_1_BYTE_ID_MAX = 240;
    static constexpr uint8_t DICTIONARY_ESCAPE_BASE = 254;

    static constexpr uint8_t DICTIONARY_NUMERIC_ID = DICTIONARY_ESCAPE_BASE;
    static constexpr uint8_t DICTIONARY_3_BYTE_ID = DICTIONARY_ESCAPE_BASE + 1; // 255

    // Calculated boundaries
    static constexpr size_t DICTIONARY_2_BYTE_ID_PREFIX_COUNT = DICTIONARY_ESCAPE_BASE - DICTIONARY_1_BYTE_ID_MAX; // 14
    static constexpr size_t DICTIONARY_1_BYTE_ID_CAPACITY = DICTIONARY_1_BYTE_ID_MAX; // 240
    static constexpr size_t DICTIONARY_2_BYTE_ID_CAPACITY = DICTIONARY_2_BYTE_ID_PREFIX_COUNT << 8; // 3584
    static constexpr size_t DICTIONARY_2_BYTE_ID_MAX = DICTIONARY_1_BYTE_ID_CAPACITY + DICTIONARY_2_BYTE_ID_CAPACITY; // 3824

    // Numerical Mapping Flag
    static constexpr uint32_t DICTIONARY_NUMERIC_FLAG = 0x40000000;

    // Structured Outputs
    struct Pack2ByteResult {
        uint8_t prefix;
        uint8_t ext;
    };

    struct Pack3ByteResult {
        uint8_t extb0;
        uint8_t extb1;
    };

    // ------------------------------------------------------------------------------------------------
    // Inline Encoders & Decoders
    // ------------------------------------------------------------------------------------------------

    static inline uint32_t unpack2ByteDictionaryId(uint8_t prefixb8, uint8_t extb0) noexcept {
        return static_cast<uint32_t>(DICTIONARY_1_BYTE_ID_CAPACITY + (((prefixb8 - DICTIONARY_1_BYTE_ID_MAX) << 8) | extb0));
    }

    static inline uint32_t unpack3ByteDictionaryId(uint8_t extb0, uint8_t extb1) noexcept {
        return static_cast<uint32_t>(DICTIONARY_2_BYTE_ID_MAX + (extb0 | (extb1 << 8)));
    }

    static inline Pack2ByteResult pack2ByteDictionaryId(uint32_t global_index) noexcept {
        uint32_t const rel = global_index - DICTIONARY_1_BYTE_ID_CAPACITY;
        return {
            static_cast<uint8_t>(DICTIONARY_1_BYTE_ID_MAX + (rel >> 8)),
            static_cast<uint8_t>(rel & 0xFF)
        };
    }

    static inline Pack3ByteResult pack3ByteDictionaryId(uint32_t global_index) noexcept {
        uint32_t const rel = global_index - DICTIONARY_2_BYTE_ID_MAX;
        return {
            static_cast<uint8_t>(rel & 0xFF),
            static_cast<uint8_t>((rel >> 8) & 0xFF)
        };
    }
};

} // namespace filament

#endif // TNT_FILAMENT_PRIVATE_LINEDICTIONARYUTILS_H
