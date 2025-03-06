//===------ utils/elf2yaml.cpp - obj2yaml conversion tool -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Error.h"
#include "obj2yaml.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/ELFYAML.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/YAMLTraits.h"

using namespace llvm;

namespace {

template <class ELFT>
class ELFDumper {
  typedef object::Elf_Sym_Impl<ELFT> Elf_Sym;
  typedef typename object::ELFFile<ELFT>::Elf_Shdr Elf_Shdr;
  typedef typename object::ELFFile<ELFT>::Elf_Word Elf_Word;

  const object::ELFFile<ELFT> &Obj;

  std::error_code dumpSymbol(const Elf_Sym *Sym, bool IsDynamic,
                             ELFYAML::Symbol &S);
  std::error_code dumpCommonSection(const Elf_Shdr *Shdr, ELFYAML::Section &S);
  std::error_code dumpCommonRelocationSection(const Elf_Shdr *Shdr,
                                              ELFYAML::RelocationSection &S);
  template <class RelT>
  std::error_code dumpRelocation(const Elf_Shdr *Shdr, const RelT *Rel,
                                 ELFYAML::Relocation &R);

  ErrorOr<ELFYAML::RelocationSection *> dumpRelSection(const Elf_Shdr *Shdr);
  ErrorOr<ELFYAML::RelocationSection *> dumpRelaSection(const Elf_Shdr *Shdr);
  ErrorOr<ELFYAML::RawContentSection *>
  dumpContentSection(const Elf_Shdr *Shdr);
  ErrorOr<ELFYAML::NoBitsSection *> dumpNoBitsSection(const Elf_Shdr *Shdr);
  ErrorOr<ELFYAML::Group *> dumpGroup(const Elf_Shdr *Shdr);
  ErrorOr<ELFYAML::MipsABIFlags *> dumpMipsABIFlags(const Elf_Shdr *Shdr);

public:
  ELFDumper(const object::ELFFile<ELFT> &O);
  ErrorOr<ELFYAML::Object *> dump();
};

}

template <class ELFT>
ELFDumper<ELFT>::ELFDumper(const object::ELFFile<ELFT> &O)
    : Obj(O) {}

template <class ELFT>
ErrorOr<ELFYAML::Object *> ELFDumper<ELFT>::dump() {
  auto Y = make_unique<ELFYAML::Object>();

  // Dump header
  Y->Header.Class = ELFYAML::ELF_ELFCLASS(Obj.getHeader()->getFileClass());
  Y->Header.Data = ELFYAML::ELF_ELFDATA(Obj.getHeader()->getDataEncoding());
  Y->Header.OSABI = Obj.getHeader()->e_ident[ELF::EI_OSABI];
  Y->Header.Type = Obj.getHeader()->e_type;
  Y->Header.Machine = Obj.getHeader()->e_machine;
  Y->Header.Flags = Obj.getHeader()->e_flags;
  Y->Header.Entry = Obj.getHeader()->e_entry;

  // Dump sections
  for (const Elf_Shdr &Sec : Obj.sections()) {
    switch (Sec.sh_type) {
    case ELF::SHT_NULL:
    case ELF::SHT_SYMTAB:
    case ELF::SHT_DYNSYM:
    case ELF::SHT_STRTAB:
      // Do not dump these sections.
      break;
    case ELF::SHT_RELA: {
      ErrorOr<ELFYAML::RelocationSection *> S = dumpRelaSection(&Sec);
      if (std::error_code EC = S.getError())
        return EC;
      Y->Sections.push_back(std::unique_ptr<ELFYAML::Section>(S.get()));
      break;
    }
    case ELF::SHT_REL: {
      ErrorOr<ELFYAML::RelocationSection *> S = dumpRelSection(&Sec);
      if (std::error_code EC = S.getError())
        return EC;
      Y->Sections.push_back(std::unique_ptr<ELFYAML::Section>(S.get()));
      break;
    }
    case ELF::SHT_GROUP: {
      ErrorOr<ELFYAML::Group *> G = dumpGroup(&Sec);
      if (std::error_code EC = G.getError())
        return EC;
      Y->Sections.push_back(std::unique_ptr<ELFYAML::Section>(G.get()));
      break;
    }
    case ELF::SHT_MIPS_ABIFLAGS: {
      ErrorOr<ELFYAML::MipsABIFlags *> G = dumpMipsABIFlags(&Sec);
      if (std::error_code EC = G.getError())
        return EC;
      Y->Sections.push_back(std::unique_ptr<ELFYAML::Section>(G.get()));
      break;
    }
    case ELF::SHT_NOBITS: {
      ErrorOr<ELFYAML::NoBitsSection *> S = dumpNoBitsSection(&Sec);
      if (std::error_code EC = S.getError())
        return EC;
      Y->Sections.push_back(std::unique_ptr<ELFYAML::Section>(S.get()));
      break;
    }
    default: {
      ErrorOr<ELFYAML::RawContentSection *> S = dumpContentSection(&Sec);
      if (std::error_code EC = S.getError())
        return EC;
      Y->Sections.push_back(std::unique_ptr<ELFYAML::Section>(S.get()));
    }
    }
  }

  // Dump symbols
  bool IsFirstSym = true;
  for (auto SI = Obj.symbol_begin(), SE = Obj.symbol_end(); SI != SE; ++SI) {
    if (IsFirstSym) {
      IsFirstSym = false;
      continue;
    }

    ELFYAML::Symbol S;
    if (std::error_code EC = ELFDumper<ELFT>::dumpSymbol(SI, false, S))
      return EC;

    switch (SI->getBinding())
    {
    case ELF::STB_LOCAL:
      Y->Symbols.Local.push_back(S);
      break;
    case ELF::STB_GLOBAL:
      Y->Symbols.Global.push_back(S);
      break;
    case ELF::STB_WEAK:
      Y->Symbols.Weak.push_back(S);
      break;
    default:
      llvm_unreachable("Unknown ELF symbol binding");
    }
  }

  return Y.release();
}

