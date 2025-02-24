//===- unittest/IceParseInstsTest.cpp - test instruction errors -----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <string>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Bitcode/NaCl/NaClBitcodeParser.h"
#include "llvm/Bitcode/NaCl/NaClLLVMBitCodes.h"
#pragma clang diagnostic pop

#include "BitcodeMunge.h"
#include "unittests/Bitcode/NaClMungeTest.h"

using namespace llvm;
using namespace naclmungetest;

namespace {

// The ParseError constant is passed to the BitcodeMunger to prevent translation
// when we expect a Parse error.
constexpr bool ParseError = true;

// Note: alignment stored as 0 or log2(Alignment)+1.
uint64_t getEncAlignPower(unsigned Power) { return Power + 1; }
uint64_t getEncAlignZero() { return 0; }

/// Test how we report a call arg that refers to nonexistent call argument
TEST(IceParseInstsTest, NonexistentCallArg) {
  const uint64_t BitcodeRecords[] = {
      1, naclbitc::BLK_CODE_ENTER, naclbitc::MODULE_BLOCK_ID, 2, Terminator, 1,
      naclbitc::BLK_CODE_ENTER, naclbitc::TYPE_BLOCK_ID_NEW, 2, Terminator, 3,
      naclbitc::TYPE_CODE_NUMENTRY, 3, Terminator, 3,
      naclbitc::TYPE_CODE_INTEGER, 32, Terminator, 3, naclbitc::TYPE_CODE_VOID,
      Terminator, 3, naclbitc::TYPE_CODE_FUNCTION, 0, 1, 0, 0, Terminator, 0,
      naclbitc::BLK_CODE_EXIT, Terminator, 3, naclbitc::MODULE_CODE_FUNCTION, 2,
      0, 1, 3, Terminator, 3, naclbitc::MODULE_CODE_FUNCTION, 2, 0, 0, 3,
      Terminator, 1, naclbitc::BLK_CODE_ENTER, naclbitc::FUNCTION_BLOCK_ID, 2,
      Terminator, 3, naclbitc::FUNC_CODE_DECLAREBLOCKS, 1, Terminator,
      // Note: 100 is a bad value index in next line.
      3, naclbitc::FUNC_CODE_INST_CALL, 0, 4, 2, 100, Terminator, 3,
      naclbitc::FUNC_CODE_INST_RET, Terminator, 0, naclbitc::BLK_CODE_EXIT,
      Terminator, 0, naclbitc::BLK_CODE_EXIT, Terminator};

  // Show bitcode objdump for BitcodeRecords.
  NaClObjDumpMunger DumpMunger(ARRAY_TERM(BitcodeRecords));
  EXPECT_FALSE(DumpMunger.runTest());
  EXPECT_EQ("      66:4|    3: <34, 0, 4, 2, 100>    |    call void @f0(i32 "
            "%p0, i32 @f0);\n"
            "Error(66:4): Invalid relative value id: 100 (Must be <= 4)\n",
            DumpMunger.getLinesWithSubstring("66:4"));

  // Show that we get appropriate error when parsing in Subzero.
  IceTest::SubzeroBitcodeMunger Munger(ARRAY_TERM(BitcodeRecords));
  EXPECT_FALSE(Munger.runTest(ParseError));
  EXPECT_EQ("Error(66:4): Invalid function record: <34 0 4 2 100>\n",
            Munger.getTestResults());

  // Show that we generate a fatal error when not allowing error recovery.
  Ice::ClFlags::Flags.setAllowErrorRecovery(false);
  EXPECT_DEATH(Munger.runTest(ParseError), ".*ERROR: Unable to continue.*");
}

/// Test how we recognize alignments in alloca instructions.
TEST(IceParseInstsTests, AllocaAlignment) {
  const uint64_t BitcodeRecords[] = {1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::MODULE_BLOCK_ID,
                                     2,
                                     Terminator,
                                     1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::TYPE_BLOCK_ID_NEW,
                                     2,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_NUMENTRY,
                                     4,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_INTEGER,
                                     32,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_VOID,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_FUNCTION,
                                     0,
                                     1,
                                     0,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_INTEGER,
                                     8,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator,
                                     3,
                                     naclbitc::MODULE_CODE_FUNCTION,
                                     2,
                                     0,
                                     0,
                                     3,
                                     Terminator,
                                     1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::FUNCTION_BLOCK_ID,
                                     2,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_DECLAREBLOCKS,
                                     1,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_INST_ALLOCA,
                                     1,
                                     getEncAlignPower(0),
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_INST_RET,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator};

  const uint64_t ReplaceIndex = 11; // index for FUNC_CODE_INST_ALLOCA

  // Show text when alignment is 1.
  NaClObjDumpMunger DumpMunger(ARRAY_TERM(BitcodeRecords));
  EXPECT_TRUE(DumpMunger.runTest());
  EXPECT_EQ("      62:4|    3: <19, 1, 1>            |    %v0 = alloca i8, i32 "
            "%p0, align 1;\n",
            DumpMunger.getLinesWithSubstring("62:4"));

  // Show that we can handle alignment of 1.
  IceTest::SubzeroBitcodeMunger Munger(ARRAY_TERM(BitcodeRecords));
  EXPECT_TRUE(Munger.runTest());

  // Show what happens when changing alignment to 0.
  const uint64_t Align0[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_ALLOCA,
      1,
      getEncAlignZero(),
      Terminator,
  };
  EXPECT_TRUE(Munger.runTest(ARRAY(Align0)));
  EXPECT_TRUE(DumpMunger.runTestForAssembly(ARRAY(Align0)));
  EXPECT_EQ("    %v0 = alloca i8, i32 %p0, align 0;\n",
            DumpMunger.getLinesWithSubstring("alloca"));

  // Show what happens when changing alignment to 2**30.
  const uint64_t Align30[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_ALLOCA,
      1,
      getEncAlignPower(30),
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align30), ParseError));
  EXPECT_EQ("Error(62:4): Invalid function record: <19 1 31>\n",
            Munger.getTestResults());

  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align30)));
  EXPECT_EQ("    %v0 = alloca i8, i32 %p0, align 0;\n",
            DumpMunger.getLinesWithSubstring("alloca"));
  EXPECT_EQ(
      "Error(62:4): Alignment can't be greater than 2**29. Found: 2**30\n",
      DumpMunger.getLinesWithSubstring("Error"));

  // Show what happens when changing alignment to 2**29.
  const uint64_t Align29[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_ALLOCA,
      1,
      getEncAlignPower(29),
      Terminator,
  };
  EXPECT_TRUE(Munger.runTest(ARRAY(Align29)));
  EXPECT_TRUE(DumpMunger.runTestForAssembly(ARRAY(Align29)));
  EXPECT_EQ("    %v0 = alloca i8, i32 %p0, align 536870912;\n",
            DumpMunger.getLinesWithSubstring("alloca"));
}

