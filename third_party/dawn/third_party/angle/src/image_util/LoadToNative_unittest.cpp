//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LoadToNative_unittest.cpp: Unit tests for pixel loading functions.

#include <gmock/gmock.h>
#include <vector>
#include "common/debug.h"
#include "common/mathutil.h"
#include "image_util/loadimage.h"

using namespace angle;
using namespace testing;

namespace
{

template <typename Type>
void initializeRGBInput(std::vector<Type> &rgbInput,
                        size_t width,
                        size_t height,
                        size_t depth,
                        size_t inputPixelBytes,
                        size_t inputByteOffset,
                        size_t inputRowPitch,
                        size_t inputDepthPitch)
{
    ASSERT(rgbInput.size() == inputDepthPitch * depth + inputByteOffset);
    for (size_t z = 0; z < depth; z++)
    {
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                size_t inputIndex =
                    inputByteOffset + z * inputDepthPitch + y * inputRowPitch + x * inputPixelBytes;

                rgbInput[inputIndex]     = x % 256;
                rgbInput[inputIndex + 1] = y % 256;
                rgbInput[inputIndex + 2] = z % 256;
            }
        }
    }
}

template <typename Type, uint8_t fourthValue>
void verifyRGBToRGBAResults(const char *strCase,
                            std::vector<Type> &rgbInput,
                            std::vector<Type> &rgbaOutput,
                            size_t width,
                            size_t height,
                            size_t depth,
                            size_t inputPixelBytes,
                            size_t inputByteOffset,
                            size_t inputRowPitch,
                            size_t inputDepthPitch,
                            size_t outputPixelBytes,
                            size_t outputByteOffset,
                            size_t outputRowPitch,
                            size_t outputDepthPitch)
{
    ASSERT(rgbInput.size() == inputDepthPitch * depth + inputByteOffset);
    ASSERT(rgbaOutput.size() == outputDepthPitch * depth + outputByteOffset);
    for (size_t z = 0; z < depth; z++)
    {
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                size_t inputIndex =
                    inputByteOffset + z * inputDepthPitch + y * inputRowPitch + x * inputPixelBytes;
                size_t outputIndex = outputByteOffset + z * outputDepthPitch + y * outputRowPitch +
                                     x * outputPixelBytes;

                bool rMatch = rgbInput[inputIndex] == rgbaOutput[outputIndex];
                bool gMatch = rgbInput[inputIndex + 1] == rgbaOutput[outputIndex + 1];
                bool bMatch = rgbInput[inputIndex + 2] == rgbaOutput[outputIndex + 2];
                bool aMatch = rgbaOutput[outputIndex + 3] == fourthValue;

                EXPECT_TRUE(rMatch && gMatch && bMatch && aMatch)
                    << "Case " << strCase << ": Mismatch at Index (" << x << ", " << y << ", " << z
                    << ")" << std::endl
                    << "Expected output: (" << static_cast<uint32_t>(rgbInput[inputIndex]) << ", "
                    << static_cast<uint32_t>(rgbInput[inputIndex + 1]) << ", "
                    << static_cast<uint32_t>(rgbInput[inputIndex + 2]) << ", "
                    << static_cast<uint32_t>(fourthValue) << ")" << std::endl
                    << "Actual output: (" << static_cast<uint32_t>(rgbaOutput[outputIndex]) << ", "
                    << static_cast<uint32_t>(rgbaOutput[outputIndex + 1]) << ", "
                    << static_cast<uint32_t>(rgbaOutput[outputIndex + 2]) << ", "
                    << static_cast<uint32_t>(rgbaOutput[outputIndex + 3]) << ")";
            }
        }
    }
}

