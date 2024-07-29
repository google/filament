/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FRAMEINFO_H
#define TNT_FILAMENT_FRAMEINFO_H

#include <filament/Renderer.h>

#include <backend/Handle.h>

#include <private/backend/DriverApi.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>

#include <array>
#include <atomic>
#include <chrono>
#include <ratio>
#include <type_traits>

#include <stdint.h>
#include <stddef.h>

namespace filament {
class FEngine;

namespace details {
struct FrameInfo {
    using duration = std::chrono::duration<float, std::milli>;
    duration frameTime{};            // frame period
    duration denoisedFrameTime{};    // frame period (median filter)
    bool valid = false;              // true if the data of the structure is valid
};
} // namespace details

struct FrameInfoImpl : public details::FrameInfo {
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    uint32_t const frameId;
    time_point beginFrame;           // main thread beginFrame time
    time_point endFrame;             // main thread endFrame time
    time_point backendBeginFrame;    // backend thread beginFrame time (makeCurrent time)
    time_point backendEndFrame;      // backend thread endFrame time (present time)
    std::atomic_bool ready{};        // true once backend thread has populated its data
    explicit FrameInfoImpl(uint32_t frameId) noexcept
        : frameId(frameId) {
    }
};

template<typename T, size_t CAPACITY>
class CircularQueue {
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = value_type const&;

    size_t capacity() const {
        return CAPACITY;
    }

    size_t size() const {
        return mSize;
    }

    bool empty() const noexcept {
        return !size();
    }

    void pop_back() noexcept {
        assert_invariant(!empty());
        --mSize;
        std::destroy_at(&mStorage[(mFront - mSize) % CAPACITY]);
    }

    void push_front(T const& v) noexcept {
        assert_invariant(size() < CAPACITY);
        mFront = advance(mFront);
        new(&mStorage[mFront]) T(v);
        ++mSize;
    }

    void push_front(T&& v) noexcept {
        assert_invariant(size() < CAPACITY);
        mFront = advance(mFront);
        new(&mStorage[mFront]) T(std::move(v));
        ++mSize;
    }

    template<typename ...Args>
    T& emplace_front(Args&&... args) noexcept {
        assert_invariant(size() < CAPACITY);
        mFront = advance(mFront);
        new(&mStorage[mFront]) T(std::forward<Args>(args)...);
        ++mSize;
        return front();
    }

    T& operator[](size_t pos) noexcept {
        assert_invariant(pos < size());
        size_t const index = (mFront + CAPACITY - pos) % CAPACITY;
        return *std::launder(reinterpret_cast<T*>(&mStorage[index]));
    }

    T const& operator[](size_t pos) const noexcept {
        return const_cast<CircularQueue&>(*this)[pos];
    }

    T const& front() const noexcept {
        assert_invariant(!empty());
        return operator[](0);
    }

    T& front() noexcept {
        assert_invariant(!empty());
        return operator[](0);
    }

private:
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    Storage mStorage[CAPACITY];
    uint32_t mFront = 0;    // always index 0
    uint32_t mSize = 0;
    [[nodiscard]] inline uint32_t advance(uint32_t v) noexcept {
        return (v + 1) % CAPACITY;
    }
};

class FrameInfoManager {
    static constexpr size_t POOL_COUNT = 4;
    static constexpr size_t MAX_FRAMETIME_HISTORY = 16u;

public:
    using duration = FrameInfoImpl::duration;
    using clock = FrameInfoImpl::clock;

    struct Config {
        uint32_t historySize;
    };

    explicit FrameInfoManager(backend::DriverApi& driver) noexcept;

    ~FrameInfoManager() noexcept;
    void terminate(backend::DriverApi& driver) noexcept;

    // call this immediately after "make current"
    void beginFrame(backend::DriverApi& driver, Config const& config, uint32_t frameId) noexcept;

    // call this immediately before "swap buffers"
    void endFrame(backend::DriverApi& driver) noexcept;

    details::FrameInfo getLastFrameInfo() const noexcept {
        // if pFront is not set yet, return FrameInfo(). But the `valid` field will be false in this case.
        return pFront ? *pFront : details::FrameInfo();
    }

    utils::FixedCapacityVector<Renderer::FrameInfo> getFrameInfoHistory(size_t historySize) const noexcept;

private:
    void denoiseFrameTime(Config const& config) noexcept;
    struct Query {
        backend::Handle<backend::HwTimerQuery> handle{};
        FrameInfoImpl* pInfo = nullptr;
    };
    std::array<Query, POOL_COUNT> mQueries;
    uint32_t mIndex = 0;                // index of current query
    uint32_t mLast = 0;                 // index of oldest query still active
    FrameInfoImpl* pFront = nullptr;    // the most recent slot with a valid frame time
    CircularQueue<FrameInfoImpl, MAX_FRAMETIME_HISTORY> mFrameTimeHistory;
};


} // namespace filament

#endif // TNT_FILAMENT_FRAMEINFO_H
