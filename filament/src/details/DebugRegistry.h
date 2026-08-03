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

#include <private/utils/InternalDebugRegistry.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <array>

namespace filament {

class FDebugRegistry : public filament::DebugRegistry {
public:
    FDebugRegistry() noexcept = default;

    utils::InternalDebugRegistry internalRegistry;

    template<typename... Args>
    void registerProperty(Args&&... args) noexcept {
        internalRegistry.registerProperty(std::forward<Args>(args)...);
    }

    // InternalDebugRegistry uses std::array to represent vector properties to avoid depending on
    // libs/math. Since math::floatN (TVecN) and std::array<float, N> share the exact same binary
    // layout (contiguous floats) and are standard-layout, we can safely reinterpret_cast the
    // pointers for registration.
    void registerProperty(std::string_view const name, math::float2* p) noexcept {
        internalRegistry.registerProperty(name, reinterpret_cast<std::array<float, 2>*>(p));
    }

    void registerProperty(std::string_view const name, math::float3* p) noexcept {
        internalRegistry.registerProperty(name, reinterpret_cast<std::array<float, 3>*>(p));
    }

    void registerProperty(std::string_view const name, math::float4* p) noexcept {
        internalRegistry.registerProperty(name, reinterpret_cast<std::array<float, 4>*>(p));
    }

    template<typename... Args>
    bool registerDataSource(Args&&... args) noexcept {
        return internalRegistry.registerDataSource(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void unregisterDataSource(Args&&... args) noexcept {
        internalRegistry.unregisterDataSource(std::forward<Args>(args)...);
    }

    bool hasProperty(const char* name) const noexcept { return internalRegistry.hasProperty(name); }

    template<typename T>
    bool setProperty(const char* name, T v) noexcept {
        return internalRegistry.setProperty(name, v);
    }

    // FDebugRegistry handles ferrying data to the backend InternalDebugRegistry.
    // It converts public math::floatN API calls into std::array structures to cross the API
    // boundary.
    bool setProperty(const char* name, math::float2 v) noexcept {
        return internalRegistry.setProperty(name, std::array<float, 2>{ v.x, v.y });
    }

    bool setProperty(const char* name, math::float3 v) noexcept {
        return internalRegistry.setProperty(name, std::array<float, 3>{ v.x, v.y, v.z });
    }

    bool setProperty(const char* name, math::float4 v) noexcept {
        return internalRegistry.setProperty(name, std::array<float, 4>{ v.x, v.y, v.z, v.w });
    }

    template<typename T>
    bool getProperty(const char* name, T* v) const noexcept {
        return internalRegistry.getProperty(name, v);
    }

    bool getProperty(const char* name, math::float2* v) const noexcept {
        std::array<float, 2> a;
        if (internalRegistry.getProperty(name, &a)) {
            *v = math::float2{ a[0], a[1] };
            return true;
        }
        return false;
    }

    bool getProperty(const char* name, math::float3* v) const noexcept {
        std::array<float, 3> a;
        if (internalRegistry.getProperty(name, &a)) {
            *v = math::float3{ a[0], a[1], a[2] };
            return true;
        }
        return false;
    }

    bool getProperty(const char* name, math::float4* v) const noexcept {
        std::array<float, 4> a;
        if (internalRegistry.getProperty(name, &a)) {
            *v = math::float4{ a[0], a[1], a[2], a[3] };
            return true;
        }
        return false;
    }

    void* getPropertyAddress(const char* name) noexcept {
        return internalRegistry.getPropertyAddress(name);
    }

    void const* getPropertyAddress(const char* name) const noexcept {
        return internalRegistry.getPropertyAddress(name);
    }

    filament::DebugRegistry::DataSource getDataSource(const char* name) const noexcept {
        auto ds = internalRegistry.getDataSource(name);
        return { ds.data, ds.count };
    }
};

FILAMENT_DOWNCAST(DebugRegistry)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_DEBUGREGISTRY_H
