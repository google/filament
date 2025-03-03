//===-LTOModule.h - LLVM Link Time Optimizer ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the LTOModule class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LTO_LTOMODULE_H
#define LLVM_LTO_LTOMODULE_H

#include "llvm-c/lto.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/Object/IRObjectFile.h"
#include "llvm/Target/TargetMachine.h"
#include <string>
#include <vector>

// Forward references to llvm classes.
namespace llvm {
  class Function;
  class GlobalValue;
  class MemoryBuffer;
  class TargetOptions;
  class Value;
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
/// C++ class which implements the opaque lto_module_t type.
///
struct LTOModule {
private:
  struct NameAndAttributes {
    const char        *name;
    uint32_t           attributes;
    bool               isFunction;
    const GlobalValue *symbol;
  };

  std::unique_ptr<LLVMContext> OwnedContext;

  std::string LinkerOpts;

  std::unique_ptr<object::IRObjectFile> IRFile;
  std::unique_ptr<TargetMachine> _target;
  std::vector<NameAndAttributes> _symbols;

  // _defines and _undefines only needed to disambiguate tentative definitions
  StringSet<>                             _defines;
  StringMap<NameAndAttributes> _undefines;
  std::vector<const char*>                _asm_undefines;

  LTOModule(std::unique_ptr<object::IRObjectFile> Obj, TargetMachine *TM);
  LTOModule(std::unique_ptr<object::IRObjectFile> Obj, TargetMachine *TM,
            std::unique_ptr<LLVMContext> Context);

public:
  ~LTOModule();

  /// Returns 'true' if the file or memory contents is LLVM bitcode.
  static bool isBitcodeFile(const void *mem, size_t length);
  static bool isBitcodeFile(const char *path);

  /// Returns 'true' if the memory buffer is LLVM bitcode for the specified
  /// triple.
  static bool isBitcodeForTarget(MemoryBuffer *memBuffer,
                                 StringRef triplePrefix);

  /// Create a MemoryBuffer from a memory range with an optional name.
  static std::unique_ptr<MemoryBuffer>
  makeBuffer(const void *mem, size_t length, StringRef name = "");

  /// Create an LTOModule. N.B. These methods take ownership of the buffer. The
  /// caller must have initialized the Targets, the TargetMCs, the AsmPrinters,
  /// and the AsmParsers by calling:
  ///
  /// InitializeAllTargets();
  /// InitializeAllTargetMCs();
  /// InitializeAllAsmPrinters();
  /// InitializeAllAsmParsers();
  static LTOModule *createFromFile(const char *path, TargetOptions options,
                                   std::string &errMsg);
  static LTOModule *createFromOpenFile(int fd, const char *path, size_t size,
                                       TargetOptions options,
                                       std::string &errMsg);
  static LTOModule *createFromOpenFileSlice(int fd, const char *path,
                                            size_t map_size, off_t offset,
                                            TargetOptions options,
                                            std::string &errMsg);
  static LTOModule *createFromBuffer(const void *mem, size_t length,
                                     TargetOptions options, std::string &errMsg,
                                     StringRef path = "");

  static LTOModule *createInLocalContext(const void *mem, size_t length,
                                         TargetOptions options,
                                         std::string &errMsg, StringRef path);
  static LTOModule *createInContext(const void *mem, size_t length,
                                    TargetOptions options, std::string &errMsg,
                                    StringRef path, LLVMContext *Context);

  const Module &getModule() const {
    return const_cast<LTOModule*>(this)->getModule();
  }
  Module &getModule() {
    return IRFile->getModule();
  }

  /// Return the Module's target triple.
  const std::string &getTargetTriple() {
    return getModule().getTargetTriple();
  }

  /// Set the Module's target triple.
  void setTargetTriple(StringRef Triple) {
    getModule().setTargetTriple(Triple);
  }

  /// Get the number of symbols
  uint32_t getSymbolCount() {
    return _symbols.size();
  }

  /// Get the attributes for a symbol at the specified index.
  lto_symbol_attributes getSymbolAttributes(uint32_t index) {
    if (index < _symbols.size())
      return lto_symbol_attributes(_symbols[index].attributes);
    return lto_symbol_attributes(0);
  }

  /// Get the name of the symbol at the specified index.
  const char *getSymbolName(uint32_t index) {
    if (index < _symbols.size())
      return _symbols[index].name;
    return nullptr;
  }

  const GlobalValue *getSymbolGV(uint32_t index) {
    if (index < _symbols.size())
      return _symbols[index].symbol;
    return nullptr;
  }

  const char *getLinkerOpts() {
    return LinkerOpts.c_str();
  }

  const std::vector<const char*> &getAsmUndefinedRefs() {
    return _asm_undefines;
  }

private:
  /// Parse metadata from the module
  // FIXME: it only parses "Linker Options" metadata at the moment
  void parseMetadata();

  /// Parse the symbols from the module and model-level ASM and add them to
  /// either the defined or undefined lists.
  bool parseSymbols(std::string &errMsg);

  /// Add a symbol which isn't defined just yet to a list to be resolved later.
  void addPotentialUndefinedSymbol(const object::BasicSymbolRef &Sym,
                                   bool isFunc);

  /// Add a defined symbol to the list.
  void addDefinedSymbol(const char *Name, const GlobalValue *def,
                        bool isFunction);

  /// Add a data symbol as defined to the list.
  void addDefinedDataSymbol(const object::BasicSymbolRef &Sym);
  void addDefinedDataSymbol(const char*Name, const GlobalValue *v);

  /// Add a function symbol as defined to the list.
  void addDefinedFunctionSymbol(const object::BasicSymbolRef &Sym);
  void addDefinedFunctionSymbol(const char *Name, const Function *F);

  /// Add a global symbol from module-level ASM to the defined list.
  void addAsmGlobalSymbol(const char *, lto_symbol_attributes scope);

  /// Add a global symbol from module-level ASM to the undefined list.
  void addAsmGlobalSymbolUndef(const char *);

  /// Parse i386/ppc ObjC class data structure.
  void addObjCClass(const GlobalVariable *clgv);

  /// Parse i386/ppc ObjC category data structure.
  void addObjCCategory(const GlobalVariable *clgv);

  /// Parse i386/ppc ObjC class list data structure.
  void addObjCClassRef(const GlobalVariable *clgv);

  /// Get string that the data pointer points to.
  bool objcClassNameFromExpression(const Constant *c, std::string &name);

  /// Create an LTOModule (private version).
  static LTOModule *makeLTOModule(MemoryBufferRef Buffer, TargetOptions options,
                                  std::string &errMsg, LLVMContext *Context);
};
}
#endif
