//===-- ELFDumper.cpp - ELF-specific dumper ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements the ELF-specific dumper for llvm-readobj.
///
//===----------------------------------------------------------------------===//

#include "llvm-readobj.h"
#include "ARMAttributeParser.h"
#include "ARMEHABIPrinter.h"
#include "Error.h"
#include "ObjDumper.h"
#include "StackMapPrinter.h"
#include "StreamWriter.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Support/ARMBuildAttributes.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/MipsABIFlags.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::object;
using namespace ELF;

#define LLVM_READOBJ_ENUM_CASE(ns, enum) \
  case ns::enum: return #enum;

namespace {

template<typename ELFT>
class ELFDumper : public ObjDumper {
public:
  ELFDumper(const ELFFile<ELFT> *Obj, StreamWriter &Writer)
      : ObjDumper(Writer), Obj(Obj) {}

  void printFileHeaders() override;
  void printSections() override;
  void printRelocations() override;
  void printDynamicRelocations() override;
  void printSymbols() override;
  void printDynamicSymbols() override;
  void printUnwindInfo() override;

  void printDynamicTable() override;
  void printNeededLibraries() override;
  void printProgramHeaders() override;
  void printHashTable() override;

  void printAttributes() override;
  void printMipsPLTGOT() override;
  void printMipsABIFlags() override;
  void printMipsReginfo() override;

  void printStackMap() const override;

private:
  typedef ELFFile<ELFT> ELFO;
  typedef typename ELFO::Elf_Shdr Elf_Shdr;
  typedef typename ELFO::Elf_Sym Elf_Sym;

  void printSymbol(const Elf_Sym *Symbol, bool IsDynamic);

  void printRelocations(const Elf_Shdr *Sec);
  void printRelocation(const Elf_Shdr *Sec, typename ELFO::Elf_Rela Rel);

  const ELFO *Obj;
};

template <class T> T errorOrDefault(ErrorOr<T> Val, T Default = T()) {
  if (!Val) {
    error(Val.getError());
    return Default;
  }

  return *Val;
}
} // namespace

namespace llvm {

template <class ELFT>
static std::error_code createELFDumper(const ELFFile<ELFT> *Obj,
                                       StreamWriter &Writer,
                                       std::unique_ptr<ObjDumper> &Result) {
  Result.reset(new ELFDumper<ELFT>(Obj, Writer));
  return readobj_error::success;
}

std::error_code createELFDumper(const object::ObjectFile *Obj,
                                StreamWriter &Writer,
                                std::unique_ptr<ObjDumper> &Result) {
  // Little-endian 32-bit
  if (const ELF32LEObjectFile *ELFObj = dyn_cast<ELF32LEObjectFile>(Obj))
    return createELFDumper(ELFObj->getELFFile(), Writer, Result);

  // Big-endian 32-bit
  if (const ELF32BEObjectFile *ELFObj = dyn_cast<ELF32BEObjectFile>(Obj))
    return createELFDumper(ELFObj->getELFFile(), Writer, Result);

  // Little-endian 64-bit
  if (const ELF64LEObjectFile *ELFObj = dyn_cast<ELF64LEObjectFile>(Obj))
    return createELFDumper(ELFObj->getELFFile(), Writer, Result);

  // Big-endian 64-bit
  if (const ELF64BEObjectFile *ELFObj = dyn_cast<ELF64BEObjectFile>(Obj))
    return createELFDumper(ELFObj->getELFFile(), Writer, Result);

  return readobj_error::unsupported_obj_file_format;
}

} // namespace llvm

template <typename ELFO>
static std::string getFullSymbolName(const ELFO &Obj,
                                     const typename ELFO::Elf_Sym *Symbol,
                                     bool IsDynamic) {
  StringRef SymbolName = errorOrDefault(Obj.getSymbolName(Symbol, IsDynamic));
  if (!IsDynamic)
    return SymbolName;

  std::string FullSymbolName(SymbolName);

  bool IsDefault;
  ErrorOr<StringRef> Version =
      Obj.getSymbolVersion(nullptr, &*Symbol, IsDefault);
  if (Version) {
    FullSymbolName += (IsDefault ? "@@" : "@");
    FullSymbolName += *Version;
  } else
    error(Version.getError());
  return FullSymbolName;
}

template <typename ELFO>
static void
getSectionNameIndex(const ELFO &Obj, const typename ELFO::Elf_Sym *Symbol,
                    StringRef &SectionName, unsigned &SectionIndex) {
  SectionIndex = Symbol->st_shndx;
  if (Symbol->isUndefined())
    SectionName = "Undefined";
  else if (Symbol->isProcessorSpecific())
    SectionName = "Processor Specific";
  else if (Symbol->isOSSpecific())
    SectionName = "Operating System Specific";
  else if (Symbol->isAbsolute())
    SectionName = "Absolute";
  else if (Symbol->isCommon())
    SectionName = "Common";
  else if (Symbol->isReserved() && SectionIndex != SHN_XINDEX)
    SectionName = "Reserved";
  else {
    if (SectionIndex == SHN_XINDEX)
      SectionIndex = Obj.getExtendedSymbolTableIndex(&*Symbol);
    ErrorOr<const typename ELFO::Elf_Shdr *> Sec = Obj.getSection(SectionIndex);
    if (!error(Sec.getError()))
      SectionName = errorOrDefault(Obj.getSectionName(*Sec));
  }
}

template <class ELFT>
static const typename ELFFile<ELFT>::Elf_Shdr *
findSectionByAddress(const ELFFile<ELFT> *Obj, uint64_t Addr) {
  for (const auto &Shdr : Obj->sections())
    if (Shdr.sh_addr == Addr)
      return &Shdr;
  return nullptr;
}

template <class ELFT>
static const typename ELFFile<ELFT>::Elf_Shdr *
findSectionByName(const ELFFile<ELFT> &Obj, StringRef Name) {
  for (const auto &Shdr : Obj.sections()) {
    if (Name == errorOrDefault(Obj.getSectionName(&Shdr)))
      return &Shdr;
  }
  return nullptr;
}

static const EnumEntry<unsigned> ElfClass[] = {
  { "None",   ELF::ELFCLASSNONE },
  { "32-bit", ELF::ELFCLASS32   },
  { "64-bit", ELF::ELFCLASS64   },
};

static const EnumEntry<unsigned> ElfDataEncoding[] = {
  { "None",         ELF::ELFDATANONE },
  { "LittleEndian", ELF::ELFDATA2LSB },
  { "BigEndian",    ELF::ELFDATA2MSB },
};

static const EnumEntry<unsigned> ElfObjectFileType[] = {
  { "None",         ELF::ET_NONE },
  { "Relocatable",  ELF::ET_REL  },
  { "Executable",   ELF::ET_EXEC },
  { "SharedObject", ELF::ET_DYN  },
  { "Core",         ELF::ET_CORE },
};

static const EnumEntry<unsigned> ElfOSABI[] = {
  { "SystemV",      ELF::ELFOSABI_NONE         },
  { "HPUX",         ELF::ELFOSABI_HPUX         },
  { "NetBSD",       ELF::ELFOSABI_NETBSD       },
  { "GNU/Linux",    ELF::ELFOSABI_LINUX        },
  { "GNU/Hurd",     ELF::ELFOSABI_HURD         },
  { "Solaris",      ELF::ELFOSABI_SOLARIS      },
  { "AIX",          ELF::ELFOSABI_AIX          },
  { "IRIX",         ELF::ELFOSABI_IRIX         },
  { "FreeBSD",      ELF::ELFOSABI_FREEBSD      },
  { "TRU64",        ELF::ELFOSABI_TRU64        },
  { "Modesto",      ELF::ELFOSABI_MODESTO      },
  { "OpenBSD",      ELF::ELFOSABI_OPENBSD      },
  { "OpenVMS",      ELF::ELFOSABI_OPENVMS      },
  { "NSK",          ELF::ELFOSABI_NSK          },
  { "AROS",         ELF::ELFOSABI_AROS         },
  { "FenixOS",      ELF::ELFOSABI_FENIXOS      },
  { "CloudABI",     ELF::ELFOSABI_CLOUDABI     },
  { "C6000_ELFABI", ELF::ELFOSABI_C6000_ELFABI },
  { "C6000_LINUX" , ELF::ELFOSABI_C6000_LINUX  },
  { "ARM",          ELF::ELFOSABI_ARM          },
  { "Standalone"  , ELF::ELFOSABI_STANDALONE   }
};