template <uint8_t fourthValue>
void TestLoadUbyteRGBToRGBA(ImageLoadContext &context,
                            const char *strCase,
                            size_t width,
                            size_t height,
                            size_t depth,
                            size_t inputByteOffset,
                            size_t outputByteOffset,
                            size_t inputRowAlignment)
{
    constexpr uint8_t kInitialByteValue = 0xAA;

    size_t inputPixelBytes  = 3;
    size_t inputRowPitch    = rx::roundUpPow2(width * inputPixelBytes, inputRowAlignment);
    size_t inputDepthPitch  = height * inputRowPitch;
    size_t inputActualBytes = depth * inputDepthPitch;

    size_t outputPixelBytes  = 4;
    size_t outputRowPitch    = width * outputPixelBytes;
    size_t outputDepthPitch  = height * outputRowPitch;
    size_t outputActualBytes = depth * outputDepthPitch;

    // Prepare the RGB input and RGBA output for copy. The offset values are used to add unused
    // bytes to the beginning of the input and output data, in order to test address alignments.
    std::vector<uint8_t> rgbInput(inputByteOffset + inputActualBytes, kInitialByteValue);
    initializeRGBInput<uint8_t>(rgbInput, width, height, depth, inputPixelBytes, inputByteOffset,
                                inputRowPitch, inputDepthPitch);

    std::vector<uint8_t> rgbaOutput(outputByteOffset + outputActualBytes, kInitialByteValue);

    // Call loading function.
    LoadToNative3To4<uint8_t, fourthValue>(
        context, width, height, depth, rgbInput.data() + inputByteOffset, inputRowPitch,
        inputDepthPitch, rgbaOutput.data() + outputByteOffset, outputRowPitch, outputDepthPitch);

    // Compare the input and output data.
    verifyRGBToRGBAResults<uint8_t, fourthValue>(
        strCase, rgbInput, rgbaOutput, width, height, depth, inputPixelBytes, inputByteOffset,
        inputRowPitch, inputDepthPitch, outputPixelBytes, outputByteOffset, outputRowPitch,
        outputDepthPitch);
}

template <uint8_t fourthValue>
void TestLoadSbyteRGBToRGBA(ImageLoadContext &context,
                            const char *strCase,
                            size_t width,
                            size_t height,
                            size_t depth,
                            size_t inputByteOffset,
                            size_t outputByteOffset,
                            size_t inputRowAlignment)
{
    constexpr int8_t kInitialByteValue = 0xAA;

    size_t inputPixelBytes  = 3;
    size_t inputRowPitch    = rx::roundUpPow2(width * inputPixelBytes, inputRowAlignment);
    size_t inputDepthPitch  = height * inputRowPitch;
    size_t inputActualBytes = depth * inputDepthPitch;

    size_t outputPixelBytes  = 4;
    size_t outputRowPitch    = width * outputPixelBytes;
    size_t outputDepthPitch  = height * outputRowPitch;
    size_t outputActualBytes = depth * outputDepthPitch;

    // Prepare the RGB input and RGBA output for copy. The offset values are used to add unused
    // bytes to the beginning of the input and output data, in order to test address alignments.
    std::vector<int8_t> rgbInput(inputByteOffset + inputActualBytes, kInitialByteValue);
    initializeRGBInput<int8_t>(rgbInput, width, height, depth, inputPixelBytes, inputByteOffset,
                               inputRowPitch, inputDepthPitch);

    std::vector<int8_t> rgbaOutput(outputByteOffset + outputActualBytes, kInitialByteValue);

    // Call loading function.
    LoadToNative3To4<int8_t, fourthValue>(
        context, width, height, depth,
        reinterpret_cast<uint8_t *>(rgbInput.data() + inputByteOffset), inputRowPitch,
        inputDepthPitch, reinterpret_cast<uint8_t *>(rgbaOutput.data() + outputByteOffset),
        outputRowPitch, outputDepthPitch);

    // Compare the input and output data.
    verifyRGBToRGBAResults<int8_t, fourthValue>(strCase, rgbInput, rgbaOutput, width, height, depth,
                                                inputPixelBytes, inputByteOffset, inputRowPitch,
                                                inputDepthPitch, outputPixelBytes, outputByteOffset,
                                                outputRowPitch, outputDepthPitch);
}

