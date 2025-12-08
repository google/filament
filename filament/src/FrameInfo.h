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

#include <details/SwapChain.h>

#include <backend/Platform.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <private/backend/DriverApi.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/AsyncJobQueue.h>
#include <utils/FixedCapacityVector.h>

#include <array>
#include <atomic>
#include <chrono>
#include <iterator>
#include <ratio>
#include <type_traits>

#include <stdint.h>
#include <stddef.h>

namespace filament {
class FEngine;

namespace details {
struct FrameInfo {
    using duration = std::chrono::duration<float, std::milli>;
    duration gpuFrameDuration{};     // frame period
    duration denoisedFrameTime{};    // frame period (median filter)
    bool valid = false;              // true if the data of the structure is valid
};
} // namespace details

struct FrameInfoImpl : public details::FrameInfo {
    using FrameTimestamps = backend::FrameTimestamps;
    using CompositorTiming = backend::CompositorTiming;
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    uint32_t frameId;
    // main thread beginFrame time
    time_point beginFrame{};
    // main thread endFrame time
    time_point endFrame{};
    // backend thread beginFrame time (makeCurrent time)
    time_point backendBeginFrame{};
    // backend thread endFrame time (present time)
    time_point backendEndFrame{};
    // the frame is done rendering on the gpu
    time_point gpuFrameComplete{};
    // vsync time
    time_point vsync{};
    // Actual presentation time of this frame
    FrameTimestamps::time_point_ns displayPresent{ FrameTimestamps::PENDING };
    // deadline for queuing a frame [ns]
    CompositorTiming::time_point_ns presentDeadline{ FrameTimestamps::INVALID };
    // display refresh rate [ns]
    CompositorTiming::duration_ns displayPresentInterval{ FrameTimestamps::INVALID };
    // time between the start of composition and the expected present time [ns]
    CompositorTiming::duration_ns compositionToPresentLatency{ FrameTimestamps::INVALID };
    // system's expected present time [ns]
    FrameTimestamps::time_point_ns expectedPresentTime{ FrameTimestamps::INVALID };

    // the fence used for gpuFrameComplete
    backend::FenceHandle fence{};
    // true once backend thread has populated its data
    std::atomic_bool ready{};
    explicit FrameInfoImpl(uint32_t const id) noexcept
        : frameId(id) {
    }

    ~FrameInfoImpl() noexcept {
        assert_invariant(!fence);
    }

    FrameInfoImpl(FrameInfoImpl& rhs) noexcept = delete;
    FrameInfoImpl& operator=(FrameInfoImpl& rhs) noexcept = delete;
};

template<typename T, size_t CAPACITY>
class CircularQueue {
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = value_type const&;

private:
    template<typename U>
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename std::remove_const<T>::type;
        using difference_type = std::ptrdiff_t;
        using pointer = U*;
        using reference = U&;
        using QueuePtr = typename std::conditional<std::is_const<U>::value,
                const CircularQueue*, CircularQueue*>::type;

        Iterator() = default;
        Iterator(QueuePtr queue, size_t pos) noexcept : mQueue(queue), mPos(pos) {}

        // allow conversion from iterator to const_iterator
        operator Iterator<const T>() const { return { mQueue, mPos }; }

        reference operator*() const { return (*mQueue)[mPos]; }
        pointer operator->() const { return &(*mQueue)[mPos]; }
        Iterator& operator++() { ++mPos; return *this; }
        Iterator operator++(int) { Iterator temp = *this; ++(*this); return temp; }

        friend bool operator==(const Iterator& a, const Iterator& b) { return a.mQueue == b.mQueue && a.mPos == b.mPos; }
        friend bool operator!=(const Iterator& a, const Iterator& b) { return !(a == b); }

    private:
        QueuePtr mQueue = nullptr;
        size_t mPos = 0;
    };

public:
    using iterator = Iterator<T>;
    using const_iterator = Iterator<const T>;

    CircularQueue() = default;