static const EnumEntry<unsigned> ElfMachineType[] = {
  LLVM_READOBJ_ENUM_ENT(ELF, EM_NONE         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_M32          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SPARC        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_386          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_68K          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_88K          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_IAMCU        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_860          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MIPS         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_S370         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MIPS_RS3_LE  ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_PARISC       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_VPP500       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SPARC32PLUS  ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_960          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_PPC          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_PPC64        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_S390         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SPU          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_V800         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_FR20         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_RH32         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_RCE          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ARM          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ALPHA        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SH           ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SPARCV9      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TRICORE      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ARC          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_H8_300       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_H8_300H      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_H8S          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_H8_500       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_IA_64        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MIPS_X       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_COLDFIRE     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_68HC12       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MMA          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_PCP          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_NCPU         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_NDR1         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_STARCORE     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ME16         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ST100        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TINYJ        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_X86_64       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_PDSP         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_PDP10        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_PDP11        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_FX66         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ST9PLUS      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ST7          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_68HC16       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_68HC11       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_68HC08       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_68HC05       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SVX          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ST19         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_VAX          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_CRIS         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_JAVELIN      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_FIREPATH     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ZSP          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MMIX         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_HUANY        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_PRISM        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_AVR          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_FR30         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_D10V         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_D30V         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_V850         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_M32R         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MN10300      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MN10200      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_PJ           ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_OPENRISC     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ARC_COMPACT  ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_XTENSA       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_VIDEOCORE    ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TMM_GPP      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_NS32K        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TPC          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SNP1K        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ST200        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_IP2K         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MAX          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_CR           ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_F2MC16       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MSP430       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_BLACKFIN     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SE_C33       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SEP          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ARCA         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_UNICORE      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_EXCESS       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_DXP          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ALTERA_NIOS2 ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_CRX          ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_XGATE        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_C166         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_M16C         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_DSPIC30F     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_CE           ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_M32C         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TSK3000      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_RS08         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SHARC        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ECOG2        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SCORE7       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_DSP24        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_VIDEOCORE3   ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_LATTICEMICO32),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SE_C17       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TI_C6000     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TI_C2000     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TI_C5500     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MMDSP_PLUS   ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_CYPRESS_M8C  ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_R32C         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TRIMEDIA     ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_HEXAGON      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_8051         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_STXP7X       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_NDS32        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ECOG1        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ECOG1X       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MAXQ30       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_XIMO16       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MANIK        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_CRAYNV2      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_RX           ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_METAG        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_MCST_ELBRUS  ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ECOG16       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_CR16         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ETPU         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_SLE9X        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_L10M         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_K10M         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_AARCH64      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_AVR32        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_STM8         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TILE64       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TILEPRO      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_CUDA         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_TILEGX       ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_CLOUDSHIELD  ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_COREA_1ST    ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_COREA_2ND    ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_ARC_COMPACT2 ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_OPEN8        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_RL78         ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_VIDEOCORE5   ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_78KOR        ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_56800EX      ),
  LLVM_READOBJ_ENUM_ENT(ELF, EM_AMDGPU       )
};

static const EnumEntry<unsigned> ElfSymbolBindings[] = {
  { "Local",  ELF::STB_LOCAL        },
  { "Global", ELF::STB_GLOBAL       },
  { "Weak",   ELF::STB_WEAK         },
  { "Unique", ELF::STB_GNU_UNIQUE   }
};

static const EnumEntry<unsigned> ElfSymbolTypes[] = {
  { "None",      ELF::STT_NOTYPE    },
  { "Object",    ELF::STT_OBJECT    },
  { "Function",  ELF::STT_FUNC      },
  { "Section",   ELF::STT_SECTION   },
  { "File",      ELF::STT_FILE      },
  { "Common",    ELF::STT_COMMON    },
  { "TLS",       ELF::STT_TLS       },
  { "GNU_IFunc", ELF::STT_GNU_IFUNC }
};

static const char *getElfSectionType(unsigned Arch, unsigned Type) {
  switch (Arch) {
  case ELF::EM_ARM:
    switch (Type) {
    LLVM_READOBJ_ENUM_CASE(ELF, SHT_ARM_EXIDX);
    LLVM_READOBJ_ENUM_CASE(ELF, SHT_ARM_PREEMPTMAP);
    LLVM_READOBJ_ENUM_CASE(ELF, SHT_ARM_ATTRIBUTES);
    LLVM_READOBJ_ENUM_CASE(ELF, SHT_ARM_DEBUGOVERLAY);
    LLVM_READOBJ_ENUM_CASE(ELF, SHT_ARM_OVERLAYSECTION);
    }
  case ELF::EM_HEXAGON:
    switch (Type) { LLVM_READOBJ_ENUM_CASE(ELF, SHT_HEX_ORDERED); }
  case ELF::EM_X86_64:
    switch (Type) { LLVM_READOBJ_ENUM_CASE(ELF, SHT_X86_64_UNWIND); }
  case ELF::EM_MIPS:
  case ELF::EM_MIPS_RS3_LE:
    switch (Type) {
    LLVM_READOBJ_ENUM_CASE(ELF, SHT_MIPS_REGINFO);
    LLVM_READOBJ_ENUM_CASE(ELF, SHT_MIPS_OPTIONS);
    LLVM_READOBJ_ENUM_CASE(ELF, SHT_MIPS_ABIFLAGS);
    }
  }

  switch (Type) {
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_NULL              );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_PROGBITS          );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_SYMTAB            );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_STRTAB            );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_RELA              );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_HASH              );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_DYNAMIC           );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_NOTE              );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_NOBITS            );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_REL               );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_SHLIB             );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_DYNSYM            );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_INIT_ARRAY        );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_FINI_ARRAY        );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_PREINIT_ARRAY     );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_GROUP             );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_SYMTAB_SHNDX      );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_GNU_ATTRIBUTES    );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_GNU_HASH          );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_GNU_verdef        );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_GNU_verneed       );
  LLVM_READOBJ_ENUM_CASE(ELF, SHT_GNU_versym        );
  default: return "";
  }
}

static const EnumEntry<unsigned> ElfSectionFlags[] = {
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_WRITE           ),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_ALLOC           ),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_EXCLUDE         ),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_EXECINSTR       ),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_MERGE           ),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_STRINGS         ),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_INFO_LINK       ),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_LINK_ORDER      ),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_OS_NONCONFORMING),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_GROUP           ),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_TLS             ),
  LLVM_READOBJ_ENUM_ENT(ELF, XCORE_SHF_CP_SECTION),
  LLVM_READOBJ_ENUM_ENT(ELF, XCORE_SHF_DP_SECTION),
  LLVM_READOBJ_ENUM_ENT(ELF, SHF_MIPS_NOSTRIP    )
};

static const char *getElfSegmentType(unsigned Arch, unsigned Type) {
  // Check potentially overlapped processor-specific
  // program header type.
  switch (Arch) {
  case ELF::EM_ARM:
    switch (Type) {
    LLVM_READOBJ_ENUM_CASE(ELF, PT_ARM_EXIDX);
    }
  case ELF::EM_MIPS:
  case ELF::EM_MIPS_RS3_LE:
    switch (Type) {
    LLVM_READOBJ_ENUM_CASE(ELF, PT_MIPS_REGINFO);
    LLVM_READOBJ_ENUM_CASE(ELF, PT_MIPS_RTPROC);
    LLVM_READOBJ_ENUM_CASE(ELF, PT_MIPS_OPTIONS);
    LLVM_READOBJ_ENUM_CASE(ELF, PT_MIPS_ABIFLAGS);
    }
  }

  switch (Type) {
  LLVM_READOBJ_ENUM_CASE(ELF, PT_NULL   );
  LLVM_READOBJ_ENUM_CASE(ELF, PT_LOAD   );
  LLVM_READOBJ_ENUM_CASE(ELF, PT_DYNAMIC);
  LLVM_READOBJ_ENUM_CASE(ELF, PT_INTERP );
  LLVM_READOBJ_ENUM_CASE(ELF, PT_NOTE   );
  LLVM_READOBJ_ENUM_CASE(ELF, PT_SHLIB  );
  LLVM_READOBJ_ENUM_CASE(ELF, PT_PHDR   );
  LLVM_READOBJ_ENUM_CASE(ELF, PT_TLS    );

  LLVM_READOBJ_ENUM_CASE(ELF, PT_GNU_EH_FRAME);
  LLVM_READOBJ_ENUM_CASE(ELF, PT_SUNW_UNWIND);

  LLVM_READOBJ_ENUM_CASE(ELF, PT_GNU_STACK);
  LLVM_READOBJ_ENUM_CASE(ELF, PT_GNU_RELRO);
  default: return "";
  }
}

