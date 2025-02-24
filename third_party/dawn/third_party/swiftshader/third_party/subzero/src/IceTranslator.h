//===- subzero/src/IceTranslator.h - ICE to machine code --------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the general driver class for translating ICE to machine
/// code.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETRANSLATOR_H
#define SUBZERO_SRC_ICETRANSLATOR_H

#include "IceDefs.h"
#include "IceGlobalContext.h"

#include <memory>

namespace llvm {
class Module;
} // end of namespace llvm

namespace Ice {

class ClFlags;
class Cfg;
class VariableDeclaration;
class GlobalContext;

/// Base class for translating ICE to machine code. Derived classes convert
/// other intermediate representations down to ICE, and then call the
/// appropriate (inherited) methods to convert ICE into machine instructions.
class Translator {
  Translator() = delete;
  Translator(const Translator &) = delete;
  Translator &operator=(const Translator &) = delete;

public:
  explicit Translator(GlobalContext *Ctx);

  virtual ~Translator() = default;
  const ErrorCode &getErrorStatus() const { return ErrorStatus; }

  GlobalContext *getContext() const { return Ctx; }

  /// Translates the constructed ICE function Func to machine code.
  void translateFcn(std::unique_ptr<Cfg> Func);

  /// Lowers the given list of global addresses to target. Generates list of
  /// corresponding variable declarations.
  void
  lowerGlobals(std::unique_ptr<VariableDeclarationList> VariableDeclarations);

  /// Creates a name using the given prefix and corresponding index.
  std::string createUnnamedName(const std::string &Prefix, SizeT Index);

  /// Reports if there is a (potential) conflict between Name, and using Prefix
  /// to name unnamed names. Errors are put on Ostream. Returns true if there
  /// isn't a potential conflict.
  bool checkIfUnnamedNameSafe(const std::string &Name, const char *Kind,
                              const std::string &Prefix);

  uint32_t getNextSequenceNumber() { return NextSequenceNumber++; }

protected:
  GlobalContext *Ctx;
  uint32_t NextSequenceNumber;
  /// ErrorCode of the translation.
  ErrorCode ErrorStatus;
};

class CfgOptWorkItem final : public OptWorkItem {
  CfgOptWorkItem() = delete;
  CfgOptWorkItem(const CfgOptWorkItem &) = delete;
  CfgOptWorkItem &operator=(const CfgOptWorkItem &) = delete;

public:
  CfgOptWorkItem(std::unique_ptr<Cfg> Func) : Func(std::move(Func)) {}
  std::unique_ptr<Cfg> getParsedCfg() override { return std::move(Func); }
  ~CfgOptWorkItem() override = default;

private:
  std::unique_ptr<Ice::Cfg> Func;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICETRANSLATOR_H
