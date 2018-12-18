/*
 * Copyright (C) 2017 The Android Open Source Project
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

// The noop driver is only useful for ensuring we don't have certain build issues.
// Remove it from release builds, since it uses some space needlessly.
#ifndef NDEBUG

#include "driver/noop/PlatformNoop.h"

#include "driver/noop/NoopDriver.h"

namespace filament {

Driver* PlatformNoop::createDriver(void* const sharedGLContext) noexcept {
    return NoopDriver::create();
}

} // namespace filament

#endif
