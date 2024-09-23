/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_LTC_H
#define TNT_FILAMENT_DETAILS_LTC_H

#include <backend/Handle.h>

#include "details/Texture.h"

#include <utils/compiler.h>

namespace filament {

class FEngine;

class LTC {
public:
    explicit LTC() noexcept = default;

    LTC(LTC const& rhs) = delete;
    LTC(LTC&& rhs) = delete;
    LTC& operator=(LTC const& rhs) = delete;
    LTC& operator=(LTC&& rhs) = delete;

    void init(FEngine& engine) noexcept;

    size_t getLutSize() const noexcept {
        return LTC_LUT_SIZE;
    }

    bool isValid() const noexcept {
        return mLUT != nullptr;
    }

    backend::Handle<backend::HwTexture> getTexture() const noexcept {
        return mLUT->getHwHandle();
    }

    void terminate(FEngine& engine) noexcept;

private:
    FTexture* mLUT = nullptr;

    static constexpr size_t LTC_LUT_SIZE = 64;

    static const uint16_t LTC_LUT[];
};

}  // namespace filament

#endif  // TNT_FILAMENT_DETAILS_LTC_H
