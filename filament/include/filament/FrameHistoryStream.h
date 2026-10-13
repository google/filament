/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FRAMEHISTORYSTREAM_H
#define TNT_FILAMENT_FRAMEHISTORYSTREAM_H

#include <filament/Renderer.h>

#include <utils/FixedCapacityVector.h>

#include <iterator>
#include <utility>

#include <stdint.h>

namespace filament {

/**
 * FrameHistoryStream is a public helper class to iterate over new frames rendered since the last query.
 * It takes care of keeping track of the last processed frame ID and identifying missing frames.
 *
 * Note: An instance of FrameHistoryStream must be kept alive (e.g., as a member variable)
 * rather than recreated transiently on the stack, in order to correctly track the last
 * processed frame ID across successive calls to getNewFrames().
 *
 * Example usage:
 * @code
 * // Store this instance as a member variable or long-lived object:
 * FrameHistoryStream stream(renderer);
 *
 * // ... then in the render or update loop:
 * for (auto result : stream.getNewFrames()) {
 *     if (result) {
 *         // Accessed via operator-> or operator*
 *         uint64_t presentTime = result->displayPresent;
 *     } else {
 *         // Missing frame
 *         uint32_t missingId = result.getMissingId();
 *     }
 * }
 * @endcode
 */
class FrameHistoryStream {
public:
    /**
     * Result represents either a successfully retrieved FrameInfo,
     * or a missing frame ID.
     */
    class Result {
    public:
        /**
         * Default constructor. Creates a missing frame result.
         */
        Result() : mIsMissing(true) {}

        /**
         * Constructs a valid frame result with the given FrameInfo.
         */
        explicit Result(Renderer::FrameInfo const& info) : mInfo(info), mIsMissing(false) {}

        /**
         * Constructs a missing frame result with the given frame ID.
         */
        explicit Result(uint32_t const frameId) : mIsMissing(true) {
            mInfo.frameId = frameId;
        }

        /**
         * Implicit or explicit conversion to bool. Returns true if the result
         * contains a valid FrameInfo, and false if it represents a missing frame ID.
         */
        explicit operator bool() const noexcept { return !mIsMissing; }

        /**
         * Pointer member access operator. Accesses the underlying FrameInfo structure.
         * Only valid if operator bool() returns true.
         */
        Renderer::FrameInfo const* operator->() const noexcept {
            return &mInfo;
        }

        /**
         * Dereference operator. Accesses the underlying FrameInfo structure.
         * Only valid if operator bool() returns true.
         */
        Renderer::FrameInfo const& operator*() const noexcept {
            return mInfo;
        }

        /**
         * Returns the frame ID associated with this result, regardless of whether
         * the frame is valid or missing.
         */
        uint32_t getFrameId() const noexcept {
            return mInfo.frameId;
        }

        /**
         * Returns the missing frame ID. Only valid if operator bool() returns false.
         */
        uint32_t getMissingId() const noexcept {
            return mInfo.frameId;
        }

    private:
        Renderer::FrameInfo mInfo{};
        bool mIsMissing;
    };

    /**
     * A helper class representing a range of new frames.
     * Obtained by calling FrameHistoryStream::getNewFrames().
     */
    class NewFramesRange {
    public:
        /**
         * Creates the range of the frames in history that are newer than *pLastProcessedFrameId,
         * up to (but not including) the oldest one whose timing information is still PENDING,
         * and sets *pLastProcessedFrameId to the ID of the newest frame in the range, if any.
         * A null pLastProcessedFrameId is treated as 0 and isn't updated.
         *
         * @param history frames ordered from newest to oldest, as returned by
         *                Renderer::getFrameInfoHistory().
         * @param pLastProcessedFrameId ID of the last frame processed.
         * @see FrameHistoryStream::getNewFrames()
         */
        NewFramesRange(utils::FixedCapacityVector<Renderer::FrameInfo> history,
                uint32_t* pLastProcessedFrameId) noexcept;

        /**
         * Iterator for NewFramesRange. Yields FrameHistoryStream::Result elements.
         */
        class Iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = Result;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type const*;
            using reference = value_type const&;

            Iterator() noexcept : mIsEnd(true) {}

            /**
             * Iterates from historyStart[historyIndex] (oldest) to historyStart[0] (newest).
             * Unless pLastProcessedFrameId is null or points to 0, it also yields the IDs missing
             * between *pLastProcessedFrameId and these frames. Unlike NewFramesRange, it doesn't
             * skip processed or PENDING frames and doesn't update *pLastProcessedFrameId.
             * Prefer NewFramesRange::begin().
             */
            Iterator(const Renderer::FrameInfo* historyStart, int const historyIndex,
                    uint32_t const* pLastProcessedFrameId) noexcept
                : mHistoryStart(historyStart),
                  mHistoryIndex(historyIndex),
                  mLastProcessedFrameId(pLastProcessedFrameId ? *pLastProcessedFrameId : 0),
                  mIsEnd(historyIndex < 0) {
                advance();
            }

            reference operator*() const noexcept { return mCurrentValue; }
            pointer operator->() const noexcept { return &mCurrentValue; }

