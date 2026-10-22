/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include "ProgramSpecialization.h"

#include <utils/Hash.h>

namespace filament {

size_t ProgramSpecialization::hash() const noexcept {
    size_t seed = 0;
    utils::hash::combine_fast(seed, materialCrc32);
    utils::hash::combine_fast(seed, variant.key);
    utils::hash::combine_fast(seed, specKey.key);
    // Both specKey and dynamic constants (indices 16-31) track the same dynamic state.
    // Direct slice hashing is safe and fast because indices 16-31 are unmutated defaults during cache lookup.
    utils::hash::combine_fast(seed, specializationConstants.hash());
    return seed;
}

bool ProgramSpecialization::operator==(ProgramSpecialization const& rhs) const noexcept {
    if (this == &rhs) {
        return true;
    }
    // Both specKey and dynamic constants track the same state. Direct slice comparison
    // safely avoids element-wise loop overhead because dynamic constants are unmutated defaults here.
    return materialCrc32 == rhs.materialCrc32 && variant == rhs.variant &&
           specKey == rhs.specKey &&
           specializationConstants == rhs.specializationConstants;
}

} // namespace filament
