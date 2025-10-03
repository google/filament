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
#ifndef TNT_FILAMENT_PROGRAMSPECIALIZATION_H
#define TNT_FILAMENT_PROGRAMSPECIALIZATION_H

#include <backend/Program.h>

#include <private/filament/Variant.h>

#include <utils/FixedCapacityVector.h>

#include <cstdint>

namespace filament {

// A program specialization is a collection of all properties which could yield a different compiled
// program object.
struct ProgramSpecialization {
    uint64_t programCacheId;
    Variant variant;
    utils::Slice<const backend::Program::SpecializationConstant> specializationConstants;

    size_t hash() const noexcept;
    bool operator==(ProgramSpecialization const& rhs) const noexcept;
};

} // namespace filament

namespace std {
template<>
struct hash<filament::ProgramSpecialization> {
    inline size_t operator()(const filament::ProgramSpecialization& lhs) const noexcept {
        return lhs.hash();
    }
};
} // namespace std

#endif  // TNT_FILAMENT_PROGRAMSPECIALIZATION_H
