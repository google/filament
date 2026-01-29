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

#include <gtest/gtest.h>

#include "AtlasAllocator.h"

using namespace filament;

TEST(AtlasAllocator, AllocateFirstLevel) {

    AtlasAllocator allocator(1024);

    auto a = allocator.allocateInLayer(0);
    EXPECT_TRUE(a.l == 0 && a.code == 0);

    auto b = allocator.allocateInLayer(1);
    EXPECT_TRUE(b.l < 0);

    auto c = allocator.allocateInLayer(2);
    EXPECT_TRUE(c.l < 0);

    auto d = allocator.allocateInLayer(5);
    EXPECT_TRUE(d.l < 0);
}

TEST(AtlasAllocator, AllocateSecondLevel) {

    AtlasAllocator allocator(1024);

    auto d0 = allocator.allocateInLayer(1);
    EXPECT_TRUE(d0.l == 1 && d0.code == 0);

    auto d1 = allocator.allocateInLayer(1);
    EXPECT_TRUE(d1.l == 1 && d1.code == 1);

    auto d2 = allocator.allocateInLayer(1);
    EXPECT_TRUE(d2.l == 1 && d2.code == 2);

    auto d3 = allocator.allocateInLayer(1);
    EXPECT_TRUE(d3.l == 1 && d3.code == 3);
}

TEST(AtlasAllocator, AllocateMixed0) {
    AtlasAllocator allocator(1024);

    auto e0 = allocator.allocateInLayer(1);
    EXPECT_TRUE(e0.l == 1 && e0.code == 0);

    auto e1 = allocator.allocateInLayer(1);
    EXPECT_TRUE(e1.l == 1 && e1.code == 1);

    auto e2 = allocator.allocateInLayer(1);
    EXPECT_TRUE(e2.l == 1 && e2.code == 2);

    auto e3 = allocator.allocateInLayer(2);
    EXPECT_TRUE(e3.l == 2 && e3.code == 12);

    auto e4 = allocator.allocateInLayer(1);
    EXPECT_TRUE(e4.l < 0);
}

TEST(AtlasAllocator, AllocateMixed1) {
    AtlasAllocator allocator(1024);

    auto e0 = allocator.allocateInLayer(1);
    EXPECT_TRUE(e0.l == 1 && e0.code == 0);

    auto e1 = allocator.allocateInLayer(1);
    EXPECT_TRUE(e1.l == 1 && e1.code == 1);

    auto e2 = allocator.allocateInLayer(2);
    EXPECT_TRUE(e2.l == 2 && e2.code == 8);

    auto e3 = allocator.allocateInLayer(1);
    EXPECT_TRUE(e3.l == 1 && e3.code == 3);
}

TEST(AtlasAllocator, AllocateMixed2) {
    AtlasAllocator allocator(1024);

    auto e0 = allocator.allocateInLayer(1);
    EXPECT_TRUE(e0.l == 1 && e0.code == 0);

    auto e1 = allocator.allocateInLayer(1);
    EXPECT_TRUE(e1.l == 1 && e1.code == 1);

    auto c0 = allocator.allocateInLayer(2);
    EXPECT_TRUE(c0.l == 2 && c0.code == 8);
    auto c1 = allocator.allocateInLayer(2);
    EXPECT_TRUE(c1.l == 2 && c1.code == 9);
    auto c2 = allocator.allocateInLayer(2);
    EXPECT_TRUE(c2.l == 2 && c2.code == 10);
    auto c3 = allocator.allocateInLayer(2);
    EXPECT_TRUE(c3.l == 2 && c3.code == 11);

    auto c4 = allocator.allocateInLayer(2);
    EXPECT_TRUE(c4.l == 2 && c4.code == 12);
    auto c5 = allocator.allocateInLayer(2);
    EXPECT_TRUE(c5.l == 2 && c5.code == 13);
    auto c6 = allocator.allocateInLayer(2);
    EXPECT_TRUE(c6.l == 2 && c6.code == 14);
    auto c7 = allocator.allocateInLayer(2);
    EXPECT_TRUE(c7.l == 2 && c7.code == 15);
}

