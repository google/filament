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

#include "ComputeTest.h"
#include "PlatformRunner.h"

#include <backend/Platform.h>
#include <private/backend/PlatformFactory.h>


#include "private/backend/CommandBufferQueue.h"
#include "private/backend/DriverApi.h"

using namespace filament;
using namespace filament::backend;

int main(int argc, char* argv[]) {
    auto backend = (Backend)test::parseArgumentsForBackend(argc, argv);
    ComputeTest::init(backend);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// ------------------------------------------------------------------------------------------------

static constexpr size_t CONFIG_MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
static constexpr size_t CONFIG_COMMAND_BUFFERS_SIZE     = 3 * CONFIG_MIN_COMMAND_BUFFERS_SIZE;

Backend ComputeTest::sBackend = Backend::NOOP;

void ComputeTest::init(Backend backend) {
    sBackend = backend;
}

ComputeTest::ComputeTest()
    : commandBufferQueue(CONFIG_MIN_COMMAND_BUFFERS_SIZE, CONFIG_COMMAND_BUFFERS_SIZE,
            /*paused=*/false) {
}

ComputeTest::~ComputeTest() = default;

void ComputeTest::SetUp() {
    Test::SetUp();

    auto backend = sBackend;
    Platform* platform = PlatformFactory::create(&backend);

    GTEST_ASSERT_EQ(uint8_t(backend), uint8_t(sBackend));

    driver = platform->createDriver(nullptr, {});
    commandStream = std::make_unique<CommandStream>(*driver, commandBufferQueue.getCircularBuffer());

    // We need at least feature level 2 to run the Compute tests
    if (driver->getFeatureLevel() < FeatureLevel::FEATURE_LEVEL_2) {
        GTEST_SKIP();
    }
}

void ComputeTest::TearDown() {
    Test::TearDown();
    flushAndWait();
    driver->terminate();
    delete driver;
}


void ComputeTest::executeCommands() {
    commandBufferQueue.flush();
    auto buffers = commandBufferQueue.waitForCommands();
    for (auto& item : buffers) {
        if (UTILS_LIKELY(item.begin)) {
            getDriverApi().execute(item.begin);
            commandBufferQueue.releaseBuffer(item);
        }
    }
}

void ComputeTest::flushAndWait() {
    auto& api = getDriverApi();
    api.finish();
    executeCommands();
}