template <class ELFT>
std::error_code ELFDumper<ELFT>::dumpSymbol(const Elf_Sym *Sym, bool IsDynamic,
                                            ELFYAML::Symbol &S) {
  S.Type = Sym->getType();
  S.Value = Sym->st_value;
  S.Size = Sym->st_size;
  S.Other = Sym->st_other;

  ErrorOr<StringRef> NameOrErr = Obj.getSymbolName(Sym, IsDynamic);
  if (std::error_code EC = NameOrErr.getError())
    return EC;
  S.Name = NameOrErr.get();

  ErrorOr<const Elf_Shdr *> ShdrOrErr = Obj.getSection(&*Sym);
  if (std::error_code EC = ShdrOrErr.getError())
    return EC;
  const Elf_Shdr *Shdr = *ShdrOrErr;
  if (!Shdr)
    return obj2yaml_error::success;

  NameOrErr = Obj.getSectionName(Shdr);
  if (std::error_code EC = NameOrErr.getError())
    return EC;
  S.Section = NameOrErr.get();

  return obj2yaml_error::success;
}

template <class ELFT>
template <class RelT>
std::error_code ELFDumper<ELFT>::dumpRelocation(const Elf_Shdr *Shdr,
                                                const RelT *Rel,
                                                ELFYAML::Relocation &R) {
  R.Type = Rel->getType(Obj.isMips64EL());
  R.Offset = Rel->r_offset;
  R.Addend = 0;

  auto NamePair = Obj.getRelocationSymbol(Shdr, Rel);
  if (!NamePair.first)
    return obj2yaml_error::success;

  const Elf_Shdr *SymTab = NamePair.first;
  ErrorOr<const Elf_Shdr *> StrTabSec = Obj.getSection(SymTab->sh_link);
  if (std::error_code EC = StrTabSec.getError())
    return EC;
  ErrorOr<StringRef> StrTabOrErr = Obj.getStringTable(*StrTabSec);
  if (std::error_code EC = StrTabOrErr.getError())
    return EC;
  StringRef StrTab = *StrTabOrErr;

  ErrorOr<StringRef> NameOrErr = NamePair.second->getName(StrTab);
  if (std::error_code EC = NameOrErr.getError())
    return EC;
  R.Symbol = NameOrErr.get();

  return obj2yaml_error::success;
}

template <class ELFT>
std::error_code ELFDumper<ELFT>::dumpCommonSection(const Elf_Shdr *Shdr,
                                                   ELFYAML::Section &S) {
  S.Type = Shdr->sh_type;
  S.Flags = Shdr->sh_flags;
  S.Address = Shdr->sh_addr;
  S.AddressAlign = Shdr->sh_addralign;

  ErrorOr<StringRef> NameOrErr = Obj.getSectionName(Shdr);
  if (std::error_code EC = NameOrErr.getError())
    return EC;
  S.Name = NameOrErr.get();

  if (Shdr->sh_link != ELF::SHN_UNDEF) {
    ErrorOr<const Elf_Shdr *> LinkSection = Obj.getSection(Shdr->sh_link);
    if (std::error_code EC = LinkSection.getError())
      return EC;
    NameOrErr = Obj.getSectionName(*LinkSection);
    if (std::error_code EC = NameOrErr.getError())
      return EC;
    S.Link = NameOrErr.get();
  }

  return obj2yaml_error::success;
}

