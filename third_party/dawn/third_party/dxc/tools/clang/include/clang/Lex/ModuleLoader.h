//===--- ModuleLoader.h - Module Loader Interface ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the ModuleLoader interface, which is responsible for 
//  loading named modules.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_LEX_MODULELOADER_H
#define LLVM_CLANG_LEX_MODULELOADER_H

#include "clang/Basic/Module.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"

namespace clang {

class GlobalModuleIndex;
class IdentifierInfo;
class Module;

/// \brief A sequence of identifier/location pairs used to describe a particular
/// module or submodule, e.g., std.vector.
typedef ArrayRef<std::pair<IdentifierInfo *, SourceLocation> > ModuleIdPath;

/// \brief Describes the result of attempting to load a module.
class ModuleLoadResult {
  llvm::PointerIntPair<Module *, 1, bool> Storage;

public:
  ModuleLoadResult() : Storage() { }

  ModuleLoadResult(Module *mod, bool missingExpected)
    : Storage(mod, missingExpected) { }

  operator Module *() const { return Storage.getPointer(); }

  /// \brief Determines whether the module, which failed to load, was
  /// actually a submodule that we expected to see (based on implying the
  /// submodule from header structure), but didn't materialize in the actual
  /// module.
  bool isMissingExpected() const { return Storage.getInt(); }
};

/// \brief Abstract interface for a module loader.
///
/// This abstract interface describes a module loader, which is responsible
/// for resolving a module name (e.g., "std") to an actual module file, and
/// then loading that module.
class ModuleLoader {
  // Building a module if true.
  bool BuildingModule;
public:
  explicit ModuleLoader(bool BuildingModule = false) :
    BuildingModule(BuildingModule),
    HadFatalFailure(false) {}

  virtual ~ModuleLoader();
  
  /// \brief Returns true if this instance is building a module.
  bool buildingModule() const {
    return BuildingModule;
  }
  /// \brief Flag indicating whether this instance is building a module.
  void setBuildingModule(bool BuildingModuleFlag) {
    BuildingModule = BuildingModuleFlag;
  }
 
  /// \brief Attempt to load the given module.
  ///
  /// This routine attempts to load the module described by the given 
  /// parameters.
  ///
  /// \param ImportLoc The location of the 'import' keyword.
  ///
  /// \param Path The identifiers (and their locations) of the module
  /// "path", e.g., "std.vector" would be split into "std" and "vector".
  /// 
  /// \param Visibility The visibility provided for the names in the loaded
  /// module.
  ///
  /// \param IsInclusionDirective Indicates that this module is being loaded
  /// implicitly, due to the presence of an inclusion directive. Otherwise,
  /// it is being loaded due to an import declaration.
  ///
  /// \returns If successful, returns the loaded module. Otherwise, returns 
  /// NULL to indicate that the module could not be loaded.
  virtual ModuleLoadResult loadModule(SourceLocation ImportLoc,
                                      ModuleIdPath Path,
                                      Module::NameVisibilityKind Visibility,
                                      bool IsInclusionDirective) = 0;

  /// \brief Make the given module visible.
  virtual void makeModuleVisible(Module *Mod,
                                 Module::NameVisibilityKind Visibility,
                                 SourceLocation ImportLoc) = 0;

  /// \brief Load, create, or return global module.
  /// This function returns an existing global module index, if one
  /// had already been loaded or created, or loads one if it
  /// exists, or creates one if it doesn't exist.
  /// Also, importantly, if the index doesn't cover all the modules
  /// in the module map, it will be update to do so here, because
  /// of its use in searching for needed module imports and
  /// associated fixit messages.
  /// \param TriggerLoc The location for what triggered the load.
  /// \returns Returns null if load failed.
  virtual GlobalModuleIndex *loadGlobalModuleIndex(
                                                SourceLocation TriggerLoc) = 0;

  /// Check global module index for missing imports.
  /// \param Name The symbol name to look for.
  /// \param TriggerLoc The location for what triggered the load.
  /// \returns Returns true if any modules with that symbol found.
  virtual bool lookupMissingImports(StringRef Name,
                                    SourceLocation TriggerLoc) = 0;

  bool HadFatalFailure;
};
  
}

#endif
