// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See format_utils_generator.py for modifications
// Copyright 2023-2025 The Khronos Group Inc.
// Copyright 2023-2025 Valve Corporation
// Copyright 2023-2025 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

// clang-format off

#ifdef __cplusplus
extern "C" {
#endif

#include <vulkan/vulkan.h>

#include <stdbool.h>

#define VKU_FORMAT_INVALID_INDEX 0xFFFFFFFF
#define VKU_FORMAT_MAX_PLANES 3
#define VKU_FORMAT_MAX_COMPONENTS 4

enum VKU_FORMAT_NUMERICAL_TYPE {
    VKU_FORMAT_NUMERICAL_TYPE_NONE = 0,
    VKU_FORMAT_NUMERICAL_TYPE_SFIXED5,
    VKU_FORMAT_NUMERICAL_TYPE_SFLOAT,
    VKU_FORMAT_NUMERICAL_TYPE_SINT,
    VKU_FORMAT_NUMERICAL_TYPE_SNORM,
    VKU_FORMAT_NUMERICAL_TYPE_SRGB,
    VKU_FORMAT_NUMERICAL_TYPE_SSCALED,
    VKU_FORMAT_NUMERICAL_TYPE_UFLOAT,
    VKU_FORMAT_NUMERICAL_TYPE_UINT,
    VKU_FORMAT_NUMERICAL_TYPE_UNORM,
    VKU_FORMAT_NUMERICAL_TYPE_USCALED,
};

enum VKU_FORMAT_COMPATIBILITY_CLASS {
    VKU_FORMAT_COMPATIBILITY_CLASS_NONE = 0,
    VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_2PLANE_420,
    VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_2PLANE_422,
    VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_2PLANE_444,
    VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_3PLANE_420,
    VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_3PLANE_422,
    VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_3PLANE_444,
    VKU_FORMAT_COMPATIBILITY_CLASS_128BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_2PLANE_420,
    VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_2PLANE_422,
    VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_2PLANE_444,
    VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_3PLANE_420,
    VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_3PLANE_422,
    VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_3PLANE_444,
    VKU_FORMAT_COMPATIBILITY_CLASS_16BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_2PLANE_420,
    VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_2PLANE_422,
    VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_2PLANE_444,
    VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_3PLANE_420,
    VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_3PLANE_422,
    VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_3PLANE_444,
    VKU_FORMAT_COMPATIBILITY_CLASS_192BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_24BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_256BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_32BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_32BIT_B8G8R8G8,
    VKU_FORMAT_COMPATIBILITY_CLASS_32BIT_G8B8G8R8,
    VKU_FORMAT_COMPATIBILITY_CLASS_48BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_64BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_B10G10R10G10,
    VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_B12G12R12G12,
    VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_B16G16R16G16,
    VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_G10B10G10R10,
    VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_G12B12G12R12,
    VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_G16B16G16R16,
    VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_R10G10B10A10,
    VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_R12G12B12A12,
    VKU_FORMAT_COMPATIBILITY_CLASS_8BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_2PLANE_420,
    VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_2PLANE_422,
    VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_2PLANE_444,
    VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_3PLANE_420,
    VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_3PLANE_422,
    VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_3PLANE_444,
    VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_ALPHA,
    VKU_FORMAT_COMPATIBILITY_CLASS_96BIT,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X10,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X5,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X6,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X8,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_12X10,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_12X12,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_4X4,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_5X4,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_5X5,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_6X5,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_6X6,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X5,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X6,
    VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X8,
    VKU_FORMAT_COMPATIBILITY_CLASS_BC1_RGB,
    VKU_FORMAT_COMPATIBILITY_CLASS_BC1_RGBA,
    VKU_FORMAT_COMPATIBILITY_CLASS_BC2,
    VKU_FORMAT_COMPATIBILITY_CLASS_BC3,
    VKU_FORMAT_COMPATIBILITY_CLASS_BC4,
    VKU_FORMAT_COMPATIBILITY_CLASS_BC5,
    VKU_FORMAT_COMPATIBILITY_CLASS_BC6H,
    VKU_FORMAT_COMPATIBILITY_CLASS_BC7,
    VKU_FORMAT_COMPATIBILITY_CLASS_D16,
    VKU_FORMAT_COMPATIBILITY_CLASS_D16S8,
    VKU_FORMAT_COMPATIBILITY_CLASS_D24,
    VKU_FORMAT_COMPATIBILITY_CLASS_D24S8,
    VKU_FORMAT_COMPATIBILITY_CLASS_D32,
    VKU_FORMAT_COMPATIBILITY_CLASS_D32S8,
    VKU_FORMAT_COMPATIBILITY_CLASS_EAC_R,
    VKU_FORMAT_COMPATIBILITY_CLASS_EAC_RG,
    VKU_FORMAT_COMPATIBILITY_CLASS_ETC2_EAC_RGBA,
    VKU_FORMAT_COMPATIBILITY_CLASS_ETC2_RGB,
    VKU_FORMAT_COMPATIBILITY_CLASS_ETC2_RGBA,
    VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC1_2BPP,
    VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC1_4BPP,
    VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC2_2BPP,
    VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC2_4BPP,
    VKU_FORMAT_COMPATIBILITY_CLASS_S8,
};
// Return the plane index of a given VkImageAspectFlagBits.
//     VK_IMAGE_ASPECT_PLANE_0_BIT -> 0
//     VK_IMAGE_ASPECT_PLANE_1_BIT -> 1
//     VK_IMAGE_ASPECT_PLANE_2_BIT -> 2
//     <any other value> -> VKU_FORMAT_INVALID_INDEX
inline uint32_t vkuGetPlaneIndex(VkImageAspectFlagBits aspect);

// Returns whether a VkFormat is of the numerical format SFIXED5
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsSFIXED5(VkFormat format);

// Returns whether a VkFormat is of the numerical format SFLOAT
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsSFLOAT(VkFormat format);

// Returns whether a VkFormat is of the numerical format SINT
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsSINT(VkFormat format);

// Returns whether a VkFormat is of the numerical format SNORM
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsSNORM(VkFormat format);

// Returns whether a VkFormat is of the numerical format SRGB
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsSRGB(VkFormat format);

// Returns whether a VkFormat is of the numerical format SSCALED
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsSSCALED(VkFormat format);

// Returns whether a VkFormat is of the numerical format UFLOAT
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsUFLOAT(VkFormat format);

// Returns whether a VkFormat is of the numerical format UINT
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsUINT(VkFormat format);

// Returns whether a VkFormat is of the numerical format UNORM
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsUNORM(VkFormat format);

// Returns whether a VkFormat is of the numerical format USCALED
// Format must only contain one numerical format, so formats like D16_UNORM_S8_UINT always return false
inline bool vkuFormatIsUSCALED(VkFormat format);

// Returns whether the type of a VkFormat is a OpTypeInt (SPIR-V) from "Interpretation of Numeric Format" table
inline bool vkuFormatIsSampledInt(VkFormat format);

// Returns whether the type of a VkFormat is a OpTypeFloat (SPIR-V) from "Interpretation of Numeric Format" table
inline bool vkuFormatIsSampledFloat(VkFormat format);

// Returns whether a VkFormat is a compressed format of type ASTC_HDR
inline bool vkuFormatIsCompressed_ASTC_HDR(VkFormat format);

// Returns whether a VkFormat is a compressed format of type ASTC_LDR
inline bool vkuFormatIsCompressed_ASTC_LDR(VkFormat format);

// Returns whether a VkFormat is a compressed format of type BC
inline bool vkuFormatIsCompressed_BC(VkFormat format);

// Returns whether a VkFormat is a compressed format of type EAC
inline bool vkuFormatIsCompressed_EAC(VkFormat format);

// Returns whether a VkFormat is a compressed format of type ETC2
inline bool vkuFormatIsCompressed_ETC2(VkFormat format);

// Returns whether a VkFormat is a compressed format of type PVRTC
inline bool vkuFormatIsCompressed_PVRTC(VkFormat format);

// Returns whether a VkFormat is of any compressed format type
inline bool vkuFormatIsCompressed(VkFormat format);

// Returns whether a VkFormat is either a depth or stencil format
inline bool vkuFormatIsDepthOrStencil(VkFormat format);

// Returns whether a VkFormat is a depth and stencil format
inline bool vkuFormatIsDepthAndStencil(VkFormat format);

// Returns whether a VkFormat is a depth only format
inline bool vkuFormatIsDepthOnly(VkFormat format);

// Returns whether a VkFormat is a stencil only format
inline bool vkuFormatIsStencilOnly(VkFormat format);

// Returns whether a VkFormat has a depth component
inline bool vkuFormatHasDepth(VkFormat format) { return (vkuFormatIsDepthOnly(format) || vkuFormatIsDepthAndStencil(format)); }

// Returns whether a VkFormat has a stencil component
inline bool vkuFormatHasStencil(VkFormat format) { return (vkuFormatIsStencilOnly(format) || vkuFormatIsDepthAndStencil(format)); }

// Returns the size of the depth component in bits if it has one. Otherwise it returns 0
inline uint32_t vkuFormatDepthSize(VkFormat format);

// Returns the size of the stencil component in bits if it has one. Otherwise it returns 0
inline uint32_t vkuFormatStencilSize(VkFormat format);

// Returns the numerical type of the depth component if it has one.  Otherwise it returns VKU_FORMAT_NUMERICAL_TYPE_NONE
inline enum VKU_FORMAT_NUMERICAL_TYPE vkuFormatDepthNumericalType(VkFormat format);

// Returns the numerical type of the stencil component if it has one.  Otherwise it returns VKU_FORMAT_NUMERICAL_TYPE_NONE
inline enum VKU_FORMAT_NUMERICAL_TYPE vkuFormatStencilNumericalType(VkFormat format);

// Returns whether a VkFormat is packed
inline bool vkuFormatIsPacked(VkFormat format);

// Returns whether a VkFormat is YCbCr
// This corresponds to formats with _444, _422, or _420 in their name
inline bool vkuFormatRequiresYcbcrConversion(VkFormat format);

// Returns whether a VkFormat is XChromaSubsampled
// This corresponds to formats with _422 or 420 in their name
inline bool vkuFormatIsXChromaSubsampled(VkFormat format);

// Returns whether a VkFormat is YChromaSubsampled
// This corresponds to formats with _420 in their name
inline bool vkuFormatIsYChromaSubsampled(VkFormat format);

// Returns whether a VkFormat is Multiplane
// Single-plane "_422" formats are treated as 2x1 compressed (for copies)
inline bool vkuFormatIsSinglePlane_422(VkFormat format);

// Returns number of planes in format (which is 1 by default)
inline uint32_t vkuFormatPlaneCount(VkFormat format);

// Returns whether a VkFormat is multiplane
// Note - Formats like VK_FORMAT_G8B8G8R8_422_UNORM are NOT multi-planar, they require a
//        VkSamplerYcbcrConversion and you should use vkuFormatRequiresYcbcrConversion instead
inline bool vkuFormatIsMultiplane(VkFormat format) { return ((vkuFormatPlaneCount(format)) > 1u); }

// Returns a VkFormat that is compatible with a given plane of a multiplane format
// Will return VK_FORMAT_UNDEFINED if given a plane aspect that doesn't exist for the format
inline VkFormat vkuFindMultiplaneCompatibleFormat(VkFormat mp_fmt, VkImageAspectFlagBits plane_aspect);

// Returns the extent divisors of a multiplane format given a plane
// Will return {1, 1} if given a plane aspect that doesn't exist for the VkFormat
inline VkExtent2D vkuFindMultiplaneExtentDivisors(VkFormat mp_fmt, VkImageAspectFlagBits plane_aspect);

// From table in spec vkspec.html#formats-compatible-zs-color
// Introduced in VK_KHR_maintenance8 to allow copying between color and depth/stencil formats
inline bool vkuFormatIsDepthStencilWithColorSizeCompatible(VkFormat color_format, VkFormat ds_format);

// Returns the count of components in a VkFormat
inline uint32_t vkuFormatComponentCount(VkFormat format);

// Returns the texel block extent of a VkFormat
inline VkExtent3D vkuFormatTexelBlockExtent(VkFormat format);

// Returns the Compatibility Class of a VkFormat as defined by the spec
inline enum VKU_FORMAT_COMPATIBILITY_CLASS vkuFormatCompatibilityClass(VkFormat format);

// Returns the number of texels inside a texel block
// Will always be 1 when not using compressed block formats
inline uint32_t vkuFormatTexelsPerBlock(VkFormat format);

// Returns the number of bytes in a single Texel Block.
// When dealing with a depth/stencil format, need to consider using vkuFormatStencilSize or vkuFormatDepthSize.
// When dealing with mulit-planar formats, need to consider using vkuGetPlaneIndex.
inline uint32_t vkuFormatTexelBlockSize(VkFormat format);

// Return size, in bytes, of one element of a VkFormat
// Format must not be a depth, stencil, or multiplane format
// Deprecated - Use vkuFormatTexelBlockSize - there is no "element" size in the spec
inline uint32_t vkuFormatElementSize(VkFormat format);

// Return the size in bytes of one texel of a VkFormat
// For compressed or multi-plane, this may be a fractional number
// Deprecated - Use vkuFormatTexelBlockSize - there is no "element" size in the spec
inline uint32_t vkuFormatElementSizeWithAspect(VkFormat format, VkImageAspectFlagBits aspectMask);

// Return the size in bytes of one texel of a VkFormat
// Format must not be a depth, stencil, or multiplane format
inline double vkuFormatTexelSize(VkFormat format);

// Return the size in bytes of one texel of a VkFormat
// For compressed or multi-plane, this may be a fractional number
inline double vkuFormatTexelSizeWithAspect(VkFormat format, VkImageAspectFlagBits aspectMask);

// Returns whether a VkFormat contains only 8-bit sized components
inline bool vkuFormatIs8bit(VkFormat format);

// Returns whether a VkFormat contains only 16-bit sized components
inline bool vkuFormatIs16bit(VkFormat format);

// Returns whether a VkFormat contains only 32-bit sized components
inline bool vkuFormatIs32bit(VkFormat format);

// Returns whether a VkFormat contains only 64-bit sized components
inline bool vkuFormatIs64bit(VkFormat format);

// Returns whether a VkFormat has a component of a given size
inline bool vkuFormatHasComponentSize(VkFormat format, uint32_t size);

// Returns whether a VkFormat has a Red color component
inline bool vkuFormatHasRed(VkFormat format);

// Returns whether a VkFormat has a Green color component
inline bool vkuFormatHasGreen(VkFormat format);

// Returns whether a VkFormat has a Blue color component
inline bool vkuFormatHasBlue(VkFormat format);

// Returns whether a VkFormat has a Alpha color component
inline bool vkuFormatHasAlpha(VkFormat format);

// Returns whether a VkFormat is equal to VK_FORMAT_UNDEFINED
inline bool vkuFormatIsUndefined(VkFormat format) { return (format == VK_FORMAT_UNDEFINED); }

// Returns whether a VkFormat is a "blocked image" as defined in the spec (vkspec.html#blocked-image)
inline bool vkuFormatIsBlockedImage(VkFormat format) {
    return (vkuFormatIsCompressed(format) || vkuFormatIsSinglePlane_422(format));
}

// Returns whether a VkFormat is a "color format'. Because there is no official specification definition of
// "color format", it is defined here as anything that isn't a depth/stencil format, multiplane format, or the undefined format.
inline bool vkuFormatIsColor(VkFormat format) {
    return !(vkuFormatIsUndefined(format) || vkuFormatIsDepthOrStencil(format) || vkuFormatIsMultiplane(format));
}

enum VKU_FORMAT_COMPONENT_TYPE {
    VKU_FORMAT_COMPONENT_TYPE_NONE,
    VKU_FORMAT_COMPONENT_TYPE_R,
    VKU_FORMAT_COMPONENT_TYPE_G,
    VKU_FORMAT_COMPONENT_TYPE_B,
    VKU_FORMAT_COMPONENT_TYPE_A,
    VKU_FORMAT_COMPONENT_TYPE_D,
    VKU_FORMAT_COMPONENT_TYPE_S,
};

// Compressed formats don't have a defined component size
const uint32_t VKU_FORMAT_COMPRESSED_COMPONENT = 0xFFFFFFFF;

struct VKU_FORMAT_COMPONENT_INFO {
    enum VKU_FORMAT_COMPONENT_TYPE type;
    uint32_t size;  // bits
};

// Generic information for all formats
struct VKU_FORMAT_INFO {
    enum VKU_FORMAT_COMPATIBILITY_CLASS compatibility;
    uint32_t texel_block_size;  // bytes
    uint32_t texels_per_block;
    VkExtent3D block_extent;
    uint32_t component_count;
    struct VKU_FORMAT_COMPONENT_INFO components[VKU_FORMAT_MAX_COMPONENTS];
};
inline const struct VKU_FORMAT_INFO vkuGetFormatInfo(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R4G4_UNORM_PACK8: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT, 1, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 4}, {VKU_FORMAT_COMPONENT_TYPE_G, 4}}};
            return out; }
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 4}, {VKU_FORMAT_COMPONENT_TYPE_G, 4}, {VKU_FORMAT_COMPONENT_TYPE_B, 4}, {VKU_FORMAT_COMPONENT_TYPE_A, 4}}};
            return out; }
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 4}, {VKU_FORMAT_COMPONENT_TYPE_G, 4}, {VKU_FORMAT_COMPONENT_TYPE_R, 4}, {VKU_FORMAT_COMPONENT_TYPE_A, 4}}};
            return out; }
        case VK_FORMAT_R5G6B5_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 5}, {VKU_FORMAT_COMPONENT_TYPE_G, 6}, {VKU_FORMAT_COMPONENT_TYPE_B, 5}}};
            return out; }
        case VK_FORMAT_B5G6R5_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 5}, {VKU_FORMAT_COMPONENT_TYPE_G, 6}, {VKU_FORMAT_COMPONENT_TYPE_R, 5}}};
            return out; }
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 5}, {VKU_FORMAT_COMPONENT_TYPE_G, 5}, {VKU_FORMAT_COMPONENT_TYPE_B, 5}, {VKU_FORMAT_COMPONENT_TYPE_A, 1}}};
            return out; }
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 5}, {VKU_FORMAT_COMPONENT_TYPE_G, 5}, {VKU_FORMAT_COMPONENT_TYPE_R, 5}, {VKU_FORMAT_COMPONENT_TYPE_A, 1}}};
            return out; }
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 1}, {VKU_FORMAT_COMPONENT_TYPE_R, 5}, {VKU_FORMAT_COMPONENT_TYPE_G, 5}, {VKU_FORMAT_COMPONENT_TYPE_B, 5}}};
            return out; }
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 1}, {VKU_FORMAT_COMPONENT_TYPE_B, 5}, {VKU_FORMAT_COMPONENT_TYPE_G, 5}, {VKU_FORMAT_COMPONENT_TYPE_R, 5}}};
            return out; }
        case VK_FORMAT_A8_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_ALPHA, 1, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_R8_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT, 1, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_R8_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT, 1, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_R8_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT, 1, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_R8_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT, 1, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_R8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT, 1, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_R8_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT, 1, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_R8_SRGB: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT, 1, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_R8G8_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}}};
            return out; }
        case VK_FORMAT_R8G8_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}}};
            return out; }
        case VK_FORMAT_R8G8_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}}};
            return out; }
        case VK_FORMAT_R8G8_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}}};
            return out; }
        case VK_FORMAT_R8G8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}}};
            return out; }
        case VK_FORMAT_R8G8_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}}};
            return out; }
        case VK_FORMAT_R8G8_SRGB: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8_SRGB: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8_SRGB: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_24BIT, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8A8_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8A8_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8A8_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8A8_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8A8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8A8_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_R8G8B8A8_SRGB: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8A8_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8A8_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8A8_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8A8_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8A8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8A8_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8A8_SRGB: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_A, 8}}};
            return out; }
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_A8B8G8R8_UINT_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_A8B8G8R8_SINT_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}}};
            return out; }
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}}};
            return out; }
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}}};
            return out; }
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}}};
            return out; }
        case VK_FORMAT_A2R10G10B10_UINT_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}}};
            return out; }
        case VK_FORMAT_A2R10G10B10_SINT_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}}};
            return out; }
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_A2B10G10R10_UINT_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_A2B10G10R10_SINT_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 2}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_R16_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_R16_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_R16_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_R16_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_R16_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_R16_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_R16_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_R16G16_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}}};
            return out; }
        case VK_FORMAT_R16G16_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}}};
            return out; }
        case VK_FORMAT_R16G16_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}}};
            return out; }
        case VK_FORMAT_R16G16_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}}};
            return out; }
        case VK_FORMAT_R16G16_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}}};
            return out; }
        case VK_FORMAT_R16G16_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}}};
            return out; }
        case VK_FORMAT_R16G16_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_48BIT, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_48BIT, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_48BIT, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_48BIT, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_48BIT, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_48BIT, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_48BIT, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16A16_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_A, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16A16_SNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_A, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16A16_USCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_A, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16A16_SSCALED: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_A, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16A16_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_A, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16A16_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_A, 16}}};
            return out; }
        case VK_FORMAT_R16G16B16A16_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_A, 16}}};
            return out; }
        case VK_FORMAT_R32_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}}};
            return out; }
        case VK_FORMAT_R32_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}}};
            return out; }
        case VK_FORMAT_R32_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}}};
            return out; }
        case VK_FORMAT_R32G32_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}, {VKU_FORMAT_COMPONENT_TYPE_G, 32}}};
            return out; }
        case VK_FORMAT_R32G32_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}, {VKU_FORMAT_COMPONENT_TYPE_G, 32}}};
            return out; }
        case VK_FORMAT_R32G32_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}, {VKU_FORMAT_COMPONENT_TYPE_G, 32}}};
            return out; }
        case VK_FORMAT_R32G32B32_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_96BIT, 12, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}, {VKU_FORMAT_COMPONENT_TYPE_G, 32}, {VKU_FORMAT_COMPONENT_TYPE_B, 32}}};
            return out; }
        case VK_FORMAT_R32G32B32_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_96BIT, 12, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}, {VKU_FORMAT_COMPONENT_TYPE_G, 32}, {VKU_FORMAT_COMPONENT_TYPE_B, 32}}};
            return out; }
        case VK_FORMAT_R32G32B32_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_96BIT, 12, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}, {VKU_FORMAT_COMPONENT_TYPE_G, 32}, {VKU_FORMAT_COMPONENT_TYPE_B, 32}}};
            return out; }
        case VK_FORMAT_R32G32B32A32_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_128BIT, 16, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}, {VKU_FORMAT_COMPONENT_TYPE_G, 32}, {VKU_FORMAT_COMPONENT_TYPE_B, 32}, {VKU_FORMAT_COMPONENT_TYPE_A, 32}}};
            return out; }
        case VK_FORMAT_R32G32B32A32_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_128BIT, 16, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}, {VKU_FORMAT_COMPONENT_TYPE_G, 32}, {VKU_FORMAT_COMPONENT_TYPE_B, 32}, {VKU_FORMAT_COMPONENT_TYPE_A, 32}}};
            return out; }
        case VK_FORMAT_R32G32B32A32_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_128BIT, 16, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 32}, {VKU_FORMAT_COMPONENT_TYPE_G, 32}, {VKU_FORMAT_COMPONENT_TYPE_B, 32}, {VKU_FORMAT_COMPONENT_TYPE_A, 32}}};
            return out; }
        case VK_FORMAT_R64_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}}};
            return out; }
        case VK_FORMAT_R64_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}}};
            return out; }
        case VK_FORMAT_R64_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT, 8, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}}};
            return out; }
        case VK_FORMAT_R64G64_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_128BIT, 16, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}, {VKU_FORMAT_COMPONENT_TYPE_G, 64}}};
            return out; }
        case VK_FORMAT_R64G64_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_128BIT, 16, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}, {VKU_FORMAT_COMPONENT_TYPE_G, 64}}};
            return out; }
        case VK_FORMAT_R64G64_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_128BIT, 16, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}, {VKU_FORMAT_COMPONENT_TYPE_G, 64}}};
            return out; }
        case VK_FORMAT_R64G64B64_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_192BIT, 24, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}, {VKU_FORMAT_COMPONENT_TYPE_G, 64}, {VKU_FORMAT_COMPONENT_TYPE_B, 64}}};
            return out; }
        case VK_FORMAT_R64G64B64_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_192BIT, 24, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}, {VKU_FORMAT_COMPONENT_TYPE_G, 64}, {VKU_FORMAT_COMPONENT_TYPE_B, 64}}};
            return out; }
        case VK_FORMAT_R64G64B64_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_192BIT, 24, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}, {VKU_FORMAT_COMPONENT_TYPE_G, 64}, {VKU_FORMAT_COMPONENT_TYPE_B, 64}}};
            return out; }
        case VK_FORMAT_R64G64B64A64_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_256BIT, 32, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}, {VKU_FORMAT_COMPONENT_TYPE_G, 64}, {VKU_FORMAT_COMPONENT_TYPE_B, 64}, {VKU_FORMAT_COMPONENT_TYPE_A, 64}}};
            return out; }
        case VK_FORMAT_R64G64B64A64_SINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_256BIT, 32, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}, {VKU_FORMAT_COMPONENT_TYPE_G, 64}, {VKU_FORMAT_COMPONENT_TYPE_B, 64}, {VKU_FORMAT_COMPONENT_TYPE_A, 64}}};
            return out; }
        case VK_FORMAT_R64G64B64A64_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_256BIT, 32, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 64}, {VKU_FORMAT_COMPONENT_TYPE_G, 64}, {VKU_FORMAT_COMPONENT_TYPE_B, 64}, {VKU_FORMAT_COMPONENT_TYPE_A, 64}}};
            return out; }
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 11}, {VKU_FORMAT_COMPONENT_TYPE_R, 11}}};
            return out; }
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_B, 9}, {VKU_FORMAT_COMPONENT_TYPE_G, 9}, {VKU_FORMAT_COMPONENT_TYPE_R, 9}}};
            return out; }
        case VK_FORMAT_D16_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_D16, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_D, 16}}};
            return out; }
        case VK_FORMAT_X8_D24_UNORM_PACK32: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_D24, 4, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_D, 24}}};
            return out; }
        case VK_FORMAT_D32_SFLOAT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_D32, 4, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_D, 32}}};
            return out; }
        case VK_FORMAT_S8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_S8, 1, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_S, 8}}};
            return out; }
        case VK_FORMAT_D16_UNORM_S8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_D16S8, 3, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_D, 16}, {VKU_FORMAT_COMPONENT_TYPE_S, 8}}};
            return out; }
        case VK_FORMAT_D24_UNORM_S8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_D24S8, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_D, 24}, {VKU_FORMAT_COMPONENT_TYPE_S, 8}}};
            return out; }
        case VK_FORMAT_D32_SFLOAT_S8_UINT: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_D32S8, 5, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_D, 32}, {VKU_FORMAT_COMPONENT_TYPE_S, 8}}};
            return out; }
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC1_RGB, 8, 16, {4, 4, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC1_RGB, 8, 16, {4, 4, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC1_RGBA, 8, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC1_RGBA, 8, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC2_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC2, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC2_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC2, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC3_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC3, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC3_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC3, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC4_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC4, 8, 16, {4, 4, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC4_SNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC4, 8, 16, {4, 4, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC5_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC5, 16, 16, {4, 4, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC5_SNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC5, 16, 16, {4, 4, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC6H_UFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC6H, 16, 16, {4, 4, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC6H_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC6H, 16, 16, {4, 4, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC7_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC7, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_BC7_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_BC7, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ETC2_RGB, 8, 16, {4, 4, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ETC2_RGB, 8, 16, {4, 4, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ETC2_RGBA, 8, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ETC2_RGBA, 8, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ETC2_EAC_RGBA, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ETC2_EAC_RGBA, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_EAC_R11_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_EAC_R, 8, 16, {4, 4, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 11}}};
            return out; }
        case VK_FORMAT_EAC_R11_SNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_EAC_R, 8, 16, {4, 4, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 11}}};
            return out; }
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_EAC_RG, 16, 16, {4, 4, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 11}, {VKU_FORMAT_COMPONENT_TYPE_G, 11}}};
            return out; }
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_EAC_RG, 16, 16, {4, 4, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 11}, {VKU_FORMAT_COMPONENT_TYPE_G, 11}}};
            return out; }
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_4X4, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_4X4, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_5X4, 16, 20, {5, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_5X4, 16, 20, {5, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_5X5, 16, 25, {5, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_5X5, 16, 25, {5, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_6X5, 16, 30, {6, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_6X5, 16, 30, {6, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_6X6, 16, 36, {6, 6, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_6X6, 16, 36, {6, 6, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X5, 16, 40, {8, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X5, 16, 40, {8, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X6, 16, 48, {8, 6, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X6, 16, 48, {8, 6, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X8, 16, 64, {8, 8, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X8, 16, 64, {8, 8, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X5, 16, 50, {10, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X5, 16, 50, {10, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X6, 16, 60, {10, 6, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X6, 16, 60, {10, 6, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X8, 16, 80, {10, 8, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X8, 16, 80, {10, 8, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X10, 16, 100, {10, 10, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X10, 16, 100, {10, 10, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_12X10, 16, 120, {12, 10, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_12X10, 16, 120, {12, 10, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_12X12, 16, 144, {12, 12, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_12X12, 16, 144, {12, 12, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_G8B8G8R8_422_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT_G8B8G8R8, 4, 1, {2, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_B8G8R8G8_422_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT_B8G8R8G8, 4, 1, {2, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}, {VKU_FORMAT_COMPONENT_TYPE_G, 8}}};
            return out; }
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_3PLANE_420, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_2PLANE_420, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_3PLANE_422, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_2PLANE_422, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_3PLANE_444, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_R10X6_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}}};
            return out; }
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_R10G10B10A10, 8, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_A, 10}}};
            return out; }
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_G10B10G10R10, 8, 1, {2, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_B10G10R10G10, 8, 1, {2, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}, {VKU_FORMAT_COMPONENT_TYPE_G, 10}}};
            return out; }
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_3PLANE_420, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_2PLANE_420, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_3PLANE_422, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_2PLANE_422, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_3PLANE_444, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_R12X4_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 1, {{VKU_FORMAT_COMPONENT_TYPE_R, 12}}};
            return out; }
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 12}, {VKU_FORMAT_COMPONENT_TYPE_G, 12}}};
            return out; }
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_R12G12B12A12, 8, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, 12}, {VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_B, 12}, {VKU_FORMAT_COMPONENT_TYPE_A, 12}}};
            return out; }
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_G12B12G12R12, 8, 1, {2, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_B, 12}, {VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_R, 12}}};
            return out; }
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_B12G12R12G12, 8, 1, {2, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 12}, {VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_R, 12}, {VKU_FORMAT_COMPONENT_TYPE_G, 12}}};
            return out; }
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_3PLANE_420, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_B, 12}, {VKU_FORMAT_COMPONENT_TYPE_R, 12}}};
            return out; }
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_2PLANE_420, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_B, 12}, {VKU_FORMAT_COMPONENT_TYPE_R, 12}}};
            return out; }
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_3PLANE_422, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_B, 12}, {VKU_FORMAT_COMPONENT_TYPE_R, 12}}};
            return out; }
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_2PLANE_422, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_B, 12}, {VKU_FORMAT_COMPONENT_TYPE_R, 12}}};
            return out; }
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_3PLANE_444, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_B, 12}, {VKU_FORMAT_COMPONENT_TYPE_R, 12}}};
            return out; }
        case VK_FORMAT_G16B16G16R16_422_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_G16B16G16R16, 8, 1, {2, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_B16G16R16G16_422_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_64BIT_B16G16R16G16, 8, 1, {2, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}}};
            return out; }
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_3PLANE_420, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_2PLANE_420, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_3PLANE_422, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_2PLANE_422, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_3PLANE_444, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC1_2BPP, 8, 1, {8, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC1_4BPP, 8, 1, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC2_2BPP, 8, 1, {8, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC2_4BPP, 8, 1, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC1_2BPP, 8, 1, {8, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC1_4BPP, 8, 1, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC2_2BPP, 8, 1, {8, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_PVRTC2_4BPP, 8, 1, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_4X4, 16, 16, {4, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_5X4, 16, 20, {5, 4, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_5X5, 16, 25, {5, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_6X5, 16, 30, {6, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_6X6, 16, 36, {6, 6, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X5, 16, 40, {8, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X6, 16, 48, {8, 6, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_8X8, 16, 64, {8, 8, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X5, 16, 50, {10, 5, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X6, 16, 60, {10, 6, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X8, 16, 80, {10, 8, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_10X10, 16, 100, {10, 10, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_12X10, 16, 120, {12, 10, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_ASTC_12X12, 16, 144, {12, 12, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_R, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_G, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_B, VKU_FORMAT_COMPRESSED_COMPONENT}, {VKU_FORMAT_COMPONENT_TYPE_A, VKU_FORMAT_COMPRESSED_COMPONENT}}};
            return out; }
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_8BIT_2PLANE_444, 3, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 8}, {VKU_FORMAT_COMPONENT_TYPE_B, 8}, {VKU_FORMAT_COMPONENT_TYPE_R, 8}}};
            return out; }
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_10BIT_2PLANE_444, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 10}, {VKU_FORMAT_COMPONENT_TYPE_B, 10}, {VKU_FORMAT_COMPONENT_TYPE_R, 10}}};
            return out; }
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_12BIT_2PLANE_444, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 12}, {VKU_FORMAT_COMPONENT_TYPE_B, 12}, {VKU_FORMAT_COMPONENT_TYPE_R, 12}}};
            return out; }
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT_2PLANE_444, 6, 1, {1, 1, 1}, 3, {{VKU_FORMAT_COMPONENT_TYPE_G, 16}, {VKU_FORMAT_COMPONENT_TYPE_B, 16}, {VKU_FORMAT_COMPONENT_TYPE_R, 16}}};
            return out; }
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 4}, {VKU_FORMAT_COMPONENT_TYPE_R, 4}, {VKU_FORMAT_COMPONENT_TYPE_G, 4}, {VKU_FORMAT_COMPONENT_TYPE_B, 4}}};
            return out; }
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_16BIT, 2, 1, {1, 1, 1}, 4, {{VKU_FORMAT_COMPONENT_TYPE_A, 4}, {VKU_FORMAT_COMPONENT_TYPE_B, 4}, {VKU_FORMAT_COMPONENT_TYPE_G, 4}, {VKU_FORMAT_COMPONENT_TYPE_R, 4}}};
            return out; }
        case VK_FORMAT_R16G16_SFIXED5_NV: {
            struct VKU_FORMAT_INFO out = {VKU_FORMAT_COMPATIBILITY_CLASS_32BIT, 4, 1, {1, 1, 1}, 2, {{VKU_FORMAT_COMPONENT_TYPE_R, 16}, {VKU_FORMAT_COMPONENT_TYPE_G, 16}}};
            return out; }

        default: {
            // return values for VK_FORMAT_UNDEFINED
            struct VKU_FORMAT_INFO out = { VKU_FORMAT_COMPATIBILITY_CLASS_NONE, 0, 0, {0, 0, 0}, 0, {{VKU_FORMAT_COMPONENT_TYPE_NONE, 0}, {VKU_FORMAT_COMPONENT_TYPE_NONE, 0}, {VKU_FORMAT_COMPONENT_TYPE_NONE, 0}, {VKU_FORMAT_COMPONENT_TYPE_NONE, 0}} };
            return out;
        }
    };
}

