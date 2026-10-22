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

#include <gtest/gtest.h>

#include <utils/CountDownLatch.h>
#include <atomic>
#include <thread>

using namespace utils;

template <typename T> using sptr = std::shared_ptr<T>;

TEST(CountDownLatchTest, Simple) {

    CountDownLatch send_latch(1);
    CountDownLatch receive_latch(1);

    std::atomic_int sendbuf;
    std::atomic_int receivebuf;

    std::thread t([&]()->int {
        receive_latch.await();
        int v = receivebuf.load();
        EXPECT_EQ(1, v);
        sendbuf.store(v*2);
        send_latch.latch();
        return 0;
    });

    // send an int to the thread
    receivebuf.store(1);
    receive_latch.latch();

    // receive an int back
    send_latch.await();
    EXPECT_EQ(2, sendbuf.load());

    t.join();
}
