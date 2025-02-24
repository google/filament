//===- subzero/src/IceASanInstrumentation.h - AddressSanitizer --*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the AddressSanitizer instrumentation class.
///
/// This class is responsible for inserting redzones around global and stack
/// variables, inserting code responsible for poisoning those redzones, and
/// performing any other instrumentation necessary to implement
/// AddressSanitizer.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEASANINSTRUMENTATION_H
#define SUBZERO_SRC_ICEASANINSTRUMENTATION_H

#include "IceGlobalInits.h"
#include "IceInstrumentation.h"

namespace Ice {

using VarSizeMap = std::unordered_map<Operand *, SizeT>;
using GlobalSizeMap = std::unordered_map<GlobalString, SizeT>;

class ASanInstrumentation : public Instrumentation {
  ASanInstrumentation() = delete;
  ASanInstrumentation(const ASanInstrumentation &) = delete;
  ASanInstrumentation &operator=(const ASanInstrumentation &) = delete;

public:
  ASanInstrumentation(GlobalContext *Ctx) : Instrumentation(Ctx), RzNum(0) {
    ICE_TLS_INIT_FIELD(LocalVars);
    ICE_TLS_INIT_FIELD(LocalDtors);
    ICE_TLS_INIT_FIELD(CurNode);
    ICE_TLS_INIT_FIELD(CheckedVars);
  }
  void instrumentGlobals(VariableDeclarationList &Globals) override;

private:
  std::string nextRzName();
  bool isOkGlobalAccess(Operand *Op, SizeT Size);
  ConstantRelocatable *instrumentReloc(ConstantRelocatable *Reloc);
  bool isInstrumentable(Cfg *Func) override;
  void instrumentFuncStart(LoweringContext &Context) override;
  void instrumentCall(LoweringContext &Context, InstCall *Instr) override;
  void instrumentRet(LoweringContext &Context, InstRet *Instr) override;
  void instrumentLoad(LoweringContext &Context, InstLoad *Instr) override;
  void instrumentStore(LoweringContext &Context, InstStore *Instr) override;
  void instrumentAccess(LoweringContext &Context, Operand *Op, SizeT Size,
                        Constant *AccessFunc);
  void instrumentStart(Cfg *Func) override;
  void finishFunc(Cfg *Func) override;
  ICE_TLS_DECLARE_FIELD(VarSizeMap *, LocalVars);
  ICE_TLS_DECLARE_FIELD(std::vector<InstStore *> *, LocalDtors);
  ICE_TLS_DECLARE_FIELD(CfgNode *, CurNode);
  ICE_TLS_DECLARE_FIELD(VarSizeMap *, CheckedVars);
  GlobalSizeMap GlobalSizes;
  std::atomic<uint32_t> RzNum;
  bool DidProcessGlobals = false;
  SizeT RzGlobalsNum = 0;
  std::mutex GlobalsMutex;
};
} // end of namespace Ice

#endif // SUBZERO_SRC_ICEASANINSTRUMENTATION_H