// Test how we recognize alignments in load i32 instructions.
TEST(IceParseInstsTests, LoadI32Alignment) {
  const uint64_t BitcodeRecords[] = {1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::MODULE_BLOCK_ID,
                                     2,
                                     Terminator,
                                     1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::TYPE_BLOCK_ID_NEW,
                                     2,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_NUMENTRY,
                                     2,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_INTEGER,
                                     32,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_FUNCTION,
                                     0,
                                     0,
                                     0,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator,
                                     3,
                                     naclbitc::MODULE_CODE_FUNCTION,
                                     1,
                                     0,
                                     0,
                                     3,
                                     Terminator,
                                     1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::FUNCTION_BLOCK_ID,
                                     2,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_DECLAREBLOCKS,
                                     1,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_INST_LOAD,
                                     1,
                                     getEncAlignPower(0),
                                     0,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_INST_RET,
                                     1,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator};

  const uint64_t ReplaceIndex = 9; // index for FUNC_CODE_INST_LOAD

  // Show text when alignment is 1.
  NaClObjDumpMunger DumpMunger(ARRAY_TERM(BitcodeRecords));
  EXPECT_TRUE(DumpMunger.runTest());
  EXPECT_EQ("      58:4|    3: <20, 1, 1, 0>         |    %v0 = load i32* %p0, "
            "align 1;\n",
            DumpMunger.getLinesWithSubstring("58:4"));
  IceTest::SubzeroBitcodeMunger Munger(ARRAY_TERM(BitcodeRecords));
  EXPECT_TRUE(Munger.runTest());

  // Show what happens when changing alignment to 0.
  const uint64_t Align0[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_LOAD,
      1,
      getEncAlignZero(),
      0,
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align0), ParseError));
  EXPECT_EQ("Error(58:4): Invalid function record: <20 1 0 0>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align0)));
  EXPECT_EQ("    %v0 = load i32* %p0, align 0;\n"
            "Error(58:4): load: Illegal alignment for i32. Expects: 1\n",
            DumpMunger.getLinesWithSubstring("load"));

  // Show what happens when changing alignment to 4.
  const uint64_t Align4[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_LOAD,
      1,
      getEncAlignPower(2),
      0,
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align4), ParseError));
  EXPECT_EQ("Error(58:4): Invalid function record: <20 1 3 0>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align4)));
  EXPECT_EQ("    %v0 = load i32* %p0, align 4;\n"
            "Error(58:4): load: Illegal alignment for i32. Expects: 1\n",
            DumpMunger.getLinesWithSubstring("load"));

  // Show what happens when changing alignment to 2**29.
  const uint64_t Align29[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_LOAD,
      1,
      getEncAlignPower(29),
      0,
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align29), ParseError));
  EXPECT_EQ("Error(58:4): Invalid function record: <20 1 30 0>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align29)));
  EXPECT_EQ("    %v0 = load i32* %p0, align 536870912;\n"
            "Error(58:4): load: Illegal alignment for i32. Expects: 1\n",
            DumpMunger.getLinesWithSubstring("load"));

  // Show what happens when changing alignment to 2**30.
  const uint64_t Align30[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_LOAD,
      1,
      getEncAlignPower(30),
      0,
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align30), ParseError));
  EXPECT_EQ("Error(58:4): Invalid function record: <20 1 31 0>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align30)));
  EXPECT_EQ("    %v0 = load i32* %p0, align 0;\n"
            "Error(58:4): load: Illegal alignment for i32. Expects: 1\n",
            DumpMunger.getLinesWithSubstring("load"));
}

