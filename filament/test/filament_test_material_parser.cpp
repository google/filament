/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <fstream>
#include <iostream>

#include <gtest/gtest.h>

#include "MaterialParser.h"

#include "filament_test_resources.h"

using namespace filament;

// This test checks that a material compiled with an older version of matc can still be parsed.
// If this test is failing, it probably means that MATERIAL_VERSION needs to be incremented.
// After doing so, to fix the test, run filament/test/update_test_material.sh.
// This will re-compile the test material with the current version of matc.
// To verify, rebuild and re-run test_material_parser (this test suite).
TEST(MaterialParser, Parse) {
    MaterialParser parser(backend::Backend::OPENGL,
            FILAMENT_TEST_RESOURCES_TEST_MATERIAL_DATA, FILAMENT_TEST_RESOURCES_TEST_MATERIAL_SIZE);
    bool materialOk = parser.parse();

    EXPECT_TRUE(materialOk) <<
            "Material filament/test/test_material.filamat could not be parsed by MaterialParser." << std::endl <<
            "Does MATERIAL_VERSION need to be updated?" << std::endl <<
            "See instructions in filament_test_material_parser.cpp" << std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
