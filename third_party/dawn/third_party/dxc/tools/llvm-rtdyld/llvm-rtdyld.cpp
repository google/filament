//===-- llvm-rtdyld.cpp - MCJIT Testing Tool ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is a testing tool for use with the MC-JIT LLVM components.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringMap.h"
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/RuntimeDyldChecker.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Object/MachO.h"
#include "llvm/Object/SymbolSize.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Memory.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <list>
#include <system_error>

using namespace llvm;
using namespace llvm::object;

static cl::list<std::string>
InputFileList(cl::Positional, cl::ZeroOrMore,
              cl::desc("<input file>"));

enum ActionType {
  AC_Execute,
  AC_PrintObjectLineInfo,
  AC_PrintLineInfo,
  AC_PrintDebugLineInfo,
  AC_Verify
};

static cl::opt<ActionType>
Action(cl::desc("Action to perform:"),
       cl::init(AC_Execute),
       cl::values(clEnumValN(AC_Execute, "execute",
                             "Load, link, and execute the inputs."),
                  clEnumValN(AC_PrintLineInfo, "printline",
                             "Load, link, and print line information for each function."),
                  clEnumValN(AC_PrintDebugLineInfo, "printdebugline",
                             "Load, link, and print line information for each function using the debug object"),
                  clEnumValN(AC_PrintObjectLineInfo, "printobjline",
                             "Like -printlineinfo but does not load the object first"),
                  clEnumValN(AC_Verify, "verify",
                             "Load, link and verify the resulting memory image."),
                  clEnumValEnd));

static cl::opt<std::string>
EntryPoint("entry",
           cl::desc("Function to call as entry point."),
           cl::init("_main"));

static cl::list<std::string>
Dylibs("dylib",
       cl::desc("Add library."),
       cl::ZeroOrMore);

static cl::opt<std::string>
TripleName("triple", cl::desc("Target triple for disassembler"));

static cl::opt<std::string>
MCPU("mcpu",
     cl::desc("Target a specific cpu type (-mcpu=help for details)"),
     cl::value_desc("cpu-name"),
     cl::init(""));

static cl::list<std::string>
CheckFiles("check",
           cl::desc("File containing RuntimeDyld verifier checks."),
           cl::ZeroOrMore);

static cl::opt<uint64_t>
TargetAddrStart("target-addr-start",
                cl::desc("For -verify only: start of phony target address "
                         "range."),
                cl::init(4096), // Start at "page 1" - no allocating at "null".
                cl::Hidden);

static cl::opt<uint64_t>
TargetAddrEnd("target-addr-end",
              cl::desc("For -verify only: end of phony target address range."),
              cl::init(~0ULL),
              cl::Hidden);

static cl::opt<uint64_t>
TargetSectionSep("target-section-sep",
                 cl::desc("For -verify only: Separation between sections in "
                          "phony target address space."),
                 cl::init(0),
                 cl::Hidden);

static cl::list<std::string>
SpecificSectionMappings("map-section",
                        cl::desc("For -verify only: Map a section to a "
                                 "specific address."),
                        cl::ZeroOrMore,
                        cl::Hidden);

static cl::list<std::string>
DummySymbolMappings("dummy-extern",
                    cl::desc("For -verify only: Inject a symbol into the extern "
                             "symbol table."),
                    cl::ZeroOrMore,
                    cl::Hidden);

/* *** */

// A trivial memory manager that doesn't do anything fancy, just uses the
// support library allocation routines directly.
class TrivialMemoryManager : public RTDyldMemoryManager {
public:
  SmallVector<sys::MemoryBlock, 16> FunctionMemory;
  SmallVector<sys::MemoryBlock, 16> DataMemory;

  uint8_t *allocateCodeSection(uintptr_t Size, unsigned Alignment,
                               unsigned SectionID,
                               StringRef SectionName) override;
  uint8_t *allocateDataSection(uintptr_t Size, unsigned Alignment,
                               unsigned SectionID, StringRef SectionName,
                               bool IsReadOnly) override;

  void *getPointerToNamedFunction(const std::string &Name,
                                  bool AbortOnFailure = true) override {
    return nullptr;
  }

  bool finalizeMemory(std::string *ErrMsg) override { return false; }

