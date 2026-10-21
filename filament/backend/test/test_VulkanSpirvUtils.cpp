/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "vulkan/utils/Spirv.h"

#include <backend/Program.h>

#include <utils/FixedCapacityVector.h>

#include <gtest/gtest.h>
#include <spirv/unified1/spirv.hpp>

#include <cstdint>
#include <cstring>
#include <vector>

namespace filament::backend::fvkutils {

namespace {

constexpr uint32_t makeOp(uint32_t wordCount, spv::Op op) {
    return (wordCount << 16) | static_cast<uint32_t>(op);
}

std::vector<uint32_t> makeSpirvHeader() {
    return {
        spv::MagicNumber,
        0x00010000u, // version 1.0
        0u,          // generator
        16u,         // bound
        0u,          // schema
    };
}

Program::ShaderBlob toShaderBlob(std::vector<uint32_t> const& words) {
    Program::ShaderBlob blob(words.size() * sizeof(uint32_t));
    if (!words.empty()) {
        std::memcpy(blob.data(), words.data(), blob.size());
    }
    return blob;
}

uint32_t floatBits(float v) {
    uint32_t bits = 0;
    std::memcpy(&bits, &v, sizeof(bits));
    return bits;
}

} // anonymous namespace

TEST(VulkanSpirvUtilsTest, TransformsBoolIntAndFloatSpecConstants) {
    std::vector<uint32_t> words = makeSpirvHeader();
    // Non-SpecId decoration (3 words): OpDecorate %10 RelaxedPrecision
    words.push_back(makeOp(3, spv::Op::OpDecorate));
    words.push_back(10u);
    words.push_back(static_cast<uint32_t>(spv::Decoration::DecorationRelaxedPrecision));

    // OpDecorate %1 SpecId 0
    words.push_back(makeOp(4, spv::Op::OpDecorate));
    words.push_back(1u);
    words.push_back(static_cast<uint32_t>(spv::Decoration::DecorationSpecId));
    words.push_back(0u);

    // OpDecorate %2 SpecId 1
    words.push_back(makeOp(4, spv::Op::OpDecorate));
    words.push_back(2u);
    words.push_back(static_cast<uint32_t>(spv::Decoration::DecorationSpecId));
    words.push_back(1u);

    // OpDecorate %3 SpecId 2
    words.push_back(makeOp(4, spv::Op::OpDecorate));
    words.push_back(3u);
    words.push_back(static_cast<uint32_t>(spv::Decoration::DecorationSpecId));
    words.push_back(2u);

    // %1 = OpSpecConstantTrue %bool (type id 11)
    words.push_back(makeOp(3, spv::Op::OpSpecConstantTrue));
    words.push_back(11u);
    words.push_back(1u);

    // %2 = OpSpecConstant %int 10 (type id 12)
    words.push_back(makeOp(4, spv::Op::OpSpecConstant));
    words.push_back(12u);
    words.push_back(2u);
    words.push_back(10u);

    // %3 = OpSpecConstant %float 1.0 (type id 13)
    words.push_back(makeOp(4, spv::Op::OpSpecConstant));
    words.push_back(13u);
    words.push_back(3u);
    words.push_back(floatBits(1.0f));

    auto specConstants = utils::FixedCapacityVector<Program::SpecializationConstant>::with_capacity(3);
    specConstants.push_back(false);
    specConstants.push_back(int32_t(-42));
    specConstants.push_back(3.5f);

    std::vector<uint32_t> output;
    workaroundSpecConstant(toShaderBlob(words), specConstants, output);

    // Expected: 5 header + 3 RelaxedPrecision + 3 bool + 4 int + 4 float = 19 words.
    ASSERT_EQ(output.size(), 19u);

    // RelaxedPrecision decoration preserved at offset 5.
    EXPECT_EQ(output[5], makeOp(3, spv::Op::OpDecorate));
    EXPECT_EQ(output[6], 10u);
    EXPECT_EQ(output[7], static_cast<uint32_t>(spv::Decoration::DecorationRelaxedPrecision));

    // %1 transformed to OpConstantFalse at offset 8.
    EXPECT_EQ(output[8], makeOp(3, spv::Op::OpConstantFalse));
    EXPECT_EQ(output[9], 11u);
    EXPECT_EQ(output[10], 1u);

    // %2 transformed to OpConstant -42 at offset 11.
    EXPECT_EQ(output[11], makeOp(4, spv::Op::OpConstant));
    EXPECT_EQ(output[12], 12u);
    EXPECT_EQ(output[13], 2u);
    EXPECT_EQ(output[14], static_cast<uint32_t>(int32_t(-42)));

    // %3 transformed to OpConstant 3.5f at offset 15.
    EXPECT_EQ(output[15], makeOp(4, spv::Op::OpConstant));
    EXPECT_EQ(output[16], 13u);
    EXPECT_EQ(output[17], 3u);
    EXPECT_EQ(output[18], floatBits(3.5f));
}

TEST(VulkanSpirvUtilsTest, RejectsBlobSmallerThanHeader) {
    auto specConstants = utils::FixedCapacityVector<Program::SpecializationConstant>::with_capacity(1);
    specConstants.push_back(int32_t(1));

    std::vector<uint32_t> output = { 1u, 2u, 3u };
    Program::ShaderBlob const emptyBlob;
    workaroundSpecConstant(emptyBlob, specConstants, output);
    EXPECT_TRUE(output.empty());

    output = { 1u, 2u, 3u };
    std::vector<uint32_t> const shortWords = { spv::MagicNumber, 0x00010000u, 0u };
    workaroundSpecConstant(toShaderBlob(shortWords), specConstants, output);
    EXPECT_TRUE(output.empty());
}

TEST(VulkanSpirvUtilsTest, RejectsWordCountExceedingRemainingBuffer) {
    // Matches the PoC in issue #10525: 13-word buffer where OpSpecConstant declares wordCount = 200.
    std::vector<uint32_t> words = makeSpirvHeader();
    words.push_back(makeOp(4, spv::Op::OpDecorate));
    words.push_back(1u);
    words.push_back(static_cast<uint32_t>(spv::Decoration::DecorationSpecId));
    words.push_back(0u);

    words.push_back(makeOp(200, spv::Op::OpSpecConstant));
    words.push_back(2u);
    words.push_back(1u);
    words.push_back(42u);

    auto specConstants = utils::FixedCapacityVector<Program::SpecializationConstant>::with_capacity(1);
    specConstants.push_back(int32_t(42));

    std::vector<uint32_t> output;
    workaroundSpecConstant(toShaderBlob(words), specConstants, output);

    // Only the 5-word header should remain (OpDecorate SpecId was skipped and the overflowing
    // OpSpecConstant was rejected).
    EXPECT_EQ(output.size(), 5u);

    // Also test a non-spec-constant instruction with an overflowing wordCount (e.g. 65535).
    std::vector<uint32_t> defaultWords = makeSpirvHeader();
    defaultWords.push_back(makeOp(65535, spv::Op::OpCapability));
    defaultWords.push_back(1u);

    workaroundSpecConstant(toShaderBlob(defaultWords), specConstants, output);
    EXPECT_EQ(output.size(), 5u);
}

TEST(VulkanSpirvUtilsTest, StopsOnZeroWordCount) {
    std::vector<uint32_t> words = makeSpirvHeader();
    words.push_back(makeOp(0, spv::Op::OpNop));
    words.push_back(makeOp(2, spv::Op::OpCapability));
    words.push_back(1u);

    auto specConstants = utils::FixedCapacityVector<Program::SpecializationConstant>::with_capacity(1);
    specConstants.push_back(true);

    std::vector<uint32_t> output;
    workaroundSpecConstant(toShaderBlob(words), specConstants, output);
    EXPECT_EQ(output.size(), 5u);
}

TEST(VulkanSpirvUtilsTest, RejectsTruncatedOpDecorate) {
    auto specConstants = utils::FixedCapacityVector<Program::SpecializationConstant>::with_capacity(1);
    specConstants.push_back(int32_t(7));

    // OpDecorate with wordCount = 2 at end of buffer (missing Decoration operand at cursor + 2).
    {
        std::vector<uint32_t> words = makeSpirvHeader();
        words.push_back(makeOp(2, spv::Op::OpDecorate));
        words.push_back(1u);

        std::vector<uint32_t> output;
        workaroundSpecConstant(toShaderBlob(words), specConstants, output);
        EXPECT_EQ(output.size(), 7u);
    }

    // OpDecorate SpecId with wordCount = 3 at end of buffer (missing specId operand at cursor + 3).
    {
        std::vector<uint32_t> words = makeSpirvHeader();
        words.push_back(makeOp(3, spv::Op::OpDecorate));
        words.push_back(1u);
        words.push_back(static_cast<uint32_t>(spv::Decoration::DecorationSpecId));

        std::vector<uint32_t> output;
        workaroundSpecConstant(toShaderBlob(words), specConstants, output);
        EXPECT_EQ(output.size(), 8u);
    }
}

TEST(VulkanSpirvUtilsTest, RejectsTruncatedOrMismatchedOpSpecConstant) {
    auto specConstants = utils::FixedCapacityVector<Program::SpecializationConstant>::with_capacity(1);
    specConstants.push_back(int32_t(99));

    // OpSpecConstantTrue with wordCount = 2 at end of buffer (missing targetVar at cursor + 2).
    {
        std::vector<uint32_t> words = makeSpirvHeader();
        words.push_back(makeOp(2, spv::Op::OpSpecConstantTrue));
        words.push_back(2u);

        std::vector<uint32_t> output;
        workaroundSpecConstant(toShaderBlob(words), specConstants, output);
        ASSERT_EQ(output.size(), 7u);
        EXPECT_EQ(output[5], makeOp(2, spv::Op::OpSpecConstantTrue));
    }

    // OpSpecConstant with wordCount = 3 at end of buffer (missing value word at cursor + 3).
    {
        std::vector<uint32_t> words = makeSpirvHeader();
        words.push_back(makeOp(4, spv::Op::OpDecorate));
        words.push_back(1u);
        words.push_back(static_cast<uint32_t>(spv::Decoration::DecorationSpecId));
        words.push_back(0u);
        words.push_back(makeOp(3, spv::Op::OpSpecConstant));
        words.push_back(2u);
        words.push_back(1u);

        std::vector<uint32_t> output;
        workaroundSpecConstant(toShaderBlob(words), specConstants, output);
        ASSERT_EQ(output.size(), 8u);
        EXPECT_EQ(output[5], makeOp(3, spv::Op::OpSpecConstant));
    }

    // OpSpecConstantTrue (3 words) paired with a non-bool (int) specialization constant.
    {
        std::vector<uint32_t> words = makeSpirvHeader();
        words.push_back(makeOp(4, spv::Op::OpDecorate));
        words.push_back(1u);
        words.push_back(static_cast<uint32_t>(spv::Decoration::DecorationSpecId));
        words.push_back(0u);
        words.push_back(makeOp(3, spv::Op::OpSpecConstantTrue));
        words.push_back(2u);
        words.push_back(1u);

        std::vector<uint32_t> output;
        workaroundSpecConstant(toShaderBlob(words), specConstants, output);
        ASSERT_EQ(output.size(), 8u);
        EXPECT_EQ(output[5], makeOp(3, spv::Op::OpSpecConstantTrue));
    }
}

TEST(VulkanSpirvUtilsTest, HandlesMissingOrOutOfRangeSpecId) {
    auto specConstants = utils::FixedCapacityVector<Program::SpecializationConstant>::with_capacity(1);
    specConstants.push_back(int32_t(77));

    // OpSpecConstant without any prior SpecId decoration, and one with SpecId = 50 (>= size).
    std::vector<uint32_t> words = makeSpirvHeader();
    words.push_back(makeOp(4, spv::Op::OpDecorate));
    words.push_back(2u);
    words.push_back(static_cast<uint32_t>(spv::Decoration::DecorationSpecId));
    words.push_back(50u);

    // %1 has no SpecId decoration.
    words.push_back(makeOp(4, spv::Op::OpSpecConstant));
    words.push_back(10u);
    words.push_back(1u);
    words.push_back(5u);

    // %2 has SpecId 50, which is out of range of specConstants (size 1).
    words.push_back(makeOp(4, spv::Op::OpSpecConstant));
    words.push_back(10u);
    words.push_back(2u);
    words.push_back(6u);

    std::vector<uint32_t> output;
    workaroundSpecConstant(toShaderBlob(words), specConstants, output);

    ASSERT_EQ(output.size(), 13u);
    EXPECT_EQ(output[5], makeOp(4, spv::Op::OpSpecConstant));
    EXPECT_EQ(output[8], 5u);
    EXPECT_EQ(output[9], makeOp(4, spv::Op::OpSpecConstant));
    EXPECT_EQ(output[12], 6u);
}

} // namespace filament::backend::fvkutils

