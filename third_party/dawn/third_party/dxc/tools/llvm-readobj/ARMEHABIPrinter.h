//===--- ARMEHABIPrinter.h - ARM EHABI Unwind Information Printer ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_LLVM_READOBJ_ARMEHABIPRINTER_H
#define LLVM_TOOLS_LLVM_READOBJ_ARMEHABIPRINTER_H

#include "Error.h"
#include "StreamWriter.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Object/ELF.h"
#include "llvm/Object/ELFTypes.h"
#include "llvm/Support/ARMEHABI.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/type_traits.h"

namespace llvm {
namespace ARM {
namespace EHABI {

class OpcodeDecoder {
  StreamWriter &SW;
  raw_ostream &OS;

  struct RingEntry {
    uint8_t Mask;
    uint8_t Value;
    void (OpcodeDecoder::*Routine)(const uint8_t *Opcodes, unsigned &OI);
  };
  static const RingEntry Ring[];

  void Decode_00xxxxxx(const uint8_t *Opcodes, unsigned &OI);
  void Decode_01xxxxxx(const uint8_t *Opcodes, unsigned &OI);
  void Decode_1000iiii_iiiiiiii(const uint8_t *Opcodes, unsigned &OI);
  void Decode_10011101(const uint8_t *Opcodes, unsigned &OI);
  void Decode_10011111(const uint8_t *Opcodes, unsigned &OI);
  void Decode_1001nnnn(const uint8_t *Opcodes, unsigned &OI);
  void Decode_10100nnn(const uint8_t *Opcodes, unsigned &OI);
  void Decode_10101nnn(const uint8_t *Opcodes, unsigned &OI);
  void Decode_10110000(const uint8_t *Opcodes, unsigned &OI);
  void Decode_10110001_0000iiii(const uint8_t *Opcodes, unsigned &OI);
  void Decode_10110010_uleb128(const uint8_t *Opcodes, unsigned &OI);
  void Decode_10110011_sssscccc(const uint8_t *Opcodes, unsigned &OI);
  void Decode_101101nn(const uint8_t *Opcodes, unsigned &OI);
  void Decode_10111nnn(const uint8_t *Opcodes, unsigned &OI);
  void Decode_11000110_sssscccc(const uint8_t *Opcodes, unsigned &OI);
  void Decode_11000111_0000iiii(const uint8_t *Opcodes, unsigned &OI);
  void Decode_11001000_sssscccc(const uint8_t *Opcodes, unsigned &OI);
  void Decode_11001001_sssscccc(const uint8_t *Opcodes, unsigned &OI);
  void Decode_11001yyy(const uint8_t *Opcodes, unsigned &OI);
  void Decode_11000nnn(const uint8_t *Opcodes, unsigned &OI);
  void Decode_11010nnn(const uint8_t *Opcodes, unsigned &OI);
  void Decode_11xxxyyy(const uint8_t *Opcodes, unsigned &OI);

