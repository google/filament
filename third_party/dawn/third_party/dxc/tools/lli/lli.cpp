//===- lli.cpp - LLVM Interpreter / Dynamic compiler ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This utility provides a simple wrapper around the LLVM Execution Engines,
// which allow the direct execution of LLVM programs through a Just-In-Time
// compiler, or through an interpreter if no JIT is available for this platform.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/LLVMContext.h"
#include "OrcLazyJIT.h"
#include "RemoteMemoryManager.h"
#include "RemoteTarget.h"
#include "RemoteTargetExternal.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
// #include "llvm/ExecutionEngine/MCJIT.h"    // HLSL Change
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/ExecutionEngine/OrcMCJITReplacement.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
// #include "llvm/ExecutionEngine/SectionMemoryManager.h" // HLSL Change
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Object/Archive.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/Memory.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include <cerrno>

#ifdef __CYGWIN__
#include <cygwin/version.h>
#if defined(CYGWIN_VERSION_DLL_MAJOR) && CYGWIN_VERSION_DLL_MAJOR<1007
#define DO_NOTHING_ATEXIT 1
#endif
#endif

using namespace llvm;

#define DEBUG_TYPE "lli"

namespace {

  enum class JITKind { MCJIT, OrcMCJITReplacement, OrcLazy };

  cl::opt<std::string>
  InputFile(cl::desc("<input bitcode>"), cl::Positional, cl::init("-"));

  cl::list<std::string>
  InputArgv(cl::ConsumeAfter, cl::desc("<program arguments>..."));

  cl::opt<bool> ForceInterpreter("force-interpreter",
                                 cl::desc("Force interpretation: disable JIT"),
                                 cl::init(false));

  cl::opt<JITKind> UseJITKind("jit-kind",
                              cl::desc("Choose underlying JIT kind."),
                              cl::init(JITKind::MCJIT),
                              cl::values(
                                clEnumValN(JITKind::MCJIT, "mcjit",
                                           "MCJIT"),
                                clEnumValN(JITKind::OrcMCJITReplacement,
                                           "orc-mcjit",
                                           "Orc-based MCJIT replacement"),
                                clEnumValN(JITKind::OrcLazy,
                                           "orc-lazy",
                                           "Orc-based lazy JIT."),
                                clEnumValEnd));

  // The MCJIT supports building for a target address space separate from
  // the JIT compilation process. Use a forked process and a copying
  // memory manager with IPC to execute using this functionality.
  cl::opt<bool> RemoteMCJIT("remote-mcjit",
    cl::desc("Execute MCJIT'ed code in a separate process."),
    cl::init(false));

  // Manually specify the child process for remote execution. This overrides
  // the simulated remote execution that allocates address space for child
  // execution. The child process will be executed and will communicate with
  // lli via stdin/stdout pipes.
  cl::opt<std::string>
  ChildExecPath("mcjit-remote-process",
                cl::desc("Specify the filename of the process to launch "
                         "for remote MCJIT execution.  If none is specified,"
                         "\n\tremote execution will be simulated in-process."),
                cl::value_desc("filename"), cl::init(""));

  // Determine optimization level.
  cl::opt<char>
  OptLevel("O",
           cl::desc("Optimization level. [-O0, -O1, -O2, or -O3] "
                    "(default = '-O2')"),
           cl::Prefix,
           cl::ZeroOrMore,
           cl::init(' '));

  cl::opt<std::string>
  TargetTriple("mtriple", cl::desc("Override target triple for module"));

  cl::opt<std::string>
  MArch("march",
        cl::desc("Architecture to generate assembly for (see --version)"));

  cl::opt<std::string>
  MCPU("mcpu",
       cl::desc("Target a specific cpu type (-mcpu=help for details)"),
       cl::value_desc("cpu-name"),
       cl::init(""));

  cl::list<std::string>
  MAttrs("mattr",
         cl::CommaSeparated,
         cl::desc("Target specific attributes (-mattr=help for details)"),
         cl::value_desc("a1,+a2,-a3,..."));

  cl::opt<std::string>
  EntryFunc("entry-function",
            cl::desc("Specify the entry function (default = 'main') "
                     "of the executable"),
            cl::value_desc("function"),
            cl::init("main"));

