//===-- MCAsmInfoELF.cpp - ELF asm properties -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines target asm properties related what form asm statements
// should take in general on ELF-based targets
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/Support/ELF.h"
using namespace llvm;

void MCAsmInfoELF::anchor() { }

MCSection *MCAsmInfoELF::getNonexecutableStackSection(MCContext &Ctx) const {
  return Ctx.getELFSection(".note.GNU-stack", ELF::SHT_PROGBITS, 0);
}

MCAsmInfoELF::MCAsmInfoELF() {
  HasIdentDirective = true;
  WeakRefDirective = "\t.weak\t";
  PrivateGlobalPrefix = ".L";
  PrivateLabelPrefix = ".L";
}
