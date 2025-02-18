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

#include <cstdlib>
#include <memory>

#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/DawnTest.h"
#include "gtest/gtest.h"
#include "webgpu/webgpu_glfw.h"

// Include windows.h before GLFW so GLFW's APIENTRY macro doesn't conflict with windows.h's.
#if DAWN_PLATFORM_IS(WINDOWS)
#include "dawn/common/windows_with_undefs.h"
#endif  // DAWN_PLATFORM_IS(WINDOWS)

#include "GLFW/glfw3.h"

#if defined(DAWN_USE_X11)
#include "dawn/common/xlib_with_undefs.h"
#endif  // defined(DAWN_USE_X11)

#if defined(DAWN_ENABLE_BACKEND_METAL)
#include "dawn/utils/ObjCUtils.h"
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#include "GLFW/glfw3native.h"

namespace dawn {
namespace {

struct GLFWindowDestroyer {
    void operator()(GLFWwindow* ptr) { glfwDestroyWindow(ptr); }
};

// Test for wgpu::Surface creation that only need an instance (no devices) and don't need all the
// complexity of DawnTest.
class WindowSurfaceInstanceTests : public testing::Test {
  public:
    void SetUp() override {
        glfwSetErrorCallback([](int code, const char* message) {
            ErrorLog() << "GLFW error " << code << " " << message;
        });
        DAWN_TEST_UNSUPPORTED_IF(!glfwInit());

        dawnProcSetProcs(&native::GetProcs());

        mInstance = wgpu::CreateInstance();
    }

    void TearDown() override { mWindow.reset(); }

    void AssertSurfaceCreation(const wgpu::SurfaceDescriptor* descriptor, bool succeeds) {
        wgpu::Surface surface = mInstance.CreateSurface(descriptor);
        ASSERT_EQ(native::CheckIsErrorForTesting(surface.Get()), !succeeds);
    }

    GLFWwindow* CreateWindow() {
        // Set GLFW_NO_API to avoid GLFW bringing up a GL context that we won't use.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        mWindow.reset(
            glfwCreateWindow(400, 400, "WindowSurfaceInstanceTests window", nullptr, nullptr));
        return mWindow.get();
    }

