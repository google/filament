/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_COLORGRADING_H
#define TNT_FILAMENT_DETAILS_COLORGRADING_H

#include "downcast.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <filament/ColorGrading.h>

#include <math/mathfwd.h>

namespace filament {

class FEngine;

class FColorGrading : public ColorGrading {
public:
    FColorGrading(FEngine& engine, const Builder& builder);
    FColorGrading(const FColorGrading& rhs) = delete;
    FColorGrading& operator=(const FColorGrading& rhs) = delete;

    ~FColorGrading() noexcept;

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    backend::TextureHandle getHwHandle() const noexcept { return mLutHandle; }
    uint32_t getDimension() const noexcept { return mDimension; }
    bool isOneDimensional() const noexcept { return mIsOneDimensional; }
    bool isLDR() const noexcept { return mIsLDR; }

private:
    backend::TextureHandle mLutHandle;
    uint32_t mDimension;
    bool mIsOneDimensional;
    bool mIsLDR;

    void initializeSettings(Settings& settings, const Builder& builder) noexcept;
};

FILAMENT_DOWNCAST(ColorGrading)

} // namespace filament

#endif //TNT_FILAMENT_DETAILS_COLORGRADING_H
