///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilSignatureElement.h                                                    //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Validate HLSL signature element packing.                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilSigPoint.h"
#include "dxc/DXIL/DxilSignature.h"
#include "dxc/HLSL/DxilSignatureAllocator.h"
#include "dxc/Support/Global.h"

using namespace hlsl;
using namespace llvm;

#include <assert.h> // Needed for DxilPipelineStateValidation.h
#include <functional>

#include "dxc/DxilContainer/DxilPipelineStateValidation.h"
#include "dxc/HLSL/ViewIDPipelineValidation.inl"