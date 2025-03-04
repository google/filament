//===-- llvm/MC/MCAsmInfoELF.h - ELF Asm info -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_MCASMINFOELF_H
#define LLVM_MC_MCASMINFOELF_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
class MCAsmInfoELF : public MCAsmInfo {
  virtual void anchor();
  MCSection *getNonexecutableStackSection(MCContext &Ctx) const final;

protected:
  MCAsmInfoELF();
};
}

#endif