static const EnumEntry<unsigned> ElfSegmentFlags[] = {
  LLVM_READOBJ_ENUM_ENT(ELF, PF_X),
  LLVM_READOBJ_ENUM_ENT(ELF, PF_W),
  LLVM_READOBJ_ENUM_ENT(ELF, PF_R)
};

static const EnumEntry<unsigned> ElfHeaderMipsFlags[] = {
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_NOREORDER),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_PIC),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_CPIC),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ABI2),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_32BITMODE),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_FP64),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_NAN2008),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ABI_O32),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ABI_O64),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ABI_EABI32),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ABI_EABI64),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_3900),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_4010),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_4100),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_4650),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_4120),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_4111),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_SB1),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_OCTEON),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_XLR),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_OCTEON2),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_OCTEON3),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_5400),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_5900),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_5500),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_9000),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_LS2E),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_LS2F),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MACH_LS3A),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_MICROMIPS),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_ASE_M16),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_ASE_MDMX),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_1),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_2),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_3),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_4),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_5),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_32),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_64),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_32R2),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_64R2),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_32R6),
  LLVM_READOBJ_ENUM_ENT(ELF, EF_MIPS_ARCH_64R6)
};

template<class ELFT>
void ELFDumper<ELFT>::printFileHeaders() {
  const typename ELFO::Elf_Ehdr *Header = Obj->getHeader();

  {
    DictScope D(W, "ElfHeader");
    {
      DictScope D(W, "Ident");
      W.printBinary("Magic", makeArrayRef(Header->e_ident).slice(ELF::EI_MAG0,
                                                                 4));
      W.printEnum  ("Class", Header->e_ident[ELF::EI_CLASS],
                      makeArrayRef(ElfClass));
      W.printEnum  ("DataEncoding", Header->e_ident[ELF::EI_DATA],
                      makeArrayRef(ElfDataEncoding));
      W.printNumber("FileVersion", Header->e_ident[ELF::EI_VERSION]);

      // Handle architecture specific OS/ABI values.
      if (Header->e_machine == ELF::EM_AMDGPU &&
          Header->e_ident[ELF::EI_OSABI] == ELF::ELFOSABI_AMDGPU_HSA)
        W.printHex("OS/ABI", "AMDGPU_HSA", ELF::ELFOSABI_AMDGPU_HSA);
      else
        W.printEnum  ("OS/ABI", Header->e_ident[ELF::EI_OSABI],
                      makeArrayRef(ElfOSABI));
      W.printNumber("ABIVersion", Header->e_ident[ELF::EI_ABIVERSION]);
      W.printBinary("Unused", makeArrayRef(Header->e_ident).slice(ELF::EI_PAD));
    }

    W.printEnum  ("Type", Header->e_type, makeArrayRef(ElfObjectFileType));
    W.printEnum  ("Machine", Header->e_machine, makeArrayRef(ElfMachineType));
    W.printNumber("Version", Header->e_version);
    W.printHex   ("Entry", Header->e_entry);
    W.printHex   ("ProgramHeaderOffset", Header->e_phoff);
    W.printHex   ("SectionHeaderOffset", Header->e_shoff);
    if (Header->e_machine == EM_MIPS)
      W.printFlags("Flags", Header->e_flags, makeArrayRef(ElfHeaderMipsFlags),
                   unsigned(ELF::EF_MIPS_ARCH), unsigned(ELF::EF_MIPS_ABI),
                   unsigned(ELF::EF_MIPS_MACH));
    else
      W.printFlags("Flags", Header->e_flags);
    W.printNumber("HeaderSize", Header->e_ehsize);
    W.printNumber("ProgramHeaderEntrySize", Header->e_phentsize);
    W.printNumber("ProgramHeaderCount", Header->e_phnum);
    W.printNumber("SectionHeaderEntrySize", Header->e_shentsize);
    W.printNumber("SectionHeaderCount", Header->e_shnum);
    W.printNumber("StringTableSectionIndex", Header->e_shstrndx);
  }
}

template<class ELFT>
void ELFDumper<ELFT>::printSections() {
  ListScope SectionsD(W, "Sections");

  int SectionIndex = -1;
  for (const typename ELFO::Elf_Shdr &Sec : Obj->sections()) {
    ++SectionIndex;

    StringRef Name = errorOrDefault(Obj->getSectionName(&Sec));

    DictScope SectionD(W, "Section");
    W.printNumber("Index", SectionIndex);
    W.printNumber("Name", Name, Sec.sh_name);
    W.printHex("Type",
               getElfSectionType(Obj->getHeader()->e_machine, Sec.sh_type),
               Sec.sh_type);
    W.printFlags("Flags", Sec.sh_flags, makeArrayRef(ElfSectionFlags));
    W.printHex("Address", Sec.sh_addr);
    W.printHex("Offset", Sec.sh_offset);
    W.printNumber("Size", Sec.sh_size);
    W.printNumber("Link", Sec.sh_link);
    W.printNumber("Info", Sec.sh_info);
    W.printNumber("AddressAlignment", Sec.sh_addralign);
    W.printNumber("EntrySize", Sec.sh_entsize);

    if (opts::SectionRelocations) {
      ListScope D(W, "Relocations");
      printRelocations(&Sec);
    }

    if (opts::SectionSymbols) {
      ListScope D(W, "Symbols");
      for (const typename ELFO::Elf_Sym &Sym : Obj->symbols()) {
        ErrorOr<const Elf_Shdr *> SymSec = Obj->getSection(&Sym);
        if (!SymSec)
          continue;
        if (*SymSec == &Sec)
          printSymbol(&Sym, false);
      }
    }

    if (opts::SectionData && Sec.sh_type != ELF::SHT_NOBITS) {
      ArrayRef<uint8_t> Data = errorOrDefault(Obj->getSectionContents(&Sec));
      W.printBinaryBlock("SectionData",
                         StringRef((const char *)Data.data(), Data.size()));
    }
  }
}

template<class ELFT>
void ELFDumper<ELFT>::printRelocations() {
  ListScope D(W, "Relocations");

  int SectionNumber = -1;
  for (const typename ELFO::Elf_Shdr &Sec : Obj->sections()) {
    ++SectionNumber;

    if (Sec.sh_type != ELF::SHT_REL && Sec.sh_type != ELF::SHT_RELA)
      continue;

    StringRef Name = errorOrDefault(Obj->getSectionName(&Sec));

    W.startLine() << "Section (" << SectionNumber << ") " << Name << " {\n";
    W.indent();

    printRelocations(&Sec);

    W.unindent();
    W.startLine() << "}\n";
  }
}

