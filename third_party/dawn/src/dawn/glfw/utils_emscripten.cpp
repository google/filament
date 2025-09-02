// Copyright 2025 The Dawn & Tint Authors
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

#if !defined(EMSCRIPTEN)
#error "utils_emscripten.cpp: This file requires EMSCRIPTEN to be defined."
#endif  // !defined(EMSCRIPTEN)

#include <memory>

#include "GLFW/glfw3.h"
#include "emscripten/emscripten.h"
#include "webgpu/webgpu_glfw.h"

WGPU_GLFW_EXPORT WGPUSurface wgpuGlfwCreateSurfaceForWindow(const WGPUInstance instance,
                                                            GLFWwindow* window) {
    wgpu::Surface s = wgpu::glfw::CreateSurfaceForWindow(instance, window);
    return s.MoveToCHandle();
}

namespace wgpu::glfw {

wgpu::Surface CreateSurfaceForWindow(const wgpu::Instance& instance, GLFWwindow* window) {
    auto chainedDescriptor = SetupWindowAndGetSurfaceDescriptor(window);

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = chainedDescriptor.get();
    wgpu::Surface surface = instance.CreateSurface(&descriptor);

    return surface;
}

std::unique_ptr<wgpu::ChainedStruct, void (*)(wgpu::ChainedStruct*)>
SetupWindowAndGetSurfaceDescriptor(GLFWwindow* window) {
    if (glfwGetWindowAttrib(window, GLFW_CLIENT_API) != GLFW_NO_API) {
        emscripten_log(EM_LOG_ERROR,
                       "GL context was created on the window. Disable context creation by "
                       "setting the GLFW_CLIENT_API hint to GLFW_NO_API.");
        return {nullptr, [](wgpu::ChainedStruct*) {}};
    }

    wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector* desc =
        new wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector();
    // Map "!canvas" CSS selector to the canvas held in the Module.canvas object.
    EM_ASM({self.specialHTMLTargets && (specialHTMLTargets["!canvas"] = Module.canvas)});
    desc->selector = "!canvas";
    return {desc, [](wgpu::ChainedStruct* desc) {
                delete reinterpret_cast<wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector*>(desc);
            }};
}

}  // namespace wgpu::glfw