  // Invalidate instruction cache for sections with execute permissions.
  // Some platforms with separate data cache and instruction cache require
  // explicit cache flush, otherwise JIT code manipulations (like resolved
  // relocations) will get to the data cache but not to the instruction cache.
  virtual void invalidateInstructionCache();

  void addDummySymbol(const std::string &Name, uint64_t Addr) {
    DummyExterns[Name] = Addr;
  }

  RuntimeDyld::SymbolInfo findSymbol(const std::string &Name) override {
    auto I = DummyExterns.find(Name);

    if (I != DummyExterns.end())
      return RuntimeDyld::SymbolInfo(I->second, JITSymbolFlags::Exported);

    return RTDyldMemoryManager::findSymbol(Name);
  }

  void registerEHFrames(uint8_t *Addr, uint64_t LoadAddr,
                        size_t Size) override {}
  void deregisterEHFrames(uint8_t *Addr, uint64_t LoadAddr,
                          size_t Size) override {}
private:
  std::map<std::string, uint64_t> DummyExterns;
};

uint8_t *TrivialMemoryManager::allocateCodeSection(uintptr_t Size,
                                                   unsigned Alignment,
                                                   unsigned SectionID,
                                                   StringRef SectionName) {
  sys::MemoryBlock MB = sys::Memory::AllocateRWX(Size, nullptr, nullptr);
  FunctionMemory.push_back(MB);
  return (uint8_t*)MB.base();
}

uint8_t *TrivialMemoryManager::allocateDataSection(uintptr_t Size,
                                                   unsigned Alignment,
                                                   unsigned SectionID,
                                                   StringRef SectionName,
                                                   bool IsReadOnly) {
  sys::MemoryBlock MB = sys::Memory::AllocateRWX(Size, nullptr, nullptr);
  DataMemory.push_back(MB);
  return (uint8_t*)MB.base();
}

void TrivialMemoryManager::invalidateInstructionCache() {
  for (int i = 0, e = FunctionMemory.size(); i != e; ++i)
    sys::Memory::InvalidateInstructionCache(FunctionMemory[i].base(),
                                            FunctionMemory[i].size());

  for (int i = 0, e = DataMemory.size(); i != e; ++i)
    sys::Memory::InvalidateInstructionCache(DataMemory[i].base(),
                                            DataMemory[i].size());
}

static const char *ProgramName;

static void Message(const char *Type, const Twine &Msg) {
  errs() << ProgramName << ": " << Type << ": " << Msg << "\n";
}

static int Error(const Twine &Msg) {
  Message("error", Msg);
  return 1;
}

static void loadDylibs() {
  for (const std::string &Dylib : Dylibs) {
    if (sys::fs::is_regular_file(Dylib)) {
      std::string ErrMsg;
      if (sys::DynamicLibrary::LoadLibraryPermanently(Dylib.c_str(), &ErrMsg))
        llvm::errs() << "Error loading '" << Dylib << "': "
                     << ErrMsg << "\n";
    } else
      llvm::errs() << "Dylib not found: '" << Dylib << "'.\n";
  }
}

/* *** */

