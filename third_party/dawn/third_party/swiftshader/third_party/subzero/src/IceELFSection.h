//===- subzero/src/IceELFSection.h - Model of ELF sections ------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Representation of ELF sections.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEELFSECTION_H
#define SUBZERO_SRC_ICEELFSECTION_H

#include "IceDefs.h"
#include "IceELFStreamer.h"
#include "IceFixups.h"
#include "IceOperand.h"
#include "IceStringPool.h"

using namespace llvm::ELF;

namespace Ice {

class ELFStreamer;
class ELFStringTableSection;

/// Base representation of an ELF section.
class ELFSection {
  ELFSection() = delete;
  ELFSection(const ELFSection &) = delete;
  ELFSection &operator=(const ELFSection &) = delete;

public:
  virtual ~ELFSection() = default;

  /// Sentinel value for a section number/index for before the final section
  /// index is actually known. The dummy NULL section will be assigned number 0,
  /// and it is referenced by the dummy 0-th symbol in the symbol table, so use
  /// max() instead of 0.
  enum { NoSectionNumber = std::numeric_limits<SizeT>::max() };

  /// Constructs an ELF section, filling in fields that will be known once the
  /// *type* of section is decided. Other fields may be updated incrementally or
  /// only after the program is completely defined.
  ELFSection(const std::string &Name, Elf64_Word ShType, Elf64_Xword ShFlags,
             Elf64_Xword ShAddralign, Elf64_Xword ShEntsize)
      : Name(Name), Header() {
    Header.sh_type = ShType;
    Header.sh_flags = ShFlags;
    Header.sh_addralign = ShAddralign;
    Header.sh_entsize = ShEntsize;
  }

  /// Set the section number/index after it is finally known.
  void setNumber(SizeT N) {
    // Should only set the number once: from NoSectionNumber -> N.
    assert(Number == NoSectionNumber);
    Number = N;
  }
  SizeT getNumber() const {
    assert(Number != NoSectionNumber);
    return Number;
  }

  void setSize(Elf64_Xword sh_size) { Header.sh_size = sh_size; }
  SizeT getCurrentSize() const { return Header.sh_size; }

  void setNameStrIndex(Elf64_Word sh_name) { Header.sh_name = sh_name; }

  const std::string &getName() const { return Name; }

  void setLinkNum(Elf64_Word sh_link) { Header.sh_link = sh_link; }

  void setInfoNum(Elf64_Word sh_info) { Header.sh_info = sh_info; }

  void setFileOffset(Elf64_Off sh_offset) { Header.sh_offset = sh_offset; }

  Elf64_Xword getSectionAlign() const { return Header.sh_addralign; }

  /// Write the section header out with the given streamer.
  template <bool IsELF64> void writeHeader(ELFStreamer &Str);

protected:
  /// Name of the section in convenient string form (instead of a index into the
  /// Section Header String Table, which is not known till later).
  const std::string Name;

  // The fields of the header. May only be partially initialized, but should
  // be fully initialized before writing.
  Elf64_Shdr Header;

  /// The number of the section after laying out sections.
  SizeT Number = NoSectionNumber;
};

/// Models text/code sections. Code is written out incrementally and the size of
/// the section is then updated incrementally.
class ELFTextSection : public ELFSection {
  ELFTextSection() = delete;
  ELFTextSection(const ELFTextSection &) = delete;
  ELFTextSection &operator=(const ELFTextSection &) = delete;

public:
  using ELFSection::ELFSection;

  void appendData(ELFStreamer &Str, const llvm::StringRef MoreData);
};

/// Models data/rodata sections. Data is written out incrementally and the size
/// of the section is then updated incrementally. Some rodata sections may have
/// fixed entsize and duplicates may be mergeable.
class ELFDataSection : public ELFSection {
  ELFDataSection() = delete;
  ELFDataSection(const ELFDataSection &) = delete;
  ELFDataSection &operator=(const ELFDataSection &) = delete;

public:
  using ELFSection::ELFSection;

  void appendData(ELFStreamer &Str, const llvm::StringRef MoreData);

  void appendZeros(ELFStreamer &Str, SizeT NumBytes);

  void appendRelocationOffset(ELFStreamer &Str, bool IsRela,
                              RelocOffsetT RelocOffset);

  /// Pad the next section offset for writing data elements to the requested
  /// alignment. If the section is NOBITS then do not actually write out the
  /// padding and only update the section size.
  void padToAlignment(ELFStreamer &Str, Elf64_Xword Align);
};

/// Model of ELF symbol table entries. Besides keeping track of the fields
/// required for an elf symbol table entry it also tracks the number that
/// represents the symbol's final index in the symbol table.
struct ELFSym {
  Elf64_Sym Sym;
  ELFSection *Section;
  SizeT Number;

  /// Sentinel value for symbols that haven't been assigned a number yet. The
  /// dummy 0-th symbol will be assigned number 0, so don't use that.
  enum { UnknownNumber = std::numeric_limits<SizeT>::max() };

