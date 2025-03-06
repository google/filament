//===--- HeaderSearch.h - Resolve Header File Locations ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the HeaderSearch interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_HEADERSEARCH_H
#define LLVM_CLANG_LEX_HEADERSEARCH_H

#include "clang/Lex/DirectoryLookup.h"
#include "clang/Lex/ModuleMap.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Allocator.h"
#include <memory>
#include <vector>

namespace clang {
  
class DiagnosticsEngine;  
class ExternalPreprocessorSource;
class FileEntry;
class FileManager;
class HeaderSearchOptions;
class IdentifierInfo;
class Preprocessor;

/// \brief The preprocessor keeps track of this information for each
/// file that is \#included.
struct HeaderFileInfo {
  /// \brief True if this is a \#import'd or \#pragma once file.
  unsigned isImport : 1;

  /// \brief True if this is a \#pragma once file.
  unsigned isPragmaOnce : 1;

  /// DirInfo - Keep track of whether this is a system header, and if so,
  /// whether it is C++ clean or not.  This can be set by the include paths or
  /// by \#pragma gcc system_header.  This is an instance of
  /// SrcMgr::CharacteristicKind.
  unsigned DirInfo : 2;

  /// \brief Whether this header file info was supplied by an external source.
  unsigned External : 1;

  /// \brief Whether this header is part of a module.
  unsigned isModuleHeader : 1;

  /// \brief Whether this header is part of the module that we are building.
  unsigned isCompilingModuleHeader : 1;

  /// \brief Whether this header is part of the module that we are building.
  /// This is an instance of ModuleMap::ModuleHeaderRole.
  unsigned HeaderRole : 2;
  
  /// \brief Whether this structure is considered to already have been
  /// "resolved", meaning that it was loaded from the external source.
  unsigned Resolved : 1;
  
  /// \brief Whether this is a header inside a framework that is currently
  /// being built. 
  ///
  /// When a framework is being built, the headers have not yet been placed
  /// into the appropriate framework subdirectories, and therefore are
  /// provided via a header map. This bit indicates when this is one of
  /// those framework headers.
  unsigned IndexHeaderMapHeader : 1;

  /// \brief Whether this file had been looked up as a header.
  unsigned IsValid : 1;
  
  /// \brief The number of times the file has been included already.
  unsigned short NumIncludes;

  /// \brief The ID number of the controlling macro.
  ///
  /// This ID number will be non-zero when there is a controlling
  /// macro whose IdentifierInfo may not yet have been loaded from
  /// external storage.
  unsigned ControllingMacroID;

  /// If this file has a \#ifndef XXX (or equivalent) guard that
  /// protects the entire contents of the file, this is the identifier
  /// for the macro that controls whether or not it has any effect.
  ///
  /// Note: Most clients should use getControllingMacro() to access
  /// the controlling macro of this header, since
  /// getControllingMacro() is able to load a controlling macro from
  /// external storage.
  const IdentifierInfo *ControllingMacro;

  /// \brief If this header came from a framework include, this is the name
  /// of the framework.
  StringRef Framework;
  
  HeaderFileInfo()
    : isImport(false), isPragmaOnce(false), DirInfo(SrcMgr::C_User), 
      External(false), isModuleHeader(false), isCompilingModuleHeader(false),
      HeaderRole(ModuleMap::NormalHeader),
      Resolved(false), IndexHeaderMapHeader(false), IsValid(0),
      NumIncludes(0), ControllingMacroID(0), ControllingMacro(nullptr)  {}

  /// \brief Retrieve the controlling macro for this header file, if
  /// any.
  const IdentifierInfo *
  getControllingMacro(ExternalPreprocessorSource *External);

  /// \brief Determine whether this is a non-default header file info, e.g.,
  /// it corresponds to an actual header we've included or tried to include.
  bool isNonDefault() const {
    return isImport || isPragmaOnce || NumIncludes || ControllingMacro || 
      ControllingMacroID;
  }

  /// \brief Get the HeaderRole properly typed.
  ModuleMap::ModuleHeaderRole getHeaderRole() const {
    return static_cast<ModuleMap::ModuleHeaderRole>(HeaderRole);
  }

