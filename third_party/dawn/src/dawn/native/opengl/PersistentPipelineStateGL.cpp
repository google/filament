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

#include "dawn/native/opengl/PersistentPipelineStateGL.h"

#include "dawn/native/opengl/OpenGLFunctions.h"

namespace dawn::native::opengl {

void PersistentPipelineState::SetDefaultState(const OpenGLFunctions& gl) {
    CallGLStencilFunc(gl);
}

void PersistentPipelineState::SetStencilFuncsAndMask(const OpenGLFunctions& gl,
                                                     GLenum stencilBackCompareFunction,
                                                     GLenum stencilFrontCompareFunction,
                                                     uint32_t stencilReadMask) {
    if (mStencilBackCompareFunction == stencilBackCompareFunction &&
        mStencilFrontCompareFunction == stencilFrontCompareFunction &&
        mStencilReadMask == stencilReadMask) {
        return;
    }

    mStencilBackCompareFunction = stencilBackCompareFunction;
    mStencilFrontCompareFunction = stencilFrontCompareFunction;
    mStencilReadMask = stencilReadMask;
    CallGLStencilFunc(gl);
}

void PersistentPipelineState::SetStencilReference(const OpenGLFunctions& gl,
                                                  uint32_t stencilReference) {
    if (mStencilReference == stencilReference) {
        return;
    }

    mStencilReference = stencilReference;
    CallGLStencilFunc(gl);
}

void PersistentPipelineState::CallGLStencilFunc(const OpenGLFunctions& gl) {
    gl.StencilFuncSeparate(GL_BACK, mStencilBackCompareFunction, mStencilReference,
                           mStencilReadMask);
    gl.StencilFuncSeparate(GL_FRONT, mStencilFrontCompareFunction, mStencilReference,
                           mStencilReadMask);
}

}  // namespace dawn::native::opengl
