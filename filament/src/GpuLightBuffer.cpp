/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "details/GpuLightBuffer.h"

#include "details/Engine.h"

#include <filament/EngineEnums.h>

#include <private/filament/UibGenerator.h>

namespace filament {

using namespace driver;

namespace details {

GpuLightBuffer::GpuLightBuffer(FEngine& engine) noexcept
        : mLightsUb(UibGenerator::getLightsUib()) {
    DriverApi& driverApi = engine.getDriverApi();
    mLightUbh = driverApi.createUniformBuffer(mLightsUb.getSize());
    driverApi.bindUniformBuffer(BindingPoints::LIGHTS, mLightUbh);
}

GpuLightBuffer::~GpuLightBuffer() noexcept = default;

void GpuLightBuffer::terminate(FEngine& engine) {
    DriverApi& driverApi = engine.getDriverApi();
    driverApi.destroyUniformBuffer(mLightUbh);
}

void GpuLightBuffer::commit(FEngine& engine) noexcept {
    if (UTILS_UNLIKELY(mLightsUb.isDirty())) {
        commitSlow(engine);
    }
    engine.getDriverApi().bindUniformBuffer(BindingPoints::LIGHTS, mLightUbh);
}

void GpuLightBuffer::commitSlow(FEngine& engine) noexcept {
    DriverApi& driverApi = engine.getDriverApi();
    driverApi.updateUniformBuffer(mLightUbh, UniformBuffer(mLightsUb));
    mLightsUb.clean();
}

} // namespace details
} // namespace filament