struct VKU_FORMAT_PER_PLANE_COMPATIBILITY {
    uint32_t width_divisor;
    uint32_t height_divisor;
    VkFormat compatible_format;
};

// Information for multiplanar formats
struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY {
    struct VKU_FORMAT_PER_PLANE_COMPATIBILITY per_plane[VKU_FORMAT_MAX_PLANES];
};

// Source: Vulkan spec Table 47. Plane Format Compatibility Table
inline const struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY vkuGetFormatCompatibility(VkFormat format) {
    switch (format) {
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R8_UNORM }, {2, 2, VK_FORMAT_R8_UNORM }, {2, 2, VK_FORMAT_R8_UNORM }}};
            return out; }
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R8_UNORM }, {2, 2, VK_FORMAT_R8G8_UNORM }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R8_UNORM }, {2, 1, VK_FORMAT_R8_UNORM }, {2, 1, VK_FORMAT_R8_UNORM }}};
            return out; }
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R8_UNORM }, {2, 1, VK_FORMAT_R8G8_UNORM }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R8_UNORM }, {1, 1, VK_FORMAT_R8_UNORM }, {1, 1, VK_FORMAT_R8_UNORM }}};
            return out; }
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R10X6_UNORM_PACK16 }, {2, 2, VK_FORMAT_R10X6_UNORM_PACK16 }, {2, 2, VK_FORMAT_R10X6_UNORM_PACK16 }}};
            return out; }
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R10X6_UNORM_PACK16 }, {2, 2, VK_FORMAT_R10X6G10X6_UNORM_2PACK16 }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R10X6_UNORM_PACK16 }, {2, 1, VK_FORMAT_R10X6_UNORM_PACK16 }, {2, 1, VK_FORMAT_R10X6_UNORM_PACK16 }}};
            return out; }
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R10X6_UNORM_PACK16 }, {2, 1, VK_FORMAT_R10X6G10X6_UNORM_2PACK16 }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R10X6_UNORM_PACK16 }, {1, 1, VK_FORMAT_R10X6_UNORM_PACK16 }, {1, 1, VK_FORMAT_R10X6_UNORM_PACK16 }}};
            return out; }
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R12X4_UNORM_PACK16 }, {2, 2, VK_FORMAT_R12X4_UNORM_PACK16 }, {2, 2, VK_FORMAT_R12X4_UNORM_PACK16 }}};
            return out; }
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R12X4_UNORM_PACK16 }, {2, 2, VK_FORMAT_R12X4G12X4_UNORM_2PACK16 }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R12X4_UNORM_PACK16 }, {2, 1, VK_FORMAT_R12X4_UNORM_PACK16 }, {2, 1, VK_FORMAT_R12X4_UNORM_PACK16 }}};
            return out; }
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R12X4_UNORM_PACK16 }, {2, 1, VK_FORMAT_R12X4G12X4_UNORM_2PACK16 }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R12X4_UNORM_PACK16 }, {1, 1, VK_FORMAT_R12X4_UNORM_PACK16 }, {1, 1, VK_FORMAT_R12X4_UNORM_PACK16 }}};
            return out; }
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R16_UNORM }, {2, 2, VK_FORMAT_R16_UNORM }, {2, 2, VK_FORMAT_R16_UNORM }}};
            return out; }
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R16_UNORM }, {2, 2, VK_FORMAT_R16G16_UNORM }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R16_UNORM }, {2, 1, VK_FORMAT_R16_UNORM }, {2, 1, VK_FORMAT_R16_UNORM }}};
            return out; }
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R16_UNORM }, {2, 1, VK_FORMAT_R16G16_UNORM }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R16_UNORM }, {1, 1, VK_FORMAT_R16_UNORM }, {1, 1, VK_FORMAT_R16_UNORM }}};
            return out; }
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R8_UNORM }, {1, 1, VK_FORMAT_R8G8_UNORM }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R10X6_UNORM_PACK16 }, {1, 1, VK_FORMAT_R10X6G10X6_UNORM_2PACK16 }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R12X4_UNORM_PACK16 }, {1, 1, VK_FORMAT_R12X4G12X4_UNORM_2PACK16 }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_R16_UNORM }, {1, 1, VK_FORMAT_R16G16_UNORM }, {1, 1, VK_FORMAT_UNDEFINED }}};
            return out; }
        default: {
            struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY out = {{{1, 1, VK_FORMAT_UNDEFINED}, {1, 1, VK_FORMAT_UNDEFINED}, {1, 1, VK_FORMAT_UNDEFINED}}};
            return out; }
    };
}