void TestLoadByteRGBToRGBAForAllCases(ImageLoadContext &context,
                                      size_t inputCase,
                                      size_t width,
                                      size_t height,
                                      size_t depth,
                                      size_t inputByteOffset,
                                      size_t outputByteOffset,
                                      size_t inputRowAlignment)
{
    std::string strCaseUFF = "UFF_" + std::to_string(inputCase);
    TestLoadUbyteRGBToRGBA<0xFF>(context, strCaseUFF.c_str(), width, height, depth, inputByteOffset,
                                 outputByteOffset, inputRowAlignment);
    std::string strCaseU01 = "U01_" + std::to_string(inputCase);
    TestLoadUbyteRGBToRGBA<0x01>(context, strCaseU01.c_str(), width, height, depth, inputByteOffset,
                                 outputByteOffset, inputRowAlignment);
    std::string strCaseS7F = "S7F_" + std::to_string(inputCase);
    TestLoadSbyteRGBToRGBA<0x7F>(context, strCaseS7F.c_str(), width, height, depth, inputByteOffset,
                                 outputByteOffset, inputRowAlignment);
    std::string strCaseS01 = "S01_" + std::to_string(inputCase);
    TestLoadSbyteRGBToRGBA<0x01>(context, strCaseS01.c_str(), width, height, depth, inputByteOffset,
                                 outputByteOffset, inputRowAlignment);
}

// Tests the ubyte (0xFF) RGB to RGBA loading function for one RGB pixel.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBADataOnePixelWithFourthCompOfFF)
{
    ImageLoadContext context;
    uint8_t rgbInput[]             = {1, 2, 3};
    uint8_t rgbaOutput[]           = {0, 0, 0, 0};
    constexpr uint8_t kFourthValue = 0xFF;

    LoadToNative3To4<uint8_t, kFourthValue>(context, 1, 1, 1, rgbInput, 3, 3, rgbaOutput, 4, 4);

    EXPECT_TRUE(rgbaOutput[0] == rgbInput[0] && rgbaOutput[1] == rgbInput[1] &&
                rgbaOutput[2] == rgbInput[2] && rgbaOutput[3] == kFourthValue)
        << "Pixel mismatch";
}

// Tests the ubyte (0x01) RGB to RGBA loading function for one RGB pixel.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBADataOnePixelWithFourthCompOf01)
{
    ImageLoadContext context;
    uint8_t rgbInput[]             = {1, 2, 3};
    uint8_t rgbaOutput[]           = {0, 0, 0, 0};
    constexpr uint8_t kFourthValue = 0x01;

    LoadToNative3To4<uint8_t, kFourthValue>(context, 1, 1, 1, rgbInput, 3, 3, rgbaOutput, 4, 4);

    EXPECT_TRUE(rgbaOutput[0] == rgbInput[0] && rgbaOutput[1] == rgbInput[1] &&
                rgbaOutput[2] == rgbInput[2] && rgbaOutput[3] == kFourthValue)
        << "Pixel mismatch";
}

// Tests the sbyte (0x7F) RGB to RGBA loading function for one RGB pixel.
TEST(LoadToNative3To4, LoadSbyteRGBToRGBADataOnePixelWithFourthCompOf7F)
{
    ImageLoadContext context;
    int8_t rgbInput[]              = {1, 2, 3};
    int8_t rgbaOutput[]            = {0, 0, 0, 0};
    constexpr uint8_t kFourthValue = 0x01;

    LoadToNative3To4<int8_t, kFourthValue>(context, 1, 1, 1, reinterpret_cast<uint8_t *>(rgbInput),
                                           3, 3, reinterpret_cast<uint8_t *>(rgbaOutput), 4, 4);

    EXPECT_TRUE(rgbaOutput[0] == rgbInput[0] && rgbaOutput[1] == rgbInput[1] &&
                rgbaOutput[2] == rgbInput[2] && rgbaOutput[3] == static_cast<int8_t>(kFourthValue))
        << "Pixel mismatch";
}

