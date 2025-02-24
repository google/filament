//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_REWRITEPIPELINES_H_
#define COMPILER_TRANSLATOR_MSL_REWRITEPIPELINES_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/msl/IdGen.h"
#include "compiler/translator/msl/Pipeline.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{
class DriverUniform;

// This rewrites all pipelines.
//
// For each pipeline:
//    - Discover all variables that are used by the pipeline
//    - Move the variables into an internal pipeline struct instance and update old variables to be
//      member access instead.
//    - Dependency inject the internal pipeline struct to all functions that access variables from
//      the struct.
//    - A new external pipeline struct is created if needed for impedance reasons. Otherwise the
//      external and internal pipeline structs are the same.
//    - Add `main` parameter or return value for the external pipeline struct as needed.
//    - Inside `main`, map the external struct to the internal struct if they differ and is provided
//      as a parameter to `main`.
//    - Inside `main`, map the internal struct to the external struct if they differ and is returned
//      from `main`.
[[nodiscard]] bool RewritePipelines(TCompiler &compiler,
                                    TIntermBlock &root,
                                    const std::vector<sh::ShaderVariable> &inputVaryings,
                                    const std::vector<sh::ShaderVariable> &outputVariables,
                                    IdGen &idGen,
                                    DriverUniform &angleUniformsGlobalInstanceVar,
                                    SymbolEnv &symbolEnv,
                                    PipelineStructs &outStructs);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_REWRITEPIPELINES_H_
