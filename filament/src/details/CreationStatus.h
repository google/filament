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

#ifndef TNT_FILAMENT_DETAILS_CREATIONSTATUS_H
#define TNT_FILAMENT_DETAILS_CREATIONSTATUS_H

#include <stdint.h>

namespace filament {

/**
 * Where an object is in its creation, for the classes that support asynchronous creation
 * (FTexture, FVertexBuffer, FIndexBuffer).
 *
 * Two distinct questions are asked of this, and cancellation is where they diverge:
 *  - "is the asynchronous pipeline done with this object?" — a *lifetime* question, which
 *    FEngine::destroy uses to decide whether it can free the object now. Both terminal states
 *    answer yes.
 *  - "is the resource usable?" — only CREATED answers yes.
 *
 * Objects created synchronously go straight to CREATED; asynchronously created ones start at
 * CREATING and reach exactly one terminal state, once, when the last of their creation jobs
 * reports in.
 */
enum class CreationStatus : uint8_t {
    CREATING,   //!< Creation is still in flight.
    CANCELED,   //!< Creation finished without ever populating the object.
    CREATED,    //!< Creation finished and populated the object.
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_CREATIONSTATUS_H
