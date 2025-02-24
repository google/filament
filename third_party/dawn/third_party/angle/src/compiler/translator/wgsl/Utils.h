//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_WGSL_UTILS_H_
#define COMPILER_TRANSLATOR_WGSL_UTILS_H_

#include "compiler/translator/Common.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Types.h"

namespace sh
{

// Can be used with TSymbol or TField or TFunc.
template <typename StringStreamType, typename Object>
void WriteNameOf(StringStreamType &output, const Object &namedObject)
{
    WriteNameOf(output, namedObject.symbolType(), namedObject.name());
}

template <typename StringStreamType>
void WriteNameOf(StringStreamType &output, SymbolType symbolType, const ImmutableString &name);

enum class WgslAddressSpace
{
    Uniform,
    NonUniform
};

struct EmitTypeConfig
{
    // If `addressSpace` is WgslAddressSpace::Uniform, all arrays with stride not a multiple of 16
    // will need a wrapper struct for the array element type that is of size a multiple of 16, if
    // the array element type that is not already a struct. This is to satisfy WGSL's uniform
    // address space layout constraints.
    WgslAddressSpace addressSpace = WgslAddressSpace::NonUniform;
};

template <typename StringStreamType>
void WriteWgslBareTypeName(StringStreamType &output,
                           const TType &type,
                           const EmitTypeConfig &config);
template <typename StringStreamType>
void WriteWgslType(StringStreamType &output, const TType &type, const EmitTypeConfig &config);

// From the type, creates a legal WGSL name for a struct that wraps it.
ImmutableString MakeUniformWrapperStructName(const TType *type);

// Returns true if a `type` in the uniform address space is an array that needs its element type
// wrapped in a struct.
bool ElementTypeNeedsUniformWrapperStruct(bool inUniformAddressSpace, const TType *type);

using GlobalVars = TMap<ImmutableString, TIntermDeclaration *>;
GlobalVars FindGlobalVars(TIntermBlock *root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_WGSL_UTILS_H_
