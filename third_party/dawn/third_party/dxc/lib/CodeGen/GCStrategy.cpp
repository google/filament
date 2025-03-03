//===-- GCStrategy.cpp - Garbage Collector Description --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the policy object GCStrategy which describes the
// behavior of a given garbage collector.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/GCStrategy.h"

using namespace llvm;

GCStrategy::GCStrategy()
    : UseStatepoints(false), NeededSafePoints(0), CustomReadBarriers(false),
      CustomWriteBarriers(false), CustomRoots(false), InitRoots(true),
      UsesMetadata(false) {}