// Tests the sbyte (0x01) RGB to RGBA loading function for one RGB pixel.
TEST(LoadToNative3To4, LoadSbyteRGBToRGBADataOnePixelWithFourthCompOf01)
{
    ImageLoadContext context;
    int8_t rgbInput[]              = {1, 2, 3};
    int8_t rgbaOutput[]            = {0, 0, 0, 0};
    constexpr uint8_t kFourthValue = 0x01;

    LoadToNative3To4<int8_t, kFourthValue>(context, 1, 1, 1, reinterpret_cast<uint8_t *>(rgbInput),
                                           3, 3, reinterpret_cast<uint8_t *>(rgbaOutput), 4, 4);

    EXPECT_TRUE(rgbaOutput[0] == rgbInput[0] && rgbaOutput[1] == rgbInput[1] &&
                rgbaOutput[2] == rgbInput[2] && rgbaOutput[3] == static_cast<int8_t>(kFourthValue))
        << "Pixel mismatch";
}

// Tests the ubyte (0xFF) RGB to RGBA loading function for 4 RGB pixels, which should be read
// together.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBADataFourPixelsWithFourthCompOfFF)
{
    ImageLoadContext context;
    constexpr uint8_t kPixelCount = 4;
    std::vector<uint8_t> rgbInput(kPixelCount * 3);
    std::vector<uint8_t> rgbaOutput(kPixelCount * 4, 0);
    size_t index = 0;
    for (auto &inputComponent : rgbInput)
    {
        inputComponent = ++index;
    }
    constexpr uint8_t kFourthValue = 0xFF;

    LoadToNative3To4<uint8_t, kFourthValue>(context, 4, 1, 1, rgbInput.data(), 3, 3,
                                            rgbaOutput.data(), 4, 4);

    for (index = 0; index < kPixelCount; index++)
    {
        EXPECT_TRUE(rgbaOutput[index * 4] == rgbInput[index * 3] &&
                    rgbaOutput[index * 4 + 1] == rgbInput[index * 3 + 1] &&
                    rgbaOutput[index * 4 + 2] == rgbInput[index * 3 + 2] &&
                    rgbaOutput[index * 4 + 3] == kFourthValue)
            << "Mismatch at pixel " << index;
    }
}

// Tests the ubyte (0x01) RGB to RGBA loading function for 4 RGB pixels, which should be read
// together.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBADataFourPixelsWithFourthCompOf01)
{
    ImageLoadContext context;
    constexpr uint8_t kPixelCount = 4;
    std::vector<uint8_t> rgbInput(kPixelCount * 3);
    std::vector<uint8_t> rgbaOutput(kPixelCount * 4, 0);
    size_t index = 0;
    for (auto &inputComponent : rgbInput)
    {
        inputComponent = ++index;
    }
    constexpr uint8_t kFourthValue = 0x01;

    LoadToNative3To4<uint8_t, kFourthValue>(context, 4, 1, 1, rgbInput.data(), 3, 3,
                                            rgbaOutput.data(), 4, 4);

    for (index = 0; index < kPixelCount; index++)
    {
        EXPECT_TRUE(rgbaOutput[index * 4] == rgbInput[index * 3] &&
                    rgbaOutput[index * 4 + 1] == rgbInput[index * 3 + 1] &&
                    rgbaOutput[index * 4 + 2] == rgbInput[index * 3 + 2] &&
                    rgbaOutput[index * 4 + 3] == kFourthValue)
            << "Mismatch at pixel " << index;
    }
}

