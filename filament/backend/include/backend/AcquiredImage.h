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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_ACQUIREDIMAGE_H
#define TNT_FILAMENT_BACKEND_PRIVATE_ACQUIREDIMAGE_H

#include <backend/DriverEnums.h>
#include "math/mat3.h"

namespace filament::backend {

class CallbackHandler;

// This lightweight POD allows us to bundle the state required to process an ACQUIRED stream.
// Since these types of external images need to be moved around and queued up, an encapsulation is
// very useful.

struct AcquiredImage {
    void* image = nullptr;
    backend::StreamCallback callback = nullptr;
    void* userData = nullptr;
    CallbackHandler* handler = nullptr;
    math::mat3f transform;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_ACQUIREDIMAGE_H
