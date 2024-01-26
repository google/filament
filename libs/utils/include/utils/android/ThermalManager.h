/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_UTILS_ANDROID_THERMALMANAGER_H
#define TNT_UTILS_ANDROID_THERMALMANAGER_H

#include <stdint.h>

struct AThermalManager;

namespace utils {

class ThermalManager {
public:
    enum class ThermalStatus : int8_t {
        ERROR = -1,
        NONE,
        LIGHT,
        MODERATE,
        SEVERE,
        CRITICAL,
        EMERGENCY,
        SHUTDOWN
    };

    ThermalManager();
    ~ThermalManager();

    // Movable
    ThermalManager(ThermalManager&& rhs) noexcept;
    ThermalManager& operator=(ThermalManager&& rhs) noexcept;

    // not copiable
    ThermalManager(ThermalManager const& rhs) = delete;
    ThermalManager& operator=(ThermalManager const& rhs) = delete;

    ThermalStatus getCurrentThermalStatus() const noexcept;

private:
    AThermalManager* mThermalManager = nullptr;
};

} // namespace utils

#endif // TNT_UTILS_ANDROID_THERMALMANAGER_H
