//===- NaClBitcodeHeader.cpp ----------------------------------------------===//
//     PNaCl bitcode header reader.
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Bitcode/NaCl/NaClBitcodeHeader.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Bitcode/NaCl/NaClReaderWriter.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/StreamingMemoryObject.h"
#include "llvm/Support/raw_ostream.h"

#include <cstring>
#include <iomanip>
#include <limits>

using namespace llvm;

namespace {

// The name for each ID tag.
static const char *TagName[] = {
    "Invalid",              // kInvalid
    "PNaCl Version",        // kPNaClVersion
    "Align bitcode records" // kAlignBitcodeRecords
};

// The name for each field type.
static const char *FieldTypeName[] = {
    "uint8[]", // kBufferType
    "uint32",  // kUInt32Type
    "flag",    // kFlagType
    "unknown"  // kUnknownType
};

// The type associated with each ID tag.
static const NaClBitcodeHeaderField::FieldType ExpectedType[] = {
    NaClBitcodeHeaderField::kUnknownType, // kInvalid
    NaClBitcodeHeaderField::kUInt32Type,  // kPNaClVersion
    NaClBitcodeHeaderField::kFlagType     // kAlignBitcodeRecords
};

} // end of anonymous namespace

const char *NaClBitcodeHeaderField::IDName(Tag ID) {
  return ID > kTag_MAX ? "???" : TagName[ID];
}

const char *NaClBitcodeHeaderField::TypeName(FieldType FType) {
  return FType > kFieldType_MAX ? "???" : FieldTypeName[FType];
}

NaClBitcodeHeaderField::NaClBitcodeHeaderField()
    : ID(kInvalid), FType(kBufferType), Len(0), Data(0) {}

NaClBitcodeHeaderField::NaClBitcodeHeaderField(Tag MyID)
    : ID(MyID), FType(kFlagType), Len(0), Data(0) {
  assert(MyID <= kTag_MAX);
}

NaClBitcodeHeaderField::NaClBitcodeHeaderField(Tag MyID, uint32_t MyValue)
    : ID(MyID), FType(kUInt32Type), Len(4), Data(new uint8_t[4]) {
  assert(MyID <= kTag_MAX);
  Data[0] = static_cast<uint8_t>(MyValue & 0xFF);
  Data[1] = static_cast<uint8_t>((MyValue >> 8) & 0xFF);
  Data[2] = static_cast<uint8_t>((MyValue >> 16) & 0xFF);
  Data[3] = static_cast<uint8_t>((MyValue >> 24) & 0xFF);
}

uint32_t NaClBitcodeHeaderField::GetUInt32Value() const {
  assert(FType == kUInt32Type && "Header field must be uint32");
  return static_cast<uint32_t>(Data[0]) |
         (static_cast<uint32_t>(Data[1]) << 8) |
         (static_cast<uint32_t>(Data[2]) << 16) |
         (static_cast<uint32_t>(Data[2]) << 24);
}

NaClBitcodeHeaderField::NaClBitcodeHeaderField(Tag MyID, size_t MyLen,
                                               uint8_t *MyData)
    : ID(MyID), FType(kBufferType), Len(MyLen), Data(new uint8_t[MyLen]) {
  assert(MyID <= kTag_MAX);
  for (size_t i = 0; i < MyLen; ++i) {
    Data[i] = MyData[i];
  }
}

bool NaClBitcodeHeaderField::Write(uint8_t *Buf, size_t BufLen) const {
  size_t FieldsLen = kTagLenSize + Len;
  size_t PadLen = (WordSize - (FieldsLen & (WordSize - 1))) & (WordSize - 1);
  // Ensure buffer is large enough and that length can be represented
  // in 32 bits
  if (BufLen < FieldsLen + PadLen ||
      Len > std::numeric_limits<FixedSubfield>::max())
    return false;

  WriteFixedSubfield(EncodeTypedID(), Buf);
  WriteFixedSubfield(static_cast<FixedSubfield>(Len),
                     Buf + sizeof(FixedSubfield));
  memcpy(Buf + kTagLenSize, Data, Len);
  // Pad out to word alignment
  if (PadLen) {
    memset(Buf + FieldsLen, 0, PadLen);
  }
  return true;
}

