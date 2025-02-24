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

#include "MetalSurface.hpp"
#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkImage.hpp"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#include <AppKit/NSView.h>

namespace vk {

class MetalLayer
{
public:
    void initWithLayer(const void* pLayer) API_AVAILABLE(macosx(10.11))
    {
        view = nullptr;
        layer = nullptr;

        id<NSObject> obj = (id<NSObject>)pLayer;

        if(!NSThread.isMainThread)
        {
            UNREACHABLE("MetalLayer::init(): not called from main thread");
        }
        if([obj isKindOfClass: [CAMetalLayer class]])
        {
            layer = (CAMetalLayer*)[obj retain];
            layer.framebufferOnly = false;
            layer.device = MTLCreateSystemDefaultDevice();
        }
        else
        {
            UNREACHABLE("MetalLayer::init(): view doesn't have metal backed layer");
        }
    }

    void initWithView(const void* pView) API_AVAILABLE(macosx(10.11))
    {
        view = nullptr;
        layer = nullptr;

        id<NSObject> obj = (id<NSObject>)pView;

        if([obj isKindOfClass: [NSView class]])
        {
            NSView* objView = (NSView*)[obj retain];

            initWithLayer(objView.layer);

            view = objView;
        }
    }

    void release() API_AVAILABLE(macosx(10.11))
    {
        if(layer)
        {
            [layer.device release];
            [layer release];
        }
        if(view)
        {
            [view release];
        }
    }

    // Synchronizes the drawableSize to layer.bounds.size * layer.contentsScale and returns the new value of
    // drawableSize.
    VkExtent2D syncExtent() const API_AVAILABLE(macosx(10.11))
    {
        if(layer)
        {
            CGSize drawSize = layer.bounds.size;
            CGFloat scaleFactor = layer.contentsScale;
            drawSize.width = trunc(drawSize.width * scaleFactor);
            drawSize.height = trunc(drawSize.height * scaleFactor);

            [layer setDrawableSize: drawSize];

            return { static_cast<uint32_t>(drawSize.width), static_cast<uint32_t>(drawSize.height) };
        }
        else
        {
            return { 0, 0 };
        }
    }

    id<CAMetalDrawable> getNextDrawable() const API_AVAILABLE(macosx(10.11))
    {
        if(layer)
        {
            return [layer nextDrawable];
        }

        return nil;
    }

    VkExtent2D getDrawableSize() const API_AVAILABLE(macosx(10.11)) {
        if (layer) {
            return {
                static_cast<uint32_t>([layer drawableSize].width),
                static_cast<uint32_t>([layer drawableSize].height),
            };
        }
        return {0, 0};
    }

private:
    NSView* view;
    CAMetalLayer* layer API_AVAILABLE(macosx(10.11));
};

MetalSurface::MetalSurface(const void *pCreateInfo, void *mem) : metalLayer(reinterpret_cast<MetalLayer*>(mem))
{

}

void MetalSurface::destroySurface(const VkAllocationCallbacks *pAllocator) API_AVAILABLE(macosx(10.11))
{
    if(metalLayer)
    {
        metalLayer->release();
    }

    vk::freeHostMemory(metalLayer, pAllocator);
}

size_t MetalSurface::ComputeRequiredAllocationSize(const void *pCreateInfo) API_AVAILABLE(macosx(10.11))
{
    return sizeof(MetalLayer);
}

VkResult MetalSurface::getSurfaceCapabilities(const void *pSurfaceInfoPNext,
                                              VkSurfaceCapabilitiesKHR *pSurfaceCapabilities,
                                              void *pSurfaceCapabilitiesPNext) const
    API_AVAILABLE(macosx(10.11))
{
    // The value of drawableSize in CAMetalLayer is set the first time a drawable is queried but after that it is the
    // (Metal) application's responsibility to resize the drawable when the window is resized. The best time for Swiftshader
    // to resize the drawable is when querying the capabilities of the swapchain as that's done when the Vulkan application
    // is trying to handle a window resize.
    VkExtent2D extent = metalLayer->syncExtent();
    pSurfaceCapabilities->currentExtent = extent;
    pSurfaceCapabilities->minImageExtent = extent;
    pSurfaceCapabilities->maxImageExtent = extent;

    SetCommonSurfaceCapabilities(pSurfaceInfoPNext, pSurfaceCapabilities,
                                 pSurfaceCapabilitiesPNext);
    return VK_SUCCESS;
}

VkResult MetalSurface::present(PresentImage* image) API_AVAILABLE(macosx(10.11))
{
    @autoreleasepool
    {
        auto drawable = metalLayer->getNextDrawable();
        if(drawable)
        {
            const VkExtent3D &extent = image->getImage()->getExtent();
            VkExtent2D drawableExtent = metalLayer->getDrawableSize();

            if(drawableExtent.width != extent.width || drawableExtent.height != extent.height)
            {
                return VK_ERROR_OUT_OF_DATE_KHR;
            }

            [drawable.texture replaceRegion:MTLRegionMake2D(0, 0, extent.width, extent.height)
                              mipmapLevel:0
                              withBytes:image->getImageMemory()->getOffsetPointer(0)
                              bytesPerRow:image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0)];
            [drawable present];

        }
    }
    return VK_SUCCESS;
}

#ifdef VK_USE_PLATFORM_METAL_EXT
MetalSurfaceEXT::MetalSurfaceEXT(const VkMetalSurfaceCreateInfoEXT *pCreateInfo, void *mem) API_AVAILABLE(macosx(10.11))
 : MetalSurface(pCreateInfo, mem)
{
    metalLayer->initWithLayer(pCreateInfo->pLayer);
}
#endif

#ifdef VK_USE_PLATFORM_MACOS_MVK
MacOSSurfaceMVK::MacOSSurfaceMVK(const VkMacOSSurfaceCreateInfoMVK *pCreateInfo, void *mem) API_AVAILABLE(macosx(10.11))
 : MetalSurface(pCreateInfo, mem)
{
    metalLayer->initWithView(pCreateInfo->pView);
}
#endif

}
