//===- SymbolicFile.h - Interface that only provides symbols ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the SymbolicFile interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_OBJECT_SYMBOLICFILE_H
#define LLVM_OBJECT_SYMBOLICFILE_H

#include "llvm/Object/Binary.h"

namespace llvm {
namespace object {

union DataRefImpl {
  // This entire union should probably be a
  // char[max(8, sizeof(uintptr_t))] and require the impl to cast.
  struct {
    uint32_t a, b;
  } d;
  uintptr_t p;
  DataRefImpl() { std::memset(this, 0, sizeof(DataRefImpl)); }
};

inline bool operator==(const DataRefImpl &a, const DataRefImpl &b) {
  // Check bitwise identical. This is the only legal way to compare a union w/o
  // knowing which member is in use.
  return std::memcmp(&a, &b, sizeof(DataRefImpl)) == 0;
}

inline bool operator!=(const DataRefImpl &a, const DataRefImpl &b) {
  return !operator==(a, b);
}

inline bool operator<(const DataRefImpl &a, const DataRefImpl &b) {
  // Check bitwise identical. This is the only legal way to compare a union w/o
  // knowing which member is in use.
  return std::memcmp(&a, &b, sizeof(DataRefImpl)) < 0;
}

template <class content_type>
class content_iterator {
  content_type Current;

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = content_type;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type &;

  content_iterator(content_type symb) : Current(symb) {}

  const content_type *operator->() const { return &Current; }

  const content_type &operator*() const { return Current; }

  bool operator==(const content_iterator &other) const {
    return Current == other.Current;
  }

  bool operator!=(const content_iterator &other) const {
    return !(*this == other);
  }

  content_iterator &operator++() { // preincrement
    Current.moveNext();
    return *this;
  }
};

class SymbolicFile;

/// This is a value type class that represents a single symbol in the list of
/// symbols in the object file.
class BasicSymbolRef {
  DataRefImpl SymbolPimpl;
  const SymbolicFile *OwningObject;

public:
  // FIXME: should we add a SF_Text?
  enum Flags : unsigned {
    SF_None = 0,
    SF_Undefined = 1U << 0,      // Symbol is defined in another object file
    SF_Global = 1U << 1,         // Global symbol
    SF_Weak = 1U << 2,           // Weak symbol
    SF_Absolute = 1U << 3,       // Absolute symbol
    SF_Common = 1U << 4,         // Symbol has common linkage
    SF_Indirect = 1U << 5,       // Symbol is an alias to another symbol
    SF_Exported = 1U << 6,       // Symbol is visible to other DSOs
    SF_FormatSpecific = 1U << 7, // Specific to the object file format
                                 // (e.g. section symbols)
    SF_Thumb = 1U << 8,          // Thumb symbol in a 32-bit ARM binary
    SF_Hidden = 1U << 9,         // Symbol has hidden visibility
  };

  BasicSymbolRef() : OwningObject(nullptr) { }
  BasicSymbolRef(DataRefImpl SymbolP, const SymbolicFile *Owner);

  bool operator==(const BasicSymbolRef &Other) const;
  bool operator<(const BasicSymbolRef &Other) const;

  void moveNext();

  std::error_code printName(raw_ostream &OS) const;

  /// Get symbol flags (bitwise OR of SymbolRef::Flags)
  uint32_t getFlags() const;

  DataRefImpl getRawDataRefImpl() const;
  const SymbolicFile *getObject() const;
};

typedef content_iterator<BasicSymbolRef> basic_symbol_iterator;

class SymbolicFile : public Binary {
public:
  ~SymbolicFile() override;
  SymbolicFile(unsigned int Type, MemoryBufferRef Source);

  // virtual interface.
  virtual void moveSymbolNext(DataRefImpl &Symb) const = 0;

  virtual std::error_code printSymbolName(raw_ostream &OS,
                                          DataRefImpl Symb) const = 0;

  virtual uint32_t getSymbolFlags(DataRefImpl Symb) const = 0;

  virtual basic_symbol_iterator symbol_begin_impl() const = 0;

  virtual basic_symbol_iterator symbol_end_impl() const = 0;

  // convenience wrappers.
  basic_symbol_iterator symbol_begin() const {
    return symbol_begin_impl();
  }
  basic_symbol_iterator symbol_end() const {
    return symbol_end_impl();
  }
  typedef iterator_range<basic_symbol_iterator> basic_symbol_iterator_range;
  basic_symbol_iterator_range symbols() const {
    return basic_symbol_iterator_range(symbol_begin(), symbol_end());
  }

  // construction aux.
  static ErrorOr<std::unique_ptr<SymbolicFile>>
  createSymbolicFile(MemoryBufferRef Object, sys::fs::file_magic Type,
                     LLVMContext *Context);

  static ErrorOr<std::unique_ptr<SymbolicFile>>
  createSymbolicFile(MemoryBufferRef Object) {
    return createSymbolicFile(Object, sys::fs::file_magic::unknown, nullptr);
  }
  static ErrorOr<OwningBinary<SymbolicFile>>
  createSymbolicFile(StringRef ObjectPath);

  static inline bool classof(const Binary *v) {
    return v->isSymbolic();
  }
};

inline BasicSymbolRef::BasicSymbolRef(DataRefImpl SymbolP,
                                      const SymbolicFile *Owner)
    : SymbolPimpl(SymbolP), OwningObject(Owner) {}

inline bool BasicSymbolRef::operator==(const BasicSymbolRef &Other) const {
  return SymbolPimpl == Other.SymbolPimpl;
}

inline bool BasicSymbolRef::operator<(const BasicSymbolRef &Other) const {
  return SymbolPimpl < Other.SymbolPimpl;
}

inline void BasicSymbolRef::moveNext() {
  return OwningObject->moveSymbolNext(SymbolPimpl);
}

inline std::error_code BasicSymbolRef::printName(raw_ostream &OS) const {
  return OwningObject->printSymbolName(OS, SymbolPimpl);
}

inline uint32_t BasicSymbolRef::getFlags() const {
  return OwningObject->getSymbolFlags(SymbolPimpl);
}

inline DataRefImpl BasicSymbolRef::getRawDataRefImpl() const {
  return SymbolPimpl;
}

inline const SymbolicFile *BasicSymbolRef::getObject() const {
  return OwningObject;
}

}
}

#endif
