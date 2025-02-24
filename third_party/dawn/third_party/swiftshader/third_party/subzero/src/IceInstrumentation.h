//===- subzero/src/IceInstrumentation.h - ICE instrumentation ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the Ice::Instrumentation class.
///
/// Instrumentation is an abstract class used to drive the instrumentation
/// process for tools such as AddressSanitizer and MemorySanitizer. It uses a
/// LoweringContext to enable the insertion of new instructions into a given
/// Cfg. Although Instrumentation is an abstract class, each of its virtual
/// functions has a trivial default implementation to make subclasses more
/// succinct.
///
/// If instrumentation is required by the command line arguments, a single
/// Instrumentation subclass is instantiated and installed in the
/// GlobalContext. If multiple types of instrumentation are requested, a single
/// subclass is still responsible for driving the instrumentation, but it can
/// use other Instrumentation subclasses however it needs to.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEINSTRUMENTATION_H
#define SUBZERO_SRC_ICEINSTRUMENTATION_H

#include "IceDefs.h"

#include <condition_variable>

namespace Ice {

class LoweringContext;

class Instrumentation {
  Instrumentation() = delete;
  Instrumentation(const Instrumentation &) = delete;
  Instrumentation &operator=(const Instrumentation &) = delete;

public:
  Instrumentation(GlobalContext *Ctx) : Ctx(Ctx) {}
  virtual ~Instrumentation() = default;
  virtual void instrumentGlobals(VariableDeclarationList &) {}
  void instrumentFunc(Cfg *Func);
  void setHasSeenGlobals();

protected:
  virtual void instrumentInst(LoweringContext &Context);
  LockedPtr<VariableDeclarationList> getGlobals();

private:
  virtual bool isInstrumentable(Cfg *) { return true; }
  virtual void instrumentFuncStart(LoweringContext &) {}
  virtual void instrumentAlloca(LoweringContext &, class InstAlloca *) {}
  virtual void instrumentArithmetic(LoweringContext &, class InstArithmetic *) {
  }
  virtual void instrumentBr(LoweringContext &, class InstBr *) {}
  virtual void instrumentCall(LoweringContext &, class InstCall *) {}
  virtual void instrumentCast(LoweringContext &, class InstCast *) {}
  virtual void instrumentExtractElement(LoweringContext &,
                                        class InstExtractElement *) {}
  virtual void instrumentFcmp(LoweringContext &, class InstFcmp *) {}
  virtual void instrumentIcmp(LoweringContext &, class InstIcmp *) {}
  virtual void instrumentInsertElement(LoweringContext &,
                                       class InstInsertElement *) {}
  virtual void instrumentIntrinsic(LoweringContext &, class InstIntrinsic *) {}
  virtual void instrumentLoad(LoweringContext &, class InstLoad *) {}
  virtual void instrumentPhi(LoweringContext &, class InstPhi *) {}
  virtual void instrumentRet(LoweringContext &, class InstRet *) {}
  virtual void instrumentSelect(LoweringContext &, class InstSelect *) {}
  virtual void instrumentStore(LoweringContext &, class InstStore *) {}
  virtual void instrumentSwitch(LoweringContext &, class InstSwitch *) {}
  virtual void instrumentUnreachable(LoweringContext &,
                                     class InstUnreachable *) {}
  virtual void instrumentStart(Cfg *) {}
  virtual void instrumentLocalVars(Cfg *) {}
  virtual void finishFunc(Cfg *) {}

protected:
  GlobalContext *Ctx;

private:
  bool HasSeenGlobals = false;
  std::mutex GlobalsSeenMutex;
  std::condition_variable GlobalsSeenCV;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEINSTRUMENTATION_H