// Return true if all components in a format are an SFIXED5
bool vkuFormatIsSFIXED5(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R16G16_SFIXED5_NV:
            return true;
        default:
            return false;
    }
}

// Return true if all components in a format are an SFLOAT
bool vkuFormatIsSFLOAT(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_R64G64B64_SFLOAT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            return true;
        default:
            return false;
    }
}

// Return true if all components in a format are an SINT
bool vkuFormatIsSINT(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64A64_SINT:
            return true;
        default:
            return false;
    }
}

// Return true if all components in a format are an SNORM
bool vkuFormatIsSNORM(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
            return true;
        default:
            return false;
    }
}

// Return true if all components in a format are an SRGB
bool vkuFormatIsSRGB(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
            return true;
        default:
            return false;
    }
}

// Return true if all components in a format are an SSCALED
bool vkuFormatIsSSCALED(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
            return true;
        default:
            return false;
    }
}

// Return true if all components in a format are an UFLOAT
bool vkuFormatIsUFLOAT(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            return true;
        default:
            return false;
    }
}

// Return true if all components in a format are an UINT
bool vkuFormatIsUINT(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

// Return true if all components in a format are an UNORM
bool vkuFormatIsUNORM(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R4G4_UNORM_PACK8:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16:
        case VK_FORMAT_A8_UNORM:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_R10X6_UNORM_PACK16:
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_R12X4_UNORM_PACK16:
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            return true;
        default:
            return false;
    }
}