static int printLineInfoForInput(bool LoadObjects, bool UseDebugObj) {
  assert(LoadObjects || !UseDebugObj);

  // Load any dylibs requested on the command line.
  loadDylibs();

  // If we don't have any input files, read from stdin.
  if (!InputFileList.size())
    InputFileList.push_back("-");
  for(unsigned i = 0, e = InputFileList.size(); i != e; ++i) {
    // Instantiate a dynamic linker.
    TrivialMemoryManager MemMgr;
    RuntimeDyld Dyld(MemMgr, MemMgr);

    // Load the input memory buffer.

    ErrorOr<std::unique_ptr<MemoryBuffer>> InputBuffer =
        MemoryBuffer::getFileOrSTDIN(InputFileList[i]);
    if (std::error_code EC = InputBuffer.getError())
      return Error("unable to read input: '" + EC.message() + "'");

    ErrorOr<std::unique_ptr<ObjectFile>> MaybeObj(
      ObjectFile::createObjectFile((*InputBuffer)->getMemBufferRef()));

    if (std::error_code EC = MaybeObj.getError())
      return Error("unable to create object file: '" + EC.message() + "'");

    ObjectFile &Obj = **MaybeObj;

    OwningBinary<ObjectFile> DebugObj;
    std::unique_ptr<RuntimeDyld::LoadedObjectInfo> LoadedObjInfo = nullptr;
    ObjectFile *SymbolObj = &Obj;
    if (LoadObjects) {
      // Load the object file
      LoadedObjInfo =
        Dyld.loadObject(Obj);

      if (Dyld.hasError())
        return Error(Dyld.getErrorString());

      // Resolve all the relocations we can.
      Dyld.resolveRelocations();

      if (UseDebugObj) {
        DebugObj = LoadedObjInfo->getObjectForDebug(Obj);
        SymbolObj = DebugObj.getBinary();
      }
    }

    std::unique_ptr<DIContext> Context(
      new DWARFContextInMemory(*SymbolObj,LoadedObjInfo.get()));

    std::vector<std::pair<SymbolRef, uint64_t>> SymAddr =
        object::computeSymbolSizes(*SymbolObj);

    // Use symbol info to iterate functions in the object.
    for (const auto &P : SymAddr) {
      object::SymbolRef Sym = P.first;
      if (Sym.getType() == object::SymbolRef::ST_Function) {
        ErrorOr<StringRef> Name = Sym.getName();
        if (!Name)
          continue;
        ErrorOr<uint64_t> AddrOrErr = Sym.getAddress();
        if (!AddrOrErr)
          continue;
        uint64_t Addr = *AddrOrErr;

        uint64_t Size = P.second;
        // If we're not using the debug object, compute the address of the
        // symbol in memory (rather than that in the unrelocated object file)
        // and use that to query the DWARFContext.
        if (!UseDebugObj && LoadObjects) {
          object::section_iterator Sec(SymbolObj->section_end());
          Sym.getSection(Sec);
          StringRef SecName;
          Sec->getName(SecName);
          uint64_t SectionLoadAddress =
            LoadedObjInfo->getSectionLoadAddress(SecName);
          if (SectionLoadAddress != 0)
            Addr += SectionLoadAddress - Sec->getAddress();
        }

        outs() << "Function: " << *Name << ", Size = " << Size
               << ", Addr = " << Addr << "\n";

        DILineInfoTable Lines = Context->getLineInfoForAddressRange(Addr, Size);
        DILineInfoTable::iterator  Begin = Lines.begin();
        DILineInfoTable::iterator  End = Lines.end();
        for (DILineInfoTable::iterator It = Begin; It != End; ++It) {
          outs() << "  Line info @ " << It->first - Addr << ": "
                 << It->second.FileName << ", line:" << It->second.Line << "\n";
        }
      }
    }
  }

  return 0;
}

static int executeInput() {
  // Load any dylibs requested on the command line.
  loadDylibs();

  // Instantiate a dynamic linker.
  TrivialMemoryManager MemMgr;
  RuntimeDyld Dyld(MemMgr, MemMgr);

  // FIXME: Preserve buffers until resolveRelocations time to work around a bug
  //        in RuntimeDyldELF.
  // This fixme should be fixed ASAP. This is a very brittle workaround.
  std::vector<std::unique_ptr<MemoryBuffer>> InputBuffers;

  // If we don't have any input files, read from stdin.
  if (!InputFileList.size())
    InputFileList.push_back("-");
  for(unsigned i = 0, e = InputFileList.size(); i != e; ++i) {
    // Load the input memory buffer.
    ErrorOr<std::unique_ptr<MemoryBuffer>> InputBuffer =
        MemoryBuffer::getFileOrSTDIN(InputFileList[i]);
    if (std::error_code EC = InputBuffer.getError())
      return Error("unable to read input: '" + EC.message() + "'");
    ErrorOr<std::unique_ptr<ObjectFile>> MaybeObj(
      ObjectFile::createObjectFile((*InputBuffer)->getMemBufferRef()));

    if (std::error_code EC = MaybeObj.getError())
      return Error("unable to create object file: '" + EC.message() + "'");

    ObjectFile &Obj = **MaybeObj;
    InputBuffers.push_back(std::move(*InputBuffer));

    // Load the object file
    Dyld.loadObject(Obj);
    if (Dyld.hasError()) {
      return Error(Dyld.getErrorString());
    }
  }

  // Resolve all the relocations we can.
  Dyld.resolveRelocations();
  // Clear instruction cache before code will be executed.
  MemMgr.invalidateInstructionCache();

  // FIXME: Error out if there are unresolved relocations.

  // Get the address of the entry point (_main by default).
  void *MainAddress = Dyld.getSymbolLocalAddress(EntryPoint);
  if (!MainAddress)
    return Error("no definition for '" + EntryPoint + "'");

  // Invalidate the instruction cache for each loaded function.
  for (unsigned i = 0, e = MemMgr.FunctionMemory.size(); i != e; ++i) {
    sys::MemoryBlock &Data = MemMgr.FunctionMemory[i];
    // Make sure the memory is executable.
    std::string ErrorStr;
    sys::Memory::InvalidateInstructionCache(Data.base(), Data.size());
    if (!sys::Memory::setExecutable(Data, &ErrorStr))
      return Error("unable to mark function executable: '" + ErrorStr + "'");
  }

  // Dispatch to _main().
  errs() << "loaded '" << EntryPoint << "' at: " << (void*)MainAddress << "\n";

  int (*Main)(int, const char**) =
    (int(*)(int,const char**)) uintptr_t(MainAddress);
  const char **Argv = new const char*[2];
  // Use the name of the first input object module as argv[0] for the target.
  Argv[0] = InputFileList[0].c_str();
  Argv[1] = nullptr;
  return Main(1, Argv);
}

