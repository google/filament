// Copyright 2023-2025 The Khronos Group Inc.
// Copyright 2023-2025 Valve Corporation
// Copyright 2023-2025 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>
#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 512
#include <magic_enum.hpp>
#include <magic_enum_flags.hpp>
#include <vulkan/utility/vk_format_utils.h>

#include <string_view>

// Given the string_view of a VkFormat, find the location of the letter that corresponds with a component
// EG. find_component("R8G8B8", 'G') would return 2
// returns std::string::npos if it is not found.
size_t find_component(std::string_view str, char letter) {
    size_t loc = str.find(letter, 10);
    while (loc != std::string_view::npos) {
        if (loc < str.length() - 1 && str[loc] == letter && (std::isdigit(str[loc - 1]) || str[loc - 1] == '_') &&
            std::isdigit(str[loc + 1])) {
            break;
        }
        loc = str.find(letter, loc + 1);
    }

    return loc;
}

// Given the string_view of a VkFormat, find the size of the letter that corresponds with a component
// EG. find_component_size("R16G16B16", 3) would return 16
// Returns 0 if the component can't be found
size_t find_component_size(std::string_view str, char letter) {
    size_t loc = find_component(str, letter);
    if (loc == std::string_view::npos) return 0;
    if (loc + 1 >= str.length()) return 0;
    return static_cast<size_t>(std::stoi(std::string(str.substr(loc + 1)), nullptr));
}

size_t find_component_count(std::string_view str) {
    size_t comp_count = 0;
    if (find_component(str, 'R') != std::string_view::npos) comp_count++;
    if (find_component(str, 'G') != std::string_view::npos) comp_count++;
    if (find_component(str, 'B') != std::string_view::npos) comp_count++;
    if (find_component(str, 'A') != std::string_view::npos) comp_count++;
    if (find_component(str, 'D') != std::string_view::npos) comp_count++;
    if (find_component(str, 'S') != std::string_view::npos) comp_count++;
    return comp_count;
}

TEST(format_utils, vkuFormatIsSFLOAT) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        // special case depth + stencil formats
        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
            EXPECT_FALSE(vkuFormatIsSFLOAT(format));
            continue;
        }

        if (std::string::npos != format_str.find("_SFLOAT")) {
            EXPECT_TRUE(vkuFormatIsSFLOAT(format));
        } else {
            EXPECT_FALSE(vkuFormatIsSFLOAT(format));
        }
    }
}