  void setNumber(SizeT N) {
    assert(Number == UnknownNumber);
    Number = N;
  }

  SizeT getNumber() const {
    assert(Number != UnknownNumber);
    return Number;
  }
};

/// Models a symbol table. Symbols may be added up until updateIndices is
/// called. At that point the indices of each symbol will be finalized.
class ELFSymbolTableSection : public ELFSection {
  ELFSymbolTableSection() = delete;
  ELFSymbolTableSection(const ELFSymbolTableSection &) = delete;
  ELFSymbolTableSection &operator=(const ELFSymbolTableSection &) = delete;

public:
  ELFSymbolTableSection(const std::string &Name, Elf64_Word ShType,
                        Elf64_Xword ShFlags, Elf64_Xword ShAddralign,
                        Elf64_Xword ShEntsize)
      : ELFSection(Name, ShType, ShFlags, ShAddralign, ShEntsize),
        NullSymbolName(), NullSymbol(nullptr) {}

  /// Create initial entry for a symbol when it is defined. Each entry should
  /// only be defined once. We might want to allow Name to be a dummy name
  /// initially, then get updated to the real thing, since Data initializers are
  /// read before the bitcode's symbol table is read.
  void createDefinedSym(GlobalString Name, uint8_t Type, uint8_t Binding,
                        ELFSection *Section, RelocOffsetT Offset, SizeT Size);

  /// Note that a symbol table entry needs to be created for the given symbol
  /// because it is undefined.
  void noteUndefinedSym(GlobalString Name, ELFSection *NullSection);

  const ELFSym *findSymbol(GlobalString Name) const;

  void createNullSymbol(ELFSection *NullSection, GlobalContext *Ctx);
  const ELFSym *getNullSymbol() const { return NullSymbol; }

  size_t getSectionDataSize() const {
    return (LocalSymbols.size() + GlobalSymbols.size()) * Header.sh_entsize;
  }

  size_t getNumLocals() const { return LocalSymbols.size(); }

  void updateIndices(const ELFStringTableSection *StrTab);

  void writeData(ELFStreamer &Str, bool IsELF64);

private:
  // Map from symbol name to its symbol information. This assumes symbols are
  // unique across all sections.
  using SymtabKey = GlobalString;
  using SymMap = std::map<SymtabKey, ELFSym>;

  template <bool IsELF64>
  void writeSymbolMap(ELFStreamer &Str, const SymMap &Map);

  GlobalString NullSymbolName;
  const ELFSym *NullSymbol;
  // Keep Local and Global symbols separate, since the sh_info needs to know
  // the index of the last LOCAL.
  SymMap LocalSymbols;
  SymMap GlobalSymbols;
};

/// Models a relocation section.
class ELFRelocationSection : public ELFSection {
  ELFRelocationSection() = delete;
  ELFRelocationSection(const ELFRelocationSection &) = delete;
  ELFRelocationSection &operator=(const ELFRelocationSection &) = delete;

public:
  ELFRelocationSection(const std::string &Name, Elf64_Word ShType,
                       Elf64_Xword ShFlags, Elf64_Xword ShAddralign,
                       Elf64_Xword ShEntsize)
      : ELFSection(Name, ShType, ShFlags, ShAddralign, ShEntsize),
        RelatedSection(nullptr) {}

  const ELFSection *getRelatedSection() const { return RelatedSection; }
  void setRelatedSection(const ELFSection *Section) {
    RelatedSection = Section;
  }

  /// Track additional relocations which start out relative to offset 0, but
  /// should be adjusted to be relative to BaseOff.
  void addRelocations(RelocOffsetT BaseOff, const FixupRefList &FixupRefs,
                      ELFSymbolTableSection *SymTab);

  /// Track a single additional relocation.
  void addRelocation(const AssemblerFixup &Fixup) { Fixups.push_back(Fixup); }

  size_t getSectionDataSize() const;

  template <bool IsELF64>
  void writeData(ELFStreamer &Str, const ELFSymbolTableSection *SymTab);

  bool isRela() const { return Header.sh_type == SHT_RELA; }

private:
  const ELFSection *RelatedSection;
  FixupList Fixups;
};

/// Models a string table. The user will build the string table by adding
/// strings incrementally. At some point, all strings should be known and
/// doLayout() should be called. After that, no other strings may be added.
/// However, the final offsets of the strings can be discovered and used to fill
/// out section headers and symbol table entries.
class ELFStringTableSection : public ELFSection {
  ELFStringTableSection() = delete;
  ELFStringTableSection(const ELFStringTableSection &) = delete;
  ELFStringTableSection &operator=(const ELFStringTableSection &) = delete;

public:
  using ELFSection::ELFSection;

  /// Add a string to the table, in preparation for final layout.
  void add(const std::string &Str);
  void add(GlobalString Str) {
    if (Str.hasStdString())
      add(Str.toString());
  }

  /// Finalizes the layout of the string table and fills in the section Data.
  void doLayout();

