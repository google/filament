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

#include <backend/DriverEnums.h>

#include <stddef.h>

#ifndef FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB
#    define FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB 1
#endif

namespace filament {
namespace backend {

/**
 * Returns true if the shader string requests the Google-style line directive extension.
 */
bool requestsGoogleLineDirectivesExtension(const char* shader, size_t length) noexcept;

/**
 * Edit a GLSL shader string in-place so any Google-style line directives are turned into regular
 * line directives.
 *
 * E.g.:
 * #line 100 "foobar.h"
 * is transformed to (_ denotes a space)
 * #line 100 __________
 */
void removeGoogleLineDirectives(char* shader, size_t length) noexcept;

/**
 * Returns the number of bytes per pixel for the given format. For compressed texture formats,
 * returns the number of bytes per block.
 */
size_t getFormatSize(TextureFormat format) noexcept;

/**
 * For compressed texture formats, returns the number of horizontal texels per block. Otherwise
 * returns 0.
 */
size_t getBlockWidth(TextureFormat format) noexcept;

/**
 * For compressed texture formats, returns the number of vertical texels per block. Otherwise
 * returns 0.
 */
size_t getBlockHeight(TextureFormat format) noexcept;

} // namespace backend
} // namespace filament