// Return true if all components in a format are an USCALED
bool vkuFormatIsUSCALED(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16A16_USCALED:
            return true;
        default:
            return false;
    }
}

inline bool vkuFormatIsSampledInt(VkFormat format) { return (vkuFormatIsSINT(format) || vkuFormatIsUINT(format)); }
inline bool vkuFormatIsSampledFloat(VkFormat format) {
    return (vkuFormatIsUNORM(format) || vkuFormatIsSNORM(format) ||
            vkuFormatIsUSCALED(format) || vkuFormatIsSSCALED(format) ||
            vkuFormatIsUFLOAT(format) || vkuFormatIsSFLOAT(format) ||
            vkuFormatIsSRGB(format));
}

// Return true if a format is a ASTC_HDR compressed image format
bool vkuFormatIsCompressed_ASTC_HDR(VkFormat format) {
    switch (format) {
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
            return true;
        default:
            return false;
    }
}

// Return true if a format is a ASTC_LDR compressed image format
bool vkuFormatIsCompressed_ASTC_LDR(VkFormat format) {
    switch (format) {
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
            return true;
        default:
            return false;
    }
}

// Return true if a format is a BC compressed image format
bool vkuFormatIsCompressed_BC(VkFormat format) {
    switch (format) {
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
            return true;
        default:
            return false;
    }
}

