#include "apple_wsi.h"
#include <vulkan/utility/vk_struct_helper.hpp>

// Needed to get the CAMetalLayer
#import <QuartzCore/CAMetalLayer.h>

namespace vkt {

// This function is based off of glfw's WSI code for MoltenVK.
// Main exception is we do link against the QuartzCore framework.
// https://github.com/glfw/glfw/blob/3.3.8/src/cocoa_window.m#L1832
VkMetalSurfaceCreateInfoEXT CreateMetalSurfaceInfoEXT() {
    VkMetalSurfaceCreateInfoEXT info = vku::InitStructHelper();
    info.pLayer = [CAMetalLayer layer];

    return info;
}

}  // namespace vkt
