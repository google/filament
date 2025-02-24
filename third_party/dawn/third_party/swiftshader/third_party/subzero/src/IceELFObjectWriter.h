//===- subzero/src/IceELFObjectWriter.h - ELF object writer -----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Abstraction for a writer that is responsible for writing an ELF file.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEELFOBJECTWRITER_H
#define SUBZERO_SRC_ICEELFOBJECTWRITER_H

#include "IceDefs.h"
#include "IceELFSection.h"
#include "IceELFStreamer.h"
#include "IceTypes.h"

using namespace llvm::ELF;

namespace Ice {

using VariableDeclarationPartition = std::vector<VariableDeclaration *>;

/// Higher level ELF object writer. Manages section information and writes the
/// final ELF object. The object writer will write to file the code and data as
/// it is being defined (rather than keep a copy). After all definitions are
/// written out, it will finalize the bookkeeping sections and write them out.
/// Expected usage:
///
/// (1) writeInitialELFHeader (invoke once)
/// (2) writeDataSection      (may be invoked multiple times, as long as
///                            SectionSuffix is unique)
/// (3) writeFunctionCode     (must invoke once per function)
/// (4) writeConstantPool     (must invoke once per pooled primitive type)
/// (5) setUndefinedSyms      (invoke once)
/// (6) writeNonUserSections  (invoke once)
///
/// The requirement for writeDataSection to be invoked only once can be relaxed
/// if using -fdata-sections. The requirement to invoke only once without
/// -fdata-sections is so that variables that belong to each possible
/// SectionType are contiguous in the file. With -fdata-sections, each global
/// variable is in a separate section and therefore the sections will be
/// trivially contiguous.
class ELFObjectWriter {
  ELFObjectWriter() = delete;
  ELFObjectWriter(const ELFObjectWriter &) = delete;
  ELFObjectWriter &operator=(const ELFObjectWriter &) = delete;

public:
  ELFObjectWriter(GlobalContext &Ctx, ELFStreamer &Out);

  /// Write the initial ELF header. This is just to reserve space in the ELF
  /// file. Reserving space allows the other functions to write text and data
  /// directly to the file and get the right file offsets.
  void writeInitialELFHeader();

  /// Copy initializer data for globals to file and note the offset and size of
  /// each global's definition in the symbol table. Use the given target's
  /// RelocationKind for any relocations.
  void writeDataSection(const VariableDeclarationList &Vars,
                        FixupKind RelocationKind,
                        const std::string &SectionSuffix, bool IsPIC);

  /// Copy data of a function's text section to file and note the offset of the
  /// symbol's definition in the symbol table. Copy the text fixups for use
  /// after all functions are written. The text buffer and fixups are extracted
  /// from the Assembler object.
  void writeFunctionCode(GlobalString FuncName, bool IsInternal,
                         Assembler *Asm);

  /// Queries the GlobalContext for constant pools of the given type and writes
  /// out read-only data sections for those constants. This also fills the
  /// symbol table with labels for each constant pool entry.
  template <typename ConstType> void writeConstantPool(Type Ty);

  /// Write a jump table and register fixups for the target addresses.
  void writeJumpTable(const JumpTableData &JT, FixupKind RelocationKind,
                      bool IsPIC);

  /// Populate the symbol table with a list of external/undefined symbols.
  void setUndefinedSyms(const ConstantList &UndefSyms);

  /// Do final layout and write out the rest of the object file. Finally, patch
  /// up the initial ELF header with the final info.
  void writeNonUserSections();

  /// Which type of ELF section a global variable initializer belongs to. This
  /// is used as an array index so should start at 0 and be contiguous.
  enum SectionType { ROData = 0, Data, BSS, NumSectionTypes };

  /// Create target specific section with the given information about section.
  void writeTargetRODataSection(const std::string &Name, Elf64_Word ShType,
                                Elf64_Xword ShFlags, Elf64_Xword ShAddralign,
                                Elf64_Xword ShEntsize,
                                const llvm::StringRef &SecData);

private:
  GlobalContext &Ctx;
  ELFStreamer &Str;
  bool SectionNumbersAssigned = false;
  bool ELF64;

  // All created sections, separated into different pools.
  using SectionList = std::vector<ELFSection *>;
  using TextSectionList = std::vector<ELFTextSection *>;
  using DataSectionList = std::vector<ELFDataSection *>;
  using RelSectionList = std::vector<ELFRelocationSection *>;
  TextSectionList TextSections;
  RelSectionList RelTextSections;
  DataSectionList DataSections;
  RelSectionList RelDataSections;
  DataSectionList RODataSections;
  RelSectionList RelRODataSections;
  DataSectionList BSSSections;

  // Handles to special sections that need incremental bookkeeping.
  ELFSection *NullSection;
  ELFStringTableSection *ShStrTab;
  ELFSymbolTableSection *SymTab;
  ELFStringTableSection *StrTab;

  template <typename T>
  T *createSection(const std::string &Name, Elf64_Word ShType,
                   Elf64_Xword ShFlags, Elf64_Xword ShAddralign,
                   Elf64_Xword ShEntsize);

  /// Create a relocation section, given the related section (e.g., .text,
  /// .data., .rodata).
  ELFRelocationSection *
  createRelocationSection(const ELFSection *RelatedSection);

  /// Align the file position before writing out a section's data, and return
  /// the position of the file.
  Elf64_Off alignFileOffset(Elf64_Xword Align);

  /// Assign an ordering / section numbers to each section. Fill in other
  /// information that is only known near the end (such as the size, if it
  /// wasn't already incrementally updated). This then collects all sections in
  /// the decided order, into one vector, for conveniently writing out all of
  /// the section headers.
  void assignSectionNumbersInfo(SectionList &AllSections);

  /// This function assigns .foo and .rel.foo consecutive section numbers. It
  /// also sets the relocation section's sh_info field to the related section's
  /// number.
  template <typename UserSectionList>
  void assignRelSectionNumInPairs(SizeT &CurSectionNumber,
                                  UserSectionList &UserSections,
                                  RelSectionList &RelSections,
                                  SectionList &AllSections);

  /// Link the relocation sections to the symbol table.
  void assignRelLinkNum(SizeT SymTabNumber, RelSectionList &RelSections);

  /// Helper function for writeDataSection. Writes a data section of type
  /// SectionType, given the global variables Vars belonging to that
  /// SectionType.
  void writeDataOfType(SectionType SectionType,
                       const VariableDeclarationPartition &Vars,
                       FixupKind RelocationKind,
                       const std::string &SectionSuffix, bool IsPIC);

  /// Write the final relocation sections given the final symbol table. May also
  /// be able to seek around the file and resolve function calls that are for
  /// functions within the same section.
  void writeAllRelocationSections();
  void writeRelocationSections(RelSectionList &RelSections);

  /// Write the ELF file header with the given information about sections.
  template <bool IsELF64>
  void writeELFHeaderInternal(Elf64_Off SectionHeaderOffset,
                              SizeT SectHeaderStrIndex, SizeT NumSections);
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEELFOBJECTWRITER_H
