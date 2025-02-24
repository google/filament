//===- subzero/src/IceLoopAnalyzer.h - Loop Analysis ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This analysis identifies loops in the CFG.
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICELOOPANALYZER_H
#define SUBZERO_SRC_ICELOOPANALYZER_H

#include "IceDefs.h"

namespace Ice {

struct Loop {
  Loop(CfgNode *Header, CfgNode *PreHeader, CfgUnorderedSet<SizeT> Body)
      : Header(Header), PreHeader(PreHeader), Body(Body) {}
  CfgNode *Header;
  CfgNode *PreHeader;
  CfgUnorderedSet<SizeT> Body; // Node IDs
};

CfgVector<Loop> ComputeLoopInfo(Cfg *Func);

} // end of namespace Ice

#endif //  SUBZERO_SRC_ICELOOPANALYZER_H