  cl::list<std::string>
  ExtraModules("extra-module",
         cl::desc("Extra modules to be loaded"),
         cl::value_desc("input bitcode"));

  cl::list<std::string>
  ExtraObjects("extra-object",
         cl::desc("Extra object files to be loaded"),
         cl::value_desc("input object"));

  cl::list<std::string>
  ExtraArchives("extra-archive",
         cl::desc("Extra archive files to be loaded"),
         cl::value_desc("input archive"));

  cl::opt<bool>
  EnableCacheManager("enable-cache-manager",
        cl::desc("Use cache manager to save/load mdoules"),
        cl::init(false));

  cl::opt<std::string>
  ObjectCacheDir("object-cache-dir",
                  cl::desc("Directory to store cached object files "
                           "(must be user writable)"),
                  cl::init(""));

  cl::opt<std::string>
  FakeArgv0("fake-argv0",
            cl::desc("Override the 'argv[0]' value passed into the executing"
                     " program"), cl::value_desc("executable"));

  cl::opt<bool>
  DisableCoreFiles("disable-core-files", cl::Hidden,
                   cl::desc("Disable emission of core files if possible"));

  cl::opt<bool>
  NoLazyCompilation("disable-lazy-compilation",
                  cl::desc("Disable JIT lazy compilation"),
                  cl::init(false));

  cl::opt<Reloc::Model>
  RelocModel("relocation-model",
             cl::desc("Choose relocation model"),
             cl::init(Reloc::Default),
             cl::values(
            clEnumValN(Reloc::Default, "default",
                       "Target default relocation model"),
            clEnumValN(Reloc::Static, "static",
                       "Non-relocatable code"),
            clEnumValN(Reloc::PIC_, "pic",
                       "Fully relocatable, position independent code"),
            clEnumValN(Reloc::DynamicNoPIC, "dynamic-no-pic",
                       "Relocatable external references, non-relocatable code"),
            clEnumValEnd));

  cl::opt<llvm::CodeModel::Model>
  CMModel("code-model",
          cl::desc("Choose code model"),
          cl::init(CodeModel::JITDefault),
          cl::values(clEnumValN(CodeModel::JITDefault, "default",
                                "Target default JIT code model"),
                     clEnumValN(CodeModel::Small, "small",
                                "Small code model"),
                     clEnumValN(CodeModel::Kernel, "kernel",
                                "Kernel code model"),
                     clEnumValN(CodeModel::Medium, "medium",
                                "Medium code model"),
                     clEnumValN(CodeModel::Large, "large",
                                "Large code model"),
                     clEnumValEnd));

  cl::opt<bool>
  GenerateSoftFloatCalls("soft-float",
    cl::desc("Generate software floating point library calls"),
    cl::init(false));

  cl::opt<llvm::FloatABI::ABIType>
  FloatABIForCalls("float-abi",
                   cl::desc("Choose float ABI type"),
                   cl::init(FloatABI::Default),
                   cl::values(
                     clEnumValN(FloatABI::Default, "default",
                                "Target default float ABI type"),
                     clEnumValN(FloatABI::Soft, "soft",
                                "Soft float ABI (implied by -soft-float)"),
                     clEnumValN(FloatABI::Hard, "hard",
                                "Hard float ABI (uses FP registers)"),
                     clEnumValEnd));
}

//===----------------------------------------------------------------------===//
// Object cache
//
// This object cache implementation writes cached objects to disk to the
// directory specified by CacheDir, using a filename provided in the module
// descriptor. The cache tries to load a saved object using that path if the
// file exists. CacheDir defaults to "", in which case objects are cached
// alongside their originating bitcodes.
//
class LLIObjectCache : public ObjectCache {
public:
  LLIObjectCache(const std::string& CacheDir) : CacheDir(CacheDir) {
    // Add trailing '/' to cache dir if necessary.
    if (!this->CacheDir.empty() &&
        this->CacheDir[this->CacheDir.size() - 1] != '/')
      this->CacheDir += '/';
  }
  ~LLIObjectCache() override {}

