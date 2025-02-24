//===- subzero/src/IceThreading.cpp - Threading function definitions ------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines threading-related functions.
///
//===----------------------------------------------------------------------===//

#include "IceThreading.h"

#include "IceCfg.h"
#include "IceDefs.h"

namespace Ice {

EmitterWorkItem::EmitterWorkItem(uint32_t Seq)
    : Sequence(Seq), Kind(WI_Nop), GlobalInits(nullptr), Function(nullptr),
      RawFunc(nullptr) {}
EmitterWorkItem::EmitterWorkItem(uint32_t Seq,
                                 std::unique_ptr<VariableDeclarationList> D)
    : Sequence(Seq), Kind(WI_GlobalInits), GlobalInits(std::move(D)),
      Function(nullptr), RawFunc(nullptr) {}
EmitterWorkItem::EmitterWorkItem(uint32_t Seq, std::unique_ptr<Assembler> A)
    : Sequence(Seq), Kind(WI_Asm), GlobalInits(nullptr), Function(std::move(A)),
      RawFunc(nullptr) {}
EmitterWorkItem::EmitterWorkItem(uint32_t Seq, std::unique_ptr<Cfg> F)
    : Sequence(Seq), Kind(WI_Cfg), GlobalInits(nullptr), Function(nullptr),
      RawFunc(std::move(F)) {}

void EmitterWorkItem::setGlobalInits(
    std::unique_ptr<VariableDeclarationList> GloblInits) {
  assert(getKind() == WI_Asm || getKind() == WI_Cfg);
  GlobalInits = std::move(GloblInits);
}

std::unique_ptr<VariableDeclarationList> EmitterWorkItem::getGlobalInits() {
  assert(getKind() == WI_GlobalInits || getKind() == WI_Asm ||
         getKind() == WI_Cfg);
  return std::move(GlobalInits);
}

std::unique_ptr<Assembler> EmitterWorkItem::getAsm() {
  assert(getKind() == WI_Asm);
  return std::move(Function);
}

std::unique_ptr<Cfg> EmitterWorkItem::getCfg() {
  assert(getKind() == WI_Cfg);
  return std::move(RawFunc);
}

} // end of namespace Ice
