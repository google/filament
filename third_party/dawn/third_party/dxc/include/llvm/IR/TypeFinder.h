//===-- llvm/IR/TypeFinder.h - Class to find used struct types --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the TypeFinder class. 
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_TYPEFINDER_H
#define LLVM_IR_TYPEFINDER_H

#include "llvm/ADT/DenseSet.h"
#include <vector>

namespace llvm {

class MDNode;
class Module;
class StructType;
class Type;
class Value;

/// TypeFinder - Walk over a module, identifying all of the types that are
/// used by the module.
class TypeFinder {
  // To avoid walking constant expressions multiple times and other IR
  // objects, we keep several helper maps.
  DenseSet<const Value*> VisitedConstants;
  DenseSet<const MDNode *> VisitedMetadata;
  DenseSet<Type*> VisitedTypes;

  std::vector<StructType*> StructTypes;
  bool OnlyNamed;

public:
  TypeFinder() : OnlyNamed(false) {}

  void run(const Module &M, bool onlyNamed);
  void clear();

  typedef std::vector<StructType*>::iterator iterator;
  typedef std::vector<StructType*>::const_iterator const_iterator;

  iterator begin() { return StructTypes.begin(); }
  iterator end() { return StructTypes.end(); }

  const_iterator begin() const { return StructTypes.begin(); }
  const_iterator end() const { return StructTypes.end(); }

  bool empty() const { return StructTypes.empty(); }
  size_t size() const { return StructTypes.size(); }
  iterator erase(iterator I, iterator E) { return StructTypes.erase(I, E); }

  StructType *&operator[](unsigned Idx) { return StructTypes[Idx]; }

private:
  /// incorporateType - This method adds the type to the list of used
  /// structures if it's not in there already.
  void incorporateType(Type *Ty);

  /// incorporateValue - This method is used to walk operand lists finding types
  /// hiding in constant expressions and other operands that won't be walked in
  /// other ways.  GlobalValues, basic blocks, instructions, and inst operands
  /// are all explicitly enumerated.
  void incorporateValue(const Value *V);

  /// incorporateMDNode - This method is used to walk the operands of an MDNode
  /// to find types hiding within.
  void incorporateMDNode(const MDNode *V);
};

} // end llvm namespace

#endif
