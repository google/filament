/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <filament/Fence.h>

#include "API.h"

using namespace filament;

Fence::FenceStatus Filament_Fence_Wait(Fence *fence, Fence::Mode mode, uint64_t timeoutNanoSeconds) {
    return fence->wait(mode, timeoutNanoSeconds);
}

Fence::FenceStatus Filament_Fence_WaitAndDestroy(Fence *fence, Fence::Mode mode) {
    return Fence::waitAndDestroy(fence, mode);
}
