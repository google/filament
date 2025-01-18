/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_UTILS_BINARYTREEARRAY_H
#define TNT_UTILS_BINARYTREEARRAY_H

#include <utils/compiler.h>

#include <type_traits>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {

class BinaryTreeArray {

    // Simple fixed capacity stack
    template<typename TYPE, size_t CAPACITY,
            typename = std::enable_if_t<std::is_trivial_v<TYPE>>>
    class stack {
        TYPE mElements[CAPACITY];
        size_t mSize = 0;
    public:
        bool empty() const noexcept { return mSize == 0; }
        void push(TYPE const& v) noexcept {
            assert(mSize < CAPACITY);
            mElements[mSize++] = v;
        }
        void pop() noexcept {
            assert(mSize > 0);
            --mSize;
        }
        const TYPE& back() const noexcept {
            return mElements[mSize - 1];
        }
    };

public:
    static size_t count(size_t height) noexcept { return  (1u << height) - 1; }
    static size_t left(size_t i, size_t /*height*/) noexcept { return i + 1; }
    static size_t right(size_t i, size_t height) noexcept {
        return i + (size_t(1) << (height - 1));
    }

    // this builds the depth-first binary tree array top down (post-order)
    template<typename Leaf, typename Node>
    static void traverse(size_t height, Leaf leaf, Node node) noexcept {

        struct TNode {
            uint32_t index;
            uint32_t col;
            uint32_t height;
            uint32_t next;

            bool isLeaf() const noexcept { return height == 1; }
            size_t left() const noexcept { return BinaryTreeArray::left(index, height); }
            size_t right() const noexcept { return BinaryTreeArray::right(index, height); }
        };

        stack<TNode, 16> stack;
        stack.push(TNode{ 0, 0, (uint32_t)height, (uint32_t)count(height) });

        uint32_t prevLeft = 0;
        uint32_t prevRight = 0;
        uint32_t prevIndex = 0;
        while (!stack.empty()) {
            TNode const* const UTILS_RESTRICT curr = &stack.back();
            const bool isLeaf = curr->isLeaf();
            const uint32_t index = curr->index;
            const uint32_t l = (uint32_t)curr->left();
            const uint32_t r = (uint32_t)curr->right();

            if (prevLeft == index || prevRight == index) {
                if (!isLeaf) {
                    // the 'next' node of our left node's right descendants is our right child
                    stack.push({ l, 2 * curr->col, curr->height - 1, r });
                }
            } else if (l == prevIndex) {
                if (!isLeaf) {
                    // the 'next' node of our right child is our own 'next' sibling
                    stack.push({ r, 2 * curr->col + 1, curr->height - 1, curr->next });
                }
            } else {
                if (!isLeaf) {
                    node(index, l, r, curr->next);
                } else {
                    leaf(index, curr->col, curr->next);
                }
                stack.pop();
            }

            prevLeft  = l;
            prevRight = r;
            prevIndex = index;
        }
    }
};

} // namespace utils

#endif //TNT_UTILS_BINARYTREEARRAY_H
