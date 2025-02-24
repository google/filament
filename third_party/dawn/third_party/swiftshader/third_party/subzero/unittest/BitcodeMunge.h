//===- BitcodeMunge.h - Subzero Bitcode Munger ------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Test harness for testing malformed bitcode files in Subzero. Uses NaCl's
// bitcode munger to do this.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_UNITTEST_BITCODEMUNGE_H
#define SUBZERO_UNITTEST_BITCODEMUNGE_H

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "llvm/Bitcode/NaCl/NaClBitcodeMunge.h"
#pragma clang diagnostic pop

#include "IceClFlags.h"
#include "IceGlobalContext.h"

namespace IceTest {

// Class to run tests on Subzero's bitcode parser. Runs a Subzero
// translation, using (possibly) edited bitcode record values.  For
// more details on how to represent the input arrays, see
// NaClBitcodeMunge.h.
class SubzeroBitcodeMunger : public llvm::NaClBitcodeMunger {
public:
  SubzeroBitcodeMunger(const uint64_t Records[], size_t RecordSize,
                       uint64_t RecordTerminator)
      : llvm::NaClBitcodeMunger(Records, RecordSize, RecordTerminator) {
    resetMungeFlags();
  }

  /// Runs PNaClTranslator to parse and (optionally) translate bitcode records
  /// (with defined record Munges), and puts output into DumpResults. Returns
  /// true if parse is successful.
  bool runTest(const uint64_t Munges[], size_t MungeSize,
               bool DisableTranslation = false);

  /// Same as above, but without any edits.
  bool runTest(bool DisableTranslation = false) {
    uint64_t NoMunges[] = {0};
    return runTest(NoMunges, 0, DisableTranslation);
  }

private:
  void resetMungeFlags();
};

} // end of namespace IceTest

#endif // SUBZERO_UNITTEST_BITCODEMUNGE_H
