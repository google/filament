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

#include "filament_test_resources.h"
#include "MaterialParser.h"

#include <gtest/gtest.h>

#include <array>
#include <fstream>
#include <iostream>

using namespace filament;

// This test checks that a material compiled with an older version of matc can still be parsed.
// If this test is failing, it probably means that MATERIAL_VERSION needs to be incremented.
// After doing so, to fix the test, run filament/test/update_test_material.sh.
// This will re-compile the test material with the current version of matc.
// To verify, rebuild and re-run test_material_parser (this test suite).
TEST(MaterialParser, Parse) {
    MaterialParser parser({ backend::ShaderLanguage::ESSL3 },
            FILAMENT_TEST_RESOURCES_TEST_MATERIAL_DATA, FILAMENT_TEST_RESOURCES_TEST_MATERIAL_SIZE);
    MaterialParser::ParseResult materialOk = parser.parse();

    EXPECT_TRUE(materialOk == MaterialParser::ParseResult::SUCCESS) <<
            "Material filament/test/test_material.filamat could not be parsed by MaterialParser." << std::endl <<
            "Does MATERIAL_VERSION need to be updated?" << std::endl <<
            "See instructions in filament_test_material_parser.cpp" << std::endl;
}

TEST(MaterialParser, RejectOOBDescriptorBindings) {
    // 1 descriptor: name = "u_sampler\0", type = 1, binding = MAX_DESCRIPTOR_COUNT (64) -> OOB!
    std::vector<uint8_t> buffer = {
        1,                                                  // descriptorCount
        'u', '_', 's', 'a', 'm', 'p', 'l', 'e', 'r', '\0',  // name
        1,                                                  // type
        static_cast<uint8_t>(backend::MAX_DESCRIPTOR_COUNT) // binding (64 -> invalid)
    };

    filaflat::Unflattener unflattener(buffer.data(), buffer.data() + buffer.size());
    MaterialParser::DescriptorBindingsContainer container;
    EXPECT_FALSE(ChunkDescriptorBindingsInfo::unflatten(unflattener, &container));
}

TEST(MaterialParser, AcceptValidDescriptorBindings) {
    // 1 descriptor: name = "u_sampler\0", type = 1, binding = MAX_DESCRIPTOR_COUNT - 1 (63) ->
    // Valid!
    std::vector<uint8_t> buffer = {
        1,                                                      // descriptorCount
        'u', '_', 's', 'a', 'm', 'p', 'l', 'e', 'r', '\0',      // name
        1,                                                      // type
        static_cast<uint8_t>(backend::MAX_DESCRIPTOR_COUNT - 1) // binding (63 -> valid)
    };

    filaflat::Unflattener unflattener(buffer.data(), buffer.data() + buffer.size());
    MaterialParser::DescriptorBindingsContainer container;
    EXPECT_TRUE(ChunkDescriptorBindingsInfo::unflatten(unflattener, &container));
}

TEST(MaterialParser, RejectOOBDescriptorSetLayout) {
    // 1 descriptor: type = 0, stageFlags = 1, binding = 64 (OOB), flags = 0, count = 0 (uint16_t)
    std::vector<uint8_t> buffer = {
        1,                                                   // descriptorCount
        0,                                                   // type
        1,                                                   // stageFlags
        static_cast<uint8_t>(backend::MAX_DESCRIPTOR_COUNT), // binding (64 -> invalid)
        0,                                                   // flags
        0, 0                                                 // count (uint16_t)
    };

    filaflat::Unflattener unflattener(buffer.data(), buffer.data() + buffer.size());
    MaterialParser::DescriptorSetLayoutContainer container;
    EXPECT_FALSE(ChunkDescriptorSetLayoutInfo::unflatten(unflattener, &container));
}

TEST(MaterialParser, AcceptValidDescriptorSetLayout) {
    // 1 descriptor: type = 0, stageFlags = 1, binding = 63 (valid), flags = 0, count = 0 (uint16_t)
    std::vector<uint8_t> buffer = {
        1,                                                       // descriptorCount
        0,                                                       // type
        1,                                                       // stageFlags
        static_cast<uint8_t>(backend::MAX_DESCRIPTOR_COUNT - 1), // binding (63 -> valid)
        0,                                                       // flags
        0, 0                                                     // count (uint16_t)
    };

    filaflat::Unflattener unflattener(buffer.data(), buffer.data() + buffer.size());
    MaterialParser::DescriptorSetLayoutContainer container;
    EXPECT_TRUE(ChunkDescriptorSetLayoutInfo::unflatten(unflattener, &container));
}

namespace {
constexpr uint8_t UBO = static_cast<uint8_t>(backend::DescriptorType::UNIFORM_BUFFER);
constexpr uint8_t SAMPLER = static_cast<uint8_t>(backend::DescriptorType::SAMPLER_2D_FLOAT);
constexpr uint8_t DYNAMIC = static_cast<uint8_t>(backend::DescriptorFlags::DYNAMIC_OFFSET);
} // namespace