  void notifyObjectCompiled(const Module *M, MemoryBufferRef Obj) override {
    const std::string ModuleID = M->getModuleIdentifier();
    std::string CacheName;
    if (!getCacheFilename(ModuleID, CacheName))
      return;
    if (!CacheDir.empty()) { // Create user-defined cache dir.
      SmallString<128> dir(CacheName);
      sys::path::remove_filename(dir);
      sys::fs::create_directories(Twine(dir));
    }
    std::error_code EC;
    raw_fd_ostream outfile(CacheName, EC, sys::fs::F_None);
    outfile.write(Obj.getBufferStart(), Obj.getBufferSize());
    outfile.close();
  }

  std::unique_ptr<MemoryBuffer> getObject(const Module* M) override {
    const std::string ModuleID = M->getModuleIdentifier();
    std::string CacheName;
    if (!getCacheFilename(ModuleID, CacheName))
      return nullptr;
    // Load the object from the cache filename
    ErrorOr<std::unique_ptr<MemoryBuffer>> IRObjectBuffer =
        MemoryBuffer::getFile(CacheName.c_str(), -1, false);
    // If the file isn't there, that's OK.
    if (!IRObjectBuffer)
      return nullptr;
    // MCJIT will want to write into this buffer, and we don't want that
    // because the file has probably just been mmapped.  Instead we make
    // a copy.  The filed-based buffer will be released when it goes
    // out of scope.
    return MemoryBuffer::getMemBufferCopy(IRObjectBuffer.get()->getBuffer());
  }

private:
  std::string CacheDir;

  bool getCacheFilename(const std::string &ModID, std::string &CacheName) {
    std::string Prefix("file:");
    size_t PrefixLength = Prefix.length();
    if (ModID.substr(0, PrefixLength) != Prefix)
      return false;
        std::string CacheSubdir = ModID.substr(PrefixLength);
#if defined(_WIN32)
        // Transform "X:\foo" => "/X\foo" for convenience.
        if (isalpha(CacheSubdir[0]) && CacheSubdir[1] == ':') {
          CacheSubdir[1] = CacheSubdir[0];
          CacheSubdir[0] = '/';
        }
#endif
    CacheName = CacheDir + CacheSubdir;
    size_t pos = CacheName.rfind('.');
    CacheName.replace(pos, CacheName.length() - pos, ".o");
    return true;
  }
};

static ExecutionEngine *EE = nullptr;
static LLIObjectCache *CacheManager = nullptr;

// HLSL Change: changed calling convention to __cdecl
static void __cdecl do_shutdown() {
  // Cygwin-1.5 invokes DLL's dtors before atexit handler.
#ifndef DO_NOTHING_ATEXIT
  delete EE;
  if (CacheManager)
    delete CacheManager;
  llvm_shutdown();
#endif
}

// On Mingw and Cygwin, an external symbol named '__main' is called from the
// generated 'main' function to allow static intialization.  To avoid linking
// problems with remote targets (because lli's remote target support does not
// currently handle external linking) we add a secondary module which defines
// an empty '__main' function.
static void addCygMingExtraModule(ExecutionEngine *EE,
                                  LLVMContext &Context,
                                  StringRef TargetTripleStr) {
  IRBuilder<> Builder(Context);
  Triple TargetTriple(TargetTripleStr);

  // Create a new module.
  std::unique_ptr<Module> M = make_unique<Module>("CygMingHelper", Context);
  M->setTargetTriple(TargetTripleStr);

  // Create an empty function named "__main".
  Function *Result;
  if (TargetTriple.isArch64Bit()) {
    Result = Function::Create(
      TypeBuilder<int64_t(void), false>::get(Context),
      GlobalValue::ExternalLinkage, "__main", M.get());
  } else {
    Result = Function::Create(
      TypeBuilder<int32_t(void), false>::get(Context),
      GlobalValue::ExternalLinkage, "__main", M.get());
  }
  BasicBlock *BB = BasicBlock::Create(Context, "__main", Result);
  Builder.SetInsertPoint(BB);
  Value *ReturnVal;
  if (TargetTriple.isArch64Bit())
    ReturnVal = ConstantInt::get(Context, APInt(64, 0));
  else
    ReturnVal = ConstantInt::get(Context, APInt(32, 0));
  Builder.CreateRet(ReturnVal);

  // Add this new module to the ExecutionEngine.
  EE->addModule(std::move(M));
}