TEST(AtlasAllocator, AllocateFourCascades_Test) {
    Viewport vp(0,0,1024,1024);
    AtlasAllocator allocator(1024);
    auto e0 = allocator.allocate(1024);
    EXPECT_EQ(e0.viewport, vp);
    EXPECT_EQ(e0.layer, 0);

    auto e1 = allocator.allocate(1024);
    EXPECT_EQ(e1.viewport, vp);
    EXPECT_EQ(e1.layer, 1);

    auto e2 = allocator.allocate(1024);
    EXPECT_EQ(e2.viewport, vp);
    EXPECT_EQ(e2.layer, 2);

    auto e3 = allocator.allocate(1024);
    EXPECT_EQ(e3.viewport, vp);
    EXPECT_EQ(e3.layer, 3);

    auto e4 = allocator.allocate(128);
    EXPECT_EQ(e4.layer, 4);

    auto e5 = allocator.allocate(128);
    EXPECT_EQ(e5.layer, 4);
}

TEST(AtlasAllocator, AllocateBySize) {
    AtlasAllocator allocator(256);

    Viewport vp(0,0,256,256);
    auto vp0 = allocator.allocate(256);
    EXPECT_EQ(vp0.viewport, vp);
    EXPECT_EQ(vp0.layer, 0);

    auto vp1 = allocator.allocate(128);
    EXPECT_EQ(vp1.layer, 1);
}

TEST(AtlasAllocator, AllocateBySizeOneOfEach) {
    AtlasAllocator allocator(256);

    Viewport r0(0,0,128,128);
    Viewport r1(128,0,64,64);
    Viewport r2(192,0,32,32);

    auto vp0 = allocator.allocate(128);
    auto vp1 = allocator.allocate(64);
    auto vp2 = allocator.allocate(32);
    auto vp3 = allocator.allocate(16);

    EXPECT_EQ(vp0.layer, 0);
    EXPECT_EQ(vp1.layer, 0);
    EXPECT_EQ(vp2.layer, 0);

    EXPECT_EQ(vp0.viewport, r0);
    EXPECT_EQ(vp1.viewport, r1);
    EXPECT_EQ(vp2.viewport, r2);
    EXPECT_TRUE(vp3.viewport.empty());

}

TEST(AtlasAllocator, AllocateBySizeFullLayers) {
    AtlasAllocator allocator(512);

    Viewport r(0,0,512,512);

    auto vp0 = allocator.allocate(512);
    auto vp1 = allocator.allocate(512);
    auto vp2 = allocator.allocate(512);
    auto vp3 = allocator.allocate(512);

    EXPECT_EQ(vp0.layer, 0);
    EXPECT_EQ(vp1.layer, 1);
    EXPECT_EQ(vp2.layer, 2);
    EXPECT_EQ(vp3.layer, 3);

    EXPECT_EQ(vp0.viewport, r);
    EXPECT_EQ(vp1.viewport, r);
    EXPECT_EQ(vp2.viewport, r);
    EXPECT_EQ(vp3.viewport, r);
}

TEST(AtlasAllocator, FreeSimple) {
    AtlasAllocator allocator(256);

    // Allocate a 256x256 block (takes the whole layer 0)
    auto vp0 = allocator.allocate(256);
    EXPECT_EQ(vp0.layer, 0);
    EXPECT_FALSE(vp0.viewport.empty());

    // Free it
    allocator.free(vp0);

    // Allocate it again, should get the same spot
    auto vp1 = allocator.allocate(256);
    EXPECT_EQ(vp1.layer, 0);
    EXPECT_EQ(vp1.viewport, vp0.viewport);
}

TEST(AtlasAllocator, FreeAndCoalesce) {
    AtlasAllocator allocator(256);

    // Allocate 4 128x128 blocks. They should fill layer 0.
    auto vp0 = allocator.allocate(128);
    auto vp1 = allocator.allocate(128);
    auto vp2 = allocator.allocate(128);
    auto vp3 = allocator.allocate(128);

    EXPECT_EQ(vp0.layer, 0);
    EXPECT_EQ(vp1.layer, 0);
    EXPECT_EQ(vp2.layer, 0);
    EXPECT_EQ(vp3.layer, 0);

    // Free them all
    allocator.free(vp0);
    allocator.free(vp1);
    allocator.free(vp2);
    allocator.free(vp3);

    // Now allocate a 256x256 block. It should fit in layer 0 because the 4 128s were coalesced.
    auto vpBig = allocator.allocate(256);
    EXPECT_EQ(vpBig.layer, 0);
    EXPECT_FALSE(vpBig.viewport.empty());
}

