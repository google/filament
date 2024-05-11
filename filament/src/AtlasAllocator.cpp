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

#include "AtlasAllocator.h"

#include <utils/debug.h>

namespace filament {

using namespace utils;

static inline constexpr std::pair<uint8_t, uint8_t> unmorton(uint16_t m) noexcept {
    uint32_t r = (m | (uint32_t(m) << 15u)) & 0x55555555u;
    r = (r | (r >> 1u)) & 0x33333333u;
    r = (r | (r >> 2u)) & 0x0f0f0f0fu;
    r =  r | (r >> 4u);
    return { uint8_t(r), uint8_t(r >> 16u) };
}

AtlasAllocator::AtlasAllocator(size_t maxTextureSize) noexcept {
    // round to power-of-two immediately inferior or equal to the size specified.
    mMaxTextureSizePot = (sizeof(maxTextureSize) * 8 - 1u) - utils::clz(maxTextureSize);
}

AtlasAllocator::Allocation AtlasAllocator::allocate(size_t textureSize) noexcept {
    Allocation result{};
    const size_t powerOfTwo = (sizeof(textureSize) * 8 - 1u) - utils::clz(textureSize);

    // asked for a texture size too large
    if (UTILS_UNLIKELY(powerOfTwo > mMaxTextureSizePot)) {
        return result;
    }

    // asked for a texture size too small
    if (UTILS_UNLIKELY(mMaxTextureSizePot - powerOfTwo >= QUAD_TREE_DEPTH)) {
        return result;
    }

    const size_t layer = (mMaxTextureSizePot - powerOfTwo);
    const NodeId loc = allocateInLayer(LAYERS_DEPTH + layer);
    if (loc.l >= 0) {
        assert_invariant(loc.l - LAYERS_DEPTH == int8_t(layer));
        const size_t dimension = 1u << powerOfTwo;
        // find the location of in the texture from the morton code (quadtree position)
        const auto [x, y] = unmorton(loc.code);
        // scale to our maximum allocation size
        const uint32_t mask = (1u << layer) - 1u;
        result.viewport.left   = int32_t(x & mask) << powerOfTwo;
        result.viewport.bottom = int32_t(y & mask) << powerOfTwo;
        result.viewport.width  = dimension;
        result.viewport.height = dimension;
        result.layer = loc.code >> (2 * layer);
    }
    return result;
}

void AtlasAllocator::clear(size_t maxTextureSize) noexcept {
    std::fill(mQuadTree.begin(), mQuadTree.end(), Node{});
    mMaxTextureSizePot = (sizeof(maxTextureSize) * 8 - 1u) - utils::clz(maxTextureSize);
}

AtlasAllocator::NodeId AtlasAllocator::allocateInLayer(size_t maxHeight) noexcept {
    using namespace QuadTreeUtils;

    NodeId candidate{ -1, 0 };
    if (UTILS_UNLIKELY(maxHeight > QuadTree::height())) {
        return candidate;
    }

    const int8_t n = int8_t(maxHeight);

    QuadTree::traverse(0, 0, n,
            [this, n, &candidate](NodeId const& curr) -> QuadTree::TraversalResult {

                // we should never reach a level higher than n
                assert_invariant(curr.l <= n);

                // get the node being processed
                const size_t i = index(curr.l, curr.code);
                Node const& node = mQuadTree[i];

                // if the node is allocated, the whole tree below it is unavailable
                if (node.isAllocated()) {
                    // if a node is allocated it can't have children
                    assert_invariant(!node.hasChildren());
                    return QuadTree::TraversalResult::SKIP_CHILDREN;
                }

                // we're looking for the `deepest` unallocated node that has no children,
                // up to the depth we're trying to allocate
                if (curr.l > candidate.l && !node.hasChildren()) {
                    candidate = curr;
                    // Special case where we found a fitting node, just exit.
                    if (curr.l == n) {
                        return QuadTree::TraversalResult::EXIT;
                    }
                }

                // We only want to find a fitting node that already has siblings, in order
                // to accomplish a "best fit" allocation. So if a parent (of what we're looking for)
                // has no children, we skip the whole tree below it.
                if (!node.hasChildren()) {
                    return QuadTree::TraversalResult::SKIP_CHILDREN;
                }

                // continue going down the tree
                return QuadTree::TraversalResult::RECURSE;
            });

    if (candidate.l >= 0) {
        const size_t i = index(candidate.l, candidate.code);
        Node& allocation = mQuadTree[i];
        assert_invariant(!allocation.isAllocated());
        assert_invariant(!allocation.hasChildren());

        if (candidate.l == n) {
            allocation.allocated = true;
            if (UTILS_LIKELY(n > 0)) {
                // the root doesn't have a parent
                const size_t p = parent(candidate.l, candidate.code);
                Node& parent = mQuadTree[p];
                assert_invariant(!parent.isAllocated());
                assert_invariant(parent.hasChildren());
                assert_invariant(!parent.hasAllChildren());
                parent.children++;

#ifndef NDEBUG
                // check that all the parents are not allocated and have at least 1 child.
                NodeId ppp = candidate;
                while (ppp.l > 0) {
                    const size_t pi = QuadTreeUtils::parent(ppp.l, ppp.code);
                    ppp = NodeId{ int8_t(ppp.l - 1), uint8_t(ppp.code >> 2) };
                    Node const& node = mQuadTree[pi];
                    assert_invariant(!node.isAllocated());
                    assert_invariant(node.hasChildren());
                }
#endif
            }
        } else if (candidate.l < int8_t(QuadTree::height())) {
            // we need to create the hierarchy down to the level we need
            NodeId found{ -1, 0 };
            QuadTree::traverse(candidate.l, candidate.code,
                    [this, n, &found](NodeId const& curr) -> QuadTree::TraversalResult {
                        size_t const i = index(curr.l, curr.code);
                        Node& node = mQuadTree[i];
                        if (curr.l == n) {
                            found = curr;
                            assert_invariant(!node.hasChildren());
                            node.allocated = true;
                            return QuadTree::TraversalResult::EXIT;
                        }
                        assert_invariant(!node.hasAllChildren());
                        node.children++;
                        return QuadTree::TraversalResult::RECURSE;
                    });

            assert_invariant(found.l != -1);
            candidate = found;
        }
    }
    return candidate;
}

} // namespace filament
