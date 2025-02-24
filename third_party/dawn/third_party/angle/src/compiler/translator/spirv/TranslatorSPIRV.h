//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TranslatorSPIRV:
//   A set of transformations that prepare the AST to be compatible with GL_KHR_vulkan_glsl followed
//   by a pass that generates SPIR-V.
//   See: https://www.khronos.org/registry/vulkan/specs/misc/GL_KHR_vulkan_glsl.txt
//

#ifndef COMPILER_TRANSLATOR_SPIRV_TRANSLATORSPIRV_H_
#define COMPILER_TRANSLATOR_SPIRV_TRANSLATORSPIRV_H_

#include "common/hash_containers.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

class TOutputVulkanGLSL;
class SpecConst;
class DriverUniform;

// An index -> TVariable map, tracking the declarated color input attachments, as well as TVariables
// for depth and stencil input attachments.
struct InputAttachmentMap
{
    TUnorderedMap<uint32_t, const TVariable *> color;
    const TVariable *depth   = nullptr;
    const TVariable *stencil = nullptr;
};

class TranslatorSPIRV final : public TCompiler
{
  public:
    TranslatorSPIRV(sh::GLenum type, ShShaderSpec spec);

    void assignSpirvId(TSymbolUniqueId uniqueId, uint32_t spirvId);

  protected:
    [[nodiscard]] bool translate(TIntermBlock *root,
                                 const ShCompileOptions &compileOptions,
                                 PerformanceDiagnostics *perfDiagnostics) override;
    bool shouldFlattenPragmaStdglInvariantAll() override;

    [[nodiscard]] bool translateImpl(TIntermBlock *root,
                                     const ShCompileOptions &compileOptions,
                                     PerformanceDiagnostics *perfDiagnostics,
                                     SpecConst *specConst,
                                     DriverUniform *driverUniforms);
    void assignInputAttachmentIds(const InputAttachmentMap &inputAttachmentMap);
    void assignSpirvIds(TIntermBlock *root);

    // A map from TSymbolUniqueId::mId to SPIR-V reserved ids.  Used by the SPIR-V generator to
    // quickly know when to use a reserved id and not have to resort to name matching.
    angle::HashMap<int, uint32_t> mUniqueToSpirvIdMap;
    uint32_t mFirstUnusedSpirvId;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_SPIRV_TRANSLATORSPIRV_H_
