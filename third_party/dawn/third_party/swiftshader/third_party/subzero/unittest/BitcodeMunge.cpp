//===- BitcodeMunge.cpp - Subzero Bitcode Munger ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Test harness for testing malformed bitcode files in Subzero.
//
//===----------------------------------------------------------------------===//

#include "BitcodeMunge.h"
#include "IceCfg.h"
#include "IceClFlags.h"
#include "IceTypes.h"
#include "PNaClTranslator.h"

namespace IceTest {

void IceTest::SubzeroBitcodeMunger::resetMungeFlags() {
  Ice::ClFlags::Flags.setAllowErrorRecovery(true);
  Ice::ClFlags::Flags.setDisableTranslation(false);
  Ice::ClFlags::Flags.setGenerateUnitTestMessages(true);
  Ice::ClFlags::Flags.setOptLevel(Ice::Opt_m1);
  Ice::ClFlags::Flags.setOutFileType(Ice::FT_Iasm);
  Ice::ClFlags::Flags.setTargetArch(Ice::Target_X8632);
  Ice::ClFlags::Flags.setNumTranslationThreads(0);
  Ice::ClFlags::Flags.setParseParallel(false);
}

bool IceTest::SubzeroBitcodeMunger::runTest(const uint64_t Munges[],
                                            size_t MungeSize,
                                            bool DisableTranslation) {
  const bool AddHeader = true;
  setupTest(Munges, MungeSize, AddHeader);
  Ice::GlobalContext Ctx(DumpStream, DumpStream, DumpStream, nullptr);
  Ctx.startWorkerThreads();
  Ice::PNaClTranslator Translator(&Ctx);
  const char *BufferName = "Test";
  Ice::ClFlags::Flags.setDisableTranslation(DisableTranslation);
  Translator.translateBuffer(BufferName, MungedInput.get());
  Ctx.waitForWorkerThreads();

  cleanupTest();
  return Translator.getErrorStatus().value() == 0;
}

} // end of namespace IceTest
