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

#include "MaterialCache.h"

#include <backend/DriverEnums.h>

#include <details/Engine.h>
#include <details/Material.h>

#include <utils/Hash.h>
#include <utils/Logger.h>

namespace filament {

size_t ProgramCache::Specialization::Hash::operator()(
        ProgramCache::Specialization const& value) const noexcept {
    size_t seed = 0;
    utils::hash::combine_fast(seed, value.variant.key);
    for (auto const& it: value.specializationConstants) {
        utils::hash::combine_fast(seed, it.id);
        utils::hash::combine_fast(seed, it.value);
    }
    for (auto const& set: value.pushConstants) {
        for (auto const& it: set) {
            utils::hash::combine_fast(seed, utils::CString::Hasher{}(it.name));
            utils::hash::combine_fast(seed, it.type);
        }
    }
    return seed;
}

bool ProgramCache::Specialization::operator==(
        ProgramCache::Specialization const& rhs) const noexcept {
    return variant == rhs.variant &&
            specializationConstants == rhs.specializationConstants &&
            pushConstants == rhs.pushConstants;
}

ProgramCache::ReturnValue ProgramCache::hold(
        Specialization const& specialization,
        Material const& material,
        backend::CompilerPriorityQueue const priorityQueue) noexcept {
    return mCache.hold(specialization, [specialization, &material, priorityQueue]() {
        return downcast(material).makeProgram(specialization, priorityQueue);
    });
}

ProgramCache::Program& ProgramCache::leak(
        Specialization const& specialization,
        Material const& material,
        backend::CompilerPriorityQueue const priorityQueue) noexcept {
    return mCache.leak(specialization, [specialization, &material, priorityQueue]() {
        return downcast(material).makeProgram(specialization, priorityQueue);
    });
}

ProgramCache::Program const& ProgramCache::peek(
        Specialization const& specialization) const noexcept {
    return mCache.peek(specialization);
}

} // namespace filament