  void PrintGPR(uint16_t GPRMask);
  void PrintRegisters(uint32_t Mask, StringRef Prefix);

public:
  OpcodeDecoder(StreamWriter &SW) : SW(SW), OS(SW.getOStream()) {}
  void Decode(const uint8_t *Opcodes, off_t Offset, size_t Length);
};

const OpcodeDecoder::RingEntry OpcodeDecoder::Ring[] = {
  { 0xc0, 0x00, &OpcodeDecoder::Decode_00xxxxxx },
  { 0xc0, 0x40, &OpcodeDecoder::Decode_01xxxxxx },
  { 0xf0, 0x80, &OpcodeDecoder::Decode_1000iiii_iiiiiiii },
  { 0xff, 0x9d, &OpcodeDecoder::Decode_10011101 },
  { 0xff, 0x9f, &OpcodeDecoder::Decode_10011111 },
  { 0xf0, 0x90, &OpcodeDecoder::Decode_1001nnnn },
  { 0xf8, 0xa0, &OpcodeDecoder::Decode_10100nnn },
  { 0xf8, 0xa8, &OpcodeDecoder::Decode_10101nnn },
  { 0xff, 0xb0, &OpcodeDecoder::Decode_10110000 },
  { 0xff, 0xb1, &OpcodeDecoder::Decode_10110001_0000iiii },
  { 0xff, 0xb2, &OpcodeDecoder::Decode_10110010_uleb128 },
  { 0xff, 0xb3, &OpcodeDecoder::Decode_10110011_sssscccc },
  { 0xfc, 0xb4, &OpcodeDecoder::Decode_101101nn },
  { 0xf8, 0xb8, &OpcodeDecoder::Decode_10111nnn },
  { 0xff, 0xc6, &OpcodeDecoder::Decode_11000110_sssscccc },
  { 0xff, 0xc7, &OpcodeDecoder::Decode_11000111_0000iiii },
  { 0xff, 0xc8, &OpcodeDecoder::Decode_11001000_sssscccc },
  { 0xff, 0xc9, &OpcodeDecoder::Decode_11001001_sssscccc },
  { 0xc8, 0xc8, &OpcodeDecoder::Decode_11001yyy },
  { 0xf8, 0xc0, &OpcodeDecoder::Decode_11000nnn },
  { 0xf8, 0xd0, &OpcodeDecoder::Decode_11010nnn },
  { 0xc0, 0xc0, &OpcodeDecoder::Decode_11xxxyyy },
};

void OpcodeDecoder::Decode_00xxxxxx(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; vsp = vsp + %u\n", Opcode,
                           ((Opcode & 0x3f) << 2) + 4);
}
void OpcodeDecoder::Decode_01xxxxxx(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; vsp = vsp - %u\n", Opcode,
                           ((Opcode & 0x3f) << 2) + 4);
}
void OpcodeDecoder::Decode_1000iiii_iiiiiiii(const uint8_t *Opcodes,
                                             unsigned &OI) {
  uint8_t Opcode0 = Opcodes[OI++ ^ 3];
  uint8_t Opcode1 = Opcodes[OI++ ^ 3];

  uint16_t GPRMask = (Opcode1 << 4) | ((Opcode0 & 0x0f) << 12);
  SW.startLine()
    << format("0x%02X 0x%02X ; %s",
              Opcode0, Opcode1, GPRMask ? "pop " : "refuse to unwind");
  if (GPRMask)
    PrintGPR(GPRMask);
  OS << '\n';
}
void OpcodeDecoder::Decode_10011101(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; reserved (ARM MOVrr)\n", Opcode);
}
void OpcodeDecoder::Decode_10011111(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; reserved (WiMMX MOVrr)\n", Opcode);
}
void OpcodeDecoder::Decode_1001nnnn(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; vsp = r%u\n", Opcode, (Opcode & 0x0f));
}
void OpcodeDecoder::Decode_10100nnn(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; pop ", Opcode);
  PrintGPR((((1 << ((Opcode & 0x7) + 1)) - 1) << 4));
  OS << '\n';
}
void OpcodeDecoder::Decode_10101nnn(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; pop ", Opcode);
  PrintGPR((((1 << ((Opcode & 0x7) + 1)) - 1) << 4) | (1 << 14));
  OS << '\n';
}
void OpcodeDecoder::Decode_10110000(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; finish\n", Opcode);
}
void OpcodeDecoder::Decode_10110001_0000iiii(const uint8_t *Opcodes,
                                             unsigned &OI) {
  uint8_t Opcode0 = Opcodes[OI++ ^ 3];
  uint8_t Opcode1 = Opcodes[OI++ ^ 3];

  SW.startLine()
    << format("0x%02X 0x%02X ; %s", Opcode0, Opcode1,
              ((Opcode1 & 0xf0) || Opcode1 == 0x00) ? "spare" : "pop ");
  if (((Opcode1 & 0xf0) == 0x00) && Opcode1)
    PrintGPR((Opcode1 & 0x0f));
  OS << '\n';
}
void OpcodeDecoder::Decode_10110010_uleb128(const uint8_t *Opcodes,
                                            unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X ", Opcode);

  SmallVector<uint8_t, 4> ULEB;
  do { ULEB.push_back(Opcodes[OI ^ 3]); } while (Opcodes[OI++ ^ 3] & 0x80);

  for (unsigned BI = 0, BE = ULEB.size(); BI != BE; ++BI)
    OS << format("0x%02X ", ULEB[BI]);

  uint64_t Value = 0;
  for (unsigned BI = 0, BE = ULEB.size(); BI != BE; ++BI)
    Value = Value | ((ULEB[BI] & 0x7f) << (7 * BI));

  OS << format("; vsp = vsp + %" PRIu64 "\n", 0x204 + (Value << 2));
}
void OpcodeDecoder::Decode_10110011_sssscccc(const uint8_t *Opcodes,
                                             unsigned &OI) {
  uint8_t Opcode0 = Opcodes[OI++ ^ 3];
  uint8_t Opcode1 = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X 0x%02X ; pop ", Opcode0, Opcode1);
  uint8_t Start = ((Opcode1 & 0xf0) >> 4);
  uint8_t Count = ((Opcode1 & 0x0f) >> 0);
  PrintRegisters((((1 << (Count + 1)) - 1) << Start), "d");
  OS << '\n';
}
void OpcodeDecoder::Decode_101101nn(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; spare\n", Opcode);
}
void OpcodeDecoder::Decode_10111nnn(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; pop ", Opcode);
  PrintRegisters((((1 << ((Opcode & 0x07) + 1)) - 1) << 8), "d");
  OS << '\n';
}
void OpcodeDecoder::Decode_11000110_sssscccc(const uint8_t *Opcodes,
                                             unsigned &OI) {
  uint8_t Opcode0 = Opcodes[OI++ ^ 3];
  uint8_t Opcode1 = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X 0x%02X ; pop ", Opcode0, Opcode1);
  uint8_t Start = ((Opcode1 & 0xf0) >> 4);
  uint8_t Count = ((Opcode1 & 0x0f) >> 0);
  PrintRegisters((((1 << (Count + 1)) - 1) << Start), "wR");
  OS << '\n';
}
void OpcodeDecoder::Decode_11000111_0000iiii(const uint8_t *Opcodes,
                                             unsigned &OI) {
  uint8_t Opcode0 = Opcodes[OI++ ^ 3];
  uint8_t Opcode1 = Opcodes[OI++ ^ 3];
  SW.startLine()
    << format("0x%02X 0x%02X ; %s", Opcode0, Opcode1,
              ((Opcode1 & 0xf0) || Opcode1 == 0x00) ? "spare" : "pop ");
  if ((Opcode1 & 0xf0) == 0x00 && Opcode1)
      PrintRegisters(Opcode1 & 0x0f, "wCGR");
  OS << '\n';
}
void OpcodeDecoder::Decode_11001000_sssscccc(const uint8_t *Opcodes,
                                             unsigned &OI) {
  uint8_t Opcode0 = Opcodes[OI++ ^ 3];
  uint8_t Opcode1 = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X 0x%02X ; pop ", Opcode0, Opcode1);
  uint8_t Start = 16 + ((Opcode1 & 0xf0) >> 4);
  uint8_t Count = ((Opcode1 & 0x0f) >> 0);
  PrintRegisters((((1 << (Count + 1)) - 1) << Start), "d");
  OS << '\n';
}
void OpcodeDecoder::Decode_11001001_sssscccc(const uint8_t *Opcodes,
                                             unsigned &OI) {
  uint8_t Opcode0 = Opcodes[OI++ ^ 3];
  uint8_t Opcode1 = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X 0x%02X ; pop ", Opcode0, Opcode1);
  uint8_t Start = ((Opcode1 & 0xf0) >> 4);
  uint8_t Count = ((Opcode1 & 0x0f) >> 0);
  PrintRegisters((((1 << (Count + 1)) - 1) << Start), "d");
  OS << '\n';
}
void OpcodeDecoder::Decode_11001yyy(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; spare\n", Opcode);
}
void OpcodeDecoder::Decode_11000nnn(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; pop ", Opcode);
  PrintRegisters((((1 << ((Opcode & 0x07) + 1)) - 1) << 10), "wR");
  OS << '\n';
}
void OpcodeDecoder::Decode_11010nnn(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; pop ", Opcode);
  PrintRegisters((((1 << ((Opcode & 0x07) + 1)) - 1) << 8), "d");
  OS << '\n';
}
void OpcodeDecoder::Decode_11xxxyyy(const uint8_t *Opcodes, unsigned &OI) {
  uint8_t Opcode = Opcodes[OI++ ^ 3];
  SW.startLine() << format("0x%02X      ; spare\n", Opcode);
}

void OpcodeDecoder::PrintGPR(uint16_t GPRMask) {
  static const char *GPRRegisterNames[16] = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
    "fp", "ip", "sp", "lr", "pc"
  };