template<class ELFT>
void ELFDumper<ELFT>::printDynamicRelocations() {
  W.startLine() << "Dynamic Relocations {\n";
  W.indent();
  for (typename ELFO::Elf_Rela_Iter RelI = Obj->dyn_rela_begin(),
                                    RelE = Obj->dyn_rela_end();
       RelI != RelE; ++RelI) {
    SmallString<32> RelocName;
    Obj->getRelocationTypeName(RelI->getType(Obj->isMips64EL()), RelocName);
    StringRef SymbolName;
    uint32_t SymIndex = RelI->getSymbol(Obj->isMips64EL());
    const typename ELFO::Elf_Sym *Sym = Obj->dynamic_symbol_begin() + SymIndex;
    SymbolName = errorOrDefault(Obj->getSymbolName(Sym, true));
    if (opts::ExpandRelocs) {
      DictScope Group(W, "Relocation");
      W.printHex("Offset", RelI->r_offset);
      W.printNumber("Type", RelocName, (int)RelI->getType(Obj->isMips64EL()));
      W.printString("Symbol", SymbolName.size() > 0 ? SymbolName : "-");
      W.printHex("Addend", RelI->r_addend);
    }
    else {
      raw_ostream& OS = W.startLine();
      OS << W.hex(RelI->r_offset)
        << " " << RelocName
        << " " << (SymbolName.size() > 0 ? SymbolName : "-")
        << " " << W.hex(RelI->r_addend)
        << "\n";
    }
  }
  W.unindent();
  W.startLine() << "}\n";
}

template <class ELFT>
void ELFDumper<ELFT>::printRelocations(const Elf_Shdr *Sec) {
  switch (Sec->sh_type) {
  case ELF::SHT_REL:
    for (typename ELFO::Elf_Rel_Iter RI = Obj->rel_begin(Sec),
                                     RE = Obj->rel_end(Sec);
         RI != RE; ++RI) {
      typename ELFO::Elf_Rela Rela;
      Rela.r_offset = RI->r_offset;
      Rela.r_info = RI->r_info;
      Rela.r_addend = 0;
      printRelocation(Sec, Rela);
    }
    break;
  case ELF::SHT_RELA:
    for (typename ELFO::Elf_Rela_Iter RI = Obj->rela_begin(Sec),
                                      RE = Obj->rela_end(Sec);
         RI != RE; ++RI) {
      printRelocation(Sec, *RI);
    }
    break;
  }
}

template <class ELFT>
void ELFDumper<ELFT>::printRelocation(const Elf_Shdr *Sec,
                                      typename ELFO::Elf_Rela Rel) {
  SmallString<32> RelocName;
  Obj->getRelocationTypeName(Rel.getType(Obj->isMips64EL()), RelocName);
  StringRef TargetName;
  std::pair<const Elf_Shdr *, const Elf_Sym *> Sym =
      Obj->getRelocationSymbol(Sec, &Rel);
  if (Sym.second && Sym.second->getType() == ELF::STT_SECTION) {
    ErrorOr<const Elf_Shdr *> Sec = Obj->getSection(Sym.second);
    if (!error(Sec.getError())) {
      ErrorOr<StringRef> SecName = Obj->getSectionName(*Sec);
      if (SecName)
        TargetName = SecName.get();
    }
  } else if (Sym.first) {
    const Elf_Shdr *SymTable = Sym.first;
    ErrorOr<const Elf_Shdr *> StrTableSec = Obj->getSection(SymTable->sh_link);
    if (!error(StrTableSec.getError())) {
      ErrorOr<StringRef> StrTableOrErr = Obj->getStringTable(*StrTableSec);
      if (!error(StrTableOrErr.getError()))
        TargetName = errorOrDefault(Sym.second->getName(*StrTableOrErr));
    }
  }

  if (opts::ExpandRelocs) {
    DictScope Group(W, "Relocation");
    W.printHex("Offset", Rel.r_offset);
    W.printNumber("Type", RelocName, (int)Rel.getType(Obj->isMips64EL()));
    W.printNumber("Symbol", TargetName.size() > 0 ? TargetName : "-",
                  Rel.getSymbol(Obj->isMips64EL()));
    W.printHex("Addend", Rel.r_addend);
  } else {
    raw_ostream& OS = W.startLine();
    OS << W.hex(Rel.r_offset) << " " << RelocName << " "
       << (TargetName.size() > 0 ? TargetName : "-") << " "
       << W.hex(Rel.r_addend) << "\n";
  }
}

template<class ELFT>
void ELFDumper<ELFT>::printSymbols() {
  ListScope Group(W, "Symbols");
  for (const typename ELFO::Elf_Sym &Sym : Obj->symbols())
    printSymbol(&Sym, false);
}

template<class ELFT>
void ELFDumper<ELFT>::printDynamicSymbols() {
  ListScope Group(W, "DynamicSymbols");

  for (const typename ELFO::Elf_Sym &Sym : Obj->dynamic_symbols())
    printSymbol(&Sym, true);
}

template <class ELFT>
void ELFDumper<ELFT>::printSymbol(const typename ELFO::Elf_Sym *Symbol,
                                  bool IsDynamic) {
  unsigned SectionIndex = 0;
  StringRef SectionName;
  getSectionNameIndex(*Obj, Symbol, SectionName, SectionIndex);
  std::string FullSymbolName = getFullSymbolName(*Obj, Symbol, IsDynamic);

  DictScope D(W, "Symbol");
  W.printNumber("Name", FullSymbolName, Symbol->st_name);
  W.printHex   ("Value", Symbol->st_value);
  W.printNumber("Size", Symbol->st_size);
  W.printEnum  ("Binding", Symbol->getBinding(),
                  makeArrayRef(ElfSymbolBindings));
  W.printEnum  ("Type", Symbol->getType(), makeArrayRef(ElfSymbolTypes));
  W.printNumber("Other", Symbol->st_other);
  W.printHex("Section", SectionName, SectionIndex);
}

#define LLVM_READOBJ_TYPE_CASE(name) \
  case DT_##name: return #name

static const char *getTypeString(uint64_t Type) {
  switch (Type) {
  LLVM_READOBJ_TYPE_CASE(BIND_NOW);
  LLVM_READOBJ_TYPE_CASE(DEBUG);
  LLVM_READOBJ_TYPE_CASE(FINI);
  LLVM_READOBJ_TYPE_CASE(FINI_ARRAY);
  LLVM_READOBJ_TYPE_CASE(FINI_ARRAYSZ);
  LLVM_READOBJ_TYPE_CASE(FLAGS);
  LLVM_READOBJ_TYPE_CASE(FLAGS_1);
  LLVM_READOBJ_TYPE_CASE(HASH);
  LLVM_READOBJ_TYPE_CASE(INIT);
  LLVM_READOBJ_TYPE_CASE(INIT_ARRAY);
  LLVM_READOBJ_TYPE_CASE(INIT_ARRAYSZ);
  LLVM_READOBJ_TYPE_CASE(PREINIT_ARRAY);
  LLVM_READOBJ_TYPE_CASE(PREINIT_ARRAYSZ);
  LLVM_READOBJ_TYPE_CASE(JMPREL);
  LLVM_READOBJ_TYPE_CASE(NEEDED);
  LLVM_READOBJ_TYPE_CASE(NULL);
  LLVM_READOBJ_TYPE_CASE(PLTGOT);
  LLVM_READOBJ_TYPE_CASE(PLTREL);
  LLVM_READOBJ_TYPE_CASE(PLTRELSZ);
  LLVM_READOBJ_TYPE_CASE(REL);
  LLVM_READOBJ_TYPE_CASE(RELA);
  LLVM_READOBJ_TYPE_CASE(RELENT);
  LLVM_READOBJ_TYPE_CASE(RELSZ);
  LLVM_READOBJ_TYPE_CASE(RELAENT);
  LLVM_READOBJ_TYPE_CASE(RELASZ);
  LLVM_READOBJ_TYPE_CASE(RPATH);
  LLVM_READOBJ_TYPE_CASE(RUNPATH);
  LLVM_READOBJ_TYPE_CASE(SONAME);
  LLVM_READOBJ_TYPE_CASE(STRSZ);
  LLVM_READOBJ_TYPE_CASE(STRTAB);
  LLVM_READOBJ_TYPE_CASE(SYMBOLIC);
  LLVM_READOBJ_TYPE_CASE(SYMENT);
  LLVM_READOBJ_TYPE_CASE(SYMTAB);
  LLVM_READOBJ_TYPE_CASE(TEXTREL);
  LLVM_READOBJ_TYPE_CASE(VERNEED);
  LLVM_READOBJ_TYPE_CASE(VERNEEDNUM);
  LLVM_READOBJ_TYPE_CASE(VERSYM);
  LLVM_READOBJ_TYPE_CASE(RELCOUNT);
  LLVM_READOBJ_TYPE_CASE(GNU_HASH);
  LLVM_READOBJ_TYPE_CASE(MIPS_RLD_VERSION);
  LLVM_READOBJ_TYPE_CASE(MIPS_FLAGS);
  LLVM_READOBJ_TYPE_CASE(MIPS_BASE_ADDRESS);
  LLVM_READOBJ_TYPE_CASE(MIPS_LOCAL_GOTNO);
  LLVM_READOBJ_TYPE_CASE(MIPS_SYMTABNO);
  LLVM_READOBJ_TYPE_CASE(MIPS_UNREFEXTNO);
  LLVM_READOBJ_TYPE_CASE(MIPS_GOTSYM);
  LLVM_READOBJ_TYPE_CASE(MIPS_RLD_MAP);
  LLVM_READOBJ_TYPE_CASE(MIPS_PLTGOT);
  LLVM_READOBJ_TYPE_CASE(MIPS_OPTIONS);
  default: return "unknown";
  }
}

