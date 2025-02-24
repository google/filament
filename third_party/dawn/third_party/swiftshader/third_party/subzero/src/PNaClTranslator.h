//===- subzero/src/PNaClTranslator.h - ICE from bitcode ---------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the interface for translation from PNaCl bitcode files to
/// ICE to machine code.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_PNACLTRANSLATOR_H
#define SUBZERO_SRC_PNACLTRANSLATOR_H

#include "IceTranslator.h"

#include <string>

namespace llvm {
class MemoryBuffer;
class MemoryObject;
} // end of namespace llvm

namespace Ice {

class PNaClTranslator : public Translator {
  PNaClTranslator() = delete;
  PNaClTranslator(const PNaClTranslator &) = delete;
  PNaClTranslator &operator=(const PNaClTranslator &) = delete;

public:
  explicit PNaClTranslator(GlobalContext *Ctx) : Translator(Ctx) {}
  ~PNaClTranslator() override = default;

  /// Reads the PNaCl bitcode file and translates to ICE, which is then
  /// converted to machine code. Sets ErrorStatus to 1 if any errors occurred.
  /// Takes ownership of the MemoryObject.
  void translate(const std::string &IRFilename,
                 std::unique_ptr<llvm::MemoryObject> &&MemoryObject);

  /// Reads MemBuf, assuming it is the PNaCl bitcode contents of IRFilename.
  void translateBuffer(const std::string &IRFilename,
                       llvm::MemoryBuffer *MemBuf);
};

} // end of namespace Ice

#endif // SUBZERO_SRC_PNACLTRANSLATOR_H
