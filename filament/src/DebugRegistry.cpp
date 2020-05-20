/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "details/DebugRegistry.h"

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#ifndef NDEBUG
#   define DEBUG_PROPERTIES_WRITABLE true
#else
#   define DEBUG_PROPERTIES_WRITABLE false
#endif

using namespace filament::math;
using namespace utils;

namespace filament {

FDebugRegistry::FDebugRegistry() noexcept {
    mProperties.reserve(8);
}

UTILS_NOINLINE
void *FDebugRegistry::getPropertyAddress(const char *name) noexcept {
    StaticString key = StaticString::make(name, strlen(name));
    auto &propertyMap = mPropertyMap;
    if (propertyMap.find(key) == propertyMap.end()) {
        return nullptr;
    }
    return propertyMap[key];
}

void FDebugRegistry::registerProperty(utils::StaticString name, void *p, Type type) noexcept {
    auto& propertyMap = mPropertyMap;
    if (propertyMap.find(name) == propertyMap.end()) {
        mProperties.push_back({ name.c_str(), type });
        propertyMap[name] = p;
    }
}

inline std::pair<DebugRegistry::Property const *, size_t> FDebugRegistry::getProperties() const noexcept {
    return {mProperties.data(), mProperties.size()};
}

inline bool FDebugRegistry::hasProperty(const char *name) const noexcept {
    return const_cast<FDebugRegistry *>(this)->getPropertyAddress(name) != nullptr;
}

template<typename T>
inline bool FDebugRegistry::setProperty(const char *name, T v) noexcept {
    if (DEBUG_PROPERTIES_WRITABLE) {
        T * const addr = static_cast<T *>(getPropertyAddress(name));
        if (addr) {
            *addr = v;
            return true;
        }
    }
    return false;
}

template <typename T>
inline bool FDebugRegistry::getProperty(const char* name, T* UTILS_RESTRICT p) const noexcept {
    T const * const addr = static_cast<T *>(const_cast<FDebugRegistry *>(this)->getPropertyAddress(name));
    if (addr) {
        *p = *addr;
        return true;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

std::pair<DebugRegistry::Property const*, size_t> DebugRegistry::getProperties() const noexcept {
    return upcast(this)->getProperties();
}

bool DebugRegistry::hasProperty(const char* name) const noexcept {
    return upcast(this)->hasProperty(name);
}

bool DebugRegistry::setProperty(const char* name, bool v) noexcept {
    return upcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, int v) noexcept {
    return upcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, float v) noexcept {
    return upcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, float2 v) noexcept {
    return upcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, float3 v) noexcept {
    return upcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, float4 v) noexcept {
    return upcast(this)->setProperty(name, v);
}


bool DebugRegistry::getProperty(const char* name, bool* v) const noexcept {
    return upcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, int* v) const noexcept {
    return upcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, float* v) const noexcept {
    return upcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, float2* v) const noexcept {
    return upcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, float3* v) const noexcept {
    return upcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, float4* v) const noexcept {
    return upcast(this)->getProperty(name, v);
}

void *DebugRegistry::getPropertyAddress(const char *name) noexcept {
    return  upcast(this)->getPropertyAddress(name);
}

} // namespace filament