#undef LLVM_READOBJ_TYPE_CASE

#define LLVM_READOBJ_DT_FLAG_ENT(prefix, enum) \
  { #enum, prefix##_##enum }

static const EnumEntry<unsigned> ElfDynamicDTFlags[] = {
  LLVM_READOBJ_DT_FLAG_ENT(DF, ORIGIN),
  LLVM_READOBJ_DT_FLAG_ENT(DF, SYMBOLIC),
  LLVM_READOBJ_DT_FLAG_ENT(DF, TEXTREL),
  LLVM_READOBJ_DT_FLAG_ENT(DF, BIND_NOW),
  LLVM_READOBJ_DT_FLAG_ENT(DF, STATIC_TLS)
};

static const EnumEntry<unsigned> ElfDynamicDTFlags1[] = {
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, NOW),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, GLOBAL),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, GROUP),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, NODELETE),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, LOADFLTR),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, INITFIRST),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, NOOPEN),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, ORIGIN),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, DIRECT),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, TRANS),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, INTERPOSE),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, NODEFLIB),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, NODUMP),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, CONFALT),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, ENDFILTEE),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, DISPRELDNE),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, NODIRECT),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, IGNMULDEF),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, NOKSYMS),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, NOHDR),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, EDITED),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, NORELOC),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, SYMINTPOSE),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, GLOBAUDIT),
  LLVM_READOBJ_DT_FLAG_ENT(DF_1, SINGLETON)
};

static const EnumEntry<unsigned> ElfDynamicDTMipsFlags[] = {
  LLVM_READOBJ_DT_FLAG_ENT(RHF, NONE),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, QUICKSTART),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, NOTPOT),
  LLVM_READOBJ_DT_FLAG_ENT(RHS, NO_LIBRARY_REPLACEMENT),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, NO_MOVE),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, SGI_ONLY),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, GUARANTEE_INIT),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, DELTA_C_PLUS_PLUS),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, GUARANTEE_START_INIT),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, PIXIE),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, DEFAULT_DELAY_LOAD),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, REQUICKSTART),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, REQUICKSTARTED),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, CORD),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, NO_UNRES_UNDEF),
  LLVM_READOBJ_DT_FLAG_ENT(RHF, RLD_ORDER_SAFE)
};

#undef LLVM_READOBJ_DT_FLAG_ENT

template <typename T, typename TFlag>
void printFlags(T Value, ArrayRef<EnumEntry<TFlag>> Flags, raw_ostream &OS) {
  typedef EnumEntry<TFlag> FlagEntry;
  typedef SmallVector<FlagEntry, 10> FlagVector;
  FlagVector SetFlags;

  for (const auto &Flag : Flags) {
    if (Flag.Value == 0)
      continue;

    if ((Value & Flag.Value) == Flag.Value)
      SetFlags.push_back(Flag);
  }

  for (const auto &Flag : SetFlags) {
    OS << Flag.Name << " ";
  }
}

template <class ELFT>
static void printValue(const ELFFile<ELFT> *O, uint64_t Type, uint64_t Value,
                       bool Is64, raw_ostream &OS) {
  switch (Type) {
  case DT_PLTREL:
    if (Value == DT_REL) {
      OS << "REL";
      break;
    } else if (Value == DT_RELA) {
      OS << "RELA";
      break;
    }
  // Fallthrough.
  case DT_PLTGOT:
  case DT_HASH:
  case DT_STRTAB:
  case DT_SYMTAB:
  case DT_RELA:
  case DT_INIT:
  case DT_FINI:
  case DT_REL:
  case DT_JMPREL:
  case DT_INIT_ARRAY:
  case DT_FINI_ARRAY:
  case DT_PREINIT_ARRAY:
  case DT_DEBUG:
  case DT_VERNEED:
  case DT_VERSYM:
  case DT_GNU_HASH:
  case DT_NULL:
  case DT_MIPS_BASE_ADDRESS:
  case DT_MIPS_GOTSYM:
  case DT_MIPS_RLD_MAP:
  case DT_MIPS_PLTGOT:
  case DT_MIPS_OPTIONS:
    OS << format("0x%" PRIX64, Value);
    break;
  case DT_RELCOUNT:
  case DT_VERNEEDNUM:
  case DT_MIPS_RLD_VERSION:
  case DT_MIPS_LOCAL_GOTNO:
  case DT_MIPS_SYMTABNO:
  case DT_MIPS_UNREFEXTNO:
    OS << Value;
    break;
  case DT_PLTRELSZ:
  case DT_RELASZ:
  case DT_RELAENT:
  case DT_STRSZ:
  case DT_SYMENT:
  case DT_RELSZ:
  case DT_RELENT:
  case DT_INIT_ARRAYSZ:
  case DT_FINI_ARRAYSZ:
  case DT_PREINIT_ARRAYSZ:
    OS << Value << " (bytes)";
    break;
  case DT_NEEDED:
    OS << "SharedLibrary (" << O->getDynamicString(Value) << ")";
    break;
  case DT_SONAME:
    OS << "LibrarySoname (" << O->getDynamicString(Value) << ")";
    break;
  case DT_RPATH:
  case DT_RUNPATH:
    OS << O->getDynamicString(Value);
    break;
  case DT_MIPS_FLAGS:
    printFlags(Value, makeArrayRef(ElfDynamicDTMipsFlags), OS);
    break;
  case DT_FLAGS:
    printFlags(Value, makeArrayRef(ElfDynamicDTFlags), OS);
    break;
  case DT_FLAGS_1:
    printFlags(Value, makeArrayRef(ElfDynamicDTFlags1), OS);
    break;
  default:
    OS << format("0x%" PRIX64, Value);
    break;
  }
}

template<class ELFT>
void ELFDumper<ELFT>::printUnwindInfo() {
  W.startLine() << "UnwindInfo not implemented.\n";
}

namespace {
template <> void ELFDumper<ELFType<support::little, false>>::printUnwindInfo() {
  const unsigned Machine = Obj->getHeader()->e_machine;
  if (Machine == EM_ARM) {
    ARM::EHABI::PrinterContext<ELFType<support::little, false>> Ctx(W, Obj);
    return Ctx.PrintUnwindInformation();
  }
  W.startLine() << "UnwindInfo not implemented.\n";
}
}

template<class ELFT>
void ELFDumper<ELFT>::printDynamicTable() {
  auto DynTable = Obj->dynamic_table(true);

  ptrdiff_t Total = std::distance(DynTable.begin(), DynTable.end());
  if (Total == 0)
    return;

  raw_ostream &OS = W.getOStream();
  W.startLine() << "DynamicSection [ (" << Total << " entries)\n";

  bool Is64 = ELFT::Is64Bits;

  W.startLine()
     << "  Tag" << (Is64 ? "                " : "        ") << "Type"
     << "                 " << "Name/Value\n";
  for (const auto &Entry : DynTable) {
    W.startLine()
       << "  "
       << format(Is64 ? "0x%016" PRIX64 : "0x%08" PRIX64, Entry.getTag())
       << " " << format("%-21s", getTypeString(Entry.getTag()));
    printValue(Obj, Entry.getTag(), Entry.getVal(), Is64, OS);
    OS << "\n";
  }

  W.startLine() << "]\n";
}

