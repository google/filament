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

//! \file

#ifndef TNT_FILAMENT_FENCE_H
#define TNT_FILAMENT_FENCE_H

#include <filament/FilamentAPI.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>

namespace filament {

/**
 * Fence is used to synchronize rendering operations together, with the CPU or with compute.
 *
 * \note
 * Currently Fence only provide client-side synchronization.
 *
 */
class UTILS_PUBLIC Fence : public FilamentAPI {
public:
    //! Special \p timeout value to disable wait()'s timeout.
    static constexpr uint64_t FENCE_WAIT_FOR_EVER = backend::FENCE_WAIT_FOR_EVER;

    //! Error codes for Fence::wait()
    using FenceStatus = backend::FenceStatus;

    /** Mode controls the behavior of the command stream when calling wait()
     *
     * @attention
     * It would be unwise to call `wait(..., Mode::DONT_FLUSH)` from the same thread
     * the Fence was created, as it would most certainly create a dead-lock.
     */
    enum class Mode : uint8_t {
        FLUSH,          //!< The command stream is flushed
        DONT_FLUSH      //!< The command stream is not flushed
    };

    /**
     * Client-side wait on the Fence.
     *
     * Blocks the current thread until the Fence signals.
     *
     * @param mode      Whether the command stream is flushed before waiting or not.
     * @param timeout   Wait time out. Using a \p timeout of 0 is a way to query the state of the fence.
     *                  A \p timeout value of FENCE_WAIT_FOR_EVER is used to disable the timeout.
     * @return          FenceStatus::CONDITION_SATISFIED on success,
     *                  FenceStatus::TIMEOUT_EXPIRED if the time out expired or
     *                  FenceStatus::ERROR in other cases.
     * @see #Mode
     */
    FenceStatus wait(Mode mode = Mode::FLUSH, uint64_t timeout = FENCE_WAIT_FOR_EVER);

    /**
     * Client-side wait on a Fence and destroy the Fence.
     *
     * @param fence Fence object to wait on.
     *
     * @param mode  Whether the command stream is flushed before waiting or not.
     *
     * @return  FenceStatus::CONDITION_SATISFIED on success,
     *          FenceStatus::ERROR otherwise.
     */
    static FenceStatus waitAndDestroy(Fence* fence, Mode mode = Mode::FLUSH);
};

} // namespace filament

#endif // TNT_FILAMENT_FENCE_H
