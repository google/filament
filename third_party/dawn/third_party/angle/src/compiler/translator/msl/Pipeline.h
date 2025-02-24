//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_PIPELINE_H_
#define COMPILER_TRANSLATOR_MSL_PIPELINE_H_

#include "compiler/translator/Name.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/msl/ModifyStruct.h"
#include "compiler/translator/msl/SymbolEnv.h"

namespace sh
{

// Data that is scoped as `external` and `internal` for a given pipeline.
template <typename T>
struct PipelineScoped
{
    // Data that is configured to talk externally to the program.
    // May coincide with `internal`, but may also diverge from `internal`.
    const T *external = nullptr;

    // Extra data that is configured to talk externally to the program.
    // Used for framebuffer fetch and for adjusting Metal's InstanceId.
    const T *externalExtra = nullptr;

    // Data that is configured to talk internally within the program.
    // May coincide with `external`, but may also diverge from `external`.
    const T *internal = nullptr;

    // Returns true iff the input coincides with either `external` or `internal` data.
    bool matches(const T &object) const { return external == &object || internal == &object; }

    // Both `external` and `internal` representations are non-null.
    bool isTotallyFull() const { return external && internal; }

    // Both `external` and `internal` representations are null.
    bool isTotallyEmpty() const { return !external && !internal; }

    // Both `external` and `internal` representations are the same.
    bool isUniform() const { return external == internal; }
};

// Represents a high-level program pipeline.
class Pipeline
{
  public:
    enum class Type
    {
        VertexIn,
        VertexOut,
        FragmentIn,
        FragmentOut,
        UserUniforms,
        AngleUniforms,
        NonConstantGlobals,
        InvocationVertexGlobals,
        InvocationFragmentGlobals,
        UniformBuffer,
        Texture,
        Image,
        InstanceId,
    };

    enum class Variant
    {
        // For all internal pipeline uses.
        // For external pipeline uses if pipeline does not require splitting or saturation.
        Original,

        // Only for external pipeline uses if the pipeline was split or saturated.
        Modified,
    };

  public:
    // The type of the pipeline.
    Type type;

    // Non-null if a global instance of the pipeline struct already exists.
    // If non-null struct splitting should not be needed.
    const TVariable *globalInstanceVar;

  public:
    // Returns true iff the variable belongs to the pipeline.
    bool uses(const TVariable &var) const;

    // Returns the name for the struct type that stores variables of this pipeline.
    Name getStructTypeName(Variant variant) const;

    // Returns the name for the struct instance that stores variables of this pipeline.
    Name getStructInstanceName(Variant variant) const;

    ModifyStructConfig externalStructModifyConfig() const;

    // Returns true if the pipeline always requires a non-parameter local instance declaration of
    // the pipeline structures.
    bool alwaysRequiresLocalVariableDeclarationInMain() const;

    // Returns true iff the pipeline is an output pipeline. The external pipeline structure should
    // be returned from `main`.
    bool isPipelineOut() const;

    AddressSpace externalAddressSpace() const;
};

// A collection of various pipeline structures.
struct PipelineStructs : angle::NonCopyable
{
    PipelineScoped<TStructure> fragmentIn;
    PipelineScoped<TStructure> fragmentOut;
    PipelineScoped<TStructure> vertexIn;
    PipelineScoped<TStructure> vertexOut;
    PipelineScoped<TStructure> userUniforms;
    PipelineScoped<TStructure> angleUniforms;
    PipelineScoped<TStructure> nonConstantGlobals;
    PipelineScoped<TStructure> invocationVertexGlobals;
    PipelineScoped<TStructure> invocationFragmentGlobals;
    PipelineScoped<TStructure> uniformBuffers;
    PipelineScoped<TStructure> texture;
    PipelineScoped<TStructure> image;
    PipelineScoped<TStructure> instanceId;

    bool matches(const TStructure &s, bool internal, bool external) const;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_PIPELINE_H_