TEST(MaterialParser, RejectDuplicateBindingDescriptorSetLayout) {
    std::vector<uint8_t> buffer = {
        2,                          // descriptorCount
        SAMPLER, 1, 1, 0, 0, 0,     // type, stageFlags, binding, flags, count (uint16_t)
        SAMPLER, 1, 1, 0, 0, 0,     // same binding again
    };

    filaflat::Unflattener unflattener(buffer.data(), buffer.data() + buffer.size());
    MaterialParser::DescriptorSetLayoutContainer container;
    EXPECT_FALSE(ChunkDescriptorSetLayoutInfo::unflatten(unflattener, &container));
}

TEST(MaterialParser, RejectMultipleDynamicOffsetsDescriptorSetLayout) {
    std::vector<uint8_t> buffer = {
        2,                          // descriptorCount
        UBO, 1, 0, DYNAMIC, 0, 0,   // type, stageFlags, binding, flags, count (uint16_t)
        UBO, 1, 1, DYNAMIC, 0, 0,
    };

    filaflat::Unflattener unflattener(buffer.data(), buffer.data() + buffer.size());
    MaterialParser::DescriptorSetLayoutContainer container;
    EXPECT_FALSE(ChunkDescriptorSetLayoutInfo::unflatten(unflattener, &container));
}

TEST(MaterialParser, AcceptSingleDynamicOffsetDescriptorSetLayout) {
    // Same shape as MaterialDescriptorSetLayoutChunk::flatten() in filamat.
    std::vector<uint8_t> buffer = {
        2,                          // descriptorCount
        UBO, 1, 0, DYNAMIC, 0, 0,   // type, stageFlags, binding, flags, count (uint16_t)
        SAMPLER, 1, 1, 0, 0, 0,
    };

    filaflat::Unflattener unflattener(buffer.data(), buffer.data() + buffer.size());
    MaterialParser::DescriptorSetLayoutContainer container;
    EXPECT_TRUE(ChunkDescriptorSetLayoutInfo::unflatten(unflattener, &container));
    EXPECT_EQ(container.descriptors.size(), 2u);
}

TEST(MaterialParser, TestMaterialDescriptorSetLayoutIsValid) {
    MaterialParser parser({ backend::ShaderLanguage::ESSL3 },
            FILAMENT_TEST_RESOURCES_TEST_MATERIAL_DATA, FILAMENT_TEST_RESOURCES_TEST_MATERIAL_SIZE);
    ASSERT_EQ(parser.parse(), MaterialParser::ParseResult::SUCCESS);

    MaterialParser::DescriptorSetLayoutContainer container;
    EXPECT_TRUE(parser.getDescriptorSetLayout(&container));
}

namespace {
// A package holding only a MAT_BLFN chunk, laid out as ChunkContainer::parseChunk() reads it.
std::vector<uint8_t> makeBlendFunctionPackage(uint32_t const blendFunctions) {
    std::vector<uint8_t> package;
    auto const append = [&package](uint64_t const v, size_t const size) {
        for (size_t i = 0; i < size; i++) {
            package.push_back(uint8_t(v >> (8 * i)));
        }
    };
    append(filamat::MaterialBlendFunction, sizeof(uint64_t));
    append(sizeof(uint32_t), sizeof(uint32_t));
    append(blendFunctions, sizeof(uint32_t));
    return package;
}
} // namespace

TEST(MaterialParser, RejectOutOfRangeCustomBlendFunction) {
    // 11 is the first value past SRC_ALPHA_SATURATE, 15 is the largest that fits 4 bits.
    for (uint32_t const blendFunctions : { 0x0000000Bu, 0x0F0F0F0Fu }) {
        std::vector<uint8_t> const package = makeBlendFunctionPackage(blendFunctions);
        MaterialParser parser({ backend::ShaderLanguage::ESSL3 }, package.data(), package.size());
        // No shader chunks, but the chunk container is still parsed.
        EXPECT_EQ(parser.parse(), MaterialParser::ParseResult::ERROR_MISSING_BACKEND);

        std::array<backend::BlendFunction, 4> functions{};
        EXPECT_FALSE(parser.getCustomBlendFunction(&functions)) << std::hex << blendFunctions;
    }
}

TEST(MaterialParser, AcceptValidCustomBlendFunction) {
    std::vector<uint8_t> const package = makeBlendFunctionPackage(0x0A070601);
    MaterialParser parser({ backend::ShaderLanguage::ESSL3 }, package.data(), package.size());
    EXPECT_EQ(parser.parse(), MaterialParser::ParseResult::ERROR_MISSING_BACKEND);

    std::array<backend::BlendFunction, 4> functions{};
    ASSERT_TRUE(parser.getCustomBlendFunction(&functions));
    EXPECT_EQ(functions[0], backend::BlendFunction::SRC_ALPHA_SATURATE);
    EXPECT_EQ(functions[1], backend::BlendFunction::ONE_MINUS_SRC_ALPHA);
    EXPECT_EQ(functions[2], backend::BlendFunction::SRC_ALPHA);
    EXPECT_EQ(functions[3], backend::BlendFunction::ONE);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