template<class ELFT>
void ELFDumper<ELFT>::printNeededLibraries() {
  ListScope D(W, "NeededLibraries");

  typedef std::vector<StringRef> LibsTy;
  LibsTy Libs;

  for (const auto &Entry : Obj->dynamic_table())
    if (Entry.d_tag == ELF::DT_NEEDED)
      Libs.push_back(Obj->getDynamicString(Entry.d_un.d_val));

  std::stable_sort(Libs.begin(), Libs.end());

  for (LibsTy::const_iterator I = Libs.begin(), E = Libs.end(); I != E; ++I) {
    outs() << "  " << *I << "\n";
  }
}

template<class ELFT>
void ELFDumper<ELFT>::printProgramHeaders() {
  ListScope L(W, "ProgramHeaders");

  for (typename ELFO::Elf_Phdr_Iter PI = Obj->program_header_begin(),
                                    PE = Obj->program_header_end();
       PI != PE; ++PI) {
    DictScope P(W, "ProgramHeader");
    W.printHex   ("Type",
                  getElfSegmentType(Obj->getHeader()->e_machine, PI->p_type),
                  PI->p_type);
    W.printHex   ("Offset", PI->p_offset);
    W.printHex   ("VirtualAddress", PI->p_vaddr);
    W.printHex   ("PhysicalAddress", PI->p_paddr);
    W.printNumber("FileSize", PI->p_filesz);
    W.printNumber("MemSize", PI->p_memsz);
    W.printFlags ("Flags", PI->p_flags, makeArrayRef(ElfSegmentFlags));
    W.printNumber("Alignment", PI->p_align);
  }
}

template <typename ELFT>
void ELFDumper<ELFT>::printHashTable() {
  DictScope D(W, "HashTable");
  auto HT = Obj->getHashTable();
  if (!HT)
    return;
  W.printNumber("Num Buckets", HT->nbucket);
  W.printNumber("Num Chains", HT->nchain);
  W.printList("Buckets", HT->buckets());
  W.printList("Chains", HT->chains());
}

template <class ELFT>
void ELFDumper<ELFT>::printAttributes() {
  W.startLine() << "Attributes not implemented.\n";
}

namespace {
template <> void ELFDumper<ELFType<support::little, false>>::printAttributes() {
  if (Obj->getHeader()->e_machine != EM_ARM) {
    W.startLine() << "Attributes not implemented.\n";
    return;
  }

  DictScope BA(W, "BuildAttributes");
  for (const ELFO::Elf_Shdr &Sec : Obj->sections()) {
    if (Sec.sh_type != ELF::SHT_ARM_ATTRIBUTES)
      continue;

    ErrorOr<ArrayRef<uint8_t>> Contents = Obj->getSectionContents(&Sec);
    if (!Contents)
      continue;

    if ((*Contents)[0] != ARMBuildAttrs::Format_Version) {
      errs() << "unrecognised FormatVersion: 0x" << utohexstr((*Contents)[0])
             << '\n';
      continue;
    }

    W.printHex("FormatVersion", (*Contents)[0]);
    if (Contents->size() == 1)
      continue;

    ARMAttributeParser(W).Parse(*Contents);
  }
}
}

namespace {
template <class ELFT> class MipsGOTParser {
public:
  typedef object::ELFFile<ELFT> ObjectFile;
  typedef typename ObjectFile::Elf_Shdr Elf_Shdr;
  typedef typename ObjectFile::Elf_Sym Elf_Sym;

  MipsGOTParser(const ObjectFile *Obj, StreamWriter &W);

  void parseGOT();
  void parsePLT();

private:
  typedef typename ObjectFile::Elf_Addr GOTEntry;
  typedef typename ObjectFile::template ELFEntityIterator<const GOTEntry>
  GOTIter;

  const ObjectFile *Obj;
  StreamWriter &W;
  llvm::Optional<uint64_t> DtPltGot;
  llvm::Optional<uint64_t> DtLocalGotNum;
  llvm::Optional<uint64_t> DtGotSym;
  llvm::Optional<uint64_t> DtMipsPltGot;
  llvm::Optional<uint64_t> DtJmpRel;

  std::size_t getGOTTotal(ArrayRef<uint8_t> GOT) const;
  GOTIter makeGOTIter(ArrayRef<uint8_t> GOT, std::size_t EntryNum);

  void printGotEntry(uint64_t GotAddr, GOTIter BeginIt, GOTIter It);
  void printGlobalGotEntry(uint64_t GotAddr, GOTIter BeginIt, GOTIter It,
                           const Elf_Sym *Sym, bool IsDynamic);
  void printPLTEntry(uint64_t PLTAddr, GOTIter BeginIt, GOTIter It,
                     StringRef Purpose);
  void printPLTEntry(uint64_t PLTAddr, GOTIter BeginIt, GOTIter It,
                     const Elf_Sym *Sym);
};
}

template <class ELFT>
MipsGOTParser<ELFT>::MipsGOTParser(const ObjectFile *Obj, StreamWriter &W)
    : Obj(Obj), W(W) {
  for (const auto &Entry : Obj->dynamic_table()) {
    switch (Entry.getTag()) {
    case ELF::DT_PLTGOT:
      DtPltGot = Entry.getVal();
      break;
    case ELF::DT_MIPS_LOCAL_GOTNO:
      DtLocalGotNum = Entry.getVal();
      break;
    case ELF::DT_MIPS_GOTSYM:
      DtGotSym = Entry.getVal();
      break;
    case ELF::DT_MIPS_PLTGOT:
      DtMipsPltGot = Entry.getVal();
      break;
    case ELF::DT_JMPREL:
      DtJmpRel = Entry.getVal();
      break;
    }
  }
}

template <class ELFT> void MipsGOTParser<ELFT>::parseGOT() {
  // See "Global Offset Table" in Chapter 5 in the following document
  // for detailed GOT description.
  // ftp://www.linux-mips.org/pub/linux/mips/doc/ABI/mipsabi.pdf
  if (!DtPltGot) {
    W.startLine() << "Cannot find PLTGOT dynamic table tag.\n";
    return;
  }
  if (!DtLocalGotNum) {
    W.startLine() << "Cannot find MIPS_LOCAL_GOTNO dynamic table tag.\n";
    return;
  }
  if (!DtGotSym) {
    W.startLine() << "Cannot find MIPS_GOTSYM dynamic table tag.\n";
    return;
  }

  const Elf_Shdr *GOTShdr = findSectionByAddress(Obj, *DtPltGot);
  if (!GOTShdr) {
    W.startLine() << "There is no .got section in the file.\n";
    return;
  }

  ErrorOr<ArrayRef<uint8_t>> GOT = Obj->getSectionContents(GOTShdr);
  if (!GOT) {
    W.startLine() << "The .got section is empty.\n";
    return;
  }

  if (*DtLocalGotNum > getGOTTotal(*GOT)) {
    W.startLine() << "MIPS_LOCAL_GOTNO exceeds a number of GOT entries.\n";
    return;
  }

  const Elf_Sym *DynSymBegin = Obj->dynamic_symbol_begin();
  const Elf_Sym *DynSymEnd = Obj->dynamic_symbol_end();
  std::size_t DynSymTotal = std::size_t(std::distance(DynSymBegin, DynSymEnd));

  if (*DtGotSym > DynSymTotal) {
    W.startLine() << "MIPS_GOTSYM exceeds a number of dynamic symbols.\n";
    return;
  }

  std::size_t GlobalGotNum = DynSymTotal - *DtGotSym;

  if (*DtLocalGotNum + GlobalGotNum > getGOTTotal(*GOT)) {
    W.startLine() << "Number of global GOT entries exceeds the size of GOT.\n";
    return;
  }

  GOTIter GotBegin = makeGOTIter(*GOT, 0);
  GOTIter GotLocalEnd = makeGOTIter(*GOT, *DtLocalGotNum);
  GOTIter It = GotBegin;

  DictScope GS(W, "Primary GOT");

  W.printHex("Canonical gp value", GOTShdr->sh_addr + 0x7ff0);
  {
    ListScope RS(W, "Reserved entries");

    {
      DictScope D(W, "Entry");
      printGotEntry(GOTShdr->sh_addr, GotBegin, It++);
      W.printString("Purpose", StringRef("Lazy resolver"));
    }

    if (It != GotLocalEnd && (*It >> (sizeof(GOTEntry) * 8 - 1)) != 0) {
      DictScope D(W, "Entry");
      printGotEntry(GOTShdr->sh_addr, GotBegin, It++);
      W.printString("Purpose", StringRef("Module pointer (GNU extension)"));
    }
  }
  {
    ListScope LS(W, "Local entries");
    for (; It != GotLocalEnd; ++It) {
      DictScope D(W, "Entry");
      printGotEntry(GOTShdr->sh_addr, GotBegin, It);
    }
  }
  {
    ListScope GS(W, "Global entries");

    GOTIter GotGlobalEnd = makeGOTIter(*GOT, *DtLocalGotNum + GlobalGotNum);
    const Elf_Sym *GotDynSym = DynSymBegin + *DtGotSym;
    for (; It != GotGlobalEnd; ++It) {
      DictScope D(W, "Entry");
      printGlobalGotEntry(GOTShdr->sh_addr, GotBegin, It, GotDynSym++, true);
    }
  }

  std::size_t SpecGotNum = getGOTTotal(*GOT) - *DtLocalGotNum - GlobalGotNum;
  W.printNumber("Number of TLS and multi-GOT entries", uint64_t(SpecGotNum));
}

