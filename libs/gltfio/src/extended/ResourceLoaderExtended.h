/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef GLTFIO_RESOURCELOADEREXTENDED_H
#define GLTFIO_RESOURCELOADEREXTENDED_H

#include "../FFilamentAsset.h"

#include <vector>

namespace filament::gltfio {

struct ResourceLoaderExtended {
    using BufferSlot = FFilamentAsset::ResourceInfoExtended::BufferSlot;
    static void loadResources(
        std::vector<BufferSlot> const& slots, filament::Engine* engine,
        std::vector<BufferObject*>& bufferObjects);
};

} // namespace filament::gltfio

#endif // GLTFIO_RESOURCELOADEREXTENDED_H