TEST(AtlasAllocator, FreePartialCoalesce) {
    AtlasAllocator allocator(256);

    // Allocate 4 128x128 blocks.
    auto vp0 = allocator.allocate(128);
    auto vp1 = allocator.allocate(128);
    auto vp2 = allocator.allocate(128);
    auto vp3 = allocator.allocate(128);

    // Free 3 of them
    allocator.free(vp0);
    allocator.free(vp1);
    allocator.free(vp2);

    // Try to allocate 256. Should fail (go to next layer) because vp3 is still there.
    auto vpBig = allocator.allocate(256);
    EXPECT_NE(vpBig.layer, 0);

    // Free the last one
    allocator.free(vp3);

    // Now allocate 256. Should succeed in layer 0.
    // Note: vpBig took layer 1, so layer 0 is now free.
    auto vpBig2 = allocator.allocate(256);
    EXPECT_EQ(vpBig2.layer, 0);
}

TEST(AtlasAllocator, FreeDeepHierarchy) {
    AtlasAllocator allocator(512);

    // Allocate a small block deep in the tree
    // 512 -> 256 -> 128 -> 64. 32 is too small for QUAD_TREE_DEPTH=4
    auto vpSmall = allocator.allocate(64);
    EXPECT_EQ(vpSmall.layer, 0);

    // Free it
    allocator.free(vpSmall);

    // Allocate a huge block. Should fit in layer 0 if the small block was properly coalesced up to the root.
    auto vpHuge = allocator.allocate(512);
    EXPECT_EQ(vpHuge.layer, 0);
}

TEST(AtlasAllocator, FreeInvalid) {
    AtlasAllocator allocator(256);
    AtlasAllocator::Allocation invalid;
    EXPECT_FALSE(invalid.isValid());
    // Should not crash
    allocator.free(invalid);
}

TEST(AtlasAllocator, Checkerboard) {
    AtlasAllocator allocator(256);

    // Allocate 4 128x128 blocks (indices 0, 1, 2, 3 in Morton order)
    auto vp0 = allocator.allocate(128);
    auto vp1 = allocator.allocate(128);
    auto vp2 = allocator.allocate(128);
    auto vp3 = allocator.allocate(128);

    // Free diagonals (0 and 3)
    allocator.free(vp0);
    allocator.free(vp3);

    // Try to allocate 256. Should fail (layer 0 is fragmented).
    auto vpBig = allocator.allocate(256);
    EXPECT_NE(vpBig.layer, 0);

    // Free remaining
    allocator.free(vp1);
    allocator.free(vp2);

    // Now allocate 256. Should succeed in layer 0.
    auto vpBig2 = allocator.allocate(256);
    EXPECT_EQ(vpBig2.layer, 0);
}

TEST(AtlasAllocator, LayerIndependence) {
    AtlasAllocator allocator(256);

    // Fill Layer 0 with a big block
    auto l0 = allocator.allocate(256);
    EXPECT_EQ(l0.layer, 0);

    // Fill Layer 1 with a big block
    auto l1 = allocator.allocate(256);
    EXPECT_EQ(l1.layer, 1);

    // Free Layer 0
    allocator.free(l0);

    // Allocate big block again. Should go to Layer 0.
    auto l0_new = allocator.allocate(256);
    EXPECT_EQ(l0_new.layer, 0);

    // Layer 1 should still be occupied.
    // If we try to allocate another big block, it should go to Layer 2.
    auto l2 = allocator.allocate(256);
    EXPECT_EQ(l2.layer, 2);
}

TEST(AtlasAllocator, ReuseHole) {
    AtlasAllocator allocator(256);

    // Allocate 4 128x128 blocks.
    auto vp0 = allocator.allocate(128);
    auto vp1 = allocator.allocate(128);
    auto vp2 = allocator.allocate(128);
    auto vp3 = allocator.allocate(128);

    // Free the second one (vp1)
    allocator.free(vp1);

    // Allocate a new 128 block. It should fit in the hole we just made in Layer 0.
    auto vpNew = allocator.allocate(128);
    EXPECT_EQ(vpNew.layer, 0);
    EXPECT_EQ(vpNew.viewport, vp1.viewport);
}

TEST(AtlasAllocator, NonPowerOfTwo) {
    AtlasAllocator allocator(512);

    // Request 100. Should round down to 64.
    auto vp = allocator.allocate(100);
    EXPECT_EQ(vp.viewport.width, 64);
    EXPECT_EQ(vp.viewport.height, 64);
    EXPECT_TRUE(vp.isValid());

    // Request 500. Should round down to 256.
    auto vp2 = allocator.allocate(500);
    EXPECT_EQ(vp2.viewport.width, 256);
    EXPECT_EQ(vp2.viewport.height, 256);
    EXPECT_TRUE(vp2.isValid());
}
