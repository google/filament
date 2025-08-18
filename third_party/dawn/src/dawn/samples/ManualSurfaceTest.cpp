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

// This is an example to manually test surface code. Controls are the following, scoped to the
// currently focused window:
//  - W: creates a new window.
//  - L: Latches the current surface, to check what happens when the window changes but not the
//    surface.
//  - R: switches the rendering mode, between "The Red Triangle" and color-cycling clears that's
//    (WARNING) likely seizure inducing.
//  - D: cycles the divisor for the surface size.
//  - P: switches present modes.
//  - A: switches alpha modes.
//  - F: switches formats.
//
// Closing all the windows exits the example. ^C also works.
//
// Things to test manually:
//
//  - Basic tests (with the triangle render mode):
//    - Check the triangle is red on a black background and with the pointy side up.
//    - Cycle render modes a bunch and check that the triangle background is always solid black.
//    - Check that rendering triangles to multiple windows works.
//
//  - Present mode single-window tests (with cycling color render mode):
//    - Check that Fifo cycles at about 1 cycle per second and has no tearing.
//    - Check that Mailbox cycles faster than Fifo and has no tearing.
//    - Check that Immediate cycles faster than Fifo, it is allowed to have tearing. (dragging
//      between two monitors can help see tearing)
//
//  - Present mode multi-window tests, it should have the same results as single-window tests when
//    all windows are in the same present mode. In mixed present modes only Immediate windows are
//    allowed to tear.
//
//  - Resizing tests (with the triangle render mode):
//    - Check that cycling divisors on the triangle produces lower and lower resolution triangles.
//    - Check latching the surface config and resizing the window a bunch (smaller, bigger, and
//      diagonal aspect ratio).
//
//  - Config change tests:
//    - Check that cycling between present modes.
//    - Check that cycling between alpha modes (it sometimes produce a meaningful difference).
//    - Check that cycling between formats works and gives the same color.
//
//  - Frame throttling:
//    - In all present modes, check that there isn't extra latency when going from a render mode
//      to another (like from the color cycling one to the triangle).
//
//  - Additional things to test that aren't supported in this file yet.
//    - Check cycling the same window over multiple devices.
//    - Check sRGB vs not sRGB gradients.
//    - Check wide gamut / extended color range.
//    - Check OpenGL rendering with extra usages / depth buffer / MRT.
//    - Check with GLFW transparency on / off.

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "GLFW/glfw3.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/dawn_proc.h"  // nogncheck
#include "dawn/native/DawnNative.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/CommandLineParser.h"
#include "dawn/utils/WGPUHelpers.h"
#include "dawn/webgpu_cpp_print.h"
#include "webgpu/webgpu_glfw.h"

template <typename T>
std::string NoPrefix(T wgpuThing) {
    std::ostringstream o;
    o << wgpuThing;
    std::string withPrefix = o.str();
    return withPrefix.substr(withPrefix.rfind(':') + 1);
}

template <typename T>
void CycleIn(T* value, const std::vector<T>& cycle) {
    auto it = std::find(cycle.begin(), cycle.end(), *value);
    DAWN_ASSERT(it != cycle.end());
    it++;
    if (it != cycle.end()) {
        *value = *it;
    } else {
        *value = cycle.front();
    }
}

struct WindowData {
    GLFWwindow* window = nullptr;
    uint64_t serial = 0;

    float clearCycle = 1.0f;
    bool latched = false;
    bool renderTriangle = true;
    uint32_t divisor = 1;

    std::vector<wgpu::PresentMode> presentModes;
    std::vector<wgpu::CompositeAlphaMode> alphaModes;
    std::vector<wgpu::TextureFormat> formats;

    wgpu::Surface surface = nullptr;
    wgpu::SurfaceConfiguration currentConfig;
    wgpu::SurfaceConfiguration targetConfig;
};

static std::unordered_map<GLFWwindow*, std::unique_ptr<WindowData>> windows;
static uint64_t windowSerial = 0;

static wgpu::Instance instance;
static wgpu::Adapter adapter;
static wgpu::Device device;
static wgpu::Queue queue;

