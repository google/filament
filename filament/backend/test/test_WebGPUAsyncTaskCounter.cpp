/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <gtest/gtest.h>

#include "webgpu/utils/AsyncTaskCounter.h"

#include <chrono>
#include <future>
#include <thread>

namespace test {

namespace {

constexpr int TOLERANCE_MILLISECONDS{ 5 };
constexpr int SMALLER_JOB_MILLISECONDS{ 10 };
static_assert(SMALLER_JOB_MILLISECONDS > TOLERANCE_MILLISECONDS);
constexpr int LARGER_JOB_MILLISECONDS{ 20 };
static_assert(LARGER_JOB_MILLISECONDS > (SMALLER_JOB_MILLISECONDS + TOLERANCE_MILLISECONDS));

} // namespace

TEST(AsyncTaskCounter, noWorkJobs) {
    filament::backend::webgpuutils::AsyncTaskCounter counter{};
    const auto waitFuture{ std::async(std::launch::async,
            [&counter] { counter.waitForAllToFinish(); }) };
    const auto status{ waitFuture.wait_for(std::chrono::milliseconds(TOLERANCE_MILLISECONDS)) };
    ASSERT_NE(status, std::future_status::timeout)
            << "Timed out waiting for no work to finish. This should have returned immediately. "
               "Tolerance/timeout: "
            << TOLERANCE_MILLISECONDS << " milliseconds.";
}

TEST(AsyncTaskCounter, oneWorkJob) {
    filament::backend::webgpuutils::AsyncTaskCounter counter{};
    const auto task{ [&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(SMALLER_JOB_MILLISECONDS));
        counter.finishTask();
    } };
    const auto startTime{ std::chrono::steady_clock::now() };
    counter.startTask();
    std::thread t{ task };
    const auto waitFuture{ std::async(std::launch::async,
            [&counter] { counter.waitForAllToFinish(); }) };
    const auto expectedDuration{ SMALLER_JOB_MILLISECONDS };
    const auto status{ waitFuture.wait_for(
            std::chrono::milliseconds(expectedDuration + TOLERANCE_MILLISECONDS)) };
    const auto endTime{ std::chrono::steady_clock::now() };
    const auto duration{ std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime) };
    ASSERT_NE(status, std::future_status::timeout)
            << "Timed out waiting for one work item to finish. Expected job duration: "
            << expectedDuration
            << " milliseconds. Timeout: " << (expectedDuration + TOLERANCE_MILLISECONDS)
            << " milliseconds.";
    ASSERT_GT(duration, std::chrono::milliseconds(expectedDuration - TOLERANCE_MILLISECONDS))
            << "The one job should have taken at least its expected duration of "
            << expectedDuration << " milliseconds (" << TOLERANCE_MILLISECONDS
            << " milliseconds tolerance), but took " << duration << " milliseconds";
    t.join();
}

TEST(AsyncTaskCounter, multipleWorkJobs) {
    filament::backend::webgpuutils::AsyncTaskCounter counter{};
    const auto smallerTasks{ [&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(SMALLER_JOB_MILLISECONDS));
        // validates the counter incrementing and decrementing before the work is done.
        counter.startTask();
        counter.finishTask();
        std::this_thread::sleep_for(std::chrono::milliseconds(SMALLER_JOB_MILLISECONDS));
        counter.finishTask();
    } };
    const auto largerTask{ [&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(LARGER_JOB_MILLISECONDS));
        counter.finishTask();
    } };
    const auto startTime{ std::chrono::steady_clock::now() };
    counter.startTask();
    std::thread t1{ smallerTasks };
    counter.startTask();
    std::thread t2{ largerTask };
    const auto expectedDuration{ std::max(SMALLER_JOB_MILLISECONDS * 2, LARGER_JOB_MILLISECONDS) };
    const auto waitFuture{ std::async(std::launch::async,
            [&counter] { counter.waitForAllToFinish(); }) };
    const auto status{ waitFuture.wait_for(
            std::chrono::milliseconds(expectedDuration + TOLERANCE_MILLISECONDS)) };
    const auto endTime{ std::chrono::steady_clock::now() };
    const auto duration{ std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime) };
    ASSERT_NE(status, std::future_status::timeout)
            << "Timed out waiting for 3 work items to finish. Expected job duration: "
            << expectedDuration
            << " milliseconds. Timeout: " << (expectedDuration + TOLERANCE_MILLISECONDS)
            << " milliseconds.";
    ASSERT_GT(duration, std::chrono::milliseconds(expectedDuration - TOLERANCE_MILLISECONDS))
            << "The jobs should have taken at least their expected duration of " << expectedDuration
            << " milliseconds (" << TOLERANCE_MILLISECONDS << " milliseconds tolerance), but took "
            << duration << " milliseconds";
    t1.join();
    t2.join();
}