// Test how we recognize alignments in load float instructions.
TEST(IceParseInstsTests, LoadFloatAlignment) {
  const uint64_t BitcodeRecords[] = {1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::MODULE_BLOCK_ID,
                                     2,
                                     Terminator,
                                     1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::TYPE_BLOCK_ID_NEW,
                                     2,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_NUMENTRY,
                                     3,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_FLOAT,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_INTEGER,
                                     32,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_FUNCTION,
                                     0,
                                     0,
                                     1,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator,
                                     3,
                                     naclbitc::MODULE_CODE_FUNCTION,
                                     2,
                                     0,
                                     0,
                                     3,
                                     Terminator,
                                     1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::FUNCTION_BLOCK_ID,
                                     2,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_DECLAREBLOCKS,
                                     1,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_INST_LOAD,
                                     1,
                                     getEncAlignPower(0),
                                     0,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_INST_RET,
                                     1,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator};

  const uint64_t ReplaceIndex = 10; // index for FUNC_CODE_INST_LOAD

  // Show text when alignment is 1.
  NaClObjDumpMunger DumpMunger(ARRAY_TERM(BitcodeRecords));
  EXPECT_TRUE(DumpMunger.runTest());
  EXPECT_EQ("      58:4|    3: <20, 1, 1, 0>         |    %v0 = load float* "
            "%p0, align 1;\n",
            DumpMunger.getLinesWithSubstring("58:4"));
  IceTest::SubzeroBitcodeMunger Munger(ARRAY_TERM(BitcodeRecords));
  EXPECT_TRUE(Munger.runTest());

  // Show what happens when changing alignment to 0.
  const uint64_t Align0[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_LOAD,
      1,
      getEncAlignZero(),
      0,
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align0), ParseError));
  EXPECT_EQ("Error(58:4): Invalid function record: <20 1 0 0>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align0)));
  EXPECT_EQ("    %v0 = load float* %p0, align 0;\n"
            "Error(58:4): load: Illegal alignment for float. Expects: 1 or 4\n",
            DumpMunger.getLinesWithSubstring("load"));

  // Show what happens when changing alignment to 4.
  const uint64_t Align4[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_LOAD,
      1,
      getEncAlignPower(2),
      0,
      Terminator,
  };
  EXPECT_TRUE(Munger.runTest(ARRAY(Align4)));
  EXPECT_TRUE(DumpMunger.runTestForAssembly(ARRAY(Align4)));
  EXPECT_EQ("    %v0 = load float* %p0, align 4;\n",
            DumpMunger.getLinesWithSubstring("load"));

  const uint64_t Align29[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_LOAD,
      1,
      getEncAlignPower(29),
      0,
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align29), ParseError));
  EXPECT_EQ("Error(58:4): Invalid function record: <20 1 30 0>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align29)));
  EXPECT_EQ("    %v0 = load float* %p0, align 536870912;\n"
            "Error(58:4): load: Illegal alignment for float. Expects: 1 or 4\n",
            DumpMunger.getLinesWithSubstring("load"));

  // Show what happens when changing alignment to 2**30.
  const uint64_t Align30[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_LOAD,
      1,
      getEncAlignPower(30),
      0,
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align30), ParseError));
  EXPECT_EQ("Error(58:4): Invalid function record: <20 1 31 0>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align30)));
  EXPECT_EQ("    %v0 = load float* %p0, align 0;\n"
            "Error(58:4): load: Illegal alignment for float. Expects: 1 or 4\n",
            DumpMunger.getLinesWithSubstring("load"));
}