static std::unordered_map<wgpu::TextureFormat, wgpu::RenderPipeline> trianglePipelines;
wgpu::RenderPipeline GetOrCreateTrianglePipeline(wgpu::TextureFormat format) {
    if (trianglePipelines.contains(format)) {
        return trianglePipelines[format];
    }

    // The hacky pipeline to render a triangle.
    wgpu::ShaderModule module = dawn::utils::CreateShaderModule(device, R"(
        @vertex fn vs(@builtin(vertex_index) VertexIndex : u32)
                            -> @builtin(position) vec4f {
            var pos = array(
                vec2f( 0.0,  0.5),
                vec2f(-0.5, -0.5),
                vec2f( 0.5, -0.5)
            );
            return vec4f(pos[VertexIndex], 0, 1);
        }

        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(1, 0, 0, 1);
        }
    )");

    dawn::utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = module;
    pipelineDesc.cFragment.module = module;
    pipelineDesc.cTargets[0].format = format;
    trianglePipelines[format] = device.CreateRenderPipeline(&pipelineDesc);
    return trianglePipelines[format];
}

bool IsSameConfig(const wgpu::SurfaceConfiguration& a, const wgpu::SurfaceConfiguration& b) {
    DAWN_ASSERT(a.viewFormatCount == 0);
    DAWN_ASSERT(b.viewFormatCount == 0);
    return a.device.Get() == b.device.Get() &&  //
           a.format == b.format &&              //
           a.usage == b.usage &&                //
           a.alphaMode == b.alphaMode &&        //
           a.width == b.width &&                //
           a.height == b.height &&              //
           a.presentMode == b.presentMode;
}

void OnKeyPress(GLFWwindow* window, int key, int, int action, int);

void SyncFromWindow(WindowData* data) {
    int width;
    int height;
    glfwGetFramebufferSize(data->window, &width, &height);

    data->targetConfig.width = std::max(1u, width / data->divisor);
    data->targetConfig.height = std::max(1u, height / data->divisor);
}

void AddWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(400, 400, "", nullptr, nullptr);
    glfwSetKeyCallback(window, OnKeyPress);

    wgpu::Surface surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);
    wgpu::SurfaceCapabilities caps;
    surface.GetCapabilities(adapter, &caps);

    wgpu::SurfaceConfiguration config;
    config.device = device;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.format = caps.formats[0];
    config.alphaMode = caps.alphaModes[0];
    config.presentMode = caps.presentModes[0];
    config.width = 0;
    config.height = 0;

    std::unique_ptr<WindowData> data = std::make_unique<WindowData>();
    data->window = window;
    data->serial = windowSerial++;
    data->surface = surface;
    data->currentConfig = config;
    data->targetConfig = config;
    SyncFromWindow(data.get());
    data->presentModes.assign(caps.presentModes, caps.presentModes + caps.presentModeCount);
    data->alphaModes.assign(caps.alphaModes, caps.alphaModes + caps.alphaModeCount);
    data->formats.assign(caps.formats, caps.formats + caps.formatCount);

    windows[window] = std::move(data);
}