static int checkAllExpressions(RuntimeDyldChecker &Checker) {
  for (const auto& CheckerFileName : CheckFiles) {
    ErrorOr<std::unique_ptr<MemoryBuffer>> CheckerFileBuf =
        MemoryBuffer::getFileOrSTDIN(CheckerFileName);
    if (std::error_code EC = CheckerFileBuf.getError())
      return Error("unable to read input '" + CheckerFileName + "': " +
                   EC.message());

    if (!Checker.checkAllRulesInBuffer("# rtdyld-check:",
                                       CheckerFileBuf.get().get()))
      return Error("some checks in '" + CheckerFileName + "' failed");
  }
  return 0;
}

static std::map<void *, uint64_t>
applySpecificSectionMappings(RuntimeDyldChecker &Checker) {

  std::map<void*, uint64_t> SpecificMappings;

  for (StringRef Mapping : SpecificSectionMappings) {

    size_t EqualsIdx = Mapping.find_first_of("=");
    std::string SectionIDStr = Mapping.substr(0, EqualsIdx);
    size_t ComaIdx = Mapping.find_first_of(",");

    if (ComaIdx == StringRef::npos) {
      errs() << "Invalid section specification '" << Mapping
             << "'. Should be '<file name>,<section name>=<addr>'\n";
      exit(1);
    }

    std::string FileName = SectionIDStr.substr(0, ComaIdx);
    std::string SectionName = SectionIDStr.substr(ComaIdx + 1);

    uint64_t OldAddrInt;
    std::string ErrorMsg;
    std::tie(OldAddrInt, ErrorMsg) =
      Checker.getSectionAddr(FileName, SectionName, true);

    if (ErrorMsg != "") {
      errs() << ErrorMsg;
      exit(1);
    }

    void* OldAddr = reinterpret_cast<void*>(static_cast<uintptr_t>(OldAddrInt));

    std::string NewAddrStr = Mapping.substr(EqualsIdx + 1);
    uint64_t NewAddr;

    if (StringRef(NewAddrStr).getAsInteger(0, NewAddr)) {
      errs() << "Invalid section address in mapping '" << Mapping << "'.\n";
      exit(1);
    }

    Checker.getRTDyld().mapSectionAddress(OldAddr, NewAddr);
    SpecificMappings[OldAddr] = NewAddr;
  }

  return SpecificMappings;
}

