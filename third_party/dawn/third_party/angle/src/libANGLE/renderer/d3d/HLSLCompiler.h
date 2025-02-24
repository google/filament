//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// HLSLCompiler: Wrapper for the D3DCompiler DLL.
//

#ifndef LIBANGLE_RENDERER_D3D_HLSLCOMPILER_H_
#define LIBANGLE_RENDERER_D3D_HLSLCOMPILER_H_

#include "libANGLE/Error.h"

#include "common/angleutils.h"
#include "common/platform.h"

#include <string>
#include <vector>

namespace gl
{
class InfoLog;
}  // namespace gl

namespace rx
{
namespace d3d
{
class Context;
}  // namespace d3d

struct CompileConfig
{
    UINT flags;
    std::string name;

    CompileConfig();
    CompileConfig(UINT flags, const std::string &name);
};

class HLSLCompiler : angle::NonCopyable
{
  public:
    HLSLCompiler();
    ~HLSLCompiler();

    void release();

    // Attempt to compile a HLSL shader using the supplied configurations, may output a NULL
    // compiled blob even if no GL errors are returned.
    angle::Result compileToBinary(d3d::Context *context,
                                  gl::InfoLog &infoLog,
                                  const std::string &hlsl,
                                  const std::string &profile,
                                  const std::vector<CompileConfig> &configs,
                                  const D3D_SHADER_MACRO *overrideMacros,
                                  ID3DBlob **outCompiledBlob,
                                  std::string *outDebugInfo);

    angle::Result disassembleBinary(d3d::Context *context,
                                    ID3DBlob *shaderBinary,
                                    std::string *disassemblyOut);
    angle::Result ensureInitialized(d3d::Context *context);

  private:
    bool mInitialized;
    HMODULE mD3DCompilerModule;
    pD3DCompile mD3DCompileFunc;
    pD3DDisassemble mD3DDisassembleFunc;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_HLSLCOMPILER_H_
