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

#ifndef TNT_FILAMENT_DETAILS_DEBUG_H
#define TNT_FILAMENT_DETAILS_DEBUG_H

#include "upcast.h"

#include <filament/DebugRegistry.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/vector.h>

#include <tsl/robin_map.h>

namespace filament {

class FEngine;

class FDebugRegistry : public DebugRegistry {
public:
    FDebugRegistry() noexcept;

    std::pair<Property const*, size_t> getProperties() const noexcept;

    bool hasProperty(const char* name) const noexcept;

    void* getPropertyAddress(const char* name) noexcept;

    template <typename T>
    bool setProperty(const char* name, T v) noexcept;

    template <typename T>
    bool getProperty(const char* name, T* p) const noexcept;

    template <typename T>
    void registerProperty(utils::StaticString name, T* p) noexcept {
        Type type = BOOL;
        if (std::is_same<T, bool>::value)           type = BOOL;
        if (std::is_same<T, int>::value)            type = INT;
        if (std::is_same<T, float>::value)          type = FLOAT;
        if (std::is_same<T, math::float2>::value)   type = FLOAT2;
        if (std::is_same<T, math::float3>::value)   type = FLOAT3;
        if (std::is_same<T, math::float4>::value)   type = FLOAT3;
        registerProperty(name, p, type);
    }

private:
    void registerProperty(utils::StaticString name, void* p, Type type) noexcept;
    utils::vector<Property> mProperties;
    tsl::robin_map<utils::StaticString, void*> mPropertyMap;
};

FILAMENT_UPCAST(DebugRegistry)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_DEBUG_H
