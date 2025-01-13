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

/*
 * A 2D allocator. Allocations must be square and have a power-of-two size.
 * AtlasAllocator hard codes a depth of 4, that is only 4 allocation sizes are permitted.
 * This doesn't actually allocate memory, just manages space within an abstract 2D image.
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
    /*
     * Create allocator and specify the maximum texture size. Must be a power of two.
     * Allocations size allowed are the four power-of-two smaller or equal to this size.
     */
    explicit AtlasAllocator(size_t maxTextureSize) noexcept;

    /*
     * Allocates a square of size `textureSize`. Must be one of the power-of-two allowed
     * (see above).
     * Returns the location of the allocation within the maxTextureSize^2 square.
     */
    struct Allocation {
        int32_t layer = -1;
        Viewport viewport;
    };
    Allocation allocate(size_t textureSize) noexcept;

    /*
     * Frees all allocations and reset the maximum texture size.
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
