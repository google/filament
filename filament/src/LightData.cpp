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

#include "details/LightData.h"

#include "details/Engine.h"

#include <private/filament/UibGenerator.h>

#include <utils/architecture.h>
#include <utils/Panic.h>

namespace filament {

using namespace driver;

namespace details {

LightData::LightData(FEngine& engine) noexcept
        : mEngine(engine),
          mLightUib(UibGenerator::getLightsUib()),
          mLightsUb(mLightUib)
{
    DriverApi& driverApi = mEngine.getDriverApi();
    mLightUbh = driverApi.createUniformBuffer(mLightUib.getSize());
    driverApi.bindUniforms(BindingPoints::LIGHTS, mLightUbh);
}

LightData::~LightData() noexcept = default;

void LightData::terminate() {
    DriverApi& driverApi = mEngine.getDriverApi();
    driverApi.destroyUniformBuffer(mLightUbh);
}

void LightData::commitSlow() noexcept {
    DriverApi& driverApi = mEngine.getDriverApi();
    driverApi.updateUniformBuffer(mLightUbh, UniformBuffer(mLightsUb));
    mLightsUb.clean();
}

} // namespace details
} // namespace filament
