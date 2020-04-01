/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_METALTIMERQUERY_H
#define TNT_FILAMENT_DRIVER_METALTIMERQUERY_H

#import <Metal/Metal.h>

#include <chrono>

namespace filament {
namespace backend {
namespace metal {

struct MetalTimerQuery;
struct MetalContext;

class TimerQueryInterface {
public:
    virtual ~TimerQueryInterface();
    virtual void beginTimeElapsedQuery(MetalTimerQuery* query) = 0;
    virtual void endTimeElapsedQuery(MetalTimerQuery* query) = 0;
    virtual bool getQueryResult(MetalTimerQuery* query, uint64_t* outElapsedTime) = 0;

protected:
    using clock = std::chrono::steady_clock;
};

// Uses MTLSharedEvents to implement timer queries.
// Only available on >= macOS 10.14, iOS 12.0.
class API_AVAILABLE(macos(10.14), ios(12.0)) TimerQueryFence : public TimerQueryInterface {
public:
    explicit TimerQueryFence(MetalContext& context) : mContext(context) {}

    void beginTimeElapsedQuery(MetalTimerQuery* query) override;
    void endTimeElapsedQuery(MetalTimerQuery* query) override;
    bool getQueryResult(MetalTimerQuery* query, uint64_t* outElapsedTime) override;

private:
    MetalContext& mContext;
};

class TimerQueryNoop : public TimerQueryInterface {
public:
    void beginTimeElapsedQuery(MetalTimerQuery* query) override {}
    void endTimeElapsedQuery(MetalTimerQuery* query) override {}
    bool getQueryResult(MetalTimerQuery* query, uint64_t* outElapsedTime) override;
};

} // namespace metal
} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_METALTIMERQUERY_H
