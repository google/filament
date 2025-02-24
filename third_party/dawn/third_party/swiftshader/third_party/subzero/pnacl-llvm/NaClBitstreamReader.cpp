//===- NaClBitstreamReader.cpp --------------------------------------------===//
//     NaClBitstreamReader implementation
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Bitcode/NaCl/NaClBitstreamReader.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

static const char *ErrorLevelName[] = {"Warning", "Error", "Fatal"};

} // End of anonymous namespace.

std::string llvm::naclbitc::getBitAddress(uint64_t Bit) {
  std::string Buffer;
  raw_string_ostream Stream(Buffer);
  Stream << (Bit / 8) << ":" << (Bit % 8);
  return Stream.str();
}

raw_ostream &llvm::naclbitc::ErrorAt(raw_ostream &Out, ErrorLevel Level,
                                     uint64_t BitPosition) {
  assert(Level < array_lengthof(::ErrorLevelName));
  return Out << ErrorLevelName[Level] << "("
             << naclbitc::getBitAddress(BitPosition) << "): ";
}

//===----------------------------------------------------------------------===//
//  NaClBitstreamCursor implementation
//===----------------------------------------------------------------------===//

void NaClBitstreamCursor::ErrorHandler::Fatal(
    const std::string &ErrorMessage) const {
  // Default implementation is simply print message, and the bit where
  // the error occurred.
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  naclbitc::ErrorAt(StrBuf, naclbitc::Fatal,
                    Cursor.getErrorBitNo(getCurrentBitNo()))
      << ErrorMessage;
  report_fatal_error(StrBuf.str());
}

void NaClBitstreamCursor::reportInvalidAbbrevNumber(unsigned AbbrevNo) const {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Invalid abbreviation # " << AbbrevNo << " defined for record";
  ErrHandler->Fatal(StrBuf.str());
}

void NaClBitstreamCursor::reportInvalidJumpToBit(uint64_t BitNo) const {
  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Invalid jump to bit " << BitNo;
  ErrHandler->Fatal(StrBuf.str());
}

/// EnterSubBlock - Having read the ENTER_SUBBLOCK abbrevid, enter
/// the block, and return true if the block has an error.
bool NaClBitstreamCursor::EnterSubBlock(unsigned BlockID, unsigned *NumWordsP) {
  const bool IsFixed = true;
  NaClBitcodeSelectorAbbrev CodeAbbrev(IsFixed,
                                       ReadVBR(naclbitc::CodeLenWidth));
  BlockScope.push_back(Block(BitStream->getBlockInfo(BlockID), CodeAbbrev));
  SkipToFourByteBoundary();
  unsigned NumWords = Read(naclbitc::BlockSizeWidth);
  if (NumWordsP)
    *NumWordsP = NumWords;

  // Validate that this block is sane.
  if (BlockScope.back().getCodeAbbrev().NumBits == 0 || AtEndOfStream())
    return true;

  return false;
}

void NaClBitstreamCursor::skipAbbreviatedField(const NaClBitCodeAbbrevOp &Op) {
  // Decode the value as we are commanded.
  switch (Op.getEncoding()) {
  case NaClBitCodeAbbrevOp::Literal:
    // No read necessary for literal.
    break;
  case NaClBitCodeAbbrevOp::Fixed:
    (void)Read((unsigned)Op.getValue());
    break;
  case NaClBitCodeAbbrevOp::VBR:
    (void)ReadVBR64((unsigned)Op.getValue());
    break;
  case NaClBitCodeAbbrevOp::Array:
    // This can't happen because the abbreviation must be valid.
    llvm_unreachable("Bad array abbreviation encoding!");
    break;
  case NaClBitCodeAbbrevOp::Char6:
    (void)Read(6);
    break;
  }
}

