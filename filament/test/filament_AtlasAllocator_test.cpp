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

