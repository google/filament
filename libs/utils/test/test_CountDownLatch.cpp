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

#include <utils/CountDownLatch.h>

#include <gtest/gtest.h>

#include <atomic>
#include <thread>

using namespace utils;

template <typename T> using sptr = std::shared_ptr<T>;

TEST(CountDownLatchTest, Simple) {

    CountDownLatch send_latch(1);
    CountDownLatch receive_latch(2);
    CountDownLatch thread_started_latch(1);

    std::atomic_int sendbuf{ 0 };
    std::atomic_int receivebuf{ 0 };

    EXPECT_EQ(0u, receive_latch.getCount());

    std::thread t([&]() -> int {
        thread_started_latch.latch();
        receive_latch.await();
        int v = receivebuf.load();
        EXPECT_EQ(1, v);
        sendbuf.store(v * 2);
        send_latch.latch();
        return 0;
    });

    thread_started_latch.await();

    // First latch (1 of 2) should not unblock the thread yet.
    receive_latch.latch();
    EXPECT_EQ(1u, receive_latch.getCount());
    EXPECT_EQ(0, sendbuf.load());

    // Store value before releasing the final latch count (2 of 2).
    receivebuf.store(1);
    receive_latch.latch();
    EXPECT_EQ(2u, receive_latch.getCount());
    // Calling latch() more than count times has no effect.
    receive_latch.latch();
    EXPECT_EQ(2u, receive_latch.getCount());

    // receive an int back
    send_latch.await();
    EXPECT_EQ(2, sendbuf.load());

    t.join();

    // Verify reset() resets getCount() (number of latch() calls since reset) back to 0.
    receive_latch.reset(3);
    EXPECT_EQ(0u, receive_latch.getCount());
    receive_latch.latch();
    EXPECT_EQ(1u, receive_latch.getCount());
}
