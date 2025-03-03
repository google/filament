//===- llvm/ADT/IndexedMap.h - An index map implementation ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements an indexed map. The index map template takes two
// types. The first is the mapped type and the second is a functor
// that maps its argument to a size_t. On instantiation a "null" value
// can be provided to be used as a "does not exist" indicator in the
// map. A member function grow() is provided that given the value of
// the maximally indexed key (the argument of the functor) makes sure
// the map has enough space for it.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ADT_INDEXEDMAP_H
#define LLVM_ADT_INDEXEDMAP_H

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include <cassert>
#include <functional>

namespace llvm {

template <typename T, typename ToIndexT = llvm::identity<unsigned> >
  class IndexedMap {
    typedef typename ToIndexT::argument_type IndexT;
    // Prefer SmallVector with zero inline storage over std::vector. IndexedMaps
    // can grow very large and SmallVector grows more efficiently as long as T
    // is trivially copyable.
    typedef SmallVector<T, 0> StorageT;
    StorageT storage_;
    T nullVal_;
    ToIndexT toIndex_;

  public:
    IndexedMap() : nullVal_(T()) { }

    explicit IndexedMap(const T& val) : nullVal_(val) { }

    typename StorageT::reference operator[](IndexT n) {
      assert(toIndex_(n) < storage_.size() && "index out of bounds!");
      return storage_[toIndex_(n)];
    }

    typename StorageT::const_reference operator[](IndexT n) const {
      assert(toIndex_(n) < storage_.size() && "index out of bounds!");
      return storage_[toIndex_(n)];
    }

    void reserve(typename StorageT::size_type s) {
      storage_.reserve(s);
    }

    void resize(typename StorageT::size_type s) {
      storage_.resize(s, nullVal_);
    }

    void clear() {
      storage_.clear();
    }

    void grow(IndexT n) {
      unsigned NewSize = toIndex_(n) + 1;
      if (NewSize > storage_.size())
        resize(NewSize);
    }

    bool inBounds(IndexT n) const {
      return toIndex_(n) < storage_.size();
    }

    typename StorageT::size_type size() const {
      return storage_.size();
    }
  };

} // End llvm namespace

#endif