TEST(format_utils, vkuFormatIsSINT) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_SINT")) {
            EXPECT_TRUE(vkuFormatIsSINT(format));
        } else {
            EXPECT_FALSE(vkuFormatIsSINT(format));
        }
    }
}
TEST(format_utils, vkuFormatIsSNORM) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_SNORM")) {
            EXPECT_TRUE(vkuFormatIsSNORM(format));
        } else {
            EXPECT_FALSE(vkuFormatIsSNORM(format));
        }
    }
}
TEST(format_utils, vkuFormatIsSRGB) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_SRGB")) {
            EXPECT_TRUE(vkuFormatIsSRGB(format));
        } else {
            EXPECT_FALSE(vkuFormatIsSRGB(format));
        }
    }
}
TEST(format_utils, vkuFormatIsUFLOAT) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_UFLOAT")) {
            EXPECT_TRUE(vkuFormatIsUFLOAT(format));
        } else {
            EXPECT_FALSE(vkuFormatIsUFLOAT(format));
        }
    }
}
TEST(format_utils, vkuFormatIsUINT) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        // special case depth + stencil formats
        if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT ||
            format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
            EXPECT_FALSE(vkuFormatIsUINT(format));
            continue;
        }
        if (std::string::npos != format_str.find("_UINT")) {
            EXPECT_TRUE(vkuFormatIsUINT(format));
        } else {
            EXPECT_FALSE(vkuFormatIsUINT(format));
        }
    }
}
TEST(format_utils, vkuFormatIsUNORM) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
            EXPECT_FALSE(vkuFormatIsUNORM(format));
            continue;
        }
        if (std::string::npos != format_str.find("_UNORM")) {
            EXPECT_TRUE(vkuFormatIsUNORM(format));
        } else {
            EXPECT_FALSE(vkuFormatIsUNORM(format));
        }
    }
}
TEST(format_utils, vkuFormatIsUSCALED) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_USCALED")) {
            EXPECT_TRUE(vkuFormatIsUSCALED(format));
        } else {
            EXPECT_FALSE(vkuFormatIsUSCALED(format));
        }
    }
}
TEST(format_utils, vkuFormatIsCompressed_ASTC_HDR) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        // contains ASTC and SFLOAT in the enum
        if (std::string::npos != format_str.find("_ASTC_") && std::string::npos != format_str.find("_SFLOAT_BLOCK")) {
            EXPECT_TRUE(vkuFormatIsCompressed_ASTC_HDR(format));
        } else {
            EXPECT_FALSE(vkuFormatIsCompressed_ASTC_HDR(format));
        }
    }
}
TEST(format_utils, vkuFormatIsCompressed_ASTC_LDR) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        // contains ASTC and does not contain SFLOAT in the enum
        if (std::string::npos != format_str.find("_ASTC_") && std::string::npos == format_str.find("_SFLOAT_BLOCK")) {
            EXPECT_TRUE(vkuFormatIsCompressed_ASTC_LDR(format));
        } else {
            EXPECT_FALSE(vkuFormatIsCompressed_ASTC_LDR(format));
        }
    }
}
TEST(format_utils, vkuFormatIsCompressed_BC) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_BC1") || std::string::npos != format_str.find("_BC2") ||
            std::string::npos != format_str.find("_BC3") || std::string::npos != format_str.find("_BC4") ||
            std::string::npos != format_str.find("_BC5") || std::string::npos != format_str.find("_BC6") ||
            std::string::npos != format_str.find("_BC7")) {
            EXPECT_TRUE(vkuFormatIsCompressed_BC(format));
        } else {
            EXPECT_FALSE(vkuFormatIsCompressed_BC(format));
        }
    }
}
TEST(format_utils, vkuFormatIsCompressed_EAC) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_EAC_")) {
            EXPECT_TRUE(vkuFormatIsCompressed_EAC(format));
        } else {
            EXPECT_FALSE(vkuFormatIsCompressed_EAC(format));
        }
    }
}
TEST(format_utils, vkuFormatIsCompressed_ETC2) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_ETC2_")) {
            EXPECT_TRUE(vkuFormatIsCompressed_ETC2(format));
        } else {
            EXPECT_FALSE(vkuFormatIsCompressed_ETC2(format));
        }
    }
}
TEST(format_utils, vkuFormatIsCompressed_PVRTC) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_PVRTC_")) {
            EXPECT_TRUE(vkuFormatIsCompressed_PVRTC(format));
        } else {
            EXPECT_FALSE(vkuFormatIsCompressed_PVRTC(format));
        }
    }
}

TEST(format_utils, vkuFormatIsCompressed) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        // Since the contents of FormatIsCompressed is generated, there is no easy way to check a format based on the string.
        // Instead, this function will have to be
        if (std::string::npos != format_str.find("_BLOCK")) {
            EXPECT_TRUE(vkuFormatIsCompressed(format));
        } else {
            EXPECT_FALSE(vkuFormatIsCompressed(format));
        }
    }
}