  /// \brief Set the HeaderRole properly typed.
  void setHeaderRole(ModuleMap::ModuleHeaderRole Role) {
    HeaderRole = Role;
  }
};

/// \brief An external source of header file information, which may supply
/// information about header files already included.
class ExternalHeaderFileInfoSource {
public:
  virtual ~ExternalHeaderFileInfoSource();
  
  /// \brief Retrieve the header file information for the given file entry.
  ///
  /// \returns Header file information for the given file entry, with the
  /// \c External bit set. If the file entry is not known, return a 
  /// default-constructed \c HeaderFileInfo.
  virtual HeaderFileInfo GetHeaderFileInfo(const FileEntry *FE) = 0;
};
  
/// \brief Encapsulates the information needed to find the file referenced
/// by a \#include or \#include_next, (sub-)framework lookup, etc.
class HeaderSearch {
  /// This structure is used to record entries in our framework cache.
  struct FrameworkCacheEntry {
    /// The directory entry which should be used for the cached framework.
    const DirectoryEntry *Directory;

    /// Whether this framework has been "user-specified" to be treated as if it
    /// were a system framework (even if it was found outside a system framework
    /// directory).
    bool IsUserSpecifiedSystemFramework;
  };

  /// \brief Header-search options used to initialize this header search.
  IntrusiveRefCntPtr<HeaderSearchOptions> HSOpts;

  DiagnosticsEngine &Diags;
  FileManager &FileMgr;
  /// \#include search path information.  Requests for \#include "x" search the
  /// directory of the \#including file first, then each directory in SearchDirs
  /// consecutively. Requests for <x> search the current dir first, then each
  /// directory in SearchDirs, starting at AngledDirIdx, consecutively.  If
  /// NoCurDirSearch is true, then the check for the file in the current
  /// directory is suppressed.
  std::vector<DirectoryLookup> SearchDirs;
  unsigned AngledDirIdx;
  unsigned SystemDirIdx;
  bool NoCurDirSearch;

  /// \brief \#include prefixes for which the 'system header' property is
  /// overridden.
  ///
  /// For a \#include "x" or \#include \<x> directive, the last string in this
  /// list which is a prefix of 'x' determines whether the file is treated as
  /// a system header.
  std::vector<std::pair<std::string, bool> > SystemHeaderPrefixes;

  /// \brief The path to the module cache.
  std::string ModuleCachePath;
  
  /// \brief All of the preprocessor-specific data about files that are
  /// included, indexed by the FileEntry's UID.
  std::vector<HeaderFileInfo> FileInfo;

  /// Keeps track of each lookup performed by LookupFile.
  struct LookupFileCacheInfo {
    /// Starting index in SearchDirs that the cached search was performed from.
    /// If there is a hit and this value doesn't match the current query, the
    /// cache has to be ignored.
    unsigned StartIdx;
    /// The entry in SearchDirs that satisfied the query.
    unsigned HitIdx;
    /// This is non-null if the original filename was mapped to a framework
    /// include via a headermap.
    const char *MappedName;

    /// Default constructor -- Initialize all members with zero.
    LookupFileCacheInfo(): StartIdx(0), HitIdx(0), MappedName(nullptr) {}

    void reset(unsigned StartIdx) {
      this->StartIdx = StartIdx;
      this->MappedName = nullptr;
    }
  };
  llvm::StringMap<LookupFileCacheInfo, llvm::BumpPtrAllocator> LookupFileCache;

  /// \brief Collection mapping a framework or subframework
  /// name like "Carbon" to the Carbon.framework directory.
  llvm::StringMap<FrameworkCacheEntry, llvm::BumpPtrAllocator> FrameworkMap;

  /// IncludeAliases - maps include file names (including the quotes or
  /// angle brackets) to other include file names.  This is used to support the
  /// include_alias pragma for Microsoft compatibility.
  typedef llvm::StringMap<std::string, llvm::BumpPtrAllocator>
    IncludeAliasMap;
  std::unique_ptr<IncludeAliasMap> IncludeAliases;

  /// HeaderMaps - This is a mapping from FileEntry -> HeaderMap, uniquing
  /// headermaps.  This vector owns the headermap.
  std::vector<std::pair<const FileEntry*, const HeaderMap*> > HeaderMaps;

  /// \brief The mapping between modules and headers.
  mutable ModuleMap ModMap;
  
  /// \brief Describes whether a given directory has a module map in it.
  llvm::DenseMap<const DirectoryEntry *, bool> DirectoryHasModuleMap;

