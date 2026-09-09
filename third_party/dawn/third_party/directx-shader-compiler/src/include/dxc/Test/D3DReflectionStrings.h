///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// D3DReflectionStrings.h                                                    //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Used to convert reflection data types into strings.                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/WinAdapter.h"

#include "dxc/DxilContainer/DxilRuntimeReflection.h"
#include "dxc/Support/D3DReflection.h"

namespace hlsl {
using namespace RDAT;
namespace dump {

// ToString functions for D3D types
LPCSTR ToString(D3D_CBUFFER_TYPE CBType);
LPCSTR ToString(D3D_SHADER_INPUT_TYPE Type);
LPCSTR ToString(D3D_RESOURCE_RETURN_TYPE ReturnType);
LPCSTR ToString(D3D_SRV_DIMENSION Dimension);
LPCSTR ToString(D3D_PRIMITIVE_TOPOLOGY GSOutputTopology);
LPCSTR ToString(D3D_PRIMITIVE InputPrimitive);
LPCSTR ToString(D3D_TESSELLATOR_OUTPUT_PRIMITIVE HSOutputPrimitive);
LPCSTR ToString(D3D_TESSELLATOR_PARTITIONING HSPartitioning);
LPCSTR ToString(D3D_TESSELLATOR_DOMAIN TessellatorDomain);
LPCSTR ToString(D3D_SHADER_VARIABLE_CLASS Class);
LPCSTR ToString(D3D_SHADER_VARIABLE_TYPE Type);
LPCSTR ToString(D3D_SHADER_VARIABLE_FLAGS Flag);
LPCSTR ToString(D3D_SHADER_INPUT_FLAGS Flag);
LPCSTR ToString(D3D_SHADER_CBUFFER_FLAGS Flag);
LPCSTR ToString(D3D_PARAMETER_FLAGS Flag);
LPCSTR ToString(D3D_NAME Name);
LPCSTR ToString(D3D_REGISTER_COMPONENT_TYPE CompTy);
LPCSTR ToString(D3D_MIN_PRECISION MinPrec);
LPCSTR CompMaskToString(unsigned CompMask);

// These macros declare the ToString functions for DXC types
#define DEF_RDAT_ENUMS DEF_RDAT_DUMP_DECL
#define DEF_DXIL_ENUMS DEF_RDAT_DUMP_DECL
#include "dxc/DxilContainer/RDAT_Macros.inl"

} // namespace dump
} // namespace hlsl