TEST(format_utils, vkuFormatIsDepthOrStencil) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_D16") || std::string::npos != format_str.find("_D24") ||
            std::string::npos != format_str.find("_D32") || std::string::npos != format_str.find("_S8")) {
            EXPECT_TRUE(vkuFormatIsDepthOrStencil(format));
        } else {
            EXPECT_FALSE(vkuFormatIsDepthOrStencil(format));
        }
    }
}
TEST(format_utils, vkuFormatIsDepthAndStencil) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if ((std::string::npos != format_str.find("_D16") || std::string::npos != format_str.find("_D24") ||
             std::string::npos != format_str.find("_D32")) &&
            std::string::npos != format_str.find("_S8")) {
            EXPECT_TRUE(vkuFormatIsDepthAndStencil(format));
        } else {
            EXPECT_FALSE(vkuFormatIsDepthAndStencil(format));
        }
    }
}
TEST(format_utils, vkuFormatIsDepthOnly) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        // enum contains D16, D24, or D32 but does not contain _S8
        if ((std::string::npos != format_str.find("_D16") || std::string::npos != format_str.find("_D24") ||
             std::string::npos != format_str.find("_D32")) &&
            std::string::npos == format_str.find("_S8")) {
            EXPECT_TRUE(vkuFormatIsDepthOnly(format));
        } else {
            EXPECT_FALSE(vkuFormatIsDepthOnly(format));
        }
    }
}
TEST(format_utils, vkuFormatIsStencilOnly) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        // enum contains _S8 but does not contain D16, D24, or D32
        if (std::string::npos == format_str.find("_D16") && std::string::npos == format_str.find("_D24") &&
            std::string::npos == format_str.find("_D32") && std::string::npos != format_str.find("_S8")) {
            EXPECT_TRUE(vkuFormatIsStencilOnly(format));
        } else {
            EXPECT_FALSE(vkuFormatIsStencilOnly(format));
        }
    }
}
TEST(format_utils, vkuFormatDepthSize) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_D16")) {
            EXPECT_EQ(vkuFormatDepthSize(format), 16u);
        } else if (std::string::npos != format_str.find("_D24")) {
            EXPECT_EQ(vkuFormatDepthSize(format), 24u);
        } else if (std::string::npos != format_str.find("_D32")) {
            EXPECT_EQ(vkuFormatDepthSize(format), 32u);
        } else {
            EXPECT_EQ(vkuFormatDepthSize(format), 0u);
        }
    }
}
TEST(format_utils, vkuFormatStencilSize) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_S8")) {
            EXPECT_EQ(vkuFormatStencilSize(format), 8u);
        } else {
            EXPECT_EQ(vkuFormatStencilSize(format), 0u);
        }
    }
}
TEST(format_utils, vkuFormatDepthNumericalType) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("D16_UNORM") || std::string::npos != format_str.find("D24_UNORM") ||
            std::string::npos != format_str.find("D32_UNORM")) {
            EXPECT_EQ(vkuFormatDepthNumericalType(format), VKU_FORMAT_NUMERICAL_TYPE_UNORM);
        } else if (std::string::npos != format_str.find("D32_SFLOAT")) {
            EXPECT_EQ(vkuFormatDepthNumericalType(format), VKU_FORMAT_NUMERICAL_TYPE_SFLOAT);
        } else {
            EXPECT_EQ(vkuFormatDepthNumericalType(format), VKU_FORMAT_NUMERICAL_TYPE_NONE);
        }
    }
}
TEST(format_utils, vkuFormatStencilNumericalType) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("S8_UINT")) {
            EXPECT_EQ(vkuFormatStencilNumericalType(format), VKU_FORMAT_NUMERICAL_TYPE_UINT);
        } else {
            EXPECT_EQ(vkuFormatStencilNumericalType(format), VKU_FORMAT_NUMERICAL_TYPE_NONE);
        }
    }
}
TEST(format_utils, vkuFormatIsPacked) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_PACK8") || std::string::npos != format_str.find("_PACK16") ||
            std::string::npos != format_str.find("_PACK32")) {
            EXPECT_TRUE(vkuFormatIsPacked(format));
        } else {
            EXPECT_FALSE(vkuFormatIsPacked(format));
        }
    }
}
TEST(format_utils, vkuFormatRequiresYcbcrConversion) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_444_") || std::string::npos != format_str.find("_422_") ||
            std::string::npos != format_str.find("_420_")) {
            EXPECT_TRUE(vkuFormatRequiresYcbcrConversion(format));
        } else {
            EXPECT_FALSE(vkuFormatRequiresYcbcrConversion(format));
        }
    }
}
TEST(format_utils, vkuFormatIsXChromaSubsampled) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos == format_str.find("_444_") &&
            (std::string::npos != format_str.find("_422_") || std::string::npos != format_str.find("_420_"))) {
            EXPECT_TRUE(vkuFormatIsXChromaSubsampled(format));
        } else {
            EXPECT_FALSE(vkuFormatIsXChromaSubsampled(format));
        }
    }
}
TEST(format_utils, vkuFormatIsYChromaSubsampled) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_420_")) {
            EXPECT_TRUE(vkuFormatIsYChromaSubsampled(format));
        } else {
            EXPECT_FALSE(vkuFormatIsYChromaSubsampled(format));
        }
    }
}
TEST(format_utils, vkuFormatIsSinglePlane_422) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("_422_")) {
            EXPECT_TRUE(vkuFormatIsSinglePlane_422(format));
        } else {
            EXPECT_FALSE(vkuFormatIsSinglePlane_422(format));
        }
    }
}
TEST(format_utils, vkuFormatPlaneCount) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("2PLANE")) {
            EXPECT_EQ(vkuFormatPlaneCount(format), 2u);
        } else if (std::string::npos != format_str.find("3PLANE")) {
            EXPECT_EQ(vkuFormatPlaneCount(format), 3u);
        } else {
            EXPECT_EQ(vkuFormatPlaneCount(format), 1u);
        }
    }
}
TEST(format_utils, vkuFormatIsMultiplane) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("2PLANE") || std::string::npos != format_str.find("3PLANE")) {
            EXPECT_TRUE(vkuFormatIsMultiplane(format));
        } else {
            EXPECT_FALSE(vkuFormatIsMultiplane(format));
        }
    }
}
TEST(format_utils, vkuFindMultiplaneCompatibleFormat) {
    EXPECT_EQ(vkuFindMultiplaneCompatibleFormat(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_0_BIT),
              VK_FORMAT_R10X6_UNORM_PACK16);
    EXPECT_EQ(vkuFindMultiplaneCompatibleFormat(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_1_BIT),
              VK_FORMAT_R10X6_UNORM_PACK16);
    EXPECT_EQ(vkuFindMultiplaneCompatibleFormat(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_2_BIT),
              VK_FORMAT_R10X6_UNORM_PACK16);

    EXPECT_EQ(vkuFindMultiplaneCompatibleFormat(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_IMAGE_ASPECT_PLANE_0_BIT),
              VK_FORMAT_R16_UNORM);
    EXPECT_EQ(vkuFindMultiplaneCompatibleFormat(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_IMAGE_ASPECT_PLANE_1_BIT),
              VK_FORMAT_R16G16_UNORM);
    EXPECT_EQ(vkuFindMultiplaneCompatibleFormat(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_IMAGE_ASPECT_PLANE_2_BIT),
              VK_FORMAT_UNDEFINED);
}
TEST(format_utils, vkuFindMultiplaneExtentDivisors) {
    EXPECT_EQ(
        vkuFindMultiplaneExtentDivisors(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_0_BIT).width,
        1u);
    EXPECT_EQ(
        vkuFindMultiplaneExtentDivisors(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_1_BIT).width,
        2u);
    EXPECT_EQ(
        vkuFindMultiplaneExtentDivisors(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_2_BIT).width,
        2u);

    EXPECT_EQ(
        vkuFindMultiplaneExtentDivisors(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_0_BIT).height,
        1u);
    EXPECT_EQ(
        vkuFindMultiplaneExtentDivisors(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_1_BIT).height,
        2u);
    EXPECT_EQ(
        vkuFindMultiplaneExtentDivisors(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, VK_IMAGE_ASPECT_PLANE_2_BIT).height,
        2u);

    EXPECT_EQ(vkuFindMultiplaneExtentDivisors(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_IMAGE_ASPECT_PLANE_0_BIT).width, 1u);
    EXPECT_EQ(vkuFindMultiplaneExtentDivisors(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_IMAGE_ASPECT_PLANE_1_BIT).width, 2u);
    EXPECT_EQ(vkuFindMultiplaneExtentDivisors(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_IMAGE_ASPECT_PLANE_2_BIT).width, 1u);

    EXPECT_EQ(vkuFindMultiplaneExtentDivisors(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_IMAGE_ASPECT_PLANE_0_BIT).height, 1u);
    EXPECT_EQ(vkuFindMultiplaneExtentDivisors(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_IMAGE_ASPECT_PLANE_1_BIT).height, 1u);
    EXPECT_EQ(vkuFindMultiplaneExtentDivisors(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, VK_IMAGE_ASPECT_PLANE_2_BIT).height, 1u);
}