// Scatter sections in all directions!
// Remaps section addresses for -verify mode. The following command line options
// can be used to customize the layout of the memory within the phony target's
// address space:
// -target-addr-start <s> -- Specify where the phony target addres range starts.
// -target-addr-end   <e> -- Specify where the phony target address range ends.
// -target-section-sep <d> -- Specify how big a gap should be left between the
//                            end of one section and the start of the next.
//                            Defaults to zero. Set to something big
//                            (e.g. 1 << 32) to stress-test stubs, GOTs, etc.
//
static void remapSectionsAndSymbols(const llvm::Triple &TargetTriple,
                                    TrivialMemoryManager &MemMgr,
                                    RuntimeDyldChecker &Checker) {

  // Set up a work list (section addr/size pairs).
  typedef std::list<std::pair<void*, uint64_t>> WorklistT;
  WorklistT Worklist;

  for (const auto& CodeSection : MemMgr.FunctionMemory)
    Worklist.push_back(std::make_pair(CodeSection.base(), CodeSection.size()));
  for (const auto& DataSection : MemMgr.DataMemory)
    Worklist.push_back(std::make_pair(DataSection.base(), DataSection.size()));

  // Apply any section-specific mappings that were requested on the command
  // line.
  typedef std::map<void*, uint64_t> AppliedMappingsT;
  AppliedMappingsT AppliedMappings = applySpecificSectionMappings(Checker);

  // Keep an "already allocated" mapping of section target addresses to sizes.
  // Sections whose address mappings aren't specified on the command line will
  // allocated around the explicitly mapped sections while maintaining the
  // minimum separation.
  std::map<uint64_t, uint64_t> AlreadyAllocated;

  // Move the previously applied mappings into the already-allocated map.
  for (WorklistT::iterator I = Worklist.begin(), E = Worklist.end();
       I != E;) {
    WorklistT::iterator Tmp = I;
    ++I;
    AppliedMappingsT::iterator AI = AppliedMappings.find(Tmp->first);

    if (AI != AppliedMappings.end()) {
      AlreadyAllocated[AI->second] = Tmp->second;
      Worklist.erase(Tmp);
    }
  }

  // If the -target-addr-end option wasn't explicitly passed, then set it to a
  // sensible default based on the target triple.
  if (TargetAddrEnd.getNumOccurrences() == 0) {
    if (TargetTriple.isArch16Bit())
      TargetAddrEnd = (1ULL << 16) - 1;
    else if (TargetTriple.isArch32Bit())
      TargetAddrEnd = (1ULL << 32) - 1;
    // TargetAddrEnd already has a sensible default for 64-bit systems, so
    // there's nothing to do in the 64-bit case.
  }

  // Process any elements remaining in the worklist.
  while (!Worklist.empty()) {
    std::pair<void*, uint64_t> CurEntry = Worklist.front();
    Worklist.pop_front();

    uint64_t NextSectionAddr = TargetAddrStart;

    for (const auto &Alloc : AlreadyAllocated)
      if (NextSectionAddr + CurEntry.second + TargetSectionSep <= Alloc.first)
        break;
      else
        NextSectionAddr = Alloc.first + Alloc.second + TargetSectionSep;

    AlreadyAllocated[NextSectionAddr] = CurEntry.second;
    Checker.getRTDyld().mapSectionAddress(CurEntry.first, NextSectionAddr);
  }

  // Add dummy symbols to the memory manager.
  for (const auto &Mapping : DummySymbolMappings) {
    size_t EqualsIdx = Mapping.find_first_of("=");

    if (EqualsIdx == StringRef::npos) {
      errs() << "Invalid dummy symbol specification '" << Mapping
             << "'. Should be '<symbol name>=<addr>'\n";
      exit(1);
    }

    std::string Symbol = Mapping.substr(0, EqualsIdx);
    std::string AddrStr = Mapping.substr(EqualsIdx + 1);

    uint64_t Addr;
    if (StringRef(AddrStr).getAsInteger(0, Addr)) {
      errs() << "Invalid symbol mapping '" << Mapping << "'.\n";
      exit(1);
    }

    MemMgr.addDummySymbol(Symbol, Addr);
  }
}

