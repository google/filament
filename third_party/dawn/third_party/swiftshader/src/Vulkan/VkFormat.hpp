// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VK_FORMAT_HPP_
#define VK_FORMAT_HPP_

#include "System/Types.hpp"
#include "Vulkan/VulkanPlatform.hpp"

#include <vector>

namespace vk {

class Format
{
public:
	Format() {}
	Format(VkFormat format)
	    : format(format)
	{}
	inline operator VkFormat() const { return format; }

	bool isUnsignedNormalized() const;
	bool isSignedNormalized() const;
	bool isSignedUnnormalizedInteger() const;
	bool isUnsignedUnnormalizedInteger() const;
	bool isUnnormalizedInteger() const;
	bool isUnsigned() const;

	VkImageAspectFlags getAspects() const;
	Format getAspectFormat(VkImageAspectFlags aspect) const;
	VkFormat getClearFormat() const;
	bool isStencil() const;
	bool isDepth() const;
	bool isBGRformat() const;
	bool isSRGBformat() const;
	bool isFloatFormat() const;
	bool isYcbcrFormat() const;

	bool isCompatible(Format other) const;
	std::vector<Format> getCompatibleFormats() const;

	bool isCompressed() const;
	VkFormat getDecompressedFormat() const;
	int blockWidth() const;
	int blockHeight() const;
	int bytesPerBlock() const;

	int componentCount() const;
	bool isUnsignedComponent(int component) const;

	size_t bytes() const;
	size_t pitchB(int width, int border) const;
	size_t sliceB(int width, int height, int border) const;

	sw::float4 getScale() const;

	sw::int4 bitsPerComponent() const;

	bool supportsColorAttachmentBlend() const;

	// Texture sampling utilities
	bool has16bitPackedTextureFormat() const;
	bool has8bitTextureComponents() const;
	bool has16bitTextureComponents() const;
	bool has32bitIntegerTextureComponents() const;
	bool isRGBComponent(int component) const;

	static uint8_t mapTo8bit(VkFormat format);
	static VkFormat mapFrom8bit(uint8_t format);

private:
	VkFormat getCompatibilityClassRepresentative() const;
	size_t sliceBUnpadded(int width, int height, int border) const;

	VkFormat format = VK_FORMAT_UNDEFINED;
};

}  // namespace vk

#endif  // VK_FORMAT_HPP_
