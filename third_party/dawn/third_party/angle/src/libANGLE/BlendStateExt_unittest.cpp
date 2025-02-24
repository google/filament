//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BlendStateExt_unittest.cpp: Unit tests of the BlendStateExt class.

#include <gtest/gtest.h>

#include "libANGLE/angletypes.h"

namespace angle
{

#if defined(ANGLE_IS_64_BIT_CPU)
constexpr bool is64Bit = true;
#else
constexpr bool is64Bit = false;
#endif

// Test the initial state of BlendStateExt
TEST(BlendStateExt, Init)
{
    const std::array<uint8_t, 9> allEnabledMasks = {0, 1, 3, 7, 15, 31, 63, 127, 255};

    const std::array<uint32_t, 9> allColorMasks32 = {
        0, 0xF, 0xFF, 0xFFF, 0xFFFF, 0xFFFFF, 0xFFFFFF, 0xFFFFFFF, 0xFFFFFFFF};

    const std::array<uint64_t, 9> allColorMasks64 = {
        0x0000000000000000, 0x000000000000000F, 0x0000000000000F0F,
        0x00000000000F0F0F, 0x000000000F0F0F0F, 0x0000000F0F0F0F0F,
        0x00000F0F0F0F0F0F, 0x000F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F};

    const std::array<uint64_t, 9> sourceColorAlpha = {
        0x0000000000000000, 0x0000000000000001, 0x0000000000000101,
        0x0000000000010101, 0x0000000001010101, 0x0000000101010101,
        0x0000010101010101, 0x0001010101010101, 0x0101010101010101};

    for (size_t c = 1; c <= 8; ++c)
    {
        const gl::BlendStateExt blendStateExt = gl::BlendStateExt(c);
        ASSERT_EQ(blendStateExt.getDrawBufferCount(), c);

        ASSERT_EQ(blendStateExt.getEnabledMask().to_ulong(), 0u);
        ASSERT_EQ(blendStateExt.getAllEnabledMask().to_ulong(), allEnabledMasks[c]);

        ASSERT_EQ(blendStateExt.getColorMaskBits(), blendStateExt.getAllColorMaskBits());
        ASSERT_EQ(blendStateExt.getAllColorMaskBits(),
                  is64Bit ? allColorMasks64[c] : allColorMasks32[c]);

        ASSERT_EQ(blendStateExt.getUsesAdvancedBlendEquationMask().to_ulong(), 0u);

        ASSERT_EQ(blendStateExt.getUsesExtendedBlendFactorMask().to_ulong(), 0u);

        ASSERT_EQ(blendStateExt.getSrcColorBits(), sourceColorAlpha[c]);
        ASSERT_EQ(blendStateExt.getSrcAlphaBits(), sourceColorAlpha[c]);
        ASSERT_EQ(blendStateExt.getDstColorBits(), 0u);
        ASSERT_EQ(blendStateExt.getDstAlphaBits(), 0u);

        ASSERT_EQ(blendStateExt.getEquationColorBits(), 0u);
        ASSERT_EQ(blendStateExt.getEquationAlphaBits(), 0u);

        for (size_t i = 0; i < c; ++i)
        {
            ASSERT_FALSE(blendStateExt.getEnabledMask().test(i));

            bool r, g, b, a;
            blendStateExt.getColorMaskIndexed(i, &r, &g, &b, &a);
            ASSERT_TRUE(r);
            ASSERT_TRUE(g);
            ASSERT_TRUE(b);
            ASSERT_TRUE(a);

            ASSERT_EQ(blendStateExt.getEquationColorIndexed(i), gl::BlendEquationType::Add);
            ASSERT_EQ(blendStateExt.getEquationAlphaIndexed(i), gl::BlendEquationType::Add);

            ASSERT_EQ(blendStateExt.getSrcColorIndexed(i), gl::BlendFactorType::One);
            ASSERT_EQ(blendStateExt.getDstColorIndexed(i), gl::BlendFactorType::Zero);
            ASSERT_EQ(blendStateExt.getSrcAlphaIndexed(i), gl::BlendFactorType::One);
            ASSERT_EQ(blendStateExt.getDstAlphaIndexed(i), gl::BlendFactorType::Zero);
        }
    }
}

// Test blend enabled flags
TEST(BlendStateExt, BlendEnabled)
{
    const std::array<uint8_t, 9> enabled = {0, 1, 3, 7, 15, 31, 63, 127, 255};

    for (size_t c = 1; c <= 8; c++)
    {
        gl::BlendStateExt blendStateExt = gl::BlendStateExt(c);

        blendStateExt.setEnabled(true);
        ASSERT_EQ(blendStateExt.getEnabledMask().to_ulong(), enabled[c]);

        blendStateExt.setEnabled(false);
        ASSERT_EQ(blendStateExt.getEnabledMask().to_ulong(), 0u);

        blendStateExt.setEnabledIndexed(c / 2, true);
        ASSERT_EQ(blendStateExt.getEnabledMask().to_ulong(), 1u << (c / 2));

        blendStateExt.setEnabledIndexed(c / 2, false);
        ASSERT_EQ(blendStateExt.getEnabledMask().to_ulong(), 0u);
    }
}

void validateMaskPacking(const uint8_t packed,
                         const bool r,
                         const bool g,
                         const bool b,
                         const bool a)
{
    ASSERT_EQ(gl::BlendStateExt::PackColorMask(r, g, b, a), packed);

    bool rOut, gOut, bOut, aOut;
    gl::BlendStateExt::UnpackColorMask(packed, &rOut, &gOut, &bOut, &aOut);
    ASSERT_EQ(r, rOut);
    ASSERT_EQ(g, gOut);
    ASSERT_EQ(b, bOut);
    ASSERT_EQ(a, aOut);
}

// Test color write mask packing
TEST(BlendStateExt, ColorMaskPacking)
{
    validateMaskPacking(0x0, false, false, false, false);
    validateMaskPacking(0x1, true, false, false, false);
    validateMaskPacking(0x2, false, true, false, false);
    validateMaskPacking(0x3, true, true, false, false);
    validateMaskPacking(0x4, false, false, true, false);
    validateMaskPacking(0x5, true, false, true, false);
    validateMaskPacking(0x6, false, true, true, false);
    validateMaskPacking(0x7, true, true, true, false);
    validateMaskPacking(0x8, false, false, false, true);
    validateMaskPacking(0x9, true, false, false, true);
    validateMaskPacking(0xA, false, true, false, true);
    validateMaskPacking(0xB, true, true, false, true);
    validateMaskPacking(0xC, false, false, true, true);
    validateMaskPacking(0xD, true, false, true, true);
    validateMaskPacking(0xE, false, true, true, true);
    validateMaskPacking(0xF, true, true, true, true);
}

// Test color write mask manipulations
TEST(BlendStateExt, ColorMask)
{
    const std::array<uint32_t, 9> startSingleValue32 = {0x00000000, 0x00000005, 0x00000055,
                                                        0x00000555, 0x00005555, 0x00055555,
                                                        0x00555555, 0x05555555, 0x55555555};

    const std::array<uint64_t, 9> startSingleValue64 = {
        0x0000000000000000, 0x0000000000000005, 0x0000000000000505,
        0x0000000000050505, 0x0000000005050505, 0x0000000505050505,
        0x0000050505050505, 0x0005050505050505, 0x0505050505050505};

    for (size_t c = 1; c <= 8; c++)
    {
        gl::BlendStateExt blendStateExt = gl::BlendStateExt(c);

        blendStateExt.setColorMask(true, false, true, false);
        ASSERT_EQ(blendStateExt.getColorMaskBits(),
                  is64Bit ? startSingleValue64[c] : startSingleValue32[c]);

        blendStateExt.setColorMaskIndexed(c / 2, false, true, false, true);
        for (size_t i = 0; i < c; ++i)
        {
            ASSERT_EQ(blendStateExt.getColorMaskIndexed(i), i == c / 2 ? 0xAu : 0x5u);
        }

        blendStateExt.setColorMaskIndexed(0, 0xF);
        bool r, g, b, a;
        blendStateExt.getColorMaskIndexed(0, &r, &g, &b, &a);
        ASSERT_TRUE(r);
        ASSERT_TRUE(g);
        ASSERT_TRUE(b);
        ASSERT_TRUE(a);

        blendStateExt.setColorMaskIndexed(c - 1, true, false, true, true);
        gl::BlendStateExt::ColorMaskStorage::Type otherColorMask =
            blendStateExt.expandColorMaskIndexed(c - 1);
        for (size_t i = 0; i < c; ++i)
        {
            ASSERT_EQ(gl::BlendStateExt::ColorMaskStorage::GetValueIndexed(i, otherColorMask), 0xD);
        }

        // All masks are different except the c-th
        {
            const gl::DrawBufferMask diff = blendStateExt.compareColorMask(otherColorMask);
            ASSERT_EQ(diff.to_ulong(), 127u >> (8 - c));
        }

        // Test that all-enabled color mask correctly compares with the current color mask
        {
            blendStateExt.setColorMask(true, true, true, true);
            blendStateExt.setColorMaskIndexed(c / 2, false, false, true, false);
            const gl::DrawBufferMask diff =
                blendStateExt.compareColorMask(blendStateExt.getAllColorMaskBits());
            ASSERT_EQ(diff.to_ulong(), 1u << (c / 2));
        }
    }
}

// Test blend equations manipulations
TEST(BlendStateExt, BlendEquations)
{
    gl::BlendStateExt blendStateExt = gl::BlendStateExt(7);

    blendStateExt.setEquations(GL_MIN, GL_FUNC_SUBTRACT);
    ASSERT_EQ(blendStateExt.getEquationColorBits(), 0x01010101010101u);
    ASSERT_EQ(blendStateExt.getEquationAlphaBits(), 0x04040404040404u);

    blendStateExt.setEquationsIndexed(3, GL_MAX, GL_FUNC_SUBTRACT);
    blendStateExt.setEquationsIndexed(5, GL_MIN, GL_FUNC_ADD);
    ASSERT_EQ(blendStateExt.getEquationColorBits(), 0x01010102010101u);
    ASSERT_EQ(blendStateExt.getEquationAlphaBits(), 0x04000404040404u);
    ASSERT_EQ(blendStateExt.getEquationColorIndexed(3), gl::BlendEquationType::Max);
    ASSERT_EQ(blendStateExt.getEquationAlphaIndexed(5), gl::BlendEquationType::Add);

    gl::BlendStateExt::EquationStorage::Type otherEquationColor =
        blendStateExt.expandEquationColorIndexed(0);
    gl::BlendStateExt::EquationStorage::Type otherEquationAlpha =
        blendStateExt.expandEquationAlphaIndexed(0);

    ASSERT_EQ(otherEquationColor, 0x01010101010101u);
    ASSERT_EQ(otherEquationAlpha, 0x04040404040404u);

    const gl::DrawBufferMask diff =
        blendStateExt.compareEquations(otherEquationColor, otherEquationAlpha);
    ASSERT_EQ(diff.to_ulong(), 40u);

    // Copy buffer 3 to buffer 0
    blendStateExt.setEquationsIndexed(0, 3, blendStateExt);
    ASSERT_EQ(blendStateExt.getEquationColorIndexed(0), gl::BlendEquationType::Max);
    ASSERT_EQ(blendStateExt.getEquationAlphaIndexed(0), gl::BlendEquationType::Subtract);

    // Copy buffer 5 to buffer 0
    blendStateExt.setEquationsIndexed(0, 5, blendStateExt);
    ASSERT_EQ(blendStateExt.getEquationColorIndexed(0), gl::BlendEquationType::Min);
    ASSERT_EQ(blendStateExt.getEquationAlphaIndexed(0), gl::BlendEquationType::Add);
}

// Test blend factors manipulations
TEST(BlendStateExt, BlendFactors)
{
    gl::BlendStateExt blendStateExt = gl::BlendStateExt(8);

    blendStateExt.setFactors(GL_SRC_COLOR, GL_DST_COLOR, GL_SRC_ALPHA, GL_DST_ALPHA);
    ASSERT_EQ(blendStateExt.getSrcColorBits(), 0x0202020202020202u);
    ASSERT_EQ(blendStateExt.getDstColorBits(), 0x0808080808080808u);
    ASSERT_EQ(blendStateExt.getSrcAlphaBits(), 0x0404040404040404u);
    ASSERT_EQ(blendStateExt.getDstAlphaBits(), 0x0606060606060606u);

    blendStateExt.setFactorsIndexed(0, GL_ONE, GL_DST_COLOR, GL_SRC_ALPHA, GL_DST_ALPHA);
    blendStateExt.setFactorsIndexed(3, GL_SRC_COLOR, GL_ONE, GL_SRC_ALPHA, GL_DST_ALPHA);
    blendStateExt.setFactorsIndexed(5, GL_SRC_COLOR, GL_DST_COLOR, GL_ONE, GL_DST_ALPHA);
    blendStateExt.setFactorsIndexed(7, GL_SRC_COLOR, GL_DST_COLOR, GL_SRC_ALPHA, GL_ONE);
    ASSERT_EQ(blendStateExt.getSrcColorBits(), 0x0202020202020201u);
    ASSERT_EQ(blendStateExt.getDstColorBits(), 0x0808080801080808u);
    ASSERT_EQ(blendStateExt.getSrcAlphaBits(), 0x0404010404040404u);
    ASSERT_EQ(blendStateExt.getDstAlphaBits(), 0x0106060606060606u);

    ASSERT_EQ(blendStateExt.getSrcColorIndexed(0), gl::BlendFactorType::One);
    ASSERT_EQ(blendStateExt.getDstColorIndexed(3), gl::BlendFactorType::One);
    ASSERT_EQ(blendStateExt.getSrcAlphaIndexed(5), gl::BlendFactorType::One);
    ASSERT_EQ(blendStateExt.getDstAlphaIndexed(7), gl::BlendFactorType::One);

    gl::BlendStateExt::FactorStorage::Type otherSrcColor = blendStateExt.expandSrcColorIndexed(1);
    gl::BlendStateExt::FactorStorage::Type otherDstColor = blendStateExt.expandDstColorIndexed(1);
    gl::BlendStateExt::FactorStorage::Type otherSrcAlpha = blendStateExt.expandSrcAlphaIndexed(1);
    gl::BlendStateExt::FactorStorage::Type otherDstAlpha = blendStateExt.expandDstAlphaIndexed(1);

    ASSERT_EQ(otherSrcColor, 0x0202020202020202u);
    ASSERT_EQ(otherDstColor, 0x0808080808080808u);
    ASSERT_EQ(otherSrcAlpha, 0x0404040404040404u);
    ASSERT_EQ(otherDstAlpha, 0x0606060606060606u);

    const gl::DrawBufferMask diff =
        blendStateExt.compareFactors(otherSrcColor, otherDstColor, otherSrcAlpha, otherDstAlpha);
    ASSERT_EQ(diff.to_ulong(), 169u);

    // Copy buffer 0 to buffer 1
    blendStateExt.setFactorsIndexed(1, 0, blendStateExt);
    ASSERT_EQ(blendStateExt.getSrcColorIndexed(1), gl::BlendFactorType::One);
    ASSERT_EQ(blendStateExt.getDstColorIndexed(1), gl::BlendFactorType::DstColor);
    ASSERT_EQ(blendStateExt.getSrcAlphaIndexed(1), gl::BlendFactorType::SrcAlpha);
    ASSERT_EQ(blendStateExt.getDstAlphaIndexed(1), gl::BlendFactorType::DstAlpha);

    // Copy buffer 3 to buffer 1
    blendStateExt.setFactorsIndexed(1, 3, blendStateExt);
    ASSERT_EQ(blendStateExt.getSrcColorIndexed(1), gl::BlendFactorType::SrcColor);
    ASSERT_EQ(blendStateExt.getDstColorIndexed(1), gl::BlendFactorType::One);
    ASSERT_EQ(blendStateExt.getSrcAlphaIndexed(1), gl::BlendFactorType::SrcAlpha);
    ASSERT_EQ(blendStateExt.getDstAlphaIndexed(1), gl::BlendFactorType::DstAlpha);

    // Copy buffer 5 to buffer 1
    blendStateExt.setFactorsIndexed(1, 5, blendStateExt);
    ASSERT_EQ(blendStateExt.getSrcColorIndexed(1), gl::BlendFactorType::SrcColor);
    ASSERT_EQ(blendStateExt.getDstColorIndexed(1), gl::BlendFactorType::DstColor);
    ASSERT_EQ(blendStateExt.getSrcAlphaIndexed(1), gl::BlendFactorType::One);
    ASSERT_EQ(blendStateExt.getDstAlphaIndexed(1), gl::BlendFactorType::DstAlpha);

    // Copy buffer 7 to buffer 1
    blendStateExt.setFactorsIndexed(1, 7, blendStateExt);
    ASSERT_EQ(blendStateExt.getSrcColorIndexed(1), gl::BlendFactorType::SrcColor);
    ASSERT_EQ(blendStateExt.getDstColorIndexed(1), gl::BlendFactorType::DstColor);
    ASSERT_EQ(blendStateExt.getSrcAlphaIndexed(1), gl::BlendFactorType::SrcAlpha);
    ASSERT_EQ(blendStateExt.getDstAlphaIndexed(1), gl::BlendFactorType::One);
}

// Test clip rectangle
TEST(Rectangle, Clip)
{
    const gl::Rectangle source(0, 0, 100, 200);
    const gl::Rectangle clip1(0, 0, 50, 100);
    gl::Rectangle result;

    ASSERT_TRUE(gl::ClipRectangle(source, clip1, &result));
    ASSERT_EQ(result.x, 0);
    ASSERT_EQ(result.y, 0);
    ASSERT_EQ(result.width, 50);
    ASSERT_EQ(result.height, 100);

    gl::Rectangle clip2(10, 20, 30, 40);

    ASSERT_TRUE(gl::ClipRectangle(source, clip2, &result));
    ASSERT_EQ(result.x, 10);
    ASSERT_EQ(result.y, 20);
    ASSERT_EQ(result.width, 30);
    ASSERT_EQ(result.height, 40);

    gl::Rectangle clip3(-20, -30, 10000, 400000);

    ASSERT_TRUE(gl::ClipRectangle(source, clip3, &result));
    ASSERT_EQ(result.x, 0);
    ASSERT_EQ(result.y, 0);
    ASSERT_EQ(result.width, 100);
    ASSERT_EQ(result.height, 200);

    gl::Rectangle clip4(50, 100, -20, -30);

    ASSERT_TRUE(gl::ClipRectangle(source, clip4, &result));
    ASSERT_EQ(result.x, 30);
    ASSERT_EQ(result.y, 70);
    ASSERT_EQ(result.width, 20);
    ASSERT_EQ(result.height, 30);

    // Non-overlapping rectangles
    gl::Rectangle clip5(-100, 0, 99, 200);
    ASSERT_FALSE(gl::ClipRectangle(source, clip5, nullptr));

    gl::Rectangle clip6(0, -100, 100, 99);
    ASSERT_FALSE(gl::ClipRectangle(source, clip6, nullptr));

    gl::Rectangle clip7(101, 0, 99, 200);
    ASSERT_FALSE(gl::ClipRectangle(source, clip7, nullptr));

    gl::Rectangle clip8(0, 201, 100, 99);
    ASSERT_FALSE(gl::ClipRectangle(source, clip8, nullptr));

    // Zero-width/height rectangles
    gl::Rectangle clip9(50, 0, 0, 200);
    ASSERT_FALSE(gl::ClipRectangle(source, clip9, nullptr));
    ASSERT_FALSE(gl::ClipRectangle(clip9, source, nullptr));

    gl::Rectangle clip10(0, 100, 100, 0);
    ASSERT_FALSE(gl::ClipRectangle(source, clip10, nullptr));
    ASSERT_FALSE(gl::ClipRectangle(clip10, source, nullptr));
}

// Test combine rectangles
TEST(Rectangle, Combine)
{
    const gl::Rectangle rect1(0, 0, 100, 200);
    const gl::Rectangle rect2(0, 0, 50, 100);
    gl::Rectangle result;

    gl::GetEnclosingRectangle(rect1, rect2, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), 0);
    ASSERT_EQ(result.x1(), 100);
    ASSERT_EQ(result.y1(), 200);