            Iterator& operator++() noexcept {
                advance();
                return *this;
            }

            Iterator operator++(int) noexcept {
                Iterator const tmp = *this;
                advance();
                return tmp;
            }

            bool operator==(Iterator const& rhs) const noexcept {
                if (mIsEnd && rhs.mIsEnd) {
                    return true;
                }
                return mIsEnd == rhs.mIsEnd &&
                       mHistoryStart == rhs.mHistoryStart &&
                       mHistoryIndex == rhs.mHistoryIndex &&
                       mLastProcessedFrameId == rhs.mLastProcessedFrameId;
            }

            bool operator!=(Iterator const& rhs) const noexcept {
                return !operator==(rhs);
            }

        private:
            void advance() noexcept;

            const Renderer::FrameInfo* mHistoryStart = nullptr;
            int mHistoryIndex = -1;
            uint32_t mLastProcessedFrameId = 0;
            bool mIsEnd = false;
            Result mCurrentValue;
        };

        using iterator = Iterator;
        using const_iterator = Iterator;

        /**
         * Returns an iterator pointing to the beginning of the new frames.
         */
        iterator begin() const noexcept {
            return iterator(mHistory.data() + mEndIndex, mStartIndex - mEndIndex,
                    &mLastProcessedFrameId);
        }

        /**
         * Returns an iterator pointing to the end of the new frames.
         */
        iterator end() const noexcept {
            return iterator();
        }

    private:
        utils::FixedCapacityVector<Renderer::FrameInfo> mHistory;
        // the new frames are mHistory[mStartIndex] (oldest) to mHistory[mEndIndex] (newest)
        int mStartIndex = -1;
        int mEndIndex = 0;
        // the last frame processed before this range
        uint32_t mLastProcessedFrameId = 0;
    };

    /**
     * Constructs a FrameHistoryStream for the given Renderer.
     */
    explicit FrameHistoryStream(Renderer* renderer) noexcept
        : mRenderer(renderer), mLastProcessedFrameId(0) {}

    /**
     * Queries the renderer's frame history and returns a NewFramesRange representing
     * the new frames rendered since the last query.
     *
     * The range stops before the oldest new frame whose timing information is still PENDING.
     * That frame, the frames after it, and the missing frame IDs just before it are returned by
     * a later call.
     *
     * The new frames are consumed when getNewFrames() returns, not as the range is iterated:
     * frames that aren't iterated (e.g. because the loop exits early) won't be returned again.
     * The range owns a copy of the frames, so it can be iterated more than once, and it doesn't
     * depend on this FrameHistoryStream.
     */
    NewFramesRange getNewFrames() noexcept {
        auto history = mRenderer->getFrameInfoHistory(mRenderer->getMaxFrameHistorySize());
        return NewFramesRange(std::move(history), &mLastProcessedFrameId);
    }

private:
    Renderer* mRenderer;
    uint32_t mLastProcessedFrameId;
};

inline void FrameHistoryStream::NewFramesRange::Iterator::advance() noexcept {
    if (mIsEnd) {
        return;
    }

    if (mHistoryIndex >= 0) {
        auto const& fi = mHistoryStart[mHistoryIndex];

        if (mLastProcessedFrameId != 0 && mLastProcessedFrameId + 1 < fi.frameId) {
            mLastProcessedFrameId++;
            mCurrentValue = Result(mLastProcessedFrameId);
            return;
        }

        mCurrentValue = Result(fi);
        mLastProcessedFrameId = fi.frameId;
        mHistoryIndex--;
        return;
    }

    mIsEnd = true;
}

inline FrameHistoryStream::NewFramesRange::NewFramesRange(
        utils::FixedCapacityVector<Renderer::FrameInfo> history,
        uint32_t* const pLastProcessedFrameId) noexcept
        : mHistory(std::move(history)),
          mLastProcessedFrameId(pLastProcessedFrameId ? *pLastProcessedFrameId : 0) {
    // history is ordered from newest (index 0) to oldest (back), so we scan from the back to find
    // the sub-range [endIndex, startIndex] of new, ready frames.
    int startIndex = int(mHistory.size()) - 1;
    while (startIndex >= 0 && mHistory[startIndex].frameId <= mLastProcessedFrameId) {
        --startIndex;
    }

    int endIndex = startIndex + 1;
    while (endIndex > 0) {
        auto const& fi = mHistory[endIndex - 1];
        if (fi.displayPresent == Renderer::FrameInfo::PENDING ||
            fi.presentDeadline == Renderer::FrameInfo::PENDING ||
            fi.expectedPresentLatency == Renderer::FrameInfo::PENDING) {
            break;
        }
        --endIndex;
    }

    mStartIndex = startIndex;
    mEndIndex = endIndex;

    // Consume the frames now rather than as the range is iterated, so that iterating (and the
    // const begin()) has no side effects and the range doesn't point back to its owner.
    if (pLastProcessedFrameId && startIndex >= endIndex) {
        *pLastProcessedFrameId = mHistory[endIndex].frameId;
    }
}

} // namespace filament

#endif // TNT_FILAMENT_FRAMEHISTORYSTREAM_H
