/*
 * Copyright 2019 The Android Open Source Project
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

#include <rays/PathTracer.h>

#include <imageio/ImageEncoder.h>

#include <gtest/gtest.h>

#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/JobSystem.h>

#include <fstream>
#include <vector>

using namespace image;
using namespace filament::rays;

#ifdef FILAMENT_HAS_EMBREE

utils::Mutex sLock;

TEST(EmbreeTest, Smoke) { // NOLINT
    utils::JobSystem js;
    js.adopt();

    std::vector<float> vertices = {
        -2, -2, .1,
        +2, -2, .1,
        +2, +2, .1,
        -2, +2, .1,
        -1, -1, 0,
        +1, -1, 0,
        +1, +1, 0,
        -1, +1, 0,
    };

    std::vector<uint32_t> triangles = {
        2, 1, 0,
        0, 3, 2,
        6, 5, 4,
        4, 7, 6,
    };

    SimpleMesh mesh {
        .numVertices = vertices.size() / 3,
        .numIndices = triangles.size(),
        .positions = vertices.data(),
        .positionsStride = 12,
        .indices = triangles.data()
    };

    LinearImage image(512, 512, 1);

    utils::Condition signal;

    PathTracer renderer = PathTracer::Builder()
        .outputPlane(AMBIENT_OCCLUSION, image)
        .meshes(&mesh, 1)
        .filmCamera({
            .aspectRatio = 1.0f,
            .eyePosition = {0, 0, -5},
            .targetPosition = {0, 0, 0},
            .upVector = {0, 1, 0},
            .vfovDegrees = 45
        })
        .doneCallback([](void* userData) {
            utils::Condition* signal = (utils::Condition*) userData;
            signal->notify_all();
        }, &signal)
        .build();

    renderer.render();

    std::unique_lock<utils::Mutex> lock(sLock);
    signal.wait(lock);

    std::ofstream out("embree_ao.png", std::ios::binary | std::ios::trunc);
    ImageEncoder::encode(out, ImageEncoder::Format::PNG_LINEAR, image, "", "embree_ao.png");

    js.emancipate();
}

#endif

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