TEST(format_utils, vkuFormatIsDepthStencilWithColorSizeCompatible) {
    EXPECT_FALSE(vkuFormatIsDepthStencilWithColorSizeCompatible(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
                                                                VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT));
    EXPECT_FALSE(
        vkuFormatIsDepthStencilWithColorSizeCompatible(VK_FORMAT_D16_UNORM, VK_FORMAT_R16_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT));
    EXPECT_FALSE(
        vkuFormatIsDepthStencilWithColorSizeCompatible(VK_FORMAT_R8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT));
    EXPECT_FALSE(vkuFormatIsDepthStencilWithColorSizeCompatible(VK_FORMAT_R16_UNORM, VK_FORMAT_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT));

    EXPECT_TRUE(
        vkuFormatIsDepthStencilWithColorSizeCompatible(VK_FORMAT_R16_SFLOAT, VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT));
    EXPECT_TRUE(vkuFormatIsDepthStencilWithColorSizeCompatible(VK_FORMAT_R8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
                                                               VK_IMAGE_ASPECT_STENCIL_BIT));
    EXPECT_TRUE(vkuFormatIsDepthStencilWithColorSizeCompatible(VK_FORMAT_R16_UNORM, VK_FORMAT_D16_UNORM_S8_UINT,
                                                               VK_IMAGE_ASPECT_DEPTH_BIT));
    EXPECT_TRUE(vkuFormatIsDepthStencilWithColorSizeCompatible(VK_FORMAT_R16_UNORM, VK_FORMAT_D16_UNORM_S8_UINT,
                                                               VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
    EXPECT_TRUE(vkuFormatIsDepthStencilWithColorSizeCompatible(VK_FORMAT_R8_UINT, VK_FORMAT_S8_UINT, VK_IMAGE_ASPECT_STENCIL_BIT));
}

TEST(format_utils, vkuFormatComponentCount) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (vkuFormatIsCompressed(format)) {
            // special case compressed formats because they don't typically list their components in the enum itself
            continue;
        }

        EXPECT_EQ(vkuFormatComponentCount(format), find_component_count(format_str));
        if (vkuFormatComponentCount(format) != find_component_count(format_str)) {
            std::cout << "";
        }
    }
}
TEST(format_utils, vkuFormatTexelBlockExtent) {
    constexpr auto formats = magic_enum::enum_values<VkFormat>();
    for (auto format : formats) {
        auto extent = vkuFormatTexelBlockExtent(format);
        if (vkuFormatIsCompressed(format)) {
            EXPECT_GT(extent.width, 1u);
            EXPECT_GT(extent.height, 1u);
            EXPECT_EQ(extent.depth, 1u);
        } else if (format == VK_FORMAT_UNDEFINED) {
            EXPECT_EQ(extent.width, 0u);
            EXPECT_EQ(extent.height, 0u);
            EXPECT_EQ(extent.depth, 0u);
            continue;
        } else {
            EXPECT_EQ(extent.width, 1u);
            EXPECT_EQ(extent.height, 1u);
            EXPECT_EQ(extent.depth, 1u);
        }
    }
    auto extent = vkuFormatTexelBlockExtent(static_cast<VkFormat>(10001));
    EXPECT_EQ(extent.width, 0u);
    EXPECT_EQ(extent.height, 0u);
    EXPECT_EQ(extent.depth, 0u);
}
TEST(format_utils, vkuFormatCompatibilityClass) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (format == VK_FORMAT_UNDEFINED) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_NONE);
            continue;
        }
        if (std::string::npos != format_str.find("D16") && std::string::npos != format_str.find("S8")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_D16S8);
        } else if (std::string::npos != format_str.find("D16") && std::string::npos == format_str.find("S8")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_D16);
        } else if (std::string::npos != format_str.find("D24") && std::string::npos != format_str.find("S8")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_D24S8);
        } else if (std::string::npos != format_str.find("D24") && std::string::npos == format_str.find("S8")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_D24);
        } else if (std::string::npos != format_str.find("D32") && std::string::npos != format_str.find("S8")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_D32S8);
        } else if (std::string::npos != format_str.find("D32") && std::string::npos == format_str.find("S8")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_D32);
        } else if (std::string::npos != format_str.find("S8")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_S8);
        } else if (std::string::npos != format_str.find("_PACK8")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_8BIT);
        } else if (std::string::npos != format_str.find("_PACK16")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_16BIT);
        } else if (std::string::npos != format_str.find("_PACK32")) {
            EXPECT_EQ(vkuFormatCompatibilityClass(format), VKU_FORMAT_COMPATIBILITY_CLASS_32BIT);
        } else if (vkuFormatIsCompressed(format)) {
            // special case compressed formats because they don't typically list their components in the enum itself
            continue;
        } else {
            size_t component_size_combined = find_component_size(format_str, 'R') + find_component_size(format_str, 'G') +
                                             find_component_size(format_str, 'B') + find_component_size(format_str, 'A');
            VKU_FORMAT_COMPATIBILITY_CLASS comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_NONE;
            if (component_size_combined == 8) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_8BIT;
            if (component_size_combined == 16) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_16BIT;
            if (component_size_combined == 24) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_24BIT;
            if (component_size_combined == 32) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_32BIT;
            if (component_size_combined == 48) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_48BIT;
            if (component_size_combined == 64) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_64BIT;
            if (component_size_combined == 96) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_96BIT;
            if (component_size_combined == 128) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_128BIT;
            if (component_size_combined == 192) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_192BIT;
            if (component_size_combined == 256) comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_256BIT;

            if (comp_class == VKU_FORMAT_COMPATIBILITY_CLASS_8BIT && find_component(format_str, 'A') != std::string_view::npos)
                comp_class = VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_ALPHA;

            EXPECT_EQ(vkuFormatCompatibilityClass(format), comp_class);
            if (vkuFormatCompatibilityClass(format) != comp_class) {
                std::cout << "";
            }
        }
    }
}

