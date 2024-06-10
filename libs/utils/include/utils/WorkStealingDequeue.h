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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {

/*
 * A templated, lockless, fixed-size work-stealing dequeue
 *
 *
 *     top                          bottom
 *      v                             v
 *      |----|----|----|----|----|----|
 *    steal()                      push(), pop()
 *  any thread                     main thread
 *
 *
 */
template <typename TYPE, size_t COUNT>
class WorkStealingDequeue {
    static_assert(!(COUNT & (COUNT - 1)), "COUNT must be a power of two");
    static constexpr size_t MASK = COUNT - 1;

    // mTop and mBottom must be signed integers. We use 64-bits atomics so we don't have
    // to worry about wrapping around.
    using index_t = int64_t;

    std::atomic<index_t> mTop    = { 0 };   // written/read in pop()/steal()
    std::atomic<index_t> mBottom = { 0 };   // written only in pop(), read in push(), steal()

    TYPE mItems[COUNT];

    // NOTE: it's not safe to return a reference because getItemAt() can be called
    // concurrently and the caller could std::move() the item unsafely.
    TYPE getItemAt(index_t index) noexcept { return mItems[index & MASK]; }

    void setItemAt(index_t index, TYPE item) noexcept { mItems[index & MASK] = item; }

public:
    using value_type = TYPE;

    inline void push(TYPE item) noexcept;
    inline TYPE pop() noexcept;
    inline TYPE steal() noexcept;

    size_t getSize() const noexcept { return COUNT; }

    // for debugging only...
    size_t getCount() const noexcept {
        index_t bottom = mBottom.load(std::memory_order_relaxed);
        index_t top = mTop.load(std::memory_order_relaxed);
        return bottom - top;
    }
};

/*
 * Adds an item at the BOTTOM of the queue.
 *
 * Must be called from the main thread.
 */
template <typename TYPE, size_t COUNT>
void WorkStealingDequeue<TYPE, COUNT>::push(TYPE item) noexcept {
    // std::memory_order_relaxed is sufficient because this load doesn't acquire anything from
    // another thread. mBottom is only written in pop() which cannot be concurrent with push()
    index_t bottom = mBottom.load(std::memory_order_relaxed);
    setItemAt(bottom, item);

    // std::memory_order_release is used because we release the item we just pushed to other
    // threads which are calling steal().
    mBottom.store(bottom + 1, std::memory_order_release);
}

/*
 * Removes an item from the BOTTOM of the queue.
 *
 * Must be called from the main thread.
 */
template <typename TYPE, size_t COUNT>
TYPE WorkStealingDequeue<TYPE, COUNT>::pop() noexcept {
    // std::memory_order_seq_cst is needed to guarantee ordering in steal()
    // Note however that this is not a typical acquire/release operation:
    //  - not acquire because mBottom is only written in push() which is not concurrent
    //  - not release because we're not publishing anything to steal() here
    //
    // QUESTION: does this prevent mTop load below to be reordered before the "store" part of
    //           fetch_sub()? Hopefully it does. If not we'd need a full memory barrier.
    //
    index_t bottom = mBottom.fetch_sub(1, std::memory_order_seq_cst) - 1;

    // bottom could be -1 if we tried to pop() from an empty queue. This will be corrected below.
    assert( bottom >= -1 );

    // std::memory_order_seq_cst is needed to guarantee ordering in steal()
    // Note however that this is not a typical acquire operation
    //  (i.e. other thread's writes of mTop don't publish data)
    index_t top = mTop.load(std::memory_order_seq_cst);

    if (top < bottom) {
        // Queue isn't empty and it's not the last item, just return it, this is the common case.
        return getItemAt(bottom);
    }

    TYPE item{};
    if (top == bottom) {
        // we just took the last item
        item = getItemAt(bottom);

        // Because we know we took the last item, we could be racing with steal() -- the last
        // item being both at the top and bottom of the queue.
        // We resolve this potential race by also stealing that item from ourselves.
        if (mTop.compare_exchange_strong(top, top + 1,
                std::memory_order_seq_cst,
                std::memory_order_relaxed)) {
            // success: we stole our last item from ourself, meaning that a concurrent steal()
            //          would have failed.
            // mTop now equals top + 1, we adjust top to make the queue empty.
            top++;
        } else {
            // failure: mTop was not equal to top, which means the item was stolen under our feet.
            // top now equals to mTop. Simply discard the item we just popped.
            // The queue is now empty.
            item = TYPE();
        }
    } else {
        // We could be here if the item was stolen just before we read mTop, we'll adjust
        // mBottom below.
        assert(top - bottom == 1);
    }

    // std::memory_order_relaxed used because we're not publishing any data.
    // no concurrent writes to mBottom possible, it's always safe to write mBottom.
    mBottom.store(top, std::memory_order_relaxed);
    return item;
}

/*
 * Steals an item from the TOP of another thread's queue.
 *
 * This can be called concurrently with steal(), push() or pop()
 *
 * steal() never fails, either there is an item and it atomically takes it, or there isn't and
 * it returns an empty item.
 */
template <typename TYPE, size_t COUNT>
TYPE WorkStealingDequeue<TYPE, COUNT>::steal() noexcept {
    while (true) {
        /*
         * Note: A Key component of this algorithm is that mTop is read before mBottom here
         * (and observed as such in other threads)
         */

        // std::memory_order_seq_cst is needed to guarantee ordering in pop()
        // Note however that this is not a typical acquire operation
        //  (i.e. other thread's writes of mTop don't publish data)
        index_t top = mTop.load(std::memory_order_seq_cst);

        // std::memory_order_acquire is needed because we're acquiring items published in push().
        // std::memory_order_seq_cst is needed to guarantee ordering in pop()
        index_t bottom = mBottom.load(std::memory_order_seq_cst);

        if (top >= bottom) {
            // queue is empty
            return TYPE();
        }

        // The queue isn't empty
        TYPE item(getItemAt(top));
        if (mTop.compare_exchange_strong(top, top + 1,
                std::memory_order_seq_cst,
                std::memory_order_relaxed)) {
            // success: we stole an item, just return it.
            return item;
        }
        // failure: the item we just tried to steal was pop()'ed under our feet,
        // simply discard it; nothing to do -- it's okay to try again.
    }
}


} // namespace utils

#endif // TNT_UTILS_WORKSTEALINGDEQUEUE_H
