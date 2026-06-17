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

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/Invocable.h>
#include <utils/Logger.h>
#include <utils/Panic.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <algorithm>
#include <array>
#include <functional>
#include <string_view>
#include <utility>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace filament::math;
using namespace utils;

namespace filament {

namespace {

constexpr size_t MAX_PROPERTY_NAME_LENGTH = 128;
constexpr size_t MAX_PROPERTY_VALUE_LENGTH = 128;

template<typename T>
bool parse(const char* val, T& out) { return false; }

template<>
bool parse(const char* val, bool& out) {
    std::string_view const value(val);
    if (value == "1" || value == "true") { out = true; return true; }
    if (value == "0" || value == "false") { out = false; return true; }
    return false;
}

template<>
bool parse(const char* val, int& out) { return sscanf(val, "%d", &out) == 1; }
template<>
bool parse(const char* val, float& out) { return sscanf(val, "%f", &out) == 1; }
template<>
bool parse(const char* val, float2& out) { return sscanf(val, "%f %f", &out.x, &out.y) == 2; }
template<>
bool parse(const char* val, float3& out) { return sscanf(val, "%f %f %f", &out.x, &out.y, &out.z) == 3; }
template<>
bool parse(const char* val, float4& out) { return sscanf(val, "%f %f %f %f", &out.x, &out.y, &out.z, &out.w) == 4; }

template<typename T>
bool update(void* p, const char* val) {
    T v;
    if (parse(val, v)) {
        T& current = *static_cast<T*>(p);
        if (current != v) {
            current = v;
            return true;
        }
    }
    return false;
}

#ifdef __ANDROID__
std::string_view getPropertyFromEnvironment(char const* const name, size_t nameLen,
        std::array<char, MAX_PROPERTY_VALUE_LENGTH>& storage) {
    // since API 26 there is no limit to the property name
    char propertyName[MAX_PROPERTY_NAME_LENGTH];
    snprintf(propertyName, MAX_PROPERTY_NAME_LENGTH, "debug.%s", name);
    int const length = __system_property_get(propertyName, storage.data());
    if (length > 0) {
        return { storage.data(), size_t(length) };
    }
    return {};
}
#else
std::string_view getPropertyFromEnvironment(char const* const name, size_t nameLen,
        std::array<char, MAX_PROPERTY_VALUE_LENGTH>&) {
    utils::CString cname {name, nameLen };
    std::replace(cname.data(), cname.data() + nameLen, '.', '_');
    char const* const env = getenv(cname.data());
    return env ? std::string_view{ env } : std::string_view{};
}
#endif

} // namespace

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

void FDebugRegistry::registerProperty(std::string_view const name, void* p, Type type,
        std::function<void()> fn) noexcept {
    auto& propertyMap = mPropertyMap;
    if (propertyMap.find(name) == propertyMap.end()) {
        propertyMap[name] = { p, std::move(fn) };

        std::array<char, MAX_PROPERTY_VALUE_LENGTH> storage;
        char propertyName[MAX_PROPERTY_NAME_LENGTH];
        size_t const len = std::min(name.size(), sizeof(propertyName) - 1);
        memcpy(propertyName, name.data(), len);
        propertyName[len] = '\0';

        std::string_view const value = getPropertyFromEnvironment(propertyName, len, storage);
        if (!value.empty()) {
            char const* val = value.data();
            bool changed = false;
            switch (type) {
                case BOOL:   changed = update<bool>(p, val);   break;
                case INT:    changed = update<int>(p, val);    break;
                case FLOAT:  changed = update<float>(p, val);  break;
                case FLOAT2: changed = update<float2>(p, val); break;
                case FLOAT3: changed = update<float3>(p, val); break;
                case FLOAT4: changed = update<float4>(p, val); break;
            }
            if (changed) {
                LOG(INFO) << "DebugRegistry: overriding " << name << " to " << value;
                auto const& info = propertyMap[name];
                if (info.second) {
                    info.second();
                }
            }
        }
    }
}

bool FDebugRegistry::hasProperty(const char* name) const noexcept {
    return getPropertyAddress(name) != nullptr;
}

template<typename T>
bool FDebugRegistry::setProperty(const char* name, T v) noexcept {
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

bool FDebugRegistry::registerDataSource(std::string_view const name,
        void const* data, size_t const count) noexcept {
    auto& dataSourceMap = mDataSourceMap;
    bool const found = dataSourceMap.find(name) == dataSourceMap.end();
    if (found) {
        dataSourceMap[name] = { data, count };
    }
    return found;
}

bool FDebugRegistry::registerDataSource(std::string_view const name,
        Invocable<DataSource()>&& creator) noexcept {
    auto& dataSourceCreatorMap = mDataSourceCreatorMap;
    bool const found = dataSourceCreatorMap.find(name) == dataSourceCreatorMap.end();
    if (found) {
        dataSourceCreatorMap[name] = std::move(creator);
    }
    return found;
}

void FDebugRegistry::unregisterDataSource(std::string_view const name) noexcept {
    mDataSourceCreatorMap.erase(name);
    mDataSourceMap.erase(name);
}


DebugRegistry::DataSource FDebugRegistry::getDataSource(const char* name) const noexcept {
    std::string_view const key{ name };
    auto& dataSourceMap = mDataSourceMap;
    auto const& it = dataSourceMap.find(key);
    if (UTILS_UNLIKELY(it == dataSourceMap.end())) {
        auto& dataSourceCreatorMap = mDataSourceCreatorMap;
        auto const& pos = dataSourceCreatorMap.find(key);
        if (pos == dataSourceCreatorMap.end()) {
            return { nullptr, 0u };
        }
        DataSource dataSource{ pos->second() };
        dataSourceMap[key] = dataSource;
        dataSourceCreatorMap.erase(pos);
        return dataSource;
    }
    return it->second;
}

} // namespace filament
