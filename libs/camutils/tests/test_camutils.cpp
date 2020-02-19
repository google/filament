/*
 * Copyright 2020 The Android Open Source Project
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

#include <camutils/Bookmark.h>
#include <camutils/Manipulator.h>

#include <gtest/gtest.h>

using namespace filament::math;

namespace camutils = filament::camutils;

using CamManipulator = camutils::Manipulator<float>;

class CamUtilsTest : public testing::Test {};

#define EXPECT_VEC_EQ(a, x, y, z) expectVecEq(a, {x, y, z}, __LINE__)

static void expectVecEq(float3 a, float3 b, int line) {
    EXPECT_FLOAT_EQ(a.x, b.x);
    EXPECT_FLOAT_EQ(a.y, b.y);
    EXPECT_FLOAT_EQ(a.z, b.z);
}

TEST_F(CamUtilsTest, Orbit) {

    float3 eye, targetPosition, up;

    CamManipulator* orbit = CamManipulator::Builder()
        .viewport(256, 256)
        .targetPosition(0, 0, 0)
        .upVector(0, 1, 0)
        .zoomSpeed(0.01)
        .orbitHomePosition(0, 0, 4)
        .orbitSpeed(1, 1)
        .build(camutils::Mode::ORBIT);

    orbit->getLookAt(&eye, &targetPosition, &up);
    EXPECT_VEC_EQ(eye, 0, 0, 4);
    EXPECT_VEC_EQ(targetPosition, 0, 0, 0);
    EXPECT_VEC_EQ(up, 0, 1, 0);

    orbit->grabBegin(100, 100, false);
    orbit->grabUpdate(200, 100);
    orbit->grabEnd();

    orbit->getLookAt(&eye, &targetPosition, &up);
    EXPECT_VEC_EQ(eye, 2.0254626, 0, 3.4492755);
    EXPECT_VEC_EQ(targetPosition, 1.519097, 0, 2.5869565);
    EXPECT_VEC_EQ(up, 0, 1, 0);

    delete orbit;
}

TEST_F(CamUtilsTest, Map) {

    float3 eye, targetPosition, up;

    CamManipulator* map = CamManipulator::Builder()
        .viewport(256, 256)
        .targetPosition(0, 0, 0)
        .zoomSpeed(0.01)
        .orbitHomePosition(0, 0, 4)
        .build(camutils::Mode::MAP);

    delete map;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
