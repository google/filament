//===- subzero/src/IceConverter.h - Converts LLVM to ICE --------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the LLVM to ICE converter.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECONVERTER_H
#define SUBZERO_SRC_ICECONVERTER_H

#include "IceDefs.h"
#include "IceTranslator.h"

namespace llvm {
class GlobalValue;
class Module;
} // namespace llvm

namespace Ice {

class Converter : public Translator {
  Converter() = delete;
  Converter(const Converter &) = delete;
  Converter &operator=(const Converter &) = delete;

public:
  Converter(llvm::Module *Mod, GlobalContext *Ctx)
      : Translator(Ctx), Mod(Mod),
        GlobalDeclarationsPool(new VariableDeclarationList()) {}

  ~Converter() override = default;

  /// Converts the LLVM Module to ICE. Sets exit status to false if successful,
  /// true otherwise.
  void convertToIce();

  llvm::Module *getModule() const { return Mod; }

  /// Returns the global declaration associated with the corresponding global
  /// value V. If no such global address, generates fatal error.
  GlobalDeclaration *getGlobalDeclaration(const llvm::GlobalValue *V);

private:
  llvm::Module *Mod;
  using GlobalDeclarationMapType =
      std::map<const llvm::GlobalValue *, GlobalDeclaration *>;
  GlobalDeclarationMapType GlobalDeclarationMap;

  std::unique_ptr<VariableDeclarationList> GlobalDeclarationsPool;

  /// Walks module and generates names for unnamed globals using prefix
  /// getFlags().DefaultGlobalPrefix, if the prefix is non-empty.
  void nameUnnamedGlobalVariables(llvm::Module *Mod);

  /// Walks module and generates names for unnamed functions using prefix
  /// getFlags().DefaultFunctionPrefix, if the prefix is non-empty.
  void nameUnnamedFunctions(llvm::Module *Mod);

  /// Converts functions to ICE, and then machine code.
  void convertFunctions();

  /// Converts globals to ICE, and then machine code.
  void convertGlobals(llvm::Module *Mod);

  /// Installs global declarations into GlobalDeclarationMap.
  void installGlobalDeclarations(llvm::Module *Mod);
};

} // namespace Ice

#endif // SUBZERO_SRC_ICECONVERTER_H
