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

#ifndef TNT_FILAMENT_ASYNC_CALLBACKS_TEST_H
#define TNT_FILAMENT_ASYNC_CALLBACKS_TEST_H

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/Invocable.h>

#include <stdint.h>

namespace filament {

class UTILS_PUBLIC AsyncCallbacksTest {
public:
    using FrameScheduledCallback = backend::FrameScheduledCallback;
    using FrameCompletedCallback = utils::Invocable<void(AsyncCallbacksTest* UTILS_NONNULL)>;
    using SimpleCallback = utils::Invocable<void()>;

    /**
     * Trailing auxiliary parameter (flags follows handler and callback).
     */
    void setFrameScheduledCallback(backend::CallbackHandler* UTILS_NULLABLE handler = nullptr,
            FrameScheduledCallback&& callback = {}, uint64_t flags = 0);

    /**
     * Standard 2-param noexcept method (bypasses wrapJni).
     */
    void setFrameCompletedCallback(backend::CallbackHandler* UTILS_NULLABLE handler = nullptr,
            FrameCompletedCallback&& callback = {}) noexcept;

    /**
     * Leading auxiliary parameter (channel precedes handler and callback).
     */
    void registerChannelCallback(uint32_t channel,
            backend::CallbackHandler* UTILS_NULLABLE handler,
            SimpleCallback&& callback);

    /**
     * Mixed leading and trailing auxiliary parameters.
     */
    void dispatchCustom(uint32_t channel,
            backend::CallbackHandler* UTILS_NULLABLE handler,
            SimpleCallback&& callback,
            float timeout,
            uint32_t flags);
};

} // namespace filament

#endif // TNT_FILAMENT_ASYNC_CALLBACKS_TEST_H
