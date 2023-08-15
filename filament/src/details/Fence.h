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

#ifndef TNT_FILAMENT_DETAILS_FENCE_H
#define TNT_FILAMENT_DETAILS_FENCE_H

#include "downcast.h"

#include <filament/Fence.h>

#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>

namespace filament {

class FEngine;

class FFence : public Fence {
public:
    FFence(FEngine& engine);

    void terminate(FEngine& engine) noexcept;

    FenceStatus wait(Mode mode, uint64_t timeout) noexcept;

    static FenceStatus waitAndDestroy(FFence* fence, Mode mode) noexcept;

private:
    // We assume we don't have a lot of contention of fence and have all of them
    // share a single lock/condition
    static utils::Mutex sLock;
    static utils::Condition sCondition;

    struct FenceSignal {
        explicit FenceSignal() noexcept = default;
        enum State : uint8_t { UNSIGNALED, SIGNALED, DESTROYED };
        State mState = UNSIGNALED;
        void signal(State s = SIGNALED) noexcept;
        FenceStatus wait(uint64_t timeout) noexcept;
    };

    FEngine& mEngine;
    // TODO: use custom allocator for these small objects
    std::shared_ptr<FenceSignal> mFenceSignal;
};

FILAMENT_DOWNCAST(Fence)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_FENCE_H
