//===- InstIterator.h - Classes for inst iteration --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains definitions of two iterators for iterating over the
// instructions in a function.  This is effectively a wrapper around a two level
// iterator that can probably be genericized later.
//
// Note that this iterator gets invalidated any time that basic blocks or
// instructions are moved around.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INSTITERATOR_H
#define LLVM_IR_INSTITERATOR_H

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"

namespace llvm {

// This class implements inst_begin() & inst_end() for
// inst_iterator and const_inst_iterator's.
//
template <class BB_t, class BB_i_t, class BI_t, class II_t> class InstIterator {
  typedef BB_t BBty;
  typedef BB_i_t BBIty;
  typedef BI_t BIty;
  typedef II_t IIty;
  BB_t *BBs; // BasicBlocksType
  BB_i_t BB; // BasicBlocksType::iterator
  BI_t BI;   // BasicBlock::iterator
public:
  typedef std::bidirectional_iterator_tag iterator_category;
  typedef IIty                            value_type;
  typedef signed                        difference_type;
  typedef IIty*                           pointer;
  typedef IIty&                           reference;

  // Default constructor
  InstIterator() {}

  // Copy constructor...
  template<typename A, typename B, typename C, typename D>
  InstIterator(const InstIterator<A,B,C,D> &II)
    : BBs(II.BBs), BB(II.BB), BI(II.BI) {}

  template<typename A, typename B, typename C, typename D>
  InstIterator(InstIterator<A,B,C,D> &II)
    : BBs(II.BBs), BB(II.BB), BI(II.BI) {}

  template<class M> InstIterator(M &m)
    : BBs(&m.getBasicBlockList()), BB(BBs->begin()) {    // begin ctor
    if (BB != BBs->end()) {
      BI = BB->begin();
      advanceToNextBB();
    }
  }

  template<class M> InstIterator(M &m, bool)
    : BBs(&m.getBasicBlockList()), BB(BBs->end()) {    // end ctor
  }

  // Accessors to get at the underlying iterators...
  inline BBIty &getBasicBlockIterator()  { return BB; }
  inline BIty  &getInstructionIterator() { return BI; }

  inline reference operator*()  const { return *BI; }
  inline pointer operator->() const { return &operator*(); }

  inline bool operator==(const InstIterator &y) const {
    return BB == y.BB && (BB == BBs->end() || BI == y.BI);
  }
  inline bool operator!=(const InstIterator& y) const {
    return !operator==(y);
  }

  InstIterator& operator++() {
    ++BI;
    advanceToNextBB();
    return *this;
  }
  inline InstIterator operator++(int) {
    InstIterator tmp = *this; ++*this; return tmp;
  }

  InstIterator& operator--() {
    while (BB == BBs->end() || BI == BB->begin()) {
      --BB;
      BI = BB->end();
    }
    --BI;
    return *this;
  }
  inline InstIterator  operator--(int) {
    InstIterator tmp = *this; --*this; return tmp;
  }

  inline bool atEnd() const { return BB == BBs->end(); }

private:
  inline void advanceToNextBB() {
    // The only way that the II could be broken is if it is now pointing to
    // the end() of the current BasicBlock and there are successor BBs.
    while (BI == BB->end()) {
      ++BB;
      if (BB == BBs->end()) break;
      BI = BB->begin();
    }
  }
};


typedef InstIterator<iplist<BasicBlock>,
                     Function::iterator, BasicBlock::iterator,
                     Instruction> inst_iterator;
typedef InstIterator<const iplist<BasicBlock>,
                     Function::const_iterator,
                     BasicBlock::const_iterator,
                     const Instruction> const_inst_iterator;

inline inst_iterator inst_begin(Function *F) { return inst_iterator(*F); }
inline inst_iterator inst_end(Function *F)   { return inst_iterator(*F, true); }
inline iterator_range<inst_iterator> inst_range(Function *F) {
  return iterator_range<inst_iterator>(inst_begin(F), inst_end(F));
}
inline const_inst_iterator inst_begin(const Function *F) {
  return const_inst_iterator(*F);
}
inline const_inst_iterator inst_end(const Function *F) {
  return const_inst_iterator(*F, true);
}
inline iterator_range<const_inst_iterator> inst_range(const Function *F) {
  return iterator_range<const_inst_iterator>(inst_begin(F), inst_end(F));
}
inline inst_iterator inst_begin(Function &F) { return inst_iterator(F); }
inline inst_iterator inst_end(Function &F)   { return inst_iterator(F, true); }
inline iterator_range<inst_iterator> inst_range(Function &F) {
  return iterator_range<inst_iterator>(inst_begin(F), inst_end(F));
}
inline const_inst_iterator inst_begin(const Function &F) {
  return const_inst_iterator(F);
}
inline const_inst_iterator inst_end(const Function &F) {
  return const_inst_iterator(F, true);
}
inline iterator_range<const_inst_iterator> inst_range(const Function &F) {
  return iterator_range<const_inst_iterator>(inst_begin(F), inst_end(F));
}

} // End llvm namespace

#endif
