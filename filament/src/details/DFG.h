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

#ifndef TNT_FILAMENT_DETAILS_DFG_H
#define TNT_FILAMENT_DETAILS_DFG_H

#include "details/Texture.h"

#include <backend/Handle.h>

#include <utils/compiler.h>

namespace filament {
namespace details {

class FEngine;
class FTexture;

class DFG {
public:
    explicit DFG(FEngine& engine) noexcept;

    size_t getLutSize() const noexcept {
        return DFG_LUT_SIZE;
    }

    bool isValid() const noexcept {
        return mLUT != nullptr;
    }

    backend::Handle<backend::HwTexture> getTexture() const noexcept {
        return mLUT->getHwHandle();
    }

    void terminate();

    DFG(DFG const& rhs) = delete;
    DFG(DFG&& rhs) = delete;
    DFG& operator=(DFG const& rhs) = delete;
    DFG& operator=(DFG&& rhs) = delete;

private:
    FEngine& mEngine;
    FTexture* mLUT = nullptr;

    // make sure to use the right size here
    static constexpr size_t DFG_LUT_SIZE = 128;

    // this lookup table is generated with cmgen
    static const uint16_t DFG_LUT[];
};

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_DFG_H
