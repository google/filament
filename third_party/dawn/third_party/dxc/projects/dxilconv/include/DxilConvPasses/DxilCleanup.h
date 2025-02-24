///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilCleanup.cpp                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Optimization of DXIL after conversion from DXBC.                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace llvm {
class PassRegistry;
class ModulePass;

extern char &DxilCleanupID;

llvm::ModulePass *createDxilCleanupPass();

void initializeDxilCleanupPass(llvm::PassRegistry &);

} // namespace llvm