// Tests the sbyte (0x7F) RGB to RGBA loading function for 4 RGB pixels, which should be read
// together.
TEST(LoadToNative3To4, LoadSbyteRGBToRGBADataFourPixelsWithFourthCompOf7F)
{
    ImageLoadContext context;
    constexpr uint8_t kPixelCount = 4;
    std::vector<int8_t> rgbInput(kPixelCount * 3);
    std::vector<int8_t> rgbaOutput(kPixelCount * 4, 0);
    size_t index = 0;
    for (auto &inputComponent : rgbInput)
    {
        inputComponent = ++index;
    }
    constexpr uint8_t kFourthValue = 0x01;

    LoadToNative3To4<int8_t, kFourthValue>(context, 4, 1, 1,
                                           reinterpret_cast<uint8_t *>(rgbInput.data()), 3, 3,
                                           reinterpret_cast<uint8_t *>(rgbaOutput.data()), 4, 4);

    for (index = 0; index < kPixelCount; index++)
    {
        EXPECT_TRUE(rgbaOutput[index * 4] == rgbInput[index * 3] &&
                    rgbaOutput[index * 4 + 1] == rgbInput[index * 3 + 1] &&
                    rgbaOutput[index * 4 + 2] == rgbInput[index * 3 + 2] &&
                    rgbaOutput[index * 4 + 3] == static_cast<int8_t>(kFourthValue))
            << "Mismatch at pixel " << index;
    }
}

// Tests the sbyte (0x01) RGB to RGBA loading function for 4 RGB pixels, which should be read
// together.
TEST(LoadToNative3To4, LoadSbyteRGBToRGBADataFourPixelsWithFourthCompOf01)
{
    ImageLoadContext context;
    constexpr uint8_t kPixelCount = 4;
    std::vector<int8_t> rgbInput(kPixelCount * 3);
    std::vector<int8_t> rgbaOutput(kPixelCount * 4, 0);
    size_t index = 0;
    for (auto &inputComponent : rgbInput)
    {
        inputComponent = ++index;
    }
    constexpr uint8_t kFourthValue = 0x01;

    LoadToNative3To4<int8_t, kFourthValue>(context, 4, 1, 1,
                                           reinterpret_cast<uint8_t *>(rgbInput.data()), 3, 3,
                                           reinterpret_cast<uint8_t *>(rgbaOutput.data()), 4, 4);

    for (index = 0; index < kPixelCount; index++)
    {
        EXPECT_TRUE(rgbaOutput[index * 4] == rgbInput[index * 3] &&
                    rgbaOutput[index * 4 + 1] == rgbInput[index * 3 + 1] &&
                    rgbaOutput[index * 4 + 2] == rgbInput[index * 3 + 2] &&
                    rgbaOutput[index * 4 + 3] == static_cast<int8_t>(kFourthValue))
            << "Mismatch at pixel " << index;
    }
}

// Tests the byte RGB to RGBA loading function when the width is 4-byte aligned. This loading
// function can copy 4 bytes at a time in a row.
TEST(LoadToNative3To4, LoadByteRGBToRGBADataAlignedWidth)
{
    ImageLoadContext context;
    size_t alignedTestWidths[] = {4, 20, 128, 1000, 4096};
    for (auto &width : alignedTestWidths)
    {
        ASSERT(width % 4 == 0);
        TestLoadByteRGBToRGBAForAllCases(context, width, width, 3, 1, 0, 0, 1);
    }
}

// Tests the byte RGB to RGBA loading function when the width is not 4-byte aligned, which will
// cause the loading function to copy some bytes in the beginning and end of some rows individually.
TEST(LoadToNative3To4, LoadByteRGBToRGBADataUnalignedWidth)
{
    ImageLoadContext context;
    size_t unalignedTestWidths[] = {5, 22, 127, 1022, 4097};
    for (auto &width : unalignedTestWidths)
    {
        ASSERT(width % 4 != 0);
        TestLoadByteRGBToRGBAForAllCases(context, width, width, 3, 1, 0, 0, 1);
    }
}