  OS << '{';
  bool Comma = false;
  for (unsigned RI = 0, RE = 17; RI < RE; ++RI) {
    if (GPRMask & (1 << RI)) {
      if (Comma)
        OS << ", ";
      OS << GPRRegisterNames[RI];
      Comma = true;
    }
  }
  OS << '}';
}

void OpcodeDecoder::PrintRegisters(uint32_t VFPMask, StringRef Prefix) {
  OS << '{';
  bool Comma = false;
  for (unsigned RI = 0, RE = 32; RI < RE; ++RI) {
    if (VFPMask & (1 << RI)) {
      if (Comma)
        OS << ", ";
      OS << Prefix << RI;
      Comma = true;
    }
  }
  OS << '}';
}

void OpcodeDecoder::Decode(const uint8_t *Opcodes, off_t Offset, size_t Length) {
  for (unsigned OCI = Offset; OCI < Length + Offset; ) {
    bool Decoded = false;
    for (unsigned REI = 0, REE = array_lengthof(Ring);
         REI != REE && !Decoded; ++REI) {
      if ((Opcodes[OCI ^ 3] & Ring[REI].Mask) == Ring[REI].Value) {
        (this->*Ring[REI].Routine)(Opcodes, OCI);
        Decoded = true;
        break;
      }
    }
    if (!Decoded)
      SW.startLine() << format("0x%02X      ; reserved\n", Opcodes[OCI++ ^ 3]);
  }
}

