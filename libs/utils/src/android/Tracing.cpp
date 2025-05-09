/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <utils/compiler.h>
#include <private/utils/Tracing.h>

#include <perfetto/perfetto.h>

PERFETTO_TRACK_EVENT_STATIC_STORAGE_IN_NAMESPACE(tracing);

namespace {

class SystraceStaticInitialization {
public:
    SystraceStaticInitialization() {
        perfetto::TracingInitArgs args;
        args.backends |= perfetto::kSystemBackend;
        perfetto::Tracing::Initialize(args);
        tracing::TrackEvent::Register();
    }
};

UTILS_UNUSED SystraceStaticInitialization sTracingStaticInitialization{};

}

