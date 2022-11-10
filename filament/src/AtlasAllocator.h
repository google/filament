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
        // whether this node is allocated. Implies no children.
        constexpr bool isAllocated() const noexcept { return allocated; }
        // whether this node has children. Implies it's not allocated.
        constexpr bool hasChildren() const noexcept { return children != 0; }
        // whether this node has all four children. Implies hasChildren().
        constexpr bool hasAllChildren() const noexcept { return children == 4; }
        bool allocated      : 1;    // true / false
        uint8_t children    : 3;    // 0, 1, 2, 3, 4
    };

    static constexpr size_t QUAD_TREE_DEPTH = 4u;
    using QuadTree = utils::QuadTreeArray<Node, QUAD_TREE_DEPTH>;
    using NodeId = QuadTree::NodeId;

public:
    /*
     * Create allocator and specify the maximum texture size. Must be a power of two.
     * Allocations size allowed are the four power-of-two smaller or equal to this size.
     */
    explicit AtlasAllocator(size_t maxTextureSize = 1024) noexcept;

    /*
     * Allocates a square of size `textureSize`. Must be one of the power-of-two allowed
     * (see above).
     * Returns the location of the allocation within the maxTextureSize^2 square.
     */
    Viewport allocate(size_t textureSize) noexcept;

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