template <class ELFT>
std::error_code
ELFDumper<ELFT>::dumpCommonRelocationSection(const Elf_Shdr *Shdr,
                                             ELFYAML::RelocationSection &S) {
  if (std::error_code EC = dumpCommonSection(Shdr, S))
    return EC;

  ErrorOr<const Elf_Shdr *> InfoSection = Obj.getSection(Shdr->sh_info);
  if (std::error_code EC = InfoSection.getError())
    return EC;

  ErrorOr<StringRef> NameOrErr = Obj.getSectionName(*InfoSection);
  if (std::error_code EC = NameOrErr.getError())
    return EC;
  S.Info = NameOrErr.get();

  return obj2yaml_error::success;
}

template <class ELFT>
ErrorOr<ELFYAML::RelocationSection *>
ELFDumper<ELFT>::dumpRelSection(const Elf_Shdr *Shdr) {
  assert(Shdr->sh_type == ELF::SHT_REL && "Section type is not SHT_REL");
  auto S = make_unique<ELFYAML::RelocationSection>();

  if (std::error_code EC = dumpCommonRelocationSection(Shdr, *S))
    return EC;

  for (auto RI = Obj.rel_begin(Shdr), RE = Obj.rel_end(Shdr); RI != RE; ++RI) {
    ELFYAML::Relocation R;
    if (std::error_code EC = dumpRelocation(Shdr, &*RI, R))
      return EC;
    S->Relocations.push_back(R);
  }

  return S.release();
}

template <class ELFT>
ErrorOr<ELFYAML::RelocationSection *>
ELFDumper<ELFT>::dumpRelaSection(const Elf_Shdr *Shdr) {
  assert(Shdr->sh_type == ELF::SHT_RELA && "Section type is not SHT_RELA");
  auto S = make_unique<ELFYAML::RelocationSection>();

  if (std::error_code EC = dumpCommonRelocationSection(Shdr, *S))
    return EC;

  for (auto RI = Obj.rela_begin(Shdr), RE = Obj.rela_end(Shdr); RI != RE;
       ++RI) {
    ELFYAML::Relocation R;
    if (std::error_code EC = dumpRelocation(Shdr, &*RI, R))
      return EC;
    R.Addend = RI->r_addend;
    S->Relocations.push_back(R);
  }

  return S.release();
}

template <class ELFT>
ErrorOr<ELFYAML::RawContentSection *>
ELFDumper<ELFT>::dumpContentSection(const Elf_Shdr *Shdr) {
  auto S = make_unique<ELFYAML::RawContentSection>();

  if (std::error_code EC = dumpCommonSection(Shdr, *S))
    return EC;

  ErrorOr<ArrayRef<uint8_t>> ContentOrErr = Obj.getSectionContents(Shdr);
  if (std::error_code EC = ContentOrErr.getError())
    return EC;
  S->Content = yaml::BinaryRef(ContentOrErr.get());
  S->Size = S->Content.binary_size();

  return S.release();
}

template <class ELFT>
ErrorOr<ELFYAML::NoBitsSection *>
ELFDumper<ELFT>::dumpNoBitsSection(const Elf_Shdr *Shdr) {
  auto S = make_unique<ELFYAML::NoBitsSection>();

  if (std::error_code EC = dumpCommonSection(Shdr, *S))
    return EC;
  S->Size = Shdr->sh_size;

  return S.release();
}

