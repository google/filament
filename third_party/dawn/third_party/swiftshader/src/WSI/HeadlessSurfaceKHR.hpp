// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#ifndef SWIFTSHADER_HEADLESSSURFACEKHR_HPP
#define SWIFTSHADER_HEADLESSSURFACEKHR_HPP

#include "VkSurfaceKHR.hpp"

namespace vk {

class HeadlessSurfaceKHR : public SurfaceKHR, public ObjectBase<HeadlessSurfaceKHR, VkSurfaceKHR>
{
public:
	HeadlessSurfaceKHR(const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo, void *mem);

	static size_t ComputeRequiredAllocationSize(const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo);

	void destroySurface(const VkAllocationCallbacks *pAllocator) override;
	VkResult getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const override;
	void attachImage(PresentImage *image) override;
	void detachImage(PresentImage *image) override;
	VkResult present(PresentImage *image) override;
};

}  // namespace vk
#endif  // SWIFTSHADER_HEADLESSSURFACEKHR_HPP
