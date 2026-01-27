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

#ifndef TNT_FILAMENT_ATLASALLOCATOR_H
#define TNT_FILAMENT_ATLASALLOCATOR_H

#include <utils/QuadTree.h>

#include <filament/Viewport.h>

#include <private/filament/EngineEnums.h>

#include <stdint.h>
#include <stddef.h>

class AtlasAllocator_AllocateFirstLevel_Test;
class AtlasAllocator_AllocateSecondLevel_Test;
class AtlasAllocator_AllocateMixed0_Test;
class AtlasAllocator_AllocateMixed1_Test;
class AtlasAllocator_AllocateMixed2_Test;

namespace filament {

/**
 * A 2D allocator based on a QuadTree (Buddy Allocator).
 *
 * Allocations must be square and have a power-of-two size.
 * AtlasAllocator hard codes a depth of 4, that is only 4 allocation sizes are permitted.
 * This doesn't actually allocate memory, just manages space within an abstract 2D image.
 *
 * The allocator supports both allocation and deallocation.
 * - Allocation uses a "best-fit" strategy, preferring to split nodes that already have children
 *   to preserve large contiguous blocks.
 * - Deallocation (free) automatically coalesces empty sibling nodes back into larger parent blocks
 *   (standard Buddy Allocator behavior).
 */
class AtlasAllocator {

    /*
     * A quadtree is used to track the allocated regions. Each node of the quadtree stores a
     * `Node` data structure below.
     * The `Node` tracks if it is allocated as well as the number of children it has. It doesn't
     * track which children though.
     */
    struct Node {
        // Whether this node is allocated. Implies no children.
        constexpr bool isAllocated() const noexcept { return allocated; }
        // Whether this node has children. Implies it's not allocated.
        constexpr bool hasChildren() const noexcept { return children != 0; }
        // Whether this node has all four children. Implies hasChildren().
        constexpr bool hasAllChildren() const noexcept { return children == 4; }
        bool allocated      : 1;    // true / false
        uint8_t children    : 3;    // 0, 1, 2, 3, 4
    };

    // this determines the number of layers we can use (3 layers == 64 quadtree entries)
    static constexpr size_t LAYERS_DEPTH = 3u;

    // this determines how many "sub-sizes" we can have from the base size.
    // e.g. with a max texture size of 1024, we can allocate 1024, 512, 256 and 128 textures.
    static constexpr size_t QUAD_TREE_DEPTH = 4u;

    // LAYERS_DEPTH limits the number of layers
    static_assert(CONFIG_MAX_SHADOW_LAYERS <= 1u << (LAYERS_DEPTH * 2u));

    // QuadTreeArray is limited to a maximum depth of 7
    using QuadTree = utils::QuadTreeArray<Node, LAYERS_DEPTH + QUAD_TREE_DEPTH>;
    using NodeId = QuadTree::NodeId;

public:
    /**
     * Create allocator and specify the maximum texture size.
     *
     * @param maxTextureSize The maximum size of the texture atlas. Must be a power of two.
     *                       Allocations size allowed are the four power-of-two smaller or equal
     *                       to this size.
     */
    explicit AtlasAllocator(size_t maxTextureSize) noexcept;

    /**
     * Represents a successful allocation within the atlas.
     */
    struct Allocation {
        /** The viewport of the allocation within the atlas layer. */
        Viewport viewport;
        /** The layer index of the allocation (for texture arrays). 0xffff if invalid. */
        uint16_t layer = 0xffff;
        /** Internal Morton code used for deallocation. */
        uint16_t code = 0;
        /** Internal QuadTree level used for deallocation. */
        int8_t level = 0;
        /** Returns true if the allocation is valid. */
        bool isValid() const noexcept { return layer != 0xffff; }
    };

    /**
     * Allocates a square region of the specified size.
     *
     * @param textureSize The size of the square to allocate. Must be a power of two.
     *                    If the size is not a power of two, it will be rounded down to the
     *                    nearest power of two.
     * @return An Allocation struct containing the viewport and layer of the allocated region.
     *         If the allocation fails (e.g. no space left or size too small/large),
     *         an invalid Allocation is returned (isValid() returns false).
     */
    Allocation allocate(size_t textureSize) noexcept;

    /**
     * Frees an allocation returned by allocate().
     *
     * This method marks the region as free and attempts to coalesce empty sibling nodes
     * into larger parent blocks, making them available for future large allocations.
     *
     * @param allocation The allocation to free. If the allocation is invalid, this method does nothing.
     */
    void free(Allocation const& allocation) noexcept;

    /**
     * Frees all allocations and resets the allocator with a new maximum texture size.
     *
     * @param maxTextureSize The new maximum size of the texture atlas. Defaults to 1024.
     *                       Must be a power of two.
     */
    void clear(size_t maxTextureSize = 1024) noexcept;

private:
    friend AtlasAllocator_AllocateFirstLevel_Test;
    friend AtlasAllocator_AllocateSecondLevel_Test;
    friend AtlasAllocator_AllocateMixed0_Test;
    friend AtlasAllocator_AllocateMixed1_Test;
    friend AtlasAllocator_AllocateMixed2_Test;

    QuadTree::NodeId allocateInLayer(size_t n) noexcept;

    // quad-tree array to store the allocated list
    QuadTree mQuadTree{};
    uint8_t mMaxTextureSizePot = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_ATLASALLOCATOR_H