/// skipRecord - Read the current record and discard it.
void NaClBitstreamCursor::skipRecord(unsigned AbbrevID) {
  // Skip unabbreviated records by reading past their entries.
  if (AbbrevID == naclbitc::UNABBREV_RECORD) {
    unsigned Code = ReadVBR(6);
    (void)Code;
    unsigned NumElts = ReadVBR(6);
    for (unsigned i = 0; i != NumElts; ++i)
      (void)ReadVBR64(6);
    SkipToByteBoundaryIfAligned();
    return;
  }

  const NaClBitCodeAbbrev *Abbv = getAbbrev(AbbrevID);

  for (unsigned i = 0, e = Abbv->getNumOperandInfos(); i != e; ++i) {
    const NaClBitCodeAbbrevOp &Op = Abbv->getOperandInfo(i);
    switch (Op.getEncoding()) {
    default:
      skipAbbreviatedField(Op);
      break;
    case NaClBitCodeAbbrevOp::Literal:
      break;
    case NaClBitCodeAbbrevOp::Array: {
      // Array case.  Read the number of elements as a vbr6.
      unsigned NumElts = ReadVBR(6);

      // Get the element encoding.
      const NaClBitCodeAbbrevOp &EltEnc = Abbv->getOperandInfo(++i);

      // Read all the elements.
      for (; NumElts; --NumElts)
        skipAbbreviatedField(EltEnc);
      break;
    }
    }
  }
  SkipToByteBoundaryIfAligned();
}

bool NaClBitstreamCursor::readRecordAbbrevField(const NaClBitCodeAbbrevOp &Op,
                                                uint64_t &Value) {
  switch (Op.getEncoding()) {
  case NaClBitCodeAbbrevOp::Literal:
    Value = Op.getValue();
    break;
  case NaClBitCodeAbbrevOp::Array:
    // Returns number of elements in the array.
    Value = ReadVBR(6);
    return true;
  case NaClBitCodeAbbrevOp::Fixed:
    Value = Read((unsigned)Op.getValue());
    break;
  case NaClBitCodeAbbrevOp::VBR:
    Value = ReadVBR64((unsigned)Op.getValue());
    break;
  case NaClBitCodeAbbrevOp::Char6:
    Value = NaClBitCodeAbbrevOp::DecodeChar6(Read(6));
    break;
  }
  return false;
}

uint64_t
NaClBitstreamCursor::readArrayAbbreviatedField(const NaClBitCodeAbbrevOp &Op) {
  // Decode the value as we are commanded.
  switch (Op.getEncoding()) {
  case NaClBitCodeAbbrevOp::Literal:
    return Op.getValue();
  case NaClBitCodeAbbrevOp::Fixed:
    return Read((unsigned)Op.getValue());
  case NaClBitCodeAbbrevOp::VBR:
    return ReadVBR64((unsigned)Op.getValue());
  case NaClBitCodeAbbrevOp::Array:
    // This can't happen because the abbreviation must be valid.
    llvm_unreachable("Bad array abbreviation encoding!");
    break;
  case NaClBitCodeAbbrevOp::Char6:
    return NaClBitCodeAbbrevOp::DecodeChar6(Read(6));
  }
  llvm_unreachable("Illegal abbreviation encoding for field!");
}

void NaClBitstreamCursor::readArrayAbbrev(const NaClBitCodeAbbrevOp &Op,
                                          unsigned NumArrayElements,
                                          SmallVectorImpl<uint64_t> &Vals) {
  for (; NumArrayElements; --NumArrayElements) {
    Vals.push_back(readArrayAbbreviatedField(Op));
  }
}

unsigned NaClBitstreamCursor::readRecord(unsigned AbbrevID,
                                         SmallVectorImpl<uint64_t> &Vals) {
  if (AbbrevID == naclbitc::UNABBREV_RECORD) {
    unsigned Code = ReadVBR(6);
    unsigned NumElts = ReadVBR(6);
    for (unsigned i = 0; i != NumElts; ++i)
      Vals.push_back(ReadVBR64(6));
    SkipToByteBoundaryIfAligned();
    return Code;
  }

  // Read code.
  const NaClBitCodeAbbrev *Abbv = getAbbrev(AbbrevID);
  uint64_t Value;
  unsigned Code;
  if (readRecordAbbrevField(Abbv->getOperandInfo(0), Value)) {
    // Array found, use to read all elements.
    if (Value == 0)
      ErrHandler->Fatal("No code found for record!");
    const NaClBitCodeAbbrevOp &Op = Abbv->getOperandInfo(1);
    Code = readArrayAbbreviatedField(Op);
    readArrayAbbrev(Op, Value - 1, Vals);
    SkipToByteBoundaryIfAligned();
    return Code;
  }
  Code = Value;

  // Read arguments.
  unsigned NumOperands = Abbv->getNumOperandInfos();
  for (unsigned i = 1; i != NumOperands; ++i) {
    if (readRecordAbbrevField(Abbv->getOperandInfo(i), Value)) {
      ++i;
      readArrayAbbrev(Abbv->getOperandInfo(i), Value, Vals);
      SkipToByteBoundaryIfAligned();
      return Code;
    }
    Vals.push_back(Value);
  }
  SkipToByteBoundaryIfAligned();
  return Code;
}