template <class ELFT> void MipsGOTParser<ELFT>::parsePLT() {
  if (!DtMipsPltGot) {
    W.startLine() << "Cannot find MIPS_PLTGOT dynamic table tag.\n";
    return;
  }
  if (!DtJmpRel) {
    W.startLine() << "Cannot find JMPREL dynamic table tag.\n";
    return;
  }

  const Elf_Shdr *PLTShdr = findSectionByAddress(Obj, *DtMipsPltGot);
  if (!PLTShdr) {
    W.startLine() << "There is no .got.plt section in the file.\n";
    return;
  }
  ErrorOr<ArrayRef<uint8_t>> PLT = Obj->getSectionContents(PLTShdr);
  if (!PLT) {
    W.startLine() << "The .got.plt section is empty.\n";
    return;
  }

  const Elf_Shdr *PLTRelShdr = findSectionByAddress(Obj, *DtJmpRel);
  if (!PLTShdr) {
    W.startLine() << "There is no .rel.plt section in the file.\n";
    return;
  }

  GOTIter PLTBegin = makeGOTIter(*PLT, 0);
  GOTIter PLTEnd = makeGOTIter(*PLT, getGOTTotal(*PLT));
  GOTIter It = PLTBegin;

  DictScope GS(W, "PLT GOT");
  {
    ListScope RS(W, "Reserved entries");
    printPLTEntry(PLTShdr->sh_addr, PLTBegin, It++, "PLT lazy resolver");
    if (It != PLTEnd)
      printPLTEntry(PLTShdr->sh_addr, PLTBegin, It++, "Module pointer");
  }
  {
    ListScope GS(W, "Entries");

    switch (PLTRelShdr->sh_type) {
    case ELF::SHT_REL:
      for (typename ObjectFile::Elf_Rel_Iter RI = Obj->rel_begin(PLTRelShdr),
                                             RE = Obj->rel_end(PLTRelShdr);
           RI != RE && It != PLTEnd; ++RI, ++It) {
        const Elf_Sym *Sym =
            Obj->getRelocationSymbol(&*PLTRelShdr, &*RI).second;
        printPLTEntry(PLTShdr->sh_addr, PLTBegin, It, Sym);
      }
      break;
    case ELF::SHT_RELA:
      for (typename ObjectFile::Elf_Rela_Iter RI = Obj->rela_begin(PLTRelShdr),
                                              RE = Obj->rela_end(PLTRelShdr);
           RI != RE && It != PLTEnd; ++RI, ++It) {
        const Elf_Sym *Sym =
            Obj->getRelocationSymbol(&*PLTRelShdr, &*RI).second;
        printPLTEntry(PLTShdr->sh_addr, PLTBegin, It, Sym);
      }
      break;
    }
  }
}

template <class ELFT>
std::size_t MipsGOTParser<ELFT>::getGOTTotal(ArrayRef<uint8_t> GOT) const {
  return GOT.size() / sizeof(GOTEntry);
}

template <class ELFT>
typename MipsGOTParser<ELFT>::GOTIter
MipsGOTParser<ELFT>::makeGOTIter(ArrayRef<uint8_t> GOT, std::size_t EntryNum) {
  const char *Data = reinterpret_cast<const char *>(GOT.data());
  return GOTIter(sizeof(GOTEntry), Data + EntryNum * sizeof(GOTEntry));
}

template <class ELFT>
void MipsGOTParser<ELFT>::printGotEntry(uint64_t GotAddr, GOTIter BeginIt,
                                        GOTIter It) {
  int64_t Offset = std::distance(BeginIt, It) * sizeof(GOTEntry);
  W.printHex("Address", GotAddr + Offset);
  W.printNumber("Access", Offset - 0x7ff0);
  W.printHex("Initial", *It);
}

template <class ELFT>
void MipsGOTParser<ELFT>::printGlobalGotEntry(uint64_t GotAddr, GOTIter BeginIt,
                                              GOTIter It, const Elf_Sym *Sym,
                                              bool IsDynamic) {
  printGotEntry(GotAddr, BeginIt, It);

  W.printHex("Value", Sym->st_value);
  W.printEnum("Type", Sym->getType(), makeArrayRef(ElfSymbolTypes));

  unsigned SectionIndex = 0;
  StringRef SectionName;
  getSectionNameIndex(*Obj, Sym, SectionName, SectionIndex);
  W.printHex("Section", SectionName, SectionIndex);

  std::string FullSymbolName = getFullSymbolName(*Obj, Sym, IsDynamic);
  W.printNumber("Name", FullSymbolName, Sym->st_name);
}

template <class ELFT>
void MipsGOTParser<ELFT>::printPLTEntry(uint64_t PLTAddr, GOTIter BeginIt,
                                        GOTIter It, StringRef Purpose) {
  DictScope D(W, "Entry");
  int64_t Offset = std::distance(BeginIt, It) * sizeof(GOTEntry);
  W.printHex("Address", PLTAddr + Offset);
  W.printHex("Initial", *It);
  W.printString("Purpose", Purpose);
}

template <class ELFT>
void MipsGOTParser<ELFT>::printPLTEntry(uint64_t PLTAddr, GOTIter BeginIt,
                                        GOTIter It, const Elf_Sym *Sym) {
  DictScope D(W, "Entry");
  int64_t Offset = std::distance(BeginIt, It) * sizeof(GOTEntry);
  W.printHex("Address", PLTAddr + Offset);
  W.printHex("Initial", *It);
  W.printHex("Value", Sym->st_value);
  W.printEnum("Type", Sym->getType(), makeArrayRef(ElfSymbolTypes));

  unsigned SectionIndex = 0;
  StringRef SectionName;
  getSectionNameIndex(*Obj, Sym, SectionName, SectionIndex);
  W.printHex("Section", SectionName, SectionIndex);

  std::string FullSymbolName = getFullSymbolName(*Obj, Sym, true);
  W.printNumber("Name", FullSymbolName, Sym->st_name);
}

template <class ELFT> void ELFDumper<ELFT>::printMipsPLTGOT() {
  if (Obj->getHeader()->e_machine != EM_MIPS) {
    W.startLine() << "MIPS PLT GOT is available for MIPS targets only.\n";
    return;
  }

  MipsGOTParser<ELFT> GOTParser(Obj, W);
  GOTParser.parseGOT();
  GOTParser.parsePLT();
}

