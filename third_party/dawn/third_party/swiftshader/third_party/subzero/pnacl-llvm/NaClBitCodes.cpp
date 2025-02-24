//===- NaClBitcodeHeader.cpp ----------------------------------------------===//
//     PNaCl bitcode header reader.
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation of Bitcode abbrevations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Bitcode/NaCl/NaClBitCodes.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

const bool NaClBitCodeAbbrevOp::HasValueArray[] = {
    true,  // Literal
    true,  // Fixed
    true,  // VBR
    false, // Array
    false  // Char6
};

const char *NaClBitCodeAbbrevOp::EncodingNameArray[] = {
    "Literal", "Fixed", "VBR", "Array", "Char6"};

NaClBitCodeAbbrevOp::NaClBitCodeAbbrevOp(Encoding E, uint64_t Data)
    : Enc(E), Val(Data) {
  if (isValid(E, Data))
    return;
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Invalid NaClBitCodeAbbrevOp(" << E << ", " << Data << ")";
  report_fatal_error(StrBuf.str());
}

bool NaClBitCodeAbbrevOp::isValid(Encoding E, uint64_t Val) {
  switch (NaClBitCodeAbbrevOp::Encoding(E)) {
  case Literal:
    return true;
  case Fixed:
  case VBR:
    return Val <= naclbitc::MaxAbbrevWidth;
  case Char6:
  case Array:
    return Val == 0;
  }
  llvm_unreachable("unhandled abbreviation");
}

void NaClBitCodeAbbrevOp::Print(raw_ostream &Stream) const {
  if (Enc == Literal) {
    Stream << getValue();
    return;
  }
  Stream << getEncodingName(Enc);
  if (!hasValue())
    return;
  Stream << "(" << Val << ")";
}

static void PrintExpression(raw_ostream &Stream,
                            const NaClBitCodeAbbrev *Abbrev, unsigned &Index) {
  // Bail out early, in case we are incrementally building the
  // expression and the argument is not available yet.
  if (Index >= Abbrev->getNumOperandInfos())
    return;

  const NaClBitCodeAbbrevOp &Op = Abbrev->getOperandInfo(Index);
  Op.Print(Stream);
  if (unsigned NumArgs = Op.NumArguments()) {
    Stream << "(";
    for (unsigned i = 0; i < NumArgs; ++i) {
      ++Index;
      if (i > 0)
        Stream << ",";
      PrintExpression(Stream, Abbrev, Index);
    }
    Stream << ")";
  }
}

void NaClBitCodeAbbrev::Print(raw_ostream &Stream, bool AddNewLine) const {
  Stream << "[";
  for (unsigned i = 0; i < getNumOperandInfos(); ++i) {
    if (i > 0)
      Stream << ", ";
    PrintExpression(Stream, this, i);
  }
  Stream << "]";
  if (AddNewLine)
    Stream << "\n";
}

NaClBitCodeAbbrev *NaClBitCodeAbbrev::Simplify() const {
  NaClBitCodeAbbrev *Abbrev = new NaClBitCodeAbbrev();
  for (unsigned i = 0; i < OperandList.size(); ++i) {
    const NaClBitCodeAbbrevOp &Op = OperandList[i];
    // Simplify if possible.  Currently, the only simplification known
    // is to remove unnecessary operands appearing immediately before an
    // array operator. That is, apply the simplification:
    //    Op Array(Op) -> Array(Op)
    assert(!Op.isArrayOp() || i == OperandList.size() - 2);
    while (Op.isArrayOp() && !Abbrev->OperandList.empty() &&
           Abbrev->OperandList.back() == OperandList[i + 1]) {
      Abbrev->OperandList.pop_back();
    }
    Abbrev->OperandList.push_back(Op);
  }
  return Abbrev;
}

bool NaClBitCodeAbbrev::isValid() const {
  // Verify that an array op appears can only appear if it is the
  // second to last element.
  unsigned NumOperands = getNumOperandInfos();
  if (NumOperands == 0)
    return false;
  for (unsigned i = 0; i < NumOperands; ++i) {
    const NaClBitCodeAbbrevOp &Op = getOperandInfo(i);
    if (Op.isArrayOp() && i + 2 != NumOperands)
      // Note: Unlike LLVM bitcode, we allow literals in arrays!
      return false;
  }
  return true;
}
