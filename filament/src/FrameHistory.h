/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FRAMEHISTORY_H
#define TNT_FILAMENT_FRAMEHISTORY_H

#include <fg/FrameGraphId.h>
#include <fg/FrameGraphTexture.h>

#include <utils/compiler.h>

#include <math/mat4.h>
#include <math/vec2.h>

#include <memory>

#include <stdint.h>

namespace filament {

class ResourceAllocator;

// This is where we store all the history of a frame
// when adding things here, please update:
//      FView::commitFrameHistory()
struct FrameHistoryEntry {
    std::weak_ptr<ResourceAllocator> cache;
    struct TemporalAA{
        FrameGraphTexture color;
        FrameGraphTexture::Descriptor desc;
        math::mat4 projection;     // world space to clip space
        math::float2 jitter{};
        uint32_t frameId = 0;   // used for halton sequence
    } taa;
    struct {
        FrameGraphTexture color;
        FrameGraphTexture::Descriptor desc;
        math::mat4 projection;
    } ssr;
    bool isValid() const noexcept {
        return !cache.expired();
    }
};

/*
 * This is a very simple FIFO that stores the history of previous frames.
 */
template<typename T, size_t SIZE>
class TFrameHistory {
public:
    // history size
    constexpr size_t size() const noexcept { return mContainer.size(); }

    // most recent frame history entry
    T const& front() const noexcept { return mContainer.front(); }
    T& front() noexcept { return mContainer.front(); }

    // oldest frame history entry
    T const& back() const noexcept { return mContainer.back(); }
    T& back() noexcept { return mContainer.back(); }

    // access by index
    T const& operator[](size_t n) const noexcept { return mContainer[n]; }
    T& operator[](size_t n) noexcept { return mContainer[n]; }

    // the current frame info, this is where we store the current frame information
    T& getCurrent() noexcept {
        return mCurrentEntry;
    }

    const T& getCurrent() const noexcept {
        return mCurrentEntry;
    }

    T& getPrevious() noexcept {
        return mContainer[0];
    }

    const T& getPrevious() const noexcept {
        return mContainer[0];
    }

    // This pushes the current frame info to the FIFO, effectively destroying
    // the oldest state (note: only the structure is destroyed, handles stored in it may
    // have to be destroyed prior to calling this).
    void commit() noexcept {
        auto& container = mContainer;
        if (SIZE > 1u) {
            std::move_backward(container.begin(), container.end() - 1, container.end());
        }
        container.front() = std::move(mCurrentEntry);
        mCurrentEntry = {};
    }

private:
    T mCurrentEntry{};
    std::array<T, SIZE> mContainer;
};

using FrameHistory = TFrameHistory<FrameHistoryEntry, 1u>;

} // namespace filament

#endif // TNT_FILAMENT_FRAMEHISTORY_H
