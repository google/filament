/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <glslkomi/Komi.h>
#include <gtest/gtest.h>

using namespace glslkomi;

TEST(Compress, Compress) {
  std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = texture(materialParams_sampler, vec3(0.0, 0.0));
        }
    )");
  // auto result = std::get<std::vector<uint8_t>>(compress(shaderCode));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