  /// The first byte of the string table should be \0, so it is an invalid
  /// index. Indices start out as unknown until layout is complete.
  enum { UnknownIndex = 0 };

  /// Grabs the final index of a string after layout. Returns UnknownIndex if
  /// the string's index is not found.
  size_t getIndex(const std::string &Str) const;

  llvm::StringRef getSectionData() const {
    assert(isLaidOut());
    return llvm::StringRef(reinterpret_cast<const char *>(StringData.data()),
                           StringData.size());
  }

  size_t getSectionDataSize() const { return getSectionData().size(); }

private:
  bool isLaidOut() const { return !StringData.empty(); }

  /// Strings can share a string table entry if they share the same suffix.
  /// E.g., "pop" and "lollipop" can both use the characters in "lollipop", but
  /// "pops" cannot, and "unpop" cannot either. Though, "pop", "lollipop", and
  /// "unpop" share "pop" as the suffix, "pop" can only share the characters
  /// with one of them.
  struct SuffixComparator {
    bool operator()(const std::string &StrA, const std::string &StrB) const;
  };

  using StringToIndexType = std::map<std::string, size_t, SuffixComparator>;

  /// Track strings to their index. Index will be UnknownIndex if not yet laid
  /// out.
  StringToIndexType StringToIndexMap;

  using RawDataType = std::vector<uint8_t>;
  RawDataType StringData;
};

template <bool IsELF64> void ELFSection::writeHeader(ELFStreamer &Str) {
  Str.writeELFWord<IsELF64>(Header.sh_name);
  Str.writeELFWord<IsELF64>(Header.sh_type);
  Str.writeELFXword<IsELF64>(Header.sh_flags);
  Str.writeAddrOrOffset<IsELF64>(Header.sh_addr);
  Str.writeAddrOrOffset<IsELF64>(Header.sh_offset);
  Str.writeELFXword<IsELF64>(Header.sh_size);
  Str.writeELFWord<IsELF64>(Header.sh_link);
  Str.writeELFWord<IsELF64>(Header.sh_info);
  Str.writeELFXword<IsELF64>(Header.sh_addralign);
  Str.writeELFXword<IsELF64>(Header.sh_entsize);
}

template <bool IsELF64>
void ELFSymbolTableSection::writeSymbolMap(ELFStreamer &Str,
                                           const SymMap &Map) {
  // The order of the fields is different, so branch on IsELF64.
  if (IsELF64) {
    for (auto &KeyValue : Map) {
      const Elf64_Sym &SymInfo = KeyValue.second.Sym;
      Str.writeELFWord<IsELF64>(SymInfo.st_name);
      Str.write8(SymInfo.st_info);
      Str.write8(SymInfo.st_other);
      Str.writeLE16(SymInfo.st_shndx);
      Str.writeAddrOrOffset<IsELF64>(SymInfo.st_value);
      Str.writeELFXword<IsELF64>(SymInfo.st_size);
    }
  } else {
    for (auto &KeyValue : Map) {
      const Elf64_Sym &SymInfo = KeyValue.second.Sym;
      Str.writeELFWord<IsELF64>(SymInfo.st_name);
      Str.writeAddrOrOffset<IsELF64>(SymInfo.st_value);
      Str.writeELFWord<IsELF64>(SymInfo.st_size);
      Str.write8(SymInfo.st_info);
      Str.write8(SymInfo.st_other);
      Str.writeLE16(SymInfo.st_shndx);
    }
  }
}

template <bool IsELF64>
void ELFRelocationSection::writeData(ELFStreamer &Str,
                                     const ELFSymbolTableSection *SymTab) {
  for (const AssemblerFixup &Fixup : Fixups) {
    const ELFSym *Symbol;
    if (Fixup.isNullSymbol()) {
      Symbol = SymTab->getNullSymbol();
    } else if (Fixup.valueIsSymbol()) {
      Symbol = Fixup.getSymbolValue();
    } else {
      GlobalString Name = Fixup.symbol();
      Symbol = SymTab->findSymbol(Name);
      if (!Symbol)
        llvm::report_fatal_error(Name + ": Missing symbol mentioned in reloc");
    }

    if (IsELF64) {
      Elf64_Rela Rela;
      Rela.r_offset = Fixup.position();
      Rela.setSymbolAndType(Symbol->getNumber(), Fixup.kind());
      Rela.r_addend = Fixup.offset();
      Str.writeAddrOrOffset<IsELF64>(Rela.r_offset);
      Str.writeELFXword<IsELF64>(Rela.r_info);
      Str.writeELFXword<IsELF64>(Rela.r_addend);
    } else {
      Elf32_Rel Rel;
      Rel.r_offset = Fixup.position();
      Rel.setSymbolAndType(Symbol->getNumber(), Fixup.kind());
      Str.writeAddrOrOffset<IsELF64>(Rel.r_offset);
      Str.writeELFWord<IsELF64>(Rel.r_info);
    }
  }
}

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEELFSECTION_H
