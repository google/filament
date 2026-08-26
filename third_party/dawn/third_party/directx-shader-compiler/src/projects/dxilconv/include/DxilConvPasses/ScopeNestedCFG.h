///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ScopeNestedCFG.cpp                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Pass that converts a reducible CFG into scope-nested CFG.                 //
// The pass expects that the following passes have been run                  //
// right before the pass is invoked:                                         //
//   -simplifycfg                                                            //
//   -loop-simplify                                                          //
//   -reg2mem_hlsl                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace llvm {
class Module;
class Function;
class PassRegistry;
class FunctionPass;

llvm::FunctionPass *createScopeNestedCFGPass();
void initializeScopeNestedCFGPass(llvm::PassRegistry &);

llvm::FunctionPass *createLoopSimplifyFunctionPass();
void initializeLoopSimplifyFunctionPass(llvm::PassRegistry &);

enum class BranchKind {
  Invalid = 0,

  IfBegin,
  IfEnd,
  IfNoEnd,

  SwitchBegin,
  SwitchEnd,
  SwitchNoEnd,
  SwitchBreak,

  LoopBegin,
  LoopExit,
  LoopNoEnd,
  LoopBreak,
  LoopContinue,
  LoopBackEdge,
};

} // namespace llvm
