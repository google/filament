//===- unittest/IceParseTypesTest.cpp -------------------------------------===//
//     Tests parser for PNaCl bitcode.
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Tests record errors in the types block when parsing PNaCl bitcode.

// TODO(kschimpf) Add more tests.

#include "BitcodeMunge.h"
#include "unittests/Bitcode/NaClMungeTest.h"

using namespace llvm;
using namespace naclmungetest;
using namespace IceTest;

namespace {

static const unsigned NO_LOCAL_ABBREVS =
    NaClBitsNeededForValue(naclbitc::DEFAULT_MAX_ABBREV);

const uint64_t BitcodeRecords[] = {naclbitc::ENTER_SUBBLOCK,
                                   naclbitc::BLK_CODE_ENTER,
                                   naclbitc::MODULE_BLOCK_ID,
                                   NO_LOCAL_ABBREVS,
                                   Terminator,
                                   naclbitc::ENTER_SUBBLOCK,
                                   naclbitc::BLK_CODE_ENTER,
                                   naclbitc::TYPE_BLOCK_ID_NEW,
                                   NO_LOCAL_ABBREVS,
                                   Terminator,
                                   naclbitc::UNABBREV_RECORD,
                                   naclbitc::TYPE_CODE_NUMENTRY,
                                   2,
                                   Terminator,
                                   naclbitc::UNABBREV_RECORD,
                                   naclbitc::TYPE_CODE_INTEGER,
                                   32,
                                   Terminator,
                                   naclbitc::UNABBREV_RECORD,
                                   naclbitc::TYPE_CODE_FLOAT,
                                   Terminator,
                                   naclbitc::END_BLOCK,
                                   naclbitc::BLK_CODE_EXIT,
                                   Terminator,
                                   naclbitc::END_BLOCK,
                                   naclbitc::BLK_CODE_EXIT,
                                   Terminator};

const char *ExpectedDump =
    "       0:0|<65532, 80, 69, 88, 69, 1, 0,|Magic Number: 'PEXE' (80, 69, "
    "88, 69)\n"
    "          | 8, 0, 17, 0, 4, 0, 2, 0, 0, |PNaCl Version: 2\n"
    "          | 0>                          |\n"
    "      16:0|1: <65535, 8, 2>             |module {  // BlockID = 8\n"
    "      24:0|  1: <65535, 17, 2>          |  types {  // BlockID = 17\n"
    "      32:0|    3: <1, 2>                |    count 2;\n"
    "      34:4|    3: <7, 32>               |    @t0 = i32;\n"
    "      37:6|    3: <3>                   |    @t1 = float;\n"
    "      39:4|  0: <65534>                 |  }\n"
    "      40:0|0: <65534>                   |}\n";

TEST(NaClParseTypesTest, ShowExpectedDump) {
  NaClObjDumpMunger Munger(ARRAY_TERM(BitcodeRecords));
  EXPECT_TRUE(Munger.runTest());
  EXPECT_EQ(ExpectedDump, Munger.getTestResults());
}

// Show what happens when misdefining: @t1 = float"
TEST(NaClParseTypesTest, BadFloatTypeDefinition) {
  // Index for "@t1 = float;" record.
  const uint64_t FloatTypeIndex = 4;
  const uint64_t Edit[] = {FloatTypeIndex, NaClMungedBitcode::Replace,
                           // Add extraneous 1 to end of float record.
                           naclbitc::UNABBREV_RECORD, naclbitc::TYPE_CODE_FLOAT,
                           1, Terminator};

  SubzeroBitcodeMunger Munger(ARRAY_TERM(BitcodeRecords));
  EXPECT_FALSE(Munger.runTest(ARRAY(Edit)));
  EXPECT_EQ("Error(37:6): Invalid type record: <3 1>\n",
            Munger.getTestResults());
}

// Show what happens when the count record value is way too big.
// See: https://code.google.com/p/nativeclient/issues/detail?id=4195
TEST(NaClParseTypesTest, BadTypeCountRecord) {
  // Index for "count 2;".
  const uint64_t CountRecordIndex = 2;
  const uint64_t Edit[] = {
      CountRecordIndex,          NaClMungedBitcode::Replace,
      naclbitc::UNABBREV_RECORD, naclbitc::TYPE_CODE_NUMENTRY,
      18446744073709547964ULL,   Terminator};

  SubzeroBitcodeMunger Munger(ARRAY_TERM(BitcodeRecords));
  Munger.Flags.setGenerateUnitTestMessages(false);
  EXPECT_FALSE(Munger.runTest(ARRAY(Edit)));
  EXPECT_EQ("Error(32:0): Size to big for count record: 18446744073709547964\n"
            "Error(48:4): Types block expected 4294963644 types but found: 2\n",
            Munger.getTestResults());
}

} // end of anonymous namespace
