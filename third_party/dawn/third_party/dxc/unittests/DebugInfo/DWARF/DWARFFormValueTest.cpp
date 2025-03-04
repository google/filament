//===- llvm/unittest/DebugInfo/DWARFFormValueTest.cpp ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/DebugInfo/DWARF/DWARFFormValue.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/LEB128.h"
#include "gtest/gtest.h"
#include <climits>
using namespace llvm;
using namespace dwarf;

namespace {

TEST(DWARFFormValue, FixedFormSizes) {
  // Size of DW_FORM_addr and DW_FORM_ref_addr are equal in DWARF2,
  // DW_FORM_ref_addr is always 4 bytes in DWARF32 starting from DWARF3.
  ArrayRef<uint8_t> sizes = DWARFFormValue::getFixedFormSizes(4, 2);
  EXPECT_EQ(sizes[DW_FORM_addr], sizes[DW_FORM_ref_addr]);
  sizes = DWARFFormValue::getFixedFormSizes(8, 2);
  EXPECT_EQ(sizes[DW_FORM_addr], sizes[DW_FORM_ref_addr]);
  sizes = DWARFFormValue::getFixedFormSizes(8, 3);
  EXPECT_EQ(4, sizes[DW_FORM_ref_addr]);
  // Check that we don't have fixed form sizes for weird address sizes.
  EXPECT_EQ(0U, DWARFFormValue::getFixedFormSizes(16, 2).size());
}

bool isFormClass(uint16_t Form, DWARFFormValue::FormClass FC) {
  return DWARFFormValue(Form).isFormClass(FC);
}

TEST(DWARFFormValue, FormClass) {
  EXPECT_TRUE(isFormClass(DW_FORM_addr, DWARFFormValue::FC_Address));
  EXPECT_FALSE(isFormClass(DW_FORM_data8, DWARFFormValue::FC_Address));
  EXPECT_TRUE(isFormClass(DW_FORM_data8, DWARFFormValue::FC_Constant));
  EXPECT_TRUE(isFormClass(DW_FORM_data8, DWARFFormValue::FC_SectionOffset));
  EXPECT_TRUE(
      isFormClass(DW_FORM_sec_offset, DWARFFormValue::FC_SectionOffset));
  EXPECT_TRUE(isFormClass(DW_FORM_GNU_str_index, DWARFFormValue::FC_String));
  EXPECT_TRUE(isFormClass(DW_FORM_GNU_addr_index, DWARFFormValue::FC_Address));
  EXPECT_FALSE(isFormClass(DW_FORM_ref_addr, DWARFFormValue::FC_Address));
  EXPECT_TRUE(isFormClass(DW_FORM_ref_addr, DWARFFormValue::FC_Reference));
  EXPECT_TRUE(isFormClass(DW_FORM_ref_sig8, DWARFFormValue::FC_Reference));
}

template<typename RawTypeT>
DWARFFormValue createDataXFormValue(uint16_t Form, RawTypeT Value) {
  char Raw[sizeof(RawTypeT)];
  memcpy(Raw, &Value, sizeof(RawTypeT));
  uint32_t Offset = 0;
  DWARFFormValue Result(Form);
  DataExtractor Data(StringRef(Raw, sizeof(RawTypeT)),
                     sys::IsLittleEndianHost, sizeof(void*));
  Result.extractValue(Data, &Offset, nullptr);
  return Result;
}

DWARFFormValue createULEBFormValue(uint64_t Value) {
  SmallString<10> RawData;
  raw_svector_ostream OS(RawData);
  encodeULEB128(Value, OS);
  uint32_t Offset = 0;
  DWARFFormValue Result(DW_FORM_udata);
  DataExtractor Data(OS.str(), sys::IsLittleEndianHost, sizeof(void*));
  Result.extractValue(Data, &Offset, nullptr);
  return Result;
}

DWARFFormValue createSLEBFormValue(int64_t Value) {
  SmallString<10> RawData;
  raw_svector_ostream OS(RawData);
  encodeSLEB128(Value, OS);
  uint32_t Offset = 0;
  DWARFFormValue Result(DW_FORM_sdata);
  DataExtractor Data(OS.str(), sys::IsLittleEndianHost, sizeof(void*));
  Result.extractValue(Data, &Offset, nullptr);
  return Result;
}

TEST(DWARFFormValue, SignedConstantForms) {
  // Check that we correctly sign extend fixed size forms.
  auto Sign1 = createDataXFormValue<uint8_t>(DW_FORM_data1, -123);
  auto Sign2 = createDataXFormValue<uint16_t>(DW_FORM_data2, -12345);
  auto Sign4 = createDataXFormValue<uint32_t>(DW_FORM_data4, -123456789);
  auto Sign8 = createDataXFormValue<uint64_t>(DW_FORM_data8, -1);
  EXPECT_EQ(Sign1.getAsSignedConstant().getValue(), -123);
  EXPECT_EQ(Sign2.getAsSignedConstant().getValue(), -12345);
  EXPECT_EQ(Sign4.getAsSignedConstant().getValue(), -123456789);
  EXPECT_EQ(Sign8.getAsSignedConstant().getValue(), -1);

  // Check that we can handle big positive values, but that we return
  // an error just over the limit.
  auto UMax = createULEBFormValue(LLONG_MAX);
  auto TooBig = createULEBFormValue(uint64_t(LLONG_MAX) + 1);
  EXPECT_EQ(UMax.getAsSignedConstant().getValue(), LLONG_MAX);
  EXPECT_EQ(TooBig.getAsSignedConstant().hasValue(), false);

  // Sanity check some other forms.
  auto Data1 = createDataXFormValue<uint8_t>(DW_FORM_data1, 120);
  auto Data2 = createDataXFormValue<uint16_t>(DW_FORM_data2, 32000);
  auto Data4 = createDataXFormValue<uint32_t>(DW_FORM_data4, 2000000000);
  auto Data8 = createDataXFormValue<uint64_t>(DW_FORM_data8, 0x1234567812345678LL);
  auto LEBMin = createSLEBFormValue(LLONG_MIN);
  auto LEBMax = createSLEBFormValue(LLONG_MAX);
  auto LEB1 = createSLEBFormValue(-42);
  auto LEB2 = createSLEBFormValue(42);
  EXPECT_EQ(Data1.getAsSignedConstant().getValue(), 120);
  EXPECT_EQ(Data2.getAsSignedConstant().getValue(), 32000);
  EXPECT_EQ(Data4.getAsSignedConstant().getValue(), 2000000000);
  EXPECT_EQ(Data8.getAsSignedConstant().getValue(), 0x1234567812345678LL);
  EXPECT_EQ(LEBMin.getAsSignedConstant().getValue(), LLONG_MIN);
  EXPECT_EQ(LEBMax.getAsSignedConstant().getValue(), LLONG_MAX);
  EXPECT_EQ(LEB1.getAsSignedConstant().getValue(), -42);
  EXPECT_EQ(LEB2.getAsSignedConstant().getValue(), 42);
}

} // end anonymous namespace
