/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_COMPUTE_TEST_H
#define TNT_COMPUTE_TEST_H

#include <gtest/gtest.h>

#include "private/backend/CommandBufferQueue.h"
#include "private/backend/DriverApi.h"

class ComputeTest : public ::testing::Test {
    static filament::backend::Backend sBackend;

    filament::backend::Driver* driver = nullptr;
    filament::backend::CommandBufferQueue commandBufferQueue;
    std::unique_ptr<filament::backend::DriverApi> commandStream;

protected:
    void SetUp() override;
    void TearDown() override;

    void executeCommands();
    void flushAndWait();
    filament::backend::DriverApi& getDriverApi() { return *commandStream; }
    filament::backend::Driver& getDriver() { return *driver; }

    static filament::backend::Backend getBackend() noexcept { return sBackend; };
    bool isMobile() noexcept { return driver->getShaderModel() == filament::backend::ShaderModel::MOBILE; };

public:
    static void init(filament::backend::Backend backend);

    ComputeTest();
    ~ComputeTest() override;
};

#endif // TNT_COMPUTE_TEST_H
