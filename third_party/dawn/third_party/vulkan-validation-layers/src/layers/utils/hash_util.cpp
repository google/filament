/* Copyright (c) 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// xxhash is a STB library: https://github.com/Cyan4973/xxHash/tree/v0.8.2#build-modifiers

#include "hash_util.h"

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION

// Only include xxhash.h once!
#include "external/xxhash.h"

// Currently using version v0.8.2 of xxhash
static_assert(XXH_VERSION_MAJOR == 0);
static_assert(XXH_VERSION_MINOR == 8);
static_assert(XXH_VERSION_RELEASE == 2);

namespace hash_util {

uint32_t VuidHash(std::string_view vuid) {
    constexpr uint32_t seed = 8;
    return XXH32(vuid.data(), vuid.size(), seed);
}

uint32_t ShaderHash(const void *pCode, const size_t codeSize) {
    constexpr uint32_t seed = 0;
    return XXH32(pCode, codeSize, seed);
}

uint64_t DescriptorVariableHash(const void *info, const size_t info_size) {
    constexpr uint64_t seed = 0;
    return XXH64(info, info_size, seed);
}

}  // namespace hash_util
