// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#if !defined(DAWN_ENABLE_BACKEND_METAL)
#error "utils_metal.mm requires the Metal backend to be enabled."
#endif  // !defined(DAWN_ENABLE_BACKEND_METAL)

#include "webgpu/webgpu_glfw.h"

#import <QuartzCore/CAMetalLayer.h>
#include "GLFW/glfw3.h"

#include <cstdlib>

#define GLFW_EXPOSE_NATIVE_COCOA
#include "GLFW/glfw3native.h"

namespace wgpu::glfw {

std::unique_ptr<wgpu::ChainedStruct, void (*)(wgpu::ChainedStruct*)>
SetupWindowAndGetSurfaceDescriptorCocoa(GLFWwindow* window) {
    @autoreleasepool {
        NSWindow* nsWindow = glfwGetCocoaWindow(window);
        NSView* view = [nsWindow contentView];

        // Create a CAMetalLayer that covers the whole window that will be passed to
        // CreateSurface.
        [view setWantsLayer:YES];
        [view setLayer:[CAMetalLayer layer]];

        // Use retina if the window was created with retina support.
        [[view layer] setContentsScale:[nsWindow backingScaleFactor]];

        wgpu::SurfaceSourceMetalLayer* desc = new wgpu::SurfaceSourceMetalLayer();
        desc->layer = [view layer];
        return {desc, [](wgpu::ChainedStruct* desc) {
                    delete reinterpret_cast<wgpu::SurfaceSourceMetalLayer*>(desc);
                }};
    }
}

}  // namespace wgpu::glfw