TEST(format_utils, vkuFormatTexelsPerBlock) {
    EXPECT_EQ(vkuFormatTexelsPerBlock(VK_FORMAT_R64G64_SFLOAT), 1u);
    EXPECT_EQ(vkuFormatTexelsPerBlock(VK_FORMAT_BC6H_UFLOAT_BLOCK), 16u);
    EXPECT_EQ(vkuFormatTexelsPerBlock(VK_FORMAT_ASTC_5x4_SRGB_BLOCK), 20u);
    EXPECT_EQ(vkuFormatTexelsPerBlock(VK_FORMAT_ASTC_5x5_SRGB_BLOCK), 25u);
    EXPECT_EQ(vkuFormatTexelsPerBlock(VK_FORMAT_ASTC_12x12_SRGB_BLOCK), 144u);
    EXPECT_EQ(vkuFormatTexelsPerBlock(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM), 1u);
    EXPECT_EQ(vkuFormatTexelsPerBlock(VK_FORMAT_D32_SFLOAT), 1u);
    EXPECT_EQ(vkuFormatTexelsPerBlock(VK_FORMAT_D32_SFLOAT_S8_UINT), 1u);
    EXPECT_EQ(vkuFormatTexelsPerBlock(VK_FORMAT_S8_UINT), 1u);
}

