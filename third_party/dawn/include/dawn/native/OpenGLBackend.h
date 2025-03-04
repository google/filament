// Copyright 2018 The Dawn & Tint Authors
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

#ifndef INCLUDE_DAWN_NATIVE_OPENGLBACKEND_H_
#define INCLUDE_DAWN_NATIVE_OPENGLBACKEND_H_

#include "dawn/native/DawnNative.h"
#include "webgpu/webgpu_cpp_chained_struct.h"

namespace dawn::native::opengl {

using EGLDisplay = void*;
using EGLImage = void*;
using GLuint = unsigned int;

// Define a GetProc function pointer that mirrors the one in egl.h
#if defined(_WIN32)
#define DAWN_STDCALL __stdcall
#else  // defined(_WIN32)
#define DAWN_STDCALL
#endif  // defined(_WIN32)

using EGLFunctionPointerType = void (*)();
// NOLINTNEXTLINE(readability/casting): cpplint thinks this is a C-style cast but it isn't.
using EGLGetProcProc = EGLFunctionPointerType(DAWN_STDCALL*)(const char*);
#undef DAWN_STDCALL

// Can be chained in WGPURequestAdapterOptions
struct DAWN_NATIVE_EXPORT RequestAdapterOptionsGetGLProc : wgpu::ChainedStruct {
    RequestAdapterOptionsGetGLProc();

    EGLGetProcProc getProc;
    EGLDisplay display;
};

struct DAWN_NATIVE_EXPORT ExternalImageDescriptorEGLImage : ExternalImageDescriptor {
  public:
    ExternalImageDescriptorEGLImage();

    EGLImage image;
};

DAWN_NATIVE_EXPORT WGPUTexture
WrapExternalEGLImage(WGPUDevice device, const ExternalImageDescriptorEGLImage* descriptor);

struct DAWN_NATIVE_EXPORT ExternalImageDescriptorGLTexture : ExternalImageDescriptor {
  public:
    ExternalImageDescriptorGLTexture();

    GLuint texture;
};

DAWN_NATIVE_EXPORT WGPUTexture
WrapExternalGLTexture(WGPUDevice device, const ExternalImageDescriptorGLTexture* descriptor);

}  // namespace dawn::native::opengl

#endif  // INCLUDE_DAWN_NATIVE_OPENGLBACKEND_H_