    ~CircularQueue() {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0, c = mSize; i < c; ++i) {
                size_t const index = (mFront + CAPACITY - i) % CAPACITY;
                std::destroy_at(std::launder(reinterpret_cast<T*>(&mStorage[index])));
            }
        }
    }

    CircularQueue(const CircularQueue&) = delete;
    CircularQueue& operator=(const CircularQueue&) = delete;

    CircularQueue(CircularQueue&& other) noexcept {
        for (size_t i = 0; i < other.mSize; i++) {
            size_t const index = (other.mFront + CAPACITY - i) % CAPACITY;
            new(&mStorage[index]) T(std::move(*std::launder(reinterpret_cast<T*>(&other.mStorage[index]))));
        }
        mFront = other.mFront;
        mSize = other.mSize;
        other.mSize = 0;
    }

    CircularQueue& operator=(CircularQueue&& other) noexcept {
        if (this != &other) {
            this->~CircularQueue();
            new(this) CircularQueue(std::move(other));
        }
        return *this;
    }

    iterator begin() noexcept { return iterator(this, 0); }
    iterator end() noexcept { return iterator(this, size()); }
    const_iterator begin() const noexcept { return const_iterator(this, 0); }
    const_iterator end() const noexcept { return const_iterator(this, size()); }
    const_iterator cbegin() const noexcept { return const_iterator(this, 0); }
    const_iterator cend() const noexcept { return const_iterator(this, size()); }

    size_t capacity() const noexcept {
        return CAPACITY;
    }

    size_t size() const noexcept {
        return mSize;
    }

    bool empty() const noexcept {
        return !size();
    }

    void pop_back() noexcept {
        assert_invariant(!empty());
        --mSize;
        size_t const index = (mFront + CAPACITY - mSize) % CAPACITY;
        std::destroy_at(std::launder(reinterpret_cast<T*>(&mStorage[index])));
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

    T& operator[](size_t const pos) noexcept {
        assert_invariant(pos < size());
        size_t const index = (mFront + CAPACITY - pos) % CAPACITY;
        return *std::launder(reinterpret_cast<T*>(&mStorage[index]));
    }

    T const& operator[](size_t pos) const noexcept {
        assert_invariant(pos < size());
        size_t const index = (mFront + CAPACITY - pos) % CAPACITY;
        return *std::launder(reinterpret_cast<T const*>(&mStorage[index]));
    }

    T const& front() const noexcept {
        assert_invariant(!empty());
        return operator[](0);
    }

    T& front() noexcept {
        assert_invariant(!empty());
        return operator[](0);
    }

    T const& back() const noexcept {
        assert_invariant(!empty());
        return operator[](size() - 1);
    }

    T& back() noexcept {
        assert_invariant(!empty());
        return operator[](size() - 1);
    }

private:
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    Storage mStorage[CAPACITY];
    uint32_t mFront = 0;
    uint32_t mSize = 0;
    [[nodiscard]] uint32_t advance(uint32_t const v) noexcept {
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

    explicit FrameInfoManager(FEngine& engine, backend::DriverApi& driver) noexcept;

    ~FrameInfoManager() noexcept;

    // The command queue must be empty before calling terminate()
    void terminate(FEngine& engine) noexcept;

    // call this immediately after "make current"
    void beginFrame(FSwapChain* swapChain, backend::DriverApi& driver,
            Config const& config, uint32_t frameId, std::chrono::steady_clock::time_point vsync) noexcept;

    // call this immediately before "swap buffers"
    void endFrame(backend::DriverApi& driver) noexcept;

    details::FrameInfo getLastFrameInfo() const noexcept {
        // if pFront is not set yet, return FrameInfo(). But the `valid` field will be false in this case.
        return pFront ? *pFront : details::FrameInfo{};
    }

    void updateUserHistory(FSwapChain* swapChain, backend::DriverApi& driver);

    utils::FixedCapacityVector<Renderer::FrameInfo>
            getFrameInfoHistory(size_t historySize = MAX_FRAMETIME_HISTORY) const;

private:
    using FrameHistoryQueue = CircularQueue<FrameInfoImpl, MAX_FRAMETIME_HISTORY>;
    static void denoiseFrameTime(FrameHistoryQueue& history, Config const& config) noexcept;
    struct Query {
        backend::Handle<backend::HwTimerQuery> handle{};
        FrameInfoImpl* pInfo = nullptr;
    };
    utils::FixedCapacityVector<Renderer::FrameInfo> mUserFrameHistory;
    std::array<Query, POOL_COUNT> mQueries{};
    uint32_t mIndex = 0;                // index of current query
    uint32_t mLast = 0;                 // index of oldest query still active
    FrameInfoImpl* pFront = nullptr;    // the most recent slot with a valid frame time
    FrameHistoryQueue mFrameTimeHistory{};
    utils::AsyncJobQueue mJobQueue;
    FSwapChain* mLastSeenSwapChain = nullptr;
    bool const mHasTimerQueries = false;
    bool const mDisableGpuFrameComplete = false;
};


} // namespace filament

#endif // TNT_FILAMENT_FRAMEINFO_H
