//===-- LLVMSymbolize.cpp -------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation for LLVM symbolization library.
//
//===----------------------------------------------------------------------===//

#include "LLVMSymbolize.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Config/config.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/PDB/PDB.h"
#include "llvm/DebugInfo/PDB/PDBContext.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/MachO.h"
#include "llvm/Object/SymbolSize.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compression.h"
#include "llvm/Support/DataExtractor.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include <sstream>
#include <stdlib.h>

#if defined(_MSC_VER)
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

namespace llvm {
namespace symbolize {

static bool error(std::error_code ec) {
  if (!ec)
    return false;
  errs() << "LLVMSymbolizer: error reading file: " << ec.message() << ".\n";
  return true;
}

static DILineInfoSpecifier
getDILineInfoSpecifier(const LLVMSymbolizer::Options &Opts) {
  return DILineInfoSpecifier(
      DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath,
      Opts.PrintFunctions);
}

ModuleInfo::ModuleInfo(ObjectFile *Obj, DIContext *DICtx)
    : Module(Obj), DebugInfoContext(DICtx) {
  std::unique_ptr<DataExtractor> OpdExtractor;
  uint64_t OpdAddress = 0;
  // Find the .opd (function descriptor) section if any, for big-endian
  // PowerPC64 ELF.
  if (Module->getArch() == Triple::ppc64) {
    for (section_iterator Section : Module->sections()) {
      StringRef Name;
      if (!error(Section->getName(Name)) && Name == ".opd") {
        StringRef Data;
        if (!error(Section->getContents(Data))) {
          OpdExtractor.reset(new DataExtractor(Data, Module->isLittleEndian(),
                                               Module->getBytesInAddress()));
          OpdAddress = Section->getAddress();
        }
        break;
      }
    }
  }
  std::vector<std::pair<SymbolRef, uint64_t>> Symbols =
      computeSymbolSizes(*Module);
  for (auto &P : Symbols)
    addSymbol(P.first, P.second, OpdExtractor.get(), OpdAddress);
}

void ModuleInfo::addSymbol(const SymbolRef &Symbol, uint64_t SymbolSize,
                           DataExtractor *OpdExtractor, uint64_t OpdAddress) {
  SymbolRef::Type SymbolType = Symbol.getType();
  if (SymbolType != SymbolRef::ST_Function && SymbolType != SymbolRef::ST_Data)
    return;
  ErrorOr<uint64_t> SymbolAddressOrErr = Symbol.getAddress();
  if (error(SymbolAddressOrErr.getError()))
    return;
  uint64_t SymbolAddress = *SymbolAddressOrErr;
  if (OpdExtractor) {
    // For big-endian PowerPC64 ELF, symbols in the .opd section refer to
    // function descriptors. The first word of the descriptor is a pointer to
    // the function's code.
    // For the purposes of symbolization, pretend the symbol's address is that
    // of the function's code, not the descriptor.
    uint64_t OpdOffset = SymbolAddress - OpdAddress;
    uint32_t OpdOffset32 = OpdOffset;
    if (OpdOffset == OpdOffset32 && 
        OpdExtractor->isValidOffsetForAddress(OpdOffset32))
      SymbolAddress = OpdExtractor->getAddress(&OpdOffset32);
  }
  ErrorOr<StringRef> SymbolNameOrErr = Symbol.getName();
  if (error(SymbolNameOrErr.getError()))
    return;
  StringRef SymbolName = *SymbolNameOrErr;
  // Mach-O symbol table names have leading underscore, skip it.
  if (Module->isMachO() && SymbolName.size() > 0 && SymbolName[0] == '_')
    SymbolName = SymbolName.drop_front();
  // FIXME: If a function has alias, there are two entries in symbol table
  // with same address size. Make sure we choose the correct one.
  auto &M = SymbolType == SymbolRef::ST_Function ? Functions : Objects;
  SymbolDesc SD = { SymbolAddress, SymbolSize };
  M.insert(std::make_pair(SD, SymbolName));
}

bool ModuleInfo::getNameFromSymbolTable(SymbolRef::Type Type, uint64_t Address,
                                        std::string &Name, uint64_t &Addr,
                                        uint64_t &Size) const {
  const auto &SymbolMap = Type == SymbolRef::ST_Function ? Functions : Objects;
  if (SymbolMap.empty())
    return false;
  SymbolDesc SD = { Address, Address };
  auto SymbolIterator = SymbolMap.upper_bound(SD);
  if (SymbolIterator == SymbolMap.begin())
    return false;
  --SymbolIterator;
  if (SymbolIterator->first.Size != 0 &&
      SymbolIterator->first.Addr + SymbolIterator->first.Size <= Address)
    return false;
  Name = SymbolIterator->second.str();
  Addr = SymbolIterator->first.Addr;
  Size = SymbolIterator->first.Size;
  return true;
}

DILineInfo ModuleInfo::symbolizeCode(
    uint64_t ModuleOffset, const LLVMSymbolizer::Options &Opts) const {
  DILineInfo LineInfo;
  if (DebugInfoContext) {
    LineInfo = DebugInfoContext->getLineInfoForAddress(
        ModuleOffset, getDILineInfoSpecifier(Opts));
  }
  // Override function name from symbol table if necessary.
  if (Opts.PrintFunctions != FunctionNameKind::None && Opts.UseSymbolTable) {
    std::string FunctionName;
    uint64_t Start, Size;
    if (getNameFromSymbolTable(SymbolRef::ST_Function, ModuleOffset,
                               FunctionName, Start, Size)) {
      LineInfo.FunctionName = FunctionName;
    }
  }
  return LineInfo;
}

DIInliningInfo ModuleInfo::symbolizeInlinedCode(
    uint64_t ModuleOffset, const LLVMSymbolizer::Options &Opts) const {
  DIInliningInfo InlinedContext;

  if (DebugInfoContext) {
    InlinedContext = DebugInfoContext->getInliningInfoForAddress(
        ModuleOffset, getDILineInfoSpecifier(Opts));
  }
  // Make sure there is at least one frame in context.
  if (InlinedContext.getNumberOfFrames() == 0) {
    InlinedContext.addFrame(DILineInfo());
  }
  // Override the function name in lower frame with name from symbol table.
  if (Opts.PrintFunctions != FunctionNameKind::None && Opts.UseSymbolTable) {
    DIInliningInfo PatchedInlinedContext;
    for (uint32_t i = 0, n = InlinedContext.getNumberOfFrames(); i < n; i++) {
      DILineInfo LineInfo = InlinedContext.getFrame(i);
      if (i == n - 1) {
        std::string FunctionName;
        uint64_t Start, Size;
        if (getNameFromSymbolTable(SymbolRef::ST_Function, ModuleOffset,
                                   FunctionName, Start, Size)) {
          LineInfo.FunctionName = FunctionName;
        }
      }
      PatchedInlinedContext.addFrame(LineInfo);
    }
    InlinedContext = PatchedInlinedContext;
  }
  return InlinedContext;
}

bool ModuleInfo::symbolizeData(uint64_t ModuleOffset, std::string &Name,
                               uint64_t &Start, uint64_t &Size) const {
  return getNameFromSymbolTable(SymbolRef::ST_Data, ModuleOffset, Name, Start,
                                Size);
}

const char LLVMSymbolizer::kBadString[] = "??";

std::string LLVMSymbolizer::symbolizeCode(const std::string &ModuleName,
                                          uint64_t ModuleOffset) {
  ModuleInfo *Info = getOrCreateModuleInfo(ModuleName);
  if (!Info)
    return printDILineInfo(DILineInfo());
  if (Opts.PrintInlining) {
    DIInliningInfo InlinedContext =
        Info->symbolizeInlinedCode(ModuleOffset, Opts);
    uint32_t FramesNum = InlinedContext.getNumberOfFrames();
    assert(FramesNum > 0);
    std::string Result;
    for (uint32_t i = 0; i < FramesNum; i++) {
      DILineInfo LineInfo = InlinedContext.getFrame(i);
      Result += printDILineInfo(LineInfo);
    }
    return Result;
  }
  DILineInfo LineInfo = Info->symbolizeCode(ModuleOffset, Opts);
  return printDILineInfo(LineInfo);
}

std::string LLVMSymbolizer::symbolizeData(const std::string &ModuleName,
                                          uint64_t ModuleOffset) {
  std::string Name = kBadString;
  uint64_t Start = 0;
  uint64_t Size = 0;
  if (Opts.UseSymbolTable) {
    if (ModuleInfo *Info = getOrCreateModuleInfo(ModuleName)) {
      if (Info->symbolizeData(ModuleOffset, Name, Start, Size) && Opts.Demangle)
        Name = DemangleName(Name);
    }
  }
  std::stringstream ss;
  ss << Name << "\n" << Start << " " << Size << "\n";
  return ss.str();
}

void LLVMSymbolizer::flush() {
  DeleteContainerSeconds(Modules);
  ObjectPairForPathArch.clear();
  ObjectFileForArch.clear();
}

// For Path="/path/to/foo" and Basename="foo" assume that debug info is in
// /path/to/foo.dSYM/Contents/Resources/DWARF/foo.
// For Path="/path/to/bar.dSYM" and Basename="foo" assume that debug info is in
// /path/to/bar.dSYM/Contents/Resources/DWARF/foo.
static
std::string getDarwinDWARFResourceForPath(
    const std::string &Path, const std::string &Basename) {
  SmallString<16> ResourceName = StringRef(Path);
  if (sys::path::extension(Path) != ".dSYM") {
    ResourceName += ".dSYM";
  }
  sys::path::append(ResourceName, "Contents", "Resources", "DWARF");
  sys::path::append(ResourceName, Basename);
  return ResourceName.str();
}

static bool checkFileCRC(StringRef Path, uint32_t CRCHash) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> MB =
      MemoryBuffer::getFileOrSTDIN(Path);
  if (!MB)
    return false;
  return !zlib::isAvailable() || CRCHash == zlib::crc32(MB.get()->getBuffer());
}

static bool findDebugBinary(const std::string &OrigPath,
                            const std::string &DebuglinkName, uint32_t CRCHash,
                            std::string &Result) {
  std::string OrigRealPath = OrigPath;
#if defined(HAVE_REALPATH)
  if (char *RP = realpath(OrigPath.c_str(), nullptr)) {
    OrigRealPath = RP;
    free(RP);
  }
#endif
  SmallString<16> OrigDir(OrigRealPath);
  llvm::sys::path::remove_filename(OrigDir);
  SmallString<16> DebugPath = OrigDir;
  // Try /path/to/original_binary/debuglink_name
  llvm::sys::path::append(DebugPath, DebuglinkName);
  if (checkFileCRC(DebugPath, CRCHash)) {
    Result = DebugPath.str();
    return true;
  }
  // Try /path/to/original_binary/.debug/debuglink_name
  DebugPath = OrigRealPath;
  llvm::sys::path::append(DebugPath, ".debug", DebuglinkName);
  if (checkFileCRC(DebugPath, CRCHash)) {
    Result = DebugPath.str();
    return true;
  }
  // Try /usr/lib/debug/path/to/original_binary/debuglink_name
  DebugPath = "/usr/lib/debug";
  llvm::sys::path::append(DebugPath, llvm::sys::path::relative_path(OrigDir),
                          DebuglinkName);
  if (checkFileCRC(DebugPath, CRCHash)) {
    Result = DebugPath.str();
    return true;
  }
  return false;
}

static bool getGNUDebuglinkContents(const ObjectFile *Obj, std::string &DebugName,
                                    uint32_t &CRCHash) {
  if (!Obj)
    return false;
  for (const SectionRef &Section : Obj->sections()) {
    StringRef Name;
    Section.getName(Name);
    Name = Name.substr(Name.find_first_not_of("._"));
    if (Name == "gnu_debuglink") {
      StringRef Data;
      Section.getContents(Data);
      DataExtractor DE(Data, Obj->isLittleEndian(), 0);
      uint32_t Offset = 0;
      if (const char *DebugNameStr = DE.getCStr(&Offset)) {
        // 4-byte align the offset.
        Offset = (Offset + 3) & ~0x3;
        if (DE.isValidOffsetForDataOfSize(Offset, 4)) {
          DebugName = DebugNameStr;
          CRCHash = DE.getU32(&Offset);
          return true;
        }
      }
      break;
    }
  }
  return false;
}

static
bool darwinDsymMatchesBinary(const MachOObjectFile *DbgObj,
                             const MachOObjectFile *Obj) {
  ArrayRef<uint8_t> dbg_uuid = DbgObj->getUuid();
  ArrayRef<uint8_t> bin_uuid = Obj->getUuid();
  if (dbg_uuid.empty() || bin_uuid.empty())
    return false;
  return !memcmp(dbg_uuid.data(), bin_uuid.data(), dbg_uuid.size());
}

ObjectFile *LLVMSymbolizer::lookUpDsymFile(const std::string &ExePath,
    const MachOObjectFile *MachExeObj, const std::string &ArchName) {
  // On Darwin we may find DWARF in separate object file in
  // resource directory.
  std::vector<std::string> DsymPaths;
  StringRef Filename = sys::path::filename(ExePath);
  DsymPaths.push_back(getDarwinDWARFResourceForPath(ExePath, Filename));
  for (const auto &Path : Opts.DsymHints) {
    DsymPaths.push_back(getDarwinDWARFResourceForPath(Path, Filename));
  }
  for (const auto &path : DsymPaths) {
    ErrorOr<OwningBinary<Binary>> BinaryOrErr = createBinary(path);
    std::error_code EC = BinaryOrErr.getError();
    if (EC != errc::no_such_file_or_directory && !error(EC)) {
      OwningBinary<Binary> B = std::move(BinaryOrErr.get());
      ObjectFile *DbgObj =
          getObjectFileFromBinary(B.getBinary(), ArchName);
      const MachOObjectFile *MachDbgObj =
          dyn_cast<const MachOObjectFile>(DbgObj);
      if (!MachDbgObj) continue;
      if (darwinDsymMatchesBinary(MachDbgObj, MachExeObj)) {
        addOwningBinary(std::move(B));
        return DbgObj; 
      }
    }
  }
  return nullptr;
}

LLVMSymbolizer::ObjectPair
LLVMSymbolizer::getOrCreateObjects(const std::string &Path,
                                   const std::string &ArchName) {
  const auto &I = ObjectPairForPathArch.find(std::make_pair(Path, ArchName));
  if (I != ObjectPairForPathArch.end())
    return I->second;
  ObjectFile *Obj = nullptr;
  ObjectFile *DbgObj = nullptr;
  ErrorOr<OwningBinary<Binary>> BinaryOrErr = createBinary(Path);
  if (!error(BinaryOrErr.getError())) {
    OwningBinary<Binary> &B = BinaryOrErr.get();
    Obj = getObjectFileFromBinary(B.getBinary(), ArchName);
    if (!Obj) {
      ObjectPair Res = std::make_pair(nullptr, nullptr);
      ObjectPairForPathArch[std::make_pair(Path, ArchName)] = Res;
      return Res;
    }
    addOwningBinary(std::move(B));
    if (auto MachObj = dyn_cast<const MachOObjectFile>(Obj))
      DbgObj = lookUpDsymFile(Path, MachObj, ArchName);
    // Try to locate the debug binary using .gnu_debuglink section.
    if (!DbgObj) {
      std::string DebuglinkName;
      uint32_t CRCHash;
      std::string DebugBinaryPath;
      if (getGNUDebuglinkContents(Obj, DebuglinkName, CRCHash) &&
          findDebugBinary(Path, DebuglinkName, CRCHash, DebugBinaryPath)) {
        BinaryOrErr = createBinary(DebugBinaryPath);
        if (!error(BinaryOrErr.getError())) {
          OwningBinary<Binary> B = std::move(BinaryOrErr.get());
          DbgObj = getObjectFileFromBinary(B.getBinary(), ArchName);
          addOwningBinary(std::move(B));
        }
      }
    }
  }
  if (!DbgObj)
    DbgObj = Obj;
  ObjectPair Res = std::make_pair(Obj, DbgObj);
  ObjectPairForPathArch[std::make_pair(Path, ArchName)] = Res;
  return Res;
}

ObjectFile *
LLVMSymbolizer::getObjectFileFromBinary(Binary *Bin,
                                        const std::string &ArchName) {
  if (!Bin)
    return nullptr;
  ObjectFile *Res = nullptr;
  if (MachOUniversalBinary *UB = dyn_cast<MachOUniversalBinary>(Bin)) {
    const auto &I = ObjectFileForArch.find(
        std::make_pair(UB, ArchName));
    if (I != ObjectFileForArch.end())
      return I->second;
    ErrorOr<std::unique_ptr<ObjectFile>> ParsedObj =
        UB->getObjectForArch(ArchName);
    if (ParsedObj) {
      Res = ParsedObj.get().get();
      ParsedBinariesAndObjects.push_back(std::move(ParsedObj.get()));
    }
    ObjectFileForArch[std::make_pair(UB, ArchName)] = Res;
  } else if (Bin->isObject()) {
    Res = cast<ObjectFile>(Bin);
  }
  return Res;
}

ModuleInfo *
LLVMSymbolizer::getOrCreateModuleInfo(const std::string &ModuleName) {
  const auto &I = Modules.find(ModuleName);
  if (I != Modules.end())
    return I->second;
  std::string BinaryName = ModuleName;
  std::string ArchName = Opts.DefaultArch;
  size_t ColonPos = ModuleName.find_last_of(':');
  // Verify that substring after colon form a valid arch name.
  if (ColonPos != std::string::npos) {
    std::string ArchStr = ModuleName.substr(ColonPos + 1);
    if (Triple(ArchStr).getArch() != Triple::UnknownArch) {
      BinaryName = ModuleName.substr(0, ColonPos);
      ArchName = ArchStr;
    }
  }
  ObjectPair Objects = getOrCreateObjects(BinaryName, ArchName);

  if (!Objects.first) {
    // Failed to find valid object file.
    Modules.insert(make_pair(ModuleName, (ModuleInfo *)nullptr));
    return nullptr;
  }
  DIContext *Context = nullptr;
  if (auto CoffObject = dyn_cast<COFFObjectFile>(Objects.first)) {
    // If this is a COFF object, assume it contains PDB debug information.  If
    // we don't find any we will fall back to the DWARF case.
    std::unique_ptr<IPDBSession> Session;
    PDB_ErrorCode Error = loadDataForEXE(PDB_ReaderType::DIA,
                                         Objects.first->getFileName(), Session);
    if (Error == PDB_ErrorCode::Success) {
      Context = new PDBContext(*CoffObject, std::move(Session),
                               Opts.RelativeAddresses);
    }
  }
  if (!Context)
    Context = new DWARFContextInMemory(*Objects.second);
  assert(Context);
  ModuleInfo *Info = new ModuleInfo(Objects.first, Context);
  Modules.insert(make_pair(ModuleName, Info));
  return Info;
}

std::string LLVMSymbolizer::printDILineInfo(DILineInfo LineInfo) const {
  // By default, DILineInfo contains "<invalid>" for function/filename it
  // cannot fetch. We replace it to "??" to make our output closer to addr2line.
  static const std::string kDILineInfoBadString = "<invalid>";
  std::stringstream Result;
  if (Opts.PrintFunctions != FunctionNameKind::None) {
    std::string FunctionName = LineInfo.FunctionName;
    if (FunctionName == kDILineInfoBadString)
      FunctionName = kBadString;
    else if (Opts.Demangle)
      FunctionName = DemangleName(FunctionName);
    Result << FunctionName << "\n";
  }
  std::string Filename = LineInfo.FileName;
  if (Filename == kDILineInfoBadString)
    Filename = kBadString;
  Result << Filename << ":" << LineInfo.Line << ":" << LineInfo.Column << "\n";
  return Result.str();
}

#if !defined(_MSC_VER)
// Assume that __cxa_demangle is provided by libcxxabi (except for Windows).
extern "C" char *__cxa_demangle(const char *mangled_name, char *output_buffer,
                                size_t *length, int *status);
#endif

std::string LLVMSymbolizer::DemangleName(const std::string &Name) {
#if !defined(_MSC_VER)
  // We can spoil names of symbols with C linkage, so use an heuristic
  // approach to check if the name should be demangled.
  if (Name.substr(0, 2) != "_Z")
    return Name;
  int status = 0;
  char *DemangledName = __cxa_demangle(Name.c_str(), nullptr, nullptr, &status);
  if (status != 0)
    return Name;
  std::string Result = DemangledName;
  free(DemangledName);
  return Result;
#else
  char DemangledName[1024] = {0};
  DWORD result = ::UnDecorateSymbolName(
      Name.c_str(), DemangledName, 1023,
      UNDNAME_NO_ACCESS_SPECIFIERS |       // Strip public, private, protected
          UNDNAME_NO_ALLOCATION_LANGUAGE | // Strip __thiscall, __stdcall, etc
          UNDNAME_NO_THROW_SIGNATURES |    // Strip throw() specifications
          UNDNAME_NO_MEMBER_TYPE |      // Strip virtual, static, etc specifiers
          UNDNAME_NO_MS_KEYWORDS |      // Strip all MS extension keywords
          UNDNAME_NO_FUNCTION_RETURNS); // Strip function return types

  return (result == 0) ? Name : std::string(DemangledName);
#endif
}

} // namespace symbolize
} // namespace llvm