template <typename ET>
class PrinterContext {
  StreamWriter &SW;
  const object::ELFFile<ET> *ELF;

  typedef typename object::ELFFile<ET>::Elf_Sym Elf_Sym;
  typedef typename object::ELFFile<ET>::Elf_Shdr Elf_Shdr;

  typedef typename object::ELFFile<ET>::Elf_Rel_Iter Elf_Rel_iterator;

  static const size_t IndexTableEntrySize;

  static uint64_t PREL31(uint32_t Address, uint32_t Place) {
    uint64_t Location = Address & 0x7fffffff;
    if (Location & 0x04000000)
      Location |= (uint64_t) ~0x7fffffff;
    return Location + Place;
  }

  ErrorOr<StringRef> FunctionAtAddress(unsigned Section, uint64_t Address) const;
  const Elf_Shdr *FindExceptionTable(unsigned IndexTableIndex,
                                     off_t IndexTableOffset) const;

  void PrintIndexTable(unsigned SectionIndex, const Elf_Shdr *IT) const;
  void PrintExceptionTable(const Elf_Shdr *IT, const Elf_Shdr *EHT,
                           uint64_t TableEntryOffset) const;
  void PrintOpcodes(const uint8_t *Entry, size_t Length, off_t Offset) const;

public:
  PrinterContext(StreamWriter &Writer, const object::ELFFile<ET> *File)
    : SW(Writer), ELF(File) {}

  void PrintUnwindInformation() const;
};

template <typename ET>
const size_t PrinterContext<ET>::IndexTableEntrySize = 8;