  /// \brief Set of module map files we've already loaded, and a flag indicating
  /// whether they were valid or not.
  llvm::DenseMap<const FileEntry *, bool> LoadedModuleMaps;

  /// \brief Uniqued set of framework names, which is used to track which 
  /// headers were included as framework headers.
  llvm::StringSet<llvm::BumpPtrAllocator> FrameworkNames;
  
  /// \brief Entity used to resolve the identifier IDs of controlling
  /// macros into IdentifierInfo pointers, and keep the identifire up to date,
  /// as needed.
  ExternalPreprocessorSource *ExternalLookup;

  /// \brief Entity used to look up stored header file information.
  ExternalHeaderFileInfoSource *ExternalSource;
  
  // Various statistics we track for performance analysis.
  unsigned NumIncluded;
  unsigned NumMultiIncludeFileOptzn;
  unsigned NumFrameworkLookups, NumSubFrameworkLookups;

  const LangOptions &LangOpts;

  // HeaderSearch doesn't support default or copy construction.
  HeaderSearch(const HeaderSearch&) = delete;
  void operator=(const HeaderSearch&) = delete;

  friend class DirectoryLookup;
  
public:
  HeaderSearch(IntrusiveRefCntPtr<HeaderSearchOptions> HSOpts,
               SourceManager &SourceMgr, DiagnosticsEngine &Diags,
               const LangOptions &LangOpts, const TargetInfo *Target);
  ~HeaderSearch();

  /// \brief Retrieve the header-search options with which this header search
  /// was initialized.
  HeaderSearchOptions &getHeaderSearchOpts() const { return *HSOpts; }
  
  FileManager &getFileMgr() const { return FileMgr; }

  /// \brief Interface for setting the file search paths.
  void SetSearchPaths(const std::vector<DirectoryLookup> &dirs,
                      unsigned angledDirIdx, unsigned systemDirIdx,
                      bool noCurDirSearch) {
    assert(angledDirIdx <= systemDirIdx && systemDirIdx <= dirs.size() &&
        "Directory indicies are unordered");
    SearchDirs = dirs;
    AngledDirIdx = angledDirIdx;
    SystemDirIdx = systemDirIdx;
    NoCurDirSearch = noCurDirSearch;
    //LookupFileCache.clear();
  }

  /// \brief Add an additional search path.
  void AddSearchPath(const DirectoryLookup &dir, bool isAngled) {
    unsigned idx = isAngled ? SystemDirIdx : AngledDirIdx;
    SearchDirs.insert(SearchDirs.begin() + idx, dir);
    if (!isAngled)
      AngledDirIdx++;
    SystemDirIdx++;
  }

  /// \brief Set the list of system header prefixes.
  void SetSystemHeaderPrefixes(ArrayRef<std::pair<std::string, bool> > P) {
    SystemHeaderPrefixes.assign(P.begin(), P.end());
  }

  /// \brief Checks whether the map exists or not.
  bool HasIncludeAliasMap() const { return (bool)IncludeAliases; }

  /// \brief Map the source include name to the dest include name.
  ///
  /// The Source should include the angle brackets or quotes, the dest 
  /// should not.  This allows for distinction between <> and "" headers.
  void AddIncludeAlias(StringRef Source, StringRef Dest) {
    if (!IncludeAliases)
      IncludeAliases.reset(new IncludeAliasMap);
    (*IncludeAliases)[Source] = Dest;
  }

  /// MapHeaderToIncludeAlias - Maps one header file name to a different header
  /// file name, for use with the include_alias pragma.  Note that the source
  /// file name should include the angle brackets or quotes.  Returns StringRef
  /// as null if the header cannot be mapped.
  StringRef MapHeaderToIncludeAlias(StringRef Source) {
    assert(IncludeAliases && "Trying to map headers when there's no map");

    // Do any filename replacements before anything else
    IncludeAliasMap::const_iterator Iter = IncludeAliases->find(Source);
    if (Iter != IncludeAliases->end())
      return Iter->second;
    return StringRef();
  }

  /// \brief Set the path to the module cache.
  void setModuleCachePath(StringRef CachePath) {
    ModuleCachePath = CachePath;
  }
  
  /// \brief Retrieve the path to the module cache.
  StringRef getModuleCachePath() const { return ModuleCachePath; }