bool NaClBitcodeHeaderField::Read(const uint8_t *Buf, size_t BufLen) {
  if (BufLen < kTagLenSize)
    return false;
  FixedSubfield IdField;
  ReadFixedSubfield(&IdField, Buf);
  FixedSubfield LengthField;
  ReadFixedSubfield(&LengthField, Buf + sizeof(FixedSubfield));
  size_t Length = static_cast<size_t>(LengthField);
  if (BufLen < kTagLenSize + Length)
    return false;
  if (Len != Length) {
    // Need to reallocate data buffer.
    if (Data)
      delete[] Data;
    Data = new uint8_t[Length];
  }
  Len = Length;
  DecodeTypedID(IdField, ID, FType);
  memcpy(Data, Buf + kTagLenSize, Len);
  return true;
}

std::string NaClBitcodeHeaderField::Contents() const {
  std::string buffer;
  raw_string_ostream ss(buffer);
  ss << IDName() << ": ";
  switch (FType) {
  case kFlagType:
    ss << "true";
    break;
  case kUInt32Type:
    ss << GetUInt32Value();
    break;
  case kBufferType:
    ss << "[";
    for (size_t i = 0; i < Len; ++i) {
      if (i)
        ss << " ";
      ss << format("%02x", Data[i]);
    }
    ss << "]";
    break;
  case kUnknownType:
    ss << "unknown value";
    break;
  }
  return ss.str();
}

NaClBitcodeHeader::NaClBitcodeHeader()
    : HeaderSize(0), UnsupportedMessage(), IsSupportedFlag(false),
      IsReadableFlag(false), PNaClVersion(0) {}

NaClBitcodeHeader::~NaClBitcodeHeader() {
  for (std::vector<NaClBitcodeHeaderField *>::const_iterator
           Iter = Fields.begin(),
           IterEnd = Fields.end();
       Iter != IterEnd; ++Iter) {
    delete *Iter;
  }
}

bool NaClBitcodeHeader::ReadPrefix(const unsigned char *BufPtr,
                                   const unsigned char *BufEnd,
                                   unsigned &NumFields, unsigned &NumBytes) {
  // Must contain PEXE.
  if (!isNaClBitcode(BufPtr, BufEnd)) {
    UnsupportedMessage = "Invalid PNaCl bitcode header";
    if (isBitcode(BufPtr, BufEnd)) {
      UnsupportedMessage += " (to run in Chrome, bitcode files must be "
                            "finalized using pnacl-finalize)";
    }
    return true;
  }
  BufPtr += WordSize;

  // Read #Fields and number of bytes needed for the header.
  if (BufPtr + WordSize > BufEnd)
    return UnsupportedError("Bitcode read failure");
  NumFields = static_cast<unsigned>(BufPtr[0]) |
              (static_cast<unsigned>(BufPtr[1]) << 8);
  NumBytes = static_cast<unsigned>(BufPtr[2]) |
             (static_cast<unsigned>(BufPtr[3]) << 8);
  BufPtr += WordSize;
  return false;
}

bool NaClBitcodeHeader::ReadFields(const unsigned char *BufPtr,
                                   const unsigned char *BufEnd,
                                   unsigned NumFields, unsigned NumBytes) {
  HeaderSize = NumBytes + (2 * WordSize);

  // Read in each field.
  for (size_t i = 0; i < NumFields; ++i) {
    NaClBitcodeHeaderField *Field = new NaClBitcodeHeaderField();
    Fields.push_back(Field);
    if (!Field->Read(BufPtr, BufEnd - BufPtr))
      return UnsupportedError("Bitcode read failure");
    size_t FieldSize = Field->GetTotalSize();
    BufPtr += FieldSize;
  }
  return false;
}

bool NaClBitcodeHeader::Read(const unsigned char *BufPtr,
                             const unsigned char *BufEnd) {
  unsigned NumFields;
  unsigned NumBytes;
  if (ReadPrefix(BufPtr, BufEnd, NumFields, NumBytes))
    return true; // ReadPrefix sets UnsupportedMessage
  BufPtr += 2 * WordSize;

  if (ReadFields(BufPtr, BufEnd, NumFields, NumBytes))
    return true; // ReadFields sets UnsupportedMessage
  BufPtr += NumBytes;
  InstallFields();
  return false;
}

