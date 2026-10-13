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

#include <private/utils/InternalDebugRegistry.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/Invocable.h>
#include <utils/Logger.h>
#include <utils/Mutex.h>
#include <utils/Panic.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <string_view>
#include <utility>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace utils {

namespace {

// short-hand for std::array<float, n>. We cannot define a dependency against <math> (since <math>
// depends on <utils>, and so therefore the API works on std::array floats, which client can cast
// math::floatn to.
using float2 = std::array<float, 2>;
using float3 = std::array<float, 3>;
using float4 = std::array<float, 4>;

constexpr size_t MAX_PROPERTY_NAME_LENGTH = 128;
constexpr size_t MAX_PROPERTY_VALUE_LENGTH = 128;

template<typename T>
bool parse(const char*, T&) {
    return false;
}

template<>
bool parse(const char* val, bool& out) {
    std::string_view const value(val);
    if (value == "1" || value == "true") {
        out = true;
        return true;
    }
    if (value == "0" || value == "false") {
        out = false;
        return true;
    }
    return false;
}

template<>
bool parse(const char* val, int& out) { return sscanf(val, "%d", &out) == 1; }
template<>
bool parse(const char* val, float& out) { return sscanf(val, "%f", &out) == 1; }
template<>
bool parse(const char* val, float2& out) { return sscanf(val, "%f %f", &out[0], &out[1]) == 2; }
template<>
bool parse(const char* val, float3& out) { return sscanf(val, "%f %f %f", &out[0], &out[1], &out[2]) == 3; }
template<>
bool parse(const char* val, float4& out) { return sscanf(val, "%f %f %f %f", &out[0], &out[1], &out[2], &out[3]) == 4; }


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

template<>
bool update<std::atomic<bool>>(void* p, const char* val) {
    bool v;
    if (parse(val, v)) {
        auto& current = *static_cast<std::atomic<bool>*>(p);
        if (current.load(std::memory_order_relaxed) != v) {
            current.store(v, std::memory_order_relaxed);
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
std::string_view getPropertyFromEnvironment(char const* const name, size_t const nameLen,
        std::array<char, MAX_PROPERTY_VALUE_LENGTH>&) {
    CString cname{ name, nameLen };
    std::replace(cname.data(), cname.data() + nameLen, '.', '_');
    char const* const env = getenv(cname.data());
    return env ? std::string_view{ env } : std::string_view{};
}
#endif

} // namespace

InternalDebugRegistry::InternalDebugRegistry() noexcept = default;

auto InternalDebugRegistry::getPropertyInfo(const char* name) const noexcept -> PropertyInfo {
    std::string_view const key{ name };
    auto const& propertyMap = mPropertyMap;
    auto const& it = propertyMap.find(key);
    if (it == propertyMap.end()) {
        return { nullptr, {}, BOOL };
    }
    return it->second;
}

UTILS_NOINLINE
void* InternalDebugRegistry::getPropertyAddress(const char* name) {
    auto const info = getPropertyInfo(name);
    ASSERT_PRECONDITION_NON_FATAL(!info.fn,
            "don't use InternalDebugRegistry::getPropertyAddress() when a callback is set. "
            "Use setProperty() instead.");
    return info.ptr;
}

UTILS_NOINLINE
void const* InternalDebugRegistry::getPropertyAddress(const char* name) const noexcept {
    return getPropertyInfo(name).ptr;
}

void InternalDebugRegistry::registerProperty(std::string_view const name, void* p, Type const type,
        std::function<void()> fn) noexcept {
    auto& propertyMap = mPropertyMap;
    if (propertyMap.find(name) == propertyMap.end()) {
        propertyMap[name] = { p, std::move(fn), type };

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
                case BOOL:
                    changed = update<bool>(p, val);
                    break;
                case INT:
                    changed = update<int>(p, val);
                    break;
                case FLOAT:
                    changed = update<float>(p, val);
                    break;
                case FLOAT2:
                    changed = update<float2>(p, val);
                    break;
                case FLOAT3:
                    changed = update<float3>(p, val);
                    break;
                case FLOAT4:
                    changed = update<float4>(p, val);
                    break;
                case ATOMIC_BOOL:
                    changed = update<std::atomic<bool>>(p, val);
                    break;
            }
            if (changed) {
                LOG(INFO) << "InternalDebugRegistry: overriding " << name << " to " << value;
                auto const& info = propertyMap[name];
                if (info.fn) {
                    info.fn();
                }
            }
        }
    }
}

bool InternalDebugRegistry::hasProperty(const char* name) const noexcept {
    return getPropertyInfo(name).ptr != nullptr;
}

template<typename T>
bool InternalDebugRegistry::setProperty(const char* name, T v) noexcept {
    auto info = getPropertyInfo(name);
    T* const addr = static_cast<T*>(info.ptr);
    if (addr) {
        auto old = *addr;
        *addr = v;
        if (info.fn && old != v) {
            info.fn();
        }
        return true;
    }
    return false;
}

template<>
bool InternalDebugRegistry::setProperty<bool>(const char* name, bool const v) noexcept {
    auto const [ptr, fn, type] = getPropertyInfo(name);
    if (!ptr) {
        return false;
    }
    if (type == ATOMIC_BOOL) {
        auto* addr = static_cast<std::atomic<bool>*>(ptr);
        bool const old = addr->load(std::memory_order_relaxed);
        addr->store(v, std::memory_order_relaxed);
        if (fn && old != v) {
            fn();
        }
        return true;
    }
    bool* const addr = static_cast<bool*>(ptr);
    bool const old = *addr;
    *addr = v;
    if (fn && old != v) {
        fn();
    }
    return true;
}

template bool InternalDebugRegistry::setProperty<int>(const char* name, int v) noexcept;
template bool InternalDebugRegistry::setProperty<float>(const char* name, float v) noexcept;
template bool InternalDebugRegistry::setProperty<float2>(const char* name, float2 v) noexcept;
template bool InternalDebugRegistry::setProperty<float3>(const char* name, float3 v) noexcept;
template bool InternalDebugRegistry::setProperty<float4>(const char* name, float4 v) noexcept;

template<typename T>
bool InternalDebugRegistry::getProperty(const char* name, T* p) const noexcept {
    T const* const addr = static_cast<T const*>(getPropertyAddress(name));
    if (addr) {
        *p = *addr;
        return true;
    }
    return false;
}

template<>
bool InternalDebugRegistry::getProperty<bool>(const char* name, bool* p) const noexcept {
    auto const info = this->getPropertyInfo(name);
    if (!info.ptr) {
        return false;
    }
    if (info.type == ATOMIC_BOOL) {
        auto* addr = static_cast<std::atomic<bool> const*>(info.ptr);
        *p = addr->load(std::memory_order_relaxed);
        return true;
    }
    bool const* const addr = static_cast<bool const*>(info.ptr);
    *p = *addr;
    return true;
}

template bool InternalDebugRegistry::getProperty<int>(const char* name, int* v) const noexcept;
template bool InternalDebugRegistry::getProperty<float>(const char* name, float* v) const noexcept;
template bool InternalDebugRegistry::getProperty<float2>(const char* name,
        float2* v) const noexcept;
template bool InternalDebugRegistry::getProperty<float3>(const char* name,
        float3* v) const noexcept;
template bool InternalDebugRegistry::getProperty<float4>(const char* name,
        float4* v) const noexcept;

// FIXME: registered names aren't exclusive. Each registerDataSource() overload only checks its
//        own map, and getDataSource() moves a created DataSource to mDataSourceMap, so a lazy
//        registration succeeds again once the name has been queried (and a direct and a lazy
//        registration can share a name). Since unregisterDataSource() erases the name from both
//        maps, one registrant can then remove another's registration: e.g. a View created after
//        "d.view.frame_info" was queried also becomes its owner, and destroying either View
//        removes the other's data source. Fix: keep both kinds in a single map, and create the
//        DataSource in place.
bool InternalDebugRegistry::registerDataSource(std::string_view const name, void const* data,
        size_t const count) noexcept {
    LockGuard const lock(mDataSourceLock);
    auto& dataSourceMap = mDataSourceMap;
    bool const found = dataSourceMap.find(name) == dataSourceMap.end();
    if (found) {
        dataSourceMap[name] = { data, count };
    }
    return found;
}

bool InternalDebugRegistry::registerDataSource(std::string_view const name,
        Invocable<DataSource()>&& creator) noexcept {
    LockGuard const lock(mDataSourceLock);
    auto& dataSourceCreatorMap = mDataSourceCreatorMap;
    bool const found = dataSourceCreatorMap.find(name) == dataSourceCreatorMap.end();
    if (found) {
        dataSourceCreatorMap[name] = std::move(creator);
    }
    return found;
}

void InternalDebugRegistry::unregisterDataSource(std::string_view const name) noexcept {
    // This waits for a creator running in getDataSource(), since it runs with the lock held.
    LockGuard const lock(mDataSourceLock);
    mDataSourceCreatorMap.erase(name);
    mDataSourceMap.erase(name);
}

InternalDebugRegistry::DataSource InternalDebugRegistry::getDataSource(
        const char* name) const noexcept {
    std::string_view const key{ name };
    LockGuard const lock(mDataSourceLock);
    auto const it = mDataSourceMap.find(key);
    if (UTILS_LIKELY(it != mDataSourceMap.end())) {
        return it->second;
    }
    auto const pos = mDataSourceCreatorMap.find(key);
    if (pos == mDataSourceCreatorMap.end()) {
        return { nullptr, 0u };
    }
    // Run the creator with the lock held. Unlocking around it would let unregisterDataSource(),
    // and possibly a new registration under the same name, happen while it runs, and we would
    // then publish a stale result. Holding the lock also guarantees that concurrent first
    // queries run the creator only once. Since the lock isn't recursive, the creator must not
    // call back into the data source API (see registerDataSource()).
    DataSource const dataSource{ pos->second() };
    // Publish under the registered name: `key` points to the caller's string, which may not
    // outlive this call.
    mDataSourceMap[pos->first] = dataSource;
    mDataSourceCreatorMap.erase(pos);
    return dataSource;
}

} // namespace utils
