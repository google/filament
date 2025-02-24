//===- subzero/src/IceFixups.cpp - Implementation of Assembler Fixups -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the AssemblerFixup class, a very basic target-independent
/// representation of a fixup or relocation.
///
//===----------------------------------------------------------------------===//

#include "IceFixups.h"

#include "IceOperand.h"

namespace Ice {

const Constant *AssemblerFixup::NullSymbol = nullptr;

RelocOffsetT AssemblerFixup::offset() const {
  if (isNullSymbol())
    return addend_;
  if (!ValueIsSymbol) {
    if (const auto *CR = llvm::dyn_cast<ConstantRelocatable>(ConstValue))
      return CR->getOffset() + addend_;
  }
  return addend_;
}

GlobalString AssemblerFixup::symbol() const {
  assert(!isNullSymbol());
  assert(!ValueIsSymbol);
  const Constant *C = ConstValue;
  if (const auto *CR = llvm::dyn_cast<ConstantRelocatable>(C)) {
    return CR->getName();
  }
  // NOTE: currently only float/doubles are put into constant pools. In the
  // future we may put integers as well.
  assert(llvm::isa<ConstantFloat>(C) || llvm::isa<ConstantDouble>(C));
  return C->getLabelName();
}

size_t AssemblerFixup::emit(GlobalContext *Ctx, const Assembler &Asm) const {
  static constexpr const size_t FixupSize = 4;
  if (!BuildDefs::dump())
    return FixupSize;
  Ostream &Str = Ctx->getStrEmit();
  Str << "\t.long ";
  std::string Symbol;
  if (isNullSymbol()) {
    Str << "__Sz_AbsoluteZero";
  } else {
    Symbol = symbol().toString();
    Str << Symbol;
    assert(!ValueIsSymbol);
  }

  assert(Asm.load<RelocOffsetT>(position()) == 0);

  RelocOffsetT Offset = offset();
  if (Offset != 0) {
    if (Offset > 0) {
      Str << " + " << Offset;
    } else {
      assert(Offset != std::numeric_limits<RelocOffsetT>::lowest());
      Str << " - " << -Offset;
    }
  }

  // We need to emit the '- .' for PCRel fixups. Even if the relocation kind()
  // is not PCRel, we emit the '- .' for the _GLOBAL_OFFSET_TABLE_.
  // TODO(jpp): create fixups wrt the GOT with the right fixup kind.
  if (Asm.fixupIsPCRel(kind()) || Symbol == GlobalOffsetTable)
    Str << " - .";
  Str << "\n";
  return FixupSize;
}

void AssemblerFixup::emitOffset(Assembler *Asm) const {
  Asm->store(position(), offset());
}

size_t AssemblerTextFixup::emit(GlobalContext *Ctx, const Assembler &) const {
  Ctx->getStrEmit() << Message << "\n";
  return NumBytes;
}

} // end of namespace Ice