// Tests the byte RGB to RGBA loading function when there is depth.
TEST(LoadToNative3To4, LoadByteRGBToRGBADataWithDepth)
{
    ImageLoadContext context;
    size_t unalignedTestDepths[] = {3};
    for (auto &depth : unalignedTestDepths)
    {
        TestLoadByteRGBToRGBAForAllCases(context, depth, 3, 3, depth, 0, 0, 1);
    }
}

// Tests the byte RGB to RGBA loading function when the width is less than 4 bytes. Therefore the
// loading function will copy data one byte at a time.
TEST(LoadToNative3To4, LoadByteRGBToRGBADataWidthLessThanUint32)
{
    ImageLoadContext context;
    size_t smallTestWidths[] = {1, 2, 3};
    for (auto &width : smallTestWidths)
    {
        TestLoadByteRGBToRGBAForAllCases(context, width, width, 3, 1, 0, 0, 1);
    }
}

// Tests the byte RGB to RGBA loading function when when the width is 4-byte-aligned and the input
// address has an offset.
TEST(LoadToNative3To4, LoadByteRGBToRGBAWithAlignedWidthAndInputAddressOffset)
{
    ImageLoadContext context;
    size_t inputOffsetList[] = {1, 2, 3};
    for (auto &inputOffset : inputOffsetList)
    {
        TestLoadByteRGBToRGBAForAllCases(context, inputOffset, 8, 8, 1, inputOffset, 0, 1);
    }
}

// Tests the byte RGB to RGBA loading function when when the width is not 4-byte-aligned and the
// input address has an offset.
TEST(LoadToNative3To4, LoadByteRGBToRGBAWithUnalignedWidthAndInputAddressOffset)
{
    ImageLoadContext context;
    size_t inputOffsetList[] = {1, 2, 3};
    for (auto &inputOffset : inputOffsetList)
    {
        TestLoadByteRGBToRGBAForAllCases(context, inputOffset, 7, 7, 1, inputOffset, 0, 1);
    }
}

// Tests the byte RGB to RGBA loading function when the width is 4-byte-aligned and the output
// address has an offset.
TEST(LoadToNative3To4, LoadByteRGBToRGBAWithAlignedWidthAndOutputAddressOffset)
{
    ImageLoadContext context;
    size_t outputOffsetList[] = {1, 2, 3};
    for (auto &outputOffset : outputOffsetList)
    {
        TestLoadByteRGBToRGBAForAllCases(context, outputOffset, 8, 8, 1, 0, outputOffset, 1);
    }
}

// Tests the byte RGB to RGBA loading function when the width is not 4-byte-aligned and the output
// address has an offset.
TEST(LoadToNative3To4, LoadByteRGBToRGBAWithUnalignedWidthAndOutputAddressOffset)
{
    ImageLoadContext context;
    size_t outputOffsetList[] = {1, 2, 3};
    for (auto &outputOffset : outputOffsetList)
    {
        TestLoadByteRGBToRGBAForAllCases(context, outputOffset, 7, 7, 1, 0, outputOffset, 1);
    }
}

// Tests the byte RGB to RGBA loading function when the width is 4-byte-aligned and the input row
// alignment is 4.
TEST(LoadToNative3To4, LoadByteRGBToRGBAWithAlignedWidthAndAlignment4)
{
    ImageLoadContext context;
    size_t inputRowAlignmentList[] = {4};
    for (auto &alignment : inputRowAlignmentList)
    {
        TestLoadByteRGBToRGBAForAllCases(context, alignment, 4, 4, 1, 0, 0, alignment);
    }
}

// Tests the byte RGB to RGBA loading function when the width is not 4-byte-aligned and the input
// row alignment is 4.
TEST(LoadToNative3To4, LoadByteRGBToRGBAWithUnalignedWidthAndAlignment4)
{
    ImageLoadContext context;
    size_t inputRowAlignmentList[] = {4};
    for (auto &alignment : inputRowAlignmentList)
    {
        TestLoadByteRGBToRGBAForAllCases(context, alignment, 5, 5, 1, 0, 0, alignment);
    }
}
}  // namespace