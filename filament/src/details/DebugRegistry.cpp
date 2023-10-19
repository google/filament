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

#include <utils/Panic.h>

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

FDebugRegistry::FDebugRegistry() noexcept = default;

auto FDebugRegistry::getPropertyInfo(const char* name) noexcept -> PropertyInfo {
    std::string_view const key{ name };
    auto& propertyMap = mPropertyMap;
    if (propertyMap.find(key) == propertyMap.end()) {
        return { nullptr, {} };
    }
    return propertyMap[key];
}

UTILS_NOINLINE
void* FDebugRegistry::getPropertyAddress(const char* name) {
    auto info = getPropertyInfo(name);
    ASSERT_PRECONDITION_NON_FATAL(!info.second,
            "don't use DebugRegistry::getPropertyAddress() when a callback is set. "
            "Use setProperty() instead.");
    return info.first;
}

UTILS_NOINLINE
void const* FDebugRegistry::getPropertyAddress(const char* name) const noexcept {
    auto info = const_cast<FDebugRegistry*>(this)->getPropertyInfo(name);
    return info.first;
}

void FDebugRegistry::registerProperty(std::string_view name, void* p, Type,
        std::function<void()> fn) noexcept {
    auto& propertyMap = mPropertyMap;
    if (propertyMap.find(name) == propertyMap.end()) {
        propertyMap[name] = { p, std::move(fn) };
    }
}

bool FDebugRegistry::hasProperty(const char* name) const noexcept {
    return getPropertyAddress(name) != nullptr;
}

template<typename T>
bool FDebugRegistry::setProperty(const char* name, T v) noexcept {
    if constexpr (DEBUG_PROPERTIES_WRITABLE) {
        auto info = getPropertyInfo(name);
        T* const addr = static_cast<T*>(info.first);
        if (addr) {
            auto old = *addr;
            *addr = v;
            if (info.second && old != v) {
                info.second();
            }
            return true;
        }
    }
    return false;
}

template bool FDebugRegistry::setProperty<bool>(const char* name, bool v) noexcept;
template bool FDebugRegistry::setProperty<int>(const char* name, int v) noexcept;
template bool FDebugRegistry::setProperty<float>(const char* name, float v) noexcept;
template bool FDebugRegistry::setProperty<float2>(const char* name, float2 v) noexcept;
template bool FDebugRegistry::setProperty<float3>(const char* name, float3 v) noexcept;
template bool FDebugRegistry::setProperty<float4>(const char* name, float4 v) noexcept;

template<typename T>
bool FDebugRegistry::getProperty(const char* name, T* p) const noexcept {
    T const* const addr = static_cast<T const*>(getPropertyAddress(name));
    if (addr) {
        *p = *addr;
        return true;
    }
    return false;
}

template bool FDebugRegistry::getProperty<bool>(const char* name, bool* v) const noexcept;
template bool FDebugRegistry::getProperty<int>(const char* name, int* v) const noexcept;
template bool FDebugRegistry::getProperty<float>(const char* name, float* v) const noexcept;
template bool FDebugRegistry::getProperty<float2>(const char* name, float2* v) const noexcept;
template bool FDebugRegistry::getProperty<float3>(const char* name, float3* v) const noexcept;
template bool FDebugRegistry::getProperty<float4>(const char* name, float4* v) const noexcept;

void FDebugRegistry::registerDataSource(std::string_view name,
        void const* data, size_t count) noexcept {
    auto& dataSourceMap = mDataSourceMap;
    if (dataSourceMap.find(name) == dataSourceMap.end()) {
        dataSourceMap[name] = { data, count };
    }
}

DebugRegistry::DataSource FDebugRegistry::getDataSource(const char* name) const noexcept {
    std::string_view const key{ name };
    auto& dataSourceMap = mDataSourceMap;
    auto const& it = dataSourceMap.find(key);
    if (it == dataSourceMap.end()) {
        return { nullptr, 0u };
    }
    return it->second;
}

} // namespace filament
