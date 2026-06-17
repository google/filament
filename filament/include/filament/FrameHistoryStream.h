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
        NewFramesRange(utils::FixedCapacityVector<Renderer::FrameInfo> history, uint32_t* pLastProcessedFrameId) noexcept
            : mHistory(std::move(history)), mPLastProcessedFrameId(pLastProcessedFrameId) {}

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

            Iterator(const Renderer::FrameInfo* historyStart, int const historyIndex, uint32_t* pLastProcessedFrameId) noexcept
                : mHistoryStart(historyStart),
                  mHistoryIndex(historyIndex),
                  mPLastProcessedFrameId(pLastProcessedFrameId),
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
            void advance() noexcept {
                if (mIsEnd) {
                    return;
                }

                while (mHistoryIndex >= 0) {
                    auto const& fi = mHistoryStart[mHistoryIndex];

                    if (fi.frameId <= mLastProcessedFrameId) {
                        mHistoryIndex--;
                        continue;
                    }

                    if (mLastProcessedFrameId != 0 && mLastProcessedFrameId + 1 < fi.frameId) {
                        mLastProcessedFrameId++;
                        if (mPLastProcessedFrameId) {
                            *mPLastProcessedFrameId = mLastProcessedFrameId;
                        }
                        mCurrentValue = Result(mLastProcessedFrameId);
                        return;
                    }

                    if (fi.displayPresent == Renderer::FrameInfo::PENDING ||
                        fi.presentDeadline == Renderer::FrameInfo::PENDING ||
                        fi.expectedPresentLatency == Renderer::FrameInfo::PENDING) {
                        mIsEnd = true;
                        return;
                    }

                    mCurrentValue = Result(fi);
                    mLastProcessedFrameId = fi.frameId;
                    if (mPLastProcessedFrameId) {
                        *mPLastProcessedFrameId = mLastProcessedFrameId;
                    }
                    mHistoryIndex--;
                    return;
                }

                mIsEnd = true;
            }

            const Renderer::FrameInfo* mHistoryStart = nullptr;
            int mHistoryIndex = -1;
            uint32_t* mPLastProcessedFrameId = nullptr;
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
            return iterator(mHistory.data(), int(mHistory.size()) - 1, mPLastProcessedFrameId);
        }

        /**
         * Returns an iterator pointing to the end of the new frames.
         */
        iterator end() const noexcept {
            return iterator();
        }

    private:
        utils::FixedCapacityVector<Renderer::FrameInfo> mHistory;
        uint32_t* mPLastProcessedFrameId;
    };

    /**
     * Constructs a FrameHistoryStream for the given Renderer.
     */
    explicit FrameHistoryStream(Renderer* renderer) noexcept
        : mRenderer(renderer), mLastProcessedFrameId(0) {}

    /**
     * Queries the renderer's frame history and returns a NewFramesRange representing
     * the new frames rendered since the last query.
     */
    NewFramesRange getNewFrames() noexcept {
        auto history = mRenderer->getFrameInfoHistory(mRenderer->getMaxFrameHistorySize());
        return NewFramesRange(std::move(history), &mLastProcessedFrameId);
    }

private:
    Renderer* mRenderer;
    uint32_t mLastProcessedFrameId;
};

} // namespace filament

#endif // TNT_FILAMENT_FRAMEHISTORYSTREAM_H
