/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <filament/Engine.h>
#include <filament/Material.h>

#include "filament_test_resources.h"

using namespace filament;

TEST(MaterialTransformName, QuerySamplerWithTransform) {
    Engine* engine = Engine::create(Engine::Backend::DEFAULT);

    Material* material = Material::Builder()
                                 .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_DATA,
                                         FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_SIZE)
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_STREQ(material->getParameterTransformName("sampler"), "transform");

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialTransformName, QueryMultipleSamplersWithTransforms) {
    Engine* engine = Engine::create(Engine::Backend::DEFAULT);

    Material* material = Material::Builder()
                                 .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_DATA,
                                         FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_SIZE)
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_STREQ(material->getParameterTransformName("sampler"), "transform");
    EXPECT_STREQ(material->getParameterTransformName("videoTexture"), "videoTransform");

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialTransformName, QuerySamplerWithoutTransform) {
    Engine* engine = Engine::create(Engine::Backend::DEFAULT);
    Material* material = Material::Builder()
                                 .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_DATA,
                                         FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_SIZE)
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_EQ(material->getParameterTransformName("sampler2"), nullptr);

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialTransformName, QueryMultipleSamplersWithoutTransforms) {
    Engine* engine = Engine::create(Engine::Backend::DEFAULT);

    Material* material = Material::Builder()
                                 .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_DATA,
                                         FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_SIZE)
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_EQ(material->getParameterTransformName("sampler1"), nullptr);
    EXPECT_EQ(material->getParameterTransformName("sampler3"), nullptr);

    engine->destroy(material);
    Engine::destroy(engine);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