template <typename ET>
ErrorOr<StringRef>
PrinterContext<ET>::FunctionAtAddress(unsigned Section,
                                      uint64_t Address) const {
  for (const Elf_Sym &Sym : ELF->symbols())
    if (Sym.st_shndx == Section && Sym.st_value == Address &&
        Sym.getType() == ELF::STT_FUNC)
      return ELF->getSymbolName(&Sym, false);
  return readobj_error::unknown_symbol;
}

template <typename ET>
const typename object::ELFFile<ET>::Elf_Shdr *
PrinterContext<ET>::FindExceptionTable(unsigned IndexSectionIndex,
                                       off_t IndexTableOffset) const {
  /// Iterate through the sections, searching for the relocation section
  /// associated with the unwind index table section specified by
  /// IndexSectionIndex.  Iterate the associated section searching for the
  /// relocation associated with the index table entry specified by
  /// IndexTableOffset.  The symbol is the section symbol for the exception
  /// handling table.  Use this symbol to recover the actual exception handling
  /// table.

  for (const Elf_Shdr &Sec : ELF->sections()) {
    if (Sec.sh_type == ELF::SHT_REL && Sec.sh_info == IndexSectionIndex) {
      for (Elf_Rel_iterator RI = ELF->rel_begin(&Sec), RE = ELF->rel_end(&Sec);
           RI != RE; ++RI) {
        if (RI->r_offset == static_cast<unsigned>(IndexTableOffset)) {
          typename object::ELFFile<ET>::Elf_Rela RelA;
          RelA.r_offset = RI->r_offset;
          RelA.r_info = RI->r_info;
          RelA.r_addend = 0;

          std::pair<const Elf_Shdr *, const Elf_Sym *> Symbol =
              ELF->getRelocationSymbol(&Sec, &RelA);

          ErrorOr<const Elf_Shdr *> Ret = ELF->getSection(Symbol.second);
          if (std::error_code EC = Ret.getError())
            report_fatal_error(EC.message());
          return *Ret;
        }
      }
    }
  }
  return nullptr;
}

template <typename ET>
void PrinterContext<ET>::PrintExceptionTable(const Elf_Shdr *IT,
                                             const Elf_Shdr *EHT,
                                             uint64_t TableEntryOffset) const {
  ErrorOr<ArrayRef<uint8_t> > Contents = ELF->getSectionContents(EHT);
  if (!Contents)
    return;

  /// ARM EHABI Section 6.2 - The generic model
  ///
  /// An exception-handling table entry for the generic model is laid out as:
  ///
  ///  3 3
  ///  1 0                            0
  /// +-+------------------------------+
  /// |0|  personality routine offset  |
  /// +-+------------------------------+
  /// |  personality routine data ...  |
  ///
  ///
  /// ARM EHABI Section 6.3 - The ARM-defined compact model
  ///
  /// An exception-handling table entry for the compact model looks like:
  ///
  ///  3 3 2 2  2 2
  ///  1 0 8 7  4 3                     0
  /// +-+---+----+-----------------------+
  /// |1| 0 | Ix | data for pers routine |
  /// +-+---+----+-----------------------+
  /// |  more personality routine data   |

  const support::ulittle32_t Word =
    *reinterpret_cast<const support::ulittle32_t *>(Contents->data() + TableEntryOffset);

  if (Word & 0x80000000) {
    SW.printString("Model", StringRef("Compact"));

    unsigned PersonalityIndex = (Word & 0x0f000000) >> 24;
    SW.printNumber("PersonalityIndex", PersonalityIndex);

    switch (PersonalityIndex) {
    case AEABI_UNWIND_CPP_PR0:
      PrintOpcodes(Contents->data() + TableEntryOffset, 3, 1);
      break;
    case AEABI_UNWIND_CPP_PR1:
    case AEABI_UNWIND_CPP_PR2:
      unsigned AdditionalWords = (Word & 0x00ff0000) >> 16;
      PrintOpcodes(Contents->data() + TableEntryOffset, 2 + 4 * AdditionalWords,
                   2);
      break;
    }
  } else {
    SW.printString("Model", StringRef("Generic"));

    uint64_t Address = PREL31(Word, EHT->sh_addr);
    SW.printHex("PersonalityRoutineAddress", Address);
    if (ErrorOr<StringRef> Name = FunctionAtAddress(EHT->sh_link, Address))
      SW.printString("PersonalityRoutineName", *Name);
  }
}