template <class ELFT>
ErrorOr<ELFYAML::Group *> ELFDumper<ELFT>::dumpGroup(const Elf_Shdr *Shdr) {
  auto S = make_unique<ELFYAML::Group>();

  if (std::error_code EC = dumpCommonSection(Shdr, *S))
    return EC;
  // Get sh_info which is the signature.
  const Elf_Sym *symbol = Obj.getSymbol(Shdr->sh_info);
  ErrorOr<const Elf_Shdr *> Symtab = Obj.getSection(Shdr->sh_link);
  if (std::error_code EC = Symtab.getError())
    return EC;
  ErrorOr<const Elf_Shdr *> StrTabSec = Obj.getSection((*Symtab)->sh_link);
  if (std::error_code EC = StrTabSec.getError())
    return EC;
  ErrorOr<StringRef> StrTabOrErr = Obj.getStringTable(*StrTabSec);
  if (std::error_code EC = StrTabOrErr.getError())
    return EC;
  StringRef StrTab = *StrTabOrErr;
  auto sectionContents = Obj.getSectionContents(Shdr);
  if (std::error_code ec = sectionContents.getError())
    return ec;
  ErrorOr<StringRef> symbolName = symbol->getName(StrTab);
  if (std::error_code EC = symbolName.getError())
    return EC;
  S->Info = *symbolName;
  const Elf_Word *groupMembers =
      reinterpret_cast<const Elf_Word *>(sectionContents->data());
  const long count = (Shdr->sh_size) / sizeof(Elf_Word);
  ELFYAML::SectionOrType s;
  for (int i = 0; i < count; i++) {
    if (groupMembers[i] == llvm::ELF::GRP_COMDAT) {
      s.sectionNameOrType = "GRP_COMDAT";
    } else {
      ErrorOr<const Elf_Shdr *> sHdr = Obj.getSection(groupMembers[i]);
      if (std::error_code EC = sHdr.getError())
        return EC;
      ErrorOr<StringRef> sectionName = Obj.getSectionName(*sHdr);
      if (std::error_code ec = sectionName.getError())
        return ec;
      s.sectionNameOrType = *sectionName;
    }
    S->Members.push_back(s);
  }
  return S.release();
}

template <class ELFT>
ErrorOr<ELFYAML::MipsABIFlags *>
ELFDumper<ELFT>::dumpMipsABIFlags(const Elf_Shdr *Shdr) {
  assert(Shdr->sh_type == ELF::SHT_MIPS_ABIFLAGS &&
         "Section type is not SHT_MIPS_ABIFLAGS");
  auto S = make_unique<ELFYAML::MipsABIFlags>();
  if (std::error_code EC = dumpCommonSection(Shdr, *S))
    return EC;

  ErrorOr<ArrayRef<uint8_t>> ContentOrErr = Obj.getSectionContents(Shdr);
  if (std::error_code EC = ContentOrErr.getError())
    return EC;

  auto *Flags = reinterpret_cast<const object::Elf_Mips_ABIFlags<ELFT> *>(
      ContentOrErr.get().data());
  S->Version = Flags->version;
  S->ISALevel = Flags->isa_level;
  S->ISARevision = Flags->isa_rev;
  S->GPRSize = Flags->gpr_size;
  S->CPR1Size = Flags->cpr1_size;
  S->CPR2Size = Flags->cpr2_size;
  S->FpABI = Flags->fp_abi;
  S->ISAExtension = Flags->isa_ext;
  S->ASEs = Flags->ases;
  S->Flags1 = Flags->flags1;
  S->Flags2 = Flags->flags2;
  return S.release();
}

template <class ELFT>
static std::error_code elf2yaml(raw_ostream &Out,
                                const object::ELFFile<ELFT> &Obj) {
  ELFDumper<ELFT> Dumper(Obj);
  ErrorOr<ELFYAML::Object *> YAMLOrErr = Dumper.dump();
  if (std::error_code EC = YAMLOrErr.getError())
    return EC;

  std::unique_ptr<ELFYAML::Object> YAML(YAMLOrErr.get());
  yaml::Output Yout(Out);
  Yout << *YAML;

  return std::error_code();
}

std::error_code elf2yaml(raw_ostream &Out, const object::ObjectFile &Obj) {
  if (const auto *ELFObj = dyn_cast<object::ELF32LEObjectFile>(&Obj))
    return elf2yaml(Out, *ELFObj->getELFFile());

  if (const auto *ELFObj = dyn_cast<object::ELF32BEObjectFile>(&Obj))
    return elf2yaml(Out, *ELFObj->getELFFile());

  if (const auto *ELFObj = dyn_cast<object::ELF64LEObjectFile>(&Obj))
    return elf2yaml(Out, *ELFObj->getELFFile());

  if (const auto *ELFObj = dyn_cast<object::ELF64BEObjectFile>(&Obj))
    return elf2yaml(Out, *ELFObj->getELFFile());

  return obj2yaml_error::unsupported_obj_file_format;
}
