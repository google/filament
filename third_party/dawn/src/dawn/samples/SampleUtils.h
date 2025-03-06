// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_SAMPLES_SAMPLEUTILS_H_
#define SRC_DAWN_SAMPLES_SAMPLEUTILS_H_

#ifndef __EMSCRIPTEN__
#include "GLFW/glfw3.h"
#endif  // __EMSCRIPTEN__

#include <webgpu/webgpu_cpp.h>

bool InitSample(int argc, const char** argv);

class SampleBase {
  public:
    SampleBase();
    SampleBase(uint32_t w, uint32_t h);

    static int Run(unsigned int delay);

  protected:
    virtual bool SetupImpl() = 0;
    virtual void FrameImpl() = 0;

    wgpu::Instance instance = nullptr;
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;
    wgpu::Queue queue = nullptr;

    wgpu::Surface surface = nullptr;

    wgpu::TextureFormat GetPreferredSurfaceTextureFormat() { return preferredSurfaceTextureFormat; }

  private:
    bool Setup();

    static constexpr uint32_t kWidth = 640;
    static constexpr uint32_t kHeight = 480;
    uint32_t width = kWidth;
    uint32_t height = kHeight;
    wgpu::TextureFormat preferredSurfaceTextureFormat = wgpu::TextureFormat::BGRA8Unorm;

#ifndef __EMSCRIPTEN__
    GLFWwindow* window = nullptr;
#endif  // __EMSCRIPTEN__
};

#endif  // SRC_DAWN_SAMPLES_SAMPLEUTILS_H_
