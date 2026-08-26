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

#include <cstdint>

namespace filament {
namespace backend {

struct MetalTimerQuery;
struct MetalContext;

// Implements timer queries by reading GPUStartTime/GPUEndTime off the frame's command buffer.
// Those are available on every OS version Filament supports, so there is no fallback.
//
// NOTE: the resolution is a whole command buffer. Several timer queries issued inside one
// command buffer all report that command buffer's execution time. Filament only measures frame
// boundaries, where this is what is wanted.
class MetalTimerQueryImpl {
public:
    explicit MetalTimerQueryImpl(MetalContext& context) : mContext(context) {}

    void beginTimeElapsedQuery(MetalTimerQuery* query);
    void endTimeElapsedQuery(MetalTimerQuery* query);
    bool getQueryResult(MetalTimerQuery* query, uint64_t* outElapsedTime);

private:
    MetalContext& mContext;
};

} // namespace backend
} // namespace filament

#endif //TNT_FILAMENT_DRIVER_METALTIMERQUERY_H