TEST(AsyncTaskCounter, counterReuse) {
    filament::backend::webgpuutils::AsyncTaskCounter counter{};
    const auto batchOfTasks{ [&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(SMALLER_JOB_MILLISECONDS));
        counter.startTask();
        counter.finishTask();
        std::this_thread::sleep_for(std::chrono::milliseconds(SMALLER_JOB_MILLISECONDS));
        counter.finishTask();
    } };
    // first batch of tasks...
    const auto firstStartTime{ std::chrono::steady_clock::now() };
    counter.startTask();
    std::thread firstBatch{ batchOfTasks };
    const auto firstWaitFuture{ std::async(std::launch::async,
            [&counter] { counter.waitForAllToFinish(); }) };
    const auto expectedDuration{ SMALLER_JOB_MILLISECONDS * 2 };
    const auto firstStatus{ firstWaitFuture.wait_for(
            std::chrono::milliseconds(expectedDuration + TOLERANCE_MILLISECONDS)) };
    const auto firstEndTime{ std::chrono::steady_clock::now() };
    const auto firstDuration{ std::chrono::duration_cast<std::chrono::milliseconds>(
            firstEndTime - firstStartTime) };
    ASSERT_NE(firstStatus, std::future_status::timeout)
            << "Timed out waiting for first batch to finish. Expected job duration: "
            << expectedDuration
            << " milliseconds. Timeout: " << (expectedDuration + TOLERANCE_MILLISECONDS)
            << " milliseconds.";
    ASSERT_GT(firstDuration, std::chrono::milliseconds(expectedDuration - TOLERANCE_MILLISECONDS))
            << "The first batch of tasks should have taken at least its expected duration of "
            << expectedDuration << " milliseconds (" << TOLERANCE_MILLISECONDS
            << " milliseconds tolerance), but took " << firstDuration << " milliseconds";
    firstBatch.join();
    // second batch of tasks...
    const auto secondStartTime{ std::chrono::steady_clock::now() };
    counter.startTask();
    std::thread secondBatch{ batchOfTasks };
    const auto secondWaitFuture{ std::async(std::launch::async,
            [&counter] { counter.waitForAllToFinish(); }) };
    const auto secondStatus{ secondWaitFuture.wait_for(
            std::chrono::milliseconds(expectedDuration + TOLERANCE_MILLISECONDS)) };
    const auto secondEndTime{ std::chrono::steady_clock::now() };
    const auto secondDuration{ std::chrono::duration_cast<std::chrono::milliseconds>(
            secondEndTime - secondStartTime) };
    ASSERT_NE(secondStatus, std::future_status::timeout)
            << "Timed out waiting for second batch to finish. Expected job duration: "
            << expectedDuration
            << " milliseconds. Timeout: " << (expectedDuration + TOLERANCE_MILLISECONDS)
            << " milliseconds.";
    ASSERT_GT(secondDuration, std::chrono::milliseconds(expectedDuration - TOLERANCE_MILLISECONDS))
            << "The second batch of tasks should have taken at least its expected duration of "
            << expectedDuration << " milliseconds (" << TOLERANCE_MILLISECONDS
            << " milliseconds tolerance), but took " << secondDuration << " milliseconds";
    secondBatch.join();
}

} // namespace test
