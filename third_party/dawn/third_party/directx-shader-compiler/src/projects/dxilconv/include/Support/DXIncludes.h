///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DXIncludes.h                                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// This is a common include for DXBC/Windows related things.                 //
//                                                                           //
// IMPORTANT: do not add LLVM/Clang or DXIL files to this file.              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// This is a platform-specific file.
// Do not add LLVM/Clang or DXIL files to this file.
// clang-format off
// Includes on Windows are highly order dependent.
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1
#include <windows.h>
#include <strsafe.h>

#include <dxgitype.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3d12.h>
#include "dxc/Support/d3dx12.h"
#include "DxbcSignatures.h"
#include <d3dcompiler.h>
#include <wincrypt.h>

#ifndef DECODE_D3D10_SB_TOKENIZED_PROGRAM_TYPE
#include "dxc\Support\d3d12TokenizedProgramFormat.hpp"
#endif

#include "ShaderBinary/ShaderBinary.h"
// clang-format on
