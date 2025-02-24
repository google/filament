/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "layer_validation_tests.h"

// Helper class that fails the tests with a stuck worker thread.
// Its purpose is similar to an assert. We assume the code is correct.
// Then, in case of a bug or regression, the CI will continue to operate.
// Usage Example:
//  TEST_F(VkLayerTest, MyTrickyTestWithThreads) {
//      // The constructor parameter is the number of the worker threads
//      ThreadTimeoutHelper timeout_helper(2);
//      auto worker = [&]() {
//          auto timeout_guard = timeout_helper.ThreadGuard();
//          // Some code here
//      };
//      std::thread t0(worker);
//      std::thread t1(worker);
//      if (!timeout_helper.WaitForThreads(60)) ADD_FAILURE() << "It's time to move on";
//      t0.join();
//      t1.join();
//  }
class ThreadTimeoutHelper {
  public:
    explicit ThreadTimeoutHelper(int thread_count = 1) : active_threads_(thread_count) {}
    bool WaitForThreads(int timeout_in_seconds);

    struct Guard {
        Guard(ThreadTimeoutHelper &timeout_helper) : timeout_helper_(timeout_helper) {}
        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;

        ~Guard() { timeout_helper_.OnThreadDone(); }

        ThreadTimeoutHelper &timeout_helper_;
    };
    // Mandatory elision of copy/move operations guarantees the destructor is not called
    // (even in the presence of copy/move constructor) and the object is constructed directly
    // into the destination storage: https://en.cppreference.com/w/cpp/language/copy_elision
    Guard ThreadGuard() { return Guard(*this); }

  private:
    void OnThreadDone();

    std::mutex active_thread_mutex_;
    int active_threads_;

    std::condition_variable cv_;
    std::mutex mutex_;
};

#if GTEST_IS_THREADSAFE
struct ThreadTestData {
    VkCommandBuffer commandBuffer;
    VkDevice device;
    VkEvent event;
    VkDescriptorSet descriptorSet;
    VkBuffer buffer;
    uint32_t binding;
    std::atomic<bool> *bailout;
};

void AddToCommandBuffer(ThreadTestData *);
void UpdateDescriptor(ThreadTestData *);
#endif  // GTEST_IS_THREADSAFE

void ReleaseNullFence(ThreadTestData *);