//===-- llvm-dwarfdump-fuzzer.cpp - Fuzz the llvm-dwarfdump tool ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements a function that runs llvm-dwarfdump
///  on a single input. This function is then linked into the Fuzzer library.
///
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/MemoryBuffer.h"
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
using namespace llvm;
using namespace object;

extern "C" void LLVMFuzzerTestOneInput(uint8_t *data, size_t size) {
  std::unique_ptr<MemoryBuffer> Buff = MemoryBuffer::getMemBuffer(
      StringRef((const char *)data, size), "", false);

  ErrorOr<std::unique_ptr<ObjectFile>> ObjOrErr =
      ObjectFile::createObjectFile(Buff->getMemBufferRef());
  if (!ObjOrErr)
    return;
  ObjectFile &Obj = *ObjOrErr.get();
  std::unique_ptr<DIContext> DICtx(new DWARFContextInMemory(Obj));
  DICtx->dump(nulls(), DIDT_All);
}
