//===------ utils/obj2yaml.cpp - obj2yaml conversion tool -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "obj2yaml.h"
#include "llvm/Object/COFF.h"
#include "llvm/Object/COFFYAML.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/YAMLTraits.h"

using namespace llvm;

namespace {

class COFFDumper {
  const object::COFFObjectFile &Obj;
  COFFYAML::Object YAMLObj;
  template <typename T>
  void dumpOptionalHeader(T OptionalHeader);
  void dumpHeader();
  void dumpSections(unsigned numSections);
  void dumpSymbols(unsigned numSymbols);

public:
  COFFDumper(const object::COFFObjectFile &Obj);
  COFFYAML::Object &getYAMLObj();
};

}

COFFDumper::COFFDumper(const object::COFFObjectFile &Obj) : Obj(Obj) {
  const object::pe32_header *PE32Header = nullptr;
  Obj.getPE32Header(PE32Header);
  if (PE32Header) {
    dumpOptionalHeader(PE32Header);
  } else {
    const object::pe32plus_header *PE32PlusHeader = nullptr;
    Obj.getPE32PlusHeader(PE32PlusHeader);
    if (PE32PlusHeader) {
      dumpOptionalHeader(PE32PlusHeader);
    }
  }
  dumpHeader();
  dumpSections(Obj.getNumberOfSections());
  dumpSymbols(Obj.getNumberOfSymbols());
}

template <typename T> void COFFDumper::dumpOptionalHeader(T OptionalHeader) {
  YAMLObj.OptionalHeader = COFFYAML::PEHeader();
  YAMLObj.OptionalHeader->Header.AddressOfEntryPoint =
      OptionalHeader->AddressOfEntryPoint;
  YAMLObj.OptionalHeader->Header.AddressOfEntryPoint =
      OptionalHeader->AddressOfEntryPoint;
  YAMLObj.OptionalHeader->Header.ImageBase = OptionalHeader->ImageBase;
  YAMLObj.OptionalHeader->Header.SectionAlignment =
      OptionalHeader->SectionAlignment;
  YAMLObj.OptionalHeader->Header.FileAlignment = OptionalHeader->FileAlignment;
  YAMLObj.OptionalHeader->Header.MajorOperatingSystemVersion =
      OptionalHeader->MajorOperatingSystemVersion;
  YAMLObj.OptionalHeader->Header.MinorOperatingSystemVersion =
      OptionalHeader->MinorOperatingSystemVersion;
  YAMLObj.OptionalHeader->Header.MajorImageVersion =
      OptionalHeader->MajorImageVersion;
  YAMLObj.OptionalHeader->Header.MinorImageVersion =
      OptionalHeader->MinorImageVersion;
  YAMLObj.OptionalHeader->Header.MajorSubsystemVersion =
      OptionalHeader->MajorSubsystemVersion;
  YAMLObj.OptionalHeader->Header.MinorSubsystemVersion =
      OptionalHeader->MinorSubsystemVersion;
  YAMLObj.OptionalHeader->Header.Subsystem = OptionalHeader->Subsystem;
  YAMLObj.OptionalHeader->Header.DLLCharacteristics =
      OptionalHeader->DLLCharacteristics;
  YAMLObj.OptionalHeader->Header.SizeOfStackReserve =
      OptionalHeader->SizeOfStackReserve;
  YAMLObj.OptionalHeader->Header.SizeOfStackCommit =
      OptionalHeader->SizeOfStackCommit;
  YAMLObj.OptionalHeader->Header.SizeOfHeapReserve =
      OptionalHeader->SizeOfHeapReserve;
  YAMLObj.OptionalHeader->Header.SizeOfHeapCommit =
      OptionalHeader->SizeOfHeapCommit;
  unsigned I = 0;
  for (auto &DestDD : YAMLObj.OptionalHeader->DataDirectories) {
    const object::data_directory *DD;
    if (Obj.getDataDirectory(I++, DD))
      continue;
    DestDD = COFF::DataDirectory();
    DestDD->RelativeVirtualAddress = DD->RelativeVirtualAddress;
    DestDD->Size = DD->Size;
  }
}

void COFFDumper::dumpHeader() {
  YAMLObj.Header.Machine = Obj.getMachine();
  YAMLObj.Header.Characteristics = Obj.getCharacteristics();
}