TEST(format_utils, vkuFormatTexelBlockSize) {
    EXPECT_EQ(vkuFormatTexelBlockSize(VK_FORMAT_R64G64_SFLOAT), 16u);
    EXPECT_EQ(vkuFormatTexelBlockSize(VK_FORMAT_ASTC_5x4_SRGB_BLOCK), 16u);
    EXPECT_EQ(vkuFormatTexelBlockSize(VK_FORMAT_ASTC_5x5_SRGB_BLOCK), 16u);
    EXPECT_EQ(vkuFormatTexelBlockSize(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM), 6u);
    EXPECT_EQ(vkuFormatTexelBlockSize(VK_FORMAT_D32_SFLOAT), 4u);
    EXPECT_EQ(vkuFormatTexelBlockSize(VK_FORMAT_D32_SFLOAT_S8_UINT), 5u);
    EXPECT_EQ(vkuFormatTexelBlockSize(VK_FORMAT_S8_UINT), 1u);
}

TEST(format_utils, vkuFormatIs64bit) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (std::string::npos != format_str.find("R64")) {
            EXPECT_TRUE(vkuFormatIs64bit(format));
        } else {
            EXPECT_FALSE(vkuFormatIs64bit(format));
        }
    }
}
TEST(format_utils, vkuFormatHasComponentSize) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (vkuFormatIsCompressed(format)) {
            // special case compressed formats because they don't typically list their components in the enum itself
            continue;
        }
        for (uint32_t i = 1; i < 64; i++) {
            bool has_component_size = false;
            has_component_size |= find_component_size(format_str, 'R') == i;
            has_component_size |= find_component_size(format_str, 'G') == i;
            has_component_size |= find_component_size(format_str, 'B') == i;
            has_component_size |= find_component_size(format_str, 'A') == i;
            has_component_size |= find_component_size(format_str, 'D') == i;
            has_component_size |= find_component_size(format_str, 'S') == i;
            EXPECT_EQ(vkuFormatHasComponentSize(format, i), has_component_size);
        }
    }
}

