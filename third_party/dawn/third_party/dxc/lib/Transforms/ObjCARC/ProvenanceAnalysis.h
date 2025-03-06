//===- ProvenanceAnalysis.h - ObjC ARC Optimization ---*- C++ -*-----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file declares a special form of Alias Analysis called ``Provenance
/// Analysis''. The word ``provenance'' refers to the history of the ownership
/// of an object. Thus ``Provenance Analysis'' is an analysis which attempts to
/// use various techniques to determine if locally
///
/// WARNING: This file knows about certain library functions. It recognizes them
/// by name, and hardwires knowledge of their semantics.
///
/// WARNING: This file knows about how certain Objective-C library functions are
/// used. Naive LLVM IR transformations which would otherwise be
/// behavior-preserving may break these assumptions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_OBJCARC_PROVENANCEANALYSIS_H
#define LLVM_LIB_TRANSFORMS_OBJCARC_PROVENANCEANALYSIS_H

#include "llvm/ADT/DenseMap.h"

namespace llvm {
  class Value;
  class AliasAnalysis;
  class DataLayout;
  class PHINode;
  class SelectInst;
}

namespace llvm {
namespace objcarc {

/// \brief This is similar to BasicAliasAnalysis, and it uses many of the same
/// techniques, except it uses special ObjC-specific reasoning about pointer
/// relationships.
///
/// In this context ``Provenance'' is defined as the history of an object's
/// ownership. Thus ``Provenance Analysis'' is defined by using the notion of
/// an ``independent provenance source'' of a pointer to determine whether or
/// not two pointers have the same provenance source and thus could
/// potentially be related.
class ProvenanceAnalysis {
  AliasAnalysis *AA;

  typedef std::pair<const Value *, const Value *> ValuePairTy;
  typedef DenseMap<ValuePairTy, bool> CachedResultsTy;
  CachedResultsTy CachedResults;

  bool relatedCheck(const Value *A, const Value *B, const DataLayout &DL);
  bool relatedSelect(const SelectInst *A, const Value *B);
  bool relatedPHI(const PHINode *A, const Value *B);

  void operator=(const ProvenanceAnalysis &) = delete;
  ProvenanceAnalysis(const ProvenanceAnalysis &) = delete;

public:
  ProvenanceAnalysis() {}

  void setAA(AliasAnalysis *aa) { AA = aa; }

  AliasAnalysis *getAA() const { return AA; }

  bool related(const Value *A, const Value *B, const DataLayout &DL);

  void clear() {
    CachedResults.clear();
  }
};

} // end namespace objcarc
} // end namespace llvm

#endif