  /// \brief Consider modules when including files from this directory.
  void setDirectoryHasModuleMap(const DirectoryEntry* Dir) {
    DirectoryHasModuleMap[Dir] = true;
  }
  
  /// \brief Forget everything we know about headers so far.
  void ClearFileInfo() {
    FileInfo.clear();
  }

  void SetExternalLookup(ExternalPreprocessorSource *EPS) {
    ExternalLookup = EPS;
  }

  ExternalPreprocessorSource *getExternalLookup() const {
    return ExternalLookup;
  }
  
  /// \brief Set the external source of header information.
  void SetExternalSource(ExternalHeaderFileInfoSource *ES) {
    ExternalSource = ES;
  }
  
  /// \brief Set the target information for the header search, if not
  /// already known.
  void setTarget(const TargetInfo &Target);
  
  /// \brief Given a "foo" or \<foo> reference, look up the indicated file,
  /// return null on failure.
  ///
  /// \returns If successful, this returns 'UsedDir', the DirectoryLookup member
  /// the file was found in, or null if not applicable.
  ///
  /// \param IncludeLoc Used for diagnostics if valid.
  ///
  /// \param isAngled indicates whether the file reference is a <> reference.
  ///
  /// \param CurDir If non-null, the file was found in the specified directory
  /// search location.  This is used to implement \#include_next.
  ///
  /// \param Includers Indicates where the \#including file(s) are, in case
  /// relative searches are needed. In reverse order of inclusion.
  ///
  /// \param SearchPath If non-null, will be set to the search path relative
  /// to which the file was found. If the include path is absolute, SearchPath
  /// will be set to an empty string.
  ///
  /// \param RelativePath If non-null, will be set to the path relative to
  /// SearchPath at which the file was found. This only differs from the
  /// Filename for framework includes.
  ///
  /// \param SuggestedModule If non-null, and the file found is semantically
  /// part of a known module, this will be set to the module that should
  /// be imported instead of preprocessing/parsing the file found.
  const FileEntry *LookupFile(
      StringRef Filename, SourceLocation IncludeLoc, bool isAngled,
      const DirectoryLookup *FromDir, const DirectoryLookup *&CurDir,
      ArrayRef<std::pair<const FileEntry *, const DirectoryEntry *>> Includers,
      SmallVectorImpl<char> *SearchPath, SmallVectorImpl<char> *RelativePath,
      ModuleMap::KnownHeader *SuggestedModule, bool SkipCache = false);

  /// \brief Look up a subframework for the specified \#include file.
  ///
  /// For example, if \#include'ing <HIToolbox/HIToolbox.h> from
  /// within ".../Carbon.framework/Headers/Carbon.h", check to see if
  /// HIToolbox is a subframework within Carbon.framework.  If so, return
  /// the FileEntry for the designated file, otherwise return null.
  const FileEntry *LookupSubframeworkHeader(
      StringRef Filename,
      const FileEntry *RelativeFileEnt,
      SmallVectorImpl<char> *SearchPath,
      SmallVectorImpl<char> *RelativePath,
      ModuleMap::KnownHeader *SuggestedModule);

  /// \brief Look up the specified framework name in our framework cache.
  /// \returns The DirectoryEntry it is in if we know, null otherwise.
  FrameworkCacheEntry &LookupFrameworkCache(StringRef FWName) {
    return FrameworkMap[FWName];
  }

  /// \brief Mark the specified file as a target of of a \#include,
  /// \#include_next, or \#import directive.
  ///
  /// \return false if \#including the file will have no effect or true
  /// if we should include it.
  bool ShouldEnterIncludeFile(Preprocessor &PP, const FileEntry *File,
                              bool isImport, Module *CorrespondingModule);

  /// \brief Return whether the specified file is a normal header,
  /// a system header, or a C++ friendly system header.
  SrcMgr::CharacteristicKind getFileDirFlavor(const FileEntry *File) {
    return (SrcMgr::CharacteristicKind)getFileInfo(File).DirInfo;
  }

  /// \brief Mark the specified file as a "once only" file, e.g. due to
  /// \#pragma once.
  void MarkFileIncludeOnce(const FileEntry *File) {
    HeaderFileInfo &FI = getFileInfo(File);
    FI.isImport = true;
    FI.isPragmaOnce = true;
  }

