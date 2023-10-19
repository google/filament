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
VkComponentMapping getSwizzleMap(TextureSwizzle swizzle[4]);
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
#define EXPAND_ENUM(...)\
    uint32_t size = 0;\
    VkResult result = func(__VA_ARGS__, nullptr);\
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "enumerate size error");\
    utils::FixedCapacityVector<OutType> ret(size);\
    result = func(__VA_ARGS__, ret.data());\
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "enumerate error");\
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

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANUTILITY_H
