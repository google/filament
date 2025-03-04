///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilPdbInfoWriter.h                                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
// Helper for writing a valid hlsl::DxilShaderPDBInfo part
//
#pragma once

#include "dxc/Support/Global.h"
#include <vector>

struct IMalloc;

namespace hlsl {

HRESULT WritePdbInfoPart(IMalloc *pMalloc, const void *pUncompressedPdbInfoData,
                         size_t size, std::vector<char> *outBuffer);

}