  /// \brief Mark the specified file as a system header, e.g. due to
  /// \#pragma GCC system_header.
  void MarkFileSystemHeader(const FileEntry *File) {
    getFileInfo(File).DirInfo = SrcMgr::C_System;
  }

  /// \brief Mark the specified file as part of a module.
  void MarkFileModuleHeader(const FileEntry *File,
                            ModuleMap::ModuleHeaderRole Role,
                            bool IsCompiledModuleHeader);

  /// \brief Increment the count for the number of times the specified
  /// FileEntry has been entered.
  void IncrementIncludeCount(const FileEntry *File) {
    ++getFileInfo(File).NumIncludes;
  }

  /// \brief Mark the specified file as having a controlling macro.
  ///
  /// This is used by the multiple-include optimization to eliminate
  /// no-op \#includes.
  void SetFileControllingMacro(const FileEntry *File,
                               const IdentifierInfo *ControllingMacro) {
    getFileInfo(File).ControllingMacro = ControllingMacro;
  }

  /// \brief Return true if this is the first time encountering this header.
  bool FirstTimeLexingFile(const FileEntry *File) {
    return getFileInfo(File).NumIncludes == 1;
  }

  /// \brief Determine whether this file is intended to be safe from
  /// multiple inclusions, e.g., it has \#pragma once or a controlling
  /// macro.
  ///
  /// This routine does not consider the effect of \#import
  bool isFileMultipleIncludeGuarded(const FileEntry *File);

  /// CreateHeaderMap - This method returns a HeaderMap for the specified
  /// FileEntry, uniquing them through the 'HeaderMaps' datastructure.
  const HeaderMap *CreateHeaderMap(const FileEntry *FE);

  /// \brief Retrieve the name of the module file that should be used to 
  /// load the given module.
  ///
  /// \param Module The module whose module file name will be returned.
  ///
  /// \returns The name of the module file that corresponds to this module,
  /// or an empty string if this module does not correspond to any module file.
  std::string getModuleFileName(Module *Module);

  /// \brief Retrieve the name of the module file that should be used to 
  /// load a module with the given name.
  ///
  /// \param ModuleName The module whose module file name will be returned.
  ///
  /// \param ModuleMapPath A path that when combined with \c ModuleName
  /// uniquely identifies this module. See Module::ModuleMap.
  ///
  /// \returns The name of the module file that corresponds to this module,
  /// or an empty string if this module does not correspond to any module file.
  std::string getModuleFileName(StringRef ModuleName, StringRef ModuleMapPath);

  /// \brief Lookup a module Search for a module with the given name.
  ///
  /// \param ModuleName The name of the module we're looking for.
  ///
  /// \param AllowSearch Whether we are allowed to search in the various
  /// search directories to produce a module definition. If not, this lookup
  /// will only return an already-known module.
  ///
  /// \returns The module with the given name.
  Module *lookupModule(StringRef ModuleName, bool AllowSearch = true);

  /// \brief Try to find a module map file in the given directory, returning
  /// \c nullptr if none is found.
  const FileEntry *lookupModuleMapFile(const DirectoryEntry *Dir,
                                       bool IsFramework);
  
  void IncrementFrameworkLookupCount() { ++NumFrameworkLookups; }

  /// \brief Determine whether there is a module map that may map the header
  /// with the given file name to a (sub)module.
  /// Always returns false if modules are disabled.
  ///
  /// \param Filename The name of the file.
  ///
  /// \param Root The "root" directory, at which we should stop looking for
  /// module maps.
  ///
  /// \param IsSystem Whether the directories we're looking at are system
  /// header directories.
  bool hasModuleMap(StringRef Filename, const DirectoryEntry *Root,
                    bool IsSystem);
  
  /// \brief Retrieve the module that corresponds to the given file, if any.
  ///
  /// \param File The header that we wish to map to a module.
  ModuleMap::KnownHeader findModuleForHeader(const FileEntry *File) const;
  
  /// \brief Read the contents of the given module map file.
  ///
  /// \param File The module map file.
  /// \param IsSystem Whether this file is in a system header directory.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool loadModuleMapFile(const FileEntry *File, bool IsSystem);

  /// \brief Collect the set of all known, top-level modules.
  ///
  /// \param Modules Will be filled with the set of known, top-level modules.
  void collectAllModules(SmallVectorImpl<Module *> &Modules);