  private:
    wgpu::Instance mInstance;
    std::unique_ptr<GLFWwindow, GLFWindowDestroyer> mWindow = nullptr;
};

// Test that a valid chained descriptor works (and that GLFWUtils creates a valid chained
// descriptor).
TEST_F(WindowSurfaceInstanceTests, ControlCase) {
    GLFWwindow* window = CreateWindow();
    auto chainedDescriptor = wgpu::glfw::SetupWindowAndGetSurfaceDescriptor(window);

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = chainedDescriptor.get();

    AssertSurfaceCreation(&descriptor, true);
}

// Test that just wgpu::SurfaceDescriptor isn't enough and needs a chained descriptor.
TEST_F(WindowSurfaceInstanceTests, NoChainedDescriptors) {
    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = nullptr;  // That's the default value but we set it for clarity.

    AssertSurfaceCreation(&descriptor, false);
}

// Test that a chained descriptor with a garbage sType produces an error.
TEST_F(WindowSurfaceInstanceTests, BadChainedDescriptors) {
    wgpu::ChainedStruct chainedDescriptor;
    chainedDescriptor.sType = wgpu::SType(0u);  // The default but we set it for clarity.

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = &chainedDescriptor;

    AssertSurfaceCreation(&descriptor, false);
}

// Test that a chained descriptor with HTMLCanvas produces an error.
TEST_F(WindowSurfaceInstanceTests, HTMLCanvasDescriptor) {
    wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector chainedDescriptor;
    chainedDescriptor.selector = "#myCanvas";

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = &chainedDescriptor;

    AssertSurfaceCreation(&descriptor, false);
}

// Test that it is invalid to give two valid chained descriptors
TEST_F(WindowSurfaceInstanceTests, TwoChainedDescriptors) {
    GLFWwindow* window = CreateWindow();
    auto chainedDescriptor1 = wgpu::glfw::SetupWindowAndGetSurfaceDescriptor(window);
    auto chainedDescriptor2 = wgpu::glfw::SetupWindowAndGetSurfaceDescriptor(window);

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = chainedDescriptor1.get();
    chainedDescriptor1->nextInChain = chainedDescriptor2.get();

    AssertSurfaceCreation(&descriptor, false);
}

#if DAWN_PLATFORM_IS(WINDOWS)

// Tests that GLFWUtils returns a descriptor of HWND type
TEST_F(WindowSurfaceInstanceTests, CorrectSTypeHWND) {
    GLFWwindow* window = CreateWindow();
    auto chainedDescriptor = wgpu::glfw::SetupWindowAndGetSurfaceDescriptor(window);
    ASSERT_EQ(chainedDescriptor->sType, wgpu::SType::SurfaceSourceWindowsHWND);
}

// Test with setting an invalid hwnd
TEST_F(WindowSurfaceInstanceTests, InvalidHWND) {
    wgpu::SurfaceSourceWindowsHWND chainedDescriptor;
    chainedDescriptor.hinstance = GetModuleHandle(nullptr);
    chainedDescriptor.hwnd = 0;  // This always is an invalid HWND value.

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = &chainedDescriptor;
    AssertSurfaceCreation(&descriptor, false);
}

#else  // DAWN_PLATFORM_IS(WINDOWS)

// Test using HWND when it is not supported
TEST_F(WindowSurfaceInstanceTests, HWNDSurfacesAreInvalid) {
    wgpu::SurfaceSourceWindowsHWND chainedDescriptor;
    chainedDescriptor.hinstance = nullptr;
    chainedDescriptor.hwnd = 0;

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = &chainedDescriptor;
    AssertSurfaceCreation(&descriptor, false);
}

#endif  // DAWN_PLATFORM_IS(WINDOWS)

#if defined(DAWN_USE_X11)

// Tests that GLFWUtils returns a descriptor of Xlib type
TEST_F(WindowSurfaceInstanceTests, CorrectSTypeXlib) {
    GLFWwindow* window = CreateWindow();
    auto chainedDescriptor = wgpu::glfw::SetupWindowAndGetSurfaceDescriptor(window);
    ASSERT_EQ(chainedDescriptor->sType, wgpu::SType::SurfaceSourceXlibWindow);
}

// Test with setting an invalid window
TEST_F(WindowSurfaceInstanceTests, InvalidXWindow) {
    wgpu::SurfaceSourceXlibWindow chainedDescriptor;
    chainedDescriptor.display = XOpenDisplay(nullptr);
    // From the "X Window System Protocol" "X Version 11, Release 6.8" page 2 at
    // https://www.x.org/releases/X11R7.5/doc/x11proto/proto.pdf
    //    WINDOW 32-bit value (top three bits guaranteed to be zero.
    // So UINT32_MAX should be an invalid window.
    chainedDescriptor.window = 0xFFFFFFFF;

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = &chainedDescriptor;
    AssertSurfaceCreation(&descriptor, false);
}

#else  // defined(DAWN_USE_X11)

// Test using Xlib when it is not supported
TEST_F(WindowSurfaceInstanceTests, XlibSurfacesAreInvalid) {
    wgpu::SurfaceSourceXlibWindow chainedDescriptor;
    chainedDescriptor.display = nullptr;
    chainedDescriptor.window = 0;

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = &chainedDescriptor;
    AssertSurfaceCreation(&descriptor, false);
}

#endif  // defined(DAWN_USE_X11)

#if defined(DAWN_ENABLE_BACKEND_METAL)

// Tests that GLFWUtils returns a descriptor of Metal type
TEST_F(WindowSurfaceInstanceTests, CorrectSTypeMetal) {
    GLFWwindow* window = CreateWindow();
    auto chainedDescriptor = wgpu::glfw::SetupWindowAndGetSurfaceDescriptor(window);
    ASSERT_EQ(chainedDescriptor->sType, wgpu::SType::SurfaceSourceMetalLayer);
}

// Test with setting an invalid layer
TEST_F(WindowSurfaceInstanceTests, InvalidMetalLayer) {
    auto caLayer = utils::CreatePlaceholderCALayer();

    wgpu::SurfaceSourceMetalLayer chainedDescriptor;
    chainedDescriptor.layer = caLayer.layer;

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = &chainedDescriptor;
    AssertSurfaceCreation(&descriptor, false);
}

#else  // defined(DAWN_ENABLE_BACKEND_METAL)

// Test using Metal when it is not supported
TEST_F(WindowSurfaceInstanceTests, MetalSurfacesAreInvalid) {
    wgpu::SurfaceSourceMetalLayer chainedDescriptor;
    chainedDescriptor.layer = nullptr;

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = &chainedDescriptor;
    AssertSurfaceCreation(&descriptor, false);
}

#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

}  // anonymous namespace
}  // namespace dawn