NaClBitCodeAbbrevOp::Encoding NaClBitstreamCursor::getEncoding(uint64_t Value) {
  if (!NaClBitCodeAbbrevOp::isValidEncoding(Value)) {
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << "Invalid abbreviation encoding specified in bitcode file: "
           << Value;
    ErrHandler->Fatal(StrBuf.str());
  }
  return NaClBitCodeAbbrevOp::Encoding(Value);
}

void NaClBitstreamCursor::ReadAbbrevRecord(bool IsLocal,
                                           NaClAbbrevListener *Listener) {
  NaClBitCodeAbbrev *Abbv = BlockScope.back().appendLocalCreate();
  unsigned NumOpInfo = ReadVBR(5);
  if (Listener)
    Listener->Values.push_back(NumOpInfo);
  for (unsigned i = 0; i != NumOpInfo; ++i) {
    bool IsLiteral = Read(1) ? true : false;
    if (Listener)
      Listener->Values.push_back(IsLiteral);
    if (IsLiteral) {
      uint64_t Value = ReadVBR64(8);
      if (Listener)
        Listener->Values.push_back(Value);
      Abbv->Add(NaClBitCodeAbbrevOp(Value));
      continue;
    }
    NaClBitCodeAbbrevOp::Encoding E = getEncoding(Read(3));
    if (Listener)
      Listener->Values.push_back(E);
    if (NaClBitCodeAbbrevOp::hasValue(E)) {
      unsigned Data = ReadVBR64(5);
      if (Listener)
        Listener->Values.push_back(Data);

      // As a special case, handle fixed(0) (i.e., a fixed field with zero bits)
      // and vbr(0) as a literal zero.  This is decoded the same way, and avoids
      // a slow path in Read() to have to handle reading zero bits.
      if ((E == NaClBitCodeAbbrevOp::Fixed || E == NaClBitCodeAbbrevOp::VBR) &&
          Data == 0) {
        if (Listener)
          Listener->Values.push_back(0);
        Abbv->Add(NaClBitCodeAbbrevOp(0));
        continue;
      }
      if (!NaClBitCodeAbbrevOp::isValid(E, Data)) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Invalid abbreviation encoding ("
               << NaClBitCodeAbbrevOp::getEncodingName(E) << ", " << Data
               << ")";
        ErrHandler->Fatal(StrBuf.str());
      }
      Abbv->Add(NaClBitCodeAbbrevOp(E, Data));
    } else {
      if (!NaClBitCodeAbbrevOp::isValid(E)) {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Invalid abbreviation encoding ("
               << NaClBitCodeAbbrevOp::getEncodingName(E) << ")";
        ErrHandler->Fatal(StrBuf.str());
      }
      Abbv->Add(NaClBitCodeAbbrevOp(E));
    }
  }
  SkipToByteBoundaryIfAligned();
  if (!Abbv->isValid())
    ErrHandler->Fatal("Invalid abbreviation specified in bitcode file");
  if (Listener) {
    Listener->ProcessAbbreviation(Abbv, IsLocal);
    // Reset record information of the listener.
    Listener->Values.clear();
    Listener->StartBit = GetCurrentBitNo();
  }
}

void NaClBitstreamCursor::SkipAbbrevRecord() {
  unsigned NumOpInfo = ReadVBR(5);
  for (unsigned i = 0; i != NumOpInfo; ++i) {
    bool IsLiteral = Read(1) ? true : false;
    if (IsLiteral) {
      ReadVBR64(8);
      continue;
    }
    NaClBitCodeAbbrevOp::Encoding E = getEncoding(Read(3));
    if (NaClBitCodeAbbrevOp::hasValue(E)) {
      ReadVBR64(5);
    }
  }
  SkipToByteBoundaryIfAligned();
}

namespace {

unsigned ValidBlockIDs[] = {
    naclbitc::BLOCKINFO_BLOCK_ID, naclbitc::CONSTANTS_BLOCK_ID,
    naclbitc::FUNCTION_BLOCK_ID,  naclbitc::GLOBALVAR_BLOCK_ID,
    naclbitc::MODULE_BLOCK_ID,    naclbitc::TOP_LEVEL_BLOCKID,
    naclbitc::TYPE_BLOCK_ID_NEW,  naclbitc::VALUE_SYMTAB_BLOCK_ID};

} // end of anonymous namespace

NaClBitstreamReader::BlockInfoRecordsMap::BlockInfoRecordsMap()
    : IsFrozen(false) {
  for (size_t BlockID : ValidBlockIDs) {
    std::unique_ptr<BlockInfo> Info(new BlockInfo(BlockID));
    KnownInfos.emplace(BlockID, std::move(Info));
  }
}

NaClBitstreamReader::BlockInfo *
NaClBitstreamReader::BlockInfoRecordsMap::getOrCreateUnknownBlockInfo(
    unsigned BlockID) {
  std::unique_lock<std::mutex> Lock(UnknownBlockInfoLock);
  while (true) {
    auto Pos = UnknownInfos.find(BlockID);
    if (Pos != UnknownInfos.end())
      return Pos->second.get();
    // Install, then let next iteration find.
    std::unique_ptr<BlockInfo> Info(new BlockInfo(BlockID));
    UnknownInfos.emplace(BlockID, std::move(Info));
  }
}

NaClBitstreamReader::BlockInfoRecordsMap::UpdateLock::UpdateLock(
    BlockInfoRecordsMap &BlockInfoRecords)
    : BlockInfoRecords(BlockInfoRecords),
      Lock(BlockInfoRecords.UpdateRecordsLock) {}

NaClBitstreamReader::BlockInfoRecordsMap::UpdateLock::~UpdateLock() {
  if (BlockInfoRecords.freeze())
    report_fatal_error("Global abbreviations block frozen while building.");
}

bool NaClBitstreamCursor::ReadBlockInfoBlock(NaClAbbrevListener *Listener) {
  // If this is the second read of the block info block, skip it.
  if (BitStream->BlockInfoRecords->isFrozen())
    return SkipBlock();

  NaClBitstreamReader::BlockInfoRecordsMap::UpdateLock Lock(
      *BitStream->BlockInfoRecords);
  unsigned NumWords;
  if (EnterSubBlock(naclbitc::BLOCKINFO_BLOCK_ID, &NumWords))
    return true;

  if (Listener)
    Listener->BeginBlockInfoBlock(NumWords);

  NaClBitcodeRecordVector Record;
  Block &CurBlock = BlockScope.back();
  NaClBitstreamReader::AbbrevList *UpdateAbbrevs =
      &CurBlock.GlobalAbbrevs->getAbbrevs();
  bool FoundSetBID = false;

  // Read records of the BlockInfo block.
  while (1) {
    if (Listener)
      Listener->StartBit = GetCurrentBitNo();
    NaClBitstreamEntry Entry = advance(AF_DontAutoprocessAbbrevs, Listener);

    switch (Entry.Kind) {
    case llvm::NaClBitstreamEntry::SubBlock: // PNaCl doesn't allow!
    case llvm::NaClBitstreamEntry::Error:
      return true;
    case llvm::NaClBitstreamEntry::EndBlock:
      if (Listener)
        Listener->EndBlockInfoBlock();
      return false;
    case llvm::NaClBitstreamEntry::Record:
      // The interesting case.
      break;
    }

    // Read abbrev records, associate them with CurBID.
    if (Entry.ID == naclbitc::DEFINE_ABBREV) {
      ReadAbbrevRecord(false, Listener);

      // ReadAbbrevRecord installs a local abbreviation.  Move it to the
      // appropriate BlockInfo if the corresponding SetBID record has been
      // found.
      if (FoundSetBID)
        CurBlock.moveLocalAbbrevToAbbrevList(UpdateAbbrevs);
      continue;
    }

    // Read a record.
    Record.clear();
    switch (readRecord(Entry.ID, Record)) {
    default:
      // No other records should be found!
      return true;
    case naclbitc::BLOCKINFO_CODE_SETBID:
      if (Record.size() < 1)
        return true;
      FoundSetBID = true;
      UpdateAbbrevs =
          &BitStream->getBlockInfo((unsigned)Record[0])->getAbbrevs();
      if (Listener) {
        Listener->Values = Record;
        Listener->SetBID();
      }
      break;
    }
  }
}