void check_for_letter(char letter, bool (*func)(VkFormat)) {
    for (auto [format, format_str] : magic_enum::enum_entries<VkFormat>()) {
        if (vkuFormatIsCompressed(format)) {
            // special case compressed formats because they don't typically list their components in the enum itself
            continue;
        }
        auto loc = find_component(format_str, letter);
        if (std::string::npos != loc) {
            EXPECT_TRUE(func(format));
        } else {
            EXPECT_FALSE(func(format));
        }
    }
}
TEST(format_utils, vkuFormatHasRed) { check_for_letter('R', vkuFormatHasRed); }
TEST(format_utils, vkuFormatHasGreen) { check_for_letter('G', vkuFormatHasGreen); }
TEST(format_utils, vkuFormatHasBlue) { check_for_letter('B', vkuFormatHasBlue); }
TEST(format_utils, vkuFormatHasAlpha) { check_for_letter('A', vkuFormatHasAlpha); }

TEST(format_utils, vkuFormatIsUndefined) {
    constexpr auto formats = magic_enum::enum_values<VkFormat>();
    for (auto format : formats) {
        if (format == VK_FORMAT_UNDEFINED) {
            EXPECT_TRUE(vkuFormatIsUndefined(format));
        } else {
            EXPECT_FALSE(vkuFormatIsUndefined(format));
        }
    }
}
TEST(format_utils, vkuFormatIsBlockedImage) {
    constexpr auto formats = magic_enum::enum_values<VkFormat>();
    for (auto format : formats) {
        if (vkuFormatIsCompressed(format) || vkuFormatIsSinglePlane_422(format)) {
            EXPECT_TRUE(vkuFormatIsBlockedImage(format));
        } else {
            EXPECT_FALSE(vkuFormatIsBlockedImage(format));
        }
    }
}
TEST(format_utils, vkuFormatIsColor) {
    constexpr auto formats = magic_enum::enum_values<VkFormat>();
    for (auto format : formats) {
        if (!(vkuFormatIsUndefined(format) || vkuFormatIsDepthOrStencil(format) || vkuFormatIsMultiplane(format))) {
            EXPECT_TRUE(vkuFormatIsColor(format));
        } else {
            EXPECT_FALSE(vkuFormatIsColor(format));
        }
    }
}
template <>
struct magic_enum::customize::enum_range<VkImageAspectFlagBits> {
    static constexpr bool is_flags = true;
};

TEST(format_utils, vkuGetPlaneIndex) {
    for (auto [aspect_flag, aspect_flag_str] : magic_enum::enum_entries<VkImageAspectFlagBits>()) {
        if (std::string::npos != aspect_flag_str.find("_ASPECT_PLANE_0")) {
            EXPECT_EQ(vkuGetPlaneIndex(aspect_flag), 0u);
        } else if (std::string::npos != aspect_flag_str.find("_ASPECT_PLANE_1")) {
            EXPECT_EQ(vkuGetPlaneIndex(aspect_flag), 1u);

        } else if (std::string::npos != aspect_flag_str.find("_ASPECT_PLANE_2")) {
            EXPECT_EQ(vkuGetPlaneIndex(aspect_flag), 2u);
        } else {
            EXPECT_EQ(vkuGetPlaneIndex(aspect_flag), VKU_FORMAT_INVALID_INDEX);
        }
    }
}
