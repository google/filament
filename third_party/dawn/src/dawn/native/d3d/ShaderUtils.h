// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D_SHADEUTILS_H_
#define SRC_DAWN_NATIVE_D3D_SHADEUTILS_H_

#include <bitset>
#include <string>
#include <vector>

#include "dawn/native/Blob.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/d3d/D3DCompilationRequest.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

class Device;

#define COMPILED_SHADER_MEMBERS(X) \
    X(Blob, shaderBlob)            \
    X(std::string, hlslSource)     \
    X(bool, usesVertexIndex)       \
    X(bool, usesInstanceIndex)

// `CompiledShader` holds a ref to one of the various representations of shader blobs and
// information used to emulate vertex/instance index starts. It also holds the `hlslSource` for the
// shader compilation, which is only transiently available during Compile, and cleared before it
// returns. It is not written to or loaded from the cache unless Toggle dump_shaders is true.
DAWN_SERIALIZABLE(struct, CompiledShader, COMPILED_SHADER_MEMBERS){};
#undef COMPILED_SHADER_MEMBERS

std::string CompileFlagsToString(uint32_t compileFlags);

ResultOrError<CompiledShader> CompileShader(d3d::D3DCompilationRequest r);

InterStageShaderVariablesMask ToInterStageShaderVariablesMask(const std::vector<bool>& inputMask);

void DumpFXCCompiledShader(Device* device,
                           const CompiledShader& compiledShader,
                           uint32_t compileFlags);

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_SHADEUTILS_H_
