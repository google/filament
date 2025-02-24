//===- subzero/src/IceSwitchLowering.h - Switch lowering --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Helpers for switch lowering.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICESWITCHLOWERING_H
#define SUBZERO_SRC_ICESWITCHLOWERING_H

#include "IceDefs.h"
#include "IceStringPool.h"

#include <string>

namespace Ice {

class CaseCluster;

using CaseClusterArray = CfgVector<CaseCluster>;

/// A cluster of cases can be tested by a common method during switch lowering.
class CaseCluster {
  CaseCluster() = delete;

public:
  enum CaseClusterKind {
    Range,     /// Numerically adjacent case values with same target.
    JumpTable, /// Different targets and possibly sparse.
  };

  CaseCluster(const CaseCluster &) = default;
  CaseCluster &operator=(const CaseCluster &) = default;

  /// Create a cluster of a single case represented by a unitary range.
  CaseCluster(uint64_t Value, CfgNode *Target)
      : Kind(Range), Low(Value), High(Value), Target(Target) {}
  /// Create a case consisting of a jump table.
  CaseCluster(uint64_t Low, uint64_t High, InstJumpTable *JT)
      : Kind(JumpTable), Low(Low), High(High), JT(JT) {}

  CaseClusterKind getKind() const { return Kind; }
  uint64_t getLow() const { return Low; }
  uint64_t getHigh() const { return High; }
  CfgNode *getTarget() const {
    assert(Kind == Range);
    return Target;
  }
  InstJumpTable *getJumpTable() const {
    assert(Kind == JumpTable);
    return JT;
  }

  bool isUnitRange() const { return Low == High; }
  bool isPairRange() const { return Low == High - 1; }

  /// Discover cases which can be clustered together and return the clusters
  /// ordered by case value.
  static CaseClusterArray clusterizeSwitch(Cfg *Func, const InstSwitch *Instr);

private:
  CaseClusterKind Kind;
  uint64_t Low;
  uint64_t High;
  union {
    CfgNode *Target;   /// Target for a range.
    InstJumpTable *JT; /// Jump table targets.
  };

  /// Try and append a cluster returning whether or not it was successful.
  bool tryAppend(const CaseCluster &New);
};

/// Store the jump table data so that it can be emitted later in the correct ELF
/// section once the offsets from the start of the function are known.
class JumpTableData {
  JumpTableData() = delete;
  JumpTableData &operator=(const JumpTableData &) = delete;

public:
  using TargetList = std::vector<intptr_t>;

  JumpTableData(GlobalString Name, GlobalString FuncName, SizeT Id,
                const TargetList &TargetOffsets)
      : Name(Name), FuncName(FuncName), Id(Id), TargetOffsets(TargetOffsets) {}
  JumpTableData(const JumpTableData &) = default;
  JumpTableData(JumpTableData &&) = default;
  JumpTableData &operator=(JumpTableData &&) = default;

  GlobalString getName() const { return Name; }
  GlobalString getFunctionName() const { return FuncName; }
  SizeT getId() const { return Id; }
  const TargetList &getTargetOffsets() const { return TargetOffsets; }
  static std::string createSectionName(const GlobalString Name) {
    if (Name.hasStdString()) {
      return Name.toString() + "$jumptable";
    }
    return std::to_string(Name.getID()) + "$jumptable";
  }
  std::string getSectionName() const { return createSectionName(FuncName); }

private:
  GlobalString Name;
  GlobalString FuncName;
  SizeT Id;
  TargetList TargetOffsets;
};

using JumpTableDataList = std::vector<JumpTableData>;

} // end of namespace Ice

#endif //  SUBZERO_SRC_ICESWITCHLOWERING_H
