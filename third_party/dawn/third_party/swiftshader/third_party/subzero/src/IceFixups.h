//===- subzero/src/IceFixups.h - Assembler fixup kinds ----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares generic fixup types.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEFIXUPS_H
#define SUBZERO_SRC_ICEFIXUPS_H

#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceStringPool.h"

namespace Ice {

/// Each target and container format has a different namespace of relocations.
/// This holds the specific target+container format's relocation number.
using FixupKind = uint32_t;

struct ELFSym;

/// Assembler fixups are positions in generated code/data that hold relocation
/// information that needs to be processed before finalizing the code/data.
class AssemblerFixup {
  AssemblerFixup &operator=(const AssemblerFixup &) = delete;

public:
  AssemblerFixup() = default;
  AssemblerFixup(const AssemblerFixup &) = default;
  virtual ~AssemblerFixup() = default;
  intptr_t position() const { return position_; }
  void set_position(intptr_t Position) { position_ = Position; }

  FixupKind kind() const { return kind_; }
  void set_kind(FixupKind Kind) { kind_ = Kind; }

  RelocOffsetT offset() const;
  GlobalString symbol() const;

  static const Constant *NullSymbol;
  bool isNullSymbol() const { return ConstValue == NullSymbol; }

  static constexpr AssemblerFixup *NoFixup = nullptr;

  bool valueIsSymbol() const { return ValueIsSymbol; }
  void set_value(const Constant *Value) {
    ValueIsSymbol = false;
    ConstValue = Value;
  }
  void set_value(const ELFSym *Value) {
    ValueIsSymbol = true;
    SymbolValue = Value;
  }
  const ELFSym *getSymbolValue() const {
    assert(ValueIsSymbol);
    return SymbolValue;
  }

  void set_addend(RelocOffsetT Addend) { addend_ = Addend; }
  RelocOffsetT get_addend() const { return addend_; }

  /// Emits fixup, then returns the number of bytes to skip.
  virtual size_t emit(GlobalContext *Ctx, const Assembler &Asm) const;

  /// Emits offset() (little endian) in position_. If your fixup requires
  /// something smarter, you must create your own fixup type.
  virtual void emitOffset(Assembler *Asm) const;

private:
  intptr_t position_ = 0;
  FixupKind kind_ = 0;
  // An offset addend to the fixup offset (as returned by offset()), in case the
  // assembler needs to adjust it.
  RelocOffsetT addend_ = 0;

  // Tagged union that holds either a Constant or ELFSym pointer, depending on
  // the ValueIsSymbol tag.
  bool ValueIsSymbol = false;
  union {
    const Constant *ConstValue;
    const ELFSym *SymbolValue;
  };
};

/// Extends a fixup to be textual. That is, it emits text instead of a sequence
/// of bytes. This class is used as a fallback for unimplemented emitIAS
/// methods, allowing them to generate compilable assembly code.
class AssemblerTextFixup : public AssemblerFixup {
  AssemblerTextFixup() = delete;
  AssemblerTextFixup(const AssemblerTextFixup &) = delete;
  AssemblerTextFixup &operator=(const AssemblerTextFixup &) = delete;

public:
  AssemblerTextFixup(const std::string &Message, size_t NumBytes)
      : AssemblerFixup(), Message(Message), NumBytes(NumBytes) {}
  ~AssemblerTextFixup() = default;
  size_t emit(GlobalContext *Ctx, const Assembler &Asm) const override;

private:
  const std::string Message;
  const size_t NumBytes;
};

using FixupList = std::vector<AssemblerFixup>;
using FixupRefList = std::vector<AssemblerFixup *>;

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEFIXUPS_H
