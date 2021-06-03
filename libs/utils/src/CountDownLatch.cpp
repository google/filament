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

namespace utils {

CountDownLatch::CountDownLatch(size_t count)  noexcept
    : m_initial_count(static_cast<uint32_t>(count)),
     m_remaining_count(static_cast<uint32_t>(count)) {
}

void CountDownLatch::reset(size_t count) noexcept {
    std::lock_guard<Mutex> guard(m_lock);
    m_initial_count = static_cast<uint32_t>(count);
    m_remaining_count = static_cast<uint32_t>(count);
    if (count == 0) {
        m_cv.notify_all();
    }
}

void CountDownLatch::await() noexcept {
    std::unique_lock<Mutex> guard(m_lock);
    m_cv.wait(guard, [this]{ return m_remaining_count == 0; } );
}

void CountDownLatch::latch() noexcept {
    std::lock_guard<Mutex> guard(m_lock);
    if (m_remaining_count > 0) {
        if (--m_remaining_count == 0) {
            m_cv.notify_all();
        }
    }
}

size_t CountDownLatch::getCount() const noexcept {
    std::lock_guard<Mutex> guard(m_lock);
    return m_initial_count - m_remaining_count;
}

} // namespace utils
