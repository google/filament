//===- subzero/src/IceSwitchLowering.cpp - Switch lowering ----------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements platform independent analysis of switch cases to improve
/// the generated code.
///
//===----------------------------------------------------------------------===//
#include "IceSwitchLowering.h"

#include "IceCfgNode.h"
#include "IceTargetLowering.h"

#include <algorithm>

namespace Ice {

CaseClusterArray CaseCluster::clusterizeSwitch(Cfg *Func,
                                               const InstSwitch *Instr) {
  const SizeT NumCases = Instr->getNumCases();
  CaseClusterArray CaseClusters;
  CaseClusters.reserve(NumCases);

  // Load the cases
  CaseClusters.reserve(NumCases);
  for (SizeT I = 0; I < NumCases; ++I)
    CaseClusters.emplace_back(Instr->getValue(I), Instr->getLabel(I));

  // Sort the cases
  std::sort(CaseClusters.begin(), CaseClusters.end(),
            [](const CaseCluster &x, const CaseCluster &y) {
              return x.High < y.Low;
            });

  // Merge adjacent case ranges
  auto Active = CaseClusters.begin();
  std::for_each(Active + 1, CaseClusters.end(),
                [&Active](const CaseCluster &x) {
                  if (!Active->tryAppend(x))
                    *(++Active) = x;
                });
  CaseClusters.erase(Active + 1, CaseClusters.end());

  // TODO(ascull): Merge in a cycle i.e. -1(=UINTXX_MAX) to 0. This depends on
  // the types for correct wrap around behavior.

  // A small number of cases is more efficient without a jump table
  if (CaseClusters.size() < Func->getTarget()->getMinJumpTableSize())
    return CaseClusters;

  // Test for a single jump table. This can be done in constant time whereas
  // finding the best set of jump table would be quadratic, too slow(?). If
  // jump tables were included in the search tree we'd first have to traverse
  // to them. Ideally we would have an unbalanced tree which is biased towards
  // frequently executed code but we can't do this well without profiling data.
  // So, this single jump table is a good starting point where you can get to
  // the jump table quickly without figuring out how to unbalance the tree.
  const uint64_t MaxValue = CaseClusters.back().High;
  const uint64_t MinValue = CaseClusters.front().Low;
  // Don't +1 yet to avoid (INT64_MAX-0)+1 overflow
  const uint64_t Range = MaxValue - MinValue;

  // Might be too sparse for the jump table
  if (NumCases * 2 <= Range)
    return CaseClusters;
  // Unlikely. Would mean can't store size of jump table.
  if (Range == UINT64_MAX)
    return CaseClusters;
  const uint64_t TotalRange = Range + 1;

  // Replace everything with a jump table
  auto *JumpTable =
      InstJumpTable::create(Func, TotalRange, Instr->getLabelDefault());
  for (const CaseCluster &Case : CaseClusters) {
    // Case.High could be UINT64_MAX which makes the loop awkward. Unwrap the
    // last iteration to avoid wrap around problems.
    for (uint64_t I = Case.Low; I < Case.High; ++I)
      JumpTable->addTarget(I - MinValue, Case.Target);
    JumpTable->addTarget(Case.High - MinValue, Case.Target);
    Case.Target->setNeedsAlignment();
  }
  Func->addJumpTable(JumpTable);

  CaseClusters.clear();
  CaseClusters.emplace_back(MinValue, MaxValue, JumpTable);

  return CaseClusters;
}

bool CaseCluster::tryAppend(const CaseCluster &New) {
  // Can only append ranges with the same target and are adjacent
  const bool CanAppend =
      this->Target == New.Target && this->High + 1 == New.Low;
  if (CanAppend)
    this->High = New.High;
  return CanAppend;
}

} // end of namespace Ice
