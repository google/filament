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

#include "details/DebugRegistry.h"

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

namespace filament {

using namespace math;

bool DebugRegistry::hasProperty(const char* name) const noexcept {
    return downcast(this)->hasProperty(name);
}

bool DebugRegistry::setProperty(const char* name, bool v) noexcept {
    return downcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, int v) noexcept {
    return downcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, float v) noexcept {
    return downcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, float2 v) noexcept {
    return downcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, float3 v) noexcept {
    return downcast(this)->setProperty(name, v);
}

bool DebugRegistry::setProperty(const char* name, float4 v) noexcept {
    return downcast(this)->setProperty(name, v);
}


bool DebugRegistry::getProperty(const char* name, bool* v) const noexcept {
    return downcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, int* v) const noexcept {
    return downcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, float* v) const noexcept {
    return downcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, float2* v) const noexcept {
    return downcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, float3* v) const noexcept {
    return downcast(this)->getProperty(name, v);
}

bool DebugRegistry::getProperty(const char* name, float4* v) const noexcept {
    return downcast(this)->getProperty(name, v);
}

void *DebugRegistry::getPropertyAddress(const char *name) {
    return  downcast(this)->getPropertyAddress(name);
}

void const *DebugRegistry::getPropertyAddress(const char *name) const noexcept {
    return  downcast(this)->getPropertyAddress(name);
}

DebugRegistry::DataSource DebugRegistry::getDataSource(const char* name) const noexcept {
    return  downcast(this)->getDataSource(name);
}


} // namespace filament

