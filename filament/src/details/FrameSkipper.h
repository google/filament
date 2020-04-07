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

#ifndef TNT_FILAMENT_DETAILS_FRAMESKIPPER_H
#define TNT_FILAMENT_DETAILS_FRAMESKIPPER_H

#include "backend/Handle.h"

#include <array>

namespace filament {
namespace details {

class FEngine;

class FrameSkipper {
    static constexpr size_t MAX_FRAME_LATENCY = 4;
public:
    explicit FrameSkipper(FEngine& engine, size_t latency = 2) noexcept;
    ~FrameSkipper() noexcept;

    // returns false if we need to skip this frame, because the gpu is running behind the cpu.
    // in that case, don't call endFrame().
    // returns true if rendering can proceed. Always call endFrame() when done.
    bool beginFrame() noexcept;

    void endFrame() noexcept;

private:
    FEngine& mEngine;
    using Container = std::array<backend::Handle<backend::HwSync>, MAX_FRAME_LATENCY>;
    mutable Container mDelayedSyncs{};
    size_t mLast;
};

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_FRAMESKIPPER_H