  /// \brief Load all known, top-level system modules.
  void loadTopLevelSystemModules();

private:
  /// \brief Retrieve a module with the given name, which may be part of the
  /// given framework.
  ///
  /// \param Name The name of the module to retrieve.
  ///
  /// \param Dir The framework directory (e.g., ModuleName.framework).
  ///
  /// \param IsSystem Whether the framework directory is part of the system
  /// frameworks.
  ///
  /// \returns The module, if found; otherwise, null.
  Module *loadFrameworkModule(StringRef Name, 
                              const DirectoryEntry *Dir,
                              bool IsSystem);

  /// \brief Load all of the module maps within the immediate subdirectories
  /// of the given search directory.
  void loadSubdirectoryModuleMaps(DirectoryLookup &SearchDir);

  /// \brief Return the HeaderFileInfo structure for the specified FileEntry.
  const HeaderFileInfo &getFileInfo(const FileEntry *FE) const {
    return const_cast<HeaderSearch*>(this)->getFileInfo(FE);
  }

public:
  /// \brief Retrieve the module map.
  ModuleMap &getModuleMap() { return ModMap; }
  
  unsigned header_file_size() const { return FileInfo.size(); }

  /// \brief Get a \c HeaderFileInfo structure for the specified \c FileEntry,
  /// if one exists.
  bool tryGetFileInfo(const FileEntry *FE, HeaderFileInfo &Result) const;

  // Used by external tools
  typedef std::vector<DirectoryLookup>::const_iterator search_dir_iterator;
  search_dir_iterator search_dir_begin() const { return SearchDirs.begin(); }
  search_dir_iterator search_dir_end() const { return SearchDirs.end(); }
  unsigned search_dir_size() const { return SearchDirs.size(); }

  search_dir_iterator quoted_dir_begin() const {
    return SearchDirs.begin();
  }
  search_dir_iterator quoted_dir_end() const {
    return SearchDirs.begin() + AngledDirIdx;
  }

  search_dir_iterator angled_dir_begin() const {
    return SearchDirs.begin() + AngledDirIdx;
  }
  search_dir_iterator angled_dir_end() const {
    return SearchDirs.begin() + SystemDirIdx;
  }

  search_dir_iterator system_dir_begin() const {
    return SearchDirs.begin() + SystemDirIdx;
  }
  search_dir_iterator system_dir_end() const { return SearchDirs.end(); }

  /// \brief Retrieve a uniqued framework name.
  StringRef getUniqueFrameworkName(StringRef Framework);
  
  void PrintStats();
  
  size_t getTotalMemory() const;

  static std::string NormalizeDashIncludePath(StringRef File,
                                              FileManager &FileMgr);

private:
  /// \brief Describes what happened when we tried to load a module map file.
  enum LoadModuleMapResult {
    /// \brief The module map file had already been loaded.
    LMM_AlreadyLoaded,
    /// \brief The module map file was loaded by this invocation.
    LMM_NewlyLoaded,
    /// \brief There is was directory with the given name.
    LMM_NoDirectory,
    /// \brief There was either no module map file or the module map file was
    /// invalid.
    LMM_InvalidModuleMap
  };

  LoadModuleMapResult loadModuleMapFileImpl(const FileEntry *File,
                                            bool IsSystem,
                                            const DirectoryEntry *Dir);

  /// \brief Try to load the module map file in the given directory.
  ///
  /// \param DirName The name of the directory where we will look for a module
  /// map file.
  /// \param IsSystem Whether this is a system header directory.
  /// \param IsFramework Whether this is a framework directory.
  ///
  /// \returns The result of attempting to load the module map file from the
  /// named directory.
  LoadModuleMapResult loadModuleMapFile(StringRef DirName, bool IsSystem,
                                        bool IsFramework);

  /// \brief Try to load the module map file in the given directory.
  ///
  /// \param Dir The directory where we will look for a module map file.
  /// \param IsSystem Whether this is a system header directory.
  /// \param IsFramework Whether this is a framework directory.
  ///
  /// \returns The result of attempting to load the module map file from the
  /// named directory.
  LoadModuleMapResult loadModuleMapFile(const DirectoryEntry *Dir,
                                        bool IsSystem, bool IsFramework);

  /// \brief Return the HeaderFileInfo structure for the specified FileEntry.
  HeaderFileInfo &getFileInfo(const FileEntry *FE);
};

}  // end namespace clang

#endif
