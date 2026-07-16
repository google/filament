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

#ifndef TNT_UTILS_DEBUGREGISTRY_H
#define TNT_UTILS_DEBUGREGISTRY_H

#include <utils/compiler.h>
#include <utils/Invocable.h>

#include <array>
#include <atomic>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <utility>

#include <stddef.h>

namespace utils {

class UTILS_PUBLIC InternalDebugRegistry {
public:
    enum Type { BOOL, INT, FLOAT, FLOAT2, FLOAT3, FLOAT4, ATOMIC_BOOL };

    struct DataSource {
        void const* UTILS_NULLABLE data;
        size_t count;
    };

    InternalDebugRegistry() noexcept;
    ~InternalDebugRegistry() noexcept = default;

    void registerProperty(std::string_view const name, bool* UTILS_NONNULL p) noexcept {
        registerProperty(name, p, BOOL);
    }

    void registerProperty(std::string_view const name, int* UTILS_NONNULL p) noexcept {
        registerProperty(name, p, INT);
    }

    void registerProperty(std::string_view const name, float* UTILS_NONNULL p) noexcept {
        registerProperty(name, p, FLOAT);
    }

    // We use std::array<float, N> here instead of math::float2/3/4 to avoid a cyclic dependency
    // between libs/utils and libs/math. The public API in filament::DebugRegistry takes
    // math::floatN, and the backend conversion is handled in FDebugRegistry.
    void registerProperty(std::string_view const name,
            std::array<float, 2>* UTILS_NONNULL p) noexcept {
        registerProperty(name, p, FLOAT2);
    }

    void registerProperty(std::string_view const name,
            std::array<float, 3>* UTILS_NONNULL p) noexcept {
        registerProperty(name, p, FLOAT3);
    }

    void registerProperty(std::string_view const name,
            std::array<float, 4>* UTILS_NONNULL p) noexcept {
        registerProperty(name, p, FLOAT4);
    }

    /**
     * Registers an atomic boolean property.
     *
     * std::atomic<bool> is needed because some debug properties are read concurrently
     * by the backend threads while being mutated by the main thread (from the client).
     * Using atomic variables ensures safe cross-thread flag flipping without data races.
     */
    void registerProperty(std::string_view const name,
            std::atomic<bool>* UTILS_NONNULL p) noexcept {
        registerProperty(name, p, ATOMIC_BOOL);
    }

    void registerProperty(std::string_view const name, bool* UTILS_NONNULL p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, BOOL, std::move(fn));
    }

    void registerProperty(std::string_view const name, int* UTILS_NONNULL p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, INT, std::move(fn));
    }

    void registerProperty(std::string_view const name, float* UTILS_NONNULL p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, FLOAT, std::move(fn));
    }

    void registerProperty(std::string_view const name, std::array<float, 2>* UTILS_NONNULL p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, FLOAT2, std::move(fn));
    }

    void registerProperty(std::string_view const name, std::array<float, 3>* UTILS_NONNULL p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, FLOAT3, std::move(fn));
    }

    void registerProperty(std::string_view const name, std::array<float, 4>* UTILS_NONNULL p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, FLOAT4, std::move(fn));
    }

    /**
     * Registers an atomic boolean property with a callback.
     *
     * std::atomic<bool> is needed because some debug properties are read concurrently
     * by the backend threads while being mutated by the main thread (from the client).
     * Using atomic variables ensures safe cross-thread flag flipping without data races.
     */
    void registerProperty(std::string_view const name, std::atomic<bool>* UTILS_NONNULL p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, ATOMIC_BOOL, std::move(fn));
    }

    // registers a DataSource directly
    bool registerDataSource(std::string_view name, void const* UTILS_NULLABLE data,
            size_t count) noexcept;

    // registers a DataSource lazily
    bool registerDataSource(std::string_view name,
            utils::Invocable<DataSource()>&& creator) noexcept;

    void unregisterDataSource(std::string_view name) noexcept;

    bool hasProperty(const char* UTILS_NONNULL name) const noexcept;

    void* UTILS_NULLABLE getPropertyAddress(const char* UTILS_NONNULL name);
    void const* UTILS_NULLABLE getPropertyAddress(const char* UTILS_NONNULL name) const noexcept;

    template<typename T>
    inline T* UTILS_NULLABLE getPropertyAddress(const char* UTILS_NONNULL name) {
        return static_cast<T*>(getPropertyAddress(name));
    }

    template<typename T>
    inline T const* UTILS_NULLABLE getPropertyAddress(
            const char* UTILS_NONNULL name) const noexcept {
        return static_cast<T const*>(getPropertyAddress(name));
    }

    template<typename T>
    inline bool getPropertyAddress(const char* UTILS_NONNULL name,
            T* UTILS_NULLABLE* UTILS_NONNULL p) {
        *p = getPropertyAddress<T>(name);
        return *p != nullptr;
    }

    template<typename T>
    inline bool getPropertyAddress(const char* UTILS_NONNULL name,
            T* const UTILS_NULLABLE* UTILS_NONNULL p) const noexcept {
        *p = getPropertyAddress<T>(name);
        return *p != nullptr;
    }

    // Explicitly make the void* version public so FDebugRegistry can use it for custom types like
    // math::float2
    void registerProperty(std::string_view name, void* UTILS_NULLABLE p, Type type,
            std::function<void()> fn = {}) noexcept;

    template<typename T>
    bool getProperty(const char* UTILS_NONNULL name, T* UTILS_NONNULL p) const noexcept;
    template<typename T>
    bool setProperty(const char* UTILS_NONNULL name, T v) noexcept;

    DataSource getDataSource(const char* UTILS_NONNULL name) const noexcept;

private:
    struct PropertyInfo {
        void* UTILS_NULLABLE ptr;
        std::function<void()> fn;
        Type type;
    };

    PropertyInfo getPropertyInfo(const char* UTILS_NONNULL name) noexcept;

    std::unordered_map<std::string_view, PropertyInfo> mPropertyMap;
    mutable std::unordered_map<std::string_view, DataSource> mDataSourceMap;
    mutable std::unordered_map<std::string_view, utils::Invocable<DataSource()>>
            mDataSourceCreatorMap;
};

} // namespace utils

#endif // TNT_UTILS_DEBUGREGISTRY_H