// Test how we recognize alignments in store instructions.
TEST(NaClParseInstsTests, StoreAlignment) {
  const uint64_t BitcodeRecords[] = {1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::MODULE_BLOCK_ID,
                                     2,
                                     Terminator,
                                     1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::TYPE_BLOCK_ID_NEW,
                                     2,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_NUMENTRY,
                                     3,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_FLOAT,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_INTEGER,
                                     32,
                                     Terminator,
                                     3,
                                     naclbitc::TYPE_CODE_FUNCTION,
                                     0,
                                     0,
                                     1,
                                     0,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator,
                                     3,
                                     naclbitc::MODULE_CODE_FUNCTION,
                                     2,
                                     0,
                                     0,
                                     3,
                                     Terminator,
                                     1,
                                     naclbitc::BLK_CODE_ENTER,
                                     naclbitc::FUNCTION_BLOCK_ID,
                                     2,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_DECLAREBLOCKS,
                                     1,
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_INST_STORE,
                                     2,
                                     1,
                                     getEncAlignPower(0),
                                     Terminator,
                                     3,
                                     naclbitc::FUNC_CODE_INST_RET,
                                     1,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator,
                                     0,
                                     naclbitc::BLK_CODE_EXIT,
                                     Terminator};

  const uint64_t ReplaceIndex = 10; // index for FUNC_CODE_INST_STORE

  // Show text when alignment is 1.
  NaClObjDumpMunger DumpMunger(BitcodeRecords, array_lengthof(BitcodeRecords),
                               Terminator);
  EXPECT_TRUE(DumpMunger.runTest("Good Store Alignment 1"));
  EXPECT_EQ("      62:4|    3: <24, 2, 1, 1>         |    store float %p1, "
            "float* %p0, \n",
            DumpMunger.getLinesWithSubstring("62:4"));
  IceTest::SubzeroBitcodeMunger Munger(
      BitcodeRecords, array_lengthof(BitcodeRecords), Terminator);
  EXPECT_TRUE(Munger.runTest());

  // Show what happens when changing alignment to 0.
  const uint64_t Align0[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_STORE,
      2,
      1,
      getEncAlignZero(),
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align0), ParseError));
  EXPECT_EQ("Error(62:4): Invalid function record: <24 2 1 0>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align0)));
  EXPECT_EQ(
      "    store float %p1, float* %p0, align 0;\n"
      "Error(62:4): store: Illegal alignment for float. Expects: 1 or 4\n",
      DumpMunger.getLinesWithSubstring("store"));

  // Show what happens when changing alignment to 4.
  const uint64_t Align4[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_STORE,
      2,
      1,
      getEncAlignPower(2),
      Terminator,
  };
  EXPECT_TRUE(Munger.runTest(ARRAY(Align4)));
  EXPECT_TRUE(DumpMunger.runTestForAssembly(ARRAY(Align4)));

  // Show what happens when changing alignment to 8.
  const uint64_t Align8[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_STORE,
      2,
      1,
      getEncAlignPower(3),
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align8), ParseError));
  EXPECT_EQ("Error(62:4): Invalid function record: <24 2 1 4>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align8)));
  EXPECT_EQ(
      "    store float %p1, float* %p0, align 8;\n"
      "Error(62:4): store: Illegal alignment for float. Expects: 1 or 4\n",
      DumpMunger.getLinesWithSubstring("store"));

  // Show what happens when changing alignment to 2**29.
  const uint64_t Align29[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      3,
      naclbitc::FUNC_CODE_INST_STORE,
      2,
      1,
      getEncAlignPower(29),
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align29), ParseError));
  EXPECT_EQ("Error(62:4): Invalid function record: <24 2 1 30>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align29)));
  EXPECT_EQ(
      "    store float %p1, float* %p0, align 536870912;\n"
      "Error(62:4): store: Illegal alignment for float. Expects: 1 or 4\n",
      DumpMunger.getLinesWithSubstring("store"));

  const uint64_t Align30[] = {
      ReplaceIndex,
      NaClMungedBitcode::Replace,
      // Note: alignment stored as 0 or log2(Alignment)+1.
      3,
      naclbitc::FUNC_CODE_INST_STORE,
      2,
      1,
      getEncAlignPower(30),
      Terminator,
  };
  EXPECT_FALSE(Munger.runTest(ARRAY(Align30), ParseError));
  EXPECT_EQ("Error(62:4): Invalid function record: <24 2 1 31>\n",
            Munger.getTestResults());
  EXPECT_FALSE(DumpMunger.runTestForAssembly(ARRAY(Align30)));
  EXPECT_EQ(
      "    store float %p1, float* %p0, align 0;\n"
      "Error(62:4): store: Illegal alignment for float. Expects: 1 or 4\n",
      DumpMunger.getLinesWithSubstring("store"));
}

} // end of anonymous namespace