    const gl::Rectangle rect3(50, 100, 100, 200);

    gl::GetEnclosingRectangle(rect1, rect3, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), 0);
    ASSERT_EQ(result.x1(), 150);
    ASSERT_EQ(result.y1(), 300);

    const gl::Rectangle rect4(-20, -30, 100, 200);

    gl::GetEnclosingRectangle(rect1, rect4, &result);
    ASSERT_EQ(result.x0(), -20);
    ASSERT_EQ(result.y0(), -30);
    ASSERT_EQ(result.x1(), 100);
    ASSERT_EQ(result.y1(), 200);

    const gl::Rectangle rect5(10, -30, 100, 200);

    gl::GetEnclosingRectangle(rect1, rect5, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), -30);
    ASSERT_EQ(result.x1(), 110);
    ASSERT_EQ(result.y1(), 200);
}

// Test extend rectangles
TEST(Rectangle, Extend)
{
    const gl::Rectangle source(0, 0, 100, 200);
    const gl::Rectangle extend1(0, 0, 50, 100);
    gl::Rectangle result;

    //  +------+       +------+
    //  |   |  |       |      |
    //  +---+  |  -->  |      |
    //  |      |       |      |
    //  +------+       +------+
    //
    gl::ExtendRectangle(source, extend1, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), 0);
    ASSERT_EQ(result.x1(), 100);
    ASSERT_EQ(result.y1(), 200);

    //  +------+           +------+
    //  |S     |           |      |
    //  |   +--+---+  -->  |      |
    //  |   |  |   |       |      |
    //  +---+--+   +       +------+
    //      |      |
    //      +------+
    //
    const gl::Rectangle extend2(50, 100, 100, 200);

    gl::ExtendRectangle(source, extend2, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), 0);
    ASSERT_EQ(result.x1(), 100);
    ASSERT_EQ(result.y1(), 200);

    //    +------+           +------+
    //    |S     |           |      |
    //  +-+------+---+  -->  |      |
    //  | |      |   |       |      |
    //  | +------+   +       |      |
    //  |            |       |      |
    //  +------------+       +------+
    //
    const gl::Rectangle extend3(-10, 100, 200, 200);

    gl::ExtendRectangle(source, extend3, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), 0);
    ASSERT_EQ(result.x1(), 100);
    ASSERT_EQ(result.y1(), 300);

    //    +------+           +------+
    //    |S     |           |      |
    //    |      |      -->  |      |
    //    |      |           |      |
    //  +-+------+---+       |      |
    //  |            |       |      |
    //  +------------+       +------+
    //
    for (int offsetLeft = 10; offsetLeft >= 0; offsetLeft -= 10)
    {
        for (int offsetRight = 10; offsetRight >= 0; offsetRight -= 10)
        {
            const gl::Rectangle extend4(-offsetLeft, 200, 100 + offsetLeft + offsetRight, 100);

            gl::ExtendRectangle(source, extend4, &result);
            ASSERT_EQ(result.x0(), 0) << offsetLeft << " " << offsetRight;
            ASSERT_EQ(result.y0(), 0) << offsetLeft << " " << offsetRight;
            ASSERT_EQ(result.x1(), 100) << offsetLeft << " " << offsetRight;
            ASSERT_EQ(result.y1(), 300) << offsetLeft << " " << offsetRight;
        }
    }

    // Similar to extend4, but with the second rectangle on the top, left and right.
    for (int offsetLeft = 10; offsetLeft >= 0; offsetLeft -= 10)
    {
        for (int offsetRight = 10; offsetRight >= 0; offsetRight -= 10)
        {
            const gl::Rectangle extend4(-offsetLeft, -100, 100 + offsetLeft + offsetRight, 100);

            gl::ExtendRectangle(source, extend4, &result);
            ASSERT_EQ(result.x0(), 0) << offsetLeft << " " << offsetRight;
            ASSERT_EQ(result.y0(), -100) << offsetLeft << " " << offsetRight;
            ASSERT_EQ(result.x1(), 100) << offsetLeft << " " << offsetRight;
            ASSERT_EQ(result.y1(), 200) << offsetLeft << " " << offsetRight;
        }
    }
    for (int offsetTop = 10; offsetTop >= 0; offsetTop -= 10)
    {
        for (int offsetBottom = 10; offsetBottom >= 0; offsetBottom -= 10)
        {
            const gl::Rectangle extend4(-50, -offsetTop, 50, 200 + offsetTop + offsetBottom);

            gl::ExtendRectangle(source, extend4, &result);
            ASSERT_EQ(result.x0(), -50) << offsetTop << " " << offsetBottom;
            ASSERT_EQ(result.y0(), 0) << offsetTop << " " << offsetBottom;
            ASSERT_EQ(result.x1(), 100) << offsetTop << " " << offsetBottom;
            ASSERT_EQ(result.y1(), 200) << offsetTop << " " << offsetBottom;
        }
    }
    for (int offsetTop = 10; offsetTop >= 0; offsetTop -= 10)
    {
        for (int offsetBottom = 10; offsetBottom >= 0; offsetBottom -= 10)
        {
            const gl::Rectangle extend4(100, -offsetTop, 50, 200 + offsetTop + offsetBottom);

            gl::ExtendRectangle(source, extend4, &result);
            ASSERT_EQ(result.x0(), 0) << offsetTop << " " << offsetBottom;
            ASSERT_EQ(result.y0(), 0) << offsetTop << " " << offsetBottom;
            ASSERT_EQ(result.x1(), 150) << offsetTop << " " << offsetBottom;
            ASSERT_EQ(result.y1(), 200) << offsetTop << " " << offsetBottom;
        }
    }

    //    +------+           +------+
    //    |S     |           |      |
    //    |      |      -->  |      |
    //    |      |           |      |
    //    +------+           +------+
    //  +------------+
    //  |            |
    //  +------------+
    //
    const gl::Rectangle extend5(-10, 201, 120, 100);

    gl::ExtendRectangle(source, extend5, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), 0);
    ASSERT_EQ(result.x1(), 100);
    ASSERT_EQ(result.y1(), 200);

    // Similar to extend5, but with the second rectangle on the top, left and right.
    const gl::Rectangle extend6(-10, -101, 120, 100);

    gl::ExtendRectangle(source, extend6, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), 0);
    ASSERT_EQ(result.x1(), 100);
    ASSERT_EQ(result.y1(), 200);

    const gl::Rectangle extend7(-101, -10, 100, 220);

    gl::ExtendRectangle(source, extend7, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), 0);
    ASSERT_EQ(result.x1(), 100);
    ASSERT_EQ(result.y1(), 200);

    const gl::Rectangle extend8(101, -10, 100, 220);

    gl::ExtendRectangle(source, extend8, &result);
    ASSERT_EQ(result.x0(), 0);
    ASSERT_EQ(result.y0(), 0);
    ASSERT_EQ(result.x1(), 100);
    ASSERT_EQ(result.y1(), 200);

    //  +-------------+       +-------------+
    //  |   +------+  |       |             |
    //  |   |S     |  |       |             |
    //  |   |      |  |  -->  |             |
    //  |   |      |  |       |             |
    //  |   +------+  |       |             |
    //  +-------------+       +-------------+
    //
    const gl::Rectangle extend9(-100, -100, 300, 400);

    gl::ExtendRectangle(source, extend9, &result);
    ASSERT_EQ(result.x0(), -100);
    ASSERT_EQ(result.y0(), -100);
    ASSERT_EQ(result.x1(), 200);
    ASSERT_EQ(result.y1(), 300);
}

}  // namespace angle
