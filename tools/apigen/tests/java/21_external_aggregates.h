/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_EXTERNAL_AGGREGATES_TEST_H
#define TNT_FILAMENT_EXTERNAL_AGGREGATES_TEST_H

#include <filament/Box.h>
#include <filament/Viewport.h>

#include <utils/compiler.h>

namespace filament {

class SwapChain;

class UTILS_PUBLIC ExternalAggregatesTest {
public:
    void copyFrame(SwapChain* UTILS_NONNULL dstSwapChain, const Viewport& dstViewport, const Viewport& srcViewport, uint32_t flags = 0);
    bool intersects(const Box& box) const noexcept;
    Box getBoundingBox() const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_EXTERNAL_AGGREGATES_TEST_H