// Return true if a format is a EAC compressed image format
bool vkuFormatIsCompressed_EAC(VkFormat format) {
    switch (format) {
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            return true;
        default:
            return false;
    }
}

// Return true if a format is a ETC2 compressed image format
bool vkuFormatIsCompressed_ETC2(VkFormat format) {
    switch (format) {
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return true;
        default:
            return false;
    }
}

// Return true if a format is a PVRTC compressed image format
bool vkuFormatIsCompressed_PVRTC(VkFormat format) {
    switch (format) {
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
            return true;
        default:
            return false;
    }
}
// Return true if a format is any compressed image format
bool vkuFormatIsCompressed(VkFormat format) {
    return
        vkuFormatIsCompressed_ASTC_HDR(format) ||
        vkuFormatIsCompressed_ASTC_LDR(format) ||
        vkuFormatIsCompressed_BC(format) ||
        vkuFormatIsCompressed_EAC(format) ||
        vkuFormatIsCompressed_ETC2(format) ||
        vkuFormatIsCompressed_PVRTC(format);
}

// Return true if format is a depth OR stencil format
bool vkuFormatIsDepthOrStencil(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

// Return true if format is a depth AND stencil format
bool vkuFormatIsDepthAndStencil(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

// Return true if format is a depth ONLY format
bool vkuFormatIsDepthOnly(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            return true;
        default:
            return false;
    }
}

// Return true if format is a stencil ONLY format
bool vkuFormatIsStencilOnly(VkFormat format) {
    switch (format) {
        case VK_FORMAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

// Returns size of depth component in bits
// Returns zero if no depth component
uint32_t vkuFormatDepthSize(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return 16;
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return 24;
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return 32;
        default:
            return 0;
    }
}

// Returns size of stencil component in bits
// Returns zero if no stencil component
uint32_t vkuFormatStencilSize(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_S8_UINT:
            return 8;
        default:
            return 0;
    }
}

// Returns NONE if no depth component
enum VKU_FORMAT_NUMERICAL_TYPE vkuFormatDepthNumericalType(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return VKU_FORMAT_NUMERICAL_TYPE_UNORM;
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VKU_FORMAT_NUMERICAL_TYPE_SFLOAT;
        default:
            return VKU_FORMAT_NUMERICAL_TYPE_NONE;
    }
}

// Returns NONE if no stencil component
enum VKU_FORMAT_NUMERICAL_TYPE vkuFormatStencilNumericalType(VkFormat format) {
    switch (format) {
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_S8_UINT:
            return VKU_FORMAT_NUMERICAL_TYPE_UINT;
        default:
            return VKU_FORMAT_NUMERICAL_TYPE_NONE;
    }
}

// Return true if format is a packed format
bool vkuFormatIsPacked(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R4G4_UNORM_PACK8:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_R10X6_UNORM_PACK16:
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_R12X4_UNORM_PACK16:
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            return true;
        default:
            return false;
    }
}

// Return true if format requires sampler YCBCR conversion
// for VK_IMAGE_ASPECT_COLOR_BIT image views
// Table found in spec
bool vkuFormatRequiresYcbcrConversion(VkFormat format) {
    switch (format) {
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
            return true;
        default:
            return false;
    }
}

bool vkuFormatIsXChromaSubsampled(VkFormat format) {
    switch (format) {
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
            return true;
        default:
            return false;
    }
}

bool vkuFormatIsYChromaSubsampled(VkFormat format) {
    switch (format) {
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
            return true;
        default:
            return false;
    }
}

bool vkuFormatIsSinglePlane_422(VkFormat format) {
    switch (format) {
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
            return true;
        default:
            return false;
    }
}

// Returns number of planes in format (which is 1 by default)
uint32_t vkuFormatPlaneCount(VkFormat format) {
    switch (format) {
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
            return 2;
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            return 3;
        default:
            return 1;
    }
}

// Will return VK_FORMAT_UNDEFINED if given a plane aspect that doesn't exist for the format
inline VkFormat vkuFindMultiplaneCompatibleFormat(VkFormat mp_fmt, VkImageAspectFlagBits plane_aspect) {
    const uint32_t plane_idx = vkuGetPlaneIndex(plane_aspect);
    const struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY multiplane_compatibility = vkuGetFormatCompatibility(mp_fmt);
    if ((multiplane_compatibility.per_plane[0].compatible_format == VK_FORMAT_UNDEFINED) || (plane_idx >= VKU_FORMAT_MAX_PLANES)) {
        return VK_FORMAT_UNDEFINED;
    }

    return multiplane_compatibility.per_plane[plane_idx].compatible_format;
}

inline VkExtent2D vkuFindMultiplaneExtentDivisors(VkFormat mp_fmt, VkImageAspectFlagBits plane_aspect) {
    VkExtent2D divisors = {1, 1};
    const uint32_t plane_idx = vkuGetPlaneIndex(plane_aspect);
    const struct VKU_FORMAT_MULTIPLANE_COMPATIBILITY multiplane_compatibility = vkuGetFormatCompatibility(mp_fmt);
    if ((multiplane_compatibility.per_plane[0].compatible_format == VK_FORMAT_UNDEFINED) || (plane_idx >= VKU_FORMAT_MAX_PLANES)) {
        return divisors;
    }

    divisors.width = multiplane_compatibility.per_plane[plane_idx].width_divisor;
    divisors.height = multiplane_compatibility.per_plane[plane_idx].height_divisor;
    return divisors;
}