void DoRender(WindowData* data) {
    wgpu::SurfaceTexture surfaceTexture;
    data->surface.GetCurrentTexture(&surfaceTexture);
    wgpu::TextureView view = surfaceTexture.texture.CreateView();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    if (data->renderTriangle) {
        dawn::utils::ComboRenderPassDescriptor desc({view});
        // Use Load to check the surface is lazy cleared (we shouldn't see garbage from previous
        // frames).
        desc.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
        pass.SetPipeline(GetOrCreateTrianglePipeline(data->currentConfig.format));
        pass.Draw(3);
        pass.End();
    } else {
        data->clearCycle -= 1.0 / 60.f;
        if (data->clearCycle < 0.0) {
            data->clearCycle = 1.0f;
        }

        dawn::utils::ComboRenderPassDescriptor desc({view});
        desc.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        desc.cColorAttachments[0].clearValue = {data->clearCycle, 1.0f - data->clearCycle, 0.0f,
                                                1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    wgpu::Status presentStatus = data->surface.Present();
    DAWN_ASSERT(presentStatus == wgpu::Status::Success);
}

std::ostream& operator<<(std::ostream& o, const wgpu::SurfaceConfiguration& desc) {
    // For now only render attachment is possible.
    DAWN_ASSERT(desc.usage == wgpu::TextureUsage::RenderAttachment);
    o << "RenderAttachment ";

    o << desc.width << "x" << desc.height << " ";
    o << NoPrefix(desc.format) << " ";
    o << NoPrefix(desc.presentMode) << " ";
    o << NoPrefix(desc.alphaMode) << " ";
    return o;
}

void UpdateTitle(WindowData* data) {
    std::ostringstream o;

    o << data->serial << " ";
    if (data->divisor != 1) {
        o << "Divisor:" << data->divisor << " ";
    }

    if (data->latched) {
        o << "Latched: (" << data->currentConfig << ") ";
        o << "Target: (" << data->targetConfig << ")";
    } else {
        o << "(" << data->currentConfig << ")";
    }

    glfwSetWindowTitle(data->window, o.str().c_str());
}

void OnKeyPress(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) {
        return;
    }

    DAWN_ASSERT(windows.contains(window));

    WindowData* data = windows[window].get();
    switch (key) {
        case GLFW_KEY_W:
            AddWindow();
            break;

        case GLFW_KEY_L:
            data->latched = !data->latched;
            UpdateTitle(data);
            break;

        case GLFW_KEY_R:
            data->renderTriangle = !data->renderTriangle;
            UpdateTitle(data);
            break;

        case GLFW_KEY_D:
            data->divisor *= 2;
            if (data->divisor > 32) {
                data->divisor = 1;
            }
            break;

        case GLFW_KEY_P:
            CycleIn(&data->targetConfig.presentMode, data->presentModes);
            break;

        case GLFW_KEY_A:
            CycleIn(&data->targetConfig.alphaMode, data->alphaModes);
            break;

        case GLFW_KEY_F:
            CycleIn(&data->targetConfig.format, data->formats);
            break;

        default:
            break;
    }
}

int main(int argc, const char* argv[]) {
    // Parse command line options.
    dawn::utils::CommandLineParser parser;
    auto& helpOpt = parser.AddHelp();
    auto& enableTogglesOpt = parser.AddStringList("enable-toggles", "Toggles to enable in Dawn")
                                 .ShortName('e')
                                 .Parameter("comma separated list");
    auto& disableTogglesOpt = parser.AddStringList("disable-toggles", "Toggles to disable in Dawn")
                                  .ShortName('d')
                                  .Parameter("comma separated list");
    auto& backendOpt =
        parser
            .AddEnum<wgpu::BackendType>({{"d3d11", wgpu::BackendType::D3D11},
                                         {"d3d12", wgpu::BackendType::D3D12},
                                         {"metal", wgpu::BackendType::Metal},
                                         {"null", wgpu::BackendType::Null},
                                         {"opengl", wgpu::BackendType::OpenGL},
                                         {"opengles", wgpu::BackendType::OpenGLES},
                                         {"vulkan", wgpu::BackendType::Vulkan}},
                                        "backend", "The backend to get an adapter from")
            .ShortName('b')
            .Default(wgpu::BackendType::Undefined);

    auto parserResult = parser.Parse(argc, argv);
    if (!parserResult.success) {
        std::cerr << parserResult.errorMessage << "\n";
        return 1;
    }

    if (helpOpt.GetValue()) {
        std::cout << "Usage: " << argv[0] << " <options>\n\noptions\n";
        parser.PrintHelp(std::cout);
        return 0;
    }

    // Setup GLFW, Dawn procs
    glfwSetErrorCallback([](int code, const char* message) {
        dawn::ErrorLog() << "GLFW error " << code << " " << message;
    });
    if (!glfwInit()) {
        return 1;
    }

    DawnProcTable procs = dawn::native::GetProcs();
    dawnProcSetProcs(&procs);

    // Make an instance with the toggles.
    std::vector<const char*> enableToggleNames;
    std::vector<const char*> disabledToggleNames;
    for (auto toggle : enableTogglesOpt.GetValue()) {
        enableToggleNames.push_back(toggle.c_str());
    }

    for (auto toggle : disableTogglesOpt.GetValue()) {
        disabledToggleNames.push_back(toggle.c_str());
    }
    wgpu::DawnTogglesDescriptor toggles;
    toggles.enabledToggles = enableToggleNames.data();
    toggles.enabledToggleCount = enableToggleNames.size();
    toggles.disabledToggles = disabledToggleNames.data();
    toggles.disabledToggleCount = disabledToggleNames.size();

    wgpu::InstanceDescriptor instanceDescriptor{};
    instanceDescriptor.nextInChain = &toggles;
    static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
    instanceDescriptor.requiredFeatureCount = 1;
    instanceDescriptor.requiredFeatures = &kTimedWaitAny;
    instance = wgpu::CreateInstance(&instanceDescriptor);

    // Choose an adapter we like.
    // TODO(dawn:269): allow switching the window between devices.
    wgpu::RequestAdapterOptions options = {};
    options.backendType = backendOpt.GetValue();
    if (options.backendType != wgpu::BackendType::Undefined) {
        options.featureLevel = dawn::utils::BackendRequiresCompat(options.backendType)
                                   ? wgpu::FeatureLevel::Compatibility
                                   : wgpu::FeatureLevel::Core;
    }

    wgpu::Future adapterFuture = instance.RequestAdapter(
        &options, wgpu::CallbackMode::WaitAnyOnly,
        [&](wgpu::RequestAdapterStatus status, wgpu::Adapter adapterIn, wgpu::StringView message) {
            if (status != wgpu::RequestAdapterStatus::Success) {
                dawn::ErrorLog() << "Failed to get an adapter: " << message;
                return;
            }
            adapter = adapterIn;
        });
    instance.WaitAny(adapterFuture, UINT64_MAX);

    if (adapter == nullptr) {
        return 1;
    }

    wgpu::AdapterInfo adapterInfo;
    adapter.GetInfo(&adapterInfo);
    std::cout << "Using adapter \"" << adapterInfo.device << "\" on " << adapterInfo.backendType
              << ".\n";

    wgpu::DeviceDescriptor deviceDesc;
    deviceDesc.SetUncapturedErrorCallback(
        [](const wgpu::Device&, wgpu::ErrorType errorType, wgpu::StringView message) {
            dawn::ErrorLog() << errorType << " error: " << message;
            DAWN_ASSERT(false);
        });

    // Setup the device on that adapter.
    wgpu::Future deviceFuture = adapter.RequestDevice(
        &deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
        [&](wgpu::RequestDeviceStatus status, wgpu::Device deviceIn, wgpu::StringView message) {
            if (status != wgpu::RequestDeviceStatus::Success) {
                dawn::ErrorLog() << "Failed to get a device:" << message;
                return;
            }
            device = deviceIn;
        });
    instance.WaitAny(deviceFuture, UINT64_MAX);

    if (device == nullptr) {
        return 1;
    }

    queue = device.GetQueue();

    // Create the first window, since the example exits when there are no windows.
    AddWindow();

    while (windows.size() != 0) {
        glfwPollEvents();
        instance.ProcessEvents();

        for (auto it = windows.begin(); it != windows.end();) {
            GLFWwindow* window = it->first;

            if (glfwWindowShouldClose(window)) {
                glfwDestroyWindow(window);
                it = windows.erase(it);
            } else {
                it++;
            }
        }

        for (auto& it : windows) {
            WindowData* data = it.second.get();

            SyncFromWindow(data);
            if (!IsSameConfig(data->currentConfig, data->targetConfig) && !data->latched) {
                data->surface.Configure(&data->targetConfig);
                data->currentConfig = data->targetConfig;
            }
            UpdateTitle(data);
            DoRender(data);
        }
    }
}
