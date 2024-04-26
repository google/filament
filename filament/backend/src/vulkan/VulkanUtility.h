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

// An Array that will be fixed capacity, but the "size" (as in user added elements) is variable.
// Note that this class is movable.
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

// Used to describe the descriptor binding in shader stages. We assume that the binding index does
// not exceed 31. We also assume that we have two shader stages - vertex and fragment.  The below
// types and struct are used across VulkanDescriptorSet and VulkanProgram.
using UniformBufferBitmask = uint32_t;
using SamplerBitmask = uint64_t;

// We only have at most one input attachment, so this bitmask exists only to make the code more
// general.
using InputAttachmentBitmask = uint8_t;

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

typedef enum ShaderStageFlags2 : uint8_t {
    NONE        =    0,
    VERTEX      =    0x1,
    FRAGMENT    =    0x2,
} ShaderStageFlags2;

enum class DescriptorType : uint8_t {
    UNIFORM_BUFFER,
    SAMPLER,
    INPUT_ATTACHMENT,
};

enum class DescriptorFlags : uint8_t {
    NONE = 0x00,
    DYNAMIC_OFFSET = 0x01
};

struct DescriptorSetLayoutBinding {
    DescriptorType type;
    ShaderStageFlags2 stageFlags;
    uint8_t binding;
    DescriptorFlags flags;
    uint16_t count;
};

struct DescriptorSetLayout {
    utils::FixedCapacityVector<DescriptorSetLayoutBinding> bindings;
};

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
