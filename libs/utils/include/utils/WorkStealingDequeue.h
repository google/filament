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

#ifndef TNT_UTILS_WORKSTEALINGDEQUEUE_H
#define TNT_UTILS_WORKSTEALINGDEQUEUE_H

#include <atomic>
#include <cstddef>

namespace utils {

template <typename TYPE, size_t COUNT>
class WorkStealingDequeue {
    static_assert(!(COUNT & (COUNT - 1)), "COUNT must be a power of two");
    static constexpr size_t MASK = COUNT - 1;
    std::atomic<int32_t> mTop = { 0 };      // written/read in pop()/steal()
    std::atomic<int32_t> mBottom = { 0 };   // written only in pop(), read in push(), steal()
    TYPE mItems[COUNT];

    TYPE getItemAt(int32_t index) noexcept { return mItems[index & MASK]; }
    void setItemAt(int32_t index, TYPE item) noexcept { mItems[index & MASK] = item; }

public:
    using value_type = TYPE;

    inline void push(TYPE item) noexcept;
    inline TYPE pop() noexcept;
    inline TYPE steal() noexcept;

    size_t getSize() const noexcept { return COUNT; }

    bool isEmpty() const noexcept {
        uint32_t bottom = (uint32_t)mBottom.load(std::memory_order_relaxed);
        uint32_t top = (uint32_t)mTop.load(std::memory_order_seq_cst);
        return top >= bottom;
    }

    // for debugging only...
    int32_t getCount() const noexcept {
        int32_t bottom = mBottom.load(std::memory_order_relaxed);
        int32_t top = mTop.load(std::memory_order_relaxed);
        return bottom - top;
    }
};


template <typename TYPE, size_t COUNT>
void WorkStealingDequeue<TYPE, COUNT>::push(TYPE item) noexcept {

    // mBottom is only written in pop() which cannot be concurrent with push(),
    // however, it is read in steal() so we need basic atomicity.
    int32_t bottom = mBottom.load(std::memory_order_relaxed);
    setItemAt(bottom, item);

    // memory accesses cannot be reordered after mBottom write, which notifies the
    // availability of an extra item.
    mBottom.store(bottom + 1, std::memory_order_release);
}

template <typename TYPE, size_t COUNT>
TYPE WorkStealingDequeue<TYPE, COUNT>::pop() noexcept {
    // mBottom is only written in push(), which cannot be concurrent with pop(),
    // however, it is read in steal(), so we need basic atomicity.
    //   i.e.: bottom = mBottom--;
    int32_t bottom = mBottom.fetch_sub(1, std::memory_order_relaxed) - 1;

    // we need a full memory barrier here; mBottom must be written and visible to
    // other threads before we read mTop.
    int32_t top = mTop.load(std::memory_order_seq_cst);

    if (top < bottom) {
        // Queue isn't empty and it's not the last item, just return it.
        return getItemAt(bottom);
    }

    TYPE item{};
    if (top == bottom) {
        // We took the last item in the queue
        item = getItemAt(bottom);

        // Items can be added only in push() which isn't concurrent to us, however we could
        // be racing with a steal() -- pretend to steal from ourselves to resolve this
        // potential conflict.
        if (mTop.compare_exchange_strong(top, top + 1,
                std::memory_order_seq_cst,
                std::memory_order_relaxed)) {
            // success: mTop was equal to top, mTop now equals top+1
            // We successfully poped an item, adjust top to make the queue canonically empty.
            top++;
        } else {
            // failure: mTop was not equal to top, which means the item was stolen under our feet.
            // top now equals to mTop. Simply discard the item we just poped.
            // The queue is now empty.
            item = TYPE();
        }
    }

    // no concurrent writes to mBottom possible
    mBottom.store(top, std::memory_order_relaxed);
    return item;
}

template <typename TYPE, size_t COUNT>
TYPE WorkStealingDequeue<TYPE, COUNT>::steal() noexcept {
    do {
        // mTop must be read before mBottom
        int32_t top = mTop.load(std::memory_order_seq_cst);

        // mBottom is written concurrently to the read below in pop() or push(), so
        // we need basic atomicity. Also makes sure that writes made in push()
        // (prior to mBottom update) are visible.
        int32_t bottom = mBottom.load(std::memory_order_acquire);

        if (top >= bottom) {
            // queue is empty
            return TYPE();
        }

        // The queue isn't empty
        TYPE item(getItemAt(top));
        if (mTop.compare_exchange_strong(top, top + 1,
                std::memory_order_seq_cst,
                std::memory_order_relaxed)) {
            // success: we stole a job, just return it.
            return item;
        }
        // failure: the item we just tried to steal was pop()'ed under our feet,
        // simply discard it; nothing to do.
    } while (true);
}


} // namespace utils

#endif // TNT_UTILS_WORKSTEALINGDEQUEUE_H
