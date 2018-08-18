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

#include <stddef.h>
#include <stdint.h>

namespace utils {

class BinaryTreeArray {
public:
    static size_t count(size_t height) noexcept { return  (1u << height) - 1; }
    static size_t left(size_t i, size_t height) noexcept { return i + 1; }
    static size_t right(size_t i, size_t height) noexcept { return i + (1u << (height - 1)); }

    template<typename Leaf, typename Node>
    static void traverse(size_t height, Leaf leaf, Node node) noexcept {
        traverse(0, 0, 0, height, count(height), leaf, node);
    }

private:
    // this builds the depth-first binary tree array top down, so we need to evaluate the
    // light ranges only once.
    template<typename Leaf, typename Node>
    static void traverse(size_t index, size_t parent,
                         size_t col, size_t height,
                         size_t next,
                         Leaf& leaf, Node& node) noexcept {
        if (height > 1) {
            size_t l = left(index, height);
            size_t r = right(index, height);
            // the 'next' node of our left node's right descendants is our right child
            traverse(l, index, 2 * col,     height - 1, r, leaf, node);
            // the 'next' node of our right child is our own 'next' sibling
            traverse(r, index, 2 * col + 1, height - 1, next, leaf, node);
            node(index, parent, l, r, next);
        } else {
            leaf(index, parent, col, next);
        }
    }
};

} // namespace utils

#endif //TNT_UTILS_BINARYTREEARRAY_H
