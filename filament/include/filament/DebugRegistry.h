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

#ifndef TNT_FILAMENT_DEBUG_H
#define TNT_FILAMENT_DEBUG_H

#include <filament/FilamentAPI.h>

#include <utils/compiler.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

// FIXME: could we get rid of <utility>
#include <utility>

#include <stdint.h>

namespace filament {

class UTILS_PUBLIC DebugRegistry : public FilamentAPI {
public:

    enum Type {
        BOOL, INT, FLOAT, FLOAT2, FLOAT3, FLOAT4
    };

    struct Property {
        const char* name;
        Type type;
    };

    std::pair<Property const*, size_t> getProperties() const noexcept;

    bool hasProperty(const char* name) const noexcept;

    void* getPropertyAddress(const char* name) noexcept;

    bool setProperty(const char* name, bool v) noexcept;
    bool setProperty(const char* name, int v) noexcept;
    bool setProperty(const char* name, float v) noexcept;
    bool setProperty(const char* name, math::float2 v) noexcept;
    bool setProperty(const char* name, math::float3 v) noexcept;
    bool setProperty(const char* name, math::float4 v) noexcept;

    bool getProperty(const char* name, bool* v) const noexcept;
    bool getProperty(const char* name, int* v) const noexcept;
    bool getProperty(const char* name, float* v) const noexcept;
    bool getProperty(const char* name, math::float2* v) const noexcept;
    bool getProperty(const char* name, math::float3* v) const noexcept;
    bool getProperty(const char* name, math::float4* v) const noexcept;

    template<typename T>
    inline T* getPropertyAddress(const char* name) noexcept {
        return static_cast<T*>(getPropertyAddress(name));
    }

    template<typename T>
    inline bool getPropertyAddress(const char* name, T** p) noexcept {
        *p = getPropertyAddress<T>(name);
        return *p != nullptr;
    }
};


} // namespace filament

#endif /* TNT_FILAMENT_DEBUG_H */
