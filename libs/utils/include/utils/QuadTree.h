/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_UTILS_QUADTREE_H
#define TNT_UTILS_QUADTREE_H

#include <utils/debug.h>

#include <array>
#include <type_traits>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {

namespace QuadTreeUtils {

/**
 * 16-bits morton encoding
 * @param x 8-bits horizontal coordinate
 * @param y 8-bits vertical coordinate
 * @return  morton encoding of (x,y)
 */
static inline constexpr uint16_t morton(uint8_t x, uint8_t y) noexcept {
    uint32_t r = x | (uint32_t(y) << 16);
    r = (r | (r << 4u)) & 0x0f0f0f0fu;
    r = (r | (r << 2u)) & 0x33333333u;
    r = (r | (r << 1u)) & 0x55555555u;
    return uint16_t(r | (r >> 15u));
}

/**
 * size of a quad-tree of height `height`
 * @param height height of the Quad-tree
 * @return the number of elements in the tree
 */
static inline constexpr size_t size(size_t height) noexcept {
    return QuadTreeUtils::morton(uint8_t((1u << height) - 1u), 0u);
}

/**
 * Index in the QuadTreeArray of a Quad-tree node referenced by its height and code
 * @param l     height of the node
 * @param code  morton code of the node
 * @return      index in the QuadTreeArray of this node
 */
static inline constexpr size_t index(size_t l, size_t code) noexcept {
    return size(l) + code;
}

/**
 * Index in the QuadTreeArray of the parent of the specified node
 * @param l     height of the node
 * @param code  morton code of the node
 * @return      index in the QuadTreeArray of this node's parent
 */
static inline constexpr size_t parent(size_t l, size_t code) noexcept {
    assert_invariant(l > 0);
    return index(l - 1u, code >> 2u);
}

// integrated unit-tests!
static_assert(size(0) == 0);
static_assert(size(1) == 1);
static_assert(size(2) == 5);
static_assert(size(3) == 21);
static_assert(size(4) == 85);
static_assert(size(5) == 341);
static_assert(size(6) == 1365);
static_assert(size(7) == 5461);

} // namespace QuadTreeUtils

/**
 * A Quad-tree encoded as an array.
 * @tparam T        Type of the quad-tree nodes
 * @tparam HEIGHT   Height of the quad-tree
 */
template<typename T, size_t HEIGHT>
class QuadTreeArray : public std::array<T, QuadTreeUtils::size(HEIGHT)> {
    static_assert(HEIGHT <= 7, "QuadTreeArray height must be <= 7 (16-bits morton)");

    // Simple fixed capacity stack
    template<typename TYPE, size_t CAPACITY,
            typename = typename std::enable_if<std::is_trivial<TYPE>::value>::type>
    class stack {
        TYPE mElements[CAPACITY];
        size_t mSize = 0;
    public:
        bool empty() const noexcept { return mSize == 0; }
        void push(TYPE const& v) noexcept {
            assert_invariant(mSize < CAPACITY);
            mElements[mSize++] = v;
        }
        void pop() noexcept {
            assert_invariant(mSize > 0);
            --mSize;
        }
        const TYPE& back() const noexcept {
            return mElements[mSize - 1];
        }
    };

public:
    // code_t needs to be able to encode the # of entries for the largest level supported
    // the tree. With 7 levels, the largest one as 4096 entries, 0 to 4095 so 12 bits suffice.
    using code_t = uint16_t;

    struct NodeId {
        int8_t l      :  4;     // height of the node or -1 if invalid
        code_t   code : 12;     // morton code of the node
        static_assert(12 >= (HEIGHT - 1) * 2);
    };

    enum class TraversalResult {
        EXIT,               // end traversal
        RECURSE,            // proceed with the children
        SKIP_CHILDREN       // skip children
    };

    static inline constexpr size_t height() noexcept {
        return HEIGHT;
    }

    /**
     * non-recursive depth-first traversal
     *
     * @tparam Process  closure to process each visited node
     * @param l         height of the node to start with
     * @param code      code of the node to start with
     * @param h         maximum height to visit, must be < height()
     * @param process   closure to process each visited node
     */
    template<typename Process,
            typename = typename std::enable_if<
                    std::is_invocable_r_v<TraversalResult, Process, NodeId>>::type>
    static void traverse(int8_t l, code_t code, size_t maxHeight, Process&& process) noexcept {
        assert_invariant(maxHeight < height());
        const int8_t h = int8_t(maxHeight);
        stack<NodeId, 4 * height()> stack;
        stack.push({ l, code });
        while (!stack.empty()) {
            NodeId curr = stack.back();
            stack.pop();
            TraversalResult r = process(curr);
            if (r == TraversalResult::EXIT) {
                break;
            }
            if (r == TraversalResult::RECURSE && curr.l < h) {
                for (size_t c = 0; c < 4; c++) {
                    stack.push({
                        int8_t(curr.l + 1u),
                        code_t((curr.code << 2u) | (3u - c))
                    });
                }
            }
        }
    }

    template<typename Node,
            typename = typename std::enable_if<
                    std::is_invocable_r_v<TraversalResult, Node, NodeId>>::type>
    static void traverse(int8_t l, code_t code, Node&& process) noexcept {
        traverse(l, code, height() - 1, std::forward<Node>(process));
    }
};

} // namespace utils

#endif //TNT_UTILS_QUADTREE_H
