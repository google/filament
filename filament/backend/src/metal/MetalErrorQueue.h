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

#ifndef TNT_FILAMENT_DRIVER_METALERRORQUEUE_H
#define TNT_FILAMENT_DRIVER_METALERRORQUEUE_H

#import <Foundation/Foundation.h>

#include <utils/compiler.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

class MetalErrorQueue {
public:
    void push(NSError* error) {
        std::lock_guard<std::mutex> lock(mMutex);
        mErrors.push_back(error);
        mHasErrors.store(true);
    }

    void flush(const std::function<void(NSError*)>& callback) {
        if (UTILS_LIKELY(!mHasErrors.load())) {
            return;
        }

        std::vector<NSError*> errors;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            std::swap(mErrors, errors);
            mHasErrors.store(false);
        }

        for (const auto& error: errors) {
            callback(error);
        }
    }

private:
    std::vector<NSError*> mErrors;
    std::mutex mMutex;

    // Optimization to avoid locking the mutex at each call to flush.
    std::atomic<bool> mHasErrors;
};


#endif // TNT_FILAMENT_DRIVER_METALERRORQUEUE_H
