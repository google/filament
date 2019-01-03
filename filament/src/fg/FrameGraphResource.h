/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FRAMEGRAPHRESOURCEHANDLE_H
#define TNT_FILAMENT_FRAMEGRAPHRESOURCEHANDLE_H

#include <limits>

#include <stdint.h>

namespace filament {

/*
 * A FrameGraph resource.
 *
 * This is used to represent a virtual resource.
 */

struct FrameGraphResource {

    struct TextureDesc {
        // TODO: descriptor for textures and render targets
    };

    static constexpr uint16_t UNINITIALIZED = std::numeric_limits<uint16_t>::max();
    // index to the resource handle
    uint16_t index = UNINITIALIZED;
    // version of the resource when it was created. When this version doesn't match the
    // resource handle's version, this resource has become invalid.
    uint16_t version = 0;
    bool isValid() const noexcept { return index != UNINITIALIZED; }
};

} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPHRESOURCEHANDLE_H
