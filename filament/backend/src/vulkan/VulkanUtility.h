/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANUTILITY_H
#define TNT_FILAMENT_BACKEND_VULKANUTILITY_H

#include <backend/DriverEnums.h>

#include <utils/FixedCapacityVector.h>

#include <bluevk/BlueVK.h>

#include <utility>

namespace filament::backend {

VkFormat getVkFormat(ElementType type, bool normalized, bool integer);
VkFormat getVkFormat(TextureFormat format);
VkFormat getVkFormat(PixelDataFormat format, PixelDataType type);
VkFormat getVkFormatLinear(VkFormat format);
uint32_t getBytesPerPixel(TextureFormat format);
VkCompareOp getCompareOp(SamplerCompareFunc func);
VkBlendFactor getBlendFactor(BlendFunction mode);
VkCullModeFlags getCullMode(CullingMode mode);
VkFrontFace getFrontFace(bool inverseFrontFaces);
PixelDataType getComponentType(VkFormat format);
uint32_t getComponentCount(VkFormat format);
VkComponentMapping getSwizzleMap(TextureSwizzle const swizzle[4]);
VkShaderStageFlags getShaderStageFlags(ShaderStageFlags stageFlags);

bool equivalent(const VkRect2D& a, const VkRect2D& b);
bool equivalent(const VkExtent2D& a, const VkExtent2D& b);
bool isVkDepthFormat(VkFormat format);
bool isVkStencilFormat(VkFormat format);

VkImageAspectFlags getImageAspect(VkFormat format);
uint8_t reduceSampleCount(uint8_t sampleCount, VkSampleCountFlags mask);

// Helper function for the vkEnumerateX methods. These methods have the format of
// VkResult vkEnumerateX(InputType1 arg1, InputTyp2 arg2, ..., uint32_t* size,
//         OutputType* output_arg)
// Instead of macros and explicitly listing the template params, Variadic Template was also
// considered, but because the "variadic" part of the vk methods (i.e. the inputs) are before the
// non-variadic parts, this breaks the template type matching logic. Hence, we use a macro approach
// here.
#define EXPAND_ENUM(...)                                                          \
    uint32_t size = 0;                                                            \
    VkResult result = func(__VA_ARGS__, nullptr);                                 \
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "enumerate size error"; \
    utils::FixedCapacityVector<OutType> ret(size);                                \
    result = func(__VA_ARGS__, ret.data());                                       \
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "enumerate error";      \
    return std::move(ret);

#define EXPAND_ENUM_NO_ARGS() EXPAND_ENUM(&size)
#define EXPAND_ENUM_ARGS(...) EXPAND_ENUM(__VA_ARGS__, &size)

template <typename OutType>
utils::FixedCapacityVector<OutType> enumerate(VKAPI_ATTR VkResult (*func)(uint32_t*, OutType*)) {
    EXPAND_ENUM_NO_ARGS();
}

template <typename InType, typename OutType>
utils::FixedCapacityVector<OutType> enumerate(
        VKAPI_ATTR VkResult (*func)(InType, uint32_t*, OutType*), InType inData) {
    EXPAND_ENUM_ARGS(inData);
}

template <typename InTypeA, typename InTypeB, typename OutType>
utils::FixedCapacityVector<OutType> enumerate(
        VKAPI_ATTR VkResult (*func)(InTypeA, InTypeB, uint32_t*, OutType*), InTypeA inDataA,
        InTypeB inDataB) {
    EXPAND_ENUM_ARGS(inDataA, inDataB);
}

#undef EXPAND_ENUM
#undef EXPAND_ENUM_NO_ARGS
#undef EXPAND_ENUM_ARGS

// Useful shorthands
using VkFormatList = utils::FixedCapacityVector<VkFormat>;

// Copied from
//   https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFormat.html
constexpr VkFormat ALL_VK_FORMATS[] = {
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_R4G4_UNORM_PACK8,
        VK_FORMAT_R4G4B4A4_UNORM_PACK16,
        VK_FORMAT_B4G4R4A4_UNORM_PACK16,
        VK_FORMAT_R5G6B5_UNORM_PACK16,
        VK_FORMAT_B5G6R5_UNORM_PACK16,
        VK_FORMAT_R5G5B5A1_UNORM_PACK16,
        VK_FORMAT_B5G5R5A1_UNORM_PACK16,
        VK_FORMAT_A1R5G5B5_UNORM_PACK16,
        VK_FORMAT_R8_UNORM,
        VK_FORMAT_R8_SNORM,
        VK_FORMAT_R8_USCALED,
        VK_FORMAT_R8_SSCALED,
        VK_FORMAT_R8_UINT,
        VK_FORMAT_R8_SINT,
        VK_FORMAT_R8_SRGB,
        VK_FORMAT_R8G8_UNORM,
        VK_FORMAT_R8G8_SNORM,
        VK_FORMAT_R8G8_USCALED,
        VK_FORMAT_R8G8_SSCALED,
        VK_FORMAT_R8G8_UINT,
        VK_FORMAT_R8G8_SINT,
        VK_FORMAT_R8G8_SRGB,
        VK_FORMAT_R8G8B8_UNORM,
        VK_FORMAT_R8G8B8_SNORM,
        VK_FORMAT_R8G8B8_USCALED,
        VK_FORMAT_R8G8B8_SSCALED,
        VK_FORMAT_R8G8B8_UINT,
        VK_FORMAT_R8G8B8_SINT,
        VK_FORMAT_R8G8B8_SRGB,
        VK_FORMAT_B8G8R8_UNORM,
        VK_FORMAT_B8G8R8_SNORM,
        VK_FORMAT_B8G8R8_USCALED,
        VK_FORMAT_B8G8R8_SSCALED,
        VK_FORMAT_B8G8R8_UINT,
        VK_FORMAT_B8G8R8_SINT,
        VK_FORMAT_B8G8R8_SRGB,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_R8G8B8A8_SNORM,
        VK_FORMAT_R8G8B8A8_USCALED,
        VK_FORMAT_R8G8B8A8_SSCALED,
        VK_FORMAT_R8G8B8A8_UINT,
        VK_FORMAT_R8G8B8A8_SINT,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SNORM,
        VK_FORMAT_B8G8R8A8_USCALED,
        VK_FORMAT_B8G8R8A8_SSCALED,
        VK_FORMAT_B8G8R8A8_UINT,
        VK_FORMAT_B8G8R8A8_SINT,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_A8B8G8R8_UNORM_PACK32,
        VK_FORMAT_A8B8G8R8_SNORM_PACK32,
        VK_FORMAT_A8B8G8R8_USCALED_PACK32,
        VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
        VK_FORMAT_A8B8G8R8_UINT_PACK32,
        VK_FORMAT_A8B8G8R8_SINT_PACK32,
        VK_FORMAT_A8B8G8R8_SRGB_PACK32,
        VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        VK_FORMAT_A2R10G10B10_SNORM_PACK32,
        VK_FORMAT_A2R10G10B10_USCALED_PACK32,
        VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
        VK_FORMAT_A2R10G10B10_UINT_PACK32,
        VK_FORMAT_A2R10G10B10_SINT_PACK32,
        VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        VK_FORMAT_A2B10G10R10_SNORM_PACK32,
        VK_FORMAT_A2B10G10R10_USCALED_PACK32,
        VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
        VK_FORMAT_A2B10G10R10_UINT_PACK32,
        VK_FORMAT_A2B10G10R10_SINT_PACK32,
        VK_FORMAT_R16_UNORM,
        VK_FORMAT_R16_SNORM,
        VK_FORMAT_R16_USCALED,
        VK_FORMAT_R16_SSCALED,
        VK_FORMAT_R16_UINT,
        VK_FORMAT_R16_SINT,
        VK_FORMAT_R16_SFLOAT,
        VK_FORMAT_R16G16_UNORM,
        VK_FORMAT_R16G16_SNORM,
        VK_FORMAT_R16G16_USCALED,
        VK_FORMAT_R16G16_SSCALED,
        VK_FORMAT_R16G16_UINT,
        VK_FORMAT_R16G16_SINT,
        VK_FORMAT_R16G16_SFLOAT,
        VK_FORMAT_R16G16B16_UNORM,
        VK_FORMAT_R16G16B16_SNORM,
        VK_FORMAT_R16G16B16_USCALED,
        VK_FORMAT_R16G16B16_SSCALED,
        VK_FORMAT_R16G16B16_UINT,
        VK_FORMAT_R16G16B16_SINT,
        VK_FORMAT_R16G16B16_SFLOAT,
        VK_FORMAT_R16G16B16A16_UNORM,
        VK_FORMAT_R16G16B16A16_SNORM,
        VK_FORMAT_R16G16B16A16_USCALED,
        VK_FORMAT_R16G16B16A16_SSCALED,
        VK_FORMAT_R16G16B16A16_UINT,
        VK_FORMAT_R16G16B16A16_SINT,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_FORMAT_R32_UINT,
        VK_FORMAT_R32_SINT,
        VK_FORMAT_R32_SFLOAT,
        VK_FORMAT_R32G32_UINT,
        VK_FORMAT_R32G32_SINT,
        VK_FORMAT_R32G32_SFLOAT,
        VK_FORMAT_R32G32B32_UINT,
        VK_FORMAT_R32G32B32_SINT,
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32B32A32_UINT,
        VK_FORMAT_R32G32B32A32_SINT,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_FORMAT_R64_UINT,
        VK_FORMAT_R64_SINT,
        VK_FORMAT_R64_SFLOAT,
        VK_FORMAT_R64G64_UINT,
        VK_FORMAT_R64G64_SINT,
        VK_FORMAT_R64G64_SFLOAT,
        VK_FORMAT_R64G64B64_UINT,
        VK_FORMAT_R64G64B64_SINT,
        VK_FORMAT_R64G64B64_SFLOAT,
        VK_FORMAT_R64G64B64A64_UINT,
        VK_FORMAT_R64G64B64A64_SINT,
        VK_FORMAT_R64G64B64A64_SFLOAT,
        VK_FORMAT_B10G11R11_UFLOAT_PACK32,
        VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
        VK_FORMAT_D16_UNORM,
        VK_FORMAT_X8_D24_UNORM_PACK32,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_BC1_RGB_UNORM_BLOCK,
        VK_FORMAT_BC1_RGB_SRGB_BLOCK,
        VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
        VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
        VK_FORMAT_BC2_UNORM_BLOCK,
        VK_FORMAT_BC2_SRGB_BLOCK,
        VK_FORMAT_BC3_UNORM_BLOCK,
        VK_FORMAT_BC3_SRGB_BLOCK,
        VK_FORMAT_BC4_UNORM_BLOCK,
        VK_FORMAT_BC4_SNORM_BLOCK,
        VK_FORMAT_BC5_UNORM_BLOCK,
        VK_FORMAT_BC5_SNORM_BLOCK,
        VK_FORMAT_BC6H_UFLOAT_BLOCK,
        VK_FORMAT_BC6H_SFLOAT_BLOCK,
        VK_FORMAT_BC7_UNORM_BLOCK,
        VK_FORMAT_BC7_SRGB_BLOCK,
        VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
        VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
        VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
        VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
        VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
        VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
        VK_FORMAT_EAC_R11_UNORM_BLOCK,
        VK_FORMAT_EAC_R11_SNORM_BLOCK,
        VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
        VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
        VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
        VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
        VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
        VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
        VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
        VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
        VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
        VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
        VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
        VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
        VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
        VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
        VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
        VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
        VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
        VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
        VK_FORMAT_G8B8G8R8_422_UNORM,
        VK_FORMAT_B8G8R8G8_422_UNORM,
        VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
        VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
        VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
        VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
        VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
        VK_FORMAT_R10X6_UNORM_PACK16,
        VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
        VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
        VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
        VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
        VK_FORMAT_R12X4_UNORM_PACK16,
        VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
        VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
        VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
        VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
        VK_FORMAT_G16B16G16R16_422_UNORM,
        VK_FORMAT_B16G16R16G16_422_UNORM,
        VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
        VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
        VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
        VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
        VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
        VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
        VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
        VK_FORMAT_A4R4G4B4_UNORM_PACK16,
        VK_FORMAT_A4B4G4R4_UNORM_PACK16,
        VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
        VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
        VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
        VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
        VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
        VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
        VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
        VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
        VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
// Not supported (yet) by bluevk
//        VK_FORMAT_R16G16_SFIXED5_NV,
//        VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR,
//        VK_FORMAT_A8_UNORM_KHR,
//        VK_FORMAT_A8_UNORM,
        VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT,
        VK_FORMAT_G8B8G8R8_422_UNORM_KHR,
        VK_FORMAT_B8G8R8G8_422_UNORM_KHR,
        VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR,
        VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
        VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR,
        VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR,
        VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR,
        VK_FORMAT_R10X6_UNORM_PACK16_KHR,
        VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR,
        VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR,
        VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR,
        VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR,
        VK_FORMAT_R12X4_UNORM_PACK16_KHR,
        VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR,
        VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR,
        VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR,
        VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR,
        VK_FORMAT_G16B16G16R16_422_UNORM_KHR,
        VK_FORMAT_B16G16R16G16_422_UNORM_KHR,
        VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR,
        VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR,
        VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR,
        VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR,
        VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR,
        VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT,
        VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT,
        VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT,
        VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT,
        VK_FORMAT_R16G16_S10_5_NV,
};

// An Array that will be statically fixed in capacity, but the "size" (as in user added elements) is
// variable. Note that this class is movable.
template<typename T, uint16_t CAPACITY>
class CappedArray {
private:
    using FixedSizeArray = std::array<T, CAPACITY>;

public:
    using const_iterator = typename FixedSizeArray::const_iterator;
    using iterator = typename FixedSizeArray::iterator;

    CappedArray() = default;

    // Delete copy constructor/assignment.
    CappedArray(CappedArray const& rhs) = delete;
    CappedArray& operator=(CappedArray& rhs) = delete;

    CappedArray(CappedArray&& rhs) noexcept {
        this->swap(rhs);
    }

    CappedArray& operator=(CappedArray&& rhs) noexcept {
        this->swap(rhs);
        return *this;
    }

    inline ~CappedArray() {
        clear();
    }

    inline const_iterator begin() const {
        if (mInd == 0) {
            return mArray.cend();
        }
        return mArray.cbegin();
    }

    inline const_iterator end() const {
        if (mInd > 0 && mInd < CAPACITY) {
            return mArray.begin() + mInd;
        }
        return mArray.cend();
    }

    inline iterator begin() {
        if (mInd == 0) {
            return mArray.end();
        }
        return mArray.begin();
    }

    inline iterator end() {
        if (mInd > 0 && mInd < CAPACITY) {
            return mArray.begin() + mInd;
        }
        return mArray.end();
    }

    inline T back() {
        assert_invariant(mInd > 0);
        return *(mArray.begin() + mInd);
    }

    inline void pop_back() {
        assert_invariant(mInd > 0);
        mInd--;
    }

    inline const_iterator find(T item) {
        return std::find(begin(), end(), item);
    }

    inline void insert(T item) {
        assert_invariant(mInd < CAPACITY);
        mArray[mInd++] = item;
    }

    inline void erase(T item) {
        PANIC_PRECONDITION("CappedArray::erase should not be called");
    }

    inline void clear() {
        if (mInd == 0) {
            return;
        }
        mInd = 0;
    }

    inline T& operator[](uint16_t ind) {
        return mArray[ind];
    }

    inline T const& operator[](uint16_t ind) const {
        return mArray[ind];
    }

    inline uint32_t size() const {
        return mInd;
    }

    T* data() {
        return mArray.data();
    }

    T const* data() const {
        return mArray.data();
    }

    bool operator==(CappedArray const& b) const {
        return this->mArray == b.mArray;
    }

private:
    void swap(CappedArray& rhs) {
        std::swap(mArray, rhs.mArray);
        std::swap(mInd, rhs.mInd);
    }

    FixedSizeArray mArray;
    uint32_t mInd = 0;
};

// TODO: ok to remove once Filament-side API is complete
namespace descset {

using UniformBufferBitmask = uint64_t;
using SamplerBitmask = uint64_t;

// We only have at most one input attachment, so this bitmask exists only to make the code more
// general.
using InputAttachmentBitmask = uint64_t;

constexpr UniformBufferBitmask UBO_VERTEX_STAGE = 0x1;
constexpr UniformBufferBitmask UBO_FRAGMENT_STAGE = (0x1ULL << (sizeof(UniformBufferBitmask) * 4));
constexpr SamplerBitmask SAMPLER_VERTEX_STAGE = 0x1;
constexpr SamplerBitmask SAMPLER_FRAGMENT_STAGE = (0x1ULL << (sizeof(SamplerBitmask) * 4));
constexpr InputAttachmentBitmask INPUT_ATTACHMENT_VERTEX_STAGE = 0x1;
constexpr InputAttachmentBitmask INPUT_ATTACHMENT_FRAGMENT_STAGE =
        (0x1ULL << (sizeof(InputAttachmentBitmask) * 4));


template<typename Bitmask>
static constexpr Bitmask getVertexStage() noexcept {
    if constexpr (std::is_same_v<Bitmask, UniformBufferBitmask>) {
        return UBO_VERTEX_STAGE;
    }
    if constexpr (std::is_same_v<Bitmask, SamplerBitmask>) {
        return SAMPLER_VERTEX_STAGE;
    }
    if constexpr (std::is_same_v<Bitmask, InputAttachmentBitmask>) {
        return INPUT_ATTACHMENT_VERTEX_STAGE;
    }
}

template<typename Bitmask>
static constexpr Bitmask getFragmentStage() noexcept {
    if constexpr (std::is_same_v<Bitmask, UniformBufferBitmask>) {
        return UBO_FRAGMENT_STAGE;
    }
    if constexpr (std::is_same_v<Bitmask, SamplerBitmask>) {
        return SAMPLER_FRAGMENT_STAGE;
    }
    if constexpr (std::is_same_v<Bitmask, InputAttachmentBitmask>) {
        return INPUT_ATTACHMENT_FRAGMENT_STAGE;
    }
}

} // namespace descset

namespace {
// Use constexpr to statically generate a bit count table for 8-bit numbers.
struct _BitCountHelper {
    constexpr _BitCountHelper() : data{} {
        for (uint16_t i = 0; i < 256; ++i) {
            data[i] = 0;
            for (auto j = i; j > 0; j /= 2) {
                if (j & 1) {
                    data[i]++;
                }
            }
        }
    }

    template<typename MaskType>
    constexpr uint8_t count(MaskType num) {
        uint8_t count = 0;
        for (uint8_t i = 0; i < sizeof(MaskType) * 8; i+=8) {
            count += data[(num >> i) & 0xFF];
        }
        return count;
    }

private:
    uint8_t data[256];
};
} // namespace anonymous

template<typename MaskType>
inline uint8_t countBits(MaskType num) {
    static _BitCountHelper BitCounter = {};
    return BitCounter.count(num);
}

// This is useful for counting the total number of descriptors for both vertex and fragment stages.
template<typename MaskType>
inline MaskType collapseStages(MaskType mask) {
    constexpr uint8_t NBITS_DIV_2 = sizeof(MaskType) * 4;
    // First zero out the top-half and then or the bottom-half against the original top-half.
    return ((mask << NBITS_DIV_2) >> NBITS_DIV_2) | (mask >> NBITS_DIV_2);
}

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANUTILITY_H
