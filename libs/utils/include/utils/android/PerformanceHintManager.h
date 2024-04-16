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

#ifndef TNT_UTILS_ANDROID_PERFORMANCEHINTMANAGER_H
#define TNT_UTILS_ANDROID_PERFORMANCEHINTMANAGER_H

#include <utils/compiler.h>
#include <utils/PrivateImplementation.h>

#include <stddef.h>
#include <stdint.h>

namespace utils {

namespace details {
struct PerformanceHintManager;
} // namespace details

class UTILS_PUBLIC PerformanceHintManager :
        private PrivateImplementation<details::PerformanceHintManager> {
    friend struct details::PerformanceHintManager;
    struct SessionDetails;

public:
    class UTILS_PUBLIC Session : PrivateImplementation<SessionDetails> {
        friend class PerformanceHintManager;
        friend struct PerformanceHintManager::SessionDetails;
    public:
        Session() noexcept;
        Session(PerformanceHintManager& manager,
                int32_t const* threadIds, size_t size,
                int64_t initialTargetWorkDurationNanos) noexcept;
        ~Session() noexcept;

        Session(Session&& rhs) noexcept;
        Session& operator=(Session&& rhs) noexcept;
        Session(Session const& rhs) = delete;
        Session& operator=(Session const& rhs) = delete;

        bool isValid() const;
        int updateTargetWorkDuration(int64_t targetDurationNanos) noexcept;
        int reportActualWorkDuration(int64_t actualDurationNanos) noexcept;
    };

    PerformanceHintManager() noexcept;
    ~PerformanceHintManager() noexcept;

    bool isValid() const;

    int64_t getPreferredUpdateRateNanos() const noexcept;
};

} // namespace utils

#endif //TNT_UTILS_ANDROID_PERFORMANCEHINTMANAGER_H