bool NaClBitcodeHeader::Read(MemoryObject *Bytes) {
  unsigned NumFields;
  unsigned NumBytes;
  // First, read the prefix, which is 2 * WordSize, to determine the
  // NumBytes and NumFields.
  {
    unsigned char Buffer[2 * WordSize];
    if (Bytes->readBytes(Buffer, sizeof(Buffer), 0) != sizeof(Buffer))
      return UnsupportedError("Bitcode read failure");
    if (ReadPrefix(Buffer, Buffer + sizeof(Buffer), NumFields, NumBytes))
      return true; // ReadPrefix sets UnsupportedMessage
  }
  // Then read the rest, starting after the 2 * WordSize of the prefix.
  uint8_t *Header = new uint8_t[NumBytes];
  bool failed = Bytes->readBytes(Header, NumBytes, 2 * WordSize) != NumBytes ||
                ReadFields(Header, Header + NumBytes, NumFields, NumBytes);
  delete[] Header;
  if (failed)
    return UnsupportedError("Bitcode read failure");
  InstallFields();
  return false;
}

NaClBitcodeHeaderField *
NaClBitcodeHeader::GetTaggedField(NaClBitcodeHeaderField::Tag ID) const {
  for (std::vector<NaClBitcodeHeaderField *>::const_iterator
           Iter = Fields.begin(),
           IterEnd = Fields.end();
       Iter != IterEnd; ++Iter) {
    if ((*Iter)->GetID() == ID) {
      return *Iter;
    }
  }
  return 0;
}

NaClBitcodeHeaderField *NaClBitcodeHeader::GetField(size_t index) const {
  if (index >= Fields.size())
    return 0;
  return Fields[index];
}

NaClBitcodeHeaderField *GetPNaClVersionPtr(NaClBitcodeHeader *Header) {
  if (NaClBitcodeHeaderField *Version =
          Header->GetTaggedField(NaClBitcodeHeaderField::kPNaClVersion)) {
    if (Version->GetType() == NaClBitcodeHeaderField::kUInt32Type) {
      return Version;
    }
  }
  return 0;
}

void NaClBitcodeHeader::InstallFields() {
  IsSupportedFlag = true;
  IsReadableFlag = true;
  AlignBitcodeRecords = false;
  PNaClVersion = 0;
  UnsupportedMessage.clear();
  SmallSet<unsigned, NaClBitcodeHeaderField::kTag_MAX> FieldIDs;

  auto ReportProblem = [&](bool IsReadable) {
    UnsupportedMessage.append("\n");
    IsSupportedFlag = false;
    IsReadableFlag = IsReadableFlag && IsReadable;
  };

  auto ReportProblemWithContents = [&](NaClBitcodeHeaderField *Field,
                                       bool IsReadable) {
    UnsupportedMessage.append(": ");
    UnsupportedMessage.append(Field->Contents());
    ReportProblem(IsReadable);
  };

  for (size_t i = 0, e = NumberFields(); i < e; ++i) {
    // Start by checking expected properties for any field
    NaClBitcodeHeaderField *Field = GetField(i);
    if (!FieldIDs.insert(Field->GetID()).second) {
      UnsupportedMessage.append("Specified multiple times: ");
      UnsupportedMessage.append(Field->IDName());
      ReportProblem(false);
      continue;
    }
    NaClBitcodeHeaderField::FieldType ExpectedTy = ExpectedType[Field->GetID()];
    if (Field->GetType() != ExpectedTy) {
      UnsupportedMessage.append("Expects type ");
      UnsupportedMessage.append(NaClBitcodeHeaderField::TypeName(ExpectedTy));
      ReportProblemWithContents(Field, false);
      continue;
    }
    if (Field->GetType() == NaClBitcodeHeaderField::kUnknownType) {
      UnsupportedMessage.append("Unknown value");
      ReportProblemWithContents(Field, false);
      continue;
    }

    // Check specific ID values and install.
    switch (Field->GetID()) {
    case NaClBitcodeHeaderField::kInvalid:
      UnsupportedMessage.append("Unsupported");
      ReportProblemWithContents(Field, false);
      continue;
    case NaClBitcodeHeaderField::kPNaClVersion:
      PNaClVersion = Field->GetUInt32Value();
      if (PNaClVersion != 2) {
        UnsupportedMessage.append("Unsupported");
        ReportProblemWithContents(Field, false);
        continue;
      }
      break;
    case NaClBitcodeHeaderField::kAlignBitcodeRecords:
      AlignBitcodeRecords = true;
      UnsupportedMessage.append("Unsupported");
      ReportProblemWithContents(Field, true);
      continue;
    }
  }
}