CodeGenOpt::Level getOptLevel() {
  switch (OptLevel) {
  default:
    errs() << "lli: Invalid optimization level.\n";
    exit(1);
  case '0': return CodeGenOpt::None;
  case '1': return CodeGenOpt::Less;
  case ' ':
  case '2': return CodeGenOpt::Default;
  case '3': return CodeGenOpt::Aggressive;
  }
  llvm_unreachable("Unrecognized opt level.");
}

//===----------------------------------------------------------------------===//
// main Driver function
//
// HLSL Change: changed calling convention to __cdecl
int __cdecl main(int argc, char **argv, char * const *envp) {
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);

  LLVMContext &Context = getGlobalContext();
  atexit(do_shutdown);  // Call llvm_shutdown() on exit.

  // If we have a native target, initialize it to ensure it is linked in and
  // usable by the JIT.
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  cl::ParseCommandLineOptions(argc, argv,
                              "llvm interpreter & dynamic compiler\n");

  // If the user doesn't want core files, disable them.
  if (DisableCoreFiles)
    sys::Process::PreventCoreFiles();

  // Load the bitcode...
  SMDiagnostic Err;
  std::unique_ptr<Module> Owner = parseIRFile(InputFile, Err, Context);
  Module *Mod = Owner.get();
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }

  if (UseJITKind == JITKind::OrcLazy)
    return runOrcLazyJIT(std::move(Owner), argc, argv);

  if (EnableCacheManager) {
    std::string CacheName("file:");
    CacheName.append(InputFile);
    Mod->setModuleIdentifier(CacheName);
  }

  // If not jitting lazily, load the whole bitcode file eagerly too.
  if (NoLazyCompilation) {
    if (std::error_code EC = Mod->materializeAllPermanently()) {
      errs() << argv[0] << ": bitcode didn't read correctly.\n";
      errs() << "Reason: " << EC.message() << "\n";
      exit(1);
    }
  }

  std::string ErrorMsg;
  EngineBuilder builder(std::move(Owner));
  builder.setMArch(MArch);
  builder.setMCPU(MCPU);
  builder.setMAttrs(MAttrs);
  builder.setRelocationModel(RelocModel);
  builder.setCodeModel(CMModel);
  builder.setErrorStr(&ErrorMsg);
  builder.setEngineKind(ForceInterpreter
                        ? EngineKind::Interpreter
                        : EngineKind::JIT);
  builder.setUseOrcMCJITReplacement(UseJITKind == JITKind::OrcMCJITReplacement);

  // If we are supposed to override the target triple, do so now.
  if (!TargetTriple.empty())
    Mod->setTargetTriple(Triple::normalize(TargetTriple));

  // Enable MCJIT if desired.
  RTDyldMemoryManager *RTDyldMM = nullptr;
#if 0 // HLSL Change Starts - disable MCJIT
  if (!ForceInterpreter) {
    if (RemoteMCJIT)
      RTDyldMM = new RemoteMemoryManager();
    else
      RTDyldMM = new SectionMemoryManager();

    // Deliberately construct a temp std::unique_ptr to pass in. Do not null out
    // RTDyldMM: We still use it below, even though we don't own it.
    builder.setMCJITMemoryManager(
      std::unique_ptr<RTDyldMemoryManager>(RTDyldMM));
  } else if (RemoteMCJIT) {
    errs() << "error: Remote process execution does not work with the "
              "interpreter.\n";
    exit(1);
  }