void COFFDumper::dumpSections(unsigned NumSections) {
  std::vector<COFFYAML::Section> &YAMLSections = YAMLObj.Sections;
  for (const auto &ObjSection : Obj.sections()) {
    const object::coff_section *COFFSection = Obj.getCOFFSection(ObjSection);
    COFFYAML::Section NewYAMLSection;
    ObjSection.getName(NewYAMLSection.Name);
    NewYAMLSection.Header.Characteristics = COFFSection->Characteristics;
    NewYAMLSection.Header.VirtualAddress = ObjSection.getAddress();
    NewYAMLSection.Header.VirtualSize = COFFSection->VirtualSize;
    NewYAMLSection.Alignment = ObjSection.getAlignment();

    ArrayRef<uint8_t> sectionData;
    if (!ObjSection.isBSS())
      Obj.getSectionContents(COFFSection, sectionData);
    NewYAMLSection.SectionData = yaml::BinaryRef(sectionData);

    std::vector<COFFYAML::Relocation> Relocations;
    for (const auto &Reloc : ObjSection.relocations()) {
      const object::coff_relocation *reloc = Obj.getCOFFRelocation(Reloc);
      COFFYAML::Relocation Rel;
      object::symbol_iterator Sym = Reloc.getSymbol();
      ErrorOr<StringRef> SymbolNameOrErr = Sym->getName();
      if (std::error_code EC = SymbolNameOrErr.getError())
        report_fatal_error(EC.message());
      Rel.SymbolName = *SymbolNameOrErr;
      Rel.VirtualAddress = reloc->VirtualAddress;
      Rel.Type = reloc->Type;
      Relocations.push_back(Rel);
    }
    NewYAMLSection.Relocations = Relocations;
    YAMLSections.push_back(NewYAMLSection);
  }
}

static void
dumpFunctionDefinition(COFFYAML::Symbol *Sym,
                       const object::coff_aux_function_definition *ObjFD) {
  COFF::AuxiliaryFunctionDefinition YAMLFD;
  YAMLFD.TagIndex = ObjFD->TagIndex;
  YAMLFD.TotalSize = ObjFD->TotalSize;
  YAMLFD.PointerToLinenumber = ObjFD->PointerToLinenumber;
  YAMLFD.PointerToNextFunction = ObjFD->PointerToNextFunction;

  Sym->FunctionDefinition = YAMLFD;
}

static void
dumpbfAndEfLineInfo(COFFYAML::Symbol *Sym,
                    const object::coff_aux_bf_and_ef_symbol *ObjBES) {
  COFF::AuxiliarybfAndefSymbol YAMLAAS;
  YAMLAAS.Linenumber = ObjBES->Linenumber;
  YAMLAAS.PointerToNextFunction = ObjBES->PointerToNextFunction;

  Sym->bfAndefSymbol = YAMLAAS;
}

static void dumpWeakExternal(COFFYAML::Symbol *Sym,
                             const object::coff_aux_weak_external *ObjWE) {
  COFF::AuxiliaryWeakExternal YAMLWE;
  YAMLWE.TagIndex = ObjWE->TagIndex;
  YAMLWE.Characteristics = ObjWE->Characteristics;

  Sym->WeakExternal = YAMLWE;
}

static void
dumpSectionDefinition(COFFYAML::Symbol *Sym,
                      const object::coff_aux_section_definition *ObjSD,
                      bool IsBigObj) {
  COFF::AuxiliarySectionDefinition YAMLASD;
  int32_t AuxNumber = ObjSD->getNumber(IsBigObj);
  YAMLASD.Length = ObjSD->Length;
  YAMLASD.NumberOfRelocations = ObjSD->NumberOfRelocations;
  YAMLASD.NumberOfLinenumbers = ObjSD->NumberOfLinenumbers;
  YAMLASD.CheckSum = ObjSD->CheckSum;
  YAMLASD.Number = AuxNumber;
  YAMLASD.Selection = ObjSD->Selection;

  Sym->SectionDefinition = YAMLASD;
}

static void
dumpCLRTokenDefinition(COFFYAML::Symbol *Sym,
                       const object::coff_aux_clr_token *ObjCLRToken) {
  COFF::AuxiliaryCLRToken YAMLCLRToken;
  YAMLCLRToken.AuxType = ObjCLRToken->AuxType;
  YAMLCLRToken.SymbolTableIndex = ObjCLRToken->SymbolTableIndex;

  Sym->CLRToken = YAMLCLRToken;
}

