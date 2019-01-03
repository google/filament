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

#include <iostream>

#include <gtest/gtest.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Exposure.h>

using namespace filament;

class FilamentExposureWithEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = Engine::create(Engine::Backend::OPENGL);
    }

    void TearDown() override {
        Engine::destroy(&engine);
    }

    Engine* engine = nullptr;
};

class FilamentExposureTest : public ::testing::Test {
};

TEST_F(FilamentExposureWithEngineTest, SetExposure) {
    using namespace filament;

    Camera* camera = engine->createCamera();
    camera->setExposure(16.0f, 1 / 125.0f, 100.0f);

    EXPECT_FLOAT_EQ(16.0f, camera->getAperture());
    EXPECT_FLOAT_EQ(1 / 125.0f, camera->getShutterSpeed());
    EXPECT_FLOAT_EQ(100.0f, camera->getSensitivity());

    camera->setExposure(0.0f, 1 / 100000.0f, 1.0f);

    EXPECT_GT(camera->getAperture(), 0.0f);
    EXPECT_GT(camera->getShutterSpeed(), 1 / 100000.0f);
    EXPECT_GT(camera->getSensitivity(), 1.0f);

    camera->setExposure(512, 3600.0f, 1000000.0f);

    EXPECT_LT(camera->getAperture(), 512.0f);
    EXPECT_LT(camera->getShutterSpeed(), 3600.0f);
    EXPECT_LT(camera->getSensitivity(), 1000000.0f);

    engine->destroy(camera);
}

TEST_F(FilamentExposureWithEngineTest, ComputeEV100) {
    using namespace filament;

    Camera* camera = engine->createCamera();

    camera->setExposure(16.0f, 1 / 125.0f, 100.0f);
    int32_t ev100 = static_cast<int32_t>(roundf(Exposure::ev100(*camera)));
    EXPECT_EQ(15, ev100);

    camera->setExposure(16.0f, 1 / 125.0f, 400.0f);
    ev100 = static_cast<int32_t>(roundf(Exposure::ev100(*camera)));
    EXPECT_EQ(13, ev100);

    camera->setExposure(8.0f, 1 / 125.0f, 100.0f);
    ev100 = static_cast<int32_t>(roundf(Exposure::ev100(*camera)));
    EXPECT_EQ(13, ev100);

    camera->setExposure(16.0f, 1 / 30.0f, 100.0f);
    ev100 = static_cast<int32_t>(roundf(Exposure::ev100(*camera)));
    EXPECT_EQ(13, ev100);

    camera->setExposure(22.0f, 1 / 125.0f, 100.0f);
    ev100 = static_cast<int32_t>(roundf(Exposure::ev100(*camera)));
    EXPECT_EQ(16, ev100);

    camera->setExposure(16.0f, 1 / 250.0f, 100.0f);
    ev100 = static_cast<int32_t>(roundf(Exposure::ev100(*camera)));
    EXPECT_EQ(16, ev100);

    camera->setExposure(16.0f, 1 / 125.0f, 50.0f);
    ev100 = static_cast<int32_t>(roundf(Exposure::ev100(*camera)));
    EXPECT_EQ(16, ev100);

    engine->destroy(camera);
}

TEST_F(FilamentExposureTest, ComputeEV100FromLuminance) {
    using namespace filament;

    // 4,096 cd/m^2 is equivalent to a "sunny 16" setting: f/16, 1/125s, ISO 100
    int32_t ev100 = static_cast<int32_t>(roundf(Exposure::ev100FromLuminance(4096)));
    EXPECT_EQ(15, ev100);

    int32_t sunny16 = static_cast<int32_t>(roundf(Exposure::ev100(16.0f, 1 / 125.0f, 100.0f)));
    EXPECT_EQ(ev100, sunny16);
}

TEST_F(FilamentExposureTest, ComputeLuminanceFromEV100) {
    using namespace filament;

    // 4,096 cd/m^2 is equivalent to a "sunny 16" setting: f/16, 1/125s, ISO 100
    int32_t luminance = static_cast<int32_t>(roundf(Exposure::luminance(15.0f)));
    EXPECT_EQ(4096, luminance);

    int32_t sunny16 = static_cast<int32_t>(roundf(Exposure::luminance(16.0f, 1 / 125.0f, 100.0f)));
    EXPECT_EQ(4000, sunny16);
}

TEST_F(FilamentExposureTest, ComputeEV100FromIlluminance) {
    using namespace filament;

    // 81,920 lux is equivalent to a "sunny 16" setting: f/16, 1/125s, ISO 100
    int32_t ev100 = static_cast<int32_t>(roundf(Exposure::ev100FromIlluminance(81920)));
    EXPECT_EQ(15, ev100);

    int32_t sunny16 = static_cast<int32_t>(roundf(Exposure::ev100(16.0f, 1 / 125.0f, 100.0f)));
    EXPECT_EQ(ev100, sunny16);
}


TEST_F(FilamentExposureTest, ComputeIlluminanceFromEV100) {
    using namespace filament;

    // 81,920 lux is equivalent to a "sunny 16" setting: f/16, 1/125s, ISO 100
    int32_t illuminance = static_cast<int32_t>(roundf(Exposure::illuminance(15.0f)));
    EXPECT_EQ(81920, illuminance);

    int32_t sunny16 = static_cast<int32_t>(roundf(Exposure::illuminance(16.0f, 1 / 125.0f, 100.0f)));
    EXPECT_EQ(80000, sunny16);
}
