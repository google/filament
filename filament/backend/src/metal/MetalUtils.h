/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_METALUTILS_H
#define TNT_FILAMENT_DRIVER_METALUTILS_H

#include <Metal/Metal.h>

namespace filament::backend::metal {

/**
 * Creates a texture view of the passed-in texture with the given swizzle. If swizzle is the default
 * swizzle, simply returns texture.
 */
API_AVAILABLE(ios(13.0))
id<MTLTexture> createTextureViewWithSwizzle(id<MTLTexture> texture,
        MTLTextureSwizzleChannels swizzle);

} // namespace filament::backend::metal

#endif //TNT_FILAMENT_DRIVER_METALUTILS_H