#endif // HLSL Change Ends

  builder.setOptLevel(getOptLevel());

  TargetOptions Options;
  if (FloatABIForCalls != FloatABI::Default)
    Options.FloatABIType = FloatABIForCalls;

  builder.setTargetOptions(Options);

  EE = builder.create();
  if (!EE) {
    if (!ErrorMsg.empty())
      errs() << argv[0] << ": error creating EE: " << ErrorMsg << "\n";
    else
      errs() << argv[0] << ": unknown error creating EE!\n";
    exit(1);
  }

  if (EnableCacheManager) {
    CacheManager = new LLIObjectCache(ObjectCacheDir);
    EE->setObjectCache(CacheManager);
  }

  // Load any additional modules specified on the command line.
  for (unsigned i = 0, e = ExtraModules.size(); i != e; ++i) {
    std::unique_ptr<Module> XMod = parseIRFile(ExtraModules[i], Err, Context);
    if (!XMod) {
      Err.print(argv[0], errs());
      return 1;
    }
    if (EnableCacheManager) {
      std::string CacheName("file:");
      CacheName.append(ExtraModules[i]);
      XMod->setModuleIdentifier(CacheName);
    }
    EE->addModule(std::move(XMod));
  }

  for (unsigned i = 0, e = ExtraObjects.size(); i != e; ++i) {
    ErrorOr<object::OwningBinary<object::ObjectFile>> Obj =
        object::ObjectFile::createObjectFile(ExtraObjects[i]);
    if (!Obj) {
      Err.print(argv[0], errs());
      return 1;
    }
    object::OwningBinary<object::ObjectFile> &O = Obj.get();
    EE->addObjectFile(std::move(O));
  }

  for (unsigned i = 0, e = ExtraArchives.size(); i != e; ++i) {
    ErrorOr<std::unique_ptr<MemoryBuffer>> ArBufOrErr =
        MemoryBuffer::getFileOrSTDIN(ExtraArchives[i]);
    if (!ArBufOrErr) {
      Err.print(argv[0], errs());
      return 1;
    }
    std::unique_ptr<MemoryBuffer> &ArBuf = ArBufOrErr.get();

    ErrorOr<std::unique_ptr<object::Archive>> ArOrErr =
        object::Archive::create(ArBuf->getMemBufferRef());
    if (std::error_code EC = ArOrErr.getError()) {
      errs() << EC.message();
      return 1;
    }
    std::unique_ptr<object::Archive> &Ar = ArOrErr.get();

    object::OwningBinary<object::Archive> OB(std::move(Ar), std::move(ArBuf));

    EE->addArchive(std::move(OB));
  }

  // If the target is Cygwin/MingW and we are generating remote code, we
  // need an extra module to help out with linking.
  if (RemoteMCJIT && Triple(Mod->getTargetTriple()).isOSCygMing()) {
    addCygMingExtraModule(EE, Context, Mod->getTargetTriple());
  }

  // The following functions have no effect if their respective profiling
  // support wasn't enabled in the build configuration.
  EE->RegisterJITEventListener(
                JITEventListener::createOProfileJITEventListener());
  EE->RegisterJITEventListener(
                JITEventListener::createIntelJITEventListener());

  if (!NoLazyCompilation && RemoteMCJIT) {
    errs() << "warning: remote mcjit does not support lazy compilation\n";
    NoLazyCompilation = true;
  }
  EE->DisableLazyCompilation(NoLazyCompilation);

  // If the user specifically requested an argv[0] to pass into the program,
  // do it now.
  if (!FakeArgv0.empty()) {
    InputFile = static_cast<std::string>(FakeArgv0);
  } else {
    // Otherwise, if there is a .bc suffix on the executable strip it off, it
    // might confuse the program.
    if (StringRef(InputFile).endswith(".bc"))
      InputFile.erase(InputFile.length() - 3);
  }

  // Add the module's name to the start of the vector of arguments to main().
  InputArgv.insert(InputArgv.begin(), InputFile);

  // Call the main function from M as if its signature were:
  //   int main (int argc, char **argv, const char **envp)
  // using the contents of Args to determine argc & argv, and the contents of
  // EnvVars to determine envp.
  //
  Function *EntryFn = Mod->getFunction(EntryFunc);
  if (!EntryFn) {
    errs() << '\'' << EntryFunc << "\' function not found in module.\n";
    return -1;
  }

  // Reset errno to zero on entry to main.
  errno = 0;

  int Result;

  if (!RemoteMCJIT) {
    // If the program doesn't explicitly call exit, we will need the Exit
    // function later on to make an explicit call, so get the function now.
    Constant *Exit = Mod->getOrInsertFunction("exit", Type::getVoidTy(Context),
                                                      Type::getInt32Ty(Context),
                                                      nullptr);

    // Run static constructors.
    if (!ForceInterpreter) {
      // Give MCJIT a chance to apply relocations and set page permissions.
      EE->finalizeObject();
    }
    EE->runStaticConstructorsDestructors(false);

    // Trigger compilation separately so code regions that need to be
    // invalidated will be known.
    (void)EE->getPointerToFunction(EntryFn);
    // Clear instruction cache before code will be executed.
#if 0 // HLSL Change Starts
    if (RTDyldMM)
      static_cast<SectionMemoryManager*>(RTDyldMM)->invalidateInstructionCache();
#endif // HLSL Change Ends

    // Run main.
    Result = EE->runFunctionAsMain(EntryFn, InputArgv, envp);

    // Run static destructors.
    EE->runStaticConstructorsDestructors(true);

    // If the program didn't call exit explicitly, we should call it now.
    // This ensures that any atexit handlers get called correctly.
    if (Function *ExitF = dyn_cast<Function>(Exit)) {
      std::vector<GenericValue> Args;
      GenericValue ResultGV;
      ResultGV.IntVal = APInt(32, Result);
      Args.push_back(ResultGV);
      EE->runFunction(ExitF, Args);
      errs() << "ERROR: exit(" << Result << ") returned!\n";
      abort();
    } else {
      errs() << "ERROR: exit defined with wrong prototype!\n";
      abort();
    }
  } else {
    // else == "if (RemoteMCJIT)"

    // Remote target MCJIT doesn't (yet) support static constructors. No reason
    // it couldn't. This is a limitation of the LLI implemantation, not the
    // MCJIT itself. FIXME.
    //
    RemoteMemoryManager *MM = static_cast<RemoteMemoryManager*>(RTDyldMM);
    // Everything is prepared now, so lay out our program for the target
    // address space, assign the section addresses to resolve any relocations,
    // and send it to the target.

    std::unique_ptr<RemoteTarget> Target;
    if (!ChildExecPath.empty()) { // Remote execution on a child process
#ifndef LLVM_ON_UNIX
      // FIXME: Remove this pointless fallback mode which causes tests to "pass"
      // on platforms where they should XFAIL.
      errs() << "Warning: host does not support external remote targets.\n"
             << "  Defaulting to simulated remote execution\n";
      Target.reset(new RemoteTarget);
#else
      if (!sys::fs::can_execute(ChildExecPath)) {
        errs() << "Unable to find usable child executable: '" << ChildExecPath
               << "'\n";
        return -1;
      }
      Target.reset(new RemoteTargetExternal(ChildExecPath));
#endif
    } else {
      // No child process name provided, use simulated remote execution.
      Target.reset(new RemoteTarget);
    }

    // Give the memory manager a pointer to our remote target interface object.
    MM->setRemoteTarget(Target.get());

    // Create the remote target.
    if (!Target->create()) {
      errs() << "ERROR: " << Target->getErrorMsg() << "\n";
      return EXIT_FAILURE;
    }

    // Since we're executing in a (at least simulated) remote address space,
    // we can't use the ExecutionEngine::runFunctionAsMain(). We have to
    // grab the function address directly here and tell the remote target
    // to execute the function.
    //
    // Our memory manager will map generated code into the remote address
    // space as it is loaded and copy the bits over during the finalizeMemory
    // operation.
    //
    // FIXME: argv and envp handling.
    uint64_t Entry = EE->getFunctionAddress(EntryFn->getName().str());

    DEBUG(dbgs() << "Executing '" << EntryFn->getName() << "' at 0x"
                 << format("%llx", Entry) << "\n");

    if (!Target->executeCode(Entry, Result))
      errs() << "ERROR: " << Target->getErrorMsg() << "\n";

    // Like static constructors, the remote target MCJIT support doesn't handle
    // this yet. It could. FIXME.

    // Stop the remote target
    Target->stop();
  }

  return Result;
}
