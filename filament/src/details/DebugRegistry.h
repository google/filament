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

#ifndef TNT_FILAMENT_DETAILS_DEBUGREGISTRY_H
#define TNT_FILAMENT_DETAILS_DEBUGREGISTRY_H

#include "downcast.h"

#include <filament/DebugRegistry.h>

#include <utils/compiler.h>

#include <functional>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace filament {

class FEngine;

class FDebugRegistry : public DebugRegistry {
public:
    FDebugRegistry() noexcept;

    void registerProperty(std::string_view name, bool* p) noexcept {
        registerProperty(name, p, BOOL);
    }

    void registerProperty(std::string_view name, int* p) noexcept {
        registerProperty(name, p, INT);
    }

    void registerProperty(std::string_view name, float* p) noexcept {
        registerProperty(name, p, FLOAT);
    }

    void registerProperty(std::string_view name, math::float2* p) noexcept {
        registerProperty(name, p, FLOAT2);
    }

    void registerProperty(std::string_view name, math::float3* p) noexcept {
        registerProperty(name, p, FLOAT3);
    }

    void registerProperty(std::string_view name, math::float4* p) noexcept {
        registerProperty(name, p, FLOAT4);
    }


    void registerProperty(std::string_view name, bool* p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, BOOL, std::move(fn));
    }

    void registerProperty(std::string_view name, int* p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, INT, std::move(fn));
    }

    void registerProperty(std::string_view name, float* p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, FLOAT, std::move(fn));
    }

    void registerProperty(std::string_view name, math::float2* p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, FLOAT2, std::move(fn));
    }

    void registerProperty(std::string_view name, math::float3* p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, FLOAT3, std::move(fn));
    }

    void registerProperty(std::string_view name, math::float4* p,
            std::function<void()> fn) noexcept {
        registerProperty(name, p, FLOAT4, std::move(fn));
    }

    void registerDataSource(std::string_view name, void const* data, size_t count) noexcept;

#if !defined(_MSC_VER)
private:
#endif
    template<typename T> bool getProperty(const char* name, T* p) const noexcept;
    template<typename T> bool setProperty(const char* name, T v) noexcept;

private:
    using PropertyInfo = std::pair<void*, std::function<void()>>;
    friend class DebugRegistry;
    void registerProperty(std::string_view name, void* p, Type type, std::function<void()> fn = {}) noexcept;
    bool hasProperty(const char* name) const noexcept;
    PropertyInfo getPropertyInfo(const char* name) noexcept;
    void* getPropertyAddress(const char* name);
    void const* getPropertyAddress(const char* name) const noexcept;
    DataSource getDataSource(const char* name) const noexcept;
    std::unordered_map<std::string_view, PropertyInfo> mPropertyMap;
    std::unordered_map<std::string_view, DataSource> mDataSourceMap;
};

FILAMENT_DOWNCAST(DebugRegistry)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_DEBUGREGISTRY_H
