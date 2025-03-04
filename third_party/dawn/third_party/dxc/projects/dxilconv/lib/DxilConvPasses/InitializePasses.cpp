///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// InitializePasses.cpp                                                      //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Initialization of transformation passes used in DirectX DXBC to DXIL      //
// converter.                                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "DxilConvPasses/DxilCleanup.h"
#include "DxilConvPasses/NormalizeDxil.h"
#include "DxilConvPasses/ScopeNestInfo.h"
#include "DxilConvPasses/ScopeNestedCFG.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"

#include "llvm/PassRegistry.h"

using namespace llvm;

// Place to put our private pass initialization for opt.exe.
void __cdecl initializeDxilConvPasses(PassRegistry &Registry) {
  initializeScopeNestedCFGPass(Registry);
  initializeScopeNestInfoWrapperPassPass(Registry);
  initializeNormalizeDxilPassPass(Registry);
  initializeDxilCleanupPass(Registry);
}

namespace hlsl {
HRESULT SetupRegistryPassForDxilConvPasses() {
  try {
    PassRegistry &Registry = *PassRegistry::getPassRegistry();
    initializeDxilConvPasses(Registry);
  }
  CATCH_CPP_RETURN_HRESULT();
  return S_OK;
}
} // namespace hlsl