void COFFDumper::dumpSymbols(unsigned NumSymbols) {
  std::vector<COFFYAML::Symbol> &Symbols = YAMLObj.Symbols;
  for (const auto &S : Obj.symbols()) {
    object::COFFSymbolRef Symbol = Obj.getCOFFSymbol(S);
    COFFYAML::Symbol Sym;
    Obj.getSymbolName(Symbol, Sym.Name);
    Sym.SimpleType = COFF::SymbolBaseType(Symbol.getBaseType());
    Sym.ComplexType = COFF::SymbolComplexType(Symbol.getComplexType());
    Sym.Header.StorageClass = Symbol.getStorageClass();
    Sym.Header.Value = Symbol.getValue();
    Sym.Header.SectionNumber = Symbol.getSectionNumber();
    Sym.Header.NumberOfAuxSymbols = Symbol.getNumberOfAuxSymbols();

    if (Symbol.getNumberOfAuxSymbols() > 0) {
      ArrayRef<uint8_t> AuxData = Obj.getSymbolAuxData(Symbol);
      if (Symbol.isFunctionDefinition()) {
        // This symbol represents a function definition.
        assert(Symbol.getNumberOfAuxSymbols() == 1 &&
               "Expected a single aux symbol to describe this function!");

        const object::coff_aux_function_definition *ObjFD =
            reinterpret_cast<const object::coff_aux_function_definition *>(
                AuxData.data());
        dumpFunctionDefinition(&Sym, ObjFD);
      } else if (Symbol.isFunctionLineInfo()) {
        // This symbol describes function line number information.
        assert(Symbol.getNumberOfAuxSymbols() == 1 &&
               "Expected a single aux symbol to describe this function!");

        const object::coff_aux_bf_and_ef_symbol *ObjBES =
            reinterpret_cast<const object::coff_aux_bf_and_ef_symbol *>(
                AuxData.data());
        dumpbfAndEfLineInfo(&Sym, ObjBES);
      } else if (Symbol.isAnyUndefined()) {
        // This symbol represents a weak external definition.
        assert(Symbol.getNumberOfAuxSymbols() == 1 &&
               "Expected a single aux symbol to describe this weak symbol!");

        const object::coff_aux_weak_external *ObjWE =
            reinterpret_cast<const object::coff_aux_weak_external *>(
                AuxData.data());
        dumpWeakExternal(&Sym, ObjWE);
      } else if (Symbol.isFileRecord()) {
        // This symbol represents a file record.
        Sym.File = StringRef(reinterpret_cast<const char *>(AuxData.data()),
                             Symbol.getNumberOfAuxSymbols() *
                                 Obj.getSymbolTableEntrySize())
                       .rtrim(StringRef("\0", /*length=*/1));
      } else if (Symbol.isSectionDefinition()) {
        // This symbol represents a section definition.
        assert(Symbol.getNumberOfAuxSymbols() == 1 &&
               "Expected a single aux symbol to describe this section!");

        const object::coff_aux_section_definition *ObjSD =
            reinterpret_cast<const object::coff_aux_section_definition *>(
                AuxData.data());
        dumpSectionDefinition(&Sym, ObjSD, Symbol.isBigObj());
      } else if (Symbol.isCLRToken()) {
        // This symbol represents a CLR token definition.
        assert(Symbol.getNumberOfAuxSymbols() == 1 &&
               "Expected a single aux symbol to describe this CLR Token!");

        const object::coff_aux_clr_token *ObjCLRToken =
            reinterpret_cast<const object::coff_aux_clr_token *>(
                AuxData.data());
        dumpCLRTokenDefinition(&Sym, ObjCLRToken);
      } else {
        llvm_unreachable("Unhandled auxiliary symbol!");
      }
    }
    Symbols.push_back(Sym);
  }
}

COFFYAML::Object &COFFDumper::getYAMLObj() {
  return YAMLObj;
}

std::error_code coff2yaml(raw_ostream &Out, const object::COFFObjectFile &Obj) {
  COFFDumper Dumper(Obj);

  yaml::Output Yout(Out);
  Yout << Dumper.getYAMLObj();

  return std::error_code();
}