// TODO - This should be generated, but will need updating the spec XML and table
inline bool vkuFormatIsDepthStencilWithColorSizeCompatible(VkFormat color_format, VkFormat ds_format) {
    switch (ds_format) {
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return color_format == VK_FORMAT_R32_SFLOAT || color_format == VK_FORMAT_R32_SINT || color_format == VK_FORMAT_R32_UINT;
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return color_format == VK_FORMAT_R32_SFLOAT || color_format == VK_FORMAT_R32_SINT || color_format == VK_FORMAT_R32_UINT;
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return color_format == VK_FORMAT_R16_SFLOAT || color_format == VK_FORMAT_R16_UNORM ||
                   color_format == VK_FORMAT_R16_SNORM  || color_format == VK_FORMAT_R16_UINT || color_format == VK_FORMAT_R16_SINT;
        case VK_FORMAT_S8_UINT:
            return color_format == VK_FORMAT_R8_UINT || color_format == VK_FORMAT_R8_SINT ||
                   color_format == VK_FORMAT_R8_UNORM || color_format == VK_FORMAT_R8_SNORM;
        default:
            return false;
    }
}

inline uint32_t vkuFormatComponentCount(VkFormat format) { return vkuGetFormatInfo(format).component_count; }

inline VkExtent3D vkuFormatTexelBlockExtent(VkFormat format) { return vkuGetFormatInfo(format).block_extent; }

inline enum VKU_FORMAT_COMPATIBILITY_CLASS vkuFormatCompatibilityClass(VkFormat format) { return vkuGetFormatInfo(format).compatibility; }

inline uint32_t vkuFormatTexelsPerBlock(VkFormat format) { return vkuGetFormatInfo(format).texels_per_block; }

inline uint32_t vkuFormatTexelBlockSize(VkFormat format) { return vkuGetFormatInfo(format).texel_block_size; }

// Deprecated - Use vkuFormatTexelBlockSize
inline uint32_t vkuFormatElementSize(VkFormat format) {
    return vkuFormatElementSizeWithAspect(format, VK_IMAGE_ASPECT_COLOR_BIT);
}

// Deprecated - Use vkuFormatTexelBlockSize
inline uint32_t vkuFormatElementSizeWithAspect(VkFormat format, VkImageAspectFlagBits aspectMask) {
    // Depth/Stencil aspect have separate helper functions
    if (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
        return vkuFormatStencilSize(format) / 8;
    } else if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
        return vkuFormatDepthSize(format) / 8;
    } else if (vkuFormatIsMultiplane(format)) {
        // Element of entire multiplane format is not useful,
        // Want to get just a single plane as the lookup format
        format = vkuFindMultiplaneCompatibleFormat(format, aspectMask);
    }

    return vkuGetFormatInfo(format).texel_block_size;
}

inline double vkuFormatTexelSize(VkFormat format) {
    return vkuFormatTexelSizeWithAspect(format, VK_IMAGE_ASPECT_COLOR_BIT);
}

inline double vkuFormatTexelSizeWithAspect(VkFormat format, VkImageAspectFlagBits aspectMask) {
    double texel_size = (double)(vkuFormatElementSizeWithAspect(format, aspectMask));
    VkExtent3D block_extent = vkuFormatTexelBlockExtent(format);
    uint32_t texels_per_block = block_extent.width * block_extent.height * block_extent.depth;
    if (1 < texels_per_block) {
        texel_size /= (double)(texels_per_block);
    }
    return texel_size;
}

inline bool vkuFormatIs8bit(VkFormat format) {
    switch (format) {
        case VK_FORMAT_A8_UNORM:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
            return true;
        default:
            return false;
    }
}
inline bool vkuFormatIs16bit(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
        case VK_FORMAT_R16G16_SFIXED5_NV:
            return true;
        default:
            return false;
    }
}
inline bool vkuFormatIs32bit(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT:
            return true;
        default:
            return false;
    }
}
inline bool vkuFormatIs64bit(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return true;
        default:
            return false;
    }
}

inline bool vkuFormatHasComponentSize(VkFormat format, uint32_t size) {
    const struct VKU_FORMAT_INFO format_info = vkuGetFormatInfo(format);
    bool equal_component_size = false;
    for (size_t i = 0; i < VKU_FORMAT_MAX_COMPONENTS; i++) {
        equal_component_size |= format_info.components[i].size == size;
    }
    return equal_component_size;
}

inline bool vkuFormatHasComponentType(VkFormat format, enum VKU_FORMAT_COMPONENT_TYPE component) {
    const struct VKU_FORMAT_INFO format_info = vkuGetFormatInfo(format);
    bool equal_component_type = false;
    for (size_t i = 0; i < VKU_FORMAT_MAX_COMPONENTS; i++) {
        equal_component_type |= format_info.components[i].type == component;
    }
    return equal_component_type;
}

inline bool vkuFormatHasRed(VkFormat format) { return vkuFormatHasComponentType(format, VKU_FORMAT_COMPONENT_TYPE_R); }

inline bool vkuFormatHasGreen(VkFormat format) { return vkuFormatHasComponentType(format, VKU_FORMAT_COMPONENT_TYPE_G); }

inline bool vkuFormatHasBlue(VkFormat format) { return vkuFormatHasComponentType(format, VKU_FORMAT_COMPONENT_TYPE_B); }

inline bool vkuFormatHasAlpha(VkFormat format) { return vkuFormatHasComponentType(format, VKU_FORMAT_COMPONENT_TYPE_A); }

inline uint32_t vkuGetPlaneIndex(VkImageAspectFlagBits aspect) {
    switch (aspect) {
        case VK_IMAGE_ASPECT_PLANE_0_BIT:
            return 0;
        case VK_IMAGE_ASPECT_PLANE_1_BIT:
            return 1;
        case VK_IMAGE_ASPECT_PLANE_2_BIT:
            return 2;
        default:
            return VKU_FORMAT_INVALID_INDEX;
    }
}

#ifdef __cplusplus
}
#endif

// clang-format off