template <typename ET>
void PrinterContext<ET>::PrintOpcodes(const uint8_t *Entry,
                                      size_t Length, off_t Offset) const {
  ListScope OCC(SW, "Opcodes");
  OpcodeDecoder(OCC.W).Decode(Entry, Offset, Length);
}

template <typename ET>
void PrinterContext<ET>::PrintIndexTable(unsigned SectionIndex,
                                         const Elf_Shdr *IT) const {
  ErrorOr<ArrayRef<uint8_t> > Contents = ELF->getSectionContents(IT);
  if (!Contents)
    return;

  /// ARM EHABI Section 5 - Index Table Entries
  /// * The first word contains a PREL31 offset to the start of a function with
  ///   bit 31 clear
  /// * The second word contains one of:
  ///   - The PREL31 offset of the start of the table entry for the function,
  ///     with bit 31 clear
  ///   - The exception-handling table entry itself with bit 31 set
  ///   - The special bit pattern EXIDX_CANTUNWIND, indicating that associated
  ///     frames cannot be unwound

  const support::ulittle32_t *Data =
    reinterpret_cast<const support::ulittle32_t *>(Contents->data());
  const unsigned Entries = IT->sh_size / IndexTableEntrySize;

  ListScope E(SW, "Entries");
  for (unsigned Entry = 0; Entry < Entries; ++Entry) {
    DictScope E(SW, "Entry");

    const support::ulittle32_t Word0 =
      Data[Entry * (IndexTableEntrySize / sizeof(*Data)) + 0];
    const support::ulittle32_t Word1 =
      Data[Entry * (IndexTableEntrySize / sizeof(*Data)) + 1];

    if (Word0 & 0x80000000) {
      errs() << "corrupt unwind data in section " << SectionIndex << "\n";
      continue;
    }

    const uint64_t Offset = PREL31(Word0, IT->sh_addr);
    SW.printHex("FunctionAddress", Offset);
    if (ErrorOr<StringRef> Name = FunctionAtAddress(IT->sh_link, Offset))
      SW.printString("FunctionName", *Name);

    if (Word1 == EXIDX_CANTUNWIND) {
      SW.printString("Model", StringRef("CantUnwind"));
      continue;
    }

    if (Word1 & 0x80000000) {
      SW.printString("Model", StringRef("Compact (Inline)"));

      unsigned PersonalityIndex = (Word1 & 0x0f000000) >> 24;
      SW.printNumber("PersonalityIndex", PersonalityIndex);

      PrintOpcodes(Contents->data() + Entry * IndexTableEntrySize + 4, 3, 1);
    } else {
      const Elf_Shdr *EHT =
        FindExceptionTable(SectionIndex, Entry * IndexTableEntrySize + 4);

      if (ErrorOr<StringRef> Name = ELF->getSectionName(EHT))
        SW.printString("ExceptionHandlingTable", *Name);

      uint64_t TableEntryOffset = PREL31(Word1, IT->sh_addr);
      SW.printHex("TableEntryOffset", TableEntryOffset);

      PrintExceptionTable(IT, EHT, TableEntryOffset);
    }
  }
}

template <typename ET>
void PrinterContext<ET>::PrintUnwindInformation() const {
  DictScope UI(SW, "UnwindInformation");

  int SectionIndex = 0;
  for (const Elf_Shdr &Sec : ELF->sections()) {
    if (Sec.sh_type == ELF::SHT_ARM_EXIDX) {
      DictScope UIT(SW, "UnwindIndexTable");

      SW.printNumber("SectionIndex", SectionIndex);
      if (ErrorOr<StringRef> SectionName = ELF->getSectionName(&Sec))
        SW.printString("SectionName", *SectionName);
      SW.printHex("SectionOffset", Sec.sh_offset);

      PrintIndexTable(SectionIndex, &Sec);
    }
    ++SectionIndex;
  }
}
}
}
}

#endif

