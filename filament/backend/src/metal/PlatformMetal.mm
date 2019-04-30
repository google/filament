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

#include "PlatformMetal.h"
#include "MetalDriverFactory.h"

namespace filament {

namespace backend {
MetalPlatform::~MetalPlatform() = default;
} // namespace backend


using namespace backend;

Driver* PlatformMetal::createDriver(void* sharedContext) noexcept {
    return MetalDriverFactory::create(this);
}

PlatformMetal::~PlatformMetal() = default;


} // namespace filament
