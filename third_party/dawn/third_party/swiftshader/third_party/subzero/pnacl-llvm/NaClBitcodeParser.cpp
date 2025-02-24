//===- NaClBitcodeParser.cpp ----------------------------------------------===//
//     Low-level bitcode driver to parse PNaCl bitcode files.
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Bitcode/NaCl/NaClBitcodeParser.h"

using namespace llvm;

void NaClBitcodeRecordData::Print(raw_ostream &os) const {
  os << "[" << Code;
  for (NaClRecordVector::const_iterator Iter = Values.begin(),
                                        IterEnd = Values.end();
       Iter != IterEnd; ++Iter) {
    os << ", " << *Iter;
  }
  os << "]";
}

void NaClBitcodeRecord::Print(raw_ostream &os) const {
  Block.Print(os);
  os << ", Code " << Data.Code << ", EntryID " << Entry.ID << ", <";
  for (unsigned i = 0, e = Data.Values.size(); i != e; ++i) {
    if (i > 0)
      os << " ";
    os << Data.Values[i];
  }
  os << ">";
}

NaClBitcodeBlock::NaClBitcodeBlock(unsigned BlockID,
                                   const NaClBitcodeRecord &Record)
    : NaClBitcodeData(Record), BlockID(BlockID),
      EnclosingBlock(&Record.GetBlock()), LocalStartBit(Record.GetStartBit()) {}

void NaClBitcodeBlock::Print(raw_ostream &os) const {
  os << "Block " << BlockID;
}

void NaClBitcodeParserListener::BeginBlockInfoBlock(unsigned NumWords) {
  Parser->EnterBlock(NumWords);
}

void NaClBitcodeParserListener::SetBID() {
  Parser->Record.SetStartBit(StartBit);
  Parser->Record.Entry.Kind = NaClBitstreamEntry::Record;
  Parser->Record.Entry.ID = naclbitc::UNABBREV_RECORD;
  Parser->Record.Data.Code = naclbitc::BLOCKINFO_CODE_SETBID;
  Parser->Record.Data.Values = Values;
  GlobalBlockID = Values[0];
  Parser->SetBID();
  Values.clear();
}

void NaClBitcodeParserListener::EndBlockInfoBlock() {
  Parser->Record.SetStartBit(StartBit);
  Parser->Record.Entry.Kind = NaClBitstreamEntry::EndBlock;
  Parser->Record.Entry.ID = naclbitc::END_BLOCK;
  Parser->Record.Data.Code = naclbitc::END_BLOCK;
  Parser->Record.Data.Values.clear();
  GlobalBlockID = naclbitc::BLOCKINFO_BLOCK_ID;
  Parser->ExitBlock();
}

void NaClBitcodeParserListener::ProcessAbbreviation(NaClBitCodeAbbrev *Abbrev,
                                                    bool IsLocal) {
  Parser->Record.SetStartBit(StartBit);
  Parser->Record.Entry.Kind = NaClBitstreamEntry::Record;
  Parser->Record.Entry.ID = naclbitc::DEFINE_ABBREV;
  Parser->Record.Data.Code = naclbitc::BLK_CODE_DEFINE_ABBREV;
  Parser->Record.Data.Values = Values;
  Parser->ProcessAbbreviation(IsLocal ? Parser->GetBlockID() : GlobalBlockID,
                              Abbrev, IsLocal);
}

NaClBitcodeParser::~NaClBitcodeParser() {
  if (EnclosingParser) {
    EnclosingParser->Block.LocalStartBit += Block.GetNumBits();
  }
}

bool NaClBitcodeParser::ErrorAt(naclbitc::ErrorLevel Level,
                                uint64_t BitPosition,
                                const std::string &Message) {
  naclbitc::ErrorAt(*ErrStream, Level, BitPosition) << Message << "\n";
  if (Level == naclbitc::Fatal)
    report_fatal_error("Unable to continue");
  return true;
}

bool NaClBitcodeParser::Parse() {
  Record.ReadEntry();

  if (Record.GetEntryKind() != NaClBitstreamEntry::SubBlock)
    return Error("Expected block, but not found");

  return ParseBlock(Record.GetEntryID());
}

bool NaClBitcodeParser::ParseBlockInfoInternal() {
  // BLOCKINFO is a special part of the stream. Let the bitstream
  // reader process this block.
  bool Result = Record.GetCursor().ReadBlockInfoBlock(Listener);
  if (Result)
    return Error("Malformed BlockInfoBlock");
  return Result;
}

bool NaClBitcodeParser::ParseBlockInternal() {
  // Regular block. Enter subblock.
  unsigned NumWords;
  if (Record.GetCursor().EnterSubBlock(GetBlockID(), &NumWords)) {
    return Error("Malformed block record");
  }

  EnterBlock(NumWords);

  // Process records.
  while (1) {
    if (Record.GetCursor().AtEndOfStream())
      return Error("Premature end of bitstream");

    // Read entry defining type of entry.
    Record.ReadEntry();

    switch (Record.GetEntryKind()) {
    case NaClBitstreamEntry::Error:
      return Error("malformed bitcode file");
    case NaClBitstreamEntry::EndBlock: {
      return false;
    }
    case NaClBitstreamEntry::SubBlock: {
      if (ParseBlock(Record.GetEntryID()))
        return true;
      break;
    }
    case NaClBitstreamEntry::Record:
      // The interesting case.
      if (Record.GetEntryID() == naclbitc::DEFINE_ABBREV) {
        // Since this abbreviation is local, the listener doesn't
        // have the start bit set (it is only set when processing
        // the BlockInfo block). Fix this by setting it here.
        if (Listener)
          Listener->StartBit = Record.GetStartBit();
        Record.GetCursor().ReadAbbrevRecord(true, Listener);
      } else {
        // Read in a record.
        Record.ReadValues();
        ProcessRecord();
      }
      break;
    }
  }
  return false;
}
