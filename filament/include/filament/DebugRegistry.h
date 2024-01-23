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

//! \file

#ifndef TNT_FILAMENT_DEBUGREGISTRY_H
#define TNT_FILAMENT_DEBUGREGISTRY_H

#include <filament/FilamentAPI.h>

#include <utils/compiler.h>

#include <math/mathfwd.h>

#include <stddef.h>

namespace filament {

/**
 * A registry of runtime properties used exclusively for debugging
 *
 * Filament exposes a few properties that can be queried and set, which control certain debugging
 * features of the engine. These properties can be set at runtime at anytime.
 *
 */
class UTILS_PUBLIC DebugRegistry : public FilamentAPI {
public:

    /**
     * Type of a property
     */
    enum Type {
        BOOL, INT, FLOAT, FLOAT2, FLOAT3, FLOAT4
    };

    /**
     * Information about a property
     */
    struct Property {
        const char* name;   //!< property name
        Type type;          //!< property type
    };

    /**
     * Queries whether a property exists
     * @param name The name of the property to query
     * @return true if the property exists, false otherwise
     */
    bool hasProperty(const char* name) const noexcept;

    /**
     * Queries the address of a property's data from its name
     * @param name Name of the property we want the data address of
     * @return Address of the data of the \p name property
     * @{
     */
    void* getPropertyAddress(const char* name);

    void const* getPropertyAddress(const char* name) const noexcept;

    template<typename T>
    inline T* getPropertyAddress(const char* name) {
        return static_cast<T*>(getPropertyAddress(name));
    }

    template<typename T>
    inline T const* getPropertyAddress(const char* name) const noexcept {
        return static_cast<T*>(getPropertyAddress(name));
    }

    template<typename T>
    inline bool getPropertyAddress(const char* name, T** p) {
        *p = getPropertyAddress<T>(name);
        return *p != nullptr;
    }

    template<typename T>
    inline bool getPropertyAddress(const char* name, T* const* p) const noexcept {
        *p = getPropertyAddress<T>(name);
        return *p != nullptr;
    }
    /** @}*/

    /**
     * Set the value of a property
     * @param name Name of the property to set the value of
     * @param v Value to set
     * @return true if the operation was successful, false otherwise.
     * @{
     */
    bool setProperty(const char* name, bool v) noexcept;
    bool setProperty(const char* name, int v) noexcept;
    bool setProperty(const char* name, float v) noexcept;
    bool setProperty(const char* name, math::float2 v) noexcept;
    bool setProperty(const char* name, math::float3 v) noexcept;
    bool setProperty(const char* name, math::float4 v) noexcept;
    /** @}*/

    /**
     * Get the value of a property
     * @param name Name of the property to get the value of
     * @param v A pointer to a variable which will hold the result
     * @return true if the call was successful and \p v was updated
     * @{
     */
    bool getProperty(const char* name, bool* v) const noexcept;
    bool getProperty(const char* name, int* v) const noexcept;
    bool getProperty(const char* name, float* v) const noexcept;
    bool getProperty(const char* name, math::float2* v) const noexcept;
    bool getProperty(const char* name, math::float3* v) const noexcept;
    bool getProperty(const char* name, math::float4* v) const noexcept;
    /** @}*/

    struct DataSource {
        void const* data;
        size_t count;
    };

    DataSource getDataSource(const char* name) const noexcept;

    struct FrameHistory {
        using duration_ms = float;
        duration_ms target{};
        duration_ms targetWithHeadroom{};
        duration_ms frameTime{};
        duration_ms frameTimeDenoised{};
        float scale = 1.0f;
        float pid_e = 0.0f;
        float pid_i = 0.0f;
        float pid_d = 0.0f;
    };

protected:
    // prevent heap allocation
    ~DebugRegistry() = default;
};


} // namespace filament

#endif /* TNT_FILAMENT_DEBUGREGISTRY_H */