static const EnumEntry<unsigned> ElfMipsISAExtType[] = {
  {"None",                    Mips::AFL_EXT_NONE},
  {"Broadcom SB-1",           Mips::AFL_EXT_SB1},
  {"Cavium Networks Octeon",  Mips::AFL_EXT_OCTEON},
  {"Cavium Networks Octeon2", Mips::AFL_EXT_OCTEON2},
  {"Cavium Networks OcteonP", Mips::AFL_EXT_OCTEONP},
  {"Cavium Networks Octeon3", Mips::AFL_EXT_OCTEON3},
  {"LSI R4010",               Mips::AFL_EXT_4010},
  {"Loongson 2E",             Mips::AFL_EXT_LOONGSON_2E},
  {"Loongson 2F",             Mips::AFL_EXT_LOONGSON_2F},
  {"Loongson 3A",             Mips::AFL_EXT_LOONGSON_3A},
  {"MIPS R4650",              Mips::AFL_EXT_4650},
  {"MIPS R5900",              Mips::AFL_EXT_5900},
  {"MIPS R10000",             Mips::AFL_EXT_10000},
  {"NEC VR4100",              Mips::AFL_EXT_4100},
  {"NEC VR4111/VR4181",       Mips::AFL_EXT_4111},
  {"NEC VR4120",              Mips::AFL_EXT_4120},
  {"NEC VR5400",              Mips::AFL_EXT_5400},
  {"NEC VR5500",              Mips::AFL_EXT_5500},
  {"RMI Xlr",                 Mips::AFL_EXT_XLR},
  {"Toshiba R3900",           Mips::AFL_EXT_3900}
};

static const EnumEntry<unsigned> ElfMipsASEFlags[] = {
  {"DSP",                Mips::AFL_ASE_DSP},
  {"DSPR2",              Mips::AFL_ASE_DSPR2},
  {"Enhanced VA Scheme", Mips::AFL_ASE_EVA},
  {"MCU",                Mips::AFL_ASE_MCU},
  {"MDMX",               Mips::AFL_ASE_MDMX},
  {"MIPS-3D",            Mips::AFL_ASE_MIPS3D},
  {"MT",                 Mips::AFL_ASE_MT},
  {"SmartMIPS",          Mips::AFL_ASE_SMARTMIPS},
  {"VZ",                 Mips::AFL_ASE_VIRT},
  {"MSA",                Mips::AFL_ASE_MSA},
  {"MIPS16",             Mips::AFL_ASE_MIPS16},
  {"microMIPS",          Mips::AFL_ASE_MICROMIPS},
  {"XPA",                Mips::AFL_ASE_XPA}
};

static const EnumEntry<unsigned> ElfMipsFpABIType[] = {
  {"Hard or soft float",                  Mips::Val_GNU_MIPS_ABI_FP_ANY},
  {"Hard float (double precision)",       Mips::Val_GNU_MIPS_ABI_FP_DOUBLE},
  {"Hard float (single precision)",       Mips::Val_GNU_MIPS_ABI_FP_SINGLE},
  {"Soft float",                          Mips::Val_GNU_MIPS_ABI_FP_SOFT},
  {"Hard float (MIPS32r2 64-bit FPU 12 callee-saved)",
   Mips::Val_GNU_MIPS_ABI_FP_OLD_64},
  {"Hard float (32-bit CPU, Any FPU)",    Mips::Val_GNU_MIPS_ABI_FP_XX},
  {"Hard float (32-bit CPU, 64-bit FPU)", Mips::Val_GNU_MIPS_ABI_FP_64},
  {"Hard float compat (32-bit CPU, 64-bit FPU)",
   Mips::Val_GNU_MIPS_ABI_FP_64A}
};

static const EnumEntry<unsigned> ElfMipsFlags1[] {
  {"ODDSPREG", Mips::AFL_FLAGS1_ODDSPREG},
};

static int getMipsRegisterSize(uint8_t Flag) {
  switch (Flag) {
  case Mips::AFL_REG_NONE:
    return 0;
  case Mips::AFL_REG_32:
    return 32;
  case Mips::AFL_REG_64:
    return 64;
  case Mips::AFL_REG_128:
    return 128;
  default:
    return -1;
  }
}

template <class ELFT> void ELFDumper<ELFT>::printMipsABIFlags() {
  const Elf_Shdr *Shdr = findSectionByName(*Obj, ".MIPS.abiflags");
  if (!Shdr) {
    W.startLine() << "There is no .MIPS.abiflags section in the file.\n";
    return;
  }
  ErrorOr<ArrayRef<uint8_t>> Sec = Obj->getSectionContents(Shdr);
  if (!Sec) {
    W.startLine() << "The .MIPS.abiflags section is empty.\n";
    return;
  }
  if (Sec->size() != sizeof(Elf_Mips_ABIFlags<ELFT>)) {
    W.startLine() << "The .MIPS.abiflags section has a wrong size.\n";
    return;
  }

  auto *Flags = reinterpret_cast<const Elf_Mips_ABIFlags<ELFT> *>(Sec->data());

  raw_ostream &OS = W.getOStream();
  DictScope GS(W, "MIPS ABI Flags");

  W.printNumber("Version", Flags->version);
  W.startLine() << "ISA: ";
  if (Flags->isa_rev <= 1)
    OS << format("MIPS%u", Flags->isa_level);
  else
    OS << format("MIPS%ur%u", Flags->isa_level, Flags->isa_rev);
  OS << "\n";
  W.printEnum("ISA Extension", Flags->isa_ext, makeArrayRef(ElfMipsISAExtType));
  W.printFlags("ASEs", Flags->ases, makeArrayRef(ElfMipsASEFlags));
  W.printEnum("FP ABI", Flags->fp_abi, makeArrayRef(ElfMipsFpABIType));
  W.printNumber("GPR size", getMipsRegisterSize(Flags->gpr_size));
  W.printNumber("CPR1 size", getMipsRegisterSize(Flags->cpr1_size));
  W.printNumber("CPR2 size", getMipsRegisterSize(Flags->cpr2_size));
  W.printFlags("Flags 1", Flags->flags1, makeArrayRef(ElfMipsFlags1));
  W.printHex("Flags 2", Flags->flags2);
}

template <class ELFT> void ELFDumper<ELFT>::printMipsReginfo() {
  const Elf_Shdr *Shdr = findSectionByName(*Obj, ".reginfo");
  if (!Shdr) {
    W.startLine() << "There is no .reginfo section in the file.\n";
    return;
  }
  ErrorOr<ArrayRef<uint8_t>> Sec = Obj->getSectionContents(Shdr);
  if (!Sec) {
    W.startLine() << "The .reginfo section is empty.\n";
    return;
  }
  if (Sec->size() != sizeof(Elf_Mips_RegInfo<ELFT>)) {
    W.startLine() << "The .reginfo section has a wrong size.\n";
    return;
  }

  auto *Reginfo = reinterpret_cast<const Elf_Mips_RegInfo<ELFT> *>(Sec->data());

  DictScope GS(W, "MIPS RegInfo");
  W.printHex("GP", Reginfo->ri_gp_value);
  W.printHex("General Mask", Reginfo->ri_gprmask);
  W.printHex("Co-Proc Mask0", Reginfo->ri_cprmask[0]);
  W.printHex("Co-Proc Mask1", Reginfo->ri_cprmask[1]);
  W.printHex("Co-Proc Mask2", Reginfo->ri_cprmask[2]);
  W.printHex("Co-Proc Mask3", Reginfo->ri_cprmask[3]);
}

template <class ELFT> void ELFDumper<ELFT>::printStackMap() const {
  const typename ELFFile<ELFT>::Elf_Shdr *StackMapSection = nullptr;
  for (const auto &Sec : Obj->sections()) {
    ErrorOr<StringRef> Name = Obj->getSectionName(&Sec);
    if (*Name == ".llvm_stackmaps") {
      StackMapSection = &Sec;
      break;
    }
  }

  if (!StackMapSection)
    return;

  StringRef StackMapContents;
  ErrorOr<ArrayRef<uint8_t>> StackMapContentsArray =
    Obj->getSectionContents(StackMapSection);

  prettyPrintStackMap(
              llvm::outs(),
              StackMapV1Parser<ELFT::TargetEndianness>(*StackMapContentsArray));
}
