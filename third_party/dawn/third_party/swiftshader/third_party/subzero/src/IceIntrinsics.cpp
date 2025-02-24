//===- subzero/src/IceIntrinsics.cpp - Functions related to intrinsics ----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the Intrinsics utilities for matching and then dispatching
/// by name.
///
//===----------------------------------------------------------------------===//

#include "IceIntrinsics.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IceStringPool.h"

#include <utility>

namespace Ice {

static_assert(sizeof(Intrinsics::IntrinsicInfo) == 4,
              "Unexpected sizeof(IntrinsicInfo)");

bool Intrinsics::isMemoryOrderValid(IntrinsicID ID, uint64_t Order,
                                    uint64_t OrderOther) {
  // Reject orderings not allowed by C++11.
  switch (ID) {
  default:
    llvm_unreachable("isMemoryOrderValid: Unknown IntrinsicID");
    return false;
  case AtomicFence:
  case AtomicFenceAll:
  case AtomicRMW:
    return true;
  case AtomicCmpxchg:
    // Reject orderings that are disallowed by C++11 as invalid combinations
    // for cmpxchg.
    switch (OrderOther) {
    case MemoryOrderRelaxed:
    case MemoryOrderConsume:
    case MemoryOrderAcquire:
    case MemoryOrderSequentiallyConsistent:
      if (OrderOther > Order)
        return false;
      if (Order == MemoryOrderRelease && OrderOther != MemoryOrderRelaxed)
        return false;
      return true;
    default:
      return false;
    }
  case AtomicLoad:
    switch (Order) {
    case MemoryOrderRelease:
    case MemoryOrderAcquireRelease:
      return false;
    default:
      return true;
    }
  case AtomicStore:
    switch (Order) {
    case MemoryOrderConsume:
    case MemoryOrderAcquire:
    case MemoryOrderAcquireRelease:
      return false;
    default:
      return true;
    }
  }
}

Intrinsics::ValidateIntrinsicValue
Intrinsics::FullIntrinsicInfo::validateIntrinsic(const InstIntrinsic *Intrinsic,
                                                 SizeT &ArgIndex) const {
  assert(NumTypes >= 1);
  Variable *Result = Intrinsic->getDest();
  if (Result == nullptr) {
    if (getReturnType() != IceType_void)
      return Intrinsics::BadReturnType;
  } else if (getReturnType() != Result->getType()) {
    return Intrinsics::BadReturnType;
  }
  if (Intrinsic->getNumArgs() != getNumArgs()) {
    return Intrinsics::WrongNumOfArgs;
  }
  for (size_t i = 1; i < NumTypes; ++i) {
    if (Intrinsic->getArg(i - 1)->getType() != Signature[i]) {
      ArgIndex = i - 1;
      return Intrinsics::WrongArgType;
    }
  }
  return Intrinsics::IsValidIntrinsic;
}

Type Intrinsics::FullIntrinsicInfo::getArgType(SizeT Index) const {
  assert(NumTypes > 1);
  assert(Index + 1 < NumTypes);
  return Signature[Index + 1];
}

} // end of namespace Ice