// Load and link the objects specified on the command line, but do not execute
// anything. Instead, attach a RuntimeDyldChecker instance and call it to
// verify the correctness of the linked memory.
static int linkAndVerify() {

  // Check for missing triple.
  if (TripleName == "") {
    llvm::errs() << "Error: -triple required when running in -verify mode.\n";
    return 1;
  }

  // Look up the target and build the disassembler.
  Triple TheTriple(Triple::normalize(TripleName));
  std::string ErrorStr;
  const Target *TheTarget =
    TargetRegistry::lookupTarget("", TheTriple, ErrorStr);
  if (!TheTarget) {
    llvm::errs() << "Error accessing target '" << TripleName << "': "
                 << ErrorStr << "\n";
    return 1;
  }
  TripleName = TheTriple.getTriple();

  std::unique_ptr<MCSubtargetInfo> STI(
    TheTarget->createMCSubtargetInfo(TripleName, MCPU, ""));
  assert(STI && "Unable to create subtarget info!");

  std::unique_ptr<MCRegisterInfo> MRI(TheTarget->createMCRegInfo(TripleName));
  assert(MRI && "Unable to create target register info!");

  std::unique_ptr<MCAsmInfo> MAI(TheTarget->createMCAsmInfo(*MRI, TripleName));
  assert(MAI && "Unable to create target asm info!");

  MCContext Ctx(MAI.get(), MRI.get(), nullptr);

  std::unique_ptr<MCDisassembler> Disassembler(
    TheTarget->createMCDisassembler(*STI, Ctx));
  assert(Disassembler && "Unable to create disassembler!");

  std::unique_ptr<MCInstrInfo> MII(TheTarget->createMCInstrInfo());

  std::unique_ptr<MCInstPrinter> InstPrinter(
      TheTarget->createMCInstPrinter(Triple(TripleName), 0, *MAI, *MII, *MRI));

  // Load any dylibs requested on the command line.
  loadDylibs();

  // Instantiate a dynamic linker.
  TrivialMemoryManager MemMgr;
  RuntimeDyld Dyld(MemMgr, MemMgr);
  Dyld.setProcessAllSections(true);
  RuntimeDyldChecker Checker(Dyld, Disassembler.get(), InstPrinter.get(),
                             llvm::dbgs());

  // FIXME: Preserve buffers until resolveRelocations time to work around a bug
  //        in RuntimeDyldELF.
  // This fixme should be fixed ASAP. This is a very brittle workaround.
  std::vector<std::unique_ptr<MemoryBuffer>> InputBuffers;

  // If we don't have any input files, read from stdin.
  if (!InputFileList.size())
    InputFileList.push_back("-");
  for(unsigned i = 0, e = InputFileList.size(); i != e; ++i) {
    // Load the input memory buffer.
    ErrorOr<std::unique_ptr<MemoryBuffer>> InputBuffer =
        MemoryBuffer::getFileOrSTDIN(InputFileList[i]);

    if (std::error_code EC = InputBuffer.getError())
      return Error("unable to read input: '" + EC.message() + "'");

    ErrorOr<std::unique_ptr<ObjectFile>> MaybeObj(
      ObjectFile::createObjectFile((*InputBuffer)->getMemBufferRef()));

    if (std::error_code EC = MaybeObj.getError())
      return Error("unable to create object file: '" + EC.message() + "'");

    ObjectFile &Obj = **MaybeObj;
    InputBuffers.push_back(std::move(*InputBuffer));

    // Load the object file
    Dyld.loadObject(Obj);
    if (Dyld.hasError()) {
      return Error(Dyld.getErrorString());
    }
  }

  // Re-map the section addresses into the phony target address space and add
  // dummy symbols.
  remapSectionsAndSymbols(TheTriple, MemMgr, Checker);

  // Resolve all the relocations we can.
  Dyld.resolveRelocations();

  // Register EH frames.
  Dyld.registerEHFrames();

  int ErrorCode = checkAllExpressions(Checker);
  if (Dyld.hasError()) {
    errs() << "RTDyld reported an error applying relocations:\n  "
           << Dyld.getErrorString() << "\n";
    ErrorCode = 1;
  }

  return ErrorCode;
}

int __cdecl main(int argc, char **argv) { // HLSL Change - __cdecl
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);

  ProgramName = argv[0];
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllDisassemblers();

  cl::ParseCommandLineOptions(argc, argv, "llvm MC-JIT tool\n");

  switch (Action) {
  case AC_Execute:
    return executeInput();
  case AC_PrintDebugLineInfo:
    return printLineInfoForInput(/* LoadObjects */ true,/* UseDebugObj */ true);
  case AC_PrintLineInfo:
    return printLineInfoForInput(/* LoadObjects */ true,/* UseDebugObj */false);
  case AC_PrintObjectLineInfo:
    return printLineInfoForInput(/* LoadObjects */false,/* UseDebugObj */false);
  case AC_Verify:
    return linkAndVerify();
  }
}